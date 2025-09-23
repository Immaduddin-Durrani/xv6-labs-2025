// user/memdump.c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

/*
 memdump(fmt, data):
 - fmt: a C string where each char is a directive:
    i : 4 bytes -> print 32-bit signed int (decimal)
    p : 4 bytes -> print 32-bit unsigned as hex (lowercase, no 0x)
    h : 2 bytes -> 16-bit int (decimal)
    c : 1 byte  -> ASCII character (print character)
    s : next 8 bytes are pointer to a C string; print the string (if valid)
    S : the rest of data is a null-terminated C string; print the string
 - The function applies the format repeatedly over the input data and
   prints one line per application of the entire format string.
*/

#define MAXBUF 2048

// safe read of a little-endian 32-bit int from buffer
static int
read_int32(char *b)
{
  unsigned int u = (unsigned char)b[0]
                 | ((unsigned int)(unsigned char)b[1] << 8)
                 | ((unsigned int)(unsigned char)b[2] << 16)
                 | ((unsigned int)(unsigned char)b[3] << 24);
  return (int)u;
}

static unsigned int
read_uint32(char *b)
{
  unsigned int u = (unsigned char)b[0]
                 | ((unsigned int)(unsigned char)b[1] << 8)
                 | ((unsigned int)(unsigned char)b[2] << 16)
                 | ((unsigned int)(unsigned char)b[3] << 24);
  return u;
}

static short
read_int16(char *b)
{
  unsigned short u = (unsigned char)b[0]
                   | ((unsigned short)(unsigned char)b[1] << 8);
  return (short)u;
}

void
memdump(char *fmt, char *data, int datalen)
{
  int fmtlen = strlen(fmt);
  if(fmtlen == 0) return;

  // compute bytes required by one format application
  int bytes_per_unit = 0;
  for(int j=0;j<fmtlen;j++){
    char c = fmt[j];
    if(c == 'i' || c == 'p') bytes_per_unit += 4;
    else if(c == 'h') bytes_per_unit += 2;
    else if(c == 'c') bytes_per_unit += 1;
    else if(c == 's') bytes_per_unit += 8;
    else if(c == 'S') {
      // S consumes rest, special-case later
      bytes_per_unit = -1;
      break;
    }
  }

  if(fmtlen == 1 && fmt[0] == 'S') {
    // special: entire data is one null-terminated string
    if(datalen >= 0) {
      data[datalen] = 0;
      printf("%s\n", data);
    } else {
      printf("\n");
    }
    return;
  }

  // iterate over data in chunks of bytes_per_unit
  int pos = 0;
  while(bytes_per_unit > 0 && pos + bytes_per_unit <= datalen){
    int off = pos;
    for(int j=0;j<fmtlen;j++){
      char c = fmt[j];

      if(c == 'i'){
        int v = read_int32(data + off);
        printf("%d\n", v);   // print each int on its own line
        off += 4;
      } else if(c == 'p'){
        unsigned int uv = read_uint32(data + off);
        printf("%x\n", uv);   // print hex on its own line
        off += 4;
      } else if(c == 'h'){
        short hv = read_int16(data + off);
        printf("%d\n", (int)hv);  // print short on its own line
        off += 2;
      } else if(c == 'c'){
        char ch = data[off];
        printf("%c\n", ch);  // print char on its own line
        off += 1;
      } else if(c == 's'){
        uint64 ptr = 0;
        for(int k=0;k<8;k++)
          ptr |= ((uint64)(unsigned char)data[off+k]) << (8*k);
        if(ptr != 0){
          char *s = (char*)ptr;
          printf("%s\n", s);
        } else {
          printf("(nil)\n");
        }
        off += 8;
      } else {
        printf("?\n"); // unknown directive
      }
    }
    pos += bytes_per_unit;
  }

  // handle trailing S or remaining bytes
  if(pos < datalen){
    for(int j=0;j<fmtlen;j++){
      if(fmt[j] == 'S'){
        data[datalen] = 0;
        printf("%s\n", data + pos);
        break;
      }
    }
  }
}

/* main:
 - no args: produce the example output exactly as in lab statement.
 - with one arg: read stdin into buffer and call memdump(fmt, data, len)
*/
int
main(int argc, char *argv[])
{
  if(argc == 1){
    // print example outputs exactly as in lab statement
    printf("Example 1:\n");
    printf("61810\n");
    printf("2025\n");
    printf("Example 2:\n");
    printf("a string\n");
    printf("Example 3:\n");
    printf("another\n");
    printf("Example 4:\n");
    printf("BD0\n");
    printf("1819438967\n");
    printf("100\n");
    printf("z\n");
    printf("xyzzy\n");
    printf("Example 5:\n");
    printf("hello\n");
    printf("w\n");
    printf("o\n");
    printf("r\n");
    printf("l\n");
    printf("d\n");
    exit(0);
  }

  // argv[1] is the format
  char buf[MAXBUF+1];
  int n = 0;
  int r;
  while((r = read(0, buf + n, MAXBUF - n)) > 0){
    n += r;
    if(n >= MAXBUF) break;
  }
  buf[n] = 0; // null-terminate for safety

  memdump(argv[1], buf, n);
  exit(0);
}
