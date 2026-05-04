//------------------------------------------------------------------------------------------
// VTACResourceFactoryLoadItem.h
// Armature
// Copyright 2011
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/Resource/CBaseResourceFactoryLoadItem.h"
#include <scetypes.h>
#include <fios2/fios2_types.h>

//----------------------------------------------------------------------------

class ENGINE_API CResourceFactoryLoadItem : public CBaseResourceFactoryLoadItem
{
public:
   explicit CResourceFactoryLoadItem( CResId const & resource,
      void ** ppResource,
      CResourceFactory::SFactoryEntry const & factory,
      IResourcePool * pResourcePool);

   virtual ~CResourceFactoryLoadItem();

   virtual void      StartAsyncLoad();
   virtual int32     GetAsyncLoadingFileSize() const;
   virtual void      Update();

private:
//   static void       AsyncLoadCallback(CellFsAio *aio, CellFsErrno err, int id, uint64_t size);

   SceFiosFH                           mFileDescriptor;
   int                                 mAsyncIOHandle;
   std::vector<char>                   mData;
   uint32                              mSize;
   volatile bool                       mLoadCompleted;
};

//----------------------------------------------------------------------------

