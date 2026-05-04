//------------------------------------------------------------------------------------------
// Material.cpp
//------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "Material.h"

//------------------------------------------------------------------------------------------

#include "DataArray.h"

//------------------------------------------------------------------------------------------

using namespace Open3d;

//------------------------------------------------------------------------------------------

Material::Material( XmlNode^ node )
: NamedEntity( (XmlElement^) node )
{
}

//------------------------------------------------------------------------------------------

Material^ Material::Attach( XmlNode^ node )
{
   if( node != nullptr && node->Name == Material::NodeTypeName )
      return gcnew Material( node );

   return nullptr;
}

//------------------------------------------------------------------------------------------

Material^ Material::Create( XmlDocument^ doc, String^ name )
{
   XmlElement ^ node = doc->CreateElement( Material::NodeTypeName );

   Material^ nodeFn = gcnew Material( node );
   nodeFn->Name = name;

   return nodeFn;
}

//------------------------------------------------------------------------------------------

void Material::Add( DataArray^ node )
{
   Node->AppendChild( node );
}

//------------------------------------------------------------------------------------------

String^ Material::PropertyData::get()
{
   XmlNode^ propertyData = Node["PropertyData"];
   if( propertyData != nullptr )
   {
      return propertyData->InnerXml;
   }

   return String::Empty;
}

//------------------------------------------------------------------------------------------

XmlElement ^Material::PropertyXML::get()
{
   return (XmlElement ^) Node[ "PropertyData" ];
}

//------------------------------------------------------------------------------------------

void Material::PropertyData::set( String^ value )
{
   XmlNode^ propertyData = Node["PropertyData"];
   if( propertyData == nullptr )
   {
      propertyData = Node->OwnerDocument->CreateElement( "PropertyData" );
      Node->AppendChild( propertyData );
   }

   propertyData->InnerXml = value;
}

//------------------------------------------------------------------------------------------

DataArray^ Material::GetDataArray( String^ arrayName )
{
   XmlNode^ node = Node->SelectSingleNode( String::Format("./DataArray[@name='{0}']", arrayName) );
   if( node != nullptr )
      return DataArray::Attach( node );

   return nullptr;
}

