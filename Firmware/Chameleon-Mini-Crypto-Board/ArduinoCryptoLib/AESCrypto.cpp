/* AESCrypto.cpp : 
 * Author: Maxie D. Schmidt 
 * Created: 2019.02.03
 */

#include <stdlib.h>
#include <stdio.h>

#include "AESCryptoCPP.cpp"

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
     else if(!cipher->ivSize()) {
          return true;
     }
     return cipher->setIV(saltData, saltByteCount);
}

bool EncryptDataBuffer(AESCipher_t *cipher, uint8_t *encDataBuf, 
		       const uint8_t *ptextDataBuf, size_t dataByteCount) { 
     if(cipher == NULL || encDataBuf == NULL || ptextDataBuf == NULL) { 
          if(cipher != NULL) {
               cipher->clear();
	  }
	  return false;
     }
     cipher->encrypt(encDataBuf, ptextDataBuf, dataByteCount);
     cipher->clear();
     return true;
}

bool DecryptDataBuffer(AESCipher_t *cipher, uint8_t *ptextDataBuf, const uint8_t *encDataBuf, 
		       size_t dataBufByteCount) { 
     if(cipher == NULL || ptextDataBuf == NULL || encDataBuf == NULL) { 
	  cipher->clear();
          return false;
     }
     cipher->decrypt(ptextDataBuf, encDataBuf, dataBufByteCount);
     cipher->clear();
     return true;
}
