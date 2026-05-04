//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	vr_cube.c
	ＶＲステージ 箱的
	chara	ＶＲキューブ[NewVRCube] 
	2002/02/08 K.Sigeno
	$Id: vr_cube.c,v 1.1.1.3 2002/11/19 11:49:57 Yoshizawa1 Exp $
*/
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#include "gameheader.h"
#include "libutl.h"
#include "korekado/enemy/enemy.h"
#include "korekado/enemy/enemy.x"

#include "vr.h"
#include "trg_cmd.h"

#define		BODY_SIZE_X		(500.0f)
#define		BODY_SIZE_Y		(500.0f)
#define		BODY_SIZE_Z		(500.0f)
#define		Y_SHIFT			(500.0f)
#define		BODY_SIZE		(500.0f)


#define		LV1_NUM	(1)
#define		LV2_NUM	(24)
#define		LV3_NUM	(1)


/* ワーク */
typedef	struct _Work {
	GV_ACT_EX	actor ;
	int			name ;
	int			where ;
	DG_OBJS		*objs ;
	FMATRIX		lights[2] ;
	short		mode ;
	short		inflag ;
	int			type ;
	int			finish ;

	TARGET			b_trg;
	TARGET			*core_trg01;
	TARGET			*core_trg02;
	TARGET			*core_trg03;

	void			*str_work ;

	u_char			trg_lv1_num ;
	u_char			trg_lv2_num ;
	u_char			trg_lv3_num ;
//	u_char			trg01_dmg ;

//	u_char			trg02_dmg ;
//	u_char			trg03_dmg ;
	u_char			pad ;


	POWER_TARGET	b_power;
	int	vital ;
	FVECTOR		pos ;
	HZX_D_SEGMENT	*segment[4];
	HZX_D_FLOOR		*segfloor[2] ;
	int			proc_id ;

	DG_OBJS		*hlt_objs ;
	FMATRIX		hlt_lights[2] ;
	CVECTOR		core_rgb;
	int			core_mdl ;
	int			ef_mdl_code ;
	int			amb_cnt ;
} Work ;
enum {
	ST_FLAG = 0x01 ,
};


#ifdef DEBUG_MODE
extern void PosBox(FVECTOR * ,float ,SVECTOR * );
extern void RouteView(ROUTENAVI *) ;

#endif


#define	ONE_PIECE_FLAG	(DG_FLAG_SHADE|DG_FLAG_ONEPIECE)
#define	MULTI_FLAG		(DG_FLAG_SHADE|DG_FLAG_FINISHCALC)


#define	SEG_FLAG	(HZX_SEG_NO_BULLET)
#define	FLR_FLAG	(HZX_FLOOR_NO_BULLET)

//#define TRG_VIEW	(1)

//ポイント時
#if 0
static CVECTOR	VR_HLT_DEF	= {5,48,32};
static CVECTOR	VR_HLT_EXP	= {48,16,8};
static CVECTOR	VR_HLT_TAB	= {0,80,96};
#endif
//コア
static CVECTOR	VR_CORE_DEF	= {10,96,64};
static CVECTOR	VR_CORE_EXP	= {90,32,64};
static CVECTOR	VR_CORE_TAB	= {10,160,192};


#include "inc_cube.c"

static void Act(Work *work)
{
	FVECTOR		t_pos ;
//	SVECTOR	rgb,rot ;
	int i;
	t_pos = work->pos ;
	t_pos.vy += Y_SHIFT ;
	GM_MoveTarget( &work->b_trg, &t_pos ) ;

	if(work->vital >= VR_CUBE_LIFE_MAX){
		if(work->inflag & ST_FLAG ){
#if 1
			VR_AllTrgSkip(work) ;
			work->b_trg.class &= ~TARGET_LOCKON ;
#else
			GM_SetTargetSize( &work->b_trg, &DG_ZeroVector ) ;
			work->b_trg.class |= TARGET_SKIP ;
#endif
			VR_ClearTarget() ;
			if(work->segment[0] != NULL) {
				for (i=0;i<4;i++){
					HZX_RemoveDynamicSegment(work->segment[i]);
					work->segment[i] = NULL ;
				}
			}
			if(work->segfloor[0] != NULL) {
				for(i=0;i<2;i++){
					HZX_RemoveDynamicFloor( work->segfloor[i] ) ;
					work->segfloor[i] = NULL ;
				}
			}
			if(work->type & VR_TARGET_TYPE_BLAST){
extern	void	*NewBlast3( FVECTOR *, int, int, int, int, int, int, int ) ;

				NewBlast3( &work->pos, BOTH_SIDE, 2000, 
					2000, 20, FNT_BLAST, WP_Grenade, 
					BLAST_TYPE_NO_NOISE|BLAST_TYPE_WALLTHROUGH|BLAST_TYPE_NO_PLAYER) ;
			}else {
				CallBreakObj(work->ef_mdl_code ,&work->objs->world) ;
//				CallBreakObj(work) ;
			}
//			VR_SetComboCnt() ;
			if(work->proc_id != 0){
//				GM_ExecProc( work->proc_id, NULL );
				VR_ExecProcName(work->proc_id,work->name) ;
				work->proc_id= 0;
			}
			DG_InvisibleObjs(work->objs) ;
			DG_InvisibleObjs(work->hlt_objs) ;
			VR_DestoryComboCheck(work->finish,&t_pos) ;
			work->inflag &= ~ST_FLAG ;
		}
		return ;
	} 

	if(work->amb_cnt>0){
		work->amb_cnt--;
		if((work->amb_cnt&3)<2){
			work->lights[1].m[ 3 ][ 0 ] = (float) 255 ;
			work->lights[1].m[ 3 ][ 1 ] = (float) 255 ;
			work->lights[1].m[ 3 ][ 2 ] = (float) 255 ;
		}else {
			work->lights[1].m[ 3 ][ 0 ] = (float) 16 ;
			work->lights[1].m[ 3 ][ 1 ] = (float) 16 ;
			work->lights[1].m[ 3 ][ 2 ] = (float) 16 ;
		}
	}else {
		DG_GetLightMatrix( &work->pos, work->lights );
	}

//	DG_GetLightMatrix( &work->pos, work->lights );

	{
		float fade ;
		int tmp ;
		tmp = GM_StagePlayTime & 127 ;
		if(tmp> 63 ) {
			tmp = 127 - tmp ;
		}
		fade = (float) tmp / 63.0f ;
		work->hlt_lights[1].m[ 3 ][ 0 ] = (float) work->core_rgb.r *fade;
		work->hlt_lights[1].m[ 3 ][ 1 ] = (float) work->core_rgb.g *fade;
		work->hlt_lights[1].m[ 3 ][ 2 ] = (float) work->core_rgb.b *fade;
	}


}
static void Die(Work *work)
{
	int i;
	DG_DequeueObjs( work->objs ) ;
	DG_FreeObjs( work->objs );
	DG_DequeueObjs( work->hlt_objs ) ;
	DG_FreeObjs( work->hlt_objs );

	GM_FreeTarget( &work->b_trg ) ;
	if(work->segment[0] != NULL) {
		for (i=0;i<4;i++){
			HZX_RemoveDynamicSegment(work->segment[i]);
			work->segment[i] = NULL ;
		}
	}
	if(work->segfloor[0] != NULL) {
		for(i=0;i<2;i++){
			HZX_RemoveDynamicFloor( work->segfloor[i] ) ;
			work->segfloor[i] = NULL ;
		}
	}
	GV_Free(work->core_trg01) ;
	GV_Free(work->core_trg02) ;
	GV_Free(work->core_trg03) ;
}
//#define TRG_SIZE_SHIFT (100.0f)
#define TRG_SIZE_SHIFT (0.0f)



static void SetTarget(work)
Work *work;
{
//	int flag,map;
    IVECTOR		iv[ 4 ];
    HZX_GROUP_ID	hzx_id, id;
    int			gn;    
//	float size_x,size_z ;
//	FVECTOR	b_size = { BODY_SIZE_X+TRG_SIZE_SHIFT, BODY_SIZE_Y+TRG_SIZE_SHIFT, BODY_SIZE_Z+TRG_SIZE_SHIFT ,1.0f} ;


	/*ターゲット設定*/
	SetTargetCube(work) ;
//	SetTargetCube2(work) ;

    id = GM_GetHzxGroupID( work->where );
    gn = GV_GetNo( id );
    hzx_id = GV_GetBit( gn );

	/*壁作成*/
    iv[0].vx = (int)work->pos.vx - BODY_SIZE_X;
    iv[0].vy = (int)work->pos.vy ;
    iv[0].vz = (int)work->pos.vz - BODY_SIZE_Z;
    iv[0].vw = (int)BODY_SIZE_Y*2;
    
    iv[1].vx = (int)work->pos.vx + BODY_SIZE_X ;
    iv[1].vy = (int)work->pos.vy ;
    iv[1].vz = (int)work->pos.vz - BODY_SIZE_Z;
    iv[1].vw = (int)BODY_SIZE_Y*2;
    work->segment[0] = HZX_AddDynamicSegment( hzx_id, &iv[0], &iv[1], SEG_FLAG );

    iv[0].vx = (int)work->pos.vx + BODY_SIZE_X;
    iv[0].vy = (int)work->pos.vy ;
    iv[0].vz = (int)work->pos.vz - BODY_SIZE_Z;
    iv[0].vw = (int)BODY_SIZE_Y*2;
    
    iv[1].vx = (int)work->pos.vx + BODY_SIZE_X ;
    iv[1].vy = (int)work->pos.vy ;
    iv[1].vz = (int)work->pos.vz + BODY_SIZE_Z;
    iv[1].vw = (int)BODY_SIZE_Y*2;
    work->segment[1] = HZX_AddDynamicSegment( hzx_id, &iv[0], &iv[1], SEG_FLAG );

    iv[0].vx = (int)work->pos.vx + BODY_SIZE_X;
    iv[0].vy = (int)work->pos.vy ;
    iv[0].vz = (int)work->pos.vz + BODY_SIZE_Z;
    iv[0].vw = (int)BODY_SIZE_Y*2;
    
    iv[1].vx = (int)work->pos.vx - BODY_SIZE_X ;
    iv[1].vy = (int)work->pos.vy ;
    iv[1].vz = (int)work->pos.vz + BODY_SIZE_Z;
    iv[1].vw = (int)BODY_SIZE_Y*2;
    work->segment[2] = HZX_AddDynamicSegment( hzx_id, &iv[0], &iv[1], SEG_FLAG );

    iv[0].vx = (int)work->pos.vx - BODY_SIZE_X;
    iv[0].vy = (int)work->pos.vy ;
    iv[0].vz = (int)work->pos.vz + BODY_SIZE_Z;
    iv[0].vw = (int)BODY_SIZE_Y*2;
    
    iv[1].vx = (int)work->pos.vx - BODY_SIZE_X ;
    iv[1].vy = (int)work->pos.vy ;
    iv[1].vz = (int)work->pos.vz - BODY_SIZE_Z;
    iv[1].vw = (int)BODY_SIZE_Y*2;
    work->segment[3] = HZX_AddDynamicSegment( hzx_id, &iv[0], &iv[1], SEG_FLAG );

//HZX_D_FLOOR	*HZX_AddDynamicFloorF( HZX_GROUP_ID id, FVECTOR *p, int n, u_int atr )
//	HZX_D_FLOOR		*segfloor[2] ;
//HZX_D_FLOOR	*HZX_AddDynamicFloor( HZX_GROUP_ID id, IVECTOR *p1, IVECTOR *p2,
//								 IVECTOR *p3, IVECTOR *p4, int n, u_int atr )

    iv[0].vx = (int)work->pos.vx - BODY_SIZE_X;
    iv[0].vy = (int)work->pos.vy ;
    iv[0].vz = (int)work->pos.vz - BODY_SIZE_Z;
    
    iv[1].vx = (int)work->pos.vx + BODY_SIZE_X ;
    iv[1].vy = (int)work->pos.vy ;
    iv[1].vz = (int)work->pos.vz - BODY_SIZE_Z;

    iv[2].vx = (int)work->pos.vx + BODY_SIZE_X ;
    iv[2].vy = (int)work->pos.vy ;
    iv[2].vz = (int)work->pos.vz + BODY_SIZE_Z;

    iv[3].vx = (int)work->pos.vx - BODY_SIZE_X ;
    iv[3].vy = (int)work->pos.vy ;
    iv[3].vz = (int)work->pos.vz + BODY_SIZE_Z;

	work->segfloor[0] = HZX_AddDynamicFloor( hzx_id , &iv[0], &iv[1],&iv[2], &iv[3],4, FLR_FLAG ) ;

    iv[0].vx = (int)work->pos.vx - BODY_SIZE_X;
    iv[0].vy = (int)work->pos.vy + (int)BODY_SIZE_Y*2;
    iv[0].vz = (int)work->pos.vz - BODY_SIZE_Z;
    
    iv[1].vx = (int)work->pos.vx + BODY_SIZE_X ;
    iv[1].vy = (int)work->pos.vy + (int)BODY_SIZE_Y*2;
    iv[1].vz = (int)work->pos.vz - BODY_SIZE_Z;

    iv[2].vx = (int)work->pos.vx + BODY_SIZE_X ;
    iv[2].vy = (int)work->pos.vy + (int)BODY_SIZE_Y*2;
    iv[2].vz = (int)work->pos.vz + BODY_SIZE_Z;

    iv[3].vx = (int)work->pos.vx - BODY_SIZE_X ;
    iv[3].vy = (int)work->pos.vy + (int)BODY_SIZE_Y*2;
    iv[3].vz = (int)work->pos.vz + BODY_SIZE_Z;

	work->segfloor[1] = HZX_AddDynamicFloor( hzx_id , &iv[0], &iv[1],&iv[2], &iv[3],4, FLR_FLAG ) ;

}

static int GetResources(Work *work, int name, int where)
{
	DG_DEF  *def=NULL,*hlt_def ;
	int flag;
	FVECTOR	tmp ;
	work->name = name ;
	work->where = where ;
	work->vital = 0 ;
	work->inflag = ST_FLAG ;
	if ( GCL_GetOption( 'f' ) != NULL ) {
		work->type = GCL_GetNextInt() ;
	}else {
		work->type = 0 ;
	}

	if ( GCL_GetOption( 'p' ) != NULL ) {
		work->pos.vx = (float)GCL_GetNextInt() ;
		work->pos.vy = (float)GCL_GetNextInt() ;
		work->pos.vz = (float)GCL_GetNextInt() ;
	}

	flag = ONE_PIECE_FLAG ;
	work->ef_mdl_code = MDL_VR_TRG_CUB_RGB ;
	if(work->type == VR_TARGET_TYPE_NORMAL){
		def = (DG_DEF*) GV_GetCache( GV_CacheID( MDL_VR_TRG_CUBE_02, 'k' ) ) ;
//		def = (DG_DEF*) GV_GetCache( GV_CacheID( MDL_VR_TRG_CUBE_03, 'k' ) ) ;
		work->core_rgb = VR_CORE_DEF ;
	}else if(work->type & VR_TARGET_TYPE_NG){
		def = (DG_DEF*) GV_GetCache( GV_CacheID( MDL_VR_TRG_CUBE_TAB, 'k' ) ) ;
		work->core_rgb = VR_CORE_TAB ;
	}else if(work->type & VR_TARGET_TYPE_BLAST){
		def = (DG_DEF*) GV_GetCache( GV_CacheID( MDL_VR_TRG_CUBE_EXP, 'k' ) ) ;
		work->core_rgb = VR_CORE_EXP ;
	}
	hlt_def = (DG_DEF*) GV_GetCache( GV_CacheID( MDL_VR_TRG_CUBE_HLT, 'k' ) ) ;
	ASSERT(def != NULL) ;
	work->objs = DG_MakeObjs( def, flag, DG_CHANL_MAIN ) ;
	work->hlt_objs = DG_MakeObjs( hlt_def, flag, DG_CHANL_MAIN ) ;

	DG_QueueObjs( work->objs ) ;
	DG_QueueObjs( work->hlt_objs ) ;
	DG_SetLightMatrix( work->objs, work->lights );
	DG_SetLightMatrix( work->hlt_objs, work->hlt_lights );
	{
		work->hlt_lights[1].m[ 3 ][ 0 ] = (float) work->core_rgb.r;
		work->hlt_lights[1].m[ 3 ][ 1 ] = (float) work->core_rgb.g;
		work->hlt_lights[1].m[ 3 ][ 2 ] = (float) work->core_rgb.b;
//		vr_setflagall_objs(work->hlt_objs,DG_FLAG_INVISIBLE) ;
	}


	DG_COPY_VEC( &tmp, &work->pos ) ;
	tmp.vy += BODY_SIZE_Y ;
	DG_SetPos2( &tmp, &DG_ZeroSVector ) ;
	DG_PutObjs( work->objs );
	DG_PutObjs( work->hlt_objs );
	GM_GroupObjs( work->objs, where ) ;
	GM_GroupObjs( work->hlt_objs, where ) ;

	SetTarget(work) ;
	VRMoveTrgCube(work,&tmp) ;

	if(!(work->type & VR_TARGET_TYPE_NG)){
		VR_AddTarget() ;
	}
	if ( GCL_GetOption( 'c' ) != NULL ){
		if( GCL_NextStr() != NULL ){
			work->proc_id  = GCL_GetNextInt();
		}else {
			work->proc_id  = 0;
		}
	}

	{

//extern void *NewSIG_3DPOSPrint(FMATRIX * ,int ,int * ) ;
		NewSIG_3DPOSPrint(&work->objs->world,VR_TRG_TYPE_MOVE_CUBE,&work->objs->flag,
		work->objs->bound_max.vy+150.0f) ;
	}
	work->str_work= NULL ;
	GV_SetActorChild( work,work->str_work = (void *)NewSIG_3DPrintf(HIT_PRINT_SHIFT) );
	return 1;
}

void *NewVRCube( name , where )
int	name ;
int	where ;
{
	Work *work ;
	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(!GetResources( work,name,where )){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
