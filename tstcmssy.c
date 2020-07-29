/**************************************************************************************************/
/* GCCLIB Test Suite - CMSSYS                                                                     */
/*                                                                                                */
/* Part of GCCLIB - VM/370 CMS Native Std C Library; A Historic Computing Toy                     */
/*                                                                                                */
/* Contributors: See contrib memo                                                                 */
/*                                                                                                */
/* Released to the public domain.                                                                 */
/**************************************************************************************************/
#define __FILENAME__ "TSTCMSSY"

#include <cmssys.h>
#include <stdio.h>
#include <stdlib.h>
#include <cmssys.h>

#include "tsts.h"

static void
mem_init(unsigned char* ptr, unsigned long size)
{
  unsigned long i, j;

  if (size == 0) return;
  for (i = 0; i < size; i += 2047) {
    j = (unsigned long)ptr ^ i;
    ptr[i] = ((j ^ (j >> 8)) & 0xFF);
  }
  j = (unsigned long)ptr ^ (size - 1);
  ptr[size - 1] = ((j ^ (j >> 8)) & 0xFF);
}

static int
mem_check(unsigned char* ptr, unsigned long size)
{
  unsigned long i, j;

  if (size == 0) return 0;
  for (i = 0; i < size; i += 2047) {
    j = (unsigned long)ptr ^ i;
    if (ptr[i] != ((j ^ (j >> 8)) & 0xFF)) return 1;
  }
  j = (unsigned long)ptr ^ (size - 1);
  if (ptr[size - 1] != ((j ^ (j >> 8)) & 0xFF)) return 2;
  return 0;
}

/**************************************************************************************************/
/* Two functions to help C programs avoid having any non-const static or global                   */
/* variables. For programs planning to be run from a shared segment the program they will be      */
/* running in read-only memory and the CMS linker does not allow global variables to be placed in */
/* another segment.                                                                               */
/*                                                                                                */
/* IBM calls these programs (ones that don't write to their TEXT segment) reentrant programs.     */
/* I have coined this memory - Process Global Memory.                                             */
/*                                                                                                */
/* void* CMSPGAll(size_t size) - Allocate / Reallocate Process Global Memory Block                */
/* void* CMSGetPG(void) - Get the address of the Process Global Memory Block                      */
/*                                                                                                */
/* Note: that this area is freed automatically on normal program termination.                     */
/*                                                                                                */
/**************************************************************************************************/
static void cmspg_t() {

  SUB_STRT("CMSPGAll() and CMSGetPG()");
  unsigned char *buffer1;
  unsigned char *buffer2;
  size_t size1 = 145;
  size_t size2 = 379;

  ASSERTNOTNULL("CMSPGAll(size1)", buffer1 = CMSPGAll(size1), buffer1, );
  mem_init(buffer1, size1);
  ASSERTZERO("mem_check(size1)", , mem_check(buffer1, size1), );
  ASSERTNOTZERO("CMSGetPG()", buffer2 = CMSGetPG(), buffer1 == buffer2, );

  ASSERTNOTNULL("CMSPGAll(size2)", buffer1 = CMSPGAll(size2), buffer1, );
  mem_init(buffer1, size2);
  ASSERTZERO("mem_check(size2)", , mem_check(buffer1, size2), );
  ASSERTNOTZERO("CMSGetPG()", buffer2 = CMSGetPG(), buffer1 == buffer2, );

  ASSERTNULL("CMSPGAll(0)", buffer1 = CMSPGAll(0), buffer1, );

  ASSERTNOTNULL("CMSPGAll(size1)", buffer1 = CMSPGAll(size1), buffer1, );
  mem_init(buffer1, size1);
  ASSERTZERO("mem_check(size1)", , mem_check(buffer1, size1), );
  ASSERTNOTZERO("CMSGetPG()", buffer2 = CMSGetPG(), buffer1 == buffer2, );

}

void CMSSYS_T() {
  SEC_STRT("CMSSYS Tests");
  cmspg_t();
}
