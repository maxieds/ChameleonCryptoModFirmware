#!/bin/bash

#### ChameleonUploadEncrypted.sh : Upload an encrypted dump image into the current slot
#### Author: Maxie D. Schmidt (maxieds@gmail.com)
#### Created: 2019.02.20

if [[ -z "$1" || -z "$2" || -z "$3" ]]; then
	echo "Usage: $0 <KeyIndex> <TimestampSalt> <DumpImageFilePath.dmp> [--reset]";
	exit -1;
fi

CHAMELEON_DEV=/dev/ttyACM0;
KEY_INDEX=$1;
TIMESTAMP_SALT=$2;
DUMP_IMAGE=$3;
XMODEM_PACKET_SIZE=128; #`wc -c < $DUMP_IMAGE`;

RESETCMD=`[[ -z "$4" && "$4" == "--reset" ]] && echo "1" || echo "0"`;
if [[ "$RESETCMD" != "0" ]]; then
	echo "Resetting the device ..."
	echo -ne "RESET\r" > $CHAMELEON_DEV;
	read -t 2 NoVar < $CHAMELEON_DEV;
fi

#SZCMD=`[[ -e "ChameleonCryptoUtils/UtilityBin/lsz" ]] && \
#	echo "ChameleonCryptoUtils/UtilityBin/lsz" || \
#	echo "../ChameleonCryptoUtils/UtilityBin/lsz"`;
SZCMD=`which sx`

stty -F $CHAMELEON_DEV 115200 cs8 -cstopb -parenb # 9600 #cs8 -parenb -cstopb -ixon
echo -en "UPLOAD_ENCRYPTED ${KEY_INDEX} ${TIMESTAMP_SALT}\r" > $CHAMELEON_DEV;
read -t 4 NoVar < $CHAMELEON_DEV;
echo " >> Chameleon Response: $NoVar"
$SZCMD -b -vvv -X $DUMP_IMAGE < $CHAMELEON_DEV > $CHAMELEON_DEV
#echo -ne "\x04" > $CHAMELEON_DEV;
#sleep 2;

read -t 0 NoVar < $CHAMELEON_DEV;
#sleep 3;
#echo -en "UPLOAD_STATUS\r" > $CHAMELEON_DEV;
#read NoVar < $CHAMELEON_DEV;
#read -t 0 TestOut < $CHAMELEON_DEV;
#echo " >> Chameleon Response: $NoVar [`xxd $TestOut`]"

exit 0;
