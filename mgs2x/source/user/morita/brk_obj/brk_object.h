/*
        brk_object.h
	オブジェクト破壊用 定義ファイル

        2000/02/15 T.Morita
        $Id: brk_object.h,v 1.1.1.3 2002/11/19 11:45:38 Yoshizawa1 Exp $
*/

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

//for DEBUG
#include "../include/util.h"

// for DG-LIB
#include "../include/libdg_x.h"

#include "../brk_hzd/brk_hazard.h"
#include "../brk_utl/brk_utl.x"

#define BRK_GRAVITY      3
#define BRK_N_PIECE      10
#define BRK_OBJ_BOXVITALITY 3

#define BRK_BOUND_X  300000.0f
#define BRK_BOUND_Y  300000.0f
#define BRK_BOUND_Z  300000.0f

#define BRK_PVEL_R 0.6f
#define BRK_PROT_R 16
#define BRK_VEL_R  0.6f
#define BRK_ROT_R  16

#define BRK_BOUNCE 1.28f
#define BRK_OBJ_RAD 80.0f
#define BRK_HZX_SPHERE   (BRK_OBJ_RAD/2)
#define BRK_HZX_S_SPHERE 500

#define BRK_OBJ_INACTIVE 0x8000000

enum brk_msg_t
{
    BRK_OBJ_M_REGIST_FOOT = 0,
    BRK_OBJ_M_ALREADY_MESSY,
    BRK_OBJ_M_GET_BOX_VITALITY,
    BRK_OBJ_M_GET_OBJ_POS,
    BRK_OBJ_M_GET_OBJ_NUM,
    BRK_OBJ_M_GET_BOX_POS,
} ;

enum brk_flg_t
{
    BRK_F_COLLISION  = 0x00000001,
    BRK_F_STRICT_FLR = 0x00000002,
    BRK_F_PRE_SHADE  = 0x00000004,
    BRK_F_NO_GRAVITY = 0x00000008,
    BRK_F_STABLE_X   = 0x00000010,
    BRK_F_STABLE_Y   = 0x00000020,
    BRK_F_STABLE_Z   = 0x00000040,
    BRK_F_STABLE_MSK = 0x00000070,
    BRK_F_NO_BOXSHOW = 0x00000080,
    BRK_F_NO_BOXTRG  = 0x00000100,
    BRK_F_RND_DRP_SE = 0x00000200,
    BRK_F_RND_BRK_SE = 0x00000400,
    BRK_F_BRK_SPREAD = 0x00000800,
    BRK_F_NO_KICK    = 0x00001000,

    BRK_F_INSIDE_BOX = 0x10000000,
} ;

typedef struct move_t   PIECE ;
typedef struct parts_t  PART ;
typedef struct obj_t    BRK_OBJ  ;
typedef struct work_t   Work ;
typedef struct box_t    BOX    ;

struct box_t
{
    DG_OBJS  *objs ;
    SVECTOR   brot, brot_v ;
    FVECTOR   pos  ;
    short     rot[5], rot_v[5] ;

    void    (*act)( Work* ) ;

    HZD_BOX  *hzd       ;
    int       vitality  ;
    FMATRIX   lights[2] ;

    TARGET       target ;
    POWER_TARGET power  ;
} ;



struct move_t
{
    FVECTOR  pos   ; /* 位置     */
    FVECTOR  pos_v ; /* 速度     */

    short    rot_x,  rot_y  ;
    short    rot_vx, rot_vy ;

    int      flag  ;

    DG_COMDL_POS *comdl ;
} ;

struct parts_t
{
    FVECTOR  pos    ; /* 位置     */
    FVECTOR  pos_v  ; /* 速度     */
    SVECTOR  rot    ; /* 向き     */
    SVECTOR  rot_v  ; /* 角速度   */

    int      flag    ;

    DG_OBJ  *obj     ;
    BRK_OBJ *object  ;
    int      se_flag ;
} ;


struct obj_t
{
    FVECTOR  pos    ; /* 位置     */
    FVECTOR  pos_v  ; /* 速度     */
    SVECTOR  rot    ; /* 向き     */
    SVECTOR  rot_v  ; /* 角速度   */

    int    (*act)( Work*, BRK_OBJ* ) ;

    DG_OBJS *objs    ;
    int      n_parts ;

    union
    {
	PART    *prof ;
	FVECTOR *foot ;
    } parts ;

    TARGET       target ;
    POWER_TARGET power  ;

    Work    *work     ;
} ;

struct work_t
{
    GV_ACT    actor    ;

    int       name     ;
    int       where    ;
    int       flag     ;
    int       proc_id  ; /* 壊れプロック */
    float     explode  ; /* 破裂度合 */
    float     pop_up   ; /* 破裂時の跳ね度 */
    float     stable   ; /* 安定性     */
    FVECTOR   r_size   ; /* 当たり半径 */

    BRK_OBJ  *obj      ; /* 壊れオブジェ */
    int       n_obj    ;
    DG_DEF   *part_def ; /* 壊れ後のモデル */
    int       brk_se_id ; /* 壊れの時なるSE */
    int       drp_se_id ; /* 落ちる時なるSE */

    BRK_OBJ  *foot     ; /* 足用の当たり */
    int       n_foot   ;

    BOX      *box      ; /* 箱情報 */

    int       dust_mode ;/* 埃の出る種類 */
    HZX_GROUP_ID hzx   ;
    void        *hzd   ;

    DG_COMDL  *comdl[2] ; /* 破片コモデル */
    PIECE      piece[BRK_N_PIECE] ;
    int        n_piece ;
    int        counter ;

    ENEFIND    ene_find ;/*敵兵 見つかり用*/
    union light_t
    {
	FMATRIX  mtx[2] ; /* シェード用 ライトマトリックス */
	LIT_DEF *def    ; /* プリシェード用 ライトデフ     */
    } light ;
} ;



extern void AN_Test_Eye2( FVECTOR *mov, int size ) ; /* for test */



/* brk_object.c */
extern void BRK_OBJ_FreeObj( BRK_OBJ *o ) ;

/* brk_obj_act.c */
extern float BRK_OBJ_StickPartsOnFloor( DG_OBJ *p, FVECTOR *size, FVECTOR *cen ) ;
extern void BRK_OBJ_ActSound( Work *work ) ;
extern void BRK_OBJ_ActPiece( Work *work, PIECE *p, int i ) ;
extern void BRK_OBJ_StartActPieces( Work *work, int i, FVECTOR *pos, FVECTOR *pos_v,
				    float width, float pop ) ;
extern int  BRK_OBJ_ActPartBrokenFree( Work *work, PART *p ) ;
extern int  BRK_OBJ_ActPartBroken( Work *work, PART *p ) ;
extern int  BRK_OBJ_ActPartBreaking( Work *work, PART *p ) ;
extern int  BRK_OBJ_ActNone( Work *work, BRK_OBJ *p ) ;
extern int  BRK_OBJ_ActMove( Work *work, BRK_OBJ *p ) ;
extern int  BRK_OBJ_ActBreak( Work *work, BRK_OBJ *p ) ;
extern int  BRK_OBJ_ActOnFoot( Work *work, BRK_OBJ *p ) ;
extern void BRK_OBJ_ActObject( Work *work ) ;

/* brk_obj_clb.c */
extern void BRK_OBJ_TargetCallBack( TARGET *off, TARGET *def, void *ptr ) ;
extern void BRK_OBJ_PartsTargetCallBack( TARGET *off, TARGET *def, void *ptr ) ;
extern void BRK_OBJ_BoxTargetCallBack( TARGET *off, TARGET *def, void *ptr ) ;

/* brk_obj_msg.c */
extern void BRK_OBJ_ReceiveMessage( Work *work ) ;
extern void BRK_OBJ_AlreadyMessy( Work *work ) ;

/* brk_obj_ini.c */
extern int  BRK_OBJ_InitObject( BRK_OBJ *p, DG_DEF *def, int flag, void *act ) ;
extern int  BRK_OBJ_InitBox( Work *work, int map ) ;
extern int  BRK_OBJ_GetOptions( Work *, int map ) ;
extern void BRK_OBJ_InitPart( PART *p, DG_MDL *mdl, DG_OBJ *obj, void *act ) ;
extern int  BRK_OBJ_InitHazard( Work *work, int where ) ;
extern void BRK_OBJ_InitEneFind( Work *work ) ;
extern void BRK_OBJ_InitBroken( Work *work ) ;

/* brk_obj_hzx.c */
extern void BRK_OBJ_HzdObjectCheck( Work *work, float rad ) ;
extern int  BRK_OBJ_HzdCheck( Work *work,
			      FVECTOR *pos, FVECTOR *pos_v,
			      float min, FVECTOR *bounce, float sphere ) ;

/* external character*/
extern void *NewCrushDust( FVECTOR *pos, FVECTOR *force, int mode ) ;
extern void *NewSaltSplay( FMATRIX *world ) ;
extern void *NewKetchapSpread( HZD_BOX *hzd, FVECTOR *pos, FVECTOR *force, int color, float up ) ;

extern int BRK_OBJ_NoDropSound ;
extern int BRK_OBJ_HowManyActs ;

static inline void RotateMatrix( FMATRIX *out, FMATRIX *in, SVECTOR *rot  )
{
    float r ;

    r = (rot->vz & 0x0fff) * (float)M_PI/2048.0F ;
    _sceVu0RotMatrixZ( out, in , r>(float)M_PI ? r-2.0f*(float)M_PI : r ) ;
    r = (rot->vx & 0x0fff) * (float)M_PI/2048.0F ;
    _sceVu0RotMatrixX( out, out, r>(float)M_PI ? r-2.0f*(float)M_PI : r ) ;
    r = (rot->vy & 0x0fff) * (float)M_PI/2048.0F ;
    _sceVu0RotMatrixY( out, out, r>(float)M_PI ? r-2.0f*(float)M_PI : r ) ;
}
static inline void RotateMatrixXY( FMATRIX *out, FMATRIX *in, short rot_x, short rot_y  )
{
    int r ;
    r = rot_x & 0x0fff ;
    _sceVu0RotMatrixX( out, in , (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot_y & 0x0fff ;
    _sceVu0RotMatrixY( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
}
static inline void TransMatrix( FMATRIX *mtx, FVECTOR *pos  )
{
    mtx->m[3][X] += pos->vx ;
    mtx->m[3][Y] += pos->vy ;
    mtx->m[3][Z] += pos->vz ;
}
