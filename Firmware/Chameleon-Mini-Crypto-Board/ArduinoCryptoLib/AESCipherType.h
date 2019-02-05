/* AESCipherType.h : Includes just the typedef information for AESCipher_t;
 * Author: Maxie D. Schmidt
 * Created: 2019.02.05
 */

#ifndef __AESCIPHERTYPE_H__
#define __AESSIPHERTYPE_H__

#define AES_BLOCK_SIZE                (16)
#define AES_IV_SIZE                   (16)

typedef struct AES128 AES128;
#if !defined(__cplusplus)
     struct AESCFBCipher_t;
     typedef struct AESCFBCipher_t AESCipher_t;
#else
     #include <AESCrypto/AES.h>
     #include <AESCrypto/CFB.h>
     typedef CFB<AES128> AESCipher_t;
#endif

#endif
