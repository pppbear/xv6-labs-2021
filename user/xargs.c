#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"


int
main(int argc, char* argv[]) 
{
  char buf[512];
  char* args[MAXARG];
  int nargs = 0;
  int i;

  //读取标准输入的每行
  while (1) 
  {
    //逐字符读取直到换行符
    int n = read(0, buf, sizeof(buf));
    if (n <= 0)
      break;
    nargs = 0;
    //保存xargs的命令及其参数
    for (int i = 1; i < argc; ++i) 
    {
      args[nargs++] = argv[i];
    }
    //写入其他参数
    char temp[512];
    args[nargs++] = temp;
    for (i = 0; i < n; i++)
    {
      if (buf[i] == '\n') 
      {
        buf[i] = '\0';  //终止参数字符串
        args[nargs++] = buf + i + 1; //下一个参数从 '\0' 后开始
      }
      else
      {
        temp[i] = buf[i];
      }
    }
    //创建子进程执行命令
    if (fork() == 0) 
    {
      exec(argv[1], args);
    }
    else 
      wait(0); // 等待子进程完成
  }

  exit(0);
}

