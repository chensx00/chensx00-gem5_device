from m5.params import *
from m5.SimObject import SimObject
from m5.objects import TickedObject
from m5.objects.Process import EmulatedDriver
class SimpleDeviceObj(TickedObject):
    type = "SimpleDeviceObj"
    cxx_header = "device/simple_deviceobj.hh"
    cxx_class  = "gem5::SimpleDeviceObj"

    
    #enable_side  = ResponsePort("MemBus side port, get device enable signal")
    data_side = RequestPort("MemBus side port,1.get device's input,2.set device's result")
    device_side = ResponsePort("MemBus side port, device port")
    deviceaddr = VectorParam.AddrRange([], "Uncacheable address ranges")

    addr_list = VectorParam.UInt64("Device addr")

class SimpleDriver(EmulatedDriver):
    type = "SimpleDriver"
    cxx_header = "device/simple_driver.hh"
    cxx_class = "gem5::SimpleDriver"

    deviceBaseAddr = Param.Addr("device's base address")