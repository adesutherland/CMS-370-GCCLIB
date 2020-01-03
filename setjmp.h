/**************************************************************************************************/
/* SETJMP.H: C runtime definitions of setjmp() and longjmp().                                     */
/*                                                                                                */
/* Based on code written by Paul Edwards and Dave Wade.  Released to the public domain.           */
/*                                                                                                */
/* Robert O'Hara, Redmond Washington, March 2009                                                  */
/**************************************************************************************************/
#ifndef SETJMP_INCLUDED
#define SETJMP_INCLUDED

/**************************************************************************************************/
/* void longjmp(jmp_buf envbuf, int status)                                                       */
/*                                                                                                */
/* Start execution at the point of the last call to setjmp(), which stored the needed program     */
/* state information in 'envbuf'.                                                                 */
/*    envbuf    stores program state information.  Use setjmp() to set this variable, and pass    */
/*              the saved value to longjmp().                                                     */
/*    status    is the value that setjmp() will return.  It can be used to figure out where       */
/*              longjmp() came from, and should not be set to zero.                               */
/*                                                                                                */
/* Returns:                                                                                       */
/*    nothing.                                                                                    */
/**************************************************************************************************/

/**************************************************************************************************/
/* int setjmp(jmp_buf envbuf)                                                                     */
/*                                                                                                */
/* Set execution to start at a certain point in the program.  Program state information is saved  */
/* in 'envbuf' for use by a later call to longjmp().  When you first call setjmp(), its return    */
/* value is zero.  Later, when you call longjmp(), the second argument of longjmp() is what the   */
/* return value of setjmp() will be.                                                              */
/*    envbuf    stores program state information.  Use setjmp() to set this variable, and pass    */
/*              the saved value to longjmp().                                                     */
/*                                                                                                */
/* Returns:                                                                                       */
/*    (int)     0 on a call to setjmp(), 'status' on a call from longjmp().                       */
/**************************************************************************************************/

typedef struct {
    int registers[15];                                                  /* we save registers 0-14 */
} jmp_buf[1];
#endif
