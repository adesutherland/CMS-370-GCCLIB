/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  assert.c - implementation of stuff in assert.h                   */
/*                                                                   */
/*********************************************************************/
#include "assert.h"
#include "stdio.h"
#include "stdlib.h"

int __assert(char *x, char *y, int z)
{
#ifdef __CMS__
    printf("assertion failed for statement %s in file %s on line %d\n", x, y, z);
    exit(57);
#else
    fprintf(stderr, "assertion failed for statement %s in "
            "file %s on line %d\n", x, y, z);
    abort();
#endif
    return (0);
}
