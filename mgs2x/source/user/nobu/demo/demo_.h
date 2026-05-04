/*
	demo_.h
	    デモ定義ファイル

	2000/01/13 N.Tanaka
	$Id: demo_.h,v 1.1.1.3 2002/11/19 11:46:42 Yoshizawa1 Exp $
*/

#ifndef _INC_DEMO_
#define _INC_DEMO_    /* #defined if demo_.h has been included */

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

/******************************************************************************
 * included
 */

/******************************************************************************
 * definitions and typedefs and structures
 */

#define memset(a,b,c) memset((char*)a,b,c)

#define IDMODEL_CACHEEFFECT GV_CacheID2("null", 'k')
#define IDMODEL_EFFECT      GV_StrCode("null")

#define IDMODEL_M1E1DEMO      GV_StrCode("m1e1demo")
#define IDMODEL_MI24HINDDEMO  GV_StrCode("hinddemo")

/* no shade model */
#define IDMODEL_16O4A  GV_StrCode("16d_o4a")
#define IDMODEL_16O5A  GV_StrCode("16d_o5a")
#define IDMODEL_16O6A  GV_StrCode("16d_o6a")
#define IDMODEL_16O7A  GV_StrCode("16d_o7a")
#define IDMODEL_16O8A  GV_StrCode("16d_o8a")
#define IDMODEL_16O9A  GV_StrCode("16d_o9a")
#define IDMODEL_16O10A GV_StrCode("16d_o10a")
#define IDMODEL_16O4B  GV_StrCode("16d_o4b")
#define IDMODEL_16O5B  GV_StrCode("16d_o5b")
#define IDMODEL_16O6B  GV_StrCode("16d_o6b")
#define IDMODEL_16O7B  GV_StrCode("16d_o7b")
#define IDMODEL_16O8B  GV_StrCode("16d_o8b")
#define IDMODEL_16O9B  GV_StrCode("16d_o9b")
#define IDMODEL_16O10B GV_StrCode("16d_o10b")
#define IDMODEL_16O4C  GV_StrCode("16d_o4c")
#define IDMODEL_16O5C  GV_StrCode("16d_o5c")
#define IDMODEL_16O6C  GV_StrCode("16d_o6c")
#define IDMODEL_16O7C  GV_StrCode("16d_o7c")
#define IDMODEL_16O8C  GV_StrCode("16d_o8c")
#define IDMODEL_16O9C  GV_StrCode("16d_o9c")
#define IDMODEL_16O10C GV_StrCode("16d_o10c")
#define IDMODEL_02A_R8 GV_StrCode("02a_r8")

#ifdef DISP_PAL
#define MGS_DEMOFRAMERATE 25
#else
#define MGS_DEMOFRAMERATE 60
#endif
#define MGS_DEMOSECONDRATE 1000 / MGS_DEMOFRAMERATE

#define WRITE_VRAMLINE    16

#define EFFECT_EXPLOSION             1
#define EFFECT_SMOKE                 2
#define EFFECT_FADEIN                3
#define EFFECT_FADEOUT               4
#define EFFECT_TEXT                  5
#define EFFECT_SURPRISEDMARK         6
#define EFFECT_QUESTIONMARK          7
#define EFFECT_SLEEPMARK             8
#define EFFECT_BLOOD                 9
#define EFFECT_BULLET               10
#define EFFECT_MISSILESMOKE         11
#define EFFECT_BLOODCIRCLE          12
#define EFFECT_BREATH               13
#define EFFECT_SHADOW               14
#define EFFECT_FOOTPRINTS           15
#define EFFECT_MESSAGE              16
#define EFFECT_NINJAEYE             17
#define EFFECT_BUBBLE               18
#define EFFECT_SCOPE                19
#define EFFECT_DARKVISIBLEGOGGLE    20
#define EFFECT_IRRAYSGOGGLE         21
#define EFFECT_GUNSMOKE             22
#define EFFECT_OPTICSCAMOUFLAGE     23
#define EFFECT_ENVIRONMENTMAPPING   24
#define EFFECT_PLASMA               25
#define EFFECT_WINDCIRCLE           26
#define EFFECT_SEPIA                27
#define EFFECT_METALGEARLASER       28
#define EFFECT_UNSHAPEVIEW          29
#define EFFECT_GUSMASK              30
#define EFFECT_STOP                 31
#define EFFECT_URINATIONCIRCLE      32
#define EFFECT_PLASMA2              33
#define EFFECT_BLUR                 34
#define EFFECT_STEAMSMOKE           35
#define EFFECT_MONOTONE             36
#define EFFECT_RIFLESIGHT           37
#define EFFECT_MI24HIND             38
#define EFFECT_BLACKSMOKE           39
#define EFFECT_URINATIONCIRCLE2     40
#define EFFECT_SIGHT                41
#define EFFECT_NINJASWORD           42
#define EFFECT_SUBMARINEROOM        43
#define EFFECT_BLACKSMOKE2          44
#define EFFECT_BLASTLINE            45
#define EFFECT_SMOKELINE            46
#define EFFECT_SHELLSMOKE           47
#define EFFECT_CATERPILLERSMOKE     48
#define EFFECT_CROWEYE              49
#define EFFECT_OCEROTTEBULLETFIRE   50
#define EFFECT_OCEROTTEBULLETSMOKE  51
#define EFFECT_ENVIRONMENTLIGHT     52
#define EFFECT_CRASHSMOKE           53
#define EFFECT_PADVIBRATION         54
#define EFFECT_CELOFAN              55
#define EFFECT_INVERSLIGHT          56
#define EFFECT_SPACK                57
#define EFFECT_STEAMEXPLOSION       58
#define EFFECT_INVERSLIGHT2         59
#define EFFECT_CELOFAN2             60
#define EFFECT_NINJALASER           61
#define EFFECT_GUSEFFECT            62
#define EFFECT_SUBMARINEWATER       63
#define EFFECT_PADVIBRATION2        64
#define EFFECT_GHOST                65
#define EFFECT_LIGHT                66
#define EFFECT_FAMASLIGHT           67
#define EFFECT_HUMANSHADOW          68
#define EFFECT_BLOODHAZARD          69
#define EFFECT_BLOODDRIP            70
#define EFFECT_NINJAGROUND          71
#define EFFECT_BOMBLIGHT            72
#define EFFECT_MGCROOMDISPLAY       73
#define EFFECT_TEXT2                74

/* デバッグモード用の define */
#ifdef DEMO_DEBUG_MODE
#define MODE_COUNT        2
#define VIEW_COUNT        3
#define TARGET_VIEW_COUNT 2
#define REPEAT_COUNT      6
#define MOVE_COUNT        20.0F
#define ROTATE_COUNT      12
#define ZOOM_COUNT        32

enum{
	DEMO_DEBUG_VIEW = 1,
	DEMO_DEBUG_CAMERA,		// カメラ操作モード
};

enum{
	DEMO_DEBUG_PARAM_FRAME = 1,
	DEMO_DEBUG_PARAM_STRING,
	DEMO_DEBUG_PARAM_POSITION,
};

#endif

typedef struct  {
	long	m[3][3];	/* 3x3 rotation matrix */
        long    t[3];		/* transfer vector */
} MATRIX32;

typedef struct {		/* short word type 3D vector */	
	short	vx, vy;
	short	vz;
} SVECTOREX;

typedef struct {		/* float type 3D vector */	
	float	vx, vy;
	float	vz;
} FVECTOREX;

typedef struct tagMGSDEMOSTAGE	/* ステージ */
{
	int nCacheID;	/* モデルキャッシュ ID */
	int nStrCode;	/* 文字列コード */
} MGSDEMOSTAGE, *PMGSDEMOSTAGE, *LPMGSDEMOSTAGE;

#define MGS_SCENETOSTAGE 0x00000001

typedef struct tagMGSDEMOSCENE	/* シーン */
{
	int   nID;		/* ID */
	DWORD dwStyle;	/* スタイル */
	int   nCacheID;	/* モデルキャッシュ ID */
	int   nStrCode;	/* 文字列コード */
	int   nName;	/* 名称 */
} MGSDEMOSCENE, *PMGSDEMOSCENE, *LPMGSDEMOSCENE;

typedef struct tagMGSDEMO	/* デモヘッダ */
{
	int            nSize;			/* Sync 読み込みサイズ */
	int            nTime;			/* Sync 時間(ダミー) */
	int            nFrameCount;		/* 再生するフレーム数 */
	int            nStageCount;		/* ステージ数 */
	int            nSceneCount;		/* シーン数 */
	LPMGSDEMOSTAGE lpStage;			/* ステージ */
	LPMGSDEMOSCENE lpScene;			/* シーン */
} MGSDEMO, *PMGSDEMO, *LPMGSDEMO;

typedef struct tagMGSDEMOEFFECT		/* 効果 */
{
//	int       nID;			/* 動作監視 ID */
//	int       nEffectID;	/* 効果 ID */
	BYTE      byBuffer[128];	/* 効果パラメータ */
} MGSDEMOEFFECT, *PMGSDEMOEFFECT, *LPMGSDEMOEFFECT;

/* scene */
typedef struct tagMGSDEMOSCENEMOTION	/* シーンモーション */
{
	int        nSceneID;		/* 対応するシーンの ID */
	short      bPreview;		/* プレビュー */
	FVECTOREX  fvectRotate;		/* モデルの回転 */
	FVECTOREX  fvectMove;		/* モデルの移動 */
/* 警告：モデルが表示されないとき、又は回転間接数が DG_MAX_JOINTS を越えて */
/*		 いるときオブジェクトの回転配列数は 0 になる */
	short      nRotateCount;	/* オブジェクトの回転配列数 */
	FVECTOR*   lpfvectRotate;	/* オブジェクトの回転配列 */
} MGSDEMOSCENEMOTION, *PMGSDEMOSCENEMOTION, *LPMGSDEMOSCENEMOTION;

typedef struct tagMGSDEMOFRAME	/* フレーム */
{
	int                  nSize;				/* Sync 読み込みサイズ */
	int                  nTime;				/* Sync 時間 */
	FVECTOREX            fvectCamera;		/* 視点座標 */
	FVECTOREX            fvectTarget;		/* 注視点 */
	short                nZRotate;			/* Z 回転角度 */
	unsigned short       nScreen;			/* 視点－投影面間距離 */
	short                nEffectCount;		/* 効果数 */
	short                nSMotionCount;		/* シーンモーション数 */
	LPMGSDEMOEFFECT      lpEffect;			/* 効果 */
	LPMGSDEMOSCENEMOTION lpSMotion;			/* シーンモーション */
} MGSDEMOFRAME, *PMGSDEMOFRAME, *LPMGSDEMOFRAME;

typedef struct tagMGSDEMOMODEL	/* デモモデル */
{
	CONTROL    control;		/* コントロール */
	OBJECT     object;		/* オブジェクト(KMS) */
	DG_EVMOBJ* lpevm_object;	/* オブジェクト(EVM) */
	FMATRIX    light[2];		/* ライト */
	void*      lpData;		/* 追加情報 */
} MGSDEMOMODEL, *PMGSDEMOMODEL, *LPMGSDEMOMODEL;

typedef struct tagMGSDEMOACT	/* デモプロセス */
{
	GV_ACT_EX           actor;				/* アクター */
	DWORD               dwStyle;			/* スタイル */
	int                 nVSync;				/* 開始時の VSync */
	int                 nMap;				/* マップ */
	int                 nTime;				/* デモが実行し始めてからの時間 */
	LPMGSDEMO           lpDemo;				/* デモヘッダ */
	LPMGSDEMOMODEL      lpModel;			/* 各シーンのモデル */
	void*               lpEffectPrevious;	/* effect process chain previous */
	void*               lpEffectNext;		/* effect process chain next */
#if defined(_DEMOMEMORYVIEW) || defined(_DEMOMEMORYOVERFLOWSTOP)
	int                 nMaxMemory[MAX_MEMSYS];		/* メモリ最大使用量 */
#endif
#ifdef _DEMODEBUG
	char                szPath[128];		/* デモファイルパス */
#endif
#ifdef _DEMOWRITEVRAM
	char                szAVIDir[128];		/* AVIファイル出力先ディレクトリ */
#endif
	LPVOID              lpExtMemory;		/* 開発機専用メモリのアクセス位置(デモファイルのときのみ) */
	MGSDEMOMODEL        modelEffect;		/* 効果用ダミーモデル */
	BOOL                bEffectStop;		/* 効果用停止情報 */
	BOOL                bEffectBubbleStop;	/* 効果（泡）停止情報 */
	void*               lpfnDGScreenChanl;	/* スクリーンチャネル関数の保存値 */
	int                 nGameStatus;		/* ゲームステータスの保存値 */
	GM_CameraSet*       lpGMCamera;			/* デモカメラの構造体 */
	int                 nItemSave;			/* アイテムの保存値 */
	int                 nWeaponSave;		/* 武器の保存値 */
	int                 nDemoID;			/* デモＩＤ（今だけ。将来的には使用しない */
#ifdef DEMO_DEBUG_MODE
	LPVOID              lpExtMemoryBackup;	/* ループ用のバックアップ */
	void*               lpBottomPointer;	/* デモが使用している拡張メモリのボトムポインタ */
	int                 nLoop;				/* 繰り返し再生（今だけ。将来的には使用しない */
	char                szDrawString[1024];	/* デモ中に表示する文字列 */
	int                 nDebugMode;			/* デバッグモード（今だけ。将来的には使用しない */
											/* 1 : スロー再生。逆転再生モード。 */
											/* 2 : カメラ移動モード */
	int                 nRepeatCount;		/* スローボタンのリピートカウント（今だけ。将来的には使用しない */
	int                 nViewParam;			/* パラメータを非表示にするか */
	int                 nMoveSpeedLevel;	/* カメラの移動量 */
	int                 nRotateSpeedLevel;	/* カメラの回転量 */
#endif
} MGSDEMOACT, *PMGSDEMOACT, *LPMGSDEMOACT;

typedef struct tagMGSDEMOEFFECTACT	/* 効果プロセス */
{
	void*         lpPrevious;	/* chain previous */
	void*         lpNext;		/* chain next */
	BOOL          bCheck;		/* 効果実行監視確認識別 */
	void*         lpActor;		/* 自己破棄できない効果プロセス */
	void*         lpActor2;		/* 自己破棄できない効果プロセス2 */
	MGSDEMOEFFECT effect;		/* 効果内容 */
	int           nParam1;		/* 補助 int 情報 */
	int           nParam2;		/* 補助 int 情報 */
	SVECTOR       svect1;		/* 補助 SVECTOR 情報 */
/*	MATRIX        matrix;*/		/* 補助 matrix 情報 */
	FMATRIX       matrix;		/* 補助 matrix 情報 */
} MGSDEMOEFFECTACT, *PMGSDEMOEFFECTACT, *LPMGSDEMOEFFECTACT;

/******************************************************************************
 * METALGEAR^3
 */

#define	FREE_UNIT	((void **)0)
#define	VOID_UNIT	((void **)1)
#define	USED_UNIT	((void **)2)
#define	MEMORY_DYNAMIC	(1)
#define	MEMORY_VOIDED	(2)
#define	MEMORY_FAILED	(4)

typedef	struct  {
	void		*addr ;
	void		**addr_ptr ;
} M_Unit ;

typedef	struct  {
	int		stat ;
	void		*start ;
	void		*end ;
	int		n_units ;
/*	M_Unit		units[ MAX_MEMUNIT ] ; */
	M_Unit		units[ 256 ];
} M_Sys ;

extern M_Sys MemorySystems[MAX_MEMSYS];
extern long demodebug_finish_proc;

extern void MakeFullPath(char *name, char *full);
extern void DG_ScreenChanlEx(DG_CHANL *cp, int which);

/******************************************************************************
 * exec
 */

extern BOOL CreateDemo(LPMGSDEMOACT lpAct, LPMGSDEMO lpDemo);
extern BOOL DestroyDemo(LPMGSDEMOACT lpAct);
extern BOOL FrameRunDemo(LPMGSDEMOACT lpAct, LPMGSDEMOFRAME lpDFrame);

/* libdg\chanl.c */
extern void *DG_SetChanlSystemUnits(int num, void *addr);

/* common.c  */
extern void InitChain(void* lpAddress);
extern void NextChain(void* lpManeger, void* lpChain);
extern void DeleteChain(void* lpManeger, void* lpDelete);

#if 0
//typedef struct { float vx, vy, vz, vw ; } FVECTOR;
typedef void* (*CHARAVOID)(void*);
typedef void* (*CHARAMODEL)(void*, LPMGSDEMOMODEL lpModel);
typedef void* (*CHARAFVECT)(FVECTOR* lpfvecCenter);
typedef void* (*CHARAFMATFVECT)(FMATRIX* lpfmatWorld, FVECTOR* lpfvecCenter);
typedef void* (*CHARAFMAT)(FMATRIX* lpfmatWorld);
typedef void* (*CHARASPARK)(int nPrim, FVECTOR* lpfvectCenter, float fMinSpeed, float fWideSpeed, float fGravity, SVECTOR* lpsvectRot, SVECTOR* lpsvectRotWide, FVECTOR* lpfvectColor, float fLength, int nCount);
typedef void (*CHARACAMERABLOOD)(FVECTOR* lpfvectPos, int nWhite);
typedef void* (*CHARABLOODSPREAD)(FVECTOR* lpfvectPos, SVECTOR* lpsvectRot, int nDecay, float fSize);
typedef void (*CHARAWALLBLOOD)(FVECTOR* lpfvectPos, SVECTOR* lpsvectRot, int nWhite);
typedef void* (*CHARACIRCLELIGHT)(FMATRIX* lpfmatWorld, int* lpnFlag, int nMode);
typedef void* (*CHARAFADEINOUTDEMO)(int nColorR, int nColorG, int nColorB, int nColorA, int nCount);
typedef void* (*CHARADLOODDROPS)(FVECTOR *pos, int decay, int white);
typedef void* (*CHARACTFLUSH)(int nDecay, int nMode);
typedef void* (*CHARARAINCAMERADEMO)(int nLife);

typedef void* (*CHARASPLASHDEMO)(FVECTOR *origin, FVECTOR *center, float radius, float size, int multiple, int life);
typedef void* (*CHARASPLASHPARTSDEMO)(FVECTOR *center, SVECTOR *rot, float intense);
typedef void* (*CHARASPLASHRIPPLEDEMO)(FVECTOR *center, float radius, float direction, float angle, float size, int multiple, int life);

typedef void* (*CHARASPRITEFOGWORLDDEMO)(FVECTOR *bound0, FVECTOR *bound1, FVECTOR *vec, FVECTOR *col, int tex_name, int size, int life, int mode);
typedef void* (*CHARASPRITEFOGPERSDEMO)(FVECTOR *bound0, FVECTOR *bound1, FVECTOR *vec, FVECTOR *col, int tex_name, int size, int life, int mode);
typedef void* (*CHARASPRITEFOGONCAMERADEMO)(FVECTOR *vec, FVECTOR *col, int tex_name, int size, int life, int mode);

typedef void* (*CHARAOPTICALCAMOUFLAGEBREAKDEMO)(DG_OBJS *objs, int type, int start_time, int end_time, int color);
typedef void* (*CHARACONTRASTDEMO)(int col_u_r, int col_u_g, int col_u_b, int col_d_r, int col_d_g, int col_d_b, int nega_posi_flag, int time);
#endif

typedef struct {
	FVECTOR root;
	FVECTOR joints[64];
	FVECTOR quat_buffer[4];
	FVECTOR vec;
	FMATRIX mat;
	FMATRIX root_mat;
	FMATRIX mats[64];
	FMATRIX tmp[4];
} ScrPadWork;

/* KMS 用モーション計算スクラッチパッド */
typedef struct {
	FMATRIX j_root ;
	FMATRIX joints[DG_MAX_JOINTS];
	FMATRIX tmp[4];
} ScrPad;

#define	SCRPAD   ((ScrPad*)SCRPAD_ADDR)
#define	J_ROOT   (&(SCRPAD->j_root))
#define	JOINTS   (SCRPAD->joints)
#define	WORK_MAT (&SCRPAD->tmp[0])


/******************************************************************************
 */

#ifdef __cplusplus
}                       /* End of extern "C" { */
#endif  /* __cplusplus */

#endif /* _INC_DEMO_ */
