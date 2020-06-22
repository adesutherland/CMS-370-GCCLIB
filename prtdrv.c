/**************************************************************************************************/
/* PRINTER DRIVER                                                                                 */
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
  if (strcmp(filespecwords[0],"PRINTER")) return -1; /* Nothing to do with me */
  if (_isopen("PRINTER")) {
    errno = EEXIST;
    return 0;
  }

  if (!(theFile->access & ACCESS_READ)) {
    theFile->filemaxreclen = 132;
    theFile->maxreclen = 132;
    theFile->buffer = (char*)malloc(theFile->filemaxreclen + 3) + 1; /* character control char, \n and NULL */
    strcpy(theFile->name, "PRINTER");
    strcpy(theFile->fileid, "PRINTER");
    theFile->recnum = -1;
    theFile->records = -1;
  }
  else {
    errno = EINVAL;
    return 0;
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
    free(file->buffer - 1); /* The line control charcter prefix */
    file->buffer = 0;
  }
  return CMScommand("CP CLOSE PRINTER", CMS_COMMAND);
}


/**************************************************************************************************/
/* write driver                                                                                   */
/* returns 0 on success                                                                           */
/**************************************************************************************************/
static int write(FILE * file)
{
  file->recnum = -1;
  if (file->reclen == 0) {
    /* CMS cannot handle zero length records */
    /* So for text files we add a space */
    if (file->access & ACCESS_TEXT) {
      file->reclen = 1;
      file->buffer[0] = ' ';
    }
    else return 0; /* Might as well exit - nop */
  }

  file->buffer[file->reclen] = 0;
  *(file->buffer - 1) = ' '; /* Printer control character */
  file->error = CMSprintLine(file->buffer - 1);
  if (file->error != 0) {
    errno = EIO;
    return file->error;
  }
  return 0;
}


/**************************************************************************************************/
/* read (record) driver                                                                             */
/**************************************************************************************************/
static int read(FILE * file)
{
  file->error = 9;
  errno = EBADF;
  return EOF;
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
CMSDRIVER prtiodrv = {open, close, getpos, getend, setpos, write, read, NULL};
