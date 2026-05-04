//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	vr_wall.c
	ＶＲステージ破壊壁
	chara	ＶＲ壁[NewVRWall] 
	2002/02/05 K.Sigeno
	$Id: vr_wall.c,v 1.1.1.3 2002/11/19 11:49:58 Yoshizawa1 Exp $
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

//#define		NULL_OBJ	(1)

#define		BODY_SIZE_X		(1000.0f)
#define		BODY_SIZE_Y		(1000.0f)
#define		BODY_SIZE_Z		(50.0f)
#define		Y_SHIFT			(1000.0f)
/* ワーク */
typedef	struct _Work {
	GV_ACT_EX	actor ;
	DG_OBJS		*objs ;
	FMATRIX		lights[2] ;
	CONTROL_NOEVT ctrl ;	//シナリオからのサーチのためだけに使う
	OL_COLOR		ol_col ;
	void		*str_work ;
	int			name ;
	int			where ;
	int			ef_mdl_code ;
	int			finish ;
	short			dir ;
	short		inflag ;
	int			type ;
	TARGET			b_trg;
	TARGET			b_trg2;
	POWER_TARGET	b_power;
	POWER_TARGET	b_power2;
	int	vital ;
	FVECTOR		pos ;
	HZX_D_SEGMENT	*segment;
	int			proc_id ;
	int			amb_cnt ;

} Work ;

enum {
	ST_ACT = 0x01,
};

#ifdef DEBUG_MODE
extern void PosBox(FVECTOR * ,float ,SVECTOR * );

#endif


#define	ONE_PIECE_FLAG	(DG_FLAG_SHADE|DG_FLAG_ONEPIECE)
#define	MULTI_FLAG		(DG_FLAG_SHADE|DG_FLAG_FINISHCALC)


#define	SEG_FLAG	(HZX_SEG_NO_BULLET)
//#define	SEG_FLAG	(HZX_SEG_NO_PLAYER)

#define OL_SPEED (6)

static void VR_TRG_OL_COL(Work *work){

	if(work->ol_col.col[0].cd >OL_SPEED) {
		work->ol_col.col[0].cd -= OL_SPEED ;
		GV_CallChildSignalFunc( work,OL_SIG_SetColor, (int) &work->ol_col ) ;
	}else if(work->ol_col.col[0].cd >0) {
		work->ol_col.col[0].cd = 0 ;
		GV_CallChildSignalFunc( work,OL_SIG_SetColor, (int) &work->ol_col ) ;
	}
}

static void Act(Work *work)
{
	FVECTOR		t_pos ;
//	SVECTOR	rgb ;

	if(work->vital >= VR_WALL_LIFE_MAX){
		VR_TRG_OL_COL(work) ;
		if(work->inflag & ST_ACT ){
			work->ol_col.col[0].cd = 127 ;
			GM_SetTargetSize( &work->b_trg, &DG_ZeroVector ) ;
			GM_SetTargetSize( &work->b_trg2, &DG_ZeroVector ) ;
			work->b_trg.class |= TARGET_SKIP ;
			work->b_trg.class &= ~TARGET_LOCKON ;
			work->b_trg2.class |= TARGET_SKIP ;
			work->b_trg2.class &= ~TARGET_LOCKON ;

			if(work->segment != NULL) {
				HZX_RemoveDynamicSegment(work->segment);
				work->segment = NULL ;
			}
			if(work->type & VR_TARGET_TYPE_BLAST){
extern	void	*NewBlast3( FVECTOR *, int, int, int, int, int, int, int ) ;

				NewBlast3( &work->pos, BOTH_SIDE, 2000, 2000, 20, FNT_BLAST, WP_Grenade, 
					BLAST_TYPE_NO_NOISE|BLAST_TYPE_WALLTHROUGH|BLAST_TYPE_NO_PLAYER) ;
			}
//			VR_SetComboCnt() ;
			if(work->proc_id != 0){
//				GM_ExecProc( work->proc_id, NULL );
				VR_ExecProcName(work->proc_id,work->name) ;
				work->proc_id= 0;
			}
			DG_InvisibleObjs(work->objs) ;
			CallBreakObj(work->ef_mdl_code,&work->objs->world) ;
//			VR_DestoryComboCheck(work->finish) ;
			GM_FreeControl( (CONTROL *) &work->ctrl ) ;
			GM_SeSetMode(SD_A_V_CLASHW,&work->pos,GM_SEMODE_BOMB) ;

 //壁板破壊（壊さなくても構わないモノ用）

			work->inflag &= ~ST_ACT ;
		}
		return ;
	} 

//	DG_GetLightMatrix( &work->pos, work->lights );

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

	t_pos = work->pos ;
	t_pos.vy += Y_SHIFT ;



	if(work->dir==0){
		t_pos.vz += BODY_SIZE_Z ;
		GM_MoveTarget( &work->b_trg, &t_pos ) ;
		t_pos.vz -= BODY_SIZE_Z*2 ;
		GM_MoveTarget( &work->b_trg2, &t_pos ) ;
	}else {
		t_pos.vx += BODY_SIZE_Z ;
		GM_MoveTarget( &work->b_trg, &t_pos ) ;
		t_pos.vx -= BODY_SIZE_Z*2 ;
		GM_MoveTarget( &work->b_trg2, &t_pos ) ;
	}

}
static void Die(Work *work)
{
//	if(work->inflag &ST_ACT){
		GM_FreeControl( (CONTROL *) &work->ctrl ) ;
//	}

	DG_DequeueObjs( work->objs ) ;
	DG_FreeObjs( work->objs );
	GM_FreeTarget( &work->b_trg ) ;
	GM_FreeTarget( &work->b_trg2 ) ;
	if(work->segment != NULL) {
		HZX_RemoveDynamicSegment(work->segment);
		work->segment = NULL ;
	}
}
static	void	ChildTargCallBack( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
	Work	*work ;
//	char str[24] ;
	work = ( Work * )ptr ;
//	str = "hit" ;
    if ( def->damaged & TARGET_POWER ) {
		if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
			def->power->force = off->power->force ;
		 	if( def->weapon_type & (WP_BULLET|WP_M92)){
				work->finish = VR_TrgHitFunc(work->str_work,&def->hit,&def->hit,&def->power->force,VR_TRG_LEVEL1,VR_TRG_TYPE_WALL) ;
				work->vital+= VR_GetPartsDmg(VR_TRG_TYPE_WALL,1) ; 
				if(work->vital < VR_WALL_LIFE_MAX){
					GM_SeSetMode(SD_A_V_MTHIBI,&def->hit,GM_SEMODE_BOMB) ;
				}
				work->amb_cnt = VR_TRG_FLS_CNT ;
			}else if( def->weapon_type & WP_BLAST){
				/*爆発系ヒット*/
				work->finish = VR_TrgHitFunc(work->str_work,&def->hit,&def->hit,&def->power->force,VR_TRG_LEVEL1,VR_TRG_TYPE_WALL) ;
				work->vital = 255 ; 
			}
		}
	}
	def->weapon_type = 0 ;
}
static void SetTarget(work)
Work *work;
{
	int flag ;
    IVECTOR		iv[ 2 ];
    HZX_GROUP_ID	hzx_id, id;
    int			gn;    
	float size_x,size_z ;
	FVECTOR	b_size  ;

	if(work->dir==0){
		b_size.vx = BODY_SIZE_X ;
		b_size.vy = BODY_SIZE_Y ;
		b_size.vz = BODY_SIZE_Z ;
		b_size.vw = 1.0f ;

		size_x = BODY_SIZE_X ;
		size_z = 0.0f ;
	}else {

		b_size.vx = BODY_SIZE_Z ;
		b_size.vy = BODY_SIZE_Y ;
		b_size.vz = BODY_SIZE_X ;
		b_size.vw = 1.0f ;

		size_x = 0.0f ;
		size_z = BODY_SIZE_X ;
	}

	/*ターゲット設定*/
//	flag =(TARGET_DEFENSE|TARGET_SEEK|TARGET_ROTATE|TARGET_POWER);
	flag =(TARGET_DEFENSE|TARGET_SEEK|TARGET_POWER);
	GM_SetTarget(&work->b_trg,(flag|TARGET_LOCKON),GM_CurrentMap,ENEMY_SIDE,&b_size,&DG_ZeroVector);
	GM_SetTarget(&work->b_trg2,(flag|TARGET_LOCKON),GM_CurrentMap,ENEMY_SIDE,&b_size,&DG_ZeroVector);

	GM_SetPowerTarget( &work->b_trg, &work->b_power,POWER_DECREASE, 500, 0, 0, &DG_ZeroVector );
	GM_SetPowerTarget( &work->b_trg2, &work->b_power2,POWER_DECREASE, 500, 0, 0, &DG_ZeroVector );
	GM_SetTargetCallBack( &work->b_trg, ChildTargCallBack, work ) ;
	GM_SetTargetCallBack( &work->b_trg2, ChildTargCallBack, work ) ;
	GM_PutTarget( &work->b_trg );
	GM_PutTarget( &work->b_trg2 );

    id = GM_GetHzxGroupID( work->where );
    gn = GV_GetNo( id );
    hzx_id = GV_GetBit( gn );


    iv[0].vx = (int)work->pos.vx - size_x;
    iv[0].vy = (int)work->pos.vy ;
    iv[0].vz = (int)work->pos.vz - size_z;
    iv[0].vw = 2000.0f;
    
    iv[1].vx = (int)work->pos.vx + size_x ;
    iv[1].vy = (int)work->pos.vy ;
    iv[1].vz = (int)work->pos.vz + size_z;
    iv[1].vw = 2000.0f;
    work->segment = HZX_AddDynamicSegment( hzx_id, &iv[0], &iv[1], SEG_FLAG );



#if 0
	NewTargetView( &work->b_trg, 255, 0, 0 ) ;
	NewTargetView( &work->b_trg2, 0, 0, 255 ) ;
#endif
}

static int GetResources(Work *work, int name, int where)
{
	DG_DEF  *def ;
	SVECTOR rot ;
	int model ;
	work->name = name ;
	work->where = where ;
	work->vital = 0 ;
	work->inflag = ST_ACT ;

	GM_InitControl( (CONTROL *) &work->ctrl, name, where );

	rot.vx = rot.vz = 0 ;
	if ( GCL_GetOption( 'f' ) != NULL ) {
		work->type = GCL_GetNextInt() ;
	}else {
		work->type = 0 ;
	}
	if ( GCL_GetOption( 'k' ) != NULL ) {
		model = GCL_GetNextInt() ;
	}else {
		model = GV_StrCode("ration_ibox_sh") ;
	}
	if ( GCL_GetOption( 'd' ) != NULL ) {
		work->dir = GCL_GetNextInt() ;
	}else {
		work->dir = 0 ;
	}
	if(work->dir==0){
		rot.vy = 0;
	}else {
		rot.vy = 1024;
	}
//	if ( GCL_GetOption( 'f' ) != NULL ) {
//		flag = ONE_PIECE_FLAG ;
//	}

	if ( GCL_GetOption( 'p' ) != NULL ) {
		work->pos.vx = (float)GCL_GetNextInt() ;
		work->pos.vy = (float)GCL_GetNextInt() ;
		work->pos.vz = (float)GCL_GetNextInt() ;
	}
	work->ef_mdl_code = MDL_VR_TRG_WALL_RGB ;
	def = (DG_DEF*) GV_GetCache( GV_CacheID( MDL_VR_TRG_WALL, 'k' ) ) ;
//	def = (DG_DEF*) GV_GetCache( GV_CacheID( model, 'k' ) ) ;
	work->objs = DG_MakeObjs( def, (DG_FLAG_SHADE|DG_FLAG_ONEPIECE), DG_CHANL_MAIN ) ;
	DG_QueueObjs( work->objs ) ;
	DG_SetLightMatrix( work->objs, work->lights );

	{
		FVECTOR	pos ;
		pos = work->pos ;
		pos.vy += Y_SHIFT ;

		DG_SetPos2(&pos,&rot);
		DG_PutObjs( work->objs );
		GM_GroupObjs( work->objs, where ) ;
	}
	SetTarget(work) ;
#if 0
	if(!(work->type & VR_TARGET_TYPE_NG)){
		VR_AddTarget() ;
	}
#endif

	if ( GCL_GetOption( 'c' ) != NULL ){
		if( GCL_NextStr() != NULL ){
			work->proc_id  = GCL_GetNextInt();
		}else {
			work->proc_id  = 0;
		}
	}
	work->str_work = NULL ;
	GV_SetActorChild( work,work->str_work = (void *)NewSIG_3DPrintf(HIT_PRINT_SHIFT) );


	if(work->ef_mdl_code !=0){
		int cache ;

		cache = GV_CacheID( work->ef_mdl_code, 'r' ) ;

		GV_SetActorChild( work,(void *)NewObjectOutline( cache, work->objs , OL_FLAG_ERASETRANS ) );
		
		work->ol_col.col_num  = 0 ;
		work->ol_col.col[0].r = 127 ;
		work->ol_col.col[0].g = 127 ;
		work->ol_col.col[0].b = 127 ;
		work->ol_col.col[0].cd = 0 ;

		GV_CallChildSignalFunc( work,OL_SIG_SetColor, (int) &work->ol_col ) ;
	}


	return 1;
}

/* 初期化部メイン */
void *NewVRWall( name , where )
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
