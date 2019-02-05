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

typedef struct SHA256 SHAHash_t;

SHAHash_t * GetNewHasherObject();
void DeleteHasherObject(SHAHash_t *hasherObj);
uint8_t * ComputeHashBytes(SHAHash_t *hasherObj, const uint8_t *saltData, size_t saltBytesLength, 
		           const uint8_t *dataBytes, size_t dataByteCount);
size_t GetHashByteCount(SHAHash_t *hasherObj);
void ClearHashInitData(SHAHash_t *hasherObj);

#ifdef __cplusplus
}
#endif

#endif
