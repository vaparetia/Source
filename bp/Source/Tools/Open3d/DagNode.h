//------------------------------------------------------------------------------------------
// DagNode.h
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "NamedEntity.h"

//------------------------------------------------------------------------------------------

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Xml;

//------------------------------------------------------------------------------------------

namespace Open3d
{
   ref class DataArray;
   ref class Submesh;
   ref class Transform;

	public ref class DagNode : public NamedEntity
	{
   public:
      /// identifier for this node type
      static String^ NodeTypeName = "Node";

      /// Attach scene class to given scene node
      static DagNode^ Attach( XmlNode^ node );
      
      /// Create new node with given name
      static DagNode^ Create( XmlDocument^ doc, String^ name );

      /// Adds this node as a child
      void Add( DagNode^ node );
      /// Adds this node as a child
      void Add( Submesh^ node );
      /// Adds this node as a child
      void Add( DataArray^ node );
      /// Adds this node as a child
      void Add( Transform^ node );

      /// Removes this node as a child
      void Remove( Submesh ^node );
      
      property DagNode^ Parent
      {
         DagNode^ get();
      }

      property String^ Path
      {
         String^ get();
      }

      Transform^ GetTransform();

      List<Submesh^>^ GetSubmeshes();

      DataArray^ GetDataArray( String^ arrayName );

   private:
      DagNode( XmlNode^ node );
	};
}

