// simple signature (sha512): af2fc23597d454063d5e102d3b80105071fdb365dd55a0a15e4b15a21eca89265ffe707bcbd9f92de9c38a1d6b14b5d13f96c42a9366c8560296aaf0ba43a291
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#include "kernel/fs.h"
#define fail(msg) do {printf("FAILURE: " msg "\n"); failed = 1; goto done;} while (0);
static int failed = 0;


static void
private1()
{
  char buf[BSIZE];
  int fd, blocks;
  int target = 1000;

  fd = open("big.file", O_CREATE | O_WRONLY);
  if(fd < 0){
    fail("bigfile: cannot open big.file for writing\n");
  }

  blocks = 0;
  while(1){
    *(int*)buf = blocks;
    int cc = write(fd, buf, sizeof(buf));
    if(cc <= 0)
      break;
    blocks++;
    if (blocks % 100 == 0)
      printf(".");
    if(blocks == target)
      break;
  }
  printf("\nwrote %d blocks\n", blocks);
  if(blocks != target) {
    fail("bigfile: file is too small\n");
  }

  printf("private testcase 1: ok\n");

done:
  close(fd);
  unlink("big.file");
}

static void 
private2()
{
  char buf[BSIZE];
  int fd, blocks;
  int target = 1543;

  fd = open("big.file", O_CREATE | O_WRONLY);
  if(fd < 0){
    fail("bigfile: cannot open big.file for writing\n");
  }

  blocks = 0;
  while(1){
    *(int*)buf = blocks;
    int cc = write(fd, buf, sizeof(buf));
    if(cc <= 0)
      break;
    blocks++;
    if (blocks % 100 == 0)
      printf(".");
    if(blocks == target)
      break;
  }
  printf("\nwrote %d blocks\n", blocks);
  if(blocks != target) {
    fail("bigfile: file is too small\n");
  }
  printf("private testcase 2: ok\n");

done:
  close(fd);
  unlink("big.file");
}

static void 
private3()
{
  char buf[BSIZE];
  int fd, blocks;
  int target = 67078;
  struct stat st;

  fd = open("big.file", O_CREATE | O_WRONLY);
  if(fd < 0){
    fail("bigfile: cannot open big.file for writing\n");
  }

  blocks = 0;
  while(1){
    *(int*)buf = blocks;
    int cc = write(fd, buf, sizeof(buf));
    if(cc <= 0)
      break;
    blocks++;
    if (blocks % 100 == 0)
      printf(".");
    if(blocks == target)
      break;
  }
  printf("\nwrote %d blocks\n", blocks);
  if(blocks != target) {
    fail("bigfile: file is too small\n");
  }

  *(int*)buf = blocks;
  int cc = write(fd, buf, sizeof(buf));
  if(cc > 0) {
    fail("bigfile: file is too big!\n");
  }

  close(fd);
  fd = open("big.file", O_TRUNC | O_WRONLY);

  if(fstat(fd, &st) < 0) fail("cannot stat big.file\n");
  if(st.size != 0) fail("O_TRUNC did not clean the file\n");

  printf("private testcase 3: ok\n");

done:
  close(fd);
  unlink("big.file");
}

int main() {
  private1();
  private2();
  private3();
  exit(failed);
}
