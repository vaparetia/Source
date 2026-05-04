//------------------------------------------------------------------------------------------
// CEditorAnimation.h
// Copyright 2004
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/Basics/CStringExtras.h"

#include "Engine/Math/CVector2.h"
#include "Engine/Math/CVector3.h"
#include "Engine/Math/CVector4.h"
#include "Engine/Math/CQuaternion.h"

//------------------------------------------------------------------------------------------

class CXMLTree;

//------------------------------------------------------------------------------------------

template <class T> class TEditorAnimationStream
{
public:
   typedef std::pair< real32, T >         TTimeValuePair;
   typedef std::vector< TTimeValuePair >  TTimeValuePairs;

public:
   explicit TEditorAnimationStream( std::string const & name )
   :  mName( name )
   {
   }
   
   std::string const & GetName() const { return mName; }

   explicit TEditorAnimationStream( CXMLTree const & tree )
   {
      tree.GetKeyValue( "Name", mName );
      InternalConstruct<T>( tree );
   }

   void PutTo( CXMLTree & tree )
   {
      tree.SetKeyValue( "Name", mName );
      InternalPutTo<T>( tree );
   }
   
   TTimeValuePairs const & GetElements() const { return mElements; }

   void AddElement( real32 const time, T const & value )
   {
      mElements.push_back( TTimeValuePair( time, value ) );
   }

   void Simplify()
   {
      int currentElement = 0;
      
      while( ( currentElement + 1 ) < mElements.size() )
      {
         if( close_enough( mElements[currentElement].second, mElements[currentElement + 1].second ) )
         {
            mElements.erase( mElements.begin() );
         }
         else
         {
            ++currentElement;
         }
      }

      // in case there is only one single element left, set it's time to zero
      if( mElements.size() == 1 )
      {
         mElements[0].first = 0.0f;
      }
   }

private:
   template <class R> void InternalPutTo( CXMLTree & tree )
   {
      ERROR;
   }

   template <> void InternalPutTo<real32>( CXMLTree & tree )
   {
      std::vector< std::string > & data = tree.Data();
      for( int i = 0; i < mElements.size(); ++i )
      {
         data.push_back( CStringExtras::Stringize( "%f: %f", 
                                                   mElements[i].first, 
                                                   mElements[i].second ) );
      }
   }

   template <> void InternalPutTo<CVector3>( CXMLTree & tree )
   {
      std::vector< std::string > & data = tree.Data();
      for( int i = 0; i < mElements.size(); ++i )
      {
         data.push_back( CStringExtras::Stringize( "%f: %f, %f, %f", 
                                                   mElements[i].first, 
                                                   mElements[i].second.GetX(), 
                                                   mElements[i].second.GetY(), 
                                                   mElements[i].second.GetZ() ) );
      }
   }

   template <> void InternalPutTo<CQuaternion>( CXMLTree & tree )
   {
      std::vector< std::string > & data = tree.Data();
      for( int i = 0; i < mElements.size(); ++i )
      {
         data.push_back( CStringExtras::Stringize( "%f: %f, %f, %f, %f", 
                                                   mElements[i].first, 
                                                   mElements[i].second.GetVector()[X], 
                                                   mElements[i].second.GetVector()[Y], 
                                                   mElements[i].second.GetVector()[Z],
                                                   mElements[i].second.GetScalar() ) );
      }
   }

   template <class R> void InternalConstruct( CXMLTree const & tree )
   {
      ERROR;
   }

   template <> void InternalConstruct<real32>( CXMLTree const & tree )
   {
      std::vector< std::string > const & data = tree.GetData();
      mElements.reserve( data.size() );
      
      for( int i = 0; i < data.size(); ++i )
      {
         real32 t = 0, val = 0;
         if( sscanf_s( data[i].c_str(), "%f: %f", &t, &val ) == 2 )
         {
            mElements.push_back( TTimeValuePair( t, val ) );
         }
      }
   }

   template <> void InternalConstruct<CVector3>( CXMLTree const & tree )
   {
      std::vector< std::string > const & data = tree.GetData();
      mElements.reserve( data.size() );
      
      for( int i = 0; i < data.size(); ++i )
      {
         real32 t = 0, x = 0, y = 0, z = 0;
         if( sscanf_s( data[i].c_str(), "%f: %f, %f, %f", &t, &x, &y, &z ) == 4 )
         {
            mElements.push_back( TTimeValuePair( t, CVector3( x, y, z ) ) );
         }
      }
   }

   template <> void InternalConstruct<CQuaternion>( CXMLTree const & tree )
   {
      std::vector< std::string > const & data = tree.GetData();
      mElements.reserve( data.size() );
      
      for( int i = 0; i < data.size(); ++i )
      {
         real32 t = 0, x = 0, y = 0, z = 0, w = 0;
         if( sscanf_s( data[i].c_str(), "%f: %f, %f, %f, %f", &t, &x, &y, &z, &w ) == 5 )
         {
            mElements.push_back( TTimeValuePair( t, CQuaternion( CVector3( x, y, z ), w ) ) );
         }
      }
   }

private:
   std::string       mName;
   TTimeValuePairs   mElements;
};

//------------------------------------------------------------------------------------------

typedef TEditorAnimationStream<real32>       CEditorAnimationStreamReal;
typedef TEditorAnimationStream<CVector3>     CEditorAnimationStreamVec3;
typedef TEditorAnimationStream<CQuaternion>  CEditorAnimationStreamQuat;

//------------------------------------------------------------------------------------------

class CEditorAnimationNode
{
public:
   explicit CEditorAnimationNode( std::string const & name );
   
   std::string const & GetName() const { return mName; }

   CEditorAnimationStreamReal const * const GetRealStream( std::string const & name ) const;
   CEditorAnimationStreamReal & GetOrAddRealStream( std::string const & name );
   
   CEditorAnimationStreamVec3 const * const GetVec3Stream( std::string const & name ) const;
   CEditorAnimationStreamVec3 & GetOrAddVec3Stream( std::string const & name );

   CEditorAnimationStreamQuat const * const GetQuatStream( std::string const & name ) const;
   CEditorAnimationStreamQuat & GetOrAddQuatStream( std::string const & name );


   explicit CEditorAnimationNode( CXMLTree const & tree );
   void PutTo( CXMLTree & tree );

   void Simplify();

private:
   void ParseTree( CXMLTree const & tree );

private:
   std::string    mName;

   std::vector< CEditorAnimationStreamReal > mStreamsReal;
   std::vector< CEditorAnimationStreamVec3 > mStreamsVec3;
   std::vector< CEditorAnimationStreamQuat > mStreamsQuat;
};

//------------------------------------------------------------------------------------------

class CEditorAnimation
{
public:
   explicit CEditorAnimation( real32 const minTime, real32 const maxTime );
   
   explicit CEditorAnimation( CXMLTree const & tree );
   void PutTo( CXMLTree & tree );

   CEditorAnimationNode const * const GetNode( std::string const & name ) const;
   CEditorAnimationNode & GetOrAddNode( std::string const & name );

   real32 const GetMinTime() const { return mMinTime; }
   real32 const GetMaxTime() const { return mMaxTime; }

private:
   void ParseTree( CXMLTree const & tree );

private:
   real32                              mMinTime;
   real32                              mMaxTime;
   std::vector<CEditorAnimationNode>   mNodes;
};

