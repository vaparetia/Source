//----------------------------------------------------------------------------
// TransfarringCloudFileList.h
//
// Similar to TransfarringFileList, except only stores info for files on the cloud
//----------------------------------------------------------------------------

#pragma once
#include <vector>
#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/StlExtras/BPEStlExtras.h"
#include "TransfarringDefines.h"
#include "TransfarringFileList.h"

class TransfarringCloudFileList
{
public:
   static const int skNumCloudSlots = 8;

   TransfarringCloudFileList();
   virtual ~TransfarringCloudFileList();

   void AddCloudFilePtr( TransfarringFileList::TTransfarringSaveFilePtr const & save, CloudSaveIndex index );
   TransfarringFileList::TTransfarringSaveFilePtr const GetCloudFilePtr( ESaveType saveType, ConcatenatedSaveIndex index ) const;
   TransfarringFileList::TTransfarringSaveFilePtr GetCloudFilePtr_CloudSlot( CloudSaveIndex index ) const;
   void RemoveCloudFile( CloudSaveIndex index );
   int GetNumCloudFilesForType( ESaveType saveType );
   void ClearCloudFiles();
   static bool IsCloudSaveIndexValid( CloudSaveIndex index );

   bool IsCloudFileCorrupt( ESaveType saveType, ConcatenatedSaveIndex index ) const;
   void SetCloudFileCorrupt( CloudSaveIndex index, bool bCorrupt);

   CloudSaveIndex GetCloudSaveIndexFromUniqueID( unsigned char const * const uid ) const;

private:
   struct CloudSaveFileInternal
   {
      CloudSaveFileInternal();
      CloudSaveFileInternal(TransfarringFileList::TTransfarringSaveFilePtr saveFile, bool isCorrupt);

      TransfarringFileList::TTransfarringSaveFilePtr mSaveFile;
      bool mIsCorrupt;
   };

   void ClearInternalSaveFile(CloudSaveFileInternal* save);

   bpe::reserved_vector<CloudSaveFileInternal, skNumCloudSlots> mCloudFiles;
};