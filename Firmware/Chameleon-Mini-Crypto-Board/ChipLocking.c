/* ChipLocking.cpp : Implementation of the AVR chip (EEPROM and Bootloader) locking features;
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 02.02.2019
 */

#include "ChipLocking.h"
#include <SHAHash.h>

int PassphraseHashCompare(const char *passphrase, const char *storedHashString) { 
     if(passphrase == NULL || storedHashString == NULL) {
          return passphrase == storedHashString;
     }
     SHAHash_t * hasherObj = GetNewHasherObject();
     size_t hashStrLen = strlen(storedHashString), pphLen = strlen(passphrase);
     uint8_t *pphHashBytes = ComputeHashBytes(hasherObj, passphrase, pphLen, 
		                              passphrase, pphLen);
     size_t pphHashByteCount = GetHashByteCount(hasherObj);
     int compResult = memcmp(passphrase, pphHashBytes, MIN(hashStrLen, pphHashByteCount));
     free(pphHashBytes); pphHashBytes = NULL;
     ClearHashInitData(hasherObj);
     DeleteHasherObject(hasherObj);
     return compResult;
}

size_t ChameleonLockEEPROMMemoryBits() { 
     return 0;
}

size_t ChameleonUnlockEEPROMMemoryBits() { 
     return 0;
}

size_t ChameleonLockBootloaderMemoryBits() { 
     return 0;
}

size_t ChameleonUnlockBootloaderMemoryBits() { 
     return 0;
}
