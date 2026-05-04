//------------------------------------------------------------------------------------------
// TGameObjectTypes.h
// Copyright 2005
//------------------------------------------------------------------------------------------

#include "StdAfx.h"

//------------------------------------------------------------------------------------------

#include "Engine/GameObjectSystem/TGameObjectTypes.h"
#include "Engine/GameObjectSystem/CBaseGameManager.h"
#include "Engine/GameObjectSystem/CGameObjectComponent.h"

//------------------------------------------------------------------------------------------

TComponentId const        kInvalidComponentId(TComponentId::Null());
TComponentUniqueId const  kInvalidComponentUniqueId(TComponentId::Null());
TComponentEditorId const  kInvalidComponentEditorId(TComponentId::Null());

TGameObjectId const       kInvalidGameObjectId(TGameObjectId::Null());
TGameObjectUniqueId const kInvalidGameObjectUniqueId(TGameObjectId::Null());
TGameObjectEditorId const kInvalidGameObjectEditorId(TGameObjectId::Null());

//------------------------------------------------------------------------------------------

static uint64 const skComponentUniqueIdMagicNumber = 0xFFFFFFFFFFFFFADEULL;   // Standards say that this will never match a windows generated guid
static uint64 sComponentUniqueIdCount = 1;

static uint64 const skGameObjectUniqueIdMagicNumber = 0xFFFFFFFFFFFFFEEDULL;  // Standards say that this will never match a windows generated guid
static uint64 sGameObjectUniqueIdCount = 1;

//------------------------------------------------------------------------------------------

bool TComponentId::IsUniqueId() const
{
   return (mUnionUint64_2 == skComponentUniqueIdMagicNumber);
}

//------------------------------------------------------------------------------------------

CGameObjectComponent const * TComponentId::GetFirstComponentFromUniqueOrEditorId(CBaseGameManager const &manager) const
{
   return manager.GetFirstComponentFromUniqueOrEditorId(*this);
}

//------------------------------------------------------------------------------------------

CGameObjectComponent * TComponentId::FirstComponentFromUniqueOrEditorId(CBaseGameManager &manager) const
{
   return manager.FirstComponentFromUniqueOrEditorId(*this);
}

//------------------------------------------------------------------------------------------

CGameObject const * TComponentId::GetFirstGameObjectFromUniqueOrEditorId(CBaseGameManager const &manager) const
{
   CGameObjectComponent const *pComponent = manager.GetFirstComponentFromUniqueOrEditorId(*this);
   if (pComponent)
   {
      return pComponent->GetGameObject();
   }
   return NULL;
}

//------------------------------------------------------------------------------------------

CGameObject * TComponentId::FirstGameObjectFromUniqueOrEditorId(CBaseGameManager &manager) const
{
   CGameObjectComponent *pComponent = manager.FirstComponentFromUniqueOrEditorId(*this);
   if (pComponent)
   {
      return pComponent->GameObject();
   }
   return NULL;
}

//------------------------------------------------------------------------------------------

CGameObjectComponent const * TComponentId::GetFirstComponentFromUniqueOrEditorId(CBaseGameManager const &manager, std::string const &query) const
{
   return manager.GetFirstComponentFromUniqueOrEditorId(*this, query);
}

//------------------------------------------------------------------------------------------

CGameObjectComponent * TComponentId::FirstComponentFromUniqueOrEditorId(CBaseGameManager &manager, std::string const &query) const
{
   return manager.FirstComponentFromUniqueOrEditorId(*this, query);
}

//------------------------------------------------------------------------------------------

CGameObject const * TComponentId::GetFirstGameObjectFromUniqueOrEditorId(CBaseGameManager const &manager, std::string const &query) const
{
   CGameObjectComponent const *pComponent = manager.GetFirstComponentFromUniqueOrEditorId(*this, query);
   if (pComponent)
   {
      return pComponent->GetGameObject();
   }
   return NULL;
}

//------------------------------------------------------------------------------------------

CGameObject * TComponentId::FirstGameObjectFromUniqueOrEditorId(CBaseGameManager &manager, std::string const &query) const
{
   CGameObjectComponent *pComponent = manager.FirstComponentFromUniqueOrEditorId(*this, query);
   if (pComponent)
   {
      return pComponent->GameObject();
   }
   return NULL;
}

//------------------------------------------------------------------------------------------

int TComponentId::GetComponentsFromUniqueOrEditorId(CBaseGameManager const &manager, TConstComponentListTransient &components) const
{
   return manager.GetComponentsFromUniqueOrEditorId(*this, components);
}

//------------------------------------------------------------------------------------------

int TComponentId::ComponentsFromUniqueOrEditorId(CBaseGameManager &manager, TComponentListTransient &components) const
{
   return manager.ComponentsFromUniqueOrEditorId(*this, components);
}

//------------------------------------------------------------------------------------------

TComponentId const & TComponentId::Null()
{
   static TComponentId const skNull(CGuid::Null());
   return skNull;
}

//------------------------------------------------------------------------------------------

CGameObject const * TComponentUniqueId::GetGameObject(CBaseGameManager const &manager) const
{
   return manager.GetGameObjectFromComponentUniqueId(*this);
}

//------------------------------------------------------------------------------------------

CGameObject * TComponentUniqueId::GameObject(CBaseGameManager &manager) const
{
   return manager.GameObjectFromComponentUniqueId(*this);
}

//------------------------------------------------------------------------------------------

CGameObjectComponent const * TComponentUniqueId::GetComponent(CBaseGameManager const &manager) const
{
   return manager.GetComponentFromUniqueId(*this);
}

//------------------------------------------------------------------------------------------

CGameObjectComponent * TComponentUniqueId::Component(CBaseGameManager &manager) const
{
   return manager.ComponentFromUniqueId(*this);
}

//------------------------------------------------------------------------------------------

TComponentUniqueId const TComponentUniqueId::Create()
{
   // Use special create so we can check if something is an editor or unique id
   // This also makes the PS3 and Win32 platforms use the same code
   TComponentUniqueId newId(CGuid::Null());

   newId.mUnionUint64_1 = sComponentUniqueIdCount;
   newId.mUnionUint64_2 = skComponentUniqueIdMagicNumber;
   sComponentUniqueIdCount++;

   return newId;
}

//------------------------------------------------------------------------------------------

TComponentEditorId const TComponentEditorId::Create()
{
   // Use special create so we can check if something is an editor or unique id
   // This also makes the PS3 and Win32 platforms use the same code
   TComponentEditorId newId(CGuid::Null());

   newId.mUnionUint64_1 = sComponentUniqueIdCount;
   newId.mUnionUint64_2 = 0;     // So we don't match unique id magic number
   sComponentUniqueIdCount++;

   return newId;
}

//------------------------------------------------------------------------------------------

bool TGameObjectId::IsUniqueId() const
{
   return (mUnionUint64_2 == skGameObjectUniqueIdMagicNumber);
}

//------------------------------------------------------------------------------------------

TGameObjectId const & TGameObjectId::Null()
{
   static TGameObjectId const skNull(CGuid::Null());
   return skNull;
}

//------------------------------------------------------------------------------------------

TGameObjectEditorId const TGameObjectEditorId::Create()
{
   // Use special create so we can check if something is an editor or unique id
   // This also makes the PS3 and Win32 platforms use the same code
   TGameObjectEditorId newId(CGuid::Null());

   newId.mUnionUint64_1 = sGameObjectUniqueIdCount;
   newId.mUnionUint64_2 = 0;     // So we don't match unique id magic number
   sGameObjectUniqueIdCount++;

   return newId;
}

//------------------------------------------------------------------------------------------

CGameObject const * TGameObjectUniqueId::GetGameObject(CBaseGameManager const &manager) const
{
   return manager.GetAllObjects().GetObjectById(*this);
}

//------------------------------------------------------------------------------------------

CGameObject * TGameObjectUniqueId::GameObject(CBaseGameManager &manager) const
{
   return manager.AllObjects().ObjectById(*this);
}

//------------------------------------------------------------------------------------------

TGameObjectUniqueId const TGameObjectUniqueId::Create()
{
   // Use special create so we can check if something is an editor or unique id
   // This also makes the PS3 and Win32 platforms use the same code
   TGameObjectUniqueId newId(CGuid::Null());

   newId.mUnionUint64_1 = sGameObjectUniqueIdCount;
   newId.mUnionUint64_2 = skGameObjectUniqueIdMagicNumber;
   sGameObjectUniqueIdCount++;

   return newId;
}

//------------------------------------------------------------------------------------------


