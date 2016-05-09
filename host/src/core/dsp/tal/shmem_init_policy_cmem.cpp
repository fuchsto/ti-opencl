/******************************************************************************
 * Copyright (c) 2016, Texas Instruments Incorporated - http://www.ti.com/
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *      * Neither the name of Texas Instruments Incorporated nor the
 *        names of its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include <ti/cmem.h>

#include "dspmem.h"
#include "shmem_init_policy_cmem.h"
#include "core/error_report.h"

using namespace tiocl;

static void cmem_init(std::vector<MemoryRange>& ranges);

void
InitializationPolicyCMEM::DiscoverMemoryRanges(std::vector<MemoryRange>& ranges)
{
    cmem_init(ranges);
}

void
InitializationPolicyCMEM::Destroy()
{
    ReportTrace("CMEM_exit() called\n");
    CMEM_exit();
}

/****************************************************************************
 * - 2 CMEM blocks - one for DDR, the other for on-chip shared memory
 * - If DDR CMEM block is > 1.5GB, split into two regions:
 *   o Persistent with 1.5GB
 *   o On-Demand with the remainder of the block
 *   This is to avoid mapping failures when mapping blocks > 1.5GB into the
 *   host's virtual address space
 ***************************************************************************/
void cmem_init(std::vector<MemoryRange>& ranges)
{
    const int CMEM_MIN_BLOCKS = 2;

    // Initialize the CMEM module
    if (CMEM_init() == -1)
        ReportError(ErrorType::Fatal, ErrorKind::CMEMInitFailed);

    // Valid CMEM configurations: last block for MPI (hyperlink/SRIO) buffers
    //     DDR1(OCL), MSMC2(OCL)
    // or  DDR1(OCL), MSMC2(OCL), DDR3(MPI)
    int num_Blocks = 0;
    CMEM_getNumBlocks(&num_Blocks);
    if (num_Blocks < CMEM_MIN_BLOCKS)
        ReportError(ErrorType::Fatal, ErrorKind::CMEMMinBlocks);

    CMEM_BlockAttrs pattrs0 = {0, 0};
    CMEM_getBlockAttrs(0, &pattrs0);

    DSPDevicePtr64 ddr_addr = pattrs0.phys_base;
    uint64_t       ddr_size = pattrs0.size;

    // Persistent CMEM should start within 0x8:2200_0000 - 0x8:4000_0000
    if (ddr_addr >= MPAX_USER_MAPPED_DSP_ADDR)
        ReportError(ErrorType::Fatal, ErrorKind::CMEMAllocFailed,
                    "Persistent", pattrs0.phys_base);

    // Grab all available CMEM physical address, to be managed by OCL
    CMEM_AllocParams params = CMEM_DEFAULTPARAMS;
    params.flags            = CMEM_CACHED;
    params.type             = CMEM_POOL;

    DSPDevicePtr64 alloc_dsp_addr = CMEM_allocPoolPhys2(0, 0, &params);

    if (!alloc_dsp_addr || alloc_dsp_addr != ddr_addr)
        ReportError(ErrorType::Fatal, ErrorKind::CMEMAllocFromBlockFailed,
               ddr_size, 0, alloc_dsp_addr);


    DSPDevicePtr64 addr1 = ddr_addr;
    uint64_t       size1 = ddr_size;
    DSPDevicePtr64 addr2 = 0;
    uint64_t       size2 = 0;

    // If the CMEM block goes past the 2GB limit, break it up into
    // persistent and on demand ranges. The On Demand range starts
    // at 1GB.
    if (ddr_addr + ddr_size > ALL_PERSISTENT_MAX_DSP_ADDR)
    {
        size2 = ddr_addr + ddr_size - MPAX_USER_MAPPED_DSP_ADDR;
        size1 = ddr_size - size2;
        addr2 = ddr_addr + size1;

        ReportTrace("CMEM splitting into 2 chunks: (0x%llx, %lld MB), (0x%llx, %lld MB)\n",
                    addr1, size1 >> 20, addr2, size2 >> 20);
    }

    // translate first chunk to using 32-bit aliased physical addresses
    if (ddr_addr > DSP_36BIT_ADDR)
    {
        addr1 = ddr_addr + 0xA0000000 - 0x820000000ULL;

        // If the ddr size is greater than we can currently support, limit it
        uint64_t ddr_size_limit = ALL_PERSISTENT_MAX_DSP_ADDR - ddr_addr;
        if (size1 > ddr_size_limit)
            size1 = ddr_size_limit;
    }

    #if defined (DEVICE_AM57)
    // On AM57, reserve The first 32MB for the monitor.  We use this memory
    // for a shared heap and nocache DDR memory.
    // TODO: Define a separate region in the dts file for use by monitor
    addr1 += RESERVED_CMEM_SIZE;
    size1 -= RESERVED_CMEM_SIZE;

    // Also adjust for DSP virtual to physical translation
    addr1 = addr1 - AM57_DSP_V2P_OFFSET;
    #endif

    ranges.emplace_back(addr1, size1,
                        MemoryRange::Kind::CMEM_PERSISTENT,
                        MemoryRange::Location::OFFCHIP);

    if (size2 > 0)
        ranges.emplace_back(addr2, size2,
                            MemoryRange::Kind::CMEM_ONDEMAND,
                            MemoryRange::Location::OFFCHIP);

    // Handle CMEM Block 1 - on chip shared memory
    CMEM_BlockAttrs pattrs1 = {0, 0};
    CMEM_getBlockAttrs(1, &pattrs1);

    DSPDevicePtr64 onchip_shared_addr = pattrs1.phys_base;
    uint64_t       onchip_shared_size = pattrs1.size;
    if (onchip_shared_addr < MSMC_OCL_START_ADDR ||
        onchip_shared_addr >= MSMC_OCL_END_ADDR)
        ReportError(ErrorType::Fatal, ErrorKind::CMEMAllocFailed,
                    "On-chip Shared Memory", pattrs1.phys_base);


    params.type    = CMEM_HEAP;
    alloc_dsp_addr = CMEM_allocPhys2(1, onchip_shared_size, &params);
    if (!alloc_dsp_addr || alloc_dsp_addr != onchip_shared_addr)
        ReportError(ErrorType::Fatal, ErrorKind::CMEMAllocFromBlockFailed,
               onchip_shared_size, 0, alloc_dsp_addr);

    ranges.emplace_back(onchip_shared_addr, onchip_shared_size,
                        MemoryRange::Kind::CMEM_PERSISTENT,
                        MemoryRange::Location::ONCHIP);
}
