//------------------------------------------------------------------------------------------
// DCCResourceFactoryLoadItem.h
// Dreamcast resource factory load item stub — KallistiOS synchronous I/O
//------------------------------------------------------------------------------------------

#pragma once

#include "Engine/BPEEngineAPI.h"
#include "Engine/Resource/CBaseResourceFactoryLoadItem.h"

class ENGINE_API CResourceFactoryLoadItem : public CBaseResourceFactoryLoadItem
{
public:
   explicit CResourceFactoryLoadItem(CResId const & resource,
      void ** ppResource,
      CResourceFactory::SFactoryEntry const & factory,
      IResourcePool * pResourcePool);

   virtual ~CResourceFactoryLoadItem();

   virtual void   StartAsyncLoad();
   virtual int32  GetAsyncLoadingFileSize() const;
   virtual void   Update();

private:
   std::vector<char> mData;
   uint32            mSize;
   bool              mLoadCompleted;
};
