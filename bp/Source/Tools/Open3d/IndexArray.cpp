//------------------------------------------------------------------------------------------
// IndexArray.cpp
//------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "IndexArray.h"

//------------------------------------------------------------------------------------------

using namespace Open3d;

//------------------------------------------------------------------------------------------

IndexArray::IndexArray( XmlNode^ node )
: Entity( (XmlElement^) node )
{
}

//------------------------------------------------------------------------------------------

IndexArray^ IndexArray::Attach( XmlNode^ node )
{
   if( node->Name == IndexArray::NodeTypeName )
      return gcnew IndexArray( node );

   return nullptr;
}

//------------------------------------------------------------------------------------------

IndexArray^ IndexArray::Create( XmlDocument^ doc )
{
   XmlElement ^ node = doc->CreateElement( IndexArray::NodeTypeName );
   
   IndexArray^ nodeFn = gcnew IndexArray( node );

   return nodeFn;
}

//------------------------------------------------------------------------------------------

void IndexArray::InternalSetIndices( int const * pData, int count )
{
   Text::StringBuilder^ builder = gcnew Text::StringBuilder();

   int i = 0;

   for( ; i < (count - 1); ++i )
   {
      builder->Append( pData[i].ToString() );
      builder->Append( " " );
   }

   builder->Append( pData[i].ToString() );

   Node->SetAttribute( "count", count.ToString() );
   Node->InnerText = builder->ToString();
}

//------------------------------------------------------------------------------------------

void IndexArray::GetIndices( TIntVector & indices )
{
   indices.reserve( Count );
   array<String^>^ textIndices = Node->InnerText->Split( ' ' );
   for( int i = 0; i < textIndices->Length; ++i )
   {
      indices.push_back( Int32::Parse( textIndices[i] ) );
   }
}

