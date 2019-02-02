/* Crypto.c : Implement the cryptographic helper functions for loading encrypted dumps into memory;
 * Author: Maxie D. Schmidt
 * Created: 01/24/2019
 */

#include <stdlib.h>

#include <SHA3.h>

#include "ChameleonCrypto.h"
#include "Settings.h"
#include "Terminal/Commands.h"
#include "Terminal/Terminal.h"
#include "System.h"

uint8_t EEMEM CryptoUploadBuffer[CRYPTO_UPLOAD_BUFSIZE];
size_t EEMEM CryptoUploadBufferByteCount;

const KeyData_t DEFAULT_KEY_DATA = { 
     { 0 },
     { 0 }
};

void InitCryptoDumpBuffer() { 
     uint8_t zeroFillByte = 0x00;
     for(int b = 0; b < CRYPTO_UPLOAD_HEADER_SIZE + CRYPTO_UPLOAD_BUFSIZE; b++) {
          WriteEEPBlock((uint16_t) &(CryptoUploadBuffer[b]), &zeroFillByte, 1);
     } 
     CryptoUploadBufferByteCount = 0;
}

bool ValidDumpImageHeader(uint8_t *dumpDataBuf, size_t bufLength) { 
     if(bufLength <= CRYPTO_UPLOAD_HEADER_SIZE || dumpDataBuf == NULL) { 
          return false;
     }
     char dumpHeaderBytes[CRYPTO_UPLOAD_HEADER_SIZE];
     ReadEEPBlock((uint16_t) &dumpDataBuf, (void *) dumpHeaderBytes, CRYPTO_UPLOAD_HEADER_SIZE);
     if(strncmp(dumpHeaderBytes, PSTR(CRYPTO_UPLOAD_HEADER), CRYPTO_UPLOAD_HEADER_SIZE)) { 
          return false;
     }
     return true;
}

uint8_t * GetKeyDataFromString(const char *byteString, size_t *byteBufLengthParam) { 
     size_t byteBufLen = strlen(byteString) / 2;
     if(byteBufLen == 0 || byteBufLengthParam == NULL) { 
          return NULL;
     }
     else {
          *byteBufLengthParam = byteBufLen;
     }
     uint8_t *byteBuf = (uint8_t *) malloc(byteBufLen * sizeof(uint8_t));
     int fullKeyInt = atoi(byteString);
     for(int b = 0; b < byteBufLen; b += 2) { 
          int byteMask = 0xff << sizeof(uint8_t) * b;
	  uint8_t curByte = (fullKeyInt & byteMask) >> (sizeof(uint8_t) * b);
          byteBuf[b] = curByte;
     }
     return byteBuf;
}

bool SetKeyData(size_t keyIndex, uint8_t *keyData, size_t keyLength) { 
     if(keyIndex < 0 || keyIndex >= NUM_KEYS_STORAGE || !keyData || 
        keyLength <= 0 || keyLength > MAX_KEY_LENGTH) { 
          return false;
     }
     memcpy(&(GlobalSettings.KeyData.keys[keyIndex]), keyData, keyLength);
     GlobalSettings.KeyData.keyLengths[keyIndex] = keyLength;
     SettingsSave();
     return true;
}

bool ZeroFillKeyData(size_t keyIndex, size_t keyLength) {
     if(keyIndex < 0 || keyIndex >= NUM_KEYS_STORAGE || keyLength <= 0 || 
	keyLength > MAX_KEY_LENGTH) {
          return false;
     }
     uint8_t *zeroBuf = (uint8_t *) malloc(keyLength * sizeof(uint8_t));
     memset(zeroBuf, 0, keyLength);
     bool success = SetKeyData(keyIndex, zeroBuf, keyLength);
     free(zeroBuf);
     return success;
}

bool KeyIsValid(size_t keyIndex) { 
     if(keyIndex < 0 || keyIndex >= NUM_KEYS_STORAGE) {
          return false;
     }
     size_t keyLength = GlobalSettings.KeyData.keyLengths[keyIndex];
     return (keyLength > 0) && (keyLength <= MAX_KEY_LENGTH);
}

size_t PassphraseToAESKeyData(const char *passphrase, uint8_t *keyDataBuffer, size_t maxKeyDataBytes, 
		              bool useTickTimeSalt) { 
     if(passphrase == NULL || keyDataBuffer == NULL) { 
          return 0;
     }
     size_t pphLength = strlen(passphrase);
     SHA3_256 pphHasherObj;
     pphHasherObj.clear();
     pphHasherObj.resetHMAC(passphrase, pphLength);
     pphHasherObj.update(passphrase, pphLength);
     if(useTickTimeSalt) { 
          uint16_t tickTime = SystemGetSysTick();
	  char tickTimeStrBuffer[TERMINAL_BUFFER_SIZE];
	  size_t tickTimeStrLength = IntegerToStringBuffer(tickTime, tickTimeStrBuffer, 
			                                   TERMINAL_BUFFER_SIZE);
	  pphHasherObj.update(tickTimeStrBuffer, tickTimeStrLength);
     }
     size_t KeyDataByteCount = MIN(MIN(TERMINAL_BUFFER_SIZE, BLOCK_CIPHER_KEY_BYTE_SIZE), 256);
     uint8_t keyDataHash[KeyDataByteCount];
     pphHasherObj.finalizeHMAC(passphrase, pphLength, keyDataHash, KeyDataByteCount);
     size_t copyNumBytes = MIN(maxKeyDataBytes, pphHasherObj.hashSize());
     memcpy(keyDataBuffer, keyDataHash, copyNumBytes);
     return copyNumBytes;
}

Cipher_t CreateBlockCipherObject(const uint8_t *keyData, size_t keyLength, 
		                 const uint8_t *initVecData, size_t ivLength) { 
     Cipher_t cipherObj;
     if(keyData == NULL || !keyLength || initVecData == NULL || !ivLength) {
          return cipherObj;
     }
     cipherObj.setKey(keyData, keyLength);
     cipherObj.setIV(initVecData, ivLength);
     #ifdef CIPHER_MODE_TYPE_CTR
     if(DEFAULT_COUNTER_SIZE > 0) { 
          cipherObj.setCounterSize(DEFAULT_COUNTER_SIZE);
     }
     #endif
     return cipherObj;
}

Cipher_t CreateBlockCipherObjectFromKeyIndex(size_t keyIndex, const uint8_t *initVecData, size_t ivLength) { 
     if(keyIndex < 0 || keyIndex >= NUM_KEYS_STORAGE) {
          Cipher_t cipherObj;
	  return cipherObj;
     }
     uint8_t keyData[MAX_KEY_LENGTH];
     size_t keyLength = GlobalSettings.KeyData.keyLengths[keyIndex];
     ReadEEPBlock((uint16_t) &(GlobalSettings.KeyData.keys[keyIndex]), keyData, keyLength);
     return CreateBlockCipherObject(keyData, keyLength, initVecData, ivLength);
}

uint8_t * DecryptDumpImage(Cipher_t cipherObj, const uint8_t *byteBuf, size_t byteBufLen) { 
     if(byteBuf == NULL || byteBufLen <= 0) { 
          return NULL;
     }
     uint8_t *plaintextBuf = (uint8_t *) malloc(byteBufLen * sizeof(uint8_t));
     uint8_t byteBufLocalCopy[MAX_KEY_LENGTH];
     ReadEEPBlock((uint16_t) &byteBuf, byteBufLocalCopy, byteBufLen);
     cipherObj.decrypt(plaintextBuf, byteBufLocalCopy, byteBufLen);
     return plaintextBuf;
}

