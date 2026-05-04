//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   put_vanime.c
   プット頂点アニメ

   1999/12/13 T. Morita
   $Id: put_stanime.c,v 1.4 2002/11/23 12:28:38 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

// for DG-LIB
#include "../include/libdg_x.h"
#include "../include/util.h"

//BP - endian functions.
#include "BP_EndianSupport.h"


/* Enumuration */
enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

enum
{
    PUT_ANM_PLAY   = 0x0001,
    PUT_ANM_REWIND = 0x0002,
} ;

typedef struct _put_stanime_Work
{
    GV_ACT_EX    actor  ;

    int          map    ;
    int          name   ;
    CV2_DEF     *cdef   ;
    FMATRIX   lights[2] ;
    DG_VERTS_ANIME v_anm ;

    short        ratio   ;
    short        tgl     ;
    short        BP_tgl  ; // BP: we use this to track which buffer to byte swap
    int          rate    ;
    int          n_verts ;
    int         *speed   ;
    int          mode   ;

    FMATRIX     *root   ;
    FMATRIX      world  ;
    DG_OBJS     *objs   ;
    DG_OBJS     *objs_r ;

    float        scale  ;
    short       *buffer[2] ; /* 解凍バッファ   */
    void        *lzh       ; /* 解凍ハンドラー */
} Work ;

#define RATE   (4096)
#define H_RATE (RATE/2)

extern int DM_FrameSkip ; /* デモ用のフレームスキップ */



static void Die( Work *work )
{
    if ( work->v_anm.mem[0] )
	DG_FreeAnimVertsBuffer( &work->v_anm ) ;
    UTL_LzshDecodeFree( work->lzh ) ;
    if ( work->buffer[0] )
	GV_Free( work->buffer[0] ) ;
    if ( work->objs )
	DG_DequeueObjs( work->objs ), DG_FreeObjs( work->objs );
}

static void MakeupNormal( short *a, short *b, int i, int ratio, float scale )
{
    SVECTOR *svec = (SVECTOR*)SCRPAD_ADDR ;

    scale /= RATE ;
    while( --i>=0 )
    {
        svec->vx  = (short)((a[X]*ratio + b[X]*(RATE-ratio)) * scale) ;
        svec->vy  = (short)((a[Y]*ratio + b[Y]*(RATE-ratio)) * scale) ;
        svec->vz  = (short)((a[Z]*ratio + b[Z]*(RATE-ratio)) * scale) ;
#ifdef XBOX
		svec->vx *= 16 ;
		svec->vy *= 16 ;
		svec->vz *= 16 ;
#endif
#ifndef KP_WINDOWS
        svec->pad = 4096 ;
#else
        svec->pad = 0x7fff ;
#endif
        svec++ ;

        a += XYZ ;
        b += XYZ ;
    }
}

static void MakeupShapeEX( short *a, short *b, int i, int ratio, DG_OBJS *objs )
{
    SVECTOR *svec = (SVECTOR*)SCRPAD_ADDR ;
    FVECTOR min={ 1e+32f, 1e+32f, 1e+32f, 0.0f } ;
    FVECTOR max={-1e+32f,-1e+32f,-1e+32f, 0.0f } ;
    
    while( --i>=0 )
    {
        svec->vx  = (a[X]*ratio + b[X]*(RATE-ratio))/RATE ;
        svec->vy  = (a[Y]*ratio + b[Y]*(RATE-ratio))/RATE ;
        svec->vz  = (a[Z]*ratio + b[Z]*(RATE-ratio))/RATE ;
#ifndef KP_WINDOWS
        svec->pad = 4096 ;
#else
        svec->pad = 0x7fff ;
#endif
	
	min.vx = DG_MIN( min.vx, (float)svec->vx ) ;
	min.vy = DG_MIN( min.vy, (float)svec->vy ) ;
	min.vz = DG_MIN( min.vz, (float)svec->vz ) ;
	max.vx = DG_MAX( max.vx, (float)svec->vx ) ;
	max.vy = DG_MAX( max.vy, (float)svec->vy ) ;
	max.vz = DG_MAX( max.vz, (float)svec->vz ) ;

        svec++ ;

        a += XYZ ;
        b += XYZ ;
    }

    _sceVu0CopyVector( &objs->objs->bound_min, &min ) ;
    _sceVu0CopyVector( &objs->objs->bound_max, &max ) ;
    _sceVu0CopyVector( &objs->bound_min, &min ) ;
    _sceVu0CopyVector( &objs->bound_max, &max ) ;
}

void ORG_SendPutAnimMessage( int name, int buffer )
{
    GV_MSG msg ;

    msg.address = name ;
    msg.message = &buffer ;
    msg.message_len = sizeof(buffer)/sizeof(int) ;
    GV_SendMessage( &msg ) ;
}

static int DecodeStreamToBuffer( Work *work )
{
    int ret ;

    ret = UTL_LzshStreamDecode( work->lzh ) ;
    if ( !ret )
    {
	if ( work->mode & PUT_ANM_REWIND )
	    UTL_LzshDecodeRewindIn( work->lzh ) ;
	else
	    work->mode &= ~PUT_ANM_PLAY ;
    }
       
    {
       // BP - swap bytes for endian correctness.
       // Short array of both vertex and normal data
       BP_LE_SwapSShortArray_Inp( work->buffer[ work->BP_tgl ], 2 * work->n_verts * XYZ );
       work->BP_tgl ^= 1;
    }

    return ret ;
}


static void Act( Work *work )
{
    GV_MSG *msg ;
    int i ;
    
    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
    {
        switch( msg->message[0] )
        {
        case 0:
	    work->mode &= ~PUT_ANM_PLAY ;
            break ;
        case 1:
	    work->mode |=  PUT_ANM_PLAY ;
            break ;
        case 2:
	    GV_DestroyActor( work ) ;
            break ;
	case 3:
           work->rate = H_RATE / ( msg->message[1] <= 0      ? 1      :
				   msg->message[1] >  H_RATE ? H_RATE : msg->message[1] ) * TIME_BASE / 5 ;
	   break ;
	case 4:
	    work->rate = ( msg->message[1] <= 0    ? 1    :
			  msg->message[1] >  RATE ? RATE : msg->message[1] ) * TIME_BASE / 5 ;

	   break ;
        }
    }

    if ( work->mode & PUT_ANM_PLAY )
    {
        if ( GM_GameStatus & STATE_DEMO )
	    work->ratio += work->rate*DM_FrameSkip ;

	if ( (work->ratio += work->rate) >= RATE )
	{
	    while( work->ratio >= RATE )
	    {
		work->ratio -= RATE ;
		
		if ( !DecodeStreamToBuffer( work ) )
		    break ;
		if ( work->tgl ^= 1 )
		    UTL_LzshDecodeRewindOut( work->lzh ) ;
	    }
	}

	DG_SwitchVAnimeBuffer( &work->v_anm ) ;
	MakeupShapeEX( work->buffer[work->tgl  ],
		       work->buffer[work->tgl^1],
		       work->n_verts, work->ratio, work->objs ) ;
	DG_RefineStripVertex( &work->v_anm, work->cdef->models[0].verts_index ) ; 
	MakeupNormal( work->buffer[work->tgl  ] + work->n_verts*XYZ,
		     work->buffer[work->tgl^1] + work->n_verts*XYZ,
		     work->n_verts, work->ratio, work->scale ) ;
	DG_RefineStripNormal( &work->v_anm, work->cdef->models[0].verts_index ) ; 
    }

    if ( work->root )
	_sceVu0MulMatrix( &work->objs->world, work->root, &work->world ) ;
}

static int InitAnimeModel( Work *work, int id, FMATRIX *mtx, FVECTOR *bound_u, FVECTOR *bound_l )
{
    DG_DEF  *def ;

    if ( !bound_l || !bound_u )
        PERROR( "Neither bound_l or bound_u(Maybe -b option missing) ::NewPutSTanimeObject\n" ) ;

    if ( !(def = GV_GetCache( GV_CacheID(id, 'k') )) )
        PERROR( "No KMS-DG_DEF!! :NewPutSTanimeObject\n" ) ;

    if ( !(work->objs = DG_MakeObjs( def, DG_FLAG_SHADE| DG_FLAG_ONEPIECE, 0 )) )
        PERROR( "Can't make DG_OBJS(Maybe no memory) :NewPutSTanimeObject\n" ) ;
    DG_QueueObjs( work->objs ) ;
    if ( !(work->cdef = GV_GetCache( GV_CacheID(id, 'c') ) ) )
        PERROR( "No Cv2-CV2_DEF!! :NewPutSTanimeObject\n" ) ;
    DG_MakeAnimVertsBuffer( &work->v_anm, work->objs->objs, DG_VANIME_VERTS|DG_VANIME_NORMS ) ;
    work->n_verts = work->cdef->models[0].n_verts ;

    work->objs->world = work->world = *mtx ;
    DG_SetLightMatrix( work->objs, work->lights ) ;

    return 0 ;
}

static int InitAnimation( Work *work, int id, int rate )
{
    int  size ;

    /* フレームコントロール初期化 */
    work->rate  = rate ;
    work->ratio = RATE ;
    work->tgl   = 0 ;
    work->BP_tgl = 0 ;
    work->mode |= PUT_ANM_PLAY ;

    /* アニメーション初期化 */
    size = sizeof(short) * XYZ * work->n_verts ;
    work->buffer[0] = GV_Malloc( size * 4 ) ;
    work->buffer[1] = work->buffer[0] + work->n_verts * XYZ * 2 ;
    if ( !work->buffer[0] )
        PERROR( "Can't get buffer(No memory)!! :NewPutSTanimeObject\n" ) ;

    work->lzh = UTL_LzshDecodeInit( GV_GetCache( GV_CacheID( id, 'a') ), work->buffer[0], size*2 ) ;
    if ( !work->lzh )
        PERROR( "Can't initialize LZSH handler!! :NewPutSTanimeObject\n" ) ;

    /* アニメーション最初のフレーム分を解凍（ダブルバッファのため） */
    DecodeStreamToBuffer( work ) ;

    return  0 ;
}


static int GetResourcesCall( Work *work, int name, int model,
			     DG_OBJ *obj, FVECTOR *pos, SVECTOR *rot, FVECTOR *scale,
			     FVECTOR *bound_u, FVECTOR *bound_l, int mode )
{
    FMATRIX mtx = DG_UnitMatrix ;
    int r ;

    work->scale = 1.0f ;
    if ( scale )
    {
	mtx.m[0][0] *= scale->vx ;
	mtx.m[1][1] *= scale->vy ;
	mtx.m[2][2] *= scale->vz ;
	work->scale = DG_MAX( scale->vx, scale->vy ) ;
	work->scale = DG_MAX( work->scale, scale->vz ) ;
	work->scale = 1.0f / work->scale ; 
    }
    if ( rot )
    {
   // BP See ByteSwap code in DM_Packet_Effect in demo_eft.c, it has byteswapped all of DEMO_EFFECT::data as ints
   // this is most correct, except for this case rot is an SVECTOR...
   // this case happens in d12t2 when Occelot throws his coat
   SVECTOR byteSwapRot = *rot;
   //BP_LE_SwapSIntArray_Inp(&byteSwapRot, 2);
   //BP_LE_SwapSShortArray_Inp(&byteSwapRot, 4);

	r = byteSwapRot.vx & 0x0fff ;
	_sceVu0RotMatrixX( &mtx, &mtx, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
	r = byteSwapRot.vy & 0x0fff ;
	_sceVu0RotMatrixY( &mtx, &mtx, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
	r = byteSwapRot.vz & 0x0fff ;
	_sceVu0RotMatrixZ( &mtx, &mtx, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    }
    if ( pos )
    {
	_sceVu0CopyVector( (FVECTOR *)&mtx.m[3], pos ) ;
	mtx.m[3][3] = 1.0f ;
	DG_GetLightMatrixFix( (FVECTOR *)&mtx.m[W], work->lights ) ;
    }
    else
	PERROR( "No position :NewPutSTanimeObject\n" ) ;
    work->name  = name  ;
    work->mode  = mode  ;
    if ( !model )
	PERROR( "No model ID!! :NewPutSTanimeObject\n" ) ;
    if ( InitAnimeModel( work, model, &mtx, bound_u, bound_l ) < 0 )
	return -1 ;
    if ( InitAnimation( work, model, H_RATE * TIME_BASE / 5 /*work->rateに入る*/ ) < 0 )
	return -1 ;

printf( "OBJ%x\n", obj ) ;
    if ( obj )
	_sceVu0MulMatrix( &work->objs->world, (work->root = &obj->world), &work->world ) ;
    else
	work->root = NULL ;
    return 0 ;
}


static int GetResources( Work *work, int name, int where )
{
    int      id ;
    IVECTOR buf ;
    FVECTOR bound_u, bound_l ;
    FVECTOR scale, pos ;
    SVECTOR rot ;

    if ( GCL_GetOption( 'r' ) )
	GCL_GetSV( GCL_NextStr(), (short*)&rot ) ;
    if ( GCL_GetOption( 's' ) )
    {
        GCL_GetIV( GCL_NextStr(), (int *)&buf ) ;
        vu0_IV0toFV( &buf, &scale ) ;
    }
    if ( GCL_GetOption( 'p' ) )
    {
        GCL_GetIV( GCL_NextStr(), (int *)&buf ) ;
        vu0_IV0toFV( &buf, &pos ) ;
    }
    else
	PERROR( "No position(-p option missing)!! :NewPutSTanimeObject\n" ) ;
    if ( GCL_GetOption( 'b' ) )
    {
        GCL_GetIV( GCL_NextStr(), (int *)&buf ) ;
        vu0_IV0toFV( &buf, &bound_u ) ;
        GCL_GetIV( GCL_NextStr(), (int *)&buf ) ;
        vu0_IV0toFV( &buf, &bound_l ) ;
    }
    else
	PERROR( "No Boundary(-b option missing)!! :NewPutSTanimeObject\n" ) ;

    if ( !(id = GCL_GetOptionValue( 'm', 0 )) )
	PERROR( "No model ID(-m option missing)!! :NewPutSTanimeObject\n" ) ;

    if( GetResourcesCall( work, name, id, NULL, &pos, &rot, &scale, &bound_u, &bound_l, 0 ) )
	PERROR( "Cannot get resources :NewPutSTanimeObject\n" ) ;

    return 0 ;
}

void *NewPutSTanimeObjectCall( int name, int model, DG_OBJ *obj,
			       FVECTOR *pos, SVECTOR *rot, FVECTOR *scale,
			       FVECTOR *bound_u, FVECTOR *bound_l, int mode )
{
    Work *work ;
    
    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &(work->actor), Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
        if( GetResourcesCall( work, name, model, obj, pos, rot, scale, bound_u, bound_l, mode ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}


void *NewPutSTanimeObject( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
        if( GetResources( work, name, where ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
