/**************************************************************************************************/
/* CMSSTDIO.C - High Level STDIO implementation                                                   */
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
#include <float.h>
#include <time.h>
#include <errno.h>

static void dblcvt(double num, char cnvtype, size_t nwidth, size_t nprecision,
                   char *result);

static int
examine(const char **formt, FILE *fq, char *s, va_list *arg, int chcount);

static int GetFileid(const char *fname, char *fileid);

static int vvprintf(const char *format, va_list arg, FILE *fq, char *s);

static int vvscanf(const char *format, va_list arg, FILE *fp, const char *s);

/* The following defines are used in vvprintf and vvscanf. */
#define unused(x) ((void)(x))
#define outch(ch) ((fq == NULL) ? *s++ = (char)ch : fputc(ch, fq))
#define inch() ((fp == NULL) ? (ch = (unsigned char)*s++) : (ch = fgetc(fp)))

/**************************************************************************************************/
/* void perror( const char *str )                                                                 */
/**************************************************************************************************/
void perror(const char *str) {
    if ((str != NULL) && (str[0] != '\n')) fprintf(stderr, "%s: ", str);
    fprintf(stderr, "%s\n", strerror(errno));
    return;
}


int
fprintf(FILE *file, const char *format, ...)
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
{
    int rc;
    va_list arg;

    if (file == NULL) {
        errno = EINVAL;
        return EOF;
    }
    va_start(arg, format);
    rc = vvprintf(format, arg, file,
                  NULL);                            /* write the output to the stream */
    va_end(arg);
    return rc;
}


int
fscanf(FILE *file, const char *format, ...)
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
{
    int rc;
    va_list arg;

    if (file == NULL) {
        errno = EINVAL;
        return EOF;
    }
    va_start(arg, format);
    rc = vvscanf(format, arg, file, NULL);
    va_end(arg);
    return rc;
}

int
printf(const char *format, ...)
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
{
    int rc;
    va_list arg;

    va_start(arg, format);
    rc = vvprintf(format, arg, stdout, NULL);
    va_end(arg);
    return rc;
}


int
remove(const char *fname)
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
{
    char fileid[19];

    if (GetFileid(fname, fileid) == 0) {
        errno = ENOENT;
        return NULL;
    }
    return CMSfileErase(fileid);
}


int
rename(const char *oldfname, const char *newfname)
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
{
    char newfileid[19];
    char oldfileid[19];

    if (GetFileid(oldfname, oldfileid) == 0) {
        errno = ENOENT;
        return NULL;
    }
    if (GetFileid(newfname, newfileid) == 0) {
        errno = ENOENT;
        return NULL;
    }
    return CMSfileRename(oldfileid, newfileid);
}


int
scanf(const char *format, ...)
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
{
    int rc;
    va_list arg;

    va_start(arg, format);
    rc = vvscanf(format, arg, stdin, NULL);
    va_end(arg);
    return rc;
}


int
sprintf(char *buffer, const char *format, ...)
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
{
    int rc;
    va_list arg;

    va_start(arg, format);
    rc = vvprintf(format, arg, NULL, buffer);
    if (rc >= 0) buffer[rc] = 0;
    va_end(arg);
    return (rc);
}


int
sscanf(const char *buffer, const char *format, ...)
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
{
    int rc;
    va_list arg;

    va_start(arg, format);
    rc = vvscanf(format, arg, NULL, buffer);
    va_end(arg);
    return rc;
}


int
vfprintf(FILE *file, const char *format, va_list arg)
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
{
    int ret;

    if (file == NULL) {
        errno = EINVAL;
        return EOF;
    }
    ret = vvprintf(format, arg, file, NULL);
    return ret;
}


int
vsprintf(char *buffer, const char *format, va_list arg)
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
{
    int ret;

    ret = vvprintf(format, arg, NULL, buffer);
    if (ret >= 0) buffer[ret] = 0;
    return ret;
}


/*------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------*/
/* Internal routines.                                                                             */
/*------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------*/


static void
dblcvt(double num, char cnvtype, size_t nwidth, size_t nprecision, char *result)
/**************************************************************************************************/
/* This truly cludged piece of code was concocted by Dave Wade.  His erstwhile tutors are         */
/* probably turning in their graves.  It is however placed in the Public Domain so that any one   */
/* who wishes to improve is free to do so.                                                        */
/**************************************************************************************************/
{
    double b, round;
    int i, j, exp, pdigits, format;
    char sign, work[45];

    if (num < 0) {
        b = -num;
        sign = '-';
    } else {
        b = num;
        sign = ' ';
    }


    exp = 0;
    if (b > 1.0)
        while (b >= 10.0) {
            ++exp;
            b = b / 10.0;
        }
    else if (b == 0.0) exp = 0;
    else if (b < 1.0)
        while (b < 1.0) {
            --exp;
            b = b * 10.0;
        }

/*
 Now decide how to print and save in FORMAT.
    -1 => we need leading digits.
     0 => print in exp.
    +1 => we have digits before dp.
*/
    switch (cnvtype) {
        case 'E':
        case 'e':
            format = 0;
            break;
        case 'f':
        case 'F':
            if (exp >= 0) format = 1;
            else format = -1;
            break;
        default:
            /* Style e is used if the exponent from its conversion is less than -4 or greater than or
               equal to the precision. */
            if (exp >= 0) {
                if (nprecision > exp) format = 1;
                else format = 0;
            } else {                                                    /* if ( nprecision > (-(exp+1) ) ) { */
                if (exp >= -4) format = -1;
                else format = 0;
            }
            break;
    }


    switch (format) {                                                                       /* now round */
        case 0:                                                       /* we are printing in standard form */
            if (nprecision < DBL_MANT_DIG)
                j = nprecision;                             /* we need to round */
            else j = DBL_MANT_DIG;
            round = 1.0 / 2.0;
            i = 0;
            while (++i <= j) round = round / 10.0;
            b = b + round;
            if (b >= 10.0) {
                b = b / 10.0;
                exp = exp + 1;
            }
            break;
        case 1:                     /* we have a number > 1: need to round at the exp + nprescionth digit */
            if (exp + nprecision < DBL_MANT_DIG)
                j = exp + nprecision;                 /* e need to round */
            else j = DBL_MANT_DIG;
            round = 0.5;
            i = 0;
            while (i++ < j) round = round / 10;
            b = b + round;
            if (b >= 10.0) {
                b = b / 10.0;
                exp = exp + 1;
            }
            break;
        case -1:                                                   /* we have a number that starts 0.xxxx */
            if (nprecision < DBL_MANT_DIG)
                j = nprecision + exp +
                    1;                   /* we need to round */
            else j = DBL_MANT_DIG;
            round = 5.0;
            i = 0;
            while (i++ < j) round = round / 10;
            if (j >= 0) b = b + round;
            if (b >= 10.0) {
                b = b / 10.0;
                exp = exp + 1;
            }
            if (exp >= 0) format = 1;
            break;
    }

    /* Now extract the requisite number of digits */
    if (format ==
        -1) {               /* number < 1.0 so we need to print the "0." and the leading zeros */
        result[0] = sign;
        result[1] = '0';
        result[2] = '.';
        result[3] = 0x00;
        while (++exp) {
            --nprecision;
            strcat(result, "0");
        }
        i = b;
        --nprecision;
        work[0] = (char) ('0' + i % 10);
        work[1] = 0x00;
        strcat(result, work);
        pdigits = nprecision;
        while (pdigits-- > 0) {
            b = b - i;
            b = b * 10.0;
            i = b;
            work[0] = (char) ('0' + i % 10);
            work[1] = 0x00;
            strcat(result, work);
        }
    } else if (format ==
               +1) {                                   /* number >= 1.0 just print the first digit */
        i = b;
        result[0] = sign;
        result[1] = '\0';
        work[0] = (char) ('0' + i % 10);
        work[1] = 0x00;
        strcat(result, work);
        nprecision = nprecision + exp;
        pdigits = nprecision;
        while (pdigits-- > 0) {
            if (((nprecision - pdigits - 1) == exp)) strcat(result, ".");
            b = b - i;
            b = b * 10.0;
            /* The following test needs to be adjusted to allow for numeric fuzz. */
            if (((nprecision - pdigits - 1) > exp) && (b < 0.1E-15)) {
                if (cnvtype != 'G' && cnvtype != 'g') strcat(result, "0");
            } else {
                i = b;
                work[0] = (char) ('0' + i % 10);
                work[1] = 0x00;
                strcat(result, work);
            }
        }
    } else {                                                                  /* printing in standard form */
        i = b;
        result[0] = sign;
        result[1] = '\0';
        work[0] = (char) ('0' + i % 10);
        work[1] = 0x00;
        strcat(result, work);
        strcat(result, ".");
        pdigits = nprecision;
        while (pdigits-- > 0) {
            b = b - i;
            b = b * 10.0;
            i = b;
            work[0] = (char) ('0' + i % 10);
            work[1] = 0x00;
            strcat(result, work);
        }
    }
    if (format ==
        0) {                                                      /* exp format - put exp on end */
        work[0] = 'E';
        if (exp < 0) {
            exp = -exp;
            work[1] = '-';
        } else work[1] = '+';
        work[2] = (char) ('0' + (exp / 10) % 10);
        work[3] = (char) ('0' + exp % 10);
        work[4] = 0x00;
        strcat(result, work);
    }
    /* printf(" Final Answer = <%s> fprintf goves=%g\n", result,num); */
    /*  do we need to pad? */
    if (result[0] == ' ') strcpy(work, result + 1); else strcpy(work, result);
    pdigits = nwidth - strlen(work);
    result[0] = 0x00;
    while (pdigits > 0) {
        strcat(result, " ");
        pdigits--;
    }
    strcat(result, work);
    return;
}                                                                                   /* end of dblcvt */


static int
examine(const char **formt, FILE *fq, char *s, va_list *arg, int chcount)
/**************************************************************************************************/
/* Part of vvprintf... could be inline, really.                                                   */
/**************************************************************************************************/
{
    int extraCh = 0;
    int flagMinus = 0;
    int flagPlus = 0;
    int flagSpace = 0;
    int flagHash = 0;
    int flagZero = 0;
    int width = 0;
    int precision = -1;
    int half = 0;
    int lng = 0;
    int specifier = 0;
    int fin;
    long lvalue;
    short int hvalue;
    int ivalue;
    unsigned long ulvalue;
    double vdbl;
    char *svalue;
    char work[50];
    int x;
    int y;
    int rem;
    const char *format;
    int base;
    int fillCh;
    int neg;
    int length;
    size_t slen;

    unused(chcount);
    format = *formt;
    fin = 0;
    while (!fin) {
        switch (*format) {
            case '-':
                flagMinus = 1;
                break;
            case '+':
                flagPlus = 1;
                break;
            case ' ':
                flagSpace = 1;
                break;
            case '#':
                flagHash = 1;
                break;
            case '0':
                flagZero = 1;
                break;
            case '*':
                width = va_arg(*arg, int);
                break;
            default:
                fin = 1;
                break;
        }
        if (!fin) format++;
        else {
            if (flagSpace && flagPlus) flagSpace = 0;
            if (flagMinus) flagZero = 0;
        }
    }

    if (isdigit((unsigned char) *format))
        while (isdigit((unsigned char) *format)) {      /* process width */
            width = width * 10 + (*format - '0');
            format++;
        }

    if (*format ==
        '.') {                                                           /* process precision */
        format++;
        if (*format == '*') {
            precision = va_arg(*arg, int);
            format++;
        } else {
            precision = 0;
            while (isdigit((unsigned char) *format)) {
                precision = precision * 10 + (*format - '0');
                format++;
            }
        }
    }

    if (*format ==
        'h') {                                                               /* process h/l/L */
        /* all environments should promote shorts to ints, so we should be able to ignore the 'h'
          specifier.  It will create problems otherwise.
          half = 1; */
    } else if (*format == 'l') lng = 1;
    else if (*format == 'L') lng = 1;
    else format--;
    format++;

    specifier = *format;                                                            /* process specifier */
    if (strchr("dxXuiop", specifier) != NULL && specifier != 0) {
        if (precision < 0) precision = 1;
        if (lng) lvalue = va_arg(*arg, long);
        else if (half) {
            hvalue = va_arg(*arg, short);
            if (specifier == 'u') lvalue = (unsigned short) hvalue;
            else lvalue = hvalue;
        } else {
            ivalue = va_arg(*arg, int);
            if (specifier == 'u') lvalue = (unsigned int) ivalue;
            else lvalue = ivalue;
        }
        ulvalue = (unsigned long) lvalue;
        if ((lvalue < 0) && ((specifier == 'd') || (specifier == 'i'))) {
            neg = 1;
            ulvalue = -lvalue;
        } else neg = 0;
        if ((specifier == 'X') || (specifier == 'x') || (specifier == 'p'))
            base = 16;
        else if (specifier == 'o') base = 8;
        else base = 10;
        if (specifier == 'p') {}
        x = 0;
        while (ulvalue > 0) {
            rem = (int) (ulvalue % base);
            if (rem < 10) work[x] = (char) ('0' + rem);
            else {
                if ((specifier == 'X') || (specifier == 'p'))
                    work[x] = (char) ('A' + (rem - 10));
                else work[x] = (char) ('a' + (rem - 10));
            }
            x++;
            ulvalue = ulvalue / base;
        }
        while (x < precision) {
            work[x] = '0';
            x++;
        }
        if (neg) work[x++] = '-';
        else if (flagPlus) work[x++] = '+';
        if (flagZero) fillCh = '0';
        else fillCh = ' ';
        y = x;
        if (!flagMinus)
            while (y < width) {
                outch(fillCh);
                extraCh++;
                y++;
            }
        if (flagHash && (toupper((unsigned char) specifier) == 'X')) {
            outch('0');
            outch('x');
            extraCh += 2;
        }
        x--;
        while (x >= 0) {
            outch(work[x]);
            extraCh++;
            x--;
        }
        if (flagMinus)
            while (y < width) {
                outch(fillCh);
                extraCh++;
                y++;
            }
    } else if (strchr("eEgGfF", specifier) != NULL && specifier != 0) {
        if (precision < 0) precision = 6;
        vdbl = va_arg(*arg, double);
        dblcvt(vdbl, specifier, width, precision,
               work);                                  /* 'e','f' etc. */
        slen = strlen(work);
        if (fq == NULL) {
            memcpy(s, work, slen);
            s += slen;
        } else fputs(work, fq);
        extraCh += slen;
    } else if (specifier == 's') {
        svalue = va_arg(*arg, char *);
        fillCh = ' ';
        if (precision > -1) {
            char *p;

            p = memchr(svalue, '\0', precision);
            if (p != NULL) length = (int) (p - svalue);
            else length = precision;
        } else length = strlen(svalue);
        if (!flagMinus) {
            if (length < width) {
                extraCh += (width - length);
                for (x = 0; x < (width - length); x++) outch(fillCh);
            }
        }
        for (x = 0; x < length; x++) outch(svalue[x]);
        extraCh += length;
        if (flagMinus) {
            if (length < width) {
                extraCh += (width - length);
                for (x = 0; x < (width - length); x++) outch(fillCh);
            }
        }
    }
    *formt = format;
    return (extraCh);
}

static int
GetFileid(const char *fname, char *fileid)
/**************************************************************************************************/
/* int GetFileid(const char * fname, char * fileid)                                               */
/*                                                                                                */
/* Parse the specified CMS filename / filetype / filemode in 'fname', filling in 'fileid' for use */
/* in other routines such as fopen, remove, rename.                                               */
/*                                                                                                */
/* Returns 1 if successful, 0 if not.                                                             */
/**************************************************************************************************/
{
    char *s;
    char fileinfo[40];

    strncpy(fileinfo, fname,
            sizeof(fileinfo));                /* because strtok modifies the string :-( */
    memset(fileid, ' ',
           18);                                              /* initialize fileid to blanks */
    s = strtok(fileinfo,
               " ");                                                           /* get filename */
    if (s == NULL || strlen(s) > 8) {
        errno = EINVAL;
        return 0;
    } else
        memcpy(fileid, s, strlen(s));
    s = strtok(NULL,
               " ");                                                               /* get filetype */
    if (s == NULL || strlen(s) > 8) {
        errno = EINVAL;
        return 0;
    } else
        memcpy(fileid + 8, s, strlen(s));
    s = strtok(NULL,
               " ");                                                               /* get filemode */
    if (s == NULL || strlen(s) > 2) {
        errno = EINVAL;
        return 0;
    } else
        memcpy(fileid + 16, s, strlen(s));
    fileid[18] = 0;                                          /* terminate fileid (not strictly required) */
    return 1;
}


static int
vvprintf(const char *format, va_list arg, FILE *fq, char *s)
/**************************************************************************************************/
/* static int vvprintf(const char * format, va_list arg, FILE * fq, char * s)                     */
/*                                                                                                */
/* Do the real work of printf.                                                                    */
/*    format   a string containing characters to be printed and formatting specifications for the */
/*             the variables that follow, if any.                                                 */
/*    va_list  a list of variables that are converted to text and printed according to the        */
/*             formatting specifications in 'format'.                                             */
/*    fq       a pointer to a FILE handle to which the formatted output is written.               */
/*    s        a pointer to a buffer into which the formatted output is copied.                   */
/*                                                                                                */
/* Returns:                                                                                       */
/*    the number of characters written.                                                           */
/*                                                                                                */
/* Notes:                                                                                         */
/*    1.  Either 'fq' or 's' must be NULL.                                                        */
/**************************************************************************************************/
{
    int fin = 0;
    int vint;
    double vdbl;
    unsigned int uvint;
    char *vcptr;
    int chcount = 0;
    size_t len;
    char numbuf[50];
    char *nptr;

    while (!fin) {
        if (*format == '\0') fin = 1;
        else if (*format == '%') {
            format++;
            if (*format == 'd') {
                vint = va_arg(arg, int);
                if (vint < 0) uvint = -vint;
                else uvint = vint;
                nptr = numbuf;
                do {
                    *nptr++ = (char) ('0' + uvint % 10);
                    uvint /= 10;
                } while (uvint > 0);
                if (vint < 0) *nptr++ = '-';
                do {
                    nptr--;
                    outch(*nptr);
                    chcount++;
                } while (nptr != numbuf);
            } else if (strchr("eEgGfF", *format) != NULL && *format != 0) {
                vdbl = va_arg(arg, double);
                dblcvt(vdbl, *format, 0, 6,
                       numbuf);                                       /* 'e','f' etc. */
                len = strlen(numbuf);
                if (fq == NULL) {
                    memcpy(s, numbuf, len);
                    s += len;
                } else fputs(numbuf, fq);
                chcount += len;
            } else if (*format == 's') {
                vcptr = va_arg(arg, char *);
                if (vcptr == NULL) vcptr = "(null)";
                if (fq == NULL) {
                    len = strlen(vcptr);
                    memcpy(s, vcptr, len);
                    s += len;
                    chcount += len;
                } else {
                    fputs(vcptr, fq);
                    chcount += strlen(vcptr);
                }
            } else if (*format == 'c') {
                vint = va_arg(arg, int);
                outch(vint);
                chcount++;
            } else if (*format == '%') {
                outch('%');
                chcount++;
            } else {
                int extraCh;

                extraCh = examine(&format, fq, s, &arg, chcount);
                chcount += extraCh;
                if (s != NULL) s += extraCh;
            }
        } else {
            outch(*format);
            chcount++;
        }
        format++;
    }
    return (chcount);
}                                                                                 /* end of vvprintf */


static int
vvscanf(const char *format, va_list arg, FILE *fp, const char *s)
/**************************************************************************************************/
/* vvscanf - the guts of the input scanning                                                       */
/* several mods by Dave Edwards                                                                   */
/**************************************************************************************************/
{
    int ch;
    int fin = 0;
    int cnt = 0;
    char *cptr;
    int *iptr;
    unsigned int *uptr;
    long *lptr;
    unsigned long *luptr;
    short *hptr;
    unsigned short *huptr;
    double *dptr;
    float *fptr;
    long startpos;
    const char *startp;
    int skipvar;                                             /* nonzero if we are skipping this variable */
    int modlong;                                                        /* nonzero if "l" modifier found */
    int modshort;                                                       /* nonzero if "h" modifier found */
    int informatitem;                                                /* nonzero if % format item started */
/* informatitem is 1 if we have processed "%l" but not the type letter (s,d,e,f,g,...) yet */

    if (fp != NULL) startpos = ftell(fp);
    else startp = s;
    inch();
    informatitem = 0;                                                                            /* init */
    if ((fp != NULL && ch == EOF) || (fp == NULL && ch == 0))
        return EOF;     /* at EOF or end of string */
    while (!fin) {
        if (*format == '\0') fin = 1;
        else if (*format == '%' || informatitem) {
            if (*format ==
                '%') {                                                /* starting a format item */
                format++;
                modlong = 0;                                                                          /* init */
                modshort = 0;
                skipvar = 0;
                if (*format == '*') {
                    skipvar = 1;
                    format++;
                }
            }
            if (*format ==
                '%') {                                                                 /* %% */
                if (ch != '%') return (cnt);
                inch();
                informatitem = 0;
            } else if (*format ==
                       'l') {                            /* type modifier: l (e.g. %ld or %lf) */
                modlong = 1;
                informatitem = 1;
            } else if (*format ==
                       'h') {                                  /* type modifier: h (short int) */
                modshort = 1;
                informatitem = 1;
            } else {                                                          /* process a type character */
                informatitem = 0;                                                    /* end of format item */
                if (*format == 's') {
                    if (!skipvar) cptr = va_arg(arg, char *);
                    while (ch >= 0 && isspace(ch))
                        inch();                       /* skip leading whitespace */
                    if ((fp != NULL && ch == EOF) ||
                        (fp == NULL && ch == 0)) {   /* EOF or end of string */
                        fin = 1;
                        if (!skipvar) *cptr = 0;                                     /* give a null string */
                    } else {
                        for (;;) {
                            if (isspace(ch)) break;
                            if ((fp != NULL && ch == EOF) ||
                                (fp == NULL && ch == 0)) {
                                fin = 1;
                                break;
                            }
                            if (!skipvar) *cptr++ = (char) ch;
                            inch();
                        }
                        if (!skipvar) *cptr = '\0';
                        cnt++;
                    }
                } else if (*format == '[') {
                    int reverse = 0;
                    int found;
                    const char *first;
                    const char *last;
                    size_t size;
                    size_t mcnt = 0;

                    if (!skipvar) cptr = va_arg(arg, char *);
                    format++;
                    if (*format == '?') {
                        reverse = 1;
                        format++;
                    }
                    if (*format == '\0') break;
                    first = format;
                    format++;
                    last = strchr(format, ']');
                    if (last == NULL) return (cnt);
                    size = last - first;
                    /* Note: C90 doesn't require special processing for '-' so it hasn't been added. */
                    while (1) {
                        found = (memchr(first, ch, size) != NULL);
                        if (found && reverse) break;
                        if (!found && !reverse) break;
                        if (!skipvar) *cptr++ = (char) ch;
                        mcnt++;
                        inch();
                        if ((fp != NULL && ch == EOF) ||
                            (fp == NULL && ch == 0))
                            break;                                   /* if at EOF or end of string, bug out */
                    }
                    if (mcnt > 0) {
                        if (!skipvar) *cptr++ = '\0';
                        cnt++;
                    } else break;
                    format = last + 1;
                } else if (*format == 'c') {
                    if (!skipvar) cptr = va_arg(arg, char *);
                    if ((fp != NULL && ch == EOF) || (fp == NULL && ch == 0))
                        fin = 1;                                                /* at EOF or end of string */
                    else {
                        if (!skipvar) *cptr = ch;
                        cnt++;
                        inch();
                    }
                } else if (*format == 'n') {
                    uptr = va_arg(arg, unsigned int *);
                    if (fp != NULL) *uptr = ftell(fp) - startpos;
                    else *uptr = startp - s;
                } else if (*format == 'd' || *format == 'u' || *format == 'x' ||
                           *format == 'o' ||
                           *format == 'p' || *format == 'i') {
                    int neg = 0;
                    unsigned long x = 0;
                    int undecided = 0;
                    int base = 10;
                    int reallyp = 0;
                    int mcnt = 0;

                    if (*format == 'x') base = 16;
                    else if (*format == 'p') base = 16;
                    else if (*format == 'o') base = 8;
                    else if (*format == 'i') base = 0;
                    if (!skipvar) {
                        if ((*format == 'd') || (*format == 'i')) {
                            if (modlong) lptr = va_arg(arg, long *);
                            else if (modshort) hptr = va_arg(arg, short *);
                            else iptr = va_arg(arg, int *);
                        } else {
                            if (modlong) luptr = va_arg(arg, unsigned long *);
                            else if (modshort)
                                huptr = va_arg(arg, unsigned short *);
                            else uptr = va_arg(arg, unsigned int *);
                        }
                    }
                    while (ch >= 0 && isspace(ch))
                        inch();                       /* skip leading whitespace */
                    if (ch == '-') {
                        neg = 1;
                        inch();
                    } else if (ch == '+') inch();

                    /* This logic is the same as strtoul so if you change this, change that one too. */
                    if (base == 0) undecided = 1;
                    while (!((fp != NULL && ch == EOF) ||
                             (fp == NULL && ch == 0))) {
                        if (isdigit((unsigned char) ch)) {
                            if (base == 0) {
                                if (ch == '0') base = 8;
                                else {
                                    base = 10;
                                    undecided = 0;
                                }
                            }
                            x = x * base + (ch - '0');
                            inch();
                        } else if (isalpha((unsigned char) ch)) {
                            if ((ch == 'X') || (ch == 'x')) {
                                if ((base == 0) || ((base == 8) && undecided)) {
                                    base = 16;
                                    undecided = 0;
                                    inch();
                                } else if (base == 16)
                                    inch();           /* hex values may have an optional 0x */
                                else break;
                            } else if (base <= 10) break;
                            else {
                                x = x * base +
                                    (toupper((unsigned char) ch) - 'A') + 10;
                                inch();
                            }
                        } else break;
                        mcnt++;
                    }

                    if (mcnt == 0) break;
                    if (!skipvar) {
                        if ((*format == 'd') || (*format == 'i')) {
                            int lval;

                            if (neg) lval = (long) -x;
                            else lval = (long) x;
                            if (modlong) *lptr = lval;                          /* l modifier: assign to long */
                            else if (modshort) *hptr = (short) lval;                           /* h modifier */
                            else *iptr = (int) lval;
                        } else {
                            if (modlong) *luptr = (unsigned long) x;
                            else if (modshort) *huptr = (unsigned short) x;
                            else *uptr = (unsigned int) x;
                        }
                    }
                    cnt++;
                } else if (*format == 'e' || *format == 'f' || *format == 'g' ||
                           *format == 'E' || *format == 'G') {
                    /* Floating-point (double or float) input item */
                    int negsw1, negsw2, dotsw, expsw, ndigs1, ndigs2, nfdigs;
                    int ntrailzer, expnum, expsignsw;
                    double fpval, pow10;

                    if (!skipvar) {
                        if (modlong) dptr = va_arg(arg, double *);
                        else fptr = va_arg(arg, float *);
                    }
                    negsw1 = 0;
                    negsw2 = 0;
                    dotsw = 0;
                    expsw = 0;
                    ndigs1 = 0;
                    ndigs2 = 0;
                    nfdigs = 0;
                    ntrailzer = 0;                                     /* # of trailing 0's unaccounted for */
                    expnum = 0;
                    expsignsw = 0;                                    /* nonzero means done +/- on exponent */
                    fpval = 0.0;
                    while (ch >= 0 && isspace(ch))
                        inch();                       /* skip leading whitespace */
                    if (ch == '-') {
                        negsw1 = 1;
                        inch();
                    } else if (ch == '+') inch();
                    while (ch > 0) {
                        if (ch == '.' && dotsw == 0 && expsw == 0) dotsw = 1;
                        else if (isdigit(ch)) {
                            if (expsw) {
                                ndigs2++;
                                expnum = expnum * 10 + (ch - '0');
                            } else {
                                /* To avoid overflow or loss of precision, skip leading and trailing 0 */
                                /* unless needed. (Automatic for leading 0s, since 0.0*10.0 is 0.0). */
                                ndigs1++;
                                if (dotsw) nfdigs++;
                                if (ch == '0' && fpval != 0.)
                                    ntrailzer++;                /* possible trailing 0 */
                                else {                                    /* account for any preceding zeros */
                                    while (ntrailzer > 0) {
                                        fpval *= 10.;
                                        ntrailzer--;
                                    }
                                    fpval = fpval * 10.0 + (ch - '0');
                                }
                            }
                        } else if ((ch == 'e' || ch == 'E') && expsw == 0)
                            expsw = 1;
                        else if ((ch == '+' || ch == '-') && expsw == 1 &&
                                 ndigs2 == 0 && expsignsw == 0) {
                            expsignsw = 1;
                            if (ch == '-') negsw2 = 1;
                        } else break;                                         /* bad char: end of input item */
                        inch();
                    }
                    if ((fp != NULL && ch == EOF) || (fp == NULL && ch == 0))
                        fin = 1;
                    if (ndigs1 == 0 || (expsw && ndigs2 == 0))
                        return (cnt);    /* check for valid fl-pt value */
                    if (negsw2) expnum = -expnum;                               /* complete the fl-pt value */
                    expnum += ntrailzer - nfdigs;
                    if (expnum != 0 && fpval != 0.0) {
                        negsw2 = 0;
                        if (expnum < 0) {
                            expnum = -expnum;
                            negsw2 = 1;
                        }
                        /* Multiply or divide by 10.0**expnum, using bits of expnum (fast method). */
                        pow10 = 10.0;
                        for (;;) {
                            if (expnum &
                                1) {                                             /* low-order bit */
                                if (negsw2) fpval /= pow10;
                                else fpval *= pow10;
                            }
                            expnum >>= 1;                                               /* shift right 1 bit */
                            if (expnum == 0) break;
                            pow10 *= pow10;                                  /* 10.**n where n is power of 2 */
                        }
                    }
                    if (negsw1) fpval = -fpval;
                    if (!skipvar) {                                       /* l modifier: assign to double */
                        if (modlong) *dptr = fpval;
                        else *fptr = (float) fpval;
                    }
                    cnt++;
                }
            }

        } else if (isspace((unsigned char) (*format))) {
            /* Whitespace char in format string: skip next whitespace chars in input data.  This */
            /* supports input of multiple data items. */
            while (ch >= 0 && isspace(ch)) inch();
        } else {                                               /* some other character in the format string */
            if (ch != *format) return (cnt);
            inch();
        }
        format++;
        if ((fp != NULL && ch == EOF) || (fp == NULL && ch == 0))
            fin = 1;                         /* EOF */
    }
    if (fp != NULL) ungetc(ch, fp);
    return (cnt);
}
