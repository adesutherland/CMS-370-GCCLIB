/* Test Stack */
#include <stdio.h>
#include <setjmp.h>

static jmp_buf buf;

int recurse1(unsigned int x)
{
  unsigned int space[200];
  if (x==100) {
    printf("Level 100 reached\n");
    return 100;
  }
  if (x>100) {
    printf("ERROR: Over Level 100!!\n");
    return 100;
  }
  space[x] = recurse1(x+1);
  if (space[x] != x+1) {
    printf("ERROR: Mismatched return!!\n");
  }
  return x;
}

int recurse2(unsigned int x)
{
  unsigned int space[200];

  if (x==100) {
    printf("Level 100 reached\n");
    longjmp(buf,5);
    return 100;
  }
  if (x>100) {
    printf("ERROR: Over Level 100!!\n");
    longjmp(buf,5);
    return 100;
  }
  space[50] = x + 1;
  recurse2(space[50]);
  printf("ERROR: longjmp must have failed!!\n");
  return x;
}

int recurse3(unsigned int x)
{
  unsigned int space[200];

  if (x==100) {
    printf("Level 100 reached\n");
    exit(0);
    return 100;
  }
  if (x>100) {
    printf("ERROR: Over Level 100!!\n");
    exit(5);
    return 100;
  }
  space[50] = x + 1;
  recurse3(space[50]);
  printf("ERROR: exit() must have failed!!\n");
  return x;
}

int main(int argc, char * argv[])
{
  int r;
  printf("GCCLIB Stack Test\n");
  recurse1(0);
  printf("GCCLIB Stack Test - Done\n");

  printf("GCCLIB Stack Test (with longjmp)\n");
  if (!( r=setjmp(buf)) ) recurse2(0);
  if (r != 5) printf("ERROR: set/longjmp did not return expected rc!!\n");
  printf("GCCLIB Stack Test (with longjmp) - Done\n");

  printf("GCCLIB Stack Test (with exit)\n");
  recurse3(0);
  printf("ERROR - GCCLIB Stack Test (with exit) - should not get here\n");

  return 0;
}
