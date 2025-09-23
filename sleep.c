// user/sleep.c
#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(2, "Usage: sleep <ticks>\n");
        exit(1);
    }

    int ticks = atoi(argv[1]);
    if (ticks < 0) {
        fprintf(2, "sleep: invalid number of ticks\n");
        exit(1);
    }

    // pause() sleeps for one tick, so call it in a loop
    for (int i = 0; i < ticks; i++) {
        pause();
    }

    exit(0);
}
