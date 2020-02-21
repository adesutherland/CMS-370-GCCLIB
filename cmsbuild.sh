#!/bin/sh
# Make GCCLIB on CMS

# Exit if there is an error
set -e

# IPL
herccontrol -v
herccontrol "ipl 141" -w "USER DSC LOGOFF AS AUTOLOG1"
herccontrol "/cp start c" -w "RDR"

# LOGON CMSUSER
herccontrol "/cp disc" -w "^VM/370 Online"
herccontrol "/logon cmsuser cmsuser" -w "^CMS VERSION"
herccontrol "/" -w "^Ready;"

# Read/Send TATA TXT
yata -c
herccontrol -m >tmp; read mark <tmp; rm tmp
echo "USERID  CMSUSER\n:READ  YATA     TXT     " > tmp
cat yata.txt >> tmp
netcat -q 0 localhost 3505 < tmp
rm tmp
herccontrol -w "HHCRD012I" -f $mark
herccontrol "/" -w "RDR FILE"
herccontrol "/read *" -w "^Ready;"

# Build
herccontrol "/yata -x -d f" -w "^Ready;"
herccontrol "/erase yata txt a" -w "^Ready;"
herccontrol "/build" -w "^Ready;"
herccontrol "/rename * * e = = e2" -w "^Ready;"
herccontrol "/cleanup" -w "^Ready;"

# Sanity test
herccontrol "/copy * * e = = a" -w "^Ready;"
herccontrol "/ipl cms" -w "^CMS VERSION"
herccontrol "/" -w "^Ready;"
herccontrol "/mktest" -w "^Ready;"

# Make and load Tape
herccontrol "/cp disc" -w "^VM/370 Online"
herccontrol "/logon operator operator" -w "RECONNECTED AT"
hetinit -n -d gcclibbin.aws
herccontrol "devinit 480 io/gcclibbin.aws" -w "^HHCPN098I"
herccontrol "/attach 480 to cmsuser as 181" -w "TAPE 480 ATTACH TO CMSUSER"
herccontrol "/cp disc" -w "^VM/370 Online"

# Write to tape
herccontrol "/logon cmsuser cmsuser" -w "RECONNECTED AT"
herccontrol "/begin"
herccontrol "/tape dump * * e" -w "^Ready;"
herccontrol "/detach 181" -w "^Ready;"

# Clean Up
herccontrol "/erase gcclib * a" -w "^Ready;"
herccontrol "/erase gccres * a" -w "^Ready;"
herccontrol "/erase * h a" -w "^Ready;"
herccontrol "/erase test * a" -w "^Ready;"
herccontrol "/erase mktest exec a" -w "^Ready;"
herccontrol "/erase sysprof exec a" -w "^Ready;"
herccontrol "/cleane" -w "^Ready;"
herccontrol "/copy cleanf exec f = = a" -w "^Ready;"
herccontrol "/cleanf" -w "^Ready;"
herccontrol "/erase cleanf exec a" -w "^Ready;"

# LOGOFF
herccontrol "/logoff" -w "^VM/370 Online"

# SHUTDOWN
herccontrol "/logon operator operator" -w "RECONNECTED AT"
herccontrol "/shutdown" -w "^HHCCP011I"
