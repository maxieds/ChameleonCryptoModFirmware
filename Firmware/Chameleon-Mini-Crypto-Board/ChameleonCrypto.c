/* Crypto.c : Implement the cryptographic helper functions for loading encrypted dumps into memory;
 * Author: Maxie D. Schmidt
 * Created: 01/24/2019
 */

#include <stdlib.h>

#include <util/delay.h>
#include <util/crc16.h>

#include "ChameleonCrypto.h"
#include "Settings.h"
#include "Terminal/Commands.h"
#include "Terminal/Terminal.h"
#include "System.h"
#include "LED.h"
#include "LEDHook.h"

uint8_t EEMEM CryptoUploadBuffer[CRYPTO_UPLOAD_BUFSIZE];
size_t EEMEM CryptoUploadBufferByteCount;

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
     uint8_t pphBytes[MAX_KEY_LENGTH];
     uint8_t pphByteCount = HexStringToBuffer(pphBytes, MAX_KEY_LENGTH, passphrase);
     SHAHash_t *hasherObj = GetNewHasherObject();
     uint8_t *firstRoundHashBytes = ComputeHashBytes(hasherObj, pphBytes, pphByteCount, 
		                    pphBytes, pphByteCount);
     size_t frhbCount = GetHashByteCount(hasherObj);
     uint8_t *keyData = ComputeHashBytes(hasherObj, firstRoundHashBytes, 
                                        frhbCount, pphBytes, pphByteCount);
     size_t keyDataSize = GetHashByteCount(hasherObj);
     ClearHashInitData(hasherObj);
     DeleteHasherObject(hasherObj);
     if(firstRoundHashBytes != NULL) {
          free(firstRoundHashBytes);
     }
     bool setStatus = SetKeyData(keyIndex, keyData, keyDataSize);
     if(keyData != NULL) { 
          free(keyData);
     }
     return setStatus;
}

Cipher_t PrepareBlockCipherObject(const uint8_t *keyData, size_t keyLength,  
		                  const uint8_t *initVecData, size_t ivLength) { 
     AESCipher_t *cipherObj = CreateNewCipherObject();
     if(cipherObj == NULL || keyData == NULL || !keyLength || initVecData == NULL || !ivLength) {
          return cipherObj;
     }
     // use the salt with the keyData to generate a new key for the operation:
     uint8_t *saltedKey = (uint8_t *) malloc(keyLength * sizeof(uint8_t));
     memcpy(saltedKey, keyData, keyLength);
     for(int ctr = 0; ctr < MIN(keyLength, ivLength); ctr++) {
          saltedKey[ctr] = keyData[ctr] ^ initVecData[ctr];
     }
     if(SetCipherKey(cipherObj, saltedKey, keyLength)) { 
          return cipherObj;
     }
     DeleteCipherObject(cipherObj);
     return NULL;
}

Cipher_t PrepareBlockCipherObjectFromKeyIndex(size_t keyIndex, 
		                              const uint8_t *initVecData, size_t ivLength) { 
     if(keyIndex < 0 || keyIndex >= NUM_KEYS_STORAGE) {
          Cipher_t cipherObj;
	  return cipherObj;
     }
     uint8_t keyData[MAX_KEY_LENGTH];
     size_t keyLength = GlobalSettings.KeyData.keyLengths[keyIndex];
     ReadEEPBlock((uint16_t) &(GlobalSettings.KeyData.keys[keyIndex]), keyData, keyLength);
     return PrepareBlockCipherObject(keyData, keyLength, initVecData, ivLength);
}

uint8_t * DecryptDumpImage(Cipher_t cipherObj, const uint8_t *byteBuf, 
		           size_t byteBufLen) { 
     if(byteBuf == NULL || byteBufLen <= 0) { 
          return NULL;
     }
     return NULL;
     uint8_t *plaintextBuf = (uint8_t *) malloc(byteBufLen * sizeof(uint8_t));
     uint8_t byteBufLocalCopy[MAX_KEY_LENGTH];
     ReadEEPBlock((uint16_t) &byteBuf, byteBufLocalCopy, byteBufLen);
     if(!DecryptDataBuffer(cipherObj, plaintextBuf, byteBufLocalCopy, byteBufLen)) { 
          free(plaintextBuf);
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
