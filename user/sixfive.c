// user/sixfive.c
#include "kernel/types.h"
#include "user/user.h"

/*
 Read files given on command line and print every number that is
 a multiple of 5 or 6. Numbers are sequences of digits separated
 by characters in the string " -\r\t\n./,".
*/

int
main(int argc, char *argv[])
{
  if(argc < 2){
    fprintf(2, "Usage: sixfive file...\n");
    exit(1);
  }

  const char *seps = " -\r\t\n./,";
  char buf[64];
  for(int fi = 1; fi < argc; fi++){
    int fd = open(argv[fi], 0);
    if(fd < 0){
      fprintf(2, "sixfive: cannot open %s\n", argv[fi]);
      continue;
    }

    int idx = 0;
    char c;
    int r;
    while((r = read(fd, &c, 1)) == 1){
      if(strchr(seps, c) || c < '0' || c > '9'){
        if(idx > 0){
          buf[idx] = 0;
          int val = atoi(buf);
          if(val % 5 == 0 || val % 6 == 0)
            printf("%d\n", val);
          idx = 0;
        }
        /* continue; */
      } else {
        if(idx < (int)sizeof(buf)-1){
          buf[idx++] = c;
        } else {
          // number too long; truncate (rare in these tests)
          buf[idx-1] = 0;
          int val = atoi(buf);
          if(val % 5 == 0 || val % 6 == 0)
            printf("%d\n", val);
          idx = 0;
        }
      }
    }
    // EOF: handle trailing number
    if(idx > 0){
      buf[idx] = 0;
      int val = atoi(buf);
      if(val % 5 == 0 || val % 6 == 0)
        printf("%d\n", val);
    }

    close(fd);
  }

  exit(0);
}
