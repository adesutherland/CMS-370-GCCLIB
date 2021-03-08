/**************************************************************************************************/
/* GCCLIB Test Suite - Infrastructure                                                             */
/*                                                                                                */
/* Part of GCCLIB - VM/370 CMS Native Std C Library; A Historic Computing Toy                     */
/*                                                                                                */
/* Contributors: See contrib memo                                                                 */
/*                                                                                                */
/* Released to the public domain.                                                                 */
/**************************************************************************************************/
#define __FILENAME__ "TSTS"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <cmssys.h>

#include "tsts.h"

/* Main Test Entry */
int main(int argc, char *argv[]) {
    CMSSYS_T();
    IO_CON_T();
    IO_PUN_T();
    IO_PRT_T();
    IO_RDR_T();
    IO_VFL_T();
    IO_FFL_T();
    IO_COM_T();
    REPORT();
    return 0;
}

/* Test Infrastructure */
int LEVEL1 = 0;
int LEVEL2 = 0;
int LEVEL3 = 0;
char *LEVEL1_D = "";
char *LEVEL2_D = "";
char *LEVEL3_D = "";
int ERRORS = 0;
int OK = 0;

void REPORT() {
    printf("TEST SUMMARY\n"
           "- OK      : %d\n"
           "- ERRORS  : %d\n"
           "- TOTAL   : %d\n",
           OK, ERRORS, OK + ERRORS);
}

void SEC_STRT(char *title) {
    LEVEL1++;
    LEVEL2 = 0;
    LEVEL3 = 0;
    LEVEL1_D = title;
    LEVEL2_D = "";
    LEVEL3_D = "";
    printf("\nTEST SECTION  %d: %s\n", LEVEL1, LEVEL1_D);
}

void SUB_STRT(char *title) {
    LEVEL2++;
    LEVEL3 = 0;
    LEVEL2_D = title;
    LEVEL3_D = "";
    printf("\nTEST SUBSECTION  %d.%d: %s\n", LEVEL1, LEVEL2, LEVEL2_D);
}

void TST_STRT(char *title) {
    LEVEL3++;
    LEVEL3_D = title;
}

void TST_OK() {
/*
  printf("TEST OK %d.%d.%d (%s)\n", LEVEL1, LEVEL2, LEVEL3, LEVEL3_D);
*/
    OK++;
}

void _TST_ERR(char *s, char *file, int line) {
    printf("TEST ERROR %d.%d.%d (%s) %s LINE %d %s\n", LEVEL1, LEVEL2, LEVEL3,
           LEVEL3_D, file, line, s);
    ERRORS++;
}

void _TST_PERR(char *file, int line) {
    printf("TEST ERROR %d.%d.%d (%s) %s LINE %d %s\n", LEVEL1, LEVEL2, LEVEL3,
           LEVEL3_D, file, line, strerror(errno));
    ERRORS++;
}

void MAKEFILE(char *id) {
    FILE *f;
    f = fopen(id, "w");
    if (f) {
        fputs("Test File\n", f);
        fclose(f);
    } else perror("MAKEFILE");
}

void DELFILE(char *id) {
    int status;
    status = remove(id);
    if (status != 0) printf("%d returned by remove(\"%s\")\n", status, id);
}

char *TRIM(char *str) {
    char *end;
/*
  Leading white space
  while(isspace(*str)) str++;
  if(*str == 0) return str;
*/
    /* trailing white space */
    end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;
    end[1] = '\0';
    return str;
}
