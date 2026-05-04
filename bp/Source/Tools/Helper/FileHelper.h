// FileHelper.h

#pragma once

namespace Helper
{
   public ref class FileHelper
   {
   public:
      static System::Collections::Generic::List<System::String^>^ MatchFiles(System::String^ rootFolder, System::Collections::Generic::List<System::String^>^ matchStrings, bool recursive, System::String^ resultRootFolder);
   };
}
