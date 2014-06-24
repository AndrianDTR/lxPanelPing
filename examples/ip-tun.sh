#!/bin/bash

TITLE=$1
shift

STATUS="<span color=\"#$2\">$TITLE</span>"
XRES=`ifconfig $1`
RETVAL=$?
XIP=`ifconfig $1 | awk -F"[ :]+" '/inet addr/ {print $4}'`
if [ $RETVAL -eq 0 ]; then
	STATUS="<span color=\"#$3\">$XIP</span>"
fi
echo $STATUS
exit 0