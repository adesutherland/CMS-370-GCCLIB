/******************************************************************************/
/* REXXSAA.C - REXXSAA for BREXX on VM/370                                    */
/******************************************************************************/
#include <rexxsaa.h>
#include <cmssys.h>

/* Low Level Assembler functions */
int __SVC202(PLIST *plist, EPLIST *eplist, int calltype);

/*
  APIRET RexxVariablePool(PSHVBLOCK RequestBlockList)
  Variable Pool Interface - Application programs can use the Rexx Variable Pool
  Interface to manipulate the variables of a currently active Rexx procedure.

  The RequestBlockList is the first item in a chain of request blocks.

  Return code contains the return codes from the entire set of requests:
  The possible return codes are:
    POSITIVE Register 15 is the composite OR of bits 0-5 of the SHVRET bytes
    0 success
    -1 Incorrect entry conditions
    -2 Insufficient storage was available for a requested SET
    -3 No EXECCOMM entry point found

 Implentation Notes
  Call Type X'02'
  The R1 PLIST: Register 1 must point to a PLIST which consists
  of the 8-byte string EXECCOMM

  The R0 Extended PLIST
  - First word must contain the value of Register 1 (without the user call-type
    information in the high-order byte)
  - Second and third words must be identical (for example, both 0)
  - The fourth word points to a chain of request blocks
*/
APIRET RxVarPol(PSHVBLOCK RequestBlockList) {
    PLIST plist[2] = {"EXECCOMM",
                      "\xFF" "\xFF" "\xFF" "\xFF" "\xFF" "\xFF" "\xFF" "\xFF"};
    EPLIST eplist;
    eplist.Command = plist;
    eplist.BeginArgs = 0;
    eplist.EndArgs = 0;
    eplist.CallContext = RequestBlockList;
    eplist.ArgList = 0;
    eplist.FunctionReturn = 0;
    return __SVC202(plist, &eplist, 0x02);
}
