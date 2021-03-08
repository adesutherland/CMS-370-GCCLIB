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
#include <cmssys.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]);

void __exit(int rc);

int __cstub(PLIST *plist, EPLIST *eplist, void *gcclibvt) {
    GCCCRAB gcccrab;
    CMSCRAB *cmscrab;
    int x;
    unsigned int randomseed = 1;
    struct {
        size_t magic;
        size_t page_size;
        size_t granularity;
        size_t mmap_threshold;
        size_t trim_threshold;
        unsigned int default_mflags;
    } mparams;
    int tempcounter = 0;
    struct tm gmtimetm;
    char atresult[26];
    char strfbuf[26];
    char tempname[21];

    /* Default handlers */
    SIGHANDLER *handlers[6] = {SIG_DFL, SIG_DFL, SIG_DFL, SIG_DFL, SIG_DFL,
                               SIG_DFL};

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

    /* Pass the address of the resident RTL vector table */
    gcccrab.gcclibvt = gcclibvt;

    /* Set Global Variables */
    gcccrab.exitfunc = __exit;
    gcccrab.handlers = handlers;
    gcccrab.userexits = userexits;
    gcccrab.filehandles = NULL;
    gcccrab.strtok_old = NULL;
    gcccrab.process_global = NULL;
    gcccrab.argv = NULL;
    gcccrab.argbuffer = NULL;
    gcccrab.plist = plist;
    gcccrab.eplist = eplist;
    gcccrab.calltype = -1;
    gcccrab.evalblok = NULL;
    gcccrab.isproc = 0;
    gcccrab.argc = 0;
    gcccrab.randomseed = &randomseed;
    gcccrab.tempcounter = &tempcounter;
    gcccrab.tempname = &tempname;
    mparams.magic = 0;
    gcccrab.mparams = &mparams;
    gcccrab.atresult = &atresult;
    gcccrab.gmtimetm = &gmtimetm;
    gcccrab.strfbuf = &strfbuf;

    return (__cstart(main));
}
