/**************************************************************************************************/
/* CMSRUNTM.C - CMS Runtime Logic                                                                 */
/*                                                                                                */
/* Part of GCCLIB - VM/370 CMS Native Std C Library; A Historic Computing Toy                     */
/*                                                                                                */
/* Proud Contributors:                                                                            */
/*   Robert O'Hara, Paul Edwards, Dave Wade, with a little hacking by Adrian Sutherland           */
/*                                                                                                */
/* Released to the public domain.                                                                 */
/**************************************************************************************************/
/* TODO detect overruns */
#include "stdio.h"
#include "string.h"
#include "cmsruntm.h"

#define GCCSTDIN 1
#define GCCSTDOUT 2
#define GCCSTDERR 3

typedef int (MAINFUNC)(int argc, char *argv[]); /* declare a main() function pointer */

int __cstart(MAINFUNC* mainfunc, PLIST *plist , EPLIST *eplist)
{
  GCCCRAB gcccrab;
  CMSCRAB *cmscrab;
  int calltype;
  char *argv[MAXEPLISTARGS];
  char argbuffer[ARGBUFFERLEN];
  int argc = 0;
  int a, b, i, len;
  char ch;
  GCCLIBPLIST *gcclibplist = 0;
  EPLIST *magic_eplist = 0;

  /* Fixup the GCCCRAB */
  cmscrab = GETCMSCRAB();
  cmscrab->gcccrab = &gcccrab;
  /* And add it to my parent (the root) CMSCRAB */
  cmscrab = cmscrab->backchain;
  cmscrab->gcccrab = &gcccrab;
  gcccrab.rootcmscrab = cmscrab;

  /* Setup stdout and stderr */
  gcccrab.consoleOutputFile = &(gcccrab.actualConsoleOutputFileHandle);
  gcccrab.actualConsoleOutputFileHandle.access = 3; /* set access to ACCESS_WRITE_TXT */
  gcccrab.actualConsoleOutputFileHandle.device = 0; /* Console */
  gcccrab.actualConsoleOutputFileHandle.ungetChar = -2; /* 'unget' not yet valid */
  gcccrab.actualConsoleOutputFileHandle.eof = 0;
  gcccrab.actualConsoleOutputFileHandle.error = 0;
  gcccrab.actualConsoleOutputFileHandle.next = gcccrab.consoleOutputBuffer;
  gcccrab.actualConsoleOutputFileHandle.last = gcccrab.consoleOutputBuffer + 130;
  gcccrab.actualConsoleOutputFileHandle.readPos = 0;
  gcccrab.actualConsoleOutputFileHandle.writePos = 0;
  gcccrab.actualConsoleOutputFileHandle.name[0] = 0;
  gcccrab.actualConsoleOutputFileHandle.buffer = gcccrab.consoleOutputBuffer;

  /* Setup stdin */
  gcccrab.consoleInputFile = &(gcccrab.actualConsoleInputFileHandle);
  gcccrab.actualConsoleInputFileHandle.access = 1;
  gcccrab.actualConsoleInputFileHandle.device = 0; /* Console */
  gcccrab.actualConsoleInputFileHandle.ungetChar = -2; /* 'unget' not yet valid */
  gcccrab.actualConsoleInputFileHandle.eof = 0;
  gcccrab.actualConsoleInputFileHandle.error = 0;
  gcccrab.actualConsoleInputFileHandle.next = gcccrab.consoleInputBuffer;
  gcccrab.actualConsoleInputFileHandle.last = gcccrab.consoleInputBuffer;
  gcccrab.actualConsoleInputFileHandle.readPos = 0;
  gcccrab.actualConsoleInputFileHandle.writePos = 0;
  gcccrab.actualConsoleInputFileHandle.name[0] = 0;
  gcccrab.actualConsoleInputFileHandle.buffer = gcccrab.consoleInputBuffer;

/*  FILE * stderr = GCCSTDERR;   predefined stream for error output: we map it to the console */
/*  FILE * stdin = GCCSTDIN;     predefined stream for standard input: we map it to the console */
/*  FILE * stdout = GCCSTDOUT;   predefined stream for standard output: we map it to the console */

  stdin = gcccrab.consoleInputFile; /* initialize the stdin file handle */
  stdout = stderr = gcccrab.consoleOutputFile;  /* initialize the stderr and stdout file handles */

  /* The high order byte contains the
     traditional CMS R1 flag byte.  A x'0B' or x'01' indicates the
     presence of an extended parameter accessable via R0 */
  calltype = ((int)(plist) &  0xff000000) >> 24;
  plist = (PLIST*)((int)(plist) & 0xffffff);

  /* MAGIC MARKER - NON-STANDARD - Designed to allow EPLIST to be found
     if R0 is corrupted - So see if the magic eplist marker exists */
  gcclibplist = (GCCLIBPLIST*)((int)plist - (int)&(gcclibplist->plist));
  if (!strcmp(EPLISTMARKER, gcclibplist->marker)) {
    magic_eplist = &(gcclibplist->eplist);
    eplist = magic_eplist;
    /* Delete the marker in case the PLIST space is reused (it sometimes is) */
  }

  /* Process Arguments - note 6 word eplist (for calltype 5) not implemented */
  if ((calltype == 5 && magic_eplist) || calltype == 1 || calltype == 11)
  {
    /* Function name  */
    a=0;
    for (i=0; i<8; i++)
    {
      ch = eplist->Command[i];
      if (ch && ch!=0xFF && ch!=' ') argbuffer[a++] = ch;
      else break;
    }
    argbuffer[a++] = 0;
    argv[argc++] = argbuffer;

    i=0;
    len = eplist->EndArgs-eplist->BeginArgs;
    while (i<len)
    {
       /* Strip Leading Spaces */
       for (; i<len; i++)
       {
         if (eplist->BeginArgs[i] != ' ') break;
       }

       /* Process Word */
       for (b=a; i<len; i++)
       {
         ch = eplist->BeginArgs[i];
         if (ch==' ' || ch==')' || ch=='(') break;
         argbuffer[a++] = ch;
       }

       /* A parameter was added */
       if (a != b)
       {
         argbuffer[a++] = 0;
         argv[argc++] = argbuffer + b;
       }

       /* Handle any ( or )'s as seperate parms */
       if ( ch==')' || ch=='(' )
       {
         argv[argc++] = argbuffer + a;
         argbuffer[a++] = ch;
         argbuffer[a++] = 0;
         i++;
       }
    }
  }
  else
  {
    /* Function name - always in plist[0] */
    for (a=i=0; i<8 && plist[argc][i]!=0xFF && plist[argc][i]!=' '; i++)
      argbuffer[a++] = plist[argc][i];
    argbuffer[a++] = 0;
    argv[argc++] = argbuffer;

    for (; plist[argc][0]!=0xFF; argc++)
    {
        argv[argc] = argbuffer + a;
        for (i=0; i<8 && plist[argc][i]!=' '; i++)
          argbuffer[a++] = plist[argc][i];
        argbuffer[a++] = 0;
    }
  }

  /* Call Main */
  return mainfunc(argc, argv);
}
