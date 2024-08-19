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

  //��ȡ��׼�����ÿ��
  while (1) 
  {
    //���ַ���ȡֱ�����з�
    int n = read(0, buf, sizeof(buf));
    if (n <= 0)
      break;
    nargs = 0;
    //����xargs����������
    for (int i = 1; i < argc; ++i) 
    {
      args[nargs++] = argv[i];
    }
    //д����������
    char temp[512];
    args[nargs++] = temp;
    for (i = 0; i < n; i++)
    {
      if (buf[i] == '\n') 
      {
        buf[i] = '\0';  //��ֹ�����ַ���
        args[nargs++] = buf + i + 1; //��һ�������� '\0' ��ʼ
      }
      else
      {
        temp[i] = buf[i];
      }
    }
    //�����ӽ���ִ������
    if (fork() == 0) 
    {
      exec(argv[1], args);
    }
    else 
      wait(0); // �ȴ��ӽ������
  }

  exit(0);
}

