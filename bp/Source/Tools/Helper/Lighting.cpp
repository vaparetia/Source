//----------------------------------------------------------------------------
// Lighting.cpp
//----------------------------------------------------------------------------

#include "stdafx.h"
#include "Lighting.h"

//----------------------------------------------------------------------------

System::String^ Helper::Lighting::GetBaseOutputPath(System::String^ inputScriptingPath)
{
   System::String^ baseOutputPath = System::IO::Path::Combine(System::IO::Path::GetDirectoryName(inputScriptingPath), 
                                                              System::IO::Path::GetFileNameWithoutExtension(inputScriptingPath) + "_lighting");

   return baseOutputPath;
}

//----------------------------------------------------------------------------

System::String^ Helper::Lighting::GetLitModelOutputPath(System::String^ baseOutputPath, 
                                                        System::String^ inputModelPath, 
                                                        System::Guid^ uniqueModelId)
{
   System::String^ baseName = System::IO::Path::GetFileNameWithoutExtension(inputModelPath);

   // Use unique directories with guids for each model
   System::String^ outputPath = System::IO::Path::Combine(baseOutputPath, baseName + "_" + uniqueModelId->ToString());

   return outputPath;
}

//----------------------------------------------------------------------------

System::String^ Helper::Lighting::GetLitModelPath(System::String^ litModelOutputPath,
                                                  System::String^ inputModelPath)
{
   return System::IO::Path::Combine(litModelOutputPath, System::IO::Path::GetFileName(inputModelPath));
}
