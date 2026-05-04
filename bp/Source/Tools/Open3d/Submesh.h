//------------------------------------------------------------------------------------------
// Submesh.h
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Entity.h"

//------------------------------------------------------------------------------------------

using namespace System;
using namespace System::Xml;

//------------------------------------------------------------------------------------------

namespace Open3d
{
   ref class IndexArray;
   ref class DataArray;

	public ref class Submesh : public Entity
	{
   public:
      /// identifier for this node type
      static String^ NodeTypeName = "Submesh";

      /// Attach scene class to given scene node
      static Submesh^ Attach( XmlNode^ node );

      /// Create new node with given name in given doc
      static Submesh^ Create( XmlDocument^ doc );

      property String^ Material
      {
         String^ get()
         {
            return Node->GetAttribute( "material" );
         }

         void set(String^ value )
         {
            Node->SetAttribute( "material", value );
         }
      }

      /// Adds this node as a child
      void Add( IndexArray^ node );
      IndexArray^ GetIndexArray( String^ inputName );

      /// Adds this node as a child
      void Add( DataArray^ node );
      DataArray^ GetDataArray( String^ arrayName );

   private:
      /// Attach scene class to given scene node
      Submesh( XmlNode^ node );
	};
}


