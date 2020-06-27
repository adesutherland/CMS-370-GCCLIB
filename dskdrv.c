/**************************************************************************************************/
/* DISK DRIVER                                                                                    */
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
/* reopen Helper function (to move from read to write mode)                                       */
/**************************************************************************************************/
static int reopen(FILE* theFile) {
  int rc;
  CMSfileClose(&theFile->fscb);
  rc = CMSfileOpen(theFile->fileid, theFile->buffer, theFile->filemaxreclen, theFile->fscb.format[0],
                   1, theFile->recnum, &(theFile->fscb));
  if (!rc) theFile->status &= 0xFFFF - STATUS_READ - STATUS_WRITE;
  return rc;
}


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
  if (!filespecwords[1][0]) return -1; /* Only a "filename" - nothing to do with me! */

  /* Validation */
  int rc;
  CMSFILEINFO* existingFileState;
  int i;
  char fileid[19];
  char* fname;
  char* ftype;
  char fmode[10]; /* Each filespecwords[] can be 10 chars */
  char recfm;
  int lrecl;
  int recnum = 0;

  /* fileid */
  fname = filespecwords[0];
  ftype = filespecwords[1];
  strcpy(fmode, filespecwords[2]);
  if (filespecwords[5][0]) {errno = EINVAL; return 0;} /* Too many arguments */
  if (strlen(fname) > 8) {errno = EINVAL; return 0;}
  if (strlen(ftype) > 8) {errno = EINVAL; return 0;}
  if (strlen(fmode) > 2) {errno = EINVAL; return 0;}

  if (theFile->access & ACCESS_TRUNCATE) {
    /* We are writing (and replacing) a file so need to get properities from the parameters */
    /* RECFM */
    recfm = 'V';
    if (filespecwords[3][0]) {
      if (strlen(filespecwords[3])>1) {errno = EINVAL; return 0;}
      recfm = filespecwords[3][0];
      if (recfm!='F' && recfm!='V') {errno = EINVAL; return 0;}
    }

    /* LRECL */
    lrecl = 0;
    if (filespecwords[4][0]) {
      for (i=0; filespecwords[4][i]; i++) {
        if (!isdigit(filespecwords[4][i])) {errno = EINVAL; return 0;}
      }
      lrecl = atoi(filespecwords[4]);
      if (lrecl<1 || lrecl>65535) {errno = EINVAL; return 0;}
    }
    if (recfm == 'F' && lrecl == 0) {errno = EINVAL; return 0;}
    if (recfm == 'V' && lrecl == 0) lrecl=65535;

    if (strlen(fmode) == 0) strcpy(fmode, "A1");
    else if (strlen(fmode) == 1) {
      fmode[1]='1'; fmode[2]=0; /* Append "1" */
    }

    sprintf(fileid,"%-8s%-8s%-2s", fname, ftype, fmode);

    if (_isopen(fileid)) {
      errno = EEXIST;
      return 0;
    }

    /* Erase file - now ready to be replaced! */
    CMSfileErase(fileid);
    theFile->records = 0;
  }
  else {
    /* We are reading or appending to a file so need its properties */
    if (strlen(fmode) == 0) strcpy(fmode, "*"); /* Lets search all drives! */
    sprintf(fileid,"%-8s%-8s%-2s", fname, ftype, fmode);
    if (CMSfileState(fileid, &existingFileState)) {
      /* File does not exist (or some other error) */
      errno = ENOENT;
      return 0;
    }

    fmode[0] = existingFileState->filemode[0];
    fmode[1] = existingFileState->filemode[1];
    fmode[2] = 0;
    recfm = existingFileState->format;
    lrecl = existingFileState->lrecl; /* logical record length */
    theFile->records = existingFileState->numRecords;

    sprintf(fileid,"%-8s%-8s%-2s", fname, ftype, fmode);
    if (_isopen(fileid)) {
      errno = EEXIST;
      return 0;
    }

    if (theFile->access & ACCESS_APPEND) recnum = existingFileState->numRecords + 1;

    if ((theFile->access & ACCESS_APPEND) && (recfm == 'V')) {
      /* If it is variable length and we are appending record length could be anything */
      /* Check if a value was given in the paramoters */
      lrecl = 0;
      if (filespecwords[4][0]) {
        for (i=0; filespecwords[4][i]; i++) {
          if (!isdigit(filespecwords[4][i])) {errno = EINVAL; return 0;}
        }

        lrecl = atoi(filespecwords[4]);
        if (lrecl<1 || lrecl>65535) {errno = EINVAL; return 0;}

      }
      if (lrecl == 0) lrecl=65535; /* We have to assume the worse */
    }
  }

  theFile->buffer = malloc(lrecl + 2); /* \n and NULL */
  theFile->filemaxreclen = lrecl;
  theFile->maxreclen = lrecl;
  theFile->recnum = recnum;

  sprintf(theFile->name,"%s %s %s", fname, ftype, fmode); /* copy in the filename that will be used in error msgs */
  strcpy(theFile->fileid, fileid);

  theFile->fscb.format[0] = recfm;
  rc = CMSfileOpen(theFile->fileid, theFile->buffer, theFile->filemaxreclen, theFile->fscb.format[0],
                   1, theFile->recnum, &(theFile->fscb));

  switch (rc) {
    case 0:
      theFile->reclen = -1; /* Empty Buffer */
      theFile->recpos = 0;
      break;
    case 20:    /* Bad fileid */
      errno = EINVAL;
      free(theFile->buffer);
      theFile->buffer = NULL;
      return 0;
    case 28:  /*  the file was not found */
      if (theFile->access & ACCESS_TRUNCATE) break;   /* file not found is OK if not appending */
      errno = ENOENT;
      free(theFile->buffer);
      theFile->buffer = NULL;
      return 0;
  }

  /* Supports a cache? */
/*   if (recfm == 'F') theFile->status |= STATUS_CACHE; */
  /* Supports a Cache */
  theFile->status |= STATUS_CACHE;

  return 1;
}


/**************************************************************************************************/
/* close driver                                                                                   */
/* returns 0 on success                                                                           */
/**************************************************************************************************/
static int close(FILE * stream)
{
  int rc;
  rc = CMSfileClose(&stream->fscb);
  if (rc == 6) rc = 0; /* Ignore not open error - it means not read/writen yet */
  if (stream->buffer) {
    free(stream->buffer);
    stream->buffer = 0;
  }
  if (stream->access & ACCESS_TEMPFILE) {
    CMSfileErase(stream->fileid);
  }
  return rc;
}


/**************************************************************************************************/
/* Put file in write mode                                                                         */
/* returns 0 on success                                                                           */
/**************************************************************************************************/
static int write_mode(FILE * file)
{
  if (file->status & STATUS_READ) {
    /* Need to get CMS to reopen the file */
    file->error = reopen(file);
    if (file->error != 0) {
      errno = EIO;
      return file->error;
    }
  }
  file->status |= STATUS_WRITE;
  return 0;
}

/**************************************************************************************************/
/* write driver                                                                                   */
/* returns 0 on success                                                                           */
/**************************************************************************************************/
static int write(FILE * file)
{
  int pad;
  int rc;

  rc = write_mode(file);
  if (rc) return rc;

  if ((file->fscb.format[0] == 'F') && (file->reclen < file->filemaxreclen)) { /* PAD */
    if (file->access & ACCESS_TEXT) pad = ' ';
    else pad = 0;
    memset(file->buffer + file->reclen, pad, file->filemaxreclen - file->reclen);
    file->reclen = file->filemaxreclen;
  }
  else if ((file->fscb.format[0] == 'V') && (file->reclen == 0)) {
    /* CMS cannot handle zero length variable length records */
    /* So for text files we add a space                      */
    if (file->access & ACCESS_TEXT) {
      file->reclen = 1;
      file->buffer[0] = ' ';
    }
    else return 0; /* Might as well exit - nop */
  }

  if (file->access & ACCESS_APPEND) file->fscb.recordNum = 0;         /* appending, so start at next record */
  else file->fscb.recordNum = file->recnum;

  file->error = CMSfileWrite(&file->fscb, -1, file->reclen);

  if (file->error != 0) {
    errno = EIO;
    return file->error;
  }

  file->recnum = file->fscb.recordNum;
  if (file->recnum > file->records) file->records = file->recnum;

  return 0;
}

/**************************************************************************************************/
/* Put file in read mode                                                                          */
/* returns 0 on success                                                                           */
/**************************************************************************************************/
static int read_mode(FILE * file)
{
  if (file->status & STATUS_WRITE) {
    /* Need to get CMS to reopen the file */
    file->error = reopen(file);
    if (file->error != 0) {
      errno = EIO;
      return file->error;
    }
  }
  file->status |= STATUS_READ;
  return 0;
}

/**************************************************************************************************/
/* read (record) driver                                                                             */
/**************************************************************************************************/
static int read(FILE * file)
{
  int num;
  int rc;

  rc = read_mode(file);
  if (rc) return rc;

  file->fscb.recordNum = file->recnum;
  file->error = CMSfileRead(&file->fscb, -1, &num);
  file->recnum = file->fscb.recordNum;

  switch (file->error) {
    case 0:
      file->reclen = num;
      file->maxreclen = num;
      file->buffer[num] = 0;
      /* If last record in the file the file length is "unbound" */
      if (file->recnum >= file->records) {
        file->maxreclen = file->filemaxreclen;
        file->records = file->recnum;
      }
      return 0;
    case 12:
      file->error = 0;
      file->status |= STATUS_EOF;
      file->reclen = 0;
      file->maxreclen = file->filemaxreclen;
      return EOF;
    default:
      errno = EIO;  /* error reading from the file */
      file->reclen = 0;
      file->maxreclen = file->filemaxreclen;
      return EOF;
    }
}

/**************************************************************************************************/
/* postread (record) driver                                                                             */
/**************************************************************************************************/
static int postread(FILE * file)
{
  /* If last record in the file the file length is "unbound" */
  if (file->recnum >= file->records) {
    file->maxreclen = file->filemaxreclen;
    file->records = file->recnum;
  }
  return 0;
}


/**************************************************************************************************/
/* getpos driver                                                                                  */
/* returns position on success, -1 on error                                                       */
/**************************************************************************************************/
static int getpos(FILE * file)
{
  int pos;
  if (file->fscb.format[0]=='V') { /* Only valid for fixed format files */
    errno = EBADF;
    return -1;
  }

  if (file->access & ACCESS_TEXT) {
    /* We count the virtual \n character for each record */
    pos = (file->recnum - 1) * (file->filemaxreclen + 1);
  }
  else {
    pos = (file->recnum - 1) * file->filemaxreclen;
  }
  pos += file->recpos;

  return pos;
}


/**************************************************************************************************/
/* getend driver                                                                                  */
/* returns position on success, -1 on error                                                       */
/**************************************************************************************************/
static int getend(FILE * file)
{
  int pos;
  CMSFILEINFO *fileInfo;

  if (file->fscb.format[0]=='V') { /* Only valid for fixed format files */
    errno = EBADF;
    return -1;
  }

  if (file->access & ACCESS_TEXT) {
    /* We count the virtual \n character for each record */
    pos = file->records * (file->filemaxreclen + 1);
  }
  else {
    pos = file->records * file->filemaxreclen;
  }

  /*
   Note: We never have unflushed changes at the end of the file because
    1/ fseek is mandated to call fflush() first
    2/ fixed format files only supported
  */

  return pos;
}

/**************************************************************************************************/
/* setpos driver                                                                                  */
/* returns position on success, -1 on error                                                       */
/**************************************************************************************************/
static int setpos(FILE * file, int pos)
{
  int lrecl;
  int recnum;

  if (file->fscb.format[0]=='V') { /* Only valid for fixed format files */
    errno = EBADF;
    return -1;
  }

  if (file->access & ACCESS_TEXT) lrecl = file->filemaxreclen + 1;
  else lrecl = file->filemaxreclen;

  recnum = (pos / lrecl) + 1;
  if (recnum > file->records) {
    errno = EINVAL;
    return -1;
  }

  file->recnum = recnum;
  file->recpos = pos % lrecl;

  return pos;
}


/**************************************************************************************************/
/* Driver Strusture                                                                               */
/**************************************************************************************************/
CMSDRIVER dskiodrv = {open, close, getpos, getend, setpos, write, read, postread};
