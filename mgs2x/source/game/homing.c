//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	homing.c
	ホーミング

	頭、腕、腰のアジャスト

	1999/08/17 Y.Korekado
	$Id: homing.c,v 1.1.1.3 2002/11/19 11:41:51 Yoshizawa1 Exp $
	
*/
#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"libgv.h"
#include	"libmt.h"

#include	"libutl.h"

#include "bp_vector.h"

/* 
	ホーミングシステム初期化
	void	GM_InitHomingSystem( void )

	ホーミングデータ作成
	GM_SetHoming( def )

	void	GM_SetHoming( hom, world, body, map )
	HOMING_TRG	*hom ;
	FMATRIX	*world ;	検索対象になるワールド
	OBJECT	*body ;		もしオブジェクトがあればセット（ＮＵＬＬでもよい）
	int		*map ;		マップ情報のアドレス

	ホーミング登録
	void	GM_PutHoming( hom )
	HOMING_TRG	*hom

	ホーミング削除
	void	GM_FreeHoming( hom )
	HOMING_TRG	*hom

	HOMING_TRG	*GM_GetHomingNearTrg( pos, dir, map, length, range, hom_trg, mode, hzd )
	FVECTOR	*pos ;		場所
	int		dir ;		方向
	int		map ;		マップ
	int		length ;	探索距離
	int		range ;		探索範囲
	HOMING_TRG	*hom_trg ;	現在ホーミングしているターゲット
				これがＮＵＬＬなら向いている方向から左右判定
	int		mode ;		サーチモード
				０：	右方向サーチ
				１：	左方向サーチ
	HZD_HDL		*hzd ;	ハザード
				これがＮＵＬＬならハザードチェックをしない

	HOMING_TRG	*GM_GetHomingTrg( pos, dir, map, length, range, hzd )
	FVECTOR	*pos ;		場所
	int		dir ;		方向
	int		map ;		マップ
	int		length ;	探索距離
	int		range ;		探索範囲
	HZD_HDL		*hzd ;	ハザード
				これがＮＵＬＬならハザードチェックをしない

	HOMING_TRG	*GM_GetHomingTrgPart( pos, dir, length, range, part_num, hzx_id )
	FVECTOR	*pos ;		場所
	int		dir ;		方向
	int		map ;		マップ
	int		length ;	探索距離
	int		range ;		探索範囲
	int		part_num ;	代表オブジェクト番号( 例 : HUMAN21_ATAMA, HUMAN21_ONAKA )
	HZD_HDL		*hzd ;	ハザード
				これがＮＵＬＬならハザードチェックをしない

	目標までのアジャスト用回転角度を求める
	void GM_TrgToAdjRot( pos, dir, trg, rot )
	FVECTOR	*pos ;	場所
	int		dir ;	方向
	FVECTOR	*trg ;	目標
	SVECTOR	*rot ;	出力

	GM_TrgToAdjRotに補完がついた
	void GM_TrgToNearRot( pos, dir, trg, old_rot )

	GM_TrgToNearRotにヒューマン型リミット付き
	void GM_TrgToNearRotLimit( pos, dir, trg, old_rot )

	指定された場所を向く
	void GM_TrgToNearRotAimLimit( pos, dir, before_dir, trg, old_rot, interp, rlim )
	FVECTOR	*pos ;			場所
	int		dir ;			方向
	int		before_dir ;	前フレーム方向
	FVECTOR	*trg ;			目標位置
	SVECTOR	*old_rot ;		モーションアジャストバッファ
	int		interp ;		補完係数
	ROTLIMIT	*rlim ;		稼動範囲


	指定された方向を向く
	void GM_DirToNearRotAimLimit( pos, dir, before_dir, trg_dir, old_rot, interp, rlim )
	FVECTOR	*pos ;			場所
	int		dir ;			方向
	int		before_dir ;	前フレーム方向
	int		trg_dir ;		目標方向
	SVECTOR	*old_rot ;		モーションアジャストバッファ
	int		interp ;		補完係数
	ROTLIMIT	*rlim ;		稼動範囲


	腕と頭と腰を全て計算してフラグも立てる
	void GM_MotionAdjust16( body, trg, dir, parts )
	OBJECT		*body ;		１６関節のオブジェクト
	SVECTOR		*rot ;		目標への回転角度
	int			dir ;		向いている方向
	int			parts 		アジャスト個所（ビットフラグ）
				1: 頭
				2: 右腕
				4: 左腕
				8: 腰

	２１関節用
	void GM_MotionAdjust21( body, trg, dir, parts )

	座標から上下のアジャスト用クォータニオン値を求める
	void GM_AdjustQuatX( from, to, quat )

	座標から左右のアジャスト用クォータニオン値を求める
	void GM_AdjustQuatY( from, to, dir, quat )

	座標から上下左右のアジャスト用クォータニオン値を求める
	void GM_AdjustQuatXY( from, to, dir, quat )
*/

typedef struct {
	HOMING_TRG	*start ;
	HOMING_TRG	*end ;
	int		num ;
} HOMINGSYS	;

static HOMINGSYS	Homing ;

#define	RANGE_HORIZON		1024
#define	RANGE_HORIZON_AIM	2000
#define	LIMIT_UPPER		-768
#define	LIMIT_LOWER		768
#define	LIMIT_LOWER2	1024

#define	BODY16_WEST		1
#define	BODY16_HEAD		6
#define	BODY16_RSHOULD	2
#define	BODY16_LSHOULD	7

#define	BODY21_WEST		1
#define	BODY21_CHEST	2
#define	BODY21_HEAD		12
#define	BODY21_NECK		11
#define	BODY21_RSHOULD	4
#define	BODY21_LSHOULD	8

static	inline	void	MatToVec( w, v )
FMATRIX	*w ;
FVECTOR	*v ;
{
	v->vx = w->m[3][0] ;
	v->vy = w->m[3][1] ;
	v->vz = w->m[3][2] ;
}

/* ---------------------------------------------------------------- */
static	int FVecLen2( vec )
FVECTOR		*vec ;
{
	float		r ;

	r = vec->vx * vec->vx ;
	r += vec->vz * vec->vz ;

#ifdef PSX2	
	return (int)fpu_Sqrt( r ) ;
#else
	return (int)bp_sqrtf( r ) ;   //BP_MATH - emulate PS2 sqrtf
#endif	
}


static	int	FVecDirX( vec )
FVECTOR		*vec ;
{
	float	f, d_xz ;
	int		r  ;

	d_xz = (float)FVecLen2( vec ) ;

	f = atan2f( d_xz, vec->vy ) ;
	r = (int) ( 2048.0F * f / (float)M_PI );
	r &= 4095 ;

	return r ;
}

static	int		FVecLen3( vec )
FVECTOR		*vec ;
{
	FVECTOR		tmp ;

	tmp.vx = vec->vx * vec->vx ;
	tmp.vy = vec->vy * vec->vy ;
	tmp.vz = vec->vz * vec->vz ;
	return (int)bp_sqrtf( tmp.vx + tmp.vy + tmp.vz ) ; //BP_MATH - emulate PS2 sqrtf
}

static short RevisionDir( dir )
short	dir ;
{
	dir &= 4095 ;
	if ( dir >= 2048 ) dir -= 4096 ;
	
	return dir ;
}

static void RevisionRot( rot )
SVECTOR	*rot ;
{
	rot->vx &= 4095 ;
	if ( rot->vx >= 2048 ) rot->vx -= 4096 ;
	rot->vy &= 4095 ;
	if ( rot->vy >= 2048 ) rot->vy -= 4096 ;
	rot->vz = 0 ;
}

/* ---------------------------------------------------------------- */
void	GM_InitHomingSystem( void )
{
	Homing.start = NULL ;
	Homing.end = NULL ;
	Homing.num = 0 ;
}

void	GM_SetHomingTrg( hom, world, body, map, ctrl, status )
HOMING_TRG	*hom ;
FMATRIX	*world ;
OBJECT	*body ;
int		*map ;
CONTROL	*ctrl ;
int		status ;
{
	hom->world = world ;
	hom->body = body ;
	hom->map = map ;

	hom->ctrl = ctrl ;
	hom->status = status ;
	hom->trg = NULL ;
}
void	GM_SetHomingTrgTarget( HOMING_TRG	*hom, TARGET *trg )
{
	hom->trg = trg ;
}

void	GM_PutHomingTrg( hom )
HOMING_TRG	*hom ;
{
	if ( Homing.start == NULL ) {
		Homing.start = hom ;
	}
	hom->before = Homing.end ;
	if ( hom->before != NULL ) {
		hom->before->next = hom ;
	}
	hom->next = NULL ;
	Homing.end = hom ;
	Homing.num ++ ;
//printf(" homing num[%d] start[0x%x]\n",Homing.num, Homing.start);
}

HOMING_TRG	*GM_GetHoming( void )
{
	return Homing.start ;
}

static	void	FreeHomingTrg( hom ) 
HOMING_TRG	*hom ;
{
	if ( hom->before != NULL ) {
		hom->before->next = hom->next ;
	} else {
		Homing.start = hom->next ;
	}
	if ( hom->next != NULL ) {
		hom->next->before = hom->before ;
	} else {
		Homing.end = hom->before ;
	}
	Homing.num -- ;
}
void	GM_FreeHomingTrg( hom )
HOMING_TRG	*hom ;
{
	HOMING_TRG	*cur ;
	
	cur = Homing.start ;
	/* 登録されているかチェック */
	while ( cur != NULL ) {
		if ( cur == hom ) {
			FreeHomingTrg( hom ) ;
			return ;
		}
		cur = cur->next ;
	}
}

HOMING_TRG	*GM_GetHomingNearTrg( pos, dir, length, range, hom_trg, mode, hzx_id )
FVECTOR	*pos ;
int		dir ;
int		length ;
int		range ;
HOMING_TRG	*hom_trg ;	/* 現在ホーミングしているターゲット */
int		mode ;
HZX_GROUP_ID hzx_id;
{
	HOMING_TRG	*hom, *cur, *res ;
	FVECTOR	vec, sub_vec ;
	int		min_dir, sub_dir, tmp_dir, tmp_dis ;
	int		sub_htrg_dir, htrg_dir ;

	htrg_dir = dir ;
	if ( hom_trg != NULL ) {
		/* 現在のターゲット方向計算 */
		MatToVec( hom_trg->world, &vec ) ;
		GV_SubVec3F( &vec, pos, &sub_vec ) ;
		tmp_dis = FVecLen3( &sub_vec ) ;
		if ( tmp_dis < length ) {
			tmp_dir = GV_VecDir2( &sub_vec ) ;
			sub_dir = GV_DiffDirAbs( dir, tmp_dir ) ;
			if ( sub_dir < range ) {
				/* ホーミング範囲内なら */
				htrg_dir = tmp_dir ;
			}
		}
	}

	if ( mode == RIGHT_HOMING ) {
		min_dir = -4095 ;
	} else {
		min_dir = 4095 ;
	}

	hom = Homing.start ;
	res = hom_trg ;

	while( hom != NULL ) {
		cur = hom ;
		hom = hom->next ;
		
		if ( cur->status & HOMING_SKIP )	continue ;
		if ( cur == hom_trg )	continue ;

		MatToVec( cur->world, &vec ) ;
		GV_SubVec3F( &vec, pos, &sub_vec ) ;
		tmp_dis = FVecLen3( &sub_vec ) ;
		if ( tmp_dis > length )	continue ;

		tmp_dir = GV_VecDir2( &sub_vec ) ;
		sub_dir = GV_DiffDirAbs( dir, tmp_dir ) ;
		if ( sub_dir > range )	continue ;

		sub_htrg_dir = GV_DiffDirS( htrg_dir, tmp_dir ) ;
//printf("mode[%d] h[%d] t[%d] [%d]\n",mode, htrg_dir, tmp_dir,sub_htrg_dir);
		if ( mode == RIGHT_HOMING ) {
			if ( sub_htrg_dir > 0  || sub_htrg_dir < min_dir )	continue ;
		} else if ( mode == LEFT_HOMING ) {
			if ( sub_htrg_dir < 0  || sub_htrg_dir > min_dir )	continue ;
		}
#if 1
		if ( hzx_id != 0 ) {
			if ( HZX_OnlineHazardCheck( hzx_id, pos, &vec, HZX_CHK_ALL, 
						   HZX_SEG_NO_BULLET | HZX_SEG_RECOIL_TYPE,
						   HZX_FLOOR_NO_BULLET | HZX_FLOOR_RECOIL_TYPE |
						   HZX_FLOOR_IK ) ) {
				continue ;
			}
		}
#endif
		min_dir = sub_htrg_dir ;
		res = cur ;
	}

	return res ;
}

HOMING_TRG	*GM_GetHomingTrg( pos, dir, length, range, hzx_id )
FVECTOR	*pos ;
int		dir ;
int		length ;
int		range ;
HZX_GROUP_ID hzx_id;
{
	HOMING_TRG	*hom, *cur, *res ;
	FVECTOR	vec, sub_vec ;
	int		dis, min_dis, t_dir, sub_dir ;

	min_dis = length + 100 ;

	hom = Homing.start ;
	res = NULL ;

	while( hom != NULL ) {
		cur = hom ;
		hom = hom->next ;

		if ( cur->status & HOMING_SKIP )	continue ;

		MatToVec( cur->world, &vec ) ;
		GV_SubVec3F( &vec, pos, &sub_vec ) ;
		dis = FVecLen3( &sub_vec ) ;
		if ( dis > min_dis )	continue ;

		t_dir = GV_VecDir2( &sub_vec ) ;
		sub_dir = GV_DiffDirAbs( dir, t_dir ) ;
		if ( sub_dir > range )	continue ;
#if 1
		if ( hzx_id != 0 ) {
		    if ( HZX_OnlineHazardCheck( hzx_id, pos, &vec, HZX_CHK_ALL, 
					        HZX_SEG_NO_BULLET | HZX_SEG_RECOIL_TYPE,
					        HZX_FLOOR_NO_BULLET | HZX_FLOOR_RECOIL_TYPE |
					        HZX_FLOOR_IK ) ) {
			continue ;
		    }
		}
#endif
		min_dis = dis ;
		res = cur ;
	}

	return res ;
}

/* 対象は画面内キャラ */
HOMING_TRG	*GM_GetHomingTrgInSight( FVECTOR *pos, SVECTOR *dir, SVECTOR *limit,
									 HOMING_TRG *prev, int homing_dir, 
									 HZX_GROUP_ID hzx_id )
{
	HOMING_TRG	*hom, *cur, *res ;
	FVECTOR		vec, sub_vec, *vec2 ;
	SVECTOR		dirL, rot ;
	float		dis, min_dis ;
	int			min_diffdir, diffdir, diffdir2 ;
	int			num, c ;

	hom = Homing.start ;
	res = NULL ;
	dirL = *dir ;
	num = Homing.num ;
	if ( prev != NULL ) {
		num = 0 ;
		while ( hom != prev ) {
			if ( ++ num >= Homing.num ) break ;
			hom = hom->next ;
		}
		if ( num >= Homing.num ) {	/* prev リストに無い */
			hom = Homing.start ;
			num = Homing.num ;			
		} else {
			num = Homing.num ;
			hom = hom->next ; /* prev の次から検索開始 */
		}
	} else {
		homing_dir = CENTER_HOMING ;
	}
	min_diffdir = 10000000 ;
	min_dis = 10000000.0F ;
	c = 0 ;
	for ( ; num > 0; num -- ) {
		if ( hom == NULL ) hom = Homing.start ;
		cur = hom ; 
		hom = hom->next ;
		if ( prev != NULL && cur == prev && c == 1 ) continue ;
		if ( cur->status & ( HOMING_SKIP | HOMING_UNREAL ) ) continue ;
		GV_MatToVec( cur->world, &vec ) ;
		dis = GV_VecLen3F2( pos, &vec ) ;
		/* ６ｍ以上 ＆ 画面外 */


		if ( cur->body != NULL ) {
			if ( dis > 6000.0F && DG_ObjsBoundCheckChanl( cur->body->objs, 0 ) ) continue ;
		} else {
			if ( dis > 6000.0F ) continue ;
		}
//		if ( cur->body->objs->bound_mode == 2 ) continue ; /* 画面外 */
		_sceVu0SubVector( &sub_vec, &vec, pos ) ;
		/* 角度チェック */
		GV_VecToRot( &sub_vec, &rot ) ;
#if 1
		/* 左右 */
		if ( prev != NULL && cur != prev ) {
			diffdir = GV_DiffDirS( dirL.vy, rot.vy ) ;
			if ( homing_dir == RIGHT_HOMING && diffdir > 0 ) {
printf( "homing.c : right max\n" ) ;
			    continue ;
			}
			if ( homing_dir == LEFT_HOMING  && diffdir < 0 ) {
printf( "homing.c : left max\n" ) ;
			    continue ;
			}
		}
#endif
		diffdir = GV_DiffDirAbs( dirL.vy, rot.vy ) ;
		if ( limit->vy > 0 && diffdir > limit->vy ) {
			//printf( "homing.c :limit y over\n" ) ;
			continue ;
		}
		diffdir2 = GV_DiffDirAbs( dirL.vx, rot.vx ) ;
		if ( limit->vx > 0 && diffdir2 > limit->vx ) {
			//printf( "homing.c : limit x over\n" ) ;
			continue ;
		}
		diffdir += diffdir2 ;
		if ( diffdir > min_diffdir ) continue ;
  		/* 距離チェック */
		if ( dis > min_dis ) continue ;
		/* 壁チェック */
		if ( cur->body != NULL && cur->body->objs->n_models > HUMAN21_ATAMA ) {
			/* 人間モデルっぽいなら頭位置でも壁チェック */
			vec2 = ( FVECTOR * )cur->body->objs->objs[ HUMAN21_ATAMA ].world.m[ 3 ] ;
		} else {
			vec2 = NULL ;
		}
		if ( hzx_id != 0 ) {
//			ViewFromTo( pos, &vec, 32, 232, 32 ) ;
			if ( HZX_OnlineHazardCheck( hzx_id, pos, &vec, HZX_CHK_ALL,
									    HZX_SEG_NO_ENEMY_EYES,
									    HZX_FLOOR_NO_ENEMY_EYES | HZX_FLOOR_IK ) ) {
				if ( vec2 != NULL ) {
//					ViewFromTo( pos, vec2, 32, 232, 32 ) ;
					if ( HZX_OnlineHazardCheck( hzx_id, pos, vec2, HZX_CHK_ALL,
											   HZX_SEG_NO_ENEMY_EYES,
											   HZX_FLOOR_NO_ENEMY_EYES | HZX_FLOOR_IK ) ) {				   

#if 0
		    {
			HZX_FLR   level ;
			int       atr ;
			HZX_GetOnlineHazard( &level, &atr ) ;
			HZX_ViewHazard( &level ) ;			
		    }
#endif
						continue ;
					}
				} else {
					continue ;
				}
			}
			if ( HZX_OnlineHazardCheck( hzx_id, pos, &vec, HZX_CHK_ALL,
									    HZX_SEG_NO_BULLET | HZX_SEG_NO_MISSILE,
									    HZX_FLOOR_NO_BULLET | HZX_FLOOR_NO_MISSILE | 
									    HZX_FLOOR_IK ) ) {
				if ( vec2 != NULL ) {
//					ViewFromTo( pos, vec2, 32, 232, 32 ) ;
					if ( HZX_OnlineHazardCheck( hzx_id, pos, vec2, HZX_CHK_ALL,
											   HZX_SEG_NO_BULLET | HZX_SEG_NO_MISSILE,
											   HZX_FLOOR_NO_BULLET | HZX_FLOOR_NO_MISSILE | 
											   HZX_FLOOR_IK ) ) {
						continue ;
					}
				} else {
					continue ;
				}
			}
		}
		min_diffdir = diffdir ;
		min_dis = dis ;
		c = 1 ;
		res = cur ;
	}
	return res ;
}

int		GM_CheckHomingEnable( HOMING_TRG *hom, FVECTOR *pos, SVECTOR *dir, SVECTOR *limit,
							  HZX_GROUP_ID hzx_id )
{
	HOMING_TRG	*backup_start, *backup_end, *backup_next ;
	HOMING_TRG	*res ;
	int			backup_num ;

	/* 先に存在チェック */
	{
		int			num ;
		HOMING_TRG	*list ;

		list = Homing.start ;
		num = Homing.num ;
		for ( ; num > 0; num -- ) {		
			if ( list == hom ) break ;
			list = list->next ;
		}
		if ( num == 0 ) return 0 ;	/* 存在せず */
	}

	backup_start = Homing.start ;
	backup_end = Homing.end ;
	backup_num = Homing.num ;
	backup_next = hom->next ;

	Homing.start = hom ;
	Homing.end = hom ;
	Homing.num = 1 ;
	hom->next = NULL ;

	res = GM_GetHomingTrgInSight( pos, dir, limit, NULL, CENTER_HOMING, hzx_id ) ;
	
	Homing.start = backup_start ;
	Homing.end = backup_end ;
	Homing.num = backup_num ;
	hom->next = backup_next ;

	if ( res != NULL ) return 1 ;
	return 0 ;
}

/* ---------------------------------------------------------------- */

void GM_RotToQuat( rot, quat )
SVECTOR	*rot ;
FVECTOR	*quat ;
{
	FVECTOR	radi ;

	vu0_PSDegV2RadV( rot, &radi ) ;
	MT_EulerToQuatXYZ( quat, &radi ) ;
	MT_QuatNormalize( quat, quat);

}

void GM_RotToQuatXAfterY( rot, quat )
SVECTOR	*rot ;
FVECTOR	*quat ;
{
	FVECTOR	radi ;
	float	r ;

	vu0_PSDegV2RadV( rot, &radi ) ;

	quat->vw = (float)cosf( radi.vx / 2.0F );
	r = bp_sqrtf( 1.0F - quat->vw * quat->vw ) ; //BP_MATH - emulate PS2 sqrtf

	if ( radi.vx < 0 )	r = -r ;

	quat->vx = cosf( radi.vy ) * r ;
	quat->vz = -sinf( radi.vy ) * r ;
	quat->vy = 0.0f ;
}


void GM_TrgToAdjRot( pos, dir, trg, rot )
FVECTOR	*pos ;
int		dir ;
FVECTOR	*trg ;
SVECTOR	*rot ;
{
	FVECTOR	vec ;
	int r ;
	
	_sceVu0SubVector( &vec, trg, pos ) ;
	rot->vx = (short)FVecDirX( &vec ) - 1024 ;
	
	r = GV_VecDir2( &vec ) ;
	rot->vy = (short)GV_DiffDirS( dir, r ) ;
	rot->vz = 0 ;
}

void GM_TrgToNearRot( pos, dir, trg, old_rot )
FVECTOR	*pos ;
int		dir ;
FVECTOR	*trg ;
SVECTOR	*old_rot ;
{
	SVECTOR	rot ;
	
	GM_TrgToAdjRot( pos, dir, trg, &rot ) ;
	RevisionRot( old_rot ) ;
	GV_NearExp8PV( old_rot, &rot, 2 ) ;
}

void GM_TrgToNearRotLimit( pos, dir, trg, old_rot )
FVECTOR	*pos ;
int		dir ;
FVECTOR	*trg ;
SVECTOR	*old_rot ;
{
	SVECTOR	rot ;
	
	GM_TrgToAdjRot( pos, dir, trg, &rot ) ;
	/* 改良の余地あり */
	if ( rot.vy > RANGE_HORIZON || rot.vy < -RANGE_HORIZON ||
		 rot.vx < LIMIT_UPPER || rot.vx > LIMIT_LOWER ) {
		return ;
	}
	RevisionRot( old_rot ) ;
	GV_NearExp8PV( old_rot, &rot, 2 ) ;
}

/* りみっとまでは近づける*/
void GM_TrgToNearRotLimit2( pos, dir, trg, old_rot )
FVECTOR	*pos ;
int		dir ;
FVECTOR	*trg ;
SVECTOR	*old_rot ;
{
	SVECTOR	rot ;
	
	GM_TrgToAdjRot( pos, dir, trg, &rot ) ;
	if ( rot.vy > RANGE_HORIZON ) rot.vy = RANGE_HORIZON ;
	if ( rot.vy < -RANGE_HORIZON ) rot.vy = -RANGE_HORIZON ;
	if ( rot.vx < LIMIT_UPPER ) rot.vx = LIMIT_UPPER ;
	if ( rot.vx > LIMIT_LOWER ) rot.vx = LIMIT_LOWER ;
	
	RevisionRot( old_rot ) ;
	GV_NearExp8PV( old_rot, &rot, 2 ) ;
}

void GM_TrgToNearRotLimitX( pos, dir, trg, old_rot )
FVECTOR	*pos ;
int		dir ;
FVECTOR	*trg ;
SVECTOR	*old_rot ;
{
	SVECTOR	rot ;
	
	GM_TrgToAdjRot( pos, dir, trg, &rot ) ;
	/* 改良の余地あり */
	if ( rot.vy > RANGE_HORIZON || rot.vy < -RANGE_HORIZON ||
		 rot.vx < LIMIT_UPPER || rot.vx > LIMIT_LOWER ) {
		return ;
	}
	RevisionRot( old_rot ) ;
	GV_NearExp8V( old_rot, &rot, 1 ) ;
	old_rot->vy = rot.vy ;
}

void GM_TrgToNearRotAimLimit( pos, dir, before_dir, trg, old_rot, interp, speed, rlim )
FVECTOR	*pos ;
int		dir ;
int		before_dir ;
FVECTOR	*trg ;
SVECTOR	*old_rot ;
int		interp ;
int		speed ;
ROTLIMIT	*rlim ;
{
	SVECTOR	rot ;
	int old_dir, x ;
	
	GM_TrgToAdjRot( pos, dir, trg, &rot ) ;
	if ( rot.vy > rlim->right ) {
		rot.vy = rlim->right ;
	} else if ( rot.vy < -rlim->left ) {
		rot.vy = -rlim->left ;
	}
	if ( rot.vx < -rlim->upper ) {
		rot.vx = -rlim->upper ;
	} else if ( rot.vx > rlim->lower ) {
		rot.vx = rlim->lower ;
	}
	RevisionRot( old_rot ) ;

	/* Ｙ方向は前フレームのアジャスト後の方向から補完 */
	old_dir = before_dir + old_rot->vy ;
	if ( speed != 0 ) {
		old_rot->vx = GV_NearSpeed( old_rot->vx, rot.vx, speed ) ;
		x = GV_NearSpeed( old_dir, rot.vy + dir, speed ) ;
	} else if ( interp == 0 ) {
		old_rot->vx = GV_NearExp8P( old_rot->vx, rot.vx ) ;
		x = GV_NearExp8P( old_dir, rot.vy + dir ) ;
	} else {
		old_rot->vx = GV_NearTimeP( old_rot->vx, rot.vx, interp ) ;
		x = GV_NearTimeP( old_dir, rot.vy + dir, interp ) ;
	}
	old_rot->vy = RevisionDir( (short)(x - dir) ) ;

#if 1	//11.16
	if ( old_rot->vy > rlim->right ) {
		old_rot->vy = rlim->right ;
	} else if ( old_rot->vy < -rlim->left ) {
		old_rot->vy = -rlim->left ;
	}
#endif
//printf("befor=%d old=%d x=%d vy=%d dir%d res=%d\n",before_dir, old_dir, x, rot.vy, dir, old_rot->vy ) ;
}

void GM_DirToNearRotAimLimit( pos, dir, before_dir, trg_dir, old_rot, interp, speed, rlim )
FVECTOR	*pos ;
int		dir ;
int		before_dir ;
int		trg_dir ;
SVECTOR	*old_rot ;
int		interp ;
int		speed ;
ROTLIMIT	*rlim ;
{
	SVECTOR	rot ;
	int old_dir, x ;
	
//	rot.vx = old_rot->vx ;
	rot.vx = 0 ;
	rot.vy = (short)GV_DiffDirS( dir, trg_dir ) ;
	rot.vz = 0 ;
	if ( rot.vy > rlim->right ) {
		rot.vy = rlim->right ;
	} else if ( rot.vy < -rlim->left ) {
		rot.vy = -rlim->left ;
	}
	if ( rot.vx < -rlim->upper ) {
		rot.vx = -rlim->upper ;
	} else if ( rot.vx > rlim->lower ) {
		rot.vx = rlim->lower ;
	}
	RevisionRot( old_rot ) ;

	/* Ｙ方向は前フレームのアジャスト後の方向から補完 */
	old_dir = before_dir + old_rot->vy ;
	if ( speed != 0 ) {
		old_rot->vx = GV_NearSpeed( old_rot->vx, rot.vx, speed ) ;
		x = GV_NearSpeed( old_dir, rot.vy + dir, speed ) ;
	} else if ( interp == 0 ) {
		old_rot->vx = GV_NearExp8P( old_rot->vx, rot.vx ) ;
		x = GV_NearExp8P( old_dir, rot.vy + dir ) ;
	} else {
		old_rot->vx = GV_NearTimeP( old_rot->vx, rot.vx, interp ) ;
		x = GV_NearTimeP( old_dir, rot.vy + dir, interp ) ;
	}
	old_rot->vy = RevisionDir( (short)(x - dir) ) ;

//printf("befor=%d old=%d x=%d vy=%d dir%d res=%d\n",before_dir, old_dir, x, rot.vy, dir, old_rot->vy ) ;
}

void GM_MotionAdjust16( body, trg_rot, dir, parts )
OBJECT		*body ;
SVECTOR		*trg_rot ;
int			dir ;
int			parts ;
{
	MOTION_CONTROL	*m_ctrl ;
	FVECTOR		quat ;
	SVECTOR		rot ;

	m_ctrl = body->m_ctrl ;

	rot.vx = 0 ;
	rot.vy = 0 ;
	rot.vz = 0 ;
	if ( parts & ADJ_PARTS_WEST ) {
		rot.vy = trg_rot->vy ;
		GM_RotToQuat( &rot, &quat ) ;
		m_ctrl->adjust[ BODY16_WEST ] = quat ;
		m_ctrl->adjust_flag |= 1 << BODY16_WEST ;
	}

	rot.vx = trg_rot->vx ;
	GM_RotToQuatXAfterY( &rot, &quat ) ;

	if ( parts & ADJ_PARTS_HEAD ) {
		m_ctrl->adjust[ BODY16_HEAD ] = quat ;
		m_ctrl->adjust_flag |= 1 << BODY16_HEAD ;
	}
	if ( parts & ADJ_PARTS_RARM ) {
		m_ctrl->adjust[ BODY16_RSHOULD ] = quat ;
		m_ctrl->adjust_flag |= 1 << BODY16_RSHOULD ;
	}
	if ( parts & ADJ_PARTS_LARM ) {
		m_ctrl->adjust[ BODY16_LSHOULD ] = quat ;
		m_ctrl->adjust_flag |= 1 << BODY16_LSHOULD ;
	}
}

void GM_MotionAdjust21( body, trg_rot, dir, parts )
OBJECT		*body ;
SVECTOR		*trg_rot ;
int			dir ;
int			parts ;
{
	MOTION_CONTROL	*m_ctrl ;
	FVECTOR		quat ;
	SVECTOR		rot ;

	m_ctrl = body->m_ctrl ;

	rot.vx = 0 ;
	rot.vy = 0 ;
	rot.vz = 0 ;
	if ( parts & ADJ_PARTS_WEST ) {
		if ( trg_rot->vy != trg_rot->pad ) {
			rot.vy = GV_NearExp8P( trg_rot->pad, trg_rot->vy ) ;
		} else {
			rot.vy = trg_rot->vy ;
		}
		trg_rot->pad = rot.vy ;
		GM_RotToQuat( &rot, &quat ) ;
		m_ctrl->adjust[ BODY21_WEST ] = quat ;
		m_ctrl->adjust_flag |= 1 << BODY21_WEST ;
	}

	rot.vx = trg_rot->vx ;
	GM_RotToQuatXAfterY( &rot, &quat ) ;

	if ( parts & ADJ_PARTS_HEAD ) {
		m_ctrl->adjust[ BODY21_HEAD ] = quat ;
		m_ctrl->adjust_flag |= 1 << BODY21_HEAD ;
	}
	if ( parts & ADJ_PARTS_RARM ) {
		m_ctrl->adjust[ BODY21_RSHOULD ] = quat ;
		m_ctrl->adjust_flag |= 1 << BODY21_RSHOULD ;
	}
	if ( parts & ADJ_PARTS_LARM ) {
		m_ctrl->adjust[ BODY21_LSHOULD ] = quat ;
		m_ctrl->adjust_flag |= 1 << BODY21_LSHOULD ;
	}
}

void GM_MotionAdjustWest( body, trg_rot, dir, parts )
OBJECT		*body ;
SVECTOR		*trg_rot ;
int			dir ;
int			parts ;
{
	MOTION_CONTROL	*m_ctrl ;
	FVECTOR		quat ;
	SVECTOR		rot ;

	m_ctrl = body->m_ctrl ;

	rot.vx = 0 ;
	rot.vy = 0 ;
	rot.vz = 0 ;
	if ( parts & ADJ_PARTS_WEST ) {/* 左右方向の回転 */
		if ( trg_rot->vy != trg_rot->pad ) {
			rot.vy = GV_NearExp8P( trg_rot->pad, trg_rot->vy ) ;
		} else {
			rot.vy = trg_rot->vy ;
		}
		trg_rot->pad = rot.vy ;
		GM_RotToQuat( &rot, &quat ) ;
		m_ctrl->adjust[ HUMAN21_ONAKA ] = quat ;
		m_ctrl->adjust_flag |= 1 << HUMAN21_ONAKA ;
	}

	rot.vx = trg_rot->vx ;
	GM_RotToQuatXAfterY( &rot, &quat ) ;

	m_ctrl->adjust[ HUMAN21_MUNE ] = quat ;
	m_ctrl->adjust_flag |= 1 << HUMAN21_MUNE ;
}


void GM_MotionAdjustAim( body, trg_rot, dir, parts, interp )
OBJECT		*body ;
SVECTOR		*trg_rot ;
int			dir ;
int			parts ;
int			interp ;
{
	MOTION_CONTROL	*m_ctrl ;
	FVECTOR		quat ;
	SVECTOR		rot ;

	m_ctrl = body->m_ctrl ;

	rot.vx = 0 ;
	rot.vy = 0 ;
	rot.vz = 0 ;

#if 1
	if ( parts & ADJ_PARTS_WEST_HALF ) {
		if ( interp == 0 ) {
			rot.vy = GV_NearExp8P( trg_rot->pad, trg_rot->vy/2 ) ;
		} else {
			rot.vy = GV_NearTimeP( trg_rot->pad, trg_rot->vy/2, interp ) ;
		}
		trg_rot->pad = rot.vy ;
		GM_RotToQuat( &rot, &quat ) ;
		m_ctrl->adjust[ BODY21_CHEST ] = quat ;
		m_ctrl->adjust_flag |= 1 << BODY21_CHEST ;
	} else if ( parts & ADJ_PARTS_WEST ) {
		if ( interp == 0 ) {
			rot.vy = GV_NearExp8P( trg_rot->pad, trg_rot->vy ) ;
		} else {
			rot.vy = GV_NearTimeP( trg_rot->pad, trg_rot->vy, interp ) ;
		}
		trg_rot->pad = rot.vy ;
		GM_RotToQuat( &rot, &quat ) ;
		m_ctrl->adjust[ BODY21_CHEST ] = quat ;
		m_ctrl->adjust_flag |= 1 << BODY21_CHEST ;
	}

	if ( parts & ADJ_PARTS_HEAD ) {
		if ( parts & (ADJ_PARTS_WEST_HALF|ADJ_PARTS_WEST) ) {
			rot.vy = trg_rot->vy - trg_rot->pad ;
			GM_RotToQuat( &rot, &quat ) ;
			m_ctrl->adjust[ BODY21_NECK ] = quat ;
			m_ctrl->adjust_flag |= 1 << BODY21_NECK ;
			rot.vy = trg_rot->vy ;
			rot.vx = trg_rot->vx ;
//printf("head vy[%d] vx[%d]\n",rot.vy,rot.vx ) ;
			GM_RotToQuatXAfterY( &rot, &quat ) ;
		} else {
			if ( trg_rot->pad != 0 ) {
				if ( interp == 0 ) {
					rot.vy = GV_NearExp8P( trg_rot->pad, 0 ) ;
				} else {
					rot.vy = GV_NearTimeP( trg_rot->pad, 0, interp ) ;
				}
				trg_rot->pad = rot.vy ;
				GM_RotToQuat( &rot, &quat ) ;
				m_ctrl->adjust[ BODY21_CHEST ] = quat ;
				m_ctrl->adjust_flag |= 1 << BODY21_CHEST ;
				rot.vy = trg_rot->vy ;
				rot.vx = trg_rot->vx ;
				GM_RotToQuatXAfterY( &rot, &quat ) ;
			} else {
				rot.vy = trg_rot->vy ;
				rot.vx = trg_rot->vx ;
				GM_RotToQuat( &rot, &quat ) ;
			}
		}
		m_ctrl->adjust[ BODY21_HEAD ] = quat ;
		m_ctrl->adjust_flag |= 1 << BODY21_HEAD ;
	}

	if ( parts & (ADJ_PARTS_RARM|ADJ_PARTS_LARM) ) {
		if ( parts & (ADJ_PARTS_WEST_HALF|ADJ_PARTS_WEST) ) {
			rot.vy = trg_rot->pad ;
			rot.vx = trg_rot->vx ;
			GM_RotToQuatXAfterY( &rot, &quat ) ;
		} else {
			if ( trg_rot->pad != 0 ) {
				if ( interp == 0 ) {
					rot.vy = GV_NearExp8P( trg_rot->pad, 0 ) ;
				} else {
					rot.vy = GV_NearTimeP( trg_rot->pad, 0, interp ) ;
				}
				trg_rot->pad = rot.vy ;
				rot.vx = trg_rot->vx ;
				GM_RotToQuatXAfterY( &rot, &quat ) ;
			} else {
				rot.vy = 0 ;
				rot.vx = trg_rot->vx ;
				GM_RotToQuat( &rot, &quat ) ;
			}
		}

		if ( parts & ADJ_PARTS_RARM ) {
			m_ctrl->adjust[ BODY21_RSHOULD ] = quat ;
			m_ctrl->adjust_flag |= 1 << BODY21_RSHOULD ;
		}
		if ( parts & ADJ_PARTS_LARM ) {
			m_ctrl->adjust[ BODY21_LSHOULD ] = quat ;
			m_ctrl->adjust_flag |= 1 << BODY21_LSHOULD ;
		}
	}

#else
	if ( parts & ADJ_PARTS_WEST ) {
		if ( parts & ADJ_PARTS_HEAD ) {
			rot.vy = GV_NearExp8P( trg_rot->pad, trg_rot->vy/2 ) ;
			trg_rot->pad = rot.vy ;
		} else {
			rot.vy = GV_NearExp8P( trg_rot->pad, trg_rot->vy ) ;
			trg_rot->pad = rot.vy ;
		}
		GM_RotToQuat( &rot, &quat ) ;
		m_ctrl->adjust[ BODY21_CHEST ] = quat ;
		m_ctrl->adjust_flag |= 1 << BODY21_CHEST ;
	}
	if ( parts & ADJ_PARTS_HEAD ) {
		if ( parts & ADJ_PARTS_WEST ) {
			rot.vy = trg_rot->vy - trg_rot->pad ;
//			rot.vy = -trg_rot->pad ;

//printf(" kubi y=%d adj=%d pad=%d\n",rot.vy,trg_rot->vy, trg_rot->pad ) ;
			GM_RotToQuat( &rot, &quat ) ;
			m_ctrl->adjust[ BODY21_NECK ] = quat ;
			m_ctrl->adjust_flag |= 1 << BODY21_NECK ;
			rot.vy = trg_rot->vy ;
//			rot.vy = 0 ;
			if ( !(parts&ADJ_PARTS_Y_ONLY) )	rot.vx = trg_rot->vx ;
			GM_RotToQuatXAfterY( &rot, &quat ) ;
		} else {
			if ( trg_rot->pad != 0 ) {
				rot.vy = GV_NearExp8P( trg_rot->pad, 0 ) ;
				trg_rot->pad = rot.vy ;
				GM_RotToQuat( &rot, &quat ) ;
				m_ctrl->adjust[ BODY21_CHEST ] = quat ;
				m_ctrl->adjust_flag |= 1 << BODY21_CHEST ;
			}
			rot.vy = trg_rot->vy - trg_rot->pad ;
			if ( !(parts&ADJ_PARTS_Y_ONLY) )	rot.vx = trg_rot->vx ;
			GM_RotToQuat( &rot, &quat ) ;
		}
		m_ctrl->adjust[ BODY21_HEAD ] = quat ;
		m_ctrl->adjust_flag |= 1 << BODY21_HEAD ;
	}

	if ( parts & (ADJ_PARTS_RARM|parts & ADJ_PARTS_LARM) ) {
		if ( parts & ADJ_PARTS_WEST && parts & ADJ_PARTS_HEAD ) {
			rot.vy = trg_rot->vy/2 ;
		} else {
			rot.vy = trg_rot->vy ;
		}
		rot.vx = trg_rot->vx/2 ;
		GM_RotToQuatXAfterY( &rot, &quat ) ;

		if ( parts & ADJ_PARTS_RARM ) {
			m_ctrl->adjust[ BODY21_RSHOULD ] = quat ;
			m_ctrl->adjust_flag |= 1 << BODY21_RSHOULD ;
		}
		if ( parts & ADJ_PARTS_LARM ) {
			m_ctrl->adjust[ BODY21_LSHOULD ] = quat ;
			m_ctrl->adjust_flag |= 1 << BODY21_LSHOULD ;
		}
	}
#endif

}

void GM_AdjustQuatX( from, to, quat )
FVECTOR	*from ;	/* こっから */
FVECTOR	*to ;	/* あそこへは */
FVECTOR	*quat ;	/* 上下はこんだけアジャストすればよい */
{
	FVECTOR	vec ;
	SVECTOR	rot ;

	GV_SubVec3F( to, from, &vec ) ;
	
	rot.vx = (short)FVecDirX( &vec ) - 1024 ;
	rot.vy = 0 ;
	rot.vz = 0 ;
	GM_RotToQuat( &rot, quat ) ;
}

void GM_AdjustQuatY( from, to, dir, quat )
FVECTOR	*from ;	/* こっから */
FVECTOR	*to ;	/* あそこへは */
int		dir ;	/* この方向なら */
FVECTOR	*quat ;	/* 左右はこんだけアジャストすればよい */
{
	FVECTOR	vec ;
	SVECTOR	rot ;

	GV_SubVec3F( to, from, &vec ) ;
	rot.vx = 0 ;
	rot.vy = (short)GV_DiffDirS( dir, GV_VecDir2( &vec ) ) ;
	rot.vz = 0 ;
	GM_RotToQuat( &rot, quat ) ;
}

void GM_AdjustQuatXY( from, to, dir, quat )
FVECTOR	*from ;	/* こっから */
FVECTOR	*to ;	/* あそこへは */
int		dir ;	/* この方向なら */
FVECTOR	*quat ;	/* こんだけアジャストすればよい */
{
	FVECTOR	vec ;
	SVECTOR	rot ;

	GV_SubVec3F( to, from, &vec ) ;
	rot.vx = (short)FVecDirX( &vec ) - 1024 ;
	rot.vy = (short)GV_DiffDirS( dir, GV_VecDir2( &vec ) ) ;
	rot.vz = 0 ;
	GM_RotToQuat( &rot, quat ) ;
}

void GM_AdjustRotBody( body, rot, body_num )
OBJECT	*body ;
SVECTOR	*rot ;
int		body_num ;
{
	FVECTOR		quat ;

	GM_RotToQuat( rot, &quat ) ;
	body->m_ctrl->adjust[ body_num ] = quat ;
	body->m_ctrl->adjust_flag |= I64(1) << body_num ;
}

