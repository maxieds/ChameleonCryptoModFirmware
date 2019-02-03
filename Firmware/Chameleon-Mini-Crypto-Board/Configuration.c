/*
 * Standards.c
 *
 *  Created on: 15.02.2013
 *      Author: skuser
 */

#include <avr/pgmspace.h>

#include "Configuration.h"
#include "Settings.h"
#include "Map.h"
#include "AntennaLevel.h"

/* Map IDs to text */
static const MapEntryType PROGMEM ConfigurationMap[] = {
    { .Id = CONFIG_NONE, 			.Text = "NONE" },
#ifdef CONFIG_MF_ULTRALIGHT_SUPPORT
    { .Id = CONFIG_MF_ULTRALIGHT, 	.Text = "MF_ULTRALIGHT" },
    { .Id = CONFIG_MF_ULTRALIGHT_EV1_80B,   .Text = "MF_ULTRALIGHT_EV1_80B" },
    { .Id = CONFIG_MF_ULTRALIGHT_EV1_164B,   .Text = "MF_ULTRALIGHT_EV1_164B" },
#endif
#ifdef CONFIG_MF_CLASSIC_1K_SUPPORT
    { .Id = CONFIG_MF_CLASSIC_1K, 	.Text = "MF_CLASSIC_1K" },
#endif
#ifdef CONFIG_MF_CLASSIC_1K_7B_SUPPORT
    { .Id = CONFIG_MF_CLASSIC_1K_7B, 	.Text = "MF_CLASSIC_1K_7B" },
#endif
#ifdef CONFIG_MF_CLASSIC_4K_SUPPORT
    { .Id = CONFIG_MF_CLASSIC_4K, 	.Text = "MF_CLASSIC_4K" },
#endif
#ifdef CONFIG_MF_CLASSIC_4K_7B_SUPPORT
    { .Id = CONFIG_MF_CLASSIC_4K_7B, 	.Text = "MF_CLASSIC_4K_7B" },
#endif
#ifdef CONFIG_ISO14443A_SNIFF_SUPPORT
    { .Id = CONFIG_ISO14443A_SNIFF,	.Text = "ISO14443A_SNIFF" },
#endif
#ifdef CONFIG_ISO14443A_READER_SUPPORT
    { .Id = CONFIG_ISO14443A_READER,	.Text = "ISO14443A_READER" },
#endif
};

/* Include all Codecs and Applications */
#include "Codec/Codec.h"
#include "Application/Application.h"

static void CodecInitDummy(void) { }
static void CodecDeInitDummy(void) { }
static void CodecTaskDummy(void) { }
static void ApplicationInitDummy(void) {}
static void ApplicationResetDummy(void) {}
static void ApplicationTaskDummy(void) {}
static void ApplicationTickDummy(void) {}
static uint16_t ApplicationProcessDummy(uint8_t* ByteBuffer, uint16_t ByteCount) { return 0; }
static void ApplicationGetUidDummy(ConfigurationUidType Uid) { }
static void ApplicationSetUidDummy(ConfigurationUidType Uid) { }

static const PROGMEM ConfigurationType ConfigurationTable[] = {
    [CONFIG_NONE] = 
    {
        CodecInitDummy,
        CodecDeInitDummy,
        CodecTaskDummy,
        ApplicationInitDummy,
        ApplicationResetDummy,
        ApplicationTaskDummy,
        ApplicationTickDummy,
        ApplicationProcessDummy,
        ApplicationGetUidDummy,
        ApplicationSetUidDummy,
        0,
        0,
        true, 
	0
    },
#ifdef CONFIG_MF_ULTRALIGHT_SUPPORT
    [CONFIG_MF_ULTRALIGHT] =  
    {
        ISO14443ACodecInit,
        ISO14443ACodecDeInit,
        ISO14443ACodecTask,
        MifareUltralightAppInit,
        MifareUltralightAppReset,
        MifareUltralightAppTask,
        ApplicationTickDummy,
        MifareUltralightAppProcess,
        MifareUltralightGetUid,
        MifareUltralightSetUid,
        MIFARE_ULTRALIGHT_UID_SIZE,
        MIFARE_ULTRALIGHT_MEM_SIZE,
        false, 
	0

    },
    [CONFIG_MF_ULTRALIGHT_EV1_80B] = 
    {
        ISO14443ACodecInit,
        ISO14443ACodecDeInit,
        ISO14443ACodecTask,
        MifareUltralightEV11AppInit,
        MifareUltralightAppReset,
        MifareUltralightAppTask,
        ApplicationTickDummy,
        MifareUltralightAppProcess,
        MifareUltralightGetUid,
        MifareUltralightSetUid,
        MIFARE_ULTRALIGHT_UID_SIZE,
        MIFARE_ULTRALIGHT_EV11_MEM_SIZE,
        false, 
	0

    },
    [CONFIG_MF_ULTRALIGHT_EV1_164B] =  
    {
        ISO14443ACodecInit,
        ISO14443ACodecDeInit,
        ISO14443ACodecTask,
        MifareUltralightEV12AppInit,
        MifareUltralightAppReset,
        MifareUltralightAppTask,
        ApplicationTickDummy,
        MifareUltralightAppProcess,
        MifareUltralightGetUid,
        MifareUltralightSetUid,
        MIFARE_ULTRALIGHT_UID_SIZE,
        MIFARE_ULTRALIGHT_EV12_MEM_SIZE,
        false, 
	0

    },
#endif
#ifdef CONFIG_MF_CLASSIC_1K_SUPPORT
    [CONFIG_MF_CLASSIC_1K] = 
    {
        ISO14443ACodecInit,
        ISO14443ACodecDeInit,
        ISO14443ACodecTask,
        MifareClassicAppInit1K,
        MifareClassicAppReset,
        MifareClassicAppTask,
        ApplicationTickDummy,
        MifareClassicAppProcess,
        MifareClassicGetUid,
        MifareClassicSetUid,
        MIFARE_CLASSIC_UID_SIZE,
        MIFARE_CLASSIC_1K_MEM_SIZE,
        false, 
	0

    },
#endif
#ifdef CONFIG_MF_CLASSIC_1K_7B_SUPPORT
    [CONFIG_MF_CLASSIC_1K_7B] = 
    {
        ISO14443ACodecInit,
        ISO14443ACodecDeInit,
        ISO14443ACodecTask,
        MifareClassicAppInit1K7B,
        MifareClassicAppReset,
        MifareClassicAppTask,
        ApplicationTickDummy,
        MifareClassicAppProcess,
        MifareClassicGetUid,
        MifareClassicSetUid,
        ISO14443A_UID_SIZE_DOUBLE,
        MIFARE_CLASSIC_1K_MEM_SIZE,
        false, 
	0

    },
#endif
#ifdef CONFIG_MF_CLASSIC_4K_SUPPORT
    [CONFIG_MF_CLASSIC_4K] = 
    {
        ISO14443ACodecInit,
        ISO14443ACodecDeInit,
        ISO14443ACodecTask,
        MifareClassicAppInit4K,
        MifareClassicAppReset,
        MifareClassicAppTask,
        ApplicationTickDummy,
        MifareClassicAppProcess,
        MifareClassicGetUid,
        MifareClassicSetUid,
        MIFARE_CLASSIC_UID_SIZE,
        MIFARE_CLASSIC_4K_MEM_SIZE,
        false, 
	0

    },
#endif
#ifdef CONFIG_MF_CLASSIC_4K_7B_SUPPORT
    [CONFIG_MF_CLASSIC_4K_7B] =  
    {
        ISO14443ACodecInit,
        ISO14443ACodecDeInit,
        ISO14443ACodecTask,
        MifareClassicAppInit4K7B,
        MifareClassicAppReset,
        MifareClassicAppTask,
        ApplicationTickDummy,
        MifareClassicAppProcess,
        MifareClassicGetUid,
        MifareClassicSetUid,
        ISO14443A_UID_SIZE_DOUBLE,
        MIFARE_CLASSIC_4K_MEM_SIZE,
        false, 
	0

    },
#endif
#ifdef CONFIG_ISO14443A_SNIFF_SUPPORT
    [CONFIG_ISO14443A_SNIFF] = 
    {
        Sniff14443ACodecInit,
        Sniff14443ACodecDeInit,
        Sniff14443ACodecTask,
        Sniff14443AAppInit,
        Sniff14443AAppReset,
        Sniff14443AAppTask,
        Sniff14443AAppTick,
        Sniff14443AAppProcess,
        ApplicationGetUidDummy,
        ApplicationSetUidDummy,
        0,
        0,
        true, 
	0

    },
#endif
#ifdef CONFIG_ISO14443A_READER_SUPPORT
    [CONFIG_ISO14443A_READER] = 
    {
        Reader14443ACodecInit,
        Reader14443ACodecDeInit,
        Reader14443ACodecTask,
        Reader14443AAppInit,
        Reader14443AAppReset,
        Reader14443AAppTask,
        Reader14443AAppTick,
        Reader14443AAppProcess,
        ApplicationGetUidDummy,
        ApplicationSetUidDummy,
        0,
        0,
        false, 
	0

    },
#endif
};

ConfigurationType ActiveConfiguration;

void ConfigurationInit(void)
{
    memcpy_P(&ActiveConfiguration,
            &ConfigurationTable[CONFIG_NONE], sizeof(ConfigurationType));

    ConfigurationSetById(GlobalSettings.ActiveSettingPtr->Configuration);
}

void ConfigurationSetById( ConfigurationEnum Configuration )
{
    CodecDeInit();

    CommandLinePendingTaskBreak(); // break possibly pending task

    GlobalSettings.ActiveSettingPtr->Configuration = Configuration;

    /* Copy struct from PROGMEM to RAM */
    memcpy_P(&ActiveConfiguration,
            &ConfigurationTable[Configuration], sizeof(ConfigurationType));

    CodecInit();
    ApplicationInit();
}

void ConfigurationGetByName(char* Configuration, uint16_t BufferSize)
{
    MapIdToText(ConfigurationMap, ARRAY_COUNT(ConfigurationMap), GlobalSettings.ActiveSettingPtr->Configuration, Configuration, BufferSize);
}

bool ConfigurationSetByName(const char* Configuration)
{
    MapIdType Id;

    if (MapTextToId(ConfigurationMap, ARRAY_COUNT(ConfigurationMap), Configuration, &Id)) {
        ConfigurationSetById((ConfigurationEnum) Id);
        LogEntry(LOG_INFO_CONFIG_SET, Configuration, StringLength(Configuration, CONFIGURATION_NAME_LENGTH_MAX-1));
        return true;
    } else {
        return false;
    }
}

void ConfigurationGetList(char* List, uint16_t BufferSize)
{
    MapToString(ConfigurationMap, ARRAY_COUNT(ConfigurationMap), List, BufferSize);
}

