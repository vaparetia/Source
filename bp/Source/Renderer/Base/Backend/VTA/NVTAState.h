#pragma once

#include <gxm/shader_patcher.h>

#include "Renderer/Base/Material/ProgShader/PSCShaderVertexDataBinding.h"
#include "Renderer/Base/Primitive/CVertexBuffer.h"
#include "Renderer/Base/Primitive/ProgShader/CIndexBuffer.h"
#include "Renderer/Base/Primitive/CVertexData.h"

struct SFragmentParameter;
struct SVertexParameter;
struct SVertexSemantic;

namespace NVTAState
{
   enum ENotificationObjects
   {
      // Make sure the display buffer ones are first, as
      // live metrics just use them for display buffers indexing
      // from zero
      kNO_DisplayBuffer0,
      kNO_DisplayBuffer1,
      kNO_DisplayBuffer2,

      kNO_ReallocateVertexPrograms,

      kNO_DisplayBufferCount = kNO_DisplayBuffer2 + 1
   };

   enum EProgramType
   {
      kPT_Vertex,
      kPT_Fragment
   };

   struct SLowLevelStream
   {
      SLowLevelStream()
         : mStreamData( NULL )
         , mStreamStride( 0 )
         , mStreamInstanced( 0 )
      {
      }

      SLowLevelStream( void const *pData, size_t const stride )
         : mStreamData( pData )
         , mStreamStride( stride )
         , mStreamInstanced( 0 )
      {
      }

      SLowLevelStream( void const *pData, size_t const stride, uint32 const instanced )
         : mStreamData( pData )
         , mStreamStride( stride )
         , mStreamInstanced( instanced )
      {
      }

      bool operator==( SLowLevelStream const &rhs ) const
      {
         BPE_CTASSERT( sizeof( *this ) == 12 ); // Did we add members??
         return mStreamData == rhs.mStreamData && mStreamStride == rhs.mStreamStride && mStreamInstanced == rhs.mStreamInstanced;
      }

      void const *mStreamData;
      size_t mStreamStride;
      uint32 mStreamInstanced;
   };

   struct SAttributeDesc
   {
      // Due to speed and usage patterns, no default ctor for this guy

      EVertexDataUsage mVDU;
      SceGxmVertexAttribute mAttr;
   };

   class CAttributeList
   {
   public:
      CAttributeList() : mCount( 0 ) {}

      void Add( EVertexDataUsage const vdu, SceGxmVertexAttribute const &attr )
      {
         mDescs[mCount].mVDU = vdu;
         mDescs[mCount].mAttr = attr;
         ++mCount;
      }

      SAttributeDesc const *Finalize() 
      { 
         mDescs[ mCount ].mVDU = kVDU_Invalid;
         return mDescs; 
      }
   private:
      SAttributeDesc mDescs[ kVDU_Count ];
      int mCount;
   };

   struct SCachedVertexData
   {
      SCachedVertexData(CShaderVertexDataBinding const & vertexDataBinding, CVertexData const & vertexData);
      void SetBuffer_Unsafe(int const idx, void *pBuffer);
      void SetBuffer(int const idx, CVertexBuffer const *pBuffer);
      void SetBuffer(int const idx, CDynamicVertexBufferPoolChunk_UT const *pBuffer);
      void SetBuffer(int const idx, CDynamicVertexBufferPoolChunk_RT const *pBuffer);

      NVTAState::CAttributeList vtaAttributes;
      NVTAState::SLowLevelStream streamsOut[ SCE_GXM_MAX_VERTEX_STREAMS ];
      NVTAState::SLowLevelStream *pStreamTail;
   };

   static const uint32 kDS_Depth                 = 1 << 0;
   static const uint32 kDS_VertexRegs            = 1 << 1;
   static const uint32 kDS_VertexShader          = 1 << 2;
   static const uint32 kDS_FragmentShader        = 1 << 3;
   static const uint32 kDS_Cull                  = 1 << 4;
   static const uint32 kDS_Streams               = 1 << 5; // From the last index type or streams
   static const uint32 kDS_Attributes            = 1 << 6;
   static const uint32 kDS_FragmentRegs          = 1 << 7; 
   static const uint32 kDS_Count                 = 1 << 8;

   // Initialize NVTAState
   extern void Init( SceGxmContext *pContext );
   
   // Set the address of the NULL attribute (128 bytes of zeroed memory)
   extern void SetNullAttribute( void const *pMemory );

   // Flushes the states to the hardware after the render backend has been
   // initialized
   extern void FlushInitialStates();

   // Flushes all states and clears the state bits
   extern void FlushStatesAndClear();

   // Invalidates all state, which means someone set up a bunch of a hardware state outside of here
   void InvalidateHardwareState();

   // Prints some debug state
   void PrintDebugStateCounts();

   // Sets the index buffer type for the next draw
   void SetLastIndexBufferType( EIndexType );

   // Sets and override of the index source for a stream
   // Be careful to set this back when you're done
   void SetIndexSourceOverwrite( uint32 const streamIndex, SceGxmIndexSource indexSource );

   // Blend mode setting
   void BlendDisable();
   void BlendEnableAndSetSrcDst( SceGxmBlendInfo const blendMode );
   void BlendSetOp( SceGxmBlendInfo const blendMode );
   void BlendSetMask( uint8_t const blendMask );
   void BlendEnableAndSetBlend( uint32 const blendMode );
   uint32 BlendGetBlend();
   bool BlendGetEnabled();

   void DepthSetUnitEnabled(bool const enable);
   void DepthSetFunc(SceGxmDepthFunc const func);
   void DepthSetWriteEnabled(bool const enable);
   bool DepthGetWriteEnabled();

   void PolySetMode( SceGxmPolygonMode const mode );

   void CullSetMode(SceGxmCullMode const mode);

   void AddDirtyState( uint32 const state );

   void SetVertexAndFragmentProgram( 
      void *vertexProgram, void *fragmentProgram, 
      SceGxmShaderPatcherId const vertexProgramId, SceGxmShaderPatcherId const fragmentProgramId,
      SFragmentParameter const *pFragmentParameters, SVertexParameter const *pVertexParameters, SVertexSemantic const *pVertexSemantics,
      uint32 **pParameters, uint32 **pParametersEnd );


   SceGxmShaderPatcherId RegisterShaderProgram( void *pShaderProgram );
   void UnregisterShaderProgram( void *pShaderProgram );
   void WaitShaderPatcherIdleInternal();
   void UnregisterAllTemporaryPrograms();

   void SetCachedVertexData( SCachedVertexData *cvd );
   void SetAttributes( SAttributeDesc const *pFirstAttribute );
   void UpdateLowLevelStreamPointers( SLowLevelStream const *pStreams, size_t const count );
   void SetLowLevelStreams( SLowLevelStream const *pStreams, size_t const count );
   void SetupOneLowLevelStream( SLowLevelStream const &str );

   // PENDING REMOVAL
   extern EIndexType s_LastIndexBufferType;
   inline EIndexType GetLastIndexBufferType() { return s_LastIndexBufferType; }

   extern SceGxmShaderPatcher *s_shaderPatcher;
   
}
