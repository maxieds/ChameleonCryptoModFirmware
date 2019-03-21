/* AESCrypto.h : C Wrapper for the external AES implementations;
 * Author: Maxie D. Schmidt
 * Created: 2019.02.03
 */

#ifndef __AES_CRYPTO_H__
#define __AES_CRYPTO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stddef.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#include "AESCipherType.h"

AESCipher_t * CreateNewCipherObject();
void DeleteCipherObject(AESCipher_t *cipher);
void ClearCipherObject(AESCipher_t *cipher);
bool SetCipherKey(AESCipher_t *cipher, const uint8_t *keyData, size_t keyLength);
bool SetCipherSalt(AESCipher_t *cipher, const uint8_t *saltData, size_t saltByteCount);
bool EncryptDataBuffer(AESCipher_t *cipher, uint8_t *encryptedDataBuf, 
		       const uint8_t *ptextDataBuf, size_t dataBufByteCount);
bool DecryptDataBuffer(AESCipher_t *cipher, uint8_t *ptextDataBuf, 
		       uint8_t *encDataBuf, size_t dataBufByteCount);

extern volatile bool __CipherObjectMutex;
extern AESCipher_t __CipherObject;

#ifdef __cplusplus
}
#endif

#endif
