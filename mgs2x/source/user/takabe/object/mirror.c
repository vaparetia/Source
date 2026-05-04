//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	mirror.c
	鏡面モデル管理キャラ

	2000/04/01 K.Takabe
	$Id: mirror.c,v 1.4 2002/11/23 12:28:37 Yoshizawa1 Exp $

*/
/*

chara 鏡面モデル管理[NewMirrorControl] $s:name \
	-pos	$v:鏡面座標 \
	-axis	$b:反点軸		// 0:X軸反点 1:Y軸反点 2:Z軸反点
mesg 鏡面モデル管理 $s:名前 add[0] $s:追加キャラ
mesg 鏡面モデル管理 $s:名前 del[1] $s:削除キャラ
// 指定した座標で指定した軸方向に反転したモデルを生成する
// メッセージにより追加、削除ができるので部屋の入口のトラップなどに
// 引っかけて使用すると良い

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


#define MAX_MIRROR_OBJECTS		(32)
#define MAX_OBJS_CONNECTION		(16)

#define MIRROR_DG_FLAGS (DG_FLAG_INVISIBLE1|DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_DELAYED)

/* ---------------------------------------------------------------- */
extern	DG_OBJS	*PL_GolUniformObjs ; /* からだ */
extern	DG_OBJS	*PL_GolCapObjs ;     /* キャップ。NULLのときはキャップなし */
extern	DG_OBJS *PL_ShadowHairObjs ; /* 映りこみ専用カツラ */
/* ---------------------------------------------------------------- */
typedef struct {
	DG_DEF	*def ;				/* 複製元モデル */
	DG_OBJS	*objs ;				/* 複製元オブジェクト */
} OBJS_INFO ;
typedef struct {
	int			name ;			/* オリジナルオブジェクトの名前 */
	DG_OBJS		*org_objs ;		/* オリジナルオブジェクトのモデル */
	DG_OBJS		*objs ;			/* 生成鏡面用モデル */
	short		n_child ;		/* 付随子オブジェクト数 */
	short		flag ;
	FMATRIX		light[2] ;		/* 反転ライト用 */
	OBJS_INFO	objs_list[MAX_OBJS_CONNECTION] ;	/* 接続オブジェクト記録用 */
} MIRROR_OBJECT ;

typedef	struct	{
	GV_ACT		actor ;
	int			name ;			/* キャラ名 */
	int			map ;			/* キャラ存在マップ */
	int			axis ;			/* 反転軸フラグ */
	FVECTOR		mirror_point ;	/* 鏡面位置 */
	MIRROR_OBJECT	mirror_objs[MAX_MIRROR_OBJECTS] ;

	/* プレイヤー敵兵装関連 */
	int			exec_player_flag ;	/* プレイヤーが鏡面に映っているかどうか */
	DG_OBJS		*gol_body_objs ;
	DG_OBJS		*gol_cap_objs ;
} Work ;


/* ---------------------------------------------------------------- */
/* 空きエントリの取得 */
static MIRROR_OBJECT* GetMirrorObjectEntry( Work *work, int name )
{
	MIRROR_OBJECT	*mirror_obj ;
	int				i ;

	mirror_obj = work->mirror_objs ;
	for ( i = MAX_MIRROR_OBJECTS ; i > 0 ; mirror_obj++, i-- ){
		if ( mirror_obj->name == 0 ){
			mirror_obj->name = name ;
			return ( mirror_obj );
		}
	}
	printf("mirror.c: not enough mirror object entry !!\n");
	return ( NULL );
}

/* ミラーオブジェクトエントリの開放 */
static void FreeMirrorObjectEntry( MIRROR_OBJECT *m_obj )
{
	DG_OBJS			*objs, *child_objs ;

	m_obj->name = 0 ;
	/* 関連付けされているものも含め全て開放する */
	objs = m_obj->objs ;
	while ( objs != NULL ){
		child_objs = objs->next_dgobjs ;
		DG_DequeueObjs( objs );
		DG_FreeObjs( objs );
		objs = child_objs ;
	}
}

/* 指定した名前のミラーオブジェクトエントリ検索 */
static MIRROR_OBJECT* SearchMirrorObjectEntry( Work *work, int name )
{
	MIRROR_OBJECT	*mirror_obj ;
	int				i ;

	mirror_obj = work->mirror_objs ;
	for ( i = MAX_MIRROR_OBJECTS ; i > 0 ; mirror_obj++, i-- ){
		if ( mirror_obj->name == name ){
			return ( mirror_obj );
		}
	}
	//printf("mirror.c: search faild mirror object entry !!\n");
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
	printf("mirror.c: search faild control !!\n");
	return ( NULL );
}

/* コピーオブジェクトを生成する */
static DG_OBJS *CopyObjs( DG_OBJS *org_objs, FMATRIX *light )
{
	DG_OBJS	*objs ;
	int		objs_flag ;

	objs_flag = MIRROR_DG_FLAGS ;
	if ( org_objs->flag & DG_FLAG_ONEPIECE ) objs_flag |= DG_FLAG_ONEPIECE ;
	objs = DG_MakeObjs( org_objs->def, objs_flag, 0 );
#ifdef DEBUG_MODE
	objs->fname = org_objs->fname ;
#endif
	if ( objs == NULL ) return ( NULL );
	DG_QueueObjs( objs );
	DG_SetLightMatrix( objs, light );
	return ( objs );
}

/* 指定した名前のキャラクタをミラーオブジェクトに追加 */
static void AddMirrorObject( Work *work, int name )
{
	CONTROL	*control ;
	OBJECT	*object ;
	MIRROR_OBJECT	*mirror_obj ;
	OBJS_INFO	*objs_info ;

	/* ２重登録防止 */
	if ( ( mirror_obj = SearchMirrorObjectEntry( work, name ) ) != NULL ) return ;

	/* コントロールリストから指定した名前のキャラクタを検索 */
	if ( ( control = SearchControl( work, name ) ) == NULL ) return ;
	if ( ( object = control->object ) == NULL ){
		return ;
	}

	/* ミラーオブジェクトワークの確保 */
	if ( ( mirror_obj = GetMirrorObjectEntry( work, name ) ) == NULL ) return ;
	mirror_obj->org_objs = object->objs ;
	mirror_obj->flag = 0 ;
	/* 非表示でも描画を行うタイプのオブジェクトかどうかをチェック */
	if ( ( GM_PlayerControl != NULL ) && ( name == GM_PlayerControl->name ) ){
		mirror_obj->flag = 1 ;
		work->exec_player_flag = 1 ;
	} else if ( object->evmobj != NULL ){
		mirror_obj->flag = 1 ;
	}

	{/* 関連付けされているオブジェクトも含め影用に複製する */
		DG_OBJS	*org_parent_objs, *org_objs ;
		DG_OBJS	*parent_objs, *objs ;
		int		objs_flag ;

		org_parent_objs = mirror_obj->org_objs ;
		objs_flag = MIRROR_DG_FLAGS ;
		if ( org_parent_objs->flag & DG_FLAG_ONEPIECE ) objs_flag |= DG_FLAG_ONEPIECE ;
		mirror_obj->objs = parent_objs = DG_MakeObjs( org_parent_objs->def, objs_flag, 0 );
		if ( parent_objs == NULL ){
			FreeMirrorObjectEntry( mirror_obj );
			printf("mirror.c: object init error!!\n");
			return ;
		}
		DG_QueueObjs( parent_objs );
		DG_SetLightMatrix( parent_objs, mirror_obj->light );
		/* 関連されているオブジェクトの作成 */
		objs_info = mirror_obj->objs_list ;
		mirror_obj->n_child = 0 ;
		while ( org_parent_objs->next_dgobjs != NULL ){
			if ( mirror_obj->n_child >= MAX_OBJS_CONNECTION ) break ;
			org_objs = org_parent_objs->next_dgobjs ;
			if ( ( objs = CopyObjs( org_objs, mirror_obj->light ) ) == NULL ) break ;
			DG_ConnectObjs( parent_objs, objs );
			/* 子オブジェクトの情報を記録する */
			mirror_obj->n_child++ ;
			objs_info->def = objs->def ;
			objs_info->objs = org_objs ;
			objs_info++ ;
			/* 次のオブジェクトへ */
			org_parent_objs = org_objs ;
		}
	}
	printf("mirror.c: add mirror object (%d,%d)\n", name, mirror_obj->flag );
}

/* 指定した名前のキャラクタをミラーオブジェクトから削除 */
static void DeleteMirrorObject( Work *work, int name )
{
	MIRROR_OBJECT	*mirror_obj ;

	/* 指定した名前のミラーオブジェクトを検索 */
	if ( ( mirror_obj = SearchMirrorObjectEntry( work, name ) ) == NULL ){
		printf("mirror.c: delete faild (%d)\n", name );
		return ;
	}
	if ( ( GM_PlayerControl != NULL ) && ( name == GM_PlayerControl->name ) ){
		work->exec_player_flag = 0 ;
	}

	printf("mirror.c: delete mirror object (%d)\n", name );
	FreeMirrorObjectEntry( mirror_obj );
}

/* 子オブジェクトの状態をチェックし、必要であれば開放・再確保等を行なう */
static void CheckMirrorObject( MIRROR_OBJECT *m_obj )
{
	OBJS_INFO		*objs_list ;
	DG_OBJS			*org_objs, *objs, *current_objs ;
	int				j, k ;

	org_objs = m_obj->org_objs->next_dgobjs ;
	objs = m_obj->objs->next_dgobjs ;
	objs_list = m_obj->objs_list ;
	/* 開放チェック */
	for ( j = 0 ; j < m_obj->n_child ; j++ ){
		if ( objs_list->objs != org_objs || objs_list->def != org_objs->def ){
			current_objs = objs ;
			objs = objs->next_dgobjs ;
			DG_DisconnectObjs( m_obj->objs, current_objs );
			DG_DequeueObjs( current_objs );
			DG_FreeObjs( current_objs );
			m_obj->n_child-- ;
			for ( k = j ; k < m_obj->n_child ; k++ ){
				m_obj->objs_list[k] = m_obj->objs_list[k+1] ;
			}
			j-- ;
			//printf("mirror.c: aa %08x %08x %s\n", org_objs, objs_list->objs, objs_list->objs->fname );
			//printf("mirror.c: aa %08x %08x\n", org_objs, objs_list->objs );
#ifdef DEBUG_MODE
			printf("mirror.c: connect out %08x %08x %s\n", org_objs, objs_list->objs, current_objs->fname );
#endif
		} else {
			if ( org_objs->root != NULL ) objs->world = *(org_objs->root) ;/* 接続型オブジェクト対応 */
			objs_list++ ;
			objs = objs->next_dgobjs ;
			org_objs = org_objs->next_dgobjs ;
		}
	}
	/* 生成チェック */
	while ( org_objs != NULL ){
#ifdef DEBUG_MODE
		if ( DG_SearchQueueObjs( org_objs ) == NULL ){
			printf("%08x\n", org_objs );
			ASSERT(0);
			break ;
		}
#endif
		if ( m_obj->n_child >= MAX_OBJS_CONNECTION ) break ;
		if ( ( objs = CopyObjs( org_objs, m_obj->light ) ) == NULL ) break ;
#ifdef DEBUG_MODE
		printf("mirror.c: new connect %08x %s\n", org_objs, objs->fname);
#endif
		DG_ConnectObjs( m_obj->objs, objs );
		if ( org_objs->root != NULL ) objs->world = *(org_objs->root) ;/* 接続型オブジェクト対応 */
		/* 子オブジェクトの情報を記録する */
		m_obj->n_child++ ;
		objs_list->def = objs->def ;
		objs_list->objs = org_objs ;
		objs_list++ ;
		/* 次のオブジェクトへ */
		org_objs = org_objs->next_dgobjs ;
	}
	
}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	GV_MSG		*msg ;
	int			i, j, n_msg ;
	MIRROR_OBJECT	*m_obj ;
	FMATRIX		rev_mat, rev_mat_light ;
	DG_OBJS		*org_objs, *objs, *org_child_objs, *child_objs ;
	DG_OBJ		*org_obj, *obj ;
	int			mirror_invisible, player_flag ;

	/* メッセージチェック */
	if( ( n_msg = GV_ReceiveMessage( work->name, &msg ) ) > 0 ){
		for ( ; n_msg > 0 ; n_msg--, msg++ ){
			switch ( msg->message[0] ){
			  case 0:/* ミラーオブジェクトに追加 */
				AddMirrorObject( work, msg->message[1] );
				break ;
			  case 1:/* ミラーオブジェクトから削除 */
				DeleteMirrorObject( work, msg->message[1] );
				break ;
			}
		}
	}

	/* ミラー反転用マトリクスの作成 */
	_sceVu0UnitMatrix( &rev_mat );
	_sceVu0UnitMatrix( &rev_mat_light );
	switch ( work->axis ){
	  case 0:/* Ｘ軸反転 */
		rev_mat_light.m[0][0] = -1.0f ;
		rev_mat.m[0][0] = -1.0f ;
		rev_mat.m[3][0] = work->mirror_point.vx * 2 ;
		break ;
	  case 1:/* Ｙ軸反転 */
		rev_mat_light.m[1][1] = -1.0f ;
		rev_mat.m[1][1] = -1.0f ;
		rev_mat.m[3][1] = work->mirror_point.vy * 2 ;
		break ;
	  case 2:/* Ｚ軸反転 */
		rev_mat_light.m[2][2] = -1.0f ;
		rev_mat.m[2][2] = -1.0f ;
		rev_mat.m[3][2] = work->mirror_point.vz * 2 ;
		break ;
	}

	/* プレイヤー敵兵装関連オブジェクト確保チェック */
	if ( work->gol_body_objs == NULL ){
		if ( PL_GolUniformObjs != NULL && work->exec_player_flag ){
			work->gol_body_objs = DG_MakeObjs( PL_GolUniformObjs->def, MIRROR_DG_FLAGS, 0 );
			if ( work->gol_body_objs ) DG_QueueObjs( work->gol_body_objs );
		}
	} else {
		if ( PL_GolUniformObjs == NULL || work->exec_player_flag == 0 ){
			DG_DequeueObjs( work->gol_body_objs );
			DG_FreeObjs( work->gol_body_objs );
			work->gol_body_objs = NULL ;
		}
	}
	if ( work->gol_cap_objs == NULL ){
		if ( PL_GolCapObjs != NULL && work->exec_player_flag ){
			work->gol_cap_objs = DG_MakeObjs( PL_GolCapObjs->def, MIRROR_DG_FLAGS, 0 );
			if ( work->gol_cap_objs ) DG_QueueObjs( work->gol_cap_objs );
		}
	} else {
		if ( PL_GolCapObjs == NULL || work->exec_player_flag == 0 ){
			DG_DequeueObjs( work->gol_cap_objs );
			DG_FreeObjs( work->gol_cap_objs );
			work->gol_cap_objs = NULL ;
		}
	}

	/* ミラーオブジェクトのマトリクス操作 */
	m_obj = work->mirror_objs ;
	for ( i = MAX_MIRROR_OBJECTS ; i > 0 ; m_obj++, i-- ){
		CONTROL	*ctrl ;
		if ( m_obj->name == 0 ) continue ;

		/* コントロール消滅チェック */
		if ( ( ctrl = SearchControl( work, m_obj->name ) ) == NULL ){
			printf("mirror.c: lost control error !!(%d)\n", m_obj->name );
			DeleteMirrorObject( work, m_obj->name );
			continue ;
		} else if ( ctrl->object->objs != m_obj->org_objs ){
			/* 同じ名前のコントロールが存在するが、オブジェクトが作り直されている場合 */
			printf("mirror.c: lost object error !!(%d)\n", m_obj->name );
			DeleteMirrorObject( work, m_obj->name );
			//ASSERT(0)
			continue ;
		}
		/* オブジェクトが勝手に破棄されていないかチェック */
		CheckMirrorObject( m_obj );

		//printf("name:%d %08x, %08x\n", m_obj->name, m_obj->org_objs, m_obj->objs );
		org_objs = m_obj->org_objs ;
		objs = m_obj->objs ;

		/* ライトマトリクスの反転処理 */
		//_sceVu0MulMatrix( objs->light, &rev_mat_light, org_objs->light );
		_sceVu0MulMatrix( objs->light, org_objs->light, &rev_mat_light );
		objs->light[1] = org_objs->light[1] ;

		/* プレイヤーオブジェクトの自動非表示チェック処理 */
		if ( GM_PlayerControl != NULL && m_obj->name == GM_PlayerControl->name ){
			/* プレイヤーの自動非表示条件をチェック */
			if ( GM_PlayerStatus & (PLAYER_BEYOND) ){
				/* エルードモード時に非表示にする */
				mirror_invisible = 1 ;
			} else {
				mirror_invisible = 0 ;
			}
			/* ステルス迷彩チェック */
			if ( GM_Item == IT_Stealth ) mirror_invisible = 1 ;
			player_flag = 1 ;
		} else {
			mirror_invisible = 0 ;
			player_flag = 0 ;
		}

		while ( objs != NULL ){
			org_child_objs = org_objs->next_dgobjs ;
			child_objs = objs->next_dgobjs ;

#if 0
			/* 表示非表示のチェック *//* プレイヤーの場合のみobjsのフラグを見ないようにする */
			if ( GM_PlayerControl == NULL || m_obj->name != GM_PlayerControl->name ){
				objs->flag &= ~DG_FLAG_INVISIBLE0 ;
				objs->flag |= org_objs->flag & DG_FLAG_INVISIBLE0 ;
			}
#else
			/* 表示非表示のチェック *//* フラグの立っているオブジェクトはobjsのフラグを見ないようにする */
			if ( m_obj->flag == 0 ){
				objs->flag &= ~DG_FLAG_INVISIBLE0 ;
				objs->flag |= org_objs->flag & DG_FLAG_INVISIBLE0 ;
			}
#endif
			/* 強制非表示フラグのチェック */
			if ( mirror_invisible ) objs->flag |= DG_FLAG_INVISIBLE0 ;
			/* プレイヤー映りこみ専用カツラ特別対処 */
			if ( player_flag ){
				if ( work->gol_cap_objs != NULL && org_objs == PL_ShadowHairObjs ){
					objs->flag |= DG_FLAG_INVISIBLE0 ;
				} else {
					if ( mirror_invisible == 0 ) objs->flag &= ~DG_FLAG_INVISIBLE0 ;
				}
			}
			/* 表示グループのコピー */
			objs->group_id = org_objs->group_id ;
			/* 各マトリクスの反転処理 */
			if ( org_objs->root == NULL ){
				_sceVu0MulMatrix( &objs->world, &rev_mat, &org_objs->world );
			} else {
				_sceVu0MulMatrix( &objs->world, &rev_mat, org_objs->root );
			}
			org_obj = org_objs->objs ;
			obj = objs->objs ;
			for ( j = objs->n_models ; j > 0 ; obj++, org_obj++, j-- ){
				_sceVu0MulMatrix( &obj->world, &rev_mat, &org_obj->world );
				{/* 関節ごとの表示・非表示フラグ複製処理（ダンボール壊れ対策） */
					int		flag ;
					flag = org_obj->flag & ( DG_FLAG_INVISIBLE0 ) ;
					obj->flag &= ~DG_FLAG_INVISIBLE0 ;
					obj->flag |= flag ;
				}
			}
			org_objs = org_child_objs ;
			objs = child_objs ;
		}
		/* 親オブジェクトの非表示フラグは無視する */
		if ( mirror_invisible == 0 ){
			m_obj->objs->flag &= ~DG_FLAG_INVISIBLE0 ;
		}

		/* プレイヤーの敵兵装チェック */
		if ( player_flag ){
			if ( work->gol_body_objs != NULL ){
				int		j ;
				work->gol_body_objs->world = m_obj->objs->world ;
				work->gol_body_objs->light = m_obj->objs->light ;
				for ( j = 0 ; j < 21 ; j++ ){
					work->gol_body_objs->objs[ j ].world = m_obj->objs->objs[ j ].world ;
					work->gol_body_objs->objs[ j ].light = m_obj->objs->objs[ j ].light ;
					work->gol_body_objs->objs[ j ].flag = m_obj->objs->objs[ j ].flag ;
					m_obj->objs->objs[ j ].flag |= DG_FLAG_INVISIBLE0 ;
				}
				if ( work->gol_cap_objs != NULL ){
					work->gol_cap_objs->world = m_obj->objs->world ;
					work->gol_cap_objs->light = m_obj->objs->light ;
					work->gol_cap_objs->objs[ 0 ].world = m_obj->objs->objs[ 2 ].world ;
					work->gol_cap_objs->objs[ 0 ].light = m_obj->objs->objs[ 2 ].light ;
					work->gol_cap_objs->objs[ 1 ].world = m_obj->objs->objs[ 11 ].world ;
					work->gol_cap_objs->objs[ 1 ].light = m_obj->objs->objs[ 11 ].light ;
					work->gol_cap_objs->objs[ 2 ].world = m_obj->objs->objs[ 12 ].world ;
					work->gol_cap_objs->objs[ 2 ].light = m_obj->objs->objs[ 12 ].light ;
					work->gol_cap_objs->objs[ 0 ].flag = PL_GolCapObjs->objs[ 0 ].flag ;
					work->gol_cap_objs->objs[ 1 ].flag = PL_GolCapObjs->objs[ 1 ].flag ;
					work->gol_cap_objs->objs[ 2 ].flag = PL_GolCapObjs->objs[ 2 ].flag ;

					work->gol_cap_objs->objs[ 0 ].flag |= DG_FLAG_INVISIBLE0 ;
					work->gol_body_objs->objs[ 11 ].flag |= DG_FLAG_INVISIBLE0 ;
					work->gol_body_objs->objs[ 12 ].flag |= DG_FLAG_INVISIBLE0 ;
				}
			}
		}

	}
	
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	MIRROR_OBJECT	*mirror_obj ;
	int				i ;

	mirror_obj = work->mirror_objs ;
	for ( i = MAX_MIRROR_OBJECTS ; i > 0 ; mirror_obj++, i-- ){
		if ( mirror_obj->name != 0 ){
			FreeMirrorObjectEntry( mirror_obj );
		}
	}
	if ( work->gol_body_objs != NULL ){
		DG_DequeueObjs( work->gol_body_objs );
		DG_FreeObjs( work->gol_body_objs );
	}
	if ( work->gol_cap_objs != NULL ){
		DG_DequeueObjs( work->gol_cap_objs );
		DG_FreeObjs( work->gol_cap_objs );
	}
}

/* ---------------------------------------------------------------- */
#ifdef KP_WINDOWS
static void DmyAct( Work *work ){}
static void DmyDie( Work *work ){}
#endif

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, int name, int where )
{
	work->name = name ;
	work->map = where ;

	/* 設置座標 */
    if ( GCL_GetOption( 'p' ) != NULL ) {
		work->mirror_point.vx = (float)GCL_GetNextInt() ;
		work->mirror_point.vy = (float)GCL_GetNextInt() ;
		work->mirror_point.vz = (float)GCL_GetNextInt() ;
		work->mirror_point.vw = 1.0f ;
	}

	if ( GCL_GetOption( 'a' ) != NULL ) {
		work->axis = GCL_GetNextInt() ;
	}

	return (0);
}


/* ---------------------------------------------------------------- */
void *NewMirrorControl( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
#ifndef KP_WINDOWS
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
#else
		if ( DG_CheckUseMirror() ) {
			GV_SetActor( &( work->actor ), Act, Die ) ;
			if ( GetResources( work, name, where ) < 0 ) {
				GV_DestroyActor( work ) ;
				return NULL ;
			}
		} else {
			GV_SetActor( &( work->actor ), DmyAct, DmyDie ) ;
		}
#endif
	}
	return (void *)work ;
}
