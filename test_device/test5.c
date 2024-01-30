#include <stdio.h>
static int data1[500000]; //.bss

int main(int argc, char* argv[])
{
    printf("hi\n");
    
    //printf("\n\n\n\n\ndata1 : %x ---- %x\n\n\n\n\n",data1,data1+499999);
    //data1[499999] = 166;
    //int t = 1;
    //while(t--){
    //    int a = data1[499999];
    //}
    // int *add = (int*)0x8003f0;
    // t = 100;
    // *add = 177;
    // while(t--){
    //     int b = *add;
    // }
    int* data = (int*)0x100000;
    printf("%x\n",*data);




    return 0;
}
