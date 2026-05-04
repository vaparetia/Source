//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   put_elev.c 
   昇降機

   2001/01/16 T.Morita
   $Id: elv_fence.c,v 1.1.1.3 2002/11/19 11:46:30 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef PSX2
#include <libgraph.h>
#include <libvu0.h>
#include <libdma.h>
#endif

#include  "gameheader.h"
#include  "libutl.h"

#include "../include/libdg_x.h"
#include "../include/util.h"
#include "../brk_utl/brk_utl.x"


enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;


enum
{
    PUT_FENCE_OPEN =0x0001,
    PUT_FENCE_CLOSE=0x0002,
} ;

typedef void (* FUNC)( int ) ;

typedef struct work_t
{
    GV_ACT_EX actor ;

    FVECTOR   top    ;
    FVECTOR   bottom ;
    float     vel    ;
    DG_OBJS  *fence  ;

    int       name   ;
    int       flag   ;
    FUNC      callback ;

    DG_TEX    *tex    ;
    DG_PRIM2  *shadow ;

    HZX_D_SEGMENT *segs ; //for door
} Work ;

#define DEGtoANG(_a) ((_a)*8192/45)
#define ANGtoDEG(_a) ((int)(_a)*180/32768)
#define ANGtoRAD(_a) ((_a)*(float)M_PI/32768.0f*16.0f)

static FVECTOR SePos = { 0.0f, 5000.0f, -10000.0f, 1.0f } ;


static void RotateTransMatrix( FMATRIX *mtx, FVECTOR *pos, short rot[] )
{
    _sceVu0RotMatrixY( mtx, &DG_UnitMatrix, ANGtoRAD(rot[Y]) ) ;
    _sceVu0RotMatrixX( mtx, mtx           , ANGtoRAD(rot[X]) ) ;
    _sceVu0RotMatrixZ( mtx, mtx           , ANGtoRAD(rot[Z]) ) ;
    _sceVu0CopyVectorXYZ( (FVECTOR*)mtx->m[W], pos ) ;
}

static inline void ActRecieveMessage( Work *work )
{
    GV_MSG *msg ;
    int i ;

//SD_A_EV_RMOV1,	//エレベータ屋上柵動く//ev_rmov1 1259

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
	switch( msg->message[0] )
	{
	case 0:
	    GM_SeSetMode( SD_A_EV_RMOV1,
			  &SePos,
			  GM_SEMODE_NORMAL ) ;
	    printf( "open elv_fence.c  : NewPutElevatorFence\n" ) ;
	    work->flag |=  PUT_FENCE_OPEN  ;
	    work->flag &= ~PUT_FENCE_CLOSE ;
	    break ;
	case 1:
	    GM_SeSetMode( SD_A_EV_RMOV1,
			  &SePos,
			  GM_SEMODE_NORMAL ) ;
	    printf( "close elv_fence.c  : NewPutElevatorFence\n" ) ;
	    work->flag |=  PUT_FENCE_CLOSE ;
	    work->flag &= ~PUT_FENCE_OPEN  ;
	    work->segs->atr &= ~HZX_SEG_SKIP ;
	    break ;
	default:
	    printf( "mesg %d elv_fence.c  : NewPutElevatorFence\n", msg->message[0] ) ;
	}
}

#define FTOI12(_f)      ( DG_FTOI( ( (float)(_f)*4096.0f) ) )

/* 影を動かす */
static void ActMoveShadow( Work *work )
{
    DG_PRIM2_UVRGB *u ;
    FVECTOR        *p ;
    float           t ;
    FVECTOR         v ;
    static FVECTOR Shape[] = {
#if 0
	{ -2000.0f, 5000.0f, -10450.0f,  1.0f } ,
	{ -2280.0f, 5000.0f, -10450.0f,  1.0f } ,
	{  2000.0f, 5000.0f, -10450.0f,  1.0f } ,
	{  2000.0f, 5000.0f, -10450.0f,  1.0f } ,

	{ -2280.0f, 5000.0f, -10450.0f,  1.0f } ,
	{ -2280.0f, 5000.0f, -10000.0f,  1.0f } ,
	{  2000.0f, 5000.0f, -10450.0f,  1.0f } ,
	{  2000.0f, 5000.0f, -10000.0f,  1.0f } ,
#else
	{ -2000.0f, 5000.0f, -10500.0f,  1.0f } ,
	{ -2280.0f, 5000.0f, -10500.0f,  1.0f } ,
	{  2000.0f, 5000.0f, -10500.0f,  1.0f } ,
	{  2000.0f, 5000.0f, -10500.0f,  1.0f } ,

	{ -2250.0f, 5000.0f, -10550.0f,  1.0f } ,
	{ -2250.0f, 5000.0f, -10000.0f,  1.0f } ,
	{  2000.0f, 5000.0f, -10550.0f,  1.0f } ,
	{  2000.0f, 5000.0f, -10000.0f,  1.0f } ,
#endif
    } ;
    DG_TEX *tx ;
    int i  ;
    
    tx = work->tex ;
    t  = (work->fence->world.m[W][Y] - work->bottom.vy) ;
    t /= (work->top.vy - work->bottom.vy) ;

    DG_SwitchBuffPrim2( work->shadow ) ;
    p = work->shadow->pos  [work->shadow->buffer_clock] ;
    u = work->shadow->uvrgb[work->shadow->buffer_clock] ;

    (u+0)->v = 
    (u+1)->v = FTOI12( (1.0f-t) * tx->v_scale + tx->v_offset ) ;

    for( i=0 ; i<4 ; i++ )
    {
	_sceVu0ScaleVector( p+i, Shape+i*2+1,      t ) ;
	_sceVu0ScaleVector( &v , Shape+i*2  , 1.0f-t ) ;
	_sceVu0AddVector( p+i, p+i, &v ) ;
    }
}

/* ハザードを動かす */
static inline void ActMoveHzx( Work *work, float vely )
{
    IVECTOR shift = { 0, (int)vely, 0, 0} ;

    HZX_ShiftDynamicSegment( work->segs, &shift ) ;
}


static void ActMoveDoor( Work *work, float det, float sig )
{
    if ( work->fence->world.m[W][Y] > det + 100.0f*sig )
	work->vel -= 3.0f*sig ;
    else
	work->vel += 1.0f*sig ;
    work->vel = (work->vel*sig<-50.0f*sig ?-50.0f*sig :
		 work->vel*sig> -1.0f*sig ? -1.0f*sig : work->vel) ;
    work->fence->world.m[W][Y] += work->vel ;
    
    if ( work->fence->world.m[W][Y]*sig < det*sig )
    {
	GM_SeSetMode( sig>0.0f ? SD_A_EV_RSAK1 : SD_A_EV_RSAK2,
		      &SePos,
		      GM_SEMODE_NORMAL ) ;

	if ( sig>0.0f )
 	    work->segs->atr |=  HZX_SEG_SKIP ;
	else
 	    work->segs->atr &= ~HZX_SEG_SKIP ;

	work->fence->world.m[W][Y] = det ;
	work->flag = 0 ;
	if ( work->callback )
	    (*work->callback)( (int)-sig ) ;
    }
    ActMoveHzx( work, work->vel ) ;
}


/* エレベータ全処理 */
static void Act( Work *work )
{
    /* メッセージ受取 */
    ActRecieveMessage( work ) ;

#if 0
    if ( GV_PadData[0].press & PAD_L1 )
	work->flag |= work->fence->world.m[W][Y]==work->bottom.vy ? 
	    PUT_FENCE_CLOSE : PUT_FENCE_OPEN ;
    //HZX_ViewDynamicSegment( work->segs ) ;
#endif

    if ( work->flag & (PUT_FENCE_OPEN|PUT_FENCE_CLOSE) )
	ActMoveShadow( work ) ;

    /*降下*/
    if ( work->flag & PUT_FENCE_OPEN )
	ActMoveDoor( work, work->bottom.vy, 1.0f ) ;
    /*上昇*/
    else if ( work->flag & PUT_FENCE_CLOSE )
	ActMoveDoor( work, work->top.vy, -1.0f ) ;
}

static void Die( Work *work )
{
    /* エレベータモデルを解放 */
    if ( work->fence )
    {
	DG_DequeueObjs( work->fence ) ;
	DG_FreeObjs( work->fence ) ;
    }

    /* 影を解放 */
    if ( work->shadow )
	GM_FreePrim2( work->shadow ) ;

    /* ダイナミックハザードを解放 */
    if ( work->segs )
	HZX_RemoveDynamicSegment( work->segs ) ;
}


static HZX_D_SEGMENT *MakeHzxFromOBJS( Work *work, DG_OBJS *objs, int where )
{
    IVECTOR        p[4]   ;
    HZX_GROUP_ID   hzx_id ;
    FVECTOR        uv = { objs->def->ux, objs->def->uy, objs->def->uz, 0.0f } ;
    FVECTOR        lv = { objs->def->lx, objs->def->ly, objs->def->lz, 0.0f } ;
    FVECTOR        size, offset ;
    FVECTOR        v ;
    HZX_D_SEGMENT *segs ;

    ASSERT( where ) ;

    /* モデルからサイズを取得 */
    _sceVu0SubVector  ( &size  , &uv, &lv ) ;
    _sceVu0ScaleVector( &size  , &size  , 0.5f ) ;
    size.vw = 0.0f ;
    _sceVu0AddVector  ( &offset, &uv, &lv ) ;
    _sceVu0ScaleVector( &offset, &offset, 0.5f ) ;
    offset.vx += objs->def->tx ;
    offset.vy += objs->def->ty ;
    offset.vz += objs->def->tz ;
    offset.vw  = 1.0f ;

    /* エレベータの周りに壁を貼る */
    hzx_id = GM_GetHzxGroupID( where ) ;
    v.vx = offset.vx - size.vx ;
    v.vy = offset.vy - size.vy ;
    v.vz = offset.vz - size.vz ;
    v.vw = 1.0f ;
    _sceVu0ApplyMatrix( &v, &objs->world, &v ) ;
    v.vw = 1200.0f ;
    _sceVu0FTOI0Vector( &p[0], &v ) ;

    v.vx = offset.vx + size.vx ;
    v.vy = offset.vy - size.vy ;
    v.vz = offset.vz - size.vz ;
    v.vw = 1.0f ;
    _sceVu0ApplyMatrix( &v, &objs->world, &v ) ;
    v.vw = 1200.0f ;
    _sceVu0FTOI0Vector( &p[1], &v ) ;

    segs = HZX_AddDynamicSegment( hzx_id, &p[0], &p[1], 
					   HZX_SEG_NO_BULLETHOLE|
					   HZX_SEG_NO_RECOIL ) ;
    /*レーダーに映すため*/
    if ( segs )
	segs->atr &= ~HZX_SEG_NO_DISP_RADAR ;
    if ( work->flag & 0x0080 )
	segs->atr |= HZX_SEG_SKIP ;

    return segs ;
}

static int MakeShadow( Work *work )
{
    static FVECTOR Shape[] = {
	{ -2250.0f, 5000.0f, -10500.0f,  1.0f } ,
	{  2000.0f, 5000.0f, -10500.0f,  1.0f } ,
	{ -2250.0f, 5000.0f, -10000.0f,  1.0f } ,
	{  2000.0f, 5000.0f, -10000.0f,  1.0f } ,
    } ;
    if ( !(work->shadow = BRK_UTL_MakePOLY( 1,
					    6171017, /*w12c_sdw05d_alp.bmp*/
					    SCE_GS_SET_ALPHA(0,1,0,1,0),
					    (80<<24)|0x003f3f3f )) )
	PERROR( "Prim failed. No memory : NewPutElevatorFence\n" ) ;

    work->tex = DG_GetTexture( 6171017 ) ;

    memcpy( work->shadow->pos[0]+0, Shape+0, sizeof(FVECTOR)*2 ) ;
    memcpy( work->shadow->pos[1]+0, Shape+0, sizeof(FVECTOR)*2 ) ;
    if ( work->flag & 0x0080 )
    {
	memcpy( work->shadow->pos[0]+2, Shape, sizeof(FVECTOR)*2 ) ;
	memcpy( work->shadow->pos[1]+2, Shape, sizeof(FVECTOR)*2 ) ;
    }
    else
    {
	memcpy( work->shadow->pos[0]+2, Shape+2, sizeof(FVECTOR)*2 ) ;
	memcpy( work->shadow->pos[1]+2, Shape+2, sizeof(FVECTOR)*2 ) ;
    }
    return  0 ;
}

static int GetResourcesCalled( Work *work, int name, int where,
			       FVECTOR *top, FVECTOR *bottom, SVECTOR *rot,
			       int flag, void *callback )
{
    DG_DEF  *def ;
    FVECTOR *e_pos ;

    work->name    = name     ;
    work->callback= callback ;
    work->flag    = flag & ~(PUT_FENCE_OPEN|PUT_FENCE_CLOSE) ;

    /* 昇降機モデル */
    if ( !(def = GV_GetCache( GV_CacheID( 4292763/* w12c_elvsk01 */, 'k' ) )) )
	PERROR( "No Fence Model<w12c_elvsk01.kms> : NewPutElevatorFence\n" ) ;
    if ( !( work->fence = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 )) )
	PERROR( "Cannot make OBJS(maybe no memory) : NewPutElevatorFence\n" ) ;
    DG_QueueObjs( work->fence ) ;

    _sceVu0CopyVector( &work->top   , top    ) ;
    _sceVu0CopyVector( &work->bottom, bottom ) ;
    e_pos = flag&0x0080 ? &work->bottom : &work->top ;

    RotateTransMatrix( &work->fence->world, e_pos, (short *)rot ) ;
    work->vel = 0.0f ;

    if ( !(work->segs = MakeHzxFromOBJS( work, work->fence, where )) )
	PERROR( "Cannot make DynamicSegment : NewPutElevatorFence\n" ) ;

    if ( MakeShadow( work ) )
	return -1 ;
    
    return 0 ;
}

void *NewPutElevatorFence( int name, int where,
			   FVECTOR *top, FVECTOR *bottom, SVECTOR *rot,
			   int flag, void *callback )
{
    /* エレベータは一つしか上がらない */
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
        if ( GetResourcesCalled( work, name, where,
				 top, bottom, rot,
				 flag, callback ) < 0 )
	{
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
