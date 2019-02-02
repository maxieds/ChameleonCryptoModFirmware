/* CreateEncryptedDumpImage.cpp : 
 * Author: Maxie D. Schmidt
 * Created: 02.01.2019
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include <AES.h>
#include <CTR.h>
#include <CFB.h>
#include <OFB.h>
#include <CBC.h>

#define MAX_BUFFER_SIZE              (512)
#define MAX_KEYDATA_BUFFER_SIZE      (2048)
static const size_t BITS_PER_BYTE = 8;

#if defined(BLOCK_CIPHER_TYPE_AES128)
     typedef AES128 BlockCipher_t;
     #define BLOCK_CIPHER_KEY_BYTE_SIZE (128 / BITS_PER_BYTE)
#elif defined(BLOCK_CIPHER_TYPE_AES192)
     typedef AES192 BlockCipher_t;
     #define BLOCK_CIPHER_KEY_BYTE_SIZE (192 / BITS_PER_BYTE)
#else
     typedef AES256 BlockCipher_t;
     #define BLOCK_CIPHER_KEY_BYTE_SIZE (256 / BITS_PER_BYTE)
#endif

#if defined(CIPHER_MODE_TYPE_CFB)
     typedef CFB< BlockCipher_t > Cipher_t;
     template class CFB< BlockCipher_t >;
#elif defined(CIPHER_MODE_TYPE_OFB)
     typedef OFB< BlockCipher_t > Cipher_t;
     template class OFB< BlockCipher_t >;
#elif defined(CIPHER_MODE_TYPE_CBC)
     typedef CBC< BlockCipher_t > Cipher_t;
     template class CBC< BlockCipher_t >;
#else
     typedef CTR< BlockCipher_t > Cipher_t;
     template class CTR< BlockCipher_t >;
#endif

typedef enum {
     OPERATION_ENCRYPT = 4, 
     OPERATION_DECRYPT,
     BLKCIPHER_AES128 = 256, 
     BLKCIPHER_AES192, 
     BLKCIPHER_AES256,
     BLKCIPHER_MODE_CTR = 1024, 
     BLKCIPHER_MODE_CFB, 
     BLKCIPHER_MODE_OFB, 
     BLKCIPHER_MODE_CBC, 
     KEYDATA_USE_HEXSTRING = 2048, 
     KEYDATA_FROM_PASSPHRASE,
} UtilityModesEnum_t;

typedef UtilityModesEnum_t Operation_t;
typedef UtilityModesEnum_t BlockCipherType_t;
typedef UtilityModesEnum_t BlockCipherMode_t;

class UtilityExecData_t { 

   public:	
     /* Utility execution related specs: */
     Operation_t dumpFileOperation;
     char inputDumpFilePath[MAX_BUFFER_SIZE];
     char outputDumpFilePath[MAX_BUFFER_SIZE];

     /* Encryption / Decryption types of dump files requested: */
     BlockCipherType_t blockCipherType;
     BlockCipherMode_t blockCipherMode;
     size_t ctrModeCounterBytes;
     size_t blockCipherKeyBits;
     size_t blockCipherKeyBytes;

     /* Key data storage for encryption / decryption of the input dump images: */
     char keyDataStr[MAX_KEYDATA_BUFFER_SIZE];
     uint8_t keyDataBytes[MAX_KEYDATA_BUFFER_SIZE];
     size_t keyDataByteCount;
     uint8_t initVecData[MAX_KEYDATA_BUFFER_SIZE];
     size_t ivLength;

     /* Executive tracking of the program: */
     bool optionParsingError;

     /* Default constructor with some (at least) sane initial settings: */
     inline UtilityExecData_t() : 
	                   dumpFileOperation(OPERATION_ENCRYPT), 
	                   inputDumpFilePath(""), 
			   outputDumpFilePath(""), 
			   blockCipherType(BLKCIPHER_AES256), 
			   blockCipherMode(BLKCIPHER_MODE_CTR), 
                           ctrModeCounterBytes(4), 
			   blockCipherKeyBits(256), 
			   blockCipherKeyBytes(256 / BITS_PER_BYTE), 
			   keyDataStr(""), 
			   keyDataBytes{0}, 
			   keyDataByteCount(0), 
			   initVecData{0}, 
			   ivLength(0), 
			   optionParsingError(false)
			   {} 

};

#define MIN(x, y)                    ((x) >= (y) ? (y) : (x))

size_t GetFileBytes(const char *filePath);
size_t LoadFileIntoBuffer(const char *filePath, uint8_t *contentsBuffer, size_t maxBytesToRead);
bool WriteBufferToFile(const char *outputPath, uint8_t *fileBuffer, size_t bufByteCount);

size_t HexStringToBuffer(void *Buffer, size_t MaxBytes, const char *HexIn);
Cipher_t * CreateBlockCipherObject(const uint8_t *keyData, size_t keyLength, 
	                           const uint8_t *initVecData, size_t ivLength);

void PrintUsage(const char *progName) { 
     fprintf(stderr, "Usage: %s [--encrypt] --input-dump-image=<FilePath> ", progName);
     fprintf(stderr, "[--output-dump-image=<FilePath>]\n");
     fprintf(stderr, "       [--CTR-mode-size=<Bytes>] [--key-data=<HexDataString>]\n");
     fprintf(stderr, "       [--key-data-file=<FilePath>]\n");
}

UtilityExecData_t ParseCommandLineData(int argc, char** &argv) { 
     static struct option long_options_spec[] = {
          {"encrypt",            no_argument,       NULL, 0}, 
	  {"input-dump-image",   required_argument, NULL, 0}, 
	  {"output-dump-image",  required_argument, NULL, 0}, 
	  {"CTR-mode-size",      required_argument, NULL, 0}, 
	  {"key-data",           required_argument, NULL, 0}, 
	  {"key-data-file",      required_argument, NULL, 0}, 
	  {"timestamp-salt",     required_argument, NULL, 0}, 
	  {"generate-timestamp", required_argument, NULL, 0}, 
	  {NULL,                 0,                 NULL, 0}
     };
     int option_index = 0, optch;
     bool alreadySetOutputPath = false;
     UtilityExecData_t runtimeDataOptions;
     while(true) { 
          optch = getopt_long(argc, argv, "", long_options_spec, &option_index);
	  if(optch == -1) { 
               break;
	  }
	  else if(optch != 0) { 
	       runtimeDataOptions.optionParsingError = true;
	       break;
	  }
          if(!strcmp(long_options_spec[option_index].name, "encrypt")) { 
	       runtimeDataOptions.dumpFileOperation = OPERATION_ENCRYPT;
	  }
	  else if(!strcmp(long_options_spec[option_index].name, "input-dump-image")) { 
               strncpy(runtimeDataOptions.inputDumpFilePath, optarg, MAX_BUFFER_SIZE);
	       if(!alreadySetOutputPath) { 
                    char *inputFileExt = strrchr(optarg, '.');
		    if(inputFileExt == NULL) { 
		         strncpy(runtimeDataOptions.outputDumpFilePath, 
				 runtimeDataOptions.inputDumpFilePath, MAX_BUFFER_SIZE);
			 strncat(runtimeDataOptions.outputDumpFilePath, ".edmp", MAX_BUFFER_SIZE);
			 continue;
		    }
		    size_t mainFilePathLen = inputFileExt - optarg;
		    strncpy(runtimeDataOptions.outputDumpFilePath, optarg, mainFilePathLen);
		    strncat(runtimeDataOptions.outputDumpFilePath, ".edmp", MAX_BUFFER_SIZE);
	       }
	  }
	  else if(!strcmp(long_options_spec[option_index].name, "output-dump-image")) { 
               strncpy(runtimeDataOptions.outputDumpFilePath, optarg, MAX_BUFFER_SIZE);
	       alreadySetOutputPath = true;
	  }
	  else if(!strcmp(long_options_spec[option_index].name, "CTR-mode-size")) { 
	       runtimeDataOptions.ctrModeCounterBytes = atoi(optarg);
	  }
	  else if(!strcmp(long_options_spec[option_index].name, "key-data")) { 
               strncpy(runtimeDataOptions.keyDataStr, optarg, MAX_KEYDATA_BUFFER_SIZE);
	       size_t expectedBufSize = (strlen(optarg) + 1) / 2;
	       size_t keyBufByteCount = HexStringToBuffer(runtimeDataOptions.keyDataBytes, 
			                                  MAX_KEYDATA_BUFFER_SIZE, optarg);
	       runtimeDataOptions.keyDataByteCount = keyBufByteCount;
	       if(keyBufByteCount != expectedBufSize) { 
	            runtimeDataOptions.optionParsingError = true;
		    break;
	       }
	  }
	  else if(!strcmp(long_options_spec[option_index].name, "key-data-file")) { 
               size_t keyDataBytes = GetFileBytes(runtimeDataOptions.inputDumpFilePath);
	       uint8_t *keyDataBuf = (uint8_t *) malloc(keyDataBytes * sizeof(uint8_t));
               if(!keyDataBytes || keyDataBytes != 
		  LoadFileIntoBuffer(runtimeDataOptions.inputDumpFilePath, keyDataBuf, keyDataBytes)) { 
	            runtimeDataOptions.optionParsingError = true;
                    break;
	       }
	       memcpy(runtimeDataOptions.keyDataBytes, keyDataBuf, MIN(keyDataBytes, MAX_KEYDATA_BUFFER_SIZE));
	       runtimeDataOptions.keyDataByteCount = MIN(keyDataBytes, MAX_KEYDATA_BUFFER_SIZE);
	  }
	  else if(!strcmp(long_options_spec[option_index].name, "timestamp-salt")) {
               size_t expectedBufSize = (strlen(optarg) + 1) / 2;
               size_t actualBufSize = HexStringToBuffer(runtimeDataOptions.initVecData, 
			                                MAX_KEYDATA_BUFFER_SIZE, optarg);
	       runtimeDataOptions.ivLength = actualBufSize;
	       if(expectedBufSize != actualBufSize) { 
	            runtimeDataOptions.optionParsingError = true;
		    break;
	       }
	  }
	  else if(!strcmp(long_options_spec[option_index].name, "generate-timestamp")) { 
                time_t timeSinceEpoch = time(NULL);
		fprintf(stdout, "TIMESTAMP (TIME SINCE EPOCH): %lx\n", timeSinceEpoch);
		char timestampStr[MAX_BUFFER_SIZE];
		snprintf(timestampStr, MAX_BUFFER_SIZE, "%lx\0", timeSinceEpoch);
                size_t saltBufSize = HexStringToBuffer(runtimeDataOptions.initVecData, 
				                       MAX_KEYDATA_BUFFER_SIZE, timestampStr); 
		runtimeDataOptions.ivLength = saltBufSize;
	  }
	  else { 
	       break;
	  }
     }
     return runtimeDataOptions;
}

int main(int argc, char **argv) { 

     UtilityExecData_t runtimeOptions = ParseCommandLineData(argc, argv);
     if(runtimeOptions.optionParsingError) { 
          PrintUsage(argv[0]);
	  return -1;
     } 
     else if(runtimeOptions.keyDataByteCount == 0 || runtimeOptions.ivLength == 0 || 
	     !strlen(runtimeOptions.inputDumpFilePath)) { 
          fprintf(stderr, "ERROR: Insufficient data provided which is needed to process the request ...\n");
	  PrintUsage(argv[0]);
	  return -2;
     }
     
     if(runtimeOptions.dumpFileOperation == OPERATION_ENCRYPT) { 
          size_t dataBufByteCount = GetFileBytes(runtimeOptions.inputDumpFilePath);
	  if(!dataBufByteCount) { 
               fprintf(stderr, "ERROR: Reading input dump file \"%s\" ...\n", runtimeOptions.inputDumpFilePath);
	       return 2;
	  }
	  uint8_t *unencDataBuf = (uint8_t *) malloc(dataBufByteCount * sizeof(uint8_t));
	  if(dataBufByteCount != 
	     LoadFileIntoBuffer(runtimeOptions.inputDumpFilePath, unencDataBuf, dataBufByteCount)) { 
               fprintf(stderr, "ERROR: Reading / parsing input dump file \"%s\" ...\n", 
		       runtimeOptions.inputDumpFilePath);
	       free(unencDataBuf);
	       return 3;
	  }
	  Cipher_t *cipherObj = CreateBlockCipherObject(runtimeOptions.keyDataBytes, 
		 	                                runtimeOptions.keyDataByteCount, 
						        runtimeOptions.initVecData, 
						        runtimeOptions.ivLength);
	  uint8_t *encDataBuf = (uint8_t *) malloc(dataBufByteCount * sizeof(uint8_t));
	  cipherObj->encrypt(encDataBuf, unencDataBuf, dataBufByteCount);
          if(!WriteBufferToFile(runtimeOptions.outputDumpFilePath, encDataBuf, dataBufByteCount)) { 
               fprintf(stderr, "ERROR: Writing encrypted dump buffer back to file ...\n");
	       free(unencDataBuf);
	       free(encDataBuf);
	       return 4;
	  }
	  free(unencDataBuf); unencDataBuf = NULL;
	  free(encDataBuf); encDataBuf = NULL;
	  delete cipherObj; cipherObj = NULL;
     }

     return 0;

}

size_t GetFileBytes(const char *filePath) { 
     struct stat statStructBuf;
     int statSuccessCode = stat(filePath, &statStructBuf);
     if(statSuccessCode == 0) { 
          return statStructBuf.st_size;
     }
     return 0;
}

size_t LoadFileIntoBuffer(const char *filePath, uint8_t *contentsBuffer, size_t maxBytesToRead) { 
     FILE *fpInputBuffer = fopen(filePath, "r+");
     if(!fpInputBuffer) { 
          perror("LoadFileIntoBuffer: Opening file ");
	  return 0;
     }
     size_t bytesRead = 0;
     uint8_t *curContentsBufPtr = contentsBuffer;
     while((bytesRead <= maxBytesToRead) && !feof(fpInputBuffer)) { 
          if(fread(curContentsBufPtr, sizeof(uint8_t), 1, fpInputBuffer) != 1) { 
	       perror("LoadFileIntoBuffer: Reading into buffer ");
	       return 0;
	  }
	  curContentsBufPtr++;
	  bytesRead++;
     }
     return bytesRead;
} 

bool WriteBufferToFile(const char *outputPath, uint8_t *fileBuffer, size_t bufByteCount) {
     FILE *fpOutputBuffer = fopen(outputPath, "w+");
     if(!fpOutputBuffer) { 
          perror("WriteBufferToFile: Opening output file for buffer writing ");
	  return false;
     
     }
     size_t numBytesWritten = 0;
     uint8_t *curFileBufPtr = fileBuffer;
     while(numBytesWritten < bufByteCount) { 
          if(fwrite(curFileBufPtr, sizeof(uint8_t), 1, fpOutputBuffer) != 1) { 
               perror("WriteBufferToFile: Writing buffer bytes out to file");
	       return false;
	  }
	  curFileBufPtr++;
	  numBytesWritten++;
     }
     fclose(fpOutputBuffer);
     return true;
}

#define NIBBLE_TO_HEXCHAR(x) ( (x) < 0x0A ? (x) + '0' : (x) + 'A' - 0x0A )
#define HEXCHAR_TO_NIBBLE(x) ( (x) < 'A'  ? (x) - '0' : (x) - 'A' + 0x0A )
#define VALID_HEXCHAR(x) (  ( (x) >= '0' && (x) <= '9' ) || ( (x) >= 'A' && (x) <= 'F' ) )

size_t HexStringToBuffer(void* Buffer, size_t MaxBytes, const char* HexIn) {
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

Cipher_t * CreateBlockCipherObject(const uint8_t *keyData, size_t keyLength, 
	                           const uint8_t *initVecData, size_t ivLength) { 
     Cipher_t *cipherObj = new Cipher_t();
     cipherObj->setKey(keyData, keyLength);
     cipherObj->setIV(initVecData, ivLength);
     #ifdef CIPHER_MODE_TYPE_CTR
     if(DEFAULT_COUNTER_SIZE > 0) {
          cipherObj->setCounterSize(DEFAULT_COUNTER_SIZE);
     }
     #endif
     return cipherObj;
}
