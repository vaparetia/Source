//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	line_smoke.c
	ラインスモーク
	
	2000/01/16 T.Shibata
	
	$Id: line_smoke.c,v 1.1.1.3 2002/11/19 11:48:37 Yoshizawa1 Exp $

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
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"def_dma.h"
#include	"../util/ts_util.h"

#define CLOCK_COUNT	(BP_BASE_TICK())

#define FABS(_x) ({float _a = _x;asm("abs.s %0,%1":"=f"(_a):"f"(_a));(_a);})
#define	PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)

#define		N_JOINT			(32)		//今のところ64まで
#define		N_ALLVERTS		(2*N_JOINT)
#define		N_PRIMS			(1)
#define		N_VERTS			(64)
#define		SIZE			(9.0f)//(2.5f)

typedef	struct	{
	GV_ACT_EX		actor;
	DG_PRIM2		*prim;
	FMATRIX			*world;
	FVECTOR			shift;
	int				life;
	int				timer;


	FVECTOR			center;
	FVECTOR 		joint_pos[N_JOINT];
	FVECTOR 		joint_dvec[N_JOINT];
	float			joint_alpha[N_JOINT];
	float		    joint_size[N_JOINT];

	float			rot_x;
	float			rot_y;
	float			size;
	float			rate;

	float			max_size;
	float			pow;
} Work ;

static FVECTOR jvec_z = { 0.0f, 1.0f, 0.0f, 0.0f };
static FVECTOR jvec_out = { 0.0f, 0.0f, 1.0f, 0.0f };

static void Smoke_Act( Work *work )
{
	FVECTOR _InitFvector[2] = {
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f, 0.0f },
	};
	int					i,clock;
	DG_PRIM2			*prim = work->prim;

	FVECTOR				*now_jpos, *pre_jpos;
	FVECTOR				*now_jvec, *pre_jvec;
	FVECTOR				*pos, *eye_z = (FVECTOR*)DG_Chanls[0].eye.m[2];//wind, dpos,
	FMATRIX				fmtemp;
	DG_PRIM2_UVRGB		*uvrgb0;
	SVECTOR				rot;
	float				*size, *alpha;

	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;
	
	
	pos      = prim->pos[clock];
	uvrgb0   = prim->uvrgb[clock];
	now_jpos = work->joint_pos;
	pre_jpos = now_jpos+1;
	now_jvec = work->joint_dvec;
	pre_jvec = now_jvec+1;
	size = work->joint_size;
	alpha = work->joint_alpha;
	
	//風加工
	
	for( i = 0; i < N_JOINT-1; i++ ){
		_sceVu0AddVector( now_jpos, pre_jpos, pre_jvec );
		//_sceVu0AddVector( now_jvec, &wind, pre_jvec );
		DG_COPY_VEC( now_jvec, pre_jvec );
		//_sceVu0MulVector( now_jvec, &Smoke_Rgst, now_jvec );
		
		//size[0] = size[1] + 3.0f;// + fpu_Abs(work->csmk_size2)/2.0f ;//2.0f;
		size[0] = size[1];//2.0f;
		
		//now_jvec->vy += 0.2f;

		//進んだ方向に回転 及び カメラ方向に回転 マトリクス生成
		TS_MakeMatrix2( &fmtemp, eye_z, now_jvec, now_jpos );
		_InitFvector[0].vx = *size;
		_InitFvector[1].vx = -*size;
		DG_SetPos( &fmtemp );
		DG_PutVector( _InitFvector, pos, 2 );

#if 0
		if(work->life - work->timer < N_JOINT){
			if(work->life - work->timer > i){
				uvrgb0[0].a = uvrgb0[1].a = 0;
			}else{
				uvrgb0[0].a = uvrgb0[1].a = (int)((float)(i) * (float)work->timer/(float)work->life);
			}
		}else{
			uvrgb0[0].a = uvrgb0[1].a = (int)((float)(i) * (float)work->timer/(float)work->life);
		}
#else
		alpha[0] = alpha[1]-1.0f;
		if(alpha[0] < 0.0f)alpha[0]=0.0f;
		uvrgb0[0].a = uvrgb0[1].a = (int)alpha[0];
#endif
//		uvrgb0[0].a = uvrgb0[1].a = 0x80;
		//printf("a[0] %d\n",uvrgb0[0].a);
		//PRINT_PFVEC(i,now_jpos);

		now_jpos++; now_jvec++;
		pre_jpos++; pre_jvec++;
		size++; alpha++;
		
		pos+=2;
		uvrgb0+=2;
	}

	uvrgb0[0].a = uvrgb0[1].a = 0;

	DG_COPY_VEC( now_jpos, &work->center );

	//jdvec 初期化
	TS_MakeMatrix2( &fmtemp, &jvec_z, &jvec_out, now_jpos );
	rot.vx = (short)fpu_Abs( work->rot_x );
	rot.vy = (short)fpu_Abs( work->rot_y );
	rot.vz = 0;
	now_jvec->vx = 0.0f;
	now_jvec->vy = 0.0f;
	now_jvec->vz = work->pow;
	now_jvec->vw = 0.0f;

	DG_SetPos( &fmtemp );
	DG_RotatePos( &rot );
	DG_RotVector( now_jvec, now_jvec, 1 );

	//PRINT_PFVEC( 666, next_jvec );
	
	*size = fpu_Abs(work->size) + 4.0f;
	*alpha = 32.0f * (float)work->timer/(float)work->life;
	//printf("alpha %f\n",*alpha);
	//進んだ方向に回転 及び カメラ方向に回転 マトリクス生成
	TS_MakeMatrix2( &fmtemp, eye_z, now_jvec, now_jpos );
	//回転 と タバコの端に移動
	_InitFvector[0].vx = *size;
	_InitFvector[1].vx = -*size;
	DG_SetPos( &fmtemp );
	DG_PutVector( _InitFvector, pos, 2 );
	
	//PRINT_PFVEC( 32, now_jpos );
	
#define MAX_YURAGI (256.0f)
	
	work->rot_x += 10.0f * rnd();
	if( work->rot_x > MAX_YURAGI ) work->rot_x = -MAX_YURAGI;
	work->rot_y += 10.0f * rnd();
	if( work->rot_y > MAX_YURAGI ) work->rot_y = -MAX_YURAGI;

	work->size += 0.1f * rnd();
	if( work->size > work->max_size ) work->size -= work->max_size*2.0f;

}


static void Act( Work *work )
{
	if(work->world){		
		DG_SetPos(work->world);
		DG_PutVector(&work->shift,&work->center,1);
	}else{
		DG_COPY_VEC(&work->center, &work->shift);
	}
	Smoke_Act( work );

	if(--work->timer < 0){
		DG_InvisiblePrim2(work->prim);
		GV_DestroyActor( work );
	}
}

static void Die( Work *work )
{
	if(work->prim) GM_FreePrim2(work->prim);
}

static DG_PRIM2 *InitSmokePrim( Work *work, int tex_code )
{
	int					i,u0,v0,u1,v1;
	DG_PRIM2 			*prim;
	DG_TEX				*tex;
	FVECTOR				*pos0, *pos1;
	DG_PRIM2_UVRGB		*uvrgb0, *uvrgb1;
	
	FVECTOR				*joint = work->joint_pos;
	FVECTOR				*joint_dvec = work->joint_dvec;
	float				*size = work->joint_size;
	float				*alpha = work->joint_alpha;

	tex = DG_GetTexture(tex_code);
	if(!tex){ printf("ERR!! NO TEX!! cgrt.c\n"); return (NULL); }

	prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SHADE,
						 N_PRIMS,
						 N_VERTS );
	if(!prim){ printf("ERR!! MAKE PRIM2!! cgrt.c\n"); return (NULL); }

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	// まだチャンネル１～３は無いが取りあえず フラグ立て
	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );

	pos0 = prim->pos[0];
	pos1 = prim->pos[1];
	uvrgb0 = prim->uvrgb[0];
	uvrgb1 = prim->uvrgb[1];

	for( i = 0; i < N_ALLVERTS; i++ ){
		DG_COPY_VEC( pos0, &work->center );
		DG_COPY_VEC( pos1, &work->center );

		uvrgb0->r = 0x80;
		uvrgb0->g = 0x80;
		uvrgb0->b = 0x80;
		//uvrgb0->a = 0x20 - (i/2);
		uvrgb0->a = 0x40;
		
		uvrgb0->q = 4096;
		uvrgb0->f = (i<2)?VERT_KICK_CODE:DRAW_KICK_CODE;
		
		uvrgb0->u = (i&1)?u1:u0;
		//uvrgb0->v = (i&4)?v1:v0;//FTOI12((float)(i/2) * 1.0f/32.0f * tex->v_scale + tex->v_offset );
		uvrgb0->v = FTOI12((float)(i/2) * 1.0f/32.0f * tex->v_scale + tex->v_offset );

		*uvrgb1 = *uvrgb0;
#if 1
		if( i < N_JOINT ){

			DG_COPY_VEC( joint, &work->center );
			DG_COPY_VEC( joint_dvec, &DG_ZeroVector );
			*size = 0.0f;
			*alpha = 0.0f;

//			PRINT_PFVEC(i,joint);
			joint++;joint_dvec++;
			alpha++; size++;

		}
#endif
		//*size = 0.0f;
		//*alpha = 0.0f;


		pos0++; pos1++;
		uvrgb0++; uvrgb1++;
	}
	work->rot_x = MAX_YURAGI * rnd();
	work->rot_y = MAX_YURAGI * rnd();
	work->size = 0.0f;
	
	return prim;
}

static int GetResources( Work *work, FMATRIX *world, FVECTOR *shift, int life)
{
	DG_COPY_VEC(&work->shift,shift);
	work->world = world;
	work->life = life;
	work->timer = life;
	work->rate = 1.0f;
	if(work->world){		
		DG_SetPos(work->world);
		DG_PutVector(&work->shift,&work->center,1);
	}else{
		DG_COPY_VEC(&work->center, &work->shift);
	}
	//work->prim = InitSmokePrim( work, 2133533 );
	work->prim = InitSmokePrim( work, 1084957 );
	if(!work->prim) return -1;
	return 0;
}

void *NewLineSmoke( FMATRIX *world, FVECTOR *shift, int life, float pow, float max_size )
{
	Work		*work ;
		
	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;
		work->pow = pow;
		work->max_size = max_size;
		if ( GetResources( work, world, shift, life ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}
	//printf("aaa\n");
	return (void *)work ;
}
