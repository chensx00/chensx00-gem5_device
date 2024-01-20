#ifndef __SIMPLE_DEVICEOBJ_HH__
#define __SIMPLE_DEVICEOBJ_HH__
#include "mem/port.hh"
#include "params/SimpleDeviceObj.hh"
#include "sim/clocked_object.hh"
#include "sim/cur_tick.hh"
#include "sim/sim_object.hh"
#include "sim/ticked_object.hh"
#include "wrapper_simdev.hh"

#define N 5

namespace gem5
{

// static uint16_t requestorID = 0;
class SimpleDeviceObj : public TickedObject
{
    public:
    /*0x400 : enable signal: 0xbb is true
      0x404 : inA
      0x408 : inB
      0x40c : out
      0x410 : over signal: 0xaa is true
    */
        
        static constexpr Addr AddrList[N] = {0x400,0x404,0x408,0x40c,0x410};
    private:


        // in order to get enable signal from CPU, 
        // don't need to response
        class EnablePort : public ResponsePort
        {
            private:
                SimpleDeviceObj *owner;
                PacketPtr ptk;
                bool needRetry;
            public:
                EnablePort(const std::string& name, SimpleDeviceObj *owner ) :
                    ResponsePort(name), owner(owner), ptk(nullptr)
                { }
            
                AddrRangeList getAddrRanges() const override;

            protected:

                Tick recvAtomic(PacketPtr pkt) override
                    { panic("recvAtomic unimpl."); }
                void recvFunctional(PacketPtr pkt) override
                    { panic("recvFunctional unimpl."); }
                void recvRespRetry() override
                    { panic("recvFunctional unimpl."); }

                bool recvTimingReq(PacketPtr pkt) override;
                
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

        EnablePort enablePort;

        DataPort dataPort;
        
        bool blocked;
        bool isBlocking;


        void evaluate() override;

        

        void endRTLModel();

        bool handleEnable(PacketPtr pkt);
        
        bool handleResponse(PacketPtr pkt);

        void sendRangeChange();
            //{ /*nothing*/ }
        bool handleData(Addr addr, uint8_t data);
        bool WrSetData();

        bool WrGetData();
        void MakePacketAndTrytoSend(int index, MemCmd cmd);
        inputSimDev inp;
        outputSimDev out;
        

        
        bool ReadyA;
        bool ReadyB;
        bool ReadyE;
        bool ReadyO;
        bool Waiting;
        bool Requiring;
        uint8_t ptrData;
        uint16_t requestorID = 0; 
        int RequestCnt;
        

    public:

        SimpleDeviceObj(const SimpleDeviceObjParams &params);

        void startup() override;
        void init() override;

        Port &getPort(const std::string &if_name, PortID idx=InvalidPortID) override;

        Wrapper_SimDev *wr;

        AddrRangeList deviceaddr;

        


};





} //namespace gem5


#endif