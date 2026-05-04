//------------------------------------------------------------------------------------------
// CResourceFactoryLoadItem.h
// Bluepoint
// Win32 class for loading resources.
// Copyright 2006
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/System/RVL/RVLFileIO.h"
#include "Engine/Resource/CBaseResourceFactoryLoadItem.h"

#include <revolution/dvd.h>
#include <boost/scoped_ptr.hpp>

//----------------------------------------------------------------------------

class ENGINE_API CResourceFactoryLoadItem : public CBaseResourceFactoryLoadItem
{
public:
   explicit CResourceFactoryLoadItem( CResId const & resource,
                             void ** pObject,
                             CResourceFactory::SFactoryEntry const & factory,
                             IResourcePool * pResourcePool);

   virtual ~CResourceFactoryLoadItem();
   
   virtual void      StartAsyncLoad();
   virtual int32     GetAsyncLoadingFileSize() const;
   virtual void      Update();

private:
   static void RVLFileIOCallback(s32 result, void * cookie, RVLFileIO::TFileInfo *pInfo );

   bool OpenFile( char const *filePath, char const **ppErrorString );

   bool                                mFileInfoValid;
   RVLFileIO::TFileInfo                mFileInfo;

   boost::scoped_ptr<char>             mData;
   uint32                              mSize;
   volatile bool                       mLoadCompleted;
};

//----------------------------------------------------------------------------

