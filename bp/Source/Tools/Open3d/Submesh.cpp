//------------------------------------------------------------------------------------------
// Submesh.cpp
//------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "Submesh.h"

//------------------------------------------------------------------------------------------

#include "IndexArray.h"
#include "DataArray.h"

//------------------------------------------------------------------------------------------

using namespace Open3d;

//------------------------------------------------------------------------------------------

Submesh::Submesh( XmlNode^ node )
: Entity( (XmlElement^) node )
{
}

//------------------------------------------------------------------------------------------

Submesh^ Submesh::Attach( XmlNode^ node )
{
   if( node->Name == Submesh::NodeTypeName )
      return gcnew Submesh( node );

   return nullptr;
}

//------------------------------------------------------------------------------------------

Submesh^ Submesh::Create( XmlDocument^ doc )
{
   XmlElement ^ node = doc->CreateElement( Submesh::NodeTypeName );
   
   Submesh^ nodeFn = gcnew Submesh( node );
   return nodeFn;
}

//------------------------------------------------------------------------------------------

void Submesh::Add( IndexArray^ node )
{
   Node->AppendChild( node );
}

//------------------------------------------------------------------------------------------

IndexArray^ Submesh::GetIndexArray( String^ inputName )
{
   XmlNodeList^ nodes = Node->SelectNodes( "./" + IndexArray::NodeTypeName );
   for( int i = 0; i < nodes->Count; ++i )
   {
      IndexArray^ indexArray = IndexArray::Attach( nodes[i] );
      array<String^>^ inputs = indexArray->Inputs;
      for( int j = 0; j < inputs->Length; ++j )
      {
         if( inputs[j] == inputName )
            return indexArray;
      }
   }

   return nullptr;
}

//------------------------------------------------------------------------------------------

void Submesh::Add( DataArray^ node )
{
   Node->AppendChild( node );
}

//------------------------------------------------------------------------------------------

DataArray^ Submesh::GetDataArray( String^ arrayName )
{
   XmlNode^ node = Node->SelectSingleNode( String::Format("./{0}[@name='{1}']", DataArray::NodeTypeName, arrayName) );

   if( node != nullptr )
   {
      return DataArray::Attach( node );
   }

   return nullptr;
}

