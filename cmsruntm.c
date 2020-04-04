/**************************************************************************************************/
/* CMSRUNTM.C - CMS Runtime Logic (RESLIB)                                                        */
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

int __cstart(MAINFUNC* mainfunc, PLIST *plist, EPLIST *eplist)
{
  GCCCRAB *gcccrab;
  int calltype;
  char *argv[MAXEPLISTARGS];
  char argbuffer[ARGBUFFERLEN];
  int argc = 0;
  int a, b, i, len;
  char ch;
  GCCLIBPLIST *gcclibplist = 0;
  EPLIST *magic_eplist = 0;
  FILE actualConsoleOutputFileHandle;
  char consoleOutputBuffer[132];
  FILE actualConsoleInputFileHandle;
  char consoleInputBuffer[132];

  /* Get the GCCCRAB */
  gcccrab = GETGCCCRAB();

  /* Setup stdout and stderr */
  *(gcccrab->stdout) = &actualConsoleOutputFileHandle;
  *(gcccrab->stderr) = &actualConsoleOutputFileHandle;
  actualConsoleOutputFileHandle.access = 3; /* set access to ACCESS_WRITE_TXT */
  actualConsoleOutputFileHandle.device = 0; /* Console */
  actualConsoleOutputFileHandle.ungetChar = -2; /* 'unget' not yet valid */
  actualConsoleOutputFileHandle.eof = 0;
  actualConsoleOutputFileHandle.next = consoleOutputBuffer;
  actualConsoleOutputFileHandle.error = 0;
  actualConsoleOutputFileHandle.last = consoleOutputBuffer + 130;
  actualConsoleOutputFileHandle.readPos = 0;
  actualConsoleOutputFileHandle.writePos = 0;
  actualConsoleOutputFileHandle.name[0] = 0;
  actualConsoleOutputFileHandle.buffer = consoleOutputBuffer;

  /* Setup stdin */
  *(gcccrab->stdin) = &actualConsoleInputFileHandle;
  actualConsoleInputFileHandle.access = 1;
  actualConsoleInputFileHandle.device = 0; /* Console */
  actualConsoleInputFileHandle.eof = 0;
  actualConsoleInputFileHandle.ungetChar = -2; /* 'unget' not yet valid */
  actualConsoleInputFileHandle.error = 0;
  actualConsoleInputFileHandle.next = consoleInputBuffer;
  actualConsoleInputFileHandle.last = consoleInputBuffer;
  actualConsoleInputFileHandle.readPos = 0;
  actualConsoleInputFileHandle.writePos = 0;
  actualConsoleInputFileHandle.name[0] = 0;
  actualConsoleInputFileHandle.buffer = consoleInputBuffer;

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
