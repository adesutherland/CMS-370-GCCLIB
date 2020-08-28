/**************************************************************************************************/
/* CMSIO.C - CMS Low Level IO                                                                     */
/*                                                                                                */
/* Part of GCCLIB - VM/370 CMS Native Std C Library; A Historic Computing Toy                     */
/*                                                                                                */
/* Contributors: See contrib memo                                                                 */
/*                                                                                                */
/* Released to the public domain.                                                                 */
/**************************************************************************************************/

/*
  TODO
    tmpname - Static Name and counter to CRAB. Check already opened
*/


#include <cmsruntm.h>
#include <cmssys.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <float.h>
#include <time.h>
#include <errno.h>

/* Define Drivers */
extern CMSDRIVER dskiodrv;
extern CMSDRIVER prtiodrv;
extern CMSDRIVER puniodrv;
extern CMSDRIVER rdriodrv;
extern CMSDRIVER coniodrv;

static CMSDRIVERS __iodrivers[] =
{
  {"File", &dskiodrv},
  {"Printer", &prtiodrv},
  {"Punch", &puniodrv},
  {"Reader", &rdriodrv},
  {"Console", &coniodrv},
  {0, NULL} /* End of drivers */
};

/* FILE checker */
#define badfile(f) ( (f)==NULL || (f)->device==NULL || (f)->validator1!='F' || (f)->validator2!='@' )

/* Add file to file list */
static void add_file(FILE* file) {
  FILE* tail = GETGCCCRAB()->filehandles;
  if (tail) {
    while (tail->next) tail = tail->next;
    tail->next = file;
    file->next = 0;
    file->prev = tail;
  }
  else { /* First file! */
    GETGCCCRAB()->filehandles = file;
    file->next = 0;
    file->prev = 0;
  }
}

/* Remove file from file list */
static void remove_file(FILE* file) {
  FILE* item = GETGCCCRAB()->filehandles;
  while (item) {
    if (item == file) {
      if (item->prev) item->prev->next = item->next;
      else GETGCCCRAB()->filehandles = item->next;
      if (item->next) item->next->prev = item->prev;
      file->next = 0;
      file->prev = 0;
      break;
    }
    item = item->next;
  }
}

/* Close all files - for exit routine */
void _clfiles() {
  FILE* item = GETGCCCRAB()->filehandles;
  while (item) {
    fclose(item);
    item = GETGCCCRAB()->filehandles;
  }
}

/* See if a file is open */
int _isopen(char* fileid) {
  FILE* item = GETGCCCRAB()->filehandles;
  while (item) {
    if (strcmp(item->fileid,fileid) == 0) {
      return 1;
    }
    item = item->next;
  }
  return 0;
}

/********************************************************************/
/* Init cache                                                       */
/* Returns - number of entries supported by the cache               */
/*           0 too small (<10 records). The cache was not created   */
/*          -1 on system failure                                    */
/********************************************************************/
static int init_cache(FILE* file, size_t cache_size, void* buffer) {
  CMSFILECACHE *cache;
  CMSCACHEENTRY *entry;
  CMSCACHEENTRY *lastentry;
  int i = 0;

  /* Need to work out how many entries I can store */
  int entrysize = file->filemaxreclen + sizeof(CMSCACHEENTRY);
  int noentries = (cache_size - sizeof(CMSFILECACHE)) / entrysize;
  if (noentries < 5) return 0; /* Not enouth space to make it worthwhile! */

  if (buffer) {
   memset(buffer, 0, cache_size);
   cache = buffer;
   cache->provided_cache_size = cache_size;
  }
  else {
    /* Round down the cache size */
    cache_size = (noentries * entrysize) + sizeof(CMSFILECACHE);

    cache = (CMSFILECACHE*)calloc(1, cache_size); /* Zeros the memory */
    if (!cache) return -1;
    cache->provided_cache_size = 0;
  }

  cache->noentries = noentries;
  cache->entrysize = entrysize;

  /* Setup Entry lists */
  /* First Entry */
  entry = (CMSCACHEENTRY *)((void*)cache + sizeof(CMSFILECACHE));
  cache->oldestused = entry;
  /* loop through the next enries */
  for (i=1; i<noentries; i++) { /* i is 1 because the first entry has been done */
    lastentry = entry;
    entry = (CMSCACHEENTRY *)((void*)lastentry + entrysize);
    lastentry->nextlastused = entry;
    entry->prevlastused = lastentry;
  }
  cache->newestused = entry;

  file->cache = cache;
  return noentries;
}

/********************************************************************/
/* Free Cache                                                       */
/********************************************************************/
static void free_cache(FILE* file) {
  if (file->cache) {
    if (!file->cache->provided_cache_size) free(file->cache);
    file->cache = 0;
  }
}

/********************************************************************/
/* Put the record at the top of the cache chains                    */
/********************************************************************/
static void move_top_of_lists(CMSFILECACHE *cache, CMSCACHEENTRY* e) {
  int bucket;

  /* Bucket List */
  if (e->previnbucket) { /* If no previnbucket I'm at the begining already */
    bucket = e->recnum % FCACHEBUCKETS;

    /* Remove me from the list */
    e->previnbucket->nextinbucket = e->nextinbucket;
    if (e->nextinbucket) e->nextinbucket->previnbucket = e->previnbucket;

    /* Put me at the top of the list */
    e->nextinbucket = cache->bucket[bucket];
    e->previnbucket = 0;
    cache->bucket[bucket] = e;
  }

  /* Last Used List */
  if (e->nextlastused) { /* If no nextlastused I'm at the top already */
    /* Remove me from the list */
    e->nextlastused->prevlastused = e->prevlastused;
    if (e->prevlastused) e->prevlastused->nextlastused = e->nextlastused;
    else {
      /* I was at the tail so need to update it */
      cache->oldestused = e->nextlastused;
      e->nextlastused->prevlastused = 0;
    }

    /* Put me at the top of the list */
    e->prevlastused = cache->newestused;
    e->nextlastused = 0;
    cache->newestused = e;
  }
}


/********************************************************************/
/* Get record from cache uses FILE.recnum to get required record    */
/* Return 0 in success (hit)                                        */
/*        1 on miss                                                 */
/*        -1 on system error                                        */
/********************************************************************/
static int read_cache(FILE* file) {
  CMSCACHEENTRY* topEntity;
  CMSCACHEENTRY* e;
  CMSFILECACHE *cache = file->cache;
  char* record;
  int bucket;

  if (!cache) return -1;
  bucket = file->recnum % FCACHEBUCKETS;

  topEntity = cache->bucket[bucket];

  for (e = topEntity; e; e = e->nextinbucket) {
    if (e->recnum == file->recnum) {
      /* Cache Hit */
      cache->hits++;
      move_top_of_lists(cache, e);
      record = (char*)((void*)e + sizeof(CMSCACHEENTRY));
      memcpy(file->buffer, record, e->reclen);
      file->reclen = e->reclen;
      file->maxreclen = e->maxreclen;
      file->buffer[file->reclen] = 0;
      return 0;
    }
  }
  cache->misses++;
  return 1;
}


/********************************************************************/
/* Write record to cache uses FILE.recnum to get record number      */
/* Return 0 in success, otherwise -1 on failure                     */
/********************************************************************/
static int write_cache(FILE* file) {
  CMSCACHEENTRY* topEntity;
  CMSCACHEENTRY* e;
  CMSFILECACHE *cache = file->cache;
  char* record;
  int bucket;

  if (!cache) return -1;
  bucket = file->recnum % FCACHEBUCKETS;

  topEntity = cache->bucket[bucket];

  for (e = topEntity; e; e = e->nextinbucket) {
    if (e->recnum == file->recnum) {
      /* Cache Hit */
      move_top_of_lists(cache, e);
      record = (char*)((void*)e + sizeof(CMSCACHEENTRY));
      memcpy(record, file->buffer, file->reclen);
      e->reclen = file->reclen;
      e->maxreclen = file->maxreclen;
      return 0;
    }
  }

  /* Ok we need to get a new record from the cache */
  e = cache->oldestused; /* Oldest Used */
  if (e->recnum) { /* Used before, need to remove it from its bucket */
    if (e->previnbucket) e->previnbucket->nextinbucket = e->nextinbucket;
    else cache->bucket[e->recnum % FCACHEBUCKETS] = e->nextinbucket;

    if (e->nextinbucket) e->nextinbucket->previnbucket = e->previnbucket;
  }

  e->recnum = file->recnum;

  /* Add to bucket */
  e->nextinbucket = cache->bucket[bucket];
  e->previnbucket = 0;
  cache->bucket[bucket] = e;

  move_top_of_lists(cache, e);
  record = (char*)((void*)e + sizeof(CMSCACHEENTRY));
  memcpy(record, file->buffer, file->reclen);
  e->reclen = file->reclen;
  e->maxreclen = file->maxreclen;
  return 0;
}

/********************************************************************/
/* Parse access mode string                                         */
/* returns the access mode or -1 on error                           */
/********************************************************************/
static int parse_access_mode(const char* accessmode) {
  char has_p = 0; /* i.e. a '+' */
  char has_b = 0;
  int mode = 0;

  switch (accessmode[0]) {
    case 'r':
      mode |= ACCESS_READ;
      break;
    case 'w':
      mode |= ACCESS_WRITE;
      mode |= ACCESS_TRUNCATE;
      break;
    case 'a':
      mode |= ACCESS_WRITE;
      mode |= ACCESS_APPEND;
      break;
    default:
     return -1;
  }

  if (accessmode[1]) {
    switch (accessmode[1]) {
      case '+':
        has_p = 1;
        break;
      case 'b':
        has_b = 1;
        break;
      default:
        return -1;
        break;
    }
    if (accessmode[2]) {
      switch (accessmode[2]) {
        case '+':
          if (has_p) return -1;
          has_p = 1;
          break;
        case 'b':
          if (has_b) return -1;
          has_b = 1;
          break;
        default:
          return -1;
          break;
      }
      if (accessmode[3]) return -1;
    }
  }

  if (!has_b) mode |= ACCESS_TEXT;
  if (has_p) {
    mode |= ACCESS_READ;
    mode |= ACCESS_WRITE;
    mode |= ACCESS_READWRITE;
  }
  return mode;
}

/**************************************************************************************************/
/* Worker funtion to open stream                                                                  */
/* Return 0 - All OK, Otherwise 1 (error)                                                         */
/**************************************************************************************************/
static int do_open(const char * filespec, const char * access, FILE* theFile)
{
  int w=0;
  int c=0;
  int p=0;
  char fsword[6][10] = {"","","","","",""};
  int accessMode;
  int d;
  int rc;

  /* Get the first 6 words from the filespec, for each word the first 9 charcters */
  /* This allows the drivers to detect filenames with more than 8 characters and  */
  /* too many words/options                                                       */
  for (w = 0; filespec[c] && w < 6; w++) {
    /* Leading Spaces */
    for (; filespec[c] && filespec[c] == ' '; c++);

    /* Word */
    for (p = 0; filespec[c] && p < 9 && filespec[c] != ' '; c++, p++)
      fsword[w][p] = toupper(filespec[c]);
    fsword[w][p] = 0;

    /* Word Overspill */
    for (; filespec[c] && filespec[c] != ' '; c++);
  }

  /* Get the access mode argument. */
  accessMode = parse_access_mode(access);
  if (accessMode < 0) {
    errno = EINVAL;
    return 1;
  }

  /* Setup/clean FILE* */
  memset(theFile, 0, sizeof(FILE));
  theFile->access = accessMode;
  theFile->ungetchar = -1;

  /* Ask the drivers to open the file in-turn */
  for (d=0; __iodrivers[d].name[0]; d++) {
    rc = __iodrivers[d].driver->open_func(fsword, theFile);
    if (!rc) return 1;
    else if (rc > 0) { /* -1 means the request is not for this driver, try the next one */
      theFile->device = __iodrivers[d].driver;
      theFile->validator1 = 'F';
      theFile->validator2 = '@';
      add_file(theFile);
      if (theFile->status & STATUS_CACHE) {
        size_t cache_size = theFile->filemaxreclen * 100; /* A cache for about 100 records */
        if (cache_size > 64*1024) cache_size = 64*1024; /* 64k max */
        if (cache_size < 8*1024) cache_size = 8*1024;   /* 8k min */
        init_cache(theFile, cache_size, NULL);
      }
      return 0;
    }
  }

  /* No driver accepted the request */
  errno = EINVAL;
  return 1;
}

FILE *
fopen(const char * filespec, const char * access)
/**************************************************************************************************/
/* FILE * fopen(const char * filespec, const char * access)                                       */
/*                                                                                                */
/* Open the specified file and return a stream associated with that file.                         */
/*    filespec is a pointer to a string containing the specification of the file to be opened:    */
/*       "CONSOLE"   terminal console (read or write)                                             */
/*       "PRINTER"   printer (write only)                                                         */
/*       "PUNCH"     card punch (write only)                                                      */
/*       "READER"    card reader (read only)                                                      */
/*       filename filetype [filemode [F|V [recordLength]]]                                        */
/*                   disk file (read or write), where:                                            */
/*                   filename is the up to 8 character name.                                      */
/*                   filetype is the up to 8 character type.                                      */
/*                   filemode is the up to 2 character disk mode leter and optional number.       */
/*                            Default "A1"                                                        */
/*                   F|V      specifies the record format fixed or variable.  It is ignored when  */
/*                            opening a file for reading.                                         */
/*                   reclen   specifies the record length.  It is required for fixed-length       */
/*                            files, and is taken as the maximum record length for variable-      */
/*                            length files.  It is ignored when opening a file for reading        */
/*                   Each of the above items must be separated by one or more blanks.             */
/*                                                                                                */
/*    access   specifies how the file will be accessed (i.e. for input, output, etc):             */
/*              r|w|a[+][b]                                                                       */
/*                                                                                                */
/* Returns:                                                                                       */
/*    a pointer to the stream associated with the open file, or NULL on failure.                  */
/*                                                                                                */
/* Notes:                                                                                         */
/*    1.  The maximum record length is 65535 bytes.                                               */
/*    2.  The maximum number of records in a file is 32768.                                       */
/*    3.  The maximum number of 800-byte blocks in a file is 16060.                               */
/*    4.  When writing a text file, a newline character signals the end of the current record.    */
/*    5.  When reading a text file, a newline character is inserted at the end of each record.    */
/*    6.  When writing a fixed-length text file the buffer will be padded with blanks if needed.  */
/*    7.  When writing a fixed-length binary file the buffer will be padded with NULLs if needed. */
/*    8.  Opening a file for writing causes the existing file of that name to be erased, if it    */
/*        exists.                                                                                 */
/**************************************************************************************************/
{
  FILE* theFile;
  int rc;

  theFile = (FILE*)malloc(sizeof(FILE));
  rc = do_open(filespec, access, theFile);
  if (rc) {
    free_cache(theFile);
    theFile->validator1 = 0;
    theFile->validator2 = 0;
    free(theFile);
    return NULL;
  }

  return theFile;
}


/**************************************************************************************************/
/* int fclose(FILE * file)                                                                        */
/*                                                                                                */
/* Close the open file in an appropriate and orderly fashion.                                     */
/*    stream    is a pointer to the open output stream.                                           */
/*                                                                                                */
/* Returns:                                                                                       */
/*    0 if all is well, EOF if there is an error.                                                 */
/**************************************************************************************************/
int fclose(FILE * file)
{
  int rc;

  if (badfile(file)) {
    errno = EINVAL;
    return EOF;
  }

  if (fflush(file) != 0) return EOF;                                   /* error flushing the stream */
  rc = file->device->close_func(file);
  free_cache(file);
  remove_file(file);

  file->device == NULL;
  file->validator1 = 0;
  file->validator2 = 0;

  free(file);
  return rc;
}                                                                                /* end of fclose */


/**************************************************************************************************/
/* write block/record to disk                                                                     */
/* Returns:                                                                                       */
/*    0, indicating that all is well, or a return code from the write operation.                  */
/**************************************************************************************************/
static int record_write(FILE * file) {
  int rc;
  if (!(file->status & STATUS_DIRTY)) return 0;            /* nothing to do if no unwritten data */
  rc = file->device->write_func(file);
  if (!rc) {
    file->status &= 0xFFFF - STATUS_DIRTY;
    if (file->cache) {
      write_cache(file);
    }
  }

  return rc;
}


int
fflush(FILE * file)
/**************************************************************************************************/
/* int fflush(FILE * stream)                                                                      */
/*                                                                                                */
/* Writes any unwritten characters to the specified stream.                                       */
/*                                                                                                */
/* Returns:                                                                                       */
/*    0, indicating that all is well, or a return code from the write operation.                  */
/*                                                                                                */
/* Notes:                                                                                         */
/*    1.  If the stream is not open for output, fflush is a NOP.                                  */
/**************************************************************************************************/
{
  if (badfile(file)) {
    errno = EINVAL;
    return EOF;
  }
  if (!(file->access & ACCESS_WRITE)) return 0;         /* nothing to do if not open for writing */

  return record_write(file);
}


/**************************************************************************************************/
/* read block/record from disk                                                                    */
/* Returns:                                                                                       */
/*    0, indicating that all is well, or a return code from the read operation                    */
/**************************************************************************************************/
static int record_read(FILE * file) {
  int rc = 1;
  int num;

  if (file->cache) rc = read_cache(file);

  if (rc) {
    rc = file->device->read_func(file);
    if (!rc && file->cache) write_cache(file);
  }

  num = file->reclen;
  if (!rc && (file->access & ACCESS_TEXT)) {
    /* We trim trailing spaces and add a newline - makes fixed format records easier to handle */
    for (num--; num > -1 && file->buffer[num] == ' '; num--);
    file->buffer[++num] = '\n';
    file->reclen = num++; /* reclen does not include \n */
  }
  file->buffer[num] = 0;

  /* Post Read Processing */
  if (file->device->postread_func) file->device->postread_func(file);

  return rc;
}

/**************************************************************************************************/
/* int fgetpos(FILE * stream, fpos_t * position)                                                  */
/*                                                                                                */
/* The function fills the fpos_t object pointed by pos with the information needed from the       */
/* stream's position indicator to restore the stream to its current position with a call to       */
/* fsetpos                                                                                        */
/*                                                                                                */
/* Returns:                                                                                       */
/*    0 on success, non-zero on failure.                                                          */
/**************************************************************************************************/
int fgetpos(FILE * file, fpos_t * position)
{
  if (badfile(file)) {errno = EINVAL; return 1;}
  if (file->recnum == -1) {errno = ENOTBLK; return 1;}
  position->file = file;
  position->recpos = file->recpos;
  position->recnum = file->recnum;
  return 0;
}

/**************************************************************************************************/
/* int fsetpos(FILE * stream, fpos_t * position)                                                  */
/*                                                                                                */
/* Restores the current position in the stream to pos.
/* The internal file position indicator associated with stream is set to the position represented */
/* by pos, which is a pointer to an fpos_t object whose value shall have been previously obtained */
/* by a call to fgetpos.                                                                          */
/* The end-of-file internal indicator of the stream is cleared after a successful call to this    */
/* function, and all effects from previous calls to ungetc on this stream are dropped.            */
/*                                                                                                */
/* Returns:                                                                                       */
/*    0 on success, non-zero on failure.                                                          */
/**************************************************************************************************/
int fsetpos(FILE * file, const fpos_t * position)
{
  int rc;
  if (badfile(file)) {errno = EINVAL; return 1;}
  if (position == NULL) {errno = EINVAL; return 1;}
  if (position->file != file) {errno = EINVAL; return 1;}
  if (position->recnum < 1) {errno = EINVAL; return 1;}
  if (position->recnum > file->records) {errno = EINVAL; return 1;}

  file->ungetchar = -1;
  file->status &= 0xFFFF - STATUS_EOF;
  fflush(file);

  file->recnum = position->recnum;
  rc = record_read(file);
  file->recpos = position->recpos;
  return rc;
}

/**************************************************************************************************/
/* void append(FILE * stream)                                                                     */
/*                                                                                                */
/* Non-standard GCC CMS extention                                                                 */
/* Move the file position indicator to the end of the specified stream, and clears the            */
/* error and EOF flags associated with that stream. This is the oposite of rewind()               */
/*    stream   a pointer to the open stream.                                                      */
/**************************************************************************************************/
void append(FILE * file)
{
  if (badfile(file)) return;

  if (file->recnum != -1) {
    fflush(file);
    file->recnum = file->records+1;
    file->reclen = -1; /* Empty record */
    file->recpos = 0;
  }
  else file->recpos = file->reclen;

  file->ungetchar = -1;
  file->status &= 0xFFFF - STATUS_EOF;
  file->error = 0;
  return;
}

/**************************************************************************************************/
/* int fateof(FILE * stream)                                                                      */
/* Non-standard GCC CMS extention                                                                 */
/* Detects if at EOF - unlike feof() it predicts if the next read will cause an EOF               */
/*                                                                                                */
/* Returns 1 if at EOF, EOF (-1) on error, or 0 is not at EOF                                     */
/*                                                                                                */
/**************************************************************************************************/
int fateof(FILE * stream) {
  int lrecl;
  if (badfile(stream)) {errno = EINVAL; return EOF;}

  if (stream->status & STATUS_EOF) return 1;

  if (stream->records == -1) return 0; /* Not a block device so can't detect if at EOF */

  if (stream->recnum != stream->records) return 0; /* not last record */

  lrecl = stream->reclen;
  if ((stream->access & ACCESS_TEXT) && lrecl) lrecl++;

  if (stream->recpos >= lrecl) return 1;
  else return 0;
}

/**************************************************************************************************/
/* FILE* fgethandle(char *fileName)                                                               */
/* Non-standard GCC CMS extention                                                                 */
/*                                                                                                */
/* Finds an open file that matches fileName                                                       */
/* It does not return stdin, stdout or stderr if assigned to the CONSOLE                          */
/*                                                                                                */
/* Return the FILE handle or NULL if the file is not opened                                       */
/*                                                                                                */
/**************************************************************************************************/
FILE* fgethandle(char *fileName) {
  int w=0;
  int c=0;
  int p=0;
  char fsword[3][10] = {"","",""};
  FILE* item;
  char buffer[19];

  /* Get the first 3 words from the filespec, for each word the first 9 charcters      */
  /* This allows us to detect filenames with more than 8 characters                    */
  /* This function anly considers the first 3 words ignoring any recfm or lrecl stuff  */
  for (w = 0; fileName[c] && w < 3; w++) {
    /* Leading Spaces */
    for (; fileName[c] && fileName[c] == ' '; c++);

    /* Word */
    for (p = 0; fileName[c] && p < 9 && fileName[c] != ' '; c++, p++)
      fsword[w][p] = toupper(fileName[c]);
    fsword[w][p] = 0;

    /* Word Overspill */
    for (; fileName[c] && fileName[c] != ' '; c++);
  }

  /* Basic Validation */
  if (strlen(fsword[0])>8) return NULL;
  if (strlen(fsword[0])==0) return NULL;
  if (strlen(fsword[1])>8) return NULL;
  if (strlen(fsword[2])>2) return NULL;

  item = GETGCCCRAB()->filehandles;

  if (strlen(fsword[1]) == 0) {
    /* Special files like PUNCH */
    while (item) {
      if (strcmp(item->fileid,fsword[0]) == 0) {
        return item;
      }
      item = item->next;
    }
  }
  else if (strcmp(fsword[2],"*") == 0) {
    /* "*" for a disk */
    sprintf(buffer,"%-8s%-8s", fsword[0], fsword[1]);
    while (item) {
      if (strncmp(item->fileid,buffer,16) == 0) {
        return item;
      }
      item = item->next;
    }
  }
  else {
    /* File */
    if (strlen(fsword[2])==0) {
      /* Drive A is the default */
      fsword[2][0]='A';
      fsword[2][1]=0;
    }
    else fsword[2][1]=0; /* Ignore mode number */

    sprintf(buffer,"%-8s%-8s%-1s", fsword[0], fsword[1], fsword[2]);
    while (item) {
      if (strncmp(item->fileid,buffer,17) == 0) {
        return item;
      }
      item = item->next;
    }
  }

  return 0;
}

/**************************************************************************************************/
/* int fgetrecs(FILE * stream)                                                                    */
/* Non-standard GCC CMS extention                                                                 */
/*                                                                                                */
/* Gets the number of records in a file                                                           */
/*                                                                                                */
/* Returns the number of records or EOF on error                                                  */
/*                                                                                                */
/**************************************************************************************************/
int fgetrecs(FILE * stream) {
  if (badfile(stream)) {errno = EINVAL; return EOF;}
  return stream->records;
}

/**************************************************************************************************/
/* int fgetlen(FILE * stream)                                                                     */
/*                                                                                                */
/* Non-standard GCC CMS extention                                                                 */
/* Gets the number of bytes/characters in a file                                                  */
/* Only works of fixed record length files                                                        */
/*                                                                                                */
/* Returns the number of bytes/characters or EOF on error                                         */
/*                                                                                                */
/**************************************************************************************************/
int fgetlen(FILE * stream) {
  int len;
  if (badfile(stream)) {errno = EINVAL; return EOF;}

  fflush(stream); /* TODO Need to consider if this should be done */

  len = stream->device->getend_func(stream);
  return len;
}

/**************************************************************************************************/
/* int fcachehits(FILE * stream)                                                                  */
/*                                                                                                */
/* The function returns the precentage of cache read hits achieved                                */
/* Non-standard GCC CMS extention                                                                 */
/*                                                                                                */
/* Returns:                                                                                       */
/*    Percentage of cache hits or -1 on error (device without a cache)                            */
/**************************************************************************************************/
int fcachehits(FILE * file)
{
  CMSFILECACHE *cache;
  if (badfile(file)) return -1;
  cache = file->cache;
  if (!cache) return -1;
  if (cache->hits + cache->misses == 0) return 0;
  return (cache->hits * 100) / (cache->hits + cache->misses);
}

/**************************************************************************************************/
/* int fgetrec(FILE * stream)                                                  */
/*                                                                                                */
/* The function returns the current record number (1 base)                                        */
/* Non-standard GCC CMS extention                                                                 */
/*                                                                                                */
/* Returns:                                                                                       */
/*    record number or 0 on failure / not block device                                            */
/**************************************************************************************************/
int fgetrec(FILE * file)
{
  fpos_t position;
  if (fgetpos(file, &position)) return 0;
  return position.recnum;
}

/**************************************************************************************************/
/* int fsetrec(FILE * stream, int recnum)                                                         */
/*                                                                                                */
/* The function sets/moves to the record number (1 base)                                          */
/* Non-standard GCC CMS extention                                                                 */
/*                                                                                                */
/* Returns:                                                                                       */
/*    record number or 0 on failure / not block device                                            */
/**************************************************************************************************/
int fsetrec(FILE * file, const int recnum)
{
  fpos_t position;

  if (recnum > file->records+1) {
    errno = EINVAL;
    return 0;
  }
  if (recnum < 1) {
    errno = EINVAL;
    return 0;
  }
  if (recnum > file->records) {
    append(file);
    return 0;
  }

  position.file = file;
  position.recnum = recnum;
  position.recpos = 0;
  if (fsetpos(file, &position)) return 0;
  return position.recnum;
}

void
clearerr(FILE * stream)
/**************************************************************************************************/
/* void clearerr(FILE * stream)                                                                   */
/*                                                                                                */
/* Resets the error status of the specified stream to 0.                                          */
/*    stream   a pointer to the open input stream.                                                */
/**************************************************************************************************/
{
  if (badfile(stream)) errno = EINVAL;
  else {
    stream->error = 0;
    stream->status &= 0xFFFF - STATUS_EOF;
  }
}


int
feof(FILE * stream)
/**************************************************************************************************/
/* int feof(FILE * stream)                                                                        */
/*                                                                                                */
/* Report whether end of file has been reached for the specified input stream.                    */
/*    stream   a pointer to the open input stream.                                                */
/*                                                                                                */
/* Returns:                                                                                       */
/*    1 if EOF has been reached, 0 otherwise.                                                     */
/**************************************************************************************************/
{
  if (badfile(stream)) {errno = EINVAL; return EOF;}
  return (stream->status & STATUS_EOF)? 1 : 0;
}


int
ferror(FILE * stream)
/**************************************************************************************************/
/* int ferror(FILE * stream)                                                                      */
/*                                                                                                */
/* Returns the error status of the specified stream.                                              */
/*    stream   a pointer to the open input stream.                                                */
/*                                                                                                */
/* Returns:                                                                                       */
/*    the error status, or 0 if there is no error.                                                */
/**************************************************************************************************/
{
  if (badfile(stream)) {
    errno = EINVAL;
    return EOF;
  }
  return stream->error;
}


int
fgetc(FILE * file)
/**************************************************************************************************/
/* int fgetc(FILE * stream)                                                                       */
/*                                                                                                */
/* Read the next character from the specified output stream.                                      */
/*    stream   a pointer to the open output stream.                                               */
/*                                                                                                */
/* Returns:                                                                                       */
/*    the character, or EOF if there is an error.                                                 */
/*                                                                                                */
/* Notes:                                                                                         */
/*    1.  A newline character is added at the end of each physical line read when reading TEXT    */
/*        files.                                                                                  */
/**************************************************************************************************/
{
  int c;
  int lrecl;

  if (badfile(file)) {errno = EINVAL; return EOF;}
  if (!(file->access & ACCESS_READ)) {
    file->error = 9;
    errno = EBADF;
    return EOF;
  }
  if (file->ungetchar >= 0) {                   /* was a character pushed back onto the file? */
    c = file->ungetchar;
    file->ungetchar = -1;
    return c;
  }

  if (file->access & ACCESS_TEXT) lrecl = file->reclen + 1;
  else lrecl = file->reclen;

  if ((file->recnum == 0) || (file->recpos >= lrecl)) {
    if (file->recnum != -1) file->recnum++;
    if (record_read(file) == EOF) return EOF;
    file->recpos = 0;
  }
  c = file->buffer[file->recpos++];
  return c;
}

/**************************************************************************************************/
/* int nextreclen(FILE * file)                                                                    */
/*                                                                                                */
/* This function returns the number of charcters the next fgets() will return                     */
/* including the /n (for text mode) but excluding the null terminator                             */
/* Non-standard GCC CMS extention - a REXX assist to a allow the input variable to be sized       */
/*                                                                                                */
/* Returns:                                                                                       */
/*    number of bytes for the next record, 0 for EOF, -1 for error                                */
/**************************************************************************************************/
int nextrecLen(FILE * file) {
  int c;
  int i=0;
  int lrecl;

  if (badfile(file)) {errno = EINVAL; return -1;}

  if (!(file->access & ACCESS_READ)) {
    file->error = 9;
    errno = EBADF;
    return -1;
  }

  if (file->access & ACCESS_TEXT) lrecl = file->reclen + 1;
  else lrecl = file->reclen;

  if (file->ungetchar >= 0) {  /* was a character pushed back onto the file? */
    if (file->ungetchar == '\n') return 1;
    else return lrecl - file->recpos + 1;
  }

  if ((file->recnum == 0) || (file->recpos >= lrecl)) {   /* empty buffer, read a new line */
    if (file->recnum != -1) file->recnum++;
    if (record_read(file)) return 0;
    file->recpos = 0;
    if (file->access & ACCESS_TEXT) lrecl = file->reclen + 1;
    else lrecl = file->reclen;
  }

  return lrecl - file->recpos;
}

char *
fgets(char * str, int num, FILE * file)
/**************************************************************************************************/
/* char * fgets(char * str, int num, FILE * stream)                                               */
/*                                                                                                */
/* Read up to 'num' - 1 characters from the specified file stream and place them into 'str',      */
/* terminating them with a NULL.  fgets() stops when it reaches the end of a line, in which case  */
/* 'str' will contain a newline character.  Otherwise, fgets() will stop when it reaches 'num'-1  */
/* characters or encounters the EOF character.                                                    */
/*                                                                                                */
/* Returns:                                                                                       */
/*    the a pointer to 'str', or NULL  if there is an error or EOF.                               */
/*                                                                                                */
/* Notes:                                                                                         */
/*    1.  fgets is only appropriate for a text file.                                              */
/**************************************************************************************************/
{
  int c;
  int i=0;
  int lrecl;

  if (num == 0) {errno = EINVAL; return NULL;}

  str[0] = 0; /* Safety ... */

  if (badfile(file)) {errno = EINVAL; return NULL;}

  if (!(file->access & ACCESS_READ)) {
    file->error = 9;
    errno = EBADF;
    return NULL;
  }

  if (num == 1) {
    str[0]=0;
    return str;
  }

  if (file->ungetchar >= 0) {  /* was a character pushed back onto the file? */
    str[0] = file->ungetchar;
    file->ungetchar = -1;
    if (str[0] == '\n' || num == 1) {
      str[1]=0;
      return str;
    }
    i++; /* Start position for the line read */
    num--;
  }

  if (file->access & ACCESS_TEXT) lrecl = file->reclen + 1;
  else lrecl = file->reclen;

  if ((file->recnum == 0) || (file->recpos >= lrecl)) {   /* empty buffer, read a new line */
    if (file->recnum != -1) file->recnum++;
    if (record_read(file)) return NULL;
    file->recpos = 0;
    if (file->access & ACCESS_TEXT) lrecl = file->reclen + 1;
    else lrecl = file->reclen;
  }

  num--; /* Space for null terminator */
  if (num > lrecl - file->recpos) num = lrecl - file->recpos;
  if (num) memcpy(str + i, file->buffer + file->recpos, num);
  str[i + num] = 0;
  file->recpos += num;
  return str;
}

int
fputc(int c, FILE * file)
/**************************************************************************************************/
/* int fputc(int c, FILE * stream)                                                                */
/*                                                                                                */
/* Write a character to the open output stream.  This is the same as putc().                      */
/*    c        the character to be written.                                                       */
/*    stream   a pointer to the open output stream.                                               */
/*                                                                                                */
/* Returns:                                                                                       */
/*    the character, or EOF if there is an error.                                                 */
/**************************************************************************************************/
{
  int writeRecord = 0;                          /* true if we should write out the current record */

  if (badfile(file)) {
    errno = EINVAL;
    return EOF;
  }
  if (!(file->access & ACCESS_WRITE)) {
    file->error = 9;
    errno = EBADF;
    return EOF;
  }

  if (file->recnum == 0) {
    file->recnum = 1;
    if (file->device->postread_func) file->device->postread_func(file);
  }


  if ((file->access & ACCESS_TEXT) && (c == '\n')) { /* newline means write record */
    writeRecord = 1;
  }
  else {
    file->buffer[file->recpos] = c;
    file->recpos++;
  }
  file->status |= STATUS_DIRTY;

  if (file->recpos > file->reclen) file->reclen = file->recpos;

  if (file->reclen >= file->maxreclen) writeRecord = 1;

  if (writeRecord) {
    if (record_write(file)) return EOF;
    file->reclen = 0;
    file->maxreclen = file->filemaxreclen;
    if (file->recnum != -1) {
      file->recnum++;
      if (file->recnum <= file->records) record_read(file);
    }
    file->recpos = 0;
  }
  return c;
}


/**************************************************************************************************/
/* Worker function to write a data block to a FILE one buffer full at at time                     */
/* Returns 0 on success, EOF on error                                                             */
/**************************************************************************************************/
static int write_data(const void *buffer, size_t size, FILE * file)
{
  int rc = 0;
  size_t s;
  size_t b = 0;
  while (size>0)
  {
    /* How much can we process */
    s = file->maxreclen - file->recpos;
    if (s > size) s = size;

    /* Fill the buffer */
    memcpy(file->buffer + file->recpos, buffer + b, s);
    file->recpos += s;
    b += s;
    if (file->recpos > file->reclen) file->reclen = file->recpos;
    file->status |= STATUS_DIRTY;
    if (file->recnum == 0) file->recnum = 1;
    size -= s;

    /* if the buffer is full flush it */
    if (file->reclen >= file->maxreclen) {
      rc = record_write(file);
      if (rc) return EOF;
      file->reclen = 0;
      file->maxreclen = file->filemaxreclen;
      if (file->recnum != -1) {
        file->recnum++;
        if (file->recnum <= file->records) record_read(file);
      }
      file->recpos = 0;
    }
  }
  return 0;
}


int
fputs(const char * str, FILE * file)
/**************************************************************************************************/
/* int fputs(const char * str, FILE * stream)                                                     */
/*                                                                                                */
/* Write a string to the open output stream.                                                      */
/*    str      the string to be written.                                                          */
/*    stream   a pointer to the open output stream.                                               */
/*                                                                                                */
/* Returns:                                                                                       */
/*    non-negative on success, EOF on failure.                                                    */
/**************************************************************************************************/
{
  int s;
  int rc = 0;

  if (badfile(file)) {
    errno = EINVAL;
    return EOF;
  }
  if (!(file->access & ACCESS_WRITE)) {
    file->error = 9;
    errno = EBADF;
    return EOF;
  }

  if (file->recnum == 0) {
    file->recnum = 1;
    if (file->device->postread_func) file->device->postread_func(file);
  }

  if (file->access & ACCESS_TEXT) {
    while (1) {
      /* find the next newline */
      for (s=0; str[s] && str[s]!='\n'; s++);

      /* Write this line out */
      if (s) {
        rc = write_data((void*)str, s, file);
        if (rc) return rc;
      }

      if (str[s] == 0) break; /* Found the end of str */

      /* Flush the line as it ended in a newline */
      file->status |= STATUS_DIRTY; /* Force a flush of an empty line */
      rc=record_write(file);
      if (rc) return EOF;
      file->recpos = 0;
      file->reclen = 0;
      if (file->recnum != -1) file->recnum++;
      str += s + 1;
    }
  }
  else {
    /* Weird to use fputs() for a binary FILE stream - but it is acceptable */
    return write_data((void*)str, strlen(str), file);
  }
  return 0;
}


int
fread(void * buffer, size_t size, size_t count, FILE * file)
/**************************************************************************************************/
/* int fread(void * buffer, size_t size, size_t count, FILE * stream)                             */
/*                                                                                                */
/* Read 'num' objects of size 'size' from the open input stream and write them to 'buffer'.       */
/*    buffer   the array into which the objects are to be read.                                   */
/*    size     the size of each object.                                                           */
/*    count    the number of objects to be read.                                                  */
/*    stream   a pointer to the open input stream.                                                */
/*                                                                                                */
/* Returns:                                                                                       */
/*    the number of objects read.                                                                 */
/*                                                                                                */
/* Notes:                                                                                         */
/*    1.  Use fread for writing binary data.  Newlines in the input stream are treated as any     */
/*        other character.                                                                        */
/*    2.  When the end of a record is reached, reading continues uninterrupted from the next      */
/*        record.  Thus the file is treated as a stream of bytes.                                 */
/**************************************************************************************************/
{
  size_t read, total, num;
  int lrecl;
  char *data = buffer; /* Just to give it a type */

  if (badfile(file)) {errno = EINVAL; return 0;}

  if (!file->access & ACCESS_READ) {
    file->error = 9;
    errno = EBADF;
    return 0;
  }

  total = size * count;
  read = 0;
  if (total == 0) {errno = EINVAL; return 0;}

  if (file->ungetchar >= 0) {  /* was a character pushed back onto the file? */
    data[0] = file->ungetchar;
    file->ungetchar = -1;
    read++;
    total--;
    data++;
  }

  while (total) {
    if (file->access & ACCESS_TEXT) lrecl = file->reclen + 1;
    else lrecl = file->reclen;

    if ((file->recnum == 0) || (file->recpos >= lrecl)) {   /* empty buffer, read a new line */
      if (file->recnum != -1) file->recnum++;
      file->recpos = 0;
      if (record_read(file)) return read / size; /* Number of records */
      if (file->access & ACCESS_TEXT) lrecl = file->reclen + 1;
      else lrecl = file->reclen;
    }

    num = total;
    if (num > lrecl - file->recpos) num = lrecl - file->recpos;
    if (num) memcpy(data, file->buffer + file->recpos, num);
    file->recpos += num;
    read += num;
    total -= num;
    data += num;
  }

  return read / size;
}


FILE *
freopen(const char * filespec, const char * access, FILE * stream)
/**************************************************************************************************/
/* FILE * freopen(const char * filespec, const char * access, FILE * stream)                      */
/*                                                                                                */
/* Reuses stream to either open the file specified by filename or to change its access mode.      */
/*                                                                                                */
/* If a new filename is specified, the function first attempts to close any file already          */
/* associated with stream (third parameter) and disassociates it. Then, independently of whether  */
/* that stream was successfuly closed or not, freopen opens the file specified by filename and    */
/* associates it with the stream just as fopen would do using the specified mode.                 */
/*                                                                                                */
/* If filename is a null pointer, the function attempts to change the mode of the stream.         */
/* Although a particular library implementation is allowed to restrict the changes permitted, and */
/* under which circumstances.                                                                     */
/*                                                                                                */
/* The error indicator and eof indicator are automatically cleared (as if clearerr was called).   */
/*                                                                                                */
/* If the file is successfully reopened, the function returns the pointer passed as parameter     */
/* stream, which can be used to identify the reopened stream.                                     */
/* Otherwise, a null pointer is returned.                                                         */
/* The errno variable is also set to a system-specific error code on failure.                     */
/*                                                                                                */
/**************************************************************************************************/
{
  int rc;
  char filename[21];

  if (badfile(stream)) {
    errno = EINVAL;
    return NULL;
  }

  if (!filespec) {
    strcpy(filename, stream->name);
    filespec = filename;
  }

  fflush(stream);
  stream->device->close_func(stream);
  free_cache(stream);
  remove_file(stream);

  rc = do_open(filespec, access, stream);
  if (rc) return NULL;

  return stream;
}


int
fseek(FILE * file, long offset, int origin)
/**************************************************************************************************/
/* int fseek(FILE * stream, long offset, int origin)                                              */
/*                                                                                                */
/* Moves the file position indicator for the specified stream to the specified position relative  */
/* to 'origin', which may be:                                                                     */
/*    SEEK_SET   Seek from the start of the file.                                                 */
/*    SEEK_CUR   Seek from the current location.                                                  */
/*    SEEK_END   Seek from the end of the file.                                                   */
/*                                                                                                */
/* Returns:                                                                                       */
/*    0 on success, non-zero on failure.                                                          */
/*                                                                                                */
/* Note:                                                                                          */
/*    1.  fseek is only valid for fixed record length disk files                                  */
/**************************************************************************************************/
{
  int here;
  int new;

  if (badfile(file)) {errno = EINVAL; return 1;}

  file->ungetchar = -1;
  file->status &= 0xFFFF - STATUS_EOF;
  fflush(file);

  switch (origin) {
      case SEEK_SET:
         new = offset;
         break;
      case SEEK_CUR:
         here = file->device->getpos_func(file);
         if (here == -1) return 1;
         new = here + offset;
         break;
      case SEEK_END:
         here = file->device->getend_func(file);
         if (here == -1) return 1;
         new = here + offset;
         break;
      default:
        errno = EINVAL;
        return 1;
  }

  if (file->device->setpos_func(file, new) < 0) return 1;
  return record_read(file);
}


long
ftell(FILE * file)
/**************************************************************************************************/
/* long ftell(FILE * stream)                                                                      */
/*                                                                                                */
/* Return the current file position for the specified stream.                                     */
/*    stream   a pointer to the open stream.                                                      */
/*                                                                                                */
/* Returns:                                                                                       */
/*    the position of the next character to be read or written, or -1 if there is an error.       */
/*                                                                                                */
/**************************************************************************************************/
{
  if (badfile(file)) {errno = EINVAL; return -1;}

  return file->device->getpos_func(file);
}


int
fwrite(const void * buffer, size_t size, size_t count, FILE * file)
/**************************************************************************************************/
/* int fwrite(const void * buffer, size_t size, size_t count, FILE * stream)                      */
/*                                                                                                */
/* Write 'count' objects of size 'size' from 'buffer' to the open output stream.                  */
/*    buffer   the array of objects to be written.                                                */
/*    size     the size of each object.                                                           */
/*    count    the number of objects to be written.                                               */
/*    stream   a pointer to the open output stream.                                               */
/*                                                                                                */
/* Returns:                                                                                       */
/*    the number of objects written.                                                              */
/*                                                                                                */
/* Notes:                                                                                         */
/*    1.  Use fwrite for writing binary data.  Newlines in the output stream are treated as any   */
/*        other character.                                                                        */
/*    2.  When the record length for the file is reached, that record is written to disk and a    */
/*        new record is started.  Thus the file is treated as a stream of bytes.                  */
/**************************************************************************************************/
{
 int s;
 int rc = 0;
 char *str;
 int len;

 if (badfile(file)) {
   errno = EINVAL;
   return EOF;
 }
 if (!(file->access & ACCESS_WRITE)) {
   file->error = 9;
   errno = EBADF;
   return EOF;
 }

 if (file->recnum == 0) {
   file->recnum = 1;
   if (file->device->postread_func) file->device->postread_func(file);
 }

 if (file->access & ACCESS_TEXT) {
   /* In this case we have to break records at newlines */
   str=buffer;
   len = size * count;
   while (1) {
     /* find the next newline */
     for (s=0; len && str[s]!='\n'; s++, len--);

     /* Write this line out */
     if (s) {
       rc = write_data((void*)str, s, file);
       if (rc) {
         len += s;
         break;
       }
     }

     if (!len) break; /* Found the end of str */

     /* Flush the line as it ended in a newline */
     file->status |= STATUS_DIRTY; /* Force a flush of an empty line */
     rc=record_write(file);
     if (rc) break; /* Hard to say how much was written ... */
     file->recpos = 0;
     file->reclen = 0;
     if (file->recnum != -1) file->recnum++;
     str += s + 1;
     len--;
   }
   return ((size*count)-len) / size;
 }

 else {
   if (write_data(buffer, size * count, file)) return 0;
   return count;
 }
}


char *
gets(char * str)
/**************************************************************************************************/
/* char * gets(char * str)                                                                        */
/*                                                                                                */
/* Read characters from the console and place them into 'str', terminating them with a NULL.      */
/* gets() stops when it reaches the end of a line.                                                */
/*                                                                                                */
/* Returns:                                                                                       */
/*    a pointer to 'str', or EOF if there is an error.                                            */
/*                                                                                                */
/* Notes:                                                                                         */
/*    1.  The buffer to which 'str' points must be 131 bytes long.                                */
/*    2.  Depricated in later stabdards - unsafe                                                  */
/**************************************************************************************************/
{
  int length;
  if (fgets(str, 130, stdin)) {
    length = strlen(str);
    if (length && str[length-1] =='\n')
       str[length-1] = 0;         /* remove newline */
  }
  return str;
}


int
puts(char * str)
/**************************************************************************************************/
/* int puts(const char * str)                                                                     */
/*                                                                                                */
/* Writes the C string pointed by str to the standard output (stdout)                             */
/*  *** and appends a newline character ('\n') ***                                                */
/*                                                                                                */
/* Returns:                                                                                       */
/*    non-negative on success, EOF on failure.                                                    */
/**************************************************************************************************/
{
  if (fputs(str, stdout) == EOF) return EOF;
  return fputc('\n',stdout);
}


void
rewind(FILE * file)
/**************************************************************************************************/
/* void rewind(FILE * stream)                                                                     */
/*                                                                                                */
/* Move the file position indicator to the beginning of the specified stream, and clears the      */
/* error and EOF flags associated with that stream.                                               */
/*    stream   a pointer to the open stream.                                                      */
/**************************************************************************************************/
{
  if (badfile(file)) return;
  fflush(file);
  file->recpos = 0;
  if (file->recnum != -1) file->recnum = 0;
  file->reclen = -1;
  file->ungetchar = -1;
  file->status &= 0xFFFF - STATUS_EOF - STATUS_DIRTY;
  file->error = 0;
  return;
}

void
setbuf(FILE * file, char * buffer)
/**************************************************************************************************/
/* void setbuf(FILE * file, char *buffer)                                                         */
/*                                                                                                */
/* Specifies the buffer to be used by the stream for I/O operations, used as a cache. Or,         */
/* alternatively, if buffer is a null pointer, buffering is disabled for the stream               */
/* The buffer is assumed to be at least BUFSIZ bytes in size                                      */
/*                                                                                                */
/* A call to this function is equivalent to calling setvbuf with _IOFBF as mode and BUFSIZ as     */
/* size (when buffer is not a null pointer), or equivalent to calling it with _IONBF as mode      */
/* (when it is a null pointer).                                                                   */
/*                                                                                                */
/**************************************************************************************************/
{
  if (buffer) setvbuf(file, buffer, _IOFBF, BUFSIZ);
  else setvbuf(file, buffer, _IONBF, 0);
}


int
setvbuf(FILE * file, char * buffer, int mode, size_t size)
/**************************************************************************************************/
/* int setvbuf(FILE * file, char * buffer, int mode, size_t size)                                 */
/*                                                                                                */
/* Specifies a buffer for stream. The function allows to specify the mode and size of the buffer  */
/* (in bytes).                                                                                    */
/*                                                                                                */
/* If buffer is a null pointer, the function automatically allocates a buffer                     */
/* (using size as a hint on the size to use). Otherwise, the array pointed by buffer may be       */
/* used as a buffer of size bytes.                                                                */
/*                                                                                                */
/* Notes                                                                                          */
/*   Modes _IOFBF and _IOLBF are equivalent and are a line cache for reading                      */
/*                                                                                                */
/* Returns:                                                                                       */
/*   If the buffer is correctly assigned to the file, a zero value is returned.                   */
/*   Otherwise, a non-zero value is returned                                                      */
/**************************************************************************************************/
{
  if (badfile(file)) return -1;
  if (mode == _IONBF) {
    free_cache(file);
    return 0;
  }
  else {
    if (!size) size = BUFSIZ;
    if (!(file->status & STATUS_CACHE)) return -1;
    free_cache(file);
    return (init_cache(file, size, buffer) < 1);
  }
}


FILE *
tmpfile(void)
/**************************************************************************************************/
/* FILE * tmpfile(void)                                                                           */
/*                                                                                                */
/* Creates a temporary binary file, open for update ("wb+" mode, variable length records) wit     */
/* a filename guaranteed to be different from any other existing file.                            */
/*                                                                                                */
/* The temporary file created is automatically deleted when the stream is closed (fclose)         */
/* or when the program terminates normally.                                                       */
/*                                                                                                */
/* Returns:                                                                                       */
/*   If successful, the function returns a stream pointer to the temporary file created.          */
/*   On failure, NULL is returned.                                                                */
/**************************************************************************************************/
{
  FILE *temp;
  char name[21];
  if (!tmpnam(name)) return NULL;
  temp = fopen(name, "wb+");
  if (!temp) return NULL;
  temp->access |= ACCESS_TEMPFILE;
  return temp;
}


char *
tmpnam(char * name)
/**************************************************************************************************/
/* char * tmpnam(char * name)                                                                     */
/*                                                                                                */
/* Generate temporary filename                                                                    */
/* Returns a string containing a file name different from the name of any existing file, and thus */
/* suitable to safely create a temporary file without risking to overwrite an existing file.      */
/*                                                                                                */
/* If str is a null pointer, the resulting string is stored in an internal static array that can  8/
/* be accessed by the return value. The content of this string is preserved at least until a      */
/* subsequent call to this same function, which may overwrite it.                                 */
/*                                                                                                */
/* If str is not a null pointer, it shall point to an array of at least L_tmpnam characters that  */
/* will be filled with the proposed temporary file name.                                          */
/*                                                                                                */
/**************************************************************************************************/

{
  static int counter = 0; /* CRAB */
  static char staticName[21]; /* CRAB */
  CMSFILEINFO* file_state;
  struct tm * t;
  time_t theTime;
  char fn[9];
  char ft[9];
  char fileid[19];
  char *fm = "A1";

  if (!name) name = staticName;

  time(&theTime);
  t = localtime(&theTime);
  strftime(fn, 9, "T%y%j%H", t);
  strftime(ft, 5, "%M%S", t);

  for (counter++; counter<10000; counter++) {
    sprintf(fileid, "%s%s%04d%s", fn, ft, counter, fm);
    if (CMSfileState(fileid, &file_state)) {
      /* Good - File does not exist (or some other error!) */
      sprintf(name, "%s %s%04d %s", fn, ft, counter, fm);
      return name;
    }
  }

  return NULL; /* Failed to get a free file name! */
}


int
ungetc(int c, FILE * file)
/**************************************************************************************************/
/* int ungetc(int c, FILE * stream)                                                               */
/*                                                                                                */
/* Push a character back onto the open input stream 'stream', such that it is the next character  */
/* to be read.                                                                                    */
/*    c        the character to be pushed back onto the stream.                                   */
/*    stream   a pointer to the open input stream.                                                */
/*                                                                                                */
/* Returns:                                                                                       */
/*    c if successful, EOF if not.                                                                */
/*                                                                                                */
/* Notes:                                                                                         */
/*    1.  Only 1 character can be in the unget buffer.  If a second call to ungetc is made before */
/*        the previous character is read, it is not placed in the buffer and EOF is returned.     */
/**************************************************************************************************/
{
  if (badfile(file)) {errno = EINVAL; return EOF;}
  if (!(file->access & ACCESS_READ)) {errno = EPERM; return EOF;}
  if (file->ungetchar == -1) {                                      /* OK to push back a character? */
     file->ungetchar = c;
     return c;
  }
  else return EOF;
}
