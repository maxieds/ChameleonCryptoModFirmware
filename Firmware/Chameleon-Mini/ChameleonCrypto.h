/* ChameleonCrypto.h : Defines and helper functions for handling the new encrypted dump uploads;
 * Author: Maxie D. Schmidt
 * Created: 01/24/2019
 */

#ifndef __CHAMELEON_CRYPTO_H__
#define __CHAMELEON_CRYPTO_H__

#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>

#include <CTR.h>
#include <CFB.h>
#include <OFB.h>
#include <CBC.h>
#include <AES.h>

#include "Memory.h"
#include "Log.h"
#include "Common.h"

#if !defined(NUM_KEYS_STORAGE) || NUM_KEYS_STORAGE <= 0
     #define NUM_KEYS_STORAGE          (4)
#endif
#define MAX_KEY_LENGTH            (BLOCK_CIPHER_KEY_BYTE_SIZE)

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

typedef size_t KeyAuth_t; // Stores: > 0 indicating number of remaining key data edits if authenticated 
typedef struct {
     uint8_t keys[NUM_KEYS_STORAGE][MAX_KEY_LENGTH];
     size_t keyLengths[NUM_KEYS_STORAGE];
} KeyData_t;

extern const KeyData_t DEFAULT_KEY_DATA;

#define CRYPTO_UPLOAD_HEADER      "MFCLASSIC1K-DUMPIMAGE::\n"
#define CRYPTO_UPLOAD_HEADER_SIZE 24
#define CRYPTO_UPLOAD_BUFSIZE     (1024 + CRYPTO_UPLOAD_HEADER_SIZE) // for MF1K dump sizes

extern uint8_t CryptoUploadBuffer[CRYPTO_UPLOAD_BUFSIZE];
extern size_t CryptoUploadBufferByteCount;

void InitCryptoDumpBuffer();
bool ValidDumpImageHeader(uint8_t *dumpDataBuf, size_t bufLength);

uint8_t * GetKeyDataFromString(const char *byteString, size_t *byteCount);
bool SetKeyData(size_t keyIndex, uint8_t *keyData, size_t keyLength);
bool ZeroFillKeyData(size_t keyIndex, size_t keyLength);
bool KeyIsValid(size_t keyIndex); 
size_t PassphraseToAESKeyData(const char *passphrase, uint8_t *keyDataBuffer, size_t maxKeyDataBytes, 		                                  bool useTickTimeSalt = true);

Cipher_t CreateBlockCipherObject(const uint8_t *keyData, size_t keyLength, 
		                 const uint8_t *initVecData, size_t ivLength);
Cipher_t CreateBlockCipherObjectFromKeyIndex(size_t keyIndex, const uint8_t *initVecData, size_t ivLength); 
uint8_t * DecryptDumpImage(Cipher_t cipherObj, const uint8_t *byteBuf, size_t byteBufLen);

#endif
