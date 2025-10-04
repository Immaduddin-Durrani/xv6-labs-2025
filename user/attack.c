#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"
#include "kernel/stat.h"

#define PAGE           4096
#define MAX_PAGES      256   // try up to this many single-page sbrk()s
#define MIN_SECRET_LEN 4
#define MAX_SECRET_LEN 128

static int is_alnum(char c){
  if (c >= '0' && c <= '9') return 1;
  if (c >= 'A' && c <= 'Z') return 1;
  if (c >= 'a' && c <= 'z') return 1;
  return 0;
}

int
main(int argc, char *argv[])
{
  char best[MAX_SECRET_LEN+1];
  int best_len = 0;

  for (int page = 0; page < MAX_PAGES; page++) {
    char *p = sbrk(PAGE);        // allocate a single page
    if (p == (char*)-1) break;

    // scan this page for alnum runs, keep the longest one
    for (int i = 0; i < PAGE; i++) {
      if (!is_alnum(p[i])) continue;
      // ensure start-of-token
      if (i > 0 && is_alnum(p[i-1])) continue;

      int j = i;
      while (j < PAGE && is_alnum(p[j])) j++;
      int len = j - i;
      if (len < MIN_SECRET_LEN || len > MAX_SECRET_LEN) { i = j; continue; }

      // accept if terminated by non-alnum or page end
      if (j < PAGE && is_alnum(p[j])) { i = j; continue; }

      if (len > best_len) {
        int copylen = (len < MAX_SECRET_LEN) ? len : (MAX_SECRET_LEN - 1);
        memmove(best, p + i, copylen);
        best[copylen] = '\0';
        best_len = copylen;

        // early exit if it's reasonably long (likely the secret)
        if (best_len >= 8) {
          printf("%s\n", best);
          exit(0);
        }
      }
      i = j;
    }
    // continue to next single-page allocation
  }

  if (best_len > 0) {
    printf("%s\n", best);
    exit(0);
  }

  // nothing found
  exit(1);
}
