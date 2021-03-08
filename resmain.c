#include <stdio.h>
#include <string.h>
#include <cmssys.h>
/* #include <cmsruntm.h> */

/* extern void CMSVTAB; */

int main(int argc, char *argv[]) {
/*  int anchor_addr; */

/*  anchor_addr = (int) &CMSVTAB; */

    /*
    if (argc==2 && !strcmp("ANCHOR",argv[1])) {
      return anchor_addr;
    }
    */

    /* We don't do this any more.  Instead of relying on a hard
       coded NUCON address which may or may not contain the address
       of a runtime libary address vector, we use RESLIBI to get
       accurate, up to date information from the horses mouth.      */

    /* Register Address */
    /* CMSSetNUCON((void *) STUB_ANCHOR_ADDRESS, anchor_addr); */

    printf("GCCLIB Version %s\n", GCCLIB_VERSION);

    /* printf("GCC ANCHOR Address is 0x%x saved in NUCON at 0x%x\n",
           anchor_addr, STUB_ANCHOR_ADDRESS); */

    return 0;
}
