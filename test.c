#include <stdio.h>
#include <cmssys.h>
#include <signal.h>

void signal_handler(int signal)
{
    printf("Received signal %d\n", signal);
}

int main(int argc, char * argv[])
{
  printf("GCCLIB Sanity Test\n");

  printf("Immediate Flags Test\n");
  CMSSetFlag(TRACEFLAG, 0);
  CMSSetFlag(HALTFLAG, 0);
  printf("Trace %d\n", CMSGetFlag(TRACEFLAG) );
  printf("Halt %d\n", CMSGetFlag(HALTFLAG) );

  CMSSetFlag(TRACEFLAG, 1);
  CMSSetFlag(HALTFLAG, 1);
  printf("Trace %d\n", CMSGetFlag(TRACEFLAG) );
  printf("Halt %d\n", CMSGetFlag(HALTFLAG) );

  CMSSetFlag(TRACEFLAG, 0);
  CMSSetFlag(HALTFLAG, 1);
  printf("Trace %d\n", CMSGetFlag(TRACEFLAG) );
  printf("Halt %d\n", CMSGetFlag(HALTFLAG) );

  CMSSetFlag(TRACEFLAG, 1);
  CMSSetFlag(HALTFLAG, 0);
  printf("Trace %d\n", CMSGetFlag(TRACEFLAG) );
  printf("Halt %d\n", CMSGetFlag(HALTFLAG) );

  printf("Signal Test\n");
  signal(SIGTERM, signal_handler);
  raise(SIGTERM);


}
