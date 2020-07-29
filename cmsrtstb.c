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
#include <stdlib.h>

int main(int argc, char *argv[]);

void __exit(int rc);

int __cstub(PLIST *plist , EPLIST *eplist)
{
  GCCCRAB gcccrab;
  CMSCRAB *cmscrab;
  int x;

  /* Default handlers */
  SIGHANDLER *handlers[6] = {SIG_DFL, SIG_DFL, SIG_DFL, SIG_DFL, SIG_DFL, SIG_DFL};

  /* User Exits */
  USEREXIT *userexits[__NATEXIT];

  /* Init Exit functions */
  for (x = 0; x < __NATEXIT; x++) userexits[x] = NULL;

  /* Fixup the GCCCRAB */
  cmscrab = GETCMSCRAB();
  cmscrab->gcccrab = &gcccrab;
  /* And add it to my parent (the root) CMSCRAB */
  cmscrab = cmscrab->backchain;
  cmscrab->gcccrab = &gcccrab;
  gcccrab.rootcmscrab = cmscrab;

  /* Set Global Variables */
  gcccrab.exitfunc = __exit;
  gcccrab.handlers = handlers;
  gcccrab.userexits = userexits;
  gcccrab.filehandles = NULL;
  gcccrab.strtok_old = NULL;
  gcccrab.process_global = NULL;

  return(__cstart(main, plist, eplist));
}
