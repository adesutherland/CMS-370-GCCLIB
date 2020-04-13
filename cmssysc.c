/**************************************************************************************************/
/* CMSSYSC.C - CMS System Calls (C part)                                                          */
/*                                                                                                */
/* Part of GCCLIB - VM/370 CMS Native Std C Library; A Historic Computing Toy                     */
/*                                                                                                */
/* Proud Contributors:                                                                            */
/*   Robert O'Hara, Paul Edwards, Dave Wade, with a little hacking by Adrian Sutherland           */
/*                                                                                                */
/* Released to the public domain.                                                                 */
/**************************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <cmsruntm.h>

int __SVC202(PLIST *plist , EPLIST *eplist, int calltype);

int __CMSCMD(char *command, int calltype)
{
 int i, j, len, args, maxargs;
 int useEplist = 0;
 GCCLIBPLIST gcclibplist;
 PLIST *plist = &gcclibplist.plist;
 EPLIST *eplist;

 /* NOTE: calltype 5 - 6 Word EPLIST not implemented yet */
 if (calltype == 5 || calltype == 1 || calltype == 11) useEplist = 1;

 /* Setup gcclibplist */
 if (useEplist)
 {
   eplist = &gcclibplist.eplist;
   strcpy(gcclibplist.marker, EPLISTMARKER);
   memset((void*)plist, ' ', sizeof(gcclibplist.plist));
   eplist->Command = 0;
   eplist->BeginArgs = 0;
   eplist->EndArgs = 0;
   eplist->CallContext = 0;
   eplist->ArgLlist = 0;
   eplist->FunctionReturn = 0;
   maxargs = TRUNCPLISTARGS;
 }
 else
 {
   memset((void*)plist, ' ', sizeof(gcclibplist));
   eplist = 0;
   maxargs = MAXPLISTARGS;
 }

 len = strlen(command);
 args = -1;
 for (i=0; i<len; i++)
 {
    /* Find start of next word */
    if (command[i]!=' ')
    {
      /* Process word */
      args++;
      if (args==1)
      {
        /* Start of args */
        if (eplist) eplist->BeginArgs = command + i;
      }
      for (j=0; i<len && j<8; i++, j++)
      {
        if (command[i]==' ') break;
        (plist[args])[j] = toupper(command[i]);
      }
      for (; i<len && command[i]!=' '; i++);
      if (args>=maxargs) break;
    }
  }
  if (eplist)
  {
    if (eplist->BeginArgs) eplist->EndArgs = command + len;
    eplist->Command = command;
  }

  /* PLIST FENCE */
  args++;
  for (j=0; j<8; j++)
  {
    (plist[args])[j] = 0xFF;
  }

  return __SVC202(plist, eplist, calltype);
}

/* -------------------- Alternative MORECORE functions ------------------- */

/*
  Guidelines for creating a custom version of MORECORE:

  * For best performance, MORECORE should allocate in multiples of pagesize.
  * MORECORE may allocate more memory than requested. (Or even less,
      but this will usually result in a malloc failure.)
  * MORECORE must not allocate memory when given argument zero, but
      instead return one past the end address of memory from previous
      nonzero call.
  * For best performance, consecutive calls to MORECORE with positive
      arguments should return increasing addresses, indicating that
      space has been contiguously extended.
  * Even though consecutive calls to MORECORE need not return contiguous
      addresses, it must be OK for malloc'ed chunks to span multiple
      regions in those cases where they do happen to be contiguous.
  * MORECORE need not handle negative arguments -- it may instead
      just return MFAIL when given negative arguments.
      Negative arguments are always multiples of pagesize. MORECORE
      must not misinterpret negative args as large positive unsigned
      args. You can suppress all such calls from even occurring by defining
      MORECORE_CANNOT_TRIM,

  As an example alternative MORECORE, here is a custom allocator
  kindly contributed for pre-OSX macOS.  It uses virtually but not
  necessarily physically contiguous non-paged memory (locked in,
  present and won't get swapped out).  You can use it by uncommenting
  this section, adding some #includes, and setting up the appropriate
  defines above:

      #define MORECORE osMoreCore

  There is also a shutdown routine that should somehow be called for
  cleanup upon program exit.

  #define MAX_POOL_ENTRIES 100
  #define MINIMUM_MORECORE_SIZE  (64 * 1024U)
  static int next_os_pool;
  void *our_os_pools[MAX_POOL_ENTRIES];

  void *osMoreCore(int size)
  {
    void *ptr = 0;
    static void *sbrk_top = 0;

    if (size > 0)
    {
      if (size < MINIMUM_MORECORE_SIZE)
         size = MINIMUM_MORECORE_SIZE;
      if (CurrentExecutionLevel() == kTaskLevel)
         ptr = PoolAllocateResident(size + RM_PAGE_SIZE, 0);
      if (ptr == 0)
      {
        return (void *) MFAIL;
      }
      // save ptrs so they can be freed during cleanup
      our_os_pools[next_os_pool] = ptr;
      next_os_pool++;
      ptr = (void *) ((((size_t) ptr) + RM_PAGE_MASK) & ~RM_PAGE_MASK);
      sbrk_top = (char *) ptr + size;
      return ptr;
    }
    else if (size < 0)
    {
      // we don't currently support shrink behavior
      return (void *) MFAIL;
    }
    else
    {
      return sbrk_top;
    }
  }

  // cleanup any allocated memory pools
  // called as last thing before shutting down driver

  void osCleanupMem(void)
  {
    void **ptr;

    for (ptr = our_os_pools; ptr < &our_os_pools[MAX_POOL_ENTRIES]; ptr++)
      if (*ptr)
      {
         PoolDeallocate(*ptr);
         *ptr = 0;
      }
  }

*/
