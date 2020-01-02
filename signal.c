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

#include "signal.h"
#include "stdlib.h"

static void (*handlers[])(int) = {
    __sigdfl,
    __sigdfl,
    __sigdfl,
    __sigdfl,
    __sigdfl,
    __sigdfl,
    __sigdfl };

void __sigdfl(int sig);
void __sigerr(int sig);
void __sigign(int sig);

#define SIG_DFL __sigdfl
#define SIG_ERR __sigerr
#define SIG_IGN __sigign

void (*signal(int sig, void (*func)(int)))(int)
{
    handlers[sig] = func;
    return (func);
}


int raise(int sig)
{
    (handlers[sig])(sig);
    return (0);
}

void __sigdfl(int sig)
{
    handlers[sig] = SIG_DFL;
    if (sig == SIGABRT)
    {
        exit(EXIT_FAILURE);
    }
    return;
}

void __sigerr(int sig)
{
    (void)sig;
    return;
}

void __sigign(int sig)
{
    (void)sig;
    return;
}

