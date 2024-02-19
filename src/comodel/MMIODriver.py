# Copyright (C) 2024 Microprocessor R&D Center (MPRC), Peking University
# All rights reserved.

from m5.objects.Process import EmulatedDriver
from m5.params import *


class EmulatedMMIODriver(EmulatedDriver):
    type = "EmulatedMMIODriver"
    cxx_class = "gem5::EmulatedMMIODriver"
    cxx_header = "comodel/mmio_driver.hh"

    mmio_addr = Param.Addr("Physical address of memory mapped I/O")
    mmio_size = Param.Addr("Size of memory mapped I/O range")
