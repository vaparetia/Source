//------------------------------------------------------------------------------------------
// Set.h
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
   public ref class Set : public NamedEntity
   {
   public:
      /// identifier for this node type
      static String^ NodeTypeName = "Set";

      /// Attach class to given set node
      static Set^ Attach( XmlNode^ node );

      /// Create new node with given name in given doc
      static Set^ Create( XmlDocument^ doc, String^ name );

   private:
      Set( XmlNode^ node );
   };
}




