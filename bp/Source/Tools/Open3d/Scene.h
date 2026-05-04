//----------------------------------------------------------------------------
// Scene.h
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Entity.h"

//----------------------------------------------------------------------------

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Xml;

//----------------------------------------------------------------------------

namespace Open3d
{
   ref class DagNode;
   ref class DataArray;
   ref class IndexArray;
   ref class Submesh;
   ref class Transform;
   ref class Material;
   ref class Set;

   public ref class Scene : public Entity
	{
   public:
      /// identifier for this node type
      static String^ NodeTypeName = "Open3d";

      /// Attach scene class to given scene node
      static Scene^ Attach( XmlDocument^ doc );
      static Scene^ Attach( XmlNode^ node );

      /// Create new scene node under the given doc
      static Scene^ Create( XmlDocument^ doc );

      /// Create new node with given name
      DagNode^ CreateDagNode( String^ name );
      /// Create new submesh
      Submesh^ CreateSubmesh();
      /// Create new index array
      IndexArray^ CreateIndexArray();
      /// Create new data array
      DataArray^ CreateDataArray( String^ name );
      /// Create new transform
      Transform^ CreateTransform();
      /// Create new material
      Material^ CreateMaterial( String^ name );
      /// Create new set
      Set^ CreateSet( String^ name );
      
      /// Adds this node as a child
      void Add( DagNode^ node );

      /// Adds this material to the scene
      void Add( Material^ material );

      /// Adds this set to the scene
      void Add( Set^ material );

      // Removes the following from a scene
      void Remove( Material ^material );
      void Remove( Set ^node );
      void Remove( DagNode ^node );

      List<DagNode^>^ GetMeshes();

      bool HasMaterial( String^ materialName );
      Material^ GetMaterial( String^ materialName );
      List<Material^> ^ GetMaterials();

      bool HasSet( String^ setName );
      Set^ GetSet( String^ setName );

      DagNode^ GetNodeByPath( String^ path );

   private:
      Scene( XmlNode^ node );

      void RemoveChild( XmlElement ^xmlnode );
   };
}
