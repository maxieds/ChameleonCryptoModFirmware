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
$ socat - /dev/ttyACM0,crnl
   > UPLOAD_ENCRYPTED
// Meanwhile in another terminal window: 
$ sx EncMF1KDump.edmp | socat FILE:/dev/ttyACM0,b115200,raw -
```

# New Chameleon Mini command set

The effective changes in executive functioning and interface with our patched firmware versions of the 
Chameleon Mini RevG boards are documented below in this section. 

## New command: KEYAUTH

### Description

### Usage

### Example


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
ChameleonMiniSerialTerminal$ SETKEY 0 0000000000000000
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
ChameleonMiniSerialTerminal$ GENKEY 2 MyNewKeyDataPassphrase0123*#$%^!
```

## New command: GETKEY

### Description

A debugging command to print out the HEX-valued ASCII strings associated with the 
key data for each registered key currently being tracked in the firmware. 
Note that this command is enabled by default in the stock *Makefile* that ships with this 
modification of the firmware source, but is of course sanely turned off by compile time 
C-style **&amp;ifdef ...** preprocessor macros by updating the *Makefile* options to the 
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
GETKEY 0
TODO
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
ChameleonMiniSerialTerminal$ UPLOAD_ENCRYPTED KeyIdx KeyHexData TimestampSalt
<User Initiates Upload of the Encrypted Dump via an XModem Commection from the Terminal>
```

### Example

TODO 

### Detailed example for testing in practice

TODO

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
TODO
TODO
TODO
```

## New command: LOCK_CHIP

### Description

A wrapper command which locks the EEPROM and memory bits on the Chameleon's on-board 
*ATXMega128a4u* chip. Note that if the directive **REQUIRE_PASSPHRASE_TO_LOCK_CHIP** is 
set to a non-zero value in the *Makefile*, then a password (i.e., the default backdoor 
flash password specified at compile time) is required as a *non-optional* argument to the 
command. Failure to authenticate with the appropriate password (if necessary per build 
congiguration) will result in an error and the operation failing without changing the 
current status of the lock bits on the chip. 

### Usage

```
LOCK_CHIP [FlashLockPassword]
```
* *FlashLockPassword* : Typically defined to be *MyFlashLockPwd11:-)* 
unless otherwise specified at compile time (you know who you are if/when this happens).

### Example

```
LOCK_CHIP
TODO
```
OR 
```
LOCK_CHIP MyFlashLockPwd11:-)
```

## New command: UNLOCK_CHIP

### Description

The counterpart and effective reversal operation for the **LOCK_CHIP** command 
documented above. Notice that this command will only (possibly) succeed if the 
*Makefile* option **ENABLE_CHIP_UNLOCKING** is set to a non-zero value at compile time. 

### Usage

```
UNLOCK_CHIP FlashLockPassword
```
* *FlashLockPassword* : Typically defined to be *MyFlashLockPwd11:-)* 
unless otherwise specified at compile time (you know who you are if/when this happens).

### Example

```
UNLOCK_PASSWORD MyFlashLockPwd11:-)
TODO
```

# Testing and crypto dump utilities


# Reading and resources on locking features of the AVR ATMega128 chips in the Chameleon boards

* [AVR ATMega128 chip capabilities](http://www.engbedded.com/fusecalc/) listing 
* [Memory Lock bits explained](https://www.avrfreaks.net/comment/414083#comment-414083)
* Tutorials on AVR FUSES: [here (good)](http://www.ladyada.net/learn/avr/fuses.html) and 
[here (secondard)](https://embedds.com/all-you-need-to-know-about-avr-fuses/)
* Note: It may be possible to set a flash / erase (EEPROM) password or key with ``avrdude`` directly; 

# TODO (for Maxie / "the developer") 

* Add custom debugging / logging codes associated with loading the encrypted dumps and the new 
key storage routines. 
* Explore hacking the firmware source and ``gdb-avr`` utilities for debugging? 

