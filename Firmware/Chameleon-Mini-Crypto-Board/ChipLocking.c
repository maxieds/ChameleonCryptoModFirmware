/* ChipLocking.cpp : Implementation of the AVR chip (EEPROM and Bootloader) locking features;
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 02.02.2019
 */

#include "ChipLocking.h"
#include <SHAHash.h>

//#if defined(CHAMBOARD_LOCKDOWN)
//     LOCKBITS = 0x00;
//#else
//     LOCKBITS = 0xff;
//#endif

int PassphraseHashCompare(const char *passphrase, const char *storedHashString) { 
     if(passphrase == NULL || storedHashString == NULL) {
          return passphrase == storedHashString;
     }
     SHAHash_t * hasherObj = GetNewHasherObject();
     size_t storedHashStrLen = strlen(storedHashString), pphLen = strlen(passphrase);
     uint16_t hashDataBytes = GetHashByteCount(hasherObj);
     uint8_t pphHashBytes[hashDataBytes];
     ComputeHashBytes(hasherObj, pphHashBytes, hashDataBytes, passphrase, pphLen);
     char pphHashStr[2 * hashDataBytes + 1];
     BufferToHexString(pphHashStr, 2 * hashDataBytes + 1, pphHashBytes, hashDataBytes);
     int compResult = strncmp(pphHashStr, storedHashString, MIN(storedHashStrLen, hashDataBytes));
     DeleteHasherObject(hasherObj);
     return compResult;
}

