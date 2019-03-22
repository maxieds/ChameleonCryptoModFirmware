/*
 * Flash.h
 *
 *  Created on: 20.03.2013
 *      Author: skuser
 */

#ifndef MEMORY_H_
#define MEMORY_H_

#define MEMORY_SIZE					(FLASH_DATA_SIZE) /* From makefile */
#define MEMORY_INIT_VALUE			        0x00
//#define MEMORY_SIZE_PER_SETTING		                8192
#define MEMORY_SIZE_PER_SETTING		                4096

#ifndef __ASSEMBLER__

#include "Common.h"

void MemoryInit(void);
void MemoryReadBlock(void* Buffer, uint16_t Address, uint16_t ByteCount);
void MemoryWriteBlock(const void* Buffer, uint16_t Address, uint16_t ByteCount);
void MemoryClear(void);
void MemoryRecall(void);
void MemoryStore(void);

/* For use with XModem */
bool MemoryUploadBlock(void* Buffer, uint32_t BlockAddress, uint16_t ByteCount);
bool CryptoMemoryUploadBlock(void *Buffer, uint32_t BlockAddress, uint16_t ByteCount);
bool MemoryDownloadBlock(void* Buffer, uint32_t BlockAddress, uint16_t ByteCount);

/* EEPROM functions */
uint16_t WriteEEPBlock(uint16_t Address, const void *SrcPtr, uint16_t ByteCount);
uint16_t ReadEEPBlock(uint16_t Address, void *DestPtr, uint16_t ByteCount);

/* Declarations from assembler file */
uint16_t FlashReadWord(uint32_t Address);
void FlashEraseApplicationPage(uint32_t Address);
void FlashLoadFlashWord(uint16_t Address, uint16_t Data);
void FlashEraseWriteApplicationPage(uint32_t Address);
void FlashEraseFlashBuffer(void);
void FlashWaitForSPM(void);

#endif /* __ASSEMBLER__ */

#endif /* MEMORY_H_ */
