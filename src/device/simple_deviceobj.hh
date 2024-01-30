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
    /*device register*/
    /*[0]0x00 : enable signal: 0xbb is true
      [1]0x04 : inA
      [2]0x08 : inB
      [3]0x0c : out
      [4]0x10 : over signal: 0xaa is true
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
                Tick recvAtomic(PacketPtr pkt) override{
                    panic("recvAtomic unimpl. please use MinorCPU!\n");
                }

                void recvFunctional(PacketPtr pkt) override {
                    panic("recvFunctional unimpl.\n");  
                }

                bool recvTimingReq(PacketPtr pkt) override ;

                void recvRespRetry() override ;
   
        };



        DevicePort devicePort;

        bool blocked;

        void sendRangeChange();
        
        void evaluate() override;

        void endRTLModel();

        bool handleRequest(PacketPtr pkt);

        void sendResponse();

        void ResetDeviceReg();                

        bool WrSetData();

        bool WrGetData(); 

        bool WrReset();

        inputSimDev inp;

        outputSimDev out;
        
        uint8_t ptrData;

        uint16_t requestorID;

        int requestCnt;

        enum DeviceStatus {
            IDEL,   //waiting enable signal
            RTLRun, //input data to RTL model to run
            Waiting,   //waiting for RTL model
            SetOK  //set complete signal

        };

        DeviceStatus status;
    

        uint8_t deviceReg[addressNum];

        EventFunctionWrapper event;

        PacketPtr readyToRespPkt;
        //specify the device register address
        std::vector<Addr> AddrList; 

    public:

        SimpleDeviceObj(const SimpleDeviceObjParams &params);

        void startup() override;
        
        void init() override;

        Port &getPort(const std::string &if_name, PortID idx=InvalidPortID) override;

        //pointer of RTL wrapper
        Wrapper_SimDev *wr;

        //Device's address space
        AddrRangeList deviceAddr;

        
        
};





} //namespace gem5

#endif