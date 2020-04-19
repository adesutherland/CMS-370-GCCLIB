/* Test Stack */
#include <stdio.h>

int recurse(unsigned int x)
{
  unsigned int space[200];
  if (x==100) {
    return 100;
  }
  if (x>100) {
    printf("ERROR: Over Level 100!!\n");
    return 100;
  }
  space[x] = recurse(x+1);
  if (space[x] != x+1) {
    printf("ERROR: Mismatched return!!\n");
  }
  return x;
}

int main(int argc, char * argv[])
{
  printf("GCCLIB Stack Test\n");
  recurse(0);
  printf("GCCLIB Stack Test Done\n");
  return 0;
}
