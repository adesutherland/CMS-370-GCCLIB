/**************************************************************************************************/
/* CMSSTDLB.C: Native CMS implementation of STDLIB.H.                                             */
/*                                                                                                */
/* Not implemented:                                                                               */
/*     int atexit(void (* func)(void))                                                            */
/*                                                                                                */
/* Robert O'Hara, Redmond Washington, July 2010.                                                  */
/*                                                                                                */
/* Based code written by Paul Edwards and Dave Wade.                                              */
/* Released to the public domain.                                                                 */
/**************************************************************************************************/
#define IN_RESLIB
#include <cmsruntm.h>
#include <stdio.h>
#include <stdlib.h>
// Routines in signal.h not yet implemented...
// #include <signal.h>
#include <string.h>
#include <ctype.h>

// void (*__userExit[__NATEXIT])(void);                                  // don't know what this is for
static unsigned long myseed = 1;                                 // seed for random number generator

void
abort(void)
/**************************************************************************************************/
/* void abort(void)                                                                               */
/**************************************************************************************************/
{
/* TODO  raise(SIGABRT); */
  exit(EXIT_FAILURE);
}                                                                                    // end of abort


#ifdef abs
#undef abs
#endif
int
abs(int j)
/**************************************************************************************************/
/* int abs(int j)                                                                                 */
/**************************************************************************************************/
{
if (j < 0) j = -j;
return j;
}                                                                                      // end of abs


// int atexit(void (* func)(void))
// /**************************************************************************************************/
// /* int atexit(void (* func)(void))                                                                */
// /**************************************************************************************************/
// {
// int x;
//
// for (x = 0; x < __NATEXIT; x++) {
//    if (__userExit[x] == 0) {
//       __userExit[x] = func;
//       return 0;
//       }
//    }
// return -1;
// }                                                                                   // end of atexit


double
atof(const char *nptr)
/**************************************************************************************************/
/* double atof(const char *nptr)                                                                  */
/**************************************************************************************************/
{
return (strtod(nptr, (char **) NULL));
}                                                                                     // end of atof


int
atoi(const char * nptr)
/**************************************************************************************************/
/* int atoi(const char * nptr)                                                                    */
/**************************************************************************************************/
{
return ((int) strtol(nptr, (char **) NULL, 10));
}                                                                                     // end of atoi


long int
atol(const char * nptr)
/**************************************************************************************************/
/* long int atol(const char * nptr)                                                               */
/**************************************************************************************************/
{
return (strtol(nptr, (char **) NULL, 10));
}                                                                                     // end of atol


void *
bsearch(const void * key, const void * base, size_t nmemb, size_t size,
   int (* compar) (const void *, const void *))
/**************************************************************************************************/
/* void * bsearch(const void * key, const void * base, size_t nmemb, size_t size,                 */
/*   int (* compar) (const void *, const void *))                                                 */
/**************************************************************************************************/
{
size_t try;
int res;
const void *ptr;

while (nmemb > 0) {
   try = nmemb / 2;
   ptr = (void *)((char *)base + try * size);
   res = compar(ptr, key);
   if (res == 0) return ((void *)ptr);
   else if (res < 0) {
      nmemb = nmemb - try - 1;
      base = (const void *)((const char *)ptr + size);
      }
   else nmemb = try;
   }
return NULL;
}                                                                                  // end of bsearch

div_t
div(int numer, int denom)
/**************************************************************************************************/
/* div_t div(int numer, int denom)                                                                */
/**************************************************************************************************/
{
div_t x;

x.quot = numer / denom;
x.rem = numer % denom;
return x;
}                                                                                      // end of div


void
exit(int status)
/**************************************************************************************************/
/* void exit(int status)                                                                          */
/**************************************************************************************************/
{
  /* TODO call exit functions! */
  GETGCCCRAB()->exitfunc(status);
}                                                                                     // end of exit

char * getenv(const char *name)
/**************************************************************************************************/
/* char * getenv(const char *name)                                                                */
/**************************************************************************************************/
{
return NULL;
}                                                                                   // end of getenv


#ifdef labs
#undef labs
#endif
long int labs(long int j)
/**************************************************************************************************/
/* long int labs(long int j)                                                                      */
/**************************************************************************************************/
{
if (j < 0) j = -j;
return j;
}                                                                                     // end of labs


ldiv_t ldiv(long int numer, long int denom)
/**************************************************************************************************/
/* ldiv_t ldiv(long int numer, long int denom)                                                    */
/**************************************************************************************************/
{
ldiv_t x;

x.quot = numer / denom;
x.rem = numer % denom;
return x;
}                                                                                     // end of ldiv

int
mblen(const char * s, size_t n)
/**************************************************************************************************/
/* int mblen(const char * s, size_t n)                                                            */
/**************************************************************************************************/
{
if (s == NULL) return 0;
if (n == 1) return 1;
else return -1;
}                                                                                    // end of mblen


size_t
mbstowcs(wchar_t * pwcs, const char * s, size_t n)
/**************************************************************************************************/
/* size_t mbstowcs(wchar_t * pwcs, const char * s, size_t n)                                      */
/**************************************************************************************************/
{
strncpy((char *) pwcs, s, n);
if (strlen(s) >= n) return n;
return strlen((char *) pwcs);
}                                                                                 // end of mbstowcs


int
mbtowc(wchar_t * pwc, const char * s, size_t n)
/**************************************************************************************************/
/* int mbtowc(wchar_t * pwc, const char * s, size_t n)                                            */
/**************************************************************************************************/
{
if (s == NULL) return 0;
if (n == 1) {
   if (pwc != NULL) * pwc = * s;
   return 1;
   }
else
   return -1;
}                                                                                   // end of mbtowc


void
qsort(void * base, size_t nmemb, size_t size, int (* compar)(const void *, const void *))
/**************************************************************************************************/
/* void qsort(void * base, size_t nmemb, size_t size, int (* compar)(const void *, const void *)) */
/*                                                                                                */
/* This qsort function does a little trick:  to reduce stackspace it iterates the larger interval */
/* instead of doing the recursion on both intervals.  So stackspace is limited to                 */
/*    32 * stack_for_1_iteration =                                                                */
/*    32 * 4 * (4 arguments + 1 returnaddress + 11 stored registers) = 2048 bytes.                */
/* This should be small enough to keep everyone happy.                                            */
/**************************************************************************************************/
{
char * base2 = (char *) base;
size_t i,a,b,c;

while(nmemb > 1) {
   a = 0;
   b = nmemb-1;
   c = (a + b) / 2;                                                                // middle element
   for(;;) {
      while((* compar) (&base2[size * c], &base2[size * a]) > 0)
         a++;                                                              // look for one >= middle
      while((* compar) (&base2[size * c], &base2[size * b]) < 0)
         b--;                                                              // look for one <= middle
      if (a >= b) break;                                                         // we found no pair
      for (i = 0; i < size; i++) {                                                      // swap them
         char tmp = base2[size * a + i];
         base2[size * a + i] = base2[size * b + i];
         base2[size * b + i] = tmp;
         }
      if (c == a) c = b;                                             // keep track of middle element
      else if(c == b) c = a;
      a++;                                                           // these two are already sorted
      b--;
      }                       // a points to first element of right interval now (b to last of left)
   b++;
   if (b < nmemb - b) {              // do recursion on smaller interval and iteration on larger one
      qsort(base2, b, size, compar);
      base2 = &base2[size * b];
      nmemb = nmemb - b;
      }
   else {
      qsort(&base2[size * b], nmemb - b, size, compar);
      nmemb = b;
      }
   }
}                                                                                    // end of qsort

int
rand(void)
/**************************************************************************************************/
/* int rand(void)                                                                                 */
/**************************************************************************************************/
{
int ret;

#if defined(__MVS__) || defined(__CMS__)
// This hack should be removed. It is to get around a bug in the GCC 3.2.3 MVS 3.0 optimizer.
myseed = myseed * 1103515245UL;
ret = (int)(((myseed + 12345) >> 16) & 0x8fff);
#else
myseed = myseed * 1103515245UL + 12345;
ret = (int)((myseed >> 16) & 0x8fff);
#endif
return ret;
}                                                                                     // end of rand


void
srand(unsigned int seed)
/**************************************************************************************************/
/* void srand(unsigned int seed)                                                                  */
/**************************************************************************************************/
{
myseed = seed;
return;
}                                                                                    // end of srand


double
strtod(const char * nptr, char ** endptr)
/**************************************************************************************************/
/* double strtod(const char * nptr, char ** endptr)                                               */
/**************************************************************************************************/
{
double x = 0.0;
double xs= 1.0;
double es = 1.0;
double xf = 0.0;
double xd = 1.0;

while(isspace((unsigned char) * nptr)) ++nptr;
if (* nptr == '-') {
   xs = -1;
   nptr++;
   }
else if(* nptr == '+') nptr++;

while (1) {
   if (isdigit((unsigned char) * nptr)) {
      x = x * 10 + (* nptr - '0');
      nptr++;
      }
   else {
      x = x * xs;
      break;
      }
   }
if (* nptr == '.') {
   nptr++;
   while (1) {
      if (isdigit((unsigned char) * nptr)) {
         xf = xf * 10 + (* nptr - '0');
         xd = xd * 10;
         }
      else {
         x = x + xs * (xf / xd);
         break;
         }
      nptr++;
      }
   }
if ((* nptr == 'e') || (* nptr == 'E')) {
   nptr++;
   if (* nptr == '-') {
      es = -1;
      nptr++;
      }
   xd = 1;
   xf = 0;
   while (1) {
      if (isdigit((unsigned char) * nptr)) {
         xf = xf * 10 + (*nptr - '0');
         nptr++;
         }
      else {
         while (xf > 0) {
            xd *= 10;
            xf--;
            }
         if (es < 0.0) x = x / xd;
         else x = x * xd;
         break;
         }
      }
   }
if (endptr != NULL) * endptr = (char *) nptr;
return (x);
}                                                                                   // end of strtod


long int
strtol(const char * nptr, char ** endptr, int base)
/**************************************************************************************************/
/* long int strtol(const char * nptr, char ** endptr, int base)                                   */
/**************************************************************************************************/
{
unsigned long y;
long x;
int neg = 0;

while(isspace((unsigned char) * nptr)) ++nptr;
if (* nptr == '-') {
   neg = 1;
   nptr++;
   }
else if (* nptr == '+') nptr++;
y = strtoul(nptr, endptr, base);
if (neg) x = (long)-y;
else x = (long)y;
return x;
}                                                                                   // end of strtol


unsigned long int
strtoul(const char * nptr, char ** endptr, int base)
/**************************************************************************************************/
/* unsigned long int strtoul(const char * nptr, char ** endptr, int base)                         */
/*                                                                                                */
/* This logic is also in vvscanf - if you update this, update that one too.                       */
/**************************************************************************************************/
{
unsigned long x = 0;
int undecided = 0;

if (base == 0) undecided = 1;
while(isspace((unsigned char) * nptr)) ++nptr;
while (1) {
   if (isdigit((unsigned char) * nptr)) {
      if (base == 0) {
         if (* nptr == '0') base = 8;
         else {
            base = 10;
            undecided = 0;
            }
         }
      x = x * base + (*nptr - '0');
      nptr++;
      }
   else if (isalpha((unsigned char) * nptr)) {
      if ((* nptr == 'X') || (* nptr == 'x')) {
         if ((base == 0) || ((base == 8) && undecided)) {
            base = 16;
            undecided = 0;
            nptr++;
            }
         else if (base == 16) nptr++;               // hex values are allowed to have an optional 0x
         else break;
         }
      else if (base <= 10) break;
      else {
         x = x * base + (toupper((unsigned char) * nptr) - 'A') + 10;
         nptr++;
         }
      }
   else break;
   }
if (endptr != NULL) * endptr = (char *) nptr;
return (x);
}                                                                                  // end of strtoul


int
system(const char * s)
/**************************************************************************************************/
/* int system(const char * s)                                                                     */
/**************************************************************************************************/
{
return CMScommand(s, CMS_COMMAND);
}                                                                                   // end of system


size_t
wcstombs(char * s, const wchar_t * pwcs, size_t n)
/**************************************************************************************************/
/* size_t wcstombs(char * s, const wchar_t * pwcs, size_t n)                                      */
/**************************************************************************************************/
{
strncpy(s, (const char *) pwcs, n);
if (strlen((const char *) pwcs) >= n) return n;
return strlen(s);
}                                                                                  // end of wctombs


int
wctomb(char * s, wchar_t wchar)
/**************************************************************************************************/
/* int wctomb(char * s, wchar_t wchar)                                                            */
/**************************************************************************************************/
{
if (s != NULL) {
   * s = wchar;
   return 1;
   }
else return 0;
}                                                                                   // end of wctomb
