#!/bin/sh
URL=192.168.1.134
KM="killall -s 9 mvapp.elf\n"
MVA="/usr/work0/app/mvapp.elf &\n"
EXIT="exit\n"

{
sleep 1
echo $KM
sleep 1
echo $EXIT
} | telnet $URL


$PWD/ftpupl.sh 

{
sleep 1
echo $MVA
sleep 1
echo $EXIT
} | telnet $URL

EOF
