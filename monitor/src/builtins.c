/******************************************************************************
 * Copyright (c) 2013-2014, Texas Instruments Incorporated - http://www.ti.com/
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
#include <stddef.h>
#include "monitor.h"
#include "util.h"
#include <c6x.h>

extern uint32_t ocl_l1d_mem_start;
extern uint32_t ocl_l1d_mem_size;

far void*     l1d_start = (void*)    &ocl_l1d_mem_start;
far uint32_t l1d_size   = (uint32_t) &ocl_l1d_mem_size;

/******************************************************************************
* __core_num()
******************************************************************************/
EXPORT int __core_num() { return DNUM; }

/*-----------------------------------------------------------------------------
* Variant across DSP cores, so place in L2.
*----------------------------------------------------------------------------*/
static far PRIVATE(size_t, l1d_scratch_size) = 0;

EXPORT void*  __scratch_l1d_start() { return l1d_start; }
EXPORT size_t __scratch_l1d_size()  { return l1d_scratch_size; }

EXPORT uint32_t __cache_l1d_size()
{
    switch (CACHE_getL1DSize())
    {
        case CACHE_L1_0KCACHE:  return 0;
        case CACHE_L1_4KCACHE:  return (4  << 10);
        case CACHE_L1_8KCACHE:  return (8  << 10);
        case CACHE_L1_16KCACHE: return (16 << 10);
        case CACHE_L1_32KCACHE: return (32 << 10);
        default:                return (32 << 10);
    }
}

EXPORT uint32_t __cache_l2_size()
{
    switch (CACHE_getL2Size())
    {
        case CACHE_0KCACHE:    return 0;
        case CACHE_32KCACHE:   return (32   << 10);
        case CACHE_64KCACHE:   return (64   << 10);
        case CACHE_128KCACHE:  return (128  << 10);
        case CACHE_256KCACHE:  return (256  << 10);
        case CACHE_512KCACHE:  return (512  << 10);
        case CACHE_1024KCACHE: return (1024 << 10);
        default:               return (1024 << 10);
    }
}

EXPORT int __cache_l1d_none()
{
    CACHE_wbInvAllL1d(CACHE_NOWAIT);
    __mfence();
    CACHE_setL1DSize(CACHE_L1_0KCACHE);
    CACHE_getL1DSize();
    l1d_scratch_size = l1d_size;
    return 1;
}

EXPORT int __cache_l1d_all()
{
    CACHE_setL1DSize(CACHE_L1_32KCACHE);
    CACHE_getL1DSize();
    l1d_scratch_size = 0;
    return 1;
}

EXPORT int __cache_l1d_4k()
{
    CACHE_wbInvAllL1d(CACHE_NOWAIT);
    __mfence();
    CACHE_setL1DSize(CACHE_L1_4KCACHE);
    CACHE_getL1DSize();
    l1d_scratch_size = l1d_size - (4 << 10);
    return 1;
}

EXPORT int __cache_l1d_8k()
{
    CACHE_wbInvAllL1d(CACHE_NOWAIT);
    __mfence();
    CACHE_setL1DSize(CACHE_L1_8KCACHE);
    CACHE_getL1DSize();
    l1d_scratch_size = l1d_size - (8 << 10);
    return 1;
}

EXPORT int __cache_l1d_16k()
{
    CACHE_wbInvAllL1d(CACHE_NOWAIT);
    __mfence();
    CACHE_setL1DSize(CACHE_L1_16KCACHE);
    CACHE_getL1DSize();
    l1d_scratch_size = l1d_size - (16 << 10);
    return 1;
}

EXPORT void __cache_l1d_flush()
{
    uint32_t lvInt = _disable_interrupts();
    CACHE_wbInvAllL1d(CACHE_NOWAIT);
    __mfence();
    CSL_XMC_invalidatePrefetchBuffer();
    __mfence();
    _restore_interrupts(lvInt);
}


/*-----------------------------------------------------------------------------
* for the l2 cache functions the __scratch_l2_size
* value is in kernel_config_l2[17]
*----------------------------------------------------------------------------*/
EXPORT extern kernel_config_t kernel_config_l2;

EXPORT void*    __scratch_l2_start (void) { return (void*)kernel_config_l2.L2_scratch_start; }
EXPORT uint32_t __scratch_l2_size  (void) { return kernel_config_l2.L2_scratch_size; }

EXPORT int __cache_l2_none()
{
    int32_t scratch_delta = __cache_l2_size(); // - (0 << 10);
    //uint32_t scratch_size  = kernel_config_l2.L2_scratch_size;
    //if (-scratch_delta > scratch_size) return 0;
    kernel_config_l2.L2_scratch_size += scratch_delta;

    CACHE_wbInvAllL2(CACHE_NOWAIT);
    __mfence();
    CACHE_setL2Size (CACHE_0KCACHE);
    CACHE_getL2Size ();
    return 1;
}

EXPORT int __cache_l2_32k()
{
    int32_t scratch_delta = __cache_l2_size() - (32 << 10);
    uint32_t scratch_size  = kernel_config_l2.L2_scratch_size;
    if (-scratch_delta > scratch_size) return 0;
    kernel_config_l2.L2_scratch_size += scratch_delta;

    CACHE_wbInvAllL2(CACHE_NOWAIT);
    __mfence();
    CACHE_setL2Size (CACHE_32KCACHE);
    CACHE_getL2Size ();
    return 1;
}

EXPORT int __cache_l2_64k()
{
    int32_t scratch_delta = __cache_l2_size() - (64 << 10);
    uint32_t scratch_size  = kernel_config_l2.L2_scratch_size;
    if (-scratch_delta > scratch_size) return 0;
    kernel_config_l2.L2_scratch_size += scratch_delta;

    CACHE_wbInvAllL2(CACHE_NOWAIT);
    __mfence();
    CACHE_setL2Size (CACHE_64KCACHE);
    CACHE_getL2Size ();
    return 1;
}

EXPORT int __cache_l2_128k()
{
    int32_t scratch_delta = __cache_l2_size() - (128 << 10);
    uint32_t scratch_size  = kernel_config_l2.L2_scratch_size;
    if (-scratch_delta > scratch_size) return 0;
    kernel_config_l2.L2_scratch_size += scratch_delta;

    CACHE_wbInvAllL2(CACHE_NOWAIT);
    __mfence();
    CACHE_setL2Size (CACHE_128KCACHE);
    CACHE_getL2Size ();
    return 1;
}

#ifdef DEVICE_AM572x
EXPORT int __cache_l2_256k() { return 0; }
EXPORT int __cache_l2_512k() { return 0; }
#else
EXPORT int __cache_l2_256k()
{
    int32_t scratch_delta = __cache_l2_size() - (256 << 10);
    uint32_t scratch_size  = kernel_config_l2.L2_scratch_size;
    if (-scratch_delta > scratch_size) return 0;
    kernel_config_l2.L2_scratch_size += scratch_delta;

    CACHE_wbInvAllL2(CACHE_NOWAIT);
    __mfence();
    CACHE_setL2Size (CACHE_256KCACHE);
    CACHE_getL2Size ();
    return 1;
}

EXPORT int __cache_l2_512k()
{
    int32_t scratch_delta = __cache_l2_size() - (512 << 10);
    uint32_t scratch_size  = kernel_config_l2.L2_scratch_size;
    if (-scratch_delta > scratch_size) return 0;
    kernel_config_l2.L2_scratch_size += scratch_delta;

    CACHE_wbInvAllL2(CACHE_NOWAIT);
    __mfence();
    CACHE_setL2Size (CACHE_512KCACHE);
    CACHE_getL2Size ();
    return 1;
}
#endif

EXPORT void __cache_l2_flush()
{
    uint32_t lvInt = _disable_interrupts();
    CACHE_wbInvAllL2(CACHE_NOWAIT);
    __mfence();
    CSL_XMC_invalidatePrefetchBuffer();
    __mfence();
    _restore_interrupts(lvInt);
}