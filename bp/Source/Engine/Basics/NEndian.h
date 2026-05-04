//------------------------------------------------------------------------------------------
// NEndian.h
// Bluepoint
// Used for platform specific byte swapping.
// Copyright 2006
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/BPEEngineAPI.h"

//------------------------------------------------------------------------------------------

namespace NEndian
{
   // ---
   // Inplace swap
   // ---

   ENGINE_API BPE_FORCEINLINE void Swap2Bytes( void * pData )
   {
   // Comment this back in again when we're actually running on a PS3
#if BPE_ENDIAN==BPE_ENDIAN_LITTLE
      char * pChar = (char *) pData;
      char temp = pChar[0];
      pChar[0] = pChar[1];
      pChar[1] = temp;
#endif
   }

   ENGINE_API BPE_FORCEINLINE void   Swap4Bytes( void * pData )
   {
   // Comment this back in again when we're actually running on a PS3
#if BPE_ENDIAN==BPE_ENDIAN_LITTLE
      char * pChar = (char *) pData;

      {
         char temp = pChar[0];
         pChar[0] = pChar[3];
         pChar[3] = temp;
      }

      {
         char temp = pChar[1];
         pChar[1] = pChar[2];
         pChar[2] = temp;
      }
#endif
   }

   ENGINE_API BPE_FORCEINLINE void   Swap8Bytes( void * pData )
   {
   // Comment this back in again when we're actually running on a PS3
#if BPE_ENDIAN==BPE_ENDIAN_LITTLE
      char * pChar = (char *) pData;

      {
         char temp = pChar[0];
         pChar[0] = pChar[7];
         pChar[7] = temp;
      }

      {
         char temp = pChar[1];
         pChar[1] = pChar[6];
         pChar[6] = temp;
      }

      {
         char temp = pChar[2];
         pChar[2] = pChar[5];
         pChar[5] = temp;
      }

      {
         char temp = pChar[3];
         pChar[3] = pChar[4];
         pChar[4] = temp;
      }
#endif
   }

   //---
   // Return swapped value
   //--
   ENGINE_API BPE_FORCEINLINE uint16 GetSwapped( uint16 value )
   {
      uint16 newValue = value;
      NEndian::Swap2Bytes(&newValue);
      return newValue;
   }

   ENGINE_API BPE_FORCEINLINE int16 GetSwapped( int16 value )
   {
      int16 newValue = value;
      NEndian::Swap2Bytes(&newValue);
      return newValue;
   }

   ENGINE_API BPE_FORCEINLINE uint32 GetSwapped( uint32 value )
   {
      uint32 newValue = value;
      NEndian::Swap4Bytes(&newValue);
      return newValue;
   }

   ENGINE_API BPE_FORCEINLINE int32 GetSwapped( int32 value )
   {
      int32 newValue = value;
      NEndian::Swap4Bytes(&newValue);
      return newValue;
   }

   ENGINE_API BPE_FORCEINLINE uint64 GetSwapped( uint64 value )
   {
      uint64 newValue = value;
      NEndian::Swap8Bytes(&newValue);
      return newValue;
   }

   ENGINE_API BPE_FORCEINLINE int64 GetSwapped( int64 value )
   {
      int64 newValue = value;
      NEndian::Swap8Bytes(&newValue);
      return newValue;
   }

   ENGINE_API BPE_FORCEINLINE real32 GetSwapped( real32 value )
   {
      real32 newValue = value;
      NEndian::Swap4Bytes(&newValue);
      return newValue;
   }

   ENGINE_API BPE_FORCEINLINE real64 GetSwapped( real64 value )
   {
      real64 newValue = value;
      NEndian::Swap8Bytes(&newValue);
      return newValue;
   }

}

//------------------------------------------------------------------------------------------
