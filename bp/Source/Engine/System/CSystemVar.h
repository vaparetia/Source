//------------------------------------------------------------------------------------------
// CSystemVar.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

//------------------------------------------------------------------------------------------

class CSystemVar
{
public:
   enum EExpandBehaviour
   {
      kExpandBehaviour_None,
      kExpandBehaviour_ExpandAll,
   };

   ENGINE_API explicit CSystemVar( const std::string& var, const EExpandBehaviour behaviour = kExpandBehaviour_ExpandAll );
   ENGINE_API explicit CSystemVar( const std::string& var, const std::string& defaultValue, const EExpandBehaviour behaviour = kExpandBehaviour_ExpandAll );

   const bool IsValid() const { return mbIsValid; }
   const std::string& GetValue() const { return mValue; }

   // Registry helpers
   static ENGINE_API void        SetRegString(char const * const pPath, char const * const pName, char const * const pValue);
   static ENGINE_API std::string GetRegString(char const * const pPath, char const * const pName);
   static ENGINE_API std::string GetAppRegistryPath();     // Returns PX\appname
   static ENGINE_API char const*const GetRootRegistryPath();    // Returns PX\

private:
   void Expand();

private:
   bool        mbIsValid;
   std::string mValue;
};

//------------------------------------------------------------------------------------------
