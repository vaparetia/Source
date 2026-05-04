//------------------------------------------------------------------------------------------
// THandleMap.h
// Bluepoint
// Copyright 2004
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "TLinkedList.h"

//------------------------------------------------------------------------------------------

template < class T, int size > class THandleMap
{
public:
   enum EHandle
   {
      kInvalidHandleId = -1
   };

   class CHandle
   {
   public:
      explicit CHandle( int const id )
      :  mId( id )
      ,  mData( NULL )
      ,  mSize(0)
      {
      }

      int                  mId;
      T *                  mData;
      uint32               mSize;
   };

public:
   THandleMap()
   :  mFreeHandleList( NULL )
   {
      mHandles.reserve( size );

      for( int i = 0; i < size; ++i )
      {
         mHandles.push_back( CHandle( i ) );
      }

      for( int i = 0; i < size; ++i )
      {
         TLinkedList< CHandle > * pNode = new TLinkedList<CHandle>( &mHandles[i] );
         pNode->AddToEnd( mFreeHandleList );
         mLinkedNodes.push_back( pNode );
      }
   }

   ~THandleMap()
   {
      for( int i = 0; i < mLinkedNodes.size(); ++i )
      {
         delete mLinkedNodes[i];
      }

      mLinkedNodes.clear();
   }

   int const Add( T * const pHandleData, uint32 const dataSize )
   {
      if( mFreeHandleList.IsEmpty() )
         return kInvalidHandleId;

      CHandle * const freeHandle = mFreeHandleList.GetNext();

      BPE_VERIFY( freeHandle != NULL, false, "should be impossible to not have a free handle here" );
      
      mLinkedNodes[ freeHandle->mId ]->Remove();
      freeHandle->mData = pHandleData;
      freeHandle->mSize = dataSize;

      return freeHandle->mId;
   }

   T * const Remove( int const handleId )
   {
      BPE_VERIFY( handleId < size, false, "handleId outside of range" );
      BPE_VERIFY( mLinkedNodes[ handleId ]->IsInList() == false, false, "already removed" );
      
      CHandle & handle = mHandles[ handleId ];
      T * const pData = handle.mData;
       
      handle.mData = NULL;
      handle.mSize = 0;

      mLinkedNodes[ handleId ]->AddToEnd( mFreeHandleList );

      return pData;
   }

   T const * const Get( int const handleId ) const
   {
      return mHandles[handleId].mData;
   }

   T * const Get( int const handleId )
   {
      return mHandles[handleId].mData;
   }

   uint32 const GetHandleSize(int const handleId)
   {
      return mHandles[handleId].mSize;
   }

private:
   std::vector< CHandle >                 mHandles;
   std::vector< TLinkedList< CHandle >* > mLinkedNodes;
   TLinkedList<CHandle>                   mFreeHandleList;
};
