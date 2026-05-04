//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   fort_lgt.c
   フォーチュン戦専用 ライト

   1999/12/13 T. Morita
   $Id: fort_lgt.c,v 1.1.1.3 2002/11/19 11:46:12 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "gameheader.h"

#define MAKING 0

// for DG-LIB
#include "../../include/libdg_x.h"
#include "../../include/util.h"
#include "../../brk_hzd/brk_hazard.h"
#include "../../brk_utl/brk_utl.x"
#include "../include/fort_flag.h"


#define BRK_VEL_R        0.85f
#define BRK_ROT_R        8
#define BRK_GRAVITY      6
#define BRK_BOUNCE       1.48f
#define BRK_BOUNCE_WALL  1.38f

#define FRT_LGT_N_VOLS   6
#define FRT_LGT_VOL_LENG 280.0f

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

typedef struct work_t
{
    GV_ACT_EX    actor  ;

    TARGET       def_t ;
    POWER_TARGET def_p  ;
    TARGET       off_t ;
    POWER_TARGET off_p ;

    FVECTOR   pos        ;
    FVECTOR   pos_v      ;
    short     rot[XYZ]   ;
    short     rot_v[XYZ] ;

    DG_OBJS  *volume ;  /* 光モデル */
    DG_OBJS  *objs   ;  /* 電灯のモデル */
    HZD_BOX  *hzd    ;  /* 当たりハザード */
    DG_PRIM2 *volume_p ;/* モアモア */

    DG_PRIM2 *gloom ;  /* 点灯用 PRIM 加算表示 */

    DG_DEF   *bf_def ;
    DG_DEF   *af_def ;
    int       where  ;
    int       flag   ;

    int      *brk_flag ; /*壊れ状態維持フラグ*/
    int       brk_mask ; /*壊れ状態維持マスク*/
    int       off_mask ; /*壊れ状態維持マスク*/

    short     alpha  ;
    short     blink  ;
    short     never_end ;
    short     tic    ;
    short     se_tic ;

    FMATRIX  *parent ;
    FVECTOR  *offset ;

    DG_PRIM2 *prim   ; /* 床の照り返し */
    float     floor  ;
#if MAKING
    void    * wireframe ;
#endif
} Work ;


static FVECTOR LightShape[] = {
    {-100.0f, -55.0f, -750.0f, 1.0f },
    {-100.0f, -55.0f,  750.0f, 1.0f },
    { 100.0f, -55.0f, -750.0f, 1.0f },
    { 100.0f, -55.0f,  750.0f, 1.0f },
} ;



#define DEGtoANG(_a) ((_a)*8192/45)
#define ANGtoRAD(_a) ((_a)*(float)M_PI/32768.0f)

extern int  FRT_LGT_AddLight( FVECTOR *center ) ;
extern void FRT_LGT_RemoveLight( FVECTOR *center ) ;
extern int  FRT_AIM_AddAimSpot( void *aim_id, void *nxt_id, int phase,
				FVECTOR *center, FVECTOR *front, FVECTOR *rear, u_int flag ) ;
extern int  FRT_AIM_RemoveAimSpot( void *work ) ;
extern void *NewFortSpark( FVECTOR *pos, FVECTOR *dir, HZD_BOX *hzd, int mode ) ;
extern void *NewCrushWithForce( FVECTOR *pos, FVECTOR *force );
extern void *NewBreakLight( FVECTOR *pos, FVECTOR *force,  int where ) ;

extern void *NewShakeCamera2( int chanl, int intense, int time, FVECTOR *pos ) ;
extern void FRT_LGT_SetMagnitude( FVECTOR *center, float magnitude ) ;

static void RotateTransMatrix( Work *work, FMATRIX *mtx )
{
    _sceVu0RotMatrixY( mtx, &DG_UnitMatrix, ANGtoRAD(work->rot[Y]) ) ;
    _sceVu0RotMatrixX( mtx, mtx           , ANGtoRAD(work->rot[X]) ) ;
    _sceVu0RotMatrixZ( mtx, mtx           , ANGtoRAD(work->rot[Z]) ) ;
    if ( work->offset )
    {
	_sceVu0CopyVectorXYZ( (FVECTOR*)mtx->m[W], work->offset ) ;
	_sceVu0MulMatrix( mtx, work->parent, mtx ) ;
	_sceVu0SubVector ( &work->pos_v, (FVECTOR *)mtx->m[W], &work->pos ) ;
	_sceVu0CopyVector( &work->pos  , (FVECTOR *)mtx->m[W] ) ;
    }
    else
	_sceVu0CopyVectorXYZ( (FVECTOR*)mtx->m[W], &work->pos ) ;
    GM_MoveTarget2Map( &work->def_t, mtx, work->where ) ;
}
static void RotateMatrixXY( Work *work )
{
    FMATRIX *mtx = &work->objs->world ;

    _sceVu0RotMatrixX( mtx, &DG_UnitMatrix, ANGtoRAD(work->rot[X]) ) ;
    _sceVu0RotMatrixY( mtx, mtx           , ANGtoRAD(work->rot[Y]) ) ;
}
static inline void TransMatrix( FMATRIX *mtx, FVECTOR *pos  )
{
    _sceVu0CopyVectorXYZ( (FVECTOR*)mtx->m[W], pos ) ;
}


DG_OBJS *FRT_LGT_InitModel( Work *work, DG_DEF *def, DG_OBJS *old )
{
    DG_OBJS *objs ;

    objs = DG_MakeObjs( def, DG_FLAG_PAINT|DG_FLAG_ONEPIECE, 0 ) ;
    if ( objs )
    {
	DG_QueueObjs( objs ) ;
	RotateTransMatrix( work, &objs->world ) ;
	DG_MakePreshade( objs, GM_GetMap( work->where )->light ) ;
	if ( old )
	{
	    DG_DequeueObjs( old ) ;
	    DG_FreeObjs( old ) ;

#if MAKING
	    if ( work->wireframe )
	    {
		GV_DestroyOtherActor( work->wireframe ) ;
		work->wireframe=NULL ;
	    }
#endif

	}

#if MAKING
	{
	    extern void *NewDrawWireframe( DG_OBJ * ) ;
	    work->wireframe = NewDrawWireframe( &objs->objs[0] ) ;
	}
#endif

    }
    return objs ;
}

static void FRT_LGT_TurnOff( Work *work )
{
    FRT_LGT_RemoveLight( &work->pos ) ;
    DG_DestroyLightSphere( &work->pos, 2000.0f ) ;
    work->objs = FRT_LGT_InitModel( work, work->af_def, work->objs ) ;

    /* 明るいモアモアを取り去る（床の照り返しも） */
    if ( work->gloom )
	DG_InvisiblePrim2( work->gloom ) ;
    //work->gloom->pos[0].color.vw = 0 ;
    if ( work->prim )
	DG_InvisiblePrim2( work->prim ) ;
    if ( work->volume )
	DG_InvisibleObjs( work->volume ) ;
    if ( work->volume_p )
	DG_InvisiblePrim2( work->volume_p ) ;
    work->alpha  = 0 ;
    work->se_tic = 0 ;
}

static void FRT_LGT_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work *work = (Work *)ptr ;
    FMATRIX  mtx  ;
    extern void *NewSpark( FMATRIX *world ) ;

    if ( def->damaged & TARGET_POWER )
    {
	/* clear damage */
	GM_ClearTargetDamage( def ) ;

	/* 跳弾エフェクトを出す */
	if ( off->weapon_type & (WP_BULLET|WP_M92) )
	{
	    /* 跳弾を出す */
	    GM_CurrentMap = work->where ;
	    UTL_VectoMat( &mtx, &off->power->force, &def->hit, Y ) ;
	    NewSpark( &mtx ) ;
	}

	if ( work->flag == 0 )
	    if ( off->weapon_type & (WP_BULLET|WP_M92|WP_NIKITA|WP_GRENADE) )
	    {
		FRT_LGT_TurnOff( work ) ;

		printf( "NewBreakLight %.0f %.0f %.0f\n",
			def->hit.vx,def->hit.vy,def->hit.vz ) ;
		/* ガラスの破片 */
		//NewBreakLight( &def->hit, &def->hit, work->where ) ;
		NewBreakLight( &work->pos, &work->pos, work->where ) ;

		/* 壊れフラグを制御 */
		*work->brk_flag |= work->off_mask ;

		/*割れ音*/
		GM_SeSetMode( SD_A_LAMP02, &work->pos, GM_SEMODE_BOMB ) ;
	    }
	if ( off->weapon_type & (WP_NIKITA|WP_GRENADE) )
	{
	    work->flag = 1 ;
	    work->rot_v[X] = ((irnd()&32) - 16)*16 ;
	    work->rot_v[Y] = ((irnd()&16) -  8)*16 ;
	    _sceVu0ScaleVector( &work->pos_v, &off->power->force, 0.05f ) ;

	    /*防御ターゲットを取りさって,攻撃ターゲットに変える*/
	    //GM_FreeTarget( def ) ;

	    /* 揺らすマグニチュードを設定 */
	    FRT_LGT_SetMagnitude( &def->hit, 600.0f ) ;
	    NewShakeCamera2( 0, 256, 8, &def->hit ) ;

	    /* エフェクト */
	    NewFortSpark( &def->hit, &off->power->force, work->hzd, 2 ) ;
	    NewCrushWithForce( &def->hit, &off->power->force ) ;

	    /* 壊れフラグを制御 */
	    *work->brk_flag |= work->brk_mask ;

	    /* ターゲットシステムから外す */
	    FRT_AIM_RemoveAimSpot( &work->def_t ) ;
	}
    }
}


static int FRT_LGT_InitTarget( Work *work, DG_OBJ *obj, int where )
{
    FVECTOR       t_size, t_pos ;
    TARGET       *t ;
    POWER_TARGET *p ;
    static FVECTOR OffenseSize = { 500.0f, 500.0f, 500.0f, 0.0f } ;

    t_size.vx = (obj->model->ux - obj->model->lx) * 0.5f ;
    t_size.vy = (obj->model->uy - obj->model->ly) * 0.5f ;
    t_size.vz = (obj->model->uz - obj->model->lz) * 0.5f ;
    t_pos.vx  = (obj->model->ux + obj->model->lx) * 0.5f ;
    t_pos.vy  = (obj->model->uy + obj->model->ly) * 0.5f ;
    t_pos.vz  = (obj->model->uz + obj->model->lz) * 0.5f ;

    /* 防御ターゲット */
    t = &work->def_t ;
    p = &work->def_p  ;
    GM_SetTarget( t, TARGET_DEFENSE|TARGET_POWER|TARGET_ROTATE|TARGET_SEEK,
		  where, BOTH_SIDE, &t_size, &t_pos ) ;
    GM_SetPowerTarget( t, p, POWER_DECREASE, GM_Vitality,
		       0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( t, FRT_LGT_TargetCallBack, work ) ;
    GM_PutTarget( t ) ;

    /* 攻撃ターゲット */
    t = &work->off_t ;
    p = &work->off_p ;
    GM_SetTarget( t, TARGET_OFFENSE|TARGET_POWER,
		  work->where, PLAYER_SIDE, &OffenseSize, &DG_ZeroVector ) ;
    GM_SetTargetWeaponType( t, WP_KICK ) ;
    GM_SetPowerTarget( t, p, POWER_DECREASE, GM_Vitality,
		       10, 10, &DG_ZeroVector ) ;

    //NewTargetView( t,  200, 50, 32 ) ;

    return 0 ;
}

static void Die( Work *work )
{
#if MAKING
    if ( work->wireframe )
	GV_DestroyOtherActor( work->wireframe ) ;
#endif

    if ( work->objs )
	DG_DequeueObjs( work->objs   ), DG_FreeObjs( work->objs   ) ;
    if ( work->volume )
	DG_DequeueObjs( work->volume ), DG_FreeObjs( work->volume ) ;
    if ( work->gloom )
	GM_FreePrim2( work->gloom ) ;
    //DG_DequeueComdlObjs( work->gloom ), DG_FreeComdl( work->gloom ) ;
    if ( work->prim )
	GM_FreePrim2( work->prim ) ;
    if ( work->volume_p )
	GM_FreePrim2( work->volume_p ) ;

    GM_FreeTarget( &work->def_t ) ;

    FRT_LGT_RemoveLight( &work->pos ) ;
    FRT_AIM_RemoveAimSpot( &work->def_t ) ;
}

static void FRT_LGT_ActHanging( Work *work )
{
    int i, j, r ;

    for ( i=XYZ ; --i>=0; )
    {
	r = work->rot_v[i] + work->rot[i] ;
	for ( j=6 ; r/2 && --j>=0 ; )
	    r /= 2 ;
	work->rot[i] += work->rot_v[i] -= r ;
    }
    if ( !work->rot[Z] && !work->rot_v[Z] )
	work->rot_v[Z] += (short)(work->pos_v.vx*5.0f) ;
    for ( i=XYZ ; --i>=0; )
	work->rot[i] += work->rot_v[i] ;
    RotateTransMatrix( work, &work->objs->world ) ;
#if 0
    if ( work->gloom )
	_sceVu0CopyMatrix( &work->gloom->pos[0].world, &work->objs->world ) ;
#endif
    if ( work->volume )
	_sceVu0CopyMatrix( &work->volume->world, &work->objs->world ) ;
}

static int FRT_LGT_ActDrop( Work *work )
{
    int flag = 0 ;
    static FVECTOR bounce = { BRK_BOUNCE_WALL, BRK_BOUNCE, BRK_BOUNCE_WALL, 0 } ;
    FVECTOR size ;

    work->pos_v.vy -= BRK_GRAVITY ;
    RotateMatrixXY( work ) ;
    BRK_UTL_SizeOfBound( work->objs, &work->objs->world, &size ) ;
    switch( BRK_CheckHazard( work->hzd, &work->pos, &work->pos_v, &bounce, &size ) )
    {
    case 3:
    case 1:
	work->pos_v.vx *= BRK_VEL_R ;
	work->pos_v.vz *= BRK_VEL_R ;
	if ( BRK_UTL_PutCenterHazard( work->hzd, &work->pos, &work->pos_v, 1000.0f, 25.0f ) )
	{
	    work->pos_v.vy = 30.0f ;
	    work->rot_v[X] = irnd()&10 ? 200 : -200 ;
	}
	else
	{
	    work->rot_v[X] = ((work->rot[X]> DEGtoANG(90) ? DEGtoANG(179) :
			       work->rot[X]>-DEGtoANG(90) ? DEGtoANG(  0) :
			                                   -DEGtoANG(179) ) - work->rot[X]) / 16 ;
	    if ( !(int)work->pos_v.vx && !(int)work->pos_v.vz &&
		 !(int)(work->pos_v.vy/BRK_GRAVITY) )
		work->rot_v[X]=0, flag = 1 ;
	}

	/* ガタガタしないようにする */
	if ( work->tic > 0 )
	    work->rot_v[X]=0, flag = 1 ;
	else
	{
	    int pan, vol ;
       float bp_angle;

	    vol = GM_SeGetVol( &work->pos, GM_SEMODE_BOMB ) ;
	    vol = (4 - work->se_tic++) * vol / 4 ;
	    vol = vol>10 ? vol&0x3f : 10 ;
	    pan = GM_SeGetPan( &work->pos, GM_SEMODE_BOMB, &bp_angle ) ;
	    GM_SeSet3D( pan, vol, SD_A_LANPOTI1, bp_angle ) ;
	}
	work->tic = 10 ;
	break ;
    case 0:
	work->rot[X] += work->rot_v[X] ;
	work->rot[Y] += work->rot_v[Y] ;
	_sceVu0AddVector( &work->pos, &work->pos, &work->pos_v ) ;
	break ;
    }
    TransMatrix( &work->objs->world, &work->pos ) ;

    GM_MoveTarget2Map( &work->def_t, &work->objs->world, work->where ) ;
    GM_MoveTarget2Map( &work->off_t, &work->objs->world, work->where ) ;
    if ( work->tic == 10 )
	GM_PutTarget( &work->off_t ) ;
    if ( work->tic > 0 )
	work->tic-- ;

    return flag ;
}

static int FRT_LGT_ActFalldown( Work *work )
{
    int flag = 0, r ;
    FVECTOR size ;

    work->pos_v.vy = -BRK_GRAVITY*5 ;

    r = work->rot[X] ;
    work->rot[X] += work->rot_v[X] += DEGtoANG( work->rot[X]/DEGtoANG(90) ? 5: -5 ) * (work->rot[X]>0 ? 1: -1) ;
    work->rot[Y] += work->rot_v[Y]  = work->rot_v[Y] * (BRK_ROT_R-1) / BRK_ROT_R ;
    if ( (work->rot[X]>0 && r<=0) || (work->rot[X]<=0 && r>0) )
    {
        work->rot[X] = work->rot_v[X] = 0 ;
	if ( !work->rot_v[Y] )
	    flag = 1 ;
    }
    else if ( work->rot[X]>DEGtoANG(175) || work->rot[X]<-DEGtoANG(175) )
    {
        work->rot[X] = DEGtoANG(180) ;
	work->rot_v[X] = 0 ;
	if ( !work->rot_v[Y] )
	    flag = 1 ;
    }
    RotateMatrixXY( work ) ;
    BRK_UTL_SizeOfBound( work->objs, &work->objs->world, &size ) ;
    if ( !BRK_CheckHazard( work->hzd, &work->pos, &work->pos_v, &BRK_HZD_NoBounce, &size ) )
	work->pos.vy += work->pos_v.vy, flag=0 ;
    TransMatrix( &work->objs->world, &work->pos ) ;

    GM_MoveTarget2Map( &work->def_t, &work->objs->world, work->where ) ;
    GM_MoveTarget2Map( &work->off_t, &work->objs->world, work->where ) ;

    return flag ;
}

#define BONBORI_SIZE 2500.0f
static void  FRT_LGT_ActDimLight( Work *work )
{
    static FVECTOR Shape[4] = { {-BONBORI_SIZE, 0.0f, BONBORI_SIZE,1.0f},
				{-BONBORI_SIZE, 0.0f,-BONBORI_SIZE,1.0f},
				{ BONBORI_SIZE, 0.0f, BONBORI_SIZE,1.0f},
				{ BONBORI_SIZE, 0.0f,-BONBORI_SIZE,1.0f} } ;
    FVECTOR v, *pos ;
    int i ;

    if ( work->prim )
    {
	v.vx = work->pos.vx ;
	v.vy = work->floor  ;
	v.vz = work->pos.vz ;
	v.vw = 0.0f ;

#if MAKING
	{
	    extern void *NewDrawCircleLife( FMATRIX *root,
					    FVECTOR *pos, float radius,
					    int rbga, int life ) ;

	    NewDrawCircleLife( NULL, &v, BONBORI_SIZE,0x7f007f7f, 1 ) ;
	}
#endif
	DG_SwitchBuffPrim2( work->prim ) ;
	pos = work->prim->pos[work->prim->buffer_clock] ;
	for ( i=4 ; --i>=0 ; )
	    _sceVu0AddVector( &pos[i], &Shape[i], &v ) ;


    }
}


static inline void FRT_LGT_ActVolume( Work *work )
{
    static FVECTOR front = { 0.0f, 0.0f,-250.0f, 1.0f} ;
    static FVECTOR back  = { 0.0f, 0.0f, 250.0f, 1.0f} ;
    DG_PRIM2_UVRGBWH *uv ;
    FMATRIX pers ;
    int     alpha, i ;
    FVECTOR f, b ;

    if ( work->volume_p )
    {
	DG_SwitchBuffPrim2( work->volume_p ) ;
	uv = work->volume_p->uvrgb[work->volume_p->buffer_clock] ;

	if ( !(*work->brk_flag & (work->off_mask|work->brk_mask)) )
	{
	    /* 蛍光灯の中心が画面の中央に来ているかどうかを見て,
	       前後の点の離れ具合で明るさを調整する 離れる→明  寄る→暗 */
	    _sceVu0MulMatrix( &pers, &DG_Chanl(0)->eye_pers,
			      work->volume_p->root ) ;
	    _sceVu0ApplyMatrix( &f, &pers, &front ) ;
	    _sceVu0ApplyMatrix( &b, &pers, &back  ) ;
//yano 0除算回避 2002.02.22
  		if( f.vw == 0.0f ){ f.vw = 0.00001f; }
  		if( b.vw == 0.0f ){ b.vw = 0.00001f; }
//
	    alpha = (int)(fpu_Abs(f.vx/f.vw - b.vx/b.vw) * 800.0f) ;
	    alpha = alpha<8 ? 8 : alpha>24 ? 24 : alpha ; 
	    /* 電気が消えていたら明るくしない */
	    alpha = alpha>work->alpha/4 ? work->alpha/4 : alpha ; 
	}
	else
	    alpha = 0 ;

	for( i=FRT_LGT_N_VOLS ; --i>=0 ; uv++ )
	    uv->a = alpha ;
    }
}

static void FRT_LGT_ActBlinkOff( Work *work )
{
    int det ;
    DG_PRIM2_UVRGB   *uv   ;

    if ( (*work->brk_flag & (work->off_mask|work->brk_mask)) )
    {
	work->alpha = 0 ;
	return ;
    }

    if ( work->blink < 0 )
	det = work->blink++ > -50 ;
    else if ( work->blink > 0 )
	det = work->blink-- > 50 ;
    else
	return ;

    if ( det )/* 明るくしていく */
    {
	if ( (work->alpha += 16) > 128 )
	    work->alpha = 128 ;
	DG_VisibleObjs( work->volume ) ;
    }
    else      /* 暗くしていく */
    {
	if ( (work->alpha -= 6) < 0 )
	    work->alpha = 0 ;
	DG_InvisibleObjs( work->volume ) ;
    }

    /* 明るさをセットする */
    if ( work->prim )
    {
	uv = work->prim->uvrgb[work->prim->buffer_clock] ;
	uv[0].a = uv[1].a = uv[2].a = uv[3].a = work->alpha/4 ;
    }
    if ( work->gloom )
    {
	uv = work->gloom->uvrgb[work->gloom->buffer_clock] ;
	uv[0].a = uv[1].a = uv[2].a = uv[3].a = work->alpha ;
	//work->gloom->pos[0].color.vw = work->alpha ;
    }
}


static inline void FRT_LGT_ActMessage( Work *work )
{
    GV_MSG *msg ;
    int i ;

    for ( i=GV_ReceiveMessage( 7637503/*fort_lgt*/, &msg ) ; i>0 ; i--, msg++ )
        switch( msg->message[0] )
	{
	case 0:
	    if (  work->alpha && !work->blink )
	    {
		GM_SeSetMode( SD_A_LANPKIE1, &work->pos, GM_SEMODE_NORMAL ) ;
		work->blink =  47 + (irnd() & 3) ;
	    }
	    break ;
	case 1:
	    if ( !work->alpha && !work->blink )
		work->blink = -80 - (irnd() & 7) ;
	    break ;
	}
}

static void Act( Work *work )
{
    FRT_LGT_ActMessage( work ) ;
    FRT_LGT_ActVolume( work ) ;

    switch( work->flag )
    {
	FVECTOR from ;
    case 0:
	FRT_LGT_ActDimLight( work ) ;
	FRT_LGT_ActHanging( work ) ;
	if ( work->blink )
	    FRT_LGT_ActBlinkOff( work ) ;
	break ;
    case 1:
	if ( FRT_LGT_ActDrop( work ) )
	   work->flag = 2 ;
	break ;
    case 2 :
	if ( FRT_LGT_ActFalldown( work ) )
	    work->flag = 3 ;
    case 3 :
	break ;
    case 4 :
	work->flag = (GV_Time&1 ? 63 : 5) + (irnd() & 63) ;
	_sceVu0ApplyMatrix( &from, work->parent, work->offset ) ;
	NewFortSpark( &from, (FVECTOR *)work->objs->world.m[Z], work->hzd, 2 );

	if ( --work->never_end < 0 )
	    work->flag = 3 ;
    default:
	work->flag-- ;
	break ;
    }
}


#if 0
/* 明るくする */
static DG_COMDL *FRT_LGT_InitGloomComdl( Work *work, DG_DEF *def )
{
    DG_COMDL *comdl ;

    comdl = DG_MakeComdl( def->models[0].packs, DG_COMDL_SEMITRANS, 1, 0 ) ;
    if ( comdl )
    {
	DG_QueueComdlObjs( comdl ) ;
	GM_GroupObject( comdl, work->where ) ;
	RotateTransMatrix( work, &comdl->pos[0].world ) ;

	/* 色の指定 */
	comdl->pos[0].color.vx = comdl->pos[0].color.vy =
	    comdl->pos[0].color.vz = comdl->pos[0].color.vw = 128 ;
    }
    return comdl ;
}
#endif

static int GetResources( Work *work, int hzd_id, int where,
			 FMATRIX *parent, FVECTOR *pos,
			 int bf_id, int af_id, int gl_id, int vl_id,
			 int *flag, int off_mask, int brk_mask, int flr_mask )
{
    int     i   ;
    DG_DEF *def ;

    ASSERT( where ) ;

    work->where  = where  ;
    work->parent = parent ;
    work->offset = pos    ;
    work->never_end = (irnd() & 0x1000) | 0x020 ;
    work->alpha     = 128 ;

    work->brk_flag = flag ;
    work->brk_mask = brk_mask ;
    work->off_mask = off_mask ;

    if ( !(work->hzd = BRK_InitHazard( hzd_id )) )
	PERROR( "No such Brk-Hazard-ID<%d> : NewFortLight\n", hzd_id ) ;

    if ( !(work->af_def = GV_GetCache( GV_CacheID( af_id, 'k' ) )) )
	PERROR( "No KMS<%d> in data.cnf!! : NewFortLight", af_id ) ;
    if ( !(work->bf_def = GV_GetCache( GV_CacheID( bf_id, 'k' ) )) )
	PERROR( "No KMS<%d> in data.cnf!! : NewFortLight", bf_id ) ;

    if ( !(*flag & brk_mask) )
    {
	work->flag   = 0 ;

	/* 割れているオブジェかどうか */
	def =  *flag & off_mask ? work->af_def : work->bf_def ;
	if ( !(work->objs = FRT_LGT_InitModel( work, def, NULL )) < 0 )
	    return -1 ;

	/* フォーチュンが狙う対象物である */
	FRT_AIM_AddAimSpot( &work->def_t, NULL, 0x0000,
			    (FVECTOR *)work->objs->world.m[W],
			    NULL, NULL, FRT_AIM_TypeHangLight ) ;

	if ( !(*flag & off_mask) )
	{
	    /* 光源計算用（独自影システム） */
	    FRT_LGT_AddLight( &work->pos ) ;

#if 0
	    /* 加算電灯モデル */
	    if ( !(def = GV_GetCache( GV_CacheID( gl_id, 'k' ) )) )
		PERROR( "No KMS<%d> in data.cnf!! : NewFortLight\n", gl_id ) ;
	    if ( !(work->gloom = FRT_LGT_InitGloomComdl( work, def )) )
		PERROR( "No Memory for Gloom : NewFortLight" ) ;
#endif

	    /* 加算電灯モデル */
	    work->gloom = BRK_UTL_MakePOLY( 1,
					    15821036 /*w11b2_railit_hl_add_alp*/ ,
					    SCE_GS_SET_ALPHA(0,2,0,1,0),
					    0x7f7f7f7f ) ;
	    if ( !work->gloom )
		PERROR( "No Prim(no memory) : NewFortLight\n" ) ;
	    work->gloom->root = &work->objs->world ;
	    DG_SetPrim2Buffer( work->gloom,
			       LightShape, LightShape,
			       work->gloom->uvrgb[0], work->gloom->uvrgb[1] ) ;


	    /* 傘型ボリュームモデル */
	    if ( !(def = GV_GetCache( GV_CacheID( vl_id, 'k' ) )) )
		PERROR( "No KMS<%d> in data.cnf!! : NewFortLight\n", vl_id ) ;
	    if ( !(work->volume = FRT_LGT_InitModel( work, def, NULL )) < 0 )
		PERROR( "No Memory for Volume : NewFortLight" ) ;

	    /* ボンボリプリム */
	    if ( !(*flag & flr_mask) )
	    {
		work->prim = BRK_UTL_MakePOLY( 1,
					       9776993 /*drop01_alp2_mod1021*/,
					       SCE_GS_SET_ALPHA(1,2,0,1,0),
					       0x3f707f7f ) ;
		if ( !work->prim )
		    PERROR( "No Prim(no memory) : NewFortLight\n" ) ;
		work->prim->raise =-300 ;
	    }

	    work->volume_p = BRK_UTL_MakeSPRTWH( FRT_LGT_N_VOLS,
						 12296685 /*light10_msk*/ ,
						 SCE_GS_SET_ALPHA(0,2,0,1,0),
						 600.0f,
						 0x1f707f7f ) ;
	    if ( !work->volume_p )
		PERROR( "No Prim(no memory) : NewFortLight\n" ) ;
	    work->volume_p->root = &work->objs->world ;
	    for( i=FRT_LGT_N_VOLS ; --i>=0 ; )
	    {
		static FVECTOR v = { 0,-100.0f,0,1.0f } ;
		v.vz = (i - FRT_LGT_N_VOLS/2) * FRT_LGT_VOL_LENG +
		    FRT_LGT_VOL_LENG*0.5f ;
		_sceVu0CopyVector( &work->volume_p->pos[0][i], &v ) ;
		_sceVu0CopyVector( &work->volume_p->pos[1][i], &v ) ;
	    }
	}
	/* ターゲットの初期化 */
	if ( FRT_LGT_InitTarget( work, work->objs->objs, where ) < 0 )
	    return -1 ;
    }
    else 
	work->flag = 3 ;

    if ( *flag & (brk_mask|off_mask) )
	DG_DestroyLightSphere( &work->pos, 2000.0f ) ;

    _sceVu0CopyVector( &work->pos_v, &DG_ZeroVector ) ;

#if 0
    if ( HZX_LevelHazardCheck( GM_GetHzxGroupID(where), pos,
			       HZX_CHK_ALL, HZX_FLOOR_ALL ) )
    {
	float floor[2] ;

	HZX_GetLevelHeight( floor ) ;
printf( "HZX_LevelHazardCheck %f\n", floor[0] ) ;
	work->floor = floor[0]+8.0f ;
    }
#else
    work->floor = -45000.f + 8.0f ;
#endif

    return 0 ;
}

void *NewFortLight( int hzd_id, int where,
		    FMATRIX *parent, FVECTOR *pos,
		    int bf_id, int af_id, int gl_id, int vl_id,
		    int *flag, int off_mask, int brk_mask, int flr_mask )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
        if ( GetResources( work, hzd_id,
			   where, parent, pos,
			   bf_id, af_id, gl_id, vl_id,
			   flag, off_mask, brk_mask, flr_mask ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
