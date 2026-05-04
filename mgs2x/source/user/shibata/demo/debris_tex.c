//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	debris_cm.c
	破片パーティクル（スプライト）
	
	2000/01/05 T.Shibata
	
	$Id: debris_tex.c,v 1.1.1.3 2002/11/19 11:48:30 Yoshizawa1 Exp $

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
#include	"../../mode/demo/eft_con.h"

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
#define LIFE		(15*CLOCK_COUNT)
typedef struct	{
	FVECTOR	dvec;
	float	dw,dh,w,h;
} DATA;
typedef	struct	{
	GV_ACT_EX	actor;
	DG_PRIM2	*prim;
	int			n_sprite;
	int			timer;
//	int			flags;
//	int			map_id;
//	float		velocity;

	DATA		data[0];
} Work;

#define		MEM_SCR_POS	((void*)(SCRPAD_ADDR))

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

static void Act( Work *work )
{
	FVECTOR				*pos;
	DATA				*data;
	DG_PRIM2			*prim = work->prim;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	int					i,alpha = 128,clock;

   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;
			
	pos = MEM_SCR_POS;
	uvrgbwh = prim->uvrgb[clock];
	data = work->data;

	if(work->timer < 64) alpha = work->timer*2;
printf( "n_sprite = %d alpha = %d\n", work->n_sprite, alpha );
	TS_Mem_Scr( MEM_SCR_POS, prim->pos[1-clock], sizeof(FVECTOR), work->n_sprite );

	for( i = 0; i < work->n_sprite; i++ ){
		_sceVu0AddVector( pos, pos, &data->dvec );
		data->dvec.vy += GRAVITATION;
		uvrgbwh->a = alpha;
		data->w += data->dw;
		data->h += data->dh;
		uvrgbwh->w = (int)data->w;
		uvrgbwh->h = (int)data->h;

		pos++;
		data++;
		uvrgbwh++;
	}

	TS_Scr_Mem( prim->pos[clock], MEM_SCR_POS, sizeof(FVECTOR), work->n_sprite );

	if(--work->timer < 0){
		//printf("DIE\n");
		GV_DestroyActor( work );
	}
}

static void Die( Work *work )
{
	if(work->prim) GM_FreePrim2( work->prim );
}


static int GetResources( Work *work, FVECTOR *bound, FVECTOR *force,
						 int num, int texcode,
						 int color, float w )
{
	int					i,u[2],v[2],mode = 0;
	DG_TEX				*tex;
	DG_PRIM2			*prim;
	FVECTOR				*pos0,*pos1;
	DG_PRIM2_UVRGBWH	*uvrgbwh0,*uvrgbwh1;
	FVECTOR				add,pow,pos;
	DATA				*data;
	FMATRIX				world;
	float				velocity,inner;
	
	work->n_sprite = num*16;
	velocity = force->vw;	
	work->timer = LIFE;
	
	//プリム初期化
	tex = DG_GetTexture(texcode);
	if(!tex) { printf("ERR!! NO TEX!! <debris_tex.c>\n"); return(-1); }
	prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, num, 16 );
	if(!prim){ printf("ERR!! MAKE PRIM!! <plant_sun.c>\n"); return (-1); }
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	u[0] = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	v[0] = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	u[1] = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	v[1] = FTOI12( 1.0f * tex->v_scale + tex->v_offset );

	DG_COPY_VEC(&pow,force);
	pow.vw = 0.0f;
	_sceVu0Normalize( &pow, &pow );
	inner = _sceVu0InnerProduct(&pow,(FVECTOR*)DG_UnitMatrix.m[0]);
	inner = fabs(inner);
	if( inner >= 0.999f ){
		mode = 1;
	}
#ifdef BP_PS2
	TS_MakeMatrix2( &world, &pow, &(FVECTOR){0.0f,1.0f,0.0f,0.0f}, &DG_ZeroVector );
#else
	{
		FVECTOR tmp = {0.0f,1.0f,0.0f,0.0f} ;
		TS_MakeMatrix2( &world, &pow, &tmp, &DG_ZeroVector );
	}
#endif
	//PRINT_PFVEC(0,(FVECTOR*)world.m[0]);
	//PRINT_PFVEC(1,(FVECTOR*)world.m[1]);
	//PRINT_PFVEC(2,(FVECTOR*)world.m[2]);
	
	DG_COPY_VEC(&pos,bound);
	_sceVu0SubVector( &add, &bound[1], &bound[0] );

	pos0 = prim->pos[0];
	pos1 = prim->pos[1];
	uvrgbwh0 = prim->uvrgb[0];
	uvrgbwh1 = prim->uvrgb[1];
	data = work->data;
	for( i = 0; i < work->n_sprite; i++ ){
		FMATRIX		l_world;
		FVECTOR		fvtemp = {0.0f,0.0f,velocity*0.5f*(rnd() + 1.0f),0.0f};
		float		rot_x,rot_y;
		
		//DVEC初期化
		rot_x = PI * frnd() * 0.166f;
		rot_y = PI * frnd() * 0.5f;
		if(mode){
			_sceVu0RotMatrixY( &l_world, &world, rot_y );
			_sceVu0RotMatrixX( &l_world, &l_world, rot_x );
		}else{
			_sceVu0RotMatrixX( &l_world, &world, rot_x );
			_sceVu0RotMatrixY( &l_world, &l_world, rot_y );
		}
		_RotVector( &data->dvec, &l_world, &fvtemp );

		//プリムデータ初期化
		rot_x = PI * frnd();
		pos0->vx = pos.vx + add.vx * rnd();
		pos0->vy = pos.vy + add.vy * rnd();
		pos0->vz = pos.vz + add.vz * rnd();
		pos0->vw = 1.0f;
		DG_COPY_VEC(pos1,pos0);

		uvrgbwh0->r = uvrgbwh1->r = (color>>24)&0xff;
		uvrgbwh0->g = uvrgbwh1->g = (color>>16)&0xff;
		uvrgbwh0->b = uvrgbwh1->b = (color>>8)&0xff;
		uvrgbwh0->a = uvrgbwh1->a = 128;

		uvrgbwh0->u0 = uvrgbwh1->u0 = u[0];
		uvrgbwh0->v0 = uvrgbwh1->v0 = v[0];
		uvrgbwh0->u1 = uvrgbwh1->u1 = u[1];
		uvrgbwh0->v1 = uvrgbwh1->v1 = v[1];
		
		uvrgbwh0->f0 = uvrgbwh1->f0 = 0;
		uvrgbwh0->f1 = uvrgbwh1->f1 = 0;
		uvrgbwh0->q0 = uvrgbwh1->q0 = 4096;
		uvrgbwh0->q1 = uvrgbwh1->q1 = 4096;

		uvrgbwh0->w = uvrgbwh1->w = (int)( data->w = w*cosf(rot_x) );
		uvrgbwh0->h = uvrgbwh1->h = (int)( data->h = w*sinf(rot_x) );

		data->dw = 8.0f*cosf(rot_x);
		data->dh = 8.0f*sinf(rot_x);
		pos0++; pos1++;
		uvrgbwh0++; uvrgbwh1++;
		data++;
	}
	return (0);
}
/*
FVECTOR		*bound:		生成平面もしくはポイント
FVECTOR		*force:		方向（ＶＷは速度）
int			num:		数
int			texcode:
int			color:		色
int			w,h

*/

void *NewDebris_Tex( FVECTOR *bound, FVECTOR *force,
					 int num, int texcode,
					 int color, float w )
{
	Work		*work ;

	work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ) + sizeof(DATA)*num*16 );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, bound, force, num, texcode, color, w ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;
}

void *NewDebris_Tex_Demo( FVECTOR *bound0, FVECTOR *bound1, FVECTOR *force,
						 float pow, int num, int objcode,
						 int color, float w)
{
	FVECTOR bound[2];
	FVECTOR vec;
	
	DG_COPY_VEC(&bound[0],bound0);
	DG_COPY_VEC(&bound[1],bound1);
	DG_COPY_VEC(&vec,force);
	vec.vw = pow;

	return NewDebris_Tex( bound, &vec, num, objcode, color, w);
}


void *NewDebris_Tex_Demo2( int name0, int name1,
						  float pow, int num, int objcode,
						  int color, float w )
{
	FVECTOR 	bound[2];
	FVECTOR 	vec;
	FMATRIX		world;
	EFTCONTROL	*cntrl0;
	EFTCONTROL	*cntrl1;

	cntrl0 = DM_GetEftControl( name0 );
	DG_COPY_VEC( &bound[0], &cntrl0->mov );
	if( name1 != 1 ){
		cntrl1 = DM_GetEftControl( name1 );
		DG_COPY_VEC( &bound[1], &cntrl1->mov );
	}else{
		DG_COPY_VEC( &bound[1], &cntrl0->mov );
	}
	DM_EftControlMatrix( cntrl0, &world );
	DG_COPY_VEC( &vec, (FVECTOR*)world.m[2] );
	vec.vw = pow;

	return NewDebris_Tex( bound, &vec, num, objcode, color, w );
}

void *NewDebris_Tex_DemoBlood( int name0, int name1,
						  float pow, int num, int objcode,
						  int color, float w )
{
	FVECTOR 	bound[2];
	FVECTOR 	vec;
	FMATRIX		world;
	EFTCONTROL	*cntrl0;
	EFTCONTROL	*cntrl1;

	if( GM_Configuration & GM_CONFIG_BLOOD_OFF ){
		return NULL;
	}

	cntrl0 = DM_GetEftControl( name0 );
	DG_COPY_VEC( &bound[0], &cntrl0->mov );
	if( name1 != 1 ){
		cntrl1 = DM_GetEftControl( name1 );
		DG_COPY_VEC( &bound[1], &cntrl1->mov );
	}else{
		DG_COPY_VEC( &bound[1], &cntrl0->mov );
	}
	DM_EftControlMatrix( cntrl0, &world );
	DG_COPY_VEC( &vec, (FVECTOR*)world.m[2] );
	vec.vw = pow;

	return NewDebris_Tex( bound, &vec, num, objcode, color, w );
}
