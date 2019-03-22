/* AESCrypto.cpp : 
 * Author: Maxie D. Schmidt 
 * Created: 2019.02.03
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "AESCryptoCPP.cpp"

volatile bool __CipherObjectMutex = false;
AESCipher_t __CipherObject;

/* C++ versions of these functions: */
AESCipher_t * CreateNewCipherObject() {
     //if(__CipherObject == NULL) {
     //     __CipherObject = new AESCipher_t();
     //}
     if(__CipherObjectMutex) {
          return NULL;
     }
     __CipherObjectMutex = true;
     return &__CipherObject;
}

void DeleteCipherObject(AESCipher_t *cipher) {
     if(cipher != NULL) {
     	  cipher->clear();
          //free(cipher);
     }
     __CipherObjectMutex = false;
}

void ClearCipherObject(AESCipher_t *cipher) {
     if(cipher != NULL) {
          cipher->clear();
     }
}

bool SetCipherKey(AESCipher_t *cipher, const uint8_t *keyData, size_t keyLength) {
     if(cipher == NULL || keyData == NULL) {
          return false;
     }
     cipher->setKey(keyData, keyLength);
     return true;
}

bool SetCipherSalt(AESCipher_t *cipher, const uint8_t *saltData, size_t saltByteCount) {
     if(cipher == NULL || saltData == NULL) {
          return false;
     }
     return cipher->setIV(saltData, saltByteCount);
}

bool EncryptDataBuffer(AESCipher_t *cipher, uint8_t *encDataBuf, 
		       const uint8_t *ptextDataBuf, size_t dataByteCount) { 
     if(cipher == NULL || encDataBuf == NULL || ptextDataBuf == NULL) { 
	  return false;
     }
     //int offset, curlen;
     //for(offset = 0; offset < dataByteCount; offset += AES_BLOCK_SIZE) {
     //   curlen = dataByteCount - offset;
     //   if (curlen > AES_BLOCK_SIZE)
     //       curlen = AES_BLOCK_SIZE;
     //   cipher->encryptBlock(encDataBuf + offset, ptextDataBuf + offset, curlen);
     //}
     cipher->encrypt(encDataBuf, ptextDataBuf, dataByteCount);
     return true;
}

bool DecryptDataBuffer(AESCipher_t *cipher, uint8_t *ptextDataBuf, uint8_t *encDataBuf, 
		       size_t dataBufByteCount) { 
     if(cipher == NULL || ptextDataBuf == NULL || encDataBuf == NULL) { 
          return false;
     }
     //int offset, curlen;
     //for (offset = 0; offset < dataBufByteCount; offset += AES_BLOCK_SIZE) {
     //   curlen = dataBufByteCount - offset;
     //   if (curlen > AES_BLOCK_SIZE)
     //       curlen = AES_BLOCK_SIZE;
     //   cipher->decryptBlock(ptextDataBuf + offset, encDataBuf + offset, curlen);
     //}
     cipher->decrypt(ptextDataBuf, encDataBuf, dataBufByteCount);
     return true;
}
