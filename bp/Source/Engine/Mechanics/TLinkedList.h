//------------------------------------------------------------------------------------------
// TLinkedList.h
// Bluepoint
// Copyright 2004
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------
// This template class implements both a linked list and the linked list entry.
//------------------------------------------------------------------------------------------

template < class T > class TLinkedList
{
public:
   // by default this is head of an empty list
   TLinkedList( T * const pOwner )
   :  mHead( this )
   ,  mNext( this )
   ,  mPrev( this )
   ,  mOwner( pOwner )
   {
   }

   // removes current node from the list, or if it's head of the list it clears the whole list
   ~TLinkedList()
   {
      Clear();
   }

   // returns wheter this linked list is empty or not
   bool const IsEmpty() const
   {
      return mHead->mNext == mHead;
   }

   // returns wheter this node is part of a list, will return when called on head of list
   bool const IsInList() const
   {
      return mHead != this;
   }

   // returns size of this list
   int const Size() const
   {
      TLinkedList * node;
      int count = 0;

      for( node = mHead->mNext; node != mHead; node = node->mNext )
      {
         ++count;
      }

      return count;
   }

   // if this node is at the head of the list it will clear the list, otherwise it will just remove this item from the list
   void Clear()
   {
      if( mHead == this )
      {
         while( mNext != this )
            mNext->Remove();
      }
      else
         Remove();
   }

   // places the node before the existing node in the list
   void InsertBefore( TLinkedList & node )
   {
      Remove();

      mNext = &node;
      mPrev = node.mPrev;

      node.mPrev = this;
      mPrev->mNext = this;
      mHead = node.mHead;
   }

   // places the node after the existing node in the list
   void InsertAfter( TLinkedList & node )
   {
      Remove();

      mPrev          = &node;
      mNext          = node.mNext;
      node.mNext	   = this;
      mNext->mPrev   = this;
      mHead          = node.mHead;
   }

   // adds node to end of list
   void AddToEnd( TLinkedList & node )
   {
      InsertBefore( *node.mHead );
   }

   // adds node to front of list
   void AddToFront( TLinkedList & node )
   {
      InsertAfter( *node.mHead );
   }

   // removes current node from its list
   void Remove()
   {
      mPrev->mNext = mNext;
      mNext->mPrev = mPrev;

      mNext = this;
      mPrev = this;
      mHead = this;
   }

   T * const GetNext() const
   {
      if( !mNext || ( mNext == mHead ) )
      {
         return NULL;
      }

      return mNext->GetOwner();
   }

   T * const GetPrev() const
   {
      if( !mPrev || ( mPrev == mHead ) )
      {
         return NULL;
      }
      return mPrev->GetOwner();
   }

   T * const GetOwner() const
   {
      return mOwner;
   }

   void SetOwner( T * const object )
   {
      mOwner = object;
   }

   TLinkedList * const GetListHead() const
   {
      return mHead;
   }

   TLinkedList * const GetNextNode() const
   {
      if( mNext == mHead )
         return NULL;

      return mNext;
   }

   TLinkedList * const GetPrevNode() const
   {
      if( mPrev == mHead )
         return NULL;

      return mPrev;
   }

private:
   TLinkedList *	mHead;
   TLinkedList *	mNext;
   TLinkedList *	mPrev;
   T *            mOwner;

private:
   BPE_DISABLE_COPY_AND_ASSIGNMENT( TLinkedList );
};

