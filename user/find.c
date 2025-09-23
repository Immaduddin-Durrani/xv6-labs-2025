#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

#define MAXARGS 10

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash
  for(p=path+strlen(path); p >= path && *p != '/'; p--);
  p++;

  if(strlen(p) >= DIRSIZ)
    return p;

  memmove(buf, p, strlen(p));
  buf[strlen(p)] = 0;
  return buf;
}

void
find(char *path, char *name, int execmode, int cmdargc, char *cmdargv[])
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, 0)) < 0)
    return;

  if(fstat(fd, &st) < 0){
    close(fd);
    return;
  }

  if(st.type == T_FILE){
    if(strcmp(fmtname(path), name) == 0){
      if(execmode){
        if(fork() == 0){
          char *nargv[MAXARGS];
          int j = 0;
          for(; j < cmdargc; j++)
            nargv[j] = cmdargv[j];
          nargv[j++] = path;   // append the file name
          nargv[j] = 0;
          exec(cmdargv[0], nargv);
          exit(1);
        }
        wait(0);
      } else {
        printf("%s\n", path);
      }
    }
  } else if(st.type == T_DIR){
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      close(fd);
      return;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0) continue;
      if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      find(buf, name, execmode, cmdargc, cmdargv);
    }
  }

  close(fd);
}

int
main(int argc, char *argv[])
{
  if(argc < 3){
    fprintf(2, "Usage: find path name [-exec cmd args]\n");
    exit(1);
  }

  int execmode = 0;
  int cmdargc = 0;
  char *cmdargv[MAXARGS];

  // check for -exec
  if(argc > 3 && strcmp(argv[3], "-exec") == 0){
    execmode = 1;
    cmdargc = argc - 4;
    for(int i = 0; i < cmdargc; i++)
      cmdargv[i] = argv[4 + i];
    cmdargv[cmdargc] = 0;
  }

  find(argv[1], argv[2], execmode, cmdargc, cmdargv);
  exit(0);
}
