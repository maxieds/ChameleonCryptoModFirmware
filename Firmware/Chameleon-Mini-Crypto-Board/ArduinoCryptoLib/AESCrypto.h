/* AESCrypto.h : C Wrapper for the external AES implementations;
 * Author: Maxie D. Schmidt
 * Created: 2019.02.03
 */

#ifndef __AES_CRYPTO_H__
#define __AES_CRYPTO_H__

#include <stddef.h>
#include <inttypes.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AESTiny128 AESTiny128;
typedef struct AESSmall128 AESCipher_t;

AESCipher_t * CreateNewCipherObject();
void DeleteCipherObject(AESCipher_t *cipher);
bool SetCipherKey(AESCipher_t *cipher, const uint8_t *keyData, size_t keyLength);
size_t GetCipherBlockSize(AESCipher_t *cipher);
bool EncryptDataBuffer(AESCipher_t *cipher, uint8_t *encryptedDataBuf, 
		       const uint8_t *ptextDataBuf, size_t dataBufByteCount);
bool DecryptDataBuffer(AESCipher_t *cipher, uint8_t *ptextDataBuf, 
		       const uint8_t *encDataBuf, size_t dataBufByteCount);

#ifdef __cplusplus
}
#endif

#endif
