

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
    //needRetry(false),
    blocked(false),
    isBlocking(false),
    ReadyA(false),
    ReadyB(false),
    ReadyE(false),
    ReadyO(false),
    Waiting(false),
    Requiring(true),
    ptrData(0),
    requestorID(0),
    RequestCnt(4000),
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

AddrRangeList
SimpleDeviceObj::EnablePort::getAddrRanges() const
{
    for(AddrRange range : owner->deviceaddr)
    {
        DPRINTF(Device_Obj, "EnablePort::getAddrRanges:start=%" PRIx64 " ,end=%" PRIx64 "\n",range.start(),range.end());
    }

    return owner->deviceaddr;

}

bool
SimpleDeviceObj::EnablePort::recvTimingReq(PacketPtr pkt)
{
    //bool isEnable = 
    //owner->handleEnable(pkt);
    //DPRINTF(Device_Obj, "this is %s enable signal, addr: %" PRIx64 "\n", isEnable?"a":"NOT a" ,pkt->getAddr());
    //return true;
        if (!owner->handleEnable(pkt)) {
        needRetry = true;
        return false;
    } else {
        return true;
    }
}


void
SimpleDeviceObj::DataPort::sendPacket(PacketPtr pkt)
{
    panic_if(blockedPacket != nullptr, "Should never try to send if blocked!");
    
    if(!sendTimingReq(pkt)) {
        blockedPacket = pkt;
        
    } else {
        if(pkt->getAddr() == AddrList[3]){
            owner->ReadyO = true;
        }
    }
}

bool
SimpleDeviceObj::DataPort::recvTimingResp(PacketPtr pkt)
{
    return owner->handleResponse(pkt);
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

void
SimpleDeviceObj::DataPort::recvRangeChange()
{

    DPRINTF(Device_Obj, "SimpleDeviceObj::DataPort::recvRangeChange\n");
    owner->sendRangeChange();
}

void
SimpleDeviceObj::sendRangeChange()
{
    enablePort.sendRangeChange();
}

bool 
SimpleDeviceObj::handleEnable(PacketPtr pkt)
{
    if(blocked){
        return false;
    }
    Addr addr = pkt->getAddr();
    DPRINTF(Device_Obj, "this is %s enable signal, addr: %" PRIx64 ", data: %d\n", addr == AddrList[0]?"a":"NOT a" ,pkt->getAddr(),pkt->getPtr<uint8_t>());
    if(addr == AddrList[0])
    {
        if(pkt->isWrite())
        {
            ReadyE = true;

            return true;
        }
        
    }
    blocked = true;
    return true;
}



bool
SimpleDeviceObj::handleResponse(PacketPtr pkt)
{
    //assert(blocked);
    isBlocking = false;
    
    
    DPRINTF(Device_Obj, "Got response for addr %" PRIx64 ", is %s\n", pkt->getAddr(), pkt->isRead()?"Read":"Write");
    blocked = false;

    if(pkt->isWrite())
    {

     ;   
    } else {
        DPRINTF(Device_Obj,"get Ptr ");
        Addr addr = pkt->getAddr();
        //unsigned size = pkt->getSize();
        uint8_t* data = pkt->getPtr<uint8_t>();
        //DPRINTF(Device_Obj, "handleResponse : Size is %d\n",size);
        DPRINTF(Device_Obj, "This ReadResponse pkt: addr= %" PRIx64 " ,data= %d\n", addr, *data);
        if(!handleData(addr, *data))
        { DPRINTF(Device_Obj, "handleData Unsuccess\n"); }
    }

    return true;
}

bool
SimpleDeviceObj::handleData(Addr addr, uint8_t data)
{
    
    switch (addr) {
        case SimpleDeviceObj::AddrList[0]:{
            inp.inA = data;
            if(data == 0xbb)
            {
                DPRINTF(Device_Obj, "Get inE = %d\n ",data);
                ReadyE = true;
            } else {
                Requiring = true;      
            }
            
            
            break;
        }

        case SimpleDeviceObj::AddrList[1]:{
            inp.inA = data;
            ReadyA = true;
            DPRINTF(Device_Obj, "Get inA = %d\n ",data);
            break;
        }

        case SimpleDeviceObj::AddrList[2]:{
            inp.inB = data;
            ReadyB = true;
            DPRINTF(Device_Obj, "Get inB = %d\n ",data);
            break;
        }
        default:
        {
            panic("handleData default!");
        }
    
    }
    return true;
}

void
SimpleDeviceObj::MakePacketAndTrytoSend(int index, MemCmd cmd)
{
    unsigned size = 8;
    //Flags flags = new Flags<unsigned int>();
    //MemCmd cmd = new MemCmd(MemCmd::Command::ReadReq)
    RequestPtr newReq = 
        std::make_shared<gem5::Request>(SimpleDeviceObj::AddrList[index], \
                                        size, 0x00002000|0x00001000,\
                                        requestorID++);
    PacketPtr newPtk = new gem5::Packet(newReq, cmd);
    newPtk->dataDynamic(&ptrData);
    // bool isO = false;
    // if(index == 3){
    //     isO = true;
    // }
    DPRINTF(Device_Obj,"MakePtr,addr= %" PRIx64 ", data= %d, cmd= %s\n",SimpleDeviceObj::AddrList[index], ptrData, cmd==MemCmd::Command::ReadReq?"Read":"Write");
    dataPort.sendPacket(newPtk);
    
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

void 
SimpleDeviceObj::evaluate() 
{
    wr->tick();
    inp.ena = 0;
    if(RequestCnt!=0){
            RequestCnt--;
        } else {
            RequestCnt = 4000;
            DPRINTF(Device_Obj,"evaluate!\n");
                if(Requiring && !Waiting && !isBlocking)
                {
                    // if(RequestCnt!=0){
                    //     RequestCnt--;
                    // } else {
                    //     RequestCnt = 4000;
                    Requiring = false;
                    DPRINTF(Device_Obj,"Requiring\n");
                    MemCmd cmd = MemCmd(MemCmd::Command::ReadReq);
                    isBlocking = true;
                    DPRINTF(Device_Obj,"Req E\n");
                    MakePacketAndTrytoSend(0,cmd);
                    

                }
                if(ReadyE && !Waiting && !isBlocking)
                {
                    MemCmd cmd = MemCmd(MemCmd::Command::ReadReq);
                    isBlocking = true;
                    if(!ReadyA){
                        DPRINTF(Device_Obj,"Req A\n");
                        MakePacketAndTrytoSend(1,cmd);
                    } else if(ReadyA && !ReadyB){
                        DPRINTF(Device_Obj,"Req B\n");
                        MakePacketAndTrytoSend(2,cmd);
                    }
                }
        }
    if(ReadyE && ReadyA && ReadyB)
    {
        DPRINTF(Device_Obj, "E,A,B is ready\n ");
        ReadyE = ReadyA = ReadyB = false;
        Waiting = true;
        WrSetData();


        DPRINTF(Device_Obj,"Make Enable Signal unable\n");
        MemCmd cmd = MemCmd(MemCmd::Command::WriteReq);
        ptrData = 0x00;
        MakePacketAndTrytoSend(0,cmd);
    }
    if(Waiting == true && wr->isOK())
    {
        
        
        MemCmd cmd = MemCmd(MemCmd::Command::WriteReq);
        out = wr->getData();
        DPRINTF(Device_Obj,"Wr->OK out.out=%" PRIx64 "\n",out.out);
        ptrData = (uint8_t)out.out;
        MakePacketAndTrytoSend(3,cmd);
    }
    if(ReadyO)
    {
        DPRINTF(Device_Obj,"ReadyO\n");
        ReadyO = false;
        MemCmd cmd = MemCmd(MemCmd::Command::WriteReq);
        ptrData = 0xaa;
        MakePacketAndTrytoSend(4,cmd);
        //Requiring = true;
        Waiting = false;
    }



}

void 
SimpleDeviceObj::endRTLModel()
{
    delete wr;
    wr = nullptr;
}











} //namespace gem5