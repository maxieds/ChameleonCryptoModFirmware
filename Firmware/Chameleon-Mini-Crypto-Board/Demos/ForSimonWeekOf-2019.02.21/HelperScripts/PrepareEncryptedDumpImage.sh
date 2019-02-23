#!/bin/bash

#### PrepareEncryptedDumpImage.sh : Helper utility to create the encrypted dump images 
####                                from a stock MF1K dump image;
#### Author: Maxie D. Schmidt (maxieds@gmail.com)
#### Created: 2019.02.20

if [ -z "$1" ]; then
	echo "Usage: $0 <InputDumpImage.dmp> <KeyHexStringData> <TimestampSaltString>";
	exit -1;
fi

CRYPTO_HEADER_SIZE=32;
XMODEM_BLOCK_SIZE=128;
LOCAL_ENCDEC_UTIL=`[[ -e "UtilBin/EncodeDump" ]] && \
	echo "UtilBin/EncodeDump" || \
	echo "../UtilBin/EncodeDump"`;

INPUT_DUMP_IMAGE=$1;
KEY_DATA=`[[ -z "$2" ]] && echo "1234567890ABCDEF1234567890ABCDEF" || echo $2`;
SALT_DATA=`[[ -z "$3" ]] && echo "MyTimestampSalt1" || echo $3`;

## Remove all relevant existing files associated with this output dump image:
echo -e " >> Removing previous files ..."
DUMP_IMAGE_BASEPATH=$(echo $INPUT_DUMP_IMAGE | sed 's/\.[^.]*$//');
rm -f $INPUT_DUMP_IMAGE $DUMP_IMAGE_BASEPATH.{edmp,pdmp}

## Prepend a signature header prefix on the plaintext (POD) file:
echo -e " >> Creating dump data file \"${INPUT_DUMP_IMAGE}\" ..."
cat "${INPUT_DUMP_IMAGE}-dist" > $INPUT_DUMP_IMAGE;

## Append zero bytes (zerofill) the file to a byte size which is a multiple of the 
## XMODEM_BLOCK_SIZE (typically 128) defined above: 
#POD_FILE_SIZE=$(($(wc -c < $INPUT_DUMP_IMAGE) + $CRYPTO_HEADER_SIZE));
#ZERO_BYTE_COUNT=$(($XMODEM_BLOCK_SIZE - $(($POD_FILE_SIZE % $XMODEM_BLOCK_SIZE))));
#dd if=/dev/zero bs=1 count=$ZERO_BYTE_COUNT status=none conv=notrunc >> $INPUT_DUMP_IMAGE;
#EXT_FILE_SIZE=$(wc -c < $INPUT_DUMP_IMAGE);
#echo -e " >> Extended dump data file from"\
#	"${POD_FILE_SIZE} ($(($POD_FILE_SIZE - $CRYPTO_HEADER_SIZE))+${CRYPTO_HEADER_SIZE})"\
#	"to ${EXT_FILE_SIZE} bytes ..."

## Encrypt the resulting file and perform a sanity check on the result:
$LOCAL_ENCDEC_UTIL --encrypt --input-dump-image=$INPUT_DUMP_IMAGE \
	--key-data=$KEY_DATA --timestamp-salt=$SALT_DATA #> /dev/null;
$LOCAL_ENCDEC_UTIL --decrypt --input-dump-image=$INPUT_DUMP_IMAGE \
	--key-data=$KEY_DATA --timestamp-salt=$SALT_DATA #> /dev/null;

OUTPUT_PDUMP=$(echo $INPUT_DUMP_IMAGE | sed 's/\.dmp/\.pdmp/g')
DIFF_RESULT="!!<ERROR>!!";
if [ -e "$OUTPUT_PDUMP" ]; then
	DIFF_RESULT=$(diff $INPUT_DUMP_IMAGE $OUTPUT_PDUMP);
fi
echo $DIFF_RESULT
DIFF_STATUS=`[[ "$DIFF_RESULT" == "" ]] && echo "OK!" || echo "ERROR (X)"`;
echo " >> Sanity check on the encryption / decryption routine: ${DIFF_STATUS} ... "

echo -e " >> Terminating the utility for good!"
[[ "$DIFF_RESULT" == "" ]] && exit 0 || exit -2;
