#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char*
fmtname(char* path)
{
  static char buf[DIRSIZ + 1];
  char* p;

  // Find first character after last slash.
  for (p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if (strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf + strlen(p), ' ', DIRSIZ - strlen(p));
  return buf;
}
void
find(char* path,char* filename)
{
    char buf[512], * p;
    int fd;
    struct dirent de;
    struct stat st;
    //尝试打开给定路径
    if ((fd = open(path, 0)) < 0)
    {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }
    //获取文件状态
    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }
    //根据文件类型进行处理
    switch (st.type) {
    case T_FILE:
      printf("find: %s is not a directory\n", path);
      break;
    case T_DIR:
      //确保文件路径不会过长
      if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf))
      {
        printf("find: path too long\n");
        close(fd);
        return;
      }
      strcpy(buf, path);
      p = buf + strlen(buf);
      *p++ = '/';
      //检查路径长度是否过长
      while (read(fd, &de, sizeof(de)) == sizeof(de))
      {
        if (de.inum == 0)
          continue;
        if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
          continue;
        memmove(p, de.name, DIRSIZ);//将目录项的名字复制到 buf 中
        p[DIRSIZ] = 0;
        if (stat(buf, &st) < 0)
        {
          printf("find: cannot stat %s\n", buf);
          continue;
        }
        switch (st.type)
        {
        case T_FILE:
          if (strcmp(filename, de.name) == 0)
            printf("%s\n", buf);
          break;
        case T_DIR:
          find(buf, filename);
          break;
        }
      }
      break;
    }
    close(fd);
}
int
main(int argc, char* argv[])
{
    if (argc == 3)
    {
        find(argv[1], argv[2]);
    }
    else
    {
        printf("please enter right parameters\n");
    }
    exit(0);
}
