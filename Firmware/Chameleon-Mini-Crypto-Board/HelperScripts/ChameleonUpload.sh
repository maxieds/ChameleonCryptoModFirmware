#!/bin/bash

#### ChameleonUpload.sh : Upload an unencrypted dump image into the current slot
#### Author: Maxie D. Schmidt (maxieds@gmail.com)
#### Created: 2019.02.20

if [ -z "$1" ]; then
	echo "Usage: $0 <DumpImageFilePath.dmp> [--reset]";
	exit -1;
fi

CHAMELEON_DEV=/dev/ttyACM0;
DUMP_IMAGE=$1;

RESETCMD=`[[ -z "$2" && "$2" == "--reset" ]] && echo "1" || echo "0"`;
if [[ "$RESETCMD" != "0" ]]; then
	echo "Resetting the device ...";
	echo -ne "RESET\r" > $CHAMELEON_DEV;
	sleep 3;
fi

stty -F $CHAMELEON_DEV 115200 cs8 -parenb -cstopb
echo -en "UPLOAD\r" > $CHAMELEON_DEV;
#read NoVar < $CHAMELEON_DEV;
sx -b -vvv -X $DUMP_IMAGE < $CHAMELEON_DEV > $CHAMELEON_DEV

exit 0;
