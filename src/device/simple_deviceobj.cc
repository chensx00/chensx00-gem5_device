#include "device/simple_deviceobj.hh"
#include "base/trace.hh"
#include "debug/Device_Obj.hh"
#include "base/flags.hh"
#include "debug/Device_Obj_begin.hh"


namespace gem5
{

SimpleDeviceObj::SimpleDeviceObj(const SimpleDeviceObjParams &params) :
    TickedObject(params),
    //enablePort(params.name + ".enable_side", this),
    dataPort(params.name + ".data_side", this),
    devicePort(params.name + ".device_side", this),
    blocked(false),
    ptrData(0),
    requestorID(0),
    RequestCnt(100),
    Status(IDEL),
    LastStatus(IDEL),
    event([this]{sendResponse();},name()),
    readyToRespPkt(nullptr),
    wr(nullptr),
    deviceaddr(params.deviceaddr.begin(),params.deviceaddr.end()),
    AddrList(params.addr_list)
    
    
    
{


    ResetDeviceReg();

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
    devicePort.sendRangeChange();
    //enablePort.sendRangeChange();
}

void
SimpleDeviceObj::ResetDeviceReg()
{
    for(int i = 0; i < addressNum; i++){
        deviceReg[i] = 0;
    }
}

Port &
SimpleDeviceObj::getPort(const std::string &if_name, PortID idx)
{
    panic_if(idx != InvalidPortID, "This object dosen't support vector ports");
    //printf("\n\nif_name=");
    //std::cout<<if_name<<std::endl<<std::endl<<std::endl;
    if(if_name == "data_side"){

        return dataPort;
    } /*else if(if_name == "enable_side"){

        return enablePort;
    } */
    else if(if_name == "device_side"){
        return devicePort;
    } else {
        return SimObject::getPort(if_name, idx);
    }
}

void
SimpleDeviceObj::sendRangeChange()
{
    devicePort.sendRangeChange();
}

AddrRangeList 
SimpleDeviceObj::DevicePort::getAddrRanges() const
{
    return owner -> deviceaddr;
}

bool
SimpleDeviceObj::DevicePort::recvTimingReq(PacketPtr pkt)
{
    return owner->handleRequest(pkt);
}

bool
SimpleDeviceObj::handleRequest(PacketPtr pkt)
{

    const bool expect_response = pkt->needsResponse() && !pkt->cacheResponding();
    DPRINTF(Device_Obj,\
        "recvTimingReq : Got request for addr %" PRIx64 ", is %s , is %s\n", \
        pkt->getAddr(), pkt->isRead()?"Read":"Write",\
        expect_response?"need_response":"dont need_response");

    Addr reqAddr = pkt->getAddr();
    

    if(pkt->isWrite()) {
        uint8_t* data = pkt->getPtr<uint8_t>();

        if(reqAddr == SimpleDeviceObj::AddrList[0]){ // enable
            deviceReg[0] = *data & (0x00 | 0xff);
            DPRINTF(Device_Obj,"SetE, E = %d\n",deviceReg[0]);

            if(Status == IDEL){
                if(deviceReg[0] == 0xbb){
                    deviceReg[4] = 0; // reset ok signal
                    DPRINTF(Device_Obj,"IDEL over\n");
                    Status = RTLRun;
                    
                    //sendResponse(pkt);
                }
            }
            pkt->makeResponse();


        } else if(reqAddr == SimpleDeviceObj::AddrList[1]){ // inA
            deviceReg[1] = *data & (0x00 | 0xff);
            pkt->makeResponse();
            
            DPRINTF(Device_Obj,"SetA, A = %d\n",deviceReg[1]);
            //sendResponse(pkt);


        } else if(reqAddr == SimpleDeviceObj::AddrList[2]){ // inB
            deviceReg[2] = *data & (0x00 | 0xff);
            pkt->makeResponse();
            
            DPRINTF(Device_Obj,"SetB, B = %d\n",deviceReg[2]);
            //sendResponse(pkt);


        } else if(reqAddr == SimpleDeviceObj::AddrList[3]){ // Out
            deviceReg[3] = *data & (0x00 | 0xff);
            pkt->makeResponse();
            
            DPRINTF(Device_Obj,"SetOut, Out = %d\n",deviceReg[3]);   
            //sendResponse(pkt);         


        } else if(reqAddr == SimpleDeviceObj::AddrList[4]){ // over
            deviceReg[4] = *data & (0x00 | 0xff);
            pkt->makeResponse();
            
            DPRINTF(Device_Obj,"SetOK, OK = %d\n",deviceReg[4]); 
            //sendResponse(pkt); 


        } else {
            DPRINTF(Device_Obj,"handleRequest : else write\n");
            DPRINTF(Device_Obj,"pkt : addr = %#x,data = %d\n",reqAddr,*data);
        }

    } else if(pkt->isRead()) {
        if(reqAddr == SimpleDeviceObj::AddrList[0]){
            pkt -> makeResponse(); 
            
            pkt -> setData(&deviceReg[0]);

            DPRINTF(Device_Obj,"SendE, E = %d\n",deviceReg[0]);
            //sendResponse(pkt); 

        } else if(reqAddr == SimpleDeviceObj::AddrList[1]){
            pkt->makeResponse();
            pkt -> setData(&deviceReg[1]);

            DPRINTF(Device_Obj,"SendA, A = %d\n",deviceReg[1]);
            //sendResponse(pkt);             


        } else if(reqAddr == SimpleDeviceObj::AddrList[2]){
            pkt->makeResponse();
            pkt -> setData(&deviceReg[2]);

            DPRINTF(Device_Obj,"SendB, B = %d\n",deviceReg[2]);
            //sendResponse(pkt);             


        } else if(reqAddr == SimpleDeviceObj::AddrList[3]){
            pkt->makeResponse();
            pkt -> setData(&deviceReg[3]);

            DPRINTF(Device_Obj,"SendOut, Out = %d\n",deviceReg[3]);
            //sendResponse(pkt);             


        } else if(reqAddr == SimpleDeviceObj::AddrList[4]){
            pkt->makeResponse();
            pkt -> setData(&deviceReg[4]);

            DPRINTF(Device_Obj,"SendOK, OK = %d\n",deviceReg[4]);
            //sendResponse(pkt);             


        } else {
            DPRINTF(Device_Obj,"handleRequest : else read\n");
        }

    } else {
        DPRINTF(Device_Obj,"handleRequest : else\n");
    }

    if(expect_response)
    {
        readyToRespPkt = pkt;
        schedule(event,curTick()+1000);
           
    }

    return true;


}



//static constexpr Addr AddrList[addressNum] = {0x400,0x404,0x408,0x40c,0x410};
void
SimpleDeviceObj::DataPort::recvRangeChange()
{

    DPRINTF(Device_Obj, "SimpleDeviceObj::DataPort::recvRangeChange\n");
    owner->sendRangeChange();
}

void 
SimpleDeviceObj::sendResponse()
{
    DPRINTF(Device_Obj,"sendResponse, addr = %" PRIx64 ", is %s\n",readyToRespPkt->getAddr(),readyToRespPkt->isRead()?"Read":"Write");

    devicePort.sendPacket(readyToRespPkt);
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
SimpleDeviceObj::DevicePort::sendPacket(PacketPtr pkt)
{
    panic_if(blockedPacket != nullptr, "Should never try to send if blocked!");
    
    if(!sendTimingResp(pkt)) {
        blockedPacket = pkt;   
    }
}

void 
SimpleDeviceObj::DevicePort::trySendRetry()
{
    if (blockedPacket == nullptr) {
        // Only send a retry if the port is now completely free
        
        DPRINTF(Device_Obj, "Sending retry req.\n");
        sendRetryReq();
    }
}

void
SimpleDeviceObj::DevicePort:: recvRespRetry()
{
    // We should have a blocked packet if this function is called.
    assert(blockedPacket != nullptr);

    // Grab the blocked packet.
    PacketPtr pkt = blockedPacket;
    blockedPacket = nullptr;

    DPRINTF(Device_Obj, "Retrying response pkt %s\n", pkt->print());
    // Try to resend it. It's possible that it fails again.
    sendPacket(pkt);

    // We may now be able to accept new packets
    trySendRetry();
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
        if(addr == SimpleDeviceObj::AddrList[0]){
            if(LastStatus == ReSet){
                DPRINTF(Device_Obj, "ReSet over\n");
                Status = SetOK;
            }
        } else if(addr == SimpleDeviceObj::AddrList[3]){
            if(LastStatus == SetOut){
                DPRINTF(Device_Obj, "SetOut over\n");
                Status = ReSet;
            }
        } else if(addr == SimpleDeviceObj::AddrList[4]){
            if(LastStatus == SetOK){
                DPRINTF(Device_Obj, "SetOK over\n");
                Status = IDEL;
            }
        } else {
            ;
        }
    } else {
        DPRINTF(Device_Obj,"get Ptr ");
        uint8_t* data = pkt->getPtr<uint8_t>();
        DPRINTF(Device_Obj, "This ReadResponse pkt: addr= %" PRIx64 " ,data= %d\n", addr, *data);
        if(addr == SimpleDeviceObj::AddrList[0]){
            if(LastStatus == IDEL){
                if(*data == 0xbb){
                    DPRINTF(Device_Obj, "IDEL over\n");
                    Status = GetA;
                } else {
                    DPRINTF(Device_Obj, "IDEL again\n");
                    Status = IDEL;
                }
            }
        } else if(addr == SimpleDeviceObj::AddrList[1]){
            if(LastStatus == GetA){
                DPRINTF(Device_Obj, "GetA over\n",*data);
                inp.inA = *data;
                Status = GetB;
            }
        } else if(SimpleDeviceObj::AddrList[2]){
            if(LastStatus == GetB){
                DPRINTF(Device_Obj, "GetB over\n",*data);
                inp.inB = *data;
                Status = RTLRun;
            }
        } else {
            ;
        }
    }
    return true;
}

void SimpleDeviceObj::evaluate()
{
    wr->tick();
    if(RequestCnt != 0){
        RequestCnt --;
    } else {
        RequestCnt = 300;

        DPRINTF(Device_Obj,"evaluate! Status = %d\n",Status);

        switch (Status) {
            
            case IDEL:{
                
                break;
            }

            case RTLRun:{
                DPRINTF(Device_Obj, "RTLRun\n");
                inp.inA = deviceReg[1];
                inp.inB = deviceReg[2];
                DPRINTF(Device_Obj, "inA = %ld, inB = %ld\n",inp.inA,inp.inB);
                WrSetData();
                Status = Waiting;

                break;
            }

            case Waiting:{
                if(wr->isOK()){
                    WrGetData();
                    
                    deviceReg[3] = (uint8_t)out.out;
                    DPRINTF(Device_Obj,"Waiting : wr->isOK, out = %d\n",deviceReg[3]);
                    
                    WrReset();
                    deviceReg[0] = 0;
                    

                    Status = SetOK;
                }

                break;

            }

            case SetOK:{

                deviceReg[4] = 0xaa;
                DPRINTF(Device_Obj,"SetOK, ok = %d\n",deviceReg[4]);
                Status = IDEL;
            }


            default:{
                ;
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