//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   fort_ceil.c 
   昇降機

   2001/01/16 T.Morita
   $Id: fort_ceil.c,v 1.1.1.3 2002/11/19 11:46:16 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libvu0.h>
#include <libdma.h>
#endif

#include  "gameheader.h"
#include  "camera.h"
#include  "libutl.h"

#include "../../include/libdg_x.h"
#include "../../include/util.h"
#include "../include/fort_flag.h"
#include "../../brk_utl/brk_utl.x"
#include "../../brk_hzd/brk_hazard.h"


enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

#define CEIL_N_BULLETS 5
#define CEIL_N_PARTS   3

typedef struct Work_t
{
    GV_ACT_EX actor ;

    FVECTOR      pos [CEIL_N_PARTS] ;/*瓦礫モデル位置 */
    FVECTOR      vel [CEIL_N_PARTS] ;/*瓦礫モデル位置 */
    OBJECT       body[CEIL_N_PARTS] ;/*瓦礫モデル     */

    TARGET       target[CEIL_N_BULLETS] ;
    POWER_TARGET power [CEIL_N_BULLETS] ;
    TARGET       off    ;
    POWER_TARGET p_off  ;

    int          name   ;
    int          where  ;
    HZD_BOX     *hzd    ;

    int       n_brk   ;
    int       brk_id  ;/*入れ換え天井モデルのID*/
    int       n_aim   ;/* 標的目標数 */
    int       proc    ;/*壊れた時に呼ばれるプロック*/
    int       flag    ;
    int       vibrate ;

    int       brk_tic ;

    DG_OBJS  *ceiling ;/*天井モデル*/
} Work ;

static Work *FRT_CEL_Work = NULL ;

extern int   FRT_LastCieling ; /* fort_var.c */

extern int   FRT_AIM_AddAimSpot( void *aim_id, void *nxt_id, int phase,
				 FVECTOR *center,
				 FVECTOR *front, FVECTOR *rear,
				 u_int flag ) ;
extern int   FRT_AIM_RemoveAimSpot( void *work ) ;
extern int   NewCeilingDust( DG_OBJS *world, int where, HZD_BOX *hzd,
			    int cv2_id, int kms_id, int n_dust,
			    FVECTOR *hit ) ;
extern void *NewFortFallDust( FVECTOR *pos, int n_dust, float gravity, int latency ) ;
extern int   FRT_LGT_GetMagnitudeCenter( FVECTOR *center, float *magnitude ) ;
extern void *NewBombGasEffect( FVECTOR *pos, SVECTOR *pole_rot ) ;
extern void *NewShakeCamera( int chanl, int intense, int time ) ;
extern void *NewFortVibrate( int base, int high, int low ) ;

#define FRT_PLY_CAMERAPOS  (GM_PlayerSubjectCamera[0]->position)

/* 落ちた時の攻撃ターゲットのパラメータ */
static FVECTOR OffenseSize = { 800.0f, 1000.0f, 800.0f, 0.0f } ;
static FVECTOR PushForce   = { 0.0f, 0.0f, 350.0f, 0.0f } ;


static void FreeModel( Work *work )
{
    if ( work->ceiling )
    {
	DG_DequeueObjs( work->ceiling ) ;
	DG_FreeObjs( work->ceiling ) ;
	work->ceiling = NULL ;
    }
}

static int InitModel( Work *work, int id, FVECTOR *pos )
{
    DG_DEF  *def ;
    DG_OBJS *objs ;

    /* 昇降機モデル */
    if ( !(def = GV_GetCache( GV_CacheID( id, 'k' ) )) )
	PERROR( "No Ceiling Model<%d> : NewFortCeiling\n", id ) ;
    if ( !( objs = DG_MakeObjs( def, DG_FLAG_PAINT|DG_FLAG_ONEPIECE, 0 )) )
	PERROR( "Cannot make OBJS(maybe no memory) : NewFortCeiling\n" ) ;
    DG_QueueObjs( objs ) ;
    _sceVu0CopyVectorXYZ( (FVECTOR *)objs->world.m[W], pos ) ;
    DG_MakePreshade( objs, GM_GetMap( work->where )->light ) ;
    /*フラグをコピー*/
    if ( work->ceiling )
	objs->flag = work->ceiling->flag ;

    /* モデルを解放する */
    FreeModel( work ) ;
    work->ceiling = objs ;

    return 0 ;
}



void BreakCeiling( Work *work  )
{
    TARGET *def = &work->target[--work->n_aim] ;

    NewCeilingDust( work->ceiling, work->where, work->hzd,
		    3545106/*w11c2_ceiling1_brk1*/,
		    6381674/*w11c2_ironbox_frg2_cm*/, 16, &def->hit ) ;

    FRT_AIM_RemoveAimSpot( def ) ;
    GM_FreeTarget( def ) ;

    if ( !work->n_aim )
    {
	InitModel( work, work->brk_id,
		   (FVECTOR *)work->ceiling->world.m[W] ) ;
    }
}


static void Die( Work *work )
{
    int  i ;

    FreeModel( work ) ;
    for ( i=CEIL_N_BULLETS ; --i>=0 ; )
    {
	FRT_AIM_RemoveAimSpot( &work->target[i] ) ;
	//GM_FreeTarget( &work->target[i] ) ;
    }
    for ( i=CEIL_N_PARTS ; --i>=0 ; )
	GM_FreeObject( &work->body[i] ) ;

    FRT_CEL_Work = NULL ;
}

static FVECTOR CeilingCenter = { 0.0f, -40000.0f, 0.0f, 1.0f } ;

static void Act( Work *work )
{
    GV_MSG *msg ;
    int i, j ;
    float mag ;

    /* 天井管理 */
    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
        switch ( msg->message[0] )
	{
	case 1:
            work->ceiling->flag &= ~DG_FLAG_INVISIBLE ;
            work->ceiling->flag |=  msg->message[1] ;
	    break ;
	}

    /* 揺れたら埃を出す */
    if ( FRT_LGT_GetMagnitudeCenter( &CeilingCenter, &mag ) )
	for ( i=4 ; --i>=0 ; )
	{
	    FVECTOR v = { 5000.0f*frnd(), -40000.0f, 3000.0f*frnd(), 1.0f } ;

	    for ( j=1 ; --j>=0 ; )
	    {
		v.vx += 100.0f*frnd() ;
		v.vz += 100.0f*frnd() ;

		NewFortFallDust( &v, 6, 2.0f+4.0f*rnd(),
				 (i&1)*32 + (j&1)*8 + (15 & irnd()) ) ;
	    }
	}

    /*強制壊し */
    if ( work->n_aim > 0 )
    {
	if ( work->brk_tic > 0 )
	{
	    if ( --work->brk_tic == 0 )
		BreakCeiling( work ) ;
	}
	else if ( work->n_aim != 3 - FRT_LastCieling )
	    work->brk_tic = 14 ;/* 14フレーム後に当たった処理 */
    }

    /* 瓦礫が落ちる処理 */
    else if ( work->n_aim == 0 )
    {
	int flag = MT3_SLEEP ;
	int start[] = { 50, 155, 190 } ;

	for ( i=work->n_brk ; --i>=0 ; )
	{
	    GM_ActMotion( &work->body[i] ) ;
	    work->pos[i].vx += work->vel[i].vx ;
	    work->pos[i].vy += work->vel[i].vw ;
	    work->pos[i].vz += work->vel[i].vz ;
	    DG_SetPos2( &work->pos[i], &DG_ZeroSVector ) ;
	    GM_ActObject2( &work->body[i] ) ;

	    if ( MT_CHECK_MOTION_TIME( work->body[i].m_ctrl, 0, start[i] ) ||
		 !(work->ceiling->flag & DG_FLAG_INVISIBLE0) )
		DG_VisibleObjs( work->body[i].objs ) ;

	    /* 再生が終ればモーションを眠らす */
	    if( GM_CheckObject_IsEnd( &work->body[i], 0 ) )
		work->body[i].m_ctrl->mt3_ctrl->flag |= MT3_SLEEP ;
	    flag &= work->body[i].m_ctrl->mt3_ctrl->flag ;

	    /* 地面についた物をだけをチェックする */
	    if ( !(work->flag & (1 << i)) )
		if ( work->pos[i].vy < -44000.0f )
		{
		    /* 攻撃ターゲットを置く */
		    GM_MoveTargetMap( &work->off, &work->pos[i], work->where );
		    GM_PutTarget( &work->off );

		    /* 埃の音 */
		    GM_SeSetMode( (i&1 ? SD_A_TENPARA2 : SD_A_TENPARA1),
				  &work->pos[i], GM_SEMODE_BOMB ) ;

		    /* ドシーーンと音がなる */
		    GM_SeSetMode( (i&1 ? SD_A_TENOTI2 : SD_A_TENOTI1),
				  &work->pos[i], GM_SEMODE_BOMB ) ;

		    /* ガスのエフェクトを出す */
		    NewBombGasEffect( &work->pos[i], &DG_ZeroSVector ) ;
		    /* カメラを揺らす */
		    NewShakeCamera( 0, 512, 60 ) ;

		    /* 地面についたのでチェックを省くためにビットを立てる */
		    work->flag |= (1 << i) ;
		    NewFortVibrate( 255-64, 12*5/TIME_BASE, 40*5/TIME_BASE ) ;
		    //work->vibrate = 40*5/TIME_BASE ;
		}
	}

	if ( flag & MT3_SLEEP )
	{
	    work->n_aim = -1 ;/* もう処理しない */
	    if ( work->proc )
		GCL_ExecProc( work->proc, NULL ) ;
	}
    }

#if 0
    /* 振動するか */
    if ( work->vibrate > 0 )
    {
	work->vibrate-- ;
	GM_SetVibration1( 0, 1 );
	GM_SetVibration2( 0, 192 + (irnd()&63) );
    }
#endif

}


#if 0
static void FRT_CEL_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work *work = (Work *)ptr ;

    if ( def->damaged & TARGET_POWER )
    {
	GM_ClearTargetDamage( def ) ;

	if ( off->weapon_type & WP_NIKITA )
	    BreakCeiling( work ) ;
    }
}
#endif


static int GetResources( Work *work, int name, int where )
{
    FVECTOR pos ;
    int     buf[XYZW] ;
    int     hzd_id, model_id ;
    int    i ;
    static FVECTOR t_size = { 100.0f, 100.0f, 100.0f, 0.0f } ;
    FVECTOR t_pos ;

    work->name   = name  ;
    work->where  = where ;
    work->brk_id = GV_StrCode( "w11c2_ceiling1_brk" ) ;
    model_id     = GV_StrCode( "w11c2_ceiling1"     ) ;
    hzd_id       = BRK_HZD_W11B_FORTUNE ;

    work->n_aim = 0 ;
    work->n_brk = 0 ;
    while( (i = GCL_GetNextOption()) )
        switch( i )
	{
	    TARGET       *t ;
	    POWER_TARGET *p ;
	    OBJECT       *o ;

	case 'a':    /* 的を設定する */
	    GCL_GetIV( GCL_NextStr(), buf ) ;
	    vu0_IV0toFV( (IVECTOR *)buf, &t_pos ) ;

	    t = &work->target[work->n_aim] ;
	    p = &work->power[work->n_aim] ;

	    if ( FRT_AIM_AddAimSpot( t, NULL, 0x0f00+work->n_aim,
				     &t_pos, NULL, NULL, FRT_AIM_TypeCeiling ) < 0 )
		PERROR( "Cannot Add aimming spot :NewFortCeiling\n" ) ;

	    GM_SetTarget( t, TARGET_DEFENSE, where, BOTH_SIDE, &t_size, &t_pos ) ;
#if 0
	    GM_SetTargetCallBack( t, FRT_CEL_TargetCallBack, work ) ;
	    GM_SetPowerTarget( t, p, POWER_ONCE, GM_Vitality, 0, 0, &DG_ZeroVector ) ;
	    GM_PutTarget( t ) ;
	    //NewTargetView( t, 200, 50, 32 ) ;
#endif

	    work->n_aim++ ;
	    break ;

	case 'h': /*壊れ物を設定する*/
	    hzd_id = GCL_GetNextInt() ;
	    break ;

	case 'b': /*壊れ物を設定する*/
	    o = &work->body[work->n_brk] ;
	    GM_InitObject( o, GCL_GetNextInt(), DG_FLAG_SHADE|DG_FLAG_FINISHCALC ) ;
	    GM_ConfigObjectMotion( o, 0, GCL_GetNextInt(), MT_FLAG_OBJECT ) ;
	    GM_ConfigObjectStep( o, &work->vel[work->n_brk] ) ;
	    GM_ConfigObjectAction( o, 0, GCL_GetNextInt(), 0,0xfffff,0 ) ;
	    DG_InvisibleObjs( o->objs ) ;

	    GCL_GetIV( GCL_NextStr(), buf ) ;
	    vu0_IV0toFV( (IVECTOR *)buf, &t_pos ) ;
	    _sceVu0CopyVector( &work->vel[work->n_brk], &DG_ZeroVector ) ;
	    _sceVu0CopyVector( &work->pos[work->n_brk], &t_pos ) ;
	    _sceVu0CopyVectorXYZ( (FVECTOR *)o->objs->world.m[W], &t_pos ) ;
	    work->n_brk++ ;
	    break ;

	case 'p':    /* 位置 */
	    GCL_GetIV( GCL_NextStr(), buf ) ;
	    vu0_IV0toFV( (IVECTOR *)buf, &pos ) ;
	    break ;

	case 'm':    /* デフォルトの壊れモデル */
	    model_id     = GCL_GetNextInt() ;
	    work->brk_id = GCL_GetNextInt() ;
	    break ;
	}
    work->proc = GCL_GetOptionValue( 'O', 0 ) ;
    InitModel( work, model_id, &pos ) ;
    work->hzd = BRK_InitHazard( hzd_id ) ;
    work->flag    = 0 ;
    work->vibrate = 0 ;
    work->brk_tic = 0 ;

    GM_SetTarget( &work->off, TARGET_OFFENSE|TARGET_POWER,
		  where, PLAYER_SIDE, &OffenseSize, &DG_ZeroVector ) ;
    GM_SetTargetWeaponType( &work->off, WP_PUNCH ) ;
    GM_SetPowerTarget( &work->off, &work->p_off, POWER_DECREASE, GM_Vitality,
		       20, 20, &PushForce ) ;
    return 0 ;
}

void *NewFortCeiling( int name, int where  )
{
    Work *work ;

    FRT_LastCieling = 0 ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( work != NULL )
    {
	if ( GM_CheckGameStatus( STATE_PLAY_DEMO ) )
	    GV_SetActor( &work->actor, NULL, Die ) ;
	else
	    GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
        if ( GetResources( work, name, where ) < 0 )
	{
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    FRT_CEL_Work = work ;
    return (void *)work ;
}

int NewCom_FRT_CEL_Control()
{
    if ( FRT_CEL_Work )
	if ( GCL_NextStr() )
	{
	    FRT_CEL_Work->ceiling->flag &= ~DG_FLAG_INVISIBLE ;
	    FRT_CEL_Work->ceiling->flag |=  GCL_GetNextInt()  ;
	    return  1 ;
	}
    return  0 ;
}
