//----------------------------------------------------------------------------

#include "stdafx.h"
#include "AssetToolMPP.h"
#include "ShaderCompilerMPP.h"

//----------------------------------------------------------------------------

#include "Engine/Basics/CStringExtras.h"
#include "Tools/Helper/Helper.h"

//----------------------------------------------------------------------------

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Xml;
using namespace System::Xml::XPath;
using namespace System::Diagnostics;

//----------------------------------------------------------------------------

bool AssetToolMPP::Cookers::CookEFCT( System::String^ inputPath, System::String^ outputPath, Tools::AssetSystem::Manager^ assetManager, Tools::AssetSystem::PlatformType::EPlatform const platform, bool compileDebugShaders )
{
   switch(platform)
   {
   case Tools::AssetSystem::PlatformType::EPlatform::kPS3:
      if( !ShaderCooker::WriteShaderPS3(inputPath, outputPath, assetManager, platform) )
         return false;
      break;

   case Tools::AssetSystem::PlatformType::EPlatform::kVita:
      if( !ShaderCooker::WriteShaderVTA(inputPath, outputPath, assetManager, platform) )
         return false;
      break;

   case Tools::AssetSystem::PlatformType::EPlatform::kX360:
      if( !ShaderCooker::WriteShaderX360(inputPath, outputPath, assetManager, platform, compileDebugShaders) )
         return false;
      break;

   case Tools::AssetSystem::PlatformType::EPlatform::kWin32:
      return true;

   default:
      Console::WriteLine( "CookEFCT does not know how to cook {0}", platform );
      return false;
   }
   return true;
}

namespace ShaderCooker
{
void GetDefineSettings(String^ inputPath, std::vector<SDefine> & compileSettings, std::vector<std::string> &commonDefines, std::vector<std::string> & skipQueries)
{
   XmlDocument^ effectSettings = gcnew XmlDocument();

   if( System::IO::File::Exists(inputPath) )
   {
      effectSettings->Load(inputPath);

      // Defines
      {
         XmlNodeList^ nodes = effectSettings->SelectNodes("/Settings/Define");
         for( int i = 0; i < nodes->Count; ++i )
         {
            XmlNode^ node = nodes[i];
            XmlAttribute^ name = node->Attributes["name"];
            XmlAttribute^ min = node->Attributes["min"];
            XmlAttribute^ max = node->Attributes["max"];
            XmlAttribute^ bitPos = node->Attributes["bitPos"];

            if( name && min && max && bitPos)
            {
               compileSettings.push_back(SDefine(Helper::StringHelper::ConvertString(name->Value), 
                                                 atoi(Helper::StringHelper::ConvertString(min->Value).c_str()),
                                                 atoi(Helper::StringHelper::ConvertString(max->Value).c_str()),
                                                 atoi(Helper::StringHelper::ConvertString(bitPos->Value).c_str())));
            }
            else
            {
               bpe_console_printf("Missing attribute for %s\n", Helper::StringHelper::ConvertString(node->InnerXml).c_str());
            }
         }
      }

      // Common defines
      {
         XmlNodeList^ nodes = effectSettings->SelectNodes("/Settings/CommonDefine");
         for( int i = 0; i < nodes->Count; ++i )
         {
            XmlNode^ node = nodes[i];
            XmlAttribute^ name = node->Attributes["name"];

            if (name)
            {
               commonDefines.push_back(Helper::StringHelper::ConvertString(name->Value)); 
            }
            else
            {
               bpe_console_printf("Missing attribute for %s\n", Helper::StringHelper::ConvertString(node->InnerXml).c_str());
            }
         }
      }

      // Skip queries
      {
         XmlNodeList^ nodes = effectSettings->SelectNodes("/Settings/Skip");
         for( int i = 0; i < nodes->Count; ++i )
         {
            XmlNode^ node = nodes[i];
            skipQueries.push_back(Helper::StringHelper::ConvertString(node->InnerText)); 
         }
      }

   }
}

//----------------------------------------------------------------------------

void GetCompileSettings(Tools::AssetSystem::Manager^ assetManager, List<Tools::AssetSystem::BuildAsset ^> ^buildAssets, std::string const & inputPathStr, std::vector<SDefine> & compileSettings, std::vector<std::string> & skipQueries )
{
   String ^ inputPath = gcnew String(inputPathStr.c_str());

   List<String^> ^settingsPaths = gcnew List<String^>();

   // Get common defines.
   std::vector<SDefine> commonCompileSettings;
   {
      std::vector<std::string> commonDefines;

      String ^ commonPath = System::IO::Path::GetDirectoryName(inputPath) + "\\CommonDefines" + System::IO::Path::GetExtension(inputPath) + "-settings";

      settingsPaths->Add( commonPath );

      GetDefineSettings(commonPath, commonCompileSettings, commonDefines, skipQueries);
   }

   // Get shader defines
   std::vector<SDefine> shaderCompileSettings;
   std::vector<std::string> shaderCommonDefines;
   String^ path = inputPath + "-settings";
   GetDefineSettings(path, shaderCompileSettings, shaderCommonDefines, skipQueries);

   settingsPaths->Add( path );

   // Build compile settings from shader and common defines
   {
      compileSettings = shaderCompileSettings;
      BOOST_FOREACH(SDefine const &define, commonCompileSettings)
      {
         if (std::find(shaderCommonDefines.begin(), shaderCommonDefines.end(), define.mName) != shaderCommonDefines.end())
         {
            SDefine commonDefine(define);
            commonDefine.mBitPos += 32;   // Offset bit pos to mark as common
            compileSettings.push_back(commonDefine);
         }
      }
   }

   // Add build dependencies
   for each ( String ^path in settingsPaths )
   {
      String ^assetPath = assetManager->GetRepositoryRelativePath( path );
      Tools::AssetSystem::BuildAsset::ERecookOptions recookOptions = Tools::AssetSystem::BuildAsset::ERecookOptions::kRO_Newer;

      buildAssets->Add( Tools::AssetSystem::BuildAsset::RepositoryAsset( "Settings", assetPath, recookOptions ) );
   }
}

//----------------------------------------------------------------------------

uint32 GetCombinationCount(std::vector<SDefine> const & defines)
{
   uint32 combinations = 1;
   
   for( int i = 0; i < defines.size(); ++i )
   {
      uint32 defineRange = (defines[i].mMax - defines[i].mMin) + 1;
    
      if( defineRange > 0 )
         combinations *= defineRange;
   }

   return combinations;
}

//----------------------------------------------------------------------------

std::string const GetCombinationDefines(std::vector<SDefine> const & defines, uint32 const combination)
{
   std::string definesString;

   uint32 combinationKey = combination;
   
   for(int i = 0; i < defines.size(); ++i )
   {
      if( !definesString.empty() )
         definesString.append( ";" );

      uint32 const kDefineCombinationCount = (defines[i].mMax - defines[i].mMin) + 1;

      uint32 currentDefineValue = defines[i].mMin + (combinationKey % kDefineCombinationCount);
      combinationKey /= kDefineCombinationCount;

      definesString.append( CStringExtras::Stringize("%s=%d", defines[i].mName.c_str(), currentDefineValue) );
   }

   return definesString;
}

//----------------------------------------------------------------------------

bool const CheckIfSkipCombination(LuaWrap & luaWrap, char const *combinationDefines, std::vector<std::string> const & skipQueries)
{
   std::vector<std::string> splitDefines;
   CStringExtras::Tokenize(combinationDefines, splitDefines, ";");

   // go over all defines and assign them to lua variables
   for( int i = 0; i < splitDefines.size(); ++i )
   {
      lua_dostring( luaWrap, splitDefines[i].c_str() );
   }

   // now execute queries that use those lua variables to determine if this combination should be skipped
   for( int i = 0; i < skipQueries.size(); ++i )
   {
      std::string query = std::string("return (") + skipQueries[i] + ")";

      lua_State * L = luaWrap;
      
      int const oldTop = lua_gettop(L);

      lua_dostring( L, query.c_str() );
      int newTop = lua_gettop(L);

      // Get return value
      int bResult = 0;
      for (int loop = oldTop; loop < newTop; loop++)
      {
         bResult = lua_toboolean(L, -1);
         lua_pop(L, 1);
      }

      if (bResult)
      {
         return true;
      }
   }

   return false;
}

//----------------------------------------------------------------------------

IEnumerable<String ^> ^scan_for_includes( String ^rootFolder, String ^itemPath )
{
   List< String ^> ^ret = gcnew List<String^>();
   array< Char > ^whitespace = gcnew array<Char> { ' ', '\t' };

   String ^itemFolder = System::IO::Path::GetDirectoryName( itemPath );

   System::IO::StreamReader reader( itemPath, System::Text::Encoding::ASCII );
   while ( !reader.EndOfStream )
   {
      String ^line = reader.ReadLine();

      // We are looking for #include "file"
      // Note that it's really <optional whitespace>#<optional whitespace>include<whitespace>"file"

      line = line->TrimStart( whitespace );

      if ( !line->StartsWith( "#" ) )
      {
         continue;
      }

      String ^includeString = "include";
      line = line->Substring( 1 )->TrimStart( whitespace );
      if ( !line->StartsWith( includeString )  )
      {
         continue;
      }

      // Now we trim both sides of whitespace....
      line = line->Substring( includeString->Length )->Trim( whitespace );

      // If we have a valid include string, it should start and end with quotes
      if ( line->Length > 2 && line->StartsWith( "\"" ) && line->EndsWith( "\"" ) )
      {
         String ^filePart = line->Substring( 1, line->Length - 2 );

         try
         {
            ret->Add( System::IO::Path::Combine( rootFolder, filePart ) );
         }
         catch ( System::IO::IOException ^ )
         {
         }

         try
         {
            ret->Add( System::IO::Path::Combine( itemFolder, filePart ) );
         }
         catch ( System::IO::IOException ^ )
         {
         }
      }
   }

   return ret;
}

//----------------------------------------------------------------------------

void parse_include_files_for_build_assets( System::String ^inputPath, Tools::AssetSystem::Manager ^assetManager, List< Tools::AssetSystem::BuildAsset ^ > ^buildAssets )
{
   List<String ^> ^processed = gcnew List<String^>();
   Queue<String ^> ^processing = gcnew Queue<String^>();
   bool isFirstAsset = true;

   // This adds build assets by running through all sub-include files in headers for pixel shaders
   // It is totally dumb and looks for #include tags
   // It has to look both in the path of the ".fxh" file and of the main file, so each include actually adds
   // two paths.  It only adds dependencies for files that actually exist

   String ^rootFolder = System::IO::Path::GetDirectoryName( inputPath );

   processing->Enqueue( inputPath );

   while ( processing->Count > 0 )
   {
      String ^path = Tools::Common::FileUtils::NormalizeFileName( processing->Dequeue() );

      if ( !processed->Contains( path ) )
      {
         if ( 
            assetManager->IsSystemPathInsideRepository( path ) && 
            Tools::AssetSystem::AssetTypes::GetAssetTypeForSourceAsset( path ) != nullptr &&
            System::IO::File::Exists( path ) )
         {
            // We've got a path that would be a valid build dependency
            
            // Don't add the source file as a build dependency
            if ( isFirstAsset )
            {
               isFirstAsset = false;
            }
            else
            {
               buildAssets->Add( Tools::AssetSystem::BuildAsset::StandardCookingAsset( "Include", assetManager->GetRepositoryRelativePath( path ) ) );
            }

            for each ( String ^newPath in scan_for_includes( rootFolder, path ) )
            {
               processing->Enqueue( newPath );
            }
         }

         processed->Add( path );
      }
   }
   
   String ^normalizedPath = Tools::Common::FileUtils::NormalizeFileName( inputPath );
}

}

