//------------------------------------------------------------------------------------------
// TComPtr.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------
// Wrapper around COM Interface Ptr's ( Automatic Release and AddRef )
//------------------------------------------------------------------------------------------

template<class T> class TComPtr
{
public:
   explicit TComPtr( T* const pInterface, const bool addRef = false )
   :  mpInterface( pInterface )
   {
      if( addRef )
      {
         mpInterface->AddRef();
      }
   }

   TComPtr( const TComPtr& other )
   :  mpInterface( other.mpInterface )
   {
      if( mpInterface )
      {
         mpInterface->AddRef();
      }
   }

   TComPtr()
   :  mpInterface( NULL )
   {
   }

   ~TComPtr()
   {
      if( mpInterface )
      {
         mpInterface->Release();
      }
   }

   TComPtr& operator = ( const TComPtr& other )
   {
      if( this != &other )
      {
         if( mpInterface )
         {
            mpInterface->Release();
         }
         mpInterface = other.mpInterface;
         
         if( mpInterface )
         {
            mpInterface->AddRef();
         }
      }

      return *this;
   }

   T& operator *()
   {
      BPE_ASSERT( !IsNull(), "null deref" );
      return *mpInterface; 
   }

   const bool IsNull() const { return mpInterface == NULL; }
   void Reset(T* const pInterface = NULL, bool const addRef = false)
   { 
      if(mpInterface)
      {
         mpInterface->Release();
      }

      mpInterface = pInterface;

      if( mpInterface && addRef)
      {
         mpInterface->AddRef();
      }
   }

   const T* operator -> () const { return mpInterface; }

   T* operator -> () { return mpInterface; }
   T* GetPtr() const { return mpInterface; }

   operator T* () { return mpInterface; }
private:
   T* mpInterface;
};

//------------------------------------------------------------------------------------------
