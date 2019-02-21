#include "Common.h"

const uint8_t PROGMEM BitReverseByteTable[256] = {
#   define R2(n)     n,     n + 2*64,     n + 1*64,     n + 3*64
#   define R4(n) R2(n), R2(n + 2*16), R2(n + 1*16), R2(n + 3*16)
#   define R6(n) R4(n), R4(n + 2*4 ), R4(n + 1*4 ), R4(n + 3*4 )
    R6(0), R6(2), R6(1), R6(3)
};

const uint8_t PROGMEM OddParityByteTable[] = {
    1, 	0, 	0, 	1, 	0, 	1, 	1,	0,	0,	1,	1,	0, 	1, 	0, 	0, 	1,
    0, 	1, 	1,	0, 	1, 	0, 	0, 	1, 	1, 	0, 	0, 	1, 	0, 	1, 	1, 	0,
    0,	1,	1,	0,	1,	0,	0,	1,	1,	0,	0,	1,	0,	1,	1,	0,
    1,	0,	0,	1,	0,	1,	1,	0,	0,	1,	1,	0,	1,	0,	0,	1,
    0,	1,	1,	0,	1,	0,	0,	1,	1,	0,	0,	1,	0,	1,	1,	0,
    1,	0,	0,	1,	0,	1,	1,	0,	0,	1,	1,	0,	1,	0,	0,	1,
    1,	0,	0,	1,	0,	1,	1,	0,	0,	1,	1,	0,	1,	0,	0,	1,
    0,	1,	1,	0,	1,	0,	0,	1,	1,	0,	0,	1,	0,	1,	1,	0,
    0,	1,	1,	0,	1,	0,	0,	1,	1,	0,	0,	1,	0,	1,	1,	0,
    1,	0,	0,	1,	0,	1,	1,	0,	0,	1,	1,	0,	1,	0,	0,	1,
    1,	0,	0,	1,	0,	1,	1,	0,	0,	1,	1,	0,	1,	0,	0,	1,
    0,	1,	1,	0,	1,	0,	0,	1,	1,	0,	0,	1,	0,	1,	1,	0,
    1,	0,	0,	1,	0,	1,	1,	0,	0,	1,	1,	0,	1,	0,	0,	1,
    0,	1,	1,	0,	1,	0,	0,	1,	1,	0,	0,	1,	0,	1,	1,	0,
    0,	1,	1,	0,	1,	0,	0,	1,	1,	0,	0,	1,	0,	1,	1,	0,
    1,	0,	0,	1,	0,	1,	1,	0,	0,	1,	1,	0,	1,	0,	0,	1,
};

uint16_t BufferToHexString(char* HexOut, uint16_t MaxChars, const void* Buffer, uint16_t ByteCount)
{
    uint8_t* ByteBuffer = (uint8_t*) Buffer;
    uint16_t CharCount = 0;

    /* Account for '\0' at the end */
    MaxChars--;

    while( (ByteCount > 0) && (MaxChars >= 2) ) {
        uint8_t Byte = *ByteBuffer;

        HexOut[0] = NIBBLE_TO_HEXCHAR( (Byte >> 4) & 0x0F );
        HexOut[1] = NIBBLE_TO_HEXCHAR( (Byte >> 0) & 0x0F );

        HexOut += 2;
        MaxChars -= 2;
        CharCount += 2;
        ByteBuffer++;
        ByteCount -= 1;
    }

    *HexOut = '\0';

    return CharCount;
}

uint16_t HexStringToBuffer(void* Buffer, uint16_t MaxBytes, const char* HexIn)
{
    uint8_t* ByteBuffer = (uint8_t*) Buffer;
    uint16_t ByteCount = 0;

    while( (HexIn[0] != '\0') && (HexIn[1] != '\0') && (MaxBytes > 0) ) {
        if (VALID_HEXCHAR(HexIn[0]) && VALID_HEXCHAR(HexIn[1])) {
            uint8_t Byte = 0;

            Byte |= HEXCHAR_TO_NIBBLE(HexIn[0]) << 4;
            Byte |= HEXCHAR_TO_NIBBLE(HexIn[1]) << 0;

            *ByteBuffer = Byte;

            ByteBuffer++;
            MaxBytes--;
            ByteCount++;
            HexIn += 2;
        } else {
            /* HEX chars only */
            return 0;
        }
    }

    if ( (HexIn[0] != '\0') && (HexIn[1] == '\0') ) {
        /* Odd number of characters */
        return 0;
    }

    return ByteCount;
}

size_t IntegerToStringBuffer(uint16_t IntegerDataValue, char *DestStringBuffer, size_t MaxBufferBytes) { 
     if(DestStringBuffer == NULL) { 
          return 0;
     }
     size_t strBufferBytes = snprintf(DestStringBuffer, MaxBufferBytes, "%x\0", IntegerDataValue);
     return strBufferBytes;
}

size_t ByteToBinaryString(char *BinaryStrDest, size_t MaxChars, uint8_t Byte) {
     if(BinaryStrDest == NULL || MaxChars <= 0) {
          return 0;
     }
     size_t bitsScribed;
     for(bitsScribed = 0; bitsScribed < MIN(BITS_PER_BYTE, MaxChars - 1); bitsScribed++) {
          uint8_t bitMask = (0x01 << bitsScribed);
	  BinaryStrDest[bitsScribed] = (Byte & bitMask) ? '1' : '0';
     }
     return bitsScribed;
}

uint8_t BitReverseByte(uint8_t Byte)
{
    extern const uint8_t PROGMEM BitReverseByteTable[];

    return pgm_read_byte(&BitReverseByteTable[Byte]);
}

uint8_t OddParityBit(uint8_t Byte)
{
    extern const uint8_t PROGMEM OddParityByteTable[];

    return pgm_read_byte(&OddParityByteTable[Byte]);
}

uint8_t StringLength(const char* Str, uint8_t MaxLen)
{
    uint8_t StrLen = 0;

    while(MaxLen > 0) {
        if (*Str++ == '\0')
            break;

        MaxLen--;
        StrLen++;
    }

    return StrLen;
}
