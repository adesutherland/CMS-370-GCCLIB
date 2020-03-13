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

typedef struct EPLIST {
  char *Command;
  char *BeginArgs;  /* start of Argstring */
  char *EndArgs;    /* character after end of the Argstring */
  void *CallContextBlock; /* Extention Point */
} EPLIST;

typedef char PLIST[8]; /* 8 char block */

int __SVC202(PLIST *plist , EPLIST *eplist, int calltype);

int __CMSCMD(char * command, int calltype) {
 int i, j, len, args;
 PLIST plist[33];
 EPLIST eplist;

 len = strlen(command);
 memset((void*)plist, ' ', sizeof(plist));
 args=-1;

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
   }
 }
 eplist.EndArgs = command + i;
 eplist.Command = plist[0];
 eplist.CallContextBlock = 0;
 args++;
 for (j=0; j<8; j++)
 {
   (plist[args])[j] = 0xFF;
 }

 return __SVC202(plist, &eplist,calltype);
}
