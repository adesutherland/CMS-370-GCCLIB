/**************************************************************************************************/
/* STDIO.H                                                                                        */
/*                                                                                                */
/* Part of GCCLIB - VM/370 CMS Native Std C Library; A Historic Computing Toy                     */
/*                                                                                                */
/* Contributors: See contrib memo                                                                 */
/*                                                                                                */
/* Released to the public domain.                                                                 */
/**************************************************************************************************/

#ifndef STDIO_INCLUDED
#define STDIO_INCLUDED

#include <stdarg.h>
#include <string.h>

#ifndef __FPOS_T_DEFINED
#define __FPOS_T_DEFINED
/* File Pos */
struct FPOS_T {
  struct FILE *file;
  int recpos;
  int recnum;
};
typedef struct FPOS_T fpos_t;
#endif

#define EOF -1

/* setbuf / setvbuf defines */
#define _IOFBF 1
#define _IOLBF 2
#define _IONBF 3
#define BUFSIZ (16*1024)

#define L_tmpnam 21 /* tmpname() */

/**************************************************************************************************/
/* FILE - open stream file block                                                                  */
/**************************************************************************************************/
typedef struct FILE FILE;
/* FILE Structure Specified in cmssys.h */

/* Stdlib Public Global Variables */
#include <gcccrab.h>
#define stdin ((GETGCCCRAB()->gstdin))
#define stdout ((GETGCCCRAB()->gstdout))
#define stderr ((GETGCCCRAB()->gstderr))

void
clearerr(FILE * stream);
/**************************************************************************************************/
/* void clearerr(FILE * stream)                                                                   */
/*                                                                                                */
/* Resets the error status of the specified stream to 0.                                          */
/*    stream   a pointer to the open input stream.                                                */
/**************************************************************************************************/

int
fclose(FILE * stream);
/**************************************************************************************************/
/* int fclose(FILE * stream)                                                                      */
/*                                                                                                */
/* Close the open stream in an appropriate and orderly fashion.                                   */
/*    stream    is a pointer to the open output stream.                                           */
/*                                                                                                */
/* Returns:                                                                                       */
/*    0 if all is well, EOF if there is an error.                                                 */
/**************************************************************************************************/

int
feof(FILE * stream);
/**************************************************************************************************/
/* int feof(FILE * stream)                                                                        */
/*                                                                                                */
/* Report whether end of file has been reached for the specified input stream.                    */
/*    stream   a pointer to the open input stream.                                                */
/*                                                                                                */
/* Returns:                                                                                       */
/*    1 if EOF has been reached, 0 otherwise.                                                     */
/**************************************************************************************************/

int
ferror(FILE * stream);
/**************************************************************************************************/
/* int ferror(FILE * stream)                                                                      */
/*                                                                                                */
/* Returns the error status of the specified stream.                                              */
/*    stream   a pointer to the open input stream.                                                */
/*                                                                                                */
/* Returns:                                                                                       */
/*    the error status, or 0 if there is no error.                                                */
/**************************************************************************************************/

int
fflush(FILE * stream);
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

int
fgetc(FILE * stream);
/**************************************************************************************************/
/* int fgetc(FILE * stream)                                                                       */
/*                                                                                                */
/* Read the next character from the specified output stream.                                      */
/*    stream   a pointer to the open output stream.                                               */
/*                                                                                                */
/* Returns:                                                                                       */
/*    the character, or EOF if there is an error.                                                 */
/*                                                                                                */
/**************************************************************************************************/

int
fgetpos(FILE * stream, fpos_t *position);
/**************************************************************************************************/
/* int fgetpos(FILE * stream, fpos_t * position)                                                  */
/*                                                                                                */
/* Updates 'position' with the file position indicator, the next byte to be read or written in    */
/* the file.                                                                                      */
/*                                                                                                */
/* Returns:                                                                                       */
/*    0 on success, -1 on failure.                                                                */
/*                                                                                                */
/**************************************************************************************************/

char *
fgets(char * str, int num, FILE * stream);
/**************************************************************************************************/
/* char * fgets(char * str, int num, FILE * stream)                                               */
/*                                                                                                */
/* Read up to 'num' - 1 characters from the specified file stream and place them into 'str',      */
/* terminating them with a NULL.  fgets() stops when it reaches the end of a line, in which case  */
/* 'str' will contain a newline character.  Otherwise, fgets() will stop when it reaches 'num'-1  */
/* characters or encounters the EOF character.                                                    */
/*                                                                                                */
/* Returns:                                                                                       */
/*    the a pointer to 'str', or NULL on EOF or if there is an error.                             */
/*                                                                                                */
/* Notes:                                                                                         */
/*    1.  fgets is only appropriate for a text file.                                              */
/**************************************************************************************************/

FILE *
fopen(const char * filespec, const char * access);
/**************************************************************************************************/
/* FILE * fopen(const char * filespec, const char * access)                                       */
/*                                                                                                */
/* Open the specified file and return a stream associated with that file.                         */
/*    filespec is a pointer to a string containing the specification of the file to be opened:    */
/*       "CONSOLE"   terminal console (read or write)                                             */
/*       "PRINTER"   printer (write only)                                                         */
/*       "PUNCH"     card punch (write only)                                                      */
/*       "READER"    card reader (read only)                                                      */
/*       filename filetype [filemode [F|V] [recordLength]]                                        */
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

int
fprintf(FILE * stream, const char * format, ...);
/**************************************************************************************************/
/* int fprintf(FILE * stream, const char * format, ...)                                           */
/*                                                                                                */
/* Write formatted output to the specified output stream.                                         */
/*    stream   a pointer to the open output stream.                                               */
/*    format   a string containing characters to be printed and formatting specifications for the */
/*             the variables that follow, if any.                                                 */
/*    ...      0 or more variables that are converted to text and printed according to the        */
/*             formatting specifications in 'format'.                                             */
/*                                                                                                */
/* Returns:                                                                                       */
/*    the number of characters written to the stream.                                             */
/**************************************************************************************************/

int
fputc(int c, FILE * stream);
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

int
fputs(const char * str, FILE * stream);
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

int
fread(void * buffer, size_t size, size_t count, FILE * stream);
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

FILE *
freopen(const char * filespec, const char * access, FILE * stream);
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

int
fscanf(FILE * stream, const char * format, ...);
/**************************************************************************************************/
/* int fscanf(FILE * stream, const char * format, ...)                                            */
/*                                                                                                */
/* Read formatted input from the specified input stream and store the converted values in the     */
/* specified variables.                                                                           */
/*    stream   a pointer to the open output stream.                                               */
/*    format   a string containing the conversion specifications for the incoming characters to   */
/*             the variables that follow, if any.                                                 */
/*    ...      0 or more pointers to variables that receive the values converted from text        */
/*             according to the formatting specifications in 'format'.                            */
/*                                                                                                */
/* Returns:                                                                                       */
/*    the number of variables successfully assigned values.                                       */
/**************************************************************************************************/

int
fseek(FILE * stream, long offset, int origin);
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
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

int
fsetpos(FILE * stream, const fpos_t * position);
/**************************************************************************************************/
/* int fsetpos(FILE * stream, fpos_t * position)                                                  */
/*                                                                                                */
/* Moves the file position indicator for the specified stream to the specified position.  A       */
/* position of 0 moves the indicator to the beginning of the file.                                */
/*                                                                                                */
/* Returns:                                                                                       */
/*    0 on success, non-zero on failure.                                                          */
/*                                                                                                */
/**************************************************************************************************/

long
ftell(FILE * stream);
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

int
fwrite(const void * buffer, size_t size, size_t count, FILE * stream);
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

char *
gets(char * str);
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
/**************************************************************************************************/

/**************************************************************************************************/
/* void perror( const char *str )                                                                 */
/**************************************************************************************************/
void perror( const char *str );


int
printf(const char * format, ...);
/**************************************************************************************************/
/* int printf(const char * format, ...)                                                           */
/*                                                                                                */
/* Write formatted output on the CMS console.  Note that this native implementation does not      */
/* support redirection to other files or devices.                                                 */
/*    format   a string containing characters to be printed and formatting specifications for the */
/*             the variables that follow, if any.                                                 */
/*    ...      0 or more variables that are converted to text and printed according to the        */
/*             formatting specifications in 'format'.                                             */
/*                                                                                                */
/* Returns:                                                                                       */
/*    the number of characters printed.                                                           */
/**************************************************************************************************/

int
puts(char * str);
/**************************************************************************************************/
/* int fputs(const char * str)                                                                    */
/*                                                                                                */
/* write a string to the console.                                                                 */
/*    str      the string to be written.                                                          */
/*                                                                                                */
/* Returns:                                                                                       */
/*    non-negative on success, EOF on failure.                                                    */
/**************************************************************************************************/

int
remove(const char * fname);
/**************************************************************************************************/
/* int remove(const char * fname)                                                                 */
/*                                                                                                */
/* Erases the specified file.                                                                     */
/*    fname    a string specifying the file to be erased.  It is comprised of three words,        */
/*             separated by 1 or more blanks:                                                     */
/*                filename is the up to 8 character name.                                         */
/*                filetype is the up to 8 character type.                                         */
/*                filemode is the up to 2 character disk mode leter and optional number.          */
/*                                                                                                */
/* Returns:                                                                                       */
/*    0 if the file was erased, otherwise the return code from fserase.                           */
/**************************************************************************************************/

int
rename(const char * oldfname, const char * newfname);
/**************************************************************************************************/
/* int rename(const char * oldfname, const char * newfname)                                       */
/*                                                                                                */
/* Renames file 'oldfname' to 'newfname'.  'oldfname' and 'newfname' are strings specifying the   */
/* old and new name for the file.  They are comprised of three words, separated by 1 or more      */
/* blanks:                                                                                        */
/*    filename is the up to 8 character name.                                                     */
/*    filetype is the up to 8 character type.                                                     */
/*    filemode is the up to 2 character disk mode leter and optional number.                      */
/*                                                                                                */
/* Returns:                                                                                       */
/*    0 if the file was renamed, otherwise the return code from fsrename.                         */
/**************************************************************************************************/

void
rewind(FILE * stream);
/**************************************************************************************************/
/* void rewind(FILE * stream)                                                                     */
/*                                                                                                */
/* Move the file position indicator to the beginning of the specified stream, and clears the      */
/* error and EOF flags associated with that stream.                                               */
/*    stream   a pointer to the open stream.                                                      */
/**************************************************************************************************/

int
scanf(const char * format, ...);
/**************************************************************************************************/
/* int scanf(const char * format, ...)                                                            */
/*                                                                                                */
/* Read formatted input from the console and store the converted values in the specified          */
/* variables.                                                                                     */
/*    format   a string containing the conversion specifications for the incoming characters to   */
/*             the variables that follow, if any.                                                 */
/*    ...      0 or more pointers to variables that receive the values converted from text        */
/*             according to the formatting specifications in 'format'.                            */
/*                                                                                                */
/* Returns:                                                                                       */
/*    the number of variables successfully assigned values.                                       */
/**************************************************************************************************/

void
setbuf(FILE * stream, char * buffer);
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

int
setvbuf(FILE * stream, char * buffer, int mode, size_t size);
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

/**************************************************************************************************/
/* int snprintf( char *buffer, int buff_size, const char *format, ... )                           */
/* ...unimplemented                                                                               */
/**************************************************************************************************/

int
sprintf(char * buffer, const char * format, ...);
/**************************************************************************************************/
/* int sprintf(char * buffer, const char *format, ...)                                            */
/*                                                                                                */
/* Write formatted output to a string.                                                            */
/*    buffer   a pointer to a buffer into which the formatted output is copied.                   */
/*    format   a string containing characters to be printed and formatting specifications for the */
/*             the variables that follow, if any.                                                 */
/*    ...      0 or more variables that are converted to text and printed according to the        */
/*             formatting specifications in 'format'.                                             */
/*                                                                                                */
/* Returns:                                                                                       */
/*    the number of characters written to the string.                                             */
/**************************************************************************************************/

int
sscanf(const char * buffer, const char * format, ...);
/**************************************************************************************************/
/* int sscanf(const char * buffer, const char * format, ...)                                      */
/*                                                                                                */
/* Read formatted input from a buffer and store the converted values in the specified variables.  */
/*    buffer   a string containing the text to be converted to values.                            */
/*    format   a string containing the conversion specifications for the incoming characters to   */
/*             the variables that follow, if any.                                                 */
/*    ...      0 or more pointers to variables that receive the values converted from text        */
/*             according to the formatting specifications in 'format'.                            */
/*                                                                                                */
/* Returns:                                                                                       */
/*    c if successful, EOF if not.                                                                */
/**************************************************************************************************/

FILE *
tmpfile(void);
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

char *
tmpnam(char * name);
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

int
ungetc(int c, FILE * stream);
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

int
vfprintf(FILE * stream, const char * format, va_list arg);
/**************************************************************************************************/
/* int vfprintf(FILE * stream, const char * format, va_list arg)                                  */
/*                                                                                                */
/* Write formatted output to the specified output stream.                                         */
/*    stream   a pointer to the open output stream.                                               */
/*    format   a string containing characters to be printed and formatting specifications for the */
/*             the variables that follow, if any.                                                 */
/*    arg      the argument list.                                                                 */
/*                                                                                                */
/* Returns:                                                                                       */
/*    the number of characters written to the stream.                                             */
/**************************************************************************************************/

int
vsprintf(char * buffer, const char * format, va_list arg);
/**************************************************************************************************/
/* int vsprintf(char * buffer, const char * format, va_list arg)                                  */
/*                                                                                                */
/* Write formatted output to a string using an argument list.                                     */
/*    buffer   a pointer to a buffer into which the formatted output is copied.                   */
/*    format   a string containing characters to be printed and formatting specifications for the */
/*             the variables that follow, if any.                                                 */
/*    arg      the argument list.                                                                 */
/*                                                                                                */
/* Returns:                                                                                       */
/*    the number of characters printed.                                                           */
/**************************************************************************************************/

/**************************************************************************************************/
/* int fcachehits(FILE * stream)                                                                  */
/*                                                                                                */
/* The function returns the precentage of cache read hits achieved                                */
/* Non-standard GCC CMS extention                                                                 */
/*                                                                                                */
/* Returns:                                                                                       */
/*    Percentage of cache hits or -1 on error (device without a cache)                            */
/**************************************************************************************************/
int fcachehits(FILE * file);

/**************************************************************************************************/
/* int fgetrec(FILE * stream)                                                  */
/*                                                                                                */
/* The function returns the current record number (1 base)                                        */
/* Non-standard GCC CMS extention                                                                 */
/*                                                                                                */
/* Returns:                                                                                       */
/*    record number or 0 on failure / not block device                                            */
/**************************************************************************************************/
int fgetrec(FILE * file);

/**************************************************************************************************/
/* int fsetrec(FILE * stream, int recnum)                                                         */
/*                                                                                                */
/* The function sets/moves to the record number (1 base)                                          */
/* Non-standard GCC CMS extention                                                                 */
/*                                                                                                */
/* Returns:                                                                                       */
/*    record number or 0 on failure / not block device                                            */
/**************************************************************************************************/
int fsetrec(FILE * file, const int recnum);

/**************************************************************************************************/
/* int getc(FILE * stream)                                                                        */
/*                                                                                                */
/* Read the next character from the specified output stream.  This is identical to fgetc().       */
/*    stream   a pointer to the open output stream.                                               */
/*                                                                                                */
/* Returns:                                                                                       */
/*    the character, or EOF if there is an error.                                                 */
/*                                                                                                */
/* Notes:                                                                                         */
/*    1.  A newline character is added at the end of each physical line read when reading TEXT    */
/*        files.                                                                                  */
/**************************************************************************************************/
#define getc(s) fgetc((c))

/**************************************************************************************************/
/* int getchar(void)                                                                              */
/*                                                                                                */
/* Read the next character from the console.                                                      */
/*                                                                                                */
/* Returns:                                                                                       */
/*    the character, or EOF if there is an error.                                                 */
/*                                                                                                */
/* Notes:                                                                                         */
/*    1.  A newline character is added at the end of each physical line read when reading TEXT    */
/*        files.                                                                                  */
/**************************************************************************************************/
#define getchar() fgetc(stdin)

/**************************************************************************************************/
/* int putc(int c, FILE * stream)                                                                 */
/*                                                                                                */
/* Write a character to the specified output stream.  This is the same as fputc().                */
/*    c        the character to be written.                                                       */
/*    stream   a pointer to the open output stream.                                               */
/*                                                                                                */
/* Returns:                                                                                       */
/*    the character, or EOF if there is an error.                                                 */
/**************************************************************************************************/
#define putc(c,s) fputc((c),(s))

/**************************************************************************************************/
/* int putchar(int c)                                                                             */
/*                                                                                                */
/* Write a character to the console.                                                              */
/*    c        the character to be written.                                                       */
/*                                                                                                */
/* Returns:                                                                                       */
/*    the character, or EOF if there is an error.                                                 */
/**************************************************************************************************/
#define putchar(c) fputc((c),stdout)

/**************************************************************************************************/
/* int fateof(FILE * stream)                                                                      */
/* Non-standard GCC CMS extention                                                                 */
/* Detects if at EOF - unlike feof() it predicts if the next read will cause an EOF               */
/*                                                                                                */
/* Returns 1 if at EOF, EOF on error, or 0 is not at EOF                                          */
/*                                                                                                */
/**************************************************************************************************/
int fateof(FILE * stream);

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
FILE* fgethandle(char *fileName);

/**************************************************************************************************/
/* int fgetrecs(FILE * stream)                                                                    */
/* Non-standard GCC CMS extention                                                                 */
/*                                                                                                */
/* Gets the number of records in a file                                                           */
/*                                                                                                */
/* Returns the number of records or EOF on error                                                  */
/*                                                                                                */
/**************************************************************************************************/
int fgetrecs(FILE * stream);

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
int fgetlen(FILE * stream);

/**************************************************************************************************/
/* void append(FILE * stream)                                                                     */
/*                                                                                                */
/* Non-standard GCC CMS extention                                                                 */
/* Move the file position indicator to the end of the specified stream, and clears the            */
/* error and EOF flags associated with that stream. This is the oposite of rewind()               */
/*    stream   a pointer to the open stream.                                                      */
/**************************************************************************************************/
void append(FILE * file);

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
int nextrecLen(FILE * file);


#endif
