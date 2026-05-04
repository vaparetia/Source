//------------------------------------------------------------------------------------------
// DataArray.h
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "NamedEntity.h"
#include "DataTypes.h"

//------------------------------------------------------------------------------------------

using namespace System;
using namespace System::Xml;
using namespace System::Runtime::InteropServices;

//------------------------------------------------------------------------------------------

namespace Open3d
{
	public ref class DataArray : public NamedEntity
	{
   public:
      enum class EType
      {
         Real,
         Vector2,
         Vector3,
         Vector4,
         Quat3,
         Quat4,
         Color,
         ColorF,
         Guid
      };

      /// identifier for this node type
      static String^ NodeTypeName = "DataArray";

      /// Attach data array class to given data array node
      static DataArray^ Attach( XmlNode^ node );

      /// Create new node with given name in given doc
      static DataArray^ Create( XmlDocument^ doc, String^ name );

      // float array accessors
      void SetData( array<float>^ data );
      void SetData( TFloatVector const & data );
      void GetData( array<float>^ & data );
      void GetData( TFloatVector & data );

      // vector2 array accessors
      void SetData( array<Vector2>^ data );
      void SetData( TVector2Vector const & data );
      void GetData( array<Vector2>^ & data );
      void GetData( TVector2Vector & data );

      // vector3 array accessors
      void SetData( array<Vector3>^ data );
      void SetData( TVector3Vector const & data );
      void GetData( array<Vector3>^ & data );
      void GetData( TVector3Vector & data );

      // vector4 array accessors
      void SetData( array<Vector4>^ data );
      void SetData( TVector4Vector const & data );
      void GetData( array<Vector4>^ & data );
      void GetData( TVector4Vector & data );

      // Quat4 array accessors
      void SetData( array<Quat4>^ data );
      void SetData( TQuat4Vector const & data );
      void GetData( array<Quat4>^ & data );
      void GetData( TQuat4Vector & data );

      // Color array accessors
      void SetData( array<Color>^ data );
      void SetData( TColorVector const & data );
      void GetData( array<Color>^ & data );
      void GetData( TColorVector & data );

      // ColorF array accessors
      void SetData( array<ColorF>^ data );
      void SetData( TColorFVector const & data );
      void GetData( array<ColorF>^ & data );
      void GetData( TColorFVector & data );

      // Guid array accessors
      void SetData( array<Guid>^ data );
      void SetData( TGuidVector const & data );
      void GetData( array<Guid>^ & data );
      void GetData( TGuidVector & data );

      property EType DataType
      {
         EType get() { return (EType) Enum::Parse( EType::typeid, Node->GetAttribute("type"), true ); }
      }

      property int Count
      {
         int get() { return Int32::Parse( Node->GetAttribute("count") ); }
      }

   private:
      DataArray( XmlNode^ node );

      void InternalSetData( void * pData, int count, EType type, int elementSize );
      void InternalGetData( void * pData, int count, EType type, int elementSize );
	};
}

