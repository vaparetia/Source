//----------------------------------------------------------------------------
// Lighting.h
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

namespace Helper
{
   public ref class Lighting
   {
   public:
      // the returns the base path for the lit models for the given scripting
      // used to get the output path for each lit model or lit model paths directly
      static System::String^ GetBaseOutputPath(System::String^ inputScriptingPath);

      // This function returns the repository 
      static System::String^ GetLitModelOutputPath(System::String^ baseOutputPath, 
                                                   System::String^ inputModelPath, 
                                                   System::Guid^ uniqueModelId);

      static System::String^ GetLitModelPath(System::String^ litModelOutputPath,
                                             System::String^ inputModelPath);
   };
}
