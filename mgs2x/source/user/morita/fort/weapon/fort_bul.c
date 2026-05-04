//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   fort_bul.c
   銃の弾

   1999/07/19 M.Sonoyama
   2000/02/08 T.Morita
   $Id: fort_bul.c,v 1.1.1.3 2002/11/19 11:46:18 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
// for DG-LIB
#include "../../include/libdg_x.h"
#include "../../include/util.h"


#define	TARGET_CLASS	(TARGET_OFFENSE | TARGET_ONLINE | TARGET_ONLINE_MIN | TARGET_CHILD)

#define FRT_BUL_PRIM_FLG  (DG_PRIM2_LINE        |\
			   DG_PRIM2_SHADE       |\
			   DG_PRIM2_ANTIALIASING|\
			   DG_PRIM2_TEX         |\
			   DG_PRIM2_ALPHA)
#define FTOI12(_f)      ( DG_FTOI( ( (float)(_f)*4096.0f) ) )

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

/*------------------------------------------------------------*/

extern	void	 NewWallScar( FMATRIX *, HZX_SEG *seg, HZX_FLR *flr ) ;
extern	void	 NewSpark( FMATRIX * ) ;
extern  FVECTOR *FRT_MiscBulletPos() ;
extern void *NewLinerGunPlasma( FVECTOR *pos0, FVECTOR *pos1, int life );
extern int OK_BlowLineDataCounter;
extern void *NewBlowLine( int counter, FVECTOR *pos, FVECTOR *before_pos, int flag );
extern void *NewSpotBlurLine( FVECTOR *pos, FVECTOR *before_pos );

extern int FRT_BulletEnable ;

/*------------------------------------------------------------*/

typedef struct	{
    GV_ACT_EX		actor ;
    CONTROL		control ;

    TARGET		attack ;
    POWER_TARGET	power ;

    FVECTOR		to ;

    DG_OBJS	       *bullet ;
    DG_PRIM2	       *prim ;
    u_short		time ;
    u_short             time2 ;
    u_short		flag ;

    HZX_FLR		seg ;
    u_int		atr ;
    u_int		seg_flag ;

	FVECTOR		keep_pos;
	FVECTOR		before_pos;

} Work ;


extern void BIG_VectoMat( FMATRIX *, FVECTOR *,FVECTOR *, int ) ;
extern void NewFortWallScar( FMATRIX *dir, HZX_SEG *seg, HZX_FLR *flr ) ;
/*------------------------------------------------------------*/

static void *CallSpark( FMATRIX *m, FVECTOR *v )
{
    extern void	NewSpark( FMATRIX * ) ;
    FMATRIX	world ;
    FVECTOR	pos ;

    GV_MatToVec( m, &pos ) ;

    //GM_SeSetMode( SD_W_RICOCH02, &pos, GM_SEMODE_BOMB ) ;

    DG_ReflectMatrix( v, m, &world ) ;
    GV_VecToMat( &pos, &world ) ;
    NewSpark( &world ) ;

    return m ;
}


/* 跳弾 */
static inline void CallEffect( Work *work )
{
    HZX_SEG	*seg = NULL ;
    HZX_FLR	*flr = NULL ;
    FMATRIX	mat ;
    FVECTOR	pole ;

    if ( work->seg_flag == 0 )
	return ;
    if ( work->seg_flag == 2 )	/* 床 */
    {
	if ( work->atr & HZX_FLOOR_NO_RECOIL )
	    return ;
	flr = ( HZX_FLR * )&work->seg ;
	pole.vx = flr->p1.h ;
	pole.vy = flr->p3.h ;
	pole.vz = flr->p2.h ;
    }
    else                        /* 壁 */
    {
	if ( work->atr & HZX_SEG_NO_RECOIL )
	    return ;
	seg = ( HZX_SEG * )&work->seg ;
	pole.vx = seg->p2.z - seg->p1.z ;
	pole.vy = 0.0F ;
	pole.vz = seg->p1.x - seg->p2.x ;
    }
    _sceVu0Normalize( &pole, &pole ) ;

    BIG_VectoMat( &mat, &work->control.step, &work->to, 0x0012 ) ;
    if ( seg )
	NewFortWallScar( &mat, seg, NULL ) ;
    CallSpark( &mat, &pole ) ;
}

static void Act( Work *work )
{
    FVECTOR from, to, *pos ;
    FVECTOR fvtemp ;

    /* 当たったので終了 */
    if ( work->flag == 1 )
    {
	/* 当たり終ったらフラグを戻す */
	FRT_BulletEnable = 0 ;

	/* 跳弾エフェクト等に渡すマップ */
	GM_SetCurrentMap( GM_CurrentStageMap ) ;
	CallEffect( work ) ;
	GV_DestroyActor( work ) ;
	return ;
    }

    from = work->control.mov ;

    DG_COPY_VEC( &fvtemp, &from );// added by okajima 2001/01/29

    GM_ActControl( &work->control ) ;
    to   = work->control.mov ;
    GM_MoveOnlineTarget( &work->attack, &from, &to ) ;
    GM_PutTarget( &work->attack ) ;

    if ( work->control.n_touches > 0 )
    {
	/* 動的ハザードに当たった */
	HZX_GetOnlineHazard( &work->seg, &work->atr ) ;
	HZX_GetOnlinePoint( &to ) ;
	work->seg_flag = HZX_GetOnlineHazardType() ;
	work->flag = 1 ;
    }
    else if ( --work->time <= 0 )
    {
	/* 最長到達点 */
	to = work->to ;
	work->flag = 1 ;
    }


    if ( GM_StagePlayTime & 1)/* すみませんちょっと半分にさせて頂きます 2001.08.07 T.Morita */
    {
	NewSpotBlurLine( &to, &fvtemp );

	NewBlowLine( OK_BlowLineDataCounter, &to, &fvtemp, (int)work->flag );
    }
	if( work->flag ){
		NewLinerGunPlasma( &work->keep_pos, &to, 128 );
	}else{
	    if ( GM_StagePlayTime & 1 )/* すみませんちょっと半分にさせて頂きます 2001.08.07 T.Morita */
		NewLinerGunPlasma( &work->keep_pos, &to, 16 );
	}
	DG_COPY_VEC( &work->before_pos, &to );



#if 0
    {
	extern void *NewFortWeaponLight( FVECTOR *pos, float r_range, float e_range,
					 int r,int g,int b,int a, int step ) ;
	NewFortWeaponLight( &to, 1000.0f, 4000.0f, 0x0f, 0xff, 0xff, 0x3f, 4 ) ;
    }
#else
    //DG_SetTmpLight2( &to, 1000.0f, 4000.0f, 0x3fffff0f, LIT_FLAG_BGONLY ) ;
    DG_SetTmpLight2( &to, 1000.0f, 4000.0f, 0x3fffff0f, LIT_FLAG_CHARAONLY ) ;
#endif
    BIG_VectoMat( &work->bullet->world, &work->control.step, &to, 0x0012 ) ;

    if ( !GM_CheckGameStatus( STATE_PLAY_DEMO ) )
    {
	pos = FRT_MiscBulletPos() ;
	pos->vx = to.vx ;
	pos->vy = to.vy ;
	pos->vz = to.vz ;
    }
}

static void Die( Work *work )
{
    extern void OK_RemoveDynamicLight( FVECTOR *pos ) ;

    OK_RemoveDynamicLight( (FVECTOR *)work->bullet->world.m[W] ) ;
    if ( work->prim )
	GM_FreePrim2( work->prim ) ;
    if ( work->bullet )
	DG_DequeueObjs( work->bullet ), DG_FreeObjs( work->bullet ) ;
    GM_FreeControl( &work->control ) ;
}

/*------------------------------------------------------------*/

static int InitControl( Work *work, FMATRIX *world, u_int length, u_int speed, u_int size )
{
    float    len   ;
    CONTROL *ctrl  ;
    FVECTOR  to    ;

    ctrl  = &work->control ; 
    if ( GM_InitControl( ctrl, 0, GM_CurrentStageMap ) < 0 )
	return -1 ;
    ctrl->mov.vx = world->m[ 3 ][ 0 ] ;
    ctrl->mov.vy = world->m[ 3 ][ 1 ] ;
    ctrl->mov.vz = world->m[ 3 ][ 2 ] ;
    ctrl->rot = ctrl->turn = DG_ZeroSVector ;

    /* ステップ値を計算 */
    to.vx = to.vz = 0.0F ;
    to.vy = -( float )speed ; 
    DG_SetPos( world ) ;
    DG_RotVector( &to, &to, 1 ) ;
    GV_LenVec3F( &to, &ctrl->step, GV_VecLen3F( &to ), ( float )speed ) ;
    /* 到達点を予め計算 */
    to.vx = to.vz = 0.0F ;
    to.vy = -( float )length ; 
    DG_PutVector( &to, &to, 1 ) ;

    if ( ( work->seg_flag = HZX_OnlineHazardCheck( ctrl->hzx_id, &ctrl->mov, &to, 
						   HZX_CHK_F_SEGMENT | HZX_CHK_F_FLOOR, 
						   HZX_SEG_NO_BULLET/* | HZX_SEG_RECOIL_TYPE*/, 
						   HZX_FLOOR_NO_BULLET/* | HZX_FLOOR_RECOIL_TYPE*/ ) ) )
    {
	HZX_GetOnlineVector( &to ) ;
	len = GV_VecLen3F( &to ) ;
	HZX_GetOnlinePoint( &( work->to ) ) ;
	HZX_GetOnlineHazard( &work->seg, &work->atr ) ;
    }
    else
    {
	len = ( float )length ;
	work->to = to ;
	work->atr = 0 ;
    }

    work->time = ( int )( len / ( float)speed ) + 1 ;
    /* 動的ハザードのみをチェックする */
    GM_ConfigControlHazard( ctrl, size, size, size * 2 ) ;
    ctrl->hzx_check_type = /*HZX_CHK_D_SEGMENT |*/ HZX_CHK_D_FLOOR ;
    ctrl->seg_flag |= HZX_SEG_NO_BULLET ;
    ctrl->flr_flag |= HZX_FLOOR_NO_BULLET ;
    /* onlineチェックのみ行う */
    ctrl->skip_flag |= CTRL_SKIP_NEAR_CHECK | CTRL_SKIP_FLR_CHECK ;

    return 0 ;
}

static void BulletTargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
	Work *work = (Work *)ptr;

	NewLinerGunPlasma( &work->keep_pos, &def->hit, 128 );
	NewBlowLine( OK_BlowLineDataCounter, &work->before_pos, &def->hit, 1 );

    GV_DestroyActor( work ) ;
}
static int InitTarget( Work *work, u_int side, u_int size, u_int damage, u_int weapon )
{
    TARGET	 *t = &work->attack ;
    POWER_TARGET *p = &work->power  ;
    FVECTOR	  vec ;

    vec.vx = vec.vy = vec.vz = ( float )size ;
    GM_SetTarget( t, TARGET_CLASS, GM_CurrentStageMap, side, &vec, &DG_ZeroVector ) ;
    GM_SetTargetWeaponType( t, I64(1) << weapon ) ;
    _sceVu0ScaleVector( &vec, &work->control.step, 0.2f ) ;
    GM_SetPowerTarget( t, p, POWER_ONCE, 255, 0, damage, &vec ) ;
    GM_SetTargetCallBack( t, BulletTargetCallBack, work ) ;

    return 0 ;
}
static int InitModel( Work *work )
{
    DG_DEF  *def = GV_GetCache( GV_CacheID( GV_StrCode( "lnr_bul" ), 'k' ) ) ;

    if ( !def )
	PERROR( "No Model<lnr_bul.kms> : NewFortBullet" ) ;
    if ( !(work->bullet = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 )) )
	PERROR( "Cannot make OBJS(maybe no memory) : NewFortBullet" ) ;
    DG_QueueObjs( work->bullet ) ;

    return  0 ;
}

/*------------------------------------------------------------*/

static int GetResources( Work *work, FMATRIX *world, u_int type, u_int side, u_int size,
			 u_int damage, u_int length, u_int speed, u_int weapon )
{
    extern void OK_SetDynamicLight( FVECTOR *pos, SVECTOR *rot ) ;

    if ( InitControl( work, world, length, speed, size ) < 0 )
	return -1 ;
    if ( InitTarget( work, side, size, damage, weapon ) < 0 )
	return -1 ;
    if ( InitModel( work ) )
	return -1 ;

    /* 弾が生きていることを示すフラグを立てる */
    FRT_BulletEnable = 1 ;

    //OK_SetDynamicLight( (FVECTOR *)work->bullet->world.m[W], &DG_ZeroSVector ) ;


	DG_COPY_VEC( &work->keep_pos, (FVECTOR *)world->m[3] ); // added by okajima 2001/02/01

	OK_BlowLineDataCounter++;

    return 0 ;
}

/* 弾起動 */
void	*NewFortBullet( FMATRIX *world, u_int type, u_int side, u_int size, 
			u_int damage, u_int length, u_int speed, int weapon )
{
    Work *work ;

    work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
    if ( work != NULL )
    {
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	if ( GetResources( work, world, type, side, size, damage, length, speed, weapon ) < 0 )
	{
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }
    return work ;
}
