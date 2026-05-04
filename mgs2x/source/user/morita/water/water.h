/*
	WATR.h
	    波メインヘッダ

	1999/08/10 T.Morita
	$Id: water.h,v 1.1.1.3 2002/11/19 11:46:38 Yoshizawa1 Exp $
*/
#ifndef _WATR_H_
#define _WATR_H_


#define WATR_WAVE_COL 12
#define WATR_WAVE_ROW 12

#define WATR_SIZE   3
#define WATR_SPEED  256
#define WATR_BLK    5

#define MAX_V   60
#define MAX_RND 128

#define WATR_FLAG (DG_FLAG_PAINT|DG_FLAG_ONEPIECE|DG_FLAG_BOUND)
#define WATR_TYPE (DG_TYPE_NOSHADE)

#define WATR_MEMORY_SIZE 1024*65
#define WATR_BASE 3840.0f



#define M_SIN(x) (WAV_SinTbl[ (x)       &0x3ff])
#define M_COS(x) (WAV_SinTbl[((x)+0x0ff)&0x3ff])
extern short WAV_SinTbl[] ;

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
    short *nrm  ;
    short *uvs[2] ;
    short *map  ;
    short *init ;

    DG_OBJS **objs ;
    DG_TEX  *tex ;

    int col  ;
    int row  ;
    int flag ;

    DG_DEF def ;
    DG_MDL pad ;/* one MDL data has come after DG_DEF file*/
} MDL ;

typedef struct scrach_pad_t
{
    MDL   WATR ;
    MDL   flush ;
} ScrPad ;

typedef	struct work_t
{
    GV_ACT actor;

    /* model data */
    MDL   m_wave ;

    /* WATR specific */
    FMATRIX root ;
    int   x_blk ;
    int   z_blk ;
    int   n_blk ;
    FVECTOR center ;
    int   speed ;
    int   wave0 ;

    /* internal memory system */
    MEM   mem   ;
    float eyex, eyez ; /* eye control */
} Work ;


/*
  ProtoTypes

 */

extern void  WTR_ActAllWater( Work *w ) ;

extern int   WTR_MemInit() ;
extern void *WTR_MemAlloc( int size ) ;
extern void  WTR_MemDie() ;

extern int   WTR_EyeControl( Work * ) ;

extern int   WTR_InitModelsAndPrims( Work *work ) ;

#ifdef _INIT_FILE_

static DG_DEF def ALIGN16 =
{
    1, 1,
    -2048.0f                 , -2048.0f, -2048.0f                 ,
    WATR_SIZE*4096.0f+2048.0f,  4096.0f, WATR_SIZE*4096.0f+2048.0f,
    {{ DG_TYPE_NOSHADE, 0,
       -2048.0f                 , -2048.0f, -2048.0f                 ,
       WATR_SIZE*4096.0f+2048.0f,  4096.0f, WATR_SIZE*4096.0f+2048.0f,
       0.0f,0.0f,0.0f,
       -1, NULL, 0,0,0 }}
} ;


#else
#endif /*_INIT_WATR_*/

#endif  /*_WATR_H_*/
