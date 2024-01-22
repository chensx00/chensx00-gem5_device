#include "device/simple_deviceobj.hh"
#include "base/trace.hh"
#include "debug/Device_Obj.hh"
#include "base/flags.hh"


namespace gem5
{

SimpleDeviceObj::SimpleDeviceObj(const SimpleDeviceObjParams &params) :
    TickedObject(params),
    enablePort(params.name + ".enable_side", this),
    dataPort(params.name + ".data_side", this),
    blocked(false),
    ptrData(0),
    requestorID(0),
    RequestCnt(4000),
    Status(IDEL),
    LastStatus(IDEL),
    wr(nullptr),
    deviceaddr(params.deviceaddr.begin(),params.deviceaddr.end())
{
    wr = new Wrapper_SimDev(true,"dev.vcd");
    registerExitCallback([this]() {
        delete this->wr;
        this->wr = nullptr;
    });
}



void 
SimpleDeviceObj::startup() {start(); }

//deprecated
void
SimpleDeviceObj::init() 
{
    DPRINTF(Device_Obj,"SimpleDeviceObj:enablePort.sendRangeChange\n");
    enablePort.sendRangeChange();
}

Port &
SimpleDeviceObj::getPort(const std::string &if_name, PortID idx)
{
    panic_if(idx != InvalidPortID, "This object dosen't support vector ports");
    //printf("\n\nif_name=");
    //std::cout<<if_name<<std::endl<<std::endl<<std::endl;
    if(if_name == "data_side"){

        return dataPort;
    } else if(if_name == "enable_side"){

        return enablePort;
    } else {
        return SimObject::getPort(if_name, idx);
    }
}

void
SimpleDeviceObj::sendRangeChange()
{
    enablePort.sendRangeChange();
}

//deprecated
AddrRangeList
SimpleDeviceObj::EnablePort::getAddrRanges() const
{
    return owner->deviceaddr;

}
//deprecated
// bool
// SimpleDeviceObj::EnablePort::recvTimingReq(PacketPtr pkt)
// {
//         if (!owner->handleEnable(pkt)) {
//         needRetry = true;
//         return false;
//     } else {
//         return true;
//     }
// }

// bool 
// SimpleDeviceObj::handleEnable(PacketPtr pkt)
// {
//     if(blocked){
//         return false;
//     }
//     Addr addr = pkt->getAddr();
//     DPRINTF(Device_Obj, "this is %s enable signal, addr: %" PRIx64 ", data: %d\n", addr == AddrList[0]?"a":"NOT a" ,pkt->getAddr(),pkt->getPtr<uint8_t>());
//     if(addr == AddrList[0])
//     {
//         if(pkt->isWrite())
//         {
//             ReadyE = true;

//             return true;
//         }
        
//     }
//     blocked = true;
//     return true;
// }

void
SimpleDeviceObj::DataPort::recvRangeChange()
{

    DPRINTF(Device_Obj, "SimpleDeviceObj::DataPort::recvRangeChange\n");
    owner->sendRangeChange();
}


void
SimpleDeviceObj::MakePacketAndTrytoSend(int index, MemCmd cmd)
{
    unsigned size = 8;
    RequestPtr newReq = 
        std::make_shared<gem5::Request>(SimpleDeviceObj::AddrList[index], \
                                        size, 0x00002000|0x00001000,\
                                        requestorID++);
    PacketPtr newPtk = new gem5::Packet(newReq, cmd);
    newPtk->dataDynamic(&ptrData);
    DPRINTF(Device_Obj,"MakePtr,addr= %" PRIx64 ", data= %d, cmd= %s\n",SimpleDeviceObj::AddrList[index], ptrData, cmd==MemCmd::Command::ReadReq?"Read":"Write");
    dataPort.sendPacket(newPtk);
    
}

void
SimpleDeviceObj::DataPort::sendPacket(PacketPtr pkt)
{
    panic_if(blockedPacket != nullptr, "Should never try to send if blocked!");
    
    if(!sendTimingReq(pkt)) {
        blockedPacket = pkt;   
    }
}


void 
SimpleDeviceObj::DataPort::recvReqRetry()
{
    assert(blockedPacket != nullptr);
    PacketPtr pkt = blockedPacket;
    blockedPacket = nullptr;
    sendPacket(pkt);
    delete pkt;
}

bool
SimpleDeviceObj::DataPort::recvTimingResp(PacketPtr pkt)
{
    return owner->handleResponse(pkt);
}

bool
SimpleDeviceObj::handleResponse(PacketPtr pkt)
{
    //assert(blocked);
    Addr addr = pkt->getAddr();
    DPRINTF(Device_Obj, "Got response for addr %" PRIx64 ", is %s\n", pkt->getAddr(), pkt->isRead()?"Read":"Write");
    blocked = false;
    if(pkt->isWrite())
    {
        switch(addr)
        {
            case SimpleDeviceObj::AddrList[0]:{
                if(LastStatus == ReSet){
                    DPRINTF(Device_Obj, "ReSet over\n");
                    Status = SetOK;
                }
                break;
            }
            case SimpleDeviceObj::AddrList[3]:{
                if(LastStatus == SetOut){
                    DPRINTF(Device_Obj, "SetOut over\n");
                    Status = ReSet;
                }
                break;
            }
            case SimpleDeviceObj::AddrList[4]:{
                if(LastStatus == SetOK){
                    DPRINTF(Device_Obj, "SetOK over\n");
                    Status = IDEL;
                }
                break;
            }
            default:{
                ;
            }
        }
    } else {
        DPRINTF(Device_Obj,"get Ptr ");
        uint8_t* data = pkt->getPtr<uint8_t>();
        DPRINTF(Device_Obj, "This ReadResponse pkt: addr= %" PRIx64 " ,data= %d\n", addr, *data);
        switch (addr)
        {
            case SimpleDeviceObj::AddrList[0]:{
                if(LastStatus == IDEL){
                    if(*data == 0xbb){
                        DPRINTF(Device_Obj, "IDEL over\n",*data);
                        Status = GetA;
                    }
                }
                break;
            }
            case SimpleDeviceObj::AddrList[1]:{
                if(LastStatus == GetA){
                    DPRINTF(Device_Obj, "GetA over\n",*data);
                    inp.inA = *data;
                    Status = GetB;
                }
                break;
            } 
            case SimpleDeviceObj::AddrList[2]:{
                if(LastStatus == GetB){
                    DPRINTF(Device_Obj, "GetB over\n",*data);
                    inp.inB = *data;
                    Status = RTLRun;
                }
                break;
            }
            default:{
                DPRINTF(Device_Obj,"handleData default!\n");
            }
        }
    }
    return true;
}



void
SimpleDeviceObj::evaluate()
{
    wr->tick();
    if(RequestCnt != 0){
        RequestCnt --;
    } else {
        RequestCnt = 4000;
        DPRINTF(Device_Obj,"evaluate!\n");

        switch (Status) {
            
            case IDEL:{
                LastStatus = IDEL;
                DPRINTF(Device_Obj,"IDEL\n");
                MemCmd cmd = MemCmd(MemCmd::Command::ReadReq);
                MakePacketAndTrytoSend(0,cmd);
                Status = Waiting;
                break;
            }

            case GetA:{
                LastStatus = GetA;
                DPRINTF(Device_Obj,"GetA\n");
                MemCmd cmd = MemCmd(MemCmd::Command::ReadReq);
                MakePacketAndTrytoSend(1,cmd);
                Status = Waiting;
                break;
            }

            case GetB:{
                LastStatus = GetB;
                DPRINTF(Device_Obj,"GetB\n");
                MemCmd cmd = MemCmd(MemCmd::Command::ReadReq);
                MakePacketAndTrytoSend(2,cmd);
                Status = Waiting;
                break;
            }

            case RTLRun:{
                LastStatus = RTLRun;
                DPRINTF(Device_Obj, "RTLRun\n");
                WrSetData();
                Status = Waiting;
                break;
            }

            case SetOut:{
                LastStatus = SetOut;
                DPRINTF(Device_Obj,"SetOut\n");
                MemCmd cmd = MemCmd(MemCmd::Command::WriteReq);
                out = wr->getData();
                ptrData = (uint8_t)out.out;
                MakePacketAndTrytoSend(3,cmd);
                Status = Waiting;
                break;
            }

            case SetOK:{
                LastStatus = SetOK;
                DPRINTF(Device_Obj,"SetOK\n");
                MemCmd cmd = MemCmd(MemCmd::Command::WriteReq);
                ptrData = 0xaa;
                MakePacketAndTrytoSend(4,cmd);
                Status = Waiting;
                break;
            }

            case ReSet:{
                LastStatus = ReSet;
                DPRINTF(Device_Obj,"ReSet\n");
                WrReset();
                MemCmd cmd = MemCmd(MemCmd::Command::WriteReq);
                ptrData = 0x00;
                MakePacketAndTrytoSend(0,cmd);
                Status = Waiting;
                break;
            }

            case Waiting:{
                switch (LastStatus){
                    case RTLRun:{
                        if(wr->isOK()){

                            Status = SetOut;
                            break;
                        } else {
                            Status = Waiting;
                            break;
                        }
                    }
                    default:{
                        Status = Waiting;
                    }
                }
                break;
            }

            default:{
                DPRINTF(Device_Obj,"evaluate default!\n");
            }
        }


    }
}

bool 
SimpleDeviceObj::WrSetData()
{
    inp.ena = 1;
    wr->setData(inp);
    return true;
}

bool
SimpleDeviceObj::WrGetData()
{
    out = wr->getData();
    return true;
}

bool
SimpleDeviceObj::WrReset()
{
    wr->reset();
    return true;
}

void 
SimpleDeviceObj::endRTLModel()
{
    delete wr;
    wr = nullptr;
}


} //namespace gem5