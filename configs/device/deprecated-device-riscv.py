# Copyright (c) 2015 Jason Power
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution;
# neither the name of the copyright holders nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

"""
This is the RISCV equivalent to `simple.py` (which is designed to run using the
X86 ISA). More detailed documentation can be found in `simple.py`.
"""

import m5
from m5.objects import *
# from m5.objects import Cache

#AddrRange(0xFFF0000000, size="8MB")
uncacheable_range = [
        #[AddrRange(0x1,size="512MB")],
        [0x8000400,0x8000500]
        #AddrRange(0x1C010000, size="1")
        #[0xF0000000,0xF0000001],[0xF0000001,0xF0000002],[0xF0000002,0xF0000003]   
] 

# deprecated
deviceaddr_range = [
    #[0x8000400,0x8000500]
    [0x8000400,0x8000500]
]

mem_range = [
    #[0x0,0x20000000],
    [0x0,0x8000000],
    #[0x420,0x20000000]
    [0x10000,0x20000]
]

addrlist = [
    #0x400,0x404,0x408,0x40c,0x410
    #0x7e400,0x7e404,0x7e408,0x7e40c,0x7e410
    0x8000400,0x8000404,0x8000408,0x800040c,0x8000410
]

system = System()

system.clk_domain = SrcClockDomain()
system.clk_domain.clock = "1GHz"
system.clk_domain.voltage_domain = VoltageDomain()

system.mem_mode = "timing"
system.mem_ranges = mem_range#[AddrRange("512MB")]
system.cpu = RiscvMinorCPU()

system.membus = SystemXBar()

system.device = SimpleDeviceObj(deviceaddr=deviceaddr_range, addr_list=addrlist )

# get a Cache
class L1Cache(Cache):
    assoc = 2
    tag_latency = 2
    data_latency = 2
    response_latency = 2
    mshrs = 4
    tgts_per_mshr = 20

class L1ICache(L1Cache):
    size = '16kB'

class L1DCache(L1Cache):
    size = '64kB'

system.cpu.icache = L1ICache()
system.cpu.dcache = L1DCache()

system.cpu.icache_port = system.cpu.icache.cpu_side
system.cpu.dcache_port = system.cpu.dcache.cpu_side

#system.device.data_side = system.membus.cpu_side_ports
system.device.device_side = system.membus.mem_side_ports


system.cpu.icache.mem_side = system.membus.cpu_side_ports
system.cpu.dcache.mem_side = system.membus.cpu_side_ports

system.cpu.createInterruptController()



system.cpu.mmu.pma_checker = PMAChecker(uncacheable=uncacheable_range)

system.mem_ctrl1 = MemCtrl()
system.mem_ctrl1.dram = DDR3_1600_8x8()
system.mem_ctrl1.dram.range = system.mem_ranges[0]
system.mem_ctrl1.port = system.membus.mem_side_ports

'''
system.mem_ctrl1 = MemCtrl()
system.mem_ctrl1.dram = DDR3_1600_8x8()
system.mem_ctrl1.dram.range = m5.objects.AddrRange(
0x0,
size = 0x80000,
intlvHighBit = 6,
xorHighBit = 0,
intlvBits = 1,
intlvMatch = 0
)


system.mem_ctrl1.port = system.membus.mem_side_ports

system.mem_ctrl2 = MemCtrl()
system.mem_ctrl2.dram = DDR3_1600_8x8()
system.mem_ctrl2.dram.range = m5.objects.AddrRange(
0x0,
size = 0x80000,
intlvHighBit = 6,
xorHighBit = 0,
intlvBits = 1,
intlvMatch = 1
)
system.mem_ctrl2.port = system.membus.mem_side_ports
#'''
system.system_port = system.membus.cpu_side_ports



thispath = os.path.dirname(os.path.realpath(__file__))
binary = os.path.join(
    thispath,
    "../../",
    #"test_device/test3"
    "test_device/test6"
    #"tests/test-progs/hello/bin/riscv/linux/hello",
)

system.workload = SEWorkload.init_compatible(binary)

process = Process()
process.cmd = [binary]
system.cpu.workload = process
system.cpu.createThreads()

root = Root(full_system=False, system=system)
m5.instantiate()

print(f"Beginning simulation!")
exit_event = m5.simulate()
print(f"Exiting @ tick {m5.curTick()} because {exit_event.getCause()}")
