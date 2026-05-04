/* 
   懐中電灯のヘッダ 
   2000/01/21 H.TANAKA
*/

/* ---------------------------------------------------------------- */
	/*
		補助マクロ
	*/

#define  FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

/* モデル用 */
#define   BODY_FLAG	 (DG_FLAG_TEXT | DG_FLAG_FINISHCALC | DG_FLAG_SHADE | DG_FLAG_ONEPIECE)

/* ライト用 */
#define	 MAIN_PRIM_TYPE	 (DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA)
#define	 N_MAIN_PRIMS	   1
#define	 N_MAIN_VERTS	   8
#define   MAX_BREAK_COUNT		40


/* ハレーション用 */
#define	 HL_PRIM_TYPE	   ( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_ON_CAMERA)
#define	 N_HL_PRIMS		  1
#define	 N_HL_VERTS		  4
#define	  SHIFT_X			0.0f
#define	  SHIFT_Y			100.0f
#define	  SHIFT_Z			(-150.0f)
#define	  LIMIT_RANGE			10000.0f
#define	  LIMIT_ANGLE			0.5f
#define	  SCREEN_NEAR			51.0f
#define	  ALPHA_MAX_SC			255.0f
#define	  SIZE_MAX_SC			50.0f

/* ガラス壊れのエフェクト */
#define   N_GLASS_PRIMS		 1
#define   N_GLASS_VERTS		 30		/* 3 * 10 */
#define   N_TRIANGL_VERTS	   3
#define   MAX_GLS_COUNT		 100
#define   N_GLASS_NUM		   ( N_GLASS_PRIMS * N_GLASS_VERTS / N_TRIANGL_VERTS )
#define   GLASS_PRIM_TYPE	   (DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA)
 
typedef	struct _work   Work ;

typedef  struct 
{
	DG_PRIM2	 *prim ;
	FVECTOR	  *base ;
	FVECTOR	  *speed ;
	SVECTOR	  *rot ;
	SVECTOR	  *const_rot ;
	int		  glass_count ;
	int		  (* act)(Work *work) ;
} GLASS  ;

typedef  struct
{
	DG_PRIM2	 *prim ;			/* ランプ */
	DG_PRIM2	 *prim_hl_sub ;	/* ハレーション1 */
	DG_PRIM2	 *prim_hl ;		/* ハレーション2 */
	DG_PRIM2	 *prim_hl_add ;	/* ハレーション3 */

	int		  break_count ;	
	int		  fire_count ;
	int		  (*act)(Work  *work) ;
} FLASHLIGHT ;

struct  _work
{
	GV_ACT_EX		  actor ;

	int				map;

	int				target_flag;
	TARGET		  target ;
	POWER_TARGET	power  ;

	DG_OBJS	  *objs ;
	DG_OBJS	  *brk_objs ;
	DG_DEF	   *brk_def ;
	FMATRIX	  light[2] ;	  /* モデルの色 */
	FVECTOR	  pos ;		   /* 中心位置 */
	SVECTOR	  rot ;		   /* 回転 */
	int		  id ;			/* proc番号 */
	int		  brk_flag;
	/* ランプ系(ハレーション付き) */
	FLASHLIGHT   flashlight ;
	/* 破片 */
	GLASS		*glass ;
} ;


extern void	FL_LIGHT_SetSound(int se, FVECTOR *pos, int mode) ;
extern FVECTOR FL_LIGHT_Triangle[N_TRIANGL_VERTS] ;
/* fl_light_ini.c */
extern int  FL_LIGHT_GetOptionValue(Work *work) ;
extern int  FL_LIGHT_InitModel(Work *work) ;
extern int  FL_LIGHT_InitLight(Work *work, int map) ;

extern int  FL_LIGHT_InitGlass(Work *work) ;
extern void  FL_LIGHT_FreeGlass(Work *work) ;

/* fl_light_target.c */
extern void  FL_LIGHT_TargetCallBack(TARGET * off, TARGET *def, void *ptr) ;

/* fl_light_act.c */
extern  int   FL_LIGHT_NormalAct(Work *work) ;   
extern  int   FL_LIGHT_Break_Act(Work *work) ;
extern  int   FL_LIGHT_Disapp_Act(Work *work) ;
extern  int   FL_LIGHT_None_Act(Work *work) ;
 
extern  int   FL_LIGHT_Gls_Act(Work *work) ;  
/* other */
extern  void Big_TmpLight2(FVECTOR *pos, float r_range, float e_range, int color, int flag) ;

extern  void *NewSpark1(int n_prims, FVECTOR *center, float min_speed, 
float		speed_wide, float gravity, SVECTOR *rot, SVECTOR *rot_wide, 
FVECTOR *color, float length, int count ) ;
