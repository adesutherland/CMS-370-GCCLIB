#!/bin/sh
# Make GCCLIB on CMS

# Exit if there is an error
set -e

# IPL
herccontrol "ipl 141" -w "USER DSC LOGOFF AS AUTOLOG1"
herccontrol "/cp start c" -w "RDR"
herccontrol "/cp start d class a" -w "PUN"

# LOGON MAINTC
herccontrol "/cp disc" -w "^VM/370 Online"
herccontrol "/logon maintc maintc" -w "^CMS"
herccontrol "/" -w "^Ready;"
herccontrol "/purge rdr" -w "^Ready;"
herccontrol "/ACCESS 394 A" -w "^Ready;"
herccontrol "/ERASE * * A1" -w "^Ready;"

# Send Source Code - including tools
cp tools/* .
yata -c
herccontrol -m >tmp; read mark <tmp; rm tmp
echo "USERID  MAINTC\n:READ  ARCHIVE  YATA    " > tmp
cat archive.yata >> tmp
netcat -q 0 localhost 3505 < tmp
rm tmp
herccontrol -w "HHCRD012I" -f $mark
herccontrol "/" -w "RDR FILE"

# Prepare Source
herccontrol "/yata -x -f READER -d a" -w "^Ready;" -t 120
herccontrol "/COPYFILE * MACRO    A (RECFM F LRECL 80" -w "^Ready"
herccontrol "/COPYFILE * COPY     A (RECFM F LRECL 80" -w "^Ready"
herccontrol "/COPYFILE * ASSEMBLE A (RECFM F LRECL 80" -w "^Ready"
#herccontrol "/COPYFILE * EXEC     A (RECFM F" -w "^Ready"

# Make source tape and vmarc
herccontrol "/cp disc" -w "^VM/370 Online"
herccontrol "/logon operator operator" -w "RECONNECTED AT"
hetinit -n -d gcclibsrc.aws
herccontrol "devinit 480 io/gcclibsrc.aws" -w "^HHCPN098I"
herccontrol "/attach 480 to maintc as 181" -w "TAPE 480 ATTACH"
herccontrol "devinit 00d io/gcclibsrc.vmarc" -w "^HHCPN098I"
herccontrol "/cp disc" -w "^VM/370 Online"
herccontrol "/logon maintc maintc" -w "RECONNECTED AT"
herccontrol "/begin"
herccontrol "/tape dump * * a (noprint" -w "^Ready;"
herccontrol "/detach 181" -w "^Ready;"
herccontrol "/vmarc pack * * a (pun notrace" -w "^Ready;"

# Close and remove extra record from VMARC file
herccontrol "devinit 00d dummy" -w "^HHCPN098I"
truncate -s-80 gcclibsrc.vmarc

# Put tools in the T drive
herccontrol "/COPYFILE GCCASM EXEC A = = T (REPLACE" -w "^Ready"
herccontrol "/ERASE GCCASM EXEC A" -w "^Ready"

herccontrol "/COPYFILE GCCBUILD EXEC A = = T (REPLACE" -w "^Ready"
herccontrol "/ERASE GCCBUILD EXEC A" -w "^Ready"

herccontrol "/COPYFILE GCCCOMP EXEC A = = T (REPLACE" -w "^Ready"
herccontrol "/ERASE GCCCOMP EXEC A" -w "^Ready"

herccontrol "/COPYFILE GCCGEN EXEC A = = T (REPLACE" -w "^Ready"
herccontrol "/ERASE GCCGEN EXEC A" -w "^Ready"

herccontrol "/COPYFILE GCCGENM EXEC A = = T (REPLACE" -w "^Ready"
herccontrol "/ERASE GCCGENM EXEC A" -w "^Ready"

herccontrol "/COPYFILE GCCSRCH EXEC A = = T (REPLACE" -w "^Ready"
herccontrol "/ERASE GCCSRCH EXEC A" -w "^Ready"

herccontrol "/ipl cms" -w "^CMS"
herccontrol "/" -w "^Ready;"

herccontrol "/GCCBUILD" -w "^Ready;" -t 240
herccontrol "/GCCGENM" -w "^Ready;"

herccontrol "/ipl cms" -w "^CMS"
herccontrol "/" -w "^Ready;"

herccontrol "/GCCSRCH" -w "^Ready;"
herccontrol "/GCCGEN" -w "^Ready;"

herccontrol "/ipl cms" -w "^CMS"
herccontrol "/" -w "^Ready;"

# Make binary tape and vmarc
herccontrol "/cp disc" -w "^VM/370 Online"
herccontrol "/logon operator operator" -w "RECONNECTED AT"
hetinit -n -d gcclibbin.aws
herccontrol "devinit 480 io/gcclibbin.aws" -w "^HHCPN098I"
herccontrol "/attach 480 to maintc as 181" -w "TAPE 480 ATTACH"
herccontrol "devinit 00d io/gcclibbin.vmarc" -w "^HHCPN098I"
herccontrol "/cp disc" -w "^VM/370 Online"
herccontrol "/logon maintc maintc" -w "RECONNECTED AT"
herccontrol "/begin"
herccontrol "/access 194 e" -w "^Ready;"
herccontrol "/copyfile gcclib * a = = e" -w "^Ready;"
herccontrol "/copyfile gccres * a = = e" -w "^Ready;"
#herccontrol "/tape dump gcclib * a (noprint" -w "^Ready;"
#herccontrol "/tape dump gccres * a (noprint" -w "^Ready;"
herccontrol "/tape dump * * e (noprint" -w "^Ready;"
herccontrol "/detach 181" -w "^Ready;"
#herccontrol "/vmarc pack gcc* * a (pun notrace" -w "^Ready;"
herccontrol "/vmarc pack * * e (pun notrace" -w "^Ready;"

# Close and remove extra record from VMARC file
herccontrol "devinit 00d dummy" -w "^HHCPN098I"
truncate -s-80 gcclibbin.vmarc

# LOGOFF
herccontrol "/logoff" -w "^VM/370 Online"

# SHUTDOWN
herccontrol "/logon operator operator" -w "RECONNECTED AT"
herccontrol "/shutdown" -w "^HHCCP011I"
herccontrol "detach 09F0"
