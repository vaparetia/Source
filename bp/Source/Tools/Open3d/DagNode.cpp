//----------------------------------------------------------------------------
// DagNode.cpp
//----------------------------------------------------------------------------

#include "stdafx.h"
#include "DagNode.h"

//----------------------------------------------------------------------------

#include "DataArray.h"
#include "Submesh.h"
#include "Transform.h"

//----------------------------------------------------------------------------

using namespace Open3d;

//----------------------------------------------------------------------------

DagNode::DagNode( XmlNode^ node )
: NamedEntity( (XmlElement^) node )
{
}

//----------------------------------------------------------------------------

DagNode^ DagNode::Attach( XmlNode^ node )
{
   if( node != nullptr && node->Name == DagNode::NodeTypeName )
      return gcnew DagNode( node );

   return nullptr;
}

//----------------------------------------------------------------------------

DagNode^ DagNode::Create( XmlDocument^ doc, String^ name )
{
   XmlElement ^ node = doc->CreateElement( DagNode::NodeTypeName );
   
   DagNode^ nodeFn = gcnew DagNode( node );
   nodeFn->Name = name;

   return nodeFn;
}

//----------------------------------------------------------------------------

void DagNode::Add( DagNode^ node )
{
   Node->AppendChild( node );
}

//----------------------------------------------------------------------------

void DagNode::Add( Submesh^ node )
{
   Node->AppendChild( node );
}

//----------------------------------------------------------------------------

void DagNode::Add( DataArray^ node )
{
   Node->AppendChild( node );
}

//----------------------------------------------------------------------------

void DagNode::Add( Transform^ node )
{
   Node->AppendChild( node );
}

//----------------------------------------------------------------------------

void DagNode::Remove( Submesh^ node )
{
   Node->RemoveChild( node );
}

//----------------------------------------------------------------------------

DagNode^ DagNode::Parent::get()
{
   return DagNode::Attach(Node->ParentNode);
}

//----------------------------------------------------------------------------

String^ DagNode::Path::get()
{
   String^ path = "|" + Name;
   
   for( DagNode^ currentParent = Parent; currentParent != nullptr; currentParent= currentParent->Parent )
   {
      path = "|" + currentParent->Name + path;
   }
   
   return path;
}

//----------------------------------------------------------------------------

Transform^ DagNode::GetTransform()
{
   XmlElement^ transformElement = Node[Transform::NodeTypeName];
   if( transformElement != nullptr )
   {
      return Transform::Attach(transformElement);
   }
   
   return nullptr;
}

//----------------------------------------------------------------------------

List<Submesh^>^ DagNode::GetSubmeshes()
{
   List<Submesh^>^ submeshes = gcnew List<Submesh^>();

   XmlNodeList^ nodes = Node->SelectNodes( "./" + Submesh::NodeTypeName );
   for( int i = 0; i < nodes->Count; ++i )
      submeshes->Add( Submesh::Attach( nodes[i] ) );

   return submeshes;
}

//----------------------------------------------------------------------------

DataArray^ DagNode::GetDataArray( String^ arrayName )
{
   XmlNode^ node = Node->SelectSingleNode( String::Format("./DataArray[@name='{0}']", arrayName) );
   if( node != nullptr )
      return DataArray::Attach( node );

   return nullptr;
}

