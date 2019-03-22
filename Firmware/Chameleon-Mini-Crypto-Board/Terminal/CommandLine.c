/*
 * CommandLine.c
 *
 *  Created on: 04.05.2013
 *      Author: skuser
 */

#include <ctype.h>

#include "CommandLine.h"
#include "Settings.h"
#include "System.h"

#define CHAR_GET_MODE   		'?'     /* <Command>? */
#define CHAR_SET_MODE   		'='     /* <Command>=<Param> */
#define CHAR_EXEC_MODE  		'\0'    /* <Command> */
#define CHAR_EXEC_MODE_PARAM 	        ' '	/* <Command> <Param> ... <ParamN> */

#define ESCAPE_CHAR                     0x1B

#define IS_COMMAND_DELIMITER(c) ( \
  ((c) == CHAR_EXEC_MODE) || ((c) == CHAR_GET_MODE) || ((c) == CHAR_SET_MODE) || ((c) == CHAR_EXEC_MODE_PARAM) \
)

#define IS_CHARACTER(c) ( \
  ( ((c) >= 'A') && ((c) <= 'Z') ) || \
  ( ((c) >= 'a') && ((c) <= 'z') ) || \
  ( ((c) >= '0') && ((c) <= '9') ) || \
  ( ((c) == '_') ) || isgraph((c)) || \
  ( ((c) == CHAR_GET_MODE) || ((c) == CHAR_SET_MODE) || ((c) == CHAR_EXEC_MODE_PARAM) ) \
)

#define IS_LOWERCASE(c) ( ((c) >= 'a') && ((c) <= 'z') )
#define TO_UPPERCASE(c) ( (c) - 'a' + 'A' )

#define IS_WHITESPACE(c) ( ((c) == ' ') || ((c) == '\t') || isspace((c)) )

#define NO_FUNCTION          ((void *) NULL)
#define NO_EXEC_FUNCTION     ((void *) NULL)
#define NO_PARAM_FUNCTION    ((void *) NULL)
#define NO_SET_FUNCTION      ((void *) NULL)
#define NO_GET_FUNCTION      ((void *) NULL)

/* Include all command functions */
#include "Commands.h"

const PROGMEM CommandEntryType CommandTable[] = {
  {
    COMMAND_VERSION,
    NO_EXEC_FUNCTION,
    NO_PARAM_FUNCTION,
    NO_SET_FUNCTION,
    CommandGetVersion,
  },
  {
    COMMAND_CONFIG,
    NO_EXEC_FUNCTION,
    NO_PARAM_FUNCTION,
    CommandSetConfig,
    CommandGetConfig
  },
  {
    COMMAND_UID,
    NO_EXEC_FUNCTION,
    NO_PARAM_FUNCTION,
    CommandSetUid,
    CommandGetUid
  },
  {
    COMMAND_READONLY,
    NO_EXEC_FUNCTION,
    NO_PARAM_FUNCTION,
    CommandSetReadOnly,
    CommandGetReadOnly

  },
  {
    COMMAND_UPLOAD,
    CommandExecUpload,
    NO_PARAM_FUNCTION,
    NO_SET_FUNCTION,
    NO_GET_FUNCTION
  },
  {
    COMMAND_UPLOAD_ENCRYPTED,
    NO_EXEC_FUNCTION,
    CommandExecParamUploadEncrypted,
    NO_SET_FUNCTION,
    NO_GET_FUNCTION
  },
  {
    COMMAND_UPLOAD_STATUS,
    CommandExecUploadStatus,
    NO_PARAM_FUNCTION,
    NO_SET_FUNCTION,
    NO_GET_FUNCTION
  },
  {
    COMMAND_DOWNLOAD,
    CommandExecDownload,
    NO_PARAM_FUNCTION,
    NO_SET_FUNCTION,
    NO_GET_FUNCTION
  },
  {
    COMMAND_RESET,
    CommandExecReset,
    NO_PARAM_FUNCTION,
    NO_SET_FUNCTION,
    NO_GET_FUNCTION
  },
#ifdef SUPPORT_FIRMWARE_UPGRADE
  {
    COMMAND_UPGRADE,
    CommandExecUpgrade,
    NO_PARAM_FUNCTION,
    NO_SET_FUNCTION,
    NO_GET_FUNCTION
  },
#endif
  {
    COMMAND_MEMSIZE,
    NO_EXEC_FUNCTION,
    NO_PARAM_FUNCTION,
    NO_SET_FUNCTION,
    CommandGetMemSize
  },
  {
    COMMAND_UIDSIZE,
    NO_EXEC_FUNCTION,
    NO_PARAM_FUNCTION,
    NO_SET_FUNCTION,
    CommandGetUidSize
  },
  {
    COMMAND_RBUTTON,
    NO_EXEC_FUNCTION,
    NO_PARAM_FUNCTION,
    CommandSetRButton,
    CommandGetRButton
  },
  {
    COMMAND_RBUTTON_LONG,
    NO_EXEC_FUNCTION,
    NO_PARAM_FUNCTION,
    CommandSetRButtonLong,
    CommandGetRButtonLong
  },
  {
    COMMAND_LBUTTON,
    NO_EXEC_FUNCTION,
    NO_PARAM_FUNCTION,
    CommandSetLButton,
    CommandGetLButton
  },
  {
    COMMAND_LBUTTON_LONG,
    NO_EXEC_FUNCTION,
    NO_PARAM_FUNCTION,
    CommandSetLButtonLong,
    CommandGetLButtonLong
  },
  {
    COMMAND_LEDGREEN,
    NO_EXEC_FUNCTION,
    NO_PARAM_FUNCTION,
    CommandSetLedGreen,
    CommandGetLedGreen
  },
  {
    COMMAND_LEDRED,
    NO_EXEC_FUNCTION,
    NO_PARAM_FUNCTION,
    CommandSetLedRed,
    CommandGetLedRed
  },
  {
    COMMAND_LOGMODE,
    NO_EXEC_FUNCTION,
    NO_PARAM_FUNCTION,
    CommandSetLogMode,
    CommandGetLogMode
  },
  {
    COMMAND_LOGMEM,
    NO_EXEC_FUNCTION,
    NO_PARAM_FUNCTION,
    NO_SET_FUNCTION,
    CommandGetLogMem
  },
  {
    COMMAND_LOGDOWNLOAD,
    CommandExecLogDownload,
    NO_PARAM_FUNCTION,
    NO_SET_FUNCTION,
    NO_GET_FUNCTION
  },
  {
    COMMAND_STORELOG,
    CommandExecStoreLog,
    NO_PARAM_FUNCTION,
    NO_SET_FUNCTION,
    NO_GET_FUNCTION
  },
  {
    COMMAND_LOGCLEAR,
    CommandExecLogClear,
    NO_PARAM_FUNCTION,
    NO_SET_FUNCTION,
    NO_GET_FUNCTION
  },
  {
    COMMAND_SETTING,
    NO_EXEC_FUNCTION,
    NO_PARAM_FUNCTION,
    CommandSetSetting,
    CommandGetSetting
  },
  {
    COMMAND_CLEAR,
    CommandExecClear,
    NO_PARAM_FUNCTION,
    NO_SET_FUNCTION,
    NO_GET_FUNCTION
  },
  {
    COMMAND_STORE,
    CommandExecStore,
    NO_PARAM_FUNCTION,
    NO_SET_FUNCTION,
    NO_GET_FUNCTION
  },
  {
    COMMAND_RECALL,
    CommandExecRecall,
    NO_PARAM_FUNCTION,
    NO_SET_FUNCTION,
    NO_GET_FUNCTION
  },
  {
    COMMAND_CHARGING,
    NO_EXEC_FUNCTION,
    NO_PARAM_FUNCTION,
    NO_SET_FUNCTION,
    CommandGetCharging
  },
  {
    COMMAND_HELP,
    CommandExecHelp,
    NO_PARAM_FUNCTION,
    NO_SET_FUNCTION,
    NO_GET_FUNCTION
  },
  {
    COMMAND_RSSI,
    NO_EXEC_FUNCTION,
    NO_PARAM_FUNCTION,
    NO_SET_FUNCTION,
    CommandGetRssi
  },
  {
    COMMAND_SYSTICK,
    NO_EXEC_FUNCTION,
    NO_PARAM_FUNCTION,
    NO_SET_FUNCTION,
    CommandGetSysTick
  },
  {
    COMMAND_SEND_RAW,
    NO_EXEC_FUNCTION,
    CommandExecParamSendRaw,
    NO_SET_FUNCTION,
    NO_GET_FUNCTION
  },
  {
    COMMAND_SEND,
    NO_EXEC_FUNCTION,
    CommandExecParamSend,
    NO_SET_FUNCTION,
    NO_GET_FUNCTION
  },
  {
    COMMAND_GETUID,
    CommandExecGetUid,
    NO_PARAM_FUNCTION,
    NO_SET_FUNCTION,
    NO_GET_FUNCTION
  },
  {
    COMMAND_DUMP_MFU,
    CommandExecDumpMFU,
    NO_PARAM_FUNCTION,
    NO_SET_FUNCTION,
    NO_GET_FUNCTION
  },
  {
    COMMAND_IDENTIFY_CARD,
    CommandExecIdentifyCard,
    NO_PARAM_FUNCTION,
    NO_SET_FUNCTION,
    NO_GET_FUNCTION
  },
  {
    COMMAND_TIMEOUT,
    NO_EXEC_FUNCTION,
    NO_PARAM_FUNCTION,
    CommandSetTimeout,
    CommandGetTimeout
  },
  {
    COMMAND_THRESHOLD,
    NO_EXEC_FUNCTION,
    NO_PARAM_FUNCTION,
    CommandSetThreshold,
    CommandGetThreshold
  },
  {
    COMMAND_AUTOCALIBRATE,
    CommandExecAutocalibrate,
    NO_PARAM_FUNCTION,
    NO_SET_FUNCTION,
    NO_GET_FUNCTION
  },
  {
    COMMAND_FIELD,
    NO_EXEC_FUNCTION,
    NO_PARAM_FUNCTION,
    CommandSetField,
    CommandGetField
  },
  {
    COMMAND_CLONE,
    CommandExecClone,
    NO_PARAM_FUNCTION,
    NO_SET_FUNCTION,
    NO_GET_FUNCTION
  },
  {
    COMMAND_DEVICEAUTH,
    NO_EXEC_FUNCTION,
    CommandExecParamDeviceAuth, 
    NO_SET_FUNCTION, 
    NO_GET_FUNCTION
  },
  {
    COMMAND_SETKEY,
    NO_EXEC_FUNCTION,
    CommandExecParamSetKey,
    NO_SET_FUNCTION,
    NO_GET_FUNCTION
  },
  {
    COMMAND_GENKEY,
    NO_EXEC_FUNCTION,
    CommandExecParamGenKey,
    NO_SET_FUNCTION,
    NO_GET_FUNCTION
  },
#ifdef ENABLE_ADMIN_LEVEL_DEBUGGING
  {
    COMMAND_GETKEY, 
    NO_EXEC_FUNCTION, 
    CommandExecParamGetKey, 
    NO_SET_FUNCTION, 
    NO_GET_FUNCTION
  },
#endif 
  { /* This has to be last element */
    COMMAND_LIST_END,
    NO_EXEC_FUNCTION,
    NO_PARAM_FUNCTION,
    NO_SET_FUNCTION,
    NO_GET_FUNCTION
  }
};

#define STATUS_TABLE_ENTRY(Id, Text) \
  { Id, STRINGIFY(Id) ":" Text }

typedef struct {
    CommandStatusIdType Id;
    CommandStatusMessageType Message;
} CommandStatusType;

static const CommandStatusType PROGMEM StatusTable[] = {
  STATUS_TABLE_ENTRY(COMMAND_INFO_OK_ID, COMMAND_INFO_OK),
  STATUS_TABLE_ENTRY(COMMAND_INFO_OK_WITH_TEXT_ID, COMMAND_INFO_OK_WITH_TEXT),
  STATUS_TABLE_ENTRY(COMMAND_INFO_XMODEM_WAIT_ID, COMMAND_INFO_XMODEM_WAIT),
  STATUS_TABLE_ENTRY(COMMAND_ERR_UNKNOWN_CMD_ID, COMMAND_ERR_UNKNOWN_CMD),
  STATUS_TABLE_ENTRY(COMMAND_ERR_INVALID_USAGE_ID, COMMAND_ERR_INVALID_USAGE),
  STATUS_TABLE_ENTRY(COMMAND_ERR_INVALID_PARAM_ID, COMMAND_ERR_INVALID_PARAM),
  STATUS_TABLE_ENTRY(COMMAND_INFO_FALSE_ID, COMMAND_INFO_FALSE),
  STATUS_TABLE_ENTRY(COMMAND_INFO_TRUE_ID, COMMAND_INFO_TRUE),
  STATUS_TABLE_ENTRY(COMMAND_ERR_TIMEOUT_ID, COMMAND_ERR_TIMEOUT),
  STATUS_TABLE_ENTRY(COMMAND_ERR_AUTH_FAILED_ID, COMMAND_ERR_AUTH_FAILED),
  STATUS_TABLE_ENTRY(UPLOAD_STATUS_OK_ID, UPLOAD_STATUS_OK),
  STATUS_TABLE_ENTRY(UPLOAD_STATUS_PENDING_ID, UPLOAD_STATUS_PENDING),
  STATUS_TABLE_ENTRY(UPLOAD_STATUS_ERROR_ID, UPLOAD_STATUS_ERROR),  
};

static uint16_t BufferIdx;

void (*CommandLinePendingTaskTimeout) (void) = NO_FUNCTION; // gets called on Timeout
static bool TaskPending = false;
static uint16_t TaskPendingSince;

const char* GetStatusMessageP(CommandStatusIdType StatusId)
{
    uint8_t i;

    for (i = 0; i < ARRAY_COUNT(StatusTable); i++) {
        if (pgm_read_byte(&StatusTable[i].Id) == StatusId)
            return StatusTable[i].Message;
    }
    return NULL;
}

static CommandStatusIdType CallCommandFunc(
    const CommandEntryType* CommandEntry, char CommandDelimiter, char* pParam) {
  char *pTerminalBuffer = (char *) TerminalBufferOut;
  CommandStatusIdType Status = COMMAND_ERR_INVALID_USAGE_ID;

    /* Call appropriate function depending on CommandDelimiter */
    if (CommandDelimiter == CHAR_GET_MODE) {
        CommandGetFuncType GetFunc = (CommandGetFuncType) pgm_read_ptr(&CommandEntry->GetFunc);
        if (GetFunc != NO_FUNCTION) {
            Status = GetFunc(pTerminalBuffer);
        }
    } else if (CommandDelimiter == CHAR_SET_MODE) {
        CommandSetFuncType SetFunc = (CommandSetFuncType) pgm_read_ptr(&CommandEntry->SetFunc);
        if (SetFunc != NO_FUNCTION) {
            Status = SetFunc(pTerminalBuffer, pParam);
        }
    } else if (CommandDelimiter == CHAR_EXEC_MODE) {
        CommandExecFuncType ExecFunc = (CommandExecFuncType) pgm_read_ptr(&CommandEntry->ExecFunc);
        if (ExecFunc != NO_FUNCTION) {
            Status = ExecFunc(pTerminalBuffer);
        }
    } else if (CommandDelimiter == CHAR_EXEC_MODE_PARAM) {
        CommandExecParamFuncType ExecParamFunc = (CommandExecParamFuncType) pgm_read_ptr(&CommandEntry->ExecParamFunc);
        if (ExecParamFunc != NO_FUNCTION) {
            Status = ExecParamFunc(pTerminalBuffer, pParam);
        }
    } else {
        /* This should not happen (TM) */
    }

    if (Status == TIMEOUT_COMMAND)
    {
        TaskPending = true;
        TaskPendingSince = SystemGetSysTick();
    }

    /* This delimiter has not been registered with this command */
    return Status;
}

void CommandExecute(const char* command)
{
    uint8_t i;

    for (i = 0; i < ARRAY_COUNT(CommandTable); i++) 
    {
        if (strcmp_P(command, CommandTable[i].Command) == 0)
        {
            CallCommandFunc(&CommandTable[i], CHAR_EXEC_MODE, NULL);
            break;
        }
    }
}

static void DecodeCommand(void)
{
  uint8_t i;
  bool CommandFound = false;
  CommandStatusIdType StatusId = COMMAND_ERR_UNKNOWN_CMD_ID;
  char* pTerminalBuffer = (char*) TerminalBufferOut;

  /* Do some sanity check first */
  if (!IS_COMMAND_DELIMITER(pTerminalBuffer[0])) {
    char* pCommandDelimiter = pTerminalBuffer;
    char CommandDelimiter = '\0';

    /* Search for command delimiter, store it and replace with '\0' */
    while(!(IS_COMMAND_DELIMITER(*pCommandDelimiter)))
      pCommandDelimiter++;

    CommandDelimiter = *pCommandDelimiter;
    *pCommandDelimiter = '\0';

    /* Search in command table */
    for (i = 0; i < ARRAY_COUNT(CommandTable); i++) {
      char *firstCmdWord = strchrnul(pTerminalBuffer, COMMAND_ARGSEP);
      size_t cmdBufferCheckLen = firstCmdWord - pTerminalBuffer;
      if(cmdBufferCheckLen && !strncasecmp_P(pTerminalBuffer, CommandTable[i].Command, cmdBufferCheckLen)) {
        /* Command found. Clear buffer, and call appropriate function */
        char* pParam = ++pCommandDelimiter;

        TerminalBufferOut[0] = '\0';
	CommandFound = true;

        StatusId = CallCommandFunc(&CommandTable[i], CommandDelimiter, pParam);

        break;
      }
    }
  }

  if (StatusId == TIMEOUT_COMMAND) // it is a timeout command, so we return
      return;

  /* Send command status message */
  TerminalSendStringP(GetStatusMessageP(StatusId));
  TerminalSendStringP(PSTR(STATUS_MESSAGE_TRAILER));

  if (CommandFound && (TerminalBufferOut[0] != '\0') ) {
    /* Send optional answer and/or status message */
    TerminalSendString(TerminalBufferOut);
    TerminalSendStringP(PSTR(OPTIONAL_ANSWER_TRAILER));
  }
}

void CommandLineInit(void)
{
  TerminalBufferOut[0] = '\0';
  BufferIdx = 0;
}

bool CommandLineProcessByte(uint8_t Byte) {
    if (IS_CHARACTER(Byte)) {
        /* Store uppercase character */
        //if (IS_LOWERCASE(Byte)) {
        //    Byte = TO_UPPERCASE(Byte);
        //}

        /* Prevent buffer overflow and account for '\0' */
        if (BufferIdx < TERMINAL_BUFFER_SIZE - 1) {
            TerminalBufferOut[BufferIdx++] = Byte;
        }
    } else if (Byte == '\r') {
        /* Process on \r. Terminate string and decode. */
        TerminalBufferOut[BufferIdx] = '\0';
        BufferIdx = 0;

        if (!TaskPending)
            DecodeCommand();
    } else if (Byte == '\b') {
        /* Backspace. Delete last character in buffer. */
        if (BufferIdx > 0) {
            BufferIdx--;
        }
    } else if (Byte == ESCAPE_CHAR) {
        /* Drop buffer on escape */
        BufferIdx = 0;
    } else {
        /* Ignore other chars */
    }

    return true;
}

INLINE void Timeout(void)
{
    TaskPending = false;
    TerminalSendStringP(GetStatusMessageP(COMMAND_ERR_TIMEOUT_ID));
    TerminalSendStringP(PSTR(STATUS_MESSAGE_TRAILER));

    if (CommandLinePendingTaskTimeout != NO_FUNCTION)
    {
        CommandLinePendingTaskTimeout(); // call the function that ends the task
        CommandLinePendingTaskTimeout = NO_FUNCTION;
    }
}

void CommandLineTick(void)
{
    if (TaskPending &&
            GlobalSettings.ActiveSettingPtr->PendingTaskTimeout != 0 && // 0 means no timeout
            SYSTICK_DIFF_100MS(TaskPendingSince) >= GlobalSettings.ActiveSettingPtr->PendingTaskTimeout) // timeout expired
    {
        Timeout();
    }
}

void CommandLinePendingTaskBreak(void)
{
    if (!TaskPending)
        return;

    Timeout();
}

void CommandLinePendingTaskFinished(CommandStatusIdType ReturnStatusID, char const * const OutMessage)
{
    if (!TaskPending) // if no task is pending, no task can be finished
        return;
    TaskPending = false;

    TerminalSendStringP(GetStatusMessageP(ReturnStatusID));
    TerminalSendStringP(PSTR(STATUS_MESSAGE_TRAILER));

    if (OutMessage != NULL)
    {
        TerminalSendString(OutMessage);
        TerminalSendStringP(PSTR(OPTIONAL_ANSWER_TRAILER));
    }
}

void CommandLineAppendData(void const * const Buffer, uint16_t Bytes)
{
    char* pTerminalBuffer = (char*) TerminalBufferOut;

    uint16_t tmpBytes = Bytes;
    if (Bytes > (TERMINAL_BUFFER_SIZE / 2))
        tmpBytes = TERMINAL_BUFFER_SIZE / 2;
    Bytes -= tmpBytes;

    BufferToHexString(pTerminalBuffer, TERMINAL_BUFFER_SIZE, Buffer, tmpBytes);
    TerminalSendString(pTerminalBuffer);

    uint8_t i = 1;
    while (Bytes > (TERMINAL_BUFFER_SIZE / 2))
    {
        Bytes -= TERMINAL_BUFFER_SIZE / 2;
        BufferToHexString(pTerminalBuffer, TERMINAL_BUFFER_SIZE, 
			  Buffer + i * TERMINAL_BUFFER_SIZE / 2, TERMINAL_BUFFER_SIZE);
        TerminalSendString(pTerminalBuffer);
        i++;
    }

    if (Bytes > 0)
    {
        BufferToHexString(pTerminalBuffer, TERMINAL_BUFFER_SIZE, 
			  Buffer + i * TERMINAL_BUFFER_SIZE / 2, Bytes);
        TerminalSendString(pTerminalBuffer);
    }

    TerminalSendStringP(PSTR(OPTIONAL_ANSWER_TRAILER));
}
