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
#include <cmssys.h>
#include <cmsruntm.h>

/* Low Level Assembler __SVC202 fnction */
int __SVC202(PLIST *plist , EPLIST *eplist, int calltype);

/* Worker Function - create PLIST / EPLIST and call __svc202 */
static int cmd_with_plist(char *command, int calltype)
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

/* Main Function - has the search logic for call type 11 (CMS_CONSOLE) */
int __CMSCMD(char *command, int calltype)
{

  char fileid[28];
  CMSFILEINFO * fst;
  int rc;
  int len, i, j;
  char program[9];
  char *newcommand;

  if (calltype == CMS_CONSOLE) {
    len = strlen(command);
    /* Skip spaces */
    for (i=0; i<len && command[i]==' '; i++);

    /* Get the program */
    for (j=0; i<len && command[i]!=' '; i++) {
      program[j++] = toupper(command[i]);
      if (j>8) break;
    }
    program[j] = 0;

    /* Special cases */
    if (!strcmp(program,"EXEC")) return cmd_with_plist(command, calltype);
    if (!strcmp(program,"CP")) {
      rc = cmd_with_plist(command, calltype);
      if (rc == 1) rc = -1;
      return rc;
    }

    /* Does an EXEC exist? */
    strcpy(fileid, "        EXEC    * ");
    strncpy(fileid, program, strlen(program));
    rc = CMSfileState(fileid, &fst);
    if (!rc) {
      newcommand = (char*)malloc(strlen(command) + 6);
      newcommand[0] = 0;
      strcat(newcommand, "EXEC ");
      strcat(newcommand, command);
      rc = cmd_with_plist(newcommand, calltype);
      free(newcommand);
      return rc;
    }

    /* Try the command naked */
    rc = cmd_with_plist(command, calltype);
    if (rc != -3) return rc;

    /* Finally see if it is a CP command */
    newcommand = (char*)malloc(strlen(command) + 4);
    newcommand[0] = 0;
    strcat(newcommand, "CP ");
    strcat(newcommand, command);
    rc = cmd_with_plist(newcommand, calltype);
    free(newcommand);
    if (rc == 1) return -3;
    else return rc;
  }

  /* If not call type 11 just try the command naked */
  else return cmd_with_plist(command, calltype);
}
