#include <cstring>
#include "shmem_rw_policy_rtos.h"
#include "core/error_report.h"
#include <ti/sysbios/family/arm/a15/Cache.h>


using namespace tiocl;

void
ReadWritePolicyRTOS::Configure(int32_t device_id,
                               const tiocl::MemoryProviderFactory* mpf)
{
    device_id_ = device_id;
    mp_factory_ = mpf;
}


int32_t 
ReadWritePolicyRTOS::Write(uint64_t dst, uint8_t *src, size_t size)
{
    const MemoryProvider* region = mp_factory_->GetMemoryProvider(dst);
    void* dst_host_addr = region->MapToHostAddressSpace(dst, size, false);
    if (dst_host_addr) memcpy((char*)dst_host_addr, src, size);
    else 
        ReportError(ErrorType::Fatal, 
                    ErrorKind::UnableToMapDSPAddress, "for write");
    region->UnmapFromHostAddressSpace(dst_host_addr, size, true);

    return 0;
}

int32_t 
ReadWritePolicyRTOS::Read(uint64_t src, uint8_t *dst, size_t size)
{
    const MemoryProvider* region = mp_factory_->GetMemoryProvider(src);
    void* dst_host_addr = region->MapToHostAddressSpace(src, size, true);
    if (dst_host_addr) memcpy(dst, (char*)dst_host_addr, size);
    else 
        ReportError(ErrorType::Fatal, 
                    ErrorKind::UnableToMapDSPAddress, "for read");
    region->UnmapFromHostAddressSpace(dst_host_addr, size, false);

    return 0;
}

void*   
ReadWritePolicyRTOS::Map(uint64_t addr, size_t sz,
                         bool is_read, bool allow_fail)
{
    const MemoryProvider* region = mp_factory_->GetMemoryProvider(addr);
    void* host_addr = region->MapToHostAddressSpace(addr, sz, is_read);
    if (host_addr == NULL && !allow_fail)
        ReportError(ErrorType::Fatal, 
                    ErrorKind::UnableToMapDSPAddress, "");
    return host_addr;
}

int32_t 
ReadWritePolicyRTOS::Unmap(void *host_addr, uint64_t buf_addr, size_t sz,
                           bool is_write)
{
    const MemoryProvider* region = mp_factory_->GetMemoryProvider(buf_addr);
    region->UnmapFromHostAddressSpace(host_addr, sz, is_write);
    return 0;
}

bool ReadWritePolicyRTOS::CacheWbInvAll()
{
    Cache_wbInvAll();
    return true;
}
