//----------------------------------------------------------------------------
// BP_RendererBase.cpp
//----------------------------------------------------------------------------

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/Basics/BPEAssert.h"
#include "Engine/StlExtras/BPEStlExtras.h"
#include "Engine/Mechanics/IObject.h"

#include "Engine/Math/BPEMath.h"

#include "Engine/Mechanics/HashUtils.h"

#include "boost/scoped_ptr.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/optional.hpp"

#include "boost/foreach.hpp"
#define foreach BOOST_FOREACH

#if BPE_TARGET == BPE_TARGET_WIN32
#include "d3d9.h"
#include "d3dx9.h"
#endif

#if BPE_TARGET == BPE_TARGET_PS3
#include <cell/pad.h>
#endif

#include "BP_BaseRenderer.h"

#include "BP_SoundSupport.h"
#include "BP_MovieSupport.h"
#include "BP_Network.h"
#include "BP_TUS.h"
#include "BP_UnifiedSDX.h"

#include "BP_Transfarring.h"

#if BPE_TARGET == BPE_TARGET_VITA
#include "Engine/Memory/VTAPhysContMemory.h"
#endif

//----------------------------------------------------------------------------

#include "Engine/Memory/CMemoryAllocator.h"
#include "Engine/Resource/CResourceCache.h"
#include "Engine/Resource/CResourceFactory.h"
#include "Engine/Resource/CResourcePool.h"
#include "Engine/StlExtras/BPEStlExtras.h"
#include "Engine/System/COsContext.h"
#include "Engine/System/CTaskQueue.h"
#include "Engine/System/CFileUtils.h"
#include "Boost/shared_ptr.hpp"
#include "Boost/scoped_ptr.hpp"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Frontend/CRenderer.h"
#include "Renderer/Base/CMTXFont.h"
#include "Renderer/Base/Primitive/CMesh.h"
#include "Renderer/Base/Primitive/CMeshBuffers.h"
#include "Renderer/Base/Frontend/RenderObject/CMeshRenderEntity.h"
#include "Renderer/Base/Frontend/RenderObject/CMeshChunkRenderEntity.h"
#include "Renderer/Base/Frontend/CRenderViewport.h"
#include "Renderer/Base/Material/CCompiledShaderCache.h"
#include "Renderer/Base/Material/ProgShader/Shaders/CCallbackShader.h"
#include "Renderer/Base/Material/ProgShader/CShaderParameterBuffer.h"
#include "Renderer/Base/Primitive/CVertexBuffer.h"
#include "Renderer/Base/Primitive/CVertexData.h"
#include "Renderer/Base/Primitive/ProgShader/CIndexBuffer.h"
#include "Engine/GameObjectSystem/CGameObjectRegisteredComponentGroupObjectFactories.h"
#include "Engine/Graphics/CIndexArray.h"
#include "Engine/Graphics/CVertexArray.h"
#include "Engine/StlExtras/vector_s.h"

#include "Renderer/Base/Backend/CTexture.h"

#include "BP_FileSupport.h"
#include "BP_Font.h"
#include "BP_TrophySystem.h"

#include "MGS_Common.h"
#include "BP_TextureTool.h"

#if BPE_TARGET == BPE_TARGET_PS3
#include "Renderer/Base/Material/PS3/PS3CCompiledShader.h"
#include "Engine/System/CSPURSManager.h"
#endif

#if BPE_TARGET == BPE_TARGET_X360
#ifdef BPE_DEBUG
#include <xbdm.h>
#endif
#endif

//----------------------------------------------------------------------------

namespace
{
   boost::scoped_ptr<COsContext> gOsContext;
   boost::scoped_ptr<CResourceFactory> gResourceFactory;
   boost::scoped_ptr<IResourcePool> gResourcePool;
   boost::scoped_ptr<CRenderBackend> gRenderBackend;
   boost::scoped_ptr<CRenderer> gRenderFrontend;

   static int * sTextureFlatlistMappingTable = NULL;
   static int sTextureFlatlistNumEntries = 0;
};

//TED moved this to global so it can be initialized by another system after.
//file system is initialized.  Needs revisiting!
char * gTextureFlatlistMappingData = NULL;
int gTextureFlatlistMappingDataSize = 0;

//----------------------------------------------------------------------------

class CStageAssetCache
{
public:
   typedef std::pair< uint32, uint32 > TCacheTagPathHashPair;
   typedef std::map<TCacheTagPathHashPair, IObject *> TCache;

   static TCacheTagPathHashPair BuildKey( const unsigned int cacheTag, const char * const stagePath )
   {
      const uint32 archivePathHash = HashUtils::HashLittle( stagePath );
      TCacheTagPathHashPair key( cacheTag, archivePathHash );
      return key;
   }

   IObject * TakeItem( const unsigned int cacheTag, const char * const stagePath )
   {
      const TCacheTagPathHashPair key( BuildKey( cacheTag, stagePath ) );
      TCache::iterator const found = mCache.find(key);
      if ( found == mCache.end() )
         return 0;

      IObject * const pAsset = found->second;
      mCache.erase( found );
      return pAsset;
   }

   void AddItem( const unsigned int cacheTag, const char * const stagePath, IObject * const pAsset )
   {
      const TCacheTagPathHashPair key( BuildKey( cacheTag, stagePath ) );

//      printf( "SAC: %8.8x %8.8x %s\n", key.first, key.second, stagePath );

      mCache.insert(TCache::value_type(key, pAsset));
      mpLastAddedAsset = pAsset;
   }

   void AddReferenceToLastItem( const unsigned int cacheTag, const char * const stagePath )
   {
      AddItem( cacheTag, stagePath, mpLastAddedAsset );
   }

   void VerifyEmpty( const unsigned int cacheTag )
   {
      //Items should be removed one by one via KP asset loading.
      int unclaimedCount = 0;

      for( TCache::iterator it=mCache.begin(); it != mCache.end(); ++it )
      {
         if ( it->first.first == cacheTag )
         {
            printf( "SAC ERROR: Straggler: %8.8x key: %8.8x obj: 0x%8.8x\n", it->first.first, it->first.second, it->second );

            ++unclaimedCount;
         }
      }

      BPE_VERIFY( unclaimedCount == 0, false, "Still unclaimed BP assets in this cache!" );
   }

   void ClearByTag( unsigned int const cacheTag )
   {
      std::vector< IObject * > assetsToDelete;
      assetsToDelete.reserve( mCache.size() );

      for( TCache::iterator nextit=mCache.begin(); nextit != mCache.end(); /* incremented in loop */ )
      {
         TCache::iterator it = nextit++; // Increments nextit, returns the prior value
         unsigned int cacheItemTag = it->first.first; // I love pairs

         if ( cacheItemTag == cacheTag )
         {
            IObject * pAsset = it->second;
            assetsToDelete.push_back(pAsset);
            mCache.erase( it );
         }
      }

      IObject * pLastAsset = NULL;
      std::sort( assetsToDelete.begin(), assetsToDelete.end() );
      //Just the first of each texture in the vector after it's sorted is deleted.
      for( std::vector< IObject* >::iterator it = assetsToDelete.begin(); it != assetsToDelete.end(); ++it )
      {
         IObject * pAsset = *it;
         if( pAsset != pLastAsset )
         {
            delete pAsset;
         }
         pLastAsset = pAsset;
      }
   }

   void Clear()
   {
      std::vector< IObject * > assetsToDelete;
      assetsToDelete.reserve( mCache.size() );

      for( TCache::iterator it=mCache.begin(); it != mCache.end(); ++it )
      {
         IObject * pAsset = it->second;
         assetsToDelete.push_back(pAsset);
      }
      mCache.clear();

      IObject * pLastAsset = NULL;
      std::sort( assetsToDelete.begin(), assetsToDelete.end() );
      //Just the first of each texture in the vector after it's sorted is deleted.
      for( std::vector< IObject* >::iterator it = assetsToDelete.begin(); it != assetsToDelete.end(); ++it )
      {
         IObject * pAsset = *it;
         if( pAsset != pLastAsset )
         {
            delete pAsset;
         }
         pLastAsset = pAsset;
      }
   }

private:
   TCache   mCache;
   IObject *   mpLastAddedAsset;
};

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------

void BP_InitializeSystems()
{
   //BP - COsContext
   gOsContext.reset(new COsContext());

#if BPE_TARGET == BPE_TARGET_X360
#ifdef BPE_DEBUG
   DmMapDevkitDrive();
#endif
#endif

#if BPE_TARGET == BPE_TARGET_PS3
   {
      gpSpursManager = new CSPURSManager();

      extern struct CellSpurs* g_SpursForSound;
      extern struct CellSpurs* g_SpursForMovie;
      g_SpursForSound = (struct CellSpurs*)gpSpursManager->mpSPURS_System.get();
      extern struct CellSpurs* g_SpursForMovie;
      g_SpursForMovie = (struct CellSpurs*)gpSpursManager->mpSPURS_Main.get();
   }
#endif

#if BPE_TARGET == BPE_TARGET_VITA
   NPhysContMem::InitPhysContMem();
#endif

#if BP_USE_NEW_FONT_SYSTEM()
   BP_font_init_resource(gOsContext->mControllerRegion == CBaseOsContext::kCR_Japan ? 1 : 0);
#endif

   BP_SoundSupport_Init();



   gResourceFactory.reset(new CResourceFactory());

   gResourceFactory->AddFactory( 'ctxr', CResourceFactory::SFactoryEntry(CResourceFactory::SFactoryEntry::kFF_None, CBaseTexture::FTextureFactory) );
   gResourceFactory->AddFactory( 'cfon', CResourceFactory::SFactoryEntry(CResourceFactory::SFactoryEntry::kFF_None, CMTXFont::FFontFactory) );
#if BPE_TARGET == BPE_TARGET_PS3 || BPE_TARGET == BPE_TARGET_VITA
   gResourceFactory->AddFactory( '.cfx', CResourceFactory::SFactoryEntry(CResourceFactory::SFactoryEntry::kFF_None, CCompiledShaderPackage::Factory) );
#elif BPE_TARGET == BPE_TARGET_X360
   gResourceFactory->AddFactory( '.cfx', CResourceFactory::SFactoryEntry(CResourceFactory::SFactoryEntry::kFF_None, Effect_360::FEffectFactory) );
#elif BPE_TARGET == BPE_TARGET_WIN32
   // No need for CFX factory in Win32
#else
#  error Evaluate if we need the cfx factory for this platform
#endif    
   gResourceFactory->AddFactory( 'cmdl', CResourceFactory::SFactoryEntry(CResourceFactory::SFactoryEntry::kFF_None, CMesh::FMeshFactory) );

   gResourcePool.reset(new CResourcePool(gResourceFactory.get()));
   gpResources = gResourcePool.get();
   gResourcePool->SetResourceCache(new CResourceCache());   // Cache resources for faster loading

   // Initialize render backend...
   {
      // 95% Screen size
      CRenderBackend::SRenderFrameBufferSize kFrameBufferSizes[] =
      {
         // The first size (w/h) is video mode, this is used to match up against the current screen resolution to detect which mode to use. It ALSO determines the size of the back buffer.
         // The second size (w/h) is the normal render target size. Used when the game is NOT running in full pixel mode. 
         // NOTE: if the render target size doesn't match the back buffer size, it gets rendered centered inside the back buffer.

#if BPE_TARGET==BPE_TARGET_VITA
         CRenderBackend::SRenderFrameBufferSize(CRenderBackend::SRenderFrameBufferSize::kDA_Widescreen,  960,  544,   960,  544, 960 ,  544, 16.0f / 9.0f),
#else
         CRenderBackend::SRenderFrameBufferSize( CRenderBackend::SRenderFrameBufferSize::kDA_Widescreen, 1280,  720,  1216,  684, 1280,  720, 16.0f / 9.0f),
#  if EDGE_MLAA_SUPPORT_1080
         CRenderBackend::SRenderFrameBufferSize(CRenderBackend::SRenderFrameBufferSize::kDA_Widescreen, 1920, 1080,  1824, 1026, 1920, 1080, 16.0f / 9.0f),
#  else
         CRenderBackend::SRenderFrameBufferSize(CRenderBackend::SRenderFrameBufferSize::kDA_Widescreen,  960, 1080,   912, 1026,  960, 1080, 16.0f / 9.0f),
#  endif
#  if BPE_TARGET == BPE_TARGET_X360
         // X360 - 1024x768
         CRenderBackend::SRenderFrameBufferSize(CRenderBackend::SRenderFrameBufferSize::kDA_Widescreen, 1024,  768,  1280,  720, 16.0f / 9.0f),
         CRenderBackend::SRenderFrameBufferSize(CRenderBackend::SRenderFrameBufferSize::kDA_Fullscreen, 1024,  768,  1280,  720, 4.0f  / 3.0f),

         // X360 - 848x480
         CRenderBackend::SRenderFrameBufferSize(CRenderBackend::SRenderFrameBufferSize::kDA_Widescreen,  848,  480,  1280,  720, 16.0f / 9.0f),
         CRenderBackend::SRenderFrameBufferSize(CRenderBackend::SRenderFrameBufferSize::kDA_Fullscreen,  848,  480,  1280,  720, 4.0f  / 3.0f),

         // X360 - 480p
         CRenderBackend::SRenderFrameBufferSize(CRenderBackend::SRenderFrameBufferSize::kDA_Widescreen,  640,  480,  1280,  720, 16.0f / 9.0f),
         CRenderBackend::SRenderFrameBufferSize(CRenderBackend::SRenderFrameBufferSize::kDA_Fullscreen,  640,  480,  1280,  720, 4.0f  / 3.0f),
         CRenderBackend::SRenderFrameBufferSize(CRenderBackend::SRenderFrameBufferSize::kDA_Widescreen,  720,  480,   668,  444,  700,  468, 16.0f / 9.0f),
         CRenderBackend::SRenderFrameBufferSize(CRenderBackend::SRenderFrameBufferSize::kDA_Fullscreen,  720,  480,   668,  444,  700,  468, 4.0f  / 3.0f),
#  endif

         // PS3 - 576p
         CRenderBackend::SRenderFrameBufferSize(CRenderBackend::SRenderFrameBufferSize::kDA_Widescreen,  720,  576,   668,  532,  700,  560, 16.0f / 9.0f),
         CRenderBackend::SRenderFrameBufferSize(CRenderBackend::SRenderFrameBufferSize::kDA_Fullscreen,  720,  576,   668,  532,  700,  560, 4.0f  / 3.0f)
#endif
      };

      CRenderBackend::SRenderInitialization initParams;
      initParams.mpFrameBufferSize = kFrameBufferSizes;
      initParams.mFrameBufferSizeCount = BPE_ARRAY_SIZE(kFrameBufferSizes);

      gRenderBackend.reset(new CRenderBackend( *gResourcePool.get(), initParams) );

      gRenderBackend->InitializeDisplay(CRenderBackend::kSM_Monoscopic);
   }

   gRenderFrontend.reset(new CRenderer());

   // Initialize NP communication (needs to happen before trophy or TUS)
   BP_Network_Init();
   
   // Initialize title user storage system
   BP_TUS_Init();

   // Initialize trophy system (placeholder location, will probably need moving)
   BP_TrophySystem_Init();

   BP_Movie_Init();

   Transfarring_InitializeSystems();
}

//----------------------------------------------------------------------------

void BP_ShutdownSystems()
{
#if BPE_TARGET == BPE_TARGET_PS3
   // Disable rumble
{
      CellPadActParam actParam;
      memset(&actParam, 0x00, sizeof(CellPadActParam));

      for (int i=0; i<7; i++)
         cellPadSetActDirect (i, &actParam);  // stop vibration on all controllers (docs say it will stop in 3 secs regardless)
      }

   // Shut down rendering
   gpRenderBackend->ShutDown();
#endif

   // Shut down systems
   BP_TrophySystem_Shutdown();
   BP_SoundSupport_Shutdown();
   BP_ShutdownFileSupport();
   BP_Movie_Shutdown();

#if defined(BP_VITA)
   BP_CloseUnifiedSDXArchive();
#endif
   
   
#if BPE_TARGET == BPE_TARGET_VITA
   NPhysContMem::ShutdownPhysContMem();
#endif

   // OS context
#if BPE_TARGET == BPE_TARGET_PS3
   // Release memory container but keep NP initialized for sceNpDrmProcessExitSpawn2
   OsContext()->DestroyMemoryContainer();
#else
   gOsContext.reset();
#endif
}

//----------------------------------------------------------------------------

#if BPE_TARGET == BPE_TARGET_PS3
#include "edge/geom/edgegeom_structs.h"
#endif

struct SMGSResourceCacheItem
{
   int               mIndex;
   std::string       mFileName;
   void*             mpData;
   bool              mbDeleted;
};

class CMGSResourceCache
{
public:
   typedef std::map<uint32, SMGSResourceCacheItem> TCache;
   typedef TCache::iterator TIterator;
   typedef std::pair<TIterator, TIterator> TItemRange;

public:
   SMGSResourceCacheItem * Item(unsigned int mgsAddress)
   {
      return const_cast< SMGSResourceCacheItem * >( GetItem( mgsAddress ) );
   }

   SMGSResourceCacheItem const * GetItem(unsigned int mgsAddress) const
   {
      TCache::const_iterator const found = mCache.find(mgsAddress);
      if( found != mCache.end() )
      {
         return &found->second;
      }

      return NULL;
   }

   int FindKey(void* data) const
   {
      TCache::const_iterator iter = mCache.begin();
      for ( ; iter != mCache.end(); ++iter )
      {
         if ( iter->second.mpData == data )
            return iter->first;
      }
      return -1;
   }

   void AddItem(unsigned int mgsAddress, SMGSResourceCacheItem const & item)
   {
      mCache.insert(TCache::value_type(mgsAddress, item));
   }

   TItemRange const GetRange(unsigned int mgsAddressBegin, unsigned int mgsAddressEnd)
   {
      return TItemRange(mCache.lower_bound(mgsAddressBegin), mCache.upper_bound(mgsAddressEnd));
   }

   TItemRange const GetEntireRange()
   {
      return TItemRange(mCache.begin(), mCache.end());
   }

   TIterator const RemoveItem(TIterator const item)
   {
      return mCache.erase(item);
   }

   int GetCount() const { return mCache.size(); }

private:
   TCache   mCache;
};

namespace
{
   CMGSResourceCache gMeshCache;
   CMGSResourceCache gTextureCache;
   CMGSResourceCache gResidentTextureCache;

   CStageAssetCache  gStageAssetCache;
};

//----------------------------------------------------------------------------

void BP_VerifyEmptyStageAssetCache( const unsigned int cacheTag )
{
   gStageAssetCache.VerifyEmpty( cacheTag );
}

//----------------------------------------------------------------------------

void BP_ClearStageAssetCacheByTag( const unsigned int cacheTag )
{
   gStageAssetCache.ClearByTag( cacheTag );
}

void BP_ForceClearStageAssetCache()
{
   gStageAssetCache.Clear();
}

void BP_AddReferenceToLastStageAssetCacheEntry( const unsigned int cacheTag, const char * const stagePath )
{
   gStageAssetCache.AddReferenceToLastItem( cacheTag, stagePath );
}

void BP_AddResourceToStageAssetCache( const unsigned int cacheTag, const char * const stagePath, const char * const name, void * const memory, const int size )
{
   //Construct asset.
   IObject * pAsset = NULL;
   const char * const ext = stagePath + strlen(stagePath)-5;
   if( !strcmp( ext, ".ctxr" ) )
   {
      pAsset = reinterpret_cast<IObject *>( BP_ConstructTexture( name, memory, size ) );
   }
   else if( !strcmp( ext, ".cmdl" ) )
   {
      pAsset = reinterpret_cast<IObject *>( BP_ConstructMesh( name, memory, size ) );
   }

   if( !pAsset )
      BP_TODO_BREAK;

   gStageAssetCache.AddItem( cacheTag, stagePath, pAsset );
}

//----------------------------------------------------------------------------

void BP_ConvertToFlatListFilename( char * path, const char * knownFlatPath, const char* texToolPath )
{
   char const *ext = strrchr( path, '.' );
   bool isAssetTexture = false;

   if ( ext && 0 == strcmp( ext, ".ctxr" ) )
   {
      isAssetTexture = true;
   }

#if BP_ENABLE_TEXTURE_TOOL
   // store a remapping from the original path to the flat path.
   if ( isAssetTexture )
   {
      BP_TextureTool_StorePathRemapping( texToolPath, knownFlatPath );
   }
#endif

#if !defined(BP_VITA)
   if( !BP_IsPsarcMounted() )
   {
      BPE_ASSERT( knownFlatPath != NULL, "must supply known flat path if working without remapping table" );
      strcpy( path, knownFlatPath );
   }
#else
   // On vita we always use the known flat path.
   BPE_ASSERT( knownFlatPath != NULL, "must supply known flat path if working without remapping table" );
   strcpy( path, knownFlatPath );
#endif

#if BP_ENABLE_TEXTURE_TOOL
   else if ( isAssetTexture && BP_TextureTool_GetEnable() )
   {
      strcpy( path, knownFlatPath );
   }
#endif
   //else the file is remapped within the archive and we want the unflat path.

#if BP_ENABLE_TEXTURE_TOOL
   if( !BP_TextureTool_RemapPath( path ) )
   {
      BP_InsertPlatformSubfolder( path );
   }
#else
   BP_InsertPlatformSubfolder( path );
#endif
}

//----------------------------------------------------------------------------

static CMesh *create_emptyish_mesh()
{
   CVector3 const pos[] = 
   {
      CVector3::Zero(),
      CVector3::Zero(),
      CVector3::Zero()
   };

   CVector2 const uvs[] =
   {
      CVector2(0.0f, 0.0f),
      CVector2(1.0f, 0.0f),
      CVector2(0.0f, 1.0f)
   };

   CVertexArray* pVertexArray = CVertexArray::CreateVertexArray(2);
   pVertexArray->AddStream('POS0', kVDT_Float3, 0, (void*)pos, sizeof(CVector3) * 3);
   pVertexArray->AddStream('TEX0', kVDT_Float2, 0, (void*)uvs, sizeof(CVector2) * 3);

   std::vector<uint32> indices;
   indices.reserve( 3 );
   for( int i = 0; i < 3; ++i )
      indices.push_back( i );

   CIndexArray indexArray( indices );

   std::vector<CMeshChunk> chunks;
   chunks.push_back(CMeshChunk(CAABox::MakeMaxBox(), -1, CMeshChunk::kPrimitive_TriangleStrip, 0, 3, 0, 3, bpe::vector_s<uint32>(), 0, 0, 0));

   CMesh *pMesh = new CMesh(*pVertexArray, indexArray, chunks, CMesh::TMaterials(), false, 0);

   delete pVertexArray;

   return pMesh;
}

//----------------------------------------------------------------------------

void * BP_ConstructMesh(const char* name, void* memory, int size)
{
#if BPE_TARGET == BPE_TARGET_WIN32 || !BPE_USE_EDGE_GEOM
   CResId const resId(name);
   SFactoryResourceBuildData buildData(resId, memory, size, NULL, *gpResources);
   SFactoryReturnResource output;

   CMesh::FMeshFactory(buildData, output);

   if ( output.mpResource == NULL )
   {
      return create_emptyish_mesh();
   }
   else
   {
      return output.mpResource;
   }
#else

   // Allocate system memory block for system model data
   int const systemDataSize = ((SEdgeModelHeader*)memory)->mRsxDataOffset;
   void* pMemory = BPE_MALLOC_ALIGNED(128, systemDataSize);
   memcpy(pMemory, memory, systemDataSize);

   SEdgeModelHeader* pHeader = (SEdgeModelHeader*)pMemory;
   BPE_VERIFY(pHeader->mHeader == 'EMDL', false, "Invalid Model Header" );
   BPE_VERIFY(pHeader->mVersion == 0, false, "Invalid Model Version" );

   uint32 rsxBaseAddress = 0;

         // Allocate RSX video memory for rsx only model data
         if( pHeader->mRsxDataSize )
         {
            CRenderHWAllocator::SHandle const * pRsxHandle = gpRenderBackend->AllocFixed(pHeader->mRsxDataSize, pHeader->mRsxDataSize, kRM_Video);
            pHeader->mpRsxMemory = (uint32)pRsxHandle;
            rsxBaseAddress = (uint32)pRsxHandle->mpAddress;
            memcpy(pRsxHandle->mpAddress, (void*)((uint32)memory + pHeader->mRsxDataOffset), pHeader->mRsxDataSize);
         }
         else
         {
            pHeader->mpRsxMemory = 0;
         }

#define FIX_OFFSET(OFFSET, BASE, TYPE) if( OFFSET ) OFFSET = (TYPE)((uint32)OFFSET + (uint32)BASE)

   EdgeGeomPpuConfigInfo* pFirstSegment = (EdgeGeomPpuConfigInfo*)((uint32)pMemory + pHeader->mSegmentOffset);

   for( int i = 0; i < pHeader->mSegmentCount; ++i )
   {
      EdgeGeomPpuConfigInfo* pSegment = pFirstSegment + i;
      FIX_OFFSET(pSegment->indexes, pMemory, voidptr32_t);
      FIX_OFFSET(pSegment->spuVertexes[0], pMemory, voidptr32_t);
      FIX_OFFSET(pSegment->spuVertexes[1], pMemory, voidptr32_t);

      if( pSegment->rsxOnlyVertexesSize )
         pSegment->rsxOnlyVertexes = (voidptr32_t)( rsxBaseAddress + (uint32)pSegment->rsxOnlyVertexes );

      FIX_OFFSET(pSegment->skinIndexesAndWeights, pMemory, voidptr32_t);
      FIX_OFFSET(pSegment->blendShapeSizes, pMemory, uint16ptr32_t);
      FIX_OFFSET(pSegment->blendShapes, pMemory, uint32ptr32_t);
      FIX_OFFSET(pSegment->fixedOffsets[0], pMemory, voidptr32_t);
      FIX_OFFSET(pSegment->fixedOffsets[1], pMemory, voidptr32_t);
      FIX_OFFSET(pSegment->spuInputStreamDescs[0], pMemory, voidptr32_t);
      FIX_OFFSET(pSegment->spuInputStreamDescs[1], pMemory, voidptr32_t);
      FIX_OFFSET(pSegment->spuOutputStreamDesc, pMemory, voidptr32_t);
      FIX_OFFSET(pSegment->rsxOnlyStreamDesc, pMemory, voidptr32_t);
   }

#undef FIX_OFFSET

   return pMemory;
#endif
}

void BP_BindMGSMesh(const unsigned int assetCacheTag, const char* pCurrentPath, unsigned int resourceId, unsigned int mgsAddr)
{
   char path[FILENAME_MAX];
   sprintf(path,"%s%8.8x.cmdl", pCurrentPath, resourceId);

   SMGSResourceCacheItem const * pItem = gMeshCache.GetItem(mgsAddr);

   if( pItem )
   {
      BPE_VERIFYA(false, "Trying to load two meshes with same mgs addr.");
      return;
   }

   //Find this asset in the stage cache.
   BP_InsertPlatformSubfolder( path );
   void * pAsset = gStageAssetCache.TakeItem( assetCacheTag, path );
   if ( !pAsset )
      return;

   SMGSResourceCacheItem item;
   item.mIndex = -1;
   item.mFileName = path;
   item.mpData = pAsset;
   item.mbDeleted = false;

   gMeshCache.AddItem(mgsAddr, item);
   return;
}

//----------------------------------------------------------------------------

unsigned int BP_GetMeshByMGSAddr(unsigned int mgsAddr)
{
   SMGSResourceCacheItem const * pItem = gMeshCache.GetItem(mgsAddr);

#if 0 //BP_DEBUG - Can use this to limit loading of models to specific models for debugging purposes.
   CMesh* pMesh = (CMesh*)pItem->mpData;
   if( pMesh->mDebugName.find("007471cd") == std::string::npos )
   {
      return 0;
   }
#endif

   return (unsigned int)(pItem ? pItem->mpData : 0);
}

//----------------------------------------------------------------------------

void BP_DestroyMeshes(unsigned int mgsAddrBegin, unsigned int mgsAddrEnd)
{
   CMGSResourceCache::TItemRange const range = gMeshCache.GetRange(mgsAddrBegin, mgsAddrEnd);

   // Because we can have multiple items in a stage pointing to the same mesh ptr,
   // the idea is to store the ptrs we want to delete to a vector, sort that vector, and 
   // only delete unique pointers from that vector.

   std::vector< void * > sortedMeshAssets;
   sortedMeshAssets.reserve( std::distance( range.first, range.second ) );

   for( CMGSResourceCache::TCache::iterator it = range.first; it != range.second; )
   {
      SMGSResourceCacheItem & item = it->second;

      sortedMeshAssets.push_back( item.mpData );

      it = gMeshCache.RemoveItem(it);
   }

   std::sort( sortedMeshAssets.begin(), sortedMeshAssets.end() );

   void *pLastAsset = NULL;
   for ( std::vector<void *>::const_iterator it = sortedMeshAssets.begin(); it != sortedMeshAssets.end(); ++it )
   {
      void *pAsset = *it;
      if ( pAsset == pLastAsset )
      {
         continue;
      }

      pLastAsset = pAsset;

#if BPE_TARGET == BPE_TARGET_WIN32 || !BPE_USE_EDGE_GEOM
      CMesh* pMesh = (CMesh*)pAsset;
      CMesh::DeleteMesh(pMesh);
#else
      SEdgeModelHeader* pModel = (SEdgeModelHeader*)pAsset;

      CRenderHWAllocator::SHandle const * pRsxMemory = (CRenderHWAllocator::SHandle const *)pModel->mpRsxMemory;
      if( pRsxMemory )
      {
         gpRenderBackend->Free(pRsxMemory);
      }

      BPE_FREE_ALIGNED(pModel);
#endif

   }
}

//----------------------------------------------------------------------------

int BP_GetLoadedMeshCount()
{
   return gMeshCache.GetCount();
}

//----------------------------------------------------------------------------

void * BP_ConstructTexture(const char* name, void* memory, int size)
{
   CResId const resId(name);
   SFactoryResourceBuildData buildData(resId, memory, size, NULL, *gpResources);
   SFactoryReturnResource output;

   CBaseTexture::FTextureFactory(buildData, output);
   return output.mpResource;
}

//----------------------------------------------------------------------------

static void BP_BindMGSTextureDirect(const unsigned int assetCacheTag, char * path, unsigned int mgsAddr)
{
   SMGSResourceCacheItem * pItem = gTextureCache.Item(mgsAddr);

   if( pItem )
   {
      BPE_VERIFYA(false, "Trying to load two textures with same mgs address.");
      return;
   }

   //Find this asset in the stage cache.
   BP_InsertPlatformSubfolder( path );
   void * pAsset = gStageAssetCache.TakeItem( assetCacheTag, path );
   if ( !pAsset )
   {
      printf("BP ERROR: Unable to find texture '%s' tag %8.8x in the texture cache.\n", path, assetCacheTag );
      BPE_VERIFYA( false, "Couldn't find Bluepoint asset!" );
      return;
   }
   
   SMGSResourceCacheItem item;
   item.mIndex = -1;
   item.mFileName = path;
   item.mpData = pAsset;
   item.mbDeleted = false;
   gTextureCache.AddItem(mgsAddr, item);
   return;
}

//----------------------------------------------------------------------------

void BP_BindMGSTexture(const unsigned int assetCacheTag, const char * pCurrentPath, unsigned int triId, unsigned int texId, unsigned int mgsAddr)
{
#if MGS_VERSION == 2
   unsigned int const GV_FCACHEID_RESIDENT = 0x80000000;
#endif

   texId &= ~GV_FCACHEID_RESIDENT;
   triId &= ~GV_FCACHEID_RESIDENT;

   char path[FILENAME_MAX];
   sprintf( path, "%s%8.8x/%8.8x.ctxr", pCurrentPath, triId, texId );

   BP_BindMGSTextureDirect(assetCacheTag, path, mgsAddr);
}

//----------------------------------------------------------------------------

void BP_BindMGSImage(const unsigned int assetCacheTag, const char * pCurrentPath, unsigned int imageId, unsigned int mgsAddr)
{
#if MGS_VERSION == 2
   unsigned int const GV_FCACHEID_RESIDENT = 0x80000000;
#endif

   imageId &= ~GV_FCACHEID_RESIDENT;

   char path[FILENAME_MAX];
   sprintf( path, "%s%8.8x.img.ctxr", pCurrentPath, imageId );

   BP_BindMGSTextureDirect(assetCacheTag, path, mgsAddr);
}

//----------------------------------------------------------------------------

void BP_ResetTextureByMGSAddr(unsigned int mgsAddr, const char* path)
{
   // find the current item in the cache.
   SMGSResourceCacheItem * cacheItem = 0;
   if ( gTextureCache.GetItem(mgsAddr) )
      cacheItem = gTextureCache.Item(mgsAddr);
   else
   {
      cacheItem = gResidentTextureCache.Item(gResidentTextureCache.FindKey((void*)((DG_TEX*)mgsAddr)->BP_TextureHandle));
      if ( !cacheItem )
         cacheItem = gTextureCache.Item(gTextureCache.FindKey((void*)((DG_TEX*)mgsAddr)->BP_TextureHandle));
   }

   // Make sure that the file's contents aren't in a RAM cache
   BP_IO_FlushRAMDiskCache();

   // open the file specified.
   void* handle = BP_OpenFile(path, NULL);
   if( handle )
   {
      // calculate the file's size and allocate memory for it.
      int size = BP_GetFileSize(handle);
      void* memory = malloc(size);

      // read the entire file into memory and close it.
      BP_ReadFile(handle, memory, size);
      BP_CloseFile(handle);

      // build the texture and reset the handle.
      cacheItem->mpData = BP_ConstructTexture(path, memory, size );
      if ( cacheItem->mpData != 0 )
      {
         DG_TEX* pTex = (DG_TEX*)mgsAddr;
         pTex->BP_TextureHandle = ( unsigned int )cacheItem->mpData;
      }
      else
      {
         printf("BP WARNING: Unable to create texture '%s'\n", path);
      }

      // free the memory we've allocated.
      free(memory);
      return;
   }
   else
   {
      printf("BP WARNING: Failed to load texture '%s'\n", path);
   }
}

//----------------------------------------------------------------------------

unsigned int BP_GetTextureByMGSAddr(unsigned int mgsAddr)
{
   SMGSResourceCacheItem const * pItem = gTextureCache.GetItem(mgsAddr);
   return (unsigned int)(pItem ? pItem->mpData : 0);
}

//----------------------------------------------------------------------------

const char* BP_GetTextureNameByMGSAddr(unsigned int mgsAddr)
{
   // search through the texture cache to find the given texture.
   SMGSResourceCacheItem const * pItem = gTextureCache.GetItem(mgsAddr);

   // if the texture wasn't found in the texture cache, search the resident
   // texture cache.
   if ( !pItem )
   {
      pItem = gResidentTextureCache.GetItem(gResidentTextureCache.FindKey((void*)((DG_TEX*)mgsAddr)->BP_TextureHandle));
      if ( !pItem )
         pItem = gTextureCache.GetItem(gTextureCache.FindKey((void*)((DG_TEX*)mgsAddr)->BP_TextureHandle));
   }

   // return the item's name if possible.
   return (pItem ? pItem->mFileName.c_str() : "NULL");
}

//----------------------------------------------------------------------------

void BP_TextureSetResidentTexture(unsigned int texture)
{
   CMGSResourceCache::TItemRange const range = gTextureCache.GetRange(0, 0xFFFFFFFF);

   for( CMGSResourceCache::TCache::iterator it = range.first; it != range.second; ++it )
   {
      SMGSResourceCacheItem & item = it->second;

      CBaseTexture* pTexture = (CBaseTexture*)item.mpData;

      if( (unsigned int)pTexture == texture )
      {
         pTexture->mIsResidentTexture = 1;
         gResidentTextureCache.AddItem(gResidentTextureCache.GetCount(), it->second);
         gTextureCache.RemoveItem(it);
         break;
      }
   }
}

//----------------------------------------------------------------------------

void BP_DestroyTextures(unsigned int mgsAddrBegin, unsigned int mgsAddrEnd)
{
   CMGSResourceCache::TItemRange const range = gTextureCache.GetRange(mgsAddrBegin, mgsAddrEnd);
   
   for( CMGSResourceCache::TCache::iterator it = range.first; it != range.second; ++it )
   {
      SMGSResourceCacheItem & item = it->second;

#if BP_ENABLE_TEXTURE_TOOL
      BP_TextureTool_TextureDestroyed((DG_TEX*)it->first);
#endif

      CBaseTexture* pTexture = (CBaseTexture*)item.mpData;

      if( !pTexture->mIsResidentTexture )
      {
         item.mbDeleted = true;
      }
   }
}

//----------------------------------------------------------------------------

void BP_FlushDestroyedTextures()
{
   //Ensure that each texture is only deleted once.  Multiple references can be present within a slot.
   CMGSResourceCache::TItemRange const range = gTextureCache.GetEntireRange();

   //All textures to be removed from the cache
   std::vector< CBaseTexture * > texturesToDelete;
   texturesToDelete.reserve( gTextureCache.GetCount() );
   for( CMGSResourceCache::TCache::iterator it = range.first; it != range.second; )
   {
      SMGSResourceCacheItem & item = it->second;

      CBaseTexture* pTexture = (CBaseTexture*)item.mpData;

      if( item.mbDeleted )
      {
         texturesToDelete.push_back( pTexture );
         it = gTextureCache.RemoveItem(it);
      }
      else
      {
         ++it;
      }
   }
   CBaseTexture * pLastTexture = NULL;
   std::sort( texturesToDelete.begin(), texturesToDelete.end() );
   //Just the first of each texture in the vector after it's sorted is deleted.
   for( std::vector< CBaseTexture * >::iterator it = texturesToDelete.begin(); it != texturesToDelete.end(); ++it )
   {
      CBaseTexture * pTexture = *it;
      if( pTexture != pLastTexture )
      {
         CBaseTexture::DeleteTexture(pTexture);
      }
      pLastTexture = pTexture;
   }
}

//----------------------------------------------------------------------------

int BP_GetLoadedTextureCount()
{
   return gTextureCache.GetCount();
}

//----------------------------------------------------------------------------

void BP_DestroyResources(unsigned int mgsAddrBegin, unsigned int mgsAddrEnd)
{
   BP_DestroyMeshes(mgsAddrBegin, mgsAddrEnd);
   BP_DestroyTextures(mgsAddrBegin, mgsAddrEnd);
}

//----------------------------------------------------------------------------

void BP_FlushDestroyedResources()
{
   BP_FlushDestroyedTextures();
   //Meshes are destroyed immediately so no "flush" step is necessary.
}

//----------------------------------------------------------------------------

#ifdef __cplusplus
};
#endif
