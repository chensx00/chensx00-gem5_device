/*
 * Co-model high level simulation and low level implementation in gem5.
 *
 * Sample device: a simple counter
 * 1. Read  [0x0] -- get the counter value
 * 2. Write [0x0] -- reset the counter
 * 3. Write [0x4] -- add the counter by written value
 *
 * Chun Yang    <yangchun@mprc.pku.edu.cn>
 *
 * Copyright (C) 2024 Microprocessor R&D Center (MPRC), Peking University
 * All rights reserved.
 */

#ifndef __COMODEL_SAMPLE_COUNTER_HH__
#define __COMODEL_SAMPLE_COUNTER_HH__

#include "dev/io_device.hh"
#include "params/SampleCounter.hh"

namespace gem5
{

class SampleCounter : public BasicPioDevice
{
    enum { IO_SIZE = 8 };
  public:
    SampleCounter(const SampleCounterParams &params)
        : BasicPioDevice(params, IO_SIZE), counter(0)
    { }

    Tick read(PacketPtr pkt) override;
    Tick write(PacketPtr pkt) override;

  protected:
    unsigned int counter;
};

} // namespace gem5

#endif // __COMODEL_SAMPLE_COUNTER_HH__
