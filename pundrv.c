/**************************************************************************************************/
/* PUNCH DRIVER                                                                                   */
/*                                                                                                */
/* Part of GCCLIB - VM/370 CMS Native Std C Library; A Historic Computing Toy                     */
/*                                                                                                */
/* Contributors: See contrib memo                                                                 */
/*                                                                                                */
/* Released to the public domain.                                                                 */
/**************************************************************************************************/

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
static int open(char filespecwords[][10], FILE *theFile) {
    /* Am I the right driver? */
    if (filespecwords[1][0]) return -1; /* More than one word - nothing to do with me! */
    if (strcmp(filespecwords[0], "PUNCH"))
        return -1; /* Nothing to do with me */
    if (_isopen("PUNCH")) {
        errno = EEXIST;
        return 0;
    }


    if (!(theFile->access & ACCESS_READ)) {
        theFile->filemaxreclen = 80;
        theFile->maxreclen = 80;
        theFile->buffer = malloc(theFile->filemaxreclen + 2); /* \n and NULL */
        strcpy(theFile->name, "PUNCH");
        strcpy(theFile->fileid, "PUNCH");
        theFile->recnum = -1;
        theFile->records = -1;
    } else {
        errno = EINVAL;
        return 0;
    }
    return 1;
}


/**************************************************************************************************/
/* close driver                                                                                   */
/* returns 0 on success                                                                           */
/**************************************************************************************************/
static int close(FILE *file) {
    if (file->buffer) {
        free(file->buffer);
        file->buffer = 0;
    }
    return CMScommand("CP CLOSE PUNCH", CMS_COMMAND);
}


/**************************************************************************************************/
/* write driver                                                                                   */
/* returns 0 on success                                                                           */
/**************************************************************************************************/
static int write(FILE *file) {
    file->recnum = -1;
    if (file->reclen < file->filemaxreclen) { /* PAD */
        memset(file->buffer + file->reclen, (int) ' ',
               file->filemaxreclen - file->reclen);
        file->reclen = file->filemaxreclen;
    }
    file->reclen = file->filemaxreclen;

    file->error = CMScardPunch(file->buffer);
    if (file->error != 0) {
        errno = EIO;
        return file->error;
    }
    return 0;
}


/**************************************************************************************************/
/* read (record) driver                                                                             */
/**************************************************************************************************/
static int read(FILE *file) {
    file->error = 9;
    errno = EBADF;
    return EOF;
}


/**************************************************************************************************/
/* getpos driver                                                                                  */
/* returns position on success, -1 on error                                                       */
/**************************************************************************************************/
static int getpos(FILE *file) {
    errno = EBADF;
    return -1;
}


/**************************************************************************************************/
/* getend driver                                                                                  */
/* returns position on success, -1 on error                                                       */
/**************************************************************************************************/
static int getend(FILE *file) {
    errno = EBADF;
    return -1;
}


/**************************************************************************************************/
/* setpos driver                                                                                  */
/* returns position on success, -1 on error                                                       */
/**************************************************************************************************/
static int setpos(FILE *file, int pos) {
    errno = EBADF;
    return -1;
}


/**************************************************************************************************/
/* Driver Strusture                                                                               */
/**************************************************************************************************/
CMSDRIVER puniodrv = {open, close, getpos, getend, setpos, write, read, NULL};
