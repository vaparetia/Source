//----------------------------------------------------------------------------
// ShaderCooker_X360.h
// Bluepoint
// Copyright 2011
//----------------------------------------------------------------------------
#include "stdafx.h"
#include "AssetToolMPP.h"
#include "ShaderCompilerMPP.h"

//----------------------------------------------------------------------------

#include "Engine/Basics/CStringExtras.h"
#include "Engine/Mechanics/CCRC.h"

#include "Engine/Streams/CDiskInputStream.h"
#include "Engine/Streams/CDiskOutputStream.h"

#include "Engine/System/CStopWatch.h"
#include "Engine/System/CGuid.h"

#include "Tools/Helper/Helper.h"

#include "Boost/foreach.hpp"
#include "Boost/scoped_ptr.hpp"

#include "Engine/Mechanics/TTokenSet.h"
#include "ExtLibraries/LuaWrapper/LuaWrap.h"
#include "Renderer/Base/Material/X360/X360CookedShaderFormat.h"

#include <windows.h>

namespace ShaderCooker
{

bool WriteShaderX360(System::String ^ inputPath, System::String ^ outputPath, Tools::AssetSystem::Manager ^ assetManager, Tools::AssetSystem::PlatformType::EPlatform const platform, bool compileDebugShaders)
{
   typedef bool (CALLBACK* LPFNDLL_CREATESHADERCOMBINATION)(unsigned int, std::string const &, Effect_360 &, std::vector<std::string> const &, bool, std::string &);
   HINSTANCE hDLL;               // Handle to DLL
   LPFNDLL_CREATESHADERCOMBINATION lpfnDll_CreateShaderCombination;    // Function pointer

   hDLL = LoadLibrary("AssetToolsX360.dll");
   if (hDLL == NULL)
   {
      return true;
   }
   lpfnDll_CreateShaderCombination = (LPFNDLL_CREATESHADERCOMBINATION)GetProcAddress(hDLL,
      "CreateShaderCombination");
   if (!lpfnDll_CreateShaderCombination)
   {
      // handle the error
      FreeLibrary(hDLL);       
      return false;
   }

   boost::scoped_ptr<LuaWrap> luaWrap(new LuaWrap());

   System::Collections::Generic::List<Tools::AssetSystem::BuildAsset ^> ^buildAssets = gcnew System::Collections::Generic::List<Tools::AssetSystem::BuildAsset ^>();
   parse_include_files_for_build_assets( inputPath, assetManager, buildAssets );

   try
   {
      std::string const inputPathStr = Helper::StringHelper::ConvertString(inputPath);
      std::string const outputPathStr = Helper::StringHelper::ConvertString(outputPath);

      std::vector<SDefine> compileSettings;
      std::vector<std::string> skipQueries;

      GetCompileSettings(assetManager, buildAssets, inputPathStr, compileSettings, skipQueries);

      uint32 const combinationCount = bpe::max_val( 1UL, GetCombinationCount(compileSettings) );

      Effect_360 effect;

      effect.mCombinations.reserve(combinationCount);

      real32 cgfxCompileTime = 0.0f;
      real32 processingTime = 0.0f;

      for( uint32 combination = 0; combination < combinationCount; ++combination )
      {
         // build list of combination define pairs (name=value)
         std::string const combinationDefines = GetCombinationDefines(compileSettings, combination);

         // determine if this combination should be skipped
         bool const shouldSkipCombination = CheckIfSkipCombination(*luaWrap.get(), combinationDefines.c_str(), skipQueries);

         // print information about combination
         uint32 const definesCRC = CCRC::CalculateCRC32AsString(combinationDefines.c_str());
         System::Console::WriteLine("{0} {1} / {2} ({3}) [0x{4:x00000000}]", shouldSkipCombination ? "Skipping " : "Compiling", combination + 1, combinationCount, gcnew System::String(combinationDefines.c_str()), definesCRC);

         // skip if this combination isn't necessary.
         if( shouldSkipCombination )
            continue;        

         // add compile options for combination specific defines
         std::vector<std::string> defineStrings;
         CStringExtras::Tokenize(combinationDefines.c_str(), defineStrings, "=;");
         std::string errorStrings;
         if( !lpfnDll_CreateShaderCombination(definesCRC, inputPathStr, effect, defineStrings, compileDebugShaders, errorStrings) )
         {
            System::Console::WriteLine("Compile error: {0}", gcnew System::String(errorStrings.c_str()));
            return false;
         }
      }
         
      CDiskOutputStream outStream(outputPathStr.c_str());

      Tools::AssetSystem::BuildAssets::WriteAssets( inputPath, buildAssets, assetManager, Tools::AssetSystem::PlatformType::EPlatform::kX360 );

      effect.PutTo(outStream);

      //System::Console::WriteLine("Total: {0}s Parsing time: {1}s Processing time: {2}s Compilation: {3}s", totalWatch.GetElapsedTime(), cgfxCompileTime, processingTime, timeSpendWaiting);
      return true;
   }
   finally
   {
      FreeLibrary(hDLL);
   }
}

}
