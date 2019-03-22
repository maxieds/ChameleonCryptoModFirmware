
#include "Terminal.h"
#include "../System.h"
#include "../LEDHook.h"
#include "../LUFADescriptors.h"

#define INIT_DELAY		(2000 / SYSTEM_TICK_MS)


USB_ClassInfo_CDC_Device_t TerminalHandle = {
    {
        0,
        {
            CDC_TX_EPADDR,
            CDC_TXRX_EPSIZE,
            0, 
	    1,
        }, 
        {
            CDC_RX_EPADDR,
            CDC_TXRX_EPSIZE,
            0, 
	    1,
        }, 
        {
            CDC_NOTIFICATION_EPADDR,
            CDC_NOTIFICATION_EPSIZE,
            0, 
	    1,
        },
    }
};

//uint8_t TerminalBufferIn[TERMINAL_BUFFER_SIZE]; 
uint8_t TerminalBufferOut[TERMINAL_BUFFER_SIZE]; 
TerminalStateEnum TerminalState = TERMINAL_UNINITIALIZED;
static uint8_t TerminalInitDelay = INIT_DELAY;

void TerminalSendString(const char* s) {
    CDC_Device_SendString(&TerminalHandle, s);
}

void TerminalSendStringP(const char* s) {
    char c;

    while( (c = pgm_read_byte(s++)) != '\0' ) {
        TerminalSendChar(c);
    }
}

void TerminalSendBlock(const void* Buffer, uint16_t ByteCount)
{
    CDC_Device_SendData(&TerminalHandle, Buffer, ByteCount);
}


static void ProcessByte(void) {
    int16_t Byte = CDC_Device_ReceiveByte(&TerminalHandle);

    if (Byte >= 0) {
        /* Byte received */
        LEDHook(LED_TERMINAL_RXTX, LED_PULSE);

        if (XModemProcessByte(Byte)) {
            /* XModem handled the byte */
        } 
	else if (CommandLineProcessByte(Byte)) {
            /* CommandLine handled the byte */
        }
    }
}

static void SenseVBus(void)
{
    switch(TerminalState) {
    case TERMINAL_UNINITIALIZED:
        if (TERMINAL_VBUS_PORT.IN & TERMINAL_VBUS_MASK) {
            /* Not initialized and VBUS sense high */
            TerminalInitDelay = INIT_DELAY;
            TerminalState = TERMINAL_INITIALIZING;
        }
    break;

    case TERMINAL_INITIALIZING:
        if (--TerminalInitDelay == 0) {
            SystemStartUSBClock();
            USB_Init();
            TerminalState = TERMINAL_INITIALIZED;
        }
        break;

    case TERMINAL_INITIALIZED:
        if (!(TERMINAL_VBUS_PORT.IN & TERMINAL_VBUS_MASK)) {
            /* Initialized and VBUS sense low */
            TerminalInitDelay = INIT_DELAY;
            TerminalState = TERMINAL_UNITIALIZING;
        }
        break;

    case TERMINAL_UNITIALIZING:
        if (--TerminalInitDelay == 0) {
            USB_Disable();
            SystemStopUSBClock();
            TerminalState = TERMINAL_UNINITIALIZED;
        }
        break;

    default:
        break;
    }
}

void TerminalInit(void)
{
    TERMINAL_VBUS_PORT.DIRCLR = TERMINAL_VBUS_MASK;
    TerminalBufferOut[0] = '\0';
}

void TerminalTask(void)
{
    if (TerminalState == TERMINAL_INITIALIZED) {
        CDC_Device_USBTask(&TerminalHandle);
        USB_USBTask();
        ProcessByte();
    }
}

void TerminalTick(void)
{
    SenseVBus();
    if (TerminalState == TERMINAL_INITIALIZED) {
        XModemTick();
        CommandLineTick();
    }
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
    LEDHook(LED_TERMINAL_CONN, LED_ON);
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
    LEDHook(LED_TERMINAL_CONN, LED_OFF);
}


/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
    CDC_Device_ConfigureEndpoints(&TerminalHandle);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
    CDC_Device_ProcessControlRequest(&TerminalHandle);
}


