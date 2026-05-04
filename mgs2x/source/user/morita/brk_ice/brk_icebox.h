/*
        brk_object.h
	ガラス破壊用 定義ファイル

        1999/11/26 T.Morita
        $Id: brk_icebox.h,v 1.1.1.3 2002/11/19 11:45:33 Yoshizawa1 Exp $
*/

#define MAKING 0

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

//for DEBUG
#include "../include/util.h"

// for BOX hzd
#include "../brk_hzd/brk_hazard.h"
// for DG-LIB
#include "../include/libdg_x.h"
#include "../brk_utl/brk_utl.x"

#define BRK_DUST_GRAVITY 1
#define BRK_GRAVITY      6.0f
#define BRK_VEL_R        0.85f
#define BRK_ROT_R        16
#define BRK_BOUNCE       1.68f
#define BRK_BOUNCE_WALL  1.38f
#define BRK_ICEBOX_RAD   (12.0f*2.0f)
#define BRK_HZX_SPHERE   (BRK_ICEBOX_RAD/2.0f)

#define BRK_DUST_COMDL	(GV_StrCode("shl_frg1"))

#define BRK_ICE_INACTIVE 0x8000000

#define BRK_N_SCAR  10 
#define BRK_N_VERTS 4
#define BRK_N_DUST  18
#define BRK_N_SPRAY	(24)

#define  N_SP_PRIMS	(2)
#define  N_SP_VERTS	(48)

#define SPRAY_ACTIVE	(1)



enum 
{
    BRK_FLG_NONE     = 0,
    BRK_FLG_BOTHSIDE = 0x00000001,
} ;

typedef struct move_t   MOVE   ;
typedef struct ice_t    ICEBOX ;
typedef struct box_t    BOX    ;
typedef struct work_t   Work   ;

struct move_t
{
    FVECTOR  pos    ; /* 位置     */
    FVECTOR  pos_v  ; /* 速度     */
    short    rot_x,  rot_y  ;
    short    rot_vx, rot_vy ;
} ;

struct ice_t
{
    MOVE        mov  ;
    int         flag ;
    int         proc_id ; /*壊れプロック*/

    DG_OBJS    *objs ;/* 氷モデル */
    DG_COMDL   *frg_objs ;/* 破片氷モデル */
    Work       *work ;

//    DG_PRIM2   *prim ;/* 壊れた時の壊れのポインタ */

    FMATRIX      lights[2] ;

    TARGET       target ;
    POWER_TARGET power  ;
//add shibata
	FVECTOR		 ice_scl;

	FVECTOR		 dvec[BRK_N_DUST];
	SVECTOR		 rot[BRK_N_DUST];
	int			 sound_flag;
} ;

struct box_t
{
    MOVE      mov  ;
    DG_OBJS  *objs ;
    DG_PRIM2 *scar ;

    int       flag      ;
    short     ice_fount ;
    short     non_dmg   ;

    FMATRIX   lights[2] ;

    TARGET       target ;
    POWER_TARGET power  ;

    HZD_BOX     *hzd    ;
	int 		snd_flag ;
} ;

struct work_t
{
    GV_ACT    actor  ;

    int       name   ;
    int       where  ;
    int       proc_id ; /* 壊れプロック */

    DG_DEF   *m_def   ;  /* 本体モデル       */
    DG_DEF   *p_def   ;  /* 壊れパーツモデル */
    DG_DEF   *i_def   ;  /* 氷モデル */
    DG_DEF   *frg_i_def ;/* 氷破片モデル */

    ICEBOX   *ice    ;
    int       n_ice  ;

    BOX       box    ;
  //  DG_PRIM2 *shadow ; /* じゃがいも用 丸影 */
	
//    FVECTOR  sp_mpos[BRK_N_SPRAY];
    FVECTOR  sp_lpos[BRK_N_SPRAY*4];
    FVECTOR  sp_dvec[BRK_N_SPRAY*4];
	
    DG_PRIM2 *spray ; /* 水飛沫 */
	
	int		 sp_flag ;
	
    HZD_BOX   *hzd ;

} ;



extern void AN_Test_Eye2( FVECTOR *mov, int size ) ; /* for test */



/* brk_ice_act.c */
extern void BRK_ICE_FreeIce( Work *work ) ;

/* brk_ice_act.c */
extern void BRK_ICE_ActIce( Work *work ) ;
extern void BRK_ICE_ActBox( Work *work ) ;
extern int BRK_ICE_ActSpray( Work *work ) ;

/* brk_ice_clb.c */
extern void BRK_ICE_TargetCallBack( TARGET *off, TARGET *def, void *ptr ) ;
extern void BRK_ICE_BoxTargetCallBack( TARGET *off, TARGET *def, void *ptr ) ;

/* brk_ice_msg.c */
extern void BRK_ICE_ReceiveMessage( Work *work ) ;
extern void BRK_ICE_AlreadyMessy( Work *work ) ;

/* brk_ice_ini.c */
extern void BRK_ICE_MakeSizeAndCenter( FVECTOR *t_size, FVECTOR *t_pos, float *uv, float *lv ) ;
extern int  BRK_ICE_InitIce( Work *work ) ;
extern int  BRK_ICE_InitBox( Work *work ) ;
extern int  BRK_ICE_InitOpenBox( Work *work ) ;
extern int  BRK_ICE_InitIceDustPrims( ICEBOX *ice ) ;
extern int  BRK_ICE_GetOptions( Work * ) ;
extern int  BRK_ICE_InitHazard( Work * ) ;
extern int  BRK_ICE_InitTarget( TARGET *t, POWER_TARGET *p, int map, int flag,
				float *uv, float *lv, FVECTOR *pos,
				void (*callback)( TARGET *off, TARGET *def, void *ptr ),
				void *ptr ) ;
extern int BRK_ICE_SprayPrimSet( Work *work, FVECTOR *center, FVECTOR *force ) ;
extern int BRK_ICE_SprayPrimInit( Work *work ) ;
extern void BRK_ICE_InitBroken( Work *work ) ;

//add shibata
extern int BRK_ICE_InitIceDustComdl( ICEBOX *ice );

/* brk_ice_hzx.c */
extern int BRK_ICE_HzdIceboxCheck( Work *work, float rad ) ;


static inline void RotateMatrix( FMATRIX *out, FMATRIX *in, SVECTOR *rot  )
{
    int r ;
    r = rot->vx & 0x0fff ;
    _sceVu0RotMatrixX( out, in , (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot->vy & 0x0fff ;
    _sceVu0RotMatrixY( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot->vz & 0x0fff ;
    _sceVu0RotMatrixZ( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
}
static inline void RotateMatrixXY( FMATRIX *out, FMATRIX *in, short rot_x, short rot_y )
{
    int r ;
    r = rot_x & 0x0fff ;
    _sceVu0RotMatrixX( out, in , (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot_y & 0x0fff ;
    _sceVu0RotMatrixY( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
}
static inline void RotateMatrixXZ( FMATRIX *out, FMATRIX *in, short rot_x, short rot_z )
{
    int r ;
    r = rot_z & 0x0fff ;
    _sceVu0RotMatrixZ( out, in , (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
   r = rot_x & 0x0fff ;
    _sceVu0RotMatrixX( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
 }

static inline void TransMatrix( FMATRIX *mtx, FVECTOR *pos  )
{
    _sceVu0AddVector( (FVECTOR*)&mtx->m[W], (FVECTOR*)&mtx->m[W], pos ) ;
    mtx->m[W][Y] += 1.0f ;
    mtx->m[W][W] = 1.0f ;
}
