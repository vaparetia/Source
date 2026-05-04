//------------------------------------------------------------------------------------------
// CScriptVar.cpp
// Bluepoint
// Copyright 2004
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "CScriptVar.h"

//------------------------------------------------------------------------------------------

#include "Engine/Script/IScriptManager.h"

//------------------------------------------------------------------------------------------

IScriptManager* CScriptVar::spDefaultScriptManager = NULL;

//------------------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------------------

CScriptVar::CScriptVar( std::string const & name,
                        bool const value,
                        EKeepExistingValue const keepExistingValue /* = kReplaceOldValue */)
:  mpScriptManager( NULL )
,  mType( kType_Bool )
,  mName( name )
,  mDefault( value ? 1.0f : 0.0f )
,  mKeepExistingValue( keepExistingValue )
{
}

//------------------------------------------------------------------------------------------

CScriptVar::CScriptVar( std::string const & name,
                        int const value,
                        int const min,
                        int const max,
                        EKeepExistingValue const keepExistingValue /* = kReplaceOldValue */)
:  mpScriptManager( NULL )
,  mType( kType_Int )
,  mName( name )
,  mDefault( static_cast<real32>( value ) )
,  mMinValue( static_cast<real32>( min ) )
,  mMaxValue( static_cast<real32>( max ) )
,  mKeepExistingValue( keepExistingValue )
{
}

//------------------------------------------------------------------------------------------

CScriptVar::CScriptVar( std::string const & name,
                        int const value,
                        boost::optional<int> const min /* = boost::optional() */,
                        boost::optional<int> const max /* = boost::optional() */,
                        EKeepExistingValue const keepExistingValue /* = kReplaceOldValue */)
:  mpScriptManager( NULL )
,  mType( kType_Int )
,  mName( name )
,  mDefault( static_cast<real32>(value) )
,  mKeepExistingValue( keepExistingValue )
{
   if (min) mMinValue = static_cast<real32>(*min);
   if (max) mMaxValue = static_cast<real32>(*max);
}

//------------------------------------------------------------------------------------------

CScriptVar::CScriptVar( std::string const & name,
                        real32 const value,
                        real32 const min,
                        real32 const max,
                        EKeepExistingValue const keepExistingValue /* = kReplaceOldValue */)
:  mpScriptManager( NULL )
,  mType( kType_Real )
,  mName( name )
,  mDefault( value )
,  mMinValue( min )
,  mMaxValue( max )
,  mKeepExistingValue( keepExistingValue )
{
}

//------------------------------------------------------------------------------------------

CScriptVar::CScriptVar( std::string const & name,
                        real32 const value,
                        boost::optional<real32> const min /* = boost::optional() */,
                        boost::optional<real32> const max /* = boost::optional() */,
                        EKeepExistingValue const keepExistingValue /* = kReplaceOldValue */)
:  mpScriptManager( NULL )
,  mType( kType_Real )
,  mName( name )
,  mDefault( value )
,  mMinValue( min )
,  mMaxValue( max )
,  mKeepExistingValue( keepExistingValue )
{
}

//------------------------------------------------------------------------------------------

CScriptVar::CScriptVar( std::string const & name,
                        std::string const & value,
                        EKeepExistingValue const keepExistingValue /* = kReplaceOldValue */)
:  mpScriptManager( NULL )
,  mType( kType_String )
,  mName( name )
,  mDefault(0.0f)
,  mStringDefault( value )
,  mKeepExistingValue( keepExistingValue )
{
}

//------------------------------------------------------------------------------------------

CScriptVar::CScriptVar( std::string const & name,
                        char const * const pValue,
                        EKeepExistingValue const keepExistingValue /* = kReplaceOldValue */)
:  mpScriptManager( NULL )
,  mType( kType_String )
,  mName( name )
,  mDefault(0.0f)
,  mStringDefault( pValue )
,  mKeepExistingValue( keepExistingValue )
{
}

//------------------------------------------------------------------------------------------

void CScriptVar::SetDefaultScriptManager(IScriptManager* pScriptManager)
{
   spDefaultScriptManager = pScriptManager;
}

//------------------------------------------------------------------------------------------

IScriptManager* CScriptVar::GetDefaultScriptManager()
{
   return spDefaultScriptManager;
}

//------------------------------------------------------------------------------------------

IScriptManager* CScriptVar::GetScriptManager() const
{
   BPE_VERIFY( (mpScriptManager != NULL) || (spDefaultScriptManager != NULL), false, "not registered and no default SM" );
   if (!mpScriptManager)
   {
      RegisterInternal(spDefaultScriptManager);
   }

   return mpScriptManager;
}

//------------------------------------------------------------------------------------------

bool const CScriptVar::GetBool() const
{
   BPE_VERIFY( (mType == kType_Bool) || (mType == kType_Int), false, "invalid variable type" );

   return (GetScriptManager()->GetGlobalInt( mName.c_str() ) != 0);
}

//------------------------------------------------------------------------------------------

void CScriptVar::SetBool( bool const value )
{
   BPE_VERIFY( mType == kType_Bool, false, "invalid variable type" );
   
   GetScriptManager()->SetGlobalInt( mName.c_str(), value ? 1 : 0 );
}

//------------------------------------------------------------------------------------------

int const CScriptVar::GetInt() const
{
   BPE_VERIFY( mType == kType_Int, false, "invalid variable type" );

   return GetScriptManager()->GetGlobalInt( mName.c_str() );
}

//------------------------------------------------------------------------------------------

void CScriptVar::SetInt( int const value )
{
   BPE_VERIFY( mType == kType_Int, false, "invalid variable type" );

   GetScriptManager()->SetGlobalInt( mName.c_str(), value );
}

//------------------------------------------------------------------------------------------

real32 const CScriptVar::GetReal() const
{
   BPE_VERIFY( mType == kType_Real, false, "invalid variable type" );

   return GetScriptManager()->GetGlobalReal( mName.c_str() );
}

//------------------------------------------------------------------------------------------

void CScriptVar::SetReal( real32 const value )
{
   BPE_VERIFY( mType == kType_Real, false, "invalid variable type" );

   GetScriptManager()->SetGlobalReal( mName.c_str(), value );
}

//------------------------------------------------------------------------------------------

std::string CScriptVar::GetString() const
{
   BPE_VERIFY( mType == kType_String, false, "invalid variable type" );

   return GetScriptManager()->GetGlobalString( mName.c_str() );
}

//------------------------------------------------------------------------------------------

void CScriptVar::SetString( std::string const &value )
{
   BPE_VERIFY( mType == kType_String, false, "invalid variable type" );

   GetScriptManager()->SetGlobalString( mName.c_str(), value.c_str() );
}

//------------------------------------------------------------------------------------------

void CScriptVar::Register( IScriptManager * const pScriptManager )
{
   RegisterInternal(pScriptManager);
}

//------------------------------------------------------------------------------------------

void CScriptVar::RegisterInternal( IScriptManager * const pScriptManager ) const
{
   // already registered ?
   if( mpScriptManager )
      return;

   mpScriptManager = pScriptManager;

   if ((mKeepExistingValue == kKeepExistingValue) && (mpScriptManager->GlobalExists(mName.c_str())))
   {
      // Don't reset value.
      return;
   }

   switch( mType )
   {
      case kType_Bool:
         pScriptManager->SetGlobalInt( mName.c_str(), (mDefault != 0.0f) ? 1 : 0 );
         break;

      case kType_Int:
         pScriptManager->SetGlobalInt( mName.c_str(), static_cast<int>( mDefault ) );
         break;

      case kType_Real:
         pScriptManager->SetGlobalReal( mName.c_str(), mDefault );
         break;

      case kType_String:
         pScriptManager->SetGlobalString( mName.c_str(), mStringDefault.c_str());
         break;

      default:
         BPE_VERIFYA( false, "unsupported type" );
         break;
   }
}

