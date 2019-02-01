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
SETTINGS        += -DDEFAULT_FLASH_LOCK_PASSPHRASE=\"MyFlashLockPwd11\"
SETTINGS        += -DPRIu16=\"PRIu16\"
```

# New Chameleon Mini command set

The effective changes in executive functioning and interface with our patched firmware versions of the 
Chameleon Mini RevG boards are documented below in this section. 

## New command: SETKEY

## New command: GENKEY

## New command: UPLOAD_ENCRYPTED



# Testing and crypto dump utilities



# TODO (for Maxie / "the developer") 

Add custom debugging / logging codes associated with loading the encrypted dumps and the new 
key storage routines. 


