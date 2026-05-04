//----------------------------------------------------------------------------
// AssetToolMPP.h
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

namespace AssetToolMPP
{
	public ref class Cookers
   {
   public:
      static bool CookMODL( System::String^ inputPath, 
         cli::array<System::String^> ^outputPaths, cli::array<Tools::AssetSystem::PlatformType::EPlatform> ^platforms,
         Tools::AssetSystem::Manager^ assetManager, int const verboseOutputLevel );
      static bool CookEMDL( System::String^ inputPath, System::String^ outputPath, Tools::AssetSystem::Manager^ assetManager );
      static bool CookSKIN( System::String^ inputPath, System::String^ outputPath, Tools::AssetSystem::Manager^ assetManager, Tools::AssetSystem::PlatformType::EPlatform const platform, int const verboseOutputLevel );
      static bool CookCOLL( System::String^ inputPath, System::String^ outputPath, Tools::AssetSystem::Manager^ assetManager );
      static bool CookTXTR( System::String^ inputPath, System::String^ outputPath, Tools::AssetSystem::Manager^ assetManager, Tools::AssetSystem::PlatformType::EPlatform const platform, bool debugInfo );
      static bool CookANIM( System::String^ inputPath, System::String^ outputPath, Tools::AssetSystem::Manager^ assetManager, Tools::AssetSystem::PlatformType::EPlatform const platform );
      static bool CookFONT( System::String^ inputPath, System::String^ outputPath, Tools::AssetSystem::Manager^ assetManager );
      static bool CookEFCT( System::String^ inputPath, System::String^ outputPath, Tools::AssetSystem::Manager^ assetManager, Tools::AssetSystem::PlatformType::EPlatform const platform, bool compileDebugShaders );
   };
}
