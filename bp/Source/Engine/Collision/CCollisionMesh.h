//----------------------------------------------------------------------------
// CCollisionMesh.h
// Bluepoint
// Copyright 2005
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/Math/CVector3.h"

//----------------------------------------------------------------------------

namespace Opcode
{
   class Model;
   class MeshInterface;
   class RayCollider;
   struct VertexPointers;
};

//----------------------------------------------------------------------------

class CResId;
struct SFactoryResourceBuildData;
struct SFactoryReturnResource;

class CRay;
class CMatrix34;
class CGameObjectProperties;

//----------------------------------------------------------------------------

class CCollisionMesh
{
public:
   typedef boost::shared_ptr<CGameObjectProperties> TMaterialPtr;
public:
   ENGINE_API explicit CCollisionMesh( std::vector<CVector3> const & points,
                                       std::vector<uint32> const & indices,
                                       std::vector<uint32> const & materialIndices,
                                       std::vector<TMaterialPtr> const & materials );

   ENGINE_API virtual ~CCollisionMesh();

   ENGINE_API static void FFactory(SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource);

   ENGINE_API bool const RayCast( CRay const & ray, 
                                  real32 const maxTime, 
                                  real32 & outTime,
                                  CMatrix34 const * const pWSTransform = NULL) const;

   ENGINE_API static void DataCallback( unsigned int triangleIdx, Opcode::VertexPointers & triangle, void* pUserData );

   // Add accessor functions to keep things clean
   int   GetNumTriangles() const    { return mMaterialIndices.size(); };
   int   GetNumVertices() const     { return mPoints.size(); };
   int   GetIndexStride() const     { return (sizeof(uint32) * 3); };
   int   GetVertexStride() const    { return sizeof(CVector3); };

   real32 const * const GetVertices() const { return GetNumVertices() ? (real32 const * const) &mPoints[0] : NULL; };
   int const * const    GetIndices() const  { return GetNumTriangles() ? (int const * const) &mIndices[0] : NULL; };

public:

   // array of points
   std::vector<CVector3>      mPoints;
   
   // list of triangle indices into points array
   std::vector<uint32>        mIndices;
   
   // list of material indices into the materials array, one index per triangle
   std::vector<uint32>        mMaterialIndices;

   // list of collision materials
   std::vector<TMaterialPtr>  mMaterials;

   std::auto_ptr<Opcode::MeshInterface>   mMeshInterface;
   std::auto_ptr<Opcode::Model>           mModel;
   std::auto_ptr<Opcode::RayCollider>     mRayCollider;

private:
   BPE_DISABLE_COPY_AND_ASSIGNMENT(CCollisionMesh);
};
