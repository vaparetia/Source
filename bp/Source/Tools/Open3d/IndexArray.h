//------------------------------------------------------------------------------------------
// IndexArray.h
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
	public ref class IndexArray : public Entity
	{
   public:
      /// identifier for this node type
      static String^ NodeTypeName = "IndexArray";

      /// Attach index array class to given index array node
      static IndexArray^ Attach( XmlNode^ node );

      /// Create new node with given name in given doc
      static IndexArray^ Create( XmlDocument^ doc );

      property array<String^>^ Inputs
      {
         array<String^>^ get()
         {
            return Node->GetAttribute( "input" )->Split( ';' );
         }

         void set( array<String^>^ value )
         {
            Node->SetAttribute( "input", String::Join( ";", value ) );
         }
      }

      void SetInputAttribute( String^ inputs )
      {
         Node->SetAttribute( "input", inputs );
      }

      void SetIndices( array<int> ^ indices )
      {
         pin_ptr<int> ptr = &indices[0];
         InternalSetIndices( ptr, indices->Length );
      }

      void SetIndices( TIntVector const & indices )
      {
         InternalSetIndices( &indices[0], indices.size() );
      }

      void GetIndices( TIntVector & indices );

      property int Count
      {
         int get() { return Int32::Parse( Node->GetAttribute("count") ); }
      }

   private:
      IndexArray( XmlNode^ node );
      void InternalSetIndices( int const * pData, int count );
	};
}


