/**************************************************************************************************/
/* CMSSYSC.C - CMS System Calls (C part)                                                          */
/*                                                                                                */
/* Part of GCCLIB - VM/370 CMS Native Std C Library; A Historic Computing Toy                     */
/*                                                                                                */
/* Proud Contributors:                                                                            */
/*   Robert O'Hara, Paul Edwards, Dave Wade, with a little hacking by Adrian Sutherland           */
/*                                                                                                */
/* Released to the public domain.                                                                 */
/**************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <cmssys.h>
#include <cmsruntm.h>

/* Low Level Assembler functions */
int __SVC202(PLIST *plist, EPLIST *eplist, int calltype);

int _DMSFRET(void *address, int doublewords);

void *_DMSFREE(int doublewords);

/* Worker Function - create PLIST / EPLIST and call __svc202 */
static int cmd_with_plist(char *command, int calltype) {
    int i, j, len, args;
    PLIST plist[MAXPLISTARGS + 1];
    EPLIST eplist;

    memset((void *) plist, ' ', sizeof(plist));
    eplist.Command = 0;
    eplist.BeginArgs = 0;
    eplist.EndArgs = 0;
    eplist.CallContext = 0;
    eplist.ArgList = 0;
    eplist.FunctionReturn = 0;

    len = strlen(command);
    args = -1;
    for (i = 0; i < len; i++) {
        /* Find start of next word */
        if (command[i] != ' ') {
            /* Process word */
            args++;
            if (args == 1) {
                /* Start of args */
                eplist.BeginArgs = command + i;
            }
            for (j = 0; i < len && j < 8; i++, j++) {
                if (command[i] == ' ') break;
                (plist[args])[j] = toupper(command[i]);
            }
            for (; i < len && command[i] != ' '; i++);
            if (args >= MAXPLISTARGS) break;
        }
    }
    if (eplist.BeginArgs) eplist.EndArgs = command + len;
    eplist.Command = command;

    /* PLIST FENCE */
    args++;
    for (j = 0; j < 8; j++) {
        (plist[args])[j] = 0xFF;
    }

    return __SVC202(plist, &eplist, calltype);
}

/* Main Function - has the search logic for call type 11 (CMS_CONSOLE) */
int __CMSCMD(char *command, int calltype) {

    char fileid[28];
    CMSFILEINFO *fst;
    int rc;
    int len, i, j;
    char program[9];
    char *newcommand;

    if (calltype == CMS_CONSOLE) {
        len = strlen(command);
        /* Skip spaces */
        for (i = 0; i < len && command[i] == ' '; i++);

        /* Get the program */
        for (j = 0; i < len && command[i] != ' '; i++) {
            program[j++] = toupper(command[i]);
            if (j > 8) break;
        }
        program[j] = 0;

        /* Special cases */
        if (!strcmp(program, "EXEC")) return cmd_with_plist(command, calltype);
        if (!strcmp(program, "CP")) {
            rc = cmd_with_plist(command, calltype);
            if (rc == 1) rc = -1;
            return rc;
        }

        /* Does an EXEC exist? */
        strcpy(fileid, "        EXEC    * ");
        strncpy(fileid, program, strlen(program));
        rc = CMSfileState(fileid, &fst);
        if (!rc) {
            newcommand = (char *) malloc(strlen(command) + 6);
            newcommand[0] = 0;
            strcat(newcommand, "EXEC ");
            strcat(newcommand, command);
            rc = cmd_with_plist(newcommand, calltype);
            free(newcommand);
            return rc;
        }

        /* Try the command naked */
        rc = cmd_with_plist(command, calltype);
        if (rc != -3) return rc;

        /* Finally see if it is a CP command */
        newcommand = (char *) malloc(strlen(command) + 4);
        newcommand[0] = 0;
        strcat(newcommand, "CP ");
        strcat(newcommand, command);
        rc = cmd_with_plist(newcommand, calltype);
        free(newcommand);
        if (rc == 1) return -3;
        else return rc;
    }

        /* If not call type 11 just try the command naked */
    else return cmd_with_plist(command, calltype);
}

/**************************************************************************************************/
/* Two functions to help C programs avoid having any non-const static or global                   */
/* variables. For programs planning to be run from a shared segment the program they will be      */
/* running in read-only memory and the CMS linker does not allow global variables to be placed in */
/* another segment.                                                                               */
/*                                                                                                */
/* IBM calls these programs (ones that don't write to their TEXT segment) reentrant programs.     */
/* I have coined this memory - Process Global Memory.                                             */
/*                                                                                                */
/* void* CMSPGAll(size_t size) - Allocate / Reallocate Process Global Memory Block                */
/* void* CMSGetPG(void) - Get the address of the Process Global Memory *** MACRO in cmssys.h ***  */
/*                                                                                                */
/* Note: that this area is freed automatically on normal program termination.                     */
/*                                                                                                */
/**************************************************************************************************/
void *CMSPGAll(size_t size) {
    GCCCRAB *crab;

    crab = GETGCCCRAB();

    if (crab->process_global) free(crab->process_global);

    if (size) crab->process_global = malloc(size);
    else crab->process_global = 0;

    return crab->process_global;
}

/**************************************************************************************************/
/* Call Type 5 (function) call                                                                    */
/* __CMSFND()                                                                                     */
/* int CMSfunctionDataArray(char *physical, char *logical, int is_proc, char **ret_val            */
/*              int argc, char *argv[], int lenv[])                                               */
/* Args: physical - physical function name (used to find the function and in the PLIST)           */
/*       logical  - logical function name ("as entered by the user" used in the EPLIST)           */
/*       is_proc  - 0 - if the routine is called as a function                                    */
/*                  1 - if the routine is called as a subroutine                                  */
/*       argc     - Number of arguments                                                           */
/*       argv     - Array of argument strings                                                     */
/*       lenv     - Array of argument lengths                                                     */
/*       ret_val  - pointer to pointer (handle) of the returned value                             */
/*                  if this is zero no return value is processed                                  */
/*                  On error or if there is no return value the pointer is set to zero            */
/*                  otherwise it is set to a char* buffer (the called must free() this memory)    */
/*                  The buffer is null terminated for C convenience                               */
/*                                                                                                */
/* returns >= 0 success, value is length of data returned                                         */
/*         -1 invalid arguments                                                                   */
/*         -2 error dmsfret error                                                                 */
/*         -3 routine not found (from svc202)                                                     */
/*         Other negative rc from svc202 / called function (a positive RC is make negative)       */
/**************************************************************************************************/
int
__CMSFND(char *physical, char *logical, int is_proc, char **ret_val, int argc,
         char *argv[], int lenv[]) {
    int i, j, len, a, rc;
    PLIST plist[MAXPLISTARGS + 1];
    EPLIST eplist;
    EPLIST *pointer_eplist;
    EVALBLOK *evalblock;

    /* Validate args */
    if (!(physical && strlen(physical))) return -1;
    if (!(logical && strlen(logical))) logical = physical;

    /* Clear Structures */
    memset((void *) plist, ' ', sizeof(plist));
    eplist.Command = 0;
    eplist.BeginArgs = 0;
    eplist.EndArgs = 0;
    eplist.CallContext = 0;
    eplist.ArgList = 0;
    eplist.FunctionReturn = &evalblock;
    evalblock = 0;
    if (ret_val) *ret_val = 0;

    /* Process the physical string */
    len = strlen(physical);
    a = -1;
    for (i = 0; i < len; i++) {
        /* Find start of next word */
        if (physical[i] != ' ') {
            /* Process word */
            a++;
            for (j = 0; i < len && j < 8; i++, j++) {
                if (physical[i] == ' ') break;
                (plist[a])[j] = toupper(physical[i]);
            }
            for (; i < len && physical[i] != ' '; i++);
            if (a >= MAXPLISTARGS) break;
        }
    }
    /* Add the PLIST fence */
    a++;
    for (j = 0; j < 8; j++) {
        (plist[a])[j] = 0xFF;
    }

    /* Process the logical string */
    len = strlen(logical);
    a = -1;
    for (i = 0; i < len; i++) {
        /* Find start of next word */
        if (logical[i] != ' ') {
            /* Process word */
            a++;
            if (a == 0) {
                eplist.Command = logical + i;
                for (; i < len; i++) if (logical[i] == ' ') break;
            } else {
                /* Start of a */
                eplist.BeginArgs = logical + i;
                break;
            }
        }
    }
    if (eplist.BeginArgs) eplist.EndArgs = logical + len;

    /* Process Arguments */
    eplist.ArgList = (ADLEN *) malloc((argc + 1) * sizeof(ADLEN));
    for (a = 0; a < argc; a++) {
        eplist.ArgList[a].Data = argv[a];
        eplist.ArgList[a].Len = lenv[a];
    }
    eplist.ArgList[a].Data = -1;
    eplist.ArgList[a].Len = -1;

    /* Process is_proc */
    pointer_eplist = &eplist;
    if (is_proc) {
        /* if a proc (subroutine) sets register 0 (EPLIST) Bit 0 (MSB) */
        (int) pointer_eplist |= 0x80000000;
    }

    /* Actual Call */
    rc = __SVC202(plist, pointer_eplist, 5);
    if (rc > 0) rc = 0 - rc;

    /* Handle return value */
    if (evalblock && ret_val) {
        *ret_val = malloc(evalblock->Len + 1);
        memcpy(*ret_val, evalblock->Data, evalblock->Len);
        (*ret_val)[evalblock->Len] = 0;
        rc = evalblock->Len;
    }

    /* Release evalblok memory */
    if (evalblock) {
        if (_DMSFRET((void *) evalblock, evalblock->BlokSize)) {
            /* Free allocated memory */
            free(eplist.ArgList);
            return -2;
        };
    }

    /* Free allocated memory */
    free(eplist.ArgList);

    return rc;
}

/**************************************************************************************************/
/* Call Type 5 (function) call                                                                    */
/* __CMSFNA()                                                                                     */
/* int CMSfunctionArray(char *physical, char *logical, int is_proc, char **ret_val                */
/*              int argc, char *argv[])                                                           */
/* Args: physical - physical function name (used to find the function and in the PLIST)           */
/*       logical  - logical function name ("as entered by the user" used in the EPLIST)           */
/*       is_proc  - 0 - if the routine is called as a function                                    */
/*                  1 - if the routine is called as a subroutine                                  */
/*       argc     - Number of arguments                                                           */
/*       argv     - Array of argument strings                                                     */
/*       ret_val  - pointer to pointer (handle) of the returned value                             */
/*                  if this is zero no return value is processed                                  */
/*                  On error or if there is no return value the pointer is set to zero            */
/*                  otherwise it is set to a char* buffer (the called must free() this memory)    */
/*                  The buffer is null terminated for C convenience                               */
/*                                                                                                */
/* returns >= 0 success, value is length of data returned                                         */
/*         -1 invalid arguments                                                                   */
/*         -2 error dmsfret error                                                                 */
/*         -3 routine not found (from svc202)                                                     */
/*         Other negative rc from svc202 / called function (a positive RC is make negative)       */
/**************************************************************************************************/
int
__CMSFNA(char *physical, char *logical, int is_proc, char **ret_val, int argc,
         char *argv[]) {
    int *lenv;
    int rc, a;

    /* Process Arguments */
    lenv = (int *) malloc(argc * sizeof(int));
    for (a = 0; a < argc; a++) {
        lenv[a] = strlen(argv[a]);
    }

    rc = __CMSFND(physical, logical, is_proc, ret_val, argc, argv, lenv);

    free(lenv);

    return rc;
}

/**************************************************************************************************/
/* Call Type 5 (function) call                                                                    */
/* __CMSFNC()                                                                                     */
/* int CMSfunction(char *physical, char *logical, int is_proc, char **ret_val, int argc, ...)     */
/*                                                                                                */
/* Args: physical - physical function name (used to find the function and in the PLIST)           */
/*       logical  - logical function name ("as entered by the user" used in the EPLIST)           */
/*       is_proc  - 0 - if the routine is called as a function                                    */
/*                  1 - if the routine is called as a subroutine                                  */
/*       ret_val  - pointer to pointer (handle) of the returned value                             */
/*                  if this is zero no return value is processed                                  */
/*                  On error or if there is no return value the pointer is set to zero            */
/*                  otherwise it is set to a char* buffer (the called must free() this memory)    */
/*       argc     - number of arguments                                                           */
/*       ...      - Arguments                                                                     */
/*                                                                                                */
/* returns >= 0 success, value is length of data returned                                         */
/*         -1 invalid arguments                                                                   */
/*         -2 error dmsfret error                                                                 */
/*         -3 routine not found (from svc202)                                                     */
/*         Other negative rc from svc202 / called function (a positive RC is make negative)       */
/**************************************************************************************************/
int
__CMSFNC(char *physical, char *logical, int is_proc, char **ret_val, int argc,
         ...) {
    va_list valist;
    int rc;
    int i;
    char **argv;

    /* initialize */
    va_start(valist, argc);
    argv = malloc(argc * sizeof(char *));

    /* Set up arguments */
    for (i = 0; i < argc; i++) {
        argv[i] = va_arg(valist, char*);
    }

    rc = __CMSFNA(physical, logical, is_proc, ret_val, argc, argv);

    /* release memory */
    va_end(valist);
    free(argv);

    return rc;
}

/**************************************************************************************************/
/* Get Program ARGV Vector (vector of arguments)                                                  */
/* __ARGV()                                                                                       */
/* char **CMSargv(void)                                                                           */
/**************************************************************************************************/
char **__ARGV(void) {
    return GETGCCCRAB()->argv;
}

/**************************************************************************************************/
/* Get Program ARGC value (number of arguments)                                                   */
/* __ARGC()                                                                                       */
/* int CMSargc(void)                                                                              */
/**************************************************************************************************/
int __ARGC(void) {
    return GETGCCCRAB()->argc;
}

/**************************************************************************************************/
/* Get Program PLIST Structure                                                                    */
/* __PLIST()                                                                                      */
/* PLIST *CMSplist(void)                                                                          */
/**************************************************************************************************/
PLIST *__PLIST(void) {
    return GETGCCCRAB()->plist;
}

/**************************************************************************************************/
/* Get Program EPLIST Structure                                                                   */
/* __EPLIST()                                                                                     */
/* EPLIST *CMSeplist(void)                                                                        */
/**************************************************************************************************/
EPLIST *__EPLIST(void) {
    return GETGCCCRAB()->eplist;
}

/**************************************************************************************************/
/* Get Program Call Type                                                                          */
/* __CALLTP()                                                                                     */
/* int CMScalltype(void)                                                                          */
/**************************************************************************************************/
int __CALLTP(void) {
    return GETGCCCRAB()->calltype;
}

/**************************************************************************************************/
/* Returns 1 if the calltype 5 procedure/subroutine flag was set (a return value is not required) */
/* __ISPROC()                                                                                     */
/* int CMSisproc(void)                                                                            */
/**************************************************************************************************/
int __ISPROC(void) {
    return GETGCCCRAB()->isproc;
}

/**************************************************************************************************/
/* Sets the return value (string). This is only valid if the program is called via calltype 5     */
/* __RETVAL(char*)                                                                                */
/* int CMSreturnvalue(char*)                                                                      */
/* returns 0 on success or 1 if the calltype is not 5, or the return value has already been set   */
/**************************************************************************************************/
int __RETVAL(char *value) {
    GCCCRAB *gcccrab;
    int ret_size;
    gcccrab = GETGCCCRAB();

    if ((gcccrab->calltype == 5) && !gcccrab->evalblok &&
        gcccrab->eplist->FunctionReturn) {
        ret_size = strlen(value);
        /* integer roundup -> "add the divisor minus one to the dividend" */
        int db_size = (sizeof(EVALBLOK) + ret_size + (8 - 1)) / 8;
        gcccrab->evalblok = _DMSFREE(db_size);
        gcccrab->evalblok->BlokSize = db_size;
        gcccrab->evalblok->Len = ret_size;
        strcpy(gcccrab->evalblok->Data, value);
        *(gcccrab->eplist->FunctionReturn) = gcccrab->evalblok;
        return 0;
    } else return 1;
}

/**************************************************************************************************/
/* Sets the return value (data). This is only valid if the program is called via calltype 5       */
/* __RETDATA(void* data, int len)                                                                 */
/* int CMSreturndata(void* data, int len)                                                         */
/* returns 0 on success or 1 if the calltype is not 5, or the return value has already been set   */
/**************************************************************************************************/
int __RETDATA(void *data, int len) {
    GCCCRAB *gcccrab;
    gcccrab = GETGCCCRAB();

    if ((gcccrab->calltype == 5) && !gcccrab->evalblok &&
        gcccrab->eplist->FunctionReturn) {
        /* integer roundup -> "add the divisor minus one to the dividend" */
        int db_size = (sizeof(EVALBLOK) + len + (8 - 1)) / 8;
        gcccrab->evalblok = _DMSFREE(db_size);
        gcccrab->evalblok->BlokSize = db_size;
        gcccrab->evalblok->Len = len;
        memcpy(gcccrab->evalblok->Data, data, len);
        *(gcccrab->eplist->FunctionReturn) = gcccrab->evalblok;
        return 0;
    } else return 1;
}

/**************************************************************************************************/
/* Sets the return value (int). This is only valid if the program is called via calltype 5        */
/* __RETINT(int)                                                                                  */
/* int CMSreturnvalint(int rc)                                                                    */
/* Returns 0 on success or input rc if the calltype is not 5, or the return value has already     */
/* been set                                                                                       */
/**************************************************************************************************/
int __RETINT(int value) {
    GCCCRAB *gcccrab;
    int ret_size = 11;
    gcccrab = GETGCCCRAB();

    if ((gcccrab->calltype == 5) && !gcccrab->evalblok &&
        gcccrab->eplist->FunctionReturn) {
        /* integer roundup -> "add the divisor minus one to the dividend" */
        int db_size = (sizeof(EVALBLOK) + ret_size + (4 - 1)) / 4;
        gcccrab->evalblok = _DMSFREE(db_size);
        gcccrab->evalblok->BlokSize = db_size;
        sprintf(gcccrab->evalblok->Data, "%d", value);
        gcccrab->evalblok->Len = strlen(gcccrab->evalblok->Data);
        *(gcccrab->eplist->FunctionReturn) = gcccrab->evalblok;
        return 0;
    } else return value;
}

/**************************************************************************************************/
/* Sets the debug mode (0=off, else debug on. It currently just turns on/off the memory leak check*/
/* __SDEBUG(int)                                                                                   */
/* int CMSdebug(int debug_flag)                                                                   */
/**************************************************************************************************/
void __SDEBUG(int debug_flag) {
    GCCCRAB *gcccrab;
    gcccrab = GETGCCCRAB();
    gcccrab->debug = debug_flag;
}
