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
#include "clc.h"

#define _EXPAND_SIZES(type)             \
    IMPLEMENTATION (_VEC_TYPE(type,2), type)  \
    IMPLEMENTATION (_VEC_TYPE(type,3), type)  \
    IMPLEMENTATION (_VEC_TYPE(type,4), type)  \
    IMPLEMENTATION (_VEC_TYPE(type,8), type)  \
    IMPLEMENTATION (_VEC_TYPE(type,16), type) \

#define IMPLEMENTATION(gentype, sgentype) \
_CLC_OVERLOAD _CLC_DEF gentype clamp(gentype x, gentype minval, gentype maxval)  \
    { return x > maxval ? maxval : x < minval ? minval : x; } \
_CLC_OVERLOAD _CLC_DEF gentype clamp(gentype x, sgentype minval, sgentype maxval) \
    { return x > (gentype)maxval ? (gentype)maxval : x < (gentype)minval ? (gentype)minval : x; } \

_EXPAND_TYPES()
