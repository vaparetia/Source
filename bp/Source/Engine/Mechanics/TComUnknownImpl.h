//------------------------------------------------------------------------------------------
// TComUnknownImpl.h
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------
// Template class that implements basics for an interface directly derived from IUnknown.
// Note that QueryInterface only supports IUnknown and <T>
//------------------------------------------------------------------------------------------

template <typename T> class TComUnknownImpl : public T
{
public:
   TComUnknownImpl()
   :  mRefCnt( 1 )
   {
   }

   virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, void **ppvObject )
   {
      if( riid == IID_IUnknown || riid == __uuidof( T ) )
      {
         *ppvObject = this;
         this->AddRef();
         return S_OK;
      }
      else
      {
         *ppvObject = NULL;
         return E_NOINTERFACE;
      }

   }

   virtual ULONG STDMETHODCALLTYPE AddRef()
   {
      ++mRefCnt;

      return mRefCnt;
   }

   virtual ULONG STDMETHODCALLTYPE Release()
   {
      BPE_ASSERT( mRefCnt > 0, "Invalid release" );

      --mRefCnt;

      if( mRefCnt == 0 )
      {
         delete this;
         return 0;
      }

      return mRefCnt;
   }

private:
   int   mRefCnt;
};

//------------------------------------------------------------------------------------------
