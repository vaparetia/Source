//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    gnrl_sprt.c
    汎用スプライト表示
	2000/02/23 T.Shibata
	
	$Id: gnrl_sprt2.c,v 1.1.1.3 2002/11/19 11:48:37 Yoshizawa1 Exp $

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

#include	"../util/ts_util.h"


#define		PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)

#define		N_PRIMS		(1)
#define		N_VERTS		(1)

#define		DEF_COL_R	(0x80)
#define		DEF_COL_G	(0x80)
#define		DEF_COL_B	(0x80)
#define		DEF_COL_A	(0x80)

#define		GET_COL_R(_rgba)	((_rgba)&0xff)
#define		GET_COL_G(_rgba)	(((_rgba)>>8)&0xff)
#define		GET_COL_B(_rgba)	(((_rgba)>>16)&0xff)
#define		GET_COL_A(_rgba)	(((_rgba)>>24)&0xff)

#define		DEF_SIZE	(400)

//内部フラグ
#define		GSPRT_FLAGS_RGB		(0x0001)
#define		GSPRT_FLAGS_FADE	(0x0002)
#define		GSPRT_FLAGS_INVSBL	(0x0100)


//外部モード
#define GSPRT_MODE_FIXCOR	(0x00100000)	// カラー固定起動時のみ
#define GSPRT_MODE_FOG		(0x00200000)	// フォグが掛かる
#define GSPRT_MODE_TEX		(0x00000000)	// 描画モード　テクスチャーそのまま　起動時のみ
#define GSPRT_MODE_ADD		(0x00010000)	// 描画モード　加算　起動時のみ
#define GSPRT_MODE_SUB		(0x00020000)	// 描画モード　減算　起動時のみ
#define GSPRT_MODE_ALPHA	(0x00030000)	// 描画モード　アルファ　起動時のみ

#define GSPRT_MODE_ALP_CK	(0x00030000)	// 描画モード　チェック用

#define GSPRT_MODE_VSBL		(0x80000001)	// 表示にする
#define GSPRT_MODE_INVSBL	(0x80000002)	// 非表示にする
#define GSPRT_MODE_KILL		(0x80000010)	// 非表示にして殺す

#define GSPRT_MODE_CALL		(0x80000000)	// コールチェック

typedef struct {
	GV_ACT_EX	actor;
	DG_PRIM2	*prim;
	int			life;
	FMATRIX		*root;
	FVECTOR		pos;
	float		shift;

	FVECTOR		color;
	FVECTOR		diff_col;
	FVECTOR		to_col;
	int			col_timer;
	int			fade_timer;
	int			timer;
	int			act_time;
	int			sleep_time;
	int			flags;
	int			name;
} Work ;

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
		  case 0:
			//表示
			work->flags &= ~(GSPRT_FLAGS_INVSBL);
			DG_VisiblePrim2(work->prim);				
			break;
		  case 1:
			//非表示
			work->flags |= GSPRT_FLAGS_INVSBL;
			DG_InvisiblePrim2(work->prim);
			break;
		  case 2:
			//カラー変更
			work->col_timer = DIRECT_TICK(msg->message[4]);
			if(work->col_timer){
				work->to_col.vx = (float)msg->message[1];
				work->to_col.vy = (float)msg->message[2];
				work->to_col.vz = (float)msg->message[3];
				work->flags |= GSPRT_FLAGS_RGB;
				work->diff_col.vx = (work->to_col.vx-work->color.vx)/(float)work->col_timer;
				work->diff_col.vy = (work->to_col.vy-work->color.vy)/(float)work->col_timer;
				work->diff_col.vz = (work->to_col.vz-work->color.vz)/(float)work->col_timer;
			}else{
				work->color.vx = (float)msg->message[1];
				work->color.vy = (float)msg->message[2];
				work->color.vz = (float)msg->message[3];
			}
			break;
		  case 3:
			//強制フェードインアウト
			work->fade_timer = DIRECT_TICK(msg->message[2]);
			if(work->fade_timer){
				work->flags |= GSPRT_FLAGS_FADE;
				work->to_col.vw = (float)msg->message[1];
				work->diff_col.vw = (work->to_col.vw-work->color.vw)/(float)work->col_timer;
			}else{
				work->color.vw = (float)msg->message[1];
			}
			break;
		  default:
			printf("gnrl_sprt.c Msg Err!![%d]\n",num);
			break;
		}
		msg--;
	}
}
static void Act(Work *work)
{	
	int					clock;
	FVECTOR				*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	FVECTOR				fvtemp;
	//タイマー
#if 0
	if(--work->life < 0){
		work->flags |= GSPRT_FLAGS_INVSBL;
		DG_InvisiblePrim2(work->prim);
		GV_DestroyActor( work );
	}
#endif
	CheckMesgParam( work );
	//非表示の時処理しない
	if(work->flags & GSPRT_FLAGS_INVSBL) return;

	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;

	pos = work->prim->pos[clock];
	uvrgbwh = work->prim->uvrgb[clock];

	DG_SetPos( work->root );
	DG_PutVector( &work->pos, pos, 1 );
	_sceVu0SubVector( &fvtemp, (FVECTOR*)DG_Chanls[0].eye.m[3], pos );
	_sceVu0Normalize( &fvtemp, &fvtemp );
	_sceVu0ScaleVectorXYZ( &fvtemp, &fvtemp, work->shift );
	_sceVu0AddVector( pos, pos, &fvtemp );

	if(work->flags & GSPRT_FLAGS_RGB){
		work->color.vx += work->diff_col.vx;
		work->color.vy += work->diff_col.vy;
		work->color.vz += work->diff_col.vz;
		if(--work->fade_timer < 0){
			work->color.vx = work->to_col.vx;
			work->color.vy = work->to_col.vy;
			work->color.vz = work->to_col.vz;
			work->flags &= ~GSPRT_FLAGS_RGB;
		}
	}

	if(work->flags & GSPRT_FLAGS_FADE){
		work->color.vw += work->diff_col.vw;
		if(--work->fade_timer < 0){
			work->color.vw = work->to_col.vw;
			work->flags &= ~GSPRT_FLAGS_FADE;
			if(work->color.vx > 127.0f){
				work->timer = 17;
			}else{
				work->timer = 33 + work->act_time;
			}
		}
	}else{
		if(++work->timer <= 16){
			work->color.vw += 8.0f;
		}else if(work->timer > (16 + work->act_time) && work->timer <= (16 + work->act_time + 16)){
			work->color.vw -= 8.0f;
		}else if(work->timer > (16 + work->act_time + 16 + work->sleep_time)){
			work->timer = 0;
		}
	}

	uvrgbwh->r = (int)work->color.vx;
	uvrgbwh->g = (int)work->color.vy;
	uvrgbwh->b = (int)work->color.vz;
	uvrgbwh->a = (int)work->color.vw;
}

static void Die(Work *work)
{
	if(work->prim) GM_FreePrim2(work->prim);
	//printf("die\n");
}

static void InitPrimData( Work *work, DG_PRIM2 *prim, DG_TEX *tex, int w, int h )
{
	FVECTOR				*pos0,*pos1;
	DG_PRIM2_UVRGBWH	*uvrgbwh0,*uvrgbwh1;

	pos0 = prim->pos[0];
	pos1 = prim->pos[1];
	uvrgbwh0 = prim->uvrgb[0];
	uvrgbwh1 = prim->uvrgb[1];

	printf("pointer [%p][%p]\n",pos0,pos1);

	DG_COPY_VEC( pos0, &work->pos );
	DG_COPY_VEC( pos1, &work->pos );

	uvrgbwh0->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	uvrgbwh0->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	uvrgbwh0->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	uvrgbwh0->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );

	uvrgbwh0->f0 = 0x8fff;
	uvrgbwh0->f1 = 0x0fff;
	uvrgbwh0->q0 = 4096;
	uvrgbwh0->q1 = 4096;

	uvrgbwh0->w = w;
	uvrgbwh0->h = h;

	uvrgbwh0->r = (int)work->color.vx;
	uvrgbwh0->g = (int)work->color.vy;
	uvrgbwh0->b = (int)work->color.vz;
	uvrgbwh0->a = (int)work->color.vw;
	
	*uvrgbwh1 = *uvrgbwh0;

}


static int GetResources( Work *work, int tex_code,
						 FMATRIX *world, FVECTOR *pos, float shift,
						 int init_rgba, int w,
						 int life, int mode, int act_time, int sleep_time )
{
	DG_PRIM2	*prim;
	DG_TEX		*tex;
	int prim_mode = DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA;

	tex = DG_GetTexture(tex_code);
	if(!tex) { printf("ERR!! NO TEX!! <gnrl_sprt2.c>\n"); return(-1); }

	if( mode & GSPRT_MODE_FOG ) prim_mode |= DG_PRIM2_FOG;
	prim = work->prim = GM_MakePrim2(prim_mode, N_PRIMS, N_VERTS );
	if(!prim){ printf("ERR!! MAKE PRIM!! <gnrl_sprt2.c>\n"); return (-1); }
	DG_ConfigPrim2Tex( prim, tex );

	switch(mode & GSPRT_MODE_ALP_CK){
	  case GSPRT_MODE_ADD:
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
		break;
	  case GSPRT_MODE_SUB:
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
		break;
	  case GSPRT_MODE_ALPHA:
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
		break;
	  default:
		break;
	}
	// まだチャンネル１～３は無いが取りあえず フラグ立て
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	// DataInit
	printf("%p\n",pos);
	DG_COPY_VEC(&work->pos,pos);
	work->root = world;
	work->shift = shift;
	work->life = life;
	work->color.vx = (float)GET_COL_R(init_rgba);
	work->color.vy = (float)GET_COL_G(init_rgba);
	work->color.vz = (float)GET_COL_B(init_rgba);
	work->color.vw = 0.0f;//(float)GET_COL_A(work->rgba);

	work->act_time = DIRECT_TICK(act_time);
	work->sleep_time = DIRECT_TICK(sleep_time);
	work->flags = 0;
	work->timer = 0;
	InitPrimData( work, prim, tex, w, w );

	//printf("flags = %x\n",work->flags);
	return (0);
}

void *NewGeneralSprite2( int name, int tex_code, FMATRIX *world, FVECTOR *pos, float shift,
						 int init_rgba, int w,
						 int life, int mode, int act_time, int sleep_time )
{
	Work *work = NULL;

	work = (Work*)GV_NewEffect(GV_ACTOR_USER,sizeof(Work));
	if(work){
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor );
		work->name = name;
	printf("%p\n",pos);
		if(GetResources(work,tex_code,world,pos,shift,init_rgba,w,life,mode,act_time,sleep_time) < 0){
			GV_DestroyActor(work) ;
			return NULL ;
		}
		printf("new sprite\n");
	}

	return (void *)work ;
}
