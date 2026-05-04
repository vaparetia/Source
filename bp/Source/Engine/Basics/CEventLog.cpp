//------------------------------------------------------------------------------------------
// CEventLog.cpp
// Copyright 2004
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "CEventLog.h"
#if BPE_TARGET == BPE_TARGET_WIN32
#include <windows.h>
#endif

//------------------------------------------------------------------------------------------

#include "Engine/Basics/CStringExtras.h"

//------------------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------------------

CEventLog sEventLog( "EventLog.txt" );

//------------------------------------------------------------------------------------------

CEvent::CEvent( EType const type, 
                int const fourCC,
                CTimestamp const & timeStamp, 
                std::string const & text )
:  mType( type )
,  mFourCC( fourCC )
,  mTimeStamp( timeStamp )
,  mText( text )
{
}

//------------------------------------------------------------------------------------------

std::string const CEvent::GetAsDisplayableText() const
{
   string const timeStamp = GetTimeStamp().AsString();
   
   string typeStr = "UNKNOWN";

   switch( GetType() )
   {
   case kType_Info:
      typeStr = "INFO:" + CStringExtras::FourCCToText( mFourCC );
      break;

   case kType_Warning:
      typeStr = "WARNING";
      break;

   case kType_Error:
      typeStr = "ERROR";
      break;

   case kType_System:
      typeStr = "SYSTEM";
      break;
   }

   string const prefix = timeStamp + " " + typeStr + " ";

   string const prefixEmpty( prefix.size(), ' ' );

   string inText = GetText();
   if( !inText.empty() && inText[ inText.size() - 1 ] == '\n' )
   {
      inText.erase( inText.size() - 1 );
   }

   string outText;
   outText.reserve( inText.size() );

   for( int i = 0; i < inText.size(); ++i )
   {
      if( inText[i] == '\n' )
         outText += "\r\n" + prefixEmpty;
      else
      {
         outText += inText[i];
      }
   }
   
   outText += "\r\n";

   return prefix + outText;
}

//------------------------------------------------------------------------------------------

CEventLog::CEventLog( std::string const & outputFilename )
:  mOutput( BPE_INVALID_HANDLE_VALUE )
,  mTimeSinceLastEvent( 0.0f )
{
#if BPE_TARGET == BPE_TARGET_WIN32
   /*
   mOutput = CreateFile( outputFilename.c_str(), 
                         GENERIC_WRITE,
                         FILE_SHARE_READ,
                         NULL,
                         CREATE_ALWAYS,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL );
                         */

   AddEvent( CEvent( CEvent::kType_System, 0, CTimestamp::GetCurTime(), "Started Event Log" ) );
#endif
}

//------------------------------------------------------------------------------------------
   
CEventLog::~CEventLog()
{
   FlushEvents();

#if BPE_TARGET == BPE_TARGET_WIN32
   if( mOutput != BPE_INVALID_HANDLE_VALUE )
   {
      CloseHandle( mOutput );
   }
#endif
}

//------------------------------------------------------------------------------------------

void CEventLog::Update( real32 const deltaTime )
{
   mTimeSinceLastEvent += deltaTime;
   
   if( mTimeSinceLastEvent > 0.1f )
   {
      FlushEvents();
   }
}

//------------------------------------------------------------------------------------------

void CEventLog::AddEvent( CEvent const & event )
{
   mTimeSinceLastEvent = 0.0f;

   mEvents.push_back( event );
   
   if( mEvents.size() > 128 )
   {
      FlushEvents();
   }
}

//------------------------------------------------------------------------------------------

void CEventLog::FlushEvents()
{
#if BPE_TARGET == BPE_TARGET_WIN32
   for( list<CEvent>::const_iterator it = mEvents.begin(); it != mEvents.end(); ++it )
   {
      CEvent const & event = *it;
      string const output = event.GetAsDisplayableText();

      DWORD writtenBytes = 0;
      if( mOutput != BPE_INVALID_HANDLE_VALUE )
      {
         WriteFile( mOutput, output.c_str(), output.size(), &writtenBytes, NULL );
      }
   }

   mEvents.clear();
#endif
}

//------------------------------------------------------------------------------------------

CEventLog * const CEventLog::GetEventLog()
{
   return &sEventLog;
}

//------------------------------------------------------------------------------------------

void CEventLog::Error( const char * const pFormat, ... )
{
#if BPE_TARGET == BPE_TARGET_WIN32
   char debuggerBuf[2049];

   va_list arg;
   va_start( arg, pFormat );
   vsprintf_s( debuggerBuf, pFormat, arg );
   va_end( arg );

   CEvent const event( CEvent::kType_Error,
                       0,
                       CTimestamp::GetCurTime(),
                       debuggerBuf );
   
   bpe_debugger_printf( event.GetAsDisplayableText().c_str() );

   AddEvent( event );
#else
   va_list arg;
   va_start( arg, pFormat );
   bpe_debugger_printf_va_list(pFormat, arg);
   va_end(arg);
#endif
}

//------------------------------------------------------------------------------------------

void CEventLog::Warning( const char * const pFormat, ... )
{
#if BPE_TARGET == BPE_TARGET_WIN32
   char debuggerBuf[2049];

   va_list arg;
   va_start( arg, pFormat );
   vsprintf_s( debuggerBuf, pFormat, arg );
   va_end( arg );

   AddEvent( CEvent( CEvent::kType_Warning,
                     0,
                     CTimestamp::GetCurTime(),
                     debuggerBuf ) );
#endif
}

//------------------------------------------------------------------------------------------

void CEventLog::Info( int const fourCC, const char * const pFormat, ... )
{
#if BPE_TARGET == BPE_TARGET_WIN32
   char debuggerBuf[2049];

   va_list arg;
   va_start( arg, pFormat );
   vsprintf_s( debuggerBuf, pFormat, arg );
   va_end( arg );

   AddEvent( CEvent( CEvent::kType_Info,
                     fourCC,
                     CTimestamp::GetCurTime(),
                     debuggerBuf ) );
#endif
}

