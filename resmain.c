#include <stdio.h>
#include <ctype.h>
#include <cmssys.h>
#include <cmsruntm.h>

extern void CMSVTAB;

/* Compare argument */
static int is_same_arg(const char *arg, const char *val) {
    const char *c1 = arg;
    const char *c2 = val;
    while (*c1 == toupper(*c2)) {
        if (*c1 == 0) return 1;
        c1++;
        c2++;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    int anchor_addr;
    int debug = 0;
    int version = 0;
    int *c;

    anchor_addr = (int) &CMSVTAB;

    if (argc > 2) goto error;
    if (argc == 2) {
      if (is_same_arg("VERSION", argv[1])) {
          version = 1;
      }
      else if (is_same_arg("DEBUG", argv[1])) {
          version = 1;
          debug = 1;
          CMSsetdebug(1);
      }
      else goto error;
    }

    /* Register Address */
    CMSSetNUCON((void *) STUB_ANCHOR_ADDRESS, anchor_addr);

    if (version) printf("GCCLIB Version %s\n", GCCLIB_VERSION);
    if (debug) printf("GCC ANCHOR Address is 0x%x saved in NUCON at 0x%x\n", anchor_addr,
           STUB_ANCHOR_ADDRESS);

    return 0;

    error:
      printf("Invalid arguments\n");
      printf("   GCCLIB [VERSION|DEBUG]\n");
      return -1;
}
