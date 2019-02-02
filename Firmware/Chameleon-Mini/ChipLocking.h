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

#ifdef DEFAULT_FLASH_LOCK_PASSPHRASE
     static const char *FLASH_LOCK_PASSPHRASE = DEFAULT_FLASH_LOCK_PASSPHRASE;
#endif
#ifndef DEFAULT_FLASH_LOCK_PASSPHRASE
     static const char *FLASH_LOCK_PASSPHRASE = NULL; /* disabled */
#endif

#include <avr/io.h>
#include <avr/lock.h>

/* Chip locking routines */
extern "C" { 
     uint16_t ChameleonLockEEPROMMemoryBits(const char *authPwd = NULL);
     uint16_t ChameleonUnlockEEPROMMemoryBits(const char *authPwd = NULL);
     uint16_t ChameleonLockBootloaderMemoryBits(const char *authPwd = NULL);
     uint16_t ChameleonUnlockBootloaderMemoryBits(const char *authPwd = NULL);
}

#endif
