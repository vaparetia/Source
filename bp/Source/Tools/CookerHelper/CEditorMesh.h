//------------------------------------------------------------------------------------------
// CEditorMesh.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/Graphics/CColor.h"

#include "Engine/Math/CVector2.h"
#include "Engine/Math/CVector3.h"

#include "CEditorShader.h"
#include "CMergeContainer.h"
#include "Renderer/Base/Primitive/CMeshChunk.h"

//------------------------------------------------------------------------------------------

class CMeshInfo;

//------------------------------------------------------------------------------------------

class CEditorMesh
{
public:
   class Triangle
   {
   public:
      explicit Triangle( int const index1, int const index2, int const index3 )
      :  mIndex1( index1 )
      ,  mIndex2( index2 )
      ,  mIndex3( index3 )
      {
      }
      
      int mIndex1, mIndex2, mIndex3;
   };

   class Line
   {
   public:
      explicit Line(int const index1, int const index2)
         :  mIndex1(index1 > index2 ? index1 : index2)      // Keep fixed ordering
      ,  mIndex2(index1 > index2 ? index2 : index1)
      {
      }
      
      bool operator < (Line const &rhs) const
      {
         if (rhs.mIndex1 < mIndex1)
         {
            return true;
         }
         if (rhs.mIndex1 == mIndex1)
         {
            if (rhs.mIndex2 < mIndex2)
            {
               return true;
            }
         }
         return false;
      }


      int mIndex1, mIndex2;
   };

   class Vertex
   {
   public:
      typedef std::pair<int, real32>   TWeight;
      typedef std::vector<TWeight>     TWeights;

      enum EValidFlags
      {
         kFlag_Position = 1 << 0,
         kFlag_Normal   = 1 << 1,
         kFlag_UvSet0   = 1 << 2,
         kFlag_UvSet1   = 1 << 3,
         kFlag_UvSet2   = 1 << 4,
         kFlag_UvSet3   = 1 << 5,
         kFlag_Color0   = 1 << 6,
         kFlag_Color1   = 1 << 7,
         kFlag_Color2   = 1 << 8,
         kFlag_Weights  = 1 << 9,
         
         kFlag_All = kFlag_Position | kFlag_Normal | kFlag_UvSet0 | kFlag_UvSet1 | kFlag_UvSet2 | kFlag_Color0 | kFlag_Color1 | kFlag_Color2 | kFlag_Weights
      };
   
   public:
      explicit Vertex();

      uint32 const GetHashValue() const;
      bool const operator == ( Vertex const & lhs ) const;

   public:
      uint16    mValidFlags;

      CVector4 mPosition;
      CVector3 mNormal;
      CVector4 mUvSet0, mUvSet1, mUvSet2;
      CVector4 mColor0, mColor1, mColor2;
      TWeights mWeights;

      bool     mJointsRemapped;

      int      mOriginalIndex;
   };

public:
   CEditorMesh() {}

   // This data will take ownership of the from the passed array (this prevents a memory allocation/copy).
   // NOTE: The passed arrays will be empty afterwards.
   void TakeOverData(std::vector< Triangle > & triangles,
                     std::vector< Line > & lines,
                     std::vector< Vertex > & vertices);

   void AddData(std::vector<Triangle> const & triangles,
                std::vector<Line> const & lines,
                std::vector<Vertex> const & vertices);

   std::vector<Triangle> const & GetTriangles() const { return mTriangles; }
   std::vector<Line> const & GetLines() const { return mLines; }
   std::vector<Vertex> const & GetVertices() const { return mVertices; }
   std::vector<Vertex> & Vertices() { return mVertices; }
   
   static void BuildJointRemapTable(std::vector<Triangle> const & triangles, std::vector<Vertex> const & vertices, std::vector<uint32> * remapTable);
   static void RemapJoints(std::vector<uint32> const & remapTable, std::vector<Triangle> const & triangles, std::vector<Vertex> & vertices);

   std::vector<uint32> const & GetJointRemapTable() const { return mJointRemapTable; }

   static void BuildMeshes( std::list<CMeshInfo> const & meshes,
                            std::vector<CMeshChunk> & outChunks,
                            std::vector<uint32> & outIndices,
                            std::vector<uint32> & outTriListIndices,
                            CMergeContainer<Vertex> & outVertices,
                            int const verboseOutputLevel);

public:
   std::vector<Triangle>   mTriangles;
   std::vector<Line>       mLines;
   std::vector<Vertex>     mVertices;
   std::vector<uint32>     mJointRemapTable;
};

//------------------------------------------------------------------------------------------

class CMeshInfo
{
public:
   CMeshInfo()
   :  mAdditionalFlags(0)
   ,  mUnitIdx(0)
   ,  mPacketIdx(0)
   {
   }

   explicit CMeshInfo(uint32 const additionalFlags, int32 const unitIdx, int32 const packetIdx)
   :  mAdditionalFlags(additionalFlags)
   ,  mUnitIdx(unitIdx)
   ,  mPacketIdx(packetIdx)
   {
   }

public:
   uint32      mAdditionalFlags;
   int32       mUnitIdx;
   int32       mPacketIdx;
   CEditorMesh mMeshData;
};
