/******************************************************************************
 * System	: Virtual Reality Studio KONAMI metalgear HZX file struct
 * Computer : DOS/V
 * OS		: Microsoft Windows
 * Compiler : Microsoft Visual C++
 * Module	: kernel
 */

#ifndef _INC_HZXSTRUCT2
#define _INC_HZXSTRUCT2

/******************************************************************************
 * HZX ファイルが変更。HZXStruct.h は旧バージョン。HZXStruct2.h へ。
 */

/******************************************************************************
 * definitions and typedefs and structures
 */

#define HZX_HAZARDKEYNAME          "KONAMI HZM Hazard"
#define HZX_FLOORKEYNAME           "KONAMI HZM Floor"
#define HZX_CAMERAKEYNAME          "KONAMI HZM Camera"
#define HZX_TRAPKEYNAME            "KONAMI HZM Trap"
#define HZX_ZONEKEYNAME            "KONAMI HZM Zone"
#define HZX_OVERZONEKEYNAME        "KONAMI HZM Over Zone"
#define HZX_OVERROUTEKEYNAME       "KONAMI HZM Over Route"
#define HZX_ZONETOZONEKEYNAME      "KONAMI HZX Zone to Zone"
#define HZX_CHARACTERKEYNAME       "KONAMI HZM Character"
#define HZX_CHARACTERCOORDKEYNAME  "KONAMI HZM Character coord"
#define HZX_CHARACTER2KEYNAME      "KONAMI HZM Character2"
#define HZX_CHARACTERCOORD2KEYNAME "KONAMI HZM Character coord2"
#define HZX_GROUPKEYNAME           "KONAMI HZX Group"
#define HZX_CLEARINGAREAKEYNAME    "KONAMI HZX Clearing area"
#define HZX_CLEARINGCOORDKEYNAME   "KONAMI HZX Clearing coord"
#define HZX_CLEARINGCOORD2KEYNAME  "KONAMI HZX Clearing coord2"
#define HZX_CHARACTER_VR_INFO      "KONAMI HZX Character VR data"
#define HZX_CLEARING_VR_INFO       "KONAMI HZX Clearing VR data"

/* HZX Hazard Status */
#define HZX_SEG_NO_PLAYER     0x00000010		/* プレイヤーの当たりチェックをしない */
#define HZX_SEG_NO_ENEMY      0x00000020		/* 敵兵の当たりチェックをしない */
#define HZX_SEG_NO_BULLET     0x00000040		/* ショットの当たりをチェックしない */
#define HZX_SEG_NO_MISSILE    0x00000080		/* ミサイル系の当たり判定をしない */
#define HZX_SEG_NO_C4         0x00000100		/* C4爆弾がセットできない */
#define HZX_SEG_NO_RECOIL     0x00000200		/* ショットが跳弾しない */
#define HZX_SEG_NO_HARITSUKI  0x00000400		/* 張り付かない */
#define HZX_SEG_NO_DISP_RADAR 0x00000800		/* レーダーに写らない */
#define HZX_SEG_NO_BULLETHOLE 0x00001000	/* 弾痕つかない */
#define HZX_SEG_NO_SPRAY      0x00002000	/* スプレーつかない */
#define HZX_SEG_RECOIL_TYPE   0x00000004	/* 跳弾専用壁 */
#define HZX_SEG_NO_ENEMY_EYES 0x00010000	/* 敵兵視線チェックをしない */
#define HZX_SEG_NO_KNOCK_SE   0x00020000	/* 壁ノック音なし */
#define HZX_SEG_CYPHER        0x00040000	/* サイファー専用壁 */
#define HZX_SEG_NO_ENEMY_IK   0x00004000	/* 敵IKチェックしない */
#define HZX_SEG_CLIFF         0x00080000    /* 崖当たり */

/* HZX Floor Status */
#define HZX_FLOOR_RECT               0x00000001		/* 完全な長方形 */
#define HZX_FLOOR_FLAT               0x00000002		/* 水平 */
#define HZX_FLOOR_NO_PLAYER          0x00000010		/* プレイヤーの当たりチェックをしない */
#define HZX_FLOOR_NO_ENEMY           0x00000020		/* 敵兵の当たりチェックをしない */
#define HZX_FLOOR_NO_BULLET          0x00000040		/* ショットの当たりをチェックしない */
#define HZX_FLOOR_NO_MISSILE         0x00000080		/* ミサイル系の当たり判定をしない */
#define HZX_FLOOR_NO_C4              0x00000100		/* C4爆弾がセットできない */
#define HZX_FLOOR_NO_RECOIL          0x00000200		/* ショットが跳弾しない */
#define HZX_FLOOR_NO_CLAYMORE        0x00000400		/* クレイモアがセットできない */
#define HZX_FLOOR_NO_BLOOD           0x00000800		/* 血痕つかない */
#define HZX_FLOOR_NO_BULLETHOLE      0x00001000		/* 弾痕つかない */
#define HZX_FLOOR_NO_SPRAY           0x00002000		/* スプレーつかない */
#define HZX_FLOOR_INVERSE_KINEMATICS 0x00004000		/* インバース・キネマティクスを適用する */
#define HZX_FLOOR_STEP               0x00008000		/* ここの床は階段である */
#define HZX_FLOOR_RECOILE_TYPE       0x00000004		/* 跳弾用床 */
#define HZX_FLOOR_NO_ENEMY_EYES      0x00010000		/* 敵兵視線チェックをしない */
#define HZX_FLOOR_NO_OBJECT          0x00020000		/* オブジェクト付かない。(薬莢など) */
#define HZX_FLOOR_CYPHER             0x00040000		/* サイファー専用床 */

/* HZX Trap Status */
#define HZX_TRAP_NO_CHECK            0x00000001		/* 初期状態を無効にする */

/* キャラクタプラグイン用フラグ(HZXへのコンバート時には使用しない) */
#define CHARACTER_AIM_FACE_MOVE  0x00000001	/* 注視点を見つめながら移動する(顔ハーフ) */
#define CHARACTER_AIM_GUN_MOVE   0x00000002	/* 注視点を警戒しながら移動する */
#define CHARACTER_AIM_FACE       0x00000004	/* 注視点を見つめる(顔ハーフ) */
#define CHARACTER_AIM_GUN        0x00000008	/* 注視点を警戒する */
#define CHARACTER_PINPOINT       0x00000020	/* ピンポイント移動 */
#define CHARACTER_STAGE_ONE      0x00000040	/* ステージ固有アクション */
#define CHARACTER_STAGE_LOOP     0x00000080	/* ステージ固有ループアクション */
#define CHARACTER_FACE_ONLY_MOVE 0x00000100	/* 注視点を見つめながら移動する(顔のみ) */
#define CHARACTER_FACE_ONLY      0x00000200	/* 注視点を見つめる(顔のみ) */
#define CHARACTER_NEAR_WALL      0x00000400	/* 通常より壁に接近する(r_sphere=100) */
#define CHARACTER_COUTION_STAND  0x00000800	/* 警戒モードでは立ちモーション */
#define CHARACTER_COUTION_LOOK   0x00001000	/* 警戒モードではきょろきょろモーション */
#define CHARACTER_TALK_ONESELF   0x00004000	/* 独り言 */


#define MAX_ZONEROUTE      6	/* ルートの最大数 */

#define MAX_VUSEG_VERTEX   320
#define MAX_BLOCK_SEGMENTS 76	/* １ブロック最大壁数 */
#define MAX_BLOCK_FLOORS   32	/* １ブロック最大床数 */

#define HZXCALCERROR          0.01	/* HZX 関連で用いる誤差範囲 */

#define HZX_VR_CHARACTER_GROUP_MAX 8
#define HZX_VR_CLEARING_GROUP_MAX  8

/* VRS シーンに書き込む際のフォーマット -------- ここから -------- */
/* HZM 形式と互換を保つ為、構造体先頭は HZM 構造体メンバ先頭と同様 */

typedef struct tagHZXHAZARDINFO	/* 壁補助情報 */
{
	BOOL bNoPlayer;		/* プレイヤーの当たりチェックをしない */
	BOOL bNoEnemy;		/* 敵兵の当たりチェックをしない */
	BOOL bNoBullet;		/* ショットの当たりをチェックしない */
	BOOL bNoMissile;	/* ミサイル系の当たり判定をしない */
	BOOL bNoC4;			/* C4 爆弾がセットできない */
	BOOL bNoRecoil;		/* ショットが跳弾しない */
	BOOL bNoHarituki;	/* 張り付かない */
	BOOL bNoDispRadar;	/* レーダーに写らない */
	int  nSound;		/* 叩いたときの音 */
	BOOL bNoBulletHole;	/* 弾痕が付かない */
	BOOL bNoSpray;		/* スプレーが付かない */
	BOOL bRecoilOnly;	/* 跳弾専用壁 */
	BOOL bNoEnemyEyes;	/* 敵兵の視線チェックをしない */
	BOOL bNoKnockSE;	/* 壁ノック音しない */
	BOOL bCypher;		/* サイファー専用壁 */
	BOOL bNoEnemyIK;	/* 敵IKチェックしない */
	BOOL bCliff;		/* 崖当たり */
} HZXHAZARDINFO, *PHZXHAZARDINFO, *LPHZXHAZARDINFO;

typedef struct tagHZXFLOORINFO	/* 壁補助情報 */
{
	BOOL bNoC4;					/* C4 爆弾がセットできない */
	BOOL bNoClaymore;			/* クレイモアがセットできない */
	int  nSound;				/* 叩いたときの音 */
	BOOL bNoRecoil;				/* ショットが跳弾しない */
	BOOL bNoPlayer;				/* プレイヤーの当たりチェックをしない */
	BOOL bNoEnemy;				/* 敵兵の当たりチェックをしない */
	BOOL bNoBullet;				/* ショットの当たりをチェックしない */
	BOOL bNoMissile;			/* ミサイル系の当たり判定をしない */
	BOOL bNoBulletHole;			/* 弾痕が付かない */
	BOOL bNoSpray;				/* スプレーが付かない */
	BOOL bNoBlood;				/* 血痕が付かない */
	BOOL bInverseKinematics;	/* インバース・キネマティクスを適用する */
	BOOL bStep;                 /* この床は階段の床である */
	BOOL bRecoilOnly;			/* 跳弾専用壁 */
	BOOL bNoEnemyEyes;			/* 敵兵の視線チェックをしない */
	BOOL bNoObject;				/* オブジェクト付かない。(薬莢など) */
	BOOL bCypher;				/* サイファー専用床 */
} HZXFLOORINFO, *PHZXFLOORINFO, *LPHZXFLOORINFO;

typedef struct tagHZXCAMERAINFOOLD	/* 旧カメラ補助情報 */
{
	int nTargetX;	/* ターゲット X */
	int nTargetY;	/* ターゲット Y */
	int nTargetZ;	/* ターゲット Z */
	int nCameraX;	/* カメラ X */
	int nCameraY;	/* カメラ Y */
	int nCameraZ;	/* カメラ Z */
} HZXCAMERAINFOOLD, *PHZXCAMERAINFOOLD, *LPHZXCAMERAINFOOLD;

typedef struct tagHZXCAMERAINFO	/* カメラ補助情報 */
{
	P3DXYZ xyzTarget;		/* ターゲット */
	P3DXYZ xyzCamera;		/* カメラ */
	P3DXYZ xyzRightTarget;	/* 右覗き込みターゲット */
	P3DXYZ xyzRightCamera;	/* 右覗き込みカメラ */
	P3DXYZ xyzLeftTarget;	/* 左覗き込みターゲット */
	P3DXYZ xyzLeftCamera;	/* 左覗き込みカメラ */
	int    nOptionDir;		/* オプション発生方向 */
	UINT   unOptionFlag;	/* オプションフラグ */
} HZXCAMERAINFO, *PHZXCAMERAINFO, *LPHZXCAMERAINFO;

typedef struct tagHZXTRAPINFO	/* トラップ属性情報 */
{
	BOOL bNoCheck;					/* トラップの初期状態を無効にする */
	int  nDarkArea;					/* ダークアリア指定。０がデフォルト。１～７まで */
} HZXTRAPINFO, *PHZXTRAPINFO, *LPHZXTRAPINFO;

typedef struct tagHZXZONEINFO	/* ゾーン補助情報 */
{
	int nReservedNo;	/* 予約番号 */
	int nIncremental;	/* 逐次番号(ルート編集に使用する) */
	int nFlag;			/* フラグ */
} HZXZONEINFO, *PHZXZONEINFO, *LPHZXZONEINFO;

typedef struct tagHZXOVERZONEINFO	/* ゾーン制限チェック用 */
{
	int nOverPolygonCount;	/* 制限を越えたときのポリゴン数 */
} HZXOVERZONEINFO, *PHZXOVERZONEINFO, *LPHZXOVERZONEINFO;

typedef struct tagHZXOVERROUTEINFO	/* ルート制限チェック用 */
{
	int nOverRouteCount;	/* 制限を越えたときのルート数 */
} HZXOVERROUTEINFO, *PHZXOVERROUTEINFO, *LPHZXOVERROUTEINFO;

typedef struct tagHZXZONETOZONEINFO	/* ゾーン間属性情報 */
{
	int nFlag;	/* フラグ */
} HZXZONETOZONEINFO, *PHZXZONETOZONEINFO, *LPHZXZONETOZONEINFO;

typedef struct tagHZXCHARACTERINFO	/* キャラクタ補助情報 */
{
	int  nStart;	/* 開始位置 */
	BOOL bLoop;		/* TRUE:ループする	FALSE:しない */
	int  nFlag;		/* フラグ */
} HZXCHARACTERINFO, *PHZXCHARACTERINFO, *LPHZXCHARACTERINFO;

typedef struct tagHZXCHARACTERCOORDINFO	/* キャラクタ頂点補助情報 */
{
	int    nAction;		/* アクション */
	int    nTime;		/* 時間 */
	int    nDirection;	/* 方向 */
	P3DXYZ xyzTarget;	/* 視点位置 */
	int    nFlag;		/* フラグ */
} HZXCHARACTERCOORDINFO, *PHZXCHARACTERCOORDINFO, *LPHZXCHARACTERCOORDINFO;

typedef struct tagHZXGROUPINFO	/* グループ分割数 */
{
	int nXDivide;	/* 分割数 */
	int nYDivide;
	int nZDivide;
} HZXGROUPINFO, *PHZXGROUPINFO, *LPHZXGROUPINFO;

typedef struct tagHZXCLEARINGAREAINFO	/* クリアリング エリアデータ */
{
	int nTime;	/* 突入までの時間 */
} HZXCLEARINGAREAINFO, *PHZXCLEARINGAREAINFO, *LPHZXCLEARINGAREAINFO;

typedef struct tagHZXCLEARINGCOORDINFO	/* クリアリング ポイントデータ */
{
	P3DXYZ xyzTarget;	/* 注視点座標 */
	int    nAction;		/* アクション */
	int    nTime;		/* 時間 */
	int    nDirection;	/* 方向 */
	int    nCondition;  /* フラグ */
} HZXCLEARINGCOORDINFO, *PHZXCLEARINGCOORDINFO, *LPHZXCLEARINGCOORDINFO;

typedef struct tagHZXCHARACTERVRINFO	/* キャラクターＶＲ用データ */
{
	int nVersion;	/* バージョン */
	int nVRIndexNumber;	/* VR用インデックス */
} HZXCHARACTERVRINFO, *PHZXCHARACTERVRINFO, *LPHZXCHARACTERVRINFO;

typedef struct tagHZXCLEARINGVRINFO	/* クリアリングＶＲ用データ */
{
	int nVersion;	/* バージョン */
	int nVRIndexNumber;	/* VR用インデックス */
} HZXCLEARINGVRINFO, *PHZXCLEARINGVRINFO, *LPHZXCLEARINGVRINFO;

/* VRS シーンに書き込む際のフォーマット -------- ここまで -------- */
/*-----------------------------------------------------------*/

typedef struct tagHZXCOORDXYZ	/* 座標 */
{
	short x;	/* x */
	short y;	/* y */
	short z;	/* z */
	short h;	/* h */
} HZXCOORDXYZ, *PHZXCOORDXYZ, *LPHZXCOORDXYZ;

typedef struct tagHZXCOORDXZY	/* 座標 (x, z, y, h) */
{
	short x;	/* x */
	short z;	/* z */
	short y;	/* y */
	short h;	/* h */
} HZXCOORDXZY, *PHZXCOORDXZY, *LPHZXCOORDXZY;

typedef struct tagHZXXYZ	/* 座標 */
{
	float x;	/* x */
	float y;	/* y */
	float z;	/* z */
} HZXXYZ, *PHZXXYZ, *LPHZXXYZ;

typedef struct tagHZXXZY	/* 座標 */
{
	float x;	/* x */
	float z;	/* z */
	float y;	/* y */
} HZXXZY, *PHZXXZY, *LPHZXXZY;

typedef struct tagHZXFVECTORXYZ	/* float のベクトル構造体 */
{
	float x;	/* x */
	float y;	/* y */
	float z;	/* z */
	float w;	/* w */
} HZXFVECTORXYZ, *PHZXFVECTORXYZ, *LPHZXFVECTORXYZ;

typedef struct tagHZXFVECTORXZY	/* float のベクトル構造体 */
{
	float x;	/* x */
	float z;	/* z */
	float y;	/* y */
	float w;	/* w */
} HZXFVECTORXZY, *PHZXFVECTORXZY, *LPHZXFVECTORXZY;

/* 最終ＨＺＸフォーマット */

typedef struct tagHZXVUHAZARD	/* 壁データＶｕ０形式 */
{
	HZXCOORDXYZ   hzxcdxyzBound[2];			/* バウンディング 1 2 */
	LPHZXCOORDXYZ lpHZXCoordVertexs;	/* 頂点データ列 */
	unsigned int  unAttribute;			/* アトリビュート */
	unsigned int* lpunTag;				/* システム内部処理用 */
	unsigned int  unSize;				/* システム内部処理用 */
} HZXVUHAZARD, *PHZXVUHAZARD, *LPHZXVUHAZARD;

typedef struct tagHZXVUBULLETHAZARD	/* 弾用壁データＶｕ０形式 */
{
	HZXCOORDXYZ   hzxcdxyzBound[2];			/* バウンディング 1 2 */
	LPHZXCOORDXYZ lpHZXCoordVertexs;	/* 頂点データ列 */
	unsigned int  unAttribute;			/* アトリビュート */
	unsigned int* lpunTag;				/* システム内部処理用 */
	unsigned int  unSize;				/* システム内部処理用 */
} HZXVUBULLETHAZARD, *PHZXVUBULLETHAZARD, *LPHZXVUBULLETHAZARD;

typedef struct tagHZXVUFLOOR	/* 床データＶｕ０形式 */
{
	HZXCOORDXYZ   hzxcdxyzBound[2];			/* バウンディング 1 2 */
	LPHZXCOORDXYZ lpHZXCoordVertexs;	/* 頂点データ列 */
	unsigned int  unAttribute;			/* アトリビュート */
	unsigned int* lpunTag;				/* システム内部処理用 */
	unsigned int  unSize;				/* システム内部処理用 */
} HZXVUFLOOR, *PHZXVUFLOOR, *LPHZXVUFLOOR;

typedef struct tagHZXVUBULLETFLOOR	/* 弾用床データＶｕ０形式 */
{
	HZXCOORDXYZ   hzxcdxyzBound[2];			/* バウンディング 1 2 */
	LPHZXCOORDXYZ lpHZXCoordVertexs;	/* 頂点データ列 */
	unsigned int  unAttribute;			/* アトリビュート */
	unsigned int* lpunTag;				/* システム内部処理用 */
	unsigned int  unSize;				/* システム内部処理用 */
} HZXVUBULLETFLOOR, *PHZXVUBULLETFLOOR, *LPHZXVUBULLETFLOOR;

typedef struct tagHZXTRAP	/* 空間ファイルトラップレコード */
{
	HZXCOORDXYZ  hzxcdxyzBound[2];			/* バウンディング 1 2 */
	int          nNameID;			/* トラップ ID */
} HZXTRAP, *PHZXTRAP, *LPHZXTRAP;

typedef struct tagHZXCAMERA	/* 空間ファイルカメラレコード */
{
	HZXFVECTORXZY cdxzy[2];			/* エッジの位置 cd[*].h は -2 固定 */
	HZXXYZ        xyzTarget;			/* ターゲット */
	HZXXYZ        xyzCamera;			/* カメラ */
	HZXXYZ        xyzRightTarget;		/* 右覗き込みターゲット */
	HZXXYZ        xyzRightCamera;		/* 右覗き込みカメラ */
	HZXXYZ        xyzLeftTarget;		/* 左覗き込みターゲット */
	HZXXYZ        xyzLeftCamera;		/* 左覗き込みカメラ */
	int           nPadData[2];		/*  */
} HZXCAMERA, *PHZXCAMERA, *LPHZXCAMERA;

typedef struct tagHZXZONE	/* 空間ファイルゾーンレコード */
{
	HZXXZY        xzy;						/* 中心座標  */
	short         w;						/* X 幅 */
	short         h;						/* Z 幅 */
	short         nFlag;					/* フラグ */
	unsigned char ucNear[MAX_ZONEROUTE];	/* 隣接ゾーン番号(余った領域は 0xff) */
	unsigned char ucLength[MAX_ZONEROUTE];	/* 隣接ゾーンの距離 */
	unsigned char ucFlag[MAX_ZONEROUTE];	/* 隣接ゾーンのフラグ */
} HZXZONE, *PHZXZONE, *LPHZXZONE;

typedef struct tagHZXCHARACTERCOORD	/* 空間ファイルキャラクタ座標レコード */
									/* 巡回ポイントデータ */
{
	HZXXZY xzy;			/* 座標 */
	HZXXZY xzyTarget;	/* 視点位置 */
	short  sAction;		/* アクション番号 */
	short  sTime;		/* 継続番号 */
	short  sDirection;	/* 方向（０～４０９５）*/
	short  sPad;
	int    nFlag;		/* フラグ */
} HZXCHARACTERCOORD, *PHZXCHARACTERCOORD, *LPHZXCHARACTERCOORD;

typedef struct tagHZXCHARACTER	/* 空間ファイルキャラクタレコード */
								/* 巡回ルートデータ */
{
	short               nCoord;		/* 座標配列数 */
	short               nPad;		/* 開始位置？ */
	LPHZXCHARACTERCOORD lpCoord;	/* 座標配列へのポインタ */
	int                 nFlag;		/* フラグ */
} HZXCHARACTER, *PHZXCHARACTER, *LPHZXCHARACTER;

typedef struct tagHZXCLEARINGCOORD	/* クリアリング ポイントデータ */
{
	HZXXZY xzy;			/* 座標 */
	HZXXYZ xyzTarget;	/* 注視点座標 */
	short  nAction;		/* アクション */
	short  nTime;		/* 時間 */
	short  nDirection;	/* 方向 */
	short  nCondition;	/* 条件 */
} HZXCLEARINGCOORD, *PHZXCLEARINGCOORD, *LPHZXCLEARINGCOORD;

typedef struct tagHZXCLEARINGROOT	/* クリアリング ルートデータ */
{
	short              nPoints ;	/* ポイント数 */
	short              nPad;
	int                nAlign[2] ;
	LPHZXCLEARINGCOORD lpClearingCoord;	/* クリアリングポイントデータへのポインタ */
} HZXCLEARINGROOT, *PHZXCLEARINGROOT, *LPHZXCLEARINGROOT ;

typedef struct tagHZXCLEARINGAREA	/* クリアリング エリアデータ */
{
	HZXFVECTORXYZ      fvecxyzBound[2];	/* バウンディングボックス */
	int                nNameId;			/* 名前 strcode */
	unsigned short     unTime;			/* 突入までの時間 */
	unsigned short     unRoot;			/* クリアリングルート数 */
	LPHZXCLEARINGROOT  lpRoot;			/* エリア別ルートデータ先頭アドレス */
	int                nAlign;	
} HZXCLEARINGAREA, *PHZXCLEARINGAREA, *LPHZXCLEARINGAREA;

typedef struct tagHZXDYNAMICHAZARD	/* 動的ハザードデータ */
{
	LPHZXVUHAZARD  lpHZXHazard;		/* 壁データへのポインタ プログラム渡し */
	LPHZXVUFLOOR   lpHZXFloor;		/* 床データへのポインタ プログラム渡し */
	unsigned short usHazardCount;	/* 壁数 */
	unsigned short usloorCount;	/* 床数 */
} HZXDYNAMICHAZARD, *PHZXDYNAMICHAZARD, *LPHZXDYNAMICHAZARD;

typedef struct tagHZXBLOCK/* ハザードブロックデータ */
{
	int            nCenterX;	/* ブロック中心 x 座標 */
	int            nCenterZ;	/* ブロック中心 y 座標 */
	int            nCenterY;	/* ブロック中心 z 座標 */
	int            nCenterH;	/* ブロック中心 h 座標 */
	int            nExtension;	/* 拡張ブロック番号 */

	unsigned short usHazardCount;			/* 壁 VuSeg 数 */
	unsigned short usFloorCount;			/* 床 VuSeg 数 */
	unsigned short usBulletHazardCount;		/* 弾用壁 VuSeg 数 */
	unsigned short usBulletFloorCount;		/* 弾用床 VuSeg 数 */
	unsigned short usTrapCount;				/* トラップ数 */
	unsigned short usBlockNumber;			/* ブロックナンバー */

	LPHZXVUHAZARD       lpHZXVuHazard;			/* 壁データへのポインタ */
	LPHZXVUFLOOR        lpHZXVuFloor;			/* 床データへのポインタ */
	LPHZXVUBULLETHAZARD lpHZXVuBulletHazard;	/* 弾用壁データへのポインタ */
	LPHZXVUBULLETFLOOR  lpHZXVuBulletFloor;		/* 弾用床データへのポインタ */
	LPHZXTRAP           lpHZXTrap;				/* トラップへのポインタ */
} HZXBLOCK, *PHZXBLOCK, *LPHZXBLOCK;

typedef struct tagHZXGROUP	/* 空間ファイルグループ */
{
	unsigned int unCameraCount;		/* ビハインド数 */
	unsigned int unZoneCount;		/* ゾーン数 */
	LPHZXCAMERA  lpHZXCamera;		/* ビハインドへのポインタ */
	LPHZXZONE    lpHZXZone;			/* ゾーンへのポインタ */

	unsigned int   unLinkZoneCount;
	unsigned char* lpucLinkZone;

	int          nBoundMinX;		/* グループバウンディングボックス最小値 */
	int          nBoundMinY;
	int          nBoundMinZ;
	int          nBlockSizeX;		/* １ブロックのサイズ */
	int          nBlockSizeY;
	int          nBlockSizeZ;

	unsigned char  ucDivideX;		/* 分割数 */
	unsigned char  ucDivideY;
	unsigned char  ucDivideZ;
	unsigned char  ucPad;

	unsigned short usBlockCount;		/* ブロック数 */
	unsigned short usExBlockCount;		/* ブロック数（拡張ブロック含む） */

	LPHZXBLOCK         lpHZXBlock;			/* ブロックデータへのポインタ */
    LPHZXDYNAMICHAZARD lpHZXDynamicHazard;	/* 動的ハザードへのポインタを用意 */
} HZXGROUP, *PHZXGROUP, *LPHZXGROUP;

typedef struct tagHZXFILEHEADER	/* HZX 空間ファイルヘッダ */
{
	unsigned short usVersion;		/* 3 固定 */
	unsigned short usGroupCount;	/* グループ数 */

	unsigned short usCharacterCount;		/* キャラクタ数 */
	unsigned short usClearingAreaCount;

	LPHZXCHARACTER       lpHZXCharacter;		/* キャラクタデータへのポインタ */
	LPHZXCHARACTERCOORD  lpHZXCharactarCoord;	/* キャラクタ座標データへのポインタ */

	LPHZXCLEARINGAREA    lpHZXClearingArea;	/* クリアリングエリア */
	LPHZXCLEARINGROOT    lpHZXClearingRoot;	/* クリアリングルート */
	LPHZXCLEARINGCOORD   lpHZXClearingCoord;	/* クリアリングポイント */

	unsigned char        *lpRoute;			/* ルート配列(ロードしてから設定) */

	LPHZXGROUP           lpHZXGroup;			/* グループデータへのポインタ */
} HZXFILEHEADER, *PHZXFILEHEADER, *LPHZXFILEHEADER;

/******************************************************************************
 */

#endif // _INC_HZXSTRUCT2
