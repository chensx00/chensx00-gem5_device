#include "device/simple_driver.hh"

#include <memory>
#include "cpu/thread_context.hh"
#include "base/compiler.hh"
#include "base/logging.hh"
#include "base/trace.hh"
#include "base/flags.hh"

#include "mem/port_proxy.hh"
#include "mem/se_translating_port_proxy.hh"
#include "mem/translating_port_proxy.hh"

#include "sim/process.hh"
#include "sim/se_workload.hh"
#include "sim/syscall_emul_buf.hh"
#include "sim/fd_entry.hh"

#include "debug/Device_Driver.hh"

namespace gem5
{

SimpleDriver::SimpleDriver(const SimpleDriverParams &p) : 
    EmulatedDriver(p),
    deviceBaseAddr(p.deviceBaseAddr)
    
{
    DPRINTF(Device_Driver,"SimpleDriver constructor\n");    
    

}

int
SimpleDriver::open(ThreadContext *tc, int mode, int flags)
{
    DPRINTF(Device_Driver,"opend %s\n",filename);

    Process* process = tc->getProcessPtr();
    auto device_fd_entry = std::make_shared<DeviceFDEntry>(this, filename);
    int tgt_fd = process->fds->allocFD(device_fd_entry);
    DPRINTF(Device_Driver,"fd =  %d\n",tgt_fd);
    return tgt_fd;


}

Addr
SimpleDriver::mmap(ThreadContext *tc, Addr start, uint64_t length,
                    int prot, int tgt_flags, int tgt_fd, off_t offset)
{
    
    Process * process = tc->getProcessPtr();
    auto mem_state = process->memState;

    //Addr pg_off = offset >> PAGE_SHIFT;
    
    
    
    start = mem_state->extendMmap(length);
    process->pTable->map(start, deviceBaseAddr, length, false);

    DPRINTF(Device_Driver,"mmap : start = %#x, length = %#x, offset = %#x, paddr = %#x\n",start,length,offset,deviceBaseAddr);


    return start;



}



} //namespace gem5

