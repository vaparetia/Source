//----------------------------------------------------------------------------
// TransfarringCloudFileList.cpp
//
// A TransfarringFileList that also keeps track of whether files need a conflict resolution
//----------------------------------------------------------------------------

#include "TransfarringCloudFileList.h"
#include "Engine/Basics/BPEAssert.h"

TransfarringCloudFileList::CloudSaveFileInternal::CloudSaveFileInternal()
: mSaveFile()
, mIsCorrupt( false )
{
}

TransfarringCloudFileList::CloudSaveFileInternal::CloudSaveFileInternal( TransfarringFileList::TTransfarringSaveFilePtr saveFile, bool isCorrupt )
: mSaveFile( saveFile )
, mIsCorrupt( isCorrupt )
{
}

TransfarringCloudFileList::TransfarringCloudFileList()
{
   mCloudFiles.resize( skNumCloudSlots, CloudSaveFileInternal() );
}

TransfarringCloudFileList::~TransfarringCloudFileList()
{
   ClearCloudFiles();
}

void TransfarringCloudFileList::AddCloudFilePtr( TransfarringFileList::TTransfarringSaveFilePtr const & save, CloudSaveIndex index )
{
   BPE_ASSERT(IsCloudSaveIndexValid(index), "TransfarringCloudFileList::AddCloudFile out of bounds");

   // Cloud files should show their slot number, not a local save index
   save->mSaveIndex = index;

   int adjustedIndex = index - 1;

   mCloudFiles[adjustedIndex].mSaveFile = save;
}

TransfarringFileList::TTransfarringSaveFilePtr const TransfarringCloudFileList::GetCloudFilePtr( ESaveType saveType, ConcatenatedSaveIndex index ) const
{
   ConcatenatedSaveIndex curIndex = 0;
   TransfarringFileList::TTransfarringSaveFilePtr retval;

   for( int i=0; i < mCloudFiles.size(); i++ )
   {
      TransfarringFileList::TTransfarringSaveFilePtr curFile = mCloudFiles[i].mSaveFile;
      if( curFile.get() && ( mCloudFiles[i].mIsCorrupt || curFile->mSaveType == (int)saveType ) )
      {
         if( curIndex == index)
         {
            retval = mCloudFiles[i].mSaveFile;
            break;
         }
         else
         {
            ++curIndex;
         }
      }
   }

   return retval;
}

TransfarringFileList::TTransfarringSaveFilePtr TransfarringCloudFileList::GetCloudFilePtr_CloudSlot( CloudSaveIndex index ) const
{
   BPE_ASSERT( IsCloudSaveIndexValid(index), "TransfarringCloudFileList::GetCloudFilePtr_CloudSlot out of bounds" );

   int adjustedIndex = index - 1;

   return mCloudFiles[adjustedIndex].mSaveFile;
}

void TransfarringCloudFileList::RemoveCloudFile( CloudSaveIndex index )
{
   BPE_ASSERT( IsCloudSaveIndexValid(index), "TransfarringCloudFileList::RemoveCloudFile out of bounds" );

   int adjustedIndex = index - 1;

   mCloudFiles[adjustedIndex] = CloudSaveFileInternal();
}

int TransfarringCloudFileList::GetNumCloudFilesForType(ESaveType saveType)
{
   int sum = 0;
   for( int i=0; i < skNumCloudSlots; i++ )
   {
      // Always display corrupted cloud files since we can't categorize them by type
      if( mCloudFiles[i].mSaveFile.get() 
         && ( mCloudFiles[i].mIsCorrupt || mCloudFiles[i].mSaveFile->mSaveType == (int)saveType ) )
      {
         ++sum;
      }
   }

   return sum;
}

void TransfarringCloudFileList::ClearCloudFiles()
{
   mCloudFiles.clear();
   mCloudFiles.resize( skNumCloudSlots, CloudSaveFileInternal() );
}

bool TransfarringCloudFileList::IsCloudSaveIndexValid( CloudSaveIndex index )
{
   return index > 0 && index <= skNumCloudSlots;
}

bool TransfarringCloudFileList::IsCloudFileCorrupt( ESaveType saveType, ConcatenatedSaveIndex index ) const
{
   ConcatenatedSaveIndex curIndex = 0;

   bool retval = false;

   for( int i=0; i < mCloudFiles.size(); i++ )
   {
      // Corrupt cloud files are included in every list
      TransfarringFileList::TTransfarringSaveFilePtr curFile = mCloudFiles[i].mSaveFile;
      if( curFile.get() && ( mCloudFiles[i].mIsCorrupt || curFile->mSaveType == (int)saveType ) )
      {
         if( curIndex == index)
         {
            retval = mCloudFiles[i].mIsCorrupt;
            break;
         }
         else
         {
            ++curIndex;
         }
      }
   }

   return retval;
}

void TransfarringCloudFileList::SetCloudFileCorrupt( CloudSaveIndex index, bool bCorrupt )
{
   BPE_ASSERT( IsCloudSaveIndexValid(index), "TransfarringCloudFileList::SetCloudFileCorrupt out of bounds" );

   int adjustedIndex = index - 1;

   mCloudFiles[adjustedIndex].mIsCorrupt = bCorrupt;
}

CloudSaveIndex TransfarringCloudFileList::GetCloudSaveIndexFromUniqueID( unsigned char const * const uid ) const
{
   CloudSaveIndex saveIndex = 0;

   for( int i=0; i < mCloudFiles.size(); i++ )
   {
      TransfarringFileList::TTransfarringSaveFilePtr curFile = mCloudFiles[i].mSaveFile;
      if( curFile.get() && !memcmp( curFile->transfarringID, uid, sizeof( curFile->transfarringID ) ) )
      {
         saveIndex = i + 1;
         break;
      }
   }

   return saveIndex;
}