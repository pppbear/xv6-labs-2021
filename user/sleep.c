#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char* argv[])
{
	if (argc <= 1)
	{
		printf("please enter a parameter\n");
		exit(1);
	}
	else if (argc > 2)
	{
		printf("too many parameters\n");
		exit(1);
	}
	//将输入参数转换为int
	int num = atoi(argv[1]);
	//call user.h中的sleep
	sleep(num);
	//输出结束提示
	write(1, "(nothing happens for a little while)\n", 37);
	exit(0);
}
