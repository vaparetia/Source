//------------------------------------------------------------------------------------------
// NamedEntity.h
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
	public ref class NamedEntity : public Entity
	{
   public:
      /// Attach scene class to given scene node
      NamedEntity( XmlElement^ node, String ^ name )
      : Entity( node )
      {
         Name = name;
      }

      NamedEntity( XmlElement^ node )
      : Entity( node )
      {
      }

      property String^ Name
      {
         String^ get()
         {
            return Node->Attributes["name"]->Value;
         }

         void set(String^ value)
         {
            Node->SetAttribute( "name", value );
         }
      }

	};
}



