#include "AVRGCCSupplements.h" 

void __cxa_pure_virtual() { while (1); }

void* operator new(size_t objsize) { 
    return malloc(objsize); 
} 

void operator delete(void* obj) { 
    free(obj); 
}
