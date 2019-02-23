/* SHAHash.cpp : 
 * Author: Maxie D. Schmidt
 * Created: 2019.02.03
 */

#include "SHAHash/SHA256.h"
#include "SHAHash.h"

//volatile bool __SHAHasherObjectMutex = false;
//SHAHash_t *__SHAHasherObject = NULL;
//static SHAHash_t __LocalSHAHasherObject;

SHAHash_t * GetNewHasherObject() {
     //return (SHAHash_t *) malloc(SHAHASHT_SIZE);
     return new SHAHash_t();
     /*if(__SHAHasherObject == NULL) {
          __SHAHasherObject = &__LocalSHAHasherObject; //new SHAHash_t();
     }
     if(__SHAHasherObjectMutex) {
          return NULL;
     }
     __SHAHasherObjectMutex = true;
     return __SHAHasherObject;*/
}

void DeleteHasherObject(SHAHash_t *hasherObj) { 
     if(hasherObj != NULL) { 
          free(hasherObj);
          //delete hasherObj;
     }
     //__SHAHasherObjectMutex = false;
}

uint8_t * ComputeHashBytes(SHAHash_t *hasherObj, const uint8_t *dataBytes, uint16_t dataByteCount) {
     if(hasherObj == NULL || dataBytes == NULL) {
          return NULL;
     }
     //hasherObj->clear();
     /*size_t roundSize = 64;
     uint16_t numRounds = dataByteCount / roundSize + ((dataByteCount % roundSize) ? 1 : 0);
     for(int r = 0; r < numRounds; r++) { 
	  size_t curRoundProcessSize = ((dataByteCount % roundSize) && r == (numRounds - 1)) ? 
		                       dataByteCount % roundSize : roundSize;
          hasherObj->update(dataBytes + r * roundSize, curRoundProcessSize);
     }*/
     hasherObj->update(dataBytes, dataByteCount);
     uint16_t hashDataSize = GetHashByteCount(hasherObj);
     uint8_t *hashData = (uint8_t *) malloc(hashDataSize * sizeof(uint8_t));
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
