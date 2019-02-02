# This makefile uses sh in order to build the project.
# Note when using AVR-Toolchain, you have to install cygwin and 
# append cygwin's bin directory to the PATH environment variable 
# because Atmel does not ship sh.exe anymore with the toolchain. 
SHELL = /bin/sh

#Supported configurations
SETTINGS    += -DCONFIG_MF_CLASSIC_1K_SUPPORT
SETTINGS    += -DCONFIG_MF_CLASSIC_1K_7B_SUPPORT
SETTINGS    += -DCONFIG_MF_CLASSIC_4K_SUPPORT
SETTINGS    += -DCONFIG_MF_CLASSIC_4K_7B_SUPPORT
SETTINGS    += -DCONFIG_MF_ULTRALIGHT_SUPPORT 
SETTINGS    += -DCONFIG_ISO14443A_SNIFF_SUPPORT
SETTINGS    += -DCONFIG_ISO14443A_READER_SUPPORT

#Support magic mode on mifare classic configuration
SETTINGS	+= -DSUPPORT_MF_CLASSIC_MAGIC_MODE

#Don't touch manufacturer byte with BUTTON_ACTION_UID_LEFT_(DE/IN)CREMENT
SETTINGS	+= -DSUPPORT_UID7_FIX_MANUFACTURER_BYTE

#Support activating firmware upgrade mode through command-line
#SETTINGS	+= -DSUPPORT_FIRMWARE_UPGRADE

#Default configuration
SETTINGS	+= -DDEFAULT_CONFIGURATION=CONFIG_MF_CLASSIC_1K
#SETTINGS	+= -DDEFAULT_CONFIGURATION=CONFIG_MF_CLASSIC_4K
#SETTINGS	+= -DDEFAULT_CONFIGURATION=CONFIG_MF_ULTRALIGHT
#SETTINGS	+= -DDEFAULT_CONFIGURATION=CONFIG_NONE
#SETTINGS	+= -DDEFAULT_CONFIGURATION=CONFIG_ISO14443A_READER

#Default button actions
#SETTINGS	+= -DDEFAULT_RBUTTON_ACTION=BUTTON_ACTION_UID_RANDOM
#SETTINGS	+= -DDEFAULT_RBUTTON_ACTION=BUTTON_ACTION_UID_LEFT_INCREMENT
#SETTINGS	+= -DDEFAULT_RBUTTON_ACTION=BUTTON_ACTION_UID_RIGHT_INCREMENT
#SETTINGS	+= -DDEFAULT_RBUTTON_ACTION=BUTTON_ACTION_UID_LEFT_DECREMENT
#SETTINGS	+= -DDEFAULT_RBUTTON_ACTION=BUTTON_ACTION_UID_RIGHT_DECREMENT
SETTINGS	+= -DDEFAULT_RBUTTON_ACTION=BUTTON_ACTION_CYCLE_SETTINGS
#SETTINGS	+= -DDEFAULT_RBUTTON_ACTION=BUTTON_ACTION_STORE_MEM
SETTINGS	+= -DDEFAULT_LBUTTON_ACTION=BUTTON_ACTION_RECALL_MEM

#Define if button action setting should be independent of active setting 
SETTINGS	+= -DBUTTON_SETTING_GLOBAL

#Default LED functions
SETTINGS	+= -DDEFAULT_RED_LED_ACTION=LED_SETTING_CHANGE
SETTINGS	+= -DDEFAULT_GREEN_LED_ACTION=LED_POWERED

#Define if LED function setting should be independent of active setting
SETTINGS	+= -DLED_SETTING_GLOBAL

#Default logging mode
SETTINGS	+= -DDEFAULT_LOG_MODE=LOG_MODE_OFF
#SETTINGS	+= -DDEFAULT_LOG_MODE=LOG_MODE_MEMORY
#SETTINGS	+= -DDEFAULT_LOG_MODE=LOG_MODE_TERMINAL

#Define if log settings should be global
SETTINGS	+= -DLOG_SETTING_GLOBAL

#Default setting
SETTINGS	+= -DDEFAULT_SETTING=SETTINGS_FIRST

#Default pending task timeout
SETTINGS	+= -DDEFAULT_PENDING_TASK_TIMEOUT=50 #* 100ms

#Default reader threshold
SETTINGS	+= -DDEFAULT_READER_THRESHOLD=400

#Use EEPROM to store settings
SETTINGS	+= -DENABLE_EEPROM_SETTINGS

## Setup the default cipher mode and block cipher types here:
SETTINGS        += -DCIPHER_MODE_TYPE_CTR
#SETTINGS        += -DCIPHER_MODE_TYPE_CFB
#SETTINGS        += -DCIPHER_MODE_TYPE_OFB
#SETTINGS        += -DCIPHER_MODE_TYPE_CBC
SETTINGS        += -DDEFAULT_COUNTER_SIZE=4 # OR 0 if none needed
SETTINGS        += -DBLOCK_CIPHER_TYPE_AES128
#SETTINGS        += -DBLOCK_CIPHER_TYPE_AES256

## Specify the default passphrase needed to flash / lock / unlock the device:
FLASH_LOCK_PASSPHRASE = $(shell ./BuildScripts/GetUserFlashPassword.sh | tail -n 1)
SETTINGS        += -DENABLE_ADMIN_LEVEL_DEBUGGING # Let firmware users print key storage data for debugging?
SETTINGS        += -DREQUIRE_PASSPHRASE_TO_LOCK_CHIP=0 # Descriptively named setting ... 
SETTINGS        += -DENABLE_CHIP_UNLOCKING=1 # Let users unlock the chip (with passphrase auth)? 
SETTINGS        += -DDEFAULT_FLASH_LOCK_PASSPHRASE=\"$(FLASH_LOCK_PASSPHRASE)\"
SETTINGS        += -DALWAYS_DIFFTIME_COMMANDS=1
SETTINGS        += -DPRIu16=\"PRIu16\"

#Memory definitions and objcopy flags to include sections in binaries
FLASH_DATA_ADDR = 0x10000 #Start of data section in flash
#FLASH_DATA_SIZE = 0x10000 #Size of data section in flash
FLASH_DATA_SIZE = 0x10000 #Size of data section in flash
FLASH_DATA_OBJCOPY = --set-section-flags=.flashdata="alloc,load"
SPM_HELPER_ADDR = 0x21FE0 #Start of SPM helper section. Should be last 32Byte in bootloader section
SPM_HELPER_OBJCOPY = --set-section-flags=.spmhelper="alloc,load"
EEMEM_DATA_ADDR = 0x810000

#Build configuration
ifeq ($(OS),Windows_NT)
BUILD_DATE   = "$(shell date /t)"
else
BUILD_DATE   = $(shell date +'%y%m%d')
endif
COMMIT_ID	 = $(shell git rev-parse --short HEAD)
MCU          = atxmega128a4u
ARCH         = XMEGA
BOARD        = NONE
F_CPU        = 27120000
F_USB        = 48000000
TARGET       = Chameleon-Mini-Crypto-Board
OPTIMIZATION = s
SRC         += $(TARGET).cpp LUFADescriptors.cpp System.cpp Configuration.cpp Random.cpp Common.cpp Memory.cpp MemoryAsm.S Button.cpp Log.cpp Settings.cpp LED.cpp Map.cpp AntennaLevel.cpp
SRC         += Terminal/Terminal.cpp Terminal/Commands.cpp Terminal/XModem.cpp Terminal/CommandLine.cpp
SRC         += Codec/Codec.cpp Codec/ISO14443-2A.cpp Codec/Reader14443-2A.cpp Codec/SniffISO14443-2A.cpp Codec/Reader14443-ISR.S
SRC         += Application/ISO14443-3A.cpp Application/Crypto1.cpp Application/MifareUltralight.cpp Application/MifareClassic.cpp Application/Reader14443A.cpp Application/Sniff14443A.cpp
SRC         += ChameleonCrypto.cpp ChipLocking.cpp
SRC         += $(LUFA_SRC_USB) $(LUFA_SRC_USBCLASS)
LUFA_PATH    = ../LUFA
CC_FLAGS     = -O0 -DUSE_LUFA_CONFIG_HEADER -DFLASH_DATA_ADDR=$(FLASH_DATA_ADDR) -DFLASH_DATA_SIZE=$(FLASH_DATA_SIZE) -DSPM_HELPER_ADDR=$(SPM_HELPER_ADDR) -DEEMEM_DATA_ADDR=$(EEMEM_DATA_ADDR) -DBUILD_DATE=\"$(BUILD_DATE)\" -DCOMMIT_ID=\"$(COMMIT_ID)\" -DCHAMELEON_REVSTR=\"REV:$(COMMIT_ID)@TIME:$(BUILD_DATE)\" $(SETTINGS) -IArduinoCryptoLib/libraries/Crypto -IArduinoCryptoLib/libraries/CryptoLegacy/src
LD_FLAGS     =  -Wl,--start-group -Wl,--no-as-needed -LArduinoCryptoLib -lArduinoCrypto -Wl,--section-start=.flashdata=$(FLASH_DATA_ADDR) -Wl,--section-start=.spmhelper=$(SPM_HELPER_ADDR) #-Wl,--section-start=.eeprom=$(EEMEM_DATA_ADDR)
OBJDIR       = Bin
OBJECT_FILES = 

#AVRDUDE settings
AVRDUDE_PROGRAMMER = flip2
AVRDUDE_MCU = atxmega128a4u
AVRDUDE_PORT = usb
AVRDUDE_WRITE_APP = -U application:w:$(TARGET).hex
AVRDUDE_WRITE_EEPROM = -U eeprom:w:$(TARGET).eep
AVRDUDE_FLAGS = -p $(AVRDUDE_MCU) -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER)

#AVRDUDE settings to program the precompiled firmware
AVRDUDE_WRITE_APP_LATEST = -U application:w:Latest/Chameleon-Mini.hex
AVRDUDE_WRITE_EEPROM_LATEST = -U eeprom:w:Latest/Chameleon-Mini.eep


# Default target
all: crypto_utils ArduinoCryptoLib/libCrypto.a \
	$(TARGET).hex $(TARGET).bin $(TARGET).elf echo_flash_password

# Include LUFA build script makefiles
include $(LUFA_PATH)/Build/lufa_core.mk
include $(LUFA_PATH)/Build/lufa_sources.mk
include $(LUFA_PATH)/Build/lufa_build.mk
include $(LUFA_PATH)/Build/lufa_cppcheck.mk
# include $(LUFA_PATH)/Build/lufa_doxygen.mk
# include $(LUFA_PATH)/Build/lufa_dfu.mk
# include $(LUFA_PATH)/Build/lufa_hid.mk
# include $(LUFA_PATH)/Build/lufa_avrdude.mk
# include $(LUFA_PATH)/Build/lufa_atprogram.mk

#Overwrite the LUFA versions of hex/bin file generation to include spmhelper and flashdata sections
%.hex: %.elf
	@echo $(MSG_OBJCPY_CMD) Extracting HEX file data from \"$<\"
	$(CROSS)-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature $(FLASH_DATA_OBJCOPY) $< $@
	@echo "USER-DEFINED FLASH ADMIN PASSPHRASE: " $(FLASH_LOCK_PASSPHRASE)

%.bin: %.elf
	@echo $(MSG_OBJCPY_CMD) Extracting BIN file data from \"$<\"
	$(CROSS)-objcopy -O binary -R .eeprom -R .fuse -R .lock -R .signature $(FLASH_DATA_OBJCOPY) $< $@
	@echo "USER-DEFINED FLASH ADMIN PASSPHRASE: " $(FLASH_LOCK_PASSPHRASE)

# Extract SPMHelper in the last 32 Byte of the bootloader section to externally combine it with any bootloader
spmhelper: $(TARGET).elf
	@echo $(MSG_OBJCPY_CMD) Extracting SPM helper HEX file from $(TARGET).elf
	$(CROSS)-objcopy -O ihex -j .spmhelper $(SPM_HELPER_OBJCOPY) $(TARGET).elf $(TARGET).hex
		
# Program the device using avrdude  
program: $(TARGET).hex $(TARGET).eep
	avrdude $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_APP) $(AVRDUDE_WRITE_EEPROM)

# Program the device using avrdude with the latest official firmware
program-latest:
	avrdude $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_APP_LATEST) $(AVRDUDE_WRITE_EEPROM_LATEST)

# Program the device using batchisp and the DFU bootloader
# Note that the device has to be in bootloader mode already
dfu-flip: $(TARGET).hex $(TARGET).eep
	cp $(TARGET).eep EEPROM.hex
	batchisp -hardware usb -device $(MCU) -operation erase f memory FLASH loadbuffer $(TARGET).hex program verify memory EEPROM loadbuffer EEPROM.hex program verify start reset 0
	rm EEPROM.hex

# Program the device using dfu-programmer
dfu-prog: $(TARGET).hex $(TARGET).eep
	dfu-programmer $(MCU) erase
	dfu-programmer $(MCU) flash-eeprom $(TARGET).eep
	dfu-programmer $(MCU) flash $(TARGET).hex
	dfu-programmer $(MCU) reset

ArduinoCryptoLib/libCrypto.a: ArduinoCryptoLib/libArduinoCrypto.a
	cd ArduinoCryptoLib/host/Crypto && make clean
	cd ArduinoCryptoLib/host/Crypto && make \
		CXX=avr-g++ HOSTARCH_FLAGS="-mmcu=avrxmega7 -DNEED_LIBSTDCPP" chameleon 

crypto_utils: 

ChameleonCryptoUtils/UtilityBin/EncodeDump:
	cd ArduinoCryptoLib/host/Crypto && make clean
	cd ArduinoCryptoLib/host/Crypto && make \
		CXX="g++ -Wl,--start-group -Wl,--no-as-needed -lstdc++ -frtti --std=c++11" \
		linux_box_testing
	cd ChameleonCryptoUtils && g++ CreateEncryptedDumpImage.cpp -c \
		-Wl,--start-group -Wl,--no-as-needed -std=c++11 -lstdc++ \
		-frtti -I/usr/include -I`readlink -f ../ArduinoCryptoLib/libraries/Crypto` \
		-I`readlink -f ../ArduinoCryptoLib/libraries/CryptoLegacy/src` \
		-L`readlink -f ../ArduinoCryptoLib` -lArduinoCryptoLinuxHostVersion
	cd ChameleonCryptoUtils && g++ -o UtilityBin/EncodeDump CreateEncryptedDumpImage.o \
		-Wl,--start-group -Wl,--no-as-needed -std=c++11 -lstdc++ -frtti \
		-L`readlink -f ../ArduinoCryptoLib` -lArduinoCryptoLinuxHostVersion

echo_flash_password:
	@echo "USER-DEFINED FLASH ADMIN PASSPHRASE: " $(FLASH_LOCK_PASSPHRASE)
