/* ChameleonCrypto.h : Defines and helper functions for handling the new encrypted dump uploads;
 * Author: Maxie D. Schmidt
 * Created: 01/24/2019
 */

#ifndef __CHAMELEON_CRYPTO_H__
#define __CHAMELEON_CRYPTO_H__

#include <CTR.h>
#include <CFB.h>
#include <OFB.h>
#include <CBC.h>
#include <AES.h>

#include "Memory.h"
#include "Log.h"
#include "Common.h"

#ifndef NUM_KEYS_STORAGE || NUM_KEYS_STORAGE <= 0
     #define NUM_KEYS_STORAGE          (4)
#endif
#define MAX_KEY_LENGTH            (256)

#if defined(BLOCK_CIPHER_TYPE_AES128)
     typedef AES128 BlockCipher_t;
     #define BLOCK_CIPHER_KEY_BYTE_SIZE (128 / BITS_PER_BYTE)
#elif defined(BLOCK_CIPHER_TYPE_AES192)
     typedef AES192 BlockCipher_t;
     #define BLOCK_CIPHER_KEY_BYTE_SIZE (192 / BITS_PER_BYTE)
#else
     typedef AES256 BlockCipher_t;
     #define BLOCK_CIPHER_KEY_BYTE_SIZE (256 / BITS_PER_BYTE)
#endif

#if defined(CIPHER_MODE_TYPE_CFB)
     typedef CFB< BlockCipher_t > Cipher_t;
#elif defined(CIPHER_MODE_TYPE_OFB)
     typedef OFB< BlockCipher_t > Cipher_t;
#elif defined(CIPHER_MODE_TYPE_CBC)
     typedef CBC< BlockCipher_t > Cipher_t;
#else
     typedef CTR< BlockCipher_t > Cipher_t;
#endif

typedef struct {
     uint8_t *keys[NUM_KEYS_STORAGE];
     size_t keyLengths[NUM_KEYS_STORAGE];
} KeyData_t;

#define EEP_KEY_DATA_SIZE         ((sizeof(uint8_t) * MAX_KEY_LENGTH + sizeof(size_t)) * NUM_KEYS_STORAGE)
#define EEP_KEY_DATA_START        (FRAM_LOG_START_ADDR + FRAM_LOG_SIZE + 1)
#define EEP_KEY_DATA_END          (EEP_KEY_DATA_START + EEP_KEY_DATA_SIZE)

extern uint8_t CryptoUploadBuffer[MEMORY_SIZE_PER_SETTING];
extern uint16_t CryptoUploadBufferByteCount;

void InitCryptoDumpBuffer();

uint8_t * GetKeyDataFromString(const char *byteString, size_t *byteCount);
bool SetKeyData(KeyData_t &keyDataStruct, size_t keyIndex, uint8_t *keyData, size_t keyLength);
bool ZeroFillKeyData(KeyData_t &keyDataStruct, size_t keyIndex, size_t keyLength);
bool KeyIsValid(KeyData_t &keyDataStruct, size_t keyIndex); 
size_t PassphraseToAESKeyData(const char *passphrase, uint8_t *keyDataBuffer, size_t maxKeyDataBytes, 		                                 bool useTickTimeSalt = true);

Cipher_t CreateBlockCipherObject(const uint8_t *keyData, size_t keyLength, 
		                 const uint8_t *initVecData, size_t ivLength);
Cipher_t CreateBlockCipherObjectFromKeyIndex(size_t keyIndex, const uint8_t *initVecData, size_t ivLength); 
uint8_t * DecryptDumpImage(Cipher_t cipherObj, const uint8_t *byteBuf, size_t byteBufLen);

#endif
