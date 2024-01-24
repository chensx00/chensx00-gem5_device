#include <stdio.h>
static int data1[500]; //.bss
/*
    [号] 名称              类型             地址              偏移量
       大小              全体大小          旗标   链接   信息   对齐
    [20] .bss              NOBITS           000000000008d3b8  0007c3b8
       00000000000054b0  0000000000000000  WA       0     0     8
*/
int data2[10] = {189,189,3,4,5,6,7,8,9,10};
int main(int argc, char* argv[])
{
    printf("Hello world!\n");
    data1[0] = 199;

    printf("data1:%x---%x\n",data1,data1+499);
    printf("data2:%x---%x\n",data2,data2+10);
    int t = 10;
    while(t--){
        int a = data1[0];
        int b = data2[0];
    }    

    return 0;
}
