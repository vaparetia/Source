//------------------------------------------------------------------------------------------
// Set.cpp
//------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "Set.h"

//------------------------------------------------------------------------------------------

using namespace Open3d;

//------------------------------------------------------------------------------------------

Set::Set( XmlNode^ node )
: NamedEntity( (XmlElement^) node )
{
}

//------------------------------------------------------------------------------------------

Set^ Set::Attach( XmlNode^ node )
{
   if( node != nullptr && node->Name == Set::NodeTypeName )
      return gcnew Set( node );

   return nullptr;
}

//------------------------------------------------------------------------------------------

Set^ Set::Create( XmlDocument^ doc, String^ name )
{
   XmlElement ^ node = doc->CreateElement( Set::NodeTypeName );

   Set^ nodeFn = gcnew Set( node );
   nodeFn->Name = name;

   return nodeFn;
}
