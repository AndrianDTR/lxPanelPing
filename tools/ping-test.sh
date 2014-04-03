#!/bin/bash

TITLE=$1
shift

STATUS="<span color=\"#FF0000\"><b>$TITLE</b></span>"
ping $*
RETVAL=$?
if [ $RETVAL -eq 0 ]; then
	STATUS="<span color=\"#00FF00\"><b>$TITLE</b></span>"
    #sshpass -p $4 ssh $3@$1 -p $2 $CMD
fi

echo $STATUS

exit 0