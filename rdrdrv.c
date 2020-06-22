/**************************************************************************************************/
/* READER DRIVER                                                                                  */
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
  int lrecl = 80;

  /* Am I the right driver? */
  if (filespecwords[1][0]) return -1; /* More than one word - nothing to do with me! */
  if (!strcmp(filespecwords[0],"READER132")) lrecl = 132;
  else if (!strcmp(filespecwords[0],"READER80")) lrecl = 80;
  else if (!strcmp(filespecwords[0],"READER")) lrecl = 80;
  else return -1; /* Nothing to do with me */

  if (_isopen("READER")) {
    errno = EEXIST;
    return 0;
  }

  if (theFile->access & ACCESS_WRITE) {
    errno = EINVAL;
    return 0;
  }
  else {
    theFile->filemaxreclen = lrecl;
    theFile->maxreclen = lrecl;
    theFile->buffer = malloc(theFile->filemaxreclen + 2); /* \n and NULL */
    strcpy(theFile->name, "READER");
    strcpy(theFile->fileid, "READER");
    theFile->reclen = -1; /* Empty Buffer */
    theFile->recpos = 0;
    theFile->recnum = -1;
    theFile->records = -1;
  }
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
  return CMScommand("CP CLOSE READER", CMS_COMMAND);
}


/**************************************************************************************************/
/* write driver                                                                                   */
/* returns 0 on success                                                                           */
/**************************************************************************************************/
static int write(FILE * file)
{
  file->error = 9;
  errno = EBADF;
  return EOF;
}


/**************************************************************************************************/
/* read (record) driver                                                                             */
/**************************************************************************************************/
static int read(FILE * file)
{
  int num = file->filemaxreclen;
  file->recnum = -1;
  file->error = CMScardRead(file->buffer, &num);
  if (file->error == 1) {
    file->error = 0;
    file->status |= STATUS_EOF;
    file->reclen = 0;
    return EOF;
  }
  if (file->error) {
    file->reclen = 0;
    return EOF;
  }

  file->reclen = num;
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
/* Driver Strusture                                                                               */
/**************************************************************************************************/
CMSDRIVER rdriodrv = {open, close, getpos, getend, setpos, write, read, NULL};
