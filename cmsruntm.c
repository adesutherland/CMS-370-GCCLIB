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
#include "cmsruntm.h"
#define IN_RESLIB
#include "stdio.h"
#include "stddef.h"
#include "string.h"

int __cstart(MAINFUNC* mainfunc, PLIST *plist, EPLIST *eplist)
{
  GCCCRAB *gcccrab;
  int calltype;
  char *argv[MAXEPLISTARGS];
  char argbuffer[ARGBUFFERLEN];
  int argc = 0;
  int a, b, i, len;
  int rc;
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

  /* Initialize Heap */
  creat_msp();

  /* Setup Stacks */
  CMSCRAB *currentstackframe = GETCMSCRAB();

  /* Dynamic Stack - done first as this work will effect the bootstrap stack */
  gcccrab->stackfreebin = 0;
  gcccrab->stackfreebinsize = 0;
  gcccrab->dynamicstack = morestak(currentstackframe, 0);

  /* Aux Stack (for dymanic stack memory management and aborting)   */
  /* This just starts at the next stackframe on the bootstrap stack */
  gcccrab->auxstack = currentstackframe->stackNext;
  gcccrab->auxstack->dstack = currentstackframe->dstack;
  gcccrab->auxstack->backchain = currentstackframe;
  gcccrab->auxstack->forward = 0;
  gcccrab->auxstack->gcccrab = currentstackframe->gcccrab;
  gcccrab->auxstack->stackNext = &(gcccrab->auxstack->locals);

  /* Call main() via the dynamic stack                                        */
  /* __CLVSTK() itself will use the register save area in the current stack   */
  /* frame but main() uses the register save area in the dynamic stack and    */
  /* adds a frame from there (hope that makes sense!)                         */
  rc = __CLVSTK(gcccrab->dynamicstack, mainfunc, argc, argv);

  free(gcccrab->dynamicstack);
  if (gcccrab->stackfreebin) free(gcccrab->stackfreebin);
  dest_msp();

  return rc;
}

size_t morestak(CMSCRAB* frame, size_t requested) {
  CMSCRAB *new_frame;
  int bin_size;
  GCCCRAB *gcccrab = GETGCCCRAB();

  /* Check min request */
  if (requested < offsetof(CMSCRAB, locals)) requested = offsetof(CMSCRAB, locals);

  /* Round up bin size to the next page (or min size) */
  if (requested < _DSK_MINBIN) bin_size = _DSK_MINBIN;
  else bin_size = (requested && 0xFFF000) + 0x1000;

  /* Is there a free bin of the right size?                                  */
  /* This logic is just to speedup function calls unlucky eneough to be on a */
  /* bin boundary - avoiding keeping calling malloc                          */
  new_frame = gcccrab->stackfreebin;
  if (new_frame && bin_size<=gcccrab->stackfreebinsize) {
    bin_size = gcccrab->stackfreebinsize;
    gcccrab->stackfreebin = 0;
  }
  else {
    new_frame = malloc(bin_size);
  }

  /* Set it Up */
  new_frame->dstack = ((int)new_frame+bin_size)
                    |  (int)(_DSK_FIRSTDYNAMIC << 24);
  new_frame->backchain = frame;
  new_frame->forward = 0;
  frame->forward = new_frame;
  new_frame->gcccrab = frame->gcccrab;
  new_frame->stackNext = (void*)new_frame + requested;

  return new_frame;
}

void lessstak(CMSCRAB* frame) {
  GCCCRAB *gcccrab = GETGCCCRAB();
  int framesize = ((int)frame->dstack & 0x00FFFFFF)-(int)frame;

  if (!gcccrab->stackfreebin) {
    gcccrab->stackfreebin = frame;
    gcccrab->stackfreebinsize = framesize;
  }
  else if (gcccrab->stackfreebinsize<framesize) {
    /* Keep the biggest */
    free(gcccrab->stackfreebin);
    gcccrab->stackfreebin = frame;
    gcccrab->stackfreebinsize = framesize;
  }
  else {
    /* Throw away this one */
    free(frame);
  }
}
