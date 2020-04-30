/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  signal.c - implementation of stuff in signal.h                   */
/*                                                                   */
/*********************************************************************/
#define IN_RESLIB
#include <cmsruntm.h>
#include "signal.h"
#include "stdlib.h"
#include "stdio.h"

#define handlers (GETGCCCRAB()->handlers)

void (*signal(int sig, void (*func)(int))) (int)
{
    SIGHANDLER *old_signal_handler = handlers[sig];
    handlers[sig] = func;
    return (old_signal_handler);
}

int raise(int sig)
{
    if (sig < SIGABRT) return -1;
    if (sig > SIGTERM) return -1;
    (handlers[sig])(sig);
    return (0);
}

void __sigdfl(int sig)
{
    if (sig == SIGABRT)
    {
        perror("ABNORMAL TERMINATION (NO RESOURCE CLEANUP)");
        GETGCCCRAB()->exitfunc(EXIT_FAILURE); /* Standard specifies no exit processing */
    }
    return;
}

void __sigerr(int sig)
{
    return;
}

void __sigign(int sig)
{
    return;
}
