#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int start = uptime();  // or your sys_uptime syscall
  if(argc < 2){
    fprintf(2, "Usage: time command\n");
    exit(1);
  }
  int pid = fork();
  if(pid == 0){
    exec(argv[1], argv+1);
    exit(0);
  }
  wait(0);
  int end = uptime();
  int elapsed = end - start;

  int fd = open("time.txt", O_CREATE | O_WRONLY);
  fprintf(fd, "Elapsed time: %d ticks\n", elapsed);
  close(fd);
  exit(0);
}
