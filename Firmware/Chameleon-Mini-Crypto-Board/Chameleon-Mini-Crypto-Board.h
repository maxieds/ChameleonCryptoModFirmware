#ifndef CHAMELEON_MINI_H
#define CHAMELEON_MINI_H

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/power.h>

#include "System.h"
#include "Memory.h"
#include "LED.h"
#include "LEDHook.h"
#include "Terminal/Terminal.h"
#include "Codec/Codec.h"
#include "Application/Application.h"
#include "Configuration.h"
#include "Random.h"
#include "Button.h"
#include "Log.h"
#include "AntennaLevel.h"
#include "Settings.h"

/* The specific extensions in avr-libc for the RevG boards: */
#define CHAMELEON_MINI_AVR_ARCHITECTURE  (__AVR_ATxmega128A4U__)

#define CHAMELEON_MINI_VERSION_STRING    (CHAMELEON_REVSTR)
#define CHAMELEON_MINI_FIRMWARE_MAJOR    (0)
#define CHAMELEON_MINI_FIRMWARE_MINOR    (0)
#define CHAMELEON_MINI_FIRMWARE_REVISION (2)
#define CHAMELEON_MINI_FIRMWARE_VERSION  ("v0.0.2")
#define CHAMELEON_MINI_FIRMWARE_ALTREV   (00.02)

/* Taking revisioning one step further for large scale dists: */
#define CMCBMOD_FIRMWARE_SERIAL           ("abcdef0123456789")

#endif //CHAMELEON_MINI_H

