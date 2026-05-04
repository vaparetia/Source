//------------------------------------------------------------------------------------------
// CEditorAnimation.cpp
// Copyright 2004
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "CEditorAnimation.h"

//------------------------------------------------------------------------------------------

#include "Engine/Mechanics/CXMLTree.h"

//------------------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------------------
// CEditorAnimationNode
//------------------------------------------------------------------------------------------

CEditorAnimationNode::CEditorAnimationNode( std::string const & name )
:  mName( name )
{
}

//------------------------------------------------------------------------------------------

CEditorAnimationNode::CEditorAnimationNode( CXMLTree const & tree )
{
   ParseTree( tree );
}

//------------------------------------------------------------------------------------------

void CEditorAnimationNode::PutTo( CXMLTree & tree )
{
   tree.SetKeyValue( "Name", mName );

   for( int i = 0; i < mStreamsReal.size(); ++i )
   {
      CXMLTree::TNodePtr const node( new CXMLTree( "StreamReal" ) );
      mStreamsReal[i].PutTo( *node );
      tree.AddChild( node );
   }

   for( int i = 0; i < mStreamsVec3.size(); ++i )
   {
      CXMLTree::TNodePtr const node( new CXMLTree( "StreamVec3" ) );
      mStreamsVec3[i].PutTo( *node );
      tree.AddChild( node );
   }

   for( int i = 0; i < mStreamsQuat.size(); ++i )
   {
      CXMLTree::TNodePtr const node( new CXMLTree( "StreamQuat" ) );
      mStreamsQuat[i].PutTo( *node );
      tree.AddChild( node );
   }

}

//------------------------------------------------------------------------------------------

void CEditorAnimationNode::Simplify()
{
   /*
   for( int i = 0; i < mStreamsReal.size(); ++i )
   {
      mStreamsReal[i].Simplify();
   }

   for( int i = 0; i < mStreamsVec3.size(); ++i )
   {
      mStreamsVec3[i].Simplify();
   }

   for( int i = 0; i < mStreamsQuat.size(); ++i )
   {
      mStreamsQuat[i].Simplify();
   }
   */
}

//------------------------------------------------------------------------------------------

CEditorAnimationStreamReal const * const CEditorAnimationNode::GetRealStream( std::string const & name ) const
{
   for( int i = 0; i < mStreamsReal.size(); ++i )
   {
      if( mStreamsReal[i].GetName() == name )
         return &mStreamsReal[i];
   }

   return NULL;
}

//------------------------------------------------------------------------------------------

CEditorAnimationStreamReal & CEditorAnimationNode::GetOrAddRealStream( std::string const & name )
{
   for( int i = 0; i < mStreamsReal.size(); ++i )
   {
      if( mStreamsReal[i].GetName() == name )
         return mStreamsReal[i];
   }

   mStreamsReal.push_back( CEditorAnimationStreamReal( name ) );

   return mStreamsReal.back();
}

//------------------------------------------------------------------------------------------

CEditorAnimationStreamVec3 const * const CEditorAnimationNode::GetVec3Stream( std::string const & name ) const
{
   for( int i = 0; i < mStreamsVec3.size(); ++i )
   {
      if( mStreamsVec3[i].GetName() == name )
         return &mStreamsVec3[i];
   }

   return NULL;
}

//------------------------------------------------------------------------------------------

CEditorAnimationStreamVec3 & CEditorAnimationNode::GetOrAddVec3Stream( std::string const & name )
{
   for( int i = 0; i < mStreamsVec3.size(); ++i )
   {
      if( mStreamsVec3[i].GetName() == name )
         return mStreamsVec3[i];
   }

   mStreamsVec3.push_back( CEditorAnimationStreamVec3( name ) );

   return mStreamsVec3.back();
}

//------------------------------------------------------------------------------------------

CEditorAnimationStreamQuat const * const CEditorAnimationNode::GetQuatStream( std::string const & name ) const
{
   for( int i = 0; i < mStreamsQuat.size(); ++i )
   {
      if( mStreamsQuat[i].GetName() == name )
         return &mStreamsQuat[i];
   }

   return NULL;
}

//------------------------------------------------------------------------------------------

CEditorAnimationStreamQuat & CEditorAnimationNode::GetOrAddQuatStream( std::string const & name )
{
   for( int i = 0; i < mStreamsQuat.size(); ++i )
   {
      if( mStreamsQuat[i].GetName() == name )
         return mStreamsQuat[i];
   }

   mStreamsQuat.push_back( CEditorAnimationStreamQuat( name ) );

   return mStreamsQuat.back();
}

//------------------------------------------------------------------------------------------

void CEditorAnimationNode::ParseTree( CXMLTree const & tree )
{
   tree.GetKeyValue( "Name", mName );
   
   CXMLTreeIterator it( &tree );
   it.Next();

   while( !it.IsDone() )
   {
      CXMLTree const & currentNode = it.GetCurrentNode();
      
      if( currentNode.GetName() == "StreamReal" )
      {
         mStreamsReal.push_back( CEditorAnimationStreamReal( currentNode ) );
      }
      else if( currentNode.GetName() == "StreamVec3" )
      {
         mStreamsVec3.push_back( CEditorAnimationStreamVec3( currentNode ) );
      }
      else if( currentNode.GetName() == "StreamQuat" )
      {
         mStreamsQuat.push_back( CEditorAnimationStreamQuat( currentNode ) );
      }

      it.NextSkipChildren();
   }
}

//------------------------------------------------------------------------------------------
// CEditorAnimation
//------------------------------------------------------------------------------------------

CEditorAnimation::CEditorAnimation( real32 const minTime, 
                                    real32 const maxTime )
:  mMinTime( minTime )
,  mMaxTime( maxTime )
{
}

//------------------------------------------------------------------------------------------
   
CEditorAnimation::CEditorAnimation( CXMLTree const & tree )
:  mMinTime( 0.0f )
,  mMaxTime( 0.0f )
{
   ParseTree( tree );
}
   
//------------------------------------------------------------------------------------------

void CEditorAnimation::PutTo( CXMLTree & tree )
{
   tree.SetKeyValue( "MinTime", mMinTime );
   tree.SetKeyValue( "MaxTime", mMaxTime );

   for( int i = 0; i < mNodes.size(); ++i )
   {
      CXMLTree::TNodePtr const node( new CXMLTree( "Node" ) );
      
      mNodes[i].PutTo( *node );

      tree.AddChild( node );
   }
}

//------------------------------------------------------------------------------------------

void CEditorAnimation::ParseTree( CXMLTree const & tree )
{
   tree.GetKeyValue( "MinTime", mMinTime );
   tree.GetKeyValue( "MaxTime", mMaxTime );

   CXMLTreeIterator it( &tree );

   // skip into first child
   it.Next();

   while( !it.IsDone() )
   {
      CXMLTree const & currentNode = it.GetCurrentNode();
      if( currentNode.GetName() == "Node" )
      {
         CEditorAnimationNode const node( currentNode );
         mNodes.push_back( node );
      }

      it.NextSkipChildren();
   }
}

//------------------------------------------------------------------------------------------

CEditorAnimationNode const * const CEditorAnimation::GetNode( std::string const & name ) const
{
   for( int i = 0; i < mNodes.size(); ++i )
   {
      if( mNodes[i].GetName() == name )
         return &mNodes[i];
   }

   return NULL;
}

//------------------------------------------------------------------------------------------

CEditorAnimationNode & CEditorAnimation::GetOrAddNode( std::string const & name )
{
   for( int i = 0; i < mNodes.size(); ++i )
   {
      if( mNodes[i].GetName() == name )
         return mNodes[i];
   }

   mNodes.push_back( CEditorAnimationNode( name ) );

   return mNodes.back();
}

