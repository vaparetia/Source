//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	残像のエフェクト 
	2000/10/18 S.Okajima
	$Id: afterimage.c,v 1.1.1.3 2002/11/19 11:47:42 Yoshizawa1 Exp $
*/

/* inf_cam.best_0 とは違い サイズの決定をカメラからベースの方向ベクトルと、
   ベースの移動ベクトルの外せきで求めている 
*/
/* いままでと違いデータの古い順から 並んでいる */
#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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
#include	"camera.h"
#include	"../etc/ok_util.h"

/* ---------------------------------------------------------------- */
	/*
		補助マクロ
	*/
#define  FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

#define  PRIM_TYPE	   (DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA)
#define  N_PRIMS		10
#define  N_VERTS		4
#define  N_NUM		  (N_PRIMS * N_VERTS)

/* memory  0x0000 ～ 0x0399 */
#define  MEM_POS		((void *)((char*)SCRPAD_ADDR + 0x0000))
/* memory  0x0400 ～ 0x0799 */
#define  MEM_UVRGB	  ((void *)((char*)MEM_POS + sizeof(FVECTOR) * N_NUM))
/* memory  0x0800 ～ 0x0909 */
#define  MEM_BASE	   ((void *)((char*)MEM_UVRGB + sizeof(DG_PRIM2_UVRGB) * N_NUM)) 

/* memory 0x3000 ～ 0x3099 */
#define  MEM_MOVE	   ((void *)((char*)SCRPAD_ADDR + 0x3000))
/* memory 0x3100 ～ 0x3209 */
#define  MEM_HEIGHT	  ((void *)((char*)MEM_MOVE + sizeof(FVECTOR) * (N_PRIMS + 1)))
/* memory 0x3210 ～ 0x3429 */
#define  MEM_TMP_POS   ((void *)((char*)MEM_HEIGHT + sizeof(FVECTOR) * (N_PRIMS + 1)))

#define  SIZE		  5.0F

extern  void SP_MakeTable(int n,FVECTOR   *table,FVECTOR   *pos) ;

typedef struct
{
	GV_ACT_EX	   actor ;
	
	DG_PRIM2	 *prim ;
	FVECTOR	  base[N_PRIMS + 1] ;
	FVECTOR	  shift ;
	
	FMATRIX	  *world ;
	int		  *mode ;
	int		  *group_id ;
	int		  count ;
} Work ;

enum {
	INVISIBLE,
	OPEN,
	CLOSE
} ;


extern  void _BigScrCopy( void *dst, void *src, int size, int num );
extern  void _BigMemCopy( void *dst, void *src, int size, int num );

static   int   Check_Pos(FVECTOR  *p, FVECTOR  *s )
{
	if(p->vx != s->vx) return 1 ;
	if(p->vy != s->vy) return 1 ;
	if(p->vz != s->vz) return 1 ;
	return 0 ;
}

static  void  Act_ScrPad(Work *work)
{
	FVECTOR		   *pos,*base, *move, *size, *tmp_pos ;
	DG_PRIM2_UVRGB	*uvrgb ;
	DG_PRIM2		  *prim ;
	GM_CameraSet	  *cam ;
	FVECTOR		   cam_dir ;
	int			   i ;
	FVECTOR			fvtemp ;

	/* 位置情報を更新 */
	base = MEM_BASE ;
	if(work->count == 0){
		DG_SetPos(work->world) ;
		for(i = 0 ; i < (N_PRIMS + 1) ; i++){
			DG_PutVector(&work->shift,base,1) ;
			base->vw = 0.0F ;
			base ++ ;
		}
		work->count ++ ;
		DG_VisiblePrim2(work->prim) ;
	}else{
		/* データをずらす */
		for(i = 0; i < N_PRIMS ; i++){
			base[0] = base[1] ;
			base ++ ;
		}
		DG_SetPos(work->world) ;
		DG_PutVector(&work->shift,base,1) ;
	}

	prim = work->prim ;
	if(work->mode != NULL){
		if(( *(work->mode) == -1)){
			DG_InvisiblePrim2(work->prim) ;
			GV_DestroyActor(work) ;
			return  ;
		}
		if(*(work->mode) == CLOSE){
			DG_SetPos(work->world) ;
			DG_PutVector(&work->shift,&fvtemp,1) ;
			for(i = 0; i < N_PRIMS + 1; i++) {
				DG_COPY_VEC( &work->base[ i ], &fvtemp );
			}
			base->vw = 1.0F ;
		}else{
			base->vw = 0.0F ;
		}
	}else{
		GV_DestroyActor(work) ;
		return;
	}

/* ------------------------------------------------------------ */
	/* それぞれの位置を求めるサイズの向きを求める */
	
	/* 移動ベクトルを求める */
	move = MEM_MOVE ;
	base = MEM_BASE ;
	for(i = 0; i < N_PRIMS; i++){
		/* 移動ベクトルを求める */
		_sceVu0SubVector(move,&base[1],&base[0]) ;
		move ++ ;
		base ++ ;
	}

	/* dir と moveの外せきを求めて１５倍 から 位置を求める */ 
	move = MEM_MOVE ;
	size = MEM_HEIGHT ;

	/* カメラの方向ベクトル */
	cam  = GM_GetCurrentCameraSet(0) ;
	_sceVu0SubVector(&cam_dir,&cam->target, &cam->position) ;

	for(i = 0; i < N_PRIMS; i++){
		/* dir と moveの外せきのベクトルで 長さがSIZEを求める */
#ifdef BP_PSX2_ASM		//// 下の"lqc2 vf2,0(%0) の意味がわからない。。。M.Kobayashi
		asm volatile ( "
						  lqc2		 vf2,0(%0)
						  lqc2		 vf3,0(%1)
						  lqc2		 vf4,0(%2)
						  qmtc2.i	  %3,vf7 
						  qmtc2.i	  %4,vf8
						  vopmula.xyz  ACCxyz, vf3xyz, vf4xyz
						  vopmsub.xyz  vf2xyz, vf4xyz, vf3xyz	# 外積
						  vmul.xyz	 vf6xyz, vf2xyz, vf2xyz	# size * size
						  vmulax.x	 ACCx, vf7x, vf6x
						  vmadday.x	ACCx, vf7x, vf6y
						  vmaddz.x	 vf6x, vf7x, vf6z
						  vnop 
						  vrsqrt	   Q, vf7x, vf6x			   
						  vnop 
						  vwaitq
						  vmulq.x	  vf7x, vf8x, Q
						  vnop
						  vmulx.xyz	vf2xyz, vf2xyz, vf7x		# end
						  sqc2		 vf2,0(%0)				 
					   "::
							 "r"(size),			   /* vf2 */
							 "r"(&cam_dir),			/* vf3 */
							 "r"(move),				/* vf4 */
							 "r"(1.0F),			   /* vf7 */
					 "r"(SIZE)				 /* vf8 */		
											   /* vf6 */
			  : "memory"											 
		) ;
#else
		{
			FVECTOR	vecOp;
			_sceVu0OuterProduct( &vecOp, &cam_dir, move );
			_sceVu0Normalize( &vecOp, &vecOp );
			_sceVu0ScaleVectorXYZ( size, &vecOp, SIZE );
		}
		
#endif		
		move ++ ;
		size ++ ;
	}

	/* 新しいデータを最後に入れる */
	move -- ;
#ifdef BP_PSX2_ASM	
	asm volatile ( "
						  lqc2		 vf2,0(%0)
						  lqc2		 vf3,0(%1)
						  lqc2		 vf4,0(%2)
						  qmtc2.i	  %3,vf7 
						  qmtc2.i	  %4,vf8
						  vopmula.xyz  ACCxyz, vf3xyz, vf4xyz
						  vopmsub.xyz  vf2xyz, vf4xyz, vf3xyz	# 外積
						  vmul.xyz	 vf6xyz, vf2xyz, vf2xyz	# size * size
						  vmulax.x	 ACCx, vf7x, vf6x
						  vmadday.x	ACCx, vf7x, vf6y
						  vmaddz.x	 vf6x, vf7x, vf6z
						  vnop 
						  vrsqrt	   Q, vf7x, vf6x			   
						  vnop 
						  vwaitq
						  vmulq.x	  vf7x, vf8x, Q
						  vnop
						  vmulx.xyz	vf2xyz, vf2xyz, vf7x		# end
						  sqc2		 vf2,0(%0)				 
					   "::
							 "r"(size),			   /* vf2 */
							 "r"(&cam_dir),			/* vf3 */
							 "r"(move),				/* vf4 */
							 "r"(1.0F),			   /* vf7 */
					 "r"(SIZE)				 /* vf8 */		
											   /* vf6 */
			  : "memory"
	) ;
#else
		{
			FVECTOR	vecOp;
			_sceVu0OuterProduct( &vecOp, &cam_dir, move );
			_sceVu0Normalize( &vecOp, &vecOp );
			_sceVu0ScaleVectorXYZ( size, &vecOp, SIZE );
		}
#endif	
	/* 次に位置を決定 */
	tmp_pos = MEM_TMP_POS ;
	base	= MEM_BASE ;
	size	= MEM_HEIGHT ;
	for(i = 0; i < (N_PRIMS + 1) ; i++){
		_sceVu0AddVector(&tmp_pos[0],base,size) ;
		_sceVu0SubVector(&tmp_pos[1],base,size) ;
		tmp_pos += 2 ;
		base ++ ;
		size ++ ;
	}
	/* このベースを元にデータ構築 */
	/* base の新しい点とその次の点だけ見る */
	pos	 = MEM_POS ;
	tmp_pos = MEM_TMP_POS ;
	base	= MEM_BASE ;
	uvrgb   = MEM_UVRGB ;
	for(i = 0; i < N_PRIMS; i++){
		pos[0] = tmp_pos[0] ;
		pos[1] = tmp_pos[1] ;
		pos[2] = tmp_pos[2] ;
		pos[3] = tmp_pos[3] ;
		if(Check_Pos(&base[0],&base[1]) && (base[1].vw == 1.0F)){
			if(base[0].vw != 0.0F){
				uvrgb[0].a = uvrgb[1].a = 8 * i ;
			}else{
				uvrgb[0].a = 0 ;
			}
			uvrgb[2].a = uvrgb[3].a = 8 * (i + 1) ;
		}else{
			uvrgb[0].a = uvrgb[1].a = 0.0F;
			uvrgb[2].a = uvrgb[3].a = 0.0F ;
		}
		pos	 += 4 ;
		uvrgb   += 4 ;
		tmp_pos += 2 ;
		base ++ ;
	}
}

static  void  Act(Work *work)
{
	DG_PRIM2		*prim ;

	prim = work->prim ;

	/* １つ前をコピー */
	_BigScrCopy(MEM_BASE, work->base,sizeof(FVECTOR), (N_PRIMS + 1)) ;
	_BigScrCopy(MEM_UVRGB, prim->uvrgb[prim->buffer_clock],sizeof(DG_PRIM2_UVRGB), N_NUM) ;

	/* スイッチ切り替え */
	DG_SwitchBuffPrim2(prim) ;

	Act_ScrPad(work) ;
	_BigScrCopy(work->base,MEM_BASE,sizeof(FVECTOR), (N_PRIMS + 1)) ;
	_BigScrCopy(prim->pos[prim->buffer_clock],MEM_POS,sizeof(FVECTOR), N_NUM) ;
	_BigScrCopy(prim->uvrgb[prim->buffer_clock],MEM_UVRGB,sizeof(DG_PRIM2_UVRGB),N_NUM) ;
}

static  void  Die(Work *work)
{
	work->prim = OK_FreePrim2( work->prim ) ;
}

/* ----------------------------------------------------- */
static  void Init_ScrPad(DG_TEX *tex)
{
	FVECTOR			  *pos ;
	DG_PRIM2_UVRGB	  *uvrgb;
	int				  i,j ;

	uvrgb = MEM_UVRGB ;
	pos   = MEM_POS ;
	for(i = 0; i < N_PRIMS; i++){
		*pos = DG_ZeroVector ;
		uvrgb[0].u = FTOI12(0.0F * tex->u_scale + tex->u_offset) ;
		uvrgb[0].v = FTOI12(0.0F * tex->v_scale + tex->v_offset) ;
		uvrgb[0].q = 4096 ;
		uvrgb[0].f = 0x8fff ;
		uvrgb[1].u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb[1].v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb[1].q = 4096 ;
		uvrgb[1].f = 0x8fff ;
		uvrgb[2].u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb[2].v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb[2].q = 4096 ;
		uvrgb[2].f = 0x0fff ;
		uvrgb[3].u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb[3].v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb[3].q = 4096 ;
		uvrgb[3].f = 0x0fff ;

		for(j = 0; j < 4 ; j++){
			uvrgb[j].r = 128 ;
			uvrgb[j].g = 20 ;
			uvrgb[j].b = 20 ;
			uvrgb[j].a = 0 ;
		}
		uvrgb += N_VERTS ;
	}
}

static  int  GetResources(Work *work, FMATRIX *world, FVECTOR *shift, int *mode, int *group_id)
{
	DG_PRIM2	*prim ;
	DG_TEX	  *tex ;
	FVECTOR	*base ;
	int		i ;

	/* 四角形ポリゴンの実験 */
	prim = work->prim = GM_MakePrim2(PRIM_TYPE, N_PRIMS, N_VERTS) ;
	if(prim == NULL)
	return -1 ;
	/* テクスチャを取得 */
	tex = DG_GetTexture( 11561081 /*"plasma_msk"*/ ) ;
	if(tex == NULL) return -1 ;
	DG_ConfigPrim2Tex(prim, tex) ;
	DG_SetPrim2Alpha(prim,SCE_GS_SET_ALPHA(0,2,0,1,0x00)) ;
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA(0,2,0,1,0x00) ;
	prim->group_id = *group_id ;
	work->world	= world ;
	work->mode	 = mode ;
	work->group_id = group_id ;
	work->shift = *shift ;
	if(work->mode == NULL){
		DG_VisiblePrim2(prim) ;
	}else{
		if(*(work->mode)){
			DG_VisiblePrim2(prim) ;
		}else{
			DG_InvisiblePrim2(prim) ;
		}
	}

	/* base初期化 */
	DG_SetPos(world) ;
	base = work->base ;
	for(i = 0 ; i < (N_PRIMS + 1) ; i++){
		DG_PutVector(shift,base,1) ;
		base ++ ;
	}

	/* データ初期化 */
	Init_ScrPad(tex) ;

	/* メモリーを移す */
	_BigMemCopy(prim->pos[0],MEM_POS,sizeof(FVECTOR),N_NUM) ;
	_BigMemCopy(prim->uvrgb[0],MEM_UVRGB,sizeof(DG_PRIM2_UVRGB),N_NUM) ;
	_BigMemCopy(prim->pos[1],MEM_POS,sizeof(FVECTOR),N_NUM) ;
	_BigMemCopy(prim->uvrgb[1],MEM_UVRGB,sizeof(DG_PRIM2_UVRGB),N_NUM) ;
	work->count = 0 ;
	DG_InvisiblePrim2(prim) ;
	return 0 ;
}


/* --------------------------------------------------------- */
/* 残像 */
void  *AfterImage(FMATRIX *world,FVECTOR *shift, int *mode, int *group_id)
{
	Work *work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work )) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, world, shift, mode, group_id) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
