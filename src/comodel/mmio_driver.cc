/*
 * Co-model high level simulation and low level implementation in gem5.
 *
 * Emulated driver for MMIO devices in SE mode.
 *
 * Shixuan Chen
 * Chun Yang    <yangchun@mprc.pku.edu.cn>
 *
 * Copyright (C) 2024 Microprocessor R&D Center (MPRC), Peking University
 * All rights reserved.
 */

#include "comodel/mmio_driver.hh"

#include <sys/mman.h>       // PROT_READ, PROT_WRITE

#include "sim/fd_entry.hh"
#include "sim/process.hh"

namespace gem5
{

int
EmulatedMMIODriver::open(ThreadContext *tc, int mode, int flags)
{
    auto process = tc->getProcessPtr();
    auto device_fd_entry = std::make_shared<DeviceFDEntry>(this, filename);
    int tgt_fd = process->fds->allocFD(device_fd_entry);
    return tgt_fd;
}

Addr
EmulatedMMIODriver::mmap(ThreadContext *tc, Addr start, uint64_t length,
                         int prot, int tgt_flags, int tgt_fd, off_t offset)
{
    // Align size to page and check input length.
    enum { PAGE_SHIFT = 12, PAGE_SIZE = 1 << PAGE_SHIFT };
    Addr aligned_size = mmio_size + PAGE_SIZE - 1;
    aligned_size = (aligned_size >> PAGE_SHIFT) << PAGE_SHIFT;
    if (length > aligned_size) {
        return -EINVAL;
    }

    warn_if(prot != (PROT_READ | PROT_WRITE),
            "EmulatedMMIODriver supports PROT_READ|PROT_WRITE only.\n");

    auto process = tc->getProcessPtr();
    start = process->memState->extendMmap(length);
    process->pTable->map(start, mmio_phy_addr, length, false);
    inform("EmulatedMMIODriver mapped [phy %#x] to [virt %#x].\n",
           mmio_phy_addr, start);

    return start;
}

} // namespace gem5
