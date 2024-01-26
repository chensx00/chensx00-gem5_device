#include <stdio.h>
static int data[500]; //.bss
/*
    [号] 名称              类型             地址              偏移量
       大小              全体大小          旗标   链接   信息   对齐
    [20] .bss              NOBITS           000000000008d3b8  0007c3b8
       00000000000054b0  0000000000000000  WA       0     0     8
*/
int main(int argc, char* argv[])
{
    printf("Hello world!\n");


    int *enb = (int *)0x8000400;
    int *inA = (int *)0x8000404;
    int *inB = (int *)0x8000408;
    int *out = (int *)0x800040c;
    int *ok  = (int *)0x8000410;
    *inA = 5;
    *inB = 10;
    *enb = 0xbb;
    int _ok;
    int _out;
    while(1){
         _ok = *ok;
        _ok = _ok & (0x00 | 0xff);
        if(_ok == 0xaa) break;
    }
    _out = *out;
    _out = _out & (0x00 | 0xff);
    printf("\n-----out = %d ----------\n",_out);
    
    printf("start=%x--------end=%x\n",data,data+499);


    return 0;
}
