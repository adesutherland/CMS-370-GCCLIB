/**************************************************************************************************/
/* CONSOLE DRIVER                                                                                 */
/*                                                                                                */
/* Part of GCCLIB - VM/370 CMS Native Std C Library; A Historic Computing Toy                     */
/*                                                                                                */
/* Contributors: See contrib memo                                                                 */
/*                                                                                                */
/* Released to the public domain.                                                                 */
/**************************************************************************************************/

#define IN_RESLIB
#include <cmsruntm.h>
#include <cmssys.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

/**************************************************************************************************/
/* open driver                                                                                    */
/* int open(char **filespecwords, int accessMode)                                                 */
/* returns 0 on error                                                                             */
/*         -1 if the driver does not support the filespec                                         */
/*         1 success                                                                              */
/**************************************************************************************************/
static int open(char filespecwords[][10], FILE* theFile)
{
  /* Am I the right driver? */
  if (filespecwords[1][0]) return -1; /* More than one word - nothing to do with me! */
  if (strcmp(filespecwords[0],"CONSOLE")) return -1; /* Nothing to do with me */

  if (theFile->access & ACCESS_READWRITE) {
    errno = EINVAL;
    return 0;
  }

  theFile->filemaxreclen = 130;
  theFile->maxreclen = 130;
  theFile->buffer = malloc(theFile->filemaxreclen + 2); /* \n and NULL */
  strcpy(theFile->name, "CONSOLE");
  strcpy(theFile->fileid, "CONSOLE");
  theFile->reclen = -1; /* Empty Buffer */
  theFile->recpos = 0;
  theFile->recnum = -1;
  theFile->records = -1;
  return 1;
}


/**************************************************************************************************/
/* close driver                                                                                   */
/* returns 0 on success                                                                           */
/**************************************************************************************************/
static int close(FILE * file)
{
  if (file->buffer) {
    free(file->buffer);
    file->buffer = 0;
  }
  return 0;
}


/**************************************************************************************************/
/* write driver                                                                                   */
/* returns 0 on success                                                                           */
/**************************************************************************************************/
static int write(FILE * file)
{
  file->buffer[file->reclen] = 0;
  if (file->access & ACCESS_TEXT) CMSconsoleWrite(file->buffer, CMS_EDIT);
  else CMSconsoleWrite(file->buffer, CMS_NOEDIT);
  file->recnum = -1;
  /* Always returns zero */
  return 0;
}


/**************************************************************************************************/
/* read (record) driver                                                                             */
/**************************************************************************************************/
static int read(FILE * file)
{
  file->reclen = CMSconsoleRead(file->buffer);
  file->recnum = -1;
  return 0;
}


/**************************************************************************************************/
/* getpos driver                                                                                  */
/* returns position on success, -1 on error                                                       */
/**************************************************************************************************/
static int getpos(FILE * file)
{
  errno = EBADF;
  return -1;
}


/**************************************************************************************************/
/* getend driver                                                                                  */
/* returns position on success, -1 on error                                                       */
/**************************************************************************************************/
static int getend(FILE * file)
{
  errno = EBADF;
  return -1;
}


/**************************************************************************************************/
/* setpos driver                                                                                  */
/* returns position on success, -1 on error                                                       */
/**************************************************************************************************/
static int setpos(FILE * file, int pos)
{
  errno = EBADF;
  return -1;
}


/**************************************************************************************************/
/* Driver Structure                                                                               */
/**************************************************************************************************/
CMSDRIVER coniodrv = {open, close, getpos, getend, setpos, write, read, NULL};
