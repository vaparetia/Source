/* 
   レイブンのヘッダ
   2000/02/17 H.TANAKA
*/

/* ---------------------------------------------------------------- */
	/*
		補助マクロ
	*/

#define  FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

/* モデル用 */
#define   BODY_FLAG (DG_FLAG_SHADOWMAKE | DG_FLAG_TEXT | DG_FLAG_FINISHCALC | DG_FLAG_SHADE | DG_FLAG_ONEPIECE)
#define   FIGRAVEN_VITALITY     5

#define   STABLE_COUNT     (10)


typedef struct
{
    GV_ACT_EX     actor ;

	int			name;
	int			map;

 
    TARGET        target ;
    POWER_TARGET  power  ;
 
    DG_OBJS    *objs ;
    DG_MDL     *mdl ;         
    FMATRIX    light[2] ;      /* モデルの色 */
    FVECTOR    pos ;           /* 中心位置 */
    SVECTOR    rot ;           /* 回転 */

    FMATRIX    world ;         /* ワールド */
    int        hit_count ;
    int        next_available_count ;

    FVECTOR    move ;          /* 相対距離 */
    SVECTOR    rot_move ;      /* 相対回転 */


	FVECTOR    boundary_min;
	FVECTOR    boundary_max;


	OBJECT			object;
	CONTROL_NOEVT	control;

} Work ;

extern int   FigRaven_InitTarget(Work *work, FMATRIX *world) ;
extern int   FigRaven_InitRaven(Work *work) ;
extern int   FigRaven_GetOptionValue(Work *work) ;

extern void  FigRaven_TargetCallBack(TARGET *off, TARGET *def, void *ptr) ;
extern void  *New_BB_Dan( FMATRIX *world, int map ) ;
