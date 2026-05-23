//------------------------------------------------------------------------------------------
// CObjectList.h
// Bluepoint
// Copyright 2003
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

class CGameObject;

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/GameObjectSystem/TGameObjectTypes.h"

//------------------------------------------------------------------------------------------

class ENGINE_API CObjectList
{
public:
   void AddObject( CGameObject const * const pObject );
   void RemoveObject( TGameObjectUniqueId const & id );
   
   CGameObject const * const  GetObjectById( TGameObjectUniqueId const & id ) const;
   CGameObject * const        ObjectById( TGameObjectUniqueId const & id );

   int         GetObjectCount() const                                   { return mEntities.size(); };

protected:
   virtual bool const DoesObjectPass( CGameObject const & object ) const;
private:
   typedef std::map<TGameObjectUniqueId, CGameObject *>  TGameObjectUniqueIdToGameObjectMap;
   TGameObjectUniqueIdToGameObjectMap   mEntities;
};

