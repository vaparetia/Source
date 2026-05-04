//----------------------------------------------------------------------------
// ShaderCompilerMPP.h
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#pragma once

#include "ExtLibraries/LuaWrapper/LuaWrap.h"

namespace ShaderCooker
{
   void parse_include_files_for_build_assets( System::String ^inputPath, Tools::AssetSystem::Manager ^assetManager, System::Collections::Generic::List< Tools::AssetSystem::BuildAsset ^ > ^buildAssets );
   struct SDefine
   {
      SDefine(std::string const & name, int min, int max, int bitPos)
         :  mName(name)
         ,  mMin(min)
         ,  mMax(max)
         ,  mBitPos(bitPos)
      {
      }

      std::string mName;
      int         mMin;
      int         mMax;
      int         mBitPos;
   };
   void GetDefineSettings(System::String ^ inputPath, std::vector<SDefine> & compileSettings, std::vector<std::string> & commonDefines, std::vector<std::string> & skipQueries);
   void GetCompileSettings(Tools::AssetSystem::Manager ^ assetManager, System::Collections::Generic::List<Tools::AssetSystem::BuildAsset ^> ^ buildAssets, std::string const & inputPathStr, std::vector<SDefine> & compileSettings, std::vector<std::string> & skipQueries);
   uint32 GetCombinationCount(std::vector<SDefine> const & defines);
   std::string const GetCombinationDefines(std::vector<SDefine> const & defines, uint32 const combination);
   bool const CheckIfSkipCombination(LuaWrap & luaWrap, char const * combinationDefines, std::vector<std::string> const & skipQueries);

   bool WriteShaderPS3(System::String ^ inputPath, System::String ^ outputPath, Tools::AssetSystem::Manager ^ assetManager, Tools::AssetSystem::PlatformType::EPlatform const platform);
   bool WriteShaderVTA(System::String ^ inputPath, System::String ^ outputPath, Tools::AssetSystem::Manager ^ assetManager, Tools::AssetSystem::PlatformType::EPlatform const platform);
   bool WriteShaderX360(System::String ^ inputPath, System::String ^ outputPath, Tools::AssetSystem::Manager ^ assetManager, Tools::AssetSystem::PlatformType::EPlatform const platform, bool compileDebugShaders);
}

//----------------------------------------------------------------------------

