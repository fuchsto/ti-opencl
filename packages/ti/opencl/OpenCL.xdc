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
 */

package ti.opencl;

import xdc.runtime.Types;

/*!
 *  ======== OpenCL ========
 *  OpenCL RTSC Configuration Module
 *
 *  This module must be used (via xdc.useModule) by all OpenCL RTSC
 *  applications. This module is used to setup a configuration for the
 *  runtime.
 *
 */

module OpenCL
{
    /*!
     *  ======== computeUnitList ========
     *  List of cores/compute units available to the runtime
     *  On AM57, defaults to both cores. Override with "0" or "1" for DSP0 or DSP1 only
     *
     */
    config String   computeUnitList        = "0, 1";

    /*!
     *  ======== OCL_GLOBAL_customized ========
     *  ======== OCL_GLOBAL_base ========
     *  ======== OCL_GLOBAL_len ========
     *
     *  Can be curstomized.  If customized is set to true, then base and len
     *  must be given valid values, otherwise, default platform values are used.
     */
    config Bool     OCL_GLOBAL_customized  = false;
    config UInt32   OCL_GLOBAL_base        = 0;
    config UInt32   OCL_GLOBAL_len         = 0;

internal:

    /*!
     *  ======== OCL_HOSTPROG_base ========
     *  ======== OCL_HOSTPROG_len ========
     *
     *  Can NOT be curstomized for now.
     *  If customizable, we are essentially relocating where HOST_PROG is
     *  from the platform file (code, data, stack).  TODO: We may need to
     *  program Host's MMU accordingly.
     */
    config UInt32   OCL_HOSTPROG_base        = 0;
    config UInt32   OCL_HOSTPROG_len         = 0;

    /*!
     *  ======== OCL_LOCAL_base ========
     *  ======== OCL_LOCAL_len ========
     *
     *  Can NOT be curstomized.  Must be modified in the platform file because
     *  it needs to be in sync with the DSP monitor build.
     */
    config UInt32   OCL_LOCAL_base        = 0;
    config UInt32   OCL_LOCAL_len         = 0;
}

