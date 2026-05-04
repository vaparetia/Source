/*
	vr.h
	ＶＲ用いろいろ
	2002/02/12 K.Sigeno
	$Id: vr.h,v 1.1.1.3 2002/11/19 11:49:57 Yoshizawa1 Exp $
*/


#define MDL_VR_TRG_TRI_02		(1414837)	/*vr2_tgt_triangle.kms*/
#define MDL_VR_TRG_TRI_EXP_01	(8551643)	/*vr2_tgt_triangle_exp.kms*/
#define MDL_VR_TRG_TRI_TAB_01	(8566253)	/*vr2_tgt_triangle_tab.kms*/

#define MDL_VR_TRG_CUBE_02		(12998689)	/*vr2_tgt_cube.kms*/
#define MDL_VR_TRG_CUBE_EXP		(5081330)	/*vr2_tgt_cube_exp.kms*/
#define MDL_VR_TRG_CUBE_TAB		(5095940)	/*vr2_tgt_cube_tab.kms*/

#define MDL_VR_TRG_OCT_02		(13374045)	/*vr2_tgt_octa.kms*/
#define MDL_VR_TRG_OCT_TAB_01	(925096)	/*vr2_tgt_octa_tab.kms*/
#define MDL_VR_TRG_OCT_EXP_01	(910486)	/*vr2_tgt_octa_exp.kms*/

#define MDL_VR_TRG_STR_02		(13288496)	/*vr2_tgt_straw.kms*/
#define MDL_VR_TRG_STR_TAB_01	(4065477)	/*vr2_tgt_straw_tab.kms*/
#define MDL_VR_TRG_STR_EXP_01	(4050867)	/*vr2_tgt_straw_exp.kms*/

#define MDL_VR_TRG_SQU_01		(2754014)	/*vr2_tgt_square.kms*/
#define MDL_VR_TRG_SQU_EXP		(1295310)
#define MDL_VR_TRG_SQU_TAB		(1309920)

#define MDL_VR_TRG_HEX			(13146845)	/*vr2_tgt_hexa*/
#define MDL_VR_TRG_HEX_EXP		(896286)	/*vr2_tgt_hexa_exp*/
#define MDL_VR_TRG_HEX_TAB		(910896)	/*vr2_tgt_hexa_tab*/


#define MDL_VR_TRG_TRI_NOV		(8560577)	/*vr2_tgt_triangle_nov*/
#define MDL_VR_TRG_TRI_NOV_TAB	(4818558)	/*vr2_tgt_triangle_nov_tab*/

#define MDL_VR_TRG_WALL			(13633896)


#define MDL_VR_TRG_DARK_OCTA	(919420)	/* GV_StrCode("vr2_tgt_octa_nov" ) */


//VR機雷
#define MDL_VR_TRG_MINE			(16271087)	/*vr2_tgt_watermine.kms*/


#define MDL_VR_TRG_CUB_RGB		(5094084)
#define MDL_VR_TRG_SQU_RGB		(1308064)
#define MDL_VR_TRG_TRI_RGB		(8564397)
#define MDL_VR_TRG_OCT_RGB		(923240)
#define MDL_VR_TRG_STR_RGB		(4063621)
#define MDL_VR_TRG_HEX_RGB		(909040)
#define MDL_VR_TRG_WALL_RGB		(12473816)

//#define MDL_VR_TRG_OCT_ALP		(11662166)
//#define MDL_VR_TRG_OCT_SOL		(924530)
//#define MDL_VR_TRG_OCT_HTL		(913426)
#define MDL_VR_TRG_OCT_HLT		(913178)	//オクタ
#define MDL_VR_TRG_CUBE_HLT		(5084022)	//キューブ
#define MDL_VR_TRG_SQU_HLT		(1298002)	//スクゥエア
#define MDL_VR_TRG_STR_HLT		(4053559)	//ワラ
#define MDL_VR_TRG_TRI_HLT		(8554335)	//三角
//#define MDL_VR_TRG_HEX_HLT		(898978)	//UFO

#define	MDL_VR_GOAL				(10247211)

/*部位ダメージデフォルト値*/
#define	VR_TRG_DEF_DMG_01	10 ;
#define	VR_TRG_DEF_DMG_02	5 ;
#define	VR_TRG_DEF_DMG_03	1 ;

#define BLAST_DELAY DIRECT_TICK(6)	/*爆発遅延*/


#define	NAME_TAG_OCT_DEF	(4376380)	/*nametag_octa_def_add_alp*/
#define	NAME_TAG_CUB_DEF	(10421984)	/*nametag_cube_def_add_alp.bmp */
#define	NAME_TAG_SQU_DEF	(3257670)	/*nametag_square_def_add_alp.bmp */
#define	NAME_TAG_STR_DEF	(1060028)	/*nametag_straw_def_add_alp.bmp */
#define	NAME_TAG_HEX_DEF	(13289220)	/*nametag_hexa_def_add_alp.bmp */
#define	NAME_TAG_WAL_DEF	(11761531)	/*nametag_wall_def_add_alp.bmp */
#define	NAME_TAG_TRI_DEF	(14667482)	/*nametag_triangle_def_add_alp*/

#define	NAME_TAG_GBS_DEF	(16631547)	/*nametag_gbs1_def_add_alp*/


#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )


#define	TEX_VR_NOISE_A0	(11632395)	/*vr2_noise_a0_alp.bmp*/
#define	TEX_VR_NOISE_A1	(12680971)	/*vr2_noise_a1_alp.bmp*/
#define	TEX_VR_NOISE_A2	(13729547)	/*vr2_noise_a2_alp.bmp*/
#define	TEX_VR_NOISE_B0	(11632397)	/*vr2_noise_b0_alp.bmp*/
#define	TEX_VR_NOISE_B1	(12680973)	/*vr2_noise_b1_alp.bmp*/
#define	TEX_VR_NOISE_B2	(13729549)	/*vr2_noise_b2_alp.bmp*/
#define	TEX_VR_NOISE_C0	(11632399)
#define	TEX_VR_NOISE_C1	(12680975)
#define	TEX_VR_NOISE_C2	(13729551)


#define	VR_TRG_FLS_CNT		DIRECT_TICK(8)
//#define	VR_TRG_FLS_CNT_KATANA		DIRECT_TICK(16)
#define	VR_TRG_FLS_CNT_KATANA		DIRECT_TICK(32)

enum {
	TRG_ST_NORMAL		=	0x0000,
	TRG_ST_WAIT			=	0x0001,
	TRG_ST_DESTROY		=	0x0002 ,
	TRG_ST_INVISIBLE	=	0x0004 ,
	TRG_ST_DISAPPEAR	=	0x0008 ,
	TRG_ST_FAINTBLADE	=	0x0010 ,	//峰撃ちでとどめ
	TRG_ST_HIDE			=	0x0020 ,	//隠れよう

};
enum {
	TRG_MOVE_MODE,TRG_ACTION_MODE,TRG_BOUND_MODE
};

#define VR_TRG_LIFE_MAX (255);

#define		HIT_PRINT_SHIFT		(20)
#define		COMBO_PRINT_SHIFT	(40)



#define		COMBO_DEFAULT_LIMIT	(16)

extern int	VR_SCORE ;		/*得点*/
extern int	VR_GOAL_FLAG ;
extern int	VR_TIME ;		/*時間*/
extern int	VR_TARGET_NUM ;	//標的の最大数
extern int	VR_TARGET_MAX ;	//現在の標的数

extern int VR_BOMBS_NUM ;	//C4解体 分子
extern int VR_BOMBS_MAX ;	//C4解体 分母

extern int VR_RetryCount ;		//そのステージをリトライした回数
extern int VR_ContinueCount ;	//そのステージをコンティニューした回数

extern int VR_TARGET01_LIFE_MAX ;
extern int VR_WALL_LIFE_MAX ;
extern int VR_CUBE_LIFE_MAX ;
extern float VR_TRG_SPEED_RATE ;

extern int VR_COMBO_LIMIT ;
extern int VR_COMBO_CNT  ;
extern int VR_COMBO_RATE  ;

extern void *VR_TRG_COMB_DISP ;

extern int VR_COMBO_CHAIN ;
extern int VR_COMBO_CHAIN_MAX ;

extern float VR_TARGET01_SCALE ;

extern int VR_BLAST_RANGE ;

extern int VR_STAGE_ID	;


#if 0
/*敵兵リンク*/
extern int	VR_ENEMY_NUM ;	//倒すべき敵兵 残り
extern int	VR_ENEMY_MAX ;	//倒すべき敵兵 全体数
extern int	VR_DiscoverCount ;
extern int	VR_KillCount ;
#endif

enum {
	VR_TRG_TYPE_TRI = 0 ,	/*三角*/
	VR_TRG_TYPE_OCT,		/*8面体的*/
	VR_TRG_TYPE_KATANA,		/*カタナ*/
	VR_TRG_TYPE_CROSS,		/*十字*/
	VR_TRG_TYPE_HEX,		/*スティンガー用大型*/
	VR_TRG_TYPE_FIX_CUBE,	/*固定箱*/
	VR_TRG_TYPE_MOVE_CUBE,	/*移動箱*/
	VR_TRG_TYPE_ONE_TRI,	/*三角部位なし*/
	VR_TRG_TYPE_WALL,		/*壁*/
	VR_TRG_TYPE_PUNCH,		/*素手攻撃*/
	VR_TRG_TYPE_ENEMY,		/*敵兵*/
	VR_TRG_TYPE_DARK,		/*ダークステージ専用*/
	VR_TRG_TYPE_MAX,
};

extern int VR_TRG_STRENGTH_01[VR_TRG_TYPE_MAX] ;
extern int VR_TRG_STRENGTH_02[VR_TRG_TYPE_MAX] ;
extern int VR_TRG_STRENGTH_03[VR_TRG_TYPE_MAX] ;

extern int VR_TRG_SCORE_01[VR_TRG_TYPE_MAX] ;
extern int VR_TRG_SCORE_02[VR_TRG_TYPE_MAX] ;
extern int VR_TRG_SCORE_03[VR_TRG_TYPE_MAX] ;
extern int VR_TRG_SCORE_NG ;

extern int	VR_COMBO_SC_LIST[COMBO_DEFAULT_LIMIT] ;
extern int	VR_COMBO_LIST_MAX ;
extern int	VR_TARGET_HIDE_CYCLE ;
extern int	VR_TARGET_APPEAR_CYCLE ;

extern void VR_SetScore(int);
extern void VR_AddScore(int);
extern void VR_AddTarget(void) ;

extern void VR_SetComboLimit(void) ;
//extern void VR_SetComboCnt(int)  ;
extern void VR_RestComboCnt(void) ;
extern void VR_ExecProcName(int ,int ) ;
extern void VR_ClearTarget(void) ;

//extern void VR_CheckHomingStatus(HOMING_TRG * ,int * ) ;


//extern void *NewSigBreakObj(int ,int ,FMATRIX *,float,float ,CVECTOR *,SVECTOR *,int ) ;
extern void *NewSIG_3DPOSPrint(FMATRIX * ,int ,int *,float) ;
extern int	VR_TypeFlagToNum(int) ;
extern u_char VR_GetPartsDmg(int,int) ;



enum {
	VR_GOAL_OPEN	= 0x01,
	VR_GOAL_IN		= 0x02,
};
enum {
	VR_TARGET_TYPE_NORMAL	= 0x00000000,
	VR_TARGET_TYPE_PUNCH	= 0x00000001,
	VR_TARGET_TYPE_NG		= 0x00000002,
	VR_TARGET_TYPE_BLAST	= 0x00000004,
	VR_TARGET_TYPE_LERP		= 0x00000008,
	VR_TARGET_TYPE_CIRCLE	= 0x00000010,
	VR_TARGET_TYPE_ABSSHIFT	= 0x00000020,
	VR_TARGET_TYPE_WAIT		= 0x00000040,
	VR_TARGET_TYPE_OCT		= 0x00000080,
	VR_TARGET_TYPE_DIR		= 0x00000100,
	VR_TARGET_TYPE_P_DIR	= 0x00000200,
	VR_TARGET_TYPE_KATANA	= 0x00000400,
	VR_TARGET_TYPE_CROSS	= 0x00000800,
	VR_TARGET_TYPE_MINE		= 0x00001000,
	VR_TARGET_TYPE_HEX		= 0x00002000,
	VR_TARGET_TYPE_ONE		= 0x00004000,	/*箱部位なし*/
	VR_TARGET_TYPE_ONE_TRI	= 0x00008000,	/*三角部位なし*/
	VR_TARGET_TYPE_NO_GHOST	= 0x00010000,	/*破壊後は座標更新しない 子的の制御用*/
	VR_TARGET_NO_APPEAR_EF	= 0x00020000,	/*出現時のエフェクトなし*/
	VR_TARGET_STEALTH		= 0x00040000,	/*ステルス迷彩*/
	VR_TARGET_X_ROT			= 0x00080000,	/*X軸90度傾き カタナ用*/
	VR_TARGET_FL_MARK		= 0x00100000,	/*床マーカーあり*/
	VR_TARGET_NO_SIGHT		= 0x00200000,	/*ロックオンカーソル無し*/
	VR_TARGET_DEBUG			= 0x00400000,	/*ターゲットサイズ表示*/
	VR_TARGET_HIDESTART		= 0x00800000,	/*隠れ周期有効 隠れ状態で開始*/
	VR_TARGET_VISIBLESTART	= 0x01000000,	/*隠れ周期有効 可視状態で開始*/
	VR_TARGET_FLY_PROC_CALL	= 0x02000000,	/*刀的 吹っ飛び時に壊れプロックを実行*/
	VR_TARGET_TYPE_DARK_OCTA	= 0x04000000,	/*ダークステージ専用*/
	VR_TARGET_FLY_RAISE		= 0x08000000,	/*バグ回避 吹っ飛び時に1mm浮かす*/
};

enum {
	VR_TRG_LEVEL1 = 0,
	VR_TRG_LEVEL2 ,
	VR_TRG_LEVEL3 ,
	VR_TRG_NG ,
	VR_TRG_WALL ,
	VR_TRG_KATANA ,
	VR_TRG_BLAST ,
} ;


/*共通メッセージ番号*/
enum {
	VR_TRG_MSG_WAKE = 64,
	VR_TRG_MSG_DISAPPEAR,
};
/*model code*/
//#define MDL_VR_TRG_TRI_01		(6034655)	/*vr2_tgt_triangle_mt.kms*/
//#define MDL_VR_TRG_CUBE_01		(1207616)	/*vr2_tgt_cube_mt.kms*/
//#define MDL_VR_TRG_OCT_01		(3174429)	/*vr2_tgt_octa_mt.kms*/
//#define MDL_VR_TRG_STR_01		(1699702)	/*vr2_tgt_straw_mt.kms*/

