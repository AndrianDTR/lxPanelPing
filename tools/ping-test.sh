#!/bin/bash

echo "<span color=\"#FF0000\"><b>$1</b> - $2</span>"

exit 0

CMD=''
STATUS = "777777"
ping -w 1 $2
RETVAL=$?
if [ $RETVAL -eq 0 ]; then
	STATUS="FF7700"
    #sshpass -p $4 ssh $3@$1 -p $2 $CMD
fi

echo $STATUS

exit 0