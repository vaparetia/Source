//----------------------------------------------------------------------------
// TextureCookerX360.cpp
// Bluepoint
// Copyright 2011
//----------------------------------------------------------------------------

#include "stdafx.h"

#include "Engine/Streams/CDiskOutputStream.h"
#include "Engine/Streams/CGrowableMemoryOutStream.h"
#include "Engine/System/CFileUtils.h"
#include "Engine/Graphics/CColor.h"
#include "Engine/Graphics/CColorf.h"
#include "TextureCooker.h"

#include "IL/il.h"
#include "IL/ilu.h"
#include "math.h"
#include "nvtt/nvtt.h"
#include "boost/scoped_ptr.hpp"

#include <windows.h>

namespace TextureCooker
{
   typedef bool (CALLBACK* LPFNDLLFUNC1)(std::string const &, CTextureToCook const &, bool);

   bool WriteTextureX360( std::string const & outputPath, CTextureToCook const & tex, bool debugInfo )
   {
      HINSTANCE hDLL;               // Handle to DLL
      LPFNDLLFUNC1 lpfnDllFunc1;    // Function pointer
      DWORD dwParam1;
      UINT  uParam2, uReturnVal;

      hDLL = LoadLibrary("AssetToolsX360.dll");
      if (hDLL != NULL)
      {
         lpfnDllFunc1 = (LPFNDLLFUNC1)GetProcAddress(hDLL,
            "WriteTextureX360");
         if (!lpfnDllFunc1)
         {
            // handle the error
            FreeLibrary(hDLL);       
            return false;
         }
         else
         {
            // call the function
            bool retValue = lpfnDllFunc1(outputPath, tex, debugInfo);
            FreeLibrary(hDLL);
            return retValue;
         }
      }

      return true;
   }
}
