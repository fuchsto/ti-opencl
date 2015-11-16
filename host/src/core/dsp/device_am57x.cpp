/******************************************************************************
 * Copyright (c) 2013-2015, Texas Instruments Incorporated - http://www.ti.com/
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met:
 *       * Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *       * Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in the
 *         documentation and/or other materials provided with the distribution.
 *       * Neither the name of Texas Instruments Incorporated nor the
 *         names of its contributors may be used to endorse or promote products
 *         derived from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *   THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/
#include "mbox_impl_msgq.h"

/******************************************************************************
* DSPDevice::DSPDevice(unsigned char dsp_id)
******************************************************************************/
DSPDevice::DSPDevice(unsigned char dsp_id)
    : DeviceInterface   (), 
      p_core_mail       (1), 
      p_cores           (0),
      p_num_events      (0), 
      p_dsp_mhz         (600), // 600 MHz 
      p_worker_dispatch  (0), 
      p_worker_completion(0), 
      p_stop            (false),
      p_exit_acked      (false),
      p_initialized     (false), 
      p_dsp_id          (dsp_id), 
      p_device_msmc_heap(),
      p_device_ddr_heap1(),
      p_device_ddr_heap2(),
      p_device_ddr_heap3(),
      p_dload_handle    (0),
      p_complete_pending(),
      p_mpax_default_res(NULL)
{ 
    Driver *driver = Driver::instance();

    p_cores = driver->cores_per_dsp(dsp_id);

    void *hdl = driver->create_image_handle(dsp_id);

    p_addr_local_mem     = driver->get_symbol(hdl, "ocl_local_mem_start");
    p_size_local_mem     = driver->get_symbol(hdl, "ocl_local_mem_size");

    p_addr_kernel_config = driver->get_symbol(hdl, "kernel_config_l2");

    driver->free_image_handle(hdl);

    p_addr64_global_mem = 0;
    p_size64_global_mem = 0;
    p_addr_msmc_mem     = 0;
    p_size_msmc_mem     = 0;

    DSPDevicePtr64 global2 = 0;
    uint64_t       gsize2  = 0;
    DSPDevicePtr64 global3 = 0;
    uint64_t       gsize3  = 0;

    driver->cmem_init(&p_addr64_global_mem, &p_size64_global_mem,
                          &p_addr_msmc_mem,     &p_size_msmc_mem,
                          &global3,             &gsize3);


    // cmem_init returns 0xA000_0000 as the base address. This corresponds to
    // the physical address generated by the DSP MMU. The device address
    // generated by the DSP is 0x8000_0000. MMU setup is performed using a
    // table specified in opencl_monitor_vayu/custom_rsc_table_vayu_dsp.h
    // remoteproc uses the contents of this table to initialize the DSP MMU
    p_addr64_global_mem = p_addr64_global_mem - 0x20000000;

    DSPDevicePtr64 global1 = p_addr64_global_mem;
    uint64_t       gsize1  = p_size64_global_mem;
    driver->split_ddr_memory(p_addr64_global_mem, p_size64_global_mem,
                             global1, gsize1, global2, gsize2, gsize3);

    driver->shmem_configure(global1,              gsize1, 0);
    if (gsize2 > 0) driver->shmem_configure(global2, gsize2, 0);
    if (gsize3 > 0) driver->shmem_configure(global3, gsize3, 0);
    if (p_size_msmc_mem > 0) driver->shmem_configure(p_addr_msmc_mem,
                                                     p_size_msmc_mem, 1);

    for (int core = 0; core < dspCores(); core++)
        driver->shmem_configure(((0x80 + core) << (3+20)) + p_addr_local_mem,
                                p_size_local_mem);

    /*-------------------------------------------------------------------------
    * Setup the DSP heaps for memory allocation
    *------------------------------------------------------------------------*/
    p_device_ddr_heap1.configure(global1,          gsize1);
    p_device_ddr_heap2.configure(global2,          gsize2, true);
    p_device_ddr_heap3.configure(global3,          gsize3, true);
    p_device_msmc_heap.configure(p_addr_msmc_mem,  p_size_msmc_mem);

    init_ulm(gsize1, gsize2, gsize3);

    /*-------------------------------------------------------------------------
    * initialize the mailboxes on the cores, so they can receive an exit cmd
    *------------------------------------------------------------------------*/
    setup_mailbox();

    setup_dsp_mhz();
}

void DSPDevice::setup_mailbox(void)
{
    p_mb = new MBoxMsgQ(this);
}


void DSPDevice::setup_dsp_mhz(void)
{
    mail_to(frequencyMsg);

    int ret = 0;
    do
    {
        while (!mail_query())  ;
        ret = mail_from();
    } while (ret == -1);

    p_dsp_mhz = ret;
}

