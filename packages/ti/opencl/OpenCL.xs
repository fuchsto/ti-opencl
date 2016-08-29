/*
 * Copyright (c) 2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/

var OpenCL        = null;


/*
 *  ======== module$meta$init ========
 */
function module$meta$init()
{
    /* Only process during "cfg" phase */
    if (xdc.om.$name != "cfg") {
        return;
    }

    OpenCL = this;
}

/*
 *  ======== module$use ========
 */
function module$use()
{
    var BIOS = xdc.useModule("ti.sysbios.BIOS");
    var BIOS = xdc.useModule("ti.sdo.ipc.Ipc");
    if (OpenCL.OCL_ipc_customized == false)
    {
        var ipc_cfg = xdc.loadCapsule("ti/opencl/ipc.cfg.xs");
    }

    if (OpenCL.OCL_memory_customized == false)
    {
        var Program = xdc.useModule("xdc.cfg.Program");
        OpenCL.OCL_HOSTPROG_base = Program.cpu.memoryMap["HOST_PROG"].base;
        OpenCL.OCL_HOSTPROG_len  = Program.cpu.memoryMap["HOST_PROG"].len;
        OpenCL.OCL_GLOBAL_base   = Program.cpu.memoryMap["OCL_GLOBAL"].base;
        OpenCL.OCL_GLOBAL_len    = Program.cpu.memoryMap["OCL_GLOBAL"].len;
        OpenCL.OCL_LOCAL_base    = Program.platform.$module["OCL_LOCAL"].base;
        OpenCL.OCL_LOCAL_len     = Program.platform.$module["OCL_LOCAL"].len;
    }
}


/*
 *  ======== module$static$init ========
 */
function module$static$init(mod, params)
{
}



/*
 *  ======== module$validate ========
 */
function module$validate()
{
}
