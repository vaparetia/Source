//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	windnois.c
	風音

	2000/06/28 Y.Korekado
	$Id: windnoisx.c,v 1.2 2002/12/09 19:29:39 Yoshizawa1 Exp $
*/

#ifdef KP_XBOX //BP

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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
#include	"korekado/conv/define.h"
#include	"camera.h"

#include	"sd_ee.h"
#ifdef KP_WINDOWS
#define	UPWIND		0x14	/* 風上の音 */
#define	DOWNWIND	0x15	/* 風下の音 */
#define	EARWIND_L	0x1A 	/* 主観耳風ボーボー音(左) */
#define	EARWIND_R	0x1B 	/* 主観耳風ボーボー音(右) */
#define	C_BOX_1		0x1C 	/* ダンボール音１|プラントはマイク風 */
#define	C_BOX_2		0x1D 	/* ダンボール音２|プラントはマイク風 */

#define	MIC_L		0x1C 	/* プラントはマイク風 */
#define	MIC_R		0x1D 	/* プラントはマイク風 */
#else
#define	UPWIND		0x04	/* 風上の音 */
#define	DOWNWIND	0x05	/* 風下の音 */
#define	EARWIND_L	0x0A 	/* 主観耳風ボーボー音(左) */
#define	EARWIND_R	0x0B 	/* 主観耳風ボーボー音(右) */
#define	C_BOX_1		0x0C 	/* ダンボール音１|プラントはマイク風 */
#define	C_BOX_2		0x0D 	/* ダンボール音２|プラントはマイク風 */

#define	MIC_L		0x0C 	/* プラントはマイク風 */
#define	MIC_R		0x0D 	/* プラントはマイク風 */
#endif


/*----------------------------------------------------------------*/


/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;

	char	upwind_pan ;
	char	downwind_pan ;
	char	earwind_l_pan ;
	char	earwind_r_pan ;

	int		wind_vol ;
	int		earwind_vol ;
	int		micwind_vol ;
	int		c_box_vol ;

	FVECTOR	vDirUp;
	FVECTOR	vDirDown;

	int		stage ;		//(0:tanker,rain 1:plant,fine)
	int		mode ;

	int		hUpwind;	// 風上音ハンドル
	int		hDnwind;	// 風下音ハンドル
	int		hErl;		// 主観左音ハンドル
	int		hErr;		// 主観右音ハンドル
	int		hDbl;		// ダンボール音左ハンドル
	int		hDbr;		// ダンボール音右ハンドル
	
#if 0 //BP
	SD_3D_SNG_TRACK_TABLE sng_track_table;
#endif
} Work ;

/*----------------------------------------------------------------*/
#define PRIO	0x40	/* 敵兵より後に処理 */



#ifdef KP_WINDOWS
#define	UPWIND		0x14	/* 風上の音 */
#define	DOWNWIND	0x15	/* 風下の音 */
#define	EARWIND_L	0x1A 	/* 主観耳風ボーボー音(左) */
#define	EARWIND_R	0x1B 	/* 主観耳風ボーボー音(右) */
#define	C_BOX_1		0x1C 	/* ダンボール音１|プラントはマイク風 */
#define	C_BOX_2		0x1D 	/* ダンボール音２|プラントはマイク風 */

#define	MIC_L		0x1C 	/* プラントはマイク風 */
#define	MIC_R		0x1D 	/* プラントはマイク風 */
#else
#define	UPWIND		0x04	/* 風上の音 */
#define	DOWNWIND	0x05	/* 風下の音 */
#define	EARWIND_L	0x0A 	/* 主観耳風ボーボー音(左) */
#define	EARWIND_R	0x0B 	/* 主観耳風ボーボー音(右) */
#define	C_BOX_1		0x0C 	/* ダンボール音１|プラントはマイク風 */
#define	C_BOX_2		0x0D 	/* ダンボール音２|プラントはマイク風 */

#define	MIC_L		0x0C 	/* プラントはマイク風 */
#define	MIC_R		0x0D 	/* プラントはマイク風 */
#endif
//#define PAN_INTERP_DOWN	(1)
//#define PAN_INTERP	(2)

#define	INTERP_LIMIT_COS_UP		0.9945219f	// cos( 6°)
#define	INTERP_LIMIT_COS_DOWN	0.9986295f	// cos( 3°)

//#define VOL_INTERP	(4)
#define VOL_INTERP	150

#define WIND_SPEED_MAX	(500.0f)
#define WIND_SPEED_BASE	(100.0f)

#define	VOL_LOWER_THRESHOLD	-4000		// 無音とみなす音


enum {
	WEATHER_RAIN,
	WEATHER_FAIN,
} ;
/*----------------------------------------------------------------*/
extern	FVECTOR	G_wind ;				/* 風速（単位フレームに移動する距離） */
extern	SVECTOR	G_wind_rot ;			/* 風向 */
extern	int		G_wind_intense ;		/* 強さ */
extern	int		G_wind_intense_max ;	/* 強さ */
extern	int		G_wind_sw ;				/* 突風発生中に立つ */
extern	FMATRIX	G_wind_matrix ;
/*----------------------------------------------------------------*/


static	inline	float __RcosF( d )
int	d ;
{
	float f, t ;

	f = (float)M_PI * (float)d / 2048.0F ;
	t = cosf( f ) ;

	return t ;
}

static int	LimitInterp( int goal, int cur, int limit )
{	// goal に向けて最大 limit の幅で cur を変更した結果を返す

	if ( goal > cur + limit ) {
		return cur + limit;
	} else if( goal < cur - limit ) {
		return cur - limit;
	} else {
		return goal;
	}
}

static void LimitInterpVec( FVECTOR* pvRet, FVECTOR* pvGoal, FVECTOR* pvCur,
							float fLimitCos )
{	// 上の関数の方向指示ベクトル版
	// なす角度が大きいものは指定角度内のベクトルにする
	// pvGoal, pvCur は単位ベクトル

	if( _sceVu0InnerProduct( pvGoal, pvCur ) < fLimitCos ) {
		// リミットオーバー
		FVECTOR	vNormal;
		FVECTOR	vBiNormal;

		// ２ベクトルに垂直なベクトルを出し
		_sceVu0OuterProduct( &vNormal, pvCur, pvGoal );
		
		// さらにその結果ともとのベクトルに垂直なベクトルを出し
		_sceVu0OuterProduct( &vBiNormal, &vNormal, pvCur );

		if( vBiNormal.vx == 0.f && vBiNormal.vy == 0.f && vBiNormal.vz == 0.f ) {
			// goal が正反対を向いている->一回休み
			*pvRet = *pvCur;
		} else {
			// cos値より計算する
			// *pvRet = fLimitCos * ( *pvCur ) + bp_sqrtf( 1 - fLimitCos * fLimitCos ) * vBiNormal    //BP_MATH - emulate PS2 sqrtf
			FVECTOR	vTmp0, vTmp1;

			_sceVu0Normalize( &vBiNormal, &vBiNormal );
			_sceVu0ScaleVector( &vTmp0, &vBiNormal, bp_sqrtf( 1 - fLimitCos * fLimitCos ));  //BP_MATH - emulate PS2 sqrtf
			_sceVu0ScaleVector( &vTmp1, pvCur, fLimitCos );
			_sceVu0AddVector( pvRet, &vTmp0, &vTmp1 );
		}
	} else {
		// リミット内
		*pvRet = *pvGoal;
	}
}

static void WindNoise( Work *work )
{
   BP_SOUND_TODO_BREAK;
#if 0//BP
	GM_CameraSet	*cam ;
	int	vol, box_vol, c_box ;
	float	v, f ;
	FVECTOR	vDir, vTmp;

	// 風強さ->ボリューム
	cam = GM_GetCurrentCamera( 0 ) ;	/* メイン画面のみ */
	f = ((float)G_wind_intense - WIND_SPEED_BASE) / WIND_SPEED_MAX ;
	v = f * 400.f;	// fの１は 4 dB に対応
					// (オリジナルは振幅が 0x28/0x3f mBで-394)
	vol = -400 + (int) v;
	if( vol > 0 ) vol = 0;
	box_vol = vol;
	
	// ダンボール判定
	c_box = 0 ;
	if ( work->stage == WEATHER_RAIN ) {
		/* ダンボール音 */
		if ( (GM_PlayerStatus&PLAYER_CB_BOX)&&(GM_PlayerStatus&PLAYER_WATCH) ) {
			if( GM_PlayerControl->level_found & 0x2 ) {
				box_vol = VOL_LOWER_THRESHOLD;
			}
			c_box = 1 ;
		} else {
			box_vol = VOL_LOWER_THRESHOLD;
		}
		// ボリューム計算
		work->c_box_vol = LimitInterp( box_vol, work->c_box_vol, VOL_INTERP );
		sd_3dsrc_setvol( work->hDbl, work->c_box_vol );
		sd_3dsrc_setvol( work->hDbr, work->c_box_vol );
	}

	work->wind_vol = LimitInterp( vol, work->wind_vol, VOL_INTERP );

	sd_3dsrc_setvol( work->hUpwind, work->wind_vol - c_box * 600 );	// ダンボール時 -6dB
	sd_3dsrc_setvol( work->hDnwind, work->wind_vol - c_box * 600 );	// ダンボール時 -6dB

	// 方向計算
	
	vDir = G_wind;
	if( !(vDir.x == 0.f && vDir.y == 0.f && vDir.z == 0.f) ) {
		_sceVu0Normalize( &vDir, &vDir );
		// リミットつきで方向を計算
		LimitInterpVec( &work->vDirUp, &vDir, &work->vDirUp, INTERP_LIMIT_COS_UP );
		LimitInterpVec( &work->vDirDown, &vDir, &work->vDirDown, INTERP_LIMIT_COS_DOWN );
	}
	
	// 風上・風下方向に音を置く
	// 浮動小数点の性質を考えて一応 100 の単位にしておく
	_sceVu0ScaleVectorXYZ( &vTmp, &work->vDirUp, 100.f );
	_sceVu0SubVector( &vTmp, &cam->position, &vTmp );	// Up は逆方向を向いているので
	sd_3dsrc_setpos( work->hUpwind, &vTmp );
	_sceVu0ScaleVectorXYZ( &vTmp, &work->vDirDown, 100.f );
	_sceVu0AddVector( &vTmp, &cam->position, &vTmp );
	sd_3dsrc_setpos( work->hDnwind, &vTmp );

	// 主観耳風音
	if ( GM_PlayerStatus & (PLAYER_INTRUDE|PLAYER_WATCH) ) {
		float fCos;
		FVECTOR	vTmp2;
		
		v = f * 250.f;	// fの１は 2.5 dB に対応
		vol = -600 + (int) v;

		_sceVu0SubVector( &vTmp, &cam->target, &cam->position );
		_sceVu0Normalize( &vTmp, &vTmp );

		fCos = -_sceVu0InnerProduct( &vDir, &vTmp );

		if( fCos < 0.f ) fCos = 0.f;
		vol += (int) ( fCos * 350.f );	// 3.5 dB
		if( vol > 0 ) vol = 0;

		// 音源位置設定
		// とりあえず開き具合は固定
		vTmp2.vx = vDir.vz; vTmp2.vy = 0.f; vTmp2.vz = -vDir.vx;	// 上 x 風向 本当はカメラの上ベクトルがいい
		if( !(vTmp2.vx == 0.f && vTmp2.vz == 0.f )) {
			FVECTOR vPos;
			
			_sceVu0Normalize( &vTmp2, &vTmp2 );

			// 左 vDir * (-sin) + vTmp2 * (-cos)
			// 右 vDir * (-sin) + vTmp2 * cos

			fCos = 0.7071f;	// cos( 45°)
			_sceVu0ScaleVectorXYZ( &vTmp2, &vTmp2, fCos * 100.f);
			_sceVu0ScaleVectorXYZ( &vTmp, &vDir, -bp_sqrtf( 1.f - fCos * fCos ) * 100.f );   //BP_MATH - emulate PS2 sqrtf

			_sceVu0SubVector( &vPos, &vTmp, &vTmp2 );
			_sceVu0AddVector( &vPos, &cam->position, &vPos );
			sd_3dsrc_setpos( work->hErl, &vPos );
			if( work->stage == WEATHER_FAIN ) {
				sd_3dsrc_setpos( work->hDbl, &vPos );
			}

			_sceVu0AddVector( &vPos, &vTmp, &vTmp2 );
			_sceVu0AddVector( &vPos, &cam->position, &vPos );
			sd_3dsrc_setpos( work->hErr, &vPos );
			if( work->stage == WEATHER_FAIN ) {
				sd_3dsrc_setpos( work->hDbr, &vPos );
			}
		}
	} else {
		vol = VOL_LOWER_THRESHOLD;
	}
	work->earwind_vol = LimitInterp( vol, work->earwind_vol, VOL_INTERP );
	sd_3dsrc_setvol( work->hErl, work->earwind_vol - c_box * 500 );
	sd_3dsrc_setvol( work->hErr, work->earwind_vol - c_box * 500 );

	// マイク音
	if( PL_GetPlayerWeapon( ) != WP_Mic ) {
		vol = VOL_LOWER_THRESHOLD; 
	}
	work->micwind_vol = LimitInterp( vol, work->micwind_vol, VOL_INTERP );
	if( work->stage == WEATHER_FAIN ) {
		// （ハンドルはダンボールと共通）
		sd_3dsrc_setvol( work->hDbl, work->micwind_vol - c_box * 500 );
		sd_3dsrc_setvol( work->hDbr, work->micwind_vol - c_box * 500 );
	}

//	if( GM_PlayerBody != NULL ) {
//		sd_3dsrc_setpos( work->hErl, &GM_PlayerPosition );
//		sd_3dsrc_setpos( work->hErr, &GM_PlayerPosition );
//	}
#endif
}

static	void	Act( work )
Work		*work ;
{
	switch ( work->mode ) {
		case ALERT_MODE_SNEAK :
			if ( GM_AlertMode == ALERT_MODE_SNEAK ) {
				WindNoise( work ) ;
			} else {
//				GM_InitMixConvFader( ) ;
				work->mode = GM_AlertMode ;
			}
		break ;
		default :
			if ( GM_AlertMode == ALERT_MODE_SNEAK ) {
				// もともとのフェーダは最大にしておく
				GM_MixConvFader( UPWIND, GM_PAN_CENTER, 0x3f );
				GM_MixConvFader( DOWNWIND, GM_PAN_CENTER, 0x3f );
				GM_MixConvFader( EARWIND_L, GM_PAN_CENTER, 0x3f );
				GM_MixConvFader( EARWIND_R, GM_PAN_CENTER, 0x3f );
				GM_MixConvFader( C_BOX_1, GM_PAN_CENTER, 0x3f );
				GM_MixConvFader( C_BOX_2, GM_PAN_CENTER, 0x3f );
				
				work->mode = GM_AlertMode ;
			}
		break ;
	}
}

static	void	Die( work )
Work		*work ;
{
   BP_SOUND_TODO_BREAK;
#if 0//BP
	sd_3d_release_sng_track_table( work->sng_track_table );
	// ハンドルの開放
	sd_3d_release_handle( work->hUpwind, 0 );
	sd_3d_release_handle( work->hDnwind, 0 );
	sd_3d_release_handle( work->hErl, 0 );
	sd_3d_release_handle( work->hErr, 0 );
	sd_3d_release_handle( work->hDbl, 0 );
	sd_3d_release_handle( work->hDbr, 0 );
#endif
}

/*----------------------------------------------------------------*/
static	int	GetResources( work )
Work	*work ;
{
   BP_SOUND_TODO_BREAK;
#if 0//BP
	DSI3DL2BUFFER ds3db;

	work->stage = GCL_GetOptionValue( 's', 0 ) ;

//	work->mode = ALERT_MODE_SNEAK ;
	work->mode = -1 ;

	ZeroMemory( &ds3db, sizeof( ds3db ));
	
	// ３Ｄ音バッファテーブルのセットアップ
	ZeroMemory( &work->sng_track_table, sizeof( work->sng_track_table ) );
	// ハンドルの取得
	work->sng_track_table[ UPWIND ].handle = work->hUpwind = sd_3d_get_handle();
	work->sng_track_table[ DOWNWIND ].handle = work->hDnwind = sd_3d_get_handle();
	work->sng_track_table[ EARWIND_L ].handle = work->hErl = sd_3d_get_handle();
	work->sng_track_table[ EARWIND_R ].handle = work->hErr = sd_3d_get_handle();
	work->sng_track_table[ C_BOX_1 ].handle = work->hDbl = sd_3d_get_handle();
	work->sng_track_table[ C_BOX_2 ].handle = work->hDbr = sd_3d_get_handle();

	sd_3d_set_sng_track_table( SNG_PLAY_05, work->sng_track_table );

	{	// 風上・風下音

		// 障害エフェクトなどのイニシャライズ（単に影響を無くす）
		sd_3dsrc_seti3dl2( work->hUpwind, &ds3db );
		sd_3dsrc_seti3dl2( work->hDnwind, &ds3db );

		// ボリュームを落としておく
		work->wind_vol = VOL_LOWER_THRESHOLD;
		sd_3dsrc_setvol( work->hUpwind, work->wind_vol );
		sd_3dsrc_setvol( work->hDnwind, work->wind_vol );

		
		work->vDirUp.vx = work->vDirUp.vy = work->vDirDown.vx = work->vDirDown.vy = 0.f;
		work->vDirUp.vz = work->vDirDown.vz = 1.f;	// vDirUp は実際の方向の反対を向いている
	}
	
	{	// ダンボール音・マイク音
		FVECTOR vTmp;
		
		sd_3dsrc_seti3dl2( work->hDbl, &ds3db );
		sd_3dsrc_seti3dl2( work->hDbr, &ds3db );
		
		// ボリュームを落としておく
		work->c_box_vol = VOL_LOWER_THRESHOLD;
		work->micwind_vol = VOL_LOWER_THRESHOLD;
		
		sd_3dsrc_setvol( work->hDbl, work->c_box_vol );
		sd_3dsrc_setvol( work->hDbr, work->c_box_vol );

		if( work->stage == WEATHER_RAIN ) {
			// 位置はリスナー相対・固定
			sd_3dsrc_setmode( work->hDbl, DS3DMODE_HEADRELATIVE );
			sd_3dsrc_setmode( work->hDbr, DS3DMODE_HEADRELATIVE );
			// 固定位置設定
			vTmp.vx = 50.f; vTmp.vy = 100.f; vTmp.vz = 0.f;
			sd_3dsrc_setpos( work->hDbl, &vTmp );
			vTmp.vx = -50.f;
			sd_3dsrc_setpos( work->hDbr, &vTmp );
		}
	}

	{	// 主観風音
		sd_3dsrc_seti3dl2( work->hErl, &ds3db );
		sd_3dsrc_seti3dl2( work->hErr, &ds3db );

		// ボリュームを落としておく
		work->earwind_vol = VOL_LOWER_THRESHOLD;
		sd_3dsrc_setvol( work->hErl, work->earwind_vol );
		sd_3dsrc_setvol( work->hErr, work->earwind_vol );

	}

	Act( work );
#endif			
	return 0 ;
}

void	*NewWindNoise( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &( work->actor ) ) ;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}


#endif