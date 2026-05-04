//----------------------------------------------------------------------------
// TransfarringFileList.h
//
// A list of files representing a save file, either on a local or remote system
// The base class only supports manual insertion/deletion of files, but it can be extended to use the save system to automatically populate it
//----------------------------------------------------------------------------

#pragma once

#include <vector>
#include <map>
#include "TransfarringDefines.h"
#include "TransfarringID.h"
#include "Engine/Basics/BPEAssert.h"
#include "Engine/StlExtras/reserved_vector.h"

#include "boost/shared_ptr.hpp"

class TransfarringFileList
{
public:
   typedef boost::shared_ptr<TransfarringSaveFile> TTransfarringSaveFilePtr;

   struct TransfarringSaveFileInternal
   {
      TransfarringSaveFileInternal();
      TransfarringSaveFileInternal(TTransfarringSaveFilePtr saveFile, bool hasConflict, bool isCorrupt);

      TTransfarringSaveFilePtr mSaveFile;
      bool mHasConflict;
      bool mIsCorrupt;
   };

   typedef std::vector<TransfarringSaveFileInternal>::iterator TAvailableSaveIterator;

   TransfarringFileList();
   ~TransfarringFileList();

   // All save files indices are addressed on a scale of 0 to the maximum file number-1, regardless of platform, all the fixups are done internally
   int GetNumSaveFiles(ESaveType saveType);
   TTransfarringSaveFilePtr const GetSaveFilePtr(ESaveType saveType, ConcatenatedSaveIndex index); // Gets a save file with an index that crunches gaps together
   TTransfarringSaveFilePtr const GetSaveFilePtrPlatform(ESaveType saveType, int index); // Gets a save file with a platform index that may return NULL
   void AddSaveFilePtr(TTransfarringSaveFilePtr const & save, ESaveType saveType, int index);
   void RemoveSaveFile(ESaveType saveType, int index);
   void ClearSaveFiles();

   int GetNumAvailableSaveSlots( ESaveType saveType );
   int FindFirstAvailableSaveSlot( ESaveType saveType );
   TAvailableSaveIterator GetSaveSlotIterator( ESaveType saveType );
   int FindNextAvailableSaveSlot( ESaveType saveType, TAvailableSaveIterator * pIter );

   void UpdateTransfarringID(TransfarringID tid, ESaveType saveType, int index);

   ConcatenatedSaveIndex GetConcatenatedSaveIndexFromPlatform(ESaveType saveType, int index);

   void SetConflict(bool bHasConflict, ESaveType saveType, int index);
   bool HasConflict(ESaveType saveType, int index) const;

   void SetFileCorrupt_Platform( bool bIsCorrupt, ESaveType saveType, int index );
   bool IsFileCorrupt_Platform( ESaveType saveType, int index );
   bool IsFileCorrupt( ESaveType saveType, ConcatenatedSaveIndex index );

   // MGS3 indices are different from MGS2; this gives you an index you can use anywhere, or vice versa.
   static int GetAbsoluteSaveIndexFromPlatform(int index);
   static int GetPlatformSaveIndexFromAbsolute(int index);

private:
   void ClearInternalSaveFile(TransfarringSaveFileInternal* save);

   // Save file list
   std::map<ESaveType, std::vector<TransfarringSaveFileInternal> > m_SaveFiles;
   bpe::reserved_vector< int, kST_MaxSaveTypes > m_NumSaveFiles;
};
