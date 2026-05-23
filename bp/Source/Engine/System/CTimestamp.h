//------------------------------------------------------------------------------------------
// CTimestamp.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

#include <string>

//------------------------------------------------------------------------------------------

class CTimestamp
{
public:
   explicit CTimestamp( int day, int month, int year, int hours, int minutes, int seconds )
   :  mDay(day)
   ,  mMonth(month)
   ,  mYear(year)
   ,  mHours(hours)
   ,  mMinutes(minutes)
   ,  mSeconds(seconds)
   {
   }

   ENGINE_API uint64       AsSeconds() const;
   ENGINE_API std::string  AsString() const;

   ENGINE_API static CTimestamp Null();
   ENGINE_API static const CTimestamp GetCurTime();
   ENGINE_API static const CTimestamp FromString(std::string const& timestampString);
   ENGINE_API static const CTimestamp FromFile(std::string const & filename);

   bool operator < (const CTimestamp& other) const
   {
      return this->AsSeconds() < other.AsSeconds();
   }

   bool operator > (const CTimestamp& other) const
   {
      return this->AsSeconds() > other.AsSeconds();
   }

   bool operator == ( const CTimestamp& other) const
   {
      return this->AsSeconds() == other.AsSeconds();
   }

private:
   int   mDay;
   int   mMonth;
   int   mYear;

   int   mHours;
   int   mMinutes;
   int   mSeconds;
};

//------------------------------------------------------------------------------------------

