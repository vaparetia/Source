//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	blood_water.c
	水中血
	
	2000/01/15 T.Shibata
	
	$Id: blood_water_demo.c,v 1.1.1.3 2002/11/19 11:48:29 Yoshizawa1 Exp $

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

#include 	"libutl.h"
#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"../util/ts_util.h"

#define CLOCK_COUNT	(BP_BASE_TICK())

// -----------------------------------------------------------
//			extern
extern void *NewGeneralSprite( int tex_code, FVECTOR *pos, float shift,
                               int *rgba, int w, int h, int life, int *mode );

//デバック用簡易呼び出し(万能テクスチャー使用、2147483647フレーム後死亡)
#define	NewDbugSprite(_pos,_size ) \
    NewGeneralSprite( (int)(6715088), (FVECTOR*)(_pos), (float)(0.0f), (int*)(NULL), \
					  (int)(_size), (int)(_size), (int)(0x7fffffff), (int*)(NULL) )

#define	PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)


#define GRAVITATION	(P_GRAVITY)
#define LIFE		(100*CLOCK_COUNT)

#define N_PRIMS		(1)
#define N_VERTS		(16)
#define N_ALLVERTS	(N_PRIMS*N_VERTS)

#define	SPRITE_SIZE_ADD	(0.8f)

#define FLAGS_ROTMODE	(0x0001)

typedef	struct	{
	GV_ACT_EX	actor;
	DG_PRIM2	*prim;
	int			timer;
	short		flags,count;
	float		pow;
	FVECTOR		head_pos;
	FVECTOR		head_vec;

	FVECTOR		dvec[N_ALLVERTS];
	FVECTOR		data[N_ALLVERTS];
	short		mode[N_ALLVERTS];
#if 1
	FVECTOR		pos[N_ALLVERTS];
#endif
} Work;

#ifdef PSX2S2
#define		MEM_SCR_POS	((void*)(SCRPAD_ADDR))
#define		MEM_SCR_UV	((void*)(MEM_SCR_POS+sizeof(FVECTOR)*N_ALLVERTS))
#else
#define		MEM_SCR_POS	((void*)(SCRPAD_ADDR))
#define		MEM_SCR_UV	((void*)((char *)MEM_SCR_POS+sizeof(FVECTOR)*N_ALLVERTS))
#endif

extern float GM_WaterLevel;
static inline void _RotVector( FVECTOR *out, FMATRIX *world, FVECTOR *in )
{
#ifdef BP_PSX2_ASM
	asm ("
	lqc2				vf8,0x00(%2)
	lqc2				vf4,0x00(%1)
	lqc2				vf5,0x10(%1)
	lqc2				vf6,0x20(%1)
	vmulax.xyzw			ACC, vf4,vf8
	vmadday.xyzw		ACC, vf5,vf8
	vmaddz.xyzw			vf8, vf6,vf8
	sqc2				vf8,0x00(%0)
	": : "r"(out), "r"(world), "r"(in) );//:"memory" );
#else
	DG_SetPos( world ) ;
	DG_RotVector(in, out, 1 ) ;
#endif
}

//当たりチェックと加工
//ニアチェックと水面
static int CheckHzx( FVECTOR *pos, FVECTOR *vec, float len2 )
{
	int 		len = (int)bp_sqrtf(len2); //BP_MATH - emulate PS2 sqrtf
	int			seg_num,ret = 0;
	int			atrs[2];
	HZX_SEG		segs[2];
	HZX_FLR		flr[2];
	FVECTOR		react;
	int 		flr_flag;
	float		flr_height[2];

	//ニアチェック
	seg_num = HZX_NearHazardCheck( GM_GetHzxGroupID( GM_CurrentStageMap ), pos, len,
								   HZX_CHK_ALL, HZX_TYPE_PLAYER, len );

	if(seg_num){
		HZX_GetNearHazard( segs, atrs );
		HZX_GetReactVector( &react );

		_sceVu0AddVector(pos, pos, &react);
		ret = 1;
	}
    //レベルチェック
	flr_flag = HZX_LevelHazardCheck( GM_GetHzxGroupID( GM_CurrentStageMap ),
									pos,
									HZX_CHK_ALL,
									HZX_FLOOR_NO_PLAYER );

	if( flr_flag & 1){
		
		HZX_GetLevelHazard( flr, atrs );
		/* flr_height[2] 0:flr 1:cel*/
		HZX_GetLevelHeight( flr_height );
		if((flr_flag & 1) && pos->vy < flr_height[0]+(float)len){
			pos->vy = flr_height[0]+(float)len;
			ret = 1;
		}
	}
	if(pos->vy > GM_WaterLevel-(float)len){
		pos->vy = flr_height[1]-(float)len;
		ret = 1;
	}

	if(ret){
		vec->vx = 0.0f;
		vec->vy = 0.0f;
		vec->vz = 0.0f;
		vec->vw = 0.0f;
	}
	
	return ret;
}

static void Act( Work *work )
{
	FVECTOR	to_pos;
	int					i,clock;
	DG_PRIM2			*prim = work->prim;
	FVECTOR				*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	DG_PRIM2_UVRGBWH	*p_uvrgbwh;
	
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;
			
	pos = prim->pos[clock];
	uvrgbwh = prim->uvrgb[clock];
	p_uvrgbwh = prim->uvrgb[1-clock];

	_sceVu0ScaleVector(&to_pos,&work->head_vec,work->pow);
	_sceVu0AddVector(&work->head_pos,&work->head_pos,&to_pos);
	if(work->pow>0.4f) work->pow *= 0.8f;

	
	if(work->count < N_ALLVERTS){
		FMATRIX	world;
//		FVECTOR fvtemp = {0.0f,0.0f,work->pow*(1.0f+rnd()*0.5f),0.0f};
		float	rot_x,rot_y,sprt_rot;
		sprt_rot = frnd();
		rot_x = PI * 0.333f * sprt_rot;
		rot_y = PI * 0.5f * frnd();
		sprt_rot *= PI;

		{
			FVECTOR tmp = {0.0f,1.0f,0.0f,0.0f} ;
			TS_MakeMatrix2( &world, &work->head_vec, &tmp, &DG_ZeroVector );
		}
		
		if(work->flags&FLAGS_ROTMODE){
			_sceVu0RotMatrixY( &world, &world, rot_y );
			_sceVu0RotMatrixX( &world, &world, rot_x );
		}else{
			_sceVu0RotMatrixX( &world, &world, rot_x );
			_sceVu0RotMatrixY( &world, &world, rot_y );
		}
		DG_COPY_VEC(&work->pos[work->count],&work->head_pos);
		DG_COPY_VEC(&work->dvec[work->count],world.m[2]);
		work->dvec[work->count].vw = work->pow*(1.0f+rnd()*0.5f);

		//uvegbwh初期化
		uvrgbwh[work->count].a = N_ALLVERTS;
		work->data[work->count].vx = 0.0f;
		work->data[work->count].vy = 0.0f;
		work->data[work->count].vz = SPRITE_SIZE_ADD*cosf(sprt_rot);
		work->data[work->count].vw = SPRITE_SIZE_ADD*sinf(sprt_rot);
	}
	work->count++;
	DG_SetPos(&DG_Chanls[0].eye_pers);
	for( i = 0; i < N_ALLVERTS && work->timer+i <= LIFE; i++ ){
		FVECTOR		vec;
		float		ratio = 0.0f;

		if(work->mode[i]&0x0100){
			DG_COPY_VEC(pos,&work->pos[i]);
		}else{
			work->data[i].vx += work->data[i].vz;
			work->data[i].vy += work->data[i].vw;

			DG_COPY_VEC(&vec,&work->dvec[i]);
			vec.vw = 0.0f;
			_sceVu0ScaleVector(&vec,&vec,work->dvec[i].vw);
			_sceVu0AddVector(&vec,&work->pos[i],&vec);
			
			//vecのでめり込みチェック
			if(i==work->timer%N_ALLVERTS){
				if(CheckHzx( &vec, &work->dvec[i],
				work->data[i].vx*work->data[i].vx+work->data[i].vy*work->data[i].vy )){
					work->mode[i]|=0x0100;
				}
			}
			DG_COPY_VEC(&work->pos[i],&vec);
			DG_COPY_VEC(pos,&vec);
			if(work->dvec[i].vw > 3.0f) work->dvec[i].vw *= 0.6f;
		}
		//カメラと
		DG_PutVector( pos, &vec, 1 );
		if(-vec.vz > 51.0f){
			ratio = 1.0f;
			if(-vec.vz < 1051.0f){
				ratio = (-vec.vz-51.0f)/1051.0f;
			}
		}
		if(work->timer < (N_ALLVERTS*4)+i ){
			float alpha;
			alpha = (float)(work->timer-i)/4.0f;
			if(alpha < 0.0f) alpha = 0.0f;
			uvrgbwh->a = (int)(alpha);
		}else{
			uvrgbwh->a = (int)(16.0f);
		}

		//printf("a[%2d]%2d\n",i,uvrgbwh->a);
		uvrgbwh->w = (int)work->data[i].vx;
		uvrgbwh->h = (int)work->data[i].vy;

		pos++;
		uvrgbwh++;
		p_uvrgbwh++;
	}
	//printf("\n");
	if(--work->timer < 0){
		//printf("DIE\n");
		GV_DestroyActor( work );
	}
}

static void Die( Work *work )
{
	if(work->prim) GM_FreePrim2( work->prim );
}


static int GetResources( Work *work, FVECTOR *center, FVECTOR *vec, float pow, int mode )
{
	int					i,u[2],v[2];//,mode = 0;
	DG_TEX				*tex;
	DG_PRIM2			*prim;
	FVECTOR				*pos = MEM_SCR_POS;
	DG_PRIM2_UVRGBWH	*uvrgbwh = MEM_SCR_UV;
	FVECTOR				fvtemp;
//	FMATRIX				world;
	float				inner;

	if(center->vy > GM_WaterLevel){
		printf("水面より上ですがな\n");
		return -1;
	}
	work->timer = LIFE;
	work->pow = pow;
	work->flags = 0;
	work->count = 0;
	//プリム初期化
	tex = DG_GetTexture(15625989);	//blood_2bw_alp
	if(!tex) { printf("ERR!! NO TEX!! <debris_tex.c>\n"); return(-1); }
	prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(!prim){ printf("ERR!! MAKE PRIM!! <plant_sun.c>\n"); return (-1); }
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );

	//prim->raise = 0;
	u[0] = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	v[0] = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	u[1] = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	v[1] = FTOI12( 1.0f * tex->v_scale + tex->v_offset );

	_sceVu0Normalize( &fvtemp, vec );
	fvtemp.vw = 0.0f;
	inner = _sceVu0InnerProduct(&fvtemp,(FVECTOR*)DG_UnitMatrix.m[0]);
	inner = fabs(inner);
	if( inner >= 0.999f ){
		work->flags |= FLAGS_ROTMODE;
	}
//	TS_MakeMatrix2( &world, &fvtemp, &(FVECTOR){0.0f,1.0f,0.0f,0.0f}, &DG_ZeroVector );
//	_sceVu0ScaleVector(&work->head_vec,&fvtemp,pow);
	DG_COPY_VEC( &work->head_vec, &fvtemp );
	DG_COPY_VEC( &work->head_pos, center );
	
	for( i = 0; i < N_ALLVERTS; i++ ){
		//プリムデータ初期化
		//float rot_x = PI * 0.166f;
		DG_COPY_VEC(pos,&DG_ZeroVector);

		uvrgbwh->r = (~(253))&0xff ;
		uvrgbwh->g = (~(220))&0xff ;
		uvrgbwh->b = (~(220))&0xff ;
		uvrgbwh->a = 0;

		uvrgbwh->u0 = u[0];
		uvrgbwh->v0 = v[0];
		uvrgbwh->u1 = u[1];
		uvrgbwh->v1 = v[1];
		
		uvrgbwh->f0 = 0;
		uvrgbwh->f1 = 0;
		uvrgbwh->q0 = 4096;
		uvrgbwh->q1 = 4096;

		uvrgbwh->w = 0;//(int)(SPRITE_SIZE*cosf(rot_x));
		uvrgbwh->h = 0;//(int)(SPRITE_SIZE*sinf(rot_x));
#if 0
		GV_SetActorChild( work,NewDbugSprite(&prim->pos[0][i],100.0f ));
#endif

		work->mode[i] = i;
		pos++;
		uvrgbwh++;
	}

	TS_Scr_Mem( prim->pos[0], MEM_SCR_POS, sizeof(FVECTOR), N_ALLVERTS );
	TS_Scr_Mem( prim->pos[1], MEM_SCR_POS, sizeof(FVECTOR), N_ALLVERTS );
	TS_Scr_Mem( prim->uvrgb[0], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGBWH), N_ALLVERTS );
	TS_Scr_Mem( prim->uvrgb[1], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGBWH), N_ALLVERTS );
	

#if 0
	GV_SetActorChild( work,NewDbugSprite(&work->head_pos,100.0f ));
#endif
	return (0);
}

/*
FVECTOR		*pos:	発生場所
FVECTOR		*vec:	初期方向
float		pow:	強さ(正数)
int			mode:	未対応（０下さい）
*/

void *NewBloodWater_demo( FVECTOR *pos, FVECTOR *vec, float pow, int mode )
{
	Work		*work ;
	if( GM_Configuration & GM_CONFIG_BLOOD_OFF ){
		return NULL;
	}

	work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, pos, vec, pow, mode ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;
}
