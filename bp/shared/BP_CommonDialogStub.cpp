//----------------------------------------------------------------------------
// BP_CommonDialogStub.cpp
//----------------------------------------------------------------------------

#include "BP_CommonDialog.h"

extern "C" unsigned long long BP_CommonDialog_WantsMessageDialog( char const * const pMessageString, EMessageDialogLayout const layout )
{
   return 0ULL;
}

extern "C" unsigned long long BP_CommonDialog_WantsErrorDialog( int const errorCode )
{
   return 0ULL;
}

extern "C" int BP_CommonDialog_IsStillProcessing( unsigned long long const handle )
{
   return 0;
}

extern "C" void BP_CommonDialog_Heartbeat()
{

}

extern "C" ECommonDialogResult BP_CommonDialog_GetResult( unsigned long long const handle )
{
   return kCDR_Invalid;
}


