//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	sky_prev.c
	上空 
	1999/11/16  H.TANAKA
	2000/10/18 S.Okajima
	2000/11/08 T.Shibata	//プリミチブ化

	$Id: sky_prev.c,v 1.1.1.3 2002/11/19 11:47:46 Yoshizawa1 Exp $
*/


#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <math.h>
#endif
#ifdef PSX2
#include <sys/types.h>
#endif

#ifdef PSX2
#include <eekernel.h>
#endif
#ifdef PSX2
#include <eeregs.h>
#endif
#ifdef PSX2
#include <libgraph.h>
#endif
#ifdef PSX2
#include <libdma.h>
#endif
#ifdef PSX2
#include <libdev.h>
#endif
#ifdef PSX2
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif
#include "libutl.h"

#include	"gameheader.h"
#include	"libmt.h"
#include	"camera.h"
#include	"../etc/ok_util.h"


extern int ok_flush_status;

#define N_CLOUD		(48)
#define N_ALLVERTS	(N_CLOUD*4)
#define N_VERTS		(64)
#define	N_PRIMS		(N_ALLVERTS/N_VERTS)

//#define N_VERTS		(64)
//#define	N_UNITS		128

#define	MAX_FAR			(400000.0F)   /* -400m から 400m */
#define	MAX_RANGE	  	(400000.0F)   /* 中心から 400m */
#define	NOT_FADE_RANGE 	(300000.0F)   /* 中心から 300m */
#define	BASE_LEN	   	(100000.0F)   /* MAX_RANGE － NOT_FADE_RANGE */
#define	HEIGHT		 	(100000.0F)

#define	MAX_FAR_DEMO		(400000.0F*4.0f)   /* -400m から 400m */
#define	MAX_RANGE_DEMO		(400000.0F*4.0f)   /* 中心から 400m */
#define	NOT_FADE_RANGE_DEMO (300000.0F*4.0f)   /* 中心から 300m */
#define	BASE_LEN_DEMO	   	(100000.0F*4.0f)   /* MAX_RANGE － NOT_FADE_RANGE */
#define	HEIGHT_DEMO		 	(100000.0F*4.0f)	

#if 0 //BP
//#ifdef PSX2
#define MEM_SCR_POS			((void *)(SCRPAD_ADDR))
#define MEM_SCR_BASE		((void *)(MEM_SCR_POS + sizeof(FVECTOR)*N_ALLVERTS))
#define MEM_SCR_VEC			((void *)(MEM_SCR_BASE + sizeof(FVECTOR)*N_CLOUD))
#define MEM_SCR_UV			((void *)(MEM_SCR_VEC + sizeof(FVECTOR)*N_CLOUD))
#define MEM_SCR_BOTTOM		((void *)(MEM_SCR_UV + sizeof(DG_PRIM2_UVRGB)*N_ALLVERTS))

#else
#define MEM_SCR_POS			((char *)(SCRPAD_ADDR))
#define MEM_SCR_BASE		((char *)(MEM_SCR_POS + sizeof(FVECTOR)*N_ALLVERTS))
#define MEM_SCR_VEC			((char *)(MEM_SCR_BASE + sizeof(FVECTOR)*N_CLOUD))
#define MEM_SCR_UV			((char *)(MEM_SCR_VEC + sizeof(FVECTOR)*N_CLOUD))
#define MEM_SCR_BOTTOM		((char *)(MEM_SCR_UV + sizeof(DG_PRIM2_UVRGB)*N_ALLVERTS))

#endif


#define	BODY_FLAG	  (DG_FLAG_TEXT|DG_FLAG_NOFOG|DG_FLAG_SHADE|DG_FLAG_ONEPIECE)
//#define	MAX_BRIGHT	 10.0F
#define	MAX_BRIGHT	 32.0F

#define PRINT_PFVEC(_i,_fv) printf("[%d] vx %8.8f:vy %8.8f:vz %8.8f:vw %8.8f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)
typedef struct
{
	DG_OBJS	 *objs ;
	FMATRIX	 light[2] ;
	FMATRIX	 world ;
	FVECTOR	 speed ;
} Unit ;

typedef struct {
	GV_ACT_EX	actor ;
	int			map ;
	int			name;
	int			flag;
	int			upper_shift_flag;
	int			force_game;			// デモ環境でもゲーム設定にする（特殊）

	float		max_far;
	float		max_range;
	float		not_fade_range;

	float		base_len;
	float		height;
	float		scale;
	float		angle[N_CLOUD];

//	Unit		unit[N_UNITS];

	FVECTOR		dvec[N_CLOUD];
	FVECTOR		base_pos[N_CLOUD];

	DG_PRIM2	*prim;

} Work ;

static FVECTOR	InitPolyVerts[4] = {
	{  32000.0f, 0.0f, -32000.0f, 1.0f },
	{  32000.0f, 0.0f,  32000.0f, 1.0f },
	{ -32000.0f, 0.0f, -32000.0f, 1.0f },
	{ -32000.0f, 0.0f,  32000.0f, 1.0f },
};

static void MakeMatrix( FMATRIX *out_mat, FVECTOR *force, FVECTOR *pos )
{
	static FVECTOR outer = { 0.0f, 1.0f, 0.0f, 0.0f };

/* --------------
スケールの掛かったマトリクスを作る
スケールはforceの長さ
になるはず
//  vf0	zero
//  vf1 temp
//  vf2
//  vf3	outer
//  vf4	out_mat x
//  vf5	out_mat y
//  vf6	out_mat z
//  vf7	out_mat w
*/
#ifdef BP_PSX2_ASM
	asm volatile ("
    lqc2		vf6 ,0x00(%1)
    lqc2		vf3 ,0x00(%2)
    lqc2		vf7 ,0x00(%0)

    vopmula.xyz	ACC,vf3,vf6
    vopmsub.xyz	vf4,vf6,vf3

    vmul.xyz	vf1,vf4,vf4
    vmulax.w	ACC,vf0,vf1
    vmadday.w	ACC,vf0,vf1
    vmaddz.w	vf1,vf0,vf1
    vrsqrt		Q,vf0w,vf1w
    vwaitq
    vmulq.xyz	vf2,vf4,Q

    vopmula.xyz	ACC,vf6,vf2		
    vopmsub.xyz	vf5,vf2,vf6

	sqc2		vf4 ,0x00(%3)
	sqc2		vf5 ,0x10(%3)
	sqc2		vf6 ,0x20(%3)
	sqc2		vf7 ,0x30(%3)
	": : "r"(pos), "r"(force), "r"(&outer), "r"(out_mat) );
#else
	FVECTOR tmp ;
	
	_sceVu0CopyVector( (FVECTOR *)out_mat->m[2], force ) ;
	_sceVu0CopyVector( (FVECTOR *)out_mat->m[3], pos ) ;
	_sceVu0OuterProduct( (FVECTOR *)out_mat->m[0], &outer, force ) ;
	_sceVu0Normalize( &tmp, (FVECTOR *)out_mat->m[0] ) ;
	_sceVu0OuterProduct( (FVECTOR *)out_mat->m[1], (FVECTOR *)out_mat->m[2], &tmp ) ;
#endif

}
#if 0
/**************************************************************************/
static  void   Act_Unit( Work *work, Unit *unit )
{
	FMATRIX  *world ;
	FMATRIX  *light ;
	FVECTOR  *speed ;
	FVECTOR  pos ;
	float	scale ;
	float	len ;
	float	bright ;
	float	max_len ;
	float	base_len ;


	world = &unit->world ;
	speed = &unit->speed ;

	scale  = world->m[3][3] ;
	pos.vx = world->m[3][0] ;
	pos.vz = world->m[3][2] ;

	max_len  = work->max_range * scale ;
	base_len = work->base_len  * scale ;

	/* スピードを足す */
	_sceVu0AddVector(&pos,&pos,speed) ;

	/* 長さチェック */
	len = bp_sqrtf(pos.vx * pos.vx + pos.vz * pos.vz) ;   //BP_MATH - emulate PS2 sqrtf

	light = &unit->light[1] ;
	if(len <= max_len){
		/* フェードさせる */

		if(len > (work->not_fade_range * scale)){
			len = max_len - len ;
			if(len < 0.0F)
			len = 0.0F ;
			bright = MAX_BRIGHT * len / base_len ;
			light->m[3][0] = bright ;
			light->m[3][1] = bright ;
			light->m[3][2] = bright ; 
		}
		/* ワールドの位置を決定 */
		world->m[3][0] = pos.vx ;
		world->m[3][2] = pos.vz ;
		return ;
	}else{
		light->m[3][0] = 0.0F ;
		light->m[3][1] = 0.0F ;
		light->m[3][2] = 0.0F ;
	}

	if(max_len < pos.vx){
		pos.vx = -max_len;
	}else if(pos.vx < (- max_len)){
		pos.vx = max_len ;
	}

	if(max_len < pos.vz){
		pos.vz = - max_len;
	}else if(pos.vz < (- max_len)){
		pos.vz = max_len;
	}
	/* ワールドの位置を決定 */
	world->m[3][0] = pos.vx ;
	world->m[3][2] = pos.vz ;

}
#endif

static void PrimAct( Work *work )
{
	DG_PRIM2		*prim = work->prim;
	FVECTOR			*pos,*dvec,*base,force;
	DG_PRIM2_UVRGB	*uvrgb;
	FMATRIX			world;
	int				clock,i,flag;
	float			len,bright;
	
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;

	OK_Mem_Scr( MEM_SCR_POS, prim->pos[1-clock], sizeof(FVECTOR), N_ALLVERTS );
	OK_Mem_Scr( MEM_SCR_UV, prim->uvrgb[1-clock], sizeof(DG_PRIM2_UVRGB), N_ALLVERTS );
	OK_Mem_Scr( MEM_SCR_VEC, work->dvec, sizeof(FVECTOR), N_CLOUD );
	OK_Mem_Scr( MEM_SCR_BASE, work->base_pos, sizeof(FVECTOR), N_CLOUD );

	pos = MEM_SCR_POS;
	base = MEM_SCR_BASE;
	dvec = MEM_SCR_VEC;
	uvrgb = MEM_SCR_UV;

	for( i = 0; i < N_CLOUD; i++ ){
		bright = 0.0f;
		flag = 0;
		_sceVu0AddVector( base, base, dvec );
		len = bp_sqrtf( base->vx*base->vx + base->vz*base->vz) ; //BP_MATH - emulate PS2 sqrtf

		if(len <= work->max_range){
			bright = MAX_BRIGHT;
			if(len > (work->not_fade_range)){
				len =  work->max_range - len ;
				if(len < 0.0F)
					len = 0.0F ;
				bright = MAX_BRIGHT * len / work->base_len ;
			}
		}
		if( (GM_CheckGameStatus( STATE_DEMO ) && work->force_game == 0) || work->upper_shift_flag)
			bright = bright / 5 * 7;
		uvrgb[0].r = (int)bright;
		uvrgb[0].g = (int)bright;
		uvrgb[0].b = (int)bright;
		uvrgb[1].r = (int)bright;
		uvrgb[1].g = (int)bright;
		uvrgb[1].b = (int)bright;
		uvrgb[2].r = (int)bright;
		uvrgb[2].g = (int)bright;
		uvrgb[2].b = (int)bright;
		uvrgb[3].r = (int)bright;
		uvrgb[3].g = (int)bright;
		uvrgb[3].b = (int)bright;
		
		if(work->max_range < base->vx){
			base->vx = -work->max_range;
			flag = 1;
		}else if(base->vx < (- work->max_range)){
			base->vx = work->max_range ;
			flag = 1;
		}

		if(work->max_range < base->vz){
			base->vz = -work->max_range;
			flag = 1;
		}else if(base->vz < (- work->max_range)){
			base->vz = work->max_range;
			flag = 1;
		}
		//再計算
		if(flag){
			force.vx = work->scale * cosf(work->angle[i]);
			force.vy = 0.0f;
			force.vz = work->scale * sinf(work->angle[i]);
			force.vw = 0.0f;

			MakeMatrix( &world, &force, base );
			DG_SetPos( &world );
			DG_PutVector(InitPolyVerts,pos,4);
		}else{
			_sceVu0AddVector( &pos[0], &pos[0], dvec );
			_sceVu0AddVector( &pos[1], &pos[1], dvec );
			_sceVu0AddVector( &pos[2], &pos[2], dvec );
			_sceVu0AddVector( &pos[3], &pos[3], dvec );
		}
		//AN_Test_Eye2(base,2);
		pos+=4;
		uvrgb+=4;
		dvec++;
		base++;
	}

	OK_Scr_Mem( prim->pos[clock], MEM_SCR_POS, sizeof(FVECTOR), N_ALLVERTS );
	OK_Scr_Mem( prim->uvrgb[clock], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), N_ALLVERTS );
	OK_Scr_Mem( work->dvec, MEM_SCR_VEC, sizeof(FVECTOR), N_CLOUD );
	OK_Scr_Mem( work->base_pos, MEM_SCR_BASE, sizeof(FVECTOR), N_CLOUD );
}

static int CheckMesgParam( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;

	mes_num=GV_ReceiveMessage( work->name, &msg );

	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		  case 0:	//die
			work->flag = -1;
			break;
		  case 1:	//vis
			work->flag = 0;
			break;
		  case 2:	//inv
			work->flag = 1;
			break;
		  default:
			printf("sky_prev.c Msg Err!![%d]\n",num);
			break;
		}
		msg--;
	}

	return work->flag;
}

static  void  Act(Work *work)
{
	GM_CameraSet	*current_cam;
#if 1
	if( CheckMesgParam( work )){
		DG_InvisiblePrim2( work->prim );
		if(work->flag < 0){
			GV_DestroyActor( work );
		}else{
			GV_WaitMessage( work, work->name );
		}
		return;
	}
	/* キャラがマップにないときは 何も処理をしない */
	if( !(work->map & GM_CurrentStageMap) ){
		DG_InvisiblePrim2( work->prim ) ;
		return ;
	}
//	if( ok_flush_status!=0 ) return;

	current_cam = GM_GetCurrentCamera( 0 ) ;
	if( current_cam->rotate.vx > 200 ){
		DG_InvisiblePrim2( work->prim ) ;
		return ;
	}
	
#endif
#if 0
	unit = work->unit ;
	for(i = 0 ; i < N_UNITS ; i ++,unit++){
		DG_VisibleObjs( unit->objs ) ;
		/* 位置とライト決定 */	
		Act_Unit( work, unit ) ;
		/* 表示 */
		DG_SetPos(&unit->world) ;
		DG_PutObjs(unit->objs) ;
	}
#endif
	DG_VisiblePrim2( work->prim ) ;
	PrimAct(work);

}

static  void  Die(Work *work)
{
	if(work->prim) GM_FreePrim2( work->prim );
}
#if 0
static  void  Init_Unit( Work *work, Unit *unit )
{
	FVECTOR   pos ;
	SVECTOR   rot ;
	FVECTOR   *speed ;

	/* 位置決め */
	pos.vx = frnd() * work->max_far;
	pos.vy = work->height + rnd() * 10000.0F - 5000.0F ;
	pos.vz = frnd() * work->max_far;

	/* 回転度 */
	rot.vx = 0 ;
	rot.vy = irnd() % 4096 ;
	rot.vz = 0 ;

	/* マトリックス取得 */
	DG_SetPos2(&pos,&rot) ;
	DG_GetPos(&unit->world) ;
	unit->world.m[3][3] /= work->scale ;
	unit->world.m[3][0] /= work->scale ;
	unit->world.m[3][1] /= work->scale ;
	unit->world.m[3][2] /= work->scale ;
	
	if( (GM_CheckGameStatus( STATE_DEMO ) && work->force_game == 0) || work->upper_shift_flag ){
		/* スピード決め */
		speed = &unit->speed ;
		speed->vx = 1024.0F + frnd() * 1024.0F ;
		speed->vy = 0.0F ;
		speed->vz = 0.0F;
		rot.vx = 0 ;
		rot.vy = irnd() % 2048 - 1024 ;
		rot.vz = 0 ;
	}else{
		/* スピード決め */
		speed = &unit->speed ;
		speed->vx = 128.0F + frnd() * 128.0F ;
		speed->vy = 0.0F ;
		speed->vz = 0.0F;
		rot.vx = 0 ;
		rot.vy = irnd() % 1024 - 512 ;
		rot.vz = 0 ;
	}
	DG_SetPos2(&DG_ZeroVector,&rot) ;
	DG_RotVector(speed,speed,1) ;

	speed->vx /= work->scale ;
	speed->vy /= work->scale ;
	speed->vz /= work->scale ;
}
#endif

static int InitPrimData( Work *work )
{
	DG_TEX			*tex;
	DG_PRIM2		*prim;
	FVECTOR			*pos,*dvec,*base,force;
	DG_PRIM2_UVRGB	*uvrgb;
	FMATRIX			world;
	int				u[2],v[2];
	int				i,j;
	float			temp0,temp1;

	tex = DG_GetTexture(7361654);	
//	tex = DG_GetTexture(6715088);
	if(!tex){ printf("ERR!! NO TEX!! sky_prev.c\n"); return (-1); }

	u[0] = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	u[1] = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	v[0] = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	v[1] = FTOI12( 1.0f * tex->v_scale + tex->v_offset );

	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SHADE,//|DG_PRIM2_FOG,
									  N_PRIMS,
									  N_VERTS );

	if(!prim){ printf("ERR!! MAKE PRIM2!! sky_prev.c\n"); return (-1); }

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x80 ) );
	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	pos = MEM_SCR_POS;
	base = MEM_SCR_BASE;
	dvec = MEM_SCR_VEC;
	uvrgb = MEM_SCR_UV;

	for( i = 0; i < N_CLOUD; i++ ){
		base->vx = frnd() * work->max_far;
		base->vy = work->height + rnd() * 10000.0F - 5000.0F ;
		base->vz = frnd() * work->max_far;
		base->vw = 1.0f;
		
//		PRINT_PFVEC(i,base);
		work->angle[i] = temp0 = PI * frnd();
		force.vx = work->scale * cosf(temp0);
		force.vy = 0.0f;
		force.vz = work->scale * sinf(temp0);
		force.vw = 0.0f;

		MakeMatrix( &world, &force, base );
		DG_SetPos( &world );
		DG_PutVector(InitPolyVerts,pos,4);

		//PRINT_PFVEC(i,pos);
		//PRINT_PFVEC(i,pos+1);
		//PRINT_PFVEC(i,pos+2);
		//PRINT_PFVEC(i,pos+3);
		temp0 = frnd() * PI;
//		temp1 = (-1024.0F + rnd() * 2048.0F ) * 2.0f;
		temp1 = rnd() * 2048.0F + 1024.0f * ((irnd()&0x0800000)?-1.0f:1.0f);

		if( !((GM_CheckGameStatus( STATE_DEMO ) && work->force_game == 0) || work->upper_shift_flag) ){
			temp0 *= 0.5f;
			temp1 *= 0.125f;
		}

		dvec->vx = temp1 * cosf(temp0);
		dvec->vy = 0.0f;
		dvec->vz = temp1 * sinf(temp0);
		dvec->vw = 0.0f;

//		PRINT_PFVEC(i,dvec);

		for( j = 0; j < 4; j++ ){
			uvrgb->r = (int)MAX_BRIGHT;
			uvrgb->g = (int)MAX_BRIGHT;
			uvrgb->b = (int)MAX_BRIGHT;
			uvrgb->a = 0x40;
			uvrgb->q = 4096;
			uvrgb->f = (j<2)?0x8fff:0x0fff;
			uvrgb->u = u[j>>1];
			uvrgb->v = v[j&1];

			uvrgb++;
		}
		pos += 4;
		dvec++;
		base++;
	}

	OK_Scr_Mem( prim->pos[0], MEM_SCR_POS, sizeof(FVECTOR), N_ALLVERTS );
	OK_Scr_Mem( prim->pos[1], MEM_SCR_POS, sizeof(FVECTOR), N_ALLVERTS );
	OK_Scr_Mem( prim->uvrgb[0], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), N_ALLVERTS );
	OK_Scr_Mem( prim->uvrgb[1], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), N_ALLVERTS );
	OK_Scr_Mem( work->dvec, MEM_SCR_VEC, sizeof(FVECTOR), N_CLOUD );
	OK_Scr_Mem( work->base_pos, MEM_SCR_BASE, sizeof(FVECTOR), N_CLOUD );

	return 0;
}

static  int   GetResources(Work *work)
{
	int		upper;


	work->force_game = 0;
	if( GCL_GetOption('g') != NULL){
		work->force_game = 1;
	}


	upper = 0;
	if( GCL_GetOption('u') != NULL){
		upper = GCL_GetNextInt();
	}

	work->upper_shift_flag = upper;

	if( (GM_CheckGameStatus( STATE_DEMO ) && work->force_game == 0)  ||  upper ){
		work->max_far        = MAX_FAR_DEMO;
		work->max_range      = MAX_RANGE_DEMO;
		work->not_fade_range = NOT_FADE_RANGE_DEMO;
		work->base_len       = BASE_LEN_DEMO;
		work->height         = HEIGHT_DEMO;
		work->scale          = 16.0F + rnd() * 16.0F;

		printf("demodemodemodemo\n");
	}else{
		work->max_far        = MAX_FAR;
		work->max_range      = MAX_RANGE;
		work->not_fade_range = NOT_FADE_RANGE;
		work->base_len       = BASE_LEN;
		work->height         = HEIGHT;
		work->scale          = 4.0F + rnd() * 4.0F;
		//work->scale          = 2.0F + frnd();

		printf("gamegamegame\n");
	}
	if( GCL_GetOption('h') != NULL){
		work->height = (float)GCL_GetNextInt();
	}

#if 0
	if(GCL_GetOption('m') != NULL){
		def = (DG_DEF *)GV_GetCache(GV_CacheID(GCL_GetNextInt(),'k')) ;
	}else{
		printf(" sky_prev.c : There is not model_name in w00a.gcl\n") ; 
		return -1 ;
	}
	if(def == NULL){
		printf("not sky_test\n") ;
		return  -1 ;
	}

	unit = work->unit ;
	for(i = 0; i < N_UNITS; i++,unit++){
		unit->objs = DG_MakeObjs(def,BODY_FLAG,0) ;
		if(unit->objs == NULL) return -1 ;

		/* ライトの位置 */
		DG_SetLightMatrix( unit->objs,unit->light) ;
		unit->light[1].m[3][0] = MAX_BRIGHT ;
		unit->light[1].m[3][1] = MAX_BRIGHT ;
		unit->light[1].m[3][2] = MAX_BRIGHT ;
		DG_QueueObjs(unit->objs) ;
		Init_Unit( work, unit )  ;

		DG_SetPos(&unit->world) ;
		DG_PutObjs(unit->objs) ;
	}
#endif
	work->flag = 0;
	return InitPrimData( work ) ;
}


void  *NewSky_Prev(int name, int map )
{
	Work	*work ;

	printf("SKY_PREV SCR BOTTOM[%p]\n",MEM_SCR_BOTTOM);
	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof(Work)) ;
	if( work != NULL ){
		GV_SetActor(&(work->actor), Act, Die ) ;
		GV_ActorEX( &work->actor );
		GV_SetActorMessageKill( work, name );
		work->map = map ;
		work->name = name;
		if( GetResources( work ) < 0 ){
			GV_DestroyActor( work )  ;
			return NULL ;
		}
	}
	return (void *) work ;
}
