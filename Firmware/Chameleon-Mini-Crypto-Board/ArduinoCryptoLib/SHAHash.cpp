/* SHAHash.cpp : 
 * Author: Maxie D. Schmidt
 * Created: 2019.02.03
 */

#include "SHAHash/SHA256.h"
#include "SHAHash.h"

volatile bool __SHAHasherObjectMutex = false;
SHAHash_t __SHAHasherObject;

SHAHash_t * GetNewHasherObject() {
     //if(__SHAHasherObject == NULL) {
     //     __SHAHasherObject = new SHAHash_t();
     //}
     if(__SHAHasherObjectMutex) {
          return NULL;
     }
     __SHAHasherObjectMutex = true;
     return &__SHAHasherObject;
}

void DeleteHasherObject(SHAHash_t *hasherObj) { 
     if(hasherObj != NULL) { 
          //free(hasherObj);
	  hasherObj->reset();
     }
     __SHAHasherObjectMutex = false;
}

uint8_t * ComputeHashBytes(SHAHash_t *hasherObj, uint8_t *hashData, 
		           uint16_t hashDataSize, 
		           const uint8_t *dataBytes, uint16_t dataByteCount) {
     if(hasherObj == NULL || hashData == NULL || dataBytes == NULL) {
          return NULL;
     }
     hasherObj->update(dataBytes, dataByteCount);
     hasherObj->finalize(hashData, hashDataSize);
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
          hasherObj->clear();
     }
}
