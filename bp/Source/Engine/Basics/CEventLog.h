//------------------------------------------------------------------------------------------
// CEventLog.h
// Copyright 2004
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "list"

#include "Engine/System/CTimestamp.h"

//------------------------------------------------------------------------------------------

class CEvent
{
public:
   enum EType
   {
      kType_Info,
      kType_Warning,
      kType_Error,
      kType_System,

      kType_Invalid = -1
   };

public:
   ENGINE_API explicit CEvent( EType const type,
                               int const fourCC,
                               CTimestamp const & timeStamp,
                               std::string const & text );

   EType const GetType() const { return mType; }
   int const GetFourCC() const { return mFourCC; }
   
   CTimestamp const & GetTimeStamp() const { return mTimeStamp; }

   std::string const & GetText() const { return mText; }

   ENGINE_API std::string const GetAsDisplayableText() const;
   
private:
   EType       mType;
   int         mFourCC;
   CTimestamp  mTimeStamp;
   std::string mText;
};

//------------------------------------------------------------------------------------------

class CEventLog
{
public:
   ENGINE_API explicit CEventLog( std::string const & outputFilename );
   ENGINE_API ~CEventLog();

   ENGINE_API void Update( real32 const deltaTime );
   
   ENGINE_API void Error( const char * const pFormat, ... );
   ENGINE_API void Warning( const char * const pFormat, ... );
   
   ENGINE_API void Info( int const fourCC,
                         const char * const pFormat, ... );


   ENGINE_API static CEventLog * const GetEventLog();

protected:
   void AddEvent( CEvent const & event );
   void FlushEvents();

private:
   BPE_HANDLE        mOutput;
   std::list<CEvent> mEvents;
   real32            mTimeSinceLastEvent;
};

//------------------------------------------------------------------------------------------

static inline CEventLog & EventLog()
{
   CEventLog * const pEventLog = CEventLog::GetEventLog();
   BPE_VERIFY( pEventLog != NULL, false, "Event log uninitialized" );
   return *pEventLog;
}
