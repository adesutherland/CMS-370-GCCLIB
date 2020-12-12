/**************************************************************************************************/
/* GCCLIB Test Suite - IO Infrastructure                                                          */
/*                                                                                                */
/* Part of GCCLIB - VM/370 CMS Native Std C Library; A Historic Computing Toy                     */
/*                                                                                                */
/* Contributors: See contrib memo                                                                 */
/*                                                                                                */
/* Released to the public domain.                                                                 */
/**************************************************************************************************/
#define __FILENAME__ "TSTIO"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmssys.h>
#include <errno.h>

#include "tsts.h"

char *S_INPUT[] = {
        "OUTPUT Test String 1\n",
        "OUTPUT Test String 2\n",
        "OUTPUT Test String 3\n",
        "OUTPUT Test String 4\n",
        "OUTPUT Test String 5\n",
        "OUTPUT Empty Line Next\n",
        "\n",
        "OUTPUT Empty Line Previous\n",
        0
};

char *L_INPUT =
        "OUTPUT Long line [123456789+123456789+123456789+123456789+123456789+12345"
        "6789+123456789+123456789+123456789+123456789+123456789+123456789+123456789+12345"
        "6789+123456789+123456789+123456789+123456789+123456789+123456789+]";

char *FGETC_RD(char *in, int len, FILE *test) {
    int i;
    int c;
    for (i = 0; i < len; i++) {
        c = fgetc(test);
        if (c == EOF) {
            if (feof(test)) break; /* EOF is OK */
            else return NULL; /* Report Error */
        }
        in[i] = (char) c;
        if (c == '\n') {
            i++;
            break;
        }
    }
    in[i] = 0;
    return in;
}

int FPUTC_WR(char *out, FILE *test) {
    int i;
    for (i = 0; out[i]; i++) {
        if (fputc(out[i], test) == EOF) return EOF;
    }
    return 0;
}

int FGETS_T(FILE *test, int lrecl) {
    int i = 0;
    int l, r;
    char line[400];
    char buf[200];
    int ok = 1;

    ok = 1;
    TST_STRT("fgets - short lines");
    while (S_INPUT[i]) {
        if (fgets(buf, 200, test)) {
            if (strcmp(buf, S_INPUT[i])) {
                TST_ERR(buf);
                ok = 0;
            }
        } else {
            if (feof(test)) break;
            TST_PERR();
            ok = 0;
            break;
        }
        i++;
    }
    if (ok) TST_OK();

    ok = 1;
    TST_STRT("fgets - long line");
    l = strlen(L_INPUT);
    i = 0;
    while (i < l) {
        r = lrecl;
        if (r > l - i) r = l - i;

        memcpy(line, L_INPUT + i, r);
        line[r] = '\n';
        line[r + 1] = 0;
        i += r;

        if (fgets(buf, 200, test)) {
            if (strcmp(buf, line)) {
                TST_ERR(buf);
                ok = 0;
            }
        } else {
            if (feof(test)) break;
            TST_PERR();
            ok = 0;
            break;
        }
    }
    if (ok) TST_OK();

    ok = 1;
    TST_STRT("fgets - short input buffer");
    line[0] = 0;
    while (fgets(buf, 10, test)) {

        if (!strncmp("TEST", buf, 4)) { /* Detect end of TEST data */
            while (fgets(buf, 10, test));
            break;
        }
        strcat(line, buf);
        r = strlen(line);
        if (line[r - 1] == '\n') line[r - 1] = 0; /* remove newline */
    }
    if (!feof(test)) {
        TST_PERR();
        ok = 0;
    } else {
        if (strcmp(L_INPUT, line)) {
            TST_ERR(line);
            ok = 0;
        }
    }
    if (ok) TST_OK();
}

int FPUTS_T(FILE *test) {
    int i;
    TST_STRT("fputs");

    fputs("OUTPUT Test String 1\n", test);
    fputs("OUTPUT Test String 2\n", test);
    fputs("OUTPUT Test String 3\nOUTPUT Test String 4\n", test);
    fputs("OUTPUT Test String", test);
    fputs(" 5\n", test);
    fputs("OUTPUT Empty Line Next\n\nOUTPUT Empty Line Previous\n", test);
    fputs("OUTPUT Long line [", test);
    for (i = 0; i < 20; i++) fputs("123456789+", test);
    fputs("]\n", test);
    fputs("OUTPUT Long line [" TEST_STRING TEST_STRING "]\n", test);

    TST_OK();
}

int FPUTC_T(FILE *test) {
    int i;
    TST_STRT("fputc");

    FPUTC_WR("OUTPUT Test String 1\n", test);
    FPUTC_WR("OUTPUT Test String 2\n", test);
    FPUTC_WR("OUTPUT Test String 3\nOUTPUT Test String 4\n", test);
    FPUTC_WR("OUTPUT Test String", test);
    FPUTC_WR(" 5\n", test);
    FPUTC_WR("OUTPUT Empty Line Next\n\nOUTPUT Empty Line Previous\n", test);
    FPUTC_WR("OUTPUT Long line [", test);
    for (i = 0; i < 20; i++) FPUTC_WR("123456789+", test);
    FPUTC_WR("]\n", test);
    FPUTC_WR("OUTPUT Long line [" TEST_STRING TEST_STRING "]\n", test);

    TST_OK();
}


/**************************************************************************************************/
/* FILE * freopen(const char * filespec, const char * access, FILE * stream)                      */
/*                                                                                                */
/* Reuses stream to either open the file specified by filename or to change its access mode.      */
/*                                                                                                */
/* If a new filename is specified, the function first attempts to close any file already          */
/* associated with stream (third parameter) and disassociates it. Then, independently of whether  */
/* that stream was successfuly closed or not, freopen opens the file specified by filename and    */
/* associates it with the stream just as fopen would do using the specified mode.                 */
/*                                                                                                */
/* If filename is a null pointer, the function attempts to change the mode of the stream.         */
/* Although a particular library implementation is allowed to restrict the changes permitted, and */
/* under which circumstances.                                                                     */
/*                                                                                                */
/* The error indicator and eof indicator are automatically cleared (as if clearerr was called).   */
/*                                                                                                */
/* If the file is successfully reopened, the function returns the pointer passed as parameter     */
/* stream, which can be used to identify the reopened stream.                                     */
/* Otherwise, a null pointer is returned.                                                         */
/* The errno variable is also set to a system-specific error code on failure.                     */
/*                                                                                                */
/**************************************************************************************************/
static void freopen_t() {
    FILE *test;
    const char *item = "1234567890abcdefghijklmnopqrstuvwxyz";
    const int count = 20;
    char *buffer;
    char *buffer2;
    int i;

    SUB_STRT("freopen_t()");
    buffer = malloc(count * strlen(item));
    buffer2 = malloc(count * strlen(item));
    buffer[0] = 0;
    buffer2[0] = 0;
    buffer2[count * strlen(item)] = 0;
    for (i = 0; i < count; i++) strcat(buffer, item);
    ASSERTNOTNULLP("fopen(TEST FILE A V,wb)",
                   test = fopen("TEST FILE A V", "wb"), test,);
    ASSERTNOTZERO("fwrite()", ,
                  fwrite(buffer, strlen(item), count, test) == count,);
    ASSERTNOTNULLP("freopen()", , freopen(NULL, "rb", test),);
    ASSERTNOTZERO("fread()", ,
                  fread(buffer2, strlen(item), count, test) == count,);
    ASSERTZERO("strcmp()", , strcmp(buffer, buffer2),);
    system("cp spool punch to *");
    ASSERTNOTNULLP("freopen(PUNCH)", , freopen("PUNCH", "wb", test),);
    ASSERTNOTZERO("fwrite()", ,
                  fwrite(buffer, strlen(item), count, test) == count,);
    ASSERTNOTNULLP("freopen(READER)", , freopen("READER", "rb", test),);
    ASSERTNOTZERO("fread()", ,
                  fread(buffer2, strlen(item), count, test) == count,);
    ASSERTZERO("strcmp()", , strcmp(buffer, buffer2),);
    ASSERTZEROP("fclose()", , fclose(test),);
    free(buffer);
    free(buffer2);
}


/**************************************************************************************************/
/* FILE * tmpfile(void)                                                                           */
/*                                                                                                */
/* Creates a temporary binary file, open for update ("wb+" mode, variable length records) with    */
/* a filename guaranteed to be different from any other existing or open file.                    */
/*                                                                                                */
/* The temporary file created is automatically deleted when the stream is closed (fclose)         */
/* or when the program terminates normally.                                                       */
/*                                                                                                */
/* Returns:                                                                                       */
/*   If successful, the function returns a stream pointer to the temporary file created.          */
/*   On failure, NULL is returned.                                                                */
/**************************************************************************************************/
static void tmpfile_t() {
    FILE *test;
    const char *item = "1234567890abcdefghijklmnopqrstuvwxyz";
    const int count = 20;
    char *buffer;
    char *buffer2;
    int i;

    SUB_STRT("tmpfile_t()");
    buffer = malloc(count * strlen(item));
    buffer2 = malloc(count * strlen(item));
    buffer[0] = 0;
    buffer2[0] = 0;
    buffer2[count * strlen(item)] = 0;
    for (i = 0; i < count; i++) strcat(buffer, item);
    ASSERTNOTNULLP("tmpfile()", test = tmpfile(), test,);
    ASSERTNOTZERO("fwrite()", ,
                  fwrite(buffer, strlen(item), count, test) == count,);
    rewind(test);
    ASSERTNOTZERO("fread()", ,
                  fread(buffer2, strlen(item), count, test) == count,);
    ASSERTZERO("strcmp()", , strcmp(buffer, buffer2),);
    ASSERTZEROP("fclose()", , fclose(test),);
    free(buffer);
    free(buffer2);
}

/**************************************************************************************************/
/* char * tmpnam(char * name)                                                                     */
/*                                                                                                */
/* Generate temporary filename                                                                    */
/* Returns a string containing a file name different from the name of any existing file, and thus */
/* suitable to safely create a temporary file without risking to overwrite an existing file.      */
/*                                                                                                */
/* If str is a null pointer, the resulting string is stored in an internal static array that can  8/
/* be accessed by the return value. The content of this string is preserved at least until a      */
/* subsequent call to this same function, which may overwrite it.                                 */
/*                                                                                                */
/* If str is not a null pointer, it shall point to an array of at least L_tmpnam characters that  */
/* will be filled with the proposed temporary file name.                                          */
/*                                                                                                */
/**************************************************************************************************/
static void tmpnam_t() {
    char name1[21];
    char name2[21];
    char name3[21];
    char *pch;
    int i = 1;
    int bad = 0;

    SUB_STRT("tmpnam()");
    tmpnam(name1);
    tmpnam(name2);
    tmpnam(name3);
    ASSERTNOTZERO("strcmp(name1,name2)", , strcmp(name1, name2),);
    ASSERTNOTZERO("strcmp(name1,name3)", , strcmp(name1, name3),);
    ASSERTNOTZERO("strcmp(name2,name3)", , strcmp(name2, name3),);
    TST_STRT("tmpnam() format");
    pch = strtok(name1, " ");
    while (pch != NULL && !bad) {
        if (i < 3) {
            if (strlen(pch) > 8) {
                TST_ERR("Word 1 or 2 too long");
                bad = 1;
            }
        } else if (i == 3) {
            if (strlen(pch) > 2) {
                TST_ERR("Word 3 too long");
                bad = 1;
            }
        } else {
            TST_ERR("Too many words");
            bad = 1;
        }
        i++;
        pch = strtok(NULL, " ");
    }
    if (i < 3) {
        TST_ERR("Too few words");
        bad = 1;
    }
    if (!bad) TST_OK();
}

/**************************************************************************************************/
/* Run Tests                                                                                      */
/**************************************************************************************************/
void IO_COM_T() {
    SEC_STRT("IO Tests - Common");
    freopen_t();
    tmpfile_t();
    tmpnam_t();
}
