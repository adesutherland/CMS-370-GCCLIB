#include <stdio.h>
#include <cmssys.h>

int main(int argc, char *argv[]) {
  int rc;
  printf("Call/Arg Test\n");

  rc = CMScommand("tstarg2 Argument1 Argument2 Argument3",1); /* 1=command, 11=typed */
  if (rc==1) printf(" - Call type 1 OK\n");
  else printf(" - Call type 1 Error (rc=%d)\n",rc);

  rc = CMScommand("tstarg2 Argument1 Argument2 Argument3",11); /* 1=command, 11=typed */
  if (rc==1) printf(" - Call type 11 OK\n");
  else printf(" - Call type 11 Error (rc=%d)\n",rc);

  return 0;
}
