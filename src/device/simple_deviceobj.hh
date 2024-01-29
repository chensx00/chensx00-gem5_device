#ifndef __SIMPLE_DEVICEOBJ_HH__
#define __SIMPLE_DEVICEOBJ_HH__
#include "mem/port.hh"
#include "params/SimpleDeviceObj.hh"
#include "sim/clocked_object.hh"
#include "sim/cur_tick.hh"
#include "sim/sim_object.hh"
#include "sim/ticked_object.hh"
#include "wrapper_simdev.hh"

#define addressNum 5


namespace gem5
{
    /*0x400 : enable signal: 0xbb is true
      0x404 : inA
      0x408 : inB
      0x40c : out
      0x410 : over signal: 0xaa is true
    */
class SimpleDeviceObj : public TickedObject
{
    public:
        //static constexpr Addr AddrList[addressNum] = {0x400,0x404,0x408,0x40c,0x410};

    private:
    //deprecated, since the same address cannot belong to two different ReadesponsePort

        class DevicePort : public ResponsePort{
            private:
                SimpleDeviceObj *owner;
                PacketPtr blockedPacket;

            public:
                DevicePort(const std::string& _name, SimpleDeviceObj *_owner) :
                    ResponsePort(_name),owner(_owner),blockedPacket(nullptr)
                { }

                void sendPacket(PacketPtr pkt);

                AddrRangeList getAddrRanges() const override;

                void trySendRetry();

            protected:
                Tick recvAtomic(PacketPtr pkt) override {
                    panic("recvAtomic unimpl.\n");  
                }

                void recvFunctional(PacketPtr pkt) override {
                    panic("recvFunctional unimpl.\n");  
                }

                bool recvTimingReq(PacketPtr pkt) override ;

                void recvRespRetry() override ;

                
        };


        class DataPort : public RequestPort
        {
            private:
                SimpleDeviceObj *owner;
                PacketPtr blockedPacket;
            
            public:
                DataPort(const std::string& name, SimpleDeviceObj *owner) :
                    RequestPort(name), owner(owner), blockedPacket(nullptr)
                {  }

                void sendPacket(PacketPtr pkt);
            
            protected:
                bool recvTimingResp(PacketPtr pkt) override;
                void recvReqRetry() override;
                void recvRangeChange() override;

        };     


        DataPort dataPort;

        DevicePort devicePort;

        bool blocked;

        void sendRangeChange();
        
        void evaluate() override;

        void endRTLModel();

        bool handleResponse(PacketPtr pkt);

        bool handleData(Addr addr, uint8_t data);

        bool handleRequest(PacketPtr pkt);

        void MakePacketAndTrytoSend(int index, MemCmd cmd);

        void sendResponse();

        void ResetDeviceReg();                

        bool WrSetData();

        bool WrGetData(); 

        bool WrReset();

        inputSimDev inp;

        outputSimDev out;
        
        uint8_t ptrData;

        uint16_t requestorID;

        int RequestCnt;

        enum DeviceStatus {
            IDEL,   //waiting enable signal
                GetA,   //requiring dataA
                GetB,   //requiring dataB
            RTLRun,
            Waiting,   //waiting for RTL 
                SetOut, //output result
                ReSet,  //reset RTLmodel and reset enable signal
            SetOK,  //set complete signal
            //Waiting //Waiting for changing to next status

        };

        DeviceStatus Status;
        DeviceStatus LastStatus;

        uint8_t deviceReg[addressNum];

        EventFunctionWrapper event;

        PacketPtr readyToRespPkt;

    public:

        SimpleDeviceObj(const SimpleDeviceObjParams &params);

        void startup() override;
        
        void init() override;

        Port &getPort(const std::string &if_name, PortID idx=InvalidPortID) override;

        Wrapper_SimDev *wr;

        //Device's address space
        AddrRangeList deviceaddr;

        //specify the device register address
        std::vector<Addr> AddrList;

        //static constexpr Addr AddrList[addressNum] = {0x400,0x404,0x408,0x40c,0x410};

};





} //namespace gem5

#endif