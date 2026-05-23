//----------------------------------------------------------------------------
// CMesh.h
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"
#include "boost/scoped_ptr.hpp"

#include "Renderer/Base/Primitive/CMeshChunk.h"
#include "Renderer/Base/Material/CShader.h"

#include "Engine/Mechanics/IObject.h"

//----------------------------------------------------------------------------

struct SFactoryResourceBuildData;
struct SFactoryReturnResource;

class CIndexArray;
class CVertexArray;
class CMeshBuffers;

//----------------------------------------------------------------------------

struct SEdgeSegmentInfo
{
   uint32   mVertexColorMin;
   uint32   mVertexColorMax;
   uint32   mAdditionalFlags;
};

struct SEdgeModelHeader
{
   uint32   mHeader;
   uint32   mVersion;
   uint32   mLayerCount;
   uint32   mMaterialCount;
   uint32   mSegmentCount;
   uint32   mSegmentOffset;
   uint32   mSegmentInfoOffset;
   uint32   mSegmentTableOffset;
   uint32   mRsxDataOffset;
   uint32   mRsxDataSize;
   uint32   mpRsxMemory;      // This gets filled in at construction time.
};

class RENDERER_API CMesh : public IObject
{
   friend class CRenderBackend;
public:
   typedef std::vector< boost::shared_ptr<CShader> > TMaterials;
   typedef std::vector<CMeshChunk> TMeshChunks;

public:
   virtual ~CMesh();
   static void DeleteMesh(CMesh* pMesh);

   CMesh(CVertexArray const & vertexArray, 
         CIndexArray const & indexArray, 
         TMeshChunks const & chunks,
         TMaterials const & materials,
         bool const isSkinned,
         int const jointCount);

   std::vector<CMeshChunk> const &     GetMeshChunks() const                                 { return mMeshChunks; }
   boost::shared_ptr<CShader> &        Material(int const index)                             { return mMaterials[index]; }
   boost::shared_ptr<CShader> const &  GetMaterial(int const index) const                    { return mMaterials[index]; }
   int                                 GetNumMaterials() const                               { return mMaterials.size(); }
   CAABox const &                      GetBounds() const                                     { return mBounds; }
   CMeshBuffers const &                GetMeshBuffers() const                                { return *mMeshBuffers; }
   
   void                                GetChunkRangeForUnit(int const unit, int* chunkStart, int* chunkCount) const;
   int                                 GetChunkStartForUnit( int const unit ) const;
   static void                         FMeshFactory(SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource);

   void                                GetMeshChunkRange( SMeshChunkRange *pOutRange, int startIndex, int maxCount ) const;

   int                                 GetJointCount() const { return mJointCount; }
public:
#ifndef GOLD_VERSION
   std::string mDebugName;
#endif

private:
   void                             LinkMeshChunks();

protected:
   boost::scoped_ptr<CMeshBuffers>  mMeshBuffers;
   TMaterials                       mMaterials;
   TMeshChunks                      mMeshChunks;
   CAABox                           mBounds;
   std::vector<uint32>              mMaterialChunkTable;
   int                              mJointCount;
};

