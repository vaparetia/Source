//------------------------------------------------------------------------------------------
// CObjectList.cpp
// Bluepoint
// Copyright 2003
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "CObjectList.h"

//------------------------------------------------------------------------------------------

#include "CGameObject.h"

//------------------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------------------

void CObjectList::AddObject( CGameObject const * const pObject )
{
   if( pObject )
   {
      if( DoesObjectPass( *pObject ) )
      {
         mEntities.insert(TGameObjectUniqueIdToGameObjectMap::value_type(pObject->GetId(), const_cast< CGameObject * const>( pObject )));
      }
   }
}

//------------------------------------------------------------------------------------------

void CObjectList::RemoveObject( TGameObjectUniqueId const &id )
{
   TGameObjectUniqueIdToGameObjectMap::iterator iter = mEntities.find(id);
   if (iter != mEntities.end())
   {
      mEntities.erase(iter);
   }
}

//------------------------------------------------------------------------------------------

CGameObject const * const CObjectList::GetObjectById( TGameObjectUniqueId const &id ) const
{
   TGameObjectUniqueIdToGameObjectMap::const_iterator iter = mEntities.find(id);
   if (iter != mEntities.end())
   {
      CGameObject const * const pObject = (*iter).second;
      return pObject;
   }

   return NULL;
}

//------------------------------------------------------------------------------------------

CGameObject * const CObjectList::ObjectById( TGameObjectUniqueId const &id )
{
   return const_cast<CGameObject* const>( GetObjectById( id ) );
}

//------------------------------------------------------------------------------------------
   
bool const CObjectList::DoesObjectPass( CGameObject const & /*object*/ ) const
{
   return true;
}

