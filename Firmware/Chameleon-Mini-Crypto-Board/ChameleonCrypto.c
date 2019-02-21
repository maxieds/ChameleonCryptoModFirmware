/* Crypto.c : Implement the cryptographic helper functions for loading encrypted dumps into memory;
 * Author: Maxie D. Schmidt
 * Created: 01/24/2019
 */

#include <stdlib.h>

#include <util/delay.h>
#include <util/crc16.h>

#include <AESCrypto.h>

#include "ChameleonCrypto.h"
#include "Settings.h"
#include "Terminal/Commands.h"
#include "Terminal/Terminal.h"
#include "System.h"
#include "LED.h"
#include "LEDHook.h"

uint8_t CryptoUploadBuffer[CRYPTO_UPLOAD_BUFSIZE];
uint16_t CryptoUploadBufferByteCount;

void InitCryptoDumpBuffer() { 
     uint8_t zeroFillByte = 0x00;
     memset(CryptoUploadBuffer, 0, CRYPTO_UPLOAD_BUFSIZE);
     CryptoUploadBufferByteCount = 0;
}

bool ValidDumpImageHeader(uint8_t *dumpDataBuf, size_t bufLength) { 
     if(!bufLength || bufLength < CRYPTO_UPLOAD_HEADER_SIZE || dumpDataBuf == NULL) { 
          return false;
     }
     bufLength = CRYPTO_UPLOAD_HEADER_SIZE;
     char dumpBytesHexStr[2 * bufLength + 1], headerHexStr[2 * bufLength + 1];
     BufferToHexString(dumpBytesHexStr, 2 * bufLength + 1, dumpDataBuf, bufLength);
     BufferToHexString(headerHexStr, 2 * bufLength + 1, CRYPTO_UPLOAD_HEADER, bufLength);
     if(!strncmp(dumpBytesHexStr, headerHexStr, bufLength)) { 
          return true;
     }
     return false;
}

bool SetKeyData(size_t keyIndex, uint8_t *keyData, size_t keyLength) { 
     if(keyIndex >= NUM_KEYS_STORAGE || !keyData || 
        keyLength <= 0 || keyLength > MAX_KEY_LENGTH) { 
          return false;
     }
     memcpy(&(GlobalSettings.KeyData.keys[keyIndex]), keyData, keyLength);
     SETTING_UPDATE(GlobalSettings.KeyData.keys);
     GlobalSettings.KeyData.keyLengths[keyIndex] = keyLength;
     SETTING_UPDATE(GlobalSettings.KeyData.keyLengths);
     return true;
}

bool ZeroFillKeyData(size_t keyIndex, size_t keyLength) {
     if(keyIndex >= NUM_KEYS_STORAGE || keyLength <= 0 || 
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
     if(keyIndex >= NUM_KEYS_STORAGE) {
          return false;
     }
     size_t keyLength = GlobalSettings.KeyData.keyLengths[keyIndex];
     return (keyLength > 0) && (keyLength <= MAX_KEY_LENGTH);
}

bool PassphraseToAESKeyData(size_t keyIndex, const char *passphrase) { 
     if(passphrase == NULL) { 
          return false;
     }
     else if(keyIndex >= NUM_KEYS_STORAGE) { 
          return false;
     }
     uint8_t *pphBytes = (uint8_t *) passphrase;
     uint8_t pphByteCount = strlen(passphrase);
     SHAHash_t *hasherObj = GetNewHasherObject();
     uint8_t *firstRoundHashBytes = ComputeHashBytes(hasherObj, pphBytes, pphByteCount);
     size_t frhbCount = GetHashByteCount(hasherObj);
     uint8_t *keyData = ComputeHashBytes(hasherObj, firstRoundHashBytes, frhbCount);
     size_t keyDataSize = GetHashByteCount(hasherObj);
     ClearHashInitData(hasherObj);
     DeleteHasherObject(hasherObj);
     if(firstRoundHashBytes != NULL) {
          free(firstRoundHashBytes);
     }
     bool setStatus = SetKeyData(keyIndex, keyData, MIN(keyDataSize, MAX_KEY_LENGTH));
     if(keyData != NULL) { 
          free(keyData);
     }
     return setStatus;
}

Cipher_t PrepareBlockCipherObject(const uint8_t *keyData, size_t keyLength,  
		                  const uint8_t *initVecData, size_t ivLength) { 
     AESCipher_t *cipherObj = CreateNewCipherObject();
     if(cipherObj != NULL && (keyData == NULL || !keyLength || initVecData == NULL || !ivLength)) {
          DeleteCipherObject(cipherObj);
          return NULL;
     }
     else if(cipherObj == NULL) {
          return NULL;
     }
     if(SetCipherKey(cipherObj, keyData, keyLength) && 
        SetCipherSalt(cipherObj, initVecData, ivLength)) { 
          return cipherObj;
     }
     DeleteCipherObject(cipherObj);
     return NULL;
}

Cipher_t PrepareBlockCipherObjectFromKeyIndex(size_t keyIndex, 
		                              const uint8_t *initVecData, size_t ivLength) { 
     if(keyIndex < 0 || keyIndex >= NUM_KEYS_STORAGE) {
	  return NULL;
     }
     uint8_t keyData[MAX_KEY_LENGTH];
     size_t keyLength = GlobalSettings.KeyData.keyLengths[keyIndex];
     memcpy(keyData, &(GlobalSettings.KeyData.keys[keyIndex]), keyLength);
     return PrepareBlockCipherObject(keyData, keyLength, initVecData, ivLength);
}

uint8_t * DecryptDumpImage(Cipher_t cipherObj, const uint8_t *ivSaltBytes, uint16_t saltByteCount, 
		           const uint8_t *byteBuf, uint16_t byteBufLen) { 
     if(cipherObj == NULL || byteBuf == NULL || byteBufLen <= 0) { 
          return NULL;
     }
     uint8_t *plaintextBuf = CryptoUploadBuffer; 
     if(!DecryptDataBuffer(cipherObj, plaintextBuf, byteBuf, byteBufLen)) {
          return NULL;
     }
     return plaintextBuf;
}

void IndicateCryptoUploadSuccess() { 
     


}

void IndicateCryptoUploadError() {



}

void ResetCryptoUploadError() {



}
