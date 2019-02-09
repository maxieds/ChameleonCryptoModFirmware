#include <stdlib.h>
#include <stdio.h>

#include <avr/pgmspace.h>
#include <avr/io.h>
#include <Settings.h>

#include "Commands.h"
#include "XModem.h"
#include "../Settings.h"
#include "../Chameleon-Mini-Crypto-Board.h"
#include "../LUFA/Version.h"
#include "../Configuration.h"
#include "../Random.h"
#include "../System.h"
#include "../Memory.h"
#include "../Button.h"
#include "../AntennaLevel.h"
#include "../Battery.h"
#include "../Codec/Codec.h"
#include "../ChameleonCrypto.h"
#include "../ChipLocking.h"
#include "XModem.h"

CommandStatusIdType CommandGetVersion(char* OutParam)
{
  snprintf_P(OutParam, TERMINAL_BUFFER_SIZE, PSTR(
    "ChameleonMini RevG %S using LUFA %S compiled with AVR-GCC %S. Based on the open-source NFC tool ChameleonMini. https://github.com/emsec/ChameleonMini commit %S. Firmware build date %S."
    ), PSTR(BUILD_DATE), PSTR(LUFA_VERSION_STRING), PSTR(__VERSION__), PSTR(COMMIT_ID), 
       PSTR(CHAMELEON_MINI_VERSION_STRING)
  );

  return COMMAND_INFO_OK_WITH_TEXT_ID;
}

CommandStatusIdType CommandGetConfig(char* OutParam)
{
  ConfigurationGetByName(OutParam, TERMINAL_BUFFER_SIZE);

  return COMMAND_INFO_OK_WITH_TEXT_ID;
}

CommandStatusIdType CommandSetConfig(char* OutMessage, const char* InParam)
{
    if (COMMAND_IS_SUGGEST_STRING(InParam)) {
        ConfigurationGetList(OutMessage, TERMINAL_BUFFER_SIZE);
        return COMMAND_INFO_OK_WITH_TEXT_ID;
    } else if (ConfigurationSetByName(InParam)) {
        SETTING_UPDATE(GlobalSettings.ActiveSettingPtr->Configuration);
        return COMMAND_INFO_OK_ID;
    } else {
        return COMMAND_ERR_INVALID_PARAM_ID;
    }
}

CommandStatusIdType CommandGetUid(char* OutParam)
{
  uint8_t UidBuffer[COMMAND_UID_BUFSIZE];
  uint16_t UidSize = ActiveConfiguration.UidSize;

  if (UidSize == 0)
  {
      snprintf_P(OutParam, TERMINAL_BUFFER_SIZE, PSTR("NO UID."));
      return COMMAND_INFO_OK_WITH_TEXT_ID;
  }

  ApplicationGetUid(UidBuffer);

  BufferToHexString(OutParam, TERMINAL_BUFFER_SIZE,
    UidBuffer, UidSize);

  return COMMAND_INFO_OK_WITH_TEXT_ID;
}

CommandStatusIdType CommandSetUid(char* OutMessage, const char* InParam)
{
  uint8_t UidBuffer[COMMAND_UID_BUFSIZE];
  uint16_t UidSize = ActiveConfiguration.UidSize;

  if (strcmp_P(InParam, PSTR(COMMAND_UID_RANDOM)) == 0) {
    /* Load with random bytes */
    for (uint8_t i=0; i<UidSize; i++) {
      UidBuffer[i] = RandomGetByte();
    }
  } else {
    /* Convert to Bytes */
    if (HexStringToBuffer(UidBuffer, sizeof(UidBuffer), InParam) != UidSize) {
      /* Malformed input. Abort */
      return COMMAND_ERR_INVALID_PARAM_ID;
    }
  }

  ApplicationSetUid(UidBuffer);

  return COMMAND_INFO_OK_ID;
}

CommandStatusIdType CommandGetReadOnly(char* OutParam)
{
  if (ActiveConfiguration.ReadOnly) {
    OutParam[0] = COMMAND_CHAR_TRUE;
  } else {
    OutParam[0] = COMMAND_CHAR_FALSE;
  }

  OutParam[1] = '\0';

  return COMMAND_INFO_OK_WITH_TEXT_ID;
}

CommandStatusIdType CommandSetReadOnly(char* OutMessage, const char* InParam)
{
  if (InParam[1] == '\0') {
    if (InParam[0] == COMMAND_CHAR_TRUE) {
      ActiveConfiguration.ReadOnly = true;
      return COMMAND_INFO_OK_ID;
    } else if (InParam[0] == COMMAND_CHAR_FALSE) {
      ActiveConfiguration.ReadOnly = false;
      return COMMAND_INFO_OK_ID;
    } else if (InParam[0] == COMMAND_CHAR_SUGGEST) {
        snprintf_P(OutMessage, TERMINAL_BUFFER_SIZE, PSTR("%c,%c"), COMMAND_CHAR_TRUE, COMMAND_CHAR_FALSE);
        return COMMAND_INFO_OK_WITH_TEXT_ID;
    }
  }

  return COMMAND_ERR_INVALID_PARAM_ID;
}

CommandStatusIdType CommandExecUpload(char* OutMessage)
{
    XModemReceive(MemoryUploadBlock);
    return COMMAND_INFO_XMODEM_WAIT_ID;
}

CommandStatusIdType CommandExecParamUploadEncrypted(char *outMessage, const char *InParams) { 
		
     const char *uploadEncSyntaxErrorMsg = PSTR("Syntax: UPLOAD_ENCRYPTED KeyIdx TimeStampSalt");
     char *InParamsCopy = (char *) malloc((strlen(InParams) + 1) * sizeof(char));
     strcpy(InParamsCopy, InParams);
     char *keyIndex = strtok(InParamsCopy, COMMAND_ARGSEP);
     if(keyIndex == NULL) {
	  strncpy(outMessage, uploadEncSyntaxErrorMsg, TERMINAL_BUFFER_SIZE); 
          free(InParamsCopy);
	  return COMMAND_ERR_INVALID_PARAM_ID;
     }
     char *timestampSalt = strtok(NULL, COMMAND_ARGSEP);
     if(timestampSalt == NULL) {
	  strncpy(outMessage, uploadEncSyntaxErrorMsg, TERMINAL_BUFFER_SIZE);
          free(InParamsCopy);
	  return COMMAND_ERR_INVALID_PARAM_ID;
     }
     
     size_t keyIndexNum = atoi(keyIndex);
     if(keyIndexNum >= NUM_KEYS_STORAGE) {
          snprintf_P(outMessage, TERMINAL_BUFFER_SIZE,
                     PSTR("Key index #%d out of range. Try keys indexed 0-%d."), 
	              keyIndexNum, NUM_KEYS_STORAGE - 1);
          free(InParamsCopy);
	  return COMMAND_ERR_INVALID_PARAM_ID;
     }
     size_t saltDataByteCount = 0;
     uint8_t *LocalIVSaltData = GetKeyDataFromString(timestampSalt, &saltDataByteCount);
     free(InParamsCopy);
     XModemReceiveEncrypted(CryptoMemoryUploadBlock, keyIndexNum, 
	                    LocalIVSaltData, saltDataByteCount);
     return COMMAND_INFO_XMODEM_WAIT_ID;
}

CommandStatusIdType CommandExecUploadStatus(char *OutMessage) { 
     CommandStatusIdType uploadStatus = XModemEncryptedUploadStatus;
     if(uploadStatus == UPLOAD_STATUS_ERROR_ID) { // reset to OK after checking this:
          XModemEncryptedUploadStatus = UPLOAD_STATUS_OK_ID;
     }
     return uploadStatus;
}

CommandStatusIdType CommandExecDownload(char* OutMessage)
{
    XModemSend(MemoryDownloadBlock);
    return COMMAND_INFO_XMODEM_WAIT_ID;
}

CommandStatusIdType CommandExecReset(char* OutMessage)
{
  USB_Detach();
  USB_Disable();

  SystemReset();

  return COMMAND_INFO_OK_ID;
}

#ifdef SUPPORT_FIRMWARE_UPGRADE
CommandStatusIdType CommandExecUpgrade(char* OutMessage)
{
  USB_Detach();
  USB_Disable();

  SystemEnterBootloader();

  return COMMAND_INFO_OK_ID;
}
#endif

CommandStatusIdType CommandGetMemSize(char* OutParam)
{
  snprintf_P(OutParam, TERMINAL_BUFFER_SIZE, PSTR("%u"), ActiveConfiguration.MemorySize);

  return COMMAND_INFO_OK_WITH_TEXT_ID;
}

CommandStatusIdType CommandGetUidSize(char* OutParam)
{
    snprintf_P(OutParam, TERMINAL_BUFFER_SIZE, PSTR("%u"), ActiveConfiguration.UidSize);

    return COMMAND_INFO_OK_WITH_TEXT_ID;
}

CommandStatusIdType CommandGetRButton(char* OutParam)
{
    ButtonGetActionByName(BUTTON_R_PRESS_SHORT, OutParam, TERMINAL_BUFFER_SIZE);

    return COMMAND_INFO_OK_WITH_TEXT_ID;
}

CommandStatusIdType CommandSetRButton(char* OutMessage, const char* InParam)
{
    if (COMMAND_IS_SUGGEST_STRING(InParam)) {
        /* Get suggestion list */
        ButtonGetActionList(OutMessage, TERMINAL_BUFFER_SIZE);
        return COMMAND_INFO_OK_WITH_TEXT_ID;
    } else if (ButtonSetActionByName(BUTTON_R_PRESS_SHORT, InParam)) {
        /* Try to set action name */
        SettingsSave();
        return COMMAND_INFO_OK_ID;
    } else {
        return COMMAND_ERR_INVALID_PARAM_ID;
    }
}

CommandStatusIdType CommandGetRButtonLong(char* OutParam)
{
    ButtonGetActionByName(BUTTON_R_PRESS_LONG, OutParam, TERMINAL_BUFFER_SIZE);

    return COMMAND_INFO_OK_WITH_TEXT_ID;
}

CommandStatusIdType CommandSetRButtonLong(char* OutMessage, const char* InParam)
{
    if (COMMAND_IS_SUGGEST_STRING(InParam)) {
        /* Get suggestion list */
        ButtonGetActionList(OutMessage, TERMINAL_BUFFER_SIZE);
        return COMMAND_INFO_OK_WITH_TEXT_ID;
    } else if (ButtonSetActionByName(BUTTON_R_PRESS_LONG, InParam)) {
        /* Try to set action name */
        SettingsSave();
        return COMMAND_INFO_OK_ID;
    } else {
        return COMMAND_ERR_INVALID_PARAM_ID;
    }
}

CommandStatusIdType CommandGetLButton(char* OutParam)
{
    ButtonGetActionByName(BUTTON_L_PRESS_SHORT, OutParam, TERMINAL_BUFFER_SIZE);

    return COMMAND_INFO_OK_WITH_TEXT_ID;
}

CommandStatusIdType CommandSetLButton(char* OutMessage, const char* InParam)
{
    if (COMMAND_IS_SUGGEST_STRING(InParam)) {
        /* Get suggestion list */
        ButtonGetActionList(OutMessage, TERMINAL_BUFFER_SIZE);
        return COMMAND_INFO_OK_WITH_TEXT_ID;
    } else if (ButtonSetActionByName(BUTTON_L_PRESS_SHORT, InParam)) {
        /* Try to set action name */
        SettingsSave();
        return COMMAND_INFO_OK_ID;
    } else {
        return COMMAND_ERR_INVALID_PARAM_ID;
    }
}

CommandStatusIdType CommandGetLButtonLong(char* OutParam)
{
    ButtonGetActionByName(BUTTON_L_PRESS_LONG, OutParam, TERMINAL_BUFFER_SIZE);

    return COMMAND_INFO_OK_WITH_TEXT_ID;
}

CommandStatusIdType CommandSetLButtonLong(char* OutMessage, const char* InParam)
{
    if (COMMAND_IS_SUGGEST_STRING(InParam)) {
        /* Get suggestion list */
        ButtonGetActionList(OutMessage, TERMINAL_BUFFER_SIZE);
        return COMMAND_INFO_OK_WITH_TEXT_ID;
    } else if (ButtonSetActionByName(BUTTON_L_PRESS_LONG, InParam)) {
        /* Try to set action name */
        SettingsSave();
        return COMMAND_INFO_OK_ID;
    } else {
        return COMMAND_ERR_INVALID_PARAM_ID;
    }
}

CommandStatusIdType CommandGetLedGreen(char* OutParam)
{
    LEDGetFuncByName(LED_GREEN, OutParam, TERMINAL_BUFFER_SIZE);

    return COMMAND_INFO_OK_WITH_TEXT_ID;
}

CommandStatusIdType CommandSetLedGreen(char* OutMessage, const char* InParam)
{
    if (COMMAND_IS_SUGGEST_STRING(InParam)) {
        LEDGetFuncList(OutMessage, TERMINAL_BUFFER_SIZE);
        return COMMAND_INFO_OK_WITH_TEXT_ID;
    } else if (LEDSetFuncByName(LED_GREEN, InParam)) {
        SettingsSave();
        return COMMAND_INFO_OK_ID;
    } else {
        return COMMAND_ERR_INVALID_PARAM_ID;
    }
}

CommandStatusIdType CommandGetLedRed(char* OutParam)
{
    LEDGetFuncByName(LED_RED, OutParam, TERMINAL_BUFFER_SIZE);

    return COMMAND_INFO_OK_WITH_TEXT_ID;
}

CommandStatusIdType CommandSetLedRed(char* OutMessage, const char* InParam)
{
    if (COMMAND_IS_SUGGEST_STRING(InParam)) {
        LEDGetFuncList(OutMessage, TERMINAL_BUFFER_SIZE);
        return COMMAND_INFO_OK_WITH_TEXT_ID;
    } else if (LEDSetFuncByName(LED_RED, InParam)) {
        SettingsSave();
        return COMMAND_INFO_OK_ID;
    } else {
        return COMMAND_ERR_INVALID_PARAM_ID;
    }
}

CommandStatusIdType CommandGetLogMode(char* OutParam)
{
    /* Get Logmode */
    LogGetModeByName(OutParam, TERMINAL_BUFFER_SIZE);

    return COMMAND_INFO_OK_WITH_TEXT_ID;
}

CommandStatusIdType CommandSetLogMode(char* OutMessage, const char* InParam)
{
    if (COMMAND_IS_SUGGEST_STRING(InParam)) {
        LogGetModeList(OutMessage, TERMINAL_BUFFER_SIZE);
        return COMMAND_INFO_OK_WITH_TEXT_ID;
    } else if (LogSetModeByName(InParam)) {
        SettingsSave();
        return COMMAND_INFO_OK_ID;
    } else {
        return COMMAND_ERR_INVALID_PARAM_ID;
    }
}

CommandStatusIdType CommandGetLogMem(char* OutParam)
{
    uint16_t free = LogMemFree();
    snprintf_P(OutParam, TERMINAL_BUFFER_SIZE,
        PSTR("%u (from which %u non-volatile)"), free, (free <= LOG_SIZE) ? 0 : (free - LOG_SIZE));


    return COMMAND_INFO_OK_WITH_TEXT_ID;
}


CommandStatusIdType CommandExecLogDownload(char* OutMessage)
{
    XModemSend(LogMemLoadBlock);
    return COMMAND_INFO_XMODEM_WAIT_ID;
}

CommandStatusIdType CommandExecStoreLog(char* OutMessage)
{
    LogSRAMToFRAM();
    return COMMAND_INFO_OK_ID;
}

CommandStatusIdType CommandExecLogClear(char* OutMessage)
{
    LogMemClear();
    return COMMAND_INFO_OK_ID;
}

CommandStatusIdType CommandGetSetting(char* OutParam)
{
    SettingsGetActiveByName(OutParam, TERMINAL_BUFFER_SIZE);
    return COMMAND_INFO_OK_WITH_TEXT_ID;
}

CommandStatusIdType CommandSetSetting(char* OutMessage, const char* InParam)
{
    if (SettingsSetActiveByName(InParam)) {
        return COMMAND_INFO_OK_ID;
    } else {
        return COMMAND_ERR_INVALID_PARAM_ID;
    }
}

CommandStatusIdType CommandExecClear(char* OutMessage)
{
    MemoryClear();
    return COMMAND_INFO_OK_ID;
}

CommandStatusIdType CommandExecStore(char* OutMessage)
{
    MemoryStore();
    return COMMAND_INFO_OK_ID;
}

CommandStatusIdType CommandExecRecall(char* OutMessage)
{
    MemoryRecall();
    return COMMAND_INFO_OK_ID;
}

CommandStatusIdType CommandGetCharging(char* OutMessage)
{
    if (BatteryIsCharging()) {
        return COMMAND_INFO_TRUE_ID;
    } else {
        return COMMAND_INFO_FALSE_ID;
    }
}

CommandStatusIdType CommandExecHelp(char* OutMessage)
{
    const CommandEntryType* EntryPtr = CommandTable;
    uint16_t ByteCount = TERMINAL_BUFFER_SIZE - 1; /* Account for '\0' */

    while(strcmp_P(COMMAND_LIST_END, EntryPtr->Command) != 0 && ByteCount > 0) {
        const char* CommandName = EntryPtr->Command;
        char c;

        while( (c = pgm_read_byte(CommandName)) != '\0' && ByteCount > 1) {
            *OutMessage++ = c;
            CommandName++;
            ByteCount--;
        }

        *OutMessage++ = ',';
        ByteCount--;

        EntryPtr++;
    }

    *--OutMessage = '\0';

    return COMMAND_INFO_OK_WITH_TEXT_ID;
}

CommandStatusIdType CommandGetRssi(char* OutParam)
{
    snprintf_P(OutParam, TERMINAL_BUFFER_SIZE,
        PSTR("%5u mV"), AntennaLevelGet());

    return COMMAND_INFO_OK_WITH_TEXT_ID;
}

CommandStatusIdType CommandGetSysTick(char* OutParam)
{
    snprintf_P(OutParam, TERMINAL_BUFFER_SIZE, PSTR("%4.4X"), SystemGetSysTick());

    return COMMAND_INFO_OK_WITH_TEXT_ID;
}

CommandStatusIdType CommandExecParamSend(char* OutMessage, const char* InParams)
{
    if (GlobalSettings.ActiveSettingPtr->Configuration != CONFIG_ISO14443A_READER)
        return COMMAND_ERR_INVALID_USAGE_ID;

    ApplicationReset();
    Reader14443CurrentCommand = Reader14443_Send;

    char const * paramTwo = strchr(InParams, ' ');
    uint16_t length;
    if (paramTwo == NULL) // this means, we have to calculate the length
    {
        length = strlen(InParams);
        if (length&1) // return error when length is odd
            return COMMAND_ERR_INVALID_PARAM_ID;
        length /= 2;
        if (length == 1)
        {
            ReaderSendBitCount = 7; // this is a short frame
        } else {
            ReaderSendBitCount = length * 8;
        }
    } else if(paramTwo == (InParams+4)) { // we have a bitcount prepended
        uint8_t tmp[2];
        HexStringToBuffer(tmp, 2, InParams);
        ReaderSendBitCount = (tmp[0]<<8) + tmp[1]; // set our BitCount to the given value
        InParams = ++paramTwo; // set InParams to the beginning of the second parameter
        length = strlen(InParams);
        if ((length&1) || (length / 2 * 8) < ReaderSendBitCount) // this parameter is malformed, if it is odd or if there are less bits than the BitCount indicates
            return COMMAND_ERR_INVALID_PARAM_ID;
    } else { // any other case means we have malformed parameters
        return COMMAND_ERR_INVALID_PARAM_ID;
    }

    HexStringToBuffer(ReaderSendBuffer, (ReaderSendBitCount+7)/8, InParams);

    Reader14443ACodecStart();

    return TIMEOUT_COMMAND;
}

CommandStatusIdType CommandExecParamSendRaw(char* OutMessage, const char* InParams)
{
    if (GlobalSettings.ActiveSettingPtr->Configuration != CONFIG_ISO14443A_READER)
        return COMMAND_ERR_INVALID_USAGE_ID;

    ApplicationReset();
    Reader14443CurrentCommand = Reader14443_Send_Raw;

    char const * paramTwo = strchr(InParams, ' ');
    uint16_t length;
    if (paramTwo == NULL) // this means, we have to calculate the length
    {
        length = strlen(InParams);
        if (length&1) // return error when length is odd
            return COMMAND_ERR_INVALID_PARAM_ID;
        length /= 2;
        if (length == 1)
        {
            ReaderSendBitCount = 7; // this is a short frame
        } else {
            length *= 8;
            ReaderSendBitCount = length - (length % 9); // how many bytes+paritybit match into our input?
        }
    } else if(paramTwo == (InParams+4)) { // we have a bitcount prepended
        uint8_t tmp[2];
        HexStringToBuffer(tmp, 2, InParams);
        ReaderSendBitCount = (tmp[0]<<8) + tmp[1]; // set our BitCount to the given value
        if (ReaderSendBitCount != 7 && (ReaderSendBitCount % 8)) // since we have to add parity bits here (in case this is not a short frame), the number of bits to be sent has to be a multiple of 8
            return COMMAND_ERR_INVALID_PARAM_ID;
        InParams = ++paramTwo; // set InParams to the beginning of the second parameter
        length = strlen(InParams);
        if ((length&1) || (length / 2 * 8) < ReaderSendBitCount) // this parameter is malformed, if it is odd or if there are less bits than the BitCount indicates
            return COMMAND_ERR_INVALID_PARAM_ID;
    } else { // any other case means we have malformed parameters
        return COMMAND_ERR_INVALID_PARAM_ID;
    }

    HexStringToBuffer(ReaderSendBuffer, (ReaderSendBitCount+7)/8, InParams);

    Reader14443ACodecStart();

    return TIMEOUT_COMMAND;
}

CommandStatusIdType CommandExecDumpMFU(char* OutMessage)
{
    if (GlobalSettings.ActiveSettingPtr->Configuration != CONFIG_ISO14443A_READER)
        return COMMAND_ERR_INVALID_USAGE_ID;
    ApplicationReset();

    Reader14443CurrentCommand = Reader14443_Read_MF_Ultralight;
    Reader14443AAppInit();
    Reader14443ACodecStart();
    CommandLinePendingTaskTimeout = &Reader14443AAppTimeout;
    return TIMEOUT_COMMAND;
}

CommandStatusIdType CommandExecGetUid(char* OutMessage) // this function is for reading the uid in reader mode
{
    if (GlobalSettings.ActiveSettingPtr->Configuration != CONFIG_ISO14443A_READER)
        return COMMAND_ERR_INVALID_USAGE_ID;
    ApplicationReset();

    Reader14443CurrentCommand = Reader14443_Get_UID;
    Reader14443AAppInit();
    Reader14443ACodecStart();
    CommandLinePendingTaskTimeout = &Reader14443AAppTimeout;
    return TIMEOUT_COMMAND;
}

CommandStatusIdType CommandExecIdentifyCard(char* OutMessage)
{
    if (GlobalSettings.ActiveSettingPtr->Configuration != CONFIG_ISO14443A_READER)
        return COMMAND_ERR_INVALID_USAGE_ID;
    ApplicationReset();

    Reader14443CurrentCommand = Reader14443_Identify;
    Reader14443AAppInit();
    Reader14443ACodecStart();
    CommandLinePendingTaskTimeout = &Reader14443AAppTimeout;
    return TIMEOUT_COMMAND;
}

CommandStatusIdType CommandGetTimeout(char* OutParam)
{
    snprintf_P(OutParam, TERMINAL_BUFFER_SIZE, PSTR("%u ms"), GlobalSettings.ActiveSettingPtr->PendingTaskTimeout * 100);
    return COMMAND_INFO_OK_WITH_TEXT_ID;
}

CommandStatusIdType CommandSetTimeout(char* OutMessage, const char* InParam)
{
    if (COMMAND_IS_SUGGEST_STRING(InParam))
    {
        snprintf_P(OutMessage, TERMINAL_BUFFER_SIZE, PSTR("0 = no timeout\r\n1-600 = 100 ms - 60000 ms timeout"));
        return COMMAND_INFO_OK_WITH_TEXT_ID;
    }
    uint16_t tmp = 601;
    if (!sscanf_P(InParam, PSTR("%5d"), &tmp) || tmp > 600)
        return COMMAND_ERR_INVALID_PARAM_ID;
    GlobalSettings.ActiveSettingPtr->PendingTaskTimeout = tmp;
    SETTING_UPDATE(GlobalSettings.ActiveSettingPtr->PendingTaskTimeout);
    return COMMAND_INFO_OK_ID;
}

CommandStatusIdType CommandGetThreshold(char* OutParam)
{
    snprintf_P(OutParam, TERMINAL_BUFFER_SIZE, PSTR("%u"), GlobalSettings.ActiveSettingPtr->ReaderThreshold);
    return COMMAND_INFO_OK_WITH_TEXT_ID;
}

CommandStatusIdType CommandSetThreshold(char* OutMessage, const char* InParam)
{
    if (COMMAND_IS_SUGGEST_STRING(InParam))
    {
        snprintf_P(OutMessage, TERMINAL_BUFFER_SIZE, PSTR("Any integer from 0 to %u. Reference voltage will be (VCC * THRESHOLD / 4095) mV."), CODEC_MAXIMUM_THRESHOLD);
        return COMMAND_INFO_OK_WITH_TEXT_ID;
    }
    uint16_t tmp = 0;
    if (!sscanf_P(InParam, PSTR("%5d"), &tmp) || tmp > CODEC_MAXIMUM_THRESHOLD)
        return COMMAND_ERR_INVALID_PARAM_ID;
    DACB.CH0DATA = tmp;
    GlobalSettings.ActiveSettingPtr->ReaderThreshold = tmp;
    SETTING_UPDATE(GlobalSettings.ActiveSettingPtr->ReaderThreshold);
    return COMMAND_INFO_OK_ID;
}

CommandStatusIdType CommandSetField(char* OutMessage, const char* InParam)
{
    if (COMMAND_IS_SUGGEST_STRING(InParam))
    {
        snprintf_P(OutMessage, TERMINAL_BUFFER_SIZE, PSTR("%c,%c"), COMMAND_CHAR_TRUE, COMMAND_CHAR_FALSE);
        return COMMAND_INFO_OK_WITH_TEXT_ID;
    }
    if (InParam[0] == COMMAND_CHAR_TRUE)
    {
        CodecReaderFieldStart();
    } else if(InParam[0] == COMMAND_CHAR_FALSE) {
        CodecReaderFieldStop();
    } else {
        return COMMAND_ERR_INVALID_PARAM_ID;
    }
    return COMMAND_INFO_OK_ID;
}

CommandStatusIdType CommandGetField(char* OutMessage)
{
    if (CodecGetReaderField())
        OutMessage[0] = COMMAND_CHAR_TRUE;
    else
        OutMessage[0] = COMMAND_CHAR_FALSE;
    OutMessage[1] = '\0';
    return COMMAND_INFO_OK_WITH_TEXT_ID;
}


CommandStatusIdType CommandExecAutocalibrate(char* OutMessage)
{
    if (GlobalSettings.ActiveSettingPtr->Configuration == CONFIG_ISO14443A_READER){
        ApplicationReset();

        Reader14443CurrentCommand = Reader14443_Autocalibrate;
        Reader14443AAppInit();
        Reader14443ACodecStart();
        CommandLinePendingTaskTimeout = &Reader14443AAppTimeout;
        return TIMEOUT_COMMAND;
    }
    else if (GlobalSettings.ActiveSettingPtr->Configuration == CONFIG_ISO14443A_SNIFF){
        ApplicationReset();

        Sniff14443CurrentCommand = Sniff14443_Autocalibrate;
        Sniff14443AAppInit();
        CommandLinePendingTaskTimeout = &Sniff14443AAppTimeout;
        return TIMEOUT_COMMAND;
    }
    else {
        return COMMAND_ERR_INVALID_USAGE_ID;
    }

}

CommandStatusIdType CommandExecClone(char *OutMessage)
{
    ConfigurationSetById(CONFIG_ISO14443A_READER);

    ApplicationReset();

    Reader14443CurrentCommand = Reader14443_Identify_Clone;
    Reader14443AAppInit();
    Reader14443ACodecStart();
    CommandLinePendingTaskTimeout = &Reader14443AAppTimeout;

    return TIMEOUT_COMMAND;
}

CommandStatusIdType CommandExecParamKeyAuth(char *OutMessage, const char *InParams) { 
     if(InParams == NULL || InParams[0] == '\0') {
          strncpy_P(OutMessage, PSTR("No authentication passhrase specified."), TERMINAL_BUFFER_SIZE);
	  return COMMAND_ERR_INVALID_PARAM_ID;
     }
     size_t authPassphraseLen = 0, numApprovedEdits = 0;
     const char *numAuthedEditsStr = strchrnul(InParams, COMMAND_ARGSEP);
     char authPassphrase[MAX_COMMAND_ARGLEN + 1];
     authPassphraseLen = numAuthedEditsStr - InParams;
     strncpy(authPassphrase, InParams, authPassphraseLen);
     authPassphrase[authPassphraseLen] = '\0';
     if(!AuthLockByPassphrase(authPassphrase)) { 
          strncpy_P(OutMessage, PSTR("Incorrect authentication passphrase specified."), TERMINAL_BUFFER_SIZE);
	  return COMMAND_ERR_AUTH_FAILED_ID;
     }
     else if(*numAuthedEditsStr == '\0') {
          numApprovedEdits = 1;
     }
     else {
          char *nextArgDelim = strchrnul(++numAuthedEditsStr, COMMAND_ARGSEP);
	  if(*nextArgDelim == '\0') {
               numApprovedEdits = atoi(numAuthedEditsStr);
	  }
	  else {
               strncpy_P(OutMessage, PSTR("Too many parameters specified."), TERMINAL_BUFFER_SIZE);
	       return COMMAND_ERR_INVALID_PARAM_ID;      
	  }
     }
     ActiveConfiguration.KeyChangeAuth = numApprovedEdits;
     return COMMAND_INFO_OK_ID;
}

CommandStatusIdType CommandExecParamSetKey(char *OutMessage, const char *InParams) { 
     if(InParams == NULL || InParams[0] == '\0') {
          return COMMAND_ERR_INVALID_PARAM_ID;
     }
     else if(ActiveConfiguration.KeyChangeAuth <= 0) {
          return COMMAND_ERR_AUTH_FAILED_ID;
     }
     size_t keyIndexStrLen = 0, keyDataStrLen = 0;
     const char *keyDataStr = strchrnul(InParams, COMMAND_ARGSEP);
     char *nullBytePos = strchrnul(++keyDataStr, COMMAND_ARGSEP);
     if(*keyDataStr == '\0' || *nullBytePos != '\0' || nullBytePos == keyDataStr) {
          strncpy_P(OutMessage, PSTR("Invalid number of parameters specified."), TERMINAL_BUFFER_SIZE);
          return COMMAND_ERR_INVALID_PARAM_ID;
     }
     keyIndexStrLen = keyDataStr - InParams;
     keyDataStrLen = nullBytePos - keyDataStr;
     if(keyIndexStrLen == 0 || keyDataStrLen == 0 || (keyIndexStrLen + 1) > MAX_COMMAND_ARGLEN) {
          return COMMAND_ERR_INVALID_USAGE_ID;
     }
     char keyIndexStr[MAX_COMMAND_ARGLEN];
     strncpy(keyIndexStr, InParams, keyIndexStrLen);
     keyIndexStr[keyIndexStrLen] = '\0';
     int keyIndex = atoi(keyIndexStr);
     if(keyIndex < 0 || keyIndex >= NUM_KEYS_STORAGE) {
          strncpy_P(OutMessage, PSTR("Invalid key index specified."), TERMINAL_BUFFER_SIZE);
	  return COMMAND_ERR_INVALID_USAGE_ID;
     }
     uint8_t keyDataBytes[MAX_KEY_LENGTH];
     uint16_t keyDataByteCount = HexStringToBuffer(keyDataBytes, MAX_KEY_LENGTH, keyDataStr);
     if(!SetKeyData(keyIndex, keyDataBytes, keyDataByteCount)) {
          return COMMAND_INFO_FALSE_ID;
     }
     ActiveConfiguration.KeyChangeAuth -= 1;
     return COMMAND_INFO_OK_ID;
}

CommandStatusIdType CommandExecParamGenKey(char *OutMessage, const char *InParams) { 
     if(InParams == NULL || InParams[0] == '\0') {
          return COMMAND_ERR_INVALID_PARAM_ID;
     }
     else if(ActiveConfiguration.KeyChangeAuth <= 0) {
          return COMMAND_ERR_AUTH_FAILED_ID;
     }
     size_t keyIndexStrLen = 0, pphStrLen = 0;
     const char *pphStr = strchrnul(InParams, COMMAND_ARGSEP);
     char *nullBytePos = strchrnul(++pphStr, COMMAND_ARGSEP);
     if(*pphStr == '\0' || *nullBytePos != '\0' || nullBytePos == pphStr) {
          strncpy_P(OutMessage, PSTR("Invalid number of parameters specified."), TERMINAL_BUFFER_SIZE);
          return COMMAND_ERR_INVALID_PARAM_ID;
     }
     keyIndexStrLen = pphStr - InParams;
     pphStrLen = nullBytePos - pphStr;
     if(keyIndexStrLen == 0 || pphStrLen == 0 || (keyIndexStrLen + 1) > MAX_COMMAND_ARGLEN) {
          return COMMAND_ERR_INVALID_USAGE_ID;
     }
     char keyIndexStr[MAX_COMMAND_ARGLEN];
     strncpy(keyIndexStr, InParams, keyIndexStrLen);
     keyIndexStr[keyIndexStrLen] = '\0';
     int keyIndex = atoi(keyIndexStr);
     if(keyIndex < 0 || keyIndex >= NUM_KEYS_STORAGE) {
          strncpy_P(OutMessage, PSTR("Invalid key index specified."), TERMINAL_BUFFER_SIZE);
	  return COMMAND_ERR_INVALID_USAGE_ID;
     }
     if(!PassphraseToAESKeyData(keyIndex, pphStr)) {
          return COMMAND_INFO_FALSE_ID;
     }
     ActiveConfiguration.KeyChangeAuth -= 1;
     char keyData[MAX_COMMAND_ARGLEN];
     BufferToHexString(keyData, MAX_COMMAND_ARGLEN, 
		       GlobalSettings.KeyData.keys[keyIndex], 
		       GlobalSettings.KeyData.keyLengths[keyIndex]);
     strncpy(OutMessage, keyData, TERMINAL_BUFFER_SIZE);
     return COMMAND_INFO_OK_WITH_TEXT_ID;
}

#ifdef ENABLE_ADMIN_LEVEL_DEBUGGING
CommandStatusIdType CommandExecParamGetKey(char *OutMessage, const char *InParams) { 
     if(ActiveConfiguration.KeyChangeAuth <= 0) {
          return COMMAND_ERR_AUTH_FAILED_ID;
     }
     char *nullBytePos = strchrnul(InParams, COMMAND_ARGSEP);
     if(nullBytePos == NULL || nullBytePos == InParams) {
          strncpy_P(OutMessage, PSTR("No key index parameter specified."), TERMINAL_BUFFER_SIZE);
	  return COMMAND_ERR_INVALID_PARAM_ID;
     }
     else if(*nullBytePos == COMMAND_ARGSEP) {
          strncpy_P(OutMessage, PSTR("Too many parameters specified."), TERMINAL_BUFFER_SIZE);
	  return COMMAND_ERR_INVALID_PARAM_ID;
     }
     int keyIndex = atoi(InParams);
     if(keyIndex < 0 || keyIndex >= NUM_KEYS_STORAGE) {
          strncpy_P(OutMessage, PSTR("Invalid key index specified."), TERMINAL_BUFFER_SIZE);
	  return COMMAND_ERR_INVALID_PARAM_ID;
     }
     ActiveConfiguration.KeyChangeAuth -= 1;
     char keyData[MAX_COMMAND_ARGLEN];
     BufferToHexString(keyData, MAX_COMMAND_ARGLEN, 
		       GlobalSettings.KeyData.keys[keyIndex], 
		       GlobalSettings.KeyData.keyLengths[keyIndex]);
     strncpy(OutMessage, keyData, TERMINAL_BUFFER_SIZE);
     return COMMAND_INFO_OK_WITH_TEXT_ID;
}
#endif

CommandStatusIdType CommandExecParamLockChip(char *OutMessage, const char *InParam) {
     if(REQUIRE_PASSPHRASE_TO_LOCK_CHIP) { 
          char *InParamCopy = (char *) malloc((strlen(InParam) + 1) * sizeof(char));
	  strcpy(InParamCopy, InParam);
	  char *authPwd = strtok(InParamCopy, COMMAND_ARGSEP);
          if(!AuthLockByPassphrase(authPwd)) { 
               strncpy(OutMessage, PSTR("Invalid flash password specified."), TERMINAL_BUFFER_SIZE);
	       free(InParamCopy);
	       return COMMAND_ERR_AUTH_FAILED_ID;
	  }
     }
     ChameleonLockBootloaderMemoryBits();
     ChameleonLockEEPROMMemoryBits();
     return COMMAND_INFO_TRUE_ID;
}

CommandStatusIdType CommandExecParamUnlockChip(char *OutMessage, const char *InParam) { 
     char *InParamCopy = (char *) malloc((strlen(InParam) + 1) * sizeof(char));
     strcpy(InParamCopy, InParam);
     char *authPwd = strtok(InParamCopy, COMMAND_ARGSEP);
     if(!AuthLockByPassphrase(authPwd)) { 
          strncpy(OutMessage, PSTR("Invalid flash password specified."), TERMINAL_BUFFER_SIZE);
	  free(InParamCopy);
	  return COMMAND_ERR_AUTH_FAILED_ID;
     }
     ChameleonUnlockBootloaderMemoryBits();
     ChameleonUnlockEEPROMMemoryBits();
     return COMMAND_INFO_TRUE_ID;
}
