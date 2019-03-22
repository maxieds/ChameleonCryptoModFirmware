#include "Settings.h"
#include <avr/eeprom.h>
#include "Configuration.h"
#include "Log.h"
#include "Memory.h"
#include "LEDHook.h"
#include "Terminal/CommandLine.h"
#include "System.h"
#include "ChameleonCrypto.h"

#define SETTING_TO_INDEX(S) (S - SETTINGS_FIRST)
#define INDEX_TO_SETTING(I) (I + SETTINGS_FIRST)

SettingsType GlobalSettings;
static const SettingsEntryType DefaultSEType = {
        {
                DEFAULT_RBUTTON_ACTION, 
		DEFAULT_RBUTTON_ACTION, 
		DEFAULT_LBUTTON_ACTION, 
		DEFAULT_LBUTTON_ACTION
        },
        DEFAULT_LOG_MODE,
        DEFAULT_CONFIGURATION,
        DEFAULT_RED_LED_ACTION,
        DEFAULT_GREEN_LED_ACTION,
        DEFAULT_PENDING_TASK_TIMEOUT,
        DEFAULT_READER_THRESHOLD
};

SettingsType EEMEM StoredSettings = {
    .KeyData={ 
         .keys={ 
               { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* Zero-Filled */
	       { 0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF, 
		 0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF }, /* Default for testing */
	       { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }, /* All-Ones */
               //#if defined(PSEUDO_RANDOM_AESKEY_DATA)
	       //{ PSEUDO_RANDOM_AESKEY_DATA }
               //#else
	       { 0x94, 0x23, 0x4c, 0xf3, 0xe4, 0x9b, 0x1c, 0x2d, 
	         0x62, 0xe1, 0x5c, 0x97, 0x42, 0xfd, 0x76, 0x11 }  /* Pseudo-Random Noise */
               //#endif
	  },
          .keyLengths={ 16, 16, 16, 16 }
     },   
    .ActiveErrorStatus=false, 
    .ActiveSettingIdx=SETTING_TO_INDEX(DEFAULT_SETTING),
    .ActiveSettingPtr=&GlobalSettings.Settings[SETTING_TO_INDEX(DEFAULT_SETTING)],
    .Settings = { 
	[0 ... (SETTINGS_COUNT-1)] = {
        .Configuration = DEFAULT_CONFIGURATION,
        .ButtonActions = {
                [BUTTON_L_PRESS_SHORT] = DEFAULT_LBUTTON_ACTION, 
		[BUTTON_R_PRESS_SHORT] = DEFAULT_RBUTTON_ACTION,
                [BUTTON_L_PRESS_LONG]  = DEFAULT_LBUTTON_ACTION, 
		[BUTTON_R_PRESS_LONG]  = DEFAULT_RBUTTON_ACTION
        },
        .LogMode = DEFAULT_LOG_MODE,
        .LEDRedFunction = DEFAULT_RED_LED_ACTION,
        .LEDGreenFunction = DEFAULT_GREEN_LED_ACTION,
        .PendingTaskTimeout = DEFAULT_PENDING_TASK_TIMEOUT,
        .ReaderThreshold = DEFAULT_READER_THRESHOLD
        }
    }
};

void SettingsLoad(void) {
    ReadEEPBlock((uint16_t) &StoredSettings, &GlobalSettings, sizeof(SettingsType));
    // reset the error status when we boot up the board a new time:
    GlobalSettings.ActiveErrorStatus = false;
}

void SettingsSave(void) {
#if ENABLE_EEPROM_SETTINGS
    WriteEEPBlock((uint16_t) &StoredSettings, &GlobalSettings, sizeof(SettingsType));
#endif
}

void SettingsCycle(void) {
    uint8_t i = SETTINGS_COUNT;
    uint8_t SettingIdx = GlobalSettings.ActiveSettingIdx;

    while (i-- > 0) {
        /* Try to set one of the SETTINGS_COUNT following settings.
         * But only set if it is not CONFIG_NONE. */
        SettingIdx = (SettingIdx + 1) % SETTINGS_COUNT;

        #ifdef CONFIG_NONE_SUPPORT
        if (GlobalSettings.Settings[SettingIdx].Configuration != CONFIG_NONE) {
            SettingsSetActiveById(INDEX_TO_SETTING(SettingIdx));
            break;
        }
        #endif
    }
}

bool SettingsSetActiveById(uint8_t Setting) {
    if ( (Setting >= SETTINGS_FIRST) && (Setting <= SETTINGS_LAST) ) {
        uint8_t SettingIdx = SETTING_TO_INDEX(Setting);

        /* Break potentially pending timeout task (manual timeout) */
        CommandLinePendingTaskBreak();

        if (SettingIdx != GlobalSettings.ActiveSettingIdx)
        {
            /* Store current memory contents permanently */
            MemoryStore();

            GlobalSettings.ActiveSettingIdx = SettingIdx;
            GlobalSettings.ActiveSettingPtr =
                    &GlobalSettings.Settings[SettingIdx];

            /* Settings have changed. Progress changes through system */
            ConfigurationSetById(GlobalSettings.ActiveSettingPtr->Configuration);
            LogSetModeById(GlobalSettings.ActiveSettingPtr->LogMode);

            /* Recall new memory contents */
            MemoryRecall();

            SETTING_UPDATE(GlobalSettings.ActiveSettingIdx);
            SETTING_UPDATE(GlobalSettings.ActiveSettingPtr);
        }

        /* Notify LED. blink according to current setting */
        LEDHook(LED_SETTING_CHANGE, (LEDActionEnum) (LED_BLINK + SettingIdx));

        return true;
    } else {
        return false;
    }
}

uint8_t SettingsGetActiveById(void) {
    return INDEX_TO_SETTING(GlobalSettings.ActiveSettingIdx);
}

void SettingsGetActiveByName(char* SettingOut, uint16_t BufferSize) {
    SettingOut[0] = SettingsGetActiveById() + '0';
    SettingOut[1] = '\0';
}

bool SettingsSetActiveByName(const char* Setting) {
    uint8_t SettingNr = Setting[0] - '0';

    if (Setting[1] == '\0') {
        LogEntry(LOG_INFO_SETTING_SET, Setting, 1);
        return SettingsSetActiveById(SettingNr);
    } else {
        return false;
    }
}

