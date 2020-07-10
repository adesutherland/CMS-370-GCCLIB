#include <stdio.h>
#include <string.h>
#include <cmssys.h>
#include <cmsruntm.h>

extern void CMSVTAB;

int main(int argc, char *argv[]) {
  int anchor_addr;

  anchor_addr = (int)&CMSVTAB;

  /*
  if (argc==2 && !strcmp("ANCHOR",argv[1])) {
    return anchor_addr;
  }
  */

  /* Register Address */
  CMSSetNUCON(STUB_ANCHOR_ADDRESS,anchor_addr);

  printf("GCCLIB Version %s\n", GCCLIB_VERSION);
  printf("GCC ANCHOR Address is %x saved in NUCON at %x\n", anchor_addr, STUB_ANCHOR_ADDRESS);

  return 0;
}
