#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
int main(int argc, char* argv[])
{
    int fd = open("/dev/simdev", O_RDWR);

    printf("fd = %d\n",fd);
 
    void *dataptr = mmap(NULL,0x2000,PROT_EXEC,MAP_SHARED,fd,0);

    printf("dataptr = %p\n",dataptr);
    
    int *data_e = dataptr;
    int *data_a = dataptr + 4;
    int *data_b = dataptr + 8;
    int *data_out = dataptr + 12;
    int *data_ok = dataptr + 16;

    *data_a = 5;
    *data_b = 10;
    *data_e = 0xbb;
    int _ok;
    int _out;
    _ok = *data_ok;
    while(1){
        _ok = *data_ok;
        _ok = _ok & (0x00 | 0xff);
        if(_ok == 0xaa)break;
    }
    _out = *data_out;
    _out = _out & (0x00 | 0xff);
    printf("\n-----------------\nout = %d\n-----------------\n",_out);
    printf("Hello world!\n");




    return 0;
}
