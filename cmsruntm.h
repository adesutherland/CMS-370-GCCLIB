/**************************************************************************************************/
/* CMSRUNTM.H - CMS Runtime Logic header                                                          */
/*                                                                                                */
/* Part of GCCLIB - VM/370 CMS Native Std C Library; A Historic Computing Toy                     */
/*                                                                                                */
/* Proud Contributors: See contrib memo                                                           */
/*                                                                                                */
/* Released to the public domain.                                                                 */
/**************************************************************************************************/

#ifndef CMSRUNTM_INCLUDED
#define CMSRUNTM_INCLUDED
#include <stdio.h>

#ifndef __SIZE_T_DEFINED
#define __SIZE_T_DEFINED
typedef unsigned long size_t;
#endif

/* mspace is an opaque type representing an independent
   region of space that supports malloc(), etc. */
typedef void* mspace;

/* Function Pointers */
typedef int (MAINFUNC)(int argc, char *argv[]); /* declare a main() function pointer */
typedef void (EXITFUNC)(int rc); /* declare a exit() function pointer */
typedef void (SIGHANDLER)(int);  /* Signal Handler */

/* The CMSCRAB macro maps the GCC stack. */
typedef struct CMSCRAB CMSCRAB;

/* The GCCCRAB macro maps the GCC RESLIB Global Area */
typedef struct GCCCRAB GCCCRAB;

/**************************************************************************************************/
/* M U S T   B E   S Y N C E D   W I T H   C M S C R A B   M A C R O                              */
/**************************************************************************************************/
struct CMSCRAB {
  void *dstack;                                       /* Dynamic Stack Control (see below) +00 */
  CMSCRAB *backchain;                                  /*  backchain to previous save area +04 */
  CMSCRAB *forward;                                     /* forward chain to next save area +08 */
  void *regsavearea[15];                      /* register save area and save area chaining +12 */
  GCCCRAB *gcccrab;                               /* GCC C Runtime Anchor Block (GCCCRAB)  +72 */
  void *stackNext;                                     /* next available byte in the stack +76 */
  void *numconv;                                              /* numeric conversion buffer +80 */
  void *funcrslt;                                                /* function result buffer +84 */
  unsigned char locals[];                                           /* Local Variables etc +88 */
};

/**************************************************************************************************/
/* M U S T   B E   S Y N C E D   W I T H   G C C C R A B   M A C R O                              */
/**************************************************************************************************/
struct GCCCRAB {
   CMSCRAB *rootcmscrab;
   CMSCRAB *auxstack;
   CMSCRAB *dynamicstack;
   EXITFUNC *exitfunc;
   mspace dlmspace; /* For DLMALLOC */
   size_t startmemoryusage;
   FILE **stdin;
   FILE **stdout;
   FILE **stderr;
   int *errno;
   SIGHANDLER **handlers;
};

/* To get the addresses of the crabs */
#define GETGCCCRAB() ({GCCCRAB *theCRAB; __asm__("L %0,72(13)" : "=d" (theCRAB)); theCRAB;})
#define GETCMSCRAB() ({CMSCRAB *theCRAB; __asm__("LR %0,13" : "=d" (theCRAB)); theCRAB;})

/**************************************************************************************************/
/* Dynamic Stack Control                                                                          */
/* Low 24 bits contain the address of the byte past the current stack bin                         */
/* The high byte contains the control flags (bits):                                               */
/*  0 - Stack control not in use - ignore (for backward compatability)                            */
/*  1 - Static stack - do not extend with new bins                                                */
/*  2 - Dynamic Stack - can extend                                                                */
/*  4 - I am the first stackframe in the bin (needed to for popping logic to dealocate bins)      */
/*      Note 4 is used with 2 e.g. 6 ...                                                          */
/**************************************************************************************************/
/* Min bin size */
#define _DSK_MINBIN  16*1024
/* Flags */
#define _DSK_NOTUSED 0
#define _DSK_STATIC  1
#define _DSK_DYNAMIC 2
#define _DSK_FIRST   4
#define _DSK_FIRSTDYNAMIC 6
/**************************************************************************************************/

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

/* Stdlib Public Global Variables - RESLIB Defines */
#ifdef IN_RESLIB
#define stdin (*(GETGCCCRAB()->stdin))
#define stdout (*(GETGCCCRAB()->stdout))
#define stderr (*(GETGCCCRAB()->stderr))
#define errno (*(GETGCCCRAB()->errno))
#endif

/* Startup Functions */
int __cstub(PLIST *plist , EPLIST *eplist);
int __cstart(MAINFUNC* mainfunc, PLIST *plist , EPLIST *eplist);

/*
  creat_msp creates, updates GCCCRAB, and returns a new independent space
  with the given initial capacity of the default granularity size
  (16kb).  It returns null if there is no system memory available to
  create the space. Large Chunk Trcking is Enabled.
*/
mspace creat_msp();

/*
  dest_msp destroys the memory space (from GCCCRAB), and attempts
  to return all of its memory back to the system, returning the total number
  of bytes freed. After destruction, the results of access to all memory
  used by the space become undefined.
*/
size_t dest_msp();

/*
  Creates a new stack bin and adds and returns the first frame to the bin.
  The returned frame is of the requested size and is configured and chained to
  the existing frame
  Called by the dynst.assemble routines when the current stack bin runs out of
  space.
*/
CMSCRAB* morestak(CMSCRAB* frame, size_t requested);

/*
  Removes/cleans up unused stack bins.
  Called by the dynst.assemble routines when a first frame in the bin is
  popped / exited.
*/
void lessstak(CMSCRAB* frame);

#endif
