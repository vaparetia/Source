//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	shdwctrl.c
	影管理キャラ

	2000/08/09 K.Takabe
	$Id: shdwctrl.c,v 1.1.1.3 2002/11/19 11:51:15 Yoshizawa1 Exp $

*/
/*

chara 影管理[NewShadowControl] $s:name \
	-type $b:光源タイプ \
	-length $i:光源距離（並行光源時のみ有効） \
	-parallel $v:並行光源方向直接指定（省略するとステージライトと同じになる） \
	-alpha $b:影濃度（平行光源時のみ有効０～１２８まで）
mesg 影管理 $s:名前 add[0] $s:追加キャラ [$s:投影モデル名指定]
mesg 影管理 $s:名前 del[1] $s:削除キャラ
mesg 影管理 $s:名前 demo_add[2] $s:追加キャラ [$s:投影モデル名指定]
mesg 影管理 $s:名前 parallel[3] $v:並行光源方向指定
// 光源タイプ
//   0:無限遠並行光源（ステージの平行光に対して）
//   1:点光源エミュレーション（影フラグのついた点光源に対して）
// メッセージにより追加、削除ができるので部屋の入口のトラップなどに
// 引っかけて使用すると良い
// 投影モデル名指定を指定することで特定の投影モデルのみに投影することが可能
// また、すでに影を表示しているキャラに対してさらにaddでメッセージを
// 送ると投影モデル名のみの変更が可能です。

*/

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

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"g_other.h"


#define MAX_SHADOWS		(64)

/* ---------------------------------------------------------------- */
typedef struct {
	int			name ;			/* オリジナルオブジェクトの名前 */
	DG_OBJS		*objs ;			/* 影生成オブジェクト */
	int			fade_count ;	/* フェードカウント */
	int			mode ;			/* モード */
	float		old_power ;		/* 旧点光源強度 */
	int			fade_speed ;	/* フェード変化量 */
	int			old_lit_id ;	/* 旧ライトＩＤ */
	int			pad[1] ;
	FVECTOR		old_pos ;		/* 旧点光源位置 */
	DG_SPOT		spot ;			/* 影生成用スポットライト */
} SHADOW_WORK ;

typedef	struct	{
	GV_ACT		actor ;
	int			name ;			/* キャラ名 */
	int			map ;			/* キャラ存在マップ */
	int			type ;			/* 影タイプ */
	int			ppl_alpha ;		/* 平行光源時のデフォルトアルファ値（＝光源強さ） */
	float		lt_length ;		/* 平行光源時の光源設置距離 */
	int			im_light_flag ;	/* 平行光源直接指定モードフラグ */
	FVECTOR		im_light_vec ;	/* 直接指定並行光源ベクトル */
	SHADOW_WORK	shadow[MAX_SHADOWS] ;
} Work ;

#define MAX_SPOT_CHECK_WORK	(8)
//static int	MaxSpotList = 0 ;
static GCT_ProjectionSpot_SpotParam	SpotList[ MAX_SPOT_CHECK_WORK ];


/* ---------------------------------------------------------------- */
#if 0
static void DG_MakeCameraMatrix( FMATRIX *mat, FVECTOR *from, FVECTOR *to )
{
	static FVECTOR	Lower = { 0.0F, -1.0F, 0.0F, 1.0F } ;
	static FVECTOR	prev_vx;
	FVECTOR		z_vec, y_vec, x_vec ;

	mat->m[3][ 0 ] = from->vx ;
	mat->m[3][ 1 ] = from->vy ;
	mat->m[3][ 2 ] = from->vz ;
	mat->m[3][ 3 ] = 1.0 ;
	/*
		カメラ行列を計算する
		Ｚ軸は、「カメラ位置→注目点」ベクトルを正規化
		Ｘ軸は、「下向きベクトル」×Ｚ軸の結果を正規化
		Ｙ軸は、Ｚ軸×Ｘ軸
	*/
	z_vec.vx = ( to->vx - from->vx ) ;
	z_vec.vy = ( to->vy - from->vy ) ;
	z_vec.vz = ( to->vz - from->vz ) ;
	_sceVu0OuterProduct( &x_vec, &Lower, &z_vec ) ;
	if( x_vec.vx == 0.0F && x_vec.vy == 0.0F && x_vec.vz == 0.0F ){
		x_vec = prev_vx;
	} else {
		prev_vx = x_vec;
	}

	_sceVu0Normalize( &x_vec, &x_vec ) ;
	_sceVu0Normalize( &z_vec, &z_vec ) ;
	_sceVu0OuterProduct( &y_vec, &z_vec, &x_vec ) ;
	mat->m[ 0 ][ 0 ] = x_vec.vx ;
	mat->m[ 0 ][ 1 ] = x_vec.vy ;
	mat->m[ 0 ][ 2 ] = x_vec.vz ;
	mat->m[ 0 ][ 3 ] = 0.0F ;
	mat->m[ 1 ][ 0 ] = y_vec.vx ;
	mat->m[ 1 ][ 1 ] = y_vec.vy ;
	mat->m[ 1 ][ 2 ] = y_vec.vz ;
	mat->m[ 1 ][ 3 ] = 0.0F ;
	mat->m[ 2 ][ 0 ] = z_vec.vx ;
	mat->m[ 2 ][ 1 ] = z_vec.vy ;
	mat->m[ 2 ][ 2 ] = z_vec.vz ;
	mat->m[ 2 ][ 3 ] = 0.0F ;
}
#endif
/* ---------------------------------------------------------------- */
/* 空きエントリの取得 */
static SHADOW_WORK* GetShadowEntry( Work *work, int name )
{
	SHADOW_WORK	*shadow ;
	int				i ;

	shadow = work->shadow ;
	for ( i = MAX_SHADOWS ; i > 0 ; shadow++, i-- ){
		if ( shadow->name == 0 ){
			shadow->name = name ;
			return ( shadow );
		}
	}
	printf("shdwctrl.c: not enough shadow entry !!\n");
	return ( NULL );
}

/* 影エントリの開放 */
static void FreeShadowEntry( SHADOW_WORK *shadow )
{
	shadow->name = 0 ;
	DG_DequeueSpotObjs( &shadow->spot );
}

/* 指定した名前の影エントリ検索 */
static SHADOW_WORK* SearchShadowEntry( Work *work, int name )
{
	SHADOW_WORK	*shadow ;
	int				i ;

	shadow = work->shadow ;
	for ( i = MAX_SHADOWS ; i > 0 ; shadow++, i-- ){
		if ( shadow->name == name ){
			return ( shadow );
		}
	}
	//printf("shdwctrl.c: search faild shadow entry !!(%d)\n", name );
	return ( NULL );
}

/* 指定した名前のコントロールを検索 */
static CONTROL* SearchControl( Work *work, int name )
{
	int		i ;
	CONTROL	*control, **w_list ;

	w_list = GM_WhereList ;
	for ( i = GM_N_WhereList ; i > 0 ; w_list++, i-- ){
		control = *w_list ;
		if ( control->name != name ) continue ;
		if ( ( control->map & work->map ) == 0 ) continue ;
		return ( control ) ;
	}
	printf("shdwctrl.c: search faild control !!(%d)\n", name );
	return ( NULL );
}

/* 指定した名前のキャラクタの影を追加 */
static void AddShadow( Work *work, int name, int id, int flag )
{
	CONTROL	*control ;
	OBJECT	*object ;
	SHADOW_WORK	*shadow ;

	/* コントロールリストから指定した名前のキャラクタを検索 */
	if ( ( control = SearchControl( work, name ) ) == NULL ) return ;
	if ( ( object = control->object ) == NULL ){
		return ;
	}

	/* すでにワークがある場合にはＩＤのみ変更 */
	if ( ( shadow = SearchShadowEntry( work, name ) ) != NULL ){
		shadow->spot.shadow_id = id ;
		if ( work->type == 0 ){
			shadow->mode = 2 ;
		}
		return ;
	}
	/* 影ワークの確保 */
	if ( ( shadow = GetShadowEntry( work, name ) ) == NULL ) return ;
	shadow->objs = object->objs ;
	if ( flag ) object->objs->flag |= DG_FLAG_SHADOWMAKE ;
	shadow->spot.shadow_id = id ;
	shadow->spot.objs = object->objs ;
	shadow->spot.range = 16000.0f ;
	shadow->spot.color = 0x80808080 ;
	if ( work->type == 0 ){
		shadow->mode = 2 ;
		shadow->fade_count = 0 ;
		shadow->fade_speed = 2 ;
	} else {
		shadow->mode = 1 ;
		shadow->fade_count = 1 ;
		shadow->fade_speed = 2 ;
	}
	DG_QueueSpotObjs( &shadow->spot );
	printf("shdwctrl.c: add shadow name=%d id=%d\n", name, id );
}

/* 指定した名前のキャラクタの影を削除 */
static void DeleteShadow( Work *work, int name )
{
	SHADOW_WORK	*shadow ;

	/* 指定した名前の影ワークを検索 */
	if ( ( shadow = SearchShadowEntry( work, name ) ) == NULL ) return ;

	printf("shdwctrl.c: delete shadow name=%d \n", name );
	FreeShadowEntry( shadow );
}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	GV_MSG		*msg ;
	int			i, n_msg ;
	SHADOW_WORK	*shadow ;
	DG_OBJS		*objs ;

	/* メッセージチェック */
	if( ( n_msg = GV_ReceiveMessage( work->name, &msg ) ) > 0 ){
		for ( ; n_msg > 0 ; n_msg--, msg++ ){
			switch ( msg->message[0] ){
			  case 0:/* 追加 */
			  case 2:/* 追加 */
				if ( msg->message_len == 2 ){
					AddShadow( work, msg->message[1], 0, msg->message[0] == 2 );
				} else {
					AddShadow( work, msg->message[1], msg->message[2], msg->message[0] == 2 );
				}
				break ;
			  case 1:/* 削除 */
				if ( work->type == 0 ){
					SHADOW_WORK	*shadow ;

					/* 指定した名前の影ワークを検索 */
					if ( ( shadow = SearchShadowEntry( work, msg->message[1] ) ) == NULL ) return ;

					printf("shdwctrl.c: delete shadow name=%d \n", msg->message[1] );
					shadow->mode = 1 ;
				} else {
					DeleteShadow( work, msg->message[1] );
				}
				break ;
			  case 3:
				work->im_light_vec.vx = msg->message[1] ;
				work->im_light_vec.vy = msg->message[2] ;
				work->im_light_vec.vz = msg->message[3] ;
				_sceVu0Normalize( &work->im_light_vec, &work->im_light_vec );
				break ;
			}
		}
	}

	/**/

	/* スポットライトオブジェクトの操作 */
	shadow = work->shadow ;
	for ( i = MAX_SHADOWS ; i > 0 ; shadow++, i-- ){
		FVECTOR		light_dir ;
		if ( shadow->name == 0 ) continue ;

		/* コントロール消滅チェック */
		if ( SearchControl( work, shadow->name ) == NULL ){
			printf("shdwctrl.c: lost control error !!(%d)\n", shadow->name );
			DeleteShadow( work, shadow->name );
			continue ;
		}

		//printf("name:%d %08x\n", shadow->name, shadow->objs );
		objs = shadow->objs ;
#if 0
		if ( objs->flag & DG_FLAG_SHADOWMAKE ){
			shadow->spot.flag &= ~DG_FLAG_INVISIBLE ;
		} else {
			shadow->spot.flag |= DG_FLAG_INVISIBLE ;
		}
#endif

		if ( work->type == 0 ){
			/*
				平行投影影
			*/
			/* ビュー変換マトリクス作成 */
			light_dir.vx = DG_LightMatrix.m[0][0] ;
			light_dir.vy = DG_LightMatrix.m[1][0] ;
			light_dir.vz = DG_LightMatrix.m[2][0] ;
			light_dir.vw = 1.0f ;
			if ( work->im_light_flag ){
				light_dir = work->im_light_vec ;
			}
			_sceVu0Normalize( &light_dir, &light_dir );
			DG_MakeCameraMatrix( &shadow->spot.world, &DG_ZeroVector, &light_dir );
			//work->lt_length = 2000.0f ;
			shadow->spot.world.m[3][0] = objs->world.m[3][0] - light_dir.vx * work->lt_length ;
			shadow->spot.world.m[3][1] = objs->world.m[3][1] - light_dir.vy * work->lt_length ;
			shadow->spot.world.m[3][2] = objs->world.m[3][2] - light_dir.vz * work->lt_length ;
			//AN_Test_Eye( shadow->spot.world.m[3] );

			/* 光源切り替えチェック＆切り替えフェードアニメーション */
			switch ( shadow->mode ){
			  case 0:
				//shadow->fade_speed = 4 ;
				break ;
			  case 1:
				/* フェードアウト */
				if ( ( shadow->fade_count -= shadow->fade_speed ) < 0 ){
					shadow->fade_count = 0 ;
					FreeShadowEntry( shadow );
					continue ;
				}
				break ;
			  case 2:
				/* フェードイン */
				if ( ( shadow->fade_count += shadow->fade_speed ) >= 32 ){
					shadow->mode = 0 ;
					shadow->fade_count = 32 ;
				}
				break ;
			}

			{/* 影濃度設定 */
				int		alpha ;
				alpha = shadow->fade_count * work->ppl_alpha / 32 ;
				//shadow->spot.color = ( shadow->spot.color & 0x00ffffff ) | ( work->ppl_alpha << 24 ) ;
				shadow->spot.color = ( shadow->spot.color & 0x00ffffff ) | ( alpha << 24 ) ;
			}

			/* プレイヤーであれば影生成パラメータを記録する */
			if ( GM_PlayerControl != NULL && shadow->name == GM_PlayerControl->name ){
				GC_ProjectionSpot_HitLights = 1 ;
				GC_ProjectionSpot_SnakeCheckList = SpotList ;
				SpotList[ 0 ].dir = light_dir ;
				SpotList[ 0 ].flag = 1 ;		/* 平行光源 */
				SpotList[ 0 ].name = 0 ;
				SpotList[ 0 ].angle = 0 ;
				SpotList[ 0 ].range = 5000 ;
			}
		} else {
			/*
				パース付き影（点光源対応）
			*/
			extern int DG_GetShadowLight( FVECTOR *lit_pos, FVECTOR *lit_dir, float *lit_power, FVECTOR *pos, int fix_flag );
			FVECTOR	pos, dir ;
			float	power, len ;
			int		alpha, lit_id ;

			/* 一番影響度の強い点光源を取得 */
			pos = DG_ZeroVector ;
			dir = DG_ZeroVector ;
			power = 0.0f ;
			if ( ( lit_id = DG_GetShadowLight( &pos, &dir, &power, (FVECTOR*)objs->world.m[3], 0 ) ) == 0 ){
				/* 点光源がなかった場合 */
				dir.vx = objs->world.m[3][0] - shadow->old_pos.vx ;
				dir.vy = objs->world.m[3][1] - shadow->old_pos.vy ;
				dir.vz = objs->world.m[3][2] - shadow->old_pos.vz ;
				power = shadow->old_power ;
				shadow->fade_speed = 2 ;
				//printf("no hit light\n");
			}
			pos.vw = 1.0f ;
			dir.vw = 1.0f ;

			/* 参照点光源変更チェック */
			if ( shadow->mode != 1 ){
				if ( shadow->old_lit_id != lit_id ){
					shadow->mode = 1 ;
					//printf("lit change\n");
				}
			}
			/* 光源切り替えチェック＆切り替えフェードアニメーション */
			switch ( shadow->mode ){
			  case 0:
				shadow->fade_speed = 4 ;
				break ;
			  case 1:
				/* フェードアウト */
				if ( ( shadow->fade_count -= shadow->fade_speed ) < 0 ){
					shadow->mode = 2 ;
					shadow->old_lit_id = lit_id ;
					shadow->old_pos = pos ;
					shadow->old_power = power ;
					shadow->fade_count = 0 ;
				} else {
					lit_id = shadow->old_lit_id ;
					pos = shadow->old_pos ;
					power = shadow->old_power ;
					dir.vx = objs->world.m[3][0] - pos.vx ;
					dir.vy = objs->world.m[3][1] - pos.vy ;
					dir.vz = objs->world.m[3][2] - pos.vz ;
				}
				break ;
			  case 2:
				/* フェードイン */
				if ( ( shadow->fade_count += shadow->fade_speed ) >= 32 ){
					shadow->mode = 0 ;
					shadow->fade_count = 32 ;
				}
				break ;
			}
			//printf("%f %f %f %d %d %d \n",
			//	   pos.vx, pos.vy, pos.vz, shadow->mode, shadow->fade_count, shadow->fade_speed );
			//printf("old %f %f %f\n", shadow->old_pos.vx, shadow->old_pos.vy, shadow->old_pos.vz );

			/* ビュー変換マトリクス作成 */
			light_dir = dir ;
			DG_MakeCameraMatrix( &shadow->spot.world, &DG_ZeroVector, &light_dir );
			*(FVECTOR*)shadow->spot.world.m[3] = pos ;

			/* 投影角度の設定 */
			pos.vx = pos.vx - objs->world.m[3][0] ;
			pos.vy = pos.vy - objs->world.m[3][1] ;
			pos.vz = pos.vz - objs->world.m[3][2] ;
#if 0
			len = bp_sqrtf( pos.vx * pos.vx + pos.vy * pos.vy + pos.vz * pos.vz );  //BP_MATH - emulate PS2 sqrtf
#else
			DG_MULA( pos.vx, pos.vx ) ; DG_MADDA( pos.vy, pos.vy ) ; len = DG_MADD( pos.vz, pos.vz );
			len = DG_SQRT( len );
#endif
			shadow->spot.angle = len / 1500.0f ;

			/* 影濃度設定 */
			//alpha = DG_FTOI( power * 64.0f );
			alpha = DG_FTOI( power * (float)( shadow->fade_count * 2 ) );
			shadow->spot.color = ( shadow->spot.color & 0x00ffffff ) | ( alpha << 24 ) ;

			//shadow->old_power = power ;
			
			/* プレイヤーであれば影生成パラメータを記録する */
			if ( GM_PlayerControl != NULL && shadow->name == GM_PlayerControl->name ){
				GC_ProjectionSpot_HitLights = 1 ;
				GC_ProjectionSpot_SnakeCheckList = SpotList ;
				SpotList[ 0 ].pos = *(FVECTOR*)shadow->spot.world.m[3] ;
				SpotList[ 0 ].dir = light_dir ;
				SpotList[ 0 ].flag = 0 ;		/* スポット光源 */
				SpotList[ 0 ].name = 0 ;
				SpotList[ 0 ].angle = shadow->spot.angle ;
				SpotList[ 0 ].range = 5000 ;
			}
		}
	}
	
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	SHADOW_WORK	*shadow ;
	int				i ;

	shadow = work->shadow ;
	for ( i = MAX_SHADOWS ; i > 0 ; shadow++, i-- ){
		if ( shadow->name != 0 ){
			FreeShadowEntry( shadow );
		}
	}
	GC_ProjectionSpot_HitLights = 0 ;
	GC_ProjectionSpot_SnakeCheckList = NULL ;
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, int name, int where )
{
	work->name = name ;
	work->map = where ;

	/* タイプ設定 */
	if ( GCL_GetOption( 't' ) != NULL ){
		work->type = GCL_GetNextInt() ;
	} else {
		work->type = 0 ;
	}

	/* 光源距離設定（平行光源時のみ有効） */
	if ( GCL_GetOption( 'l' ) != NULL ){
		work->lt_length = GCL_GetNextInt() ;
	} else {
		work->lt_length = 3000.0f ;
	}

	/* 平行光源直接指定 */
	if ( GCL_GetOption( 'p' ) != NULL ){
		work->im_light_flag = 1 ;
		work->im_light_vec.vx = GCL_GetNextInt();
		work->im_light_vec.vy = GCL_GetNextInt();
		work->im_light_vec.vz = GCL_GetNextInt();
		_sceVu0Normalize( &work->im_light_vec, &work->im_light_vec );
	}

	/* 影濃度設定（平行光源時のみ有効） */
	if ( GCL_GetOption( 'a' ) != NULL ){
		work->ppl_alpha = GCL_GetNextInt() ;
	} else {
		work->ppl_alpha = 64 ;
	}

	/* 影情報通知処理 */
	GC_ProjectionSpot_HitLights = 0 ;
	GC_ProjectionSpot_SnakeCheckList = SpotList ;

	return (0);
}


/* ---------------------------------------------------------------- */
void *NewShadowControl( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
//	work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
