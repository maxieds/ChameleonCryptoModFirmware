#!/bin/bash

#### GetUserFlashPassword.sh : Compile / make-time user input of a (custom) admin passphrase on the board;
#### Author: Maxie D. Schmidt (maxieds@gmail.com)
#### Created: 02.02.2019

DEFAULT_FLASH_LOCK_PWD="MyFlashLockPwd11";
USER_RESPONSE_TIMEOUT_SECONDS=0;
RANDOMIZE_DEFAULT_WITH_APG=0;
APGPasswordSuggestion="";

if [[ "$RANDOMIZE_DEFAULT_WITH_APG" != "0" ]]; then
     APG=$(which apg);
     RandomSeed=`read -n 16 URSEED < /dev/urandom && echo $URSEED`;
     APGPasswordSuggestion=$($APG -n 1 -a 1 -M NCL -m 8 -x 12 -c $RandomSeed);
     APGPasswordSuggestion="::"$APGPasswordSuggestion;
fi

echo -ne "Enter a backdoor administrator password for this build of the Chameleon Mini firmware ...\n";
echo -ne "(Press [ENTER] or let the script timeout to keep the default setting)\n";
echo -ne "DEFAULT PASSPHRASE > " $DEFAULT_FLASH_LOCK_PASSWORD$APGPasswordSuggestion "\n"
echo -ne "ENTER NEW PASSPHRASE NOW (WILL NOT ECHO) > ";
read -s -t $USER_RESPONSE_TIMEOUT_SECONDS UserInputFlashPassword;
echo -ne "\n\n"

if [[ "$UserInputFlashPassword" == "" ]]; then
     UserInputFlashPassword="$DEFAULT_FLASH_LOCK_PWD$APGPasswordSuggestion";
fi

echo -ne "$UserInputFlashPassword\n"

exit 0;
