//----------------------------------------------------------------------------
// ModelCooker.cpp
//----------------------------------------------------------------------------

#include "stdafx.h"
#include < vcclr.h >

#include "AssetToolMPP.h"

//----------------------------------------------------------------------------

#include "Renderer/Base/Primitive/CVertexData.h"

#include "Engine/Animation/CAnimationEventState.h"
#include "Engine/Basics/CStringExtras.h"
#include "Engine/Graphics/CIndexArray.h"
#include "Engine/Math/CMatrix34.h"
#include "Engine/Math/CMatrix4.h"
#include "Engine/Math/CloseEnough.h"
#include "Engine/Primitives/CPlane.h"
#include "Engine/Streams/CDiskOutputStream.h"
#include "Engine/Streams/CLabeledOffsetStream.h"

#include "Tools/CookerHelper/CEditorMesh.h"
#include "Tools/CookerHelper/CMeshUtils.h"

#include "Tools/Helper/Helper.h"
#include "Tools/Open3d/STLSupport.h"

#include "Engine/Streams/CGrowableMemoryOutStream.h"

#include "Edge/libedgegeomtool/libedgegeomtool_wrap.h"
#include "Edge/libedgegeomtool/libedgegeomtool.h"

//----------------------------------------------------------------------------

using namespace Open3d;
using namespace System::Collections::Generic;
using namespace System::Xml;
using namespace System::Xml::XPath;
using namespace System;
using namespace Tools::Property::Build;
using namespace Tools::Property::Cook;
using namespace Tools::Property::Misc;
using namespace Tools::Property::Source;
using namespace Tools;

//----------------------------------------------------------------------------

namespace
{
   template<class T> void GetDataArray(DagNode^ node, T & output, String^ name, String^ fallbackName)
   {
      DataArray^ dataArray = node->GetDataArray(name);
      
      if( dataArray == nullptr && !String::IsNullOrEmpty(fallbackName) )
         dataArray = node->GetDataArray(fallbackName);

      if( dataArray != nullptr )
         dataArray->GetData(output);
      else
         output.clear();
   }

   void GetUVDataArray(DagNode^ node, TVector4Vector & output, String^ name, String^ fallbackName)
   {
      DataArray^ dataArray = node->GetDataArray(name);

      if( dataArray == nullptr && !String::IsNullOrEmpty(fallbackName) )
         dataArray = node->GetDataArray(fallbackName);

      if( dataArray != nullptr )
      {
         if( dataArray->DataType == Open3d::DataArray::EType::Vector2)
         {
            TVector2Vector uvs;
            dataArray->GetData(uvs);

            output.clear();
            output.reserve(uvs.size());

            for( int i = 0; i < uvs.size(); ++i )
            {
               CVector2 const uv = uvs[i];
               output.push_back(CVector4(uv.mX, uv.mY, 0, 0));
            }
         }
         else
         {
            // Assume this is a CVector4 array.
            dataArray->GetData(output);
         }
      }
      else
         output.clear();
   }

   void GetDataArrays(DagNode^ node, 
                      TVector4Vector & points, 
                      TVector3Vector & normals, 
                      TVector4Vector & uv0, TVector4Vector & uv1, TVector4Vector & uv2,
                      TVector4Vector & color0, TVector4Vector & color1, TVector4Vector & color2,
                      TColorVector & skinIndex, TVector4Vector & skinWeight)
   {
      GetDataArray(node, points, "Points", "");
      GetDataArray(node, normals, "Normals", "");

      GetUVDataArray(node, uv0, "UV0", "UV");
      GetUVDataArray(node, uv1, "UV1", "");
      GetUVDataArray(node, uv2, "UV2", "");

      GetDataArray(node, color0, "VertColor0", "Color1");
      GetDataArray(node, color1, "VertColor1", "");
      GetDataArray(node, color2, "VertColor2", "");

      GetDataArray(node, skinIndex, "SkinIndex", "");
      GetDataArray(node, skinWeight, "SkinWeight", "");
   }

   void GetIndexArray(Submesh^ subMesh, TIntVector & output, String^ name, String^ fallbackName)
   {
      IndexArray^ indexArray = subMesh->GetIndexArray(name);
      if(
         indexArray == nullptr && !String::IsNullOrEmpty(fallbackName) )
         indexArray = subMesh->GetIndexArray(fallbackName);

      if( indexArray != nullptr )
         indexArray->GetIndices(output);
      else
         output.clear();
   }

   void GetIndexArrays(Submesh^ subMesh, 
                       TIntVector & points, 
                       TIntVector & normals, 
                       TIntVector & uv0, TIntVector & uv1, TIntVector & uv2,
                       TIntVector & color0, TIntVector & color1, TIntVector & color2,
                       TIntVector & skinIndexIndices, TIntVector & skinWeightIndices )
   {
      GetIndexArray(subMesh, points, "Points", "");
      GetIndexArray(subMesh, normals, "Normals", "");

      GetIndexArray(subMesh, uv0, "UV0", "UV");
      GetIndexArray(subMesh, uv1, "UV1", "");
      GetIndexArray(subMesh, uv2, "UV2", "");

      GetIndexArray(subMesh, color0, "VertColor0", "Color1");
      GetIndexArray(subMesh, color1, "VertColor1", "");
      GetIndexArray(subMesh, color2, "VertColor2", "");

      GetIndexArray(subMesh, skinIndexIndices, "SkinIndex", "");
      GetIndexArray(subMesh, skinWeightIndices, "SkinWeight", "");
   }

   struct SVertexSortPred
   {
      bool operator()(CEditorMesh::Vertex::TWeight const & lhs, CEditorMesh::Vertex::TWeight const & rhs)
      {
         return lhs.second > rhs.second;
      }
   };

   void ExtractVertexData( CEditorMesh::Vertex & vertex, 
                           TVector4Vector const & points, 
                           TVector3Vector const & normals, 
                           TVector4Vector const & uv0, TVector4Vector const &uv1, TVector4Vector const & uv2, 
                           TVector4Vector const & color0, TVector4Vector const & color1, TVector4Vector const & color2, 
                           TColorVector const & skinIndices, TVector4Vector const & skinWeights,
                           int const pointIndex, int const normalIndex, int const uv0Index, int const uv1Index, int const uv2Index, int const color0Index, int const color1Index, int const color2Index, int const skinIndexIndex, int const skinWeightIndex)
   {
      if( !points.empty() && pointIndex != -1 )
      {
         vertex.mPosition = points[pointIndex];
         vertex.mValidFlags |= CEditorMesh::Vertex::kFlag_Position;
      }

      if( !normals.empty() && normalIndex != -1 )
      {
         vertex.mNormal = normals[normalIndex];
         vertex.mValidFlags |= CEditorMesh::Vertex::kFlag_Normal;
      }

      if( !uv0.empty() && uv0Index != -1 )
      {
         vertex.mUvSet0 = uv0[uv0Index];
         vertex.mValidFlags |= CEditorMesh::Vertex::kFlag_UvSet0;
      }

      if( !uv1.empty() && uv1Index != -1 )
      {
         vertex.mUvSet1 = uv1[uv1Index];
         vertex.mValidFlags |= CEditorMesh::Vertex::kFlag_UvSet1;
      }

      if( !uv2.empty() && uv2Index != -1 )
      {
         vertex.mUvSet2 = uv2[uv2Index];
         vertex.mValidFlags |= CEditorMesh::Vertex::kFlag_UvSet2;
      }

      if( !color0.empty() && color0Index != -1 )
      {
         vertex.mColor0 = color0[color0Index];
         vertex.mValidFlags |= CEditorMesh::Vertex::kFlag_Color0;
      }

      if( !color1.empty() && color1Index != -1 )
      {
         vertex.mColor1 = color1[color1Index];
         vertex.mValidFlags |= CEditorMesh::Vertex::kFlag_Color1;
      }

      if( !color2.empty() && color2Index != -1 )
      {
         vertex.mColor2 = color2[color2Index];
         vertex.mValidFlags |= CEditorMesh::Vertex::kFlag_Color2;
      }

      if( !skinIndices.empty() && skinIndexIndex != -1 && !skinWeights.empty() && skinWeightIndex != -1)
      {
         CColor const skinIndex = skinIndices[skinIndexIndex];
         CVector4 const skinWeight = skinWeights[skinWeightIndex];

         if(skinWeight.GetX() > 0.0f )
            vertex.mWeights.push_back(CEditorMesh::Vertex::TWeight(skinIndex.GetR(), skinWeight.GetX()));

         if(skinWeight.GetY() > 0.0f )
            vertex.mWeights.push_back(CEditorMesh::Vertex::TWeight(skinIndex.GetG(), skinWeight.GetY()));

         if(skinWeight.GetZ() > 0.0f )
            vertex.mWeights.push_back(CEditorMesh::Vertex::TWeight(skinIndex.GetB(), skinWeight.GetZ()));

         if(skinWeight.GetW() > 0.0f )
            vertex.mWeights.push_back(CEditorMesh::Vertex::TWeight(skinIndex.GetA(), skinWeight.GetW()));

         if( !vertex.mWeights.empty() )
         {
            std::sort(vertex.mWeights.begin(), vertex.mWeights.end(), SVertexSortPred());
         }

         vertex.mValidFlags |= CEditorMesh::Vertex::kFlag_Weights;
      }
   }

   uint64 const kAF_MeshClipU = 1ULL << 32;
   uint64 const kAF_MeshClipV = 1ULL << 33;
   uint64 const kAF_FixWinding = 1ULL << 34;
   uint64 const kAF_IgnoreMaterial = 1ULL << 35;

   void GetAdditionalFlagsForNode(XmlNode^ node, uint64* pValue)
   {
      uint64 mask = 0;
      uint64 value = 0;

      if( node != nullptr )
      {
         XmlNode^ enableCullingNode = node->SelectSingleNode(".//EnableCulling");
         if( enableCullingNode != nullptr )
         {
            mask |= CMeshChunk::kAF_EnableCulling;
            value |= Tools::Common::Misc::ParseBool(enableCullingNode->InnerText) ? CMeshChunk::kAF_EnableCulling : 0;
         }

         XmlNode^ meshUVClip = node->SelectSingleNode(".//MeshUVClip");
         if( meshUVClip != nullptr )
         {
            mask |= kAF_MeshClipU|kAF_MeshClipV;
            value |= Tools::Common::Misc::ParseBool(meshUVClip->InnerText) ? (kAF_MeshClipU|kAF_MeshClipV) : 0;
         }

         XmlNode^ fixWinding = node->SelectSingleNode(".//FixWinding");
         if( fixWinding != nullptr )
         {
            mask |= kAF_FixWinding;
            value |= Tools::Common::Misc::ParseBool(fixWinding->InnerText) ? kAF_FixWinding : 0;
         }

      }

      *pValue = *pValue & (~mask) | value;
   }

   uint64 GetAdditionalFlags(XmlDocument^ globalInfo, XmlDocument^ modelInfo, Scene^ scene, Material^ material, int const submeshIdx)
   {
      uint64 additionalFlags = 0;

      if( Tools::Common::Misc::ParseBool(material->Node->GetAttribute("ignore"), false) )
      {
         additionalFlags |= kAF_IgnoreMaterial;
      }

      int texLayerCount = 0;
      Int32::TryParse(material->Node->GetAttribute("layerCount"), texLayerCount);

      // Global overrides
      if( globalInfo != nullptr )
      {
         XmlNode^ globalNode = globalInfo->SelectSingleNode("//Global");
         GetAdditionalFlagsForNode(globalNode, &additionalFlags);

         for( int layerIdx = 0; layerIdx < texLayerCount; ++layerIdx )
         {
            XmlElement^ layerElement = (XmlElement^)material->Node->SelectSingleNode(String::Format("./TexLayer[@index='{0}']", layerIdx));
            if( layerElement != nullptr )
            {
               XmlNode^ textureNode = globalInfo->SelectSingleNode(String::Format("//Texture[@name='{0}']", layerElement->GetAttribute("texture")));
               GetAdditionalFlagsForNode(textureNode, &additionalFlags);
            }
         }

         XmlNode^ materialNode = globalInfo->SelectSingleNode(String::Format("//Material[@name='{0}']", material->Name));
         GetAdditionalFlagsForNode(materialNode, &additionalFlags);
      }

      // Per model overrides
      if( modelInfo != nullptr )
      {
         XmlNode^ globalNode = modelInfo->SelectSingleNode("//Global");
         GetAdditionalFlagsForNode(globalNode, &additionalFlags);

         for( int layerIdx = 0; layerIdx < texLayerCount; ++layerIdx )
         {
            XmlElement^ layerElement = (XmlElement^)material->Node->SelectSingleNode(String::Format("./TexLayer[@index='{0}']", layerIdx));
            if( layerElement != nullptr )
            {
               XmlNode^ textureNode = modelInfo->SelectSingleNode(String::Format("//Texture[@name='{0}']", layerElement->GetAttribute("texture")));
               GetAdditionalFlagsForNode(textureNode, &additionalFlags);
            }
         }

         XmlNode^ materialNode = modelInfo->SelectSingleNode(String::Format("//Material[@name='{0}']", material->Name));
         GetAdditionalFlagsForNode(materialNode, &additionalFlags);

         XmlNode^ chunkNode = modelInfo->SelectSingleNode(String::Format("//Chunk[@index='{0}']", submeshIdx));
         GetAdditionalFlagsForNode(chunkNode, &additionalFlags);
      }

      // Validate clamp flags against surface clamp flags
      if( additionalFlags & (kAF_MeshClipU|kAF_MeshClipV) )
      {
         int surfaceClampU = 0;
         int surfaceClampV = 0;

         // Check how many clamp u/v flags are set for all the texture layers
         for( int layerIdx = 0; layerIdx < texLayerCount; ++layerIdx )
         {
            XmlElement^ layerElement = (XmlElement^)material->Node->SelectSingleNode(String::Format("./TexLayer[@index='{0}']", layerIdx));
            if( layerElement != nullptr )
            {
               if( Tools::Common::Misc::ParseBool(layerElement->GetAttribute("clampU"), false) )
                  surfaceClampU++;

               if( Tools::Common::Misc::ParseBool(layerElement->GetAttribute("clampV"), false) )
                  surfaceClampV++;
            }
         }

         // For now just support single texture layers!

         // Remove clampU flag if the clamp flag of the material disallows it
         if( texLayerCount != 1 || !surfaceClampU )
            additionalFlags &= ~kAF_MeshClipU;

         // Remove clampV flag if the clamp flag of the material disallows it
         if( texLayerCount != 1 || !surfaceClampV )
            additionalFlags &= ~kAF_MeshClipV;

      }

      return additionalFlags;
   }

   CEditorMesh::Vertex const ScaleVertex(CEditorMesh::Vertex const & in, real32 const scale)
   {
      CEditorMesh::Vertex result;
      result.mValidFlags = in.mValidFlags;

      result.mPosition = in.mPosition * scale;
      result.mNormal = in.mNormal * scale;
      result.mUvSet0 = in.mUvSet0 * scale;
      result.mUvSet1 = in.mUvSet1 * scale;
      result.mUvSet2 = in.mUvSet2 * scale;
      result.mColor0 = in.mColor0 * scale;
      result.mColor1 = in.mColor1 * scale;
      result.mColor2 = in.mColor2 * scale;
      
      return result;
   }

   CEditorMesh::Vertex const AddVertex(CEditorMesh::Vertex const & lhs, CEditorMesh::Vertex const & rhs)
   {
      CEditorMesh::Vertex result;
      result.mValidFlags = lhs.mValidFlags & rhs.mValidFlags;
      result.mPosition = lhs.mPosition + rhs.mPosition;
      result.mNormal = lhs.mNormal + rhs.mNormal;
      result.mUvSet0 = lhs.mUvSet0 + rhs.mUvSet0;
      result.mUvSet1 = lhs.mUvSet1 + rhs.mUvSet1;
      result.mUvSet2 = lhs.mUvSet2 + rhs.mUvSet2;
      result.mColor0 = lhs.mColor0 + rhs.mColor0;
      result.mColor1 = lhs.mColor1 + rhs.mColor1;
      result.mColor2 = lhs.mColor2 + rhs.mColor2;
      
      return result;
   }

   void ClipPolygon(std::vector<CVector2> const & in, std::vector<CVector2> & out, CVector2 const & planeNormal, real32 const planeDist)
   {
      out.clear();

      if( in.empty() )
         return;

      CVector2 prev = in[in.size() - 1];

      for( int i = 0; i < in.size(); ++i )
      {
         CVector2 const current = in[i];
         
         int const sideA = (CVector2::Dot(planeNormal, prev) >= planeDist) ? 1 : 0;
         int const sideB = (CVector2::Dot(planeNormal, current) >= planeDist) ? 1 : 0;

         if( sideA ^ sideB )
         {
            // prev/current on different sides

            CVector2 l = current - prev;
            l.Normalize();
            
            CVector2 const p0 = planeNormal * planeDist;

            real32 const d = CVector2::Dot( p0 - prev, planeNormal) / CVector2::Dot(l, planeNormal);

            CVector2 const p = prev + l * d;

            out.push_back(p);

            if( sideB )
               out.push_back(current);
         }
         else
         {
            // prev/current on same side
            if( sideA )
               out.push_back(current);

         }

         prev = current;
      }
   }

   // Realtime Collision Detection page 47
   void CalculateBarycentric(CVector2 const & a, CVector2 const & b, CVector2 const & c, CVector2 const & p, real32 & u, real32 & v, real32 & w)
   {
      CVector2 const v0 = b - a;
      CVector2 const v1 = c - a;
      CVector2 const v2 = p - a;

      real32 const d00 = CVector2::Dot(v0, v0);
      real32 const d01 = CVector2::Dot(v0, v1);
      real32 const d11 = CVector2::Dot(v1, v1);
      real32 const d20 = CVector2::Dot(v2, v0);
      real32 const d21 = CVector2::Dot(v2, v1);

      float const denom = d00 * d11 - d01 * d01;

      v = (d11 * d20 - d01 * d21) / denom;
      w = (d00 * d21 - d01 * d20) / denom;
      u = 1.0f - v - w;
   }

   void PerformUVClipping(std::vector<CEditorMesh::Vertex> * pVertices, std::vector<CEditorMesh::Triangle>* pTriangles, bool clipU, bool clipV)
   {
      // nothing to do, done!
      if( !clipU && !clipV )
         return;

      std::vector<CEditorMesh::Vertex> newVerts;
      std::vector<CEditorMesh::Triangle> newTris;

      std::vector<CVector2> points;
      points.reserve(8);

      std::vector<CVector2> clippedPoints;
      clippedPoints.reserve(8);

      for( int triIdx = 0; triIdx < pTriangles->size(); ++triIdx )
      {
         CEditorMesh::Triangle const & tri = (*pTriangles)[triIdx];

         CEditorMesh::Vertex const vtxA = (*pVertices)[tri.mIndex1];
         CEditorMesh::Vertex const vtxB = (*pVertices)[tri.mIndex2];
         CEditorMesh::Vertex const vtxC = (*pVertices)[tri.mIndex3];

         // Skip degenerate input triangles
         {
            CVector3 const edgeA(vtxB.mPosition.DropW() - vtxA.mPosition.DropW());
            CVector3 const edgeB(vtxC.mPosition.DropW() - vtxB.mPosition.DropW());
            CVector3 const edgeC(vtxA.mPosition.DropW() - vtxC.mPosition.DropW());
            real32 const longestEdge = bpe::max_val(edgeA.GetLength(), bpe::max_val(edgeB.GetLength(), edgeC.GetLength()));
            real32 const relativeArea = CVector3::Cross(edgeA, edgeB).GetLength() / longestEdge;
            real32 const kEpsilon = 1e-5f;

            if (relativeArea < kEpsilon)
               continue;
         }

         CVector2 const pntA = vtxA.mUvSet0.GetXY();
         CVector2 const pntB = vtxB.mUvSet0.GetXY();
         CVector2 const pntC = vtxC.mUvSet0.GetXY();

         // Don't attempt to clip degenerate triangles (in uv space)
         real32 const area = fabsf(CVector2::Cross(pntB - pntA, pntC - pntA));
         if( area > gkEpsilon32 )
         {
            // start with original triangle UVs
            points.clear();
            points.push_back(pntA);
            points.push_back(pntB);
            points.push_back(pntC);

            // clip against U?
            if( clipU )
            {
               // clip against minU
               ClipPolygon(points, clippedPoints, CVector2::XAxis(), 0.0f);
               points.swap(clippedPoints);

               // clip against maxU
               ClipPolygon(points, clippedPoints, CVector2::NegXAxis(), -1.0f);
               points.swap(clippedPoints);
            }

            // clip against V?
            if( clipV )
            {
               // clip against minV
               ClipPolygon(points, clippedPoints, CVector2::YAxis(), 0.0f);
               points.swap(clippedPoints);

               // clip against maxV
               ClipPolygon(points, clippedPoints, CVector2::NegYAxis(), -1.0f);
               points.swap(clippedPoints);
            }
         }

         // Output triangle fan for polygon formed by points
         int const startIndex = newVerts.size();

         for( int pntIdx = 0; pntIdx < points.size(); ++pntIdx )
         {
            CVector2 const p = points[pntIdx];

            // Calculate barycentric coordinates to interpolate proper vertex data from original triangle verts
            real32 u, v, w;
            CalculateBarycentric(pntA, pntB, pntC, p, u, v, w);

            CEditorMesh::Vertex const vtx = AddVertex( ScaleVertex(vtxA, u), AddVertex(ScaleVertex(vtxB, v), ScaleVertex(vtxC, w)));
            
            newVerts.push_back(vtx);
         }

         for( int outTriIdx = 0; outTriIdx < (int)points.size() - 2; ++outTriIdx )
         {
            int const idxA = startIndex;
            int const idxB = startIndex + outTriIdx + 1;
            int const idxC = startIndex + outTriIdx + 2;

            // Don't export degenerate triangle
            {
               CEditorMesh::Vertex const vtxA = newVerts[idxA];
               CEditorMesh::Vertex const vtxB = newVerts[idxB];
               CEditorMesh::Vertex const vtxC = newVerts[idxC];

               CVector3 const edgeA(vtxB.mPosition.DropW() - vtxA.mPosition.DropW());
               CVector3 const edgeB(vtxC.mPosition.DropW() - vtxB.mPosition.DropW());
               CVector3 const edgeC(vtxA.mPosition.DropW() - vtxC.mPosition.DropW());
               real32 const longestEdge = bpe::max_val(edgeA.GetLength(), bpe::max_val(edgeB.GetLength(), edgeC.GetLength()));
               real32 const relativeArea = CVector3::Cross(edgeA, edgeB).GetLength() / longestEdge;
               real32 const kEpsilon = 1e-5f;

               if (relativeArea < kEpsilon)
                  continue;
            }

            newTris.push_back(CEditorMesh::Triangle(idxA, idxB, idxC));
         }
      }

      *pVertices = newVerts;
      *pTriangles = newTris;
   }

   void RemoveDegenerates(std::vector<CEditorMesh::Vertex> const & vertices, std::vector<CEditorMesh::Triangle> * pTriangles)
   {
      std::vector<CEditorMesh::Triangle> newTris;
      newTris.reserve(pTriangles->size());

      for( int i = 0; i < pTriangles->size(); ++i )
      {
         CEditorMesh::Triangle & tri = (*pTriangles)[i];

         CEditorMesh::Vertex const & v0 = vertices[tri.mIndex1];
         CEditorMesh::Vertex const & v1 = vertices[tri.mIndex2];
         CEditorMesh::Vertex const & v2 = vertices[tri.mIndex3];

         CVector3 d01 = v1.mPosition.DropW() - v0.mPosition.DropW();
         CVector3 d02 = v2.mPosition.DropW() - v0.mPosition.DropW();

         if( d01.TryNormalize() == 0.0f )
            continue;

         if( d02.TryNormalize() == 0.0f )
            continue;

         newTris.push_back(tri);
      }

      *pTriangles = newTris;
   }

   void PerformFixWinding(std::vector<CEditorMesh::Vertex> const & vertices, std::vector<CEditorMesh::Triangle> & triangles)
   {
      // no verts!
      if( vertices.empty() )
         return;

      // no normals!
      if( (vertices[0].mValidFlags & CEditorMesh::Vertex::kFlag_Normal) == 0 )
         return;

      for( int i = 0; i < triangles.size(); ++i )
      {
         CEditorMesh::Triangle & tri = triangles[i];

         CEditorMesh::Vertex const & v0 = vertices[tri.mIndex1];
         CEditorMesh::Vertex const & v1 = vertices[tri.mIndex2];
         CEditorMesh::Vertex const & v2 = vertices[tri.mIndex3];

         CVector3 d01 = v1.mPosition.DropW() - v0.mPosition.DropW();
         CVector3 d02 = v2.mPosition.DropW() - v0.mPosition.DropW();
         
         if( d01.TryNormalize() == 0.0f )
            continue;

         if( d02.TryNormalize() == 0.0f )
            continue;

         CVector3 const faceNormal = CVector3::Cross(d01, d02);

         int matchingDirCount = 0;

         matchingDirCount += (CVector3::Dot(faceNormal, v0.mNormal) >= 0.0f) ? 1 : 0;
         matchingDirCount += (CVector3::Dot(faceNormal, v1.mNormal) >= 0.0f) ? 1 : 0;
         matchingDirCount += (CVector3::Dot(faceNormal, v2.mNormal) >= 0.0f) ? 1 : 0;

         if( matchingDirCount == 0 )
         {
            std::swap(tri.mIndex2, tri.mIndex3);
         }
      }
   }

   void write_obj(System::String^ outputPath, bool append, System::String^ objName, std::vector<CEditorMesh::Triangle> const & tris, std::vector<CEditorMesh::Vertex> const & verts)
   {
      System::IO::TextWriter^ writer = gcnew System::IO::StreamWriter(outputPath, append);

      writer->WriteLine("g {0}", objName);

      for( int i = 0; i < tris.size(); ++i )
      {
         CEditorMesh::Triangle const & tri = tris[i];
         CEditorMesh::Vertex const & vtx0 = verts[tri.mIndex1];
         CEditorMesh::Vertex const & vtx1 = verts[tri.mIndex2];
         CEditorMesh::Vertex const & vtx2 = verts[tri.mIndex3];
         writer->WriteLine("v {0} {1} {2}", vtx0.mPosition[0], vtx0.mPosition[1], vtx0.mPosition[2]);
         writer->WriteLine("vt {0} {1}", vtx0.mUvSet0[0], vtx0.mUvSet0[1]);
         writer->WriteLine("v {0} {1} {2}", vtx1.mPosition[0], vtx1.mPosition[1], vtx1.mPosition[2]);
         writer->WriteLine("vt {0} {1}", vtx1.mUvSet0[0], vtx1.mUvSet0[1]);
         writer->WriteLine("v {0} {1} {2}", vtx2.mPosition[0], vtx2.mPosition[1], vtx2.mPosition[2]);
         writer->WriteLine("vt {0} {1}", vtx2.mUvSet0[0], vtx2.mUvSet0[1]);
         writer->WriteLine("f -3/-3 -2/-2 -1/-1");
      }

      writer->Close();
   }

   void GenerateEdgeSegmentFormat( EdgeGeomScene const & edgeScene, int const isSkinned, int const hasUvs, int const hasColor, int const hasNormal, EdgeGeomSegmentFormat & edgeFormat ) 
   {
      edgeFormat.m_spuInputVertexFormats[0] = edgeGeomGetSpuVertexFormat(EDGE_GEOM_SPU_VERTEX_FORMAT_F32c3);

      // Generate secondary SPU input vertex format, if necessary.  
      // The secondary SPU input stream will only contain normals (i.e. attributes that need to be processed by Edge).
      // EXCEPTION: For static scenes, these attributes are not needed on the SPU and are included in the RSX-only stream below.

      if (isSkinned && hasNormal)
      {
         EdgeGeomSpuVertexFormat *format = (EdgeGeomSpuVertexFormat*)new char[sizeof(EdgeGeomSpuVertexFormat)];
         memset(format, 0, sizeof(EdgeGeomSpuVertexFormat));

         edgeFormat.m_spuInputVertexFormats[1] = format;

         for(uint8_t iAttr=0; iAttr < edgeScene.m_numVertexAttributes; ++iAttr)
         {
            if (edgeScene.m_vertexAttributeIds[iAttr] != EDGE_GEOM_ATTRIBUTE_ID_NORMAL )
               continue;

            format->m_attributeDefinition[format->m_numAttributes].m_attributeId = edgeScene.m_vertexAttributeIds[iAttr];
            format->m_attributeDefinition[format->m_numAttributes].m_byteOffset = format->m_vertexStride;

            format->m_attributeDefinition[format->m_numAttributes].m_type = kSpuAttr_X11Y11Z10N;
            format->m_attributeDefinition[format->m_numAttributes].m_count = 1;
            format->m_vertexStride += 4;

            format->m_numAttributes++;
         }
      }

      // Generate RSX-only vertex format, if necessary.  It contains any remaining attributes that Edge
      // doesn't care about, such as texture coordinates.
      // EXCEPTION: For static geometry, all non-position attributes go into the RSX-only stream.
      if (hasUvs || hasColor || (!isSkinned && hasNormal) )
      {
         EdgeGeomRsxVertexFormat *format = (EdgeGeomRsxVertexFormat*)edgeGeomAlloc(sizeof(EdgeGeomRsxVertexFormat));
         memset(format, 0, sizeof(EdgeGeomRsxVertexFormat));
         
         edgeFormat.m_rsxOnlyVertexFormat = format;
         
         for(uint8_t iAttr=0; iAttr<edgeScene.m_numVertexAttributes; ++iAttr)
         {
            // Skip position always, since it's part of the first input stream.
            if (edgeScene.m_vertexAttributeIds[iAttr] == EDGE_GEOM_ATTRIBUTE_ID_POSITION)
               continue;

            // Skip normals if this is a skinned mesh.
            if (isSkinned && edgeScene.m_vertexAttributeIds[iAttr] == EDGE_GEOM_ATTRIBUTE_ID_NORMAL )
               continue;

            format->m_attributeDefinition[format->m_numAttributes].m_attributeId = edgeScene.m_vertexAttributeIds[iAttr];
            format->m_attributeDefinition[format->m_numAttributes].m_byteOffset = format->m_vertexStride;

            // Choose an appropriate type for this attribute, based on its ID.
            // This is difficult to generalize, so we err on the side of safety.
            switch(format->m_attributeDefinition[format->m_numAttributes].m_attributeId)
            {
            case EDGE_GEOM_ATTRIBUTE_ID_NORMAL:
               format->m_attributeDefinition[format->m_numAttributes].m_type = kRsxAttr_X11Y11Z10N;
               format->m_attributeDefinition[format->m_numAttributes].m_count = 1;
               format->m_vertexStride += 4;
               break;

            case EDGE_GEOM_ATTRIBUTE_ID_COLOR:
               format->m_attributeDefinition[format->m_numAttributes].m_type = kRsxAttr_F16;
               format->m_attributeDefinition[format->m_numAttributes].m_count = 4;
               format->m_vertexStride += 4 * sizeof(uint16_t);
               break;

            case EDGE_GEOM_ATTRIBUTE_ID_UV0:
            case EDGE_GEOM_ATTRIBUTE_ID_UV1:
            case EDGE_GEOM_ATTRIBUTE_ID_UV2:
            case EDGE_GEOM_ATTRIBUTE_ID_UV3:
               format->m_attributeDefinition[format->m_numAttributes].m_type = kRsxAttr_F16;
               format->m_attributeDefinition[format->m_numAttributes].m_count = 2; // assume two-component UV pairs
               format->m_vertexStride += 2 * sizeof(uint16_t);
               break;

            default:
               break;
            }

            format->m_numAttributes++;
         }		
      }

      // Generate output vertex format
      {
         EdgeGeomRsxVertexFormat *format = (EdgeGeomRsxVertexFormat*)edgeGeomAlloc(sizeof(EdgeGeomRsxVertexFormat));
         memset(format, 0, sizeof(EdgeGeomRsxVertexFormat));

         edgeFormat.m_spuOutputVertexFormat = format;

         for(uint8_t iAttr=0; iAttr<edgeScene.m_numVertexAttributes; ++iAttr)
         {
            bool storeAttribute = false;

            if( edgeScene.m_vertexAttributeIds[iAttr] == EDGE_GEOM_ATTRIBUTE_ID_POSITION )
               storeAttribute = true;
            
            if( edgeScene.m_vertexAttributeIds[iAttr] == EDGE_GEOM_ATTRIBUTE_ID_NORMAL && isSkinned )
               storeAttribute = true;

            if( !storeAttribute )
               continue;

            format->m_attributeDefinition[format->m_numAttributes].m_attributeId = edgeScene.m_vertexAttributeIds[iAttr];
            format->m_attributeDefinition[format->m_numAttributes].m_byteOffset = format->m_vertexStride;

            // Choose an appropriate type for this attribute, based on its ID.
            // This is difficult to generalize, so we err on the side of safety.
            switch(format->m_attributeDefinition[format->m_numAttributes].m_attributeId)
            {
            case EDGE_GEOM_ATTRIBUTE_ID_POSITION:
               format->m_attributeDefinition[format->m_numAttributes].m_type = kRsxAttr_F32;
               format->m_attributeDefinition[format->m_numAttributes].m_count = 3;
               format->m_vertexStride += 3 * sizeof(float);
               break;

            case EDGE_GEOM_ATTRIBUTE_ID_NORMAL:
               format->m_attributeDefinition[format->m_numAttributes].m_type = kRsxAttr_X11Y11Z10N;
               format->m_attributeDefinition[format->m_numAttributes].m_count = 1;
               format->m_vertexStride += 4;
               break;
            
            default:
               break;
            }

            format->m_numAttributes++;
         }
      }
   }
   
   struct EdgeSegmentInfo
   {
      bool operator == (EdgeSegmentInfo const & rhs) const
      {
         return mMaterialId == rhs.mMaterialId &&
                mLayerNumber == rhs.mLayerNumber &&
                mAdditionalFlags == rhs.mAdditionalFlags;
      }

      uint32   mMaterialId;
      uint32   mLayerNumber;
      uint32   mAdditionalFlags;
   };

   int GetOrAddEdgeSegmentInfo(CMeshInfo const & info, std::vector<EdgeSegmentInfo> & outEdgeSegmentInfos)
   {
      EdgeSegmentInfo segmentInfo;
      //segmentInfo.mMaterialId = info.mMaterialIndex;
      //segmentInfo.mLayerNumber = info.mLayerNumber;
      segmentInfo.mAdditionalFlags = info.mAdditionalFlags;

      std::vector<EdgeSegmentInfo>::const_iterator found = std::find(outEdgeSegmentInfos.begin(), outEdgeSegmentInfos.end(), segmentInfo);

      if( found != outEdgeSegmentInfos.end() )
         return (found - outEdgeSegmentInfos.begin());

      outEdgeSegmentInfos.push_back(segmentInfo);
      return outEdgeSegmentInfos.size() - 1;
   }

   uint32_t EncodeMaterialId(int const segmentInfoIdx, int const isOpaque)
   {
      return (segmentInfoIdx << 1) | (isOpaque ? 1 : 0);
   }

   void DecodeMaterialId(uint32_t const materialId, int * segmentInfoIdx)
   {
      *segmentInfoIdx = (materialId >> 1);
   }

   void build_edge_model(std::list<CMeshInfo> const & mergedMeshes, std::vector<EdgeGeomSegment> & outSegments, std::vector<EdgeSegmentInfo> & outEdgeSegmentInfos)
   {
      EdgeGeomScene scene;
      scene.m_numTriangles = 0;
      scene.m_numVertexes = 0;
      
      uint16 validFlags = 0;

      // Tally up all triangles and vertices
      foreach(CMeshInfo const & meshInfo, mergedMeshes)
      {
         CEditorMesh const & mesh = meshInfo.mMeshData;
         
         validFlags |= mesh.mVertices.front().mValidFlags;

         scene.m_numTriangles += mesh.mTriangles.size();
         scene.m_numVertexes += mesh.mVertices.size();
      }

      int const kMaxAttributeCount = 16;
      scene.m_vertexAttributeIndexes = new uint16_t[kMaxAttributeCount];
      scene.m_vertexAttributeIds = new EdgeGeomAttributeId[kMaxAttributeCount];

      scene.m_numFloatsPerVertex = 0;
      scene.m_numVertexAttributes = 0;

      scene.m_matrixIndexesPerVertex = NULL;
      scene.m_skinningWeightsPerVertex = NULL;

      // Determine vertex attributes
      int uvCount = 0;
      {
         if( validFlags & CEditorMesh::Vertex::kFlag_Position )
         {
            scene.m_vertexAttributeIndexes[scene.m_numVertexAttributes] = scene.m_numFloatsPerVertex;
            scene.m_vertexAttributeIds[scene.m_numVertexAttributes] = EDGE_GEOM_ATTRIBUTE_ID_POSITION;

            scene.m_numFloatsPerVertex += 3;
            scene.m_numVertexAttributes++;
         }

         if( validFlags & CEditorMesh::Vertex::kFlag_Normal )
         {
            scene.m_vertexAttributeIndexes[scene.m_numVertexAttributes] = scene.m_numFloatsPerVertex;
            scene.m_vertexAttributeIds[scene.m_numVertexAttributes] = EDGE_GEOM_ATTRIBUTE_ID_NORMAL;

            scene.m_numFloatsPerVertex += 3;
            scene.m_numVertexAttributes++;
         }

         if( validFlags & CEditorMesh::Vertex::kFlag_Color0 )
         {
            scene.m_vertexAttributeIndexes[scene.m_numVertexAttributes] = scene.m_numFloatsPerVertex;
            scene.m_vertexAttributeIds[scene.m_numVertexAttributes] = EDGE_GEOM_ATTRIBUTE_ID_COLOR;

            scene.m_numFloatsPerVertex += 4;
            scene.m_numVertexAttributes++;
         }

         if( validFlags & CEditorMesh::Vertex::kFlag_UvSet0 )
         {
            scene.m_vertexAttributeIndexes[scene.m_numVertexAttributes] = scene.m_numFloatsPerVertex;
            scene.m_vertexAttributeIds[scene.m_numVertexAttributes] = EDGE_GEOM_ATTRIBUTE_ID_UV0;

            scene.m_numFloatsPerVertex += 2;
            scene.m_numVertexAttributes++;
            ++uvCount;
         }

         if( validFlags & CEditorMesh::Vertex::kFlag_UvSet1 )
         {
            scene.m_vertexAttributeIndexes[scene.m_numVertexAttributes] = scene.m_numFloatsPerVertex;
            scene.m_vertexAttributeIds[scene.m_numVertexAttributes] = EDGE_GEOM_ATTRIBUTE_ID_UV1;

            scene.m_numFloatsPerVertex += 2;
            scene.m_numVertexAttributes++;
            ++uvCount;
         }

         if( validFlags & CEditorMesh::Vertex::kFlag_Weights )
         {
            scene.m_matrixIndexesPerVertex = new int32_t[kEdgeGeomNumInputBonesPerVertex * scene.m_numVertexes];
            scene.m_skinningWeightsPerVertex = new float[kEdgeGeomNumInputBonesPerVertex * scene.m_numVertexes];
         }
      }

      scene.m_triangles = new uint32_t [scene.m_numTriangles * 3];
      scene.m_materialIdPerTriangle = new int32_t [scene.m_numTriangles];

      scene.m_vertexes = new float [scene.m_numVertexes * scene.m_numFloatsPerVertex];

      uint32_t* pIndices = scene.m_triangles;
      int32_t* pMaterialId = scene.m_materialIdPerTriangle;
      real32* pVertexData = scene.m_vertexes;

      int32_t* pMatrixIndicesPerVertex = scene.m_matrixIndexesPerVertex;
      real32* pSkinWeightsPerVertex = scene.m_skinningWeightsPerVertex;

      int currentBaseVertexOffset = 0;

      foreach(CMeshInfo const & meshInfo, mergedMeshes)
      {
         int const segmentIdx = GetOrAddEdgeSegmentInfo(meshInfo, outEdgeSegmentInfos);

         CEditorMesh const & mesh = meshInfo.mMeshData;

         foreach(CEditorMesh::Triangle const & tri, mesh.mTriangles)
         {
            *pIndices++ = currentBaseVertexOffset + tri.mIndex1;
            *pIndices++ = currentBaseVertexOffset + tri.mIndex2;
            *pIndices++ = currentBaseVertexOffset + tri.mIndex3;

            int const alpha0 = int( mesh.mVertices[tri.mIndex1].mColor0.GetW() * 255 );
            int const alpha1 = int( mesh.mVertices[tri.mIndex2].mColor0.GetW() * 255 );
            int const alpha2 = int( mesh.mVertices[tri.mIndex3].mColor0.GetW() * 255 );

            int const isOpaque = (alpha0 >= 127 && alpha1 >= 127 && alpha2 >= 127) ? 1 : 0;

            *pMaterialId++ = EncodeMaterialId(segmentIdx, isOpaque);
         }

         currentBaseVertexOffset += mesh.mVertices.size();

         foreach(CEditorMesh::Vertex const & vert, mesh.mVertices)
         {
            if( validFlags & CEditorMesh::Vertex::kFlag_Position )
            {
               *pVertexData++ = vert.mPosition[0];
               *pVertexData++ = vert.mPosition[1];
               *pVertexData++ = vert.mPosition[2];
            }

            if( validFlags & CEditorMesh::Vertex::kFlag_Normal)
            {
               *pVertexData++ = vert.mNormal[0];
               *pVertexData++ = vert.mNormal[1];
               *pVertexData++ = vert.mNormal[2];
            }

            if( validFlags & CEditorMesh::Vertex::kFlag_Color0)
            {
               *pVertexData++ = vert.mColor0[0];
               *pVertexData++ = vert.mColor0[1];
               *pVertexData++ = vert.mColor0[2];
               *pVertexData++ = vert.mColor0[3];
            }

            if( validFlags & CEditorMesh::Vertex::kFlag_UvSet0)
            {
               *pVertexData++ = vert.mUvSet0[0];
               *pVertexData++ = vert.mUvSet0[1];
            }

            if( validFlags & CEditorMesh::Vertex::kFlag_UvSet1)
            {
               *pVertexData++ = vert.mUvSet1[0];
               *pVertexData++ = vert.mUvSet1[1];
            }

            if( validFlags & CEditorMesh::Vertex::kFlag_Weights )
            {
               float totalWeights = 0.0f;
               
               int weightsWritten = 0;
               for( ; weightsWritten < vert.mWeights.size(); ++weightsWritten )
               {
                  pMatrixIndicesPerVertex[weightsWritten] = vert.mWeights[weightsWritten].first;
                  totalWeights += vert.mWeights[weightsWritten].second;
                  pSkinWeightsPerVertex[weightsWritten] = vert.mWeights[weightsWritten].second;
               }

               for( ; weightsWritten < kEdgeGeomNumInputBonesPerVertex; ++weightsWritten )
               {
                  pMatrixIndicesPerVertex[weightsWritten] = -1;
                  pSkinWeightsPerVertex[weightsWritten] = 0.0f;
               }

               if( totalWeights > 0.0f )
               {
                  for( int i = 0; i < 4; ++i )
                     pSkinWeightsPerVertex[i] /= totalWeights;
               }

               pMatrixIndicesPerVertex += 4;
               pSkinWeightsPerVertex += 4;
            }
         }
      }

      scene.m_numBlendShapes = 0;
      scene.m_numFloatsPerDelta = 0;
      scene.m_vertexDeltas = NULL;
      scene.m_numBlendedAttributes = 0;
      scene.m_blendedAttributeIndexes = NULL;
      scene.m_blendedAttributeIds = NULL;

      edgeGeomMergeIdenticalVertexes(scene);

      EdgeGeomSegmentFormat edgeFormat;
      memset(&edgeFormat, 0, sizeof(EdgeGeomSegmentFormat));

      edgeFormat.m_indexesType = kIndexesCompressedTriangleListCCW;

      edgeFormat.m_skinType = ( validFlags & CEditorMesh::Vertex::kFlag_Weights ) ? kSkinUniformScaling : kSkinNone;
      edgeFormat.m_skinMatrixFormat = kMatrix4x4ColumnMajor;

      // Generate output vertex format
      {
         edgeFormat.m_spuInputVertexFormats[0] = NULL;
         edgeFormat.m_spuInputVertexFormats[1] = NULL;
         edgeFormat.m_spuInputVertexDeltaFormat = NULL;
         edgeFormat.m_spuOutputVertexFormat = NULL;
         edgeFormat.m_rsxOnlyVertexFormat = NULL;
         GenerateEdgeSegmentFormat(scene, validFlags & CEditorMesh::Vertex::kFlag_Weights, uvCount, validFlags & CEditorMesh::Vertex::kFlag_Color0, validFlags & CEditorMesh::Vertex::kFlag_Normal, edgeFormat);
      }

      EdgeGeomSegment* pSegments = NULL;
      uint32_t segmentCount = 0;

      edgeGeomPartitionSceneIntoSegments(scene, edgeFormat, &pSegments, &segmentCount);

      delete scene.m_triangles;
      delete scene.m_materialIdPerTriangle;

      delete scene.m_matrixIndexesPerVertex;
      delete scene.m_skinningWeightsPerVertex;

      delete scene.m_vertexes;
      delete scene.m_vertexAttributeIndexes;
      delete scene.m_vertexAttributeIds;

      // Append the segments onto the allSegments vector.  We can then delete this scene's segments
      // array (but not each segment's contents; those are still being referenced by allSegments)
      outSegments.reserve(outSegments.size() + segmentCount);
      for(uint32_t i = 0; i < segmentCount; ++i)
      {
         outSegments.push_back(pSegments[i]);
      }

      edgeGeomFree(pSegments);
   }

   void free_edge_model(std::vector<EdgeGeomSegment> & segments)
   {
      for(uint32_t i = 0; i < segments.size(); ++i)
         edgeGeomFreeSegmentData(segments[i]);

      segments.clear();
   }

   void build_model_regular( XmlDocument^ globalInfo, 
                             XmlDocument^ modelInfo, 
                             Scene^ scene, 
                             AssetSystem::PlatformType::EPlatform const platform, 
                             AssetSystem::Manager^ assetManager, 
                             CookProperties::CookInfo^ cookInfo, 
                             CookProperties::CookOutput^ cookOutput, 
                             std::vector<CMeshChunk> * pOutChunks, 
                             int const verboseOutputLevel, 
                             CIndexArray* pOutIndexArray, 
                             CVertexArray** pOutVertexArray,
                             std::vector<EdgeGeomSegment>* pEdgeSegments,
                             std::vector<EdgeSegmentInfo>* pOutEdgeSegmentInfos,
                             int *highestMaterialIdx,
                             int *highestJointIndex )
   {
      std::list< CMeshInfo > meshes;
      bool const bRemapJoints = ( platform != AssetSystem::PlatformType::EPlatform::kVita );

      int originalTriangleCount = 0;
      int clippedTriangleCount = 0;

      *highestJointIndex = -1;

      List<DagNode^>^ meshNodes = scene->GetMeshes();
      for( int meshIdx = 0; meshIdx < meshNodes->Count; ++meshIdx )
      {
         DagNode^ node = meshNodes[meshIdx];

         TVector4Vector points;
         TVector3Vector normals;
         TVector4Vector uv0;
         TVector4Vector uv1;
         TVector4Vector uv2;
         TVector4Vector color0;
         TVector4Vector color1;
         TVector4Vector color2;
         TColorVector   skinIndices;
         TVector4Vector skinWeights;

         GetDataArrays(node, points, normals, uv0, uv1, uv2, color0, color1, color2, skinIndices, skinWeights);
#if 0
         // weld points.
         if ( skinIndices.size() && skinWeights.size() )
         {
            for ( TVector4Vector::iterator it = points.begin(); it != points.end(); ++it )
            {
               for ( unsigned int meshIdx2 = meshIdx; meshIdx2 < meshNodes->Count; ++meshIdx2 )
               {
                  TVector4Vector points2;
                  GetDataArray(node, points2, "Points", "");
                  for ( TVector4Vector::iterator jt = points2.begin(); jt != points2.end(); ++jt )
                  {
                     // get the two vertices.
                     CVector4& a = *it;
                     CVector4& b = *jt;
                     CVector4 delta = a - b;
                     delta.SetW( 0.0f );

                     // check to see if the vertices are similar.
                     if ( delta.GetLengthSquared() <= 0.25f )
                        a = b;
                  }
               }
            }
         }
#endif
         List<Material^> materials = scene->GetMaterials();

#if 0
         for each(Material^ material in materials)
         {
            int const materialIndex = Int32::Parse(material->Node->GetAttribute("index"));

            if(materialIndex > *highestMaterialIdx)
            {
               *highestMaterialIdx = materialIndex;
            }
         }
#endif

         // iterate over all submeshes
         List<Submesh^>^ submeshes = node->GetSubmeshes();
         for( int submeshIdx = 0; submeshIdx < submeshes->Count; ++submeshIdx )
         {
            Submesh^ mesh = submeshes[submeshIdx];

            TIntVector pointIndices;
            TIntVector normalIndices;
            TIntVector uv0Indices;
            TIntVector uv1Indices;
            TIntVector uv2Indices;
            TIntVector uv3Indices;
            TIntVector color0Indices;
            TIntVector color1Indices;
            TIntVector color2Indices;
            TIntVector skinIndexIndices;
            TIntVector skinWeightIndices;

            GetIndexArrays(mesh, pointIndices, normalIndices, uv0Indices, uv1Indices, uv2Indices, color0Indices, color1Indices, color2Indices, skinIndexIndices, skinWeightIndices);

            std::vector<CEditorMesh::Triangle> triangles;
            std::vector<CEditorMesh::Vertex> vertices;

            int const triCount = pointIndices.size() / 3;

            triangles.reserve(triCount);
            vertices.reserve(triCount * 3);

            Material^ material = scene->GetMaterial(mesh->Material);
            int const unitIndex = Int32::Parse(mesh->Node->GetAttribute("unitIdx"));
            int const packetIndex = Int32::Parse(mesh->Node->GetAttribute("packetIdx"));

            //uint64 const additionalFlags = GetAdditionalFlags(globalInfo, modelInfo, scene, material, submeshIdx);
            uint64 const additionalFlags = 0;   //AndyO: TEMP: Allow MGS2 models to cook.
            
            if( !(additionalFlags & kAF_IgnoreMaterial) )
            {
               for( int triIdx = 0; triIdx < triCount; ++triIdx )
               {
                  int indices[3];
                  for( int index = 0; index < 3; ++index )
                  {
                     int const offset = triIdx * 3 + index;

                     CEditorMesh::Vertex vertex;

                     int const pointIndex = !pointIndices.empty() ? pointIndices[offset] : -1;
                     int const normalIndex = !normalIndices.empty() ? normalIndices[offset] : -1;
                     int const uv0Index = !uv0Indices.empty() ? uv0Indices[offset] : -1;
                     int const uv1Index = !uv1Indices.empty() ? uv1Indices[offset] : -1;
                     int const uv2Index = !uv2Indices.empty() ? uv2Indices[offset] : -1;
                     int const color0Index = !color0Indices.empty() ? color0Indices[offset] : -1;
                     int const color1Index = !color1Indices.empty() ? color1Indices[offset] : -1;
                     int const color2Index = !color2Indices.empty() ? color2Indices[offset] : -1;
                     int const skinIndexIndex = !skinIndexIndices.empty() ? skinIndexIndices[offset] : -1;
                     int const skinWeightIndex = !skinWeightIndices.empty() ? skinWeightIndices[offset] : -1;

                     ExtractVertexData(vertex,
                        points, normals, uv0, uv1, uv2, color0, color1, color2, skinIndices, skinWeights,
                        pointIndex, normalIndex, uv0Index, uv1Index, uv2Index, color0Index, color1Index, color2Index, skinIndexIndex, skinWeightIndex);

                     vertex.mOriginalIndex = pointIndex;

                     // store index of triangle vertex
                     indices[index] = vertices.size();

                     // store vertex we just created
                     vertices.push_back(vertex);
                  }

                  triangles.push_back( CEditorMesh::Triangle( indices[0], indices[1], indices[2] ) );
               }
            }

            originalTriangleCount += triangles.size();

            RemoveDegenerates(vertices, &triangles);

            uint64 const doClip = additionalFlags & (kAF_MeshClipU|kAF_MeshClipV);
            if( doClip )
            {
#if 0
               String^ objOutputPath = String::Format("C:/clipTest_{0}.obj", submeshIdx);
               write_obj(objOutputPath, false, "original_" + material->Name, triangles, vertices);
#endif
               PerformUVClipping(&vertices, &triangles, ( additionalFlags & kAF_MeshClipU ) == kAF_MeshClipU, ( additionalFlags & kAF_MeshClipV ) == kAF_MeshClipV);
#if 0
               write_obj(objOutputPath, true, "clipped_" + material->Name, triangles, vertices);
#endif
            }

            uint64 const doFixWinding = additionalFlags & kAF_FixWinding;
            if( doFixWinding )
            {
#if 0
               String^ objOutputPath = String::Format("C:/windingTest_{0}.obj", submeshIdx);
               write_obj(objOutputPath, false, "original_" + material->Name, triangles, vertices);
#endif
               PerformFixWinding(vertices, triangles);
#if 0
               write_obj(objOutputPath, true, "fixed_" + material->Name, triangles, vertices);
#endif
            }

            clippedTriangleCount += triangles.size();

            meshes.push_back(CMeshInfo(additionalFlags, unitIndex, packetIndex));

            // NOTE: the triangles, lines and vertices array will be empty after this function call. Look at the TakeOverData function to understand why.
            {
               CMeshInfo & meshInfo = meshes.back();
               std::vector<CEditorMesh::Line> lines;
               meshInfo.mMeshData.TakeOverData(triangles, lines, vertices);
            }
         }
      }

      if( verboseOutputLevel && originalTriangleCount != clippedTriangleCount )
      {
         System::Console::WriteLine("UV mesh clipping turned {0} triangles into {1}", originalTriangleCount, clippedTriangleCount);
      }

      // merge all meshes by material index
      std::list<CMeshInfo> mergedMeshes;
      {
         std::list<CMeshInfo> remainingMeshes = meshes;

         while( !remainingMeshes.empty() )
         {
            mergedMeshes.push_back(remainingMeshes.front());
            remainingMeshes.pop_front();

            CMeshInfo * pMergedMesh = &mergedMeshes.back();

            for(std::list<CMeshInfo>::iterator it = remainingMeshes.begin(); it != remainingMeshes.end(); )
            {
               CMeshInfo const & meshToAdd = *it;

               if( meshToAdd.mAdditionalFlags == pMergedMesh->mAdditionalFlags &&
                  meshToAdd.mUnitIdx == pMergedMesh->mUnitIdx &&
                  meshToAdd.mPacketIdx == pMergedMesh->mPacketIdx )
               {
                  pMergedMesh->mMeshData.AddData(meshToAdd.mMeshData.GetTriangles(), meshToAdd.mMeshData.GetLines(), meshToAdd.mMeshData.GetVertices());
                  it = remainingMeshes.erase(it);
               }
               else
               {
                  ++it;
               }
            }
         }
      }

      if( pEdgeSegments && platform == AssetSystem::PlatformType::EPlatform::kPS3 )
      {
         build_edge_model(mergedMeshes, *pEdgeSegments, *pOutEdgeSegmentInfos);
      }

      std::list<CMeshInfo> & meshesForBPModel = mergedMeshes;

      foreach(CMeshInfo & meshToAdd, meshesForBPModel)
      {
         CEditorMesh & meshData = meshToAdd.mMeshData;
         std::vector<uint32> remapTable;

         CEditorMesh::BuildJointRemapTable( meshData.mTriangles, meshData.mVertices, &remapTable );

         // Get the highest joint index
         foreach ( uint32 const jointIndex, remapTable )
         {
            if ( int( jointIndex ) > *highestJointIndex )
            {
               *highestJointIndex = int( jointIndex );
            }
         }

         // Only set the mapping table current if we're remapping joints
         if ( bRemapJoints )
         {
            meshData.mJointRemapTable = remapTable;
            meshData.RemapJoints(meshData.mJointRemapTable, meshData.mTriangles, meshData.mVertices);
         }
      }

      std::vector<uint32> indices;
      std::vector<uint32> triListIndices;
      CMergeContainer<CEditorMesh::Vertex> vertices;

      CEditorMesh::BuildMeshes( meshesForBPModel, 
         *pOutChunks, 
         indices,
         triListIndices,
         vertices,
         verboseOutputLevel);

      // WE NEVER WANT TANGENTS/BINORMALS FOR PS2 MODELS.
      bool buildTextureBasisVectors = false;

      *pOutVertexArray = CMeshUtils::BuildVertexArray( vertices.GetArray(), triListIndices, buildTextureBasisVectors, verboseOutputLevel );
      *pOutIndexArray = CIndexArray(indices);
   }

   bool build_model(String^ inputPath,
                    AssetSystem::PlatformType::EPlatform const platform, 
                    AssetSystem::Manager^ assetManager, 
                    bool const isSkinnedModel, 
                    int const verboseOutputLevel, 
                    CookProperties::CookInfo^ cookInfo, 
                    CookProperties::CookOutput^ cookOutput,
                    CIndexArray* pOutIndexArray, 
                    CVertexArray** pOutVertexArray, 
                    std::vector<CMeshChunk> * pOutChunks,
                    std::vector<EdgeGeomSegment>* pOutEdgeSegments,
                    std::vector<EdgeSegmentInfo>* pOutEdgeSegmentInfos,
                    int *highestMaterialIdx,
                    int *pHighestJointIndex)
   {
      XmlDocument^ doc = gcnew XmlDocument();
      doc->Load( inputPath );
      Scene^ scene = Scene::Attach( doc );

      XmlDocument^ globalInfo = gcnew XmlDocument();
      try
      {
         globalInfo->Load(IO::Path::Combine(IO::Path::GetDirectoryName(inputPath), "Global.xml"));
      }
      catch(System::Exception^ e)
      {
         globalInfo = nullptr;
      }

      XmlDocument^ modelInfo = gcnew XmlDocument();
      try
      {
         modelInfo->Load(IO::Path::ChangeExtension(inputPath, "xml"));
      }
      catch(System::Exception^ e)
      {
         modelInfo = nullptr;
      }

      build_model_regular(globalInfo, modelInfo, scene, platform, assetManager, cookInfo, cookOutput, pOutChunks, verboseOutputLevel, pOutIndexArray, pOutVertexArray, pOutEdgeSegments, pOutEdgeSegmentInfos, highestMaterialIdx, pHighestJointIndex);

      return true;
   }

   //----------------------------------------------------------------------------

   static const uint32 kVertexPerElementSize[] =
   {
      4,    // 2D float expanded to (value, value, 0, 1)
      4,   // 3D float expanded to (value, value, value, 1)
      4,   // 4D float

      1,    // Each of 4 bytes is normalized by dividing to 255.0
      1,    // 4D unsigned byte

      2,    // Two 16-bit floating point values, expanded to (value, value, 0, 1)
      2,    // Four 16-bit floating point values

      2,    // 2D signed short normalized (v[0]/32767.0,v[1]/32767.0,0,1)
      2,    // 4D signed short normalized (v[0]/32767.0,v[1]/32767.0,v[2]/32767.0,v[3]/32767.0)

      4,    // 3d signed normalized (11:11:10 on PS3, 10:10:10 on D3D) expanded to (value, value, value, 1)

      4,    // int32

      2,    // short2

      2     // short4
   };

   BPE_CTASSERT(BPE_ARRAY_SIZE(kVertexPerElementSize) == kVDT_Count);

   void write_vertex_array(CVertexArray const & vertexArray, COutputStream & stream, COutputStream::EOutputEndian const endian)
   {
      CLabeledOffsetStream vertexStream;
      vertexStream.SetEndian(endian);

      int const streamCount = vertexArray.GetStreamCount();
      
      vertexStream.AddLabel("Start");

      // write out stream count
      vertexStream.WriteUint32(streamCount);
      
      // write out stream headers
      for( int i = 0; i < streamCount; ++i )
      {
         CVertexArray::Stream const * pStream = vertexArray.GetStreamByIndex(i);
         vertexStream.WriteUint32(pStream->mId);
         vertexStream.WriteUint16(pStream->mType);
         vertexStream.WriteUint16(CVertexArray::kFlags_IsOffset);
         vertexStream.AddOffset32(CStringExtras::Stringize("%8.8x", pStream->mId), "Start");
         vertexStream.WriteUint32(pStream->mSize);
      }

      // write out stream data
      for( int i = 0; i < streamCount; ++i )
      {
         CVertexArray::Stream const * pStream = vertexArray.GetStreamByIndex(i);

         vertexStream.AddAlignmentPadding(16);
         vertexStream.AddLabel(CStringExtras::Stringize("%8.8x", pStream->mId));

         int const perElementSize = kVertexPerElementSize[pStream->mType];
         int const numElements = pStream->mSize / perElementSize;

         switch(perElementSize)
         {
         case 4:
            {
               uint32* pData = (uint32*)pStream->GetDataPtr();
               
               for( int j = 0; j < numElements; ++j )
               {
                  vertexStream.WriteUint32(pData[j]);
               }
            }
            break;

         case 2:
            {
               uint16* pData = (uint16*)pStream->GetDataPtr();

               for( int j = 0; j < numElements; ++j )
               {
                  vertexStream.WriteUint16(pData[j]);
               }
            }
            break;

         case 1:
            {
               vertexStream.Put((void*)pStream->mData, pStream->mSize);
            }
            break;
         }
      }

      vertexStream.PatchOffsets();

      stream.WriteUint32(vertexStream.GetDataSize());
      stream.Put(vertexStream.GetData(), vertexStream.GetDataSize());
   }

   //----------------------------------------------------------------------------

   bool write_cooked_model( String^ inputPath, String^ outputPath, AssetSystem::Manager^ assetManager, bool const isSkinnedModel, Tools::AssetSystem::PlatformType::EPlatform platform, int const verboseOutputLevel)
   {
      CookProperties::CookInfo^ cookInfo = gcnew CookProperties::CookInfo();
      CookProperties::CookOutput ^cookOutput = gcnew CookProperties::CookOutput();

      cookInfo->mPlatform = platform;
      {
         CIndexArray indexArray;
         CVertexArray* pVertexArray = NULL;
         std::vector<CMeshChunk> chunks;
         std::vector<EdgeGeomSegment> edgeSegments;
         std::vector<EdgeSegmentInfo> edgeSegmentInfos;
         int highestMaterialIdx = 0;
         int highestJointIndex = -1;

         if( !build_model(inputPath, platform, assetManager, isSkinnedModel, verboseOutputLevel, cookInfo, cookOutput, &indexArray, &pVertexArray, &chunks, &edgeSegments, &edgeSegmentInfos, &highestMaterialIdx, &highestJointIndex) )
         {
            return false;
         }

         CDiskOutputStream outStream( Helper::StringHelper::ConvertString( outputPath ) );

         // Write WIN32 model format
#if 0 //BP - Only write out original win32 style mesh format for now.
         if( platform == Tools::AssetSystem::PlatformType::EPlatform::kWin32 )
#endif
         {
            // write header
            outStream.WriteUint32('MODL');
            // write version
            outStream.WriteUint32(2);

            // write out vertex array
            COutputStream::EOutputEndian outputEndian;

            switch ( platform )
            {
            case Tools::AssetSystem::PlatformType::EPlatform::kWin32:
            case Tools::AssetSystem::PlatformType::EPlatform::kVita:
               outputEndian = COutputStream::EOutputEndian::kOE_LittleEndian;
               break;

            case Tools::AssetSystem::PlatformType::EPlatform::kX360:
            case Tools::AssetSystem::PlatformType::EPlatform::kPS3:
               outputEndian = COutputStream::EOutputEndian::kOE_BigEndian;
               break;
            default:
               Console::WriteLine( "Don't know endianness of platform {0}", platform );

               return false;
            }
            write_vertex_array(*pVertexArray, outStream, outputEndian);

            // write out index array
            indexArray.PutTo(outStream);

            // write out material count and material data
            outStream.WriteInt32(0);

            // write out mesh chunks
            outStream.WriteInt32( chunks.size() );
            for( int i = 0; i < chunks.size(); ++i )
            {
               outStream.Put( chunks[i] );
            }

            // Write out the joint count
            outStream.WriteInt32( highestJointIndex + 1 );

            if (verboseOutputLevel)
            {
               System::Console::WriteLine("");
               System::Console::WriteLine("Statistics:");
               System::Console::WriteLine("Indices: {0} Vertices: {1}", indexArray.GetIndices().size(), pVertexArray->GetStreamByIndex(0)->GetElementCount());
            
               for( int i = 0; i < chunks.size(); ++i )
               {
                  CMeshChunk const & chunk = chunks[i];

                  System::Console::Write("Chunk[{0}]: Unit: {1} Packet: {2} Triangles: {4}", i, chunk.mUnitIdx, chunk.mPacketIdx, chunk.mMaterialIndex, CMeshChunk::GetPrimitiveCount(chunk.mPrimitiveType, chunk.mIndicesCount), chunk.mAdditionalFlags);
                  if( chunk.mJointMap.empty() )
                  {
                     System::Console::WriteLine("");
                  }
                  else
                  {
                     System::Console::Write(" Joints: ");
                     for( int i = 0; i < chunk.mJointMap.size(); ++i )
                     {
                        System::Console::Write("{0} ", chunk.mJointMap[i]);
                     }
                     System::Console::WriteLine("");
                  }
               }
            }

         }
#if 0 //BP - Only write out original win32 style mesh format for now.
         else if( platform == Tools::AssetSystem::PlatformType::EPlatform::kPS3 )
         {
            // Determine highest used counts
            int highestLayerIdx = 0;
            {
               for(int i = 0; i < edgeSegments.size(); ++i )
               {
                  EdgeGeomSegment const & segment = edgeSegments[i];
                  
                  int segmentInfoIdx;
                  DecodeMaterialId(segment.m_materialId, &segmentInfoIdx);
                  EdgeSegmentInfo const & segmentInfo = edgeSegmentInfos[segmentInfoIdx];

                  if( segmentInfo.mLayerNumber > highestLayerIdx )
                     highestLayerIdx = segmentInfo.mLayerNumber;
               }
            }

            int const materialCount = highestMaterialIdx + 1;
            int const layerCount = highestLayerIdx + 1;

            // Build per material segment count array
            std::vector<int> materialSegmentCounts(materialCount * layerCount);
            {
               for(int i = 0; i < edgeSegments.size(); ++i )
               {
                  EdgeGeomSegment const & segment = edgeSegments[i];

                  int segmentInfoIdx;
                  DecodeMaterialId(segment.m_materialId, &segmentInfoIdx);
                  EdgeSegmentInfo const & segmentInfo = edgeSegmentInfos[segmentInfoIdx];

                  materialSegmentCounts[segmentInfo.mLayerNumber*materialCount + segmentInfo.mMaterialId]++;
               }
            }

            CLabeledOffsetStream stream;

            stream.AddLabel("Header");

            // write header
            stream.WriteUint32('EMDL');
            // write version
            stream.WriteUint32(0);

            // write number of layers
            stream.WriteUint32(layerCount);
            // write number of materials
            stream.WriteUint32(materialCount);

            // write number of segments
            stream.WriteUint32(edgeSegments.size());
            // write offset to segments
            stream.AddOffset32("Segments", "Header");
            // write offset to additional segment info array
            stream.AddOffset32("SegmentInfo", "Header");

            // write offset to segment table
            stream.AddOffset32("SegmentTable", "Header");

            // write size of resource without RSX only data (this is the portion of data that is kept in system memory)
            stream.AddOffset32("RsxOnlyDataBegin", "Header");

            // write size of RSX only data (this is the portion that gets copies to video memory
            stream.AddOffset32("RsxOnlyDataEnd", "RsxOnlyDataBegin");

            // pad to 128 alignment
            stream.AddAlignmentPadding(128);

            // Write segment table
            {
               stream.AddLabel("SegmentTable");
               int currentSegmentOffset = 0;
               for( int i = 0; i < materialSegmentCounts.size(); ++i )
               {
                  int const segmentCount = materialSegmentCounts[i];

                  stream.WriteUint32(currentSegmentOffset);
                  stream.WriteUint32(segmentCount);

                  currentSegmentOffset += segmentCount;
               }
            }

            // Write segment info
            {
               stream.AddAlignmentPadding(16); stream.AddLabel("SegmentInfo");
               for( int i = 0; i < edgeSegments.size(); ++i )
               {
                  EdgeGeomSegment const & segment = edgeSegments[i];

                  int segmentInfoIdx;
                  DecodeMaterialId(segment.m_materialId, &segmentInfoIdx);
                  EdgeSegmentInfo const & segmentInfo = edgeSegmentInfos[segmentInfoIdx];

                  // write min vertex color
                  stream.WriteUint32(segment.mVertexColorMin);
                  // write max vertex color
                  stream.WriteUint32(segment.mVertexColorMax);
                  // write additional flags
                  stream.WriteUint32(segmentInfo.mAdditionalFlags);
               }
            }

            stream.AddAlignmentPadding(16); stream.AddLabel("Segments");

            // Write segment headers
            for(int i = 0; i < edgeSegments.size(); ++i )
            {
               EdgeGeomSegment const & segment = edgeSegments[i];

               EdgeGeomSpuConfigInfo const & spuConfig = *(EdgeGeomSpuConfigInfo*)segment.m_spuConfigInfo;
               stream.Put(segment.m_spuConfigInfo, sizeof(EdgeGeomSpuConfigInfo));

               segment.m_indexes ? stream.AddOffset32(CStringExtras::Stringize("m_indexes[%d]", i), "Header") : stream.WriteUint32(0);
               stream.WriteUint16(segment.m_indexesSizes[0]);
               stream.WriteUint16(segment.m_indexesSizes[1]);

               segment.m_spuVertexes[0] ? stream.AddOffset32(CStringExtras::Stringize("m_spuVertexes[0][%d]", i), "Header") : stream.WriteUint32(0);
               segment.m_spuVertexes[1] ? stream.AddOffset32(CStringExtras::Stringize("m_spuVertexes[1][%d]", i), "Header") : stream.WriteUint32(0);
               stream.WriteUint16(segment.m_spuVertexesSizes[0]);
               stream.WriteUint16(segment.m_spuVertexesSizes[1]);
               stream.WriteUint16(segment.m_spuVertexesSizes[2]);
               stream.WriteUint16(segment.m_spuVertexesSizes[3]);
               stream.WriteUint16(segment.m_spuVertexesSizes[4]);
               stream.WriteUint16(segment.m_spuVertexesSizes[5]);

               segment.m_rsxOnlyVertexes ? stream.AddOffset32(CStringExtras::Stringize("m_rsxOnlyVertexes[%d]", i), "RsxOnlyDataBegin") : stream.WriteUint32(0);
               stream.WriteUint32(segment.m_rsxOnlyVertexesSize);

               stream.WriteUint16(segment.m_skinMatricesByteOffsets[0]);
               stream.WriteUint16(segment.m_skinMatricesByteOffsets[1]);
               stream.WriteUint16(segment.m_skinMatricesSizes[0]);
               stream.WriteUint16(segment.m_skinMatricesSizes[1]);
               stream.WriteUint16(segment.m_skinIndexesAndWeightsSizes[0]);
               stream.WriteUint16(segment.m_skinIndexesAndWeightsSizes[1]);
               segment.m_skinIndexesAndWeights ? stream.AddOffset32(CStringExtras::Stringize("m_skinIndexesAndWeights[%d]", i), "Header") : stream.WriteUint32(0);

               stream.WriteUint32(segment.m_ioBufferSize);
               stream.WriteUint32(segment.m_scratchSize);
               
               stream.WriteUint32(segment.m_numBlendShapes);
               segment.m_blendShapeSizes ? stream.AddOffset32(CStringExtras::Stringize("m_blendShapeSizes[%d]", i), "Header") : stream.WriteUint32(0);
               segment.m_blendShapes ? stream.AddOffset32(CStringExtras::Stringize("m_blendShapes[%d]", i), "Header") : stream.WriteUint32(0);
               
               stream.WriteUint32(segment.m_fixedOffsetsSize[0]);
               stream.WriteUint32(segment.m_fixedOffsetsSize[1]);
               segment.m_fixedOffsetPtrs[0] ? stream.AddOffset32(CStringExtras::Stringize("m_fixedOffsetPtrs[0][%d]", i), "Header") : stream.WriteUint32(0);
               segment.m_fixedOffsetPtrs[1] ? stream.AddOffset32(CStringExtras::Stringize("m_fixedOffsetPtrs[1][%d]", i), "Header") : stream.WriteUint32(0);

               segment.m_spuInputStreamDescriptions[0] ? stream.AddOffset32(CStringExtras::Stringize("m_spuInputStreamDescriptions[0][%d]", i), "Header") : stream.WriteUint32(0);
               segment.m_spuInputStreamDescriptions[1] ? stream.AddOffset32(CStringExtras::Stringize("m_spuInputStreamDescriptions[1][%d]", i), "Header") : stream.WriteUint32(0);

               segment.m_spuOutputStreamDescription ? stream.AddOffset32(CStringExtras::Stringize("m_spuOutputStreamDescription[%d]", i), "Header") : stream.WriteUint32(0);
               segment.m_rsxOnlyStreamDescription ? stream.AddOffset32(CStringExtras::Stringize("m_rsxOnlyStreamDescription[%d]", i), "Header") : stream.WriteUint32(0);

               stream.WriteUint16(segment.m_spuInputStreamDescriptionSizes[0]);
               stream.WriteUint16(segment.m_spuInputStreamDescriptionSizes[1]);

               stream.WriteUint16(segment.m_spuOutputStreamDescriptionSize);
               stream.WriteUint16(segment.m_rsxOnlyStreamDescriptionSize);
            }

            // Write segment data
            for(int i = 0; i < edgeSegments.size(); ++i )
            {
               EdgeGeomSegment const & segment = edgeSegments[i];

               if( segment.m_indexes )
               {
                  stream.AddAlignmentPadding(16); stream.AddLabel(CStringExtras::Stringize("m_indexes[%d]", i));
                  stream.Put(segment.m_indexes, segment.m_indexesSizes[0] + segment.m_indexesSizes[1]);
               }

               if( segment.m_spuVertexes[0] )
               {
                  stream.AddAlignmentPadding(16); stream.AddLabel(CStringExtras::Stringize("m_spuVertexes[0][%d]", i));
                  stream.Put(segment.m_spuVertexes[0], segment.m_spuVertexesSizes[0] + segment.m_spuVertexesSizes[1] + segment.m_spuVertexesSizes[2]);
               }

               if( segment.m_spuVertexes[1] )
               {
                  stream.AddAlignmentPadding(16); stream.AddLabel(CStringExtras::Stringize("m_spuVertexes[1][%d]", i));
                  stream.Put(segment.m_spuVertexes[1], segment.m_spuVertexesSizes[3] + segment.m_spuVertexesSizes[4] + segment.m_spuVertexesSizes[5]);
               }

               if( segment.m_skinIndexesAndWeights )
               {
                  stream.AddAlignmentPadding(16); stream.AddLabel(CStringExtras::Stringize("m_skinIndexesAndWeights[%d]", i));
                  stream.Put(segment.m_skinIndexesAndWeights, segment.m_skinIndexesAndWeightsSizes[0] + segment.m_skinIndexesAndWeightsSizes[1]);
               }

               if( segment.m_numBlendShapes )
               {
                  stream.AddAlignmentPadding(16); stream.AddLabel(CStringExtras::Stringize("m_blendShapeSizes[%d]", i));
                  for( int j = 0; j < segment.m_numBlendShapes; ++j )
                     stream.WriteUint16(segment.m_blendShapeSizes[j]);

                  stream.AddAlignmentPadding(16); stream.AddLabel(CStringExtras::Stringize("m_blendShapes[%d]", i));
                  {
                     for( int j = 0; j < segment.m_numBlendShapes; ++j )
                     {
                        stream.AddOffset32(CStringExtras::Stringize("m_blendShapes[%d][%d]", i, j));
                     }

                     for( int j = 0; j < segment.m_numBlendShapes; ++j )
                     {
                        stream.AddAlignmentPadding(16); stream.AddLabel(CStringExtras::Stringize("m_blendShapes[%d][%d]", i, j));
                        stream.Put(segment.m_blendShapes[j], segment.m_blendShapeSizes[j]);
                     }
                  }
               }

               if( segment.m_fixedOffsetPtrs[0] )
               {
                  stream.AddAlignmentPadding(16); stream.AddLabel(CStringExtras::Stringize("m_fixedOffsetPtrs[0][%d]", i));
                  stream.Put(segment.m_fixedOffsetPtrs[0], segment.m_fixedOffsetsSize[0]);
               }

               if( segment.m_fixedOffsetPtrs[1] )
               {
                  stream.AddAlignmentPadding(16); stream.AddLabel(CStringExtras::Stringize("m_fixedOffsetPtrs[1][%d]", i));
                  stream.Put(segment.m_fixedOffsetPtrs[1], segment.m_fixedOffsetsSize[1]);
               }

               if( segment.m_spuInputStreamDescriptions[0] )
               {
                  stream.AddAlignmentPadding(16); stream.AddLabel(CStringExtras::Stringize("m_spuInputStreamDescriptions[0][%d]", i));
                  stream.Put(segment.m_spuInputStreamDescriptions[0], segment.m_spuInputStreamDescriptionSizes[0]);
               }

               if( segment.m_spuInputStreamDescriptions[1] )
               {
                  stream.AddAlignmentPadding(16); stream.AddLabel(CStringExtras::Stringize("m_spuInputStreamDescriptions[1][%d]", i));
                  stream.Put(segment.m_spuInputStreamDescriptions[1], segment.m_spuInputStreamDescriptionSizes[1]);
               }

               if( segment.m_spuOutputStreamDescription )
               {
                  stream.AddAlignmentPadding(16); stream.AddLabel(CStringExtras::Stringize("m_spuOutputStreamDescription[%d]", i));
                  stream.Put(segment.m_spuOutputStreamDescription, segment.m_spuOutputStreamDescriptionSize);
               }

               if( segment.m_rsxOnlyStreamDescription )
               {
                  stream.AddAlignmentPadding(16); stream.AddLabel(CStringExtras::Stringize("m_rsxOnlyStreamDescription[%d]", i));
                  stream.Put(segment.m_rsxOnlyStreamDescription, segment.m_rsxOnlyStreamDescriptionSize);
               }
            }

            stream.AddAlignmentPadding(16); stream.AddLabel("RsxOnlyDataBegin");

            // Write RSX only segment data at this point
            for(int i = 0; i < edgeSegments.size(); ++i )
            {
               EdgeGeomSegment const & segment = edgeSegments[i];

               if( segment.m_rsxOnlyVertexes )
               {
                  stream.AddAlignmentPadding(16); stream.AddLabel(CStringExtras::Stringize("m_rsxOnlyVertexes[%d]", i));
                  stream.Put(segment.m_rsxOnlyVertexes, segment.m_rsxOnlyVertexesSize);
               }
            }

            stream.AddAlignmentPadding(16); stream.AddLabel("RsxOnlyDataEnd");

            stream.PatchOffsets();

            outStream.Put(stream.GetData(), stream.GetDataSize());
         }
#endif
      }

      // write assets file
      //AssetSystem::PackageAssets::WriteAssets(inputPath, cookOutput->PackageAssets, assetManager, platform);
      AssetSystem::BuildAssets::WriteAssets(inputPath, cookOutput->BuildAssets, assetManager, platform);

      return true;
   }
}
//----------------------------------------------------------------------------

bool AssetToolMPP::Cookers::CookMODL( String^ inputPath, cli::array<System::String^> ^outputPaths, cli::array<Tools::AssetSystem::PlatformType::EPlatform> ^platforms, AssetSystem::Manager^ assetManager, int const verboseOutputLevel )
{
   bool const kIsSkinnedModel = false;

   // platform not important for regular models, only for skinned models
   for ( int i = 0; i < outputPaths->Length; ++i )
   {
      String ^outputPath = outputPaths[i];
      
      Tools::AssetSystem::PlatformType::EPlatform platform = platforms[i];
      bool success = false;

      switch ( platform )
      { 
      case Tools::AssetSystem::PlatformType::EPlatform::kPS3:
      case Tools::AssetSystem::PlatformType::EPlatform::kWin32:
      case Tools::AssetSystem::PlatformType::EPlatform::kRVL:
      case Tools::AssetSystem::PlatformType::EPlatform::kX360:
      case Tools::AssetSystem::PlatformType::EPlatform::kVita:
         success = write_cooked_model(inputPath, outputPath, assetManager, kIsSkinnedModel, platform, verboseOutputLevel);
         break;
      default:
         Console::WriteLine( "ERROR: Cannot cook model on platform {0}", platform );
         break;
      }

      if ( !success )
      {
         return false;
      }
   }

   return true;
}

//----------------------------------------------------------------------------

bool AssetToolMPP::Cookers::CookEMDL( System::String^ inputPath, System::String^ outputPath, Tools::AssetSystem::Manager^ assetManager )
{
   return false;
}

//----------------------------------------------------------------------------

bool AssetToolMPP::Cookers::CookSKIN( String^ inputPath, String^ outputPath, AssetSystem::Manager^ assetManager, Tools::AssetSystem::PlatformType::EPlatform const platform, int const verboseOutputLevel )
{
   return true;
}

//----------------------------------------------------------------------------

bool AssetToolMPP::Cookers::CookANIM( System::String^ inputPath, System::String^ outputPath, AssetSystem::Manager^ assetManager, Tools::AssetSystem::PlatformType::EPlatform const platform )
{
   return true;
}
