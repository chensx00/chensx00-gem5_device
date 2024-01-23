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
    //data[2] = 1;
    // printf("%x\n",data);
    // // for(int i = 0 ;i < 50 ;i++){
    // //     printf("%x\n",data+i);
    // // }
    // printf("%x\n",data+499);
    
    int *enable,*inA,*inB,*out,*ok;
    enable  = (int*)0x800400;
    inA     = (int*)0x800404;
    inB     = (int*)0x800408;
    out     = (int*)0x80040c;
    ok      = (int*)0x800410;
    *out = (int)0;
    *ok  = (int)0;
    *inA = (int)5;
    *inB = (int)6;
    *enable = (int)0xbb;
    int _ok;
    int _out;
    //_out = *inA;
    while(1){
        _ok = *ok;
        _ok = _ok & (0x00 | 0xff);
            //*inA = (int)5;
            //*inB = (int)6;
            //if(*inB == 5){;}
            //if(*inA == 6){;}
        //printf("ok = %x\n",*ok);
        //_out = *out;
        if(_ok == 0xaa) break;
    }
    
    _out = *out;
    //printf("%x\n",&_ok);
    printf("_out = %x(10)\n",_out);
    //_out = _out & (0x00 | 0xff);
    printf("out = %d(10)\n",(*out)&(0xff|0x00));


    //printf()



    return 0;
}
