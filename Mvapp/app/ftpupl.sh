#!/bin/bash

filename="mvapp.elf"
hostname="192.168.1.23"
username="root"
password="chipbox"
ftp -inv $hostname << EOF
quote USER $username
quote PASS $password
binary
cd /usr/work0/app
put $filename
chmod 777 mvapp.elf
quit

EOF
