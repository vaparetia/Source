//------------------------------------------------------------------------------------------
// CGameObject.h
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

#include "Engine/GameObjectSystem/TGameObjectTypes.h"
#include "Engine/GameObjectSystem/CGameObjectComponent.h"

//------------------------------------------------------------------------------------------

class CGameObjectProperties;
class CGameObjectConstructionParams;
class CGameObjectComponentMethods;
class IGameObjectFactory;
class TiXmlNode;

//------------------------------------------------------------------------------------------

class ENGINE_API CGameObject
{
public:
   typedef CGameObjectComponent *                  TComponent;
   typedef CGameObjectComponent const *            TConstComponent;
   typedef std::vector< TComponent >               TComponents;
   typedef std::vector< CGameObject * >            TChildren;

   explicit CGameObject( TGameObjectUniqueId const &id );
   explicit CGameObject(boost::shared_ptr<CGameObjectProperties const> pProperties,
                        IGameObjectFactory &factory, 
                        TGameObjectUniqueId const &id );
   virtual ~CGameObject();

   virtual void Initialize(CBaseGameManager &baseGameManager, TGameObjectUniqueId const &packageRootId, CGameObjectConstructionParams const &params);
   virtual void Uninitialize(CBaseGameManager &baseGameManager);

   void BindMethods(CGameObjectComponentMethods &methods) const;
   virtual CGameObjectComponentMethods &GetBindMethods();

   /// Returns the id of this object
   TGameObjectUniqueId const &   GetId() const { return mId; }

   /// Returns parent id of this object
   TGameObjectUniqueId const &   GetParentId() const;
   CGameObject const *           GetParent() const { return mpParent; };
   CGameObject *                 Parent()          { return mpParent; };

   /// Returns package root id of this object
   TGameObjectUniqueId const &   GetPackageRootId() const { return mPackageRootId; }

   /// Adds a child to the list of children
   void AddChild( CGameObject *pGO );
   /// Removes a child from the list of children
   void RemoveChild( CGameObject *pGO );

   /// Deletes all children from itself and base game manager.
   void DeleteAllChildren(CBaseGameManager &baseGameManager);
   void DeleteYourself(CBaseGameManager &baseGameManager);

   /// Returns a const ref to the children list to allow for iteration
   TChildren const & GetChildren() const { return mChildren; }

   TComponents &        Components()            { return mpComponents; };
   TComponents const &  GetComponents() const   { return mpComponents; };

   /// Find component by it's group/name hash
   /// Will return null shared_ptr if can't be found
   TComponent        ComponentByType(uint32 const groupType, uint32 const componentType);
   TConstComponent   GetComponentByType(uint32 const groupType, uint32 const componentType) const;

   /// Find component by it's group hash
   /// Will return null shared_ptr if can't be found
   TComponent        BaseComponentByType(uint32 const groupType);
   TConstComponent   GetBaseComponentByType(uint32 const groupType) const;

   /// Find component by it's editor id
   /// Will return null shared_ptr if can't be found
   TComponent        ComponentByEditorId(TComponentEditorId const & componentEditorId);
   TConstComponent   GetComponentByEditorId(TComponentEditorId const & componentEditorId) const;

   /// Find component in this object or its children
   TConstComponent   GetComponentByEditorIdIncludingChildren(CBaseGameManager const &manager, TComponentEditorId const & componentEditorId) const;
   //TComponent        ComponentByEditorIdIncludingChildren(TObjectId const & componentEditorId);
   void              RemapMessagesAndLinksToUniqueIds(CBaseGameManager &manager, CGameObject const &remapParent, CGameObject const * const pRemapToGenerator, int32 const * const pExcludeMessageType = NULL);
   /// Used to notify components that all other gameobjects/components within their package have been loaded and that all links/messages are now valid.
   virtual void      PostLoadInitializeRecursive(CBaseGameManager &manager);

   boost::shared_ptr<CGameObjectProperties const>
                     GetProperties() const { return mpProperties; };

   // Time
   real64            GetInstanceTime() const       { return mInstanceTime; };
   void              SetInstanceTime(real64 time)  { mInstanceTime = time; };

   void              SetIdentifier(std::string const &identifier);
   std::string const & GetIdentifier() const       { return mIdentifierString; };

   std::string const & GetEditorName() const;

   bool              PendingDeletion() const       { return ((mSharedFlags & (1UL << skSF_PendingDeletion)) != 0); };
   bool              Deleted() const               { return ((mSharedFlags & (1UL << skSF_Deleted)) != 0); };
   bool              PostLoadInitialized() const   { return ((mSharedFlags & (1UL << skSF_PostLoadInitialized)) != 0); };

   // Debug logging
   virtual TiXmlNode * LogInfo(CBaseGameManager const &manager, TiXmlNode * pXml) const;

protected:
   void              UninitializeComponents(CBaseGameManager &baseGameManager);

private:
   void              SetParent( CGameObject *pGO );

protected:
   // Shared flags are used in base class and also by derived classes to improve cache coherency.
   // Be careful to not to overwrite these flags in derived classes.
   static int const skSF_PendingDeletion =      0;
   static int const skSF_Deleted =              1;
   static int const skSF_PostLoadInitialized =  2;
   uint32                                                   mSharedFlags;

   void              SetSharedFlag(int const bitPos, uint32 const value)   { mSharedFlags &= ~( 1UL << bitPos);    mSharedFlags |= (value << bitPos); };

public:
   // We'll put this in here as the DAG support later will be common to all types.
   uint32                                                   mLastUpdateCount;

private:
   // These ordered members near declaration top to improve cache coherency.
   real64                                                   mInstanceTime;
   TChildren                                                mChildren;
   TGameObjectUniqueId                                      mId;
   CGameObject *                                            mpParent;
   // The package root is the object that parents all game objects constructed from a package.
   // This object can be queried for implementation specific state that affects all other objects within this package.
   // E.g. Package transform, package members etc.
   TGameObjectUniqueId                                      mPackageRootId;   
   boost::shared_ptr<CGameObjectProperties const>           mpProperties;
   TComponents                                              mpComponents;
   std::string                                              mIdentifierString;
};

