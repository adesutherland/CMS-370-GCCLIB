/**************************************************************************************************/
/* Declare and intialize global variables for the GCCLIB runtime environment.                     */
/*                                                                                                */
/* Robert O'Hara, Redmond Washington, July 2010.                                                  */
/*                                                                                                */
/* Released to the public domain.                                                                 */
/**************************************************************************************************/
#define STDIO_DEFINED
#include "stdio.h"

#define GCCSTDIN 1
#define GCCSTDOUT 2
#define GCCSTDERR 3
FILE * stderr = GCCSTDERR;           // predefined stream for error output: we map it to the console
FILE * stdin = GCCSTDIN;           // predefined stream for standard input: we map it to the console
FILE * stdout = GCCSTDOUT;        // predefined stream for standard output: we map it to the console

void CMSinit(FILE * consoleInput, FILE * consoleOutput)
{
stdin = consoleInput;                                            // initialize the stdin file handle
stdout = stderr = consoleOutput;                    // initialize the stderr and stdout file handles
return;
}                                                                                  // end of CMSinit
