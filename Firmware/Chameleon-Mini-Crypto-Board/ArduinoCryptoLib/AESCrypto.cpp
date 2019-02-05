/* AESCrypto.cpp : 
 * Author: Maxie D. Schmidt 
 * Created: 2019.02.03
 */

#include "AESCrypto/AES.h"
#include "AESCrypto.h"

extern "C" {

AESCipher_t * CreateNewCipherObject() {
     return new AESCipher_t();
}

void DeleteCipherObject(AESCipher_t *cipher) { 
     if(cipher != NULL) {
          delete cipher;
          cipher = NULL;
     }
}

bool SetCipherKey(AESCipher_t *cipher, const uint8_t *keyData, size_t keyLength) {
     if(cipher == NULL || keyData == NULL) {
          return false;
     }
     cipher->setKey(keyData, keyLength);
     return true;
}

size_t GetCipherBlockSize(AESCipher_t *cipher) {
     if(cipher == NULL) { 
          return 0;
     }
     return cipher->blockSize();
}

bool EncryptDataBuffer(AESCipher_t *cipher, uint8_t *encDataBuf, 
		       const uint8_t *ptextDataBuf, size_t dataByteCount) { 
     if(cipher == NULL || encDataBuf == NULL || ptextDataBuf == NULL) { 
          cipher->clear();
	  return false;
     }
     size_t blockSize = GetCipherBlockSize(cipher);
     if(dataByteCount % blockSize) { 
	  cipher->clear();
          return false;
     }
     size_t totalBlocks = dataByteCount / blockSize;
     for(size_t b = 0; b < totalBlocks; b++) { 
          size_t dataBufOffset = b * blockSize;
	  cipher->encryptBlock(encDataBuf + dataBufOffset, ptextDataBuf + dataBufOffset);
	       return false;
     }
     cipher->clear();
     return true;
}

bool DecryptDataBuffer(AESCipher_t *cipher, uint8_t *ptextDataBuf, const uint8_t *encDataBuf, 
		       size_t dataBufByteCount) { 
     if(cipher == NULL || ptextDataBuf == NULL || encDataBuf == NULL) { 
	  cipher->clear();
          return false;
     }
     size_t blockSize = GetCipherBlockSize(cipher);
     if(dataBufByteCount % blockSize) { 
          cipher->clear();
	  return false;
     }
     size_t totalBlocks = dataBufByteCount / blockSize;
     for(size_t b = 0; b < totalBlocks; b++) { 
          size_t blockOffset = b * blockSize;
	  cipher->decryptBlock(ptextDataBuf + blockOffset, encDataBuf + blockOffset);
     }
     cipher->clear();
     return true;
}

}
