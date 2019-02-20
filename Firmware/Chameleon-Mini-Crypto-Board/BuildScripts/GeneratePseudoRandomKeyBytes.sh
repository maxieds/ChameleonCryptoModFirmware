#!/bin/bash

#### GeneratePseudoRandomKeyBytes.sh : 
#### Generates a C/C++-style byte array representing (semi) randomized 
#### key data generated with a PRG (i.e., /dev/urandom) and the 
#### openssl command;
#### Author: Maxie D. Schmidt (maxieds@gmail.com)
#### Created: 2019.02.20
#### 
#### Usage (AES-128): ./GeneratePseudoRandomKeyBytes.sh 16 
#### Usage (AES-256): ./GeneratePseudoRandomKeyBytes.sh 256 --bit-size

function GetRandomizedByteString {
	if [ -z "$1" ]; then
		echo "GetRandomizedByteString: Invalid parameter specification ...";
		exit -1;
	fi
	aesKeyByteSize=$1;
	prandomKeyData=$(openssl rand -rand /dev/urandom $aesKeyByteSize 2> /dev/null);
	prandomKeyData=$(echo $prandomKeyData | xxd -p);
	prandKeyBytes=$(echo $prandomKeyData | sed 's/.\{2\}/0x&, /g' | sed 's/, 0x0//g');
	truncLastComma=`[[ -z "$2" || "$2" == "0" ]] && echo "0" || echo "1"`;
	if [[ "$truncLastComma" != "0" ]]; then
		prandKeyBytes=$(echo $prandKeyBytes | sed 's/..$//');
	fi
	echo $prandKeyBytes;
}

## Default to AES-128 key size (16 bytes, or 32 hexadecimal characters of key data):
AESKEY_BYTES=16;
KEYSIZE_PARAM=`[[ -z "$1" ]] && echo $AESKEY_BYTES || echo $1`;
if [ ! -z "${KEYSIZE_PARAM##*[!0-9]*}" ]; then
	KEYSIZE_SPEC_ARGS=`[[ -z "$2" ]] && echo "--byte-size" || echo $2`;
	if [[ "$KEYSIZE_SPEC_ARGS" == "--bit-size" ]]; then
		AESKEY_BYTES=$(($1 / 8));
	else
		AESKEY_BYTES=$1;
	fi
fi

BYTE_CHUNK_SIZE=8;
if [[ "$(($AESKEY_BYTES % $BYTE_CHUNK_SIZE))" != "0" ]]; then
	echo "$0: Error: Need specify a key size in multiples of $BYTE_CHUNK_SIZE";
	exit -2;
fi
KEY_CSTRING_BYTES=`GetRandomizedByteString $AESKEY_BYTES 1`;
echo $KEY_CSTRING_BYTES;

exit 0;
