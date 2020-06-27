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

#include <ctype.h>                             /* needed for stricmp */
#include <string.h>
#include <errno.h>
#include <gcccrab.h>

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

#ifdef memcpy
#undef memcpy
#endif
void *memcpy(void *s1, const void *s2, size_t sz)
{
  if (!s1) return 0;
  if (!s2) return s1;
  if (!sz) return s1;
  if (s1 == s2) return s1;

  /* MVCL uses register pairs so we have to force the right register assignment by gcc */
  register size_t src_addr __asm__("2") = s2;  /* Source Addr */
  register size_t src_len __asm__("3") = sz & 0x00ffffff; /* Source Length */
  register size_t dest_addr __asm__("4") = s1; /* Dest Addr */
  register size_t dest_len __asm__("5") = sz & 0x00ffffff; /* Dest Length */

  /* Use MVCL for memory move */
  __asm__("MVCL 4,2"
          :
          : "d" (src_addr), "d" (src_len), "d" (dest_addr), "d" (dest_len)
         );
  return s1;
}

#ifdef memmove
#undef memmove
#endif
void *memmove(void *s1, const void *s2, size_t sz)
{
  if (!s1) return 0;
  if (!s2) return s1;
  if (!sz) return s1;
  if (s1 == s2) return s1;
  if (s1<s2) return memcpy(s1, s2, sz); /* So called non-destructive overlap */
  int delta = s1-s2;
  if (delta >= sz) return memcpy(s1, s2, sz); /* No Overlap */

  s1 += sz;
  s2 += sz;

  if (delta>=255) {
    while (sz>255) {
     s1 -= 255;
     s2 -= 255;
     sz -= 255;
      __asm__("MVC 0(255,%0),0(%1)"
              :
              : "d" (s1), "d" (s2)
            );
    }
  }
  if (delta>=64) {
    while (sz>64) {
     s1 -= 64;
     s2 -= 64;
     sz -= 64;
     __asm__("MVC 0(64,%0),0(%1)"
              :
              : "d" (s1), "d" (s2)
            );
   }
  }
  if (delta>=16) {
   while (sz>16) {
     s1 -= 16;
     s2 -= 16;
     sz -= 16;
     __asm__("MVC 0(16,%0),0(%1)"
              :
              : "d" (s1), "d" (s2)
            );
   }
  }
  if (delta>=4) {
   while (sz>4) {
     s1 -= 4;
     s2 -= 4;
     sz -= 4;
     __asm__("MVC 0(4,%0),0(%1)"
              :
              : "d" (s1), "d" (s2)
            );
   }
  }

  /* do the rest bytewise */
  while (sz--)
    *(--(char*)s1) = *(--(char*)s2);

  return s1;
}

#ifdef memset
#undef memset
#endif
void *memset(void *s, int c, size_t sz)
{
  if (!s) return 0;
  if (!sz) return s;

  /* MVCL uses register pairs so we have to force the right register assignment by gcc */
  register size_t src_addr __asm__("2") = s;  /* Source Addr */
  register size_t src_len_pad __asm__("3") = (size_t)(c & 0xff) << 24; /* Fill Char in high byte + 0 length */
  register size_t dest_addr __asm__("4") = s; /* Dest Addr */
  register size_t dest_len __asm__("5") = sz & 0x00ffffff; /* Dest Length */

  /* Use MVCL for memory move / set */
  __asm__("MVCL 4,2"
          :
          : "d" (src_addr), "d" (src_len_pad), "d" (dest_addr), "d" (dest_len)
         );

  return s;
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
    case 0: return ("No error code has been recorded.");
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
    case ESTACKOVERFLOW: return("Auxiliary stack overflow.");
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
    char *p;
    size_t len;
    size_t remain;

    if (s1 != NULL) (GETGCCCRAB()->strtok_old) = s1;
    if ((GETGCCCRAB()->strtok_old) == NULL) return (NULL);
    p = (GETGCCCRAB()->strtok_old);
    len = strspn(p, s2);
    remain = strlen(p);
    if (remain <= len) { (GETGCCCRAB()->strtok_old) = NULL; return (NULL); }
    p += len;
    len = strcspn(p, s2);
    remain = strlen(p);
    if (remain <= len) { (GETGCCCRAB()->strtok_old) = NULL; return (p); }
    *(p + len) = '\0';
    (GETGCCCRAB()->strtok_old) = p + len + 1;
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
