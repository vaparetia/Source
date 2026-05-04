//------------------------------------------------------------------------------------------
// TDelayComPtr.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

#include <xtl.h>

extern std::vector<IDirect3DResource9*> g_ToReleaseArray;
inline void AddResourceToBeReleased(IDirect3DResource9 * pResource)
{ 
   g_ToReleaseArray.push_back(pResource);
}

//------------------------------------------------------------------------------------------
// Wrapper around COM Interface Ptr's ( Automatic Release and AddRef )
//------------------------------------------------------------------------------------------
template<class T> 
class TDelayComPtr
{
private:
   TDelayComPtr( TDelayComPtr const & other )
   {
   }

   TDelayComPtr & operator = ( TDelayComPtr const & other )
   {
      return *this;
   }
public:
   explicit TDelayComPtr( T * const pInterface )
   : mpInterface( pInterface )
   {
   }

   TDelayComPtr()
   : mpInterface( NULL )
   {
   }

   ~TDelayComPtr()
   {
      if( mpInterface )
      {
         g_ToReleaseArray.push_back( mpInterface );
      }
   }

   TDelayComPtr & operator = ( T * other )
   {
      if( mpInterface != other )
      {
         if( mpInterface )
         {
            g_ToReleaseArray.push_back( mpInterface );
         }
         mpInterface = other;
      }

      return *this;
   }

   T & operator *()
   {
      BPE_ASSERT( !IsNull(), "null deref" );
      return *mpInterface; 
   }

   const bool IsNull() const { return mpInterface == NULL; }
   void Reset(T * const pInterface = NULL)
   { 
      if( mpInterface )
      {
         g_ToReleaseArray.push_back( mpInterface );
      }

      mpInterface = pInterface;
   }

   T const * operator -> () const { return mpInterface; }

   T * operator -> () { return mpInterface; }
   T * GetPtr() const { return mpInterface; }

   operator T * () { return mpInterface; }
private:
   T * mpInterface;
};

//------------------------------------------------------------------------------------------
