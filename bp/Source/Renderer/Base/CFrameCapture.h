//----------------------------------------------------------------------------
// CFrameCapture.h
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"

//----------------------------------------------------------------------------

class CTexture;
class CIndexBuffer;
class CShaderVertexDataBinding;
class CVertexData;
class CMatrix34;
class CMatrix4;

//----------------------------------------------------------------------------

#define CAPTURE_BEGIN()
#define CAPTURE_TEXTURE(texUnit, pTexture)
#define CAPTURE_TEXTURE_ADDRESS(texUnit, wrapU, wrapV)
#define CAPTURE_INDEX_BUFFER(pBuffer)
#define CAPTURE_VERTEX_DATA(binding, data)
#define CAPTURE_MODEL_MATRIX(matrix)
#define CAPTURE_SET_JOINT_REMAP(pMatrixIndices, count)
#define CAPTURE_VIEW_MATRIX(matrix)
#define CAPTURE_PROJECTION_MATRIX(matrix)
#define CAPTURE_PRIMITIVES(type, vertexBufferOffset, vertexCount, indexBufferOffset, indexCount)
#define CAPTURE_PRIMITIVES_NOINDICES(type, vertexBufferOffset, vertexCount)
#define CAPTURE_SCOPE_ENABLE_BEGIN()
#define CAPTURE_SCOPE_ENABLE_END()
#define CAPTURE_BEGIN_LAYER(pName)
#define CAPTURE_END_LAYER()

#define CAPTURE_END()
#define CAPTURE_DO_CAPTURE_FRAME()
#define CAPTURE_IS_ENABLED()

//----------------------------------------------------------------------------
