/*
  brk_speech.h
  ガラス破壊用 定義ファイル

  2001/06/20 T.Morita
  $Id: brk_speech.h,v 1.1.1.3 2002/11/19 11:45:46 Yoshizawa1 Exp $
*/

// for box hzd
#include  "../brk_hzd/brk_hazard.h"
// for DG-LIB
#include "../include/libdg_x.h"
#include "../include/util.h"

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

#define BRK_SPC_NBEND   7
#define BRK_SPC_NVERTS (BRK_SPC_NBEND*2 + 1)
#define BRK_SPC_INACTIVE 0x8000000

typedef struct
{
    FVECTOR  pos    ; /* 位置     */
    FVECTOR  pos_v  ; /* 速度     */
    short    rot_x,  rot_y  ;
    short    rot_vx, rot_vy ;

    float    scale ;

    DG_COMDL_POS *comdl ;

    int      flag   ;
} PIECE ;

typedef struct work_t
{
    GV_ACT       actor  ;

    TARGET       target  ;
    POWER_TARGET power   ;

    HZD_BOX     *hzd     ;
    int          name    ;
    int          where   ;
    int          proc    ;
    

    DG_PRIM2    *broken  ;

    DG_COMDL    *piece_l ;
    DG_COMDL    *piece_s ;
    PIECE        piece[BRK_N_PIECE] ;
    int          n_piece ;

    DG_OBJS     *objs    ;

    int          model_nm  ;
    int          lpiece_nm ;
    int          spiece_nm ;
} Work ;
#define DEGtoANG(_a) ((_a)*8192/45)
#define ANGtoRAD(_a) ((_a)*(float)M_PI/32768.0f)
#define DEGtoRAD(_a) ((_a)*(float)M_PI/2048.0f)

#define FTOI12(_f)      ( DG_FTOI( ( (float)(_f)*4096.0f) ) )

extern void AN_Test_Eye2( FVECTOR *mov, int size ) ; /* for test */


extern void BRK_SPC_TargetCallBack( TARGET *off, TARGET *def, void *ptr ) ;

extern int  BRK_SPC_InitTarget( Work *work, int where ) ;
extern int  BRK_SPC_InitHazard( Work *, int where ) ;
extern int  BRK_SPC_InitPieces( Work *work, int where ) ;
extern int  BRK_SPC_GetOptions( Work *work, int where ) ;

extern void BRK_SPC_ActPieces( Work *work ) ;
extern void BRK_SPC_StartActPieces( Work *work, FVECTOR *frc, int i ) ;


static inline void RotateMatrixXY( PIECE *p )
{
    int      r ;
    FMATRIX *m = &p->comdl->world ;

    _sceVu0CopyMatrix( m, &DG_UnitMatrix ) ;
    m->m[X][X] = p->scale ;
    m->m[Z][Z] = p->scale ;
    r = p->rot_x & 0x0fff ;
    _sceVu0RotMatrixX( m, m, (r-4096*(r>>11))*(float)M_PI/2048.0f ) ;
    r = p->rot_y & 0x0fff ;
    _sceVu0RotMatrixY( m, m, (r-4096*(r>>11))*(float)M_PI/2048.0f ) ;
}
