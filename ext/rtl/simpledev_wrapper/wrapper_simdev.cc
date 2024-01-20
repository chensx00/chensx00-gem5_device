
#include "wrapper_simdev.hh"

Wrapper_SimDev::Wrapper_SimDev(bool traceOn, std::string name):
    tickcount(0),
    fst(NULL),
    fstname(name),
    traceOn(traceOn)
    
{
    top = new VTop;

    Verilated::traceEverOn(traceOn);
    fst = new VerilatedVcdC;
    if (!fst) {
        return;
    }
    top->trace(fst,5);

    std::cout << fstname << std::endl;
    fst->open(fstname.c_str());
    reset();
}

Wrapper_SimDev::~Wrapper_SimDev() 
{
    if (fst) {
        fst->dump(tickcount);
        fst->close();
        delete fst;
    }
    top->final();
    delete top;
    exit(EXIT_SUCCESS);
}

outputSimDev
Wrapper_SimDev::getData()
{
    outputSimDev out;
    out.out = top->out;
    out.ok  = top->ok;
    return out;
}

void
Wrapper_SimDev::setData(inputSimDev in)
{
    top->ena = in.ena;
    top->inA = in.inA;
    top->inB = in.inB;
    printf("set data:ena=%" PRIx64 " inA=%" PRIx64 " inB=%" PRIx64 " \n",in.ena,in.inA,in.inB);
}

void
Wrapper_SimDev::tick()
{
    top->clk = 0;
    top->eval();
    advanceTickCount();
    top->clk = 1;
    top->eval();
    advanceTickCount();
}

void
Wrapper_SimDev::reset()
{
    top->rst = 1;
    this->tick();
    top->rst = 0;
}

bool
Wrapper_SimDev::isOK()
{
    
    if(top->ok == 1){
        //printf("isOK:ena=%d inA=%d inB=%d out=%d\n",top->ena,top->inA,top->inB,top->out);
        return true;
        }
    else return false;
}

void 
Wrapper_SimDev::advanceTickCount()
{
    if (fst and traceOn) {
        fst->dump(tickcount);
    }
    tickcount++;
}