/* CreateEncryptedDumpImage.cpp : 
 * Author: Maxie D. Schmidt
 * Created: 02.01.2019
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include <AESCryptoCPP.cpp>

typedef AESCipher_t * Cipher_t;

#define MAX_BUFFER_SIZE              (2048)
#define MAX_KEYDATA_BUFFER_SIZE      (2048)
static const size_t BITS_PER_BYTE = 8;

#define CRYPTO_UPLOAD_HEADER         ("MFCLASSIC1K-DMP")
#define CRYPTO_UPLOAD_HEADER_SIZE    (15)

#define MIN(x, y)                    ((x) >= (y) ? (y) : (x))


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
     uint8_t saltDataBytes[MAX_BUFFER_SIZE];
     size_t saltDataByteCount;

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
			   saltDataBytes{0}, 
			   saltDataByteCount(0), 
			   keyDataStr(""), 
			   keyDataBytes{0}, 
			   keyDataByteCount(0), 
			   optionParsingError(false)
			   {} 

};

#define MIN(x, y)                    ((x) >= (y) ? (y) : (x))

int GetFileBytes(const char *filePath);
int LoadFileIntoBuffer(const char *filePath, uint8_t *contentsBuffer, int maxBytesToRead);
bool WriteBufferToFile(const char *outputPath, uint8_t *fileBuffer, size_t bufByteCount);

size_t HexStringToBuffer(void *Buffer, size_t MaxBytes, const char *HexIn);
Cipher_t PrepareBlockCipherObject(const uint8_t *keyData, size_t keyLength, 
                                  const uint8_t *initVecData, size_t ivLength, bool setSalt = true);

void PrintUsage(const char *progName) { 
     fprintf(stderr, "Usage: %s [--encrypt|--decrypt]\n", progName);
     fprintf(stderr, "       --input-dump-image=<FilePath>\n");
     fprintf(stderr, "       --timestamp-salt=<TimeStampAsciiString>\n");
     fprintf(stderr, "       [--output-dump-image=<FilePath>]\n");
     fprintf(stderr, "       [--key-data=<HexDataString=32HexChars>]\n\n");
}

UtilityExecData_t ParseCommandLineData(int argc, char** &argv) { 
     static struct option long_options_spec[] = {
          {"encrypt",            no_argument,       NULL, 0}, 
	  {"decrypt",            no_argument,       NULL, 0},
	  {"input-dump-image",   required_argument, NULL, 0}, 
	  {"output-dump-image",  required_argument, NULL, 0}, 
	  {"CTR-mode-size",      required_argument, NULL, 0}, 
	  {"key-data",           required_argument, NULL, 0}, 
	  {"timestamp-salt",     required_argument, NULL, 0}, 
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
	  else if(!strcmp(long_options_spec[option_index].name, "decrypt")) {
	       runtimeDataOptions.dumpFileOperation = OPERATION_DECRYPT;
	  }
	  else if(!strcmp(long_options_spec[option_index].name, "input-dump-image")) { 
               char *inputExtPos = strrchr(optarg, '.');
	       size_t basePathLen = inputExtPos ? inputExtPos - optarg : MAX_BUFFER_SIZE;
	       strncpy(runtimeDataOptions.inputDumpFilePath, optarg, basePathLen);
               runtimeDataOptions.inputDumpFilePath[basePathLen] = '\0';
	       const char *inputFileExt = runtimeDataOptions.dumpFileOperation == OPERATION_ENCRYPT ? 
			                  ".dmp" : ".edmp";
	       strcat(runtimeDataOptions.inputDumpFilePath, inputFileExt);
	       if(!alreadySetOutputPath) { 
                    char *inputFileExtPos = strrchr(optarg, '.');
		    const char *outFileExt = runtimeDataOptions.dumpFileOperation == OPERATION_ENCRYPT ? 
			                     ".edmp" : ".pdmp";
		    if(inputFileExtPos == NULL) { 
		         strncpy(runtimeDataOptions.outputDumpFilePath, 
				 runtimeDataOptions.inputDumpFilePath, MAX_BUFFER_SIZE);
			 strncat(runtimeDataOptions.outputDumpFilePath, outFileExt, MAX_BUFFER_SIZE);
			 continue;
		    }
		    size_t mainFilePathLen = inputFileExtPos - optarg;
		    strncpy(runtimeDataOptions.outputDumpFilePath, optarg, mainFilePathLen);
		    strncat(runtimeDataOptions.outputDumpFilePath, outFileExt, MAX_BUFFER_SIZE);
	       }
	  }
	  else if(!strcmp(long_options_spec[option_index].name, "output-dump-image")) { 
               strncpy(runtimeDataOptions.outputDumpFilePath, optarg, MAX_BUFFER_SIZE);
	       alreadySetOutputPath = true;
	  }
	  else if(!strcmp(long_options_spec[option_index].name, "CTR-mode-size")) { 
	       runtimeDataOptions.ctrModeCounterBytes = atoi(optarg);
	  }
	  else if(!strcmp(long_options_spec[option_index].name, "timestamp-salt")) { 
               size_t saltDataByteCount = MIN(MAX_BUFFER_SIZE, strlen(optarg));
               memcpy(runtimeDataOptions.saltDataBytes, (uint8_t *) optarg, saltDataByteCount);
	       runtimeDataOptions.saltDataByteCount = saltDataByteCount;
	  }
	  else if(!strcmp(long_options_spec[option_index].name, "key-data")) { 
               strncpy(runtimeDataOptions.keyDataStr, optarg, MAX_KEYDATA_BUFFER_SIZE);
	       size_t expectedBufSize = (strlen(optarg) + 1) / 2;
	       size_t keyBufByteCount = HexStringToBuffer(runtimeDataOptions.keyDataBytes, 
			                                  expectedBufSize, optarg);
	       runtimeDataOptions.keyDataByteCount = keyBufByteCount;
	       if(keyBufByteCount != expectedBufSize) { 
	            runtimeDataOptions.optionParsingError = true;
		    break;
	       }
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
     else if(runtimeOptions.keyDataByteCount == 0 || !strlen(runtimeOptions.inputDumpFilePath)) { 
          fprintf(stderr, "ERROR: Insufficient data provided which is needed to process the request ...\n");
	  PrintUsage(argv[0]);
	  return -2;
     }
     
     fprintf(stderr, "sizeof(AESCipher_t) = %d\n", sizeof(AESCipher_t));
     //int dataBufByteCount = GetFileBytes(runtimeOptions.inputDumpFilePath);
     //uint8_t *dataBuf = (uint8_t *) malloc(dataBufByteCount * sizeof(uint8_t));
     //LoadFileIntoBuffer(runtimeOptions.inputDumpFilePath, dataBuf, dataBufByteCount);
     //WriteBufferToFile(runtimeOptions.outputDumpFilePath, dataBuf, dataBufByteCount);
     //return 0;

     if(runtimeOptions.dumpFileOperation == OPERATION_ENCRYPT) { 
          int dataBufByteCount = GetFileBytes(runtimeOptions.inputDumpFilePath);
	  if(!dataBufByteCount) { 
               fprintf(stderr, "ERROR: Reading input dump file \"%s\" ...\n", runtimeOptions.inputDumpFilePath);
	       return 2;
	  }
	  uint8_t *unencDataBuf = (uint8_t *) malloc(dataBufByteCount * sizeof(uint8_t));
	  int actualByteCount = 0;
	  if(dataBufByteCount != (actualByteCount = 
	     LoadFileIntoBuffer(runtimeOptions.inputDumpFilePath, unencDataBuf, dataBufByteCount))) { 
               fprintf(stderr, "ERROR: Reading / parsing input dump file \"%s\" (%d != %d) ...\n", 
		       runtimeOptions.inputDumpFilePath, dataBufByteCount, actualByteCount);
	       free(unencDataBuf);
	       return 3;
	  }
	  Cipher_t cipherObj = PrepareBlockCipherObject(runtimeOptions.keyDataBytes, 
		 	                                runtimeOptions.keyDataByteCount, 
							runtimeOptions.saltDataBytes, 
							runtimeOptions.saltDataByteCount);
	  uint8_t *encDataBuf = (uint8_t *) malloc(dataBufByteCount * sizeof(uint8_t));
 	  memset(encDataBuf, 0xBA, sizeof(encDataBuf));
	  if(cipherObj == NULL || encDataBuf == NULL || unencDataBuf == NULL) {
               fprintf(stderr, "ERROR: NULL pointer(s), %p : %p : %p [%02x]\n", cipherObj, encDataBuf, 
		       unencDataBuf, dataBufByteCount);
	  }
          if(!EncryptDataBuffer(cipherObj, encDataBuf, unencDataBuf, dataBufByteCount)) {
	       fprintf(stderr, "ERROR: Encrypting plaintext to buffer ...\n");
	       free(unencDataBuf);
	       free(encDataBuf);
	       return 4;
	  }
	  fprintf(stdout, "Writing encrypted buffer to file @ \"%s\" ...\n", runtimeOptions.outputDumpFilePath);
	  if(!WriteBufferToFile(runtimeOptions.outputDumpFilePath, encDataBuf, dataBufByteCount)) { 
               fprintf(stderr, "ERROR: Writing encrypted dump buffer back to file ...\n");
	       free(unencDataBuf);
	       free(encDataBuf);
	       return 5;
	  }
	  fprintf(stdout, "DONE!\n");
	  free(unencDataBuf); unencDataBuf = NULL;
	  free(encDataBuf); encDataBuf = NULL;
          ClearCipherObject(cipherObj);
	  DeleteCipherObject(cipherObj);
     }
     else { // decrypt:
          int dataBufByteCount = GetFileBytes(runtimeOptions.inputDumpFilePath);
	  if(!dataBufByteCount) { 
               fprintf(stderr, "ERROR: Reading input dump file \"%s\" ...\n", runtimeOptions.inputDumpFilePath);
	       return 2;
	  }
	  uint8_t *encDataBuf = (uint8_t *) malloc(dataBufByteCount * sizeof(uint8_t));
	  int actualByteCount = 0;
	  if(dataBufByteCount != (actualByteCount = 
	     LoadFileIntoBuffer(runtimeOptions.inputDumpFilePath, encDataBuf, dataBufByteCount))) { 
               fprintf(stderr, "ERROR: Reading / parsing input dump file \"%s\" (%d != %d) ...\n", 
		       runtimeOptions.inputDumpFilePath, dataBufByteCount, actualByteCount);
	       free(encDataBuf);
	       return 3;
	  }
	  Cipher_t cipherObj = PrepareBlockCipherObject(runtimeOptions.keyDataBytes, 
		 	                                runtimeOptions.keyDataByteCount, 
							runtimeOptions.saltDataBytes, 
							runtimeOptions.saltDataByteCount);
	  uint8_t *unencDataBuf = (uint8_t *) malloc(dataBufByteCount * sizeof(uint8_t));
	  memcpy(unencDataBuf, encDataBuf, dataBufByteCount);
	  if(!DecryptDataBuffer(cipherObj, unencDataBuf, encDataBuf, dataBufByteCount) || 
	     memcmp(unencDataBuf, CRYPTO_UPLOAD_HEADER, CRYPTO_UPLOAD_HEADER_SIZE)) {
               uint8_t headerStr[CRYPTO_UPLOAD_HEADER_SIZE + 1];
	       memcpy(headerStr, unencDataBuf, CRYPTO_UPLOAD_HEADER_SIZE);
	       headerStr[CRYPTO_UPLOAD_HEADER_SIZE] = '\0';
	       fprintf(stderr, "ERROR: Consistency in decrypted text, header data NOT correct ...\n%s\n", 
		       headerStr);
	       free(unencDataBuf);
	       free(encDataBuf);
	       return 6;
	  }
          if(!WriteBufferToFile(runtimeOptions.outputDumpFilePath, unencDataBuf, dataBufByteCount)) { 
               fprintf(stderr, "ERROR: Writing plaintext dump buffer back to file ...\n");
	       free(unencDataBuf);
	       free(encDataBuf);
	       return 5;
	  }
	  fprintf(stdout, "DONE!\n");
	  //free(unencDataBuf); unencDataBuf = NULL;
	  //free(encDataBuf); encDataBuf = NULL;
          ClearCipherObject(cipherObj);
	  DeleteCipherObject(cipherObj);
     }

     return 0;

}

int GetFileBytes(const char *filePath) { 
     struct stat statStructBuf;
     int statSuccessCode = stat(filePath, &statStructBuf);
     if(statSuccessCode == 0) { 
          return statStructBuf.st_size;
     }
     return 0;
}

int LoadFileIntoBuffer(const char *filePath, uint8_t *contentsBuffer, int maxBytesToRead) { 
     int fpInputBuffer = open(filePath, O_RDONLY | O_NONBLOCK, 0);
     if(fpInputBuffer == -1) { 
          perror("LoadFileIntoBuffer: Opening file ");
	  return 0;
     }
     int bytesRead = 0, rbytes;
     uint8_t *curContentsBufPtr = contentsBuffer;
     sleep(1);
     while(bytesRead < maxBytesToRead) { 
          if((rbytes = read(fpInputBuffer, (uint8_t *) curContentsBufPtr, maxBytesToRead)) != maxBytesToRead && 
	     errno != 0) { 
	       perror("LoadFileIntoBuffer: Reading into buffer incorrect byte count ");
	  }
	  curContentsBufPtr += rbytes;
	  bytesRead += rbytes;
	  if(errno == 0 && bytesRead < maxBytesToRead) { // we encountered a non-terminal EOF character:
	       *curContentsBufPtr = EOF;
	       if(rbytes == 0) { 
	            bytesRead++;
	       }
	  }
	  //fprintf(stderr, "Total bytes read = % 9d\n", bytesRead);
     }
     close(fpInputBuffer);
     return bytesRead;
} 

bool WriteBufferToFile(const char *outputPath, uint8_t *fileBuffer, size_t bufByteCount) {
     FILE *fpOutputBuffer = fopen(outputPath, "wb");
     if(!fpOutputBuffer) { 
          perror("WriteBufferToFile: Opening output file for buffer writing ");
	  return false;
     
     }
     int numBytesWritten = 0, wbytes;
     uint8_t *curFileBufPtr = fileBuffer;
     while(numBytesWritten < bufByteCount) { 
          if((wbytes = write(fileno(fpOutputBuffer), curFileBufPtr, bufByteCount)) == -1) { 
               perror("WriteBufferToFile: Writing buffer bytes out to file");
	  }
	  else if(wbytes != -1) {
	       curFileBufPtr += bufByteCount;
	       numBytesWritten += bufByteCount;
	       break;
	  }
     }
     fclose(fpOutputBuffer);
     return numBytesWritten == bufByteCount;
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

Cipher_t PrepareBlockCipherObject(const uint8_t *keyData, size_t keyLength,  
		                  const uint8_t *initVecData, size_t ivLength, 
				  bool setSalt) { 
     Cipher_t cipherObj = CreateNewCipherObject();
     if(cipherObj == NULL || keyData == NULL || !keyLength || initVecData == NULL || !ivLength) {
	  DeleteCipherObject(cipherObj);
          return NULL;
     }
     if(SetCipherKey(cipherObj, keyData, keyLength) && setSalt && 
        SetCipherSalt(cipherObj, initVecData, ivLength)) { 
          return cipherObj;
     }
     DeleteCipherObject(cipherObj);
     return NULL;
}

