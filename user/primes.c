#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void 
filter(int p[2]) {
  int prime;
  int n;
  //从管道中读取第一个数字，即素数
  if (read(p[0], &prime, sizeof(prime)) == 0) 
  {
    close(p[0]);
    exit(0);
  }
  printf("prime %d\n", prime);

  int new_p[2];
  pipe(new_p);

  if (fork() == 0) 
  {
    //子进程：递归调用filter筛选下一个数字
    close(new_p[1]); // 关闭新管道的写端
    filter(new_p);
  }
  else 
  {
    //父进程：从左相邻管道读取，向右相邻管道写入
    close(new_p[0]); //关闭新管道的读端
    while (read(p[0], &n, sizeof(n)) != 0) 
    {
      if (n % prime != 0)//过滤掉当前的素数的倍数 
        write(new_p[1], &n, sizeof(n));
    }
    close(p[0]);
    close(new_p[1]);
    //等待子进程结束
    wait(0);
    exit(0);
  }
}

int 
main(int argc, char* argv[]) 
{
  int p[2];
  pipe(p);
  if (fork() == 0)
  {
    //子进程：开始筛选素数
    close(p[1]); //关闭管道的写端
    filter(p);
  }
  else 
  {
    //父进程：将数字 2 到 35 写入管道
    close(p[0]); // 关闭管道的读端
    for (int i = 2; i <= 35; i++) 
    {
      write(p[1], &i, sizeof(i));
    }
    close(p[1]); //写入完毕后关闭管道的写端
    //等待子进程结束
    wait(0);
  }
  exit(0);
}
