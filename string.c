/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  string.c - implementation of routines in string.h                */
/*                                                                   */
/*********************************************************************/

#include "ctype.h"                             /* needed for stricmp */
#include "string.h"
#include "errno.h"

#ifdef memchr
#undef memchr
#endif
void *memchr(const void *s, int c, size_t n)
{
    const unsigned char *p;
    size_t x = 0;

    p = (const unsigned char *)s;
    while (x < n)
    {
        if (*p == (unsigned char)c) return ((void *)p);
        p++;
        x++;
    }
    return (NULL);
}

#ifdef memcmp
#undef memcmp
#endif
int memcmp(const void *s1, const void *s2, size_t n)
{
    const unsigned char *p1;
    const unsigned char *p2;
    size_t x = 0;

    p1 = (const unsigned char *)s1;
    p2 = (const unsigned char *)s2;
    while (x < n)
    {
        if (p1[x] < p2[x]) return (-1);
        else if (p1[x] > p2[x]) return (1);
        x++;
    }
    return (0);
}

#ifndef USE_ASSEMBLER
#ifdef memcpy
#undef memcpy
#endif
#ifndef __32BIT__
void *memcpy(void *s1, const void *s2, size_t n)
{
    register const unsigned char *f = s2;
    register const unsigned char *fe;
    register unsigned char *t = s1;

    fe = f + n;
    while (f != fe)
    {
        *t++ = *f++;
    }
    return (s1);
}
#else
void *memcpy(void *s1, const void *s2, size_t n)
{
    register unsigned int *p = (unsigned int *)s1;
    register unsigned int *cs2 = (unsigned int *)s2;
    register unsigned int *endi;

    endi = (unsigned int *)((char *)p + (n & ~0x03));
    while (p != endi)
    {
        *p++ = *cs2++;
    }
    switch (n & 0x03)
    {
        case 0:
            break;
        case 1:
            *(char *)p = *(char *)cs2;
            break;
        case 2:
            *(char *)p = *(char *)cs2;
            p = (unsigned int *)((char *)p + 1);
            cs2 = (unsigned int *)((char *)cs2 + 1);
            *(char *)p = *(char *)cs2;
            break;
        case 3:
            *(char *)p = *(char *)cs2;
            p = (unsigned int *)((char *)p + 1);
            cs2 = (unsigned int *)((char *)cs2 + 1);
            *(char *)p = *(char *)cs2;
            p = (unsigned int *)((char *)p + 1);
            cs2 = (unsigned int *)((char *)cs2 + 1);
            *(char *)p = *(char *)cs2;
            break;
    }
    return (s1);
}
#endif /* 32BIT */
#endif /* USE_ASSEMBLER */

#ifdef memmove
#undef memmove
#endif
void *memmove(void *s1, const void *s2, size_t n)
{
    char *p = s1;
    const char *cs2 = s2;
    size_t x;

    if (p <= cs2)
    {
        for (x=0; x < n; x++)
        {
            *p = *cs2;
            p++;
            cs2++;
        }
    }
    else
    {
        if (n != 0)
        {
            for (x=n-1; x > 0; x--)
            {
                *(p+x) = *(cs2+x);
            }
            *(p+x) = *(cs2+x);
        }
    }
    return (s1);
}

#ifdef memset
#undef memset
#endif
void *memset(void *s, int c, size_t n)
{
    size_t x = 0;

    for (x = 0; x < n; x++)
    {
        *((char *)s + x) = (unsigned char)c;
    }
    return (s);
}

#ifdef strcat
#undef strcat
#endif
char *strcat(char *s1, const char *s2)
{
    char *p = s1;

    while (*p != '\0') p++;
    while ((*p = *s2) != '\0')
    {
        p++;
        s2++;
    }
    return (s1);
}

#ifdef strchr
#undef strchr
#endif
char *strchr(const char *s, int c)
{
    while (*s != '\0')
    {
        if (*s == (char)c) return ((char *)s);
        s++;
    }
    if (c == '\0') return ((char *)s);
    return (NULL);
}

#ifdef strcmp
#undef strcmp
#endif
int strcmp(const char *s1, const char *s2)
{
    const unsigned char *p1;
    const unsigned char *p2;

    p1 = (const unsigned char *)s1;
    p2 = (const unsigned char *)s2;
    while (*p1 != '\0')
    {
        if (*p1 < *p2) return (-1);
        else if (*p1 > *p2) return (1);
        p1++;
        p2++;
    }
    if (*p2 == '\0') return (0);
    else return (-1);
}

#ifdef strcoll
#undef strcoll
#endif
int strcoll(const char *s1, const char *s2)
{
    return (strcmp(s1, s2));
}

#ifdef strcpy
#undef strcpy
#endif
char *strcpy(char *s1, const char *s2)
{
    char *p = s1;

    while ((*p++ = *s2++) != '\0') ;
    return (s1);
}

#ifdef strcspn
#undef strcspn
#endif
size_t strcspn(const char *s1, const char *s2)
{
    const char *p1;
    const char *p2;

    p1 = s1;
    while (*p1 != '\0')
    {
        p2 = s2;
        while (*p2 != '\0')
        {
            if (*p1 == *p2) return ((size_t)(p1 - s1));
            p2++;
        }
        p1++;
    }
    return ((size_t)(p1 - s1));
}

#ifdef strerror
#undef strerror
#endif
char *strerror(int errnum)
{
  switch(errnum)
  {
    case 0: return ("No error has occurred.");
    case EPERM: return("Operation not permitted.");
    case ENOENT: return("No such file or directory.");
    case EINTR: return("Interrupted system call.");
    case EIO: return("Input/output error.");
    case ENXIO: return("No such device or address.");
    case E2BIG: return("Argument list too long.");
    case ENOEXEC: return("Exec format error.");
    case EBADF: return("Bad file descriptor.");
    case EDEADLK: return("Resource deadlock avoided.");
    case ENOMEM: return("Cannot allocate memory.");
    case EACCES: return("Permission denied.");
    case EFAULT: return("Bad address.");
    case ENOTBLK: return("Block device required.");
    case EBUSY: return("Device or resource busy.");
    case EEXIST: return("File exists.");
    case ENODEV: return("No such device.");
    case ENOTDIR: return("Not a directory.");
    case EISDIR: return("Is a directory.");
    case EINVAL: return("Invalid argument.");
    case EMFILE: return("Too many open files.");
    case ENFILE: return("Too many open files in system.");
    case EFBIG: return("File too large.");
    case ENOSPC: return("No space left on device.");
    case ESPIPE: return("Illegal seek.");
    case EROFS: return("Read-only file system.");
    case EDOM: return("Numerical argument out of domain.");
    case ERANGE: return("Numerical result out of range.");
    case EAGAIN: return("Resource temporarily unavailable.");
    case ENAMETOOLONG: return("File name too long.");
    case ESTALE: return("Stale file handle.");
    case EFTYPE: return("Inappropriate file type or format.");
    case EAUTH: return("Authentication error.");
    case ENOSYS: return("Function not implemented.");
    case ENOTSUP: return("Not supported.");
    case EGREGIOUS: return("You really blew it this time.");
    case EIEIO: return("Computer bought the farm.");
    case EGRATUITOUS: return("Gratuitous error.");
    case EBADMSG: return("Bad message.");
    case EIDRM: return("Identifier removed.");
    case ENODATA: return("No data available.");
    case EOVERFLOW: return("Value too large for defined data type.");
    default: return("Unknown error.");
  }
}

#ifdef stricmp
#undef stricmp
#endif
int stricmp(const char *s1, const char *s2)
{
    const unsigned char *p1;
    const unsigned char *p2;

    p1 = (const unsigned char *)s1;
    p2 = (const unsigned char *)s2;
    while (*p1 != '\0')
    {
        if (toupper(*p1) < toupper(*p2)) return (-1);
        else if (toupper(*p1) > toupper(*p2)) return (1);
        p1++;
        p2++;
    }
    if (*p2 == '\0') return (0);
    else return (-1);
}

#ifdef strlwr
#undef strlwr
#endif
char *strlwr(char *s1)
{
    char *p = s1;

    while ((*p++ = tolower(*p)) != '\0') ;
    return (s1);
}

#ifdef strncat
#undef strncat
#endif
char *strncat(char *s1, const char *s2, size_t n)
{
    char *p = s1;
    size_t x = 0;

    while (*p != '\0') p++;
    while ((*s2 != '\0') && (x < n))
    {
        *p = *s2;
        p++;
        s2++;
        x++;
    }
    *p = '\0';
    return (s1);
}

#ifdef strncmp
#undef strncmp
#endif
int strncmp(const char *s1, const char *s2, size_t n)
{
    const unsigned char *p1;
    const unsigned char *p2;
    size_t x = 0;

    p1 = (const unsigned char *)s1;
    p2 = (const unsigned char *)s2;
    while (x < n)
    {
        if (p1[x] < p2[x]) return (-1);
        else if (p1[x] > p2[x]) return (1);
        else if (p1[x] == '\0') return (0);
        x++;
    }
    return (0);
}

#ifdef strncpy
#undef strncpy
#endif
char *strncpy(char *s1, const char *s2, size_t n)
{
    char *p = s1;
    size_t x;

    for (x=0; x < n; x++)
    {
        *p = *s2;
        if (*s2 == '\0') break;
        p++;
        s2++;
    }
    for (; x < n; x++)
    {
        *p++ = '\0';
    }
    return (s1);
}

#ifdef strpbrk
#undef strpbrk
#endif
char *strpbrk(const char *s1, const char *s2)
{
    const char *p1;
    const char *p2;

    p1 = s1;
    while (*p1 != '\0')
    {
        p2 = s2;
        while (*p2 != '\0')
        {
            if (*p1 == *p2) return ((char *)p1);
            p2++;
        }
        p1++;
    }
    return (NULL);
}

#ifdef strrchr
#undef strrchr
#endif
char *strrchr(const char *s, int c)
{
    const char *p;

    p = s + strlen(s);
    while (p >= s)
    {
        if (*p == (char)c) return ((char *)p);
        p--;
    }
    return (NULL);
}

#ifdef strlen
#undef strlen
#endif
size_t strlen(const char *s)
{
    const char *p;

    p = s;
    while (*p != '\0') p++;
    return ((size_t)(p - s));
}

#ifdef strspn
#undef strspn
#endif
size_t strspn(const char *s1, const char *s2)
{
    const char *p1;
    const char *p2;

    p1 = s1;
    while (*p1 != '\0')
    {
        p2 = s2;
        while (*p2 != '\0')
        {
            if (*p1 == *p2) break;
            p2++;
        }
        if (*p2 == '\0') return ((size_t)(p1 - s1));
        p1++;
    }
    return ((size_t)(p1 - s1));
}


/* strstr by Frank Adam */
/* modified by Paul Edwards */

#ifdef strstr
#undef strstr
#endif
char *strstr(const char *s1, const char *s2)
{
    const char *p = s1, *p1, *p2 = s2;

    while (*p)
    {
        if (*p == *s2)
        {
            p1 = p;
            p2 = s2;
            while ((*p2 != '\0') && (*p1++ == *p2++)) ;
            if (*p2 == '\0')
            {
                return (char *)p;
            }
        }
        p++;
    }
    return NULL;
}

#ifdef strtok
#undef strtok
#endif
char *strtok(char *s1, const char *s2)
{
    static char *old = NULL;
    char *p;
    size_t len;
    size_t remain;

    if (s1 != NULL) old = s1;
    if (old == NULL) return (NULL);
    p = old;
    len = strspn(p, s2);
    remain = strlen(p);
    if (remain <= len) { old = NULL; return (NULL); }
    p += len;
    len = strcspn(p, s2);
    remain = strlen(p);
    if (remain <= len) { old = NULL; return (p); }
    *(p + len) = '\0';
    old = p + len + 1;
    return (p);
}

#ifdef strupr
#undef strupr
#endif
char *strupr(char *s1)
{
    char *p = s1;

    while ((*p++ = toupper(*p)) != '\0') ;
    return (s1);
}

#ifdef strxfrm
#undef strxfrm
#endif
size_t strxfrm(char *s1, const char *s2, size_t n)
{
    size_t oldlen;

    oldlen = strlen(s2);
    if (oldlen < n)
    {
        memcpy(s1, s2, oldlen);
        s1[oldlen] = '\0';
    }
    return (oldlen);
}
