#ifndef __PACKET_SIMDEV_HH__
#define __PACKET_SIMDEV_HH__

#include <cstdlib>
#include <iostream>

struct inputSimDev
{
    uint64_t    ena;
    uint64_t    rst;
    uint64_t    inA;
    uint64_t    inB;
};

struct outputSimDev
{
    uint64_t    out;
    uint64_t    ok;
};


#endif