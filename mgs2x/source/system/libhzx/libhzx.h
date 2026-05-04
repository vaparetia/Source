/*
   libhzx.h
   ハザード処理関連定義ファイル

   1999/07/07 M.Sonoyama
   2000/01/24 K.Uehara		libhzd->libhzx
   $Id: libhzx.h,v 1.4 2002/11/23 11:48:44 Yoshizawa1 Exp $			
*/
   
#ifndef	__LIBHZX__H__
#define	__LIBHZX__H__

#include	"fmt_hzx.h"
#include	"libhzx.cnf"

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------*/

#define	HZX_MAX_GROUPS	(31)	

#define	HZX_NO_ZONE	(255)

#define	HZX_D_SEGMENTS	(32)
#define	HZX_D_FLOORS	(32)
#define	HZX_D_SEGMENTS_START	(10000)
#define	HZX_D_FLOORS_START	(10000)

enum {
    HZX_REACH,
    HZX_DIRECT_REACH,
    HZX_INDIRECT_REACH,
    HZX_UNREACH
} ;

/*----------------------------------------------------------------*/

/* 処理するHZXのグループを示すビットフラグ */
typedef unsigned int HZX_GROUP_ID;

/* マップ情報付きゾーンアドレス */
typedef unsigned int HZX_ZONE_ADD ;
/*----------------------------------------------------------------*/

/*
   トラップ関連
*/

//#ifdef DEBUG_MODE
#define	HZX_DTRP
//#endif

#define	HZX_MAX_TRAPS			(12)
#define	HZX_MAX_INSIDE_BINDS	(24)
#define	HZX_MAX_BINDS			(512)

/* 発生イベント名 */
#define	EV_ENTER	(6753643)	/* GV_StrCode( "入る" )	*/
#define	EV_LEAVE	(6411627)	/* GV_StrCode( "出る" )	*/
#define	EV_BOTH		(5321)		/* GV_StrCode( "？" )	*/
#define	EV_INSIDE	(5547371)	/* GV_StrCode( "いる" )	*/
#define	EV_BOTH2	(5398)		/* GV_StrCode( "＊" )	*/

/* ターゲットキャラクタ */
#define	EV_CHARA_EVERY	(5321)		/* GV_StrCode( "？" )	*/

/* 半角文字 */
#define	EV_CHARA_EVERY_HALF	(63)
#define	EV_BOTH_HALF		(63)
#define	EV_BOTH2_HALF		(42)

/* プレイヤー通常トラップチェック用変換値 */
#define	HZX_PLAYER_NORMAL	I64(0x80000000)

/* トラップフラグ */
enum {
    HZX_BND_FLAG_OUT    = 0x0000,	/* 条件不成立 */
    HZX_BND_FLAG_IN 	= 0x0001,	/* 条件成立 */
    HZX_BND_FLAG_INSIDE = 0x0002,	/* 条件成立中 */
} ;

/* トラップトリガータイプ */
enum {
    HZX_BND_CHECK_NORMAL = 0x0000,
    HZX_BND_CHECK_DIR    = 0x0001,
    HZX_BND_CHECK_STATE  = 0x0002,
    HZX_BND_CHECK_BUTTON = 0x0004,
    HZX_BND_CHECK_ITEM   = 0x0008,
    HZX_BND_CHECK_WEAPON = 0x0010,
	HZX_BND_CHECK_ALERT  = 0x0020,
	HZX_BND_CHECK_KEY	 = 0x0040,
    HZX_BND_CHECK_AND    = 0x1000,
} ;

#define HZX_BND_TRIGGER_MASK	0x007f

/* トラップタイプ */
enum {
    HZX_BND_TYPE_NORMAL = 		0x0000,
    HZX_BND_TYPE_CAMERA = 		0x0001,
    HZX_BND_TYPE_PROC	= 		0x0002,
	HZX_BND_TYPE_MISSILE   	= 	0x0004,
	HZX_BND_TYPE_MAPCONNECT =	0x0008,
} ;

/* トラップアトリビュート */
/* HZX_TRP->b1.vw に入っている */
enum {
	HZX_TRP_ATR_HAS_NAME = 0x0001,
	HZX_TRP_ATR_NO_CHECK = 0x0002,
	HZX_TRP_ATR_DARK =		0x00f0,
	HZX_TRP_ATR_CHECK_SCN = 0x1000,
	HZX_TRP_ATR_NO_CHECK_SCN = 0x2000,
} ;

/* トラップイベント構造体 */
typedef	ALIGN16_DECL(struct)	{
    u_int		name ;		/* トラップ名 */
    u_int		chara ;		/* 判定キャラ */
    u_int		event ;		/* 入る／出る／いる */
    HZX_GROUP_ID	id;		/* グループID */
    
    u_short	    type ;		/* トラップタイプ */
    u_short		check ;		/* チェックタイプ */
    u_short		time ;		/* 実行待ち時間 */
    u_short		flag ;		/* 各種フラグ */
    char		*command ;	/* 実行ブロック */

    short		dir[ 4 ] ;	/* 方向 */
    short		dir_w[ 4 ] ;	/* 方向幅 */
    u_short		which ;		/* ボタンを見るパッド番号 */
	u_short		alert ;		/* 危険モード */

    signed char		button[ 4 ] ;	/* ボタン */
    signed char		state[ 4 ] ;	/* プレイヤー状態 */
    signed char		item[ 4 ] ;	/* アイテム */
    signed char		weapon[ 4 ] ;	/* 武器 */
} HZX_BND  ;

/*----------------------------------------------------------------*/

typedef ALIGN16_DECL(struct) 	{
    short	vx, vy ;
} DVECTOR  ;

typedef ALIGN16_DECL(struct) 	{
    float	vx, vy ;
} FDVECTOR  ;

typedef	ALIGN16_DECL(struct)	{
    float	x, z, y, h ;
} HZX_FVEC  ;

/*----------------------------------------------------------------*/

/* トラップ進入状態チェックフラグ */
enum {
    HZX_EVT_FLAG_NO_CAMERA = 0x0001,
    HZX_EVT_FLAG_CAMERA_ONLY = 0x0002,
	HZX_EVT_FLAG_MISSILE	=	0x0004,
	HZX_EVT_FLAG_MAPCONNECT =	0x0008,
	HZX_EVT_FLAG_NO_EXEC_EVENT = 0x1000,
	HZX_EVT_FLAG_CHECKALL =		0x2000,
} ;

/* トラップ進入状態構造体 */
typedef	ALIGN16_DECL(struct)	{
    FVECTOR		*mov ;	/* イベント発生位置 */
    SVECTOR		*rot ;	/* イベント発生キャラ向き */
    u_int		chara ;	
    u_int		flag ;

    u_int		object ;
    u_short		n_inside ; /* 進入トラップ数 */
    u_short		n_inside2 ; /* 条件成立バインド数 */
    u_int		inside[ HZX_MAX_TRAPS ] ; /* 進入トラップ */
    HZX_BND		*inside2[ HZX_MAX_INSIDE_BINDS ] ; /* 条件成立バインド */
} HZX_EVT  ;

/*----------------------------------------------------------------*/

/* マップリンク壁 */
typedef	ALIGN16_DECL(struct)	{
	HZX_GROUP_ID		hzx_id1, hzx_id2 ;
	int					z1, z2 ;
	HZX_HZD				segment ;
} HZX_MAPLINK  ;

/*----------------------------------------------------------------*/

/* 当たりハンドラー */
typedef	ALIGN16_DECL(struct)	{
    HZX_DEF		*def ;		/* ファイルへのポインタ */

    HZX_GRP		*grp;		/* 現在のグループ */
    HZX_BLOCK	*block ;   	/* 現在のグループのブロック */

    u_char		*route ;
    u_int		id ;		
	int			*tag ;
    short		map;		/* 現在ＯＮになっているグループ */
    short		reserved[ 3 ] ;
} HZX_HDL  ;

/*---------------------------------------------------------------------*/
#ifdef PSX2
//BP_PS2 extern	sceDmaChan		*HZX_Vif0Dma ;
#endif
extern	HZX_HDL			*HZX_CurrentHzx ;		// カレントのHZXファイル
extern	HZX_GROUP_ID	HZX_CurrentGroupID ;
extern	HZX_GROUP_ID	HZX_AllMapID ;

/*----------------------------------------------------------------*/
/*
	inline 関数
*/

#define HZX_DEBUG_MODE	1

EXTERN_INLINE int HZX_GetGroupNo( HZX_GROUP_ID id )
// IDを一つのみ取得。２つ以上ある場合は,ASSERTをおこす。
{
	int no;
	no = GV_GetNo( id );
#ifdef HZX_DEBUG_MODE
	if( GV_GetBit( no ) != (int)id ){
//BP		printf( "GROUP ERR %X\n", id );
		HANGUP();
	}
#endif
	return no;
}

/* データ取得関連 */

/* カレントのＨＺＤを取得 */
EXTERN_INLINE HZX_HDL *HZX_GetCurrentHzx( void )
{
    return HZX_CurrentHzx ;
}

/* 被検索グループリセット */
EXTERN_INLINE void	HZX_ResetCurrentGroup( void )
{
    HZX_CurrentGroupID = 0 ;
}

/* 被検索グループに追加 */
EXTERN_INLINE void	HZX_AddCurrentGroup( int group )
{
    HZX_CurrentGroupID |= group;
}

/* 被検索グループから削除 */
EXTERN_INLINE void	HZX_DeleteCurrentGroup( int group )
{
    HZX_CurrentGroupID &= ~group;
}

EXTERN_INLINE void HZX_SetCurrentGroup( int group )
{
	HZX_CurrentGroupID = group;
}

/* ＳＥ番号を返す */
EXTERN_INLINE	int	HZX_GetSeCode( int atr )
{
    return ( ( atr & 0xf0000000 ) >> 28 ) ;
}

/* ＳＥ番号をセットする */
EXTERN_INLINE	int		HZX_SeCode( int seNo )
{
	return ( ( seNo & 0xf ) << 28 ) ;
}

/* ナビゲート関連 */

EXTERN_INLINE	int	HZX_Address( int id, int zone1, int zone2 )
{
	int		mapno ;

	mapno = HZX_GetGroupNo( id );
	if ( zone2 < zone1 ) {
		int tmp;
		tmp = zone1 ;
		zone1 = zone2 ;
		zone2 = tmp ;
	}
	return ( 255 & zone1 ) | ( ( 255 & zone2 ) << 8 ) | ( mapno << 16 ) ;
}

EXTERN_INLINE int HZX_AddressNo( int no, int zone1, int zone2 )
{
	if ( zone2 < zone1 ) {
		int tmp;
		tmp = zone1 ;
		zone1 = zone2 ;
		zone2 = tmp ;
	}
	return ( 255 & zone1 ) | ( ( 255 & zone2 ) << 8 ) | ( no << 16 ) ;
}

EXTERN_INLINE	int	HZX_ZoneMapNo( int address )
{
	return ( 255 & ( address >> 16 ) );
}

EXTERN_INLINE	int	HZX_ZoneGroupID( int address )
{
	return GV_GetBit( 255 & ( address >> 16 ) );
}

EXTERN_INLINE	int	HZX_Zone1( int address )
{
	return 255 & address ;
}

EXTERN_INLINE	int	HZX_Zone2( int address )
{
	return 255 & ( address >> 8 ) ;
}

EXTERN_INLINE HZX_GRP	*HZX_GetGroup( HZX_GROUP_ID id )
{
	int		group ;

    group = HZX_GetGroupNo( id );
    return HZX_CurrentHzx->grp + group;
}

EXTERN_INLINE	HZX_ZON	*HZX_GetZone( HZX_GROUP_ID id, int zone )
{
    return ( HZX_GetGroup( id ) )->zones + zone ;
}

EXTERN_INLINE HZX_GRP *HZX_GetGroupFromNo( int no )
{
	return HZX_CurrentHzx->grp + no;
}

EXTERN_INLINE	HZX_ZON	*HZX_GetZoneNo( int no, int zone )
{
    return ( HZX_GetGroupFromNo( no ) )->zones + zone ;
}

EXTERN_INLINE	HZX_ZON	*HZX_GetZoneFromAdd( HZX_ZONE_ADD zoneadd )
{
	int no ;
	int zone ;

	no = HZX_ZoneMapNo( zoneadd ) ;
	zone = HZX_Zone1( zoneadd ) ;
    return HZX_GetZoneNo( no, zone ) ;
}

EXTERN_INLINE int	HZX_GetLinkGroupNo( HZX_ZON	*link_zone )
{
	return link_zone->near_flag[ 5 ] ;
}

EXTERN_INLINE int	HZX_GetLinkZone( 
		HZX_ZON	*link_zone )
{
	return link_zone->nears[ 5 ] ;
}

/* ゾーンフラグセット */
EXTERN_INLINE void	HZX_SetZoneFlag( HZX_ZONE_ADD zoneadd, int flag )
{
	HZX_ZON	*z ;

	z = HZX_GetZoneFromAdd( zoneadd ) ;
	z->flag |= flag ;
}
EXTERN_INLINE void	HZX_UnsetZoneFlag( HZX_ZONE_ADD zoneadd, int flag )
{
	HZX_ZON	*z ;

	z = HZX_GetZoneFromAdd( zoneadd ) ;
	z->flag &= ~flag ;
}

/* 法線を返す */
EXTERN_INLINE void	HZX_GetNormal( FVECTOR *norm, HZX_HZD *hzd )
{
	if ( hzd->type == HZX_TYPE_SEGMENT ) {
		norm->vx = hzd->p2.z - hzd->p1.z ;
		norm->vy = 0.0F ;
		norm->vz = hzd->p1.x - hzd->p2.x ;
	} else {
		norm->vx = hzd->p1.h ;
		norm->vy = hzd->p3.h ;
		norm->vz = hzd->p2.h ;
	}
}

/*----------------------------------------------------------------*/

#define HZX_HEIGHT_NO_USE	(-32767.0F) 

/* 跳弾専用ハザード用番号シフト */
#define		HZX_RECOIL_TYPE_SHIFT	(1024)

/* SEGMENT アトリビュート */

#define	HZX_SEG_ALL	      	  0x0000	/* 全ての当たりをチェックする */
#define HZX_SEG_NO_PLAYER     0x0010	/* プレイヤーの当たりチェックをしない */
#define HZX_SEG_NO_ENEMY      0x0020	/* 敵兵の当たりチェックをしない */
#define HZX_SEG_NO_BULLET     0x0040	/* ショットの当たりをチェックしない */
#define HZX_SEG_NO_MISSILE    0x0080	/* ミサイル系の当たり判定をしない */
#define HZX_SEG_NO_C4         0x0100	/* C4爆弾がセットできない */
#define HZX_SEG_NO_RECOIL     0x0200	/* ショットが跳弾しない */
#define HZX_SEG_NO_HARITSUKI  0x0400	/* 張り付かない */
#define HZX_SEG_NO_DISP_RADAR 0x0800	/* レーダーに写らない */
#define HZX_SEG_NO_BULLETHOLE 0x1000	/* 弾痕つかない */
#define HZX_SEG_NO_SPRAY      0x2000	/* スプレーつかない */
#define	HZX_SEG_NO_ENEMY_IK	  0x4000	/* 敵ＩＫチェックしない */
#define	HZX_SEG_NO_ENEMY_EYES 0x00010000	/* 敵兵視線通り抜ける */
#define	HZX_SEG_NO_KNOCK_SE	  0x00020000	/* ノック音しない */
#define	HZX_SEG_CYPHER 		  0x00040000	/* サイファー専用 */
#define	HZX_SEG_CLIFF 		  0x00080000	/* 崖あたり */
#define HZX_SEG_RECOIL_TYPE   0x0004	/* 跳弾専用壁 */
#define	HZX_SEG_SKIP	      0x0008	/* チェックをしない（動的壁専用） */

#define	HZX_SEG_NO_PEEPINTRPT 0x00100000	/* 覗き込みの邪魔にならない */
#define	HZX_SEG_DYNAMIC	      0x01000000	/* 動的壁 */
#define	HZX_SEG_DOOR		  0x02000000	/* ドア壁（レーダー表示用） */

#define	HZX_SEG_ATR_ALL	(0x00f3fff0)		/* ここを変えるときはcommand.defも変えましょう */

/* NearHazardCheck, OnlineHazardCheck の引数用 */

#define HZX_TYPE_PLAYER		HZX_SEG_NO_PLAYER
#define HZX_TYPE_ENEMY		HZX_SEG_NO_ENEMY
#define HZX_TYPE_BULLET		HZX_SEG_NO_BULLET
#define HZX_TYPE_MISSILE	HZX_SEG_NO_MISSILE

/*----------------------------------------------------------------*/

#define HZX_CHK_F_FLOOR		0x01
#define HZX_CHK_D_FLOOR		0x02
#define HZX_CHK_F_SEGMENT	0x04
#define HZX_CHK_D_SEGMENT	0x08

#define	HZX_CHK_NOCHECK_FLOOR	0x10	/* 床はチェックしない */
#define	HZX_CHK_NOCHECK_CEIL	0x20	/* 天井はチェックしない */

#define	HZX_CHK_RECOIL_TYPE_ONLY	0x40	/* 跳弾属性のみチェック */
#define	HZX_CHK_RECOIL_TYPE			0x80	/* 跳弾属性もチェック */

#define	HZX_CHK_CYPHER				0x0100	/* サイファー当たりをチェック */
#define	HZX_CHK_CLIFF				0x0200	/* 崖当たりをチェック */

#define HZX_CHK_ALL		( HZX_CHK_F_FLOOR | HZX_CHK_D_FLOOR | \
						  HZX_CHK_F_SEGMENT | HZX_CHK_D_SEGMENT )

#define HZX_CHK_FIX		( HZX_CHK_F_FLOOR | HZX_CHK_F_SEGMENT )
#define HZX_CHK_DYNAMIC		( HZX_CHK_D_FLOOR | HZX_CHK_D_SEGMENT )

#define	HZX_CHK_SEGMENT		(HZX_CHK_F_SEGMENT|HZX_CHK_D_SEGMENT)
#define	HZX_CHK_FLOOR		(HZX_CHK_F_FLOOR|HZX_CHK_D_FLOOR)

/*----------------------------------------------------------------*/

/* FLOOR アトリビュート */

//#define HZX_FLOOR_DYNAMIC	0x01

#define	HZX_FLOOR_ALL					0x0000	/* 全チェック */
#define HZX_FLOOR_RECT                	0x0001	/* 完全な長方形 */
#define HZX_FLOOR_FLAT                	0x0002	/* 水平 */
#define HZX_FLOOR_NO_PLAYER           	0x0010	/* プレイヤーの当たりチェックをしない */
#define HZX_FLOOR_NO_ENEMY            	0x0020	/* 敵兵の当たりチェックをしない */
#define HZX_FLOOR_NO_BULLET           	0x0040	/* ショットの当たりをチェックしない */
#define HZX_FLOOR_NO_MISSILE          	0x0080	/* ミサイル系の当たり判定をしない */
#define HZX_FLOOR_NO_C4              	0x0100	/* C4爆弾がセットできない */
#define HZX_FLOOR_NO_RECOIL           	0x0200	/* ショットが跳弾しない */
#define HZX_FLOOR_NO_CLAYMORE     		0x0400	/* クレイモアがセットできない */
#define HZX_FLOOR_NO_BLOOD            	0x0800	/* 血痕つかない */
#define HZX_FLOOR_NO_BULLETHOLE  		0x1000	/* 弾痕つかない */
#define HZX_FLOOR_NO_SPRAY            	0x2000	/* スプレーつかない */
#define HZX_FLOOR_IK 					0x4000	/* インバース・キネマティクスを適用する */
#define HZX_FLOOR_STEP               	0x8000	/* ここの床は階段である */
#define	HZX_FLOOR_NO_ENEMY_EYES			0x00010000	/* 敵兵視線通りぬける */
#define	HZX_FLOOR_NO_OBJECT				0x00020000	/* オブジェクトつかない（薬莢、マガジンなど） */
#define	HZX_FLOOR_CYPHER 		  		0x00040000	/* サイファー専用 */
#define	HZX_FLOOR_RECOIL_TYPE			0x0004	/* 跳弾専用 */
#define	HZX_FLOOR_SKIP		        	0x0008	/* チェックをしない（動的床専用） */

#define	HZX_FLOOR_DYNAMIC	        	0x01000000	/* 動的床 */

#define	HZX_FLOOR_PITFALL				0x02000000	/* 落とし穴（動的床専用） */

#define	HZX_FLOOR_ATR_ALL				(0x0003fff0) /* ここを変えるときはcommand.defも変えましょう */

/*----------------------------------------------------------------*/

/* ビハインドフラグ */

enum {
    HZX_BEHIND_NORMAL = 	0x0000,		/* ノーマル */
    HZX_BEHIND_CHECK_DIR =	0x0001,		/* 方向指定付きビハインド */
	HZX_BEHIND_NO_LEFT_OUT = 0x0002,	/* 左飛び出し不可 */
	HZX_BEHIND_NO_RIGHT_OUT = 0x0004,	/* 右飛び出し不可 */
	HZX_BEHIND_CAMERA_TYPE2 = 0x0010,	/* ビハインドカメラ補完タイプ２ */
	HZX_BEHIND_CAMERA_TYPE3 = 0x0020,	/* ビハインドカメラ補完タイプ３（一瞬） */
	HZX_BEHIND_CAMERA_TYPE4 = 0x0040,	/* ビハインドカメラ補完タイプ４（戻り一瞬） */
	HZX_BEHIND_CAMERA_TYPE5 = 0x0080,	/* ビハインドカメラ補完タイプ５（入りが２で戻り一瞬） */
	HZX_BEHIND_RIGHT_PEEP_SIDE = 0x0100,	/* ビハインド右横覗き */
	HZX_BEHIND_RIGHT_PEEP_BACK = 0x0200,	/* ビハインド右後覗き */
	HZX_BEHIND_LEFT_PEEP_SIDE = 0x0400,	/* ビハインド左横覗き */
	HZX_BEHIND_LEFT_PEEP_BACK = 0x0800,	/* ビハインド左後覗き */
	HZX_BEHIND_CAMERA_TYPE6 = 0x1000,	/* ビハインドカメラ補完タイプ６ */
	HZX_BEHIND_CAMERA_TYPE7 = 0x2000,	/* ビハインドカメラ補完タイプ７ */
	HZX_BEHIND_CAMERA_TYPE8 = 0x4000,	/* ビハインドカメラ補完タイプ８ */
	HZX_BEHIND_CAMERA_TYPE9 = 0x8000,	/* ビハインドカメラ補完タイプ９ */
} ;

/*----------------------------------------------------------------*/

/* インライン */

/* 動的ハザードのアトリビュートを変更する */
static	inline	void	HZX_DynamicSegmentSetAttribute( HZX_D_SEGMENT *seg, int atr )
{
	*( int * )GV_UNCACHE( &seg->atr ) = seg->atr = atr | HZX_SEG_DYNAMIC ;
}

static	inline	void	HZX_DynamicFloorSetAttribute( HZX_D_FLOOR *seg, int atr )
{
	*( int * )GV_UNCACHE( &seg->atr ) = seg->atr = atr | HZX_FLOOR_DYNAMIC ;
}

static	inline	void	HZX_DynamicSegmentSetSkip( HZX_D_SEGMENT *seg )
{
	*( int * )GV_UNCACHE( &seg->atr ) = seg->atr = seg->atr | HZX_SEG_SKIP ;
}

static	inline	void	HZX_DynamicSegmentResetSkip( HZX_D_SEGMENT *seg )
{
	*( int * )GV_UNCACHE( &seg->atr ) = seg->atr = seg->atr & ~HZX_SEG_SKIP ;
}

/*----------------------------------------------------------------*/

/*
	関数プロトタイプ
*/

// in trap.c

extern	void HZX_EnterTrap( HZX_GROUP_ID id, HZX_EVT *ev );
extern	void HZX_FlashTrap( HZX_GROUP_ID id, HZX_EVT *ev );
extern	int  HZX_CheckBehind( HZX_GROUP_ID id, HZX_BEHIND **bhds, FVECTOR *pos, SVECTOR *rot );

extern	int	HZX_EnterTrapNoExecEvent( HZX_GROUP_ID id, HZX_EVT *ev ) ;
extern	int	HZX_CheckInsideTrap( HZX_GROUP_ID id, FVECTOR *pos, int name_id ) ;
extern	int	HZX_CheckInsideAllTrap( FVECTOR *pos, int name_id ) ;

extern	int	HZX_FindTrap( HZX_GROUP_ID hzx_id, int name, HZX_BLOCK **blkp, HZX_TRP **trpp ) ;
extern	void	HZX_SwitchTrapDefault( void ) ;
extern	void HZX_SwitchTrapQuick( HZX_GROUP_ID, int, int ) ;
extern	void	HZX_InitTrapBehindSystem( void ) ;

// in bind.c

extern	long64	HZX_PlayerStateMask[] ;

extern	int  HZX_ExecEvent( HZX_GROUP_ID id, HZX_EVT *ev, u_int mode );
extern	void HZX_SetEvent( HZX_EVT *ev, int chara, FVECTOR *mov, SVECTOR *rot );
extern	void HZX_SetBind( HZX_BND *bnd, char *top );
extern	void HZX_InitTrapBind( void );

// in dynamic.c

extern	void	HZX_DynamicSegmentExecCallback( HZX_D_SEGMENT *seg, int flag ) ;
extern	void	HZX_DynamicFloorExecCallback( HZX_D_FLOOR *seg, int flag ) ;

extern	HZX_D_CALLBACK	*HZX_SetDynamicCallback( void *, HZX_D_CALLBACK *, 
										 HZX_CALLBACK_FUNC, void *, void * ) ;
extern	void	HZX_RemoveDynamicCallback( void *, HZX_D_CALLBACK * ) ;
extern	void	HZX_CallbackUpdatePos( void *, FVECTOR *, FVECTOR * ) ;
extern	void	HZX_CallbackUpdateMatrix( void *hzd, FMATRIX *old, FMATRIX *pNew ) ;

extern	int		HZX_IsDynamic( HZX_HZD *hzd ) ;

extern	void HZX_MoveDynamicSegment( HZX_D_SEGMENT *seg, IVECTOR *p1, IVECTOR *p2 );
extern	void HZX_RotateDynamicSegment( HZX_D_SEGMENT *seg, IVECTOR *axis, SVECTOR *rotate );
extern	void HZX_RotateDynamicSegment2( HZX_D_SEGMENT *seg, IVECTOR *axis, FMATRIX *mat ) ;
extern	void	HZX_ShiftDynamicSegment( HZX_D_SEGMENT *seg, IVECTOR *shift ) ;
extern	HZX_D_SEGMENT *HZX_AddDynamicSegment( HZX_GROUP_ID id, IVECTOR *p1, IVECTOR *p2, u_int atr );
extern	void HZX_RemoveDynamicSegment( HZX_D_SEGMENT *seg );
extern	void HZX_MoveDynamicFloor( HZX_D_FLOOR *flr, IVECTOR *p1, IVECTOR *p2
								  , IVECTOR *p3, IVECTOR *p4 );
extern	void HZX_RotateDynamicFloor( HZX_D_FLOOR *flr, IVECTOR *axis, SVECTOR *rotate ) ;
extern	void HZX_RotateDynamicFloor2( HZX_D_FLOOR *flr, IVECTOR *axis, FMATRIX *mat ) ;
extern	void	HZX_ShiftDynamicFloor( HZX_D_FLOOR *seg, IVECTOR *shift ) ;
extern	HZX_D_FLOOR	*HZX_AddDynamicFloor( HZX_GROUP_ID id, IVECTOR *p1, IVECTOR *p2
								  , IVECTOR *p3, IVECTOR *p4, int n, u_int atr );

extern	HZX_D_FLOOR	*HZX_AddDynamicFloorF( HZX_GROUP_ID id, FVECTOR *p, int n, u_int atr ) ;

extern	void HZX_RemoveDynamicFloor( HZX_D_FLOOR *flr ) ;

extern	void	HZX_FixDynamicSegment( HZX_D_SEGMENT * ) ;
extern	void	HZX_FixDynamicFloor( HZX_D_FLOOR * ) ;

#ifdef HZX_DTRP
extern	HZX_D_TRP	*HZX_AddDynamicTrap( HZX_GROUP_ID hzx_id,
								 int   name_id,
								 float min_x, float min_y, float min_z,
								 float max_x, float max_y, float max_z, 
								 int atr ) ;
extern	void		HZX_RemoveDynamicTrap( HZX_GROUP_ID hzx_id, HZX_D_TRP *trp ) ;
#endif

// in nearx.c

extern	int	HZX_NearHazardCheck( HZX_GROUP_ID id, FVECTOR *from, int sphere
						  , int chk_flag, int seg_flag, int r_sphere );
extern	void HZX_GetNearHazard( HZX_SEG *segs, int *atrs );
extern	void HZX_GetNearHazardPtr( SVECTOR **segs, HZX_BLOCK **blk );
extern	void HZX_MakeNearHazard( HZX_SEG *segs, SVECTOR **v, HZX_BLOCK **blk );
extern	void HZX_GetNearVector( FVECTOR *vect_ptr );
extern	void HZX_GetIsEdge( signed char *ie );
extern	void HZX_GetReactVector( FVECTOR *react );
extern	void HZX_GetNearHazardAtr( int *atrs );
extern	void HZX_GetNearHazardF( HZX_SEG **segs );
extern	int	*HZX_GetNearBlockID( HZX_GRP *grp, FVECTOR *from, int sphere, int *n_blocks );

// in onlinex.c

extern	int HZX_OnlineHazardCheck( HZX_GROUP_ID id, FVECTOR *from, FVECTOR *to,
			       int chk_flag, int seg_flag, int flr_flag ) ;
extern	int	HZX_OnlineHazardCheckOneSegment( HZX_SEG *, FVECTOR *, FVECTOR * ) ;

extern	void HZX_GetOnlineHazard( HZX_FLR *seg, int *atr );
extern	void HZX_GetOnlineHazard2( HZX_HZD *seg ) ;
extern	void HZX_GetOnlineHazardPtr( SVECTOR *v, HZX_BLOCK *blk );
extern	void HZX_GetOnlineVector( FVECTOR *vect_ptr );
extern	void HZX_GetOnlinePoint( FVECTOR *ptp_ptr );
extern	int	 HZX_GetOnlineHazardAtr( void );
extern	int	 HZX_GetOnlineHazardFloorType( void );
extern	int	 HZX_GetOnlineHazardType( void );
extern	int	 HZX_GetOnlineHazardGroup( void );
extern	HZX_SEG	*HZX_GetOnlineHazardF( void );

// in levelx.c

extern	int		HZX_LevelHazardCheck( HZX_GROUP_ID id, FVECTOR *from, int chk_flag, int flr_flag );
extern	void	HZX_GetLevelHazard( HZX_FLR *flr, int *atr );
extern	void	HZX_GetLevelHazardPtr( SVECTOR **flr, HZX_BLOCK **blk );
extern	void	HZX_GetLevelHazardType( int *type );
extern	void	HZX_GetLevelHeight( float *lvl_ptr );
extern	float	HZX_GetFloorLevel( void );
extern	int		HZX_GetLevelAtr( void );
extern	void	HZX_GetLevelHazardF( HZX_FLR **flrs );
extern	void	HZX_SlopeFloorLevel( float *h, FVECTOR *mov, HZX_FLR *flr );
extern	HZX_BLOCK	*HZX_GetInsideBlock( HZX_GROUP_ID id, FVECTOR *mov );
extern	void	HZX_GetBounding( HZX_HZD *, FVECTOR *, FVECTOR * ) ;

// in navigate.c
extern	int HZX_RouteCourseNearFlag( int near_flag ) ;
extern	void HZX_ChangeRouteCourse( int course ) ;
extern	void HZX_ClearRouteCourse( int course ) ;
extern	int	HZX_GetAddress( HZX_GROUP_ID id, FVECTOR *pos, int address );
extern	int	HZX_ReachTo( int addr1, int addr2 );
extern	int	HZX_Navigate( int addr1, int addr2, FVECTOR *pos );
extern	int	HZX_NavigateMapToMap( int addr1, int addr2 );
extern	int	HZX_LinkZoneAddress ( HZX_GROUP_ID id, int zone ); 
extern	int HZX_NextZone( HZX_GROUP_ID id, int zone_from, int zone_to ) ;
extern	HZX_ZONE_ADD HZX_NextZoneCrossGroup( HZX_ZONE_ADD from_addr, HZX_ZONE_ADD to_addr ) ;
extern	HZX_ZONE_ADD HZX_NextZoneCrossGroup2( HZX_ZONE_ADD from_addr, HZX_ZONE_ADD to_addr ) ;
extern	int HZX_ZoneDistance( HZX_GROUP_ID id, int zone_from, int zone_to );
extern	int HZX_ZoneDistanceCrossGroup( int from_addr, int to_addr ) ;
extern	int HZX_OverDistanceZone( HZX_GROUP_ID id, long64 zone_from, long64 zone_to,int dis_th );
extern	int HZX_BoundOutZone( HZX_GROUP_ID id, int zone_from, int zone_to, int dis_th );
extern	int HZX_BoundOutZoneCrossGroup( int from_addr, int to_addr, int dis_th ) ;
extern	int	HZX_InsideZone( HZX_GROUP_ID id, FVECTOR *pos, int zone );
extern	int	HZX_NearZones( HZX_GROUP_ID id, int zone, int *near );
extern	int	HZX_FarZoneNavigate( HZX_GROUP_ID id, int zone1, int zone2, int *rout );
extern	int	HZX_FarZoneNavigate2( HZX_GROUP_ID id, int nowzone, int trgzone1, int trgzone2, int *rout );
extern	int	HZX_GetRoute( HZX_GROUP_ID id, int zone1, int zone2 );
extern	int	HZX_GetRouteCrossGroup( int , int  );
extern	int	HZX_GetHzxIDbyZoneEx( HZX_GROUP_ID id, FVECTOR *pos, int *zone_num, int near_check_flag ) ;
extern	int	HZX_GetHzxIDbyZone( HZX_GROUP_ID id, FVECTOR *pos, int *zone_num ) ;
//#define HZX_GetHzxIDbyZone( _i, _p, _z )	HZX_GetHzxIDbyZoneEx( _i, _p, _z, 1 )
extern	void	HZX_GetInterruptZone( FVECTOR *pos, int dir, int len, HZX_ZONE_ADD *zoneadd ) ;
extern	void	HZX_SetNearFlag( HZX_ZONE_ADD zadd1, HZX_ZONE_ADD zadd2, int flag ) ;
extern	void	HZX_UnsetNearFlag( HZX_ZONE_ADD zadd1, HZX_ZONE_ADD zadd2, int flag ) ;
extern	int	HZX_GetNearFlag( HZX_ZONE_ADD from, HZX_ZONE_ADD to ) ;

extern	HZX_ZON *HZX_CheckChangeGroup( HZX_GROUP_ID *hzx_id, FVECTOR *pos
							   , HZX_ZON *prev_link, int *addr );
extern	HZX_ZON	*HZX_CheckChangeGroupMapLink( HZX_GROUP_ID *hzx_id, FVECTOR *from, FVECTOR *to,
											  HZX_ZON *prev_link, int *addr ) ;

extern	HZX_ZONE_ADD HZX_ZoneStatusZ2Z( HZX_ZONE_ADD from_addr,HZX_ZONE_ADD to_addr, int status ) ;
int HZX_ReadOnlinInfo(int addr1,int addr2) ;

extern	int		HZX_GetLowerZone( HZX_GROUP_ID hzx_id, FVECTOR *pos ) ;
extern	void	HZX_Pos2Zone( FVECTOR *, int *, int * ) ;
extern	int	HZX_Zone2ZoneCornerCount( HZX_ZONE_ADD from, HZX_ZONE_ADD to ) ;

// in route.c

extern	void 	HZX_MakeRoute( HZX_GRP *grp, u_char *routes );

// in hzxd.c

extern	void HZX_ResetSystem( void );
extern	void HZX_ResetMemory( void );
extern	void HZX_Initialize( void );
extern	void HZX_StartDaemon( void );

extern	void	*HZX_StartTrapSwitchSystem( void ) ;
extern	int	 	HZX_SwitchTrap( int, int ) ;

//BP extern	int				NewHzxSetGroupAdd( void ) ;
//BP extern	int				NewHzxResetGroupAdd( void ) ;
extern	HZX_GROUP_ID	HZX_AddGroupID( HZX_GROUP_ID id ) ;

extern	int				HZX_N_MapLinks ;
extern	HZX_MAPLINK		*HZX_MapLink ;

// in loadhzx.c

extern	void	HZX_InitHzxCache( void );
extern	HZX_HDL	*HZX_GetHandler( int id );
extern	void	HZX_FreeHandler( void ) ;
extern	void	HZX_SetupGroup( HZX_HDL *hzx );
extern	int		HZX_LoadHzx( HZX_DEF *hzx, int id );

/*---------------------------------------------------------------------*/

/* デバッグ用 */
#ifdef DEBUG_MODE
//#define HZX_CLOCK_CHECK
#ifdef HZX_CLOCK_CHECK
#define	HZXD( a )	a
#else
#define	HZXD( a )
#endif

extern	void	*NewLineView( FVECTOR *, int, u_char, u_char, u_char ) ;
extern	void	*NewTriangleView( FVECTOR *, int, u_char, u_char, u_char ) ;
extern	void	*NewSquareView( FVECTOR *, int, u_char, u_char, u_char ) ;
extern	void	*NewPointView( FVECTOR *, u_char, u_char, u_char ) ;
extern	void	*NewBoundingBoxView( FVECTOR *, FVECTOR *, u_char, u_char, u_char ) ;
extern	void	HZX_ViewSegment( HZX_SEG * ) ;
extern	void	HZX_ViewFloor( HZX_FLR *, int ) ;
extern	void	HZX_ViewHazard( HZX_HZD *hzd ) ;
extern	void	HZX_ViewVuSegRGB( HZX_BLOCK *, HZX_VuSEG *, u_char, u_char, u_char ) ;
#define	HZX_ViewVuSeg( _b, _s )		HZX_ViewVuSegRGB( _b, _s, 32, 32, 240 )
extern	void	HZX_ViewVuFlr( HZX_BLOCK *, HZX_VuSEG * ) ;
extern	void	HZX_ViewDynamicSegment( HZX_D_SEGMENT * ) ;
extern	void	HZX_ViewDynamicFloor( HZX_D_FLOOR *, int ) ;
extern	void	HZX_ViewZone1( HZX_ZON *zon, u_char r, u_char g, u_char b ) ;
extern	void	HZX_ViewZone( HZX_GROUP_ID, int ) ;
extern	void	HZX_ViewMatrix( FMATRIX *world, float len ) ;
extern	void	HZX_ViewSeNo( HZX_HZD *hzd, FVECTOR *pos ) ;

extern	void	DumpSegment( HZX_SEG * ) ;
extern	void	DumpFloor( HZX_FLR * ) ;
extern	void	DumpVec( FVECTOR * ) ;
extern	void	DumpSVec( SVECTOR * ) ;
extern	void	ViewFromTo( FVECTOR *, FVECTOR *, u_char, u_char, u_char ) ;

extern	int		HZX_OnlineDebugFlag ;
extern	int		HZX_NearDebug ;
extern	int		HZX_LevelDebug ;
#else
#define		HZXD( a )
#define		NewLineView( _f, _n, _r, _g, _b )
#define		NewTriangleView( _f, _n, _r, _g, _b )
#define		NewSquareView( _f, _n, _r, _g, _b )
#define		NewPointView( _f, _r, _g, _b )
#define		NewBoundingBoxView( _l, _h, _r, _g, _b )
#define		HZX_ViewSegment( _s )
#define		HZX_ViewFloor( _f, _n )
#define		HZX_ViewHazard( _h ) ;
#define		HZX_ViewVuSegRGB( _l, _s, _r, _g, _b )
#define		HZX_ViewVuSeg( _b, _s )		HZX_ViewVuSegRGB( _b, _s, 32, 32, 240 )
#define		HZX_ViewVuFlr( _b, _s )
#define		HZX_ViewDynamicSegment( _s )
#define		HZX_ViewDynamicFloor( _f, _n )
#define		HZX_ViewZone1( _z, _r, _g, _b )
#define		HZX_ViewZone( _i, _f )
#define		HZX_ViewMatrix( _w, _l )
#define		HZX_ViewSeNo( _h, _p )
#define		DumpSegment( _s )
#define		DumpFloor( _f )
#define		DumpVec( _v ) ;
#define		DumpSVec( _v ) ;
#define		ViewFromTo( _f, _t, _r, _g, _b )

#endif


#if 1 //BP
//#ifdef KP_XBOX
//x/ 足りないフラグを追加
enum {
	HZX_KIND_NOTHING	= 0x00,
	HZX_KIND_WALL		= 0x01,
	HZX_KIND_FLOOR		= 0x02,
	HZX_KIND_WHICHMASK	= 0x03,
	HZX_KIND_DYNAMIC	= 0x04,
};
#endif

#ifdef __cplusplus
}
#endif

#endif



