/**************************************************************************************************/
/* GCCLIB Test Suite - CONSOLE IO                                                                 */
/*                                                                                                */
/* Part of GCCLIB - VM/370 CMS Native Std C Library; A Historic Computing Toy                     */
/*                                                                                                */
/* Contributors: See contrib memo                                                                 */
/*                                                                                                */
/* Released to the public domain.                                                                 */
/**************************************************************************************************/
#define __FILENAME__ "TSTCONIO"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmssys.h>
#include <errno.h>

#include "tsts.h"


/**************************************************************************************************/
/* FILE * fopen(const char * filespec, const char * access)                                       */
/*       "CONSOLE"   terminal console (read or write)                                             */
/*    access   specifies how the file will be accessed (i.e. for input, output, etc):             */
/*              r|w|a[+][b]                                                                       */
/* Returns:                                                                                       */
/*    a pointer to the stream associated with the open file, or NULL on failure.                  */
/**************************************************************************************************/
static void fopen_t() {
  FILE* f;
  SUB_STRT("fopen()");
  ASSERTNOTNULL("fopen(CONSOLE,r)", f=fopen("CONSOLE","r"), f, if (f) fclose(f));
  ASSERTNOTNULL("fopen(CONSOLE,w)", f=fopen("CONSOLE","w"), f, if (f) fclose(f));
  ASSERTNOTNULL("fopen(CONSOLE,a)", f=fopen("CONSOLE","a"), f, if (f) fclose(f));
  ASSERTNOTNULL("fopen(CONSOLE,rb)", f=fopen("CONSOLE","rb"), f, if (f) fclose(f));
  ASSERTNOTNULL("fopen(CONSOLE,wb)", f=fopen("CONSOLE","wb"), f, if (f) fclose(f));
  ASSERTNOTNULL("fopen(CONSOLE,ab)", f=fopen("CONSOLE","ab"), f, if (f) fclose(f));
  ASSERTNULL("fopen(CONSOLE,r+)", f=fopen("CONSOLE","r+"), f, if (f) fclose(f));
  ASSERTNULL("fopen(CONSOLE,w+)", f=fopen("CONSOLE","w+"), f, if (f) fclose(f));
  ASSERTNULL("fopen(CONSOLE,a+)", f=fopen("CONSOLE","a+"), f, if (f) fclose(f));
  ASSERTNULL("fopen(CONSOLE,rb+)", f=fopen("CONSOLE","rb+"), f, if (f) fclose(f));
  ASSERTNULL("fopen(CONSOLE,wb+)", f=fopen("CONSOLE","wb+"), f, if (f) fclose(f));
  ASSERTNULL("fopen(CONSOLE,ab+)", f=fopen("CONSOLE","ab+"), f, if (f) fclose(f));
  ASSERTNULL("fopen(CONSOLE,r+b)", f=fopen("CONSOLE","r+b"), f, if (f) fclose(f));
  ASSERTNULL("fopen(CONSOLE,w+b)", f=fopen("CONSOLE","w+b"), f, if (f) fclose(f));
  ASSERTNULL("fopen(CONSOLE,a+b)", f=fopen("CONSOLE","a+b"), f, if (f) fclose(f));
  ASSERTNULL("fopen(CONSOLE,x)", f=fopen("CONSOLE","x"), f, if (f) fclose(f));
  ASSERTNULL("fopen(CONSOLE,+)", f=fopen("CONSOLE","+"), f, if (f) fclose(f));
  ASSERTNULL("fopen(CONSOLE,b)", f=fopen("CONSOLE","b"), f, if (f) fclose(f));
  ASSERTNULL("fopen(CONSOLE,)", f=fopen("CONSOLE",""), f, if (f) fclose(f));
  ASSERTNULL("fopen(CONSOLE,r+bx)", f=fopen("CONSOLE","r+bx"), f, if (f) fclose(f));
}

/**************************************************************************************************/
/* int fclose(FILE * file)                                                                        */
/* Returns:                                                                                       */
/*    0 if all is well, EOF if there is an error.                                                 */
/**************************************************************************************************/
static void fclose_t() {
  FILE* f;
  SUB_STRT("fclose()");
  ASSERTNOTNULL("fopen(CONSOLE,w)", f=fopen("CONSOLE","w"), f, );
  ASSERTNOTEOF("fclose(f)", ,fclose(f), );
  ASSERTEOF("fputs(SHOULD NOT SEE,f)", ,fputs("SHOULD NOT SEE\n",f), );
  ASSERTEOF("fclose(f)", ,fclose(f), );
}

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
static void fflush_t() {
  FILE* test;
  FILE* reader;
  char buf[200];
  int rc;

  SUB_STRT("fflush()");
  ASSERTNOTNULLP("fopen(CONSOLE,w)", test=fopen("CONSOLE","w"), test, );
  ASSERTNOTEOFP("fputc()", ,FPUTC_WR(TEST_SHORT_STRING,test), );
  system("CP SPOOL CONS * START");
  rc = fflush(test);
  system("CP SPOOL CONS STOP CLOSE");
  ASSERTZEROP("fflush()", , rc, );
  ASSERTZEROP("fclose(console)", , fclose(test), );
  ASSERTNOTNULLP("fopen(READER132,r)", reader = fopen("READER132","r"), reader, );
  ASSERTNOTNULLP("fgets(reader)", , fgets(buf, 200, reader), );
  ASSERTZERO("strcmp()", , strcmp(buf, TEST_SHORT_STRING_NL), );
  ASSERTNULL("fgets(reader)", , fgets(buf, 200, reader), );
  ASSERTNOTZERO("feof(reader)", ,feof(reader), );
  ASSERTZEROP("fclose(reader)", , fclose(reader), );
}

/**************************************************************************************************/
/* int ferror(FILE * stream)                                                                      */
/*                                                                                                */
/* Returns the error status of the specified stream.                                              */
/*    stream   a pointer to the open input stream.                                                */
/*                                                                                                */
/* Returns:                                                                                       */
/*    the error status, or 0 if there is no error.                                                */
/**************************************************************************************************/
static void ferror_t() {
  FILE* test;

  SUB_STRT("ferror()");
  ASSERTNOTNULLP("fopen(CONSOLE,w)", test=fopen("CONSOLE","w"), test, );
  ASSERTEOF("fgetc()", , fgetc(test), );
  ASSERTNOTZERO("ferror()", , ferror(test), );
  ASSERTZEROP("fclose(console)", , fclose(test), );
}

/**************************************************************************************************/
/* void clearerr(FILE * stream)                                                                   */
/*                                                                                                */
/* Resets the error status of the specified stream to 0.                                          */
/*    stream   a pointer to the open input stream.                                                */
/**************************************************************************************************/
static void clearerr_t() {
  FILE* test;

  SUB_STRT("clearerr()");
  ASSERTNOTNULLP("fopen(CONSOLE,w)", test=fopen("CONSOLE","w"), test, );
  ASSERTEOF("fgetc()", , fgetc(test), );
  clearerr(test);
  ASSERTZERO("ferror()", , ferror(test), );
  ASSERTZEROP("fclose(console)", , fclose(test), );
}

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
static void fgetc_t() {
  FILE* test;
  char buf[300];
  char buf2[300];

  SUB_STRT("fgetc() - text");
  ASSERTNOTNULLP("fopen(CONSOLE,r)", test=fopen("CONSOLE","r"), test, );
  CMSstackLine(TEST_STRING, CMS_STACKLIFO);
  ASSERTNOTNULL("FGETC_RD()", , FGETC_RD(buf, 200, test), );
  ASSERTZERO("strcmp()", , strcmp(buf,TEST_STRING_NL), );
  ASSERTZEROP("fclose()", , fclose(test), );

  SUB_STRT("fgetc() - binary");
  ASSERTNOTNULLP("fopen(CONSOLE,rb)", test=fopen("CONSOLE","rb"), test, );
  CMSstackLine(TEST_STRING, CMS_STACKLIFO);
  CMSstackLine(TEST_STRING, CMS_STACKLIFO);
  ASSERTNOTNULL("FGETC_RD()", , FGETC_RD(buf, strlen(TEST_STRING)*2, test), );
  strcpy(buf2,TEST_STRING);
  strcat(buf2,TEST_STRING);
  ASSERTZERO("strcmp()", , strcmp(buf,buf2), );
  ASSERTZEROP("fclose()", , fclose(test), );
}

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
static void fgets_t() {
  FILE* test;
  char buf[300];

  SUB_STRT("fgets() - text");
  ASSERTNOTNULLP("fopen(CONSOLE,r)", test=fopen("CONSOLE","r"), test, );
  CMSstackLine(TEST_STRING, CMS_STACKLIFO);
  ASSERTNOTNULL("fgets()", , fgets(buf, 200, test), );
  ASSERTZERO("strcmp()", , strcmp(buf,TEST_STRING_NL), );
  ASSERTZEROP("fclose()", , fclose(test), );

  SUB_STRT("fgets() - binary");
  ASSERTNOTNULLP("fopen(CONSOLE,rb)", test=fopen("CONSOLE","rb"), test, );
  CMSstackLine(TEST_STRING, CMS_STACKLIFO);
  ASSERTNOTNULL("fgets()", , fgets(buf, strlen(TEST_STRING) + 1, test), );
  ASSERTZERO("strcmp()", , strcmp(buf,TEST_STRING), );
  ASSERTZEROP("fclose()", , fclose(test), );
}

/**************************************************************************************************/
/* int feof(FILE * stream)                                                                        */
/*                                                                                                */
/* Report whether end of file has been reached for the specified input stream.                    */
/*    stream   a pointer to the open input stream.                                                */
/*                                                                                                */
/* Returns:                                                                                       */
/*    1 if EOF has been reached, 0 otherwise.                                                     */
/**************************************************************************************************/
static void feof_t() {
  FILE* test;

  SUB_STRT("feof()");
  ASSERTNOTNULLP("fopen(CONSOLE,w)", test=fopen("CONSOLE","w"), test, );
  ASSERTZERO("feof()", , feof(test), );
  ASSERTZEROP("fclose()", , fclose(test), );
  ASSERTNOTNULLP("fopen(CONSOLE,r)", test=fopen("CONSOLE","r"), test, );
  ASSERTZERO("feof()", , feof(test), );
  ASSERTZEROP("fclose()", , fclose(test), );
}

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
static void ungetc_t() {
  FILE* test;
  char buf[200];
  char buf2[200];

  SUB_STRT("ungetc() - opened for writing");
  ASSERTNOTNULLP("fopen(CONSOLE,w)", test=fopen("CONSOLE","w"), test, );
  ASSERTEOF("ungetc(y)", , ungetc('y',test), );
  ASSERTZEROP("fclose()", , fclose(test), );
  SUB_STRT("ungetc() - text");
  ASSERTNOTNULLP("fopen(CONSOLE,r)", test=fopen("CONSOLE","r"), test, );
  CMSstackLine(TEST_SHORT_STRING, CMS_STACKLIFO);
  ASSERTNOTEOF("ungetc(x)", , ungetc('x',test), );
  ASSERTEOF("ungetc(y)", , ungetc('y',test), );
  ASSERTNOTNULL("fgets()", , fgets(buf, 200, test), );
  ASSERTNOTEOF("ungetc(z)", , ungetc('z',test), );
  buf2[0] = 'x'; buf2[1]=0; strcat(buf2,TEST_SHORT_STRING_NL);
  ASSERTZERO("strcmp()", , strcmp(buf,buf2), );
  ASSERTZEROP("fclose()", , fclose(test), );
  SUB_STRT("ungetc() - binary");
  ASSERTNOTNULLP("fopen(CONSOLE,rb)", test=fopen("CONSOLE","rb"), test, );
  CMSstackLine(TEST_SHORT_STRING, CMS_STACKLIFO);
  ASSERTNOTEOF("ungetc(x)", , ungetc('x',test), );
  ASSERTEOF("ungetc(y)", , ungetc('y',test), );
  ASSERTNOTNULL("fgets()", , fgets(buf, strlen(TEST_SHORT_STRING) + 2, test), );
  ASSERTNOTEOF("ungetc(z)", , ungetc('z',test), );
  buf2[0] = 'x'; buf2[1]=0; strcat(buf2,TEST_SHORT_STRING);
  ASSERTZERO("strcmp()", , strcmp(buf,buf2), );
  ASSERTZEROP("fclose()", , fclose(test), );
}

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
static void fputc_t() {
  FILE* test;
  char buf[201];
  char buf2[201];
  char buf3[201];
  int i;

  SUB_STRT("fputc() - text");
  ASSERTNOTNULLP("fopen(CONSOLE,w)", test=fopen("CONSOLE","w"), test, );
  system("CP SPOOL CONS * START");
  FPUTC_T(test);
  fclose(test);
  system("CP SPOOL CONS STOP CLOSE");
  ASSERTNOTNULLP("fopen(READER132,r)", test=fopen("READER132","r"), test, );
  FGETS_T(test,130);
  ASSERTZEROP("fclose()", , fclose(test), );

  SUB_STRT("fputc() - binary");
  ASSERTNOTNULLP("fopen(CONSOLE,wb)", test=fopen("CONSOLE","wb"), test, );
  system("CP SPOOL CONS * START");
  buf2[0]=0;
  for (i=0; i<20; i++) {
    FPUTC_WR("123456789+",test);
    strcat(buf2,"123456789+");
  }
  fclose(test);
  system("CP SPOOL CONS STOP CLOSE");
  ASSERTNOTNULLP("fopen(READER132,rb)", test=fopen("READER132","rb"), test, );
  ASSERTNOTNULL("fgets()", , fgets(buf, 201, test), );
  buf[130] = 0; /* Console -> Reader Spool adds 2 blanks (nothing to do with GCCLIB!) */
  ASSERTNOTNULL("fgets()", , fgets(buf3, strlen(buf2)-130+1, test), );
  strcat(buf,buf3);
  ASSERTZERO("strcmp()", , strcmp(buf,buf2), );
  ASSERTZEROP("fclose()", , fclose(test), );
}

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
static void fputs_t() {
  FILE* test;
  char buf[201];
  char buf2[201];
  char buf3[201];
  int i;

  SUB_STRT("fputs() - text");
  ASSERTNOTNULLP("fopen(CONSOLE,w)", test=fopen("CONSOLE","w"), test, );
  system("CP SPOOL CONS * START");
  FPUTS_T(test);
  fclose(test);
  system("CP SPOOL CONS STOP CLOSE");
  ASSERTNOTNULLP("fopen(READER132,r)", test=fopen("READER132","r"), test, );
  FGETS_T(test,130);
  ASSERTZEROP("fclose()", , fclose(test), );

  SUB_STRT("fputs() - binary");
  ASSERTNOTNULLP("fopen(CONSOLE,wb)", test=fopen("CONSOLE","wb"), test, );
  system("CP SPOOL CONS * START");
  buf2[0]=0;
  for (i=0; i<20; i++) {
    fputs("123456789+",test);
    strcat(buf2,"123456789+");
  }
  fclose(test);
  system("CP SPOOL CONS STOP CLOSE");
  ASSERTNOTNULLP("fopen(READER132,rb)", test=fopen("READER132","rb"), test, );
  ASSERTNOTNULL("fgets()", , fgets(buf, 201, test), );
  buf[130] = 0; /* Console -> Reader Spool adds 2 blanks (nothing to do with GCCLIB!) */
  ASSERTNOTNULL("fgets()", , fgets(buf3, strlen(buf2)-130+1, test), );
  strcat(buf,buf3);
  ASSERTZERO("strcmp()", , strcmp(buf,buf2), );
  ASSERTZEROP("fclose()", , fclose(test), );
}

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
/*    2.  Depricated in later standards - unsafe                                                  */
/**************************************************************************************************/
static void gets_t() {
  char buf[300];
  void* saved_stdin;

  SUB_STRT("gets()");
  saved_stdin = stdin;
  ASSERTNOTNULLP("fopen(CONSOLE,r)", stdin=fopen("CONSOLE","r"), stdin, );
  CMSstackLine(TEST_STRING, CMS_STACKLIFO);
  ASSERTNOTNULL("gets()", , gets(buf), );
  ASSERTZEROP("fclose()", , fclose(stdin), );
  stdin = saved_stdin;
  ASSERTZERO("strcmp()", , strcmp(buf,TEST_STRING), );
}

/**************************************************************************************************/
/* int puts(const char * str)                                                                    */
/*                                                                                                */
/* Writes the C string pointed by str to the standard output (stdout)                             */
/*  *** and appends a newline character ('\n') ***                                                */
/*                                                                                                */
/* Returns:                                                                                       */
/*    non-negative on success, EOF on failure.                                                    */
/**************************************************************************************************/
static void puts_t() {
  FILE* test;
  char buf[201];
  int i;
  void* saved_stdout;

  SUB_STRT("puts()");
  saved_stdout = stdout;
  ASSERTNOTNULLP("fopen(CONSOLE,w)", stdout=fopen("CONSOLE","w"), stdout, );
  system("CP SPOOL CONS * START");
  i = puts("123456789+");
  system("CP SPOOL CONS STOP CLOSE");
  fclose(stdout);
  stdout = saved_stdout;
  ASSERTNOTEOF("puts()", , i, );
  ASSERTNOTNULLP("fopen(READER132,r)", test=fopen("READER132","r"), test, );
  ASSERTNOTNULL("fgets()", , fgets(buf, 201, test), );
  ASSERTZERO("strcmp()", , strcmp(buf,"123456789+\n"), );
  ASSERTZEROP("fclose()", , fclose(test), );
}

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
/*                                                                                                */
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
static void fwriteread_t() {
  FILE* test;
  const char *item = "1234567890abcdefghijklmnopqrstuvwxyz";
  const int count = 2; /* Need to avoid line breaks as console to reader adds 2 blanks for each record */
  char* buffer;
  char* buffer2;
  int i;

  SUB_STRT("fwrite() and fread()");
  buffer =  malloc((count * strlen(item)) + 1);
  buffer2 = malloc((count * strlen(item)) + 1);
  buffer[0] = 0;
  buffer2[0] = 0;
  buffer2[(count * strlen(item)) + 1] = 0;

  for (i=0; i<count; i++) strcat(buffer,item);

  ASSERTNOTNULLP("fopen(CONSOLE,wb)", test=fopen("CONSOLE","wb"), test, );
  system("CP SPOOL CONS * START");
  i = fwrite(buffer, strlen(item), count, test);
  fflush(test);
  system("CP SPOOL CONS STOP CLOSE");
  ASSERTNOTZERO("fwrite()", ,i == count, );
  ASSERTZEROP("fclose()", , fclose(test), );
  ASSERTNOTNULLP("fopen(READER132,rb)", test=fopen("READER132","rb"), test, );
  ASSERTNOTZERO("fread()", ,fread(buffer2, strlen(item), count, test)==count, );
  ASSERTZERO("strcmp()", , strcmp(buffer,buffer2), );
  ASSERTZEROP("fclose()", , fclose(test), );
  CMSstackLine(item, CMS_STACKLIFO);
  CMSstackLine(item, CMS_STACKLIFO);
  ASSERTNOTNULLP("fopen(CONSOLE,rb)", test=fopen("CONSOLE","rb"), test, );
  ASSERTNOTZERO("fread()", ,fread(buffer2, strlen(item), count, test)==count, );
  ASSERTZERO("strcmp()", , strcmp(buffer,buffer2), );
  ASSERTZEROP("fclose()", , fclose(test), );


  ASSERTNOTNULLP("fopen(CONSOLE,w)", test=fopen("CONSOLE","w"), test, );
  system("CP SPOOL CONS * START");
  i = fwrite(buffer, strlen(item), count, test);
  fflush(test);
  system("CP SPOOL CONS STOP CLOSE");
  ASSERTNOTZERO("fwrite()", ,i == count, );
  ASSERTZEROP("fclose()", , fclose(test), );
  ASSERTNOTNULLP("fopen(READER132,r)", test=fopen("READER132","r"), test, );
  ASSERTNOTZERO("fread()", ,fread(buffer2, strlen(item), count, test)==count, );
  ASSERTZERO("strcmp()", , strcmp(buffer,buffer2), );
  ASSERTZEROP("fclose()", , fclose(test), );
  CMSstackLine(item, CMS_STACKLIFO);
  ASSERTNOTNULLP("fopen(CONSOLE,r)", test=fopen("CONSOLE","r"), test, );
  buffer2[strlen(item)] = 0;
  ASSERTNOTZERO("fread()", ,fread(buffer2, strlen(item), 1, test)==1, );
  ASSERTZERO("strcmp()", , strcmp(item,buffer2), );
  ASSERTZEROP("fclose()", , fclose(test), );

  free(buffer);
  free(buffer2);
}

/**************************************************************************************************/
/* long ftell(FILE * stream)                                                                      */
/*                                                                                                */
/* Return the current file position for the specified stream.                                     */
/*    stream   a pointer to the open stream.                                                      */
/*                                                                                                */
/* Returns:                                                                                       */
/*    the position of the next character to be read or written, or -1 if there is an error.       */
/*                                                                                                */
/* Notes:                                                                                         */
/*    1.  ftell is only valid for a disk file that is open for reading.                           */
/*    2.  Functioally, ftell is equivalent to fgetpos.                                            */
/**************************************************************************************************/
static void ftell_t() {
  FILE* test;

  SUB_STRT("ftell() - write");
  ASSERTNOTNULLP("fopen(CONSOLE,w)", test=fopen("CONSOLE","w"), test, );
  ASSERTNOTZERO("ftell()", , ftell(test)==-1, );
  ASSERTZEROP("fclose()", , fclose(test), );
  SUB_STRT("ftell() - read");
  ASSERTNOTNULLP("fopen(CONSOLE,r)", test=fopen("CONSOLE","r"), test, );
  ASSERTNOTZERO("ftell()", , ftell(test)==-1, );
  ASSERTZEROP("fclose()", , fclose(test), );
}

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
static void fseek_t() {
  FILE* test;

  SUB_STRT("fseek() - write");
  ASSERTNOTNULLP("fopen(CONSOLE,w)", test=fopen("CONSOLE","w"), test, );
  ASSERTNOTZERO("fseek(SEEK_SET)", , fseek(test, 1, SEEK_SET)!=0, );
  ASSERTNOTZERO("fseek(SEEK_CUR)", , fseek(test, 1, SEEK_CUR)!=0, );
  ASSERTNOTZERO("fseek(SEEK_END)", , fseek(test, 1, SEEK_END)!=0, );
  ASSERTZEROP("fclose()", , fclose(test), );
  SUB_STRT("fseek() - read");
  ASSERTNOTNULLP("fopen(CONSOLE,r)", test=fopen("CONSOLE","r"), test, );
  ASSERTNOTZERO("fseek(SEEK_SET)", , fseek(test, 1, SEEK_SET)!=0, );
  ASSERTNOTZERO("fseek(SEEK_CUR)", , fseek(test, 1, SEEK_CUR)!=0, );
  ASSERTNOTZERO("fseek(SEEK_END)", , fseek(test, 1, SEEK_END)!=0, );
  ASSERTZEROP("fclose()", , fclose(test), );
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
/*                                                                                                */
/*                                                                                                */
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
static void fgetsetpos_t() {
  FILE* test;
  fpos_t position;

  SUB_STRT("fgetpos() and fsetpos() - write");
  ASSERTNOTNULLP("fopen(CONSOLE,w)", test=fopen("CONSOLE","w"), test, );
  ASSERTNOTZERO("fgetpos()", , fgetpos(test, &position) != 0, );
  ASSERTNOTZERO("fsetpos()", , fsetpos(test, &position) != 0, );
  ASSERTZEROP("fclose()", , fclose(test), );
  SUB_STRT("fgetpos() and fsetpos() - read");
  ASSERTNOTNULLP("fopen(CONSOLE,r)", test=fopen("CONSOLE","r"), test, );
  ASSERTNOTZERO("fgetpos()", , fgetpos(test, &position) != 0, );
  ASSERTNOTZERO("fsetpos()", , fsetpos(test, &position) != 0, );
  ASSERTZEROP("fclose()", , fclose(test), );
}

/**************************************************************************************************/
/* void rewind(FILE * stream)                                                                     */
/*                                                                                                */
/* Move the file position indicator to the beginning of the specified stream, and clears the      */
/* error and EOF flags associated with that stream.                                               */
/*    stream   a pointer to the open stream.                                                      */
/**************************************************************************************************/
static void rewind_t() {
  FILE* test;

  SUB_STRT("rewind() - write");
  ASSERTNOTNULLP("fopen(CONSOLE,w)", test=fopen("CONSOLE","w"), test, );
  ASSERTNOTZERO("rewind()",rewind(test) , 1, );
  ASSERTZEROP("fclose()", , fclose(test), );
  SUB_STRT("rewind() - read");
  ASSERTNOTNULLP("fopen(CONSOLE,r)", test=fopen("CONSOLE","r"), test, );
  ASSERTNOTZERO("rewind()",rewind(test) , 1, );
  ASSERTZEROP("fclose()", , fclose(test), );
}

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
static void setbuf_t() {
  FILE* test;
  char* buffer;

  SUB_STRT("setbuf() - write");
  buffer = malloc(BUFSIZ);
  ASSERTNOTNULLP("fopen(CONSOLE,w)", test=fopen("CONSOLE","w"), test, );
  setbuf(test, buffer); /* No error detection! */
  ASSERTZEROP("fclose()", , fclose(test), );
  free(buffer);
  SUB_STRT("setbuf() - read");
  buffer = malloc(BUFSIZ);
  ASSERTNOTNULLP("fopen(CONSOLE,r)", test=fopen("CONSOLE","r"), test, );
  setbuf(test, buffer); /* No error detection! */
  ASSERTZEROP("fclose()", , fclose(test), );
  free(buffer);
}

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
/*   Buffer is used a a cache for Fixed Record Length files only                                  */
/*   Modes _IOFBF and _IOLBF are equivalent and are a line cache for reading                      */
/*                                                                                                */
/* Returns:                                                                                       */
/*   If the buffer is correctly assigned to the file, a zero value is returned.                   */
/*   Otherwise, a non-zero value is returned                                                      */
/**************************************************************************************************/
static void setvbuf_t() {
  FILE* test;
  char* buffer;

  SUB_STRT("setvbuf() - write");
  ASSERTNOTNULLP("fopen(CONSOLE,w)", test=fopen("CONSOLE","w"), test, );
  ASSERTNOTZERO("setvbuf()", , setvbuf(test, 0, _IOLBF, BUFSIZ), );
  ASSERTZEROP("fclose()", , fclose(test), );
  SUB_STRT("setvbuf() - read");
  ASSERTNOTNULLP("fopen(CONSOLE,r)", test=fopen("CONSOLE","r"), test, );
  ASSERTNOTZERO("setvbuf()", , setvbuf(test, 0, _IOLBF, BUFSIZ), );
  ASSERTZEROP("fclose()", , fclose(test), );
}

/**************************************************************************************************/
/* int fgetrec(FILE * stream)                                                                     */
/*                                                                                                */
/* The function returns the current record number (1 base)                                        */
/* Non-standard GCC CMS extention                                                                 */
/*                                                                                                */
/* Returns:                                                                                       */
/*    record number or 0 on failure / not block device                                            */
/*                                                                                                */
/* int fsetrec(FILE * stream, int recnum)                                                         */
/*                                                                                                */
/* The function sets/moves to the record number (1 base)                                          */
/* Non-standard GCC CMS extention                                                                 */
/*                                                                                                */
/* Returns:                                                                                       */
/*    record number or 0 on failure / not block device                                            */
/**************************************************************************************************/
static void fgetsetrec_t() {
  FILE* test;
  int record;

  SUB_STRT("fgetrec() and fsetrec()");
  ASSERTNOTNULLP("fopen(CONSOLE,w)", test=fopen("CONSOLE","w"), test, );
  ASSERTZERO("fgetrec()", , record = fgetrec(test), );
  ASSERTZERO("fsetrec()", , fsetrec(test, record), );
  ASSERTZEROP("fclose()", , fclose(test), );
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
static int fcachehits_t() {
  FILE* test;

  SUB_STRT("fcachehits()");
  ASSERTNOTNULLP("fopen(CONSOLE,w)", test=fopen("CONSOLE","w"), test, );
  ASSERTNOTZERO("fcachehits()", , fcachehits(test) == -1, );
  ASSERTZEROP("fclose()", , fclose(test), );
}


/**************************************************************************************************/
/* int fateof(FILE * stream)                                                                      */
/* Non-standard GCC CMS extention                                                                 */
/* Detects if at EOF - unlike feof() it predicts if the next read will cause an EOF               */
/*                                                                                                */
/* Returns 1 if at EOF, EOF on error, or 0 is not at EOF                                          */
/*                                                                                                */
/**************************************************************************************************/
static int fateof_t() {
  FILE* test;

  SUB_STRT("fateof()");
  ASSERTNOTNULLP("fopen(CONSOLE,w)", test=fopen("CONSOLE","w"), test, );
  ASSERTZERO("fateof()", , fateof(test), );
  ASSERTZEROP("fclose()", , fclose(test), );
  ASSERTNOTNULLP("fopen(CONSOLE,r)", test=fopen("CONSOLE","r"), test, );
  ASSERTZERO("fateof()", , fateof(test), );
  ASSERTZEROP("fclose()", , fclose(test), );
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
static int fgethandle_t() {
  FILE* test;

  SUB_STRT("fgethandle()");
  ASSERTNOTNULLP("fopen(CONSOLE,w)", test=fopen("CONSOLE","w"), test, );
  ASSERTNOTZERO("fgethandle()", , fgethandle("CONSOLE")==test, );
  ASSERTZEROP("fclose()", , fclose(test), );
  ASSERTZERO("fgethandle()", , fgethandle("CONSOLE"), );
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
static int fgetrecs_t() {
  FILE* test;

  SUB_STRT("fgetrecs()");
  ASSERTNOTNULLP("fopen(CONSOLE,w)", test=fopen("CONSOLE","w"), test, );
  ASSERTEOF("fgetrecs()", , fgetrecs(test), );
  ASSERTZEROP("fclose()", , fclose(test), );
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
static int fgetlen_t() {
  FILE* test;

  SUB_STRT("fgetlen()");
  ASSERTNOTNULLP("fopen(CONSOLE,w)", test=fopen("CONSOLE","w"), test, );
  ASSERTEOF("fgetlen()", , fgetlen(test), );
  ASSERTZEROP("fclose()", , fclose(test), );
}

/**************************************************************************************************/
/* void append(FILE * stream)                                                                     */
/*                                                                                                */
/* Non-standard GCC CMS extention                                                                 */
/* Move the file position indicator to the end of the specified stream, and clears the            */
/* error and EOF flags associated with that stream. This is the oposite of rewind()               */
/*    stream   a pointer to the open stream.                                                      */
/**************************************************************************************************/
static int append_t() {
  FILE* test;

  SUB_STRT("append()");
  ASSERTNOTNULLP("fopen(CONSOLE,w)", test=fopen("CONSOLE","w"), test, );
  ASSERTNOTZERO("append()",append(test) , 1, );
  ASSERTZEROP("fclose()", , fclose(test), );
  SUB_STRT("append() - read");
  ASSERTNOTNULLP("fopen(CONSOLE,r)", test=fopen("CONSOLE","r"), test, );
  ASSERTNOTZERO("append()",append(test) , 1, );
  ASSERTZEROP("fclose()", , fclose(test), );
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
static void nextrecLen_t() {
  FILE* test;
  char buf[300];

  SUB_STRT("nextrecLen() - text");
  ASSERTNOTNULLP("fopen(CONSOLE,r)", test=fopen("CONSOLE","r"), test, );
  CMSstackLine(TEST_STRING, CMS_STACKLIFO);
  ASSERTNOTZERO("nextrecLen()", , nextrecLen(test)==strlen(TEST_STRING_NL), );
  ASSERTNOTEOF("fgetc()", , fgetc(test), );
  ASSERTNOTZERO("nextrecLen()", , nextrecLen(test)==strlen(TEST_STRING_NL)-1, );
  ASSERTZEROP("fclose()", , fclose(test), );

  SUB_STRT("nextrecLen() - binary");
  ASSERTNOTNULLP("fopen(CONSOLE,rb)", test=fopen("CONSOLE","rb"), test, );
  CMSstackLine(TEST_STRING, CMS_STACKLIFO);
  ASSERTNOTZERO("nextrecLen()", , nextrecLen(test)==strlen(TEST_STRING), );
  ASSERTNOTEOF("fgetc()", , fgetc(test), );
  ASSERTNOTZERO("nextrecLen()", , nextrecLen(test)==strlen(TEST_STRING)-1, );
  ASSERTZEROP("fclose()", , fclose(test), );
}


/**************************************************************************************************/
/* Run Tests                                                                                      */
/**************************************************************************************************/
void IO_CON_T() {
  SEC_STRT("IO Tests - Console");
  fopen_t();
  fclose_t();
  fflush_t();
  ferror_t();
  clearerr_t();
  fgetc_t();
  fgets_t();
  feof_t();
  ungetc_t();
  fputc_t();
  fputs_t();
  gets_t();
  puts_t();
  fwriteread_t();
  ftell_t();
  fseek_t();
  fgetsetpos_t();
  rewind_t();
  setbuf_t();
  setvbuf_t();
  fgetsetrec_t();
  fcachehits_t();
  fateof_t();
  fgethandle_t();
  fgetrecs_t();
  fgetlen_t();
  append_t();
  nextrecLen_t();
}
