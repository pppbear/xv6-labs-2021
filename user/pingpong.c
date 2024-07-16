#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int 
main(int argc, char* argv[])
{
	int pipes1[2];//0为读取端，1为写入端
  int pipes2[2];
	int cid;
	int cstatus = 0;
	pipe(pipes1);
  pipe(pipes2);
	//创建子进程
	cid = fork();
	if (cid > 0)//父级进程
	{
		write(pipes1[1], "t", 1);
		//等待子进程结束
		wait(&cstatus);
    char* temp = "";
    read(pipes2[0], temp, 1);
		printf("%d: received pong\n", getpid());
	}
	else if (cid == 0)//子进程
	{
    char* temp = "";
		read(pipes1[0], temp, 1);
		printf("%d: received ping\n", getpid());
    write(pipes2[1], "t", 1);
	}
	exit(0);
}
