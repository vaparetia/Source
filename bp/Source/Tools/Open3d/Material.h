//------------------------------------------------------------------------------------------
// Material.h
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "NamedEntity.h"

//------------------------------------------------------------------------------------------

using namespace System;
using namespace System::Xml;

//------------------------------------------------------------------------------------------

namespace Open3d
{
   ref class DataArray;

	public ref class Material : public NamedEntity
	{
   public:
      /// identifier for this node type
      static String^ NodeTypeName = "Material";

      /// Attach index array class to given index array node
      static Material^ Attach( XmlNode^ node );

      /// Create new node with given name in given doc
      static Material^ Create( XmlDocument^ doc, String^ name );

      /// Adds this node as a child
      void Add( DataArray^ node );

      /// Accessor for property data
      property String^ PropertyData
      {
         String^ get();
         void set(String^ value);
      }

      property XmlElement ^PropertyXML
      {
         XmlElement ^get();
      }

      DataArray^ GetDataArray( String^ arrayName );

   private:
      Material( XmlNode^ node );
	};
}




