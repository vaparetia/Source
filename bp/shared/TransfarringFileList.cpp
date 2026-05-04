//------------------------------------------------------------------------------------------
// TransfarringFileList.cpp
//
//
//------------------------------------------------------------------------------------------

#include "Engine/Stdafx.h"
#include "TransfarringFileList.h"

#if MGS_VERSION == 2

int skMaxSaveNum[kST_MaxSaveTypes] = { DATA_FILE_MAX, VR_FILE_MAX, SNAKE_TALES_FILE_MAX };

#endif

#if MGS_VERSION == 3

int skMaxSaveNum[kST_MaxSaveTypes] = { /*MC_GAME_LIST_MAX*/ 32 };

#endif

TransfarringFileList::TransfarringFileList()
{
   m_NumSaveFiles.resize(kST_MaxSaveTypes, 0);

   for (int saveType = kST_Game; saveType < kST_MaxSaveTypes; saveType++)
   {
      m_SaveFiles[(ESaveType)saveType].resize(skMaxSaveNum[saveType], TransfarringSaveFileInternal() );
   }
}

TransfarringFileList::~TransfarringFileList()
{
   ClearSaveFiles();
}

TransfarringFileList::TransfarringSaveFileInternal::TransfarringSaveFileInternal()
: mSaveFile()
, mHasConflict( false )
, mIsCorrupt( false )
{
}

TransfarringFileList::TransfarringSaveFileInternal::TransfarringSaveFileInternal( TTransfarringSaveFilePtr saveFile, bool hasConflict, bool isCorrupt )
: mSaveFile( saveFile )
, mHasConflict( hasConflict )
, mIsCorrupt( isCorrupt )
{
}

void TransfarringFileList::ClearSaveFiles()
{
   for (int saveType = kST_Game; saveType < kST_MaxSaveTypes; saveType++)
   {
      std::vector<TransfarringSaveFileInternal> * curList = &m_SaveFiles[(ESaveType)saveType];

      curList->clear();
      curList->resize(skMaxSaveNum[saveType], TransfarringSaveFileInternal() );

      m_NumSaveFiles[(ESaveType)saveType] = 0;
   }
}

void TransfarringFileList::AddSaveFilePtr(TTransfarringSaveFilePtr const &save, ESaveType saveType, int index)
{
   int adjustedIndex = GetAbsoluteSaveIndexFromPlatform( index );
   std::map<ESaveType, std::vector<TransfarringSaveFileInternal> >::iterator saveIter = m_SaveFiles.find( saveType );

   TransfarringSaveFileInternal newInternalSave( save, false, false );

   TransfarringSaveFileInternal* oldInternalSave = &saveIter->second[adjustedIndex];

   newInternalSave.mIsCorrupt = oldInternalSave->mIsCorrupt;

   if ( !saveIter->second[adjustedIndex].mSaveFile )
   {
      m_NumSaveFiles[saveType]++;
   }
   else
   {
      // Preserve conflicts even if the file is overwritten
      newInternalSave.mHasConflict = oldInternalSave->mHasConflict;
   }

   *oldInternalSave = newInternalSave;
}

void TransfarringFileList::RemoveSaveFile(ESaveType saveType, int index)
{
   index = GetAbsoluteSaveIndexFromPlatform(index);
   std::map<ESaveType, std::vector<TransfarringSaveFileInternal> >::iterator saveIter = m_SaveFiles.find( saveType );
   TransfarringSaveFileInternal* internalSave = &saveIter->second[index];
   if( internalSave->mSaveFile.get() )
   {
      *internalSave = TransfarringSaveFileInternal();
      m_NumSaveFiles[saveType]--;
   }
}

TransfarringFileList::TTransfarringSaveFilePtr const TransfarringFileList::GetSaveFilePtr(ESaveType saveType, ConcatenatedSaveIndex index)
{
   if (index >= m_SaveFiles[saveType].size())
   {
      return TTransfarringSaveFilePtr();
   }

   // UI doesn't care about empty slots, just wants the number
   int numFoundSaves = 0;
   for (int i=0; i < m_SaveFiles[saveType].size(); i++)
   {
      TTransfarringSaveFilePtr saveFile = m_SaveFiles[saveType][i].mSaveFile;
      if (saveFile.get() != NULL)
      {
         if (numFoundSaves == index)
         {
            return saveFile;
         }

         numFoundSaves++;
      }
   }

   return TTransfarringSaveFilePtr();
}

TransfarringFileList::TTransfarringSaveFilePtr const TransfarringFileList::GetSaveFilePtrPlatform(ESaveType saveType, int index)
{
   index = GetAbsoluteSaveIndexFromPlatform(index);

   if (index >= m_SaveFiles[saveType].size())
   {
      return TTransfarringSaveFilePtr();
   }

   return m_SaveFiles[saveType][index].mSaveFile;
}

int TransfarringFileList::GetNumSaveFiles(ESaveType saveType)
{
   return m_NumSaveFiles[saveType];
}

int TransfarringFileList::GetNumAvailableSaveSlots(ESaveType saveType)
{
   int numAvailable = 0;
   for (int i=0; i < m_SaveFiles[saveType].size(); i++)
   {
      if (m_SaveFiles[saveType][i].mSaveFile.get() == NULL)
      {
         numAvailable++;
      }
   }

   return numAvailable;
}

int TransfarringFileList::FindFirstAvailableSaveSlot(ESaveType saveType)
{
   for (int i=0; i < m_SaveFiles[saveType].size(); i++)
   {
      if (m_SaveFiles[saveType][i].mSaveFile.get() == NULL)
      {
         return GetPlatformSaveIndexFromAbsolute(i);
      }
   }

   return -1;
}

TransfarringFileList::TAvailableSaveIterator TransfarringFileList::GetSaveSlotIterator( ESaveType saveType  )
{
   return m_SaveFiles[saveType].begin();
}

int TransfarringFileList::FindNextAvailableSaveSlot( ESaveType saveType, TransfarringFileList::TAvailableSaveIterator * pIter )
{
   int nextAvailable = -1;

   while( *pIter != m_SaveFiles[saveType].end() )
   {
      TTransfarringSaveFilePtr saveFile = (*pIter)->mSaveFile;
      if( saveFile.get() == NULL )
      {
         nextAvailable = GetPlatformSaveIndexFromAbsolute( *pIter - m_SaveFiles[saveType].begin() );
      }

      ++(*pIter);

      if( nextAvailable != -1 )
      {
         break;
      }
   }

   return nextAvailable;
}

void TransfarringFileList::UpdateTransfarringID(TransfarringID tid, ESaveType saveType, int index)
{
   ConcatenatedSaveIndex concatIndex = GetConcatenatedSaveIndexFromPlatform(saveType, index);
   TTransfarringSaveFilePtr const saveFile = GetSaveFilePtr(saveType, concatIndex);
   if (saveFile.get() )
   {
      if( tid.m_Type != kTT_Invalid )
      {
         memcpy(saveFile->transfarringID, tid.m_UniqueID, TransfarringDefines::kHashLengthBytes);
      }
      else
      {
         memset(saveFile->transfarringID, 0, TransfarringDefines::kHashLengthBytes);
      }

      memcpy(saveFile->cloudVersionHash, tid.m_VersionID, TransfarringDefines::kHashLengthBytes);
      memcpy(saveFile->psnAccountHash, tid.m_PSNAccountName, TransfarringDefines::kHashLengthBytes);
   }
}

void TransfarringFileList::SetConflict( bool bHasConflict, ESaveType saveType, int index )
{
   int adjustedIndex = GetAbsoluteSaveIndexFromPlatform(index);
   std::map<ESaveType, std::vector<TransfarringSaveFileInternal> >::iterator saveIter = m_SaveFiles.find(saveType);
   BPE_ASSERT( saveIter != m_SaveFiles.end(), "TransfarringFileList::SetConflict - Out of bounds" );
   saveIter->second[adjustedIndex].mHasConflict = bHasConflict;
}

bool TransfarringFileList::HasConflict(ESaveType saveType, int index) const
{
   int adjustedIndex = GetAbsoluteSaveIndexFromPlatform(index);
   std::map<ESaveType, std::vector<TransfarringSaveFileInternal> >::const_iterator saveIter = m_SaveFiles.find(saveType);
   BPE_ASSERT( saveIter != m_SaveFiles.end(), "TransfarringFileList::SetConflict - Out of bounds" );
   return saveIter->second[adjustedIndex].mHasConflict;
}

void TransfarringFileList::SetFileCorrupt_Platform( bool isCorrupt, ESaveType saveType, int index )
{
   int adjustedIndex = GetAbsoluteSaveIndexFromPlatform(index);
   std::map<ESaveType, std::vector<TransfarringSaveFileInternal> >::iterator saveIter = m_SaveFiles.find(saveType);
   BPE_ASSERT( saveIter != m_SaveFiles.end(), "TransfarringFileList::SetCorrupt - Out of bounds" );
   saveIter->second[adjustedIndex].mIsCorrupt = isCorrupt;
}

bool TransfarringFileList::IsFileCorrupt( ESaveType saveType, ConcatenatedSaveIndex index )
{
   int numFoundSaves = 0;
   for (int i=0; i < m_SaveFiles[saveType].size(); i++)
   {
      TTransfarringSaveFilePtr saveFile = m_SaveFiles[saveType][i].mSaveFile;
      if (saveFile.get() != NULL)
      {
         if (numFoundSaves == index)
         {
            return m_SaveFiles[saveType][i].mIsCorrupt;
         }

         numFoundSaves++;
      }
   }

   return false;
}

bool TransfarringFileList::IsFileCorrupt_Platform( ESaveType saveType, int index )
{
   int adjustedIndex = GetAbsoluteSaveIndexFromPlatform(index);
   std::map<ESaveType, std::vector<TransfarringSaveFileInternal> >::const_iterator saveIter = m_SaveFiles.find(saveType);
   BPE_ASSERT( saveIter != m_SaveFiles.end(), "TransfarringFileList::IsFileCorruptPlatform - Out of bounds" );
   return saveIter->second[adjustedIndex].mIsCorrupt;
}

int TransfarringFileList::GetAbsoluteSaveIndexFromPlatform(int index)
{
#if MGS_VERSION == 3
   --index;
#endif

   return index;
}

int TransfarringFileList::GetPlatformSaveIndexFromAbsolute(int index)
{
#if MGS_VERSION == 3
   ++index;
#endif

   return index;
}

ConcatenatedSaveIndex TransfarringFileList::GetConcatenatedSaveIndexFromPlatform(ESaveType saveType, int index)
{
   int numFoundSaves = 0;
   for (int i=0; i < m_SaveFiles[saveType].size(); i++)
   {
      if (m_SaveFiles[saveType][i].mSaveFile.get() != NULL)
      {
         if (GetAbsoluteSaveIndexFromPlatform(index) == i)
         {
            return numFoundSaves;
         }

         numFoundSaves++;
      }
   }

   return -1;
}