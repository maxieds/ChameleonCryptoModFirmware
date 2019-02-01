/* Crypto.c : Implement the cryptographic helper functions for loading encrypted dumps into memory;
 * Author: Maxie D. Schmidt
 * Created: 01/24/2019
 */

#include <stdlib.h>

#include "ChameleonCrypto.h"
#include "Configuration.h"
#include "Terminal/Commands.h"
#include "Terminal/Terminal.h"

uint8_t CryptoUploadBuffer[MEMORY_SIZE_PER_SETTING];
uint16_t CryptoUploadBufferByteCount;

void InitCryptoDumpBuffer() { 
     memset(CryptoUploadBuffer, 0, MEMORY_SIZE_PER_SETTING);
     CryptoUploadBufferByteCount = 0;
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

bool SetKeyData(KeyData_t &keyDataStruct, size_t keyIndex, uint8_t *keyData, size_t keyLength) { 
     if(keyIndex < 0 || keyIndex >= NUM_KEYS_STORAGE || !keyData || 
        keyLength <= 0 || keyLength > MAX_KEY_LENGTH) { 
          return false;
     }
     if(keyDataStruct.keys[keyIndex] == NULL) { 
          keyDataStruct.keys[keyIndex] = (uint8_t *) malloc(keyLength * sizeof(uint8_t));
     }
     else if(keyDataStruct.keyLengths[keyIndex] < keyLength) {
          keyDataStruct.keys[keyIndex] = (uint8_t *) realloc(keyDataStruct.keys[keyIndex], keyLength);
     }
     for(int b = 0; b < keyLength; b++) { 
          keyDataStruct.keys[keyIndex][b] = keyData[b];
     }
     WriteEEPBlock(EEP_KEY_DATA_START, &keyDataStruct, sizeof(keyDataStruct)); // store "forever" 
     return true;
}

bool ZeroFillKeyData(KeyData_t &keyDataStruct, size_t keyIndex, size_t keyLength) {
     if(keyIndex < 0 || keyIndex >= NUM_KEYS_STORAGE || keyLength <= 0 || 
	keyLength > MAX_KEY_LENGTH) {
          return false;
     }
     uint8_t *zeroBuf = (uint8_t *) malloc(keyLength * sizeof(uint8_t));
     memset(zeroBuf, 0, keyLength);
     bool success = SetKeyData(keyDataStruct, keyIndex, zeroBuf, keyLength);
     free(zeroBuf);
     return success;
}

bool KeyIsValid(KeyData_t &keyDataStruct, size_t keyIndex) { 
     if(keyIndex < 0 || keyIndex >= NUM_KEYS_STORAGE) {
          return false;
     }
     return keyDataStruct.keys[keyIndex] != NULL;
}

Cipher_t CreateBlockCipherObject(const uint8_t *keyData, size_t keyLength, 
		                 const uint8_t *initVecData, size_t ivLength) { 
     Cipher_t cipherObj;
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
     return CreateBlockCipherObject(ActiveConfiguration.KeyData.keys[keyIndex], 
		                    ActiveConfiguration.KeyData.keyLengths[keyIndex], 
		                    initVecData, ivLength);
}

uint8_t * DecryptDumpImage(Cipher_t cipherObj, const uint8_t *byteBuf, size_t byteBufLen) { 
     if(byteBuf == NULL || byteBufLen <= 0) { 
          return NULL;
     }
     uint8_t *plaintextBuf = (uint8_t *) malloc(byteBufLen * sizeof(uint8_t));
     cipherObj.decrypt(plaintextBuf, byteBuf, byteBufLen);
     return plaintextBuf;
}

