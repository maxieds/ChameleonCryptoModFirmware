/* AESCryptoCPP.cpp : 
 * Author: Maxie D. Schmidt
 * Created: 2019.02.05
 */

#include <string.h>

#include <AESCipherType.h>
#include <AESCrypto/AES.h>
#include <AESCrypto/CFB.h>

typedef CFB< AES128 > AESCFBCipher_t;

extern "C" {
     AESCipher_t * CreateNewCipherObject();
     void DeleteCipherObject(AESCipher_t *cipher);
     void ClearCipherObject(AESCipher_t *cipher);
     bool SetCipherKey(AESCipher_t *cipher, const uint8_t *keyData, size_t keyLength);
     bool SetCipherSalt(AESCipher_t *cipher, const uint8_t *saltData, size_t saltByteCount);
     size_t GetCipherBlockSize(AESCipher_t *cipher);
     bool EncryptDataBuffer(AESCipher_t *cipher, uint8_t *encryptedDataBuf,
                            const uint8_t *ptextDataBuf, size_t dataBufByteCount);
     bool DecryptDataBuffer(AESCipher_t *cipher, uint8_t *ptextDataBuf,
                            uint8_t *encDataBuf, size_t dataBufByteCount);
}

