/* SHAHash.h : 
 * Author: Maxie D. Schmidt
 * Created: 2019.02.03
 */

#ifndef __SHA_HASH_H__
#define __SHA_HASH_H__

#include <stdlib.h>
#include <stddef.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SHAHASHT_SIZE                        (120)

typedef struct SHA256 SHAHash_t;
//#if !defined(__cplusplus)
//     typedef struct SHA256 SHAHash_t;
//#else
//     #include <SHAHash/SHA256.h>
//     typedef SHA256 SHAHash_t;
//#endif

SHAHash_t * GetNewHasherObject();
void DeleteHasherObject(SHAHash_t *hasherObj);
uint8_t * ComputeHashBytes(SHAHash_t *hasherObj, uint8_t *hashData, uint16_t hashDataSize, 
		           const uint8_t *dataBytes, uint16_t dataByteCount);
size_t GetHashByteCount(SHAHash_t *hasherObj);
void ClearHashInitData(SHAHash_t *hasherObj);

extern volatile bool __SHAHasherObjectMutex;
extern SHAHash_t __SHAHasherObject;

#ifdef __cplusplus
}
#endif

#endif
