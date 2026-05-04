//------------------------------------------------------------------------------------------
// Transform.h
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Entity.h"
#include "DataTypes.h"

//------------------------------------------------------------------------------------------

using namespace System;
using namespace System::Xml;

//------------------------------------------------------------------------------------------

namespace Open3d
{
	public ref class Transform : public Entity
	{
   public:
      /// identifier for this node type
      static String^ NodeTypeName = "Transform";

      /// Attach index array class to given transform node
      static Transform^ Attach( XmlNode^ node );

      /// Create new node with given name in given doc
      static Transform^ Create( XmlDocument^ doc );

      property CVector3 Translation
      {
         CVector3 get();
         void set( CVector3 value );
      }

      property CMatrix3 Orientation
      {
         CMatrix3 get();
         void set( CMatrix3 value );
      }

      property CVector3 Scale
      {
         CVector3 get();
         void set( CVector3 value );
      }

      CMatrix34 GetTransform();
      void SetTransform(CMatrix34 const & transform);

      // does this = lhs * this
      void PreMultiplyInplace(Transform^ lhs);
      
      // does this = inverse(lhs) * this
      void PreMultiplyInverseInplace(Transform^ lhs);

      // does this = this * rhs
      void PostMultiplyInplace(Transform^ rhs);
      
      // does this = this * inverse(rhs)
      void PostMultiplyInverseInplace(Transform^ rhs);

   private:
      Transform( XmlNode^ node );
	};
}



