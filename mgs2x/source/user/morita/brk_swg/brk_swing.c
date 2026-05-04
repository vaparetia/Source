//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_swing.c
   揺れ落下

   1999/12/13 T. Morita
   $Id: brk_swing.c,v 1.1.1.3 2002/11/19 11:45:47 Yoshizawa1 Exp $
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


enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

// for DG-LIB
#include "../include/libdg_x.h"
#include "../brk_utl/brk_utl.x"
#include "../include/util.h"


#define DEGtoANG(_a) ((_a)*8192/45)
#define ANGtoDEG(_a) ((int)(_a)*180/32768)
#define ANGtoRAD(_a) ((_a)*(float)M_PI/32768.0f)

#define BRK_VEL_R 16
#define BRK_ROT_R 16

#define BRK_SWG_INACTIVE   0x8000000
#define BRK_SWG_MOVE_RATE  0.2f
#define BRK_SWG_MOVE_FORCE 10.0f
#define BRK_SEG_N_SEGS 4
#define BRK_SEG_N_FLRS 2

#define HZX_OFF_FRAME (24*5/TIME_BASE)
#define GRAVITY       11.0f

typedef struct work_t  Work ;
struct work_t
{
    GV_ACT   actor   ;

    TARGET       target ;
    POWER_TARGET power  ;

    FVECTOR   pos   ; /* 位置     */
    float     pos_vy ;
    short     rot_x,  rot_y,  rot_z  ;/*現在の角度*/
    short     rot_vx, rot_vy, rot_vz ;/*角速度  */
    short     rot_dx, rot_dy, rot_dz ;/*目標角度*/

    float     rate  ;
    int       name  ;
    int       where ;
    int       proc  ;
    short     splash ;
    short     life  ;
    short     mode  ;
    DG_OBJS  *objs  ;

    int       time  ;

    HZX_D_SEGMENT *segs[BRK_SEG_N_SEGS] ;
    HZX_D_FLOOR   *flrs[BRK_SEG_N_FLRS] ;
} ;


#if 0
FVECTOR BRK_SWG_Wall[] = {
    { -1450,-4000,-2000, 1.0f },  { -700,-4000, 4000, 1.0f },
    {     0,-4000, 1100, 1.0f },  {  550,-4000, 4000, 1.0f },
    { -1450,-4000,    0, 1.0f },  {    0,-4000,    0, 1.0f },
    { -1450,-3900, 3450, 1.0f },  { 1060,-3400, 3550, 1.0f },
} ;

float BRK_SWG_WallHeight[] = {
    6000.0f,6000.0f,    4000.0f,4000.0f,
    6000.0f,6000.0f,    2000.0f,2000.0f
} ;

#else

FVECTOR BRK_SWG_Wall[] = {
    { -4250,-6000,-150000, 6000.0f },  {-4250,-6000,-146500, 6000.0f },
    { -3000,-6000,-148750, 6000.0f },  {-3000,-6000,-146500, 6000.0f },
    { -4250,-6000,-150000, 6000.0f },  {-3000,-6000,-150000, 6000.0f },
    { -4250,-6000,-146500, 4000.0f },  {-3000,-6000,-146500, 4000.0f },
} ;

#endif

FVECTOR BRK_SWG_Floor[] = {
    { -1450, -200,     0, 1.0f },/* 階段踊り場 */
    { -1450, -450,  1250, 1.0f },
    {   660, -100,  1250, 1.0f },
    {   660,    0,     0, 1.0f },

    { -1450, -450,  1250, 1.0f },/* スロープ */
    { -1450,-3800,  3450, 1.0f },
    {  1060,-3300,  3550, 1.0f },
    {   660, -100,  1250, 1.0f },
} ;

static FVECTOR BRK_SWG_SplashPos[] = {
    { -1000, -200,    0, 1.0f },
    { - 500,    0,    0, 1.0f },
    { - 200,-3800, 3450, 1.0f },
    { - 800,-2962, 2900, 1.0f },
    { -   0,-2125, 2350, 1.0f },
    { - 900,-1112, 1800, 1.0f },
    { - 600, -100, 1250, 1.0f },
} ;


extern void AN_Test_Eye3( FVECTOR *mov ) ; /* for test */

static inline void MakeObjWorld( Work *work )
{
    FMATRIX *mtx = &work->objs->world ;

    _sceVu0RotMatrixY( mtx, &DG_UnitMatrix, ANGtoRAD(work->rot_y) ) ;
    _sceVu0RotMatrixX( mtx,  mtx          , ANGtoRAD(work->rot_x) ) ;
    _sceVu0RotMatrixZ( mtx,  mtx          , ANGtoRAD(work->rot_z) ) ;
    _sceVu0CopyVectorXYZ( (FVECTOR *)mtx->m[W], &work->pos ) ;
    GM_MoveTarget2Map( &work->target, mtx, work->where ) ;
}


static void CallSpark( Work *work, TARGET *def, TARGET *off, FVECTOR *pos )
{
    FMATRIX mat ;
    FVECTOR v   ;
    extern void *NewSpark( FMATRIX *world ) ;

    /* 跳弾を出す */
    /* 跳弾の方向計算 */
    _sceVu0ApplyMatrix( &v, &work->objs->world, pos ) ;
    UTL_VectoMat( &mat, &off->power->force, &v, Y ) ;
    GM_CurrentMap = work->where ;
    NewSpark( &mat ) ;

    GM_SeSetMode( SD_A_OTAMA_DS, &v, GM_SEMODE_BOMB ) ;

    /* この音を聞いて敵兵が寄る */
    GM_SetNoise( NOISE_S, &v, work->where ) ;
}

static int Is_PlayerOn( Work*work )
{
#if 0
    if ( GM_PlayerControl->level[0].attribute & HZX_FLOOR_DYNAMIC &&
	 (GM_PlayerControl->level[0].ptr == work->flrs[0] ||
	  GM_PlayerControl->level[0].ptr == work->flrs[1] )
	 return 1 ;
#else
    /* プレーヤーが乗っている */
    if ( GM_PlayerControl->mov.vx >
	 work->pos.vx + work->objs->def->lx &&
	 GM_PlayerControl->mov.vx <
	 work->pos.vx + work->objs->def->ux &&
	 GM_PlayerControl->mov.vz >
	 work->pos.vz + work->objs->def->lz &&
	 GM_PlayerControl->mov.vz <
	 work->pos.vz + work->objs->def->uz )
	return 1 ; 
#endif
    return  0 ;
}

static void BreakSwing( Work *work, FVECTOR *hit, FVECTOR *frc, int damage )
{
#if 0
    /*  */
    {
	extern void *NewFallFragile( FVECTOR *pos, int n_piece,
				     int life, float width, int interval ) ;

	NewFallFragile( &offs, 16, 180, 80.0f, 1 ) ;
    }
#endif

    if ( (work->life -= damage) < 0 )
    {
	work->rot_vx = (short)(-frc->vz * work->rate*0.5f) ;
	work->rot_vz = (short)( frc->vx * work->rate*0.5f) ;
	work->mode = HZX_OFF_FRAME ;

	/* 落ち音を呼ぶ */
	GM_SeSetMode( SD_A_KAIDANFA, &work->pos, GM_SEMODE_BOMB ) ;
    }
    else
    {
	/* 動かす */
	work->mode = 1 ;
	work->rot_vx = (short)(-frc->vz * work->rate) ;
	work->rot_vz = (short)( frc->vx * work->rate) ;

	if ( Is_PlayerOn( work ) )
	    work->rot_vx /= 4, work->rot_vz /= 4 ;
	return ;
    }
}



void BRK_SWG_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work *work = (Work *)ptr ;
    int         i, j ;
    int         flag ; 
    SVECTOR    *s, *n ;
    DG_MDLPACK *m ;
    FMATRIX     inv ;
    FVECTOR     offs, frc ;
    FVECTOR     hit ;
    float       max ;

    if ( def->damaged & TARGET_POWER )
    {
	/* clear damage */
	GM_ClearTargetDamage( def ) ;

	if ( GM_StagePlayTime - work->time < 4 )
	    return ;
	work->time = GM_StagePlayTime ;

	/* 武器によってダメージは違う */
	if ( off->weapon_type & (WP_BULLET|WP_M92) )
	{
		DG_OBJ         *obj  = work->objs->objs ;
#ifdef KP_XBOX
		DG_VERTEX_KMSS *kmss = obj->vbuff ;
#endif
	    /*ローカル座標系での着弾位置と着弾方向を求める*/
	    _sceVu0CopyVector( &frc , &off->power->force ) ;
	    _sceVu0CopyVector( &offs, &def->hit ) ;
	    offs.vw = 1.0f ;
	    frc.vw  = 0.0f ;
	    _sceVu0InversMatrix( &inv, &work->objs->world ) ;
	    _sceVu0ApplyMatrix( &frc , &inv, &frc  ) ;
	    _sceVu0ApplyMatrix( &offs, &inv, &offs ) ;
	    _sceVu0CopyVector( &hit, &offs ) ; 

	    max = 0.0f ;
	    flag = 0 ;
	    /*弾のポリゴン当たり判定*/
	    for ( m=obj->model->packs, i=obj->model->n_packs ; --i>=0 ; m++ )
	    {
#ifdef PSX2
			s = (SVECTOR *)m->verts ;
			n = (SVECTOR *)m->norms ;
			for ( j=m->n_verts ; --j>=0 ; s++, n++ ) {
				if ( !(n->pad & 0x8000) ) {
					if ( BRK_UTL_CheckLineInPolygon( &hit, &frc, s-2 ) ) {
						/* 跳弾エフェクト */
						CallSpark( work, off, def, &hit ) ;
						/* 揺らし処理 */
						BreakSwing( work, &hit, &frc, 1 ) ;
						
						return ;
					}
				}
			}
#else
			int v0, v1, v2 ;
			
			v0 = v1= v2= *(short*)m->index ;
			for ( j=0 ; j<m->n_indices ; j++, kmss++ ) {
				if ( v0!=v1 && v0!=v2 && v1!=v2 ) {
					if ( BRK_UTL_CheckLineInPolygon( &offs, &frc,
													&kmss[v0],
													&kmss[v1],
													&kmss[v2] ) ) {
						/* 跳弾エフェクト */
						CallSpark( work, off, def, &hit ) ;
						/* 揺らし処理 */
						BreakSwing( work, &hit, &frc, 1 ) ;

						return ;
					}
				}
				v2 = v1 ;
				v1 = v0 ;
				v0 =*((short*)m->index + j) ;
			}
#endif
	    }
	}
	else if ( off->weapon_type & WP_BLAST )
	{
	    frc.vx = frnd()*400.0f ;
	    frc.vy = frnd()*400.0f ;
	    frc.vz = frnd()*400.0f ;
	    frc.vw = 0.0f ;

	    /* 揺らし処理 */
	    BreakSwing( work, &off->center, &frc, 3 ) ;
	}
    }
}



static void Die( Work *work )
{
    int i ;

    if ( work->objs )
	DG_DequeueObjs( work->objs ), DG_FreeObjs( work->objs ) ;
    for ( i=BRK_SEG_N_SEGS ; --i>=0 ; )
	if ( work->segs[i] )
	    HZX_RemoveDynamicSegment( work->segs[i] ) ;
    for ( i=BRK_SEG_N_FLRS ; --i>=0 ; )
	if ( work->flrs[i] )
	    HZX_RemoveDynamicFloor( work->flrs[i] ) ;

    GM_FreeTarget( &work->target ) ;
}



/* 位置の登録 */
static inline void RecieveMessage( Work *work )
{
    GV_MSG  *msg   ;
    int i ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
	switch( msg->message[0] )
	{
	case 0:
	    if ( work->mode < 2 )
	    {
		work->mode = HZX_OFF_FRAME ;
		GM_SeSetMode( SD_A_KAIDANFA, &work->pos, GM_SEMODE_BOMB ) ;
	    }
	    break ;

	case 1:
	    if ( work->mode <= 1 )
	    {
		work->rot_vx = (short)(-400.0f * work->rate*0.5f) ;
		work->rot_vz = (short)(1000.0f * work->rate*0.5f) ;
		work->rot_vx /= 4, work->rot_vz /= 4 ;
		work->mode = 1 ;
	    }
	    break ;
	}
}


/* ハザードを動かす */
static void BRK_SWG_ActMoveHzx( Work *work )
{
    int     i, j ;
    IVECTOR p[4] ;
    FVECTOR v    ;

#if 0
    for ( i=BRK_SEG_N_SEGS ; --i>=0 ; )
    {
	_sceVu0ApplyMatrix( &v, &work->objs->world, &BRK_SWG_Wall[i*2+0] ) ;
	v.vw = BRK_SWG_WallHeight[i*2+1] ;
	_sceVu0FTOI0Vector( &p[0], &v ) ;
	_sceVu0ApplyMatrix( &v, &work->objs->world, &BRK_SWG_Wall[i*2+1] ) ;
	v.vw = BRK_SWG_WallHeight[i*2+1] ;
	_sceVu0FTOI0Vector( &p[1], &v ) ;
	HZX_MoveDynamicSegment( work->segs[i], &p[0], &p[1] ) ;
	
	//HZX_ViewDynamicSegment( work->segs[i] ) ;
    }
#endif

    for ( i=BRK_SEG_N_FLRS ; --i>=0 ; )
    {
	for ( j=4 ; --j>=0 ; )
	{
	    _sceVu0ApplyMatrix( &v,
				&work->objs->world,
				&BRK_SWG_Floor[i*4+j] ) ;
	    _sceVu0FTOI0Vector( &p[j], &v ) ;
	}
	HZX_MoveDynamicFloor( work->flrs[i],
			      &p[0],&p[1],&p[2],&p[3] ) ;

	//HZX_ViewDynamicFloor( work->flrs[i], 4 ) ;
    }
}

static void ActSetSound( Work *work, int prv, int cur, int vol )
{
    int     pan ;
    float   dis ;
    FVECTOR v   ;
    
    /* 回転速度の符号が反転したら音がなる */
    if ( (prv>0 && cur <= 0) || (prv<=0 && cur> 0) )
    {
	_sceVu0SubVector( &v, &GM_PlayerControl->mov, &work->pos ) ;
	dis = _sceVu0InnerProduct( &v, &v ) ;
	dis = sceVu0Sqrt( dis ) ;
#if 0
	if ( Is_PlayerOn( work ) )
	    vol = (vol>0 ? vol : -vol) >> 2 ;
	else
	    vol = (vol>0 ? vol : -vol) >> 5 ;
#endif
	vol = (int)( ((vol>0 ? vol : -vol) >> 2) * 500.0f / dis ) ;
	vol = vol>64 ? 64 : vol ;
	if ( vol )
	{
      float bp_angle;
	    pan = GM_SeGetPan( &work->pos, GM_SEMODE_BOMB, &bp_angle ) ;
	    GM_SeSet3D( pan, vol, SD_A_KAIDANGR, bp_angle ) ;
	}
    }
}

static void Act( Work *work )
{
    int      i, r ;
    int      cur, prv ;
    extern void *SetSplushSequence2( FVECTOR *pos ) ;

    RecieveMessage( work ) ;

    BRK_SWG_ActMoveHzx( work ) ;

    switch ( work->mode )
    {
    case 3:
	/* 床を消す */
	for ( i=BRK_SEG_N_FLRS ; --i>=0 ; )
	{
	    work->flrs[i]->atr |= HZX_FLOOR_SKIP ;
	    HZX_DynamicFloorExecCallback( work->flrs[i],
					  HZX_CALLBACK_FLAG_DESTROY ) ;
	}
	for ( i=BRK_SEG_N_SEGS ; --i>=0 ; )
	    work->segs[i]->atr |= HZX_SEG_SKIP ;

	/* 落ちプロックを呼ぶ */
	if ( work->proc )
	    GCL_ExecProc( work->proc, NULL ) ;
    default:
	work->mode-- ;
    case 2:
	work->pos.vy += work->pos_vy -= GRAVITY ;
	work->rot_x += work->rot_vx /* *= 0.99f*/ ;
	work->rot_z += work->rot_vz /* *= 0.99f*/ ;

	/* ワールドを計算 */
	MakeObjWorld( work ) ;

	for( i=sizeof(BRK_SWG_SplashPos)/sizeof(FVECTOR) ; --i>=0 ; )
	    if ( !(work->splash & (1<<i)) )
	    {
		FVECTOR  v ;
		_sceVu0ApplyMatrix( &v,
				    &work->objs->world,
				    &BRK_SWG_SplashPos[i] ) ;
		if ( v.vy < GM_WaterLevel+1000.0f )
		{
		    work->splash |= (1<<i) ;
		    SetSplushSequence2( &v ) ;
		}
	    }

	if ( work->pos.vy < -45000.0f )
	    GV_DestroyActor( work ) ;
	break ;

    case 1:
	/*各軸の揺れ計算*/
	r = work->rot_vx + work->rot_x - work->rot_dx ;
	for ( i=6 ; r/2 && --i>=0 ; r/=2 ) ;
	work->rot_x += work->rot_vx -= r ;

	prv = work->rot_vz ;
	r = work->rot_vz + work->rot_z - work->rot_dz ;
	for ( i=6 ; r/2 && --i>=0 ; r/=2 ) ;
	work->rot_z += work->rot_vz -= r ;
	cur = work->rot_vz ;

	ActSetSound( work, prv, cur, work->rot_z ) ;

	/* 揺れがデフォルトになったかどうか */
	if ( (work->rot_x==work->rot_dx) && (!work->rot_vx) &&
	     (work->rot_z==work->rot_dz) && (!work->rot_vz) )
	    work->mode = 0 ;

	/* ワールドを計算 */
	MakeObjWorld( work ) ;
    case 0:
	;
    }
}


int BRK_SWG_MakeHzx( Work *work, DG_OBJS *objs, int where )
{
    IVECTOR p[4] ;
    int     i, j ;
    int     flag ;
    FVECTOR v ;
    HZX_GROUP_ID hzx_id ;

    ASSERT( where ) ;

    hzx_id = GM_GetHzxGroupID( where ) ;
    for ( i=BRK_SEG_N_SEGS ; --i>=0 ; )
    {
#if 0
	_sceVu0ApplyMatrix( &v, &objs->world, &BRK_SWG_Wall[i*2+0] ) ;
	v.vw = BRK_SWG_WallHeight[i*2+0] ;
	_sceVu0FTOI0Vector( &p[0], &v ) ;

	_sceVu0ApplyMatrix( &v, &objs->world, &BRK_SWG_Wall[i*2+1] ) ;
	v.vw = BRK_SWG_WallHeight[i*2+1] ;
	_sceVu0FTOI0Vector( &p[1], &v ) ;
#else
	_sceVu0FTOI0Vector( &p[0], &BRK_SWG_Wall[i*2+0] ) ;
	_sceVu0FTOI0Vector( &p[1], &BRK_SWG_Wall[i*2+1] ) ;
#endif

	flag = HZX_SEG_NO_BULLETHOLE| HZX_SEG_NO_C4 | HZX_SEG_NO_BULLET ;
	if ( i==3 || i==1 ) /* 張りつき と コンコンなし */
	    flag |=  HZX_SEG_NO_KNOCK_SE| HZX_SEG_NO_HARITSUKI ;
	work->segs[i] = HZX_AddDynamicSegment( hzx_id, &p[0], &p[1], flag ) ;
	/*レーダーに映すため*/
	work->segs[i]->atr &= ~HZX_SEG_NO_DISP_RADAR ;
    }

    for ( i=BRK_SEG_N_FLRS ; --i>=0 ; )
    {
	for ( j=4 ; --j>=0 ; )
	{
	    _sceVu0ApplyMatrix( &v, &objs->world, &BRK_SWG_Floor[i*4+j] ) ;
	    _sceVu0FTOI0Vector( &p[j], &v ) ;
	}

	flag = (HZX_FLOOR_NO_BULLETHOLE| HZX_FLOOR_NO_BULLET  |
		HZX_FLOOR_NO_BLOOD     | HZX_FLOOR_NO_C4      |
		HZX_FLOOR_NO_SPRAY     | HZX_FLOOR_NO_CLAYMORE) ;
	if ( i )
	    flag |= HZX_FLOOR_STEP ;
	work->flrs[i] = HZX_AddDynamicFloor( hzx_id,
					     &p[0],&p[1],&p[2],&p[3], 4,
					     flag ) ;
    }

    return 0 ;
}


int BRK_SWG_InitTarget( Work *work, int where )
{
    FVECTOR t_size, t_pos ;
    TARGET       *t   = &work->target   ;
    POWER_TARGET *p   = &work->power    ;
    DG_DEF       *def = work->objs->def ;
    FVECTOR uv = { def->ux, def->uy, def->uz, 0 } ;
    FVECTOR lv = { def->lx, def->ly, def->lz, 0 } ;

    _sceVu0SubVector( &t_size, &uv, &lv ) ;
    _sceVu0ScaleVector( &t_size,  &t_size, 0.5f ) ;
    _sceVu0AddVector( &t_pos, &uv, &lv ) ;
    _sceVu0ScaleVector( &t_pos,  &t_pos, 0.5f ) ;
    GM_SetTarget( t, TARGET_DEFENSE|TARGET_ROTATE|TARGET_THROUGH, where, BOTH_SIDE, &t_size, &t_pos ) ;
    GM_SetPowerTarget( t, p, POWER_DECREASE, GM_Vitality, 0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( t, BRK_SWG_TargetCallBack, work ) ;
    GM_PutTarget( t ) ;

    //NewTargetView( t,  200, 50, 32 ) ;

    return 0 ;
}

static int GetResources( Work *work, int name, int where )
{
    IVECTOR  buf ;
    SVECTOR  rot ;
    DG_DEF  *def ;
    LIT_DEF *lit ;

    work->where  = where ;
    work->name   = name  ;
    work->rot_vx = 0 ;
    work->rot_vz = 0 ;
    work->mode   = 0 ;

    work->rate = BRK_SWG_MOVE_RATE ;
    work->rate *= (float)GCL_GetOptionValue( 'E', 100 ) / 100.0f ;
    work->life = GCL_GetOptionValue( 'L', 3 ) ;
    if ( !GCL_GetOption( 'm' ) )
	PERROR( "No model specified(-model option missing) :: NewPutSwingObject\n" ) ;
    if ( !(def = GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ) )) )
	PERROR( "Cannot find KMS-Model( Not in data.cnf ) :: NewPutSwingObject\n" ) ;
    if ( !(work->objs = DG_MakeObjs( def, DG_FLAG_PAINT|DG_FLAG_ONEPIECE, 0 )) )
	PERROR( "Cannot create DG_OBJS( Maybe no memory ) :: NewPutSwingObject\n" ) ;
    DG_QueueObjs( work->objs ) ;
    GM_GroupObjs( work->objs, where ) ;


    /* ライトデータを取得 */
    lit = GM_GetMap( where )->light ;
    if ( GCL_GetOption( 'l' ) )
	if ( !(lit = GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'l' ))) )
	    PERROR( "Cannot find Light data (not in data.cnf) :: NewPutFedralObject\n" ) ;

    /* 向き(静止向き) */
    if ( GCL_GetOption( 'r' ) )
    {
	GCL_GetSV( GCL_NextStr(), (short*)&rot ) ;
	work->rot_x = work->rot_dx = rot.vx*65536/4096 ;
	work->rot_y = work->rot_dy = rot.vy*65536/4096 ;
	work->rot_z = work->rot_dz = rot.vz*65536/4096 ;
    }

    /* 原点の位置 */
    if ( GCL_GetOption( 'p' ) )
    {
	GCL_GetIV( GCL_NextStr(), (int *)&buf ) ;
	vu0_IV0toFV( &buf, &work->pos ) ;
    }

    work->proc = GCL_GetOptionValue( 'C', 0 ) ;

    BRK_SWG_InitTarget( work, where ) ;
    MakeObjWorld( work ) ;
    DG_MakePreshade( work->objs, lit ) ;

    if ( BRK_SWG_MakeHzx( work, work->objs, where ) < 0 )
	PERROR( "HZXDATA(No memory) :: NewPutFedralObject\n" ) ;

    return 0 ;
}

void *NewPutSwingObject( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        if( GetResources( work, name, where ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
