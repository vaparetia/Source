//------------------------------------------------------------------------------------------
// CScriptVar.h
// Bluepoint
// Copyright 2004
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "boost/optional.hpp"

//------------------------------------------------------------------------------------------

class IScriptManager;

//------------------------------------------------------------------------------------------

class CScriptVar
{
public:
   enum EType
   {
      kType_Bool,
      kType_Int,
      kType_Real,
      kType_String
   };

   enum EKeepExistingValue
   {
      kKeepExistingValue,
      kReplaceExistingValue
   };

public:
   ENGINE_API explicit CScriptVar( std::string const & name,
                                   bool const value,
                                   EKeepExistingValue const keepExistingValue = kKeepExistingValue);

   ENGINE_API explicit CScriptVar( std::string const & name,
                                   int const value,
                                   int const min,
                                   int const max,
                                   EKeepExistingValue const keepExistingValue = kKeepExistingValue);

   ENGINE_API explicit CScriptVar( std::string const & name,
                                   int const value,
                                   boost::optional<int> const min = boost::optional<int>(),
                                   boost::optional<int> const max = boost::optional<int>(),
                                   EKeepExistingValue const keepExistingValue = kKeepExistingValue);

   ENGINE_API explicit CScriptVar( std::string const & name,
                                   real32 const value,
                                   real32 const min,
                                   real32 const max,
                                   EKeepExistingValue const keepExistingValue = kKeepExistingValue);

   ENGINE_API explicit CScriptVar( std::string const & name,
                                   real32 const value,
                                   boost::optional<real32> const min = boost::optional<real32>(),
                                   boost::optional<real32> const max = boost::optional<real32>(),
                                   EKeepExistingValue const keepExistingValue = kKeepExistingValue);

   ENGINE_API explicit CScriptVar( std::string const & name,
                                   std::string const & value,
                                   EKeepExistingValue const keepExistingValue = kKeepExistingValue);

   // Prevent the compiler from getting confused.
   ENGINE_API explicit CScriptVar( std::string const & name,
                                   char const * const pValue,
                                   EKeepExistingValue const keepExistingValue = kKeepExistingValue);

   ENGINE_API void Register( IScriptManager * const pScriptManager );

   ENGINE_API bool const GetBool() const;
   ENGINE_API void SetBool( bool const value );

   ENGINE_API int const GetInt() const;
   ENGINE_API void SetInt( int const value );

   ENGINE_API real32 const GetReal() const;
   ENGINE_API void SetReal( real32 const value );

   ENGINE_API std::string GetString() const;
   ENGINE_API void SetString( std::string const &value );

   static ENGINE_API void              SetDefaultScriptManager(IScriptManager* pScriptManager);
   static ENGINE_API IScriptManager*   GetDefaultScriptManager();

private:
   IScriptManager*            GetScriptManager() const;
   void                       RegisterInternal( IScriptManager * const pScriptManager ) const;

   static IScriptManager*     spDefaultScriptManager;
   mutable IScriptManager*    mpScriptManager;
   
   EType                      mType;
   std::string                mName;

   real32                     mDefault;
   std::string                mStringDefault;
   boost::optional<real32>    mMinValue;
   boost::optional<real32>    mMaxValue;
   EKeepExistingValue         mKeepExistingValue;
};


