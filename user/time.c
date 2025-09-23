// user/time.c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if(argc < 2){
    fprintf(2, "Usage: time command [args...]\n");
    exit(1);
  }

  int start = uptime();

  if(fork() == 0){
    exec(argv[1], argv+1);
    fprintf(2, "time: exec %s failed\n", argv[1]);
    exit(1);
  }

  wait(0);

  int end = uptime();
  int elapsed = end - start;   // ✅ no division

  int fd = open("time.txt", O_CREATE | O_WRONLY | O_TRUNC);
  if(fd >= 0){
    fprintf(fd, "%d\n", elapsed);  // ✅ write raw ticks
    close(fd);
  } else {
    fprintf(2, "time: cannot create time.txt\n");
  }

  exit(0);
}
