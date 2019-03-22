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

#include <AESCrypto.h>
#include <SHAHash.h>

#include "Memory.h"
#include "Log.h"
#include "Common.h"

#if !defined(NUM_KEYS_STORAGE) || NUM_KEYS_STORAGE <= 0
     #define NUM_KEYS_STORAGE          (4)
#endif
#define BLOCK_CIPHER_KEY_BYTE_SIZE      (128 / BITS_PER_BYTE)
#define MAX_KEY_LENGTH                  (BLOCK_CIPHER_KEY_BYTE_SIZE)
#define MF1K_MAX_DUMP_SIZE              (1024)

#define CRYPTO_UPLOAD_HEADER_SIZE (32)
#define CRYPTO_UPLOAD_BUFSIZE     (MF1K_MAX_DUMP_SIZE + CRYPTO_UPLOAD_HEADER_SIZE) //(1152) 

typedef AESCipher_t* Cipher_t;

typedef size_t KeyAuth_t; // Stores: > 0 indicating number of remaining key data edits if authenticated 
typedef struct {
     uint8_t keys[NUM_KEYS_STORAGE][MAX_KEY_LENGTH];
     size_t keyLengths[NUM_KEYS_STORAGE];
} KeyData_t;

/* The uploaded encrypted dump buffer is uploaded in its entirety before decryption 
 * can happen. To keep from running out of space and/or clobbering other heap,stack 
 * variables, we carve out a special one-time placeholder for this buffer data in the EEPROM segment: 
 */
extern uint8_t CryptoUploadBuffer[CRYPTO_UPLOAD_BUFSIZE];
extern uint16_t CryptoUploadBufferByteCount;

/* Administrative checking and prep routines: */
void InitCryptoDumpBuffer();
bool VerifyDataHash(uint8_t *dataHashBytes, uint8_t *dataBytes, uint16_t dataByteCount);
bool ValidDumpImageHeader(uint8_t *dumpDataBuf, uint16_t bufLength);

/* Key management: */
bool SetKeyData(size_t keyIndex, uint8_t *keyData, size_t keyLength);
bool ZeroFillKeyData(size_t keyIndex, size_t keyLength);
bool GenKeyDataFromUSBSerialID(size_t keyIndex);
bool KeyIsValid(size_t keyIndex); 

/* Uses a hash function to hash(hash(passphrase string)) to obtain the long-ish complete key data: */
bool PassphraseToAESKeyData(size_t keyIndex, const char *passphrase);

/* We will use the on-board AES crypto facilities of this advanced AVR chip: */
Cipher_t PrepareBlockCipherObject(const uint8_t *keyData, size_t keyLength, 
	                          const uint8_t *initVecData, size_t ivLength);
Cipher_t PrepareBlockCipherObjectFromKeyIndex(size_t keyIndex, 
	                                      const uint8_t *initVecData, size_t ivLength); 
uint8_t * EncryptDumpImage(Cipher_t cipherObj, const uint8_t *byteBuf, uint16_t byteBufLen);
uint8_t * DecryptDumpImage(Cipher_t cipherObj, const uint8_t *ivSaltBytes, uint16_t saltByteCount, 
		           const uint8_t *byteBuf, uint16_t byteBufLen);

/* Physical indicators of success and/or errors by blinking LEDs on the board, or a permanent 
 * RED LED indicator. The new "UPLOAD_STATUS" command can be used to verify status of the uploading of 
 * encrypted dump image operations for use with an automated library setting: 
 */
void IndicateCryptoUploadSuccess();
void IndicateCryptoUploadError();
void ResetCryptoUploadError();

#endif
