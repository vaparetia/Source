//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	short_spark.c
	電線ショート火花
	2001/3/09 S.Okajima
	$Id: short_spark.c,v 1.1.1.3 2002/11/19 11:47:23 Yoshizawa1 Exp $

*/

#ifdef PSX2
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"


#define	N_VERTS		(64)
#define	N_POLYS		(N_VERTS/2)
#define	N_PRIMS		(4)

#define	MUL_MAX		(64)
#define	STOP_ADD	(16)

#define	DUST_GRAVITY	( P_GRAVITY )

#define	COL_R		(255)
#define	COL_G		(128)
#define	COL_B		(32)

#define	ALP_MIN		(64.0f)
#define	ALP_MAX		(255.0f - ALP_MIN)

#define	SUB_ALPHA	(1.0f)

#define	SHADOW_SIZE	(1000.0f)

#define	DECAY_RATIO	(0.97f)
#define	VEC_MAX		(200.0f)

#define	ROT_WIDTH	(512)

#define	CONT_TIME	(120)
#define	CONT_MIN	(60)

#define	CONT_TIME2	(8)
#define	CONT_MIN2	(1)

#define	DECAY_REACT	(0.25f)

#define	FLOOR_MARGINE	(500.0f)

#define	LIMIT_NEXT		(-P_GRAVITY*0.5f)

/*----------------------------------------------------------------*/
enum {
	REQ_OFF=0,
	REQ_ON,
	REQ_KILL,
	REQ_NO
};

typedef	struct	{
	int			flag;
	float		limit;
} ATARI ;


typedef	struct	{
	GV_ACT_EX		actor ;

	HZX_GROUP_ID	map_id;

	int				seed;

	int				map;
	int				name;

	int			invisible_flag;
	int			kill_flag;

	int			life;
	int			count;
	int			stop_count;
	int			cont_count;
	int			cont_flag;
	int			flash_count;
	FVECTOR		*flash_center;

	FVECTOR		center;

	float		rad;
	float		flr_height;

	ATARI		flr;
	ATARI		sg0;
	ATARI		sg1;

	FVECTOR		vec[N_PRIMS*N_POLYS];
	DG_PRIM2	*prim ;
} Work ;

/*----------------------------------------------------------------*/
static inline void AddVector( FVECTOR *r, FVECTOR *a, FVECTOR *b  )
{
#ifdef BP_PSX2_ASM	
	asm volatile ("
	lqc2		vf1, 0(%0)
	lqc2		vf2, 0(%1)
	vadd.xyz	vf1, vf2, vf1
	sqc2		vf1, 0(%2)
	" : : "r"(a), "r"(b), "r"(r) ) ;
#else
	BP_Vec3_AddVec(r, a, b);
#endif	
}

static inline void MulVector( FVECTOR *output, FVECTOR *input )
{
#ifdef BP_PSX2_ASM
	asm volatile ("
	lqc2			vf8,0x00(%0)
	lqc2			vf9,0x00(%1)
	vmul.xyz		vf8,vf8,vf9
	sqc2			vf8,0x00(%0)
	":  : "r" (output) ,"r"(input) :"memory" );
#else
	output->vx *= input->vx;
	output->vy *= input->vy;
	output->vz *= input->vz;
#endif	
	return;
}

/*----------------------------------------------------------------*/
static void InitVectors( Work *work, FVECTOR *pos, FVECTOR *vec, SVECTOR *master_rot, float vec_master )
{
	FVECTOR	fvtemp;
	SVECTOR	svtemp;

	if( vec->vw <= 0.0f ){
		DG_COPY_VEC( pos++, &work->center );
		DG_COPY_VEC( pos  , &work->center );
		fvtemp.vx = 0;
		fvtemp.vy = 0;
		fvtemp.vz = vec_master*GM_Rnd( &work->seed );
		svtemp.vx = master_rot->vx + (GM_IRnd( &work->seed )>>8)%master_rot->vz;
		svtemp.vy = master_rot->vy + (GM_IRnd( &work->seed )>>8)%master_rot->pad;
		svtemp.vz = 0;
		DG_SetPos2( &DG_ZeroVector, &svtemp );
		DG_PutVector( &fvtemp, vec, 1 );
		vec->vw = ALP_MAX*GM_Rnd( &work->seed ) + ALP_MIN;
	}
}

static void CheckMesgParam( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;

	mes_num=GV_ReceiveMessage( work->name, &msg );
	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		  case REQ_OFF:
			work->invisible_flag = 1;
			break;
		  case REQ_ON:
			work->invisible_flag = 0;
			break;
		  case REQ_KILL:
		  case -1:
			work->kill_flag = 1;
			break;
		  default:
			break;
		}
		msg--;
	}
}

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int		i,j;
	int		clock;
	int		alpha;
	float		vec_master;
	float		ftemp;
	float		flr_height[2];
	DG_PRIM2 	*prim;
	FVECTOR		ratio;
	FVECTOR	*vec;
	FVECTOR	*pos;
	FVECTOR	*pos_before;
	DG_PRIM2_UVRGB		*uvrgb ;
	SVECTOR		master_rot;
	CVECTOR		col;

	if( work->name!=-1 ) CheckMesgParam( work );

	if( work->kill_flag ) GV_DestroyActor( work ) ;

	if( work->invisible_flag == 0  ){
		DG_VisiblePrim2( work->prim ) ;
	}else{
		DG_InvisiblePrim2( work->prim ) ;
		vec = work->vec;
		for ( i=0; i<N_PRIMS*N_POLYS; i++ ){
			vec->vw = 0.0f;
			vec++;
		}
		return;
	}

	if( work->map!=-1 ){
		if( (work->map & GM_CurrentStageMap) != 0 ){
			DG_VisiblePrim2( work->prim ) ;
		}else{
			DG_InvisiblePrim2( work->prim ) ;
			return;
		}
	}else{
		DG_VisiblePrim2( work->prim ) ;
	}

	work->map_id = GM_GetHzxGroupID( GM_CurrentStageMap );

	col.r  = COL_R/4;
	col.g  = COL_G/4;
	col.b  = COL_B/4;

	prim = work->prim;
	DG_VisiblePrim2( prim ) ;
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;

	ratio.vx = DECAY_RATIO;
	ratio.vy = DECAY_RATIO;
	ratio.vz = DECAY_RATIO;

	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;

	vec          = work->vec;
	pos          = prim->pos[  clock];
	pos_before   = prim->pos[1-clock];
	pos_before++;
	uvrgb        = prim->uvrgb[  clock];
	for ( i=0; i<N_PRIMS*N_POLYS; i++ ){
		if( vec->vw > 0.0f ){
			DG_COPY_VEC( pos++, pos_before );
			MulVector( vec, &ratio );
			vec->vy += (DUST_GRAVITY);
			AddVector( pos, pos_before, vec );

			alpha = (int)(vec->vw);
			vec->vw-= SUB_ALPHA;

			if( OK_StepCheckHzd( pos, pos_before, vec, DECAY_REACT*GM_Rnd( &work->seed ), 1.0f, work->map_id ) ){
				if( DG_FABS(vec->vy) < LIMIT_NEXT ){
					vec->vw = 0.0f;
					alpha = 0;
				}else{
					vec->vw = ALP_MAX;		// 跳ね返りをはっきり見せる
					alpha = (int)ALP_MAX;
				}
			}

			if( HZX_LevelHazardCheck( work->map_id, pos, HZX_CHK_FLOOR, 0 ) & 1 ){
				HZX_GetLevelHeight( flr_height );
				ftemp = pos->vy - (*flr_height);
				if( ftemp  <  FLOOR_MARGINE ){
					if( pos->vy > (*flr_height) ){
						ftemp /= FLOOR_MARGINE;
						col.cd = (int)(vec->vw*0.125f);
//						col.cd = alpha;
//						OK_PutFloorLight( pos, (*flr_height), FLOOR_MARGINE*2.0f, col );
						OK_PutFloorLight( pos, (*flr_height), FLOOR_MARGINE, col );
					}
				}

			}

			(uvrgb++)->a = alpha;
			(uvrgb++)->a = alpha;
		}else{
			DG_COPY_VEC( pos++, pos_before );
			(uvrgb++)->a = 0;
			(uvrgb++)->a = 0;
		}

		pos++;
		pos_before+=2;
		vec++;
	}

	master_rot.vx = ((GM_IRnd( &work->seed )>>8)&2047)-1024 - 256;
	master_rot.vy = ((GM_IRnd( &work->seed )>>8)&4095);
	master_rot.vz =  (GM_IRnd( &work->seed )>>8)%ROT_WIDTH+ROT_WIDTH;
	master_rot.pad=  (GM_IRnd( &work->seed )>>8)%ROT_WIDTH+ROT_WIDTH;
	if( ( work->name==-1  &&  ((GM_IRnd( &work->seed )>>8)&3)==0 )
	 || ( --work->stop_count < 0  &&  !work->cont_flag  &&  ((GM_IRnd( &work->seed )>>8)&7)==0 )
	  ){
		pos = prim->pos[  clock];
		vec = work->vec;
		pos+= work->count*2;
		vec+= work->count;

		GM_SeSetMode( SD_A_TRSPARK1, pos, GM_SEMODE_NORMAL ) ;

		j = (GM_IRnd( &work->seed )>>8)%(MUL_MAX/2) + (MUL_MAX/2);
		vec_master = VEC_MAX*0.5f*GM_Rnd( &work->seed )+VEC_MAX*0.5f;
		if( work->flash_center==NULL ){
			work->flash_center = pos;
			work->flash_count = ((GM_IRnd( &work->seed )>>8)&1) + 1;
		}

		work->stop_count = j + 1;
		for( i=0; i<j; i++ ){
			InitVectors( work, pos, vec, &master_rot, vec_master );
			pos+= 2;
			vec++;
			work->count++;
			if( work->count >= N_PRIMS*N_POLYS-1 ){
				work->count = 0;
				pos = prim->pos[  clock];
				vec = work->vec;
			}
		}
	}else if( work->cont_flag ){
		if( ((GM_IRnd( &work->seed )>>8)&31)==0 ){
			pos = prim->pos[  clock];
			vec = work->vec;
			pos+= work->count*2;
			vec+= work->count;

			GM_SeSetMode( SD_A_TRSPARK2, pos, GM_SEMODE_NORMAL ) ;

			vec_master = VEC_MAX*0.01f*GM_Rnd( &work->seed );
			InitVectors( work, pos, vec, &master_rot, vec_master );

			work->count++;
			if( work->count >= N_PRIMS*N_POLYS-1 ){
				work->count = 0;
			}
		}
	}

	if( --work->cont_count < 0 ){
		work->cont_flag = 1 - work->cont_flag;
		if( work->cont_flag ){
			work->cont_count = (GM_IRnd( &work->seed )>>8)%CONT_TIME + CONT_MIN;
		}else{
			work->cont_count = (GM_IRnd( &work->seed )>>8)%CONT_TIME2 + CONT_MIN2;
		}
	}

	if( work->flash_count ){
		if( ((GM_IRnd( &work->seed )>>8)&3)==0 ){
			work->flash_count--;
			if( work->flash_center!=NULL ){
				ftemp = 500.0f + 1000.0f*GM_Rnd( &work->seed );
				DG_SetTmpLight2(
					work->flash_center,
					ftemp,
					ftemp*2.0f,
					COL_R | COL_G<<8 | COL_B<<16,
					LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;
			}
			if( work->flash_count<=0 ){
				work->flash_center = NULL;
			}
		}
	}

	if( work->life > 0 ){
		work->life--;
		if( work->life <= 0 ) GV_DestroyActor( work ) ;
	}

}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}




static void InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex, FVECTOR *center )
{
	int	i,j;
	FVECTOR	fvtemp;
	FVECTOR	*pos_0;
	FVECTOR	*pos_1;
	FVECTOR	*vec;
	DG_PRIM2_UVRGB		*uvrgb_0 ;
	DG_PRIM2_UVRGB		*uvrgb_1 ;

	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	DG_COPY_VEC( &fvtemp, center );

	uvrgb_0 = prim->uvrgb[ 0 ] ;
	uvrgb_1 = prim->uvrgb[ 1 ] ;
	pos_0   = prim->pos[ 0 ] ;
	pos_1   = prim->pos[ 1 ] ;
	vec     = work->vec;
	for ( i=0; i<N_PRIMS; i++ ){
		for ( j=0; j<N_POLYS; j++ ){
			DG_COPY_VEC( vec++, &DG_ZeroVector );

			DG_COPY_VEC( pos_0, &fvtemp );
			DG_COPY_VEC( pos_1, &fvtemp );
			uvrgb_0->u = uvrgb_1->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb_0->v = uvrgb_1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb_0->f = uvrgb_1->f = 0x8fff ;
			uvrgb_0->q = uvrgb_1->q = 4096 ;
			uvrgb_0->r = uvrgb_1->r = COL_R ;
			uvrgb_0->g = uvrgb_1->g = COL_G ;
			uvrgb_0->b = uvrgb_1->b = COL_B ;
			uvrgb_0->a = uvrgb_1->a = 0 ;
			uvrgb_0++;
			uvrgb_1++;
			pos_0++;
			pos_1++;


			DG_COPY_VEC( pos_0, center );
			DG_COPY_VEC( pos_1, center );
			uvrgb_0->u = uvrgb_1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb_0->v = uvrgb_1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb_0->f = uvrgb_1->f = 0x0fff ;
			uvrgb_0->q = uvrgb_1->q = 4096 ;
			uvrgb_0->r = uvrgb_1->r = COL_R ;
			uvrgb_0->g = uvrgb_1->g = COL_G ;
			uvrgb_0->b = uvrgb_1->b = COL_B ;
			uvrgb_0->a = uvrgb_1->a = 0 ;
			uvrgb_0++;
			uvrgb_1++;
			pos_0++;
			pos_1++;
		}
	}
}




/*----------------------------------------------------------------*/
static int GetResourcesCalled( Work *work, FVECTOR *center )
{
	DG_TEX				*tex ;
	DG_PRIM2			*prim ;
	float		flr_height[2];


	work->invisible_flag = 0;
	work->kill_flag = 0;

	work->count = 0;
	work->stop_count = 0;
	work->cont_count = 0;
	work->cont_flag  = 0;
	work->flash_count = 0;
	work->flash_center = NULL;

	_sceVu0CopyVector( &work->center, center ) ;
	work->center.vw = 1.0f ;

	tex = DG_GetTexture( 10984814 /*"chi01_msk"*/ );
//	tex = DG_GetTexture( 7338993 /*"col256_bld100"*/ );
	prim  = work->prim = GM_MakePrim2( DG_PRIM2_LINE|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ANTIALIASING|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if( prim == NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket( work, prim, tex, &work->center );



	work->map_id = GM_GetHzxGroupID( GM_CurrentStageMap );


	if( HZX_LevelHazardCheck( work->map_id, &work->center, HZX_CHK_ALL, 0 ) & 1 ){
		HZX_GetLevelHeight( flr_height );
		work->flr_height = flr_height[0];
	}else{
		work->flr_height =-FLOAT_MAX ;
	}

	return 0 ;
}

/*----------------------------------------------------------------*/
static int GetResourcesProg( Work *work, FVECTOR *pos )
{
	return GetResourcesCalled( work, pos ) ;
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work )
{
	FVECTOR center ;

	work->seed = 5963;

	/* 設置中心座標 */
	if ( GCL_GetOption( 'p' ) != NULL ) {
		center.vx = (float)GCL_GetNextInt() ;
		center.vy = (float)GCL_GetNextInt() ;
		center.vz = (float)GCL_GetNextInt() ;
		center.vw = 1.0f ;
	}

	return GetResourcesCalled( work, &center ) ;
}

void *NewShortSparkProg( FVECTOR *pos, int life )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name = -1;
		work->map = -1;

		work->life = life;

		if ( GetResourcesProg( work, pos ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

void *NewShortSparkLineCalled( int name, int where, FVECTOR *center )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name = name;
		work->map = where;

		work->life = -1;

		if ( GetResourcesCalled( work, center ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

void *NewShortSparkLine( int name, int where )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name = name;
		work->map = where;

		work->life = -1;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
