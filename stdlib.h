/**************************************************************************************************/
/* Native CMS implementation of STDLIB.H.                                                         */
/*                                                                                                */
/* Robert O'Hara, Redmond Washington, May 2009                                                    */
/*                                                                                                */
/* Based code written by Paul Edwards.                                                            */
/* Released to the public domain.                                                                 */
/**************************************************************************************************/
#ifndef __STDLIB_INCLUDED
#define __STDLIB_INCLUDED

#ifndef __SIZE_T_DEFINED
#define __SIZE_T_DEFINED
typedef unsigned long size_t;
#endif

#ifndef __WCHAR_T_DEFINED
#define __WCHAR_T_DEFINED
#ifndef _WCHAR_T_DEFINED
#define _WCHAR_T_DEFINED
#endif
typedef char wchar_t;
#endif

typedef struct { int quot; int rem; } div_t;
typedef struct { long int quot; long int rem; } ldiv_t;

#define NULL ((void *)0)
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 12
#define RAND_MAX 32767
#define MB_CUR_MAX 1
#define __NATEXIT 32

extern void (*__userExit[__NATEXIT])(void);
/**************************************************************************************************/
/* __userExit                                                                                     */
/**************************************************************************************************/

void abort(void);
/**************************************************************************************************/
/* abort                                                                                          */
/**************************************************************************************************/

int abs(int j);
/**************************************************************************************************/
/* abs                                                                                            */
/**************************************************************************************************/

int atexit(void (*func)(void));
/**************************************************************************************************/
/* atexit                                                                                         */
/**************************************************************************************************/

double atof(const char *nptr);
/**************************************************************************************************/
/* atof                                                                                           */
/**************************************************************************************************/

int atoi(const char *nptr);
/**************************************************************************************************/
/**************************************************************************************************/

long int atol(const char *nptr);

void *bsearch(const void *key, const void *base, size_t nmemb, size_t size,
   int (*compar)(const void *, const void *));

void * calloc(size_t nmemb, size_t size);

div_t div(int numer, int denom);

#define exit(i1) (__exit((i1)))

void free(void * ptr);

char * getenv(const char *name);

long int labs(long int j);

ldiv_t ldiv(long int numer, long int denom);

void * malloc(size_t size);

int mblen(const char *s, size_t n);

size_t mbstowcs(wchar_t *pwcs, const char *s, size_t n);

int mbtowc(wchar_t *pwc, const char *s, size_t n);

void qsort(void *, size_t, size_t, int (*)(const void *, const void *));

int rand(void);

void * realloc(void *ptr, size_t size);

void srand(unsigned int seed);

double strtod(const char *nptr, char **endptr);

long int strtol(const char *nptr, char **endptr, int base);

unsigned long int strtoul(const char *nptr, char **endptr, int base);

int system(const char * s);

size_t wcstombs(char *s, const wchar_t *pwcs, size_t n);

int wctomb(char *s, wchar_t wchar);

#endif
