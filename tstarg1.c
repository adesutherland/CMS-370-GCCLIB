#include <stdio.h>
#include <stdlib.h>
#include <cmssys.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int rc;
    char *ret;
    char *binary = "BINARY\x00ZZZ";
    printf("Call/Arg Test\n");

    rc = CMScommand("tstarg2 Argument1 Argument2 Argument3", 0);
    if (rc == 1) printf(" - Call type 0 OK\n");
    else printf(" - Call type 0 Error (rc=%d)\n", rc);


    rc = CMScommand("tstarg2 Argument1 Argument2 Argument3", 1);
    if (rc == 1) printf(" - Call type 1 OK\n");
    else printf(" - Call type 1 Error (rc=%d)\n", rc);


    rc = CMScommand("tstarg2 Argument1 Argument2 Argument3", 11);
    if (rc == 1) printf(" - Call type 11 OK\n");
    else printf(" - Call type 11 Error (rc=%d)\n", rc);


    rc = CMSfunction("TSTARG2", "TSTARG2 tstarg2", 0, &ret, 3, "Argument 1",
                     "Argument 2", "Argument 3");
    if (rc > 0 && ret && strcmp(ret, "1") == 0) printf(" - Call type 5 OK\n");
    else {
        printf(" - Call type 5 Error (rc=%d)\n", rc);
        if (ret) printf(" - Call return: %s\n", ret);
        else printf(" - No Return String\n");
    }
    if (ret) free(ret);


    rc = CMSsimplefunction("tstarg2", &ret, 3, "Argument 1", "Argument 2",
                           "Argument 3");
    if (rc > 0 && ret && strcmp(ret, "1") == 0)
        printf(" - Call type 5 CMSsimplefunction() OK\n");
    else {
        printf(" - Call type 5 Error CMSsimplefunction() (rc=%d)\n", rc);
        if (ret) printf(" - Call return: %s\n", ret);
        else printf(" - No Return String\n");
    }
    if (ret) free(ret);


    rc = CMSsimpleprocedure("tstarg2", 3, "Argument 1", "Argument 2",
                            "Argument 3");
    if (rc == 0) printf(" - Call type 5 CMSsimpleprocedure() OK\n");
    else printf(" - Call type 5 Error CMSsimpleprocedure() (rc=%d)\n", rc);


    rc = CMSfunction("TSTARG2", "TSTARG2 stringreturn", 0, &ret, 3,
                     "Argument 1", "Argument 2", "Argument 3");
    if (rc == strlen("text return value") && ret &&
        strcmp(ret, "text return value") == 0)
        printf(" - Call type 5 (text return) OK\n");
    else {
        printf(" - Call type 5 (text return) Error (rc=%d)\n", rc);
        if (ret) printf(" - Call return: %s\n", ret);
        else printf(" - No Return String\n");
    }
    if (ret) free(ret);


    rc = CMSfunction("TSTARG2", "TSTARG2 binaryreturn", 0, &ret, 3,
                     "Argument 1", "Argument 2", "Argument 3");
    if (rc == 10 && ret && memcmp(ret, binary, 10) == 0)
        printf(" - Call type 5 (binary return) OK\n");
    else {
        printf(" - Call type 5 (binary return) Error (rc=%d)\n", rc);
        if (ret) printf(" - Call return: %s\n", ret);
        else printf(" - No Return String\n");
    }
    if (ret) free(ret);


    rc = CMSfunction("TSTARG2", "TSTARG2 isproc", 1, &ret, 3, "Argument 1",
                     "Argument 2", "Argument 3");
    if (rc > 0 && ret && strcmp(ret, "1") == 0)
        printf(" - Call type 5 (proc) OK\n");
    else {
        printf(" - Call type 5 Error (proc) (rc=%d)\n", rc);
        if (ret) printf(" - Call return: %s\n", ret);
        else printf(" - No Return String\n");
    }
    if (ret) free(ret);


    rc = CMSfunction("TSTARG2", "TSTARG2 isfunc", 0, &ret, 3, "Argument 1",
                     "Argument 2", "Argument 3");
    if (rc > 0 && ret && strcmp(ret, "1") == 0)
        printf(" - Call type 5 (func) OK\n");
    else {
        printf(" - Call type 5 Error (func) (rc=%d)\n", rc);
        if (ret) printf(" - Call return: %s\n", ret);
        else printf(" - No Return String\n");
    }
    if (ret) free(ret);


    char *argv2[] = {"Argument\x001", "Argument\x002", "Argument\x003"};
    int lenv[] = {10, 10, 10};
    rc = CMSfunctionDataArray("TSTARG2", "TSTARG2 binaryargs", 0, &ret, 3,
                              argv2, lenv);
    if (rc == 10 && ret && memcmp(ret, binary, rc) == 0)
        printf(" - Call type 5 (binary args) OK\n");
    else {
        printf(" - Call type 5 (binary args) Error (rc=%d)\n", rc);
        if (ret) printf(" - Call return: %s\n", ret);
        else printf(" - No Return String\n");
    }
    if (ret) free(ret);

    return 0;
}
