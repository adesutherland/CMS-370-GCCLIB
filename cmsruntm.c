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
#include <cmssys.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <signal.h>

#define CMSINLINE static __inline __attribute__ ((always_inline))

/* Low level function to allocate memory */
void *_DMSFREE(int doublewords);

int __cstart(MAINFUNC *mainfunc) {
    GCCCRAB *gcccrab;
    int a, b, i, len;
    int rc;
    char ch;
    char messagebuffer[100];
    ADLEN *param;

    /* Get GCCCRAB */
    gcccrab = GETGCCCRAB();

    /* Initialize Memory System */
    struct mallinfo memoryinfo;
    creat_msp();
    memoryinfo = mallinfo();
    gcccrab->startmemoryusage = memoryinfo.uordblks;

    /* Setup stdout and stderr */
    stdout = fopen("+CONSOLE+", "w");
    stderr = fopen("+CONSOLE+", "w");
    stdin = fopen("+CONSOLE+", "r");

    /* Process parameters */
    gcccrab->argv = malloc(MAXEPLISTARGS);
    gcccrab->argbuffer = malloc(ARGBUFFERLEN);

    /* The high order byte contains the
       traditional CMS R1 flag byte.  A x'0B' or x'01' indicates the
       presence of an extended parameter accessable via R0 (the eplist parameter) */
    gcccrab->calltype = ((int) (gcccrab->plist) & 0xff000000) >> 24;
    gcccrab->plist = (PLIST *) ((int) (gcccrab->plist) & 0xffffff);

    /* Process Arguments */
    gcccrab->argc = 0;
    if (gcccrab->calltype == 5) {
        /* Decode 6 word eplist */

        /* Proc / function flag */
        gcccrab->isproc = (int) gcccrab->eplist & 0x80000000 ? 1 : 0;

        /* *** Function / Program name ***                                       */
        /* Its quite unclear what is the most natural / consistent value to use  */
        /* for arg[0] which should be the name of the function called. So ...    */
        /* - if there is a BeginArgs / EndArgs we will use the first word of that*/
        /* - otherwise we will use the Command String                            */
        a = 0;
        i = 0;
        if (gcccrab->eplist->BeginArgs &&
            (len = gcccrab->eplist->EndArgs - gcccrab->eplist->BeginArgs)) {
            while (i < len) {
                /* Strip Leading Spaces */
                for (; i < len; i++) {
                    if (gcccrab->eplist->BeginArgs[i] != ' ') break;
                }

                /* Process 1st Word */
                for (; i < len; i++) {
                    ch = gcccrab->eplist->BeginArgs[i];
                    if (ch == ' ') break;
                    gcccrab->argbuffer[a++] = ch;
                }

                /* Got the first work */
                if (a) {
                    gcccrab->argbuffer[a++] = 0;
                    gcccrab->argv[gcccrab->argc++] = gcccrab->argbuffer;
                }

                /* Don't care about anything else */
                break;
            }
        }

        if (!a) { /* Nothing from the EPLIST Args - use the command */
            for (i = 0; i < 8; i++) {
                ch = gcccrab->eplist->Command[i];
                if (ch && ch != 0xFF && ch != ' ') gcccrab->argbuffer[a++] = ch;
                else break;
            }
            gcccrab->argbuffer[a++] = 0;
            gcccrab->argv[gcccrab->argc++] = gcccrab->argbuffer;
        }

        /* Now do parameters - unfortunately we need to copy them as they need  */
        /* to be null terminated                                                */
        param = gcccrab->eplist->ArgList;
        if (param) {
            for (i = 0; (int) (param[i].Data) != -1; i++) {
                gcccrab->argv[gcccrab->argc++] = gcccrab->argbuffer + a;
                memcpy(gcccrab->argbuffer + a, param[i].Data, param[i].Len);
                a += param[i].Len;
                gcccrab->argbuffer[a++] = 0;
            }
        }
    } else if (gcccrab->calltype == 1 || gcccrab->calltype == 11) {
        /* Decode 4 word eplist */
        /* Function name  */
        len = gcccrab->eplist->BeginArgs - gcccrab->eplist->Command;
        if (len > 8) len = 8;
        a = 0;
        for (i = 0; i < len; i++) {
            if (gcccrab->eplist->Command[i] != ' ')
                gcccrab->argbuffer[a++] = gcccrab->eplist->Command[i];
        }
        gcccrab->argbuffer[a++] = 0;
        gcccrab->argv[gcccrab->argc++] = gcccrab->argbuffer;

        i = 0;
        len = gcccrab->eplist->EndArgs - gcccrab->eplist->BeginArgs;
        while (i < len) {
            /* Strip Leading Spaces */
            for (; i < len; i++) {
                if (gcccrab->eplist->BeginArgs[i] != ' ') break;
            }

            /* Process Word */
            for (b = a; i < len; i++) {
                ch = gcccrab->eplist->BeginArgs[i];
                if (ch == ' ' || ch == ')' || ch == '(') break;
                gcccrab->argbuffer[a++] = ch;
            }

            /* A parameter was added */
            if (a != b) {
                gcccrab->argbuffer[a++] = 0;
                gcccrab->argv[gcccrab->argc++] = gcccrab->argbuffer + b;
            }

            /* Handle any ( or )'s as seperate parms */
            if (ch == ')' || ch == '(') {
                gcccrab->argv[gcccrab->argc++] = gcccrab->argbuffer + a;
                gcccrab->argbuffer[a++] = ch;
                gcccrab->argbuffer[a++] = 0;
                i++;
            }
        }
    } else {
        /* Decode plist */
        /* Function name - always in plist[0] */
        for (a = i = 0; i < 8 && gcccrab->plist[gcccrab->argc][i] != 0xFF &&
                        gcccrab->plist[gcccrab->argc][i] != ' '; i++)
            gcccrab->argbuffer[a++] = gcccrab->plist[gcccrab->argc][i];
        gcccrab->argbuffer[a++] = 0;
        gcccrab->argv[gcccrab->argc++] = gcccrab->argbuffer;

        for (; gcccrab->plist[gcccrab->argc][0] != 0xFF; gcccrab->argc++) {
            gcccrab->argv[gcccrab->argc] = gcccrab->argbuffer + a;
            for (i = 0; i < 8 && gcccrab->plist[gcccrab->argc][i] != ' '; i++)
                gcccrab->argbuffer[a++] = gcccrab->plist[gcccrab->argc][i];
            gcccrab->argbuffer[a++] = 0;
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
    rc = __CLVSTK(gcccrab->dynamicstack, mainfunc, gcccrab->argc,
                  gcccrab->argv);

    /* Call Exit functions */
    for (i = __NATEXIT - 1; i >= 0; i--) {
        if (gcccrab->userexits[i]) {
            (gcccrab->userexits[i])();
        }
    }

    /* Caltype 5 - int return to string assist */
    if ((gcccrab->calltype == 5) && !gcccrab->evalblok) {
        CMSreturnvalint(rc);
        rc = 0;
    }

    /* Close Files */
    _clfiles();

    /* Free Process Global Memory */
    if (gcccrab->process_global) free(gcccrab->process_global);

    /* Free argument structures */
    if (gcccrab->argv) free(gcccrab->argv);
    if (gcccrab->argbuffer) free(gcccrab->argbuffer);

    /* Deallocate Stack */
    lessstak(gcccrab->dynamicstack);
    free(gcccrab->dynamicstack);

    /* Shutdown dlmalloc */
    memoryinfo = mallinfo();
    dest_msp();
    size_t leaked = memoryinfo.uordblks - gcccrab->startmemoryusage;
    if (leaked) {
        if (gcccrab->debug) {
            sprintf(messagebuffer, "WARNING: MEMORY FREED (%d BYTES LEAKED)",
                    leaked);
            CMSconsoleWrite(messagebuffer, CMS_EDIT);
        }
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

CMSINLINE CMSCRAB *getNextBin(CMSCRAB *current) {
    CMSCRAB **nextbinhandle = (CMSCRAB **) ((size_t) current->dstack &
                                            0x00FFFFFF);
    size_t flags = (size_t) current->dstack >> 24;
    if (flags & _DSK_DYNAMIC) {
        return *nextbinhandle;
    }
    return 0;
}

CMSINLINE void setNextBin(CMSCRAB *current, CMSCRAB *next) {
    CMSCRAB **nextbinhandle = (CMSCRAB **) ((size_t) current->dstack &
                                            0x00FFFFFF);
    size_t flags = (size_t) current->dstack >> 24;
    if (flags & _DSK_DYNAMIC) {
        *nextbinhandle = next;
    }
}

/* Returns the usable bin size (i.e. excluding the NEXT frame pointer) */
CMSINLINE size_t getBinSize(CMSCRAB *current) {
    size_t size = ((size_t) current->dstack & 0x00FFFFFF) - (size_t) current;
    return size;
}

CMSINLINE CMSCRAB *makeNewBin(size_t size, CMSCRAB *nextbin) {
    /* Round up bin size to the next page (or min size) */
    if ((size + sizeof(size_t)) < _DSK_MINBIN) size = _DSK_MINBIN;
    else size = ((size + sizeof(size_t)) && 0xFFF000) + 0x1000;

    CMSCRAB *crab = malloc(size);
    CMSCRAB **nextbinhandle = (CMSCRAB **) ((size_t) crab + size -
                                            sizeof(size_t));
    crab->dstack = (void *) ((size_t) nextbinhandle |
                             (size_t) (_DSK_FIRSTDYNAMIC << 24));
    *nextbinhandle = nextbin;
    return crab;
}

CMSCRAB *morestak(CMSCRAB *frame, size_t requested) {
    CMSCRAB *new_frame;
    size_t bin_size;

    /* Check min request */
    if (requested < offsetof(CMSCRAB, locals))
        requested = offsetof(CMSCRAB, locals);

    /* Round up bin size to the next page (or min size) */
    if ((requested + sizeof(size_t)) < _DSK_MINBIN) bin_size = _DSK_MINBIN;
    else bin_size = ((requested + sizeof(size_t)) && 0xFFF000) + 0x1000;

    /* Is the next free bin big enough                                         */
    /* This logic is just to speedup function calls unlucky eneough to be on a */
    /* bin boundary - avoiding keeping calling malloc - and to keep a list of  */
    /* bins so that lessstak can free then after a longjmp                     */
    new_frame = getNextBin(frame);
    if (!(new_frame && requested <= getBinSize(new_frame))) {
        /* Need to make a new frame a link it in */
        new_frame = makeNewBin(requested, new_frame);
        setNextBin(frame, new_frame);
    }

    /* Set it Up */
    new_frame->backchain = frame;
    new_frame->forward = 0;
    frame->forward = new_frame;
    new_frame->gcccrab = frame->gcccrab;
    new_frame->stackNext = (void *) new_frame + requested;

    return new_frame;
}

void lessstak(CMSCRAB *frame) {
    /* Keep this frame (for a future morestak)- but free others on the chain.   */
    /* This means that if a bunch of bins have been left over from a longjmp    */
    /* then they are all cleared if and only if a function returns that happens */
    /* to empty a stack bin. Provides expected set/longjmp behavour.            */
    CMSCRAB *nextbin;
    CMSCRAB *bin = getNextBin(frame);

    setNextBin(frame, 0);

    while (bin) {
        nextbin = getNextBin(
                bin); /* Get the next bin before deleting this one - paranoid */
        free(bin);
        bin = nextbin;
    }
}
