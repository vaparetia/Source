#ifndef __MORITA_UTL_H__
#define __MORITA_UTL_H__

#undef MIN
#undef MAX
#define MIN(_a,_b) ((_a)< (_b)?(_a):(_b))
#define MAX(_a,_b) ((_a)>=(_b)?(_a):(_b))


static inline void ClampVector( FVECTOR *a, FVECTOR *b, FVECTOR *min, FVECTOR *max  )
{
#ifdef BP_PSX2_ASM
    asm volatile ("
    lqc2        vf1, 0(%1)
    lqc2        vf2, 0(%2)
    lqc2        vf3, 0(%3)
    vmax.xyz    vf1, vf2, vf1
    vmini.xyz   vf1, vf3, vf1
    sqc2        vf1, 0(%0)
    " : : "r"(a), "r"(b), "r"(min), "r"(max) ) ;
#else
    b->vx = MAX( max->vx, a->vx) ; b->vx = MIN( min->vx, b->vx) ;
    b->vy = MAX( max->vy, a->vy) ; b->vy = MIN( min->vy, b->vy) ;
    b->vz = MAX( max->vz, a->vz) ; b->vz = MIN( min->vz, b->vz) ;

#endif
}


static inline void ScaleVector( FVECTOR *r, FVECTOR *a, float b  )
{
#ifdef BP_PSX2_ASM
    asm volatile ("
    lqc2        vf1, 0(%0)
    qmtc2       %1 , vf2
    vmulx.wxyz  vf1, vf1, vf2x
    sqc2        vf1, 0(%2)
    " : : "r"(a), "r"(b), "r"(r) ) ;
#else
	r->vx = a->vx * b ;
	r->vy = a->vy * b ;
	r->vz = a->vz * b ;
#endif
}
static inline void ScaleAndAddVector( FVECTOR *a, FVECTOR *b, float c )
{
#ifdef BP_PSX2_ASM
    asm volatile ("
    lqc2        vf1, 0(%1)
    qmtc2.ni    %2, vf2
    vmulx.xyzw  vf1, vf1, vf2
    lqc2        vf2, 0(%0)
    vadd.xyzw   vf2, vf2, vf1
    sqc2        vf2, 0(%0)
    " : : "r"(a), "r"(b), "r"(c) ) ;
#else
	a->vx = a->vx + b->vx * c ;
	a->vy = a->vy + b->vy * c ;
	a->vz = a->vz + b->vz * c ;
#endif
}


static inline void AddVectorXYZW( FVECTOR *r, FVECTOR *a, FVECTOR *b  )
{
#ifdef BP_PSX2_ASM
    asm volatile ("
    lqc2        vf1, 0(%1)
    lqc2        vf2, 0(%2)
    vadd.xyzw   vf1, vf2, vf1
    sqc2        vf1, 0(%0)
    " : : "r"(r), "r"(a), "r"(b) ) ;
#else
	r->vx = a->vx + b->vx ;
	r->vy = a->vy + b->vy ;
	r->vz = a->vz + b->vz ;
	r->vw = a->vw + b->vw ;
#endif
}

static inline void AddSVector( SVECTOR *r, SVECTOR *a, SVECTOR *b  )
{
#ifdef BP_PSX2_ASM
    asm volatile ("
    ld          $8, 0(%0)
    ld          $9, 0(%1)
    paddsh      $8, $9, $8
    sd          $8, 0(%2)
    " : : "r"(a), "r"(b), "r"(r) :"$8" ) ;
#else
	r->vx = a->vx + b->vx ;
	r->vy = a->vy + b->vy ;
	r->vz = a->vz + b->vz ;
#endif
}
static inline void SubSVector( SVECTOR *r, SVECTOR *a, SVECTOR *b  )
{
#ifdef BP_PSX2_ASM
    asm volatile ("
    ld          $8, 0(%0)
    ld          $9, 0(%1)
    psubsh      $8, $8, $9
    sd          $8, 0(%2)
    " : : "r"(a), "r"(b), "r"(r) : "$8", "$9" ) ;
#else
	r->vx = a->vx - b->vx ;
	r->vy = a->vy - b->vy ;
	r->vz = a->vz - b->vz ;
#endif
}
static inline void ScaleSVector( SVECTOR *r, SVECTOR *a, int f ) /* 8bit 固定小数*/
{
#ifdef BP_PSX2_ASM
    asm volatile ("
    ld          $8, 0(%0)
    pcpyh       $9, %1
    pmulth      $8, $9, $8
    pmfhl.lw    $8
    pmfhl.uw    $9
    pextlw      $8, $9, $8
    li          $9, 256
    pdivbw      $8, $9
    pmflo       $8
    ppach       $8, $0, $8
    sd          $8, 0(%2)
    " : : "r"(a), "r"(f), "r"(r) :"$8", "$9" ) ;

    /* psrah $8,$9,8  マイナスの時には, 256で割るのと8bit算術右シフトは違う！！ */
#else
	r->vx = a->vx * f /256 ;
	r->vy = a->vy * f /256 ;
	r->vz = a->vz * f /256 ;
#endif
}

static inline void SVector2FVector( FVECTOR *a, SVECTOR *t )
{
#ifdef BP_PSX2_ASM
    asm volatile( "
         ld           $8 ,0(%0)      /* load <vel[i]> to $8      */
         pextlh       $8 ,$8 ,$0     /* SVECTOR -> IVECTOR(16bitfix) */
         psraw        $8 ,$8 ,4      /* IVECTOR(16) -> IVECTOR(12bitfix) */
         qmtc2.ni     $8 ,vf1        /* send to cop2             */
         vitof12.xyz  vf1,vf1        /* IVECTOR(12) -> FVECTOR   */
         sqc2         vf1,0(%1)      /* store <pos[i]>           */
        " : : "r"(t), "r"(a) : "$8" ) ;
#else
	a->vx = t->vx ;
	a->vy = t->vy ;
	a->vz = t->vz ;
#endif
}

static inline void AddVector( FVECTOR *r, FVECTOR *a, FVECTOR *b  )
{
#ifdef BP_PSX2_ASM
    asm volatile ("
    lqc2        vf1, 0(%0)
    lqc2        vf2, 0(%1)
    vadd.xyzw   vf1, vf2, vf1
    sqc2        vf1, 0(%2)
    " : : "r"(a), "r"(b), "r"(r) ) ;
#else
	r->vx = a->vx + b->vx ;
	r->vy = a->vy + b->vy ;
	r->vz = a->vz + b->vz ;
#endif
}

static inline void SubVector( FVECTOR *r, FVECTOR *a, FVECTOR *b  )
{
#ifdef BP_PSX2_ASM
    asm volatile ("
    lqc2        vf1, 0(%0)
    lqc2        vf2, 0(%1)
    vsub.xyz    vf1, vf1, vf2
    sqc2        vf1, 0(%2)
    " : : "r"(a), "r"(b), "r"(r) ) ;
#else
	r->vx = a->vx - b->vx ;
	r->vy = a->vy - b->vy ;
	r->vz = a->vz - b->vz ;
#endif
}

static inline void MinMaxVector( FVECTOR *min, FVECTOR *max, FVECTOR *a  )
{
#ifdef BP_PSX2_ASM
    asm volatile ("
    lqc2        vf3, 0(%2)
    lqc2        vf1, 0(%0)
    lqc2        vf2, 0(%1)
    vmini.xyz   vf1, vf1, vf3
    vmax.xyz    vf2, vf2, vf3
    sqc2        vf1, 0(%0)
    sqc2        vf2, 0(%1)
    " : : "r"(min), "r"(max), "r"(a) ) ;
#else
    max->vx = MAX( max->vx, a->vx) ; min->vx = MIN( min->vx, a->vx) ;
    max->vy = MAX( max->vy, a->vy) ; min->vy = MIN( min->vy, a->vy) ;
    max->vz = MAX( max->vz, a->vz) ; min->vz = MIN( min->vz, a->vz) ;
#endif
}

static inline void MaxVector( FVECTOR *r, FVECTOR *a, FVECTOR *b  )
{
#ifdef BP_PSX2_ASM
    asm volatile ("
    lqc2        vf1, 0(%0)
    lqc2        vf2, 0(%1)
    vmax.xyz    vf1, vf2, vf1
    sqc2        vf1, 0(%2)
    " : : "r"(a), "r"(b), "r"(r) ) ;
#else
    r->vx = MAX( b->vx, a->vx ) ;
    r->vy = MAX( b->vy, a->vy ) ;
    r->vz = MAX( b->vz, a->vz ) ;
#endif
}

static inline void MinVector( FVECTOR *r, FVECTOR *a, FVECTOR *b  )
{
#ifdef BP_PSX2_ASM
    asm volatile ("
    lqc2        vf1, 0(%0)
    lqc2        vf2, 0(%1)
    vmini.xyz   vf1, vf2, vf1
    sqc2        vf1, 0(%2)
    " : : "r"(a), "r"(b), "r"(r) ) ;
#else
    r->vx = MIN( b->vx, a->vx ) ;
    r->vy = MIN( b->vy, a->vy ) ;
    r->vz = MIN( b->vz, a->vz ) ;
#endif
}


static inline void MinMaxVector4( FVECTOR *min, FVECTOR *max, FVECTOR *a  )
{
#ifdef BP_PSX2_ASM
    asm volatile ("
    lqc2        vf16, 0(%1)
    lqc2        vf17, 0(%0)
    lqc2        vf1, 0x00(%2)
    lqc2        vf2, 0x10(%2)
    lqc2        vf3, 0x20(%2)
    lqc2        vf4, 0x30(%2)
    vmax.xyz    vf1, vf17, vf1
    vmax.xyz    vf2, vf17, vf2
    vmax.xyz    vf3, vf17, vf3
    vmax.xyz    vf4, vf17, vf4
    vmini.xyz   vf1, vf16, vf1
    vmini.xyz   vf2, vf16, vf2
    vmini.xyz   vf3, vf16, vf3
    vmini.xyz   vf4, vf16, vf4
    sqc2        vf1, 0x00(%2)
    sqc2        vf2, 0x10(%2)
    sqc2        vf3, 0x20(%2)
    sqc2        vf4, 0x30(%2)
    " : : "r"(min), "r"(max), "r"(a) ) ;
#else
    (a+0)->vx = MAX( min->vx, (a+0)->vx	) ; (a+0)->vx = MIN( max->vx, (a+0)->vx ) ;
    (a+0)->vy = MAX( min->vy, (a+0)->vy	) ; (a+0)->vy = MIN( max->vy, (a+0)->vy ) ;
    (a+0)->vz = MAX( min->vz, (a+0)->vz	) ; (a+0)->vz = MIN( max->vz, (a+0)->vz ) ;
              
    (a+1)->vx = MAX( min->vx, (a+1)->vx	) ; (a+1)->vx = MIN( max->vx, (a+1)->vx ) ;
    (a+1)->vy = MAX( min->vy, (a+1)->vy	) ; (a+1)->vy = MIN( max->vy, (a+1)->vy ) ;
    (a+1)->vz = MAX( min->vz, (a+1)->vz	) ; (a+1)->vz = MIN( max->vz, (a+1)->vz ) ;
              
    (a+2)->vx = MAX( min->vx, (a+2)->vx	) ; (a+2)->vx = MIN( max->vx, (a+2)->vx ) ;
    (a+2)->vy = MAX( min->vy, (a+2)->vy	) ; (a+2)->vy = MIN( max->vy, (a+2)->vy ) ;
    (a+2)->vz = MAX( min->vz, (a+2)->vz	) ; (a+2)->vz = MIN( max->vz, (a+2)->vz ) ;
              
    (a+3)->vx = MAX( min->vx, (a+3)->vx	) ; (a+3)->vx = MIN( max->vx, (a+3)->vx ) ;
    (a+3)->vy = MAX( min->vy, (a+3)->vy	) ; (a+3)->vy = MIN( max->vy, (a+3)->vy ) ;
    (a+3)->vz = MAX( min->vz, (a+3)->vz	) ; (a+3)->vz = MIN( max->vz, (a+3)->vz ) ;
#endif
}

static inline void SwapVector( FVECTOR *a, FVECTOR *b  )
{
#ifdef BP_PSX2_ASM
    asm volatile ("
    lqc2        vf1, 0(%0)
    lqc2        vf2, 0(%1)
    sqc2        vf2, 0(%0)
    sqc2        vf1, 0(%1)
    " : : "r"(a), "r"(b) ) ;
#else
	FVECTOR tmp ;
	tmp = *a  ;
	*a  = *b  ;
	*b  = tmp ;
#endif
}

#include <libutl.h>
static inline void ApplyMatrixXYZ( FVECTOR *a, FMATRIX *m, FVECTOR *v )
{
#ifdef BP_PSX2_ASM
    asm volatile  ("
    lqc2        vf1,0x00(%1)
    lqc2        vf2,0x00(%0)
    lqc2        vf3,0x10(%0)
    lqc2        vf4,0x20(%0)
    vmulax.xyz  ACC,vf2,vf1x
    vmadday.xyz ACC,vf3,vf1y
    vmaddz.xyz  vf2,vf4,vf1z
    sqc2        vf2,0x00(%2)
    " : : "r"(m), "r"(v), "r"(a) );
#else
	vu0_ApplyMatrixROT( a, m, v ) ;
#endif
}







/*

    Print

*/

static inline int UTL_DBG_printf( char *fmt, ... )
{
#ifdef DEBUG_MODE
#ifdef KP_XBOX
	va_list ap;
	
	va_start( ap, fmt ) ;
	vprintf( fmt, ap ) ;
	
	va_end( ap ) ;
#endif
#endif
	return -1 ;
}
#define PERROR return UTL_DBG_printf



/*

  Debug Print

*/

static inline void PrintMatrixF( FMATRIX *mtx, char *s )
{
	enum { X=0, Y, Z, XY=Z, W, XYZ=W, XYZW } ;

    printf( "%s\t|%f %f %f %f|\n""\t|%f %f %f %f|\n""\t|%f %f %f %f|\n""\t|%f %f %f %f|\n", (s ? s : "mtx"),
	    mtx->m[X][X],mtx->m[X][Y],mtx->m[X][Z],mtx->m[X][W],
	    mtx->m[Y][X],mtx->m[Y][Y],mtx->m[Y][Z],mtx->m[Y][W],
	    mtx->m[Z][X],mtx->m[Z][Y],mtx->m[Z][Z],mtx->m[Z][W],
	    mtx->m[W][X],mtx->m[W][Y],mtx->m[W][Z],mtx->m[W][W] ) ;
}
static inline void PrintMatrix( FMATRIX *mtx, char *s )
{
	enum { X=0, Y, Z, XY=Z, W, XYZ=W, XYZW } ;

    printf( "%s\t|%f %f %f|\n""\t|%f %f %f|\n""\t|%f %f %f|\n", (s ? s : "mtx"),
	    mtx->m[X][X],mtx->m[X][Y],mtx->m[X][Z],
	    mtx->m[Y][X],mtx->m[Y][Y],mtx->m[Y][Z],
	    mtx->m[Z][X],mtx->m[Z][Y],mtx->m[Z][Z] ) ;
}

static inline void PrintSVector( SVECTOR *vec, char *s )
{
    printf( "%s\t(%d %d %d %d)\n", (s ? s : "svec"),
	    vec->vx,  vec->vy, vec->vz,  vec->pad ) ;
}

static inline void PrintVectorF( FVECTOR *vec, char *s )
{
    printf( "%s\t(%.2f %.2f %.2f %.2f)\n", (s ? s : "vec"),
	    vec->vx,  vec->vy, vec->vz,  vec->vw ) ;
}
static inline void PrintVector( FVECTOR *vec, char *s )
{
    printf( "%s\t(%.2f %.2f %.2f)\n", (s ? s : "vec"),
	    vec->vx,  vec->vy, vec->vz ) ;
}



#endif
