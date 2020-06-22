#!/bin/sh
# Install GCCLIB on CMS

# Exit if there is an error
set -e

# IPL
herccontrol -v
herccontrol "ipl 141" -w "USER DSC LOGOFF AS AUTOLOG1"

# LOGON GCCCMS AND READ TAPE
herccontrol "/cp disc" -w "^VM/370 Online"
herccontrol "/logon gcccms gcccms" -w "^CMS VERSION"
herccontrol "/" -w "^Ready;"
herccontrol "devinit 480 io/gcclibbin.aws" -w "^HHCPN098I"
herccontrol "/attach 480 to gcccms as 181" -w "TAPE 480 ATTACH"

# GCC 202 Disk - Headers
herccontrol "/access 202 h" -w "^Ready;"
herccontrol "/erase * h h" -w "^Ready;"
herccontrol "/erase * maclib h" -w "^Ready;"
herccontrol "/erase * parm h" -w "^Ready"
herccontrol "/tape load * h h (noprint" -w "^Ready;"
herccontrol "/tape rew" -w "^Ready;"
herccontrol "/tape load * maclib h (noprint" -w "^Ready;"
herccontrol "/tape rew" -w "^Ready;"
herccontrol "/tape load * parm h (noprint" -w "^Ready;"
herccontrol "/detach 181" -w "^Ready;"
herccontrol "/rename * * h = = h1" -w "^Ready;"
herccontrol "/release h"  -w "^Ready;"

# LOGON MAINT AND READ TAPE
herccontrol "/cp disc" -w "^VM/370 Online"
herccontrol "/logon maint cpcms" -w "^CMS VERSION"
herccontrol "/" -w "^Ready;"
herccontrol "devinit 480 io/gcclibbin.aws" -w "^HHCPN098I"
herccontrol "/attach 480 to maint as 181" -w "TAPE 480 ATTACH"

# MAINT 19D DISK
herccontrol "/access 19D z" -w "^Ready;"
herccontrol "/tape load * memo z (noprint" -w "^Ready;"
herccontrol "/tape rew" -w "^Ready;"
herccontrol "/tape load * helpcmd z (noprint" -w "^Ready;"
herccontrol "/tape rew" -w "^Ready;"
herccontrol "/tape load * helpcmd2 z (noprint" -w "^Ready;"
herccontrol "/tape rew" -w "^Ready;"
herccontrol "/release z"  -w "^Ready;"

# MAINT 190 CMS SYSTEM DISK
herccontrol "/access 190 z" -w "^Ready;"
herccontrol "/tape load * maclib z (noprint" -w "^Ready;"
herccontrol "/tape rew" -w "^Ready;"
herccontrol "/tape load * txtlib z (noprint" -w "^Ready;"
herccontrol "/tape rew" -w "^Ready;"
herccontrol "/tape load * text z (noprint" -w "^Ready;"
herccontrol "/tape rew" -w "^Ready;"
herccontrol "/tape load sysprof exec z (noprint" -w "^Ready;"
herccontrol "/tape rew" -w "^Ready;"
herccontrol "/release z"  -w "^Ready;"

# Done with tape
herccontrol "/detach 181" -w "^Ready;"

# Regenerate System
herccontrol "/define storage 16m"  -w "^CP ENTERED"
herccontrol "/ipl 190" -w "^CMS VERSION"
herccontrol "/access ( noprof" -w "^Ready;"
herccontrol "/access 093 b" -w "^Ready;"
herccontrol "/access 193 c" -w "^Ready;"
herccontrol "/cmsxgen f00000 cmsseg" -w "^Ready;"
herccontrol "/ipl 190" -w "^CMS VERSION"
herccontrol "/savesys cms" -w "^CMS VERSION"
herccontrol "/" -w "^Ready;"
herccontrol "/logoff" -w "^VM/370 Online"

# LOGON CMSUSER
herccontrol "/logon cmsuser cmsuser" -w "^CMS VERSION"
herccontrol "/" -w "^Ready;"

# Sanity test
herccontrol "/listf gcclib * *" -w "^Ready;"
herccontrol "/listf gccres * *" -w "^Ready;"

# LOGOFF
herccontrol "/logoff" -w "^VM/370 Online"

# SHUTDOWN
herccontrol "/logon operator operator" -w "RECONNECTED AT"
herccontrol "/shutdown" -w "^HHCCP011I"
