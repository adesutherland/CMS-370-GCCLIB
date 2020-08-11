#include <stdio.h>
#include <stdlib.h>
#include <cmssys.h>

int main(int argc, char *argv[]) {
  int rc;
  printf("Call/Arg Test\n");
  char *ret;

  rc = CMScommand("tstarg2 Argument1 Argument2 Argument3",0);
  if (rc==1) printf(" - Call type 0 OK\n");
  else printf(" - Call type 0 Error (rc=%d)\n",rc);


  rc = CMScommand("tstarg2 Argument1 Argument2 Argument3",1);
  if (rc==1) printf(" - Call type 1 OK\n");
  else printf(" - Call type 1 Error (rc=%d)\n",rc);


  rc = CMScommand("tstarg2 Argument1 Argument2 Argument3",11);
  if (rc==1) printf(" - Call type 11 OK\n");
  else printf(" - Call type 11 Error (rc=%d)\n",rc);


  rc = CMSfunction("TSTARG2", "TSTARG2 tstarg2", 0, &ret, 3, "Argument 1", "Argument 2", "Argument 3" );
  if (rc==1 && ret && strcmp(ret,"1")==0) printf(" - Call type 5 OK\n");
  else {
    printf(" - Call type 5 Error (rc=%d)\n",rc);
    if (ret) printf(" - Call return: %s\n", ret);
    else printf(" - No Return String\n");
  }
  if (ret) free(ret);


  rc = CMSsimplefunction("tstarg2", &ret, 3, "Argument 1", "Argument 2", "Argument 3" );
  if (rc==1 && ret && strcmp(ret,"1")==0) printf(" - Call type 5 CMSsimplefunction() OK\n");
  else {
    printf(" - Call type 5 Error CMSsimplefunction() (rc=%d)\n",rc);
    if (ret) printf(" - Call return: %s\n", ret);
    else printf(" - No Return String\n");
  }
  if (ret) free(ret);


  rc = CMSsimpleprocedure("tstarg2", 3, "Argument 1", "Argument 2", "Argument 3" );
  if (rc==1) printf(" - Call type 5 CMSsimpleprocedure() OK\n");
  else printf(" - Call type 5 Error CMSsimpleprocedure() (rc=%d)\n",rc);


  rc = CMSfunction("TSTARG2", "TSTARG2 stringreturn", 0, &ret, 3, "Argument 1", "Argument 2", "Argument 3" );
  if (rc==1 && ret && strcmp(ret,"text return value")==0) printf(" - Call type 5 (text return) OK\n");
  else {
    printf(" - Call type 5 (text return) Error (rc=%d)\n",rc);
    if (ret) printf(" - Call return: %s\n", ret);
    else printf(" - No Return String\n");
  }
  if (ret) free(ret);

  rc = CMSfunction("TSTARG2", "TSTARG2 isproc", 1, &ret, 3, "Argument 1", "Argument 2", "Argument 3" );
  if (rc==1 && ret && strcmp(ret,"1")==0) printf(" - Call type 5 (proc) OK\n");
  else {
    printf(" - Call type 5 Error (proc) (rc=%d)\n",rc);
    if (ret) printf(" - Call return: %s\n", ret);
    else printf(" - No Return String\n");
  }
  if (ret) free(ret);

  rc = CMSfunction("TSTARG2", "TSTARG2 isfunc", 0, &ret, 3, "Argument 1", "Argument 2", "Argument 3" );
  if (rc==1 && ret && strcmp(ret,"1")==0) printf(" - Call type 5 (func) OK\n");
  else {
    printf(" - Call type 5 Error (func) (rc=%d)\n",rc);
    if (ret) printf(" - Call return: %s\n", ret);
    else printf(" - No Return String\n");
  }
  if (ret) free(ret);

  return 0;
}
