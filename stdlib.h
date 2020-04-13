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

div_t div(int numer, int denom);

void exit(int status);

char * getenv(const char *name);

long int labs(long int j);

ldiv_t ldiv(long int numer, long int denom);

int mblen(const char *s, size_t n);

size_t mbstowcs(wchar_t *pwcs, const char *s, size_t n);

int mbtowc(wchar_t *pwc, const char *s, size_t n);

void qsort(void *, size_t, size_t, int (*)(const void *, const void *));

int rand(void);

void srand(unsigned int seed);

double strtod(const char *nptr, char **endptr);

long int strtol(const char *nptr, char **endptr, int base);

unsigned long int strtoul(const char *nptr, char **endptr, int base);

int system(const char * s);

size_t wcstombs(char *s, const wchar_t *pwcs, size_t n);

int wctomb(char *s, wchar_t wchar);

/*
  malloc(size_t n)
  Returns a pointer to a newly allocated chunk of at least n bytes, or
  null if no space is available, in which case errno is set to ENOMEM
  on ANSI C systems.
*/
void* malloc(size_t bytes);

/*
  free(void* p)
  Releases the chunk of memory pointed to by p, that had been previously
  allocated using malloc or a related routine such as realloc.
  It has no effect if p is null. If p was not malloced or already
  freed, free(p) will by default cuase the current program to abort.
*/
void free(void* mem);

/*
  realloc(void* p, size_t n)
  Returns a pointer to a chunk of size n that contains the same data
  as does chunk p up to the minimum of (n, p's size) bytes, or null
  if no space is available.

  The returned pointer may or may not be the same as p. The algorithm
  prefers extending p in most cases when possible, otherwise it
  employs the equivalent of a malloc-copy-free sequence.

  If p is null, realloc is equivalent to malloc.

  If space is not available, realloc returns null, errno is set (if on
  ANSI) and p is NOT freed.

  if n is for fewer bytes than already held by p, the newly unused
  space is lopped off and freed if possible.  realloc with a size
  argument of zero (re)allocates a minimum-sized chunk.
*/
void* realloc(void* mem, size_t newsize);

/*
  calloc(size_t n_elements, size_t element_size);
  Returns a pointer to n_elements * element_size bytes, with all locations
  set to zero.
*/
void* calloc(size_t n_elements, size_t elem_size);

/*
  memalign(size_t alignment, size_t n);
  Returns a pointer to a newly allocated chunk of n bytes, aligned
  in accord with the alignment argument.

  The alignment argument should be a power of two. If the argument is
  not a power of two, the nearest greater power is used.
  8-byte alignment is guaranteed by normal malloc calls, so don't
  bother calling memalign with an argument of 8 or less.

  Overreliance on memalign is a sure way to fragment space.
*/
void* memalign(size_t alignment, size_t bytes);

/*
  icalloc (independent_calloc)
  independent_calloc(size_t n_elements, size_t element_size, void* chunks[]);

  independent_calloc is similar to calloc, but instead of returning a
  single cleared space, it returns an array of pointers to n_elements
  independent elements that can hold contents of size elem_size, each
  of which starts out cleared, and can be independently freed,
  realloc'ed etc. The elements are guaranteed to be adjacently
  allocated (this is not guaranteed to occur with multiple callocs or
  mallocs), which may also improve cache locality in some
  applications.

  The "chunks" argument is optional (i.e., may be null, which is
  probably the most typical usage). If it is null, the returned array
  is itself dynamically allocated and should also be freed when it is
  no longer needed. Otherwise, the chunks array must be of at least
  n_elements in length. It is filled in with the pointers to the
  chunks.

  In either case, independent_calloc returns this pointer array, or
  null if the allocation failed.  If n_elements is zero and "chunks"
  is null, it returns a chunk representing an array with zero elements
  (which should be freed if not wanted).

  Each element must be individually freed when it is no longer
  needed. If you'd like to instead be able to free all at once, you
  should instead use regular calloc and assign pointers into this
  space to represent elements.  (In this case though, you cannot
  independently free elements.)

  independent_calloc simplifies and speeds up implementations of many
  kinds of pools.  It may also be useful when constructing large data
  structures that initially have a fixed number of fixed-sized nodes,
  but the number is not known at compile time, and some of the nodes
  may later need to be freed.
*/
void** icalloc(size_t n_elements, size_t elem_size, void* chunks[]);
#define independent_calloc(a,b,c) icalloc((a),(b),(c))

/*
  icomalloc (independent_comalloc)
  independent_comalloc(size_t n_elements, size_t sizes[], void* chunks[]);

  independent_comalloc allocates, all at once, a set of n_elements
  chunks with sizes indicated in the "sizes" array.    It returns
  an array of pointers to these elements, each of which can be
  independently freed, realloc'ed etc. The elements are guaranteed to
  be adjacently allocated (this is not guaranteed to occur with
  multiple callocs or mallocs), which may also improve cache locality
  in some applications.

  The "chunks" argument is optional (i.e., may be null). If it is null
  the returned array is itself dynamically allocated and should also
  be freed when it is no longer needed. Otherwise, the chunks array
  must be of at least n_elements in length. It is filled in with the
  pointers to the chunks.

  In either case, independent_comalloc returns this pointer array, or
  null if the allocation failed.  If n_elements is zero and chunks is
  null, it returns a chunk representing an array with zero elements
  (which should be freed if not wanted).

  Each element must be individually freed when it is no longer
  needed. If you'd like to instead be able to free all at once, you
  should instead use a single regular malloc, and assign pointers at
  particular offsets in the aggregate space. (In this case though, you
  cannot independently free elements.)

  independent_comallac differs from independent_calloc in that each
  element may have a different size, and also that it does not
  automatically clear elements.

  independent_comalloc can be used to speed up allocation in cases
  where several structs or objects must always be allocated at the
  same time.
*/
void** icomalloc(size_t n_elements, size_t sizes[], void* chunks[]);
#define independent_comalloc(a,b,c) icomalloc((a),(b),(c))

/*
  footprint() returns the number of bytes obtained from the
  system for this application.
*/
size_t footprint();

/*
  mfootprint() returns the peak number of bytes obtained from the
  system for this application.
*/
size_t mfootprint();

/*
  mallinfo()
  Returns (by copy) a struct containing various summary statistics:

  arena:     current total non-mmapped bytes allocated from system
  ordblks:   the number of free chunks
  smblks:    always zero.
  hblks:     current number of mmapped regions
  hblkhd:    total bytes held in mmapped regions
  usmblks:   the maximum total allocated space. This will be greater
                than current total if trimming has occurred.
  fsmblks:   always zero
  uordblks:  current total allocated space (normal or mmapped)
  fordblks:  total free space
  keepcost:  the maximum number of bytes that could ideally be released
               back to system via malloc_trim. ("ideally" means that
               it ignores page restrictions etc.)

  Because these fields are ints, but internal bookkeeping may
  be kept as longs, the reported values may wrap around zero and
  thus be inaccurate.
*/
#ifndef STRUCT_MALLINFO_DECLARED
#define STRUCT_MALLINFO_DECLARED
struct mallinfo {
  size_t arena;    /* non-mmapped space allocated from system */
  size_t ordblks;  /* number of free chunks */
  size_t smblks;   /* always 0 */
  size_t hblks;    /* always 0 */
  size_t hblkhd;   /* space in mmapped regions */
  size_t usmblks;  /* maximum total allocated space */
  size_t fsmblks;  /* always 0 */
  size_t uordblks; /* total allocated space */
  size_t fordblks; /* total free space */
  size_t keepcost; /* releasable (via malloc_trim) space */
};
#endif /* __MALLINFO_DEFINED */

struct mallinfo mallinfo();

/*
  mallocus (malloc_usable_size);
  malloc_usable_size(void* p);

  Returns the number of bytes you can actually use in
  an allocated chunk, which may be more than you requested (although
  often not) due to alignment and minimum size constraints.
  You can use this many bytes without worrying about
  overwriting other allocated objects. This is not a particularly great
  programming practice. malloc_usable_size can be more useful in
  debugging and assertions
*/
size_t mallocus(const void* mem);
#define malloc_usable_size(a) mallocus((a))

/*
  mallocst (malloc_stats)
  malloc_stats();
  Prints on stderr the amount of space obtained from the system (both
  via sbrk and mmap), the maximum amount (which may be more than
  current if malloc_trim and/or munmap got called), and the current
  number of bytes allocated via malloc (or realloc, etc) but not yet
  freed. Note that this is the number of bytes allocated, not the
  number requested. It will be larger than the number requested
  because of alignment and bookkeeping overhead. Because it includes
  alignment wastage as being in use, this figure may be greater than
  zero even when no user-level chunks are allocated.

  The reported current and maximum system memory can be inaccurate if
  a program makes other calls to system memory allocation functions
  (normally sbrk) outside of malloc.

  malloc_stats prints only the most commonly interesting statistics.
  More information can be obtained by calling mallinfo.
*/
void mallocst();
#define malloc_stats() mallocst()

/*
  mtrim (malloc_trim)
  malloc_trim(size_t pad);

  If possible, gives memory back to the system (via negative arguments
  to sbrk) if there is unused memory at the `high' end of the malloc
  pool or in unused MMAP segments. You can call this after freeing
  large blocks of memory to potentially reduce the system-level memory
  requirements of a program. However, it cannot guarantee to reduce
  memory. Under some allocation patterns, some large free blocks of
  memory will be locked between two used chunks, so they cannot be
  given back to the system.

  The `pad' argument to malloc_trim represents the amount of free
  trailing space to leave untrimmed. If this argument is zero, only
  the minimum amount of memory to maintain internal data structures
  will be left. Non-zero arguments can be supplied to maintain enough
  trailing space to service future expected allocations without having
  to re-obtain memory from the system.

  Malloc_trim returns 1 if it actually released any memory, else 0.
*/
int mtrim(size_t pad);
#define malloc_trim(a) mtrim((a))

/*
  mallopt(int parameter_number, int parameter_value)
  Sets tunable parameters The format is to provide a
  (parameter-number, parameter-value) pair.  mallopt then sets the
  corresponding parameter to the argument value if it can (i.e., so
  long as the value is meaningful), and returns 1 if successful else
  0.  SVID/XPG/ANSI defines four standard param numbers for mallopt,
  normally defined in malloc.h.  None of these are use in this malloc,
  so setting them has no effect. But this malloc also supports other
  options in mallopt:

  Symbol            param #  default    allowed param values
  M_TRIM_THRESHOLD     -1   2*1024*1024   any   (-1U disables trimming)
  M_GRANULARITY        -2     page size   any power of 2 >= page size
  M_MMAP_THRESHOLD     -3      256*1024   any   (or 0 if no MMAP support)
*/
int mallopt(int param, int value);

#endif
