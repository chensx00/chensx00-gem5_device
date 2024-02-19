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

#ifndef __COMODEL_MMIO_DRIVER_HH__
#define __COMODEL_MMIO_DRIVER_HH__

#include "cpu/thread_context.hh"
#include "params/EmulatedMMIODriver.hh"
#include "sim/emul_driver.hh"

namespace gem5
{

/**
 * Emulated driver for MMIO devices in SE mode. This driver maps the physical
 * address range for a device, such as an accelerator, and thus enables direct
 * access to the device in user mode.
 */
class EmulatedMMIODriver : public EmulatedDriver
{
  public:
    EmulatedMMIODriver(const EmulatedMMIODriverParams &params)
        : EmulatedDriver(params),
          mmio_phy_addr(params.mmio_addr),
          mmio_size(params.mmio_size)
    { }

    /**
     * Invoked when the user program calls open() on the device driver.
     * The parameters are the same as those passed to openFunc() (q.v.).
     * @return A newly allocated target fd, or -1 on error.
     */
    int open(ThreadContext *tc, int mode, int flags) override;

    /**
     * Invoked when the user program calls ioctl() on the file descriptor
     * returned by a previous open().
     * No ioctl is supported by this driver.
     */
    int ioctl(ThreadContext *tc, unsigned req, Addr buf) override
    { return -EBADF; }

    /**
     * Invoked when the user program calls mmap() on the file descriptor
     * returned by a previous open().  The parameters are the same as those
     * passed in to mmapFunc() (q.v.).
     * @return The return ptr for the mmap, or the negation of the errno
     * (see the SyscallReturn class).
     */
    Addr mmap(ThreadContext *tc, Addr start, uint64_t length,
              int prot, int tgt_flags, int tgt_fd, off_t offset) override;

  protected:
    /** Physical address to be mapped. */
    Addr mmio_phy_addr;

    /** Size of the device's address range. */
    Addr mmio_size;
};

} // namespace gem5

#endif // __COMODEL_MMIO_DRIVER_HH__
