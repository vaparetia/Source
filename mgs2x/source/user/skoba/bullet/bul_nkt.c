//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   bul_nkt.c
   発射されたニキータミサイル
   
	2000/02/25 M.Sonoyama
	2000/07/25 S.Okajima
   $Id: bul_nkt.c,v 1.1.1.3 2002/11/19 11:50:01 Yoshizawa1 Exp $
*/

#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif


#include <libutl.h>
#include	"../../okajima/etc/ok_util.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include	"gameheader.h"
#include	"camera.h"
#include	"libmt.h"

extern int gBP_NikitaMissilesCheat;

/*------------------------------------------------------------*/

extern	void	*NewBlast( FVECTOR *, int, int, int, int, int, int ) ;

#define	BODY_NAME	(7429929)	/* nkt_msl */
#define	BODY_FLAG	(DG_FLAG_SHADE | DG_FLAG_ONEPIECE)

#define	BURST_TIME	DIRECT_TICK(48)
extern float BP_AdjustTick3(float);
#define	SPEED_LOW	(BP_AdjustTick3(24.0F))
#define	SPEED_HIGH	(BP_AdjustTick3(72.0F))

#define	CAMERA_NAME	(1471751)	/* ニキータ主観カメラ */
#define	PAD_SUBJECT	PAD_R1


#define	RAISE					(0)
#define	RAISE_SHIFT_WORLD		(20.0f)

#define	DUMMY_SIZE		(0)
#define	DUMMY_COLOR		(32)
#define	DUMMY_ALPHA		(0)

#define	COL_R			(32)
#define	COL_G			(24)
#define	COL_B			(8)

#define	N_PRIMS			(1)
#define	N_VERTS			(32)
#define	N_VP			(N_PRIMS * N_VERTS)

#define	SIZE_BASE		(10)
#define	SIZE_STEP		(3)

#define	LIGHT_LEN		(300.0f)

#define	ALPHA_BASE			(96.0f)
#define	ALPHA_MIN			(16)

static FVECTOR s_pos[N_VP];
static FVECTOR ShiftMain    = {    0.0f,  280.0f,    0.0f, 0.0f };
//static FVECTOR ShiftSub0    = {   50.0f,    0.0f,   30.0f, 0.0f };
//static FVECTOR ShiftSub1    = {  -50.0f,    0.0f,   30.0f, 0.0f };
//static FVECTOR ShiftBonbori = {    0.0f,  200.0f,  150.0f, 0.0f };
//static SVECTOR RotMain      = {    0,    0,   0 };
//static SVECTOR RotSub0      = {    0, 1024,   0 };
//static SVECTOR RotSub1      = {    0,-1024,   0 };
//static SVECTOR RotBonbori   = {    0,    0,   0 };

enum {
	FLAG_NORMAL = 0x0000,
	FLAG_WATCH =  0x0001,
	FLAG_INTERP_IN = 0x0002,
	FLAG_INTERP_OUT = 0x0004,
	FLAG_HIT =	  0x0008,
	FLAG_CANNOT_OPERATE = 0x0010,
	FLAG_FLASH_TRAP = 0x0020,
	FLAG_NO_CONTROL = 0x0040, // 範囲外
	FLAG_NO_CONTROL2 = 0x0080, // 範囲外
} ;

#define	FLAG_INTERP	(FLAG_INTERP_IN|FLAG_INTERP_OUT)

typedef struct _Work {
	GV_ACT		actor ;
	CONTROL		control ;
	OBJECT		body ;
	FMATRIX		lights[ 2 ] ;
	FMATRIX		world ;
	FVECTOR		speed ;
	FVECTOR		camera ;
	TARGET		off ;
	TARGET		def ;
	GM_BOMB		list ;

	CONTROL		*ctrl ;
	GV_PAD		*pad ;
	int			chanl ;
	int			side ;
	int			phase ;
	int			flag ;
	int			burst ;
	int			count ;
	int			no_countrol_life ; // 制御不能時のライフ
	int         pad_data_x; // 1frame前の情報
	int         pad_data_y; // 1frame前の情報
	GM_CameraSet	*subject ;

	DG_PRIM2	*prim_main;
	DG_PRIM2	*prim_sub0;
	DG_PRIM2	*prim_sub1;
	DG_PRIM2	*prim_bonbori;

#ifdef DEBUG_MODE
	int			sw ;
#endif
} Work ;


static	Work	*NikitaWork = NULL ;


// プロトタイプ
extern void *NewSubMazzleFlushMngNormal( int , FMATRIX * , FVECTOR * , FVECTOR * , int , float , float , int );
extern void *NewSubMazzleFlushMngRed( int , FMATRIX * , FVECTOR * , FVECTOR * , int , float , float , int );
extern void *NewSmokeNormal( int , FMATRIX *, FVECTOR *, FVECTOR * , int , float , FVECTOR * , float );
// koba4
static void SK_CamAutoMove( Work * );

/*---------------------------------------------------------------*/

static	void	Hit( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
	Work	*work ;

	/* 中心間チェック */
	if ( GM_TargetCheckCenter2Center( off, def, HZX_CHK_ALL, 
									  HZX_SEG_NO_MISSILE | HZX_SEG_RECOIL_TYPE,
									  HZX_FLOOR_NO_MISSILE | HZX_FLOOR_RECOIL_TYPE ) ) {
		GM_TargetHitCancel( off, def ) ;
		return ;
	}

	//printf( "ouch!!!!!!!!!!\n" ) ;
	work = ( Work * )ptr ;
	work->flag |= FLAG_HIT ;
}

static	void	Hitted( TARGET *off, TARGET *def, void *ptr )
{
	Work		*work ;

	/* 中心間チェック */
	if ( GM_TargetCheckCenter2Center( off, def, HZX_CHK_ALL, 
									  HZX_SEG_NO_MISSILE | HZX_SEG_RECOIL_TYPE,
									  HZX_FLOOR_NO_MISSILE | HZX_FLOOR_RECOIL_TYPE ) ) {
		GM_TargetHitCancel( off, def ) ;
		return ;
	}

	work = ( Work * )ptr ;
	work->flag |= FLAG_HIT ;
}

/*---------------------------------------------------------------*/

static	int		AdjustCameraRotX( Work *work, int rx )
{
	HZX_HZD		*flr ;
	float		max, min, d, y ;

	y = work->control.mov.vy + 250.0F ; // -
	rx = GV_DiffDirS( 0, rx ) ;
	if ( work->control.grounded & 1 ) {
		flr = work->control.level[ 0 ] ;
		if ( flr->attribute & HZX_FLOOR_FLAT ) return 0 ;
		max = min = flr->p1.y ;
		if ( max < flr->p2.y ) max = flr->p2.y ;
		if ( max < flr->p3.y ) max = flr->p3.y ;
		if ( max < flr->p4.y ) max = flr->p4.y ;
		if ( min > flr->p2.y ) min = flr->p2.y ;
		if ( min > flr->p3.y ) min = flr->p3.y ;
		if ( min > flr->p4.y ) min = flr->p4.y ;
		d = max - min ;
		if ( ( max - y < d * 0.30F ) ||
			( y - min < d * 0.30F ) ) {
			return rx / 20 ;
		}
		return rx ;
	} else if ( work->control.grounded & 2 ) {
		flr = work->control.level[ 1 ] ;
		if ( flr->attribute & HZX_FLOOR_FLAT ) return 0 ;
		max = min = flr->p1.y ;
		if ( max < flr->p2.y ) max = flr->p2.y ;
		if ( max < flr->p3.y ) max = flr->p3.y ;
		if ( max < flr->p4.y ) max = flr->p4.y ;
		if ( min > flr->p2.y ) min = flr->p2.y ;
		if ( min > flr->p3.y ) min = flr->p3.y ;
		if ( min > flr->p4.y ) min = flr->p4.y ;
		d = max - min ;
		if ( ( max - y < d * 0.30F ) ||
			( y - min < d * 0.30F ) ) {
			return rx / 20 ;
		}
		return rx ;
	}
	return 0 ;
}

// コバ４追加
#define RANGE (ANALOG_MARGIN)
static void AnarogLevel( short *pData ) // 遊び
{
	if ( *pData < RANGE && *pData> -RANGE ){
		*pData = 0;
	} else {
		if ( *pData < 0 ){
			*pData += RANGE;
		} else {
			*pData -= RANGE;
		}
	}
}

static void AdjustCameraRot( Work *pWork )
{
	short level_x;
	short level_y;
	float	dummy = 0.0F ;

	// アナログ情報
	if ( pWork->flag & ( FLAG_NO_CONTROL | FLAG_NO_CONTROL2 ) ){
		level_x = 0;
		level_y = 0;
	} else {
		level_x = GV_PadData[ 0 ].right_dx - 128;
		AnarogLevel( &level_x );
		level_y = GV_PadData[ 0 ].right_dy - 128;
		AnarogLevel( &level_y );
		PL_ShukanReverse( &dummy, &level_y ) ;
	}
	pWork->subject->rotate.vx = pWork->control.rot.vx + 1024 + ( level_y * 6 );
	pWork->subject->rotate.vy = pWork->control.rot.vy - ( level_x * 6 );

	if ( pWork->subject->rotate.vy > pWork->control.rot.vy + 843 ){
		pWork->subject->rotate.vy = pWork->control.rot.vy + 843;
	}
	if ( pWork->subject->rotate.vy < pWork->control.rot.vy - 843 ){
		pWork->subject->rotate.vy = pWork->control.rot.vy - 843;
	}
	if ( level_x == 0 ){
		// 補間
		if ( pWork->pad_data_x != level_x ){
			pWork->subject->rotate.vy += ( pWork->control.rot.vy - pWork->subject->rotate.vy ) / 20;
		} else {
	   		pWork->subject->rotate.vy = pWork->control.rot.vy;
		}
	}
	if ( level_y == 0 ){
		if ( pWork->pad_data_y != level_y ){
			pWork->subject->rotate.vx += ( ( pWork->control.rot.vx + 1024 ) - pWork->subject->rotate.vx ) / 20;
		} else {
			pWork->control.rot.vx = pWork->subject->rotate.vx - 1024;
		}
	}
//	printf("%d %d\n" , pWork->subject->rotate.vx  , pWork->subject->rotate.vy );
	pWork->pad_data_x = level_x;
	pWork->pad_data_y = level_y;
}

/*---------------------------------------------------------------*/

static	inline	void	IntoSubject( work )
Work		*work ;
{
	work->flag |= FLAG_WATCH | FLAG_INTERP_IN ;
	if ( work->subject->on == 0 ) {
		work->subject->on = 1 ;
		GM_ChangeCamera( work->chanl ) ;
	}
}

static	inline	void	LeaveSubject( work )
Work		*work ;
{
	work->flag &= ~FLAG_WATCH ;
	work->flag |= FLAG_INTERP_OUT ;
	if ( work->subject->on ) {
		work->subject->on = 0 ;
		GM_ChangeCamera( work->chanl ) ;
	}
}

#if 0
static	void	CheckWatch( work ) 
Work		*work ;
{
//	if ( work->pad->status & PAD_SUBJECT ) {
	if ( work->count <= GM_NikitaLifeMax - 32 ) {
		if ( !( work->flag & FLAG_WATCH ) ) IntoSubject( work ) ;
	} else {
		if ( work->flag & FLAG_WATCH ) LeaveSubject( work ) ;
	}
}
#endif

static	void	CheckVWait( work )
Work		*work ;
{
	float		len ;
	GM_CameraSet	*cam1, *cam2 ;
	int			flag ;

	flag = work->flag ;
	if ( !( flag & FLAG_INTERP ) ) return ;
	if ( flag & FLAG_INTERP_IN ) {
		cam1 = GM_GetCurrentCamera( work->chanl ) ;
		cam2 = work->subject ;
		len = GV_VecLen3F2( &cam1->position, &cam2->position ) ;
		if ( len < 450.0F ) {
			work->flag &= ~FLAG_INTERP_IN ;
			DG_InvisibleObjsChanl( work->body.objs, work->chanl ) ;
		}
	} else {
		cam1 = work->subject ;
		cam2 = GM_GetNextCamera( work->chanl ) ;
		len = GV_VecLen3F2( &cam1->position, &cam2->position ) ;
		if ( len > 450.0F ) {
			work->flag &= ~FLAG_INTERP_OUT ;
			DG_VisibleObjsChanl( work->body.objs, work->chanl ) ;
		}	
	}
}

#define	VALUE_ADJ	(127.0F - ANALOG_MARGIN_F)
static	void	SubjectTurn( work )
Work		*work ;
{
	int		hstep ;
	float	dx ;

	hstep = GM_SubjectHStep * 2 ;
	dx = 0.0F ;
	if ( work->flag & ( FLAG_NO_CONTROL | FLAG_NO_CONTROL2 ) ){
		return;
	}
	if ( work->pad->analog_input & GV_PAD_ANALOG_L_USE ) {
		dx = -( ( float )work->pad->left_dx - 128.0F ) ;
		dx += ( dx > 0.0F ) ? -ANALOG_MARGIN_F : ANALOG_MARGIN_F ;	
	} else {
		if ( work->pad->status & PAD_L ) dx = ( 128.0F - ANALOG_MARGIN_F ) ;
		else dx = -( 128.0F - ANALOG_MARGIN_F ) ;
	}
	work->control.turn.vy += ( int )( ( float )hstep * dx / VALUE_ADJ ) ;
}

static	void	Operation( work )
Work		*work ;
{
	CheckVWait( work ) ;
	if ( work->burst < BURST_TIME && ++work->burst == BURST_TIME )
   {
      work->speed.vy = -SPEED_HIGH ;
      GM_SeSetMode( SD_W_NIKSRCH1 , &work->ctrl->mov, GM_SEMODE_BOMB ) ; // se
	}
   if (gBP_NikitaMissilesCheat & 2)
   {
      work->speed.vy = -SPEED_LOW;
   }
	if ( work->flag & FLAG_CANNOT_OPERATE ) {
		// sight off
	  	GM_SetSightStatus( SGT_Nikita ) ;
		LeaveSubject( work ) ;
		return ;
	}
#if 0
	CheckWatch( work ) ;
#endif
//	if ( work->flag & FLAG_WATCH ) {
		if ( work->pad->status & ( PAD_L | PAD_R ) ) {
			SubjectTurn( work ) ;
			work->burst = 0 ;
			work->speed.vy = -SPEED_LOW ;
		}
//	} else {
//		if ( work->pad->dir != -1 &&
//			work->pad->dir != work->control.rot.vy ) {
//			work->control.turn.vy = work->pad->dir ;
//			work->burst = 0 ;
//			work->speed.vz = SPEED_LOW ;
//		}
//	}
}

/* トラップ情報の戻し */
#if 0
static	void	FlashTrap( work )
Work		*work ;
{
	CONTROL	*ctrl ;
	FVECTOR	*buf, mov ;
	u_int	*inside ;
	int		i, name ;

	if ( work->flag & FLAG_FLASH_TRAP ) return ;
	work->flag |= FLAG_FLASH_TRAP ;

	ctrl = work->ctrl ;
	/* ミサイル側のトラップをコピー */
#if 1
	HZX_FlashTrap( work->control.hzx_id, &work->control.evt ) ;
#else
	inside = work->control.evt.inside ;
	for ( i = 0; i < work->control.evt.n_inside; i ++ ) {
		name = *( inside ++ ) ;
		ctrl->evt.n_inside = 
			AppendInsideList( ctrl->evt.inside, ctrl->evt.n_inside, name ) ;
	}
#endif
	/* 発射キャラのカメラチェック再開 */
	ctrl->evt.flag &= ~HZX_EVT_FLAG_NO_CAMERA ;
	/* フレーム空きが無いように一回チェックする */
	/* いるカメラに入り直すためにinside2をカメラだけクリアする */
	{
		HZX_BND		*inside2[ HZX_MAX_INSIDE_BINDS ] ;
		HZX_BND		**bnd, **bnd2 ;
		int		n_inside2 ;
	
		memcpy( inside2, &ctrl->evt.inside2, sizeof( HZX_BND * ) * HZX_MAX_INSIDE_BINDS ) ;
		bnd = inside2 ;
		bnd2 = ctrl->evt.inside2 ;
		n_inside2 = 0 ;
		for ( i = 0; i < ctrl->evt.n_inside2; i ++, bnd ++ ) {
			if ( ( *bnd )->type & HZX_BND_TYPE_CAMERA ) continue ;
			*( bnd2 ++ ) = *bnd ;
			n_inside2 ++ ;
		}
		ctrl->evt.n_inside2 = n_inside2 ;
	}

	if ( ctrl->root_offset != NULL ) {
		buf = ctrl->evt.mov ;
		_sceVu0AddVector( &mov, &ctrl->mov, ctrl->root_offset ) ;
		ctrl->evt.mov = &mov ;
		HZX_EnterTrap( ctrl->hzx_id, &( ctrl->evt ) ) ;
		ctrl->evt.mov = buf ;
	} else {
		HZX_EnterTrap( ctrl->hzx_id, &( ctrl->evt ) ) ;
	}	
}
#else
#define	FlashTrap( _w )
#endif

static	void	MapConnectReset( void )
{
	/* マップ接合変更禁止解除 */
	GM_EnableMapConnectOff() ;
	/* マップ接合ＯＦＦ */
	GM_ChangeMapConnection2( GM_NikitaMapConnection, 0 ) ;
	GM_ChangeMapConnectID( GM_NikitaMapConnection, 0, 0 ) ;	
}

/*---------------------------------------------------------------*/
extern void *NewShakeCamera2( int chanl, int intense, int time, FVECTOR *pos );

static	void	Act( work )
Work			*work ;
{
	SVECTOR		rot ;
	FVECTOR		step, pmov ;
	FVECTOR		v1, v2 ;
	FMATRIX		world ;
	float		pheight ;
	DG_PRIM2 	*prim;
	int			clock, wallhit ;
	int			j;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */

	/* マップ接合情報の更新 */
	GM_ChangeMapConnectID( GM_NikitaMapConnection, GM_PlayerMap, work->control.map ) ;	

	if ( GM_CheckGameStatus( STATE_PLAY_DEMO ) ||
		 ( work->list.flag & ( GM_BMB_FLAG_DESTROY | GM_BMB_FLAG_INVISIBLE ) ) ) {
		GV_DestroyActor( work ) ;
		GM_NikitaAlive[ work->chanl ] = NKT_NONE ;
		return ;
	}
	if ( work->list.flag & GM_BMB_FLAG_NOBLAST ) {
		if ( PL_FallBulletFunc != NULL ) {
			PL_FallBulletFunc( &work->body.objs->world, NULL, &work->control.step, 
							   BODY_NAME, 300.0F, 100.0F, FALLBUL_TYPE_YDOWN ) ;
			GV_DestroyActor( work ) ;
			GM_NikitaAlive[ work->chanl ] = NKT_NONE ;
			return ;
		}
	}

	GM_SetCurrentMap( GM_StageMapAll ) ;

	switch( work->phase ) {
	case 0 :
		if ( PL_AttackDisable() ||
			 GM_NikitaAlive[ work->chanl ] == NKT_CANCEL ||
			 GM_IsGameOver() ) {
			/* プレイヤーのダメージ等によるキャンセル ゲームオーバー時も */
			work->flag |= FLAG_CANNOT_OPERATE ;
			work->control.skip_flag |= CTRL_SKIP_TRAP ;
			MapConnectReset() ;
			FlashTrap( work ) ;
		}
		// Chaffを使用している時は操作不能
		if ( GM_CheckGameStatus( STATE_CHAFF ) ||
			( work->list.flag & GM_BMB_FLAG_NOCONTROL ) ) {
			work->flag |= FLAG_CANNOT_OPERATE ;
			work->control.skip_flag |= CTRL_SKIP_TRAP ;
			GM_NikitaAlive[ work->chanl ] = NKT_JAMMING;
			MapConnectReset() ;
			FlashTrap( work ) ;
		}
		/* bomblost内に入ったら
		   残りタイム激減＆制御不能 */
		if ( work->count <= DIRECT_TICK( 61 ) ){
			work->flag |= FLAG_NO_CONTROL; // FLAG_CANNOT_OPERATE ;
			work->control.skip_flag |= CTRL_SKIP_TRAP ;
//			GM_NikitaAlive[ work->chanl ] = NKT_JAMMING;
//			MapConnectReset() ;
			FlashTrap( work ) ;
//			if ( work->count > DIRECT_TICK( 48 ) ) work->count = DIRECT_TICK( 48 ) ;
		} else if ( ( work->control.mov.vx < GM_MissileBoundMin.vx ||
				   work->control.mov.vx > GM_MissileBoundMax.vx ||	
				   work->control.mov.vy < GM_MissileBoundMin.vy ||
				   work->control.mov.vy > GM_MissileBoundMax.vy ||	
				   work->control.mov.vz < GM_MissileBoundMin.vz ||
				   work->control.mov.vz > GM_MissileBoundMax.vz ) || 
				   HZX_CheckInsideTrap( work->control.hzx_id, &work->control.mov, GM_TRP_BOMBLOST ) ) {
			/* 指定範囲外も同様 */
			work->flag |= FLAG_CANNOT_OPERATE | FLAG_NO_CONTROL2;
			work->control.skip_flag |= CTRL_SKIP_TRAP ;
			GM_NikitaAlive[ work->chanl ] = NKT_JAMMING;
			MapConnectReset() ;
			FlashTrap( work ) ;
#if 0
			work->flag |= FLAG_NO_CONTROL2; // FLAG_CANNOT_OPERATE ;
			work->control.skip_flag |= CTRL_SKIP_TRAP ;
			work->no_countrol_life = 0;
			FlashTrap( work ) ;
#endif
		}

		if ( ( work->flag & FLAG_HIT ) ||
			 ( work->list.flag & GM_BMB_FLAG_BLAST ) ) {
//			LeaveSubject( work ) ;	
            if ( work->list.flag & GM_BMB_FLAG_BLAST ) {
				/* ダメージなし */
				NewBlast( &work->control.mov, BOTH_SIDE, 10, 20, 0, 0, WP_Nikita ) ;
			} else {
				NewBlast( &work->control.mov, BOTH_SIDE, 1000, 2000, DMG_BLAST, FNT_BLAST, WP_Nikita ) ;
			}
			work->phase = 1 ;
			work->count = 60 ;
			DG_COPY_VEC( &work->control.step, &DG_ZeroVector ) ;
			work->control.skip_flag |= CTRL_SKIP_SEG_CHECK ; 
			DG_InvisibleObjsChanl( work->body.objs, work->chanl ) ;
			GM_NikitaAlive[ work->chanl ] = NKT_BLAST ;
            break ;
		}
//		DG_SetPos2( &DG_ZeroVector, &work->control.rot ) ;
//		DG_RotVector( &work->speed, &work->control.step, 1 ) ;
		DG_COPY_VEC( &pmov, &work->control.mov ) ;
#if 0
		{
			SVECTOR rot;

			rot.vx = -1024; 
			rot.vy = rot.vz = 0;
			DG_SetPos2( &DG_ZeroVector , &rot );
			DG_RotVector( &work->control.step , &work->control.step , 1 );
#endif
			pheight = work->control.mov.vy ;
			DG_SetPos( &work->body.objs->world );
			DG_RotVector( &work->speed , &work->control.step , 1 );
//			printf("speed %f %f %f\n",work->speed.vx , work->speed.vy , work->speed.vz );
			GM_ActControl( &work->control ) ;
#if 0
			rot.vx = 1024 ; 
			rot.vy = rot.vz = 0;
			DG_SetPos2( &DG_ZeroVector , &rot );
			DG_RotVector( &work->control.step , &work->control.step , 1 );
		}
#endif
//		printf("flr %x , grounded %x \n",  work->control.flr_atrs[ 0 ] , work->control.grounded );
		{
			FVECTOR fvtmp;
			if ( ( work->control.flr_atrs[ 0 ] & HZX_FLOOR_STEP ) &&
				 ( work->control.grounded & 1 )) {
			  	fvtmp = DG_ZeroVector;
				fvtmp.vy = 300.0f;
				DG_RotVector( &fvtmp , &fvtmp , 1 );
			  	_sceVu0AddVector( &work->control.step , &work->control.step , &fvtmp );
//				work->control.mov.vy += 250.0F ;
//				pheight += 250.0F ;
//				printf("uyaaaaaa %f \n" , fvtmp.vy);
				pheight += fvtmp.vy ;
			}
		}
		DG_COPY_VEC( &step, &work->control.step );
		step.vy = 0.0f;

		/* ターゲット設定 */
		if ( work->count < GM_NikitaLifeMax - DIRECT_TICK( 90 ) ) {
			/* プレイヤーにもあたるようになる */
			work->off.side = BOTH_SIDE ;
		} 
		DG_GetPos( &world ) ;
		GM_MoveTarget2( &work->off, &world ) ;
		GM_PutTarget( &work->off ) ;
		//NewTargetView( &work->off, 232, 23, 184 ) ;
		GM_MoveTarget2( &work->def, &world ) ;

//		rot = work->control.rot ; rot.vx = -1024 ;
//		DG_SetPos2( &work->control.mov, &rot ) ;
		{
			FVECTOR		diff ;

			_sceVu0SubVector( &diff, &work->control.mov, &pmov ) ;
			GV_VecToRot( &diff, &rot ) ;
			DG_SetPos2( &work->control.mov, &rot ) ;			

			DG_COPY_VEC( &work->subject->position, &work->control.mov ) ;
			rot.vx = AdjustCameraRotX( work, rot.vx ) ;
			work->subject->rotate.vx = GV_NearExp8P( work->subject->rotate.vx, rot.vx ) ;
			// ここをいじればよい
			AdjustCameraRot( work );
		}
		DG_SetPos2( &work->control.mov , &work->control.rot );
		GM_ActObject( &work->body ) ;
		DG_GetLightMatrix( &work->control.mov, work->lights ) ;
		Operation( work ) ;

#if 0
printf( "%f %f [%f]\n", work->control.levels[ 0 ], 
	     work->control.levels[ 1 ], work->control.mov.vy ) ;
printf( "%f %f\n", DG_FABS( pheight - work->control.mov.vy ), GV_VecLen3F( &step ) ) ;
DumpVec( &step ) ;
		printf("pheight = %f\n" , pheight );
		printf("mov.vy = %f \n" , work->control.mov.vy );
		printf("gou = %f %f\n" , DG_FABS( pheight - work->control.mov.vy ) , GV_VecLen3F( &step ) );
#endif
		DG_COPY_VEC( &v1, &pmov ) ;	v1.vy = 0.0F ;
		DG_COPY_VEC( &v2, &work->control.mov ) ; v2.vy = 0.0F ;
		wallhit = 0 ;
      if (gBP_NikitaMissilesCheat & 1)
      {
         // prevent countdown
         ++work->count;
      }
		if ( -- work->count == 0 ||				/* 時間切れ */
			/* 壁に当たった */
			( work->control.attribute & ( CTRL_ATR_ONLINE_SEG | CTRL_ATR_ONLINE_FLR ) ) || 
			/* 斜め床 */
			( DG_FABS( pheight - work->control.mov.vy ) > GV_VecLen3F( &step ) ) ||
			/* はまっているようだ */
			( GV_VecLen3F2( &v1, &v2 ) < GV_VecLen3F( &step ) - 10.0F ) ||
			/* 武器外され */			
			PL_GetPlayerWeapon() != WP_Nikita ) {	
			wallhit = 1 ;
			printf( "normal hit!\n" ) ;
		}
		/* 再壁チェック */
		if ( wallhit == 0 && 
			 HZX_OnlineHazardCheck( HZX_CurrentGroupID, &pmov, &work->control.mov,
								    HZX_CHK_ALL, 
								    HZX_SEG_NO_MISSILE | HZX_SEG_RECOIL_TYPE,
								    HZX_FLOOR_NO_MISSILE | HZX_FLOOR_RECOIL_TYPE ) ) {
			printf( "online hit!\n" ) ;
			DG_COPY_VEC( &work->control.mov, &pmov ) ;
			wallhit = 1 ;
		}
		/* 独自ニアチェック */
		if ( wallhit == 0 &&
			 HZX_NearHazardCheck( HZX_CurrentGroupID, &work->control.mov, 128,
								  HZX_CHK_ALL, 
								  HZX_SEG_NO_MISSILE | HZX_SEG_RECOIL_TYPE, 
								  128 ) ) {
			printf( "near hit!\n" ) ;
			wallhit = 1 ;
		}
#if 0
	   		printf( "%f %f \n ", DG_FABS( pheight - work->control.mov.vy ) , GV_VecLen3F( &step ) );
			printf("work - %x \n" , work->control.attribute & CTRL_ATR_ONLINE_SEG );
			printf("pheight = %f %f\n" , DG_FABS( pheight - work->control.mov.vy ) , GV_VecLen3F( &step ) );
		    printf(" pheight naka = %f , %f \n" , pheight , work->control.mov.vy );
#endif
		if ( wallhit ) work->flag |= FLAG_HIT ;

		// effect by koba4
		{
			FMATRIX fmtmp;
			FVECTOR shift;
			FVECTOR power;

			power.vx = 0.0f;
			power.vy = 60.0f;
			power.vz = 100.0f;
			power.vw = 1.0f;

			if ( !( work->flag & FLAG_WATCH ) ) {
				DG_SetPos2( &work->control.mov , &work->control.rot );
				DG_GetPos( &fmtmp );
				DG_COPY_VEC( &shift , (FVECTOR *)fmtmp.m[ 1 ] );
				_sceVu0ScaleVector( &shift , &shift , 380.f ); 
				_sceVu0AddVector( (FVECTOR *)fmtmp.m[ 3 ] , (FVECTOR *)fmtmp.m[ 3 ] , &shift );
				NewSubMazzleFlushMngNormal( 1 , &fmtmp , &DG_ZeroVector ,&power , 10 , 100.0f, 2.0f , 32 );
//				NewSubMazzleFlushMngRed( 1 , &fmtmp , &DG_ZeroVector ,&power , 10 , 200.0f, 2.0f , 32 );
			}
		}
		if ( work->flag & FLAG_NO_CONTROL || work->flag & FLAG_NO_CONTROL2 ){
			SK_CamAutoMove( work );
			if ( work->no_countrol_life <= 0 ){
				GM_NikitaAlive[ work->chanl ] = NKT_JAMMING;
//				MapConnectReset() ;
				work->flag |= FLAG_HIT;
			} else {
				work->no_countrol_life--;
			}
		}
		GM_NikitaLife = work->count ;
		break ;
	case 1 :
		GM_NikitaLife = 0 ;

		if ( PL_AttackDisable() ||
			 GM_NikitaAlive[ work->chanl ] == NKT_CANCEL ) {
			/* プレイヤーのダメージ等によるキャンセル */
			work->flag |= FLAG_CANNOT_OPERATE ;
			work->control.skip_flag |= CTRL_SKIP_TRAP ;
			FlashTrap( work ) ;
			MapConnectReset() ;
			GV_DestroyActor( work ) ;
			GM_NikitaAlive[ work->chanl ] = NKT_NONE ;
			return ;
		}	
		/* 爆発エフェクトにカメラ照準 */
		if ( -- work->count <= 0 ) {
			GV_DestroyActor( work ) ;
			GM_NikitaAlive[ work->chanl ] = NKT_NONE ;
		} else {
			GM_ActControl( &work->control ) ;
			if ( !( work->flag & FLAG_CANNOT_OPERATE ) ) {
				FVECTOR		to, hit ;
				float		len ;
				int			flrflag ;

				/* 爆発を見せるカメラ */
				/* プレイヤーから４ｍ以上離れているとき */
				/* ニキータ装備中のとき */
				if ( ( GV_VecLen3F2( &work->control.mov, &work->ctrl->mov ) > 4000.0F ) &&
					 ( PL_GetPlayerWeapon() == WP_Nikita ) ) {
					if ( work->subject->on == 0 ) {
						work->subject->on = 1 ;
						GM_ChangeCamera( work->chanl ) ;
					}
					GV_SetVec3( &to, 0.0F, 0.0F, -3000.0F ) ;
					DG_SetPos2( &work->control.mov, &work->subject->rotate ) ;
					DG_PutVector( &to, &to, 1 ) ;
					if ( HZX_OnlineHazardCheck( work->control.hzx_id, &work->control.mov, &to,
											   HZX_CHK_ALL, 
											   HZX_SEG_RECOIL_TYPE, HZX_FLOOR_RECOIL_TYPE ) ) {
						HZX_GetOnlineVector( &hit ) ;
						len = GV_VecLen3F( &hit ) ;
						if ( len > 100.0F ) {
							GV_LenVec3F( &hit, &hit, 0.0F, len - 100.0F ) ;
						} 
						_sceVu0AddVector( &to, &work->control.mov, &hit ) ;
					}	
					GV_NearExp4VF( &work->subject->position, &to, 3 ) ;
					/* 壁床に近すぎるときは抜ける */
					if ( HZX_NearHazardCheck( HZX_CurrentGroupID, &work->subject->position, 
											  128,
											  HZX_CHK_ALL, 
											  HZX_SEG_RECOIL_TYPE, 
											  128 ) ) {
						goto blast_camera_cancel ;
					}
					flrflag = HZX_LevelHazardCheck( HZX_CurrentGroupID, &work->subject->position,
												    HZX_CHK_ALL,
												    HZX_FLOOR_RECOIL_TYPE ) ;
					if ( flrflag ) {
						float		levels[ 2 ] ;

						HZX_GetLevelHeight( levels ) ;
						if ( !( flrflag & 1 ) ) levels[ 0 ] = GM_WORLD_LIMIT_BOTTOM ;
						if ( !( flrflag & 2 ) ) levels[ 1 ] = GM_WORLD_LIMIT_UPPER ;
						if ( ( levels[ 1 ] - work->subject->position.vy < 128.0F ) ||	 
							 ( work->subject->position.vy - levels[ 0 ] < 128.0F ) ) {
							goto blast_camera_cancel ;
						}
					}
					/* コントロールリストに載っているものに近いときは抜ける */
					{
						CONTROL		*ctrl ;
						int			i ;
						
						for ( i = 0; i < GM_N_WhereList; i ++ ) {
							ctrl = GM_WhereList[ i ] ;
							if ( GV_VecLen3F2( &work->subject->position, 
											   &ctrl->mov ) < 1500.0F ) {
								goto blast_camera_cancel ;
							}
						}
					}

				} else {
blast_camera_cancel :
					GV_DestroyActor( work ) ;				
					GM_NikitaAlive[ work->chanl ] = NKT_NONE ;
				}
			} else {
				GV_DestroyActor( work ) ;				
				GM_NikitaAlive[ work->chanl ] = NKT_NONE ;
			}
		}
	}	

#if 0
	/* カメラ位置を奪う */
	if ( !( work->flag & FLAG_CANNOT_OPERATE ) ) {
		GV_NearExp8VF( &work->camera, &work->control.mov, 3 ) ;
		DG_COPY_VEC( &GM_CameraTargetValue[ work->chanl ], &work->camera ) ;
		GM_CameraDirValue[ work->chanl ] = work->control.rot ;
		GM_PlayerDir = work->control.rot.vy & 4095 ;
		GM_PlayerMap = GM_CurrentMap ;
		DG_COPY_VEC( &work->subject->position, &work->control.mov ) ;
	}
#endif
	DG_COPY_VEC( &GM_NikitaPosition[ 0 ], &work->control.mov ) ;

	/* 表示マップ設定 */
	if ( work->subject->on ) {
		GM_SetChanlTargetMap( work->chanl, work->control.map ) ;
	}

	/**********/
	/* effect */
	if ( work->flag & FLAG_WATCH ) {
		DG_InvisiblePrim2( work->prim_main ) ;
#if 0
		DG_InvisiblePrim2( work->prim_sub0 ) ;
		DG_InvisiblePrim2( work->prim_sub1 ) ;
		DG_InvisiblePrim2( work->prim_bonbori ) ;
#endif
	} else {
		DG_VisiblePrim2( work->prim_main ) ;
#if 0
		DG_VisiblePrim2( work->prim_sub0 ) ;
		DG_VisiblePrim2( work->prim_sub1 ) ;
		DG_VisiblePrim2( work->prim_bonbori ) ;
#endif
	}
	GM_GroupPrim2( work->prim_main,    GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( work->prim_main );
#if 0
	GM_GroupPrim2( work->prim_sub0,    GM_CurrentStageMap ) ;
	GM_GroupPrim2( work->prim_sub1,    GM_CurrentStageMap ) ;
	GM_GroupPrim2( work->prim_bonbori, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( work->prim_sub0 );
	DG_SwitchBuffPrim2( work->prim_sub1 );
	DG_SwitchBuffPrim2( work->prim_bonbori );
#endif
	clock = work->prim_main->buffer_clock;

	prim = work->prim_main;
	DG_SetPos( &world );
	DG_MovePos( &ShiftMain );
	uvrgbwh = prim->uvrgb[clock];
	DG_PutVector( s_pos, prim->pos[clock], N_VP );
	for( j=0; j<N_VP; j++ ){
		uvrgbwh->w = uvrgbwh->h = (N_VP-j)*SIZE_STEP + SIZE_BASE;
		uvrgbwh->a = ALPHA_BASE * (N_VP-j) / N_VP + ALPHA_MIN;
		uvrgbwh++;
	}

#if 0
	prim = work->prim_sub0;
	DG_SetPos( &world );
	DG_MovePos( &ShiftSub0 );
	DG_RotatePos( &RotSub0 );
	uvrgbwh = prim->uvrgb[clock];
	DG_PutVector( s_pos, prim->pos[clock], N_VP );
	for( j=0; j<N_VP; j++ ){
		uvrgbwh->w = uvrgbwh->h = (N_VP-j)*SIZE_STEP + SIZE_BASE;
		uvrgbwh->a = ALPHA_BASE * (N_VP-j) / N_VP + ALPHA_MIN;
		uvrgbwh++;
	}

	prim = work->prim_sub1;
	DG_SetPos( &world );
	DG_MovePos( &ShiftSub1 );
	DG_RotatePos( &RotSub1 );
	uvrgbwh = prim->uvrgb[clock];
	DG_PutVector( s_pos, prim->pos[clock], N_VP );
	for( j=0; j<N_VP; j++ ){
		uvrgbwh->w = uvrgbwh->h = (N_VP-j)*SIZE_STEP + SIZE_BASE;
		uvrgbwh->a = ALPHA_BASE * (N_VP-j) / N_VP + ALPHA_MIN;
		uvrgbwh++;
	}

	prim = work->prim_bonbori;
	DG_SetPos( &world );
	DG_MovePos( &ShiftBonbori );
	DG_RotatePos( &RotBonbori );
	uvrgbwh = prim->uvrgb[clock];
	DG_PutVector( s_pos, prim->pos[clock], N_VP );
	for( j=0; j<N_VP; j++ ){
		uvrgbwh->w = uvrgbwh->h = (N_VP-j)*SIZE_STEP + SIZE_BASE;
		uvrgbwh->a = ALPHA_BASE * (N_VP-j) / N_VP + ALPHA_MIN;
		uvrgbwh++;
	}
#endif
}

#if 0
static	inline	int	AppendInsideList( inside, n_inside, name )
u_int			*inside ;	/* トラップ侵入リスト	*/
int			n_inside ;	/* トラップ侵入リスト長	*/
int			name ;		/* トラップ名		*/
{
	int		i ;

	for ( i = n_inside ; i > 0 ; -- i ) {
		if ( *( inside ++ ) == name ) {
			/* すでにリストに存在している */
			return n_inside ;
		}
	}
	*inside = name ;
	return n_inside + 1 ;
}
#endif

static	void	Die( work )
Work		*work ;
{
	GM_FreeControl( &work->control ) ;
	GM_FreeObject( &work->body ) ;
	GM_FreeTarget( &work->def ) ;
	GM_RemoveBombList( &work->list ) ;

//	/* 最初の壁チェックで終わったとき用 */
//	if ( work->phase == 0 ) return ;

	FlashTrap( work ) ;

	GM_DeleteCamera( work->subject ) ;
#ifdef DEBUG_MODE
	work->sw = 0 ;
#endif
	GM_NikitaAlive[ work->chanl ] = NKT_NONE ;
	GM_WeaponAlive &= ~WP_ALIVE_NIKITA ;
	GM_NikitaLife = 0 ;

	if( work->prim_main != NULL )    GM_FreePrim2( work->prim_main );
#if 0
	if( work->prim_sub0 != NULL )    GM_FreePrim2( work->prim_sub0 );
	if( work->prim_sub1 != NULL )    GM_FreePrim2( work->prim_sub1 );
	if( work->prim_bonbori != NULL ) GM_FreePrim2( work->prim_bonbori );
#endif
	MapConnectReset() ;	
	NikitaWork = NULL ;
}


/* ---------------------------------------------------------------- */
static void InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex, int n_prims, int n_verts )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		i,j,k ;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	prim->raise=RAISE;

	//-------------------------------
	for ( i=0; i<2; i++ ){
		uvrgbwh = prim->uvrgb[ i ] ;
		for ( j=0; j<n_prims; j++ ){
			for ( k=0; k<n_verts; k++ ){
				uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
				uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
				uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
				uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
				uvrgbwh->q0 = 4096 ;
				uvrgbwh->q1 = 4096 ;
				uvrgbwh->f0 = 0x0fff ;
				uvrgbwh->f1 = 0x0fff ;
				uvrgbwh->h = uvrgbwh->w = DUMMY_SIZE ;
				uvrgbwh->r = COL_R;
				uvrgbwh->g = COL_G;
				uvrgbwh->b = COL_B;
				uvrgbwh->a = DUMMY_ALPHA ;
				uvrgbwh ++ ;
			}
		}
	}
}

#define NO_CONTROL_LIFE (60)
static void SK_CamAutoMove( Work *pWork )
{
	int tmp;

	if ( pWork->flag & FLAG_NO_CONTROL ) {
		tmp = DIRECT_TICK( NO_CONTROL_LIFE ) - pWork->no_countrol_life;
		tmp = tmp == 0 ? 1 : tmp;
		tmp *= 4;
		pWork->subject->rotate.vx = pWork->control.rot.vx + 1024 + irnd() % tmp - ( tmp / 2 );
		pWork->subject->rotate.vy = pWork->control.rot.vy + irnd() % tmp - ( tmp / 2 );
	}
	if ( pWork->no_countrol_life % 20 ){
    	GM_SeSetMode( SD_W_SIGNAL02 , &pWork->ctrl->mov , GM_SEMODE_BOMB ) ; // se
	}
}

/*---------------------------------------------------------------*/

static	void	SetTarget( work )
Work		*work ;
{
	TARGET	*t ;
	FVECTOR	size ;
	
	t = &work->off ;
	size.vx = 50.0F ;
	size.vy = 50.0F ;
	size.vz = 250.0F ;
	GM_SetTarget( t, TARGET_OFFENSE | TARGET_POWER | TARGET_ROTATE | TARGET_THROUGH, 
				  0, ENEMY_SIDE, &size, &DG_ZeroVector ) ;
	GM_SetTargetWeaponType( t, WP_WEAPONCORE ) ;
	GM_SetTargetName( t, WP_Nikita ) ;
	GM_SetTargetCallBack( t, Hit, work ) ;

	t = &work->def ;
	GV_SetVec3( &size, 50.0F, 50.0F, 250.0F ) ;
	GM_SetTarget( t, TARGET_DEFENSE|TARGET_POWER|TARGET_ROTATE|TARGET_NO_CLAYMORE,	
				  0, BOTH_SIDE, &size, &DG_ZeroVector ) ;
	GM_SetTargetName( t, WP_Nikita ) ;
	GM_SetTargetCallBack( t, Hitted, work ) ;
	GM_PutTarget( t ) ;
}

static	int	InitControl( work, world, chara )
Work		*work ;
FMATRIX		*world ;
CONTROL		*chara ;
{
	CONTROL	*ctrl ;
	SVECTOR	rot ;

	/* 新ニキータ */
	/* 常に主観 */
	NewShakeCamera2( 0, 512, 10, &work->control.mov );

	ctrl = &work->control ;
	if ( GM_InitControl( ctrl, WP_Nikita, chara->map ) < 0 ) return -1 ;
	rot.vx = chara->turn.vx - 1024;
	rot.vy = chara->turn.vy;
	rot.vz = chara->turn.vz;
	GV_MatToVec( world, &ctrl->mov ) ;
	GM_ConfigControlPosition( ctrl, &ctrl->mov, &rot ) ;
	GM_ConfigControlMapCheck( ctrl ) ;
	GM_ConfigControlTrapCheck( ctrl ) ;
	ctrl->evt.chara = GV_StrCode( "ニキータ" ) ;
	ctrl->evt.flag |= HZX_EVT_FLAG_MISSILE | HZX_EVT_FLAG_MAPCONNECT | HZX_EVT_FLAG_CHECKALL ;
	GM_ConfigControlHazard( ctrl, 150, 500, 500 ) ;
	ctrl->seg_flag = HZX_SEG_NO_MISSILE | HZX_SEG_RECOIL_TYPE ;
	ctrl->flr_flag = HZX_FLOOR_NO_MISSILE | HZX_FLOOR_RECOIL_TYPE ;
	ctrl->skip_flag |= CTRL_SKIP_NEAR_CHECK;
	return 0 ;
#if 0	
	/* カメラトラップを奪う為特殊な構造になっている */
	ctrl = &work->control ;
	/* キャラの名前で登録 */
	if ( GM_InitControl( ctrl, WP_Nikita, chara->map ) < 0 ) return -1 ;
	rot.vx = rot.vz = 0 ; rot.vy = chara->rot.vy ;
	/* 最初はキャラの位置でリンクゾーン等のチェックをする */
	/* その後発射位置に配置 */
	GV_MatToVec( world, &ctrl->mov ) ;
	GM_ConfigControlPosition( ctrl, &ctrl->mov, &rot ) ;
	GM_ConfigControlMapCheck( ctrl ) ;
	GM_ConfigControlTrapCheck( ctrl ) ;
	//GM_ConfigControlCameraTrapCheck( ctrl ) ;
	ctrl->evt.flag |= HZX_EVT_FLAG_MISSILE ;
	GM_ConfigControlHazard( ctrl, 200, 500, 500 ) ;
	ctrl->evt.chara = chara->evt.chara ;
	ctrl->seg_flag = HZX_SEG_NO_MISSILE | HZX_SEG_RECOIL_TYPE ;
//	ctrl->seg_flag = 0 ;
	ctrl->flr_flag = HZX_FLOOR_NO_MISSILE | HZX_FLOOR_RECOIL_TYPE | HZX_FLOOR_IK ;
//	ctrl->flr_flag = HZX_FLOOR_IK ;
	ctrl->skip_flag |= CTRL_SKIP_NEAR_CHECK ;
	/* カメラトラップのみチェック */
	ctrl->evt.flag |= HZX_EVT_FLAG_CAMERA_ONLY ;
	/* 発射キャラのトラップ情報をコピー */
	ctrl->evt.n_inside = chara->evt.n_inside ;
	memcpy( ctrl->evt.inside, chara->evt.inside, sizeof( u_int ) * HZX_MAX_TRAPS ) ;
	/* バインド情報はコピーしない */
	ctrl->evt.n_inside2 = 0 ;

	/* 発射キャラは一時的にカメラをチェックしない */
	chara->evt.flag |= HZX_EVT_FLAG_NO_CAMERA ;
#ifdef DEBUG_MODE
	{
		//	NewTrapView( &work->control, 255, 255, 0, &work->sw ) ;
	}
#endif
	return 0 ;
#endif
}	

static	int	InitObject( work, world )
Work		*work ;
FMATRIX		*world ;
{
	OBJECT	*body ;

	body = &work->body ;
	GM_InitObject( body, BODY_NAME, BODY_FLAG ) ;
	if ( body->objs == NULL ) return -1 ;
	body->map_name = GM_CurrentStageMap ;
	GM_GroupObjs( body->objs, GM_CurrentStageMap ) ;
	DG_SetPos2( &work->control.mov, &work->control.rot ) ;
  	DG_PutObjs( body->objs ) ;
	GM_ConfigObjectLight( body, work->lights ) ;
	DG_InvisibleObjsChanl( body->objs, 0 ) ;	/* 主画面非表示 */
	return 0 ;
}

static	int	InitCamera( work )
Work		*work ;
{
	GM_CameraSet	*cam ;

	work->subject = cam = NewProgramCamera( CAMERA_NAME, GM_CurrentCameraChanl,
										   GM_CAMERA_SUBJECT, 0 ) ;
	if ( cam == NULL ) {
		printf( "prev nikita active!!\n" ) ;
		return -1 ;
	}
	GM_SetCameraType( cam, GM_CAM_TYPE_CAMERA_AND_ROTATE, 
					  CAM_FLAG_PAD_ADJUST ) ;
	GM_SetCameraRotate( cam, &( work->control.rot ) ) ;
	GM_SetCameraTrack( cam, 500 ) ;
	GM_SetCameraAngle( cam, 1.50F ) ;
	GM_SetCameraInterpMode( cam, GM_CAM_INTERP_EXP4, 
						   GM_CAM_INTERP_QUICK, 0, 0 ) ;
	DG_COPY_VEC( &cam->position, &work->control.mov ) ;
	IntoSubject( work ) ;
	return 0 ;
}

static	int	GetResources( work, world, ctrl, side, chanl )
Work		*work ;
FMATRIX		*world ;
CONTROL		*ctrl ;
int		side, chanl ;
{
	FVECTOR	from, to;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	int			i;

	/* まずオンラインチェックで
	   キャラと発射点の間に壁があるかチェック */
	GV_MatToVec( world, &to ) ;
	DG_COPY_VEC( &from, &ctrl->mov ) ; from.vy = to.vy ;
#if 0
	if ( HZX_OnlineHazardCheck( ctrl->hzx_id, &from, &to,
							   HZX_CHK_ALL, HZX_SEG_NO_MISSILE | HZX_SEG_RECOIL_TYPE,
							   HZX_FLOOR_NO_MISSILE | HZX_FLOOR_RECOIL_TYPE ) > 0 ) {
		HZX_GetOnlinePoint( &hit ) ;
		NewBlast( &hit, BOTH_SIDE, 1000, 2000, DMG_BLAST, FNT_BLAST, WP_Nikita ) ;
		return -1 ;
	}
#endif
	if ( InitControl( work, world, ctrl ) < 0 ) return -1 ;
	if ( InitObject( work, world ) < 0 ) return -1 ;
	SetTarget( work ) ;
	if ( InitCamera( work ) < 0 ) return -1 ;

	work->ctrl = ctrl ;
	work->count = GM_NikitaLifeMax ;
	work->no_countrol_life = DIRECT_TICK( NO_CONTROL_LIFE );
	work->pad = &GV_PadData[ chanl ] ;
	work->chanl = chanl ;
	work->speed.vx = work->speed.vz = 0.0F ;
	work->speed.vy = -SPEED_LOW ;
	DG_COPY_VEC( &work->camera, &GM_CameraTargetValue[ chanl ] ) ;
	GM_NikitaAlive[ chanl ] = NKT_NORMAL ;
	GM_WeaponAlive |= WP_ALIVE_NIKITA ;
	DG_COPY_VEC( &GM_NikitaPosition[ 0 ], &work->control.mov ) ;

	GM_InitBombList( &work->list, WP_Nikita, &work->control.mov, &work->control.step ) ;
	GM_AddBombList( &work->list ) ;

	//-----------------------------
	tex = DG_GetTexture( GV_StrCode( "light05_msk" ) );
	prim = work->prim_main = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket( work, prim, tex, N_PRIMS, N_VERTS );
#if 0
	prim = work->prim_sub0 = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket( work, prim, tex, N_PRIMS, N_VERTS );
	prim = work->prim_sub1 = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket( work, prim, tex, N_PRIMS, N_VERTS );

	tex = DG_GetTexture( GV_StrCode( "rcm_l_msk" ) );
	prim = work->prim_bonbori = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 
											  N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket( work, prim, tex, N_PRIMS, N_VERTS );
#endif
	for ( i = 0 ; i < N_VP ; i++ ){
		s_pos[i].vx = 0.0f;
		s_pos[i].vy = (float)(i+1) * LIGHT_LEN / (float)(N_VP);
		s_pos[i].vz = 0.0f;
	}

	work->pad_data_x = 0; // 追加こば４
	work->pad_data_y = 0; // 追加こば４

	return 0 ;
}

/* ニキータ起動 */
void	*NewBulletNikita( world, ctrl, side, chanl )
FMATRIX		*world ;
CONTROL		*ctrl ;
int		side, chanl ;
{
	Work	*work ;

	/* プレイヤーより後でカメラプロセスよりも前 */
	work = ( Work * )GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, world, ctrl, side, chanl ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		/* 発砲数 */
		if ( ++ GM_ShootCount > 30000 ) GM_ShootCount = 30000 ;
		GM_DecrementWeapon( WP_Nikita, 1 ) ;
		NewPadVibration2( GV_StrCode( "rai_rgb_fire" ), 0 ) ;
		GM_SetWeaponFire( WP_Nikita ) ;
	}
	NikitaWork = work ; 

	/* マップＯＦＦ禁止 */
	GM_ForbidMapConnectOff() ;
	/* マップ接合ＯＮ */
	GM_ChangeMapConnectID( GM_NikitaMapConnection, GM_PlayerMap, GM_PlayerMap ) ;
	GM_ChangeMapConnection2( GM_NikitaMapConnection, 1 ) ;
	return work ;
}

/*----------------------------------------------------------------*/

/* ニキータの速度を返す。飛行していなければ０ */
int		GM_NikitaSpeed( void )
{
	if ( GM_NikitaLife <= 0 || NikitaWork == NULL ) return 0 ;
	return ( int )( - NikitaWork->speed.vy ) ;
}

/* ニキータの強制爆発 */
int		GM_DestroyNikita( void )
{
	if ( GM_NikitaLife <= 0 || NikitaWork == NULL ) return 0 ;
	NikitaWork->flag |= FLAG_HIT ;
	return 1 ;
}

