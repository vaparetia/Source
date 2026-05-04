//------------------------------------------------------------------------------------------
// CSystemVar.cpp
// Bluepoint
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/System/CSystemVar.h"
#include "Engine/System/CFileUtils.h"

#if BPE_TARGET == BPE_TARGET_WIN32
#include <windows.h>
#endif

//------------------------------------------------------------------------------------------

CSystemVar::CSystemVar( const std::string& var, const CSystemVar::EExpandBehaviour behaviour )
: mbIsValid(false)
{
#if BPE_TARGET == BPE_TARGET_WIN32
   enum { kBufferSize = 1024 };
   char buffer[kBufferSize];

   if( GetEnvironmentVariable( var.c_str(), buffer, kBufferSize ) )
   {
      this->mValue = buffer;
      mbIsValid = true;
      if( behaviour == kExpandBehaviour_ExpandAll )
      {
         Expand();
      }
   }
   else
   {
      mbIsValid = false;
   }
#else

   BPE_VERIFYA(false, "Only supported on Windows.");

#endif
}

//------------------------------------------------------------------------------------------

CSystemVar::CSystemVar( const std::string& var, const std::string& defaultValue, const CSystemVar::EExpandBehaviour behaviour )
: mbIsValid(false)
{
#if BPE_TARGET == BPE_TARGET_WIN32
   enum { kBufferSize = 1024 };
   char buffer[kBufferSize];

   if( GetEnvironmentVariable( var.c_str(), buffer, kBufferSize ) )
   {
      this->mValue = buffer;
      mbIsValid = true;
   }
   else
   {
      this->mValue = defaultValue;
      mbIsValid = true;

      SetEnvironmentVariable( var.c_str(), defaultValue.c_str() );
   }

   if( behaviour == kExpandBehaviour_ExpandAll )
   {
      Expand();
   }
#else
   BPE_VERIFYA(false, "Not supported");
#endif
}

//------------------------------------------------------------------------------------------

void CSystemVar::Expand()
{
#if BPE_TARGET == BPE_TARGET_WIN32
   enum { kBufferSize = 1024 };
   char buffer[kBufferSize];

   if( ExpandEnvironmentStrings( mValue.c_str(), buffer, kBufferSize ) < kBufferSize )
   {
      mValue = buffer;
   }
#else
   BPE_VERIFYA(false, "Not supported");
#endif
}

//------------------------------------------------------------------------------------------

void CSystemVar::SetRegString(char const * const pPath, char const * const pName, char const * const pValue)
{
#if BPE_TARGET == BPE_TARGET_WIN32
   HKEY  hKey;

   if (ERROR_SUCCESS == RegCreateKeyEx( HKEY_CURRENT_USER, pPath, 0l, NULL, REG_OPTION_NON_VOLATILE, NULL, NULL, &hKey, NULL ))
   {
      RegCloseKey( hKey );
   }

   if (ERROR_SUCCESS == RegOpenKeyExA( HKEY_CURRENT_USER, pPath, 0, KEY_ALL_ACCESS, &hKey ))
   {
      RegDeleteValue( hKey, pName );
      RegSetValueExA( hKey, pName, 0, REG_SZ, (BYTE*) pValue, strlen(pValue) );         
      RegCloseKey(hKey);
   }
#else
   BPE_VERIFYA(false, "Not supported");
#endif
}

//------------------------------------------------------------------------------------------

std::string CSystemVar::GetRegString(char const * const pPath, char const * const pName)
{
   std::string out;
#if BPE_TARGET == BPE_TARGET_WIN32
   // Open the appropriate registry key
   HKEY  hKey;
   LONG lResult = RegOpenKeyEx( HKEY_CURRENT_USER,
                               pPath,
                               0, KEY_READ, &hKey );
   if( ERROR_SUCCESS != lResult )
   {
      return std::string();
   }

   enum { kBufferSize = 1024 };
   char buffer[kBufferSize + 1];

   DWORD dwType;
   lResult = ERROR_MORE_DATA;
   while (lResult == ERROR_MORE_DATA)  // read docs on RegQueryValueEx, do this so we can store any size string
   {
      DWORD dwSize = kBufferSize;
      lResult = RegQueryValueEx( hKey, pName, NULL,
                              &dwType, (BYTE*)buffer, &dwSize );
      buffer[kBufferSize] = 0;// RegQueryValueEx doesn't NULL term if buffer too small
      out += buffer;
   }

   RegCloseKey( hKey );
#else
   BPE_VERIFYA(false, "Not supported");
#endif
   return out;
}

//------------------------------------------------------------------------------------------

std::string CSystemVar::GetAppRegistryPath()
{
   std::string appPath(GetRootRegistryPath());

#if BPE_TARGET == BPE_TARGET_WIN32
   enum { kBufferSize = 1024 };
   char buffer[kBufferSize + 1];

   GetModuleFileName(NULL, buffer, kBufferSize);

   appPath += CFileUtils::GetFileName(buffer);
#else
   BPE_VERIFYA(false, "Not supported");
#endif

   return appPath;
}

//------------------------------------------------------------------------------------------

char const * const CSystemVar::GetRootRegistryPath()
{
   char const * const projectXPath = "Software\\BluepointEngine\\";
   return projectXPath;
}

//------------------------------------------------------------------------------------------
