#ifndef __SIMPLE_DRIVER_HH__
#define __SIMPLE_DRIVER_HH__

#include <cassert>
#include <cstdint>
#include <set>
#include <unordered_map>

#include "params/SimpleDriver.hh"
#include "base/addr_range_map.hh"
#include "mem/request.hh"
#include "sim/emul_driver.hh"
#include "device/simple_deviceobj.hh"

namespace gem5
{

#define PAGE_SHIFT 0xc


//class SimpleDeviceObj;
class SimpleDriver : public EmulatedDriver
{
    public:
        SimpleDriver(const SimpleDriverParams &p);
        int ioctl(ThreadContext *tc, unsigned req, Addr ioc_buf) override
            {  panic("ioctl not implement!");   }
        int open(ThreadContext *tc, int mode, int flags) override;
        Addr mmap(ThreadContext *tc, Addr start, uint64_t length,
              int prot, int tgt_flags, int tgt_fd, off_t offset) override;


        Addr deviceBaseAddr;
        //AddrRangeList deviceaddr;

};





} //namespace gem5



#endif