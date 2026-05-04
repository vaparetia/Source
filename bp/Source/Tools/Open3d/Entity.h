//------------------------------------------------------------------------------------------
// Entity.h
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

using namespace System;
using namespace System::Xml;

//------------------------------------------------------------------------------------------

namespace Open3d
{
	public ref class Entity
	{
   public:
      /// Attach scene class to given scene node
      Entity( XmlElement^ node );

      operator XmlElement^ ()
      {
         return mNode;
      }

      property XmlElement^ Node
      {
         XmlElement^ get()
         {
            return mNode;
         }
      }
   protected:
      XmlElement^ mNode;
	};
}


