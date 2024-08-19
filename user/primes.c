#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void 
filter(int p[2]) {
  int prime;
  int n;
  //�ӹܵ��ж�ȡ��һ�����֣�������
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
    //�ӽ��̣��ݹ����filterɸѡ��һ������
    close(new_p[1]); // �ر��¹ܵ���д��
    filter(new_p);
  }
  else 
  {
    //�����̣��������ڹܵ���ȡ���������ڹܵ�д��
    close(new_p[0]); //�ر��¹ܵ��Ķ���
    while (read(p[0], &n, sizeof(n)) != 0) 
    {
      if (n % prime != 0)//���˵���ǰ�������ı��� 
        write(new_p[1], &n, sizeof(n));
    }
    close(p[0]);
    close(new_p[1]);
    //�ȴ��ӽ��̽���
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
    //�ӽ��̣���ʼɸѡ����
    close(p[1]); //�رչܵ���д��
    filter(p);
  }
  else 
  {
    //�����̣������� 2 �� 35 д��ܵ�
    close(p[0]); // �رչܵ��Ķ���
    for (int i = 2; i <= 35; i++) 
    {
      write(p[1], &i, sizeof(i));
    }
    close(p[1]); //д����Ϻ�رչܵ���д��
    //�ȴ��ӽ��̽���
    wait(0);
  }
  exit(0);
}
