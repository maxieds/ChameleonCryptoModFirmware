/*
 * CommandLine.h
 *
 *  Created on: 04.05.2013
 *      Author: skuser
 */

#ifndef COMMANDLINE_H_
#define COMMANDLINE_H_

#include "Terminal.h"
#include "Commands.h"

#define STATUS_MESSAGE_TRAILER    "\r\n"
#define OPTIONAL_ANSWER_TRAILER    "\r\n"

const char* GetStatusMessageP(CommandStatusIdType StatusId);

void CommandLineInit(void);
bool CommandLineProcessByte(uint8_t Byte);
void CommandLineTick(void);

void CommandExecute(const char* command);
void CommandLineAppendData(void const * const Buffer, uint16_t Bytes);

/* Functions for timeout commands */
void CommandLinePendingTaskFinished(CommandStatusIdType ReturnStatusID, char const * const OutMessage); // must be called, when the intended task is finished
extern void (*CommandLinePendingTaskTimeout) (void); // gets called on timeout to end the pending task
void CommandLinePendingTaskBreak(void); // this manually triggers a timeout

#endif /* COMMANDLINE_H_ */
