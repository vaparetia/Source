//------------------------------------------------------------------------------------------
// CResourceFactoryLoadItem.h
// Bluepoint
// Win32 class for loading resources.
// Copyright 2006
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/Resource/CBaseResourceFactoryLoadItem.h"

#include <windows.h>

//----------------------------------------------------------------------------

class ENGINE_API CResourceFactoryLoadItem : public CBaseResourceFactoryLoadItem
{
public:
   explicit CResourceFactoryLoadItem(  CResId const & resource,
                                       void ** ppResource,
                                       CResourceFactory::SFactoryEntry const & factory,
                                       IResourcePool * pResourcePool);

   virtual ~CResourceFactoryLoadItem();
   
   virtual void      StartAsyncLoad();
   virtual int32     GetAsyncLoadingFileSize() const;
   virtual void      Update();

private:
   HANDLE                              mFileHandle;
   OVERLAPPED                          mOverlapped;
   std::vector<char>                   mData;
   uint32                              mSize;
};

//----------------------------------------------------------------------------

