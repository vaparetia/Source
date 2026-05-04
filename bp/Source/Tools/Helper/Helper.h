// Helper.h

#pragma once

#include "string"

namespace Helper
{
   class StringHelper
   {
   public:
      static std::string const ConvertString( System::String^ value )
      {
         System::IntPtr ptr = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(value);
         
         char const* str = (char const*)(void*) ptr;

         std::string ret( str );

         System::Runtime::InteropServices::Marshal::FreeHGlobal( ptr );

         return ret;
      }

      static std::wstring const ConvertStringUnicode( System::String^ value )
      {
         System::IntPtr ptr = System::Runtime::InteropServices::Marshal::StringToHGlobalUni(value);

         wchar_t const* str = (wchar_t const*)(void*) ptr;

         std::wstring ret( str );

         System::Runtime::InteropServices::Marshal::FreeHGlobal( ptr );

         return ret;
      }

   };
}
