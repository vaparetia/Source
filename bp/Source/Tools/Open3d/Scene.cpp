//----------------------------------------------------------------------------
// Scene.cpp
//----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"

//----------------------------------------------------------------------------

#include "DagNode.h"
#include "DataArray.h"
#include "IndexArray.h"
#include "Material.h"
#include "Submesh.h"
#include "Transform.h"
#include "Set.h"

//----------------------------------------------------------------------------

using namespace Open3d;

//----------------------------------------------------------------------------

Scene::Scene( XmlNode^ node )
: Entity( (XmlElement^) node )
{
}

//----------------------------------------------------------------------------

Scene^ Scene::Attach( XmlDocument^ doc )
{
   return Attach( doc->SelectSingleNode( "//" + Scene::NodeTypeName ) );
}

//----------------------------------------------------------------------------


Scene^ Scene::Attach( XmlNode^ node )
{
   if( node->Name == Scene::NodeTypeName )
      return gcnew Scene( node );

   return nullptr;
}

//----------------------------------------------------------------------------

Scene^ Scene::Create( XmlDocument^ doc )
{
   XmlElement^ scene = doc->CreateElement( Scene::NodeTypeName );
   scene->SetAttribute( "version", "1.0" );

   Scene^ sceneFn = gcnew Scene( scene );
   return sceneFn;
}

//----------------------------------------------------------------------------

DagNode^ Scene::CreateDagNode( String^ name )
{
   return DagNode::Create( Node->OwnerDocument, name );
}

//----------------------------------------------------------------------------

Submesh^ Scene::CreateSubmesh()
{
   return Submesh::Create( Node->OwnerDocument );
}

//----------------------------------------------------------------------------

IndexArray^ Scene::CreateIndexArray()
{
   return IndexArray::Create( Node->OwnerDocument );
}

//----------------------------------------------------------------------------

DataArray^ Scene::CreateDataArray( String^ name )
{
   return DataArray::Create( Node->OwnerDocument, name );
}

//----------------------------------------------------------------------------

Transform^ Scene::CreateTransform()
{
   return Transform::Create( Node->OwnerDocument );
}

//----------------------------------------------------------------------------

Material^ Scene::CreateMaterial( String^ name )
{
   return Material::Create( Node->OwnerDocument, name );
}

//----------------------------------------------------------------------------

Set^ Scene::CreateSet( String^ name )
{
   return Set::Create( Node->OwnerDocument, name );
}

//----------------------------------------------------------------------------

void Scene::Add( DagNode^ node )
{
   Node->AppendChild( node );
}

//----------------------------------------------------------------------------

void Scene::Add( Material^ material )
{
   Node->AppendChild( material );
}

//----------------------------------------------------------------------------

void Scene::Add( Set^ set )
{
   Node->AppendChild( set );
}

//----------------------------------------------------------------------------

void Scene::Remove( Material ^mat )
{
   RemoveChild( mat->Node );
}

//----------------------------------------------------------------------------

void Scene::Remove( Set ^node )
{
   RemoveChild( node->Node );
}

//----------------------------------------------------------------------------

void Scene::Remove( DagNode ^node )
{
   RemoveChild( node->Node );
}

//----------------------------------------------------------------------------

void Scene::RemoveChild( XmlElement ^xmlnode )
{
   Node->RemoveChild( xmlnode );
}

//----------------------------------------------------------------------------

List<DagNode^>^ Scene::GetMeshes()
{
   List<DagNode^>^ meshNodes = gcnew List<DagNode^>();

   XmlNodeList^ nodes = Node->SelectNodes( "//" + DagNode::NodeTypeName + "[" + Submesh::NodeTypeName + "]" );
   for( int i = 0; i < nodes->Count; ++i )
      meshNodes->Add( DagNode::Attach( nodes[i] ) );

   return meshNodes;
}

//----------------------------------------------------------------------------

bool Scene::HasMaterial( String^ materialName )
{
   return Node->SelectSingleNode( "//" + Material::NodeTypeName + "[@name=\"" + materialName + "\"]" ) != nullptr;
}

//----------------------------------------------------------------------------

Material^ Scene::GetMaterial( String^ materialName )
{
   return Material::Attach( Node->SelectSingleNode( "//" + Material::NodeTypeName + "[@name=\"" + materialName + "\"]" ) );
}

//----------------------------------------------------------------------------
List<Material^> ^Scene::GetMaterials()
{
   List<Material ^>^ materials = gcnew List< Material ^ >();
   System::Collections::IEnumerator ^nodesEnum = Node->SelectNodes( "//" + Material::NodeTypeName )->GetEnumerator();
   while ( nodesEnum->MoveNext() )
   {
      materials->Add( Material::Attach( ( XmlNode ^) nodesEnum->Current ) );
   }

   return materials;
}

//----------------------------------------------------------------------------

bool Scene::HasSet( String^ setName )
{
   return Node->SelectSingleNode( "//" + Set::NodeTypeName + "[@name=\"" + setName + "\"]" ) != nullptr;
}

//----------------------------------------------------------------------------

Set^ Scene::GetSet( String^ setName )
{
   return Set::Attach( Node->SelectSingleNode( "//" + Set::NodeTypeName + "[@name=\"" + setName + "\"]" ) );
}
//----------------------------------------------------------------------------

DagNode^ Scene::GetNodeByPath( String^ path )
{
   array<String^>^ pathElements = path->Split('|');
   
   String^ query = "/";
   
   for( int i = 0; i < pathElements->Length; ++i )
   {
      if( pathElements[i]->Length > 0 )
      {
         query += String::Format("/Node[@name='{0}']", pathElements[i]);
      }
   }

   XmlNode^ node = Node->SelectSingleNode(query);
   if( node != nullptr )
      return DagNode::Attach(node);

   return nullptr;
}

