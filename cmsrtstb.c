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

/* Stdlib Public Global Variables */
FILE *stdin;               /* predefined stream for standard input: we map it to console */
FILE *stdout;              /* predefined stream for standard output: we map it to console */
FILE *stderr;              /* predefined stream for error output: we map it to console */
int errno = 0;             /* Std error number */

int main(int argc, char *argv[]);

int __cstub(PLIST *plist , EPLIST *eplist)
{
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

  return(__cstart(main, plist, eplist));
}
