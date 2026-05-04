#ifndef _WAVE_H_
#define _WAVE_H_


#define HEIGHT 2.0F
#define REFRACTION 0.28F

#define NRING 128
#define NVERT 225



typedef struct polygon_t
{
        short i[4] ;
} POLYGON ;

/* define wave type */
typedef struct ripple_t
{
        float h;            /* height */
        int aa, a1, a2;     /* amplitude */
        int ws;             /* speed */
        int a0, a;
        int top;
        float h_ring[NRING];/* ring height buffer */
        short d_tbl[NVERT]; /* distance table */
} RIPPLE;

typedef	struct work_t
{
    GV_ACT actor;
    OBJECT body ;

    RIPPLE r0 ;
    RIPPLE r1 ;

    FVECTOR  norms[NVERT] ;

    DG_PRIM *prim ;
} Work ;

extern int InitPrimitive( Work * ) ;
extern int InitRipple( Work * ) ;
extern int InitParameter( Work * ) ;

extern void ActRipple( Work * ) ;
extern void ChngTexture( Work * ) ;

extern POLYGON wave_plys[] ;
extern FVECTOR wave_vtxs[] ;
extern FVECTOR wave_texs[] ;
extern int num_plys ;
extern int num_vtxs ;
#ifdef _MAIN_WAVE_
#else
#endif

#endif  /*_WAVE_H_*/
