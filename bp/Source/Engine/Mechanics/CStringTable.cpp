//----------------------------------------------------------------------------
// CStringTable.cpp
// Bluepoint
// Copyright 2008
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CStringTable.h"
#include "Engine/Streams/CInputStream.h"
#include "Engine/Streams/COutputStream.h"

//----------------------------------------------------------------------------

CStringTable::CStringTable()
{
   BPE_ASSERTA("Should not be called.");
}

//----------------------------------------------------------------------------

CStringTable::~CStringTable()
{
}

//----------------------------------------------------------------------------

char const * const CStringTable::GetString(uint32 const index) const
{
   BPE_ASSERT(index < mNumStrings, "Index out of range");
   char const * const pThis = reinterpret_cast<char const * const>(this);
   char const * const pString = pThis + (&mStringOffsets)[index];
   return pString;
}

//----------------------------------------------------------------------------

void CStringTable::PutTo(COutputStream& stream) const
{
   stream.WriteUint32(mAllocSize);
   stream.WriteUint32(mNumStrings);
   for (int loop = 0; loop < mNumStrings; loop++)
   {
      stream.WriteUint32((&mStringOffsets)[loop]);
   }

   uint32 const stringsSize = mAllocSize - sizeof(uint32) * (2 + mNumStrings);
   char const * const pStrings = reinterpret_cast<char const * const>(this) + (mAllocSize - stringsSize);
   stream.Put(pStrings, stringsSize);
}

//----------------------------------------------------------------------------

CStringTable * CStringTable::Factory(std::vector<std::string> const &strings)
{
   // Calculate allocation required for offsets and strings
   uint32 allocSize = sizeof(uint32) * (2 + strings.size());
   foreach(std::string const &string, strings)
   {
      allocSize += string.size() + 1;
   }
   // Note that malloc is called deliberately, don't invoke constructor.
   CStringTable * pST = reinterpret_cast<CStringTable*>(malloc(allocSize));
   pST->mAllocSize = allocSize;
   pST->mNumStrings = strings.size();

   uint32 stringOffset = sizeof(uint32) * (2 + pST->mNumStrings);
   for (int loop = 0; loop < pST->mNumStrings; loop++)
   {
      // Set offset
      (&pST->mStringOffsets)[loop] = stringOffset;

      // Copy string into buffer
      char * const pDestString = reinterpret_cast<char * const>(pST) + stringOffset;
      std::string const &srcString = strings[loop];
      strcpy(pDestString, srcString.c_str());

      // Update offset
      stringOffset += srcString.size() + 1;
   }

   return pST;
}

//----------------------------------------------------------------------------

CStringTable * CStringTable::Factory(void * pMemory)
{
   CStringTable * pST = reinterpret_cast<CStringTable*>(pMemory);
   // Swap endian for PC
   NEndian::Swap4Bytes(&pST->mAllocSize);
   NEndian::Swap4Bytes(&pST->mNumStrings);
   for (int loop = 0; loop < pST->mNumStrings; loop++)
   {
      NEndian::Swap4Bytes(&pST->mStringOffsets + loop);
   }

   return pST;
}

//----------------------------------------------------------------------------

CStringTable * CStringTable::Factory(CInputStream &stream)
{
   uint32 const allocSize = stream.ReadUint32();
   // Note that malloc is called deliberately, don't invoke constructor.
   CStringTable * pST = reinterpret_cast<CStringTable*>(malloc(allocSize));
   pST->mAllocSize = allocSize;

   // Re-endian swap for in place memory initialization
   NEndian::Swap4Bytes(&pST->mAllocSize);

   // Read rest of strings
   stream.Get(&pST->mNumStrings, allocSize - sizeof(uint32));

   // 'Construct' from memory
   return CStringTable::Factory(pST);
}

//----------------------------------------------------------------------------
// Testing code for CStringTable

/*
   #include "Engine/Streams/CDiskOutputStream.h"

   {
      std::vector<std::string>   strings;
      strings.push_back("ABCDE_1");
      strings.push_back("ABCDE_2");
      strings.push_back("ABCDE_3");
      strings.push_back("ABCDE_4");
      strings.push_back("ABCDE_5");
      CStringTable * pST = CStringTable::Factory(strings);
      {
         CDiskOutputStream outStream("c:\\temp\\stringtable.bin");
         pST->PutTo(outStream);
      }
      delete pST;
      {
         CDiskInputStream inStream("c:\\temp\\stringtable.bin");
         pST = CStringTable::Factory(inStream);
         for (uint32 loop = 0; loop < pST->GetNumStrings(); loop++)
         {
            bpe_debugger_and_console_printf("%d: %s\n", loop, pST->GetString(loop));
         }
         delete pST;
      }
   }
*/


