# ChameleonCryptoModFirmware homepage

## Project specs and high-level description

The [ChameleonCryptoModFIrmware project](https://github.com/maxieds/ChameleonCryptoModFirmware) is 
a custom modification of the 
[Chameleon Mini (RevG) firmware](https://github.com/emsec/ChameleonMini) distribution created by 
[Maxie D. Schmidt](https://github.com/maxieds) 
on behalf of a private project financed by Simon Yorkston and Martin Lore. 
The developer believes that these modifications could eventually be incorporated into the main 
firmware sources. The code contained in this private repository is financed, and hence subject to the 
existing licensing of this derivative code base, the property of Simon Yorkston and Martin Lore 
via their private *PARKLINK* (*Toy Foundary*) company. 

The primary modification of the existing Chameleon Mini firmware sources is made to support loading 
of encrypted *MIFARE_CLASSIC_1K* dump images. This modification to the source then also includes 
support of administrative key-related functionality to generate and manage the keys help within the 
Chameleon RevG board firmware EEPROM. A significant change effectively "upgrading" the firmware 
source from C to C++ has been made in the code contained in this repository. This change was 
necessary to support the C++-based 
[Arduino Crypto Library](http://rweather.github.io/arduinolibs/crypto.html) 
which was requested by Simon Yorkston as a part of the original project spec to ease the coding 
requirements of handling the new cryptographic routines within our source code. 

## List of primary modifications to the firmware

The main changes / modifications / new files added to the stock Chameleon Mini firmware source 
distribution for our purposes here include the following file lists:

* **./Firmware/Chameleon-Mini/ChameleonCrypto.cpp/h** : The bulk of our new routine support stems 
from the functions defined and implemented here;
* **./Firmware/Chameleon-Mini/ChipLocking.cpp/h** : Enable / disable (authed) locking of the ATMega 
chips found onboard the Chameleon Mini boards;
* **./Firmware/Chameleon-Mini/Configuration.h** : Added the built-in *KeyData* field in the 
*ConfigurationType* struct (i.e., in the usual *ActiveConfiguration* variable referenced in many 
places throughout the firmware code); 
* **./Firmware/Chameleon-Mini/Terminal/XModem.cpp/h** : Needed to modify the XModem upload process 
to handle custom processing of encrypted dumps loaded via the new **UPLOAD_ENCRYPTED** command 
documented below; 
* **./Firmware/Chameleon-Mini/Terminal/Commands.cpp/h** : Implementation of the new commands. 
* **./Firmware/Chameleon-Mini/ChameleonCryptoUtils/&ast;** : Utilities mostly for development and 
testing purposes. Also includes a small supply of unencrypted default dump images which we can 
test the loading routines for in **MF1KDumpSamples**;
* **./Firmware/Chameleon-Mini/ArduninoCryptoLib/&ast;** : The external crypto library source we 
started the implementation with plus some custom extensions and necessary modifications. For 
example, we needed to compile the library with the **-mcpu=avrxmega7** option to ``avr-g++`` which 
as it turns out omits some key details of the **stdlibc++** implementation we needed to fix and 
include for our purposes here. 
* **./Firmware/Chameleon-Mini/Makefile** : Customizations and new definitions (some hash-mark-commented) 
for the crypto support specs added to the existing *Makefile* sources. These implicitly include the 
modifications we have made to the **./Firmware/LUFA/Build/lufa_build.mk** headers to support the 
necessary feature set we require for this project.

## Makefile options (crypto routine configuration, etc.) 

See and peruse the source file **./Firmware/Chameleon-Mini/Makefile** in some detail to view some 
of the obvious modifications and new definitions we have included in this file. 
Notably, we have chosen to disable *SUPPORT_FIRMWARE_UPGRADE* options per default expectations 
to make it harder for non-administrative user-level usage of the Chameleon board chips we ship with 
this modified firmware patch: 
```
#Support activating firmware upgrade mode through command-line
#SETTINGS       += -DSUPPORT_FIRMWARE_UPGRADE
```
We support encryption of the dumps (configured via compile-time C++ defined options) via the AES 
block cipher in key sizes of *128/192/256 bits*. The supported cipher modes for use with the 
specified AES key-size configuration include *CTR(size-4)/CFB/OFB/CBC*. The default built-in 
(or as-is) configuration is specified through the compile time build options specified below: 
```
## Setup the default cipher mode and block cipher types here:
SETTINGS        += -DCIPHER_MODE_TYPE_CTR
#SETTINGS        += -DCIPHER_MODE_TYPE_CFB
#SETTINGS        += -DCIPHER_MODE_TYPE_OFB
#SETTINGS        += -DCIPHER_MODE_TYPE_CBC
SETTINGS        += -DDEFAULT_COUNTER_SIZE=4 # OR 0 if none needed
#SETTINGS        += -DBLOCK_CIPHER_TYPE_AES128
#SETTINGS        += -DBLOCK_CIPHER_TYPE_AES192
SETTINGS        += -DBLOCK_CIPHER_TYPE_AES256
```
Additionally, we have included the following defines configured via the customized 
*Makefile* setup we have created here:
```
## Specify the default passphrase needed to flash / lock / unlock the device:
FLASH_LOCK_PASSPHRASE = $(shell ./BuildScripts/GetUserFlashPassword.sh | tail -n 1)
SETTINGS        += -DENABLE_ADMIN_LEVEL_DEBUGGING # Let firmware users print key storage data for debugging?
SETTINGS        += -DREQUIRE_PASSPHRASE_TO_LOCK_CHIP=0 # Descriptively named setting ... 
SETTINGS        += -DENABLE_CHIP_UNLOCKING=1 # Let users unlock the chip (with passphrase auth)? 
SETTINGS        += -DDEFAULT_FLASH_LOCK_PASSPHRASE=\"$(FLASH_LOCK_PASSPHRASE)\"
SETTINGS        += -DPRIu16=\"PRIu16\"
```

## Building the firmware source and flashing on Linux

### Configuring the platform 

Any modern Ubuntu Linux or Linux Mint system is considered to be our target platform for flashing chips 
from the commandline. In order to compile the firmware software and then flash it onto the AVR chip 
onboard the Chameleon, we will need to first install the following packages:
```
$ sudo apt-get install avra avrdude gcc-avr gdb-avr avr-libc
```
Additionally, to get the standard *UDEV* USB device rules configured correctly, follow the advice given on 
[this page](https://rawgit.com/emsec/ChameleonMini/master/Doc/Doxygen/html/_page__getting_started.html) 
to install 
[**./Drivers/98-ChameleonMini.rules**](https://github.com/maxieds/ChameleonCryptoModFirmware/blob/master/Drivers/98-ChameleonMini.rules) 
into the UDEV rule set stack:
```
$ sudo cp Drivers/98-ChameleonMini.rules /etc/udev/rules.d/
$ sudo udevadm control --reload
``` 

### Building the firmware from source

Perform the following steps to build the firmware:
```
$ cd Firmware/Chameleon-Mini
$ make clean && make
$ ls -lh Chameleon-Mini-Crypto-Board.*
```

### Flashing the firmware 

We will need the *Chameleon-Mini-Crypto-Board.eep* and *Chameleon-Mini-Crypto-Board.hex* binary files 
generated by compiling the modified crypto-based source code as in the previous section. We can then 
attach the Chameleon Mini (RevG) board to the Linux system's USB port while 
*holding down* **RBUTTON** *to get the device to enter the bootloader mode*. You can tell that the 
connected Chameleon device is in *Bootloader Mode* if there are no green LED lights turned on nor 
flashing even though the board is securely connected to the USB power source on your machine. 
Once this is done, flash the firmware binaries onto the AVR chip using the following command:
```
$ export FWBIN="Chameleon-Mini-Crypto-Board"
$ sudo avrdude -c flip2 -p ATXMega128A4U -B 60 -P usb -U application:w:$FWBIN.hex:i -U eeprom:w:$FWBIN.eep:i
```
Now you can disconnect the device over USB, plug it into the USB port on your system again as normal, and 
then proceed to communicate with the device as if the original firmware installation is present via a 
serial terminal: 
```
$ sudo apt-get install socat lrzsz
$ socat - /dev/ttyACM0,crnl,raw,echo=0 
   > do UPLOAD_ENCRYPTED command as normal (or via the ChameleonMiniUSBInterface library in Android)
// Meanwhile in another terminal window: 
$ sx EncMF1KDump.edmp | socat FILE:/dev/ttyACM0,b115200,raw -
```

# New Chameleon Mini command set

The effective changes in executive functioning and interface with our patched firmware versions of the 
Chameleon Mini RevG boards are documented below in this section. 

## New command: DEVICEAUTH

### Description

Mechanism by which we may load new key data into the short list of tracked key data 
stored within the modified firmware. This requires authentication via a "*backdoor*" 
flash lock password specified at compile time (typically, by default, the unquoted string 
"*MyFlashLockPwd11:-)*"). 
The *Makefile* option to change this default password setting is as follows: 
```
## Specify the default passphrase needed to flash / lock / unlock the device:
FLASH_LOCK_PASSPHRASE = $(shell ./BuildScripts/GetUserFlashPassword.sh | tail -n 1)
SETTINGS        += -DDEFAULT_FLASH_LOCK_PASSPHRASE=\"$(FLASH_LOCK_PASSPHRASE)\"
```

### Usage

```
DEVICEAUTH FlashLockPassword [NumberOfKeyChangesToAuth=1]
```
* *FlashLockPassword* : Typically defined to be *MyFlashLockPwd11:-)*
unless otherwise specified at compile time (you know who you are if/when this happens); 
* *NumberOfKeyChangesToAuth* : Specify the (fixed, finite) number of key change operations 
to allow after authenticating with the correct *FlashLockPassword* string. This setting 
defaults to the singular allowed change by specified if not otherwise manually reset on the 
command line. 

### Example

```
deviceauth MyFlashLockPwd11:-) 5
100:OK
```

## New command: SETKEY 

### Description

The command sets the hex-byte-valued key data for one of the predefined (four) keys now 
stored internally by the Chameleon Mini RevG boards. 

### Usage

```
SETKEY KeyIdx KeyData
```
* *KeyIdx* : An integer between 0-3
* *KeyData* : Hexadecimal string key data of length at most **2 x MAX_AESBLOCKCIPHER_BITSIZE** (e.g., 
at most *512/8 = 64* hexadecimal characters)

### Return value

A standard Chameleon [return code](https://github.com/maxieds/ChameleonCryptoModFirmware/blob/master/Firmware/Chameleon-Mini/Terminal/Commands.h#L12) 
indicating success or failure of the operation. 

### Example

```
keyauth MyFlashLockPwd11:-) 10
100:OK
setkey 1 1234567890ABCDEF1234567890ABCDEF
100:OK
getkey 1
101:OK WITH TEXT
1234567890ABCDEF1234567890ABCDEF

```   

## New command: GENKEY

### Description

Similar to the **SETKEY** command. Sets the key data for a given built-in key by means of an 
ascii-string-valued passphrase which we use to generate the key data. 

### Usage

```
GENKEY KeyIdx PassphraseSrcString
```
* *KeyIdx* : An integer between 0-3
* *PassphraseSrcString* : An (unquoted) ascii-valued source string in plaintext format used as a 
passphrase to effectively 

### Return value

A standard Chameleon [return code](https://github.com/maxieds/ChameleonCryptoModFirmware/blob/master/Firmware/Chameleon-Mini/Terminal/Commands.h#L12) 
indicating success or failure of the operation. 
If the key generation command is successful then **COMMAND_INFO_OK_WITH_TEXT_ID** (101) is returned 
with text the hexadecimal string value of the key data just updated by the command. 

### Example

```
genkey 2 ggggggggg
101:OK WITH TEXT
97BF8A1835734FBA87CA2643CB0F1C06
getkey 2
101:OK WITH TEXT
97BF8A1835734FBA87CA2643CB0F1C06
```

## New command: GETKEY

### Description

A debugging command to print out the HEX-valued ASCII strings associated with the 
key data for each registered key currently being tracked in the firmware. 
Note that this command is enabled by default in the stock *Makefile* that ships with this 
modification of the firmware source, but is of course sanely turned off by compile time 
C-style **&#35;ifdef ...** preprocessor macros by updating the *Makefile* options to the 
following settings:
```
#SETTINGS += -DENABLE_ADMIN_LEVEL_DEBUGGING # Let firmware users print key storage data for debugging?
```

### Usage

```
GETKEY AESKeyIndex
```
* *AESKeyIndex* : a single decimal digit in the range 0-3 indicating the 
key index to grok internally stored data from.

### Example

```
getkey 2                          
101:OK WITH TEXT 
3070971AB7CE45063FD2573F49F5420D
```

## New command: UPLOAD_ENCRYPTED

### Description

Similar to the functionality, feel and usage of the existing standard **UPLOAD** command to 
upload the card dump data from a raw unecrypted file format. The dump uploaded here (with 
specified key data used in the decryption) should be suitably encrypted using the pre-configured 
AES+CipherMode algorithms for the decryption routine to run correctly (see detailed example below 
for testing purposes). 

### Usage

```
UPLOAD_ENCRYPTED KeyIndex TimestampSaltAsciiString
<User Initiates Upload of the Encrypted Dump via an XModem Commection from the Terminal>
```
* *KeyIndex* : Index of the preloaded internally stored key to use for decryption; 
* *TimestampSaltAsciiString* : Short-ish (exactly 16-character) 
timestamp string to use as a "salted" initial vector for the 
AES block cipher used for decryption.

### Example

```
$ cd ./Firmware/Chameleon-Mini-Crypto-Board
$ ./HelperScripts/ChameleonUploadEncrypted.sh 1 MyTimestampSalt1 ChameleonCryptoUtils/MF1KDumpSamples/8956-1972-D463.edmp --reset
$ minicom chameleoncb
> upload_status
181:UPLOAD OK
> uid?
F6B48DD6
> config?
MF_CLASSIC_1K
# <CTRL+A> then 'q' to exit
```

## New command: UPLOAD_STATUS

### Description

Check on the status of a pending or recently processed **UPLOAD_ENCRYPTED** command 
issued to load an encrypted Mifare1K dump image into memory. 

### Usage

```
UPLOAD_STATUS
```
* No parameters are passed to this command

### Example

```
UPLOAD_STATUS
181:UPLOAD OK
```

# Testing and crypto dump utilities

## Basic encryption scheme specs

Since Simon has graciously granted me free will to implement the crypto standard for the uploaded MF1K dump 
images as best as I can get them running on the Chameleon RevG hardware, we are in need of a local 
(read: Linux command line) utility to create the encrypted dump images for testing with the Chameleon board. 
Given an initial unencoded binary dump file representing a MF1K image that can be initialized into the 
Chameleon's active memory slot with a call to the pre-existing stock **UPLOAD** command, we also require the 
convention that the dump image be prepended with an ASCII-plaintext header tag (namely, the string 
"*MFCLASSIC1K-DMP*" without the quotes): 
```
# Shell commands to create the base unencrypted dump images:
$ cd ./Firmware/Chameleon-Mini-Crypto-Board/ChameleonCryptoUtils/MF1KDumpSamples
$ export INITDMPIMGBASE=3E46-E4ED-516D.dmp
$ echo "MFCLASSIC1K-DMP" > $INITDMPIMGBASE
$ cat "${INITDMPIMGBASE}-dist" >> $INITDMPIMGBASE
```
The rationale behind prepending a header tag to the data we encrypt is to add another verification 
mechanism that we are correctly decrypting the image into a non-garbage binary buffer for loading into 
Chameleon memory at slot &#35;X. That is, it is so unprobable that we could incorrectly decrypt the image and 
still have this key chunk of header text be accurate as intended that if the first few bytes of the 
decrypted dump image match this header string, we will assume that we have correctly decrypted the dump 
image. This scheme is not entirely unlike the *Authenticated encryption with associated data (AEAD)* crypto 
schemes which are supported by the unmodified (untrimmed) source for the library we utilize for 
crypto (e.g., the [ChaChaPoly scheme](http://rweather.github.io/arduinolibs/classChaChaPoly.html#details), among 
a couple of noteworthy others). After we prepend the header tag to the base dump image data, we will then 
proceed to perform decryption (encryption by API or external command line utility) using a standardized 
*AES128* block cipher endowed with the standardized *CFB* cipher mode.

## Testing out (verifying) the library code is implemented correctly via our subroutines

Let's work on the problem of generating the appropriate encrypted dump images for us to test with the new 
Chameleon Mini **UPLOAD_ENCRYPTED** command. For example, observe:
```
$ cd .. && make clean && make && cd ChameleonCryptoUtils
$ ../HelperScripts/PrepareEncryptedDumpImage.sh MF1KDumpSamples/5076-4309-1469.dmp 1234567890ABCDEF1234567890ABDCEF MyTimestampSalt1
 >> Removing previous files ...
 >> Creating dump data file "ChameleonCryptoUtils/MF1KDumpSamples/5076-4309-1469.dmp" ...
 >> Prepending crypto header string "MFCLASSIC1K-DMP" ...
 >> Extending dump data file from 1187 to 1280 bytes ...
 >> Sanity check on the encryption / decryption routine: OK! ... 
 >> Terminating the utility for good!
```
The source repository features the encrypted dumps resulting from running the encryption command above 
on each of the three prepared sample dump files:
```
$ ls MF2KDumpSamples/*.edmp
MF1KDumpSamples/3E46-E4ED-516D.edmp  MF1KDumpSamples/8956-1972-D463.edmp
MF1KDumpSamples/5076-4309-1469.edmp
```
At this point, we are ready to 
[load up some new key data](https://github.com/maxieds/ChameleonCryptoModFirmware#new-chameleon-mini-command-set) 
and then test out the encrypted dump 
[upload functionality](https://github.com/maxieds/ChameleonCryptoModFirmware#new-command-upload_encrypted) 
on a live Chameleon Mini device. 
The basic instructions for communicating with the Chameleon over a serial USB terminal 
[described here](https://github.com/maxieds/ChameleonCryptoModFirmware#flashing-the-firmware) 
may now be repeated and followed thusly in testing out the API's functionality. 

# Reading and resources on locking features of the AVR ATMega128 chips in the Chameleon boards

## Programatic locking functionality added to the Chameleon

* Added **DEVICEAUTH** which lets users with a "*secret*" (Makefile / compile-time defined) backdoor 
password authenticate with the system to load keys and view keys. 
Note that only a SHA256 hash of this password is stored within the firmware, so in principle this is 
as safe as if a would-be reverse engineer hardware hacker gal were to obtain the */etc/shadow* 
(*/etc/passwd*) files on a Unix system. It's not a perfect solution, but for now this is the only 
sane way to do development without permanenty bricking some expensive test sets of Chameleon Mini 
hardware.
* The *Makefile* option **-DSUPPORT_FIRMWARE_UPGRADE** is enabled and password protected via the 
**DEVICEAUTH** mechanism in the previous point. This provides another sane (RE: *for development*) 
layer of getting the device back to bootloader mode where it can be reflashed with ``avrdude``.

## Further reading for future development 

* [AVR ATMega128 chip capabilities](http://www.engbedded.com/fusecalc/) (FUSE bites) listing
* [SPIEN=0 FUSE setting](https://www.avrfreaks.net/forum/spien) (*WARNING:* It should be hard to recover these expensiove boards after enabling this, so use only with caution ...) 

