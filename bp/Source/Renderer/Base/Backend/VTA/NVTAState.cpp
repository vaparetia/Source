//----------------------------------------------------------------------------
// NVTAState.cpp
// Bluepoint/Armature
// Copyright 2011
//----------------------------------------------------------------------------

#include "StdAfx.h"

#include "NVTAState.h"

#include <sdk_version.h>
#include <gxm.h>
#include <gxm/shader_patcher.h>
#include <gxm/structs.h>
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Material/VTA/VTACCompiledShader.h"

//#undef BPE_CHECK_SCE

namespace NVTAState
{
   void MarkAllStateDirty();

   static int const skNumShaderPatcherTrackPrograms = 4096;

   EIndexType s_LastIndexBufferType = kIT_Uint32;

   static uint32 sVertexRegStateToDirty = 0;

   struct SState
   {
      inline void Clear_IndexSourceOverwrite()
      {
         for( int ii=0;ii<SCE_GXM_MAX_VERTEX_STREAMS;++ii )
         {
            mLastIndexSourceOverwrite[ii] = (SceGxmIndexSource)-1;
         }
      }

      SState()
         : _mDirtyState( 0 )
         , mBlendInfoEnabled( false )
         , mFragmentBlendingEnabled( false )
         , mDepthUnitEnabled( false )
         , mLastPolygonMode( SCE_GXM_POLYGON_MODE_TRIANGLE_FILL )
         , mLastCullMode( SCE_GXM_CULL_NONE )
         , mpCurrentUnpatchedVertexProgramPtr( NULL )
         , mpCurrentUnpatchedFragmentProgramPtr( NULL )
         , mCurrentVertexProgramId( NULL )
         , mCurrentFragmentProgramId( NULL )
         , mpCurrentFragmentParameters( NULL )
         , mpCurrentVertexParameters( NULL )
         , mppCntParametersBegin( NULL )
         , mppCntParametersEnd( NULL )
         , mNormalStreamCount( 0 )
         , mNullStreamCount( 0 )
         , mGxmAttributeValid( 0 )
         , mShaderPatcherVertexProgramsNeedHousecleaning( false )
         , mShaderPatcherFragmentProgramsNeedHousecleaning( false )
         , mLastCachedVertexData( NULL )
      {
         mBlendInfo = (SceGxmBlendInfo) { 0 };
         mBlendInfo.colorMask = SCE_GXM_COLOR_MASK_ALL;
         mBlendInfo.colorFunc = SCE_GXM_BLEND_FUNC_NONE;
         mBlendInfo.alphaFunc = SCE_GXM_BLEND_FUNC_NONE;
         mBlendInfo.colorSrc = SCE_GXM_BLEND_FACTOR_ONE;
         mBlendInfo.alphaSrc = SCE_GXM_BLEND_FACTOR_ONE;
         mBlendInfo.colorDst = SCE_GXM_BLEND_FACTOR_ZERO;
         mBlendInfo.alphaDst = SCE_GXM_BLEND_FACTOR_ZERO;

         Clear_IndexSourceOverwrite();

         mDepthFuncs[0] = SCE_GXM_DEPTH_FUNC_ALWAYS;
         mDepthFuncs[1] = SCE_GXM_DEPTH_FUNC_ALWAYS;

         mDepthWriteModes[0] = SCE_GXM_DEPTH_WRITE_DISABLED;
         mDepthWriteModes[1] = SCE_GXM_DEPTH_WRITE_DISABLED;

         MarkAllStateDirty();
      }

      uint32 _mDirtyState;

      SceGxmBlendInfo mBlendInfo;
      bool mBlendInfoEnabled;

      bool mFragmentBlendingEnabled;

      bool mDepthUnitEnabled;
      // following two indexed on depth unit enabled
      SceGxmDepthFunc mDepthFuncs[2];
      SceGxmDepthWriteMode mDepthWriteModes[2];

      SceGxmPolygonMode mLastPolygonMode;
      SceGxmCullMode mLastCullMode;

      SceGxmProgram const *mpCurrentUnpatchedVertexProgramPtr;
      SceGxmProgram const *mpCurrentUnpatchedFragmentProgramPtr;

      SceGxmShaderPatcherId mCurrentVertexProgramId;
      SceGxmShaderPatcherId mCurrentFragmentProgramId;
      SFragmentParameter const *mpCurrentFragmentParameters;
      SVertexParameter const *mpCurrentVertexParameters;
      SVertexSemantic const *mpCurrentVertexSemantics;

      uint32 **mppCntParametersBegin;
      uint32 **mppCntParametersEnd;

      size_t mNormalStreamCount;
      size_t mNullStreamCount;
      SceGxmVertexStream mCachedFinalStreams[ SCE_GXM_MAX_VERTEX_STREAMS ];

      SceGxmVertexAttribute mGxmAttributes[ kVDU_Count ]; // Number of semantics
      uint32 mGxmAttributeValid;

      SceGxmVertexAttribute mCachedFinalAttributes[ kVDU_Count ]; // Number of semantics
      size_t mCachedFinalAttributeCount;

      SceGxmIndexSource mLastIndexSourceOverwrite[SCE_GXM_MAX_VERTEX_STREAMS];

      bool mShaderPatcherVertexProgramsNeedHousecleaning;
      bpe::reserved_vector< SceGxmVertexProgram *, skNumShaderPatcherTrackPrograms > mShaderPatcherVertexPrograms;
      bool mShaderPatcherFragmentProgramsNeedHousecleaning;
      bpe::reserved_vector< SceGxmFragmentProgram *, skNumShaderPatcherTrackPrograms > mShaderPatcherFragmentPrograms;

      SCachedVertexData *mLastCachedVertexData;
   };

   typedef void ( *TFlushFunction )();
   struct SFlushFunctionOffsets 
   { 
      SFlushFunctionOffsets() : mBegin( 0 ), mEnd( 0 ) {}
      SFlushFunctionOffsets( size_t begin, size_t end ) : mBegin( begin ), mEnd( end ) {}

      size_t mBegin; 
      size_t mEnd; 
   };


   
   struct SFragmentShaderHistoryItem
   {
      SFragmentShaderHistoryItem()
         : mFragmentProgramId( 0 )
         , mBlendMode( 0 )
         , mpVertexProgram( 0 )
         , mpResultantProgram( 0 )
         , mOutputFormat( SCE_GXM_OUTPUT_REGISTER_FORMAT_UCHAR4 )
         , mMultisampleMode( SCE_GXM_MULTISAMPLE_NONE )
      {
      }

      SFragmentShaderHistoryItem(
         SceGxmShaderPatcherId fragmentProgramId,
         SceGxmBlendInfo const &blendMode,
         SceGxmProgram const *pVertexProgram,
         SceGxmFragmentProgram *pResultantProgram,
         SceGxmOutputRegisterFormat outputFormat,
         SceGxmMultisampleMode multisampleMode)
         : mFragmentProgramId( fragmentProgramId )
         , mBlendMode( *reinterpret_cast<uint32 const *>( &blendMode ) )
         , mpVertexProgram( pVertexProgram )
         , mpResultantProgram( pResultantProgram )
         , mOutputFormat( outputFormat )
         , mMultisampleMode( multisampleMode )
      {
         // We assume that the SceGxmBlendInfo struct is a word
         BPE_CTASSERT( sizeof( SceGxmBlendInfo ) == sizeof( mBlendMode ) );
      }

      SceGxmShaderPatcherId mFragmentProgramId;
      uint32 mBlendMode;
      SceGxmProgram const *mpVertexProgram;
      SceGxmFragmentProgram *mpResultantProgram;
      SceGxmOutputRegisterFormat mOutputFormat;
      SceGxmMultisampleMode mMultisampleMode;
   };

   static char const *skStateNames[] =
   {
      "Depth",
      "VertexRegs",
      "Vertex Shader",
      "Fragment Shader",
      "Cull",
      "Streams",
      "Attributes",
      "FragmentRegs",
   };
   
   BPE_CTASSERT( ( 1 << BPE_ARRAY_SIZE( skStateNames ) ) == kDS_Count );

   static const uint32 kDS_Blend = kDS_FragmentShader;

   static const uint32 skStatesCausingAttributeFlush = kDS_Attributes;
   static const uint32 skStatesCausingStreamFlush = kDS_Streams; // NOTE - needs attributes because attribute could cause stream change
   static const uint32 skStatesCausingVertexShaderFlush = skStatesCausingAttributeFlush | skStatesCausingStreamFlush | kDS_VertexShader;
   static const uint32 skStatesCausingFragmentShaderFlush = kDS_FragmentShader;
   static const uint32 skStatesCausingVertexRegisterFlush = kDS_VertexRegs;
   static const uint32 skStatesCausingFragmentRegisterFlush = kDS_FragmentRegs;

   SceGxmContext *sContext = NULL;
   SState sState;
   void const *mpNullAttribute = NULL;
   SceGxmShaderPatcher *s_shaderPatcher = NULL;
   float32x4_t mVertexRegisters[ 256 ];
   float32x4_t mFragmentRegisters[ 256 ];

   typedef std::map<uint32, SceGxmShaderPatcherId > TShaderProgramMap;
   TShaderProgramMap sShaderProgramMap;

   std::vector< TFlushFunction > sFlushFunctionTable;
   SFlushFunctionOffsets sFlushFunctionTableOffsets[ kDS_Count ];

   static int const skFragmentShaderHistoryCount = 4;
   SFragmentShaderHistoryItem sFragmentShaderHistory[ skFragmentShaderHistoryCount ];
   static int sFragmentShaderHistoryIndex = 0;

   void FlushDepth();
   void FlushCull();
   void ManualState_FlushPolyMode();
   void FlushAttributeState();
   void FlushStreamState();
   void FlushVertexShaderState();
   void FlushFragmentShaderState();
   void FlushVertexRegisters();
   void FlushFragmentRegisters();
   void InitFlushFunctionTable();

   void Init( SceGxmContext *pContext )
   {
      sContext = pContext;

      SState *pState = &sState;

      pState->~SState();
      new(pState) SState();

      InitFlushFunctionTable();
      memset( mVertexRegisters, 0, sizeof( mVertexRegisters ) );

      sShaderProgramMap.clear();
   }

   void InitFlushFunctionTable()
   {
      sFlushFunctionTable.clear();

      for ( size_t state = 0; state < kDS_Count; ++state )
      {
         int const begin = sFlushFunctionTable.size();

         if ( state & kDS_Depth )                              sFlushFunctionTable.push_back( &FlushDepth );
         if ( state & kDS_Cull )                               sFlushFunctionTable.push_back( &FlushCull );
         if ( state & skStatesCausingAttributeFlush )          sFlushFunctionTable.push_back( &FlushAttributeState );
         if ( state & skStatesCausingStreamFlush )             sFlushFunctionTable.push_back( &FlushStreamState );
         if ( state & skStatesCausingVertexShaderFlush )       sFlushFunctionTable.push_back( &FlushVertexShaderState );
         if ( state & skStatesCausingFragmentShaderFlush )     sFlushFunctionTable.push_back( &FlushFragmentShaderState );
         if ( state & skStatesCausingVertexRegisterFlush )     sFlushFunctionTable.push_back( &FlushVertexRegisters );
         if ( state & skStatesCausingFragmentRegisterFlush )   sFlushFunctionTable.push_back( &FlushFragmentRegisters );

         int const end = sFlushFunctionTable.size();

         sFlushFunctionTableOffsets[ state ] = SFlushFunctionOffsets( begin, end );
      }
   }

   template <class T>
   inline void assign_and_dirty( T *dst, T const &src, uint32 const dirtyState )
   {
      if ( *dst != src )
      {
         *dst = src;
         sState._mDirtyState |= dirtyState;
      }
   }

   void AddDirtyState( uint32 const state )
   {
      sState._mDirtyState |= state;
   }

   void MarkAllStateDirty()
   {
      sState._mDirtyState = kDS_Count - 1;
   }

#ifdef VITA_PROFILE_STATE_CHANGES
   static uint32 sStateUsageCounts[8] = { 0 };

   static uint32 sTotalStateChange = 0;
   static uint32 sTotalPrim = 0;

   void add_state_to_count( uint32 const state )
   {
      for( int i = 0; ( 1 << i ) != kDS_Count; ++i )
      {
         if ( state & ( 1 << i ) )
         {
            ++( sStateUsageCounts[ i ] );
         }
      }

      if ( state )
      {
         ++sTotalStateChange;
      }

      ++sTotalPrim;
   }

   void clear_state_counts()
   {
      for ( int i = 0; i < BPE_ARRAY_SIZE( sStateUsageCounts ); ++i )
      {
         sStateUsageCounts[i] = 0;
      }

      sTotalStateChange = 0;
      sTotalPrim = 0;
   }

   void PrintDebugStateCounts()
   {
      for ( int i = 0; i < 8; ++i )
      {
#if 0
         if ( i == 4 || i == 7 ) 
         {
            printf( "\n" );
         }
#endif
         printf( "%s: %d ", skStateNames[i], sStateUsageCounts[i] );
      }

      printf( "\nTotal State: %d Total Prim: %d\n", sTotalStateChange, sTotalPrim );

      clear_state_counts();
   }
#else
   inline void add_state_to_count( uint32 const ) {}
   void PrintDebugStateCounts() {}
#endif

   //----------------------------------------------------------------------------

   void SetupOneLowLevelStream( SLowLevelStream const &str )
   {
      BPE_CHECK_SCE( sceGxmSetVertexStream( sContext, 0, str.mStreamData ) );

      sState.mLastCachedVertexData = NULL;

      if ( sState.mNormalStreamCount != 1 || sState.mCachedFinalStreams[0].stride != str.mStreamStride || 
           !!(sState.mCachedFinalStreams[0].indexSource & 2) != !!str.mStreamInstanced)
      {
         sState.mNormalStreamCount = 1;
         sState.mCachedFinalStreams[0].stride = str.mStreamStride;
         sState.mCachedFinalStreams[0].indexSource = str.mStreamInstanced ? SCE_GXM_INDEX_SOURCE_INSTANCE_16BIT : SCE_GXM_INDEX_SOURCE_INDEX_16BIT;
         for ( int i = 1; i < SCE_GXM_MAX_VERTEX_STREAMS; ++i )
         {
            sState.mCachedFinalStreams[i].stride = 0;
            sState.mCachedFinalStreams[i].indexSource = 0;
         }

         AddDirtyState( kDS_VertexShader | kDS_Streams | kDS_VertexRegs );
      }
   }

   //----------------------------------------------------------------------------

   void UpdateLowLevelStreamPointers( SLowLevelStream const *pStreams, size_t const count )
   {
      for ( size_t streamIndex = 0; streamIndex < count; ++streamIndex )
      {
         BPE_CHECK_SCE( sceGxmSetVertexStream( sContext, streamIndex, pStreams[streamIndex].mStreamData ) );
      }
   }

   void SetLowLevelStreams( SLowLevelStream const *pStreams, size_t const count )
   {
      sState.mLastCachedVertexData = NULL;

      for ( size_t streamIndex = 0; streamIndex < count; ++streamIndex )
      {
         BPE_CHECK_SCE( sceGxmSetVertexStream( sContext, streamIndex, pStreams[streamIndex].mStreamData ) );
         
         if ( pStreams[ streamIndex ].mStreamStride != sState.mCachedFinalStreams[ streamIndex ].stride )
         {
            AddDirtyState( kDS_VertexShader );
            sState.mCachedFinalStreams[ streamIndex ].stride = pStreams[ streamIndex ].mStreamStride;
            AddDirtyState( sVertexRegStateToDirty );
         }
         if ( !!pStreams[ streamIndex ].mStreamInstanced != !!(sState.mCachedFinalStreams[ streamIndex ].indexSource & 2) )
         {
            AddDirtyState( kDS_Streams );
            sState.mCachedFinalStreams[ streamIndex ].indexSource = pStreams[ streamIndex ].mStreamInstanced ? SCE_GXM_INDEX_SOURCE_INSTANCE_16BIT : SCE_GXM_INDEX_SOURCE_INDEX_16BIT;
            AddDirtyState( sVertexRegStateToDirty );
         }
      }

      for ( size_t streamIndex = count; streamIndex < SCE_GXM_MAX_VERTEX_STREAMS; ++streamIndex )
      {
         sState.mCachedFinalStreams[ streamIndex ].stride = 0;
         sState.mCachedFinalStreams[ streamIndex ].indexSource = 0;
      }

      if ( sState.mNormalStreamCount != count )
      {
         sState.mNormalStreamCount = count;
         AddDirtyState( kDS_VertexShader );
      }
   }

   void FlushDepth()
   {
      bool const enabled = sState.mDepthUnitEnabled
#ifdef VITA_DEBUG_DEPTH
         && !( gVTADebugDepthDrawNum < mCurrentSceneDrawNum && gVTADebugDepthDrawNum != -1 ) 
#endif
         ;

      sceGxmSetFrontDepthWriteEnable( sContext, sState.mDepthWriteModes[ enabled ] );
      sceGxmSetFrontDepthFunc( sContext, sState.mDepthFuncs[ enabled ] );
   }

   // This function manually flushes the poly mode, which is a state that's tracked but only
   // set when necessary instead of being implicitly flushed by the dirty state bits.
   inline void ManualState_FlushPolyMode()
   {
      sceGxmSetFrontPolygonMode( sContext, sState.mLastPolygonMode );
   }

   inline void FlushCull()
   {
      sceGxmSetCullMode( sContext, sState.mLastCullMode );
   }

   void FlushAttributeState()
   {
      SceGxmProgram const *vProgram = sceGxmShaderPatcherGetProgramFromId( sState.mCurrentVertexProgramId );

      sState.mCachedFinalAttributeCount = 0;
      sState.mNullStreamCount = 0;

      for ( SVertexSemantic const *pParam = sState.mpCurrentVertexSemantics; pParam->mVDUType != 0xFF; ++pParam )
      {
         EVertexDataUsage vduIndex = EVertexDataUsage( pParam->mVDUType );

         if ( sState.mGxmAttributeValid & ( 1 << vduIndex ) )
         {
            sState.mCachedFinalAttributes[ sState.mCachedFinalAttributeCount ] = sState.mGxmAttributes[ vduIndex ];
         }
         else
         {
            // Load the null attribute as the last stream
            if ( sState.mNullStreamCount == 0 )
            {
               sState.mNullStreamCount = 1;
               BPE_CHECK_SCE( sceGxmSetVertexStream( sContext, sState.mNormalStreamCount, mpNullAttribute ) );
            }

            sState.mCachedFinalAttributes[ sState.mCachedFinalAttributeCount ].streamIndex = sState.mNormalStreamCount;
            sState.mCachedFinalAttributes[ sState.mCachedFinalAttributeCount ].offset = 0;
            sState.mCachedFinalAttributes[ sState.mCachedFinalAttributeCount ].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;
            sState.mCachedFinalAttributes[ sState.mCachedFinalAttributeCount ].componentCount = 4;
         }

         sState.mCachedFinalAttributes[ sState.mCachedFinalAttributeCount++ ].regIndex = pParam->mRegisterIndex;

         //      printf( "Param %d: %s - %d\n",
         //         paramIndex, sceGxmProgramParameterGetName( param ), sceGxmProgramParameterGetResourceIndex( param ) );
      }
   }

   void FlushStreamState()
   {
      BPE_CTASSERT( SCE_GXM_INDEX_SOURCE_INDEX_16BIT == ( kIT_Uint16 ^ 1 ) );
      BPE_CTASSERT( SCE_GXM_INDEX_SOURCE_INDEX_32BIT == ( kIT_Uint32 ^ 1 ) );

      SceGxmIndexSource indexSource = SceGxmIndexSource( int(s_LastIndexBufferType) ^ 1 );

      for ( int i = 0; i < BPE_ARRAY_SIZE( sState.mCachedFinalStreams ); ++i )
      {
         sState.mCachedFinalStreams[i].indexSource = ( sState.mLastIndexSourceOverwrite[i] == -1 ) ? ((indexSource&1) | (sState.mCachedFinalStreams[i].indexSource&2)) : sState.mLastIndexSourceOverwrite[i];
      }
   }

   void RegisterTemporaryFragmentProgram( SceGxmFragmentProgram *tempFragmentProgram )
   {
      // If the program's ref count is 1, that means it just now got created.
      // Cache it if that's the case.
      // This assumes PreRegisterTemporary*Program has been called

      uint32_t refcount = 0;
      BPE_CHECK_SCE( sceGxmShaderPatcherGetFragmentProgramRefCount( s_shaderPatcher, tempFragmentProgram, &refcount ) );
      if ( refcount == 1 )
      {
         // Just got created.  Register it.
         sState.mShaderPatcherFragmentPrograms.push_back( tempFragmentProgram );
      }
      else
      {
         // We need to decrement the ref count to get it back to 1
         BPE_CHECK_SCE( sceGxmShaderPatcherReleaseFragmentProgram( s_shaderPatcher, tempFragmentProgram ) );
      }
   }

   void UnregisterAllTemporaryFragmentProgramsInternal()
   {
      sState.mShaderPatcherFragmentProgramsNeedHousecleaning = false;

      for ( int programIndex = 0; programIndex < sState.mShaderPatcherFragmentPrograms.size(); ++programIndex )
      {
         uint32_t refcount = 0;
         BPE_CHECK_SCE( sceGxmShaderPatcherGetFragmentProgramRefCount( s_shaderPatcher, sState.mShaderPatcherFragmentPrograms[ programIndex ], &refcount ) );

         for ( int i = 0; i < refcount; ++i )
         {
            sceGxmShaderPatcherReleaseFragmentProgram( s_shaderPatcher, sState.mShaderPatcherFragmentPrograms[ programIndex ] );
         }
      }

      sState.mShaderPatcherFragmentPrograms.clear();
      for ( int i = 0; i < BPE_ARRAY_SIZE( sFragmentShaderHistory ); ++i )
      {
         sFragmentShaderHistory[ i ] = SFragmentShaderHistoryItem();
      }
   }

   void UnregisterAllTemporaryFragmentPrograms()
   {
      printf( "BP WARNING: Unregistering all temp fragment programs\n" );

      WaitShaderPatcherIdleInternal();
      UnregisterAllTemporaryFragmentProgramsInternal();
   }

   void PreRegisterTemporaryFragmentProgram()
   {
      // Make sure that we have room for a new program. If not, unregister.

      if ( sState.mShaderPatcherFragmentPrograms.size() == sState.mShaderPatcherFragmentPrograms.capacity() ||
         sState.mShaderPatcherFragmentProgramsNeedHousecleaning )
      {
         UnregisterAllTemporaryFragmentPrograms();
      }
   }

//#define DEBUG_TEMP_VERTEX_PROGRAMS

   void RegisterTemporaryVertexProgram( SceGxmVertexProgram *tempVertexProgram )
   {
      // If the program's ref count is 1, that means it just now got created.
      // Cache it if that's the case.
      // This assumes PreRegisterTemporary*Program has been called

      uint32_t refcount = 0;
      BPE_CHECK_SCE( sceGxmShaderPatcherGetVertexProgramRefCount( s_shaderPatcher, tempVertexProgram, &refcount ) );
      if ( refcount == 1 )
      {
         // Just got created.  Register it.

#ifdef DEBUG_TEMP_VERTEX_PROGRAMS
         // Make sure that we haven't added this already
         BPE_VERIFY( std::find( sState.mShaderPatcherVertexPrograms.begin(), sState.mShaderPatcherVertexPrograms.end(), tempVertexProgram ) == 
            sState.mShaderPatcherVertexPrograms.end(), false, "Trying to add two of the same refcount 1 vertex programs?!" );

#endif

         sState.mShaderPatcherVertexPrograms.push_back( tempVertexProgram );

      }
      else
      {
         // We need to decrement the ref count to get it back to 1
         BPE_CHECK_SCE( sceGxmShaderPatcherReleaseVertexProgram( s_shaderPatcher, tempVertexProgram ) );

#ifdef DEBUG_TEMP_VERTEX_PROGRAMS
         // Make sure that it's already here!

         if ( std::find( sState.mShaderPatcherVertexPrograms.begin(), sState.mShaderPatcherVertexPrograms.end(), tempVertexProgram ) == 
            sState.mShaderPatcherVertexPrograms.end() )
         {
            printf( "ERROR: Couldn't find %8.8x rc %d in list\n", tempVertexProgram, refcount );
            BPE_VERIFYA( false, "Couldn't find an item in the vertex program list" );
         }
#endif
      }
   }

   void UnregisterAllTemporaryVertexProgramsInternal()
   {
      sState.mShaderPatcherVertexProgramsNeedHousecleaning = false;

      for ( int programIndex = 0; programIndex < sState.mShaderPatcherVertexPrograms.size(); ++programIndex )
      {
         uint32_t refcount = 0;
         BPE_CHECK_SCE( sceGxmShaderPatcherGetVertexProgramRefCount( s_shaderPatcher, sState.mShaderPatcherVertexPrograms[ programIndex ], &refcount ) );

         for ( int i = 0; i < refcount; ++i )
         {
            sceGxmShaderPatcherReleaseVertexProgram( s_shaderPatcher, sState.mShaderPatcherVertexPrograms[ programIndex ] );
         }
      }
      sState.mShaderPatcherVertexPrograms.clear();

   }

   void UnregisterAllTemporaryVertexPrograms()
   {
      printf( "BP WARNING: Unregistering all temp vertex programs\n" );

      WaitShaderPatcherIdleInternal();
      UnregisterAllTemporaryVertexProgramsInternal();
   }

   void PreRegisterTemporaryVertexProgram()
   {
      // Make sure that we have room for a new program. If not, unregister.

      if ( sState.mShaderPatcherVertexPrograms.size() == sState.mShaderPatcherVertexPrograms.capacity() ||
         sState.mShaderPatcherVertexProgramsNeedHousecleaning )
      {
         UnregisterAllTemporaryVertexPrograms();
      }
   }

   void FlushVertexShaderState()
   {
      SceGxmProgram const *vProgram = sceGxmShaderPatcherGetProgramFromId( sState.mCurrentVertexProgramId );
      SceGxmVertexProgram *tempVertexProgram = NULL;

      PreRegisterTemporaryVertexProgram();

//#define DEBUG_VERTEX_PROGRAM_FAILURE

#ifdef DEBUG_VERTEX_PROGRAM_FAILURE
      static int sFailCheck = 1000;

      if ( --sFailCheck == 0 )
      {
         UnregisterAllTemporaryVertexPrograms();
         sFailCheck = 2000;
      }
#endif


      if ( SCE_OK != sceGxmShaderPatcherCreateVertexProgram( 
         s_shaderPatcher, sState.mCurrentVertexProgramId, 
         sState.mCachedFinalAttributes, sState.mCachedFinalAttributeCount, 
         sState.mCachedFinalStreams, sState.mNormalStreamCount + sState.mNullStreamCount,
         &tempVertexProgram )  )
      {
         UnregisterAllTemporaryVertexPrograms();

         BPE_CHECK_SCE( sceGxmShaderPatcherCreateVertexProgram( 
            s_shaderPatcher, sState.mCurrentVertexProgramId, 
            sState.mCachedFinalAttributes, sState.mCachedFinalAttributeCount, 
            sState.mCachedFinalStreams, sState.mNormalStreamCount + sState.mNullStreamCount,
            &tempVertexProgram ) );
      }

      RegisterTemporaryVertexProgram( tempVertexProgram );

      sceGxmSetVertexProgram( sContext, tempVertexProgram );
   }

   void FlushFragmentShaderState()
   {
      SceGxmProgram const *vProgram = sState.mpCurrentUnpatchedVertexProgramPtr;
      SceGxmProgram const *fProgram = sState.mpCurrentUnpatchedFragmentProgramPtr;

      SceGxmBlendInfo info = { 0 };
      SceGxmBlendInfo *pInfoPtr;

      // If we're using FRAGCOLOR  or returning __nativecolor, then the shader patcher cannot apply 
      // blending.
      if ( sState.mFragmentBlendingEnabled )
      {
         // Otherwise let's mutate the incoming blend info based 
         // on if the backend thinks that alpha blending is enabled
         // Note that color/alpha write flags are unaffected

         info = sState.mBlendInfo;
         pInfoPtr = &info;

         if ( sState.mBlendInfoEnabled == false )
         {
            info.colorFunc = SCE_GXM_BLEND_FUNC_NONE;
            info.alphaFunc = SCE_GXM_BLEND_FUNC_NONE;
         }
      }
      else
      {
         pInfoPtr = NULL;
      }

      SceGxmFragmentProgram *tempFragmentProgram = NULL;
      uint32 const blendInfoAsUInt = *reinterpret_cast<uint32 const *>( &info );
      
      SceGxmOutputRegisterFormat outputFormat = gpRenderBackend->GetCurrentOutputFormat();
      SceGxmMultisampleMode multisampleMode = gpRenderBackend->GetCurrentMultisampleMode();

      // Search a small fragment shader cache to see if we've built this recently
      for ( int foundShader = 0; foundShader < skFragmentShaderHistoryCount; ++foundShader )
      {
         SFragmentShaderHistoryItem const &item = sFragmentShaderHistory[ foundShader ];

         if ( item.mBlendMode == blendInfoAsUInt && item.mFragmentProgramId == sState.mCurrentFragmentProgramId && item.mpVertexProgram == vProgram && item.mOutputFormat == outputFormat && item.mMultisampleMode == multisampleMode )
         {
            // We're good!

            tempFragmentProgram = item.mpResultantProgram;
            break;
         }
      }

      // No fragment program, let's find one and create a new history item
      if ( tempFragmentProgram == NULL )
      {
         PreRegisterTemporaryFragmentProgram();

         if ( SCE_OK != sceGxmShaderPatcherCreateFragmentProgram( 
            s_shaderPatcher, sState.mCurrentFragmentProgramId, 
            outputFormat,
            multisampleMode,
            pInfoPtr,
            vProgram,
            &tempFragmentProgram ) )
         {
            // We failed to create a fragment program, so let's 
            // wipe the slate clean and try again

            UnregisterAllTemporaryFragmentPrograms();

            BPE_CHECK_SCE( sceGxmShaderPatcherCreateFragmentProgram( 
               s_shaderPatcher, sState.mCurrentFragmentProgramId, 
               outputFormat,
               multisampleMode,
               pInfoPtr,
               vProgram,
               &tempFragmentProgram ) );
         }

         RegisterTemporaryFragmentProgram( tempFragmentProgram );

         sFragmentShaderHistory[ sFragmentShaderHistoryIndex ] = SFragmentShaderHistoryItem( sState.mCurrentFragmentProgramId, info, vProgram, tempFragmentProgram, outputFormat, multisampleMode );
         sFragmentShaderHistoryIndex = ( sFragmentShaderHistoryIndex + 1 ) % skFragmentShaderHistoryCount;

      }

      sceGxmSetFragmentProgram( sContext, tempFragmentProgram );
   }

   //----------------------------------------------------------------------------

#if 0
   static inline void myGxmSetUniformDataF( void *uniform, SceGxmProgramParameter const *pParam, uint32_t const offset, int const totalSize, vector float const *registers )
   {
      SceGxmParameterType type = sceGxmProgramParameterGetType( pParam );
      size_t wordOffset = offset + sceGxmProgramParameterGetResourceIndex( pParam );
      void *pOut = reinterpret_cast<uint32 *>( uniform )+ wordOffset;

      switch ( type )
      {
      case SCE_GXM_PARAMETER_TYPE_F32:
         {
            float32x4_t const *pRead = registers;
            float32x4_t *pWrite = reinterpret_cast<float32x4_t *>( pOut );
            for ( int i = 0; i < totalSize; i += 4 )
            {
               *(pWrite++) = *(pRead++);
            }
         }
         break;
      case SCE_GXM_PARAMETER_TYPE_F16:
         {
            float32x4_t const *pRead = registers;
            float16x4_t *pWrite = reinterpret_cast<float16x4_t *>( pOut );
            for ( int i = 0; i < totalSize; i += 4 )
            {
               *(pWrite++) = vcvt_f16_f32( *(pRead++) );
            }
         }
         break;
      default:
         {
            BPE_ASSERTA( "Invalid gxm parameter type!" );
         }
         break;
      }
   }
#endif
   //----------------------------------------------------------------------------

   static void add_vertex_uniforms_from_registers( SceGxmProgram const *program, void *uniform, SVertexParameter const *pParameters )
   {
      for ( SVertexParameter const *pParam = pParameters; pParam->mCRC; ++pParam )
      {
         if ( pParam->mBackendRegister != gkUint16Max )
         {
            SceGxmProgramParameter const *gxmParam = sceGxmProgramGetParameter( program, pParam->mParameterIndex );

            int const totalSize = pParam->mTotalFloatParameterCount;

            sceGxmSetUniformDataF( uniform, gxmParam, 0, totalSize, (const float *) gpRenderBackend->GetVertexRegsDataPtr( pParam->mBackendRegister ) );
            //            myGxmSetUniformDataF( uniform, gxmParam, 0, totalSize, gpRenderBackend->GetVertexRegsDataPtr( pParam->mBackendRegister ) );

         }
      }
   }

   //----------------------------------------------------------------------------

   static void add_fragment_uniforms_from_registers( SceGxmProgram const *program, void *uniform, SFragmentParameter const *pParameters )
   {
      for ( SFragmentParameter const *pParam = pParameters; pParam->mCRC; ++pParam )
      {
         if ( pParam->mBackendRegister != gkUint16Max )
         {
            SceGxmProgramParameter const *gxmParam = sceGxmProgramGetParameter( program, pParam->mParameterIndex );

            int const totalSize = pParam->mTotalFloatParameterCount;

            sceGxmSetUniformDataF( uniform, gxmParam, 0, totalSize, (const float *) gpRenderBackend->GetFragmentRegsDataPtr( pParam->mBackendRegister ) );

         }
      }
   }

   //----------------------------------------------------------------------------

   static void add_vertex_uniforms_from_params( SceGxmProgram const *program, void *uniform, uint32 **ppParamBufferBegin, uint32 **ppParamBufferEnd, SVertexParameter const *pVertexParam )
   {
      SVertexParameter const *pCurrentVtxParam = pVertexParam;
      uint32 **ppParamBufferCurrent = ppParamBufferBegin;

      while ( pCurrentVtxParam->mCRC && ppParamBufferCurrent != ppParamBufferEnd )
      {
         uint32 const *pParamBufferParam = *ppParamBufferCurrent;
         uint32 const paramBufferCrc = pParamBufferParam[0];

         if ( pCurrentVtxParam->mCRC == paramBufferCrc )
         {
            // The same!
            SceGxmProgramParameter const *gxmParam = sceGxmProgramGetParameter( program, pCurrentVtxParam->mParameterIndex );

            int const totalSize = pCurrentVtxParam->mTotalFloatParameterCount;
            BPE_CHECK_SCE( sceGxmSetUniformDataF( uniform, gxmParam, 0, totalSize, reinterpret_cast<const float *>( pParamBufferParam ) + 2 ) );

            ++pCurrentVtxParam;
            ++ppParamBufferCurrent;
         }
         else if ( pCurrentVtxParam->mCRC < paramBufferCrc )
         {
            // Current vertex param ccrc is less than the buffer one, increment and try next
            ++pCurrentVtxParam;
         }
         else // pCurrentVtxParam.mCRC > paramBufferCrc
         {
            ++ppParamBufferCurrent;
         }
      }
   }

   //----------------------------------------------------------------------------

   static void add_fragment_uniforms_from_params( SceGxmProgram const *program, void *uniform, uint32 **ppParamBufferBegin, uint32 **ppParamBufferEnd, SFragmentParameter const *pFragmentParam )
   {
      SFragmentParameter const *pCurrentFragParam = pFragmentParam;
      uint32 **ppParamBufferCurrent = ppParamBufferBegin;

      while ( pCurrentFragParam->mCRC && ppParamBufferCurrent != ppParamBufferEnd )
      {
         uint32 const *pParamBufferParam = *ppParamBufferCurrent;
         uint32 const paramBufferCrc = pParamBufferParam[0];

         if ( pCurrentFragParam->mCRC == paramBufferCrc )
         {
            // The same!
            SceGxmProgramParameter const *gxmParam = sceGxmProgramGetParameter( program, pCurrentFragParam->mParameterIndex );

            int const totalSize = pCurrentFragParam->mTotalFloatParameterCount;
            BPE_CHECK_SCE( sceGxmSetUniformDataF( uniform, gxmParam, 0, totalSize, reinterpret_cast<const float *>( pParamBufferParam ) + 2 ) );

            ++pCurrentFragParam;
            ++ppParamBufferCurrent;
         }
         else if ( pCurrentFragParam->mCRC < paramBufferCrc )
         {
            // Current vertex param ccrc is less than the buffer one, increment and try next
            ++pCurrentFragParam;
         }
         else // pCurrentVtxParam.mCRC > paramBufferCrc
         {
            ++ppParamBufferCurrent;
         }
      }
   }

   void FlushVertexRegisters()
   {
      void *uniform;
      sceGxmReserveVertexDefaultUniformBuffer( gpRenderBackend->Context(), &uniform );
      if ( uniform )
      {
         add_vertex_uniforms_from_registers( sState.mpCurrentUnpatchedVertexProgramPtr, uniform, sState.mpCurrentVertexParameters );
         add_vertex_uniforms_from_params( sState.mpCurrentUnpatchedVertexProgramPtr, uniform, 
            sState.mppCntParametersBegin, sState.mppCntParametersEnd, sState.mpCurrentVertexParameters );
      }
   }

   void FlushFragmentRegisters()
   {
      void *uniform;
      sceGxmReserveFragmentDefaultUniformBuffer( gpRenderBackend->Context(), &uniform );
      if ( uniform )
      {
         add_fragment_uniforms_from_registers( sState.mpCurrentUnpatchedFragmentProgramPtr, uniform, sState.mpCurrentFragmentParameters );
         add_fragment_uniforms_from_params( sState.mpCurrentUnpatchedFragmentProgramPtr, uniform, 
            sState.mppCntParametersBegin, sState.mppCntParametersEnd, sState.mpCurrentFragmentParameters );
      }
   }

   inline void ClearDirtyState()
   {
      sState._mDirtyState = 0;
   }

   void FlushStatesAndClear()
   {
      BPE_ASSERT( sState._mDirtyState < kDS_Count, "Dirty state out of bounds" );

      if (!sState._mDirtyState)
         return;

      SFlushFunctionOffsets const offsets = sFlushFunctionTableOffsets[ sState._mDirtyState ];

      for ( int i = offsets.mBegin; i != offsets.mEnd; ++i )
      {
         (sFlushFunctionTable[ i ])();
      }

      add_state_to_count( sState._mDirtyState );

      ClearDirtyState();
   }

}

void NVTAState::SetNullAttribute( void const *pMemory )
{
   mpNullAttribute = pMemory;
}

void NVTAState::FlushInitialStates()
{
  FlushDepth();
  FlushCull();
  ManualState_FlushPolyMode();
}

void NVTAState::SetLastIndexBufferType( EIndexType indexType )
{
   if ( s_LastIndexBufferType != indexType )
   {
      s_LastIndexBufferType = indexType;
      AddDirtyState( kDS_Streams );
   }
}

void NVTAState::SetIndexSourceOverwrite( uint32 const streamIndex, SceGxmIndexSource indexSource )
{
   if ( sState.mLastIndexSourceOverwrite[streamIndex] != indexSource )
   {
      sState.mLastIndexSourceOverwrite[streamIndex] = indexSource;
      AddDirtyState( kDS_Streams );
   }
}

#if 0

// ASSUMPTION - We are assumping that SceGxmBlendInfo still matches this.

#include <include_common/gxm/blending.h>

typedef struct SceGxmBlendInfo {
   uint8_t colorMask;
   uint8_t colorFunc:4;
   uint8_t alphaFunc:4;
   uint8_t colorSrc:4;
   uint8_t colorDst:4;
   uint8_t alphaSrc:4;
   uint8_t alphaDst:4;
} SceGxmBlendInfo;
#endif

struct SceGxmBlendInfoAlternate
{
   uint8_t colorMask;
   uint8_t colorAlphaFuncs;
   uint16_t colorSrcDst;
};

BPE_CTASSERT( sizeof( SceGxmBlendInfoAlternate ) == 4 );
BPE_CTASSERT( sizeof( SceGxmBlendInfo ) == 4 );

void NVTAState::BlendDisable()
{
   assign_and_dirty( &NVTAState::sState.mBlendInfoEnabled, false, NVTAState::kDS_Blend );
}

void NVTAState::BlendEnableAndSetSrcDst( SceGxmBlendInfo const blendMode )
{
#if SCE_PSP2_SDK_VERSION > 0x01600061
#  error Please check SDK version to verify that SceGxmBlendInfo still matches "Assumption" above!
#endif

   SceGxmBlendInfoAlternate const *pSrcBlend = reinterpret_cast< SceGxmBlendInfoAlternate const * >( &blendMode );
   SceGxmBlendInfoAlternate *pDstBlend = reinterpret_cast< SceGxmBlendInfoAlternate * >( &( sState.mBlendInfo ) );

   if ( !sState.mBlendInfoEnabled || pDstBlend->colorSrcDst != pSrcBlend->colorSrcDst )
   {
      sState.mBlendInfoEnabled = true;
      pDstBlend->colorSrcDst = pSrcBlend->colorSrcDst;
      AddDirtyState( kDS_Blend );
   }
}

void NVTAState::BlendSetOp( SceGxmBlendInfo const blendMode )
{
#if SCE_PSP2_SDK_VERSION > 0x01600061
#  error Please check SDK version to verify that SceGxmBlendInfo still matches "Assumption" above!
#endif

   SceGxmBlendInfoAlternate const *pSrcBlend = reinterpret_cast< SceGxmBlendInfoAlternate const * >( &blendMode );
   SceGxmBlendInfoAlternate *pDstBlend = reinterpret_cast< SceGxmBlendInfoAlternate * >( &( sState.mBlendInfo ) );

   assign_and_dirty( &pDstBlend->colorAlphaFuncs, pSrcBlend->colorAlphaFuncs, kDS_Blend );
}

void NVTAState::BlendEnableAndSetBlend( uint32 const blendMode )
{
#if SCE_PSP2_SDK_VERSION > 0x01600061
#  error Please check SDK version to verify that SceGxmBlendInfo still matches "Assumption" above!
#endif
   uint32 src = blendMode;
   uint32 *dst = reinterpret_cast< uint32* >( &(sState.mBlendInfo) );

   uint32 d = *dst;
   src = (src & 0xFFFFFF00) | (d & 0xFF);
   if ( !sState.mBlendInfoEnabled || src != d)
   {
      sState.mBlendInfoEnabled = true;
      *dst = src;
      AddDirtyState( kDS_Blend );
   }
}

void NVTAState::BlendSetMask( uint8_t const blendMask )
{
   assign_and_dirty( &sState.mBlendInfo.colorMask, blendMask, kDS_Blend );
}

uint32 NVTAState::BlendGetBlend()
{
   return *reinterpret_cast< uint32* >(&sState.mBlendInfo);
}

bool NVTAState::BlendGetEnabled()
{
   return sState.mBlendInfoEnabled;
}

void NVTAState::DepthSetUnitEnabled(bool const enable)
{
   assign_and_dirty( &sState.mDepthUnitEnabled, enable, kDS_Depth );
}

void NVTAState::DepthSetFunc(SceGxmDepthFunc const func)
{
   assign_and_dirty( &( sState.mDepthFuncs[1] ), func, kDS_Depth );
}

void NVTAState::DepthSetWriteEnabled(bool const enable) 
{
   assign_and_dirty( & ( sState.mDepthWriteModes[1] ), enable ? SCE_GXM_DEPTH_WRITE_ENABLED : SCE_GXM_DEPTH_WRITE_DISABLED, kDS_Depth );
}

bool NVTAState::DepthGetWriteEnabled()
{
   return sState.mDepthWriteModes[1] == SCE_GXM_DEPTH_WRITE_ENABLED;
}

void NVTAState::CullSetMode(SceGxmCullMode const mode) 
{
   assign_and_dirty( &sState.mLastCullMode, mode, kDS_Cull );
}

void NVTAState::SetVertexAndFragmentProgram( 
   void *vertexProgram, void *fragmentProgram, 
   SceGxmShaderPatcherId const vertexProgramId, SceGxmShaderPatcherId const fragmentProgramId,
   SFragmentParameter const *pFragmentParameters, SVertexParameter const *pVertexParameters, SVertexSemantic const *pVertexSemantics,
   uint32 **pParameters, uint32 **pParametersEnd )
{
   uint32 **ppFixedParameters;
   uint32 **ppFixedParametersEnd;

   if ( pParameters == pParametersEnd )
   {
      ppFixedParameters = ppFixedParametersEnd = NULL;
   }
   else
   {
      ppFixedParameters = pParameters;
      ppFixedParametersEnd = pParametersEnd;

      AddDirtyState( kDS_VertexRegs );
      AddDirtyState( kDS_FragmentRegs );
   }

   if ( sState.mppCntParametersBegin != ppFixedParameters ||
      sState.mppCntParametersEnd != ppFixedParametersEnd )
   {
      AddDirtyState( kDS_VertexRegs );
      AddDirtyState( kDS_FragmentRegs );

      sState.mppCntParametersBegin = ppFixedParameters;
      sState.mppCntParametersEnd = ppFixedParametersEnd;
   }

   if ( sState.mpCurrentUnpatchedVertexProgramPtr != vertexProgram )
   {
      AddDirtyState( kDS_VertexShader );
      AddDirtyState( kDS_Attributes );

      sState.mpCurrentUnpatchedVertexProgramPtr = reinterpret_cast<SceGxmProgram const *>( vertexProgram );
      sState.mCurrentVertexProgramId = vertexProgramId;

      sState.mpCurrentVertexParameters = pVertexParameters;
      sState.mpCurrentVertexSemantics = pVertexSemantics;

      if (sceGxmProgramGetDefaultUniformBufferSize(sState.mpCurrentUnpatchedVertexProgramPtr))
      {
         AddDirtyState( kDS_VertexRegs );
         sVertexRegStateToDirty = kDS_VertexRegs;
      }
      else
         sVertexRegStateToDirty = 0;
   }
   else if ( sState.mpCurrentVertexParameters != pVertexParameters || sState.mpCurrentVertexSemantics != pVertexSemantics )
   {
      AddDirtyState( kDS_VertexShader );
      AddDirtyState( kDS_Attributes );

      sState.mpCurrentVertexParameters = pVertexParameters;
      sState.mpCurrentVertexSemantics = pVertexSemantics;

      AddDirtyState( sVertexRegStateToDirty );
   }

   if ( sState.mpCurrentUnpatchedFragmentProgramPtr != fragmentProgram )
   {
      AddDirtyState( kDS_FragmentShader );

      sState.mpCurrentUnpatchedFragmentProgramPtr = reinterpret_cast<SceGxmProgram const *>( fragmentProgram );
      sState.mCurrentFragmentProgramId = fragmentProgramId;

      sState.mpCurrentFragmentParameters = pFragmentParameters;

      sState.mFragmentBlendingEnabled = !(sceGxmProgramIsFragColorUsed( sState.mpCurrentUnpatchedFragmentProgramPtr ) || sceGxmProgramIsNativeColorUsed( sState.mpCurrentUnpatchedFragmentProgramPtr ) );
      // we don't bother caching the fragment reg state to dirty, as this is the only case it gets looked at
      if (sceGxmProgramGetDefaultUniformBufferSize(sState.mpCurrentUnpatchedFragmentProgramPtr))
         AddDirtyState( kDS_FragmentRegs );
   }
   else if ( sState.mpCurrentFragmentParameters != pFragmentParameters )
   {
      AddDirtyState( kDS_FragmentShader );

      sState.mpCurrentFragmentParameters = pFragmentParameters;
   }
}

void NVTAState::PolySetMode( SceGxmPolygonMode const mode )
{
   if ( mode != sState.mLastPolygonMode )
   {
      sState.mLastPolygonMode = mode;
      ManualState_FlushPolyMode(); // May as well flush the poly mode now - only set right before draws
   }
}

//----------------------------------------------------------------------------

SceGxmShaderPatcherId NVTAState::RegisterShaderProgram( void *pShaderProgram )
{
   // Note, this function is reentrant.  If a shader register fails and flush-on-fail is true, it will
   // call itself in the process of reregistering shaders.

   TShaderProgramMap::const_iterator found = sShaderProgramMap.find( uint32( pShaderProgram ) );

   if( found == sShaderProgramMap.end() )
   {
      SceGxmShaderPatcherId programId;

      BPE_CHECK_SCE( sceGxmProgramCheck( (SceGxmProgram *) pShaderProgram ) );
      BPE_CHECK_SCE( sceGxmShaderPatcherRegisterProgram( s_shaderPatcher, (SceGxmProgram *) pShaderProgram, &programId ) );
      sShaderProgramMap.insert( TShaderProgramMap::value_type( (uint32) pShaderProgram, programId ) );

      return programId;
   }
   else
   {
      return found->second;
   }
}

//----------------------------------------------------------------------------

void NVTAState::UnregisterShaderProgram( void *pShaderProgram )
{
   WaitShaderPatcherIdleInternal();
   UnregisterAllTemporaryVertexProgramsInternal();
   UnregisterAllTemporaryFragmentProgramsInternal();

   TShaderProgramMap::iterator it = sShaderProgramMap.find( (uint32) pShaderProgram );

   if ( it != sShaderProgramMap.end() )
   {
      BPE_CHECK_SCE( sceGxmShaderPatcherForceUnregisterProgram( s_shaderPatcher, it->second ) );
      sShaderProgramMap.erase( it );
   }
}

//----------------------------------------------------------------------------

void NVTAState::UnregisterAllTemporaryPrograms()
{
   WaitShaderPatcherIdleInternal();
   UnregisterAllTemporaryVertexProgramsInternal();
   UnregisterAllTemporaryFragmentProgramsInternal();
}

//----------------------------------------------------------------------------

void NVTAState::WaitShaderPatcherIdleInternal()
{
   volatile uint32_t *pNotificationRegion = sceGxmGetNotificationRegion();

   pNotificationRegion[kNO_ReallocateVertexPrograms] = 0;
   SceGxmNotification notification = { &( pNotificationRegion[kNO_ReallocateVertexPrograms] ), 1 };

   // First finish the prior scene
   sceGxmFinish( gpRenderBackend->Context() );

   // Now try to finish the processing in the current scene, if there's one in progress
   // If there's not one in progress, flush will fail, which is why there's no BPE_CHECK_SCE
   if ( SCE_OK == sceGxmMidSceneFlush( gpRenderBackend->Context(), 
      SCE_GXM_MIDSCENE_PRESERVE_DEFAULT_UNIFORM_BUFFERS,
      NULL, &notification ) )
   {
      sceGxmNotificationWait( &notification );
   }
}
//----------------------------------------------------------------------------

static inline uint64 copy_gxm_and_return_inequality( SceGxmVertexAttribute *pDst, SceGxmVertexAttribute const *pSrc )
{
   BPE_CTASSERT( sizeof( *pDst ) == sizeof( uint64 ) );
   uint64 *pDstU64 = reinterpret_cast<uint64 *>( pDst );
   uint64 const *pSrcU64 = reinterpret_cast<uint64 const *>( pSrc );

   bool not_equ = ( *pDstU64 != *pSrcU64 );

   *pDstU64 = *pSrcU64;
   return not_equ;
}

//----------------------------------------------------------------------------

void NVTAState::SetAttributes( SAttributeDesc const *pFirstAttribute )
{
   sState.mLastCachedVertexData = NULL;

   // First attributes...
   {
      uint32 const oldValidity = sState.mGxmAttributeValid;
      uint32 newValidity = 0;
      bool dirty = false;

      for ( SAttributeDesc const * pAttr = pFirstAttribute; pAttr->mVDU != kVDU_Invalid; ++pAttr )
      {
         newValidity |= 1 << pAttr->mVDU;
         dirty |= copy_gxm_and_return_inequality( sState.mGxmAttributes + pAttr->mVDU, &pAttr->mAttr );
      }

      dirty |= ( newValidity != oldValidity );
      sState.mGxmAttributeValid = newValidity;

      if ( dirty )
      {
         AddDirtyState( kDS_Attributes );
         AddDirtyState( sVertexRegStateToDirty );
      }
   }
}

//----------------------------------------------------------------------------

void NVTAState::SetCachedVertexData(SCachedVertexData *cvd)
{
   if (cvd == sState.mLastCachedVertexData)
   {
      // just update pointers
      NVTAState::UpdateLowLevelStreamPointers( cvd->streamsOut, cvd->pStreamTail - cvd->streamsOut );
   }
   else
   {
      NVTAState::SetLowLevelStreams( cvd->streamsOut, cvd->pStreamTail - cvd->streamsOut );
      NVTAState::SetAttributes( cvd->vtaAttributes.Finalize() );
      sState.mLastCachedVertexData = cvd;
   }
}

NVTAState::SCachedVertexData::SCachedVertexData(CShaderVertexDataBinding const & vertexDataBinding, CVertexData const & vertexData)
{
   struct SVitaAttrDesc
   {
      SceGxmAttributeFormat mFormat;
      int mElementCount;
   };

   static const SVitaAttrDesc skTypeToFormat[] = 
   {
      { SCE_GXM_ATTRIBUTE_FORMAT_F32, 2 }, // kVDT_Float2,  
      { SCE_GXM_ATTRIBUTE_FORMAT_F32, 3 }, // kVDT_Float3,  
      { SCE_GXM_ATTRIBUTE_FORMAT_F32, 4 }, // kVDT_Float4,  

      { SCE_GXM_ATTRIBUTE_FORMAT_U8N, 4 }, // kVDT_UByte4N, 
      { SCE_GXM_ATTRIBUTE_FORMAT_U8,  4 }, // kVDT_UByte4,  

      { SCE_GXM_ATTRIBUTE_FORMAT_F16, 2 }, // kVDT_Half2,   
      { SCE_GXM_ATTRIBUTE_FORMAT_F16, 4 }, // kVDT_Half4,   

      { SCE_GXM_ATTRIBUTE_FORMAT_S16N,2 }, // kVDT_Short2N, 
      { SCE_GXM_ATTRIBUTE_FORMAT_S16N,4 }, // kVDT_Short4N, 

      { SCE_GXM_ATTRIBUTE_FORMAT_U8,  4 }, // kVDT_Packed3N, // Unsupported on Vita!

      { SCE_GXM_ATTRIBUTE_FORMAT_U8,  4 }, // kVDT_Int32, // Unsupported on Vita!

      { SCE_GXM_ATTRIBUTE_FORMAT_S16, 2 }, // kVDT_Short2,  
      { SCE_GXM_ATTRIBUTE_FORMAT_S16, 4 } // kVDT_Short4
   };

   pStreamTail = streamsOut;

   for ( int bufferIndex = 0; bufferIndex < vertexData.GetBufferCount(); ++bufferIndex )
   {
      *pStreamTail = NVTAState::SLowLevelStream( 
         reinterpret_cast<uint8 const *>( vertexData.GetBufferPtrByBufferIndex( bufferIndex ) ),
         vertexData.GetStrideByBufferIndex( bufferIndex ),
         vertexData.GetInstancedByBufferIndex( bufferIndex ));

      ++pStreamTail;
   }

   for ( int vduIndex = 0; vduIndex < kVDU_Count; ++vduIndex )
   {
      if ( vertexDataBinding.HasStream( EVertexDataUsage( vduIndex ) ) )
      {
         EVertexDataStream streamIdx = vertexDataBinding.GetStream( EVertexDataUsage( vduIndex ) );

         if ( vertexData.HasAttribute( streamIdx ) )
         {
            CVertexData::SVertexAttribute const &srcAttr = vertexData.GetAttribute( streamIdx );

            SceGxmVertexAttribute dstAttr;

            dstAttr.streamIndex = srcAttr.mBufferIndex;
            dstAttr.offset = srcAttr.mOffset;
            dstAttr.format = skTypeToFormat[ srcAttr.mType ].mFormat;
            dstAttr.componentCount = skTypeToFormat[ srcAttr.mType ].mElementCount;
            dstAttr.regIndex = -1;

            vtaAttributes.Add( EVertexDataUsage( vduIndex ), dstAttr );
         }
      }
   }
}

void NVTAState::SCachedVertexData::SetBuffer_Unsafe(int idx, void *pBuffer)
{
   streamsOut[idx].mStreamData = pBuffer;
}

void NVTAState::SCachedVertexData::SetBuffer(int const idx, CVertexBuffer const *pBuffer)
{
   SetBuffer_Unsafe( idx, pBuffer ? pBuffer->mpMemory_RT->mpAddress : NULL );
}

void NVTAState::SCachedVertexData::SetBuffer(int const idx, CDynamicVertexBufferPoolChunk_UT const *pBuffer)
{
   SetBuffer_Unsafe( idx, pBuffer->GetRenderMemory() );
}

void NVTAState::SCachedVertexData::SetBuffer(int const idx, CDynamicVertexBufferPoolChunk_RT const *pBuffer)
{
   SetBuffer_Unsafe( idx, pBuffer->GetRenderMemory() );
}

//----------------------------------------------------------------------------

void NVTAState::InvalidateHardwareState()
{
   MarkAllStateDirty();
   ManualState_FlushPolyMode();
}
