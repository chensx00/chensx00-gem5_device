from m5.params import *
from m5.SimObject import SimObject
from m5.objects import TickedObject



class SimpleDriver(EmulatedDriver):
    type = "SimpleDriver"
    cxx_header = "device/simple_driver.hh"
    cxx_class = "gem5::SimpleDriver"

    deviceBaseAddr = Param.Addr("device's base address")