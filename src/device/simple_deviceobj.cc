#include "device/simple_deviceobj.hh"
#include "base/trace.hh"
#include "debug/Device_Obj.hh"
#include "base/flags.hh"
#include "debug/Device_Obj_begin.hh"


namespace gem5
{

SimpleDeviceObj::SimpleDeviceObj(const SimpleDeviceObjParams &params) :
    TickedObject(params),
    devicePort(params.name + ".device_side", this),
    blocked(false),
    ptrData(0),
    requestorID(0),
    requestCnt(100),
    status(IDEL),
    event([this]{sendResponse();},name()),
    readyToRespPkt(nullptr),
    wr(nullptr),
    deviceAddr(params.device_addr.begin(),params.device_addr.end())
    //AddrList(params.addr_list)
    
    
    
{


    ResetDeviceReg();

    Addr addr = deviceAddr.front().start();
    DPRINTF(Device_Obj,"this_addr = %#x\n",addr);

    //init AddrList 
    for(int i=0; i<6; i++){
        AddrList.push_back(addr);
        addr = addr + 4;

    }
        for(int i=0; i<6; i++){
        DPRINTF(Device_Obj,"AddrList[%d] = %#x\n",i,AddrList[i]);
    }
    

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
    devicePort.sendRangeChange();
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
    if(if_name == "device_side"){
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
    return owner -> deviceAddr;
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
        if(reqAddr == SimpleDeviceObj::AddrList[0]){ // if enable = true
            deviceReg[0] = *data & (0x00 | 0xff);
            DPRINTF(Device_Obj,"SetE, E = %d\n",deviceReg[0]);

            if(status == IDEL){
                if(deviceReg[0] == 0xbb){
                    deviceReg[4] = 0; // reset OK signal
                    DPRINTF(Device_Obj,"IDEL over\n");
                    status = RTLRun;
                }
            }
            pkt->makeResponse();


        } else if(reqAddr == SimpleDeviceObj::AddrList[1]){ // inA
            deviceReg[1] = *data & (0x00 | 0xff);
            pkt->makeResponse(); 
            DPRINTF(Device_Obj,"SetA, A = %d\n",deviceReg[1]);


        } else if(reqAddr == SimpleDeviceObj::AddrList[2]){ // inB
            deviceReg[2] = *data & (0x00 | 0xff);
            pkt->makeResponse();
            DPRINTF(Device_Obj,"SetB, B = %d\n",deviceReg[2]);


        } else if(reqAddr == SimpleDeviceObj::AddrList[3]){ // Out
            deviceReg[3] = *data & (0x00 | 0xff);
            pkt->makeResponse();
            DPRINTF(Device_Obj,"SetOut, Out = %d\n",deviceReg[3]);            


        } else if(reqAddr == SimpleDeviceObj::AddrList[4]){ // over
            deviceReg[4] = *data & (0x00 | 0xff);
            pkt->makeResponse();           
            DPRINTF(Device_Obj,"SetOK, OK = %d\n",deviceReg[4]); 



        } else {
            DPRINTF(Device_Obj,"handleRequest : else write\n");
            DPRINTF(Device_Obj,"pkt : addr = %#x,data = %d\n",reqAddr,*data);
        }

    } else if(pkt->isRead()) {
        if(reqAddr == SimpleDeviceObj::AddrList[0]){
            pkt -> makeResponse();         
            pkt -> setData(&deviceReg[0]);
            DPRINTF(Device_Obj,"SendE, E = %d\n",deviceReg[0]);


        } else if(reqAddr == SimpleDeviceObj::AddrList[1]){
            pkt->makeResponse();
            pkt -> setData(&deviceReg[1]);
            DPRINTF(Device_Obj,"SendA, A = %d\n",deviceReg[1]);
      


        } else if(reqAddr == SimpleDeviceObj::AddrList[2]){
            pkt->makeResponse();
            pkt -> setData(&deviceReg[2]);
            DPRINTF(Device_Obj,"SendB, B = %d\n",deviceReg[2]);
           


        } else if(reqAddr == SimpleDeviceObj::AddrList[3]){
            pkt->makeResponse();
            pkt -> setData(&deviceReg[3]);
            DPRINTF(Device_Obj,"SendOut, Out = %d\n",deviceReg[3]);
        


        } else if(reqAddr == SimpleDeviceObj::AddrList[4]){
            pkt->makeResponse();
            pkt -> setData(&deviceReg[4]);
            DPRINTF(Device_Obj,"SendOK, OK = %d\n",deviceReg[4]);
           


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



void 
SimpleDeviceObj::sendResponse()
{
    DPRINTF(Device_Obj,"sendResponse, addr = %" PRIx64 ", is %s\n",readyToRespPkt->getAddr(),readyToRespPkt->isRead()?"Read":"Write");

    devicePort.sendPacket(readyToRespPkt);
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




void SimpleDeviceObj::evaluate()
{
    wr->tick();
    if(requestCnt != 0){
        requestCnt --;
    } else {
        requestCnt = 300;

        DPRINTF(Device_Obj,"evaluate! status = %d\n",status);

        switch (status) {
            
            case IDEL:{
                
                break;
            }

            case RTLRun:{
                DPRINTF(Device_Obj, "RTLRun\n");
                inp.inA = deviceReg[1];
                inp.inB = deviceReg[2];
                DPRINTF(Device_Obj, "inA = %ld, inB = %ld\n",inp.inA,inp.inB);
                WrSetData();
                status = Waiting;

                break;
            }

            case Waiting:{

                if(wr->isOK()){ 
                    WrGetData();
                    
                    deviceReg[3] = (uint8_t)out.out;
                    DPRINTF(Device_Obj,"Waiting : wr->isOK, out = %d\n",deviceReg[3]);
                    
                    WrReset();
                    deviceReg[0] = 0;
                    

                    status = SetOK;
                }

                break;

            }

            case SetOK:{

                deviceReg[4] = 0xaa;
                DPRINTF(Device_Obj,"SetOK, OK = %d\n",deviceReg[4]);
                status = IDEL;
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