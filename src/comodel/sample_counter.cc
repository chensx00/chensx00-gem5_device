/*
 * Co-model high level simulation and low level implementation in gem5.
 *
 * Sample device: a simple counter
 *
 * Chun Yang    <yangchun@mprc.pku.edu.cn>
 *
 * Copyright (C) 2024 Microprocessor R&D Center (MPRC), Peking University
 * All rights reserved.
 */

#include "comodel/sample_counter.hh"

#include "debug/ComoIO.hh"
#include "mem/packet_access.hh"

namespace gem5
{

Tick
SampleCounter::read(PacketPtr pkt)
{
    const Addr offset = pkt->getAddr() - pioAddr;
    const unsigned int size = pkt->getSize();

    DPRINTF(ComoIO, "Reading %u bytes @ 0x%x:\n", size, offset);
    panic_if(offset != 0, "Bad read offset 0x%x\n", offset);
    panic_if(size != 4, "Bad read size @ 0x%x: %u\n", offset, size);

    uint32_t value = counter;
    DPRINTF(ComoIO, "    value: 0x%x (%d)\n", value, value);
    pkt->makeResponse();
    pkt->setLE<uint32_t>(value);

    return 0;
}

Tick
SampleCounter::write(PacketPtr pkt)
{
    const Addr offset = pkt->getAddr() - pioAddr;
    const unsigned int size = pkt->getSize();

    DPRINTF(ComoIO, "Writing %u bytes @ 0x%x:\n", size, offset);
    panic_if(offset != 0 && offset != 4, "Bad write offset 0x%x\n", offset);
    panic_if(size != 4, "Bad write size @ 0x%x: %u\n", offset, size);

    uint32_t value = pkt->getLE<uint32_t>();
    DPRINTF(ComoIO, "    value: 0x%x (%d)\n", value, value);
    pkt->makeResponse();

    if (offset == 0) {  // Reset
        counter = 0;
    } else if (offset == 4) {   // Count
        counter += value;
    }

    return 0;
}

} // namespace gem5
