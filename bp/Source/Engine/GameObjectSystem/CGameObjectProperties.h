//------------------------------------------------------------------------------------------
// CGameObjectProperties.h
// Container for multiple component properties.
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

#include "TGameObjectTypes.h"
#include "CGameObjectComponentProperties.h"
//------------------------------------------------------------------------------------------

class IGameObjectFactory;

//------------------------------------------------------------------------------------------

class ENGINE_API CGameObjectProperties
{
public:
   typedef  std::vector< boost::shared_ptr<CGameObjectComponentProperties const> >  TComponentProperties;
   typedef  std::vector< boost::shared_ptr<CGameObjectProperties const> >           TChildGameObjectProperties;

   static const int32 kVersion  = 2;  // Must match that in CookProperties.cs

   enum ELoadFlags
   {
      // These flags must match those in CookProperties.cs
      kLF_None                            = 0,
      kLF_DontCreateOnLoad                = 1,     // If true, a script object will not be automatically constructed from these properties.
      kLF_All                             = 0xFFFFFFFF
   };

   explicit CGameObjectProperties(  TGameObjectEditorId const & editorId, 
                                    std::string const &editorName,
                                    uint32 const flags, 
                                    TComponentProperties const &componentProperties);
   virtual ~CGameObjectProperties();

   static CGameObjectProperties * const FGameObjectProperties(CInputStream &inStream, IGameObjectFactory * const pFactory);
   static TChildGameObjectProperties    FChildGameObjectProperties(CInputStream &inStream, IGameObjectFactory * const pFactory); 

   std::string                   mEditorName;
   TGameObjectEditorId           mEditorId;
   uint32                        mFlags;
   TComponentProperties          mComponentProperties;
   TChildGameObjectProperties    mChildGameObjectProperties;
};

