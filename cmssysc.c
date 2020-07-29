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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <cmssys.h>
#include <cmsruntm.h>

/* Low Level Assembler __SVC202 fnction */
int __SVC202(PLIST *plist , EPLIST *eplist, int calltype);

/* Worker Function - create PLIST / EPLIST and call __svc202 */
static int cmd_with_plist(char *command, int calltype)
{
  int i, j, len, args;
  PLIST plist[MAXPLISTARGS + 1];
  EPLIST eplist;

  memset((void*)plist, ' ', sizeof(plist));
  eplist.Command = 0;
  eplist.BeginArgs = 0;
  eplist.EndArgs = 0;
  eplist.CallContext = 0;
  eplist.ArgLlist = 0;
  eplist.FunctionReturn = 0;

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
        eplist.BeginArgs = command + i;
      }
      for (j=0; i<len && j<8; i++, j++)
      {
        if (command[i]==' ') break;
        (plist[args])[j] = toupper(command[i]);
      }
      for (; i<len && command[i]!=' '; i++);
      if (args>=MAXPLISTARGS) break;
    }
  }
  if (eplist.BeginArgs) eplist.EndArgs = command + len;
  eplist.Command = command;

  /* PLIST FENCE */
  args++;
  for (j=0; j<8; j++)
  {
    (plist[args])[j] = 0xFF;
  }

  return __SVC202(plist, &eplist, calltype);
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

/**************************************************************************************************/
/* Two functions to help C programs avoid having any non-const static or global                   */
/* variables. For programs planning to be run from a shared segment the program they will be      */
/* running in read-only memory and the CMS linker does not allow global variables to be placed in */
/* another segment.                                                                               */
/*                                                                                                */
/* IBM calls these programs (ones that don't write to their TEXT segment) reentrant programs.     */
/* I have coined this memory - Process Global Memory.                                             */
/*                                                                                                */
/* void* CMSPGAll(size_t size) - Allocate / Reallocate Process Global Memory Block                */
/* void* CMSGetPG(void) - Get the address of the Process Global Memory Block                      */
/*                                                                                                */
/* Note: that this area is freed automatically on normal program termination.                     */
/*                                                                                                */
/**************************************************************************************************/
void* CMSPGAll(size_t size) {
  GCCCRAB *crab;

  crab = GETGCCCRAB();

  if (crab->process_global) free (crab->process_global);

  if (size) crab->process_global = malloc(size);
  else crab->process_global = 0;

  return crab->process_global;
}

void* CMSGetPG(void) {
  GCCCRAB *crab;
  crab = GETGCCCRAB();
  return crab->process_global;
}
