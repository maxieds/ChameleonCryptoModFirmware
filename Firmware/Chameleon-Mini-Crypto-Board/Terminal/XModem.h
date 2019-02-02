/*
 * TerminalXModem.h
 *
 *  Created on: 22.03.2013
 *      Author: skuser
 */

#ifndef XMODEM_H_
#define XMODEM_H_

#include "../Common.h"
#include "Commands.h"

extern CommandStatusIdType XModemEncryptedUploadStatus;

typedef bool (*XModemCallbackType) (void* ByteBuffer, uint32_t BlockAddress, uint16_t ByteCount);

void XModemReceive(XModemCallbackType CallbackFunc);
void XModemReceiveEncrypted(XModemCallbackType CallbackFunc, size_t keyIndex, 
		            uint8_t *ivSaltData, size_t saltDataByteCount);
void XModemSend(XModemCallbackType CallbackFunc);

bool XModemProcessByte(uint8_t Byte);
void XModemTick(void);

#endif /* TERMINALXMODEM_H_ */
