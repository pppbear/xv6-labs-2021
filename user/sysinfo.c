// user/sysinfotest.c

#include "kernel/types.h"
#include "kernel/sysinfo.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    struct sysinfo info;
    //µ÷ÓÃsysinfoº¯Êý
    if(sysinfo(&info) < 0)
    {
        printf("sysinfotest: sysinfo failed\n");
        exit(1);
    }

    printf("free memory: %d bytes\n", info.freemem);
    printf("number of processes: %d\n", info.nproc);

    if(info.freemem > 0 && info.nproc > 0)
    {
        printf("sysinfotest: OK\n");
        exit(0);
    } else 
    {
        printf("sysinfotest: ERROR\n");
        exit(1);
    }
}
