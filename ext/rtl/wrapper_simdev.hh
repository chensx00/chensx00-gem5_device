#ifndef __WRAPPER_SIMDEV_HH__
#define __WRAPPER_SIMDEV_HH__

#include <cstdlib>
#include <iostream>
#include "VTop.h"
#include "verilated.h"
#include "verilated_vcd_c.h"
#include "packet_simdev.hh"
#include "packet_simdev.hh"




class Wrapper_SimDev
{
    public:
        Wrapper_SimDev(bool traceOn, std::string name);
        ~Wrapper_SimDev ();

        
        void tick();
        void reset();

        outputSimDev getData();
        void setData(inputSimDev in);
        bool isOK();

    private:
        VTop *top;
        uint64_t tickcount;
        
        std::string fstname;
        bool traceOn;
        VerilatedVcdC *fst;
        void advanceTickCount();
        //uint64_t tickcount;



};



#endif