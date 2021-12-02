#!/bin/sh
grep "WORK_DIR = " config.mk -v > .temp.mk
mv .temp.mk config.mk
