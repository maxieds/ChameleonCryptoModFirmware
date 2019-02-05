/* SHAHash.cpp : 
 * Author: Maxie D. Schmidt
 * Created: 2019.02.03
 */

#include "SHAHash.h"
#include "SHAHash/SHA256.h"

SHAHash_t * GetNewHasherObject() {
     return new SHAHash_t();
}

void DeleteHasherObject(SHAHash_t *hasherObj) { 
     if(hasherObj != NULL) { 
          delete hasherObj;
	  hasherObj = NULL;
     }
}

uint8_t * ComputeHashBytes(SHAHash_t *hasherObj, const uint8_t *saltData, size_t saltBytesLen, 
		           const uint8_t *dataBytes, size_t dataByteCount) {
     if(hasherObj == NULL || saltData == NULL || dataBytes == NULL) {
          return NULL;
     }
     hasherObj->resetHMAC(saltData, saltBytesLen);
     hasherObj->update(dataBytes, dataByteCount);
     uint8_t *hashData = (uint8_t *) malloc(256 * sizeof(uint8_t));
     hasherObj->finalizeHMAC(saltData, saltBytesLen, hashData, 256);
     return hashData;
}

size_t GetHashByteCount(SHAHash_t *hasherObj) { 
     if(hasherObj == NULL) { 
          return 0;
     }
     return hasherObj->hashSize();
}

void ClearHashInitData(SHAHash_t *hasherObj) { 
     if(hasherObj != NULL) {
          hasherObj->reset();
     }
}
