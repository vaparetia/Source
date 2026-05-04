/*
	wave.h
	    波メインヘッダ

	1999/08/10 T.Morita
	$Id: wave.h,v 1.1.1.3 2002/11/19 11:46:39 Yoshizawa1 Exp $
*/
#ifndef _WAVE_H_
#define _WAVE_H_

// for DG-LIB
#include "../include/libdg_x.h"
#include "../include/util.h"

//#define ADD 1

#define WAVE_WAVE_COL 12
#define WAVE_WAVE_ROW 12
#define WAVE_BUBL_COL 12 
#define WAVE_BUBL_ROW 12
#define WAVE_BACK_COL 12 
#define WAVE_BACK_ROW 12
#define WAVE_SIDE_COL 12
#define WAVE_SIDE_ROW 12

#define WAVE_SIZE   7
#define WAVE_SPEED  256
#define WAVE_SHAPE  1
#define WAVE_BLK    5
#define WAVE_FOG    6

#define WAVE_CONTRAST (4096)

#define WAVE_N_BRK  250
#define WAVE_MAX_SPSH 10
#define WAVE_MAX_BAK  4
#define WAVE_MAX_BIG_SPSH 5

#define MAX_V   60
#define MAX_RND 128

#define WAVE_FLAG (DG_FLAG_PAINT|DG_FLAG_ONEPIECE|DG_FLAG_BOUND|DG_FLAG_INVISIBLE)
#define WAVE_TYPE (DG_TYPE_NOSHADE)
#define BACK_FLAG (DG_FLAG_PAINT|DG_FLAG_ONEPIECE|DG_FLAG_BOUND|DG_FLAG_INVISIBLE)
#define BACK_TYPE (DG_TYPE_NOSHADE)
#define BUBL_FLAG (DG_FLAG_PAINT|DG_FLAG_ONEPIECE|DG_FLAG_BOUND|DG_FLAG_INVISIBLE)
#define BUBL_TYPE (DG_TYPE_NOSHADE)
#define SIDE_FLAG (DG_FLAG_PAINT|DG_FLAG_ONEPIECE|DG_FLAG_BOUND|DG_FLAG_INVISIBLE)
#define SIDE_TYPE (DG_TYPE_NOSHADE)

#define SPRT_FLG  (DG_PRIM_ON_WORLD|DG_PRIM_SPRT3D2)

#define WAV_MEMORY_SIZE (12032-6224-592)
#define WAVE_BASE 3840.0f

#define CENTER_DISTANCE 20000.0f
#define SQR_SIZE        8000.0f


/*
  Enumerations

 */
enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

enum
{
    R=0, G, B, K, RGB=K, RGBK
} ;

enum
{
    MDL_INIT_NON  = 0x0000,

    MDL_INIT_VTX  = 0x0001,
    MDL_INIT_NRM  = 0x0002,
    MDL_INIT_UVS  = 0x0004,
    MDL_INIT_MAP  = 0x0008,
    MDL_INIT_INI  = 0x0010,
    MDL_INIT_TYP  = 0x0020,
    MDL_INIT_UVS2 = 0x0040,

    MDL_INIT_ALL  = 0x003f,
    MDL_INIT_ALL2 = 0x005f,

    MDL_FLAG      = 0xffff,
} ;



/*
  Type Definitions 

 */
typedef struct mem_t
{
    u_int block ;
    u_int current ; /* current position */
} MEM ;

#define MAP(_mp,_x,_z,_c) ((_mp)[(_i)+(_x)+(_z)*(_c)])
typedef struct mdl_data_t
{
    short *vtx[2] ;
    short *map  ;

    int col  ;
    int row  ;
    int flag ;
} MDL ;

typedef struct scrach_pad_t
{
    MDL   wave ;
    MDL   flush ;
} ScrPad ;

typedef ALIGN16_DECL(struct) volume_t
{
    SVECTOR   vel[WAVE_N_BRK]     ;/* <pad> will be next index */
    DG_PRIM2 *prim  ;
    int       n_idx ;
} VOL ;

typedef	struct work_t
{
    GV_ACT actor;

    int name     ;
    int no_sound ;
    /* model data */
    MDL   m_wave ;

    /* wave specific */
    FVECTOR center ;
    int   speed ;
    short wave0 ;

    /* big splash */
    short   n_big ;
    SVECTOR big_pos[WAVE_MAX_BIG_SPSH] ;/* pad: 0/1=dead/alive  vy:active height */
    FMATRIX big_mtx[WAVE_MAX_BIG_SPSH] ;
    int     big_alp[WAVE_MAX_BIG_SPSH] ;
    int     big_cnt[WAVE_MAX_BIG_SPSH] ;

    /* internal memory system */
    MEM   mem   ;
    float eyex, eyez ; /* eye control */
} Work ;

/*
  ProtoTypes

 */

/* wave_act.c */
extern void  WAV_ActAllWave( Work *w ) ;

/* wave_mem.c */
extern int   WAV_MemInit() ;
extern void *WAV_MemAlloc( int size ) ;
extern void  WAV_MemDie() ;

/* wave_eye.c */
extern int   WAV_EyeControl( Work * ) ;

/* wave_ini.c */
extern int   WAV_GetOptionValue( Work *work, int col, int row ) ;
extern int   WAV_InitModelsAndPrims( Work *work ) ;

/* other external variable */
extern int     G_wind_sw ;
extern FVECTOR G_wind ;

#ifdef _INIT_FILE_

#if 0 /*とっちゃえー*/
static ALIGN16_PRE DG_DEF def ALIGN16_POST =
{
    13112177,      /* data_format */
    1, 1,   /* n_models, n_x_models */
    0,      /* texture */
    {0,0},  /* pad[2]  */
    -2048.0f                 , -2048.0f, -2048.0f                 ,/* lx, ly, lz */
    WAVE_SIZE*4096.0f+2048.0f,  4096.0f, WAVE_SIZE*4096.0f+2048.0f,/* ux, uy, uz */
    0.0f,0.0f,0.0f,                                                /* tx, ty, tz */
    {{ DG_TYPE_NOSHADE, 0,    /* type, n_pack */
       -2048.0f                 , -2048.0f, -2048.0f                 ,/* lx, ly, lz */
       WAVE_SIZE*4096.0f+2048.0f,  4096.0f, WAVE_SIZE*4096.0f+2048.0f,/* ux, uy, uz */
       0.0f,0.0f,0.0f,                                                /* tx, ty, tz */
       -1, NULL,  /* parent, packs    */
       0,0,0 }}   /* pad0, pad1, pad2 */
} ;
#endif

#else
#endif /*_INIT_WAVE_*/

#endif  /*_WAVE_H_*/
