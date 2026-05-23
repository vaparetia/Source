//------------------------------------------------------------------------------------------
// TGameObjectTypes.h
// Copyright 2005
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------
#include "Engine/System/CGuid.h"

// Make unique ids a little more typesafe.
// I should also make typesafe comparison operators, but I'm getting worn out with this!

//------------------------------------------------------------------------------------------
class CGameObjectComponent;
class CGameObject;
class CBaseGameManager;

// We can replace this later with something that uses a short term allocator or a pool
typedef std::vector<CGameObjectComponent *>        TComponentListTransient;
typedef std::vector<CGameObjectComponent const *>  TConstComponentListTransient;
//------------------------------------------------------------------------------------------

// TComponentId used when value could be editor or unique id
class TComponentId : public CGuid   
{
public:
   explicit TComponentId(CInputStream &stream)
      : CGuid(stream)
   {
   };

   bool valid() const
   {
      return (*this) != CGuid::Null();
   }

   ENGINE_API bool IsUniqueId() const;

   ENGINE_API CGameObjectComponent const *  GetFirstComponentFromUniqueOrEditorId(CBaseGameManager const &manager) const;
   ENGINE_API CGameObjectComponent *        FirstComponentFromUniqueOrEditorId(CBaseGameManager &manager) const;

   ENGINE_API CGameObject const *           GetFirstGameObjectFromUniqueOrEditorId(CBaseGameManager const &manager) const;
   ENGINE_API CGameObject *                 FirstGameObjectFromUniqueOrEditorId(CBaseGameManager &manager) const;

   // These versions run a query on the component(s) and return the first one that passes
   ENGINE_API CGameObjectComponent const *  GetFirstComponentFromUniqueOrEditorId(CBaseGameManager const &manager, std::string const &query) const;
   ENGINE_API CGameObjectComponent *        FirstComponentFromUniqueOrEditorId(CBaseGameManager &manager, std::string const &query) const;

   ENGINE_API CGameObject const *           GetFirstGameObjectFromUniqueOrEditorId(CBaseGameManager const &manager, std::string const &query) const;
   ENGINE_API CGameObject *                 FirstGameObjectFromUniqueOrEditorId(CBaseGameManager &manager, std::string const &query) const;

   // Returns the number of added components
   int                           GetComponentsFromUniqueOrEditorId(CBaseGameManager const &manager, TConstComponentListTransient &components) const;
   int                           ComponentsFromUniqueOrEditorId(CBaseGameManager &manager, TComponentListTransient &components) const;

   ENGINE_API static TComponentId const & Null();

   // Helper function for when you have to cast from guid
   static inline TComponentId const FromGuid(CGuid const &guid)
   {
      return TComponentId(guid);
   }

protected:
   // Prevent from being created from a guid
   explicit TComponentId(CGuid const &guid)
      : CGuid(guid)
   {
   };
};

//------------------------------------------------------------------------------------------
class TComponentEditorId;

class TComponentUniqueId : public TComponentId
{
public:
   explicit TComponentUniqueId(CInputStream &stream)
      : TComponentId(stream)
   {
   };

   // Allow implicit conversion from TComponentId
   TComponentUniqueId(TComponentId const &id)
      : TComponentId(id)
   {
   };

   ENGINE_API CGameObject const *               GetGameObject(CBaseGameManager const &manager) const;
   ENGINE_API CGameObject *                     GameObject(CBaseGameManager &manager) const;
   ENGINE_API CGameObjectComponent const *      GetComponent(CBaseGameManager const &manager) const;
   ENGINE_API CGameObjectComponent *            Component(CBaseGameManager &manager) const;

   ENGINE_API static TComponentUniqueId const   Create();

private:
   // Prevent conversion from TComponentEditorId
   TComponentUniqueId(TComponentEditorId const &id);

   explicit TComponentUniqueId(CGuid const &guid)
      : TComponentId(guid)
   {
   };
};

//------------------------------------------------------------------------------------------

class TComponentEditorId : public TComponentId
{
public:
   explicit TComponentEditorId(CInputStream &stream)
      : TComponentId(stream)
   {
   };

   // Allow implicit conversion from TComponentId
   TComponentEditorId(TComponentId const &id)
      : TComponentId(id)
   {
   };

   ENGINE_API static TComponentEditorId const Create();

private:
   // Prevent conversion from TComponentUniqueId
   TComponentEditorId(TComponentUniqueId const &id)
      : TComponentId(id)
   {
   };

   explicit TComponentEditorId(CGuid const &guid)
      : TComponentId(guid)
   {
   };
};

//------------------------------------------------------------------------------------------

inline TComponentUniqueId::TComponentUniqueId(TComponentEditorId const &id)
: TComponentId(id)
{
}

//------------------------------------------------------------------------------------------

// TGameObjectId used when value could be editor or unique id
class TGameObjectId : public CGuid   
{
public:
   explicit TGameObjectId(CInputStream &stream)
      : CGuid(stream)
   {
   };
   
   bool valid() const
   {
      return (*this) != CGuid::Null();
   }

   ENGINE_API bool IsUniqueId() const;

   ENGINE_API static TGameObjectId const & Null();

   // Helper function for when you have to cast from guid
   static inline TGameObjectId const FromGuid(CGuid const &guid)
   {
      return TGameObjectId(guid);
   }

protected:
   explicit TGameObjectId(CGuid const &guid)
      : CGuid(guid)
   {
   };
};

//------------------------------------------------------------------------------------------
class TGameObjectEditorId;

class TGameObjectUniqueId : public TGameObjectId
{
public:
   explicit TGameObjectUniqueId(CInputStream &stream)
      : TGameObjectId(stream)
   {
   };

   // Allow implicit conversion from TGameObjectId
   TGameObjectUniqueId(TGameObjectId const &id)
      : TGameObjectId(id)
   {
   };

   ENGINE_API CGameObject const *              GetGameObject(CBaseGameManager const &manager) const;
   ENGINE_API CGameObject *                    GameObject(CBaseGameManager &manager) const;

   ENGINE_API static TGameObjectUniqueId const Create();

private:
   // Prevent conversion from TGameObjectEditorId
   TGameObjectUniqueId(TGameObjectEditorId const &id);

   explicit TGameObjectUniqueId(CGuid const &guid)
      : TGameObjectId(guid)
   {
   };
};

//------------------------------------------------------------------------------------------

class TGameObjectEditorId : public TGameObjectId
{
public:
   explicit TGameObjectEditorId(CInputStream &stream)
      : TGameObjectId(stream)
   {
   };

   // Allow implicit conversion from TGameObjectId
   TGameObjectEditorId(TGameObjectId const &id)
      : TGameObjectId(id)
   {
   };

   ENGINE_API static TGameObjectEditorId const Create();

private:
   // Prevent conversion from TGameObjectUniqueId
   TGameObjectEditorId(TGameObjectUniqueId const &id)
      : TGameObjectId(id)
   {
   };

   explicit TGameObjectEditorId(CGuid const &guid)
      : TGameObjectId(guid)
   {
   };
};

//------------------------------------------------------------------------------------------

inline TGameObjectUniqueId::TGameObjectUniqueId(TGameObjectEditorId const &id)
 : TGameObjectId(id)
{
};

//------------------------------------------------------------------------------------------

extern ENGINE_API TComponentId const        kInvalidComponentId;
extern ENGINE_API TComponentUniqueId const  kInvalidComponentUniqueId;
extern ENGINE_API TComponentEditorId const  kInvalidComponentEditorId;

extern ENGINE_API TGameObjectId const       kInvalidGameObjectId;
extern ENGINE_API TGameObjectUniqueId const kInvalidGameObjectUniqueId;
extern ENGINE_API TGameObjectEditorId const kInvalidGameObjectEditorId;

//------------------------------------------------------------------------------------------






