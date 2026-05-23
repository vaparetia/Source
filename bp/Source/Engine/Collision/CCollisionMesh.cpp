//----------------------------------------------------------------------------
// CCollisionMesh.cpp
// Bluepoint
// Copyright 2005
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CCollisionMesh.h"

//----------------------------------------------------------------------------

#include "TypeConversion/OpcodeConversion.h"

#if BPE_TARGET != BPE_TARGET_DREAMCAST
#include "ExtLibraries/Opcode/Opcode.h"
#endif

#include "Engine/GameObjectSystem/CGameObjectProperties.h"
#include "Engine/Math/CRay.h"
#include "Engine/Resource/CResourceFactory.h"
#include "Engine/Streams/CMemoryInputStream.h"

//----------------------------------------------------------------------------

using namespace std;

//----------------------------------------------------------------------------

CCollisionMesh::CCollisionMesh(std::vector<CVector3> const & points,
                               std::vector<uint32> const & indices,
                               std::vector<uint32> const & materialIndices,
                               std::vector<TMaterialPtr> const & materials)
:  mPoints( points )
,  mIndices( indices )
,  mMaterialIndices(materialIndices)
,  mMaterials(materials)
{
#if BPE_TARGET != BPE_TARGET_DREAMCAST
   // We might actually want to build this on demand, as we're using the physics API at the moment.
   mMeshInterface.reset( new Opcode::MeshInterface() );
   mMeshInterface->SetNbTriangles(GetNumTriangles());
   mMeshInterface->SetNbVertices(GetNumVertices());
   // ODE uses pointers, so we'll work with those for now
   //mMeshInterface->SetCallback( CCollisionMesh::DataCallback, this );
   mMeshInterface->SetPointers((IndexedTriangle const *) GetIndices(), (Point const*) GetVertices());
   mMeshInterface->SetStrides(GetIndexStride(), GetVertexStride());

   Opcode::OPCODECREATE create;

   create.mIMesh = mMeshInterface.get();
   create.mNoLeaf = false;
   create.mQuantized = false;

   mModel.reset( new Opcode::Model() );
   mModel->Build( create );

   mRayCollider.reset( new Opcode::RayCollider() );
   mRayCollider->SetFirstContact( false );
   mRayCollider->SetClosestHit( true );
   mRayCollider->SetCulling( false );
   mRayCollider->SetTemporalCoherence( false );
#endif
}

//----------------------------------------------------------------------------

CCollisionMesh::~CCollisionMesh()
{
}

//----------------------------------------------------------------------------
// Not currently used.
void CCollisionMesh::DataCallback( unsigned int triangleIdx, Opcode::VertexPointers & triangle, void* pUserData )
{
#if BPE_TARGET != BPE_TARGET_DREAMCAST
   CCollisionMesh * pThis = (CCollisionMesh*)pUserData;

   for( int i = 0; i < 3; ++i )
   {
      int const index = pThis->mIndices[triangleIdx*3 + i];
      triangle.Vertex[i] = reinterpret_cast<IceMaths::Point*>(&pThis->mPoints[index]);
   }
#else
   BPE_UNREF(triangleIdx); BPE_UNREF(triangle); BPE_UNREF(pUserData);
#endif
}

//----------------------------------------------------------------------------

void CCollisionMesh::FFactory(SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource)
{
   IGameObjectFactory * const pFactory = reinterpret_cast<IGameObjectFactory *>(buildData.mpFactoryUserData);

   BPE_VERIFY( buildData.mpMemory != NULL, false, "must have memory" );
   CMemoryInputStream stream( buildData.mpMemory, buildData.mSize, CMemoryInputStream::kOwner_App );

   // read and check header
   uint32 const header = stream.ReadUint32();
   BPE_VERIFY( header == 'COLL', false, "invalid file header" );

   // read and check version
   uint32 const version = stream.ReadUint32();
   BPE_VERIFY( version == 1, false, "invalid version, try recooking the collision models." );

   // read materials
   std::vector<TMaterialPtr> materials;
   {
      int const materialCount = stream.ReadInt32();

      materials.reserve(materialCount);

      for( int i = 0; i < materialCount; ++i )
      {
         TMaterialPtr const collisionMaterial( CGameObjectProperties::FGameObjectProperties( stream, pFactory ) );
         materials.push_back(collisionMaterial);
      }
   }

   // read points
   bpe::vector_s<CVector3> points(stream);
   
   // read indices
   bpe::vector_s<uint32> indices(stream);

   // read material indices
   bpe::vector_s<uint32> materialIndices(stream);

   returnResource.mpResource = new CCollisionMesh( points, indices, materialIndices, materials );
}

//----------------------------------------------------------------------------

bool const CCollisionMesh::RayCast( CRay const & ray,
                                    real32 const maxTime,
                                    real32 & outTime,
                                    CMatrix34 const * const pWSTransform /* = NULL */) const
{
#if BPE_TARGET != BPE_TARGET_DREAMCAST
   Ray myRay( Point( ray.GetOrigin().GetX(), ray.GetOrigin().GetY(), ray.GetOrigin().GetZ() ),
              Point( ray.GetDirection().GetX(), ray.GetDirection().GetY(), ray.GetDirection().GetZ() ) );

   Opcode::CollisionFaces collisionFaces;
   mRayCollider->SetDestination( (Opcode::CollisionFaces*) &collisionFaces );

   mRayCollider->SetMaxDist( maxTime );

   bool collideResult;
   if ( pWSTransform )
   {
      Matrix4x4 const mtx = OpcodeConversion::Matrix4x4FromCMatrix34(*pWSTransform);
      collideResult = mRayCollider->Collide( myRay, *mModel, &mtx );
   }
   else
   {
      collideResult = mRayCollider->Collide( myRay, *mModel, NULL );
   }

   if( collideResult )
   {
      if( mRayCollider->GetContactStatus() != 0 )
      {
         outTime = collisionFaces.GetFaces()[0].mDistance;
         return true;
      }
   }

   return false;
#else
   BPE_UNREF(ray); BPE_UNREF(maxTime); BPE_UNREF(outTime); BPE_UNREF(pWSTransform);
   return false;
#endif
}

//----------------------------------------------------------------------------

