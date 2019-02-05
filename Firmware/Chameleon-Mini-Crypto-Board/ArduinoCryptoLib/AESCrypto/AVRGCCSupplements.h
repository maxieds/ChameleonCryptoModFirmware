/* AVRGCCSupplements.h : Necessary implementations for the new / delete C++ operators when 
 *                       compiling with the avr-gcc compilers, e.g., for the 
 *                       ChameleonMini board source;
 * Author: Maxie D. Schmidt
 * Created: 01/28/2019
 */

#ifndef __AVRGCC_SUPPLEMENTS__
#define __AVRGCC_SUPPLEMENTS__

#include <stdlib.h>

#ifdef NEED_LIBSTDCPP
    extern "C" void __cxa_pure_virtual();
    void* operator new(size_t objsize);
    void operator delete(void *obj);
#endif

#endif
