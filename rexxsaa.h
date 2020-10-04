/******************************************************************************/
/* REXXSAA.H - REXXSAA for BREXX on VM/370                                    */
/* This header file is source code compatible with REXXSAA for                */
/* the Rexx Variable Pool API, and is binary compatible with the CP/CMS       */
/* structures for EXECCOMM. To achieve this the order of fields in the        */
/* structs have been changed from the REXXSAA standard                        */
/******************************************************************************/
#ifndef __REXXSAA_H_INCLUDED
#define __REXXSAA_H_INCLUDED

typedef unsigned long ULONG;
typedef unsigned char UCHAR;
typedef void* PVOID;

typedef struct RXSTRING
{
  char* strptr;
  ULONG strlength;
} RXSTRING;
typedef RXSTRING *PRXSTRING;

#define APIRET ULONG
#define MAKERXSTRING(x,c,l)   ((x).strptr=(c),(x).strlength=(l))
#define RXNULLSTRING(x)       (!(x).strptr)
#define RXSTRLEN(x)           ((x).strptr ? (x).strlength : 0UL)
#define RXSTRPTR(x)           ((x).strptr)
#define RXVALIDSTRING(x)      ((x).strptr && (x).strlength)
#define RXZEROLENSTRING(x)    ((x).strptr && !(x).strlength)

/* shvret - return code flags */
#define RXSHV_OK       0x00    /* Everything OK */
#define RXSHV_NEWV     0x01    /* Var not previously set */
#define RXSHV_LVAR     0x02    /* Last var in a NEXTV sequence */
#define RXSHV_TRUNC    0x04    /* Name or value has been truncated */
#define RXSHV_BADN     0x08    /* Bad/invalid name */
#define RXSHV_MEMFL    0x10    /* Memory problem, e.g. out of memory *REXXSAA* */
#define RXSHV_BADV     0x10    /* Variable value invalid             *CP/CMS*  */
#define RXSHV_BADF     0x80    /* Invalid function code */
#define RXSHV_NOAVL    0x90    /* Interface is not available         *REXXSAA* */

/* shvcode - function codes */
#define RXSHV_SET      'S'    /* Set variable */
#define RXSHV_FETCH    'F'    /* Get value of variable */
#define RXSHV_DROPV    'D'    /* Drop variable */
#define RXSHV_SYSET    's'    /* Set symbolic variable */
#define RXSHV_SYFET    'f'    /* Get value of symbolic variable */
#define RXSHV_SYDRO    'd'    /* Drop symbolic variable */
#define RXSHV_NEXTV    'N'    /* Get next var in a NEXTV sequence */
#define RXSHV_PRIV     'P'    /* Get private information */

typedef struct SHVBLOCK
{
  struct SHVBLOCK *shvnext;      /* Ptr to next SHVBLOCK or 0 */
  ULONG           shvnamelen;    /* Length of name buffer, RXSHV_NEXTV only */
  UCHAR           shvcode;       /* Function code */
  UCHAR           shvret;        /* Return code flags */
  short           reserved;      /* 0 (16 bit) */
  ULONG           shvvaluelen;   /* Length of the value buffer */
  RXSTRING        shvname;       /* variable name */
  RXSTRING        shvvalue;      /* Value */
} SHVBLOCK;

typedef SHVBLOCK *PSHVBLOCK ;

/******************************************************************************/
/*
  APIRET RexxVariablePool(PSHVBLOCK RequestBlockList)
  Variable Pool Interface - Application programs can use the Rexx Variable Pool
  Interface to manipulate the variables of a currently active Rexx procedure.

  The RequestBlockList is the first item in a chain of request blocks.

  See notes at end for command details

  Return code contains the return codes from the entire set of requests:
  The possible return codes are:
    POSITIVE Register 15 is the composite OR of bits 0-5 of the SHVRET bytes
    0 success
    -1 Incorrect entry conditions
    -2 Insufficient storage was available for a requested SET
    -3 No EXECCOMM entry point found
*/
APIRET RxVarPol(PSHVBLOCK RequestBlockList);
#define RexxVariablePool(x) RxVarPol((x))
#define REXXVARIABLEPOOL(x) RxVarPol((x))
/******************************************************************************/

/******************************************************************************/
/* RexxVariablePool NOTES

*** D and d Drop variable.
The SHVNAMA/SHVNAML adlen describes the name of the
variable to be dropped. SHVVALA/SHVVALL are not used. The name is validated to
ensure that it does not contain incorrect characters, and the variable is then
dropped, if it exists. If the name given is a stem, all variables starting with
that stem are dropped.

*** F and f Fetch variable.
The SHVNAMA/SHVNAML adlen
describes the name of the variable to be fetched. SHVVALA specifies the address
of a buffer into which the data is to be copied, and SHVBUFL contains the length
of the buffer. The name is validated to ensure that it does not contain
incorrect characters, and the variable is then located and copied to the buffer.
The total length of the variable is put into SHVVALL, and, if the value was
truncated (because the buffer was not big enough), the SHVTRUNC bit is set. If
the variable is shorter than the length of the buffer, no padding takes place.
If the name is a stem, the initial value of that stem (if any) is returned.
SHVNEWV is set if the variable did not exist before the operation, and in this
case the value copied to the buffer is the derived name of the variable (after
substitution and so forth); see Compound Symbols.

*** N Fetch Next variable.
This function can search through all the variables known
to the language processor (that is, all those of the current generation,
excluding those hidden by PROCEDURE instructions). The order in which the
variables are revealed is not specified. The language processor maintains a
pointer to its list of variables: this is reset to point to the first variable
in the list whenever a host command is issued, or any function other than N or P
is run through the EXECCOMM interface.

Whenever an N (Next) function is run, the name and value of the next variable
available are copied to two buffers supplied by the caller.

SHVNAMA specifies the address of a buffer into which the name is to be copied,
and SHVUSER contains the length of that buffer. The total length of the name is
put into SHVNAML, and if the name was truncated (because the buffer was not big
enough) the SHVTRUNC bit is set. If the name is shorter than the length of the
buffer, no padding takes place. The value of the variable is copied to the
user's buffer area using exactly the same protocol as for the Fetch operation.

If SHVRET has SHVLVAR set, the end of the list of known variables has been
found, the internal pointers have been reset, and no valid data has been copied
to the user buffers. If SHVTRUNC is set, either the name or the value has been
truncated.

By repeatedly executing the N function (until the SHVLVAR flag is set) a user
program can locate all the REXX variables of the current generation.

*** P Fetch private information.
This interface is identical with the F fetch
interface, except that the name refers to certain fixed information items that
are available. Only the first letter of each name is checked (though callers
should supply the whole name), and the following names are recognized: ARG Fetch
primary argument string. The first argument string that would be parsed by the
ARG instruction is copied to the user's buffer. PARM Fetch the number of
argument strings. The number of argument strings supplied to the program is
placed in the caller's buffer. The number is formatted as a character string.
Note When specifying PARM, each letter must be supplied. PARM.n Fetch the nth
argument string. Argument string n is placed in the caller's buffer. Returns a
null string if argument string n cannot be supplied (whether omitted, null, or
fewer than n argument strings specified). Parm.1 returns the same result as ARG.
Note When specifying PARM.n, 'PARM.' must be supplied. SOURCE Fetch source
string. The source string, as described for PARSE SOURCE in PARSE, is copied to
the user's buffer. VERSION Fetch version string. The version string, as
described for PARSE VERSION in PARSE, is copied to the user's buffer. S and s
Set variable. The SHVNAMA/SHVNAML adlen describes the name of the variable to be
set, and SHVVALA/SHVVALL describes the value to be assigned to it. The name is
validated to ensure that it does not contain incorrect characters, and the
variable is then set from the value given. If the name is a stem, all variables
with that stem are set, just as though this was a REXX assignment. SHVNEWV is
set if the variable did not exist before the operation.
*/
#endif /* __REXXSAA_H_INCLUDED */
