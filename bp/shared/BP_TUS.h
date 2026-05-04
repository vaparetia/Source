#pragma once
#include "TransfarringDefines.h"

#ifdef __cplusplus
extern "C"
{
#endif

static int const kTUSSlotDataSize = 64 * 1024;
static int const kTUSAccessoryDataSize = 384;

void BP_TUS_Init();

void BP_TUS_WantsSignin();

void BP_TUS_HeartBeat();

#if BP_PS3
void BP_TUS_ForceTerm();
#endif

// *****************************
// Functions to post work
// *****************************
// They return 0 if the post failed because the TUS system is busy
// They return anything else if it works, and it returns a work id

// Posts a work item to get slot data.
unsigned long long BP_TUS_Post_GetSlotData( int const slotIndex );

// Posts a work item to set a slot's data.  
unsigned long long BP_TUS_Post_SetSlotData( int const slotIndex, void const *slotData, int const slotSize, void const *accessoryData );

// Posts a work item to get valid slots.  
unsigned long long BP_TUS_Post_GetSlotInfos();

// Posts a work item to increment the version variable number.
unsigned long long BP_TUS_Post_IncrementSlotVariable(int const slotIndex, unsigned long long incrementBy);

// Posts a work item to delete a file from the cloud.
unsigned long long BP_TUS_Post_DeleteSlotData(int const slotIndex);

// *****************************
// Functions to consume work
// *****************************
// They return 0 is the work is still in-progress, otherwise they 
// return non zero.

// GetSlotData copies the resultant data into pBuffer and accessoryData
// note that pBuffer has to be kTUSSlotDataSize and accessorData has to be kTUSAccessoryDataSize
int BP_TUS_Consume_GetSlotData( unsigned long long workItem, STransfarringError *pError, void *pBuffer, void *accessoryData );

// Waits for SetSlotData to complete
int BP_TUS_Consume_SetSlotData( unsigned long long workItem, STransfarringError *pError );

// Waits for GetSlotInfos to complete
int BP_TUS_Consume_GetSlotInfos( unsigned long long workItem, STransfarringError *pError, void *pSlotInfosOut, unsigned char *pValidSlots );

// Waits for IncrementSlotVariable to complete
int BP_TUS_Consume_IncrementSlotVariable( unsigned long long workItem, STransfarringError *pError, unsigned long long *pOptionalOutOldValue, unsigned long long *pOutNewValue );

// Waits for DeleteSlotData to complete
int BP_TUS_Consume_DeleteSlotData( unsigned long long workItem, STransfarringError *pError );

#ifdef __cplusplus
}
#endif
