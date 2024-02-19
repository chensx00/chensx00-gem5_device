# Copyright (C) 2024 Microprocessor R&D Center (MPRC), Peking University
# All rights reserved.

from m5.objects.Device import BasicPioDevice


class SampleCounter(BasicPioDevice):
    type = "SampleCounter"
    cxx_class = "gem5::SampleCounter"
    cxx_header = "comodel/sample_counter.hh"
