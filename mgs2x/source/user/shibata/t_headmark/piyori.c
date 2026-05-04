//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	piyori.c

	2000/01/14     H.TANAKA

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include "libutl.h"


#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"def_dma.h"


#include	"utl_dma.h"

/* ---------------------------------------------------------------- */
	/*
		補助マクロ
	*/

#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
/* ---------------------------------------------------------------- */
	/*
		プログラム使用定数の定義
	*/


#define  N_PRIMS	 1
#define  N_VERTS         3
#define  PRIM_TYPE       (DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA)
#define  WIDE            25

#define  FIRST_TIME     440
#define  SECOND_TIME    140
#define  THIRD_TIME      40

#define    MEM_ADDR1    ((void *)( SCRPAD_ADDR + 0x0000 ))
#define    MEM_ADDR2    ((void *)( SCRPAD_ADDR + 0x0400 ))
#define    MEM_ADDR3    ((void *)( SCRPAD_ADDR + 0x1200 )) 

typedef  struct _work   Work ;

struct	_work{
	GV_ACT_EX		actor ;
	
	DG_PRIM2	*prim;
	FMATRIX		*world ;
	FVECTOR         base[N_PRIMS * N_VERTS] ;
	FVECTOR         speed[N_PRIMS * N_VERTS] ;
	int             count ;
	int				*map;
	int             (*act)( Work * ) ;
};


/* ------------------------------------------------------------------- */
                  /* 
                      プログラム使用サブルーチン
                  */

/* スクラッチパッドからメインメモリへの転送 */
extern void  _BigMemCopy( void *dst, void *src, int size, int num ) ;

#if 1
static  int  ReceiveSignal( void *workp, int signal, int value)
{
    Work *work = workp ;

    switch(signal){
	case 0:
		if(value == 0){
			if(work->count > THIRD_TIME) work->count = THIRD_TIME;
		}else if(value == 1){
			if(work->count > SECOND_TIME) work->count = SECOND_TIME;
		}else if(value == 2){
			if(work->count > FIRST_TIME) work->count = FIRST_TIME;
		}
		break;

	default:
		return GV_DefaultSignalFunc( work, signal, value );// Must do this!!!
	}
	//printf("count %d\n",work->count);
	return 0;
}
#endif

static   void  Set_PiyoWorld(FVECTOR *out, FMATRIX *world)
{
    FVECTOR  shift ;
    FVECTOR  center ;
    SVECTOR  ang ;

    shift.vx = 0.0F ;
    shift.vy = 100.0F ;
    shift.vz = 300.0F ;
    DG_SetPos(world) ;
    DG_RotVector(&shift,&shift,1) ;
    center.vx = shift.vx ;
    center.vy = 250.0F ;
    center.vz = shift.vz ;
    _sceVu0AddVector(&center,&center,(FVECTOR *)world->m[3]) ;
    ang.vx = 0 ;
    ang.vy = GV_Time * 128 ;
    ang.vz = 0 ;
    DG_SetPos2(&center,&ang) ;
	DG_COPY_VEC( out, &center );
}

static void Init_Speed(
FVECTOR  *speed,
FVECTOR  *pos,
FVECTOR  *center
)
{
    _sceVu0SubVector(speed,pos,center) ;
    speed->vw = 0.0F ;
    _sceVu0Normalize(speed,speed) ;
    speed->vx = speed->vx * 8.0F ;
    speed->vy = 46.0F ;
    speed->vz = speed->vz * 8.0F ;	
}

/* ------------------------------------------------------------------- */
static   int   Act_LAST(Work *work)
{
    DG_PRIM2          *prim ;
    FVECTOR           *base ;
    FVECTOR           *pos  ;
    FVECTOR           *prim_pos ;
    DG_PRIM2_UVRGBWH  *uvrgbwh ;
    FVECTOR           center ;
    u_short           alpha[3] ;
    FVECTOR           *speed ;
    int               i ;
    
    prim = work->prim ;
    base = work->base ;
    pos  = MEM_ADDR1 ;
    speed = work->speed ;
    
    /* 中心地点をセット */
    Set_PiyoWorld(&center,work->world) ;
    uvrgbwh = prim->uvrgb[prim->buffer_clock] ;
    prim_pos = prim->pos[prim->buffer_clock] ;
    for(i = 0; i < 3; i++)
    {
	_sceVu0AddVector(pos,prim_pos,speed) ;
	speed->vy -= 3.0F ;
	alpha[i] = uvrgbwh->a ;
	if(alpha[i] >= 4)
	    alpha[i] -= 4 ;
	else
	    alpha[i] = 0 ;
	base  ++ ;
	pos   ++ ;
	speed ++ ;
	prim_pos ++ ;
	uvrgbwh ++ ;
    }
    DG_SwitchBuffPrim2( prim ) ;
    _BigMemCopy(prim->pos[prim->buffer_clock],MEM_ADDR1,sizeof(FVECTOR),N_VERTS) ;
    uvrgbwh = prim->uvrgb[prim->buffer_clock] ;
    uvrgbwh[0].a = alpha[0] ;
    uvrgbwh[1].a = alpha[1] ;
    uvrgbwh[2].a = alpha[2] ;
    if(work->count < 0)
    {
	return -1 ;
    }
	//printf("piyo4\n");
    return 0;
}

static   int   Act_THIRD(Work *work)
{
    DG_PRIM2  *prim ;
    FVECTOR   *base ;
    FVECTOR   *pos ;
    FVECTOR   *prim_pos ;
    FVECTOR   center ;
    FVECTOR   *speed ;
    DG_PRIM2_UVRGBWH *uvrgbwh ;
    u_short       alpha[3] ;
    int            i; 
    
    prim = work->prim ;
    base = work->base ;
    pos  = MEM_ADDR1 ;
    speed = work->speed ;
  
    /* 中心地点をセット */
    Set_PiyoWorld(&center,work->world) ;

    /* １つめそのまま */
    DG_PutVector(base,pos,1) ;
   
    /* 2つめと3つめが 飛ぶ */
    uvrgbwh = prim->uvrgb[prim->buffer_clock] ;
    prim_pos = prim->pos[prim->buffer_clock] ;
    
    base ++ ;
    pos  ++ ;
    prim_pos ++ ;
    speed ++ ;
    uvrgbwh ++ ;
    for(i = 1; i< N_VERTS; i++)
    {
	_sceVu0AddVector(pos,prim_pos,speed) ;
	speed->vy -= 3.0F ;
	alpha[i] = uvrgbwh->a ;
	if(alpha[i] >= 4)
	    alpha[i] -= 4 ;
	else
	    alpha[i] = 0 ;
	base ++ ;
	pos ++ ;
	speed ++ ;
	prim_pos ++ ;
	uvrgbwh  ++ ;
    }

    /* 通常 */
    DG_SwitchBuffPrim2( prim ) ;
    _BigMemCopy(prim->pos[prim->buffer_clock],MEM_ADDR1,sizeof(FVECTOR),N_VERTS) ;
    uvrgbwh = prim->uvrgb[prim->buffer_clock] ;
    uvrgbwh[1].a = alpha[1] ;
    uvrgbwh[2].a = alpha[2] ;

    /* １つめの星 設定 */
    if(work->count <= THIRD_TIME)
    {
	speed = work->speed ;
	pos   = MEM_ADDR1 ;
	Init_Speed(speed,pos,&center) ;
	work->act = Act_LAST ;
    }
	//printf("piyo3\n");
    return 0 ;
}

static   int   Act_SECOND(Work *work)
{
    DG_PRIM2  *prim ;
    FVECTOR   *base ;
    FVECTOR   *pos ;
    FVECTOR   *prim_pos ;
    FVECTOR   center ;
    FVECTOR   *speed ;
    DG_PRIM2_UVRGBWH *uvrgbwh ;
    u_short       alpha ;
    FVECTOR   shift ;
    SVECTOR   base_ang ;
    
    prim = work->prim ;
    base = work->base ;
    pos  = MEM_ADDR1 ;
    speed = work->speed ;

    /* ------------------------------------*/
    /* 1つめと2つめ回転 */
    /* 2つめのベースの位置を設定 */
    shift.vx = 100.0F ;
    shift.vy = 0.0F ;
    shift.vz = 0.0F ;
    base_ang.vx = 0 ;
    base_ang.vy = 4098 / 3 + 50 * (FIRST_TIME - work->count) ;
    if(base_ang.vy > 2048)
	base_ang.vy = 2048 ;
    base_ang.vz = 0 ;
    DG_SetPos2(&DG_ZeroVector, &base_ang) ;
    DG_PutVector(&shift,&base[1],1) ;
    
    /* 中心位置をセット */
    Set_PiyoWorld(&center,work->world) ;
    DG_PutVector(base,pos,2) ;

    /* 3つめが 飛ぶ */
    prim_pos = prim->pos[prim->buffer_clock] ;
    prim_pos += 2 ;
    speed    += 2 ;
    pos      += 2 ;
    _sceVu0AddVector(pos,prim_pos,speed) ;
    speed->vy -= 3.0F ;
    uvrgbwh = prim->uvrgb[prim->buffer_clock] ;
    uvrgbwh += 2 ;
    alpha = uvrgbwh->a ;
    if(alpha >= 4)
	alpha -= 4 ;
    else
	alpha = 0 ;

    DG_SwitchBuffPrim2( prim ) ;
    _BigMemCopy(prim->pos[prim->buffer_clock],MEM_ADDR1,sizeof(FVECTOR),N_VERTS) ;
    uvrgbwh = prim->uvrgb[prim->buffer_clock] ;
    uvrgbwh[0].a = alpha ;
    
    /* ２つめの星の速度を決定 */
    if(work->count <= SECOND_TIME)
    {
	speed = &work->speed[1] ;
	pos   = MEM_ADDR1 ;
	pos   ++ ;
	Init_Speed(speed,pos,&center) ;
	work->act = Act_THIRD ;
    }
	//printf("piyo2\n");
    return 0 ;
}

static   int   Act_FIRST(Work *work)
{
    DG_PRIM2  *prim ;
    FVECTOR   *pos ;
    FVECTOR   *base ;
    FVECTOR   *speed ;
    FVECTOR   center ;

    /* バッファ交代 */
    prim = work->prim ;
    DG_SwitchBuffPrim2( prim ) ;
    base = work->base ;
    pos  = MEM_ADDR1 ;
    
    Set_PiyoWorld(&center,work->world) ;
    DG_PutVector(base, pos, 3) ; 
    _BigMemCopy(prim->pos[prim->buffer_clock],MEM_ADDR1,sizeof(FVECTOR),N_VERTS) ;

    /* 3つめの星を飛ばす */
    if(work->count <= FIRST_TIME){
		speed = &work->speed[2] ;
		Init_Speed(speed,&pos[2],&center) ;

		work->act = Act_SECOND ;
    }

	//printf("piyo1\n");
    return 0 ;
}

static   void   Act( Work *work )
{
    int  addr ;
	GM_GroupPrim2( work->prim, *work->map );
    addr = (int)work ;
    if(work->world == NULL){
		GV_CallParentSignalFunc(work,GV_SIGNAL_DIE,addr) ;
		GV_DestroyActor( work ) ;return ;
    }    

    if((*work->act)(work) == -1){
		GV_CallParentSignalFunc(work,GV_SIGNAL_DIE,addr) ;
		GV_DestroyActor(work) ;
		return ;
    }
    //work->count -- ;
}

static   void   Die( Work *work )
{
    if(work->prim != NULL)
	GM_FreePrim2(work->prim) ;

	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
//		void	CPE_DeletCamPoseSquare( DG_PRIM2 *prim ) ;
//		CPE_DeletCamPoseSquare( work->prim ) ;
		void	VRFUNC_CPE_DeletCamPoseSquare( DG_PRIM2 *prim ) ;
		VRFUNC_CPE_DeletCamPoseSquare( work->prim ) ;
	}
}

/* ---------------------------------------------------------------- */
static int Init_Scr( Work *work,DG_TEX *tex )
{
    FVECTOR          *pos ;
    FVECTOR          *base ;
    DG_PRIM2_UVRGBWH *uvrgbwh ;
    FVECTOR          shift ;
    SVECTOR          ang ;
    int              i ;

    base = MEM_ADDR3 ;
    shift.vx = 100.0F; 
    shift.vy = 0.0F ;
    shift.vz = 0.0F ;
    ang.vx = 0;
    ang.vy = 0;
    ang.vz = 0;
    for(i = 0; i < N_VERTS; i++)
    {
	DG_SetPos2(&DG_ZeroVector,&ang) ;
	DG_PutVector(&shift,base,1) ;
	ang.vy = ang.vy + 4096 / N_VERTS ;
	base ++ ;
    }

    pos     = MEM_ADDR1 ;
    uvrgbwh = MEM_ADDR2 ;
    base    = MEM_ADDR3 ;

    /* 位置を確定 */
    Set_PiyoWorld(&shift,work->world);
    DG_PutVector(base,pos,3) ;
    
    /* テクスチャ情報 */
    for(i = 0; i < N_VERTS; i++)
    {
	uvrgbwh->u0 = FTOI12( tex->u_offset ) ;
	uvrgbwh->v0 = FTOI12( tex->v_offset ) ;
	uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
	uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
	uvrgbwh->q0 = 4096 ;
	uvrgbwh->q1 = 4096 ;
	uvrgbwh->f0 = 0x0fff ;
	uvrgbwh->f1 = 0x0fff ;
	uvrgbwh->w = WIDE ;
	uvrgbwh->h = WIDE ;
	uvrgbwh->r = 200  ;
	uvrgbwh->g = 200  ;
	uvrgbwh->b = 64   ;
	uvrgbwh->a = 128 ;

	base ++ ;
	uvrgbwh ++ ;
    }   

	return 1;

}

static int GetResources( Work *work, int *map )
{
	DG_TEX	       *tex ;
	DG_PRIM2       *prim ;

	if(work->world == NULL)
	    return -1 ;

	prim = work->prim = GM_MakePrim2(PRIM_TYPE,N_PRIMS,N_VERTS) ;
	if(prim == NULL)
	    return -1 ;

//	tex = DG_GetTexture( GV_StrCode("hosi_msk")) ;
	tex = DG_GetTexture(12885879) ;
	if(tex== NULL)
	    return -1 ;
	DG_ConfigPrim2Tex(prim,tex) ;
	DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA(0,2,0,1,0x00)) ;
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA(0,2,0,1,0x00) ;

	work->map = map;

	/* スクラッチパット上で初期化したデータを実際のワークにコピーする */
	Init_Scr(work,tex) ;
	_BigMemCopy(prim->pos[0],MEM_ADDR1,sizeof(FVECTOR),N_VERTS) ;
	_BigMemCopy(prim->uvrgb[0],MEM_ADDR2,sizeof(DG_PRIM2_UVRGBWH),N_VERTS) ;
	_BigMemCopy(prim->pos[1],MEM_ADDR1,sizeof(FVECTOR),N_VERTS) ;
	_BigMemCopy(prim->uvrgb[1],MEM_ADDR2,sizeof(DG_PRIM2_UVRGBWH),N_VERTS) ;
	_BigMemCopy(work->base,MEM_ADDR3,sizeof(FVECTOR),N_VERTS) ;
	work->act = Act_FIRST ;
	
	/* シグナル設定 */
	GV_SetActorSignalFunc( work, ReceiveSignal ) ;

	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
//		void	CPE_SetCamPoseSquare( DG_PRIM2 *prim, FMATRIX *world ) ;
//		CPE_SetCamPoseSquare( prim, work->world ) ;
		void	VRFUNC_CPE_SetCamPoseSquare( DG_PRIM2 *prim, FMATRIX *world ) ;
		VRFUNC_CPE_SetCamPoseSquare( prim, work->world ) ;
	}

	return (0);
}

/* ---------------------------------------------------------------- */
void *NewPiyori(FMATRIX *world, int *map )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) 
	{
	    work->world = world ;
	    work->count  = 1800;

	    GV_SetActor( &( work->actor ), Act, Die ) ;
	    GV_ActorEX( &work->actor ) ;
	    if ( GetResources( work, map ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	    }
	}
	return (void *)work ;
}
