/**************************************************************************************************/
/* MAKELIB.C - Make GCCLIB Dynamic Library Stubs                                                  */
/*                                                                                                */
/* Part of GCCLIB - VM/370 CMS Native Std C Library; A Historic Computing Toy                     */
/*                                                                                                */
/* Contributors: See contrib memo                                                                 */
/*                                                                                                */
/* Released to the public domain.                                                                 */
/**************************************************************************************************/

#include <stdio.h>
#include <string.h>
#include <cmssys.h>
#include <errno.h>
#include <cmsruntm.h>

static char *trim(char *str);

static char *toUpperString(char *string);

static void writeStub(int n);

#define MAXFUNCTIONS 300
#define DRIVE "A"
#define VTABLE "CMSVTAB ASSEMBLE"

static char functions[MAXFUNCTIONS][9];

int main(int argc, char *argv[]) {
    FILE *input;
    FILE *vtable;
    char parm_name[30];
    char line[100];
    char *function;
    int count = 0;
    int i;
    int line_number = 0;

    printf("GCCLIB - Dynamic Library Stub Builder - v1.0\n");
    if (argc != 4) {
        printf("Usage: makelib fn ft fm\n");
        printf("This reads the input file (e.g. MEMBERS PARM), and generates\n");
        printf("the VTABLE (" VTABLE ") and STUB files, and stacks the stub file\n");
        printf("names so they can be read by an EXEC and assembled\n");
        printf("See MAKELIB C and MKGCCLIB EXEC\n");
        return 1;
    }

    /* Read input file */
    strcpy(parm_name, argv[1]);
    strcat(parm_name, " ");
    strcat(parm_name, argv[2]);
    strcat(parm_name, " ");
    strcat(parm_name, argv[3]);
    input = fopen(parm_name, "r");

    if (!input) {
        perror("Cannot open input file");
        return 1;
    }
    while (!feof(input)) {
        if (fgets(line, 100, input)) {
            line_number++;
            function = trim(line);
            if (function[0] && function[0] != '*') {
                toUpperString(function);
                if (strlen(function) > 8) {
                    printf("Error on line %d, function %s is too long\n",
                           line_number, function);
                    return 1;
                }
                if (count >= MAXFUNCTIONS) {
                    printf("Error: More than %d stubs\n", MAXFUNCTIONS);
                    return 1;
                }
                strcpy(functions[count], function);
                count++;
            }
        } else if (ferror(input)) {
            perror("Error reading input file");
            return 1;
        }
    }
    fclose(input);

    /* Create Vector Table */
    vtable = fopen(VTABLE " " DRIVE " F 80", "w");
    if (!vtable) {
        perror("Cannot open/create " VTABLE " " DRIVE);
        return 1;
    }

    fputs("*   * * * A U T O   G E N E R A T E D * * *\n", vtable);
    fputs("*   ANY CHANGES WILL BE OVERWRITTEN!\n", vtable);
    fputs("*   Refer to and make changes in MAKELIB C\n", vtable);
    fputs("CMSVTAB  CSECT\n", vtable);
    fputs("         DC    CL8'GCCLIB'\n", vtable);
    fputs("         DC    AL1(MAJORVR)\n", vtable);
    fputs("         DC    AL1(MINORVR)\n", vtable);
    fputs("         DC    AL1(REVISION)\n", vtable);
    fputs("         DC    AL1(0)\n", vtable);
    fputs("         DC    AL4(0)\n", vtable);
    for (i = 0; i < count; i++) {
        fprintf(vtable, "         DC    V(%s)\n", functions[i]);
    }
    fputs("         GCCLIBVR\n", vtable);
    fputs("         END\n", vtable);
    fclose(vtable);

    /* Create Stubs */
    for (i = 0; i < count; i++) writeStub(i);

    return 0;
}

void writeStub(int n) {
    char fileName[30];
    FILE *file;
    sprintf(fileName, "STUB%04d ASSEMBLE " DRIVE " F 80", n);
    file = fopen(fileName, "w");

    if (!file) {
        fprintf(stderr, "ERROR Creating file %s. ", fileName);
        perror("");
        exit(1);
    }

    fprintf(file, "*   * * * A U T O   G E N E R A T E D * * *\n");
    fprintf(file, "*   ANY CHANGES WILL BE OVERWRITTEN!\n");
    fprintf(file, "*   Refer to and make changes in MAKELIB C\n");
    fprintf(file, "%-8s CSECT\n", functions[n]);
    fprintf(file, "         USING CMSCRAB,13  Address of save area, stack etc\n");
    fprintf(file, "         L     15,GCCCRABA Get address of ... \n");
    fprintf(file, "         USING GCCCRAB,15  ... GCC C runtime anchor block\n");
    fprintf(file, "         L     15,GCCLIBVT Get address of GCCLIB vector table\n");
    fprintf(file,
            "         L     15,%d(15)  Address of routine, offset in CMSVTAB\n",
            n * 4);
    fprintf(file, "         BR    15\n");
    fprintf(file, "         GCCCRAB\n");
    fprintf(file, "         CMSCRAB\n");
    fprintf(file, "         END\n");

/*
  fprintf(file, "*   * * * A U T O   G E N E R A T E D * * *\n");
  fprintf(file, "*   ANY CHANGES WILL BE OVERWRITTEN!\n");
  fprintf(file, "*   Refer to and make changes in MAKELIB C\n");
  fprintf(file, "%-8s CSECT\n",functions[n]);
  fprintf(file, "         USING NUCON,0\n");
  fprintf(file, "         L     15,GCCPTR         Load address of CMSVTAB\n");
  fprintf(file, "         L     15,%d(,15)        Load address of real routine\n", n*4);
  fprintf(file, "         BR    15\n");
  fprintf(file, "         LTORG\n");
  fprintf(file, "         REGEQU\n");
  fprintf(file, "         NUCON ,\n");
  fprintf(file, "GCCPTR   EQU   NUCRSV7          Reserved word for the last 45 years\n");
  fprintf(file, "         END\n");
*/
    fclose(file);

    sprintf(fileName, "STUB%04d", n);
    CMSstackLine(fileName, CMS_STACKFIFO);
}

static char *trim(char *str) {
    char *end;

    while (*str == ' ' || *str == '\n' || *str == '\t') str++;

    end = str + strlen(str) - 1;
    while (end >= str &&
           (*end == ' ' || *end == '\n' || *end == '\t' || *end == '\r'))
        end--;

    /* Terminate */
    end[1] = 0;

    return str;
}

static char *toUpperString(char *string) {
    int i;
    for (i = 0; string[i]; i++) string[i] = toupper(string[i]);
    return string;
}
