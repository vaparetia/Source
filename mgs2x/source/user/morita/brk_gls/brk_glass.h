/*
        brk_glass.h
	ガラス破壊用 定義ファイル

        1999/11/26 T.Morita
        $Id: brk_glass.h,v 1.1.1.3 2002/11/19 11:45:29 Yoshizawa1 Exp $
*/

// for box hzd
#include  "../brk_hzd/brk_hazard.h"
// for DG-LIB
#include "../include/libdg_x.h"
#include "../include/util.h"

#include  "../brk_utl/brk_utl.x"

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

#define BRK_PIECE_GRAVITY 4
#define BRK_PVEL_R    0.88f 
#define BRK_VEL_R 16
#define BRK_ROT_R 16

#define BRK_N_LPIECE  50
#define BRK_N_SPIECE (BRK_N_LPIECE*3)
#define BRK_N_PIECE  (BRK_N_LPIECE + BRK_N_SPIECE)

#define BRK_GLS_NBEND   7
#define BRK_GLS_NVERTS (BRK_GLS_NBEND*2 + 1)
#define BRK_GLS_INACTIVE 0x8000000



typedef struct {
	DG_DMATAG			dmatag ;
	struct _gif_packet {
		DG_GIFTAG		giftag ;
		struct _gif_data{
			DG_GSREG	alpha ;
			DG_GSREG	clamp ;
			DG_GSREG	tex0 ;
			DG_GSREG	test1 ;
			DG_GSREG	prim ;
			DG_GSREG	rgbq ;
			DG_GSREG	uv0 ;
			DG_GSREG	xyz0 ;
			DG_GSREG	uv1 ;
			DG_GSREG	xyz1 ;
			DG_GSREG	test2 ;
		} gif_data ;
	} gif_packet ;
} SCREEN_DRAW ;


/* ポリゴン情報 */
typedef struct poly_t POLY ;
struct poly_t
{
    short    flag ;           /* このポリゴンがどうなっているか（正常:0 割れ:1 消え:2） */
    short    i0,  i1,  i2 ;   /* 共有頂点インデックス（現在事実上使っていない） */
    FVECTOR *v0, *v1, *v2 ;   /* 共有頂点 */

    FVECTOR        *b_gls_p ; /* このポリゴンに関して割り当てられているPRIMの情報 */
    DG_PRIM2_UVRGB *b_gls_u ;
    FVECTOR        *f_gls_p ;
    DG_PRIM2_UVRGB *f_gls_u ;
    FVECTOR        *thick_p ;
    DG_PRIM2_UVRGB *thick_u ;

    POLY    *p0, *p1, *p2 ;/* 辺 共有情報 p0:v0-v1 p1:v1-v2 p2:v2-v0の辺につながっている */
} ;

typedef struct
{
    DG_COMDL_POS *comdl ;

    int      flag   ;
    short    rot_x,  rot_y  ;
    short    rot_vx, rot_vy ;
    FVECTOR  pos    ; /* 位置     */
    FVECTOR  pos_v  ; /* 速度     */
} PIECE ;

typedef struct work_t
{
    GV_ACT       actor   ;

    HZD_BOX     *hzd     ;
    int          where   ;
    int          name    ;
    int          proc    ;
    float        thickness ;

    TARGET      *offense ;
    TARGET       target  ;
    POWER_TARGET power   ;
    ENEFIND      ene_find ; /*  */

    FMATRIX      world   ;
    DG_PRIM2    *glass   ;
    DG_PRIM2    *broken  ;
    DG_PRIM2    *thick   ;
    DG_PRIM2    *mirror  ;
    DG_PRIM2    *mirror_bg ;
    DG_PRIM2    *mirror_br ;
    DG_PRIM2    *mirror_th ;
    void        *mirror_brpos ;
    void        *mirror_bruvs ;
    void        *mirror_thpos ;
    void        *mirror_thuvs ;

    POLY        *poly    ;
    short          n_poly  ;
    short          flag    ;
    DG_DEF      *def     ;
    CV2_DEF     *cdef    ; 

    void        *fog_mist ;

    int          model_nm  ;
    int          lpiece_nm ;
    int          spiece_nm ;
    DG_TEX      *glass_tx  ;
    DG_TEX      *broken_tx ;
    DG_TEX      *mirror_tx ;

    DG_DMAPACK	*dmapack  ;
    void	*packet   ;
} Work ;

#define FTOI12(_f)      ( DG_FTOI( ( (float)(_f)*4096.0f) ) )

extern void AN_Test_Eye2( FVECTOR *mov, int size ) ; /* for test */


extern void BRK_GLS_TargetCallBack( TARGET *off, TARGET *def, void *ptr ) ;
extern int  BRK_GLS_BreakAtRandom( Work *work, int se_id ) ;

extern int  BRK_GLS_InitTarget( Work *work, int where ) ;
extern int  BRK_GLS_InitHazard( Work *, int where ) ;
extern int  BRK_GLS_InitPolygons( Work *work ) ;
extern int  BRK_GLS_InitPieces( Work *work, int where ) ;
extern void BRK_GLS_InitEneFind( Work *work ) ;
extern void BRK_GLS_InitBroken( Work *work ) ;

extern int  BRK_GLS_FogMist( Work *work, int where ) ;
extern int  BRK_GLS_GetOptions( Work *work, int where ) ;

extern void BRK_GLS_ActPieces( Work *work ) ;
extern void BRK_GLS_ActGlass( Work *work ) ;
extern void BRK_GLS_StartActPieces( Work *work, 
				    FVECTOR *pos, FVECTOR *frc,
				    int i ) ;

extern void BRK_GLS_ReceiveMessage( Work *work ) ;

#ifdef KP_XBOX
extern void BRK_GLS_ActFlipTheFace( Work *work ) ;
#endif

static inline void RotateMatrixXY( FMATRIX *out, short rot_x, short rot_y )
{
    int r ;

    r = rot_x & 0x0fff ;
    _sceVu0RotMatrixX( out, &DG_UnitMatrix, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot_y & 0x0fff ;
    _sceVu0RotMatrixY( out, out           , (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
}

static inline float BackFaceCullingXY( FVECTOR *v1, FVECTOR *v2, FVECTOR *v3 )
{
    return (v1->vx - v2->vx)*(v3->vy - v2->vy) - (v1->vy - v2->vy)*(v3->vx - v2->vx) ;
}

static inline float BackFaceCulling( FVECTOR *v1, FVECTOR *v2, FVECTOR *v3 )
{
    FVECTOR a ;

    _sceVu0OuterProduct( &a, v1, v2 ) ;
    return _sceVu0InnerProduct( &a, v3 ) ;
}
