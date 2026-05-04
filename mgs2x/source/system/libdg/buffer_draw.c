//----------------------------------------------------------------------------
//
// Buffer the channels and the data they point to so that
// the guts of DG_EndFrame can run on a separate thread to the game
//
//----------------------------------------------------------------------------

//stdafx.h to get precompiled common.h in Windows
#include "stdafx.h"

#include "libdg.h"
#include "BP_Debug.h"
#include "BP_Memory.h"
#include "BP_Renderer.h"
#include "BP_RenderBufferTypes.h"

#if BP_VITA
#  include <sce_atomic.h>
#endif

/*----------------------------------------------------------------*/
// GLOBALS

DG_CHANL *DG_Chanls_Buf = NULL;
DG_PLUGIN *DG_Plugins_Buf = NULL;

#define SCENE_BUF_SIZE (5000000)
static char *sSceneBufferMem = NULL;
static volatile int sSceneBufferCurOffset = 0;
extern int gBP_SceneBufferAllocSize;
extern int gBP_SceneBufferPeakSize;
extern char gBP_SceneBufferPeakArea[256];

static int sSceneBufferTimestamp = 0;

#define BUFFERDRAW_DEBUG_ALLOC_HISTORY 0

#if BUFFERDRAW_DEBUG_ALLOC_HISTORY

#define skNumDebugAllocHistory 1024
static int sAllocSizeHistory[ skNumDebugAllocHistory ] = { 0 };
static volatile int sAllocPivot = 0;

#define skMaxDebugAllocCheckpoint 1024
static int sCheckpointHistory[ skMaxDebugAllocCheckpoint ];
static char const *sCheckpointNames[ skMaxDebugAllocCheckpoint ];
int sCheckpointCurrent = 0;

static void _sceneBufferDebugAllocResetCheck()
{
   sCheckpointCurrent = 0;
}

static void _sceneBufferDebugAllocCheckpoint( char const *name )
{
   if ( sCheckpointCurrent != skMaxDebugAllocCheckpoint )
   {
      sCheckpointHistory[ sCheckpointCurrent ] = sSceneBufferCurOffset;
      sCheckpointNames[ sCheckpointCurrent ] = name;

      ++sCheckpointCurrent;
   }
}

static void _sceneBufferDebugAllocPrint()
{
   int i;

   for ( i = 0; i < sCheckpointCurrent; ++i )
   {
      printf( "Checkpoint: %s %d\n", sCheckpointNames[ i ], sCheckpointHistory[ i ] );
   }
}

void _sceneBufferPostWork( TBP_Render_Ult_Function func, void const *param, int const size )
{
   func( (SULTParam const *) param );
}

#else

static void _sceneBufferDebugAllocResetCheck()
{
}

#define _sceneBufferDebugAllocCheckpoint(x) (0)

static void _sceneBufferDebugAllocPrint()
{
}

#define _sceneBufferPostWork(x,y,z) BP_Render_PostUltWork(x,y,z)

#endif

/*----------------------------------------------------------------*/
// CONTEXT

typedef struct SBufferContext_s
{
   int mBufferPrimTextures;
}
SBufferContext;

static void _sceneBufferContextInit( SBufferContext *pContext )
{
   memset( pContext, 0, sizeof( *pContext ) );

   if ( !strcmp( GM_GetArea(), "title" ) )
   {
      pContext->mBufferPrimTextures = 1;
   }
}

/*----------------------------------------------------------------*/
// SCENE BUFFER

static void _sceneBufferInit(int which)
{
   if (sSceneBufferMem == NULL)
   {
      sSceneBufferMem = (char *)BP_Memory_Alloc(SCENE_BUF_SIZE, 16, kMT_Permanent, kMC_EndFrame);
      gBP_SceneBufferAllocSize = SCENE_BUF_SIZE;

      printf("Scene buffer extents: %08x - %08x\n", sSceneBufferMem, sSceneBufferMem + SCENE_BUF_SIZE);
   }
   // 16 byte aligned

   sSceneBufferCurOffset =  0;

   _sceneBufferDebugAllocResetCheck();
}

static inline char *_sceneBufferAlloc(int size)
{
   int const alignedSize = ( size + 15 ) & ( ~15 );
   int oldCurrentOffset = sSceneBufferCurOffset;

#if BP_VITA
   oldCurrentOffset = sceAtomicAdd32( &sSceneBufferCurOffset, alignedSize );
#else
   oldCurrentOffset = sSceneBufferCurOffset;
   sSceneBufferCurOffset += alignedSize;
#endif

#if BUFFERDRAW_DEBUG_ALLOC_HISTORY
   {
      int newIndex;

      for ( ;; )
      {
         int oldPivot = sAllocPivot;
         int wantNewPivot = ( oldPivot + 1 ) % skNumDebugAllocHistory;

         if ( oldPivot == sceAtomicCompareAndSwap32( &sAllocPivot, oldPivot, wantNewPivot ) )
         {
            newIndex = oldPivot;
            break;
         }
      }

      sAllocSizeHistory[ newIndex ] = size;
   }
#endif

   if ( sSceneBufferCurOffset > SCENE_BUF_SIZE)
   {
#if BUFFERDRAW_DEBUG_ALLOC_HISTORY
      int i;
#endif
      printf("Need to increase the size of the scene buffer to at least %d!\n", sSceneBufferCurOffset);
#if BUFFERDRAW_DEBUG_ALLOC_HISTORY
      _sceneBufferDebugAllocPrint();

      for ( i = 0; i < skNumDebugAllocHistory; ++i )
      {
         printf( "History %d: %d\n", i, sAllocSizeHistory[i] );
      }

      printf( "Pivot: %d\n", sAllocPivot );
      HANGUP();
#endif
      assert(sSceneBufferCurOffset <= SCENE_BUF_SIZE);
   }

   return sSceneBufferMem + oldCurrentOffset;
}

static inline char *_sceneBufferAllocCopy(void const *src, int size)
{
   char *dst = _sceneBufferAlloc(size);
   memcpy(dst, src, size);
   return dst;
}

/*----------------------------------------------------------------*/
// OBJECTS

#define max(x,y) ((x)>(y)?(x):(y))
#define copy_n_of_x(dst,src,m,x,n) { dst->m = (x *)_sceneBufferAlloc(n*sizeof(x)); memcpy(dst->m, src->m, n*sizeof(x)); }


static DG_OBJS *_bufferSingleObjs(DG_OBJS *objs)
{
   int j;
   DG_OBJS *bufObjs = (DG_OBJS *)_sceneBufferAllocCopy(objs, sizeof(DG_OBJS) + objs->n_models*sizeof(DG_OBJ));

   // keep a pointer to the buffered copy for later channels (i.e., spots)
   objs->mASBufferedCopy = bufObjs;
   assert(objs->mASBufferedCopyTimestamp != sSceneBufferTimestamp);
   objs->mASBufferedCopyTimestamp = sSceneBufferTimestamp;

   bufObjs->mASBufferedCopy = bufObjs;
   bufObjs->mASBufferedCopyTimestamp = sSceneBufferTimestamp;

   // These tests mimic the tests for usage in screen.c
   if ( objs->flag & ( DG_FLAG_ONEPIECE | DG_FLAG_FINISHCALC ) )
   {
      // do nothing with rots and movs!
   }
   else
   {
      if (objs->rots != NULL)
      {
         bufObjs->rots = (FVECTOR *)_sceneBufferAllocCopy(objs->rots, objs->def->n_models*sizeof(FVECTOR));
      }
      else if ( objs->movs != NULL )
      {
         bufObjs->movs = (FVECTOR *)_sceneBufferAllocCopy(objs->movs, objs->def->n_models*sizeof(FVECTOR));
      }
   }

   // now buffer DG_OBJS
   if (objs->root != NULL)
   {
      bufObjs->root = (FMATRIX *)_sceneBufferAllocCopy(objs->root, sizeof(FMATRIX));
   }
//   printf( "FR:%6d OBJS: %8.8x, Light: %8.8x\n", sSceneBufferTimestamp, objs, objs->light );
   bufObjs->light = (FMATRIX *)_sceneBufferAllocCopy(objs->light, 2*sizeof(FMATRIX));

   // buffer objects for shadows
   if (objs->low != NULL)
   {
      bufObjs->low = _bufferSingleObjs(objs->low);
   }

   // buffer DG_OBJ
   for (j = 0; j < objs->n_models; ++j)
   {
      DG_OBJ *obj = &objs->objs[j];
      DG_OBJ *bufObj = &bufObjs->objs[j];
      bufObj->BP_DGObjs = bufObjs;

      // obj->light is almost always equal to objs->light
      if ( obj->light == objs->light )
      {
         bufObj->light = bufObjs->light;
      }
      else
      {
         bufObj->light = (FMATRIX *)_sceneBufferAllocCopy(obj->light, 2*sizeof(FMATRIX));
      }

      bufObj->matrix_addr = NULL;
#if 0
      // matrix_addr is for temp storage during render
         
      if (obj->matrix_addr)
      {
         bufObj->matrix_addr = (KP_MatrixData *)_sceneBufferAllocCopy(obj->matrix_addr, sizeof(KP_MatrixData));
      }
#endif
      bufObj->packets = (DG_OBJ_PACKET *)_sceneBufferAllocCopy(obj->packets, obj->n_packs*sizeof(DG_OBJ_PACKET));
   }
#if 0
   // and buffer all DG_OBJ
   for (j = 0; j < objs->n_models; ++j)
   {
      int k;
      int totalVerts = 0;
      int totalNorms = 0;
      int totalUVs[3] = { 0 };
      int totalRGBs = 0;
      DG_OBJ *bufObj = &bufObjs->objs[j];
      DG_OBJ *obj = &objs->objs[j];
      for (k = 0; k < bufObj->n_packs; ++k)
      {
         DG_OBJ_PACKET *pkt = &bufObj->packets[k];
         totalVerts = max(totalVerts, pkt->n_verts + pkt->verts_offset);
         totalNorms = max(totalNorms, pkt->n_verts + pkt->norms_offset);
         totalUVs[0] = max(totalUVs[0], pkt->n_verts + pkt->uvs_offset[0]);
         totalUVs[1] = max(totalUVs[1], pkt->n_verts + pkt->uvs_offset[1]);
         totalUVs[2] = max(totalUVs[2], pkt->n_verts + pkt->uvs_offset[2]);
         totalRGBs = max(totalRGBs, pkt->n_verts + pkt->rgbs_offset);
      }
      // now copy these
      copy_n_of_x(bufObj, obj, verts, SVECTOR, totalVerts);
      copy_n_of_x(bufObj, obj, norms, SVECTOR, totalNorms);
      if (obj->uvs[0] != NULL)
      {
         copy_n_of_x(bufObj, obj, uvs[0], short, totalUVs[0]);
      }
      if (obj->uvs[1] != NULL)
      {
         copy_n_of_x(bufObj, obj, uvs[1], short, totalUVs[1]);
      }
      if (obj->uvs[2] != NULL)
      {
         copy_n_of_x(bufObj, obj, uvs[2], short, totalUVs[2]);
      }
      if (obj->rgbs != NULL)
      {
         copy_n_of_x(bufObj, obj, rgbs, u_int, totalRGBs);
      }
   }
#endif
   return bufObjs;
}

typedef struct SBufferSingleObjWork_s
{
   DG_OBJS **mBufObjQueueOut;
   DG_OBJS **mQueueIn;
   int mQueueInIndexStart;
   int mQueueInIndexEnd;
   int mQueueOutObjStart;
   int mVisibleChannelFlags;
   int pad[1]; // padding for ult
} SBufferSingleObjWork;

KP_CTASSERT( sizeof( SBufferSingleObjWork ) == sizeof( SULTParam ) );

static void _bufferObjsThreaded( SULTParam const *pUlt )
{
   SBufferSingleObjWork const *work = (SBufferSingleObjWork const *) pUlt;
   int num_objs = work->mQueueOutObjStart;
   int i;
   DG_OBJS **oque = work->mQueueIn;
   DG_OBJS **bufObjQueue = work->mBufObjQueueOut;
   int const visibleChannelFlags = work->mVisibleChannelFlags;

   for ( i =work->mQueueInIndexStart; i < work->mQueueInIndexEnd; ++i )
   {
      DG_OBJS *objs = oque[i];
      
      if (((objs->flag & visibleChannelFlags) != visibleChannelFlags) || (objs->flag & (DG_FLAG_SHADOWWRITE | DG_FLAG_SHADOWMAKE)))
      {
         bufObjQueue[num_objs++] = _bufferSingleObjs(objs);
      }
   }

}

static void _bufferObjs(DG_OBJ_QUEUE *bufQueue, DG_OBJ_QUEUE *queue, int visibleChannelFlags)
{
   DG_OBJ_BUFFER *obj_buff = &queue->objs_buffer;

   if ( obj_buff->n_queue != 0 )
   {
      int i;
      DG_OBJS **oque = (DG_OBJS **) obj_buff->queue;
      int num_objs = 0;


      // count visible DG_OBJS
      for (i = 0; i < obj_buff->n_queue; ++i, ++oque)
      {
         DG_OBJS *objs = *oque;
         // needs to be invisible on ALL visible channels
         if (((objs->flag & visibleChannelFlags) != visibleChannelFlags) || (objs->flag & (DG_FLAG_SHADOWWRITE | DG_FLAG_SHADOWMAKE)))
         {
            ++num_objs;
         }
      }
      bufQueue->objs_buffer.n_queue = num_objs;

      if (num_objs > 0)
      {
         static int const skNumObjsPerThread = 0xF; // Must be power of two minus 1

         DG_OBJS **bufObjQueue = (DG_OBJS **)_sceneBufferAlloc(num_objs*sizeof(DG_OBJS *));
         SBufferSingleObjWork soWork = { 0 };

         bufQueue->objs_buffer.queue = (void **)bufObjQueue;
         oque = (DG_OBJS **) obj_buff->queue;
         num_objs = 0;

         // SBufferSingleObjectWork is a ULT Param

         soWork.mBufObjQueueOut = bufObjQueue;
         soWork.mVisibleChannelFlags = visibleChannelFlags;
         soWork.mQueueIn = oque;
         soWork.mQueueOutObjStart = 0;
         soWork.mQueueInIndexStart = soWork.mQueueInIndexEnd = 0;

         // go through the objects, and when we've found skNumObjsPerThread #
         // of visible objects, flush those out to a ULT work item
         for (i = 0; i < obj_buff->n_queue; ++i, ++oque)
         {
            DG_OBJS *objs = *oque;
            if (((objs->flag & visibleChannelFlags) != visibleChannelFlags) || (objs->flag & (DG_FLAG_SHADOWWRITE | DG_FLAG_SHADOWMAKE)))
            {
               ++num_objs;

               if ( ( num_objs & skNumObjsPerThread ) == 0 )
               {
                  // Tell the current work item to stop here
                  soWork.mQueueInIndexEnd = i + 1;
                  _sceneBufferPostWork( _bufferObjsThreaded, (SULTParam const *) (&soWork), sizeof( soWork ) );

                  // Tell the next work item to start here.
                  soWork.mQueueInIndexStart = i + 1;
                  soWork.mQueueOutObjStart = num_objs;
               }
            }
         }

         // If there are any remaining items, flush them
         if ( soWork.mQueueOutObjStart != num_objs )
         {
            soWork.mQueueInIndexEnd = i;
            _sceneBufferPostWork( _bufferObjsThreaded, (SULTParam const *) (&soWork), sizeof( soWork ) );
         }
      }
   }
}

static void _bufferObjsUlt( SULTParam const *param )
{
   _bufferObjs( (DG_OBJ_QUEUE *) param->p0, (DG_OBJ_QUEUE *) param->p1, param->p2 );
}

/*----------------------------------------------------------------*/
// EVMs

static void _bufferEvm(DG_OBJ_QUEUE *bufQueue, DG_OBJ_QUEUE *queue, int visibleChannelFlags)
{
   DG_OBJ_BUFFER *obj_buff = &queue->evmobj_buffer;
   if ( obj_buff->n_queue != 0 )
   {
      int i;
      DG_EVMOBJ **oque = (DG_EVMOBJ **) obj_buff->queue;
      int num_objs = 0;
      // count visible DG_EVMOBJ
      for (i = 0; i < obj_buff->n_queue; ++i, ++oque)
      {
         DG_EVMOBJ *evmobj = *oque;
         // needs to be invisible on ALL visible channels
         if ((evmobj->flag & visibleChannelFlags) != visibleChannelFlags)
         {
            ++num_objs;
         }
      }
      bufQueue->evmobj_buffer.n_queue = num_objs;
      if (num_objs > 0)
      {
         DG_EVMOBJ **bufObjQueue = (DG_EVMOBJ **)_sceneBufferAlloc(num_objs*sizeof(DG_EVMOBJ *));
         bufQueue->evmobj_buffer.queue = (void **)bufObjQueue;
         oque = (DG_EVMOBJ **) obj_buff->queue;
         num_objs = 0;
         for (i = 0; i < obj_buff->n_queue; ++i, ++oque)
         {
            DG_EVMOBJ *evmobj = *oque;
            if ((evmobj->flag & visibleChannelFlags) != visibleChannelFlags)
            {
               DG_EVMOBJ *bufEvm = (DG_EVMOBJ *)_sceneBufferAllocCopy(evmobj, sizeof(DG_EVMOBJ));
               bufObjQueue[num_objs++] = bufEvm;

               if (evmobj->root != NULL)
               {
                  bufEvm->root = (FMATRIX *)_sceneBufferAllocCopy(evmobj->root, sizeof(FMATRIX));
               }
               bufEvm->light = (FMATRIX *)_sceneBufferAllocCopy(evmobj->light, 2*sizeof(FMATRIX));

               bufEvm->matrix[bufEvm->use_buffer] = (FMATRIX *)_sceneBufferAllocCopy(evmobj->matrix[evmobj->use_buffer], evmobj->n_skeleton*sizeof(FMATRIX));

               bufEvm->packs = (DG_EVMPACK *)_sceneBufferAllocCopy(evmobj->packs, evmobj->n_packet*sizeof(DG_EVMPACK));

               // looks like the pack stuff just points to the def
               // might need to buffer it if it is changed
            }
         }
      }
   }
}

/*----------------------------------------------------------------*/
// PRIMS

static void _bufferPrims(DG_OBJ_QUEUE *bufQueue, DG_OBJ_QUEUE *queue, int visibleChannelFlags, SBufferContext *pContext)
{
   DG_OBJ_BUFFER *obj_buff = &queue->prim2_buffer;
   if ( obj_buff->n_queue != 0 )
   {
      int i;
      DG_PRIM2 **oque = (DG_PRIM2 **) obj_buff->queue;
      int num_objs = 0;
      // count visible DG_PRIM2
      for (i = 0; i < obj_buff->n_queue; ++i, ++oque)
      {
         DG_PRIM2 *prim = *oque;
         // needs to be invisible on ALL visible channels
         if ((prim->flag & visibleChannelFlags) != visibleChannelFlags)
         {
            ++num_objs;
         }
      }
      bufQueue->prim2_buffer.n_queue = num_objs;
      if (num_objs > 0)
      {
         DG_PRIM2 **bufObjQueue = (DG_PRIM2 **)_sceneBufferAlloc(num_objs*sizeof(DG_PRIM2 *));
         bufQueue->prim2_buffer.queue = (void **)bufObjQueue;
         oque = (DG_PRIM2 **) obj_buff->queue;
         num_objs = 0;
         for (i = 0; i < obj_buff->n_queue; ++i, ++oque)
         {
            DG_PRIM2 *src = *oque;
            if ((src->flag & visibleChannelFlags) != visibleChannelFlags)
            {
               DG_PRIM2 *dst = (DG_PRIM2 *)_sceneBufferAllocCopy(src, sizeof(DG_PRIM2));
               bufObjQueue[num_objs++] = dst;

               if (dst->root != NULL)
               {
                  dst->root = (FMATRIX *)_sceneBufferAllocCopy(src->root, sizeof(FMATRIX));
               }

               {
                  // only copy the arrays pointed to by buffer_clock!

#define BPI_NONE        0
#define BPI_PACKETS     1
#define BPI_EVERYTHING  2

#define BUFFER_PRIM_INTERNALS BPI_PACKETS

#if BUFFER_PRIM_INTERNALS > BPI_NONE

                  int bufclock = src->buffer_clock;
                  int numVerts = src->n_prims * src->packet_verts;

#  if BUFFER_PRIM_INTERNALS == BPI_EVERYTHING
                  int uvrgb_size = src->type < DG_PRIM2_SPRT ? sizeof(DG_PRIM2_UVRGB) : sizeof(DG_PRIM2_UVRGBWH);
                  dst->pos[bufclock] = (FVECTOR *)_sceneBufferAllocCopy(src->pos[bufclock], numVerts*sizeof(FVECTOR));
                  dst->uvrgb[bufclock] = (void *)_sceneBufferAllocCopy(src->uvrgb[bufclock], numVerts*uvrgb_size);
#  endif // BUFFER_PRIM_INTERNALS == BPI_EVERYTHING

                  if ( pContext->mBufferPrimTextures )
                  {
                     dst->BP_tex = (DG_TEX *)_sceneBufferAllocCopy(src->BP_tex, sizeof(DG_TEX));
                  }

                  dst->packet[bufclock] = (DG_PRIM2_PACKET *)_sceneBufferAllocCopy(src->packet[bufclock], src->n_prims*sizeof(DG_PRIM2_PACKET));

                  // buffer packets.
                  {
                     DG_PRIM2_PACKET *packet = dst->packet[bufclock];
#  if BUFFER_PRIM_INTERNALS == BPI_EVERYTHING
                     FVECTOR *pos = dst->pos[bufclock];
                     void *uvrgb = dst->uvrgb[bufclock];
#  endif // BUFFER_PRIM_INTERNALS == BPI_EVERYTHING
                     int j;
                     for ( j = dst->n_prims ; j > 0  ; --j, ++packet )
                     {
//                        PREFETCH( packet + 1 ); // not defined for MGS2 yet
                        packet->prim = dst;
#  if BUFFER_PRIM_INTERNALS == BPI_EVERYTHING
                        packet->pos_addr = pos ;
                        packet->uvrgb_addr = uvrgb ;
                        pos += src->packet_verts ;
                        uvrgb = (char *) uvrgb + uvrgb_size * src->packet_verts;
#  endif // BUFFER_PRIM_INTERNALS == BPI_EVERYTHING
                     }
                  }
#endif // BUFFER_PRIM_INTERNALS > BPI_NONE
               }
            }
         }
      }
   }
}

static void _bufferPrimsUlt( SULTParam const *ult )
{
   //DG_OBJ_QUEUE *bufQueue, DG_OBJ_QUEUE *queue, int visibleChannelFlags, SBufferContext *pContext
   _bufferPrims( (DG_OBJ_QUEUE *) ult->p0, (DG_OBJ_QUEUE *) ult->p1, ult->p2, (SBufferContext *) ult->p3 );
}


/*----------------------------------------------------------------*/
// CLONE MODELS

static void _bufferComdl(DG_OBJ_QUEUE *bufQueue, DG_OBJ_QUEUE *queue, int visibleChannelFlags)
{
   // clone models
   DG_OBJ_BUFFER *obj_buff = &queue->comdl_buffer;
   if ( obj_buff->n_queue != 0 )
   {
      int i;
      DG_COMDL **oque = (DG_COMDL **) obj_buff->queue;
      int num_objs = 0;
      // count visible DG_COMDL
      for (i = 0; i < obj_buff->n_queue; ++i, ++oque)
      {
         DG_COMDL *comdl = *oque;
         // needs to be invisible on ALL visible channels
         if ((comdl->flag & visibleChannelFlags) != visibleChannelFlags)
         {
            ++num_objs;
         }
      }
      bufQueue->comdl_buffer.n_queue = num_objs;
      if (num_objs > 0)
      {
         DG_COMDL **bufObjQueue = (DG_COMDL **)_sceneBufferAlloc(num_objs*sizeof(DG_COMDL *));
         bufQueue->comdl_buffer.queue = (void **)bufObjQueue;
         oque = (DG_COMDL **) obj_buff->queue;
         num_objs = 0;
         for (i = 0; i < obj_buff->n_queue; ++i, ++oque)
         {
            DG_COMDL *comdl = *oque;
            if ((comdl->flag & visibleChannelFlags) != visibleChannelFlags)
            {
               DG_COMDL *bufComdl = (DG_COMDL *)_sceneBufferAllocCopy(comdl, sizeof(DG_COMDL) + comdl->n_objs*sizeof(DG_COMDL_POS));
               bufObjQueue[num_objs++] = bufComdl;
            }
         }
      }
   }
}

/*----------------------------------------------------------------*/
// SPOT SHADOWS

static void _bufferSpot(DG_OBJ_QUEUE *bufQueue, DG_OBJ_QUEUE *queue)
{
   DG_OBJ_BUFFER *obj_buff = &queue->spot_buffer;
   if ( obj_buff->n_queue != 0 )
   {
      int i;
      DG_SPOT **oque = (DG_SPOT **) obj_buff->queue;
      int num_objs = 0;
      // count visible DG_SPOT
      for (i = 0; i < obj_buff->n_queue; ++i, ++oque)
      {
         DG_SPOT *spot = *oque;
         if (!(spot->flag & DG_FLAG_INVISIBLE))
         {
            ++num_objs;
         }
      }
      bufQueue->spot_buffer.n_queue = num_objs;
      if (num_objs > 0)
      {
         DG_SPOT **bufObjQueue = (DG_SPOT **)_sceneBufferAlloc(num_objs*sizeof(DG_SPOT *));
         bufQueue->spot_buffer.queue = (void **)bufObjQueue;
         oque = (DG_SPOT **) obj_buff->queue;
         num_objs = 0;
         for (i = 0; i < obj_buff->n_queue; ++i, ++oque)
         {
            DG_SPOT *spot = *oque;
            if (!(spot->flag & DG_FLAG_INVISIBLE))
            {
               DG_OBJS *objs;
               DG_SPOT *bufSpot = (DG_SPOT *)_sceneBufferAllocCopy(spot, sizeof(DG_SPOT));
               bufObjQueue[num_objs++] = bufSpot;

               if (spot->root != NULL)
               {
                  bufSpot->root = (FMATRIX *)_sceneBufferAllocCopy(spot->root, sizeof(FMATRIX));
               }

               if (spot->objs != NULL)
               {
                  // redirect objs pointer to the buffered copy
                  objs = (DG_OBJS *)spot->objs;
                  if (objs->mASBufferedCopyTimestamp != sSceneBufferTimestamp)
                  {
                     // didn't get buffered somehow...
                     bufSpot->objs = _bufferSingleObjs(objs);
                  }
                  else
                  {
                     bufSpot->objs = objs->mASBufferedCopy;
                  }

                  // Need to buffer the next_dgobjs in the spot's dgobj list. They aren't guaranteed to be in 
                  // the main obj_buffer.
                  objs = (DG_OBJS*)bufSpot->objs;
                  while (objs->next_dgobjs != NULL)
                  {
                     if (objs->next_dgobjs->mASBufferedCopyTimestamp != sSceneBufferTimestamp)
                     {
                        _bufferSingleObjs(objs->next_dgobjs);
                     }
                     objs->next_dgobjs = objs->next_dgobjs->mASBufferedCopy;
                     objs = objs->next_dgobjs;
                  }
               }
            }
         }
      }
   }
}

/*----------------------------------------------------------------*/
// DMAPACKs

#if BP_VITA
__thread
#endif
extern char *BP_CurrentDmaPackPtr;
extern void DG_ExecAuto2DPrim(void *addr);

static void _bufferPluginDmapacks(DG_OBJ_BUFFER *bufPlug, DG_OBJ_BUFFER *plugin, int visibleChannelFlags)
{
   int i;
   bufPlug->queue = NULL;
   if ( plugin->n_queue != 0 )
   {
      DG_DMAPACK **oque = (DG_DMAPACK **) plugin->queue;
      int num_objs = 0;
      // count visible DG_DMAPACK
      for (i = 0; i < plugin->n_queue; ++i, ++oque)
      {
         DG_DMAPACK *pack = *oque;
         if ((pack->flag & visibleChannelFlags) != visibleChannelFlags)
         {
            ++num_objs;
         }
      }
      bufPlug->n_queue = num_objs;
      if (num_objs > 0)
      {
         DG_DMAPACK **bufObjQueue = (DG_DMAPACK **)_sceneBufferAlloc(plugin->n_queue*sizeof(DG_DMAPACK *));
         bufPlug->queue = (void **)bufObjQueue;
         oque = (DG_DMAPACK **) plugin->queue;
         num_objs = 0;
         for (i = 0; i < plugin->n_queue; ++i, ++oque)
         {
            DG_DMAPACK *src = *oque;
            if ((src->flag & visibleChannelFlags) != visibleChannelFlags)
            {
               DG_DMAPACK *dst = (DG_DMAPACK *)_sceneBufferAllocCopy(src, sizeof(DG_DMAPACK));
               bufObjQueue[num_objs++] = dst;

               if (!src->BP_renderCallback)
               {
                  if (src->buildAutoPacketCallback)
                  {
                     // dst autopacket is just copied to the render buffer stream
                     dst->autopacket = BP_CurrentDmaPackPtr = sSceneBufferMem + sSceneBufferCurOffset;
                     src->buildAutoPacketCallback(src->buildAutoPacketCallbackParam);
                     sSceneBufferCurOffset = BP_CurrentDmaPackPtr - sSceneBufferMem;
                     BP_CurrentDmaPackPtr = NULL;
                     dst->autopacketSize = ( sSceneBufferMem + sSceneBufferCurOffset ) - (char *)dst->autopacket;
                     dst->buildAutoPacketCallback = NULL;
                  }
                  else if (src->autopacket)
                  {
                     dst->autopacket = BP_CurrentDmaPackPtr = sSceneBufferMem + sSceneBufferCurOffset;
                     DG_ExecAuto2DPrim(src->autopacket);
                     sSceneBufferCurOffset = BP_CurrentDmaPackPtr - sSceneBufferMem;
                     BP_CurrentDmaPackPtr = NULL;
                     dst->autopacketSize = ( sSceneBufferMem + sSceneBufferCurOffset ) - (char *)dst->autopacket;
                  }
                  else
                  {
                     // packets are already double buffered
                  }
               }
               else if (src->BP_bufferCallback)
               {
                  // If we have a buffer callback, let it do some buffering and set up a new work ptr
                  void *newWork = src->BP_bufferCallback(src->BP_callbackParam, dst);
                  dst->BP_callbackParam = newWork;
               }
            }
         }
      }
   }
}

/*----------------------------------------------------------------*/
// PLUGIN CLONES

static void _bufferPluginComdls(DG_OBJ_BUFFER *bufPlug, DG_OBJ_BUFFER *plugin, int visibleChannelFlags)
{
   int i;
   bufPlug->queue = NULL;
   if ( plugin->n_queue != 0 )
   {
      DG_COMDL **oque = (DG_COMDL **) plugin->queue;
      int num_objs = 0;
      // count visible DG_COMDL
      for (i = 0; i < plugin->n_queue; ++i, ++oque)
      {
         DG_COMDL *comdl = *oque;
         if ((comdl->flag & visibleChannelFlags) != visibleChannelFlags)
         {
            ++num_objs;
         }
      }
      bufPlug->n_queue = num_objs;
      if (num_objs > 0)
      {
         DG_COMDL **bufObjQueue = (DG_COMDL **)_sceneBufferAlloc(num_objs*sizeof(DG_COMDL *));
         bufPlug->queue = (void **)bufObjQueue;

         oque = (DG_COMDL **) plugin->queue;
         num_objs = 0;
         for (i = 0; i < plugin->n_queue; ++i, ++oque)
         {
            DG_COMDL *comdl = *oque;
            if ((comdl->flag & visibleChannelFlags) != visibleChannelFlags)
            {
               DG_COMDL *bufComdl = (DG_COMDL *)_sceneBufferAllocCopy(comdl, sizeof(DG_COMDL) + comdl->n_objs*sizeof(DG_COMDL_POS));
               bufObjQueue[num_objs++] = bufComdl;
            }
         }
      }
   }
}

/*----------------------------------------------------------------*/
// PATCHES

static void _bufferPluginPatch(DG_OBJ_BUFFER *bufPlug, DG_OBJ_BUFFER *plugin, int visibleChannelFlags)
{
   int i;
   bufPlug->queue = NULL;
   if ( plugin->n_queue != 0 )
   {
      DG_PATCH **oque = (DG_PATCH **) plugin->queue;
      int num_objs = 0;
      // count visible DG_PATCH
      for (i = 0; i < plugin->n_queue; ++i, ++oque)
      {
         DG_PATCH *patch = *oque;
         if ((patch->flag & visibleChannelFlags) != visibleChannelFlags)
         {
            ++num_objs;
         }
      }
      bufPlug->n_queue = num_objs;
      if (num_objs > 0)
      {
         DG_PATCH **bufObjQueue = (DG_PATCH **)_sceneBufferAlloc(num_objs*sizeof(DG_PATCH *));
         bufPlug->queue = (void **)bufObjQueue;

         oque = (DG_PATCH **) plugin->queue;
         num_objs = 0;
         for (i = 0; i < plugin->n_queue; ++i, ++oque)
         {
            DG_PATCH *patch = *oque;
            if ((patch->flag & visibleChannelFlags) != visibleChannelFlags)
            {
               DG_PATCH *bufPatch = (DG_PATCH *)_sceneBufferAllocCopy(patch, sizeof(DG_PATCH));
               bufObjQueue[num_objs++] = bufPatch;

               if (patch->root)
               {
                  bufPatch->root = (FMATRIX *)_sceneBufferAllocCopy(patch->root, sizeof(FMATRIX));
               }
               bufPatch->light = (FMATRIX *)_sceneBufferAllocCopy(patch->light, 2*sizeof(FMATRIX));
            }
         }
      }
   }
}

/*----------------------------------------------------------------*/
// OPTICAL CAMO

static void _bufferPluginOptCmf(DG_OBJ_BUFFER *bufPlug, DG_OBJ_BUFFER *plugin, int visibleChannelFlags)
{
   int i;
   bufPlug->queue = NULL;
   if ( plugin->n_queue != 0 )
   {
      DG_OBJS **oque = (DG_OBJS **) plugin->queue;
      int num_objs = 0;
      // count visible DG_OBJS
      for (i = 0; i < plugin->n_queue; ++i, ++oque)
      {
         DG_OBJS *objs = *oque;
         if ((objs->flag & visibleChannelFlags) != visibleChannelFlags
            && (objs->flag & (DG_FLAG_OPTCMF & DG_FLAG_PLUGINMASK))
            && !(objs->flag & DG_FLAG_PAINT))
         {
            ++num_objs;
         }
      }
      bufPlug->n_queue = num_objs;
      if (num_objs > 0)
      {
         DG_OBJS **bufObjQueue = (DG_OBJS **)_sceneBufferAlloc(num_objs*sizeof(DG_OBJS *));
         bufPlug->queue = (void **)bufObjQueue;

         oque = (DG_OBJS **) plugin->queue;
         num_objs = 0;
         for (i = 0; i < plugin->n_queue; ++i, ++oque)
         {
            DG_OBJS *objs = *oque;
            if ((objs->flag & visibleChannelFlags) != visibleChannelFlags
               && (objs->flag & (DG_FLAG_OPTCMF & DG_FLAG_PLUGINMASK))
               && !(objs->flag & DG_FLAG_PAINT))
            {
               DG_OBJS *bufObjs = (DG_OBJS *)_sceneBufferAllocCopy(objs, sizeof(DG_OBJS) + objs->n_models*sizeof(DG_OBJ));
               bufObjQueue[num_objs++] = bufObjs;

               bufObjs->extend_data = (FMATRIX *)_sceneBufferAllocCopy(objs->extend_data, 2*sizeof(FMATRIX));

               if (objs->rots != NULL)
               {
                  bufObjs->rots = (FVECTOR *)_sceneBufferAllocCopy(objs->rots, objs->def->n_models*sizeof(FVECTOR));
               }

               // now buffer DG_OBJS
               if (objs->root != NULL)
               {
                  bufObjs->root = (FMATRIX *)_sceneBufferAllocCopy(objs->root, sizeof(FMATRIX));
               }
               bufObjs->light = (FMATRIX *)_sceneBufferAllocCopy(objs->light, 2*sizeof(FMATRIX));
            }
         }
      }
   }
}

/*----------------------------------------------------------------*/
// PLUGINS

static void _bufferPlugins(int objsVisibleMask, int comdlVisibleMask, int patchVisibleMask, int dmapkVisibleMask)
{
   DG_PLUGIN *plugin = DG_AS_GetFirstPlugin();
   DG_PLUGIN *previous_plugin = NULL;

   DG_Plugins_Buf = NULL;
   while (plugin != DG_AS_GetLastPlugin())
   {
      DG_PLUGIN *bufPlug = (DG_PLUGIN *)_sceneBufferAllocCopy(plugin, sizeof(DG_PLUGIN));
      bufPlug->prev = NULL;
      bufPlug->next = NULL;
      if (previous_plugin != NULL)
      {
         previous_plugin->next = bufPlug;
      }
      else
      {
         DG_Plugins_Buf = bufPlug;
      }
      previous_plugin = bufPlug;

      if (plugin->obj_buffer != NULL)
      {
         static int sInitializedIds = FALSE;
         static int skDmapack_id;
         static int skComdl_id;
         static int skPatch_id;
         static int skOptCmf_id;

         if (!sInitializedIds)
         {
            skDmapack_id = GV_StrCode("dmapack");
            skComdl_id   = GV_StrCode("comdl");
            skPatch_id   = GV_StrCode("patch");
            skOptCmf_id  = GV_StrCode("opt_cmf");
            sInitializedIds = TRUE;
         }

         if (plugin->obj_buffer->n_queue > 0)
         {
            bufPlug->obj_buffer = (DG_OBJ_BUFFER *)_sceneBufferAllocCopy(plugin->obj_buffer, sizeof(DG_OBJ_BUFFER));

            // buffer the object list, depending on the plugin type
            if (plugin->object_id == skDmapack_id)
            {
               _bufferPluginDmapacks(bufPlug->obj_buffer, plugin->obj_buffer, dmapkVisibleMask);
               _sceneBufferDebugAllocCheckpoint( "pl dma" );

            }
            else if (plugin->object_id == skComdl_id)
            {
               _bufferPluginComdls(bufPlug->obj_buffer, plugin->obj_buffer, comdlVisibleMask);
               _sceneBufferDebugAllocCheckpoint( "pl comdl" );
            }
            else if (plugin->object_id == skPatch_id)
            {
               _bufferPluginPatch(bufPlug->obj_buffer, plugin->obj_buffer, patchVisibleMask);
               _sceneBufferDebugAllocCheckpoint( "pl patch" );
            }
            else if (plugin->object_id == skOptCmf_id)
            {
               _bufferPluginOptCmf(bufPlug->obj_buffer, plugin->obj_buffer, objsVisibleMask);
               _sceneBufferDebugAllocCheckpoint( "pl opt" );
            }
         }
      }

      plugin = plugin->next;
   }
}

/*----------------------------------------------------------------*/

static void _redirectNextObjs(DG_OBJ_QUEUE *queue)
{
   DG_OBJ_BUFFER *obj_buff = &queue->objs_buffer;

   if ( obj_buff->n_queue != 0 )
   {
      int i;
      DG_OBJS **oque = (DG_OBJS **) obj_buff->queue;
      for (i = 0; i < obj_buff->n_queue; ++i, ++oque)
      {
         DG_OBJS *objs = *oque;

#if BUFFERDRAW_DEBUG_ALLOC_HISTORY
         if ( objs->mASBufferedCopyTimestamp != sSceneBufferTimestamp )
         {
            printf( "DID NOT PROCESS OBJ[%d][%8.8x] buffer %8.8x!!!\n", i, objs, obj_buff );
            HANGUP();
         }
//         printf( "WARNING: BUFFERING OBJS %8.8x NEXT %8.8x FNAMEs:\n %s\n %s\n", 
//            objs, objs->next_dgobjs, objs->fname, objs->next_dgobjs->fname );
#endif


         while (objs->next_dgobjs != NULL && objs->next_dgobjs != objs )
         {
            if (objs->next_dgobjs->mASBufferedCopyTimestamp != sSceneBufferTimestamp)
            {
#if BUFFERDRAW_DEBUG_ALLOC_HISTORY
               printf( "WARNING: BUFFERING %8.8x OBJS %8.8x NEXT %8.8x FNAMEs:\n %s\n %s\n", 
                  obj_buff, objs, objs->next_dgobjs, objs->fname, objs->next_dgobjs->fname );
#endif
               _bufferSingleObjs(objs->next_dgobjs);
            }
            objs->next_dgobjs = objs->next_dgobjs->mASBufferedCopy;
            objs = objs->next_dgobjs;
         }
      }
   }
}


/*----------------------------------------------------------------*/
// PUBLIC INTERFACE

void *DG_AS_SceneBufferAlloc( int size )
{
   return _sceneBufferAlloc( size );
}

void *DG_AS_SceneBufferAllocCopy( void const *src, int size )
{
   return _sceneBufferAllocCopy(src, size);
}

void AS_ResetBufferedScene()
{
   DG_Chanls_Buf = NULL;
}

void AS_BufferSceneNOP( int which )
{
   extern void DG_PreScreen(DG_OBJ_QUEUE *);
   DG_PreScreen(&DG_ObjQueue);

   DG_Chanls_Buf = DG_Chanls;

   DG_Plugins_Buf = DG_AS_GetFirstPlugin();
}

void AS_BufferScene(int which)
{
   int channelIndex;
   int objsVisibleMask = 0;
   int evmsVisibleMask = 0;
   int primVisibleMask = 0;
   int comdlVisibleMask = 0;
   int patchVisibleMask = 0;
   int dmapkVisibleMask = 0;
   int i;
   SBufferContext context;
   DG_OBJ_QUEUE *queue, *bufQueue;
   BP_Debug_PushCPUMarker("BufferChannels");

   _sceneBufferContextInit( &context );

   _sceneBufferInit(which);

   ++sSceneBufferTimestamp;
   if (sSceneBufferTimestamp == 0)
   {
      ++sSceneBufferTimestamp;
   }

   // buffer the object queues
   // first determine which channels are active
   // and make a mask for the objects
   for ( channelIndex = 0; channelIndex < 5; ++channelIndex )
   {
      DG_CHANL *cp = DG_Chanls + channelIndex;

      // in use?
      if (cp->flag != 0)
      {
         objsVisibleMask  |= DG_FLAG_INVISIBLE0    << cp->chanl_num;
         evmsVisibleMask  |= DG_EVMOBJ_INVISIBLE0  << cp->chanl_num;
         primVisibleMask  |= DG_PRIM2_INVISIBLE0   << cp->chanl_num;
         comdlVisibleMask |= DG_COMDL_INVISIBLE0   << cp->chanl_num;
         patchVisibleMask |= DG_PATCH_INVISIBLE0   << cp->chanl_num;
         dmapkVisibleMask |= DG_DMAPACK_INVISIBLE0 << cp->chanl_num;
      }
   }

   queue = &DG_ObjQueue;
   {
      extern void DG_PreScreen(DG_OBJ_QUEUE *);
      DG_PreScreen(&DG_ObjQueue);
   }

   bufQueue = (DG_OBJ_QUEUE *)_sceneBufferAllocCopy(queue, sizeof(DG_OBJ_QUEUE));

   // buffer the channel structures
   DG_Chanls_Buf = (DG_CHANL *)_sceneBufferAllocCopy(DG_Chanls, 5 * sizeof(DG_CHANL));

   for ( channelIndex = 0; channelIndex < 5; ++channelIndex )
   {
      DG_CHANL *cp = DG_Chanls_Buf + channelIndex;
      cp->obj_queue = bufQueue;
   }

   _sceneBufferDebugAllocCheckpoint( "charlie" );

   // Prims are sent to the ULT first followed by OBJ's, since prims will still be executing 
   // while we buffer obj work items, so the threads won't ping-pong or anything
   BP_Debug_PushCPUMarker("Buffer prims");
   {
      SULTParam ult = { (int) bufQueue, (int) queue, primVisibleMask, (int) ( &context ) };

      _sceneBufferPostWork( _bufferPrimsUlt, &ult, sizeof( ult ) );
   }
   BP_Debug_PopCPUMarker();

   _sceneBufferDebugAllocCheckpoint( "prims" );

   // obj's end up threaded
   BP_Debug_PushCPUMarker("Buffer objs");
   _bufferObjs(bufQueue, queue, objsVisibleMask);
   BP_Debug_PopCPUMarker();

   _sceneBufferDebugAllocCheckpoint( "objs" );

   BP_Debug_PushCPUMarker("Buffer evm");
   _bufferEvm(bufQueue, queue, evmsVisibleMask);
   BP_Debug_PopCPUMarker();

   _sceneBufferDebugAllocCheckpoint( "evm" );

   BP_Debug_PushCPUMarker("Buffer comdl");
   _bufferComdl(bufQueue, queue, comdlVisibleMask);
   BP_Debug_PopCPUMarker();

   _sceneBufferDebugAllocCheckpoint( "comdl" );

   // queue->particle_buffer and queue->shdwwrite_buffer unused

   // WE NEED TO SYNC UP ULT BEFORE WE DO PLUGINS
   // (and spots, since that requires the buffered objs)
   BP_Debug_PushCPUMarker("Buffer ult drain");
   BP_Render_HelpDrainUltWork();
   BP_Debug_PopCPUMarker();

   BP_Debug_PushCPUMarker("Buffer spot");
   _bufferSpot(bufQueue, queue);
   BP_Debug_PopCPUMarker();

   _sceneBufferDebugAllocCheckpoint( "spot" );

   // redirect the "next_dgobj" pointers
   _redirectNextObjs(bufQueue);

   // the plugins own the user buffers
   BP_Debug_PushCPUMarker("Buffer plugins");
   _bufferPlugins(objsVisibleMask, comdlVisibleMask, patchVisibleMask, dmapkVisibleMask);
   BP_Debug_PopCPUMarker();

   _sceneBufferDebugAllocCheckpoint( "plugins" );

   // for info
   {
      if (sSceneBufferCurOffset > gBP_SceneBufferPeakSize)
      {
         gBP_SceneBufferPeakSize = sSceneBufferCurOffset;
         if (GM_GetArea() != NULL)
         {
            strcpy(gBP_SceneBufferPeakArea, GM_GetArea());
         }
         // printf takes a millisecond!
         printf("Scene buffer peak = %d\n", sSceneBufferCurOffset);
      }
   }
   BP_Debug_PopCPUMarker();
}
