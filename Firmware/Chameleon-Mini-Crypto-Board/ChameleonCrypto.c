/* ChameleonCrypto.c : Implement the cryptographic helper functions for loading encrypted dumps into memory;
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
#include "Common.h"

uint8_t CryptoUploadBuffer[CRYPTO_UPLOAD_BUFSIZE];
uint16_t CryptoUploadBufferByteCount;

void InitCryptoDumpBuffer() { 
     memset(CryptoUploadBuffer, 0, CRYPTO_UPLOAD_BUFSIZE);
     CryptoUploadBufferByteCount = 0;
}

bool VerifyDataHash(uint8_t *dataHashBytes, uint8_t *dataBytes, uint16_t dataByteCount) {
     if(dataHashBytes == NULL || dataBytes == NULL) {
          return false;
     }
     SHAHash_t *hasherObj = GetNewHasherObject();
     uint16_t hashDataSize = GetHashByteCount(hasherObj);
     uint8_t actualDataHashBytes[hashDataSize];
     ComputeHashBytes(hasherObj, actualDataHashBytes, hashDataSize, dataBytes, dataByteCount);
     if(hashDataSize != CRYPTO_UPLOAD_HEADER_SIZE) {
	  return false;
     }
     int hashCompareResult = memcmp(dataHashBytes, actualDataHashBytes, CRYPTO_UPLOAD_HEADER_SIZE);
     DeleteHasherObject(hasherObj);
     if(hashCompareResult) {
          return false;
     }
     return true;
}

bool ValidDumpImageHeader(uint8_t *dumpDataBuf, uint16_t bufLength) { 
     if(!bufLength || bufLength < CRYPTO_UPLOAD_HEADER_SIZE || dumpDataBuf == NULL) { 
          return false;
     }
     uint8_t dataHeaderBytes[CRYPTO_UPLOAD_HEADER_SIZE];
     memcpy(dataHeaderBytes, dumpDataBuf, CRYPTO_UPLOAD_HEADER_SIZE);
     if(!VerifyDataHash(dataHeaderBytes, dumpDataBuf + CRYPTO_UPLOAD_HEADER_SIZE, 
			bufLength - CRYPTO_UPLOAD_HEADER_SIZE)) {
          return false;
     }
     return true;
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
     uint8_t zeroBuf[keyLength];
     memset(zeroBuf, 0, keyLength);
     bool success = SetKeyData(keyIndex, zeroBuf, keyLength);
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
     uint8_t pphByteCount = strlen_FW(passphrase);
     SHAHash_t *hasherObj = GetNewHasherObject();
     uint16_t hashDataBytes = GetHashByteCount(hasherObj);
     uint8_t firstRoundHashBytes[hashDataBytes], keyData[hashDataBytes];
     ComputeHashBytes(hasherObj, firstRoundHashBytes, hashDataBytes, pphBytes, pphByteCount);
     ComputeHashBytes(hasherObj, keyData, hashDataBytes, firstRoundHashBytes, hashDataBytes);
     ClearHashInitData(hasherObj);
     DeleteHasherObject(hasherObj);
     bool setStatus = SetKeyData(keyIndex, keyData, MIN(hashDataBytes, MAX_KEY_LENGTH));
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
          DeleteCipherObject(cipherObj);
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
