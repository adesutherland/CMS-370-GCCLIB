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
/* TODO Paramter Overruns */
#include <cmsruntm.h>

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <signal.h>

#define CMSINLINE static __inline __attribute__ ((always_inline))

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
  char messagebuffer[100];

  /* Get GCCCRAB */
  gcccrab = GETGCCCRAB();

  /* Initialize Memory System */
  struct mallinfo memoryinfo;
  creat_msp();
  memoryinfo = mallinfo();
  gcccrab->startmemoryusage = memoryinfo.uordblks;

  /* Setup stdout and stderr */
  stdout = fopen("+CONSOLE+","w");
  stderr = fopen("+CONSOLE+","w");
  stdin  = fopen("+CONSOLE+","r");

  /* Process parameters */
  /* The high order byte contains the
     traditional CMS R1 flag byte.  A x'0B' or x'01' indicates the
     presence of an extended parameter accessable via R0 (the eplist parameter) */
  calltype = ((int)(plist) &  0xff000000) >> 24;
  plist = (PLIST*)((int)(plist) & 0xffffff);

  /* Process Arguments - note 6 word eplist (for calltype 5) not implemented */
  if (calltype == 5 || calltype == 1 || calltype == 11)
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

  /* Setup Stacks */
  CMSCRAB *currentstackframe = GETCMSCRAB();

  /* Dynamic Stack - done first as this work will effect the bootstrap stack */
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

  /* Call Exit functions */
  for (i = __NATEXIT - 1; i >= 0; i--) {
    if (gcccrab->userexits[i]) {
       (gcccrab->userexits[i])();
    }
  }

  /* Close Files */
  _clfiles();

   /* Free Process Global Memory */
   if (gcccrab->process_global) free(gcccrab->process_global);

  /* Deallocate Stack */
  lessstak(gcccrab->dynamicstack);
  free(gcccrab->dynamicstack);

  /* Shutdown dlmalloc */
  memoryinfo = mallinfo();
  dest_msp();
  size_t leaked = memoryinfo.uordblks - gcccrab->startmemoryusage;
  if (leaked) {
    sprintf( messagebuffer, "WARNING: MEMORY FREED (%d BYTES LEAKED)", leaked);
    CMSconsoleWrite(messagebuffer, CMS_EDIT);
  }

  return rc;
}

/*********************************************************************/
/* The Dynamic Stack Bins are generally _DSK_MINBIN in size (16kb)   */
/* but can be bigger for large stack frames - in which case they are */
/* rounded up to the nearest page.                                   */
/* Each bin starts with a frame - we can get the bin size from that  */
/* frames dstack member which points to the address after the last   */
/* usable space - which is the address of a pointer to the next      */
/* frame (see following)                                             */
/* The last double word (size_t) of each bin is a pointer to the next*/
/* bin (or zero). This chaining is used 1/ to allow bins to be reused*/
/* and reduce mallocs, and 2/ to allow bins "later" than the current */
/* one to be freed by lessstak(). This means that longjmps do not    */
/* leak memory.                                                      */
/*                                                                   */
/* This also means we have to be careful to add/subtract size_t for  */
/* dynamic bins to store the "next" double word pointer.             */
/* This (ugly) code in encapsulated in the 4 helper Functions        */
/* following.                                                        */
/*********************************************************************/

CMSINLINE CMSCRAB* getNextBin(CMSCRAB *current) {
  CMSCRAB **nextbinhandle = (CMSCRAB **)((size_t)current->dstack & 0x00FFFFFF);
  size_t flags = (size_t)current->dstack >> 24;
  if (flags & _DSK_DYNAMIC) {
    return *nextbinhandle;
  }
  return 0;
}

CMSINLINE void setNextBin(CMSCRAB *current, CMSCRAB *next) {
  CMSCRAB **nextbinhandle = (CMSCRAB **)((size_t)current->dstack & 0x00FFFFFF);
  size_t flags = (size_t)current->dstack >> 24;
  if (flags & _DSK_DYNAMIC) {
    *nextbinhandle = next;
  }
}

/* Returns the usable bin size (i.e. excluding the NEXT frame pointer) */
CMSINLINE size_t getBinSize(CMSCRAB *current) {
  size_t size = ((size_t)current->dstack & 0x00FFFFFF) - (size_t)current;
  return size;
}

CMSINLINE CMSCRAB* makeNewBin(size_t size, CMSCRAB *nextbin) {
  /* Round up bin size to the next page (or min size) */
  if ((size + sizeof(size_t)) < _DSK_MINBIN) size = _DSK_MINBIN;
  else size = ((size + sizeof(size_t)) && 0xFFF000) + 0x1000;

  CMSCRAB* crab = malloc(size);
  CMSCRAB **nextbinhandle = (CMSCRAB **)((size_t)crab + size - sizeof(size_t));
  crab->dstack = (void*)((size_t)nextbinhandle | (size_t)(_DSK_FIRSTDYNAMIC << 24));
  *nextbinhandle = nextbin;
  return crab;
}

CMSCRAB* morestak(CMSCRAB* frame, size_t requested) {
  CMSCRAB *new_frame;
  size_t bin_size;

  /* Check min request */
  if (requested < offsetof(CMSCRAB, locals)) requested = offsetof(CMSCRAB, locals);

  /* Round up bin size to the next page (or min size) */
  if ((requested + sizeof(size_t)) < _DSK_MINBIN) bin_size = _DSK_MINBIN;
  else bin_size = ((requested + sizeof(size_t)) && 0xFFF000) + 0x1000;

  /* Is the next free bin big enough                                         */
  /* This logic is just to speedup function calls unlucky eneough to be on a */
  /* bin boundary - avoiding keeping calling malloc - and to keep a list of  */
  /* bins so that lessstak can free then after a longjmp                     */
  new_frame = getNextBin(frame);
  if ( !(new_frame && requested<=getBinSize(new_frame)) ) {
    /* Need to make a new frame a link it in */
    new_frame = makeNewBin(requested, new_frame);
    setNextBin(frame, new_frame);
  }

  /* Set it Up */
  new_frame->backchain = frame;
  new_frame->forward = 0;
  frame->forward = new_frame;
  new_frame->gcccrab = frame->gcccrab;
  new_frame->stackNext = (void*)new_frame + requested;

  return new_frame;
}

void lessstak(CMSCRAB* frame) {
  /* Keep this frame (for a future morestak)- but free others on the chain.   */
  /* This means that if a bunch of bins have been left over from a longjmp    */
  /* then they are all cleared if and only if a function returns that happens */
  /* to empty a stack bin. Provides expected set/longjmp behavour.            */
  CMSCRAB* nextbin;
  CMSCRAB* bin=getNextBin(frame);

  setNextBin(frame,0);

  while (bin) {
    nextbin = getNextBin(bin); /* Get the next bin before deleting this one - paranoid */
    free(bin);
    bin = nextbin;
  }
}
