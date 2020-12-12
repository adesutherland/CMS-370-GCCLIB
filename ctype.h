/**************************************************************************************************/
/* ctype.h - ctype header file                                                                    */
/*                                                                                                */
/* Part of GCCLIB - VM/370 CMS Native Std C Library; A Historic Computing Toy                     */
/*                                                                                                */
/* Contributors: See contrib memo                                                                 */
/*                                                                                                */
/* Released to the public domain.                                                                 */
/**************************************************************************************************/
#ifndef __CTYPE_INCLUDED
#define __CTYPE_INCLUDED

/* These Macros need reinstating for performance reasons
   This means that we need to be able to expose the RESLIB address of
   __issuff[] etc. to a client program

extern unsigned short *__isbuf;
extern short *__tolow;
extern short *__toup;

#define isalnum(c) (__isbuf[(c)] & 0x0001U)
#define isalpha(c) (__isbuf[(c)] & 0x0002U)
#define iscntrl(c) (__isbuf[(c)] & 0x0004U)
#define isdigit(c) (__isbuf[(c)] & 0x0008U)
#define isgraph(c) (__isbuf[(c)] & 0x0010U)
#define islower(c) (__isbuf[(c)] & 0x0020U)
#define isprint(c) (__isbuf[(c)] & 0x0040U)
#define ispunct(c) (__isbuf[(c)] & 0x0080U)
#define isspace(c) (__isbuf[(c)] & 0x0100U)
#define isupper(c) (__isbuf[(c)] & 0x0200U)
#define isxdigit(c) (__isbuf[(c)] & 0x0400U)
#define tolower(c) (__tolow[(c)])
#define toupper(c) (__toup[(c)])
*/

int isalnum(int c);

int isalpha(int c);

int iscntrl(int c);

int isdigit(int c);

int isgraph(int c);

int islower(int c);

int isprint(int c);

int ispunct(int c);

int isspace(int c);

int isupper(int c);

int isxdigit(int c);

int tolower(int c);

int toupper(int c);

#endif
