/* ChipLocking.h : Control chip "locking" / unlocking features of the stock Chameleon Mini 
 *                 boards which feature an onboard AVR ATMega128 microcontroller (along with 
 *                 all of the standard Memory Locking and customization features that entails);
 * Author: Maxie D. Schmidt
 * Created: 02/01/2019
 */

#ifndef __CHAMELEON_MINI_BOARD_CHIP_LOCKING_FEATURES_H__
#define __CHAMELEON_MINI_BOARD_CHIP_LOCKING_FEATURES_H__

#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/lock.h>

#include "Common.h"
#include "ChameleonCrypto.h"

static const char PROGMEM FLASH_LOCK_PASSPHRASE_CONSTANT[MAX_KEY_LENGTH] = 
     DEFAULT_FLASH_LOCK_PASSPHRASE;
static const size_t PROGMEM FLASH_LOCK_PPH_LENGTH = strlen(DEFAULT_FLASH_LOCK_PASSPHRASE);

INLINE bool AuthLockByPassphrase(const char *authPwd) { 
     if(authPwd == NULL || !FLASH_LOCK_PPH_LENGTH || strcmp(authPwd, DEFAULT_FLASH_LOCK_PASSPHRASE)) {
          return false;
     }
     return true;
}

/* Chip locking routines */
extern "C" { 
     size_t ChameleonLockEEPROMMemoryBits();
     size_t ChameleonUnlockEEPROMMemoryBits();
     size_t ChameleonLockBootloaderMemoryBits();
     size_t ChameleonUnlockBootloaderMemoryBits();
}

#endif
