/**************************************************************************************************/
/* GCCCRAB.H - GCC CRAB FOR GLOBAL VARIABLES NOT STORED IN THE TEXT SEGMENT                       */
/*                                                                                                */
/* Part of GCCLIB - VM/370 CMS Native Std C Library; A Historic Computing Toy                     */
/*                                                                                                */
/* Contributors: See contrib memo                                                                 */
/*                                                                                                */
/* Released to the public domain.                                                                 */
/**************************************************************************************************/

#ifndef GCCCRAB_INCLUDED
#define GCCCRAB_INCLUDED
#include <stdio.h>
#include <cmssys.h>

#ifndef __SIZE_T_DEFINED
#define __SIZE_T_DEFINED
typedef unsigned long size_t;
#endif

/* Function Pointers */
typedef void (EXITFUNC)(int rc); /* declare a exit() function pointer */
typedef void (SIGHANDLER)(int);  /* Signal Handler */
typedef void (USEREXIT)(void);   /* User Exits */

/* The GCCCRAB macro maps the GCC RESLIB Global Area */
typedef struct GCCCRAB GCCCRAB;

/**************************************************************************************************/
/* M U S T   B E   S Y N C E D   W I T H   G C C C R A B   M A C R O                              */
/**************************************************************************************************/
struct GCCCRAB {
   struct CMSCRAB *rootcmscrab;
   struct CMSCRAB *auxstack;
   struct CMSCRAB *dynamicstack;
   EXITFUNC *exitfunc;
   void* dlmspace; /* For DLMALLOC */
   size_t startmemoryusage;
   FILE *gstdin;
   FILE *gstdout;
   FILE *gstderr;
   int gerrno;
   SIGHANDLER **handlers;
   USEREXIT **userexits;
   FILE* filehandles;
   char *strtok_old;
   void *process_global;
   char **argv;
   char *argbuffer;
   PLIST *plist;
   EPLIST *eplist;
   int calltype;
   EVALBLOK *evalblok;
   int isproc;
   int argc;
};

/* To get the addresses of the crab */
#define GETGCCCRAB() ({GCCCRAB *theCRAB; __asm__("L %0,72(13)" : "=d" (theCRAB)); theCRAB;})

#endif
