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
        static constexpr Addr AddrList[addressNum] = {0x400,0x404,0x408,0x40c,0x410};

    private:
    //deprecated, since the same address cannot belong to two different ReadesponsePort
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
                bool recvTimingReq(PacketPtr pkt) override
                    { panic("recvFunctional unimpl."); }


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

        void sendRangeChange();
        
        void evaluate() override;

        void endRTLModel();

        bool handleResponse(PacketPtr pkt);

        bool handleData(Addr addr, uint8_t data);

        void MakePacketAndTrytoSend(int index, MemCmd cmd);                

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
            SetOut, //output result
            ReSet,  //reset RTLmodel and reset enable signal
            SetOK,  //set complete signal
            Waiting //Waiting for changing to next status

        };

        DeviceStatus Status;
        DeviceStatus LastStatus;

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