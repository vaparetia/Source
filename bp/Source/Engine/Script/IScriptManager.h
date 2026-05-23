//------------------------------------------------------------------------------------------
// IScriptManager.h
// Bluepoint
// Copyright 2004
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

class IScriptManager
{
public:
   virtual void SetGlobalBool( char const * const pName, bool const value ) = 0;
   virtual bool const GetGlobalBool( char const * const pName ) = 0;

   virtual void SetGlobalInt( char const * const pName, int const value ) = 0;
   virtual int const GetGlobalInt( char const * const pName ) = 0;

   virtual void SetGlobalReal( char const * const pName, real32 const value ) = 0;
   virtual real32 const GetGlobalReal( char const * const pName ) = 0;

   virtual void SetGlobalString( char const * const pName, char const * const pValue) = 0;
   virtual char const * GetGlobalString( char const * const pName ) = 0;

   virtual bool const GlobalExists( char const * const pName ) = 0;
};

