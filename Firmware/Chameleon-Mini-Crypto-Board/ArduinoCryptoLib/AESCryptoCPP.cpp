/* AESCryptoCPP.cpp : 
 * Author: Maxie D. Schmidt
 * Created: 2019.02.05
 */

#include <string.h>

#include <AESCipherType.h>
#include <AESCrypto/AES.h>
#include <AESCrypto/CFB.h>

typedef CFB< AESTiny128 > AESCFBCipher_t;

/* C++ versions of these functions: */
inline AESCipher_t * CreateNewCipherObject() {
     return new AESCipher_t();
}

inline void DeleteCipherObject(AESCipher_t *cipher) {
     if(cipher != NULL) {
	  delete cipher;
     }
}

bool SetCipherKey(AESCipher_t *cipher, const uint8_t *keyData, size_t keyLength);
bool SetCipherSalt(AESCipher_t *cipher, const uint8_t *saltData, size_t saltByteCount);
size_t GetCipherBlockSize(AESCipher_t *cipher);
bool EncryptDataBuffer(AESCipher_t *cipher, uint8_t *encryptedDataBuf,
                       const uint8_t *ptextDataBuf, size_t dataBufByteCount);
bool DecryptDataBuffer(AESCipher_t *cipher, uint8_t *ptextDataBuf,
                       const uint8_t *encDataBuf, size_t dataBufByteCount);
