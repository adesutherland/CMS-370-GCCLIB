/**************************************************************************************************/
/* CMSRUNTM.H - CMS Runtime Logic header                                                          */
/*                                                                                                */
/* Part of GCCLIB - VM/370 CMS Native Std C Library; A Historic Computing Toy                     */
/*                                                                                                */
/* Proud Contributors:                                                                            */
/*   Robert O'Hara, Paul Edwards, Dave Wade, with a little hacking by Adrian Sutherland           */
/*                                                                                                */
/* Released to the public domain.                                                                 */
/**************************************************************************************************/
#ifndef CMSRUNTM_INCLUDED
#define CMSRUNTM_INCLUDED
#include <stdio.h>

/* The CMSCRAB macro maps the GCC stack.  In the first stack frame are pointers to useful global
   variables used by routines in CMSSTDIO. */
typedef struct CMSCRAB CMSCRAB;
typedef struct GCCCRAB GCCCRAB;

struct CMSCRAB {
  void *unused;                                                           /* unused by GCC +00 */
  CMSCRAB *backchain;                                  /*  backchain to previous save area +04 */
  CMSCRAB *forward;                                     /* forward chain to next save area +08 */
  void *regsavearea[15];                      /* register save area and save area chaining +12 */
  GCCCRAB *gcccrab;                               /* GCC C Runtime Anchor Block (GCCCRAB)  +72 */
  void *stackNext;                                     /* next available byte in the stack +76 */
  void *numconv;                                              /* numeric conversion buffer +80 */
  void *funcrslt;                                                /* function result buffer +84 */
};

struct GCCCRAB {
   CMSCRAB *rootcmscrab;
   FILE **stdin;
   FILE **stdout;
   FILE **stderr;
   int *errno;
};

/* To get the addresses of the crabs */
#define GETGCCCRAB() ({GCCCRAB *theCRAB; __asm__("L %0,72(13)" : "=d" (theCRAB)); theCRAB;})
#define GETCMSCRAB() ({CMSCRAB *theCRAB; __asm__("LR %0,13" : "=d" (theCRAB)); theCRAB;})

/* Call parameters */
#define MAXEPLISTARGS 32
#define MAXPLISTARGS 16
#define TRUNCPLISTARGS MAXPLISTARGS-8 /* Truncated PLIST: 8 = fence + marker + 6 block eplist */
#define ARGBUFFERLEN 300

typedef char PLIST[8]; /* 8 char block */

typedef struct EVALBLOK EVALBLOK;

struct EVALBLOK {
  EVALBLOK *Next;  /* Reserved - but obvious what the intention was! */
  int BlokSize;    /* Total block size in DW's */
  int Len;         /* length of data in bytes */
  int Pad;         /* (reserved) */
  char Data[];     /* the data... */
};

typedef struct ADLEN {
  char *Data;    /* data... */
  int Len;       /* length of data in bytes */
} ADLEN;

typedef struct EPLIST {
  char *Command;
  char *BeginArgs;          /* start of Argstring */
  char *EndArgs;            /* character after end of the Argstring */
  void *CallContext;        /* Extention Point - IBM points it to a FBLOCK */
  ADLEN *ArgLlist;          /* FUNCTION ARGUMENT LIST - Calltype 5 only */
  EVALBLOK *FunctionReturn; /* RETURN OF FUNCTION - Calltype 5 only */
} EPLIST;

#define EPLISTMARKER "*EPLIST" /* Includes the terminating null */

/* The structure used by GCCLIB designed so that we can find the EPLIST
   when R0 is corrupted */
typedef struct GCCLIBPLIST {
  PLIST plist[TRUNCPLISTARGS + 1];
  char marker[8];
  EPLIST eplist;
} GCCLIBPLIST;

/* Startup Functions */
typedef int (MAINFUNC)(int argc, char *argv[]); /* declare a main() function pointer */
int __cstub(PLIST *plist , EPLIST *eplist);
int __cstart(MAINFUNC* mainfunc, PLIST *plist , EPLIST *eplist);

/* Stdlib Public Global Variables - RESLIB Defines */
#ifdef IN_RESLIB
#define stdin (*(GETGCCCRAB()->stdin))
#define stdout (*(GETGCCCRAB()->stdout))
#define stderr (*(GETGCCCRAB()->stderr))
#define errno (*(GETGCCCRAB()->errno))
#endif

#endif
