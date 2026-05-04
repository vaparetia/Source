//----------------------------------------------------------------------------
// CTimestamp.cpp
// Bluepoint
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include <stdio.h>
#include "CTimestamp.h"

//----------------------------------------------------------------------------

#if BPE_TARGET == BPE_TARGET_PS3
#include "cell/cell_fs.h"
#include "time.h"
#elif BPE_TARGET == BPE_TARGET_RVL
#include "time.h"
#elif BPE_TARGET==BPE_TARGET_WIN32
#include <windows.h>
#elif BPE_TARGET==BPE_TARGET_X360
#include <xtl.h>
#endif

//----------------------------------------------------------------------------

CTimestamp CTimestamp::Null()
{
   return CTimestamp(0, 0, 0, 0, 0, 0);
}

//----------------------------------------------------------------------------

const CTimestamp CTimestamp::GetCurTime()
{
#if BPE_TARGET==BPE_TARGET_PS3 || BPE_TARGET==BPE_TARGET_RVL || BPE_TARGET==BPE_TARGET_VITA
   BPE_VERIFYA(false, "not implemented");
#else
   SYSTEMTIME currentTime;
   GetLocalTime( &currentTime );

   return CTimestamp( currentTime.wDay,
                      currentTime.wMonth,
                      currentTime.wYear,
                      currentTime.wHour,
                      currentTime.wMinute,
                      currentTime.wSecond );
#endif
}

//----------------------------------------------------------------------------

const CTimestamp CTimestamp::FromFile(std::string const & filename)
{

#if BPE_TARGET==BPE_TARGET_PS3
   CellFsStat stat = { 0 };

   if( cellFsStat(filename.c_str(), &stat) == CELL_FS_SUCCEEDED )
   {
      tm* pTime = localtime(&stat.st_mtime);
      return CTimestamp(pTime->tm_mday, pTime->tm_mon + 1, pTime->tm_year + 1900, pTime->tm_hour, pTime->tm_min, pTime->tm_sec);
   }

   return CTimestamp(0, 0, 0, 0, 0, 0);
#elif BPE_TARGET==BPE_TARGET_VITA
   return CTimestamp( 0, 0, 0, 0, 0, 0 );
#elif BPE_TARGET==BPE_TARGET_RVL
   return CTimestamp(0, 0, 0, 0, 0, 0);
#elif BPE_TARGET==BPE_TARGET_X360
   WIN32_FIND_DATA FindData;
   HANDLE hFind = FindFirstFile( filename.c_str(), &FindData );
   if (hFind != INVALID_HANDLE_VALUE )
   {
      FindClose( hFind );

      FILETIME LocalFileTime;
      FileTimeToLocalFileTime( &FindData.ftLastWriteTime, &LocalFileTime );

      SYSTEMTIME LocalSysTime;
      FileTimeToSystemTime( &LocalFileTime, &LocalSysTime );

      return CTimestamp(LocalSysTime.wDay, LocalSysTime.wMonth + 1, LocalSysTime.wYear + 1900, LocalSysTime.wHour, LocalSysTime.wMinute, LocalSysTime.wSecond);
   }
   return CTimestamp(0, 0, 0, 0, 0, 0);
#else
   HANDLE file = CreateFile( filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

   SYSTEMTIME fileSystemTimeLocal = { 0 };

   if( file != INVALID_HANDLE_VALUE )
   {

      FILETIME creation;
      FILETIME lastAccess;
      FILETIME lastWrite;
      
      GetFileTime(file, &creation, &lastAccess, &lastWrite);

      CloseHandle(file);

      SYSTEMTIME fileSystemTimeGlobal;
      FileTimeToSystemTime(&lastWrite, &fileSystemTimeGlobal);
      SystemTimeToTzSpecificLocalTime(NULL, &fileSystemTimeGlobal, &fileSystemTimeLocal);
   }

   return CTimestamp(fileSystemTimeLocal.wDay,
                     fileSystemTimeLocal.wMonth,
                     fileSystemTimeLocal.wYear,
                     fileSystemTimeLocal.wHour,
                     fileSystemTimeLocal.wMinute,
                     fileSystemTimeLocal.wSecond);
#endif
}

//----------------------------------------------------------------------------
// From documentation.
//
// A file time is a 64-bit value that represents the number of 100-nanosecond intervals that have elapsed since 12:00 A.M. January 1, 1601 (UTC). 
// The system records file times whenever applications create, access, and write to files. FAT records file times in local time. 
// NTFS records file times natively in FILETIME format, so they are not affected by changes in time zone or daylight saving time. 

uint64 CTimestamp::AsSeconds() const
{
   uint64 seconds = 0;

#if BPE_TARGET == BPE_TARGET_PS3 || BPE_TARGET==BPE_TARGET_VITA
   tm timeStruct = { 0 };

   timeStruct.tm_mday = mDay;
   timeStruct.tm_mon = (mMonth - 1);
   timeStruct.tm_year = (mYear - 1900);
   timeStruct.tm_hour = mHours;
   timeStruct.tm_min = mMinutes;
   timeStruct.tm_sec = mSeconds;

   // mktime returns calender time of passed in time struct
   seconds = mktime(&timeStruct);
#elif BPE_TARGET == BPE_TARGET_RVL
   tm timeStruct = { 0 };

   timeStruct.tm_mday = mDay;
   timeStruct.tm_mon = (mMonth - 1);
   timeStruct.tm_year = (mYear - 1900);
   timeStruct.tm_hour = mHours;
   timeStruct.tm_min = mMinutes;
   timeStruct.tm_sec = mSeconds;

   // mktime returns calender time of passed in time struct
   seconds = mktime(&timeStruct);
#else
   SYSTEMTIME time;
   memset(&time, 0, sizeof(time));
   time.wYear = mYear;
   time.wMonth = mMonth;
   time.wDay = mDay;
   time.wHour = mHours;
   time.wMinute = mMinutes;
   time.wSecond = mSeconds;

   FILETIME fileTime;
   SystemTimeToFileTime(&time, &fileTime);
   {
      ULARGE_INTEGER fileTimeULI;
      fileTimeULI.LowPart = fileTime.dwLowDateTime;
      fileTimeULI.HighPart = fileTime.dwHighDateTime;

      seconds = fileTimeULI.QuadPart / (10 * 1000 * 1000);   // 100ns (10e-7)
   }
#endif

   // Seconds in local time.
   return seconds;
}

//----------------------------------------------------------------------------

std::string CTimestamp::AsString() const
{
   char temp[1024];
   sprintf( temp, "%2.2d/%2.2d/%4.4d-%2.2d:%2.2d:%2.2d", mMonth, mDay, mYear, mHours, mMinutes, mSeconds );
   BPE_ASSERT(strlen(temp) < sizeof(temp), "Buffer overflow.");
   return temp; 
}

//----------------------------------------------------------------------------

const CTimestamp CTimestamp::FromString(std::string const& timestampString)
{
   int vals[6];
   sscanf(timestampString.c_str(), "%d/%d/%d-%d:%d:%d", &vals[1], &vals[0], &vals[2], &vals[3], &vals[4], &vals[5] );
   return CTimestamp( vals[0], vals[1], vals[2], vals[3], vals[4], vals[5] );
}

//----------------------------------------------------------------------------
