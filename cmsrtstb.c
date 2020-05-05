/**************************************************************************************************/
/* CMSRTSTB.C - CMS Runtime Stub to be linked to the Program (STUB)                               */
/*                                                                                                */
/* Part of GCCLIB - VM/370 CMS Native Std C Library; A Historic Computing Toy                     */
/*                                                                                                */
/* Proud Contributors:                                                                            */
/*   Robert O'Hara, Paul Edwards, Dave Wade, with a little hacking by Adrian Sutherland           */
/*                                                                                                */
/* Released to the public domain.                                                                 */
/**************************************************************************************************/
#include <cmsruntm.h>
#include <stdio.h>
#include <signal.h>
#include <cmssys.h> /* For __wrterm() workaround */

/* Stdlib Public Global Variables */
FILE *stdin;               /* predefined stream for standard input: we map it to console */
FILE *stdout;              /* predefined stream for standard output: we map it to console */
FILE *stderr;              /* predefined stream for error output: we map it to console */
int errno = 0;             /* Std error number */

/* Space for Signal Handlers, need to add the actual default handlers when in RESLIB */
static void (*handlers[])(int) = {0, 0, 0, 0, 0, 0};

int main(int argc, char *argv[]);

void __exit(int rc);

int __cstub(PLIST *plist , EPLIST *eplist)
{
  __wrterm("",0); /* Workaround to reset stdout - TODO root fix, probably in stdio */

  GCCCRAB gcccrab;
  CMSCRAB *cmscrab;

  /* Fixup the GCCCRAB */
  cmscrab = GETCMSCRAB();
  cmscrab->gcccrab = &gcccrab;
  /* And add it to my parent (the root) CMSCRAB */
  cmscrab = cmscrab->backchain;
  cmscrab->gcccrab = &gcccrab;
  gcccrab.rootcmscrab = cmscrab;

  /* Set Global Variables */
  gcccrab.stdin = &stdin;
  gcccrab.stdout = &stdout;
  gcccrab.stderr = &stderr;
  gcccrab.errno = &errno;
  gcccrab.exitfunc = __exit;
  gcccrab.handlers = handlers;

  return(__cstart(main, plist, eplist));
}
