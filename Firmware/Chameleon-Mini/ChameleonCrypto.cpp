/* Crypto.c : Implement the cryptographic helper functions for loading encrypted dumps into memory;
 * Author: Maxie D. Schmidt
 * Created: 01/24/2019
 */

#include <stdlib.h>

#include <SHA3.h>

#include "ChameleonCrypto.h"
#include "Configuration.h"
#include "Terminal/Commands.h"
#include "Terminal/Terminal.h"
#include "System.h"

uint8_t CryptoUploadBuffer[CRYPTO_UPLOAD_BUFSIZE] EEMEM;
uint16_t CryptoUploadBufferByteCount EEMEM;

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
     memcpy(keyDataStruct.keys[keyIndex], keyData, keyLength);
     keyDataStruct.keyLengths[keyIndex] = keyLength;
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

