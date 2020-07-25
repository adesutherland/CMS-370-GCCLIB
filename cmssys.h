/**************************************************************************************************/
/* CMSSYS.H: CMS system functions implemented in CMSSYS ASSEMBLE                                  */
/*                                                                                                */
/* Released to the public domain.                                                                 */
/*                                                                                                */
/* Robert O'Hara, Redmond Washington, July 2010.                                                  */
/**************************************************************************************************/
#ifndef CMSSYS_INCLUDED
#define CMSSYS_INCLUDED

#define GCCLIB_VERSION "0.7.15"

/**************************************************************************************************/
/* CMSFILE maps the CMS FSCB (File System Control Block).                                         */
/**************************************************************************************************/
typedef struct {
   char command[8];                                 /* command (note: no trailing NULL character) */
   char filename[8];                               /* filename (note: no trailing NULL character) */
   char filetype[8];                               /* filetype (note: no trailing NULL character) */
   char filemode[2];                               /* filemode (note: no trailing NULL character) */
   short recordNum;                                              /* relative record (item) number */
   char * buffer;                                               /* address of read / write buffer */
   int bufferSize;                                                            /* length of buffer */
   char format[2];              /* record format: 'F ' or 'V ' (note: no trailing NULL character) */
   short numRecords;                                        /* number of records to read or write */
   int bytesRead;                                                /* number of bytes actually read */
   } CMSFILE;

/**************************************************************************************************/
/* CMSFILEINFO maps the CMS FST (File Status Table).                                              */
/**************************************************************************************************/
typedef struct {
   char filename[8];                               /* filename (note: no trailing NULL character) */
   char filetype[8];                               /* filetype (note: no trailing NULL character) */
   short filedate;                                            /* date file was last written, MMDD */
   short filetime;                                            /* time file was last written, HHMM */
   short writePtr;                                                 /* write pointer (item) number */
   short readptr;                                                   /* read pointer (item) number */
   char filemode[2];           /* filemode (letter and number) (note: no trailing NULL character) */
   short numRecords;                                                         /* number of records */
   short chainLink;                                                   /* first chain link pointer */
   char format;                                                    /* record format: 'F ' or 'V ' */
   char flag;                                                                    /* FST flag byte */
   int lrecl;                                                            /* logical record length */
   short numBlocks;                                                 /* number of 800-bypte blocks */
   short fileYear;                                                  /* year file was last written */
   } CMSFILEINFO;

/**************************************************************************************************/
/* HI, TS, TE - Immediate flags                                                                   */
/**************************************************************************************************/
#define TRACEFLAG 0x1
#define HALTFLAG 0x10

/**************************************************************************************************/
/* int CMSGetFlag(int flag)                                                                       */
/*                                                                                                */
/* Get the value of the CMS Flag (TRACEFLAG or HALTFLAG)                                          */
/* Returns 1 or 0                                                                                 */
/* Implemented as a macro for performance - polling                                               */
/* The flag is stored at 0x5e6                                                                    */
/**************************************************************************************************/
#define CMSGetFlag(flag)   (int)(((*(char*)0x5e6) & (flag)) ? 1 : 0)

/**************************************************************************************************/
/* void CMSSetFlag(int flag, int value) - thanks to Bob Bolch                                     */
/*                                                                                                */
/* Set the value of the CMS Flag (TRACEFLAG or HALTFLAG)                                          */
/* Args flag and value (0 or 1)                                                                   */
/**************************************************************************************************/
void CMSSETFL(int flag, int value);
#define CMSSetFlag(s1, s2) (CMSSETFL((s1),(s2)))

/**************************************************************************************************/
/* void CMSSetNUCON(void *address, int value)                                                     */
/*                                                                                                */
/* Set memory in NUCON                                                                            */
/**************************************************************************************************/
void CMSSETNU(void *address, int value);
#define CMSSetNUCON(s1, s2) (CMSSETNU((s1),(s2)))

/**************************************************************************************************/
/* char CMSGetNUCON(void *address)                                                                 */
/*                                                                                                */
/* Get memory in NUCON                                                                            */
/**************************************************************************************************/
#define CMSGetNUCON(address)   ((*(int*)(address)))

/**************************************************************************************************/
/* char * CMSargstring(void)                                                                      */
/*                                                                                                */
/* Return the unedited argument string passed to the user program when it was invoked as a CMS    */
/* command.                                                                                       */
/*                                                                                                */
/* Returns:                                                                                       */
/*    a pointer to the argument string.                                                           */
/**************************************************************************************************/
char *__argstr(void);
#define CMSargstring() (__argstr())

/**************************************************************************************************/
/* int CMScardPunch(char * line)                                                                  */
/*                                                                                                */
/* Write a line to the virtual card punch.                                                        */
/*    line   is a pointer to the line to be written to the punch. The line must be 80 bytes long. */
/*                                                                                                */
/* Returns:                                                                                       */
/*     0     Success.                                                                             */
/*     2     Unit check.                                                                          */
/*     3     Unknown error.                                                                       */
/*   100     Punch not attached.                                                                  */
/*                                                                                                */
/* Notes:                                                                                         */
/*    1.  Use the CP CLOSE command to close the virtual card punch.  You can issue this via the   */
/*        CMScommand function.                                                                    */
/**************************************************************************************************/
int __punchc(char * line);
#define CMScardPunch(s1) (__punchc((s1)))

/**************************************************************************************************/
/* int CMScardRead(char * line, int * len)                                                        */
/*                                                                                                */
/* Read a line from the virtual card reader.                                                      */
/*    line   is a pointer to the buffer into which the line is read.  As it is possible to read   */
/*           printer output files, the buffer should be at least 133 bytes in size.  The incoming */
/*           line is terminated with a null character, forming a C string.                        */
/*    len    is a poitner to a variable that receives the number of characters read.              */
/*                                                                                                */
/* Returns:                                                                                       */
/*     0     Success.                                                                             */
/*     1     End-of-file.                                                                         */
/*     2     Unit check.                                                                          */
/*     3     Unknown error.                                                                       */
/*     5     Length not equal to requested length.                                                */
/*   100     Punch not attached.                                                                  */
/**************************************************************************************************/
int __rdcard(char * line, int * len);
#define CMScardRead(s1, i2) (__rdcard((s1),(i2)))

/**************************************************************************************************/
/* CMSclock(void * clock)                                                                         */
/*                                                                                                */
/* Returns the system clock time as well as the number of seconds since 1/1/1970.                 */
/*    clock  is an 8-byte variable into which the system clock time will be stored.               */
/* Returns:                                                                                       */
/*    (int)  The number of seconds since 1/1/1970.                                                */
/**************************************************************************************************/
int __getclk(void * clock);
#define CMSclock(s1) (__getclk((s1)))

/**************************************************************************************************/
/* int CMScommand(char * cmdLine, int cmdFlag)                                                    */
/*                                                                                                */
/* Invoke a CMS command or service.                                                               */
/*    cmdLine   is the command line to be executed by CMS.                                        */
/*    cmdFlag   specifies how the command to be invoked:                                          */
/*              CMS_COMMAND   the command is invoked as if called from an EXEC.                   */
/*              CMS_CONSOLE   the command is invoked as if typed at the console.                  */
/*              CMS_FUNCTION  the command is invoked as a CMS function.                           */
/*                                                                                                */
/* Returns:                                                                                       */
/*    the return code from the invoked command.                                                   */
/*                                                                                                */
/* Notes:                                                                                         */
/*    1.  Both a standard and extended parameter list (plist) are passed to the command.          */
/*    2.  Be aware that the command invoked can potentially overlay your program in memory.       */
/**************************************************************************************************/
int __cmscmd(char * cmdLine, int cmdFlag);
#define CMScommand(s1, i1) (__cmscmd((s1),(i1)))
#define CMS_COMMAND  1
#define CMS_CONSOLE 11
#define CMS_FUNCTION 0

/**************************************************************************************************/
/* int CMSconsoleRead(char * line)                                                                */
/*                                                                                                */
/* Read a line from the terminal.                                                                 */
/*    line   is a pointer to the 131-character buffer into which the line is read.  The incoming  */
/*           line is terminated with a null character, forming a C string.                        */
/*                                                                                                */
/* Returns:                                                                                       */
/*    the length of the string placed in the buffer.                                              */
/**************************************************************************************************/
int __rdterm(char * line);
#define CMSconsoleRead(s1) (__rdterm((s1)))

/**************************************************************************************************/
/* int CMSconsoleWait(void)                                                                       */
/*                                                                                                */
/* Waits for terminal I/O to complete.                                                            */
/*                                                                                                */
/* Returns:                                                                                       */
/*    Return code from the WAITT function, always 0.                                              */
/**************************************************************************************************/
int __waitt(void);
#define CMSconsoleWait() (__waitt())

/**************************************************************************************************/
/* int CMSconsoleWrite(char * line, int edit)                                                     */
/*                                                                                                */
/* Display a line on the terminal.                                                                */
/*    line   is a pointer to the line to be displayed.  The line may contain embedded newline     */
/*           characters (\n) which are honored.  Normally the line should end with a newline      */
/*           character, unless a subsequent call is to continue writing on the same line.  The    */
/*           string must be terminated with a null character.  The maximum number of characters   */
/*           to be displayed is 130; if the string pointed to by 'line' is greater than this, it  */
/*           is truncated without warning.                                                        */
/*    edit   is CMS_EDIT (1) if trailing blanks are to be removed and a newline character added   */
/*           to the end of the line; CMS_NOEDIT (0) if not.                                       */
/*                                                                                                */
/* Returns:                                                                                       */
/*    0                                                                                           */
/*                                                                                                */
/* Notes:                                                                                         */
/*    1. On a 3270 display terminal a newline character is always appended to the line.           */
/**************************************************************************************************/
#define CMS_EDIT 1
#define CMS_NOEDIT 0
int __wrterm(char * line, int edit);
#define CMSconsoleWrite(s1, i2) (__wrterm((s1),(i2)))

/**************************************************************************************************/
/* int CMSdebug(int retcode)                                                                      */
/*                                                                                                */
/* Call the debug function (where a breakpoint can be set using the PER command) and return with  */
/* the specified return code.                                                                     */
/*    retcode  is the return code.                                                                */
/*                                                                                                */
/* Returns:                                                                                       */
/*    (int)    as specified.                                                                      */
/**************************************************************************************************/
int __debug(int retcode);
#define CMSdebug(i1) (__debug((i1)))

/**************************************************************************************************/
/* int CMSfileClose(CMSFILE* file)                                                               */
/*                                                                                                */
/* Close an open file and save its current status to disk.                                        */
/*    file       is a pointer to the file handle (FSCB) of the file to be closed.                 */
/*                                                                                                */
/* Returns:                                                                                       */
/*     0     Success.                                                                             */
/*     6     The file is not open.                                                                */
/**************************************************************************************************/
int __fsclos(CMSFILE* file);
#define CMSfileClose(s1) (__fsclos((s1)))

/**************************************************************************************************/
/* int CMSfileErase(char * fileid)                                                                */
/*                                                                                                */
/* Delete a CMS file.                                                                             */
/*    fileid     is a pointer to the CMS fileid, an 18 character field.  The first 8 characters   */
/*               are the filename, the next 8 are the filetype, and the last 2 are the filemode.  */
/*               Filename, filetype, and filemode must be in uppercase.  None of these fields are */
/*               terminated with a NULL character.                                                */
/*                                                                                                */
/* Returns:                                                                                       */
/*     0     Success.                                                                             */
/*    24     Parameter list error (invalid character in fileid).                                  */
/*    28     The file was not found.                                                              */
/*    36     Disk not accessed.                                                                   */
/**************************************************************************************************/
int __fseras(char * fileid);
#define CMSfileErase(s1) (__fseras((s1)))

/**************************************************************************************************/
/* int CMSfileOpen(char * fileid, char * buffer, int bufferSize, char format, int numRecords,     */
/*                 int recordNum, CMSFILE * file)                                                 */
/*                                                                                                */
/* Open a file for reading or writing.                                                            */
/*    fileid     is a pointer to the CMS fileid, an 18 character field.  The first 8 characters   */
/*               are the filename, the next 8 are the filetype, and the last 2 are the filemode.  */
/*               Filename, filetype, and filemode must be in uppercase.  None of these fields are */
/*               terminated with a NULL character.                                                */
/*    buffer     is the address of the buffer from which records are read from or written to.     */
/*    bufferSize is the size of the buffer.                                                       */
/*    format     is 'F' if you are writing a file of fixed-length records, 'V' if you are writing */
/*               a file of variable-length records.  This parameter is ignored if you are reading */
/*               from the file.                                                                   */
/*    numRecords is the number of records to be read or written at a time.  This parameter can be */
/*               greater than 1 only when reading or writing fixed-length records.                */
/*    recordNum  is the 1-based record number at which to begin reading or writing.  Specify 0    */
/*               when writing to append records to the file.                                      */
/*    file       is a pointer to a CMS file handle (FSCB).  It is updated by CMS when the file is */
/*               opened.  Pass this as an argument to other file I/O functions.                   */
/*                                                                                                */
/* Returns:                                                                                       */
/*     0     Success.                                                                             */
/*    20     The fileid is invalid.                                                               */
/*    28     The file was not found.  (Not an error if you intend to write a new file.)           */
/*                                                                                                */
/* Notes:                                                                                         */
/*    1. After the file has been opened, the first call to either CMSfileRead or CMSfileWrite     */
/*       determines whether the file is open for reading or writing.  To switch between reading   */
/*       writing, the file must be closed, then re-opened.                                        */
/*    2. It is possible to overwrite a record within a file.  However, care must be taken that    */
/*       the record being written is the same length as the one it is replacing.  Failing to do   */
/*       this can result in corruption of the file and possibly the entire mini-disk.             */
/*    3. To overwrite an existing file with new records, call CMSfileErase to first erase the     */
/*       file, then open the file and begin writing from record 1.                                */
/**************************************************************************************************/
int __fsopen(char * fileid, char * buffer, int bufferSize, char format,
                int numRecords, int recordNum, CMSFILE * file);
#define CMSfileOpen(s1, s2, i3, c4, i5, i6, s7) (__fsopen((s1),(s2),(i3),(c4),(i5),(i6),(s7)))

/**************************************************************************************************/
/* int CMSfilePoint(CMSFILE * file, int recordNum, int readWrite)                                 */
/*                                                                                                */
/* Set the read or write pointer to a specific record in the file.                                */
/*    file       is a pointer to a CMS file handle (FSCB).  It is updated by CMS when the file is */
/*               opened.  Pass this as an argument to other file I/O functions.                   */
/*    recordNum  is the 1-based record number at which to set the read or write pointer.          */
/*    readWrite  is CMS_POINTREAD to set the read pointer, CMS_POINTWRITE to set the write        */
/*               pointer.                                                                         */
/*                                                                                                */
/* Returns:                                                                                       */
/*     0     Success.                                                                             */
/*     1     The file was not found.                                                              */
/*     2     Parameter list error.                                                                */
/*                                                                                                */
/* Notes:                                                                                         */
/*    1. Pointing to an record number greater than the number of records in the file yields       */
/*       unpredictable results.                                                                   */
/**************************************************************************************************/
int __fspoin(CMSFILE * file, int recordNum, int readWrite);
#define CMSfilePoint(s1, i2, i3) (__fspoin((s1),(i2),(i3)))
#define CMS_POINTREAD 0
#define CMS_POINTWRITE 1

/**************************************************************************************************/
/* int CMSfileRead(CMSFILE * file, int recordNum, int *bytesRead)                                */
/*                                                                                                */
/* Read one or more records from an open file.                                                    */
/*    file       is the file handle (FSCB) of the open file.                                      */
/*    recordNum  is the record number at which to begin reading.  To read sequential records,     */
/*               specify 1 on the first call to CMSfileRead, and 0 on subsequent calls.           */
/*               Alternately, specify 1 as the 'recordNum' parameter in CMSfileOpen, and set      */
/*               'recordNum' to 0 on all CMSfileRead calls.  To use the record number currently   */
/*               set in the file handle (FSCB), specify -1.                                       */
/*    bytesRead  (returned) is the number of bytes actually read.                                 */
/*                                                                                                */
/* Returns:                                                                                       */
/*     0     Success.                                                                             */
/*     1     The file was not found.                                                              */
/*     2     Invalid buffer address.                                                              */
/*     3     Permanent I/O error.                                                                 */
/*     5     Number of records is less than 1, or greater than 32,768.                            */
/*     7     Invalid record format (only checked when file is first opened for reading).          */
/*     8     Incorrect length.                                                                    */
/*     9     File open for output.                                                                */
/*    11     Number of records greater than 1 for variable-length file.                           */
/*    12     End of file reached.                                                                 */
/*    13     Variable-length file has invalid displacement in active file table.                  */
/*    14     Invalid character in filename.                                                       */
/*    15     Invalid character in filetype.                                                       */
/*                                                                                                */
/* Notes:                                                                                         */
/*    1. The bytes read are placed in the buffer specified in the previous call to CMSfileOpen.   */
/*       They are NOT terminated with a null character.                                           */
/**************************************************************************************************/
int __fsread(CMSFILE * file, int recordNum, int *bytesRead);
#define CMSfileRead(s1, i2, s3) (__fsread((s1),(i2),(s3)))

/**************************************************************************************************/
/* int CMSfileRename(char * oldFileid, char * newFileid)                                          */
/*                                                                                                */
/* Rename a file.                                                                                 */
/*    oldFileid  is a pointer to the fileid of the file to be renamed.                            */
/*    newFileid  is a pointer to the fileid of the new name for the file.                         */
/*                                                                                                */
/*    Note that both oldFileid and newFileid are pointers to a CMS fileid, an 18 character field. */
/*    The first 8 characters are the filename, the next 8 are the filetype, and the last 2 are    */
/*    the filemode.  Filename, filetype, and filemode must be in uppercase.  None of these fields */
/*    are terminated with a NULL character.                                                       */
/*                                                                                                */
/* Returns:                                                                                       */
/*     0     Success.                                                                             */
/*    20     The fileid is invalid.                                                               */
/*    24     Parameter list error (invalid character in fileid).                                  */
/*    28     The file was not found, or newFileid already exists.                                 */
/*                                                                                                */
/* Notes:                                                                                         */
/*    1. This function calls the CMS RENAME command, which executes in the CMS transient area.    */
/*       Thus this function cannot be called from a C program executing in the transient area.    */
/*    2. The RENAME command may issue error messages for certain errors.                          */
/**************************************************************************************************/
int __rename(char * oldFileid, char * newFileid);
#define CMSfileRename(s1, s2) (__rename((s1),(s2)))

/**************************************************************************************************/
/* int CMSfileState(char * fileid, CMSFILEINFO ** fileInfo)                                       */
/*                                                                                                */
/* Determine whether or not a file exists.                                                        */
/*    fileid     is a pointer to the CMS fileid, an 18 character field.  It may be specified in   */
/*               any case, but will be translated to upper case.  The first 8 characters are the  */
/*               filename, the next 8 are the filetype, and the last 2 are the filemode.  The     */
/*               filename, filetype, and filemode may each be specified as *, in which case the   */
/*               first file satisfying the rest of the fileid is used.  This string does not need */
/*               to be NULL terminated.                                                           */
/*    fileinfo   is a handle to a copy of the CMS file state table (FST) for this file.  The FST  */
/*               provides details of the specified file.                                          */
/*                                                                                                */
/* Returns:                                                                                       */
/*     0     Success.                                                                             */
/*    20     The fileid is invalid.                                                               */
/*    24     Parameter list error (invalid character in fileid).                                  */
/*    28     The file was not found.                                                              */
/*    36     Disk not accessed.                                                                   */
/**************************************************************************************************/
int __fsstat(char * fileid, CMSFILEINFO **fileInfo);
#define CMSfileState(s1, s2) (__fsstat((s1),(s2)))

/**************************************************************************************************/
/* int CMSfileWrite(CMSFILE * file, int recordNum, int recordLen)                                 */
/*                                                                                                */
/* Writes one or more records to an open file.                                                    */
/*    file       is the file handle (FSCB) of the open file.                                      */
/*    recordNum  is the record number at which to begin writing.  To use the record number        */
/*               currently set in the CMSFILE structure (the FSCB), specify -1.  To write         */
/*               sequential records, from the start of the file specify 'recordNum' of 1 on the   */
/*               call to CMSfileOpen, then specify -1 for 'recordNum' on calls to CMSfileWrite.   */
/*               Alternatively, specify 1 on the first call to CMSfileWrite, and 0 on subsequent  */
/*               calls.                                                                           */
/*    recordLen  is the length of the record to be written (in 'buffer').                         */
/*                                                                                                */
/* Returns:                                                                                       */
/*     0     Success.                                                                             */
/*     2     Invalid buffer address.                                                              */
/*     4     First character of filemode is illegal.                                              */
/*     5     Second character of filemode is illegal.                                             */
/*     6     Record number too large.                                                             */
/*     7     Attempt to skip over unwritten variable-length record.                               */
/*     8     Buffer size not specified.                                                           */
/*     9     File open for input.                                                                 */
/*    10     Maximum number of files reached.                                                     */
/*    11     Record format not F or V.                                                            */
/*    12     Attempt to write on read-only disk.                                                  */
/*    13     Disk is full.                                                                        */
/*    14     Number of bytes written is not integrally divisible by the number of records to be   */
/*           written.                                                                             */
/*    15     Length of fixed-length recod not the same as previous record.                        */
/*    16     Record format specified is not the same as that of the file.                         */
/*    17     Variable-length record is greater than 65K bytes.                                    */
/*    18     Number of records to write is greater than 1 for a variable length file.             */
/*    19     Maximum number of data blocks per file (16060) has been reached.                     */
/*    20     Invalid character detected in filename.                                              */
/*    21     Invalid character detected in filetype.                                              */
/*    22     Virtual storage capacity exceeded.                                                   */
/*    25     Insufficent free storage available for file directory buffers.                       */
/*    27     Attempting to write a variable-length record of different length from the record     */
/*           being replaced.                                                                      */
/*                                                                                                */
/* Notes:                                                                                         */
/*    1. It is possible to overwrite a record within a file.  However, care must be taken that    */
/*       the record being written is the same length as the one it is replacing.  Failing to do   */
/*       this can result in corruption of the file and possibly the entire mini-disk.             */
/*    2. To overwrite an existing file with new records, call CMSfileErase to first erase the     */
/*       file, then open the file and begin writing from record 1.                                */
/**************************************************************************************************/
int __fswrit(CMSFILE * file, int recordNum, int recordLen);
#define CMSfileWrite(s1, i2, i3) (__fswrit((s1),(i2),(i3)))

/**************************************************************************************************/
/* void * CMSmemoryAlloc(int bytes, int type)                                                     */
/*                                                                                                */
/* Allocate free memory.                                                                          */
/*   bytes      is the number of bytes to be allocated.                                           */
/*   type       is the type of memory: CMS_NUCLEUS for NUCLEUS memory, CMS_USER for USER memory.  */
/*                                                                                                */
/* Returns:                                                                                       */
/*    the address of the allocated memory, or NULL if the memory could not be allocated.          */
/*                                                                                                */
/* Notes:                                                                                         */
/*    1.  If you allocate NUCLEUS memory, your program must be generated with the SYSTEM option.  */
/*        Note that if such a program abnormally terminates, CMS does not release this memory.    */
/**************************************************************************************************/
void * __dmsfre(int bytes, int type);
#define CMSmemoryAlloc(i1, i2) (__dmsfre((i1),(i2)))
#define CMS_NUCLEUS 0
#define CMS_USER 1

/**************************************************************************************************/
/* int CMSmemoryFree(void * memory)                                                               */
/*                                                                                                */
/* Release allocated memory.                                                                      */
/*   memory     is the address of the memory to be freed.                                         */
/*                                                                                                */
/* Returns:                                                                                       */
/*    the return code from DMSFRET.                                                               */
/*     0     Success.                                                                             */
/*     5     Returned size not positive (no doubt memory was overwritten).                        */
/*     6     Memory block overlaps other memory.                                                  */
/*     7     Returned block is not double-word aligned.                                           */
/*     8     Illegal code or argument.                                                            */
/*                                                                                                */
/* Notes:                                                                                         */
/*    1.  If you allocate NUCLEUS memory, your program must be generated with the SYSTEM option.  */
/*        Note that if such a program abnormally terminates, CMS does not release this memory.    */
/**************************************************************************************************/
int __dmsfrt(void * memory);
#define CMSmemoryFree(s1) (__dmsfrt((s1)))

/**************************************************************************************************/
/* int CMSprintLine(char * line)                                                                  */
/*                                                                                                */
/* Write a line to the virtual printer.                                                           */
/*    line   is a pointer to the line to be printeed.  The first character of the line must be a  */
/*           carriage control character (a blank skips to a new line).  The string must be NULL   */
/*           terminated.  The maximum length of the line is 133 characters for a virtual 1403     */
/*           printer (the default), 151 characters for a virtual 3211 printer; a line longer than */
/*           this results in an error.                                                            */
/*                                                                                                */
/* Returns:                                                                                       */
/*     0     Success.                                                                             */
/*     1     Line too long.                                                                       */
/*     2     Channel 12 punch sensed (virtual 3211 only).                                         */
/*     3     Channel 9 punch sensed (virtual 3211 only).                                          */
/*     4     Intervention required.                                                               */
/*     5     Unknown error.                                                                       */
/*   100     Printer not attached.                                                                */
/*                                                                                                */
/* Notes:                                                                                         */
/*    1.  Use the CP CLOSE command to close the virtual printer.  You can issue this via the      */
/*        CMScommand function.                                                                    */
/**************************************************************************************************/
int __printl(char * line);
#define CMSprintLine(s1) (__printl((s1)))

/**************************************************************************************************/
/* int CMSstackLine(char * line, int order)                                                       */
/*                                                                                                */
/* Place a line on the CMS console stack.                                                         */
/*    line   is a pointer to the line to be stacked.  In VM/370 the line may be no longer than    */
/*           130 characters; on VM/SP or z/VM it may be no longer than 256 characters.  If the    */
/*           string pointed to by 'line' is greater than this, it is truncated without warning.   */
/*    order  is 0 if the line is to be stacked 'LIFO', anything else for 'FIFO'.  The definitions */
/*           CMS_STACKLIFO and CMS_STACKFIFO may be used for this.                                */
/*                                                                                                */
/* Returns:                                                                                       */
/*    0                                                                                           */
/**************************************************************************************************/
int __attn(char * line, int order);
#define CMSstackLine(s1, i2) (__attn((s1),(i2)))
#define CMS_STACKLIFO 0
#define CMS_STACKFIFO 1

/**************************************************************************************************/
/* int CMSstackQuery(void)                                                                */
/*                                                                                                */
/* Return the number of lines currently on the console stack.                                     */
/*                                                                                                */
/* Returns:                                                                                       */
/*    the number of lines currently on the console stack.                                         */
/**************************************************************************************************/
int __stackn(void);
#define CMSstackQuery() (__stackn())

#endif
