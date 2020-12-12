/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  errno.h - errno header file.                                     */
/*                                                                   */
/*********************************************************************/

#ifndef __ERRNO_INCLUDED
#define __ERRNO_INCLUDED

#include <gcccrab.h>

#define EPERM 10
#define ENOENT 20
#define EINTR 30
#define EIO 40
#define ENXIO 50
#define E2BIG 60
#define ENOEXEC 70
#define EBADF 80
#define EDEADLK 90
#define ENOMEM 100
#define EACCES 110
#define EFAULT 120
#define ENOTBLK 130
#define EBUSY 140
#define EEXIST 150
#define ENODEV 160
#define ENOTDIR 170
#define EISDIR 180
#define EINVAL 190
#define EMFILE 200
#define ENFILE 210
#define EFBIG 220
#define ENOSPC 230
#define ESPIPE 240
#define EROFS 250
#define EDOM 260
#define ERANGE 270
#define EAGAIN 280
#define ENAMETOOLONG 290
#define ESTALE 300
#define EFTYPE 310
#define EAUTH 320
#define ENOSYS 330
#define ENOTSUP 340
#define EGREGIOUS 350
#define EIEIO 360
#define EGRATUITOUS 370
#define EBADMSG 380
#define EIDRM 390
#define ENODATA 400
#define EOVERFLOW 410
#define ESTACKOVERFLOW 420 /* sync with dynstk.assemble */
#define EMALLOC 430

#define errno ((GETGCCCRAB()->gerrno))

#endif
