#!/bin/bash

filename="oscam-chipbox"
hostname="192.168.1.23"
username="root"
password="chipbox"
ftp -inv $hostname << EOF
quote USER $username
quote PASS $password
binary
cd /usr/work1/
put $filename
chmod 777 $filename
quit 
EOF