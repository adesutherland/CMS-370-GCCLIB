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

#include <cmsruntm.h>
#include <cmssys.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define handlers (GETGCCCRAB()->handlers)

void (*signal(int sig, void (*func)(int))) (int)
{
    if (sig < SIGABRT) return SIG_ERR;
    if (sig > SIGTERM) return SIG_ERR;
    SIGHANDLER *old_signal_handler = handlers[sig];
    handlers[sig-1] = func;
    return (old_signal_handler);
}

int raise(int sig)
{
    char message[130];
    if (sig < SIGABRT) return -1;
    if (sig > SIGTERM) return -1;
    if (handlers[sig-1] == SIG_DFL)
    {
      if (sig == SIGABRT)
      {
          sprintf(message, "ABNORMAL TERMINATION (NO RESOURCE CLEANUP) ERRNO %d %s", errno, strerror(errno));
          CMSconsoleWrite(message, CMS_EDIT);
          GETGCCCRAB()->exitfunc(EXIT_FAILURE); /* Standard specifies no exit processing */
      }
    }
    else (handlers[sig-1])(sig);
    return (0);
}
