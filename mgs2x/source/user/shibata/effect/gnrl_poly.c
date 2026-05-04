//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    gnrl_sprt.c
    汎用スプライト表示
	2000/09/28 T.Shibata
	
	$Id: gnrl_poly.c,v 1.1.1.3 2002/11/19 11:48:36 Yoshizawa1 Exp $

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#define		N_PRIMS		(1)
#define		N_VERTS		(4)

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
#define		GPOLY_FLAGS_MODE	(0x0001)
#define		GPOLY_FLAGS_LIFE	(0x0002)
#define		GPOLY_FLAGS_SHIFT	(0x0004)
#define		GPOLY_FLAGS_RGBA	(0x0008)

#define		GPOLY_FLAGS_MESG	(0x0010)

#define		GPOLY_FLAGS_INVSBL	(0x0100)


#define MEM_SCR_POS		((void*)(SCRPAD_ADDR))
#define MEM_SCR_UV		((void*)(SCRPAD_ADDR+sizeof(FVECTOR)*256))

//外部モード
#define GPOLY_MODE_FOG		(0x0010)	// フォグが掛かる
#define GPOLY_MODE_TEX		(0x0000)	// 描画モード　テクスチャーそのまま　起動時のみ
#define GPOLY_MODE_ADD		(0x0001)	// 描画モード　加算　起動時のみ
#define GPOLY_MODE_SUB		(0x0002)	// 描画モード　減算　起動時のみ
#define GPOLY_MODE_ALPHA	(0x0003)	// 描画モード　アルファ　起動時のみ

#define GPOLY_MODE_ALP_CK	(0x0003)	// 描画モード　チェック用

typedef struct {
	GV_ACT_EX	actor ;
	DG_PRIM2	*prim;
	DG_TEX		*tex;

	int			poly_flags[16];
	int			flags;
	int			name;
	int			map;

	int			n_poly;
	int			now_tex_num[16];
	int			divi_w;
	int			divi_h;

	float		alpha[16];
	float		to_alpha[16];
	float		diff_alpha[16];
	int			time[16];
} Work ;


extern int DM_FrameSkip ;

static void SetStrUV( DG_PRIM2_UVRGB *uvrgb, DG_TEX *tex, int index, int divi_w, int divi_h  )
{
	int			tw,th,off_u,off_v,tex_w,tex_h;
	int			u12[2],v12[2];
	int			u,v,w,h;

	DG_GetTexelInfo( &tex_w, &tex_h, &off_u, &off_v, tex );

	tw = ( tex->tex_trans.tex0.data >> 26 ) & 0x0f ;
	th = ( tex->tex_trans.tex0.data >> 30 ) & 0x0f ;
	tw = 1 << tw ;
	th = 1 << th ;

	w = tex_w/divi_w;
	h = tex_h/divi_h;

	u = (index%divi_w)*w;
	v = (index/divi_w)*h;

	//printf("[%d,%d][%d,%d]\n",u,u+w,v,v+h);

	u12[0] = FTOI12( ((float)(off_u + u)+0.5f)/(float)tw );
	v12[0] = FTOI12( ((float)(off_v + v)+0.5f)/(float)th );
	u12[1] = FTOI12( ((float)(off_u + u + w)-0.5f)/(float)tw );
	v12[1] = FTOI12( ((float)(off_v + v + h)-0.5f)/(float)th );
#if 0
	printf("u[%f]<->[%f]\n", tex->u_offset, tex->u_offset+tex->u_scale );
	printf("v[%f]<->[%f]\n", tex->v_offset, tex->v_offset+tex->v_scale );
	printf("u0 %f: u1 %f:\n",
		   ((float)(off_u + u)+0.5f)/(float)tw,
		   ((float)(off_u + u + w)-0.5f)/(float)tw);
	printf("v0 %f: v1 %f:\n",
		   ((float)(off_v + v)+0.5f)/(float)th,
		   ((float)(off_v + v + h)-0.5f)/(float)th);
#endif
	uvrgb[0].u = u12[0];
	uvrgb[0].v = v12[0];
	uvrgb[1].u = u12[1];
	uvrgb[1].v = v12[0];
	uvrgb[2].u = u12[0];
	uvrgb[2].v = v12[1];
	uvrgb[3].u = u12[1];
	uvrgb[3].v = v12[1];	
}

static int SetPrimAlpha( DG_PRIM2 *prim, int flag, float *alpha, int n_poly )
{
	int ret = flag,i;
	DG_PRIM2_UVRGB	*uvrgb;
	if( !(ret & 0x0001) ){
		DG_SwitchBuffPrim2( prim );
		ret |= 0x0001;
	}

	uvrgb = prim->uvrgb[prim->buffer_clock];
	for( i = 0; i < n_poly; i++ ){
		
		uvrgb[0].a = (int)*alpha;
		uvrgb[1].a = (int)*alpha;
		uvrgb[2].a = (int)*alpha;
		uvrgb[3].a = (int)*alpha;
		
		alpha++;
		uvrgb+=4;
	}
	
	return ret;
}

static int AlphaAct( Work *work )
{
	int i,ret=0;
	for( i = 0; i < work->n_poly; i++ ){
		if( work->poly_flags[i] & 0x0001 ){
			work->alpha[i] += work->diff_alpha[i]*(float)(DM_FrameSkip+1);

			if( work->diff_alpha[i] > 0.0f ){
				if( work->alpha[i] > work->to_alpha[i] ){
					work->alpha[i] = work->to_alpha[i];
				}
			}else{
				if( work->alpha[i] < work->to_alpha[i] ){
					work->alpha[i] = work->to_alpha[i];
				}
			}

			work->time[i] -= DM_FrameSkip+1;
			if( work->time[i] < 0 ){
				work->poly_flags[i] &= ~0x0001;
				work->alpha[i] = work->to_alpha[i];
			}
			ret = 1;
		}
	}
	return ret;
}

static void Act(Work *work)
{	
	//
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
			work->flags &= ~(GPOLY_FLAGS_INVSBL);
			DG_VisiblePrim2(work->prim);				
			break;
		  case 1:
			//非表示
			work->flags |= GPOLY_FLAGS_INVSBL;
			DG_InvisiblePrim2(work->prim);
			break;
			
		  case 2:
			//切り替え
			{
				int clock = work->prim->buffer_clock;
				DG_PRIM2_UVRGB	*uvrgb;
				if( !(work->flags & 0x0001) )
            {
               int const buffSwitch = DG_SwitchBuffPrim2( work->prim );
               //バッファー切り替え
					work->flags |= 0x0001;
					clock = work->prim->buffer_clock;
					memcpy( work->prim->uvrgb[clock], work->prim->uvrgb[buffSwitch ^ clock], sizeof(DG_PRIM2_UVRGB)*work->n_poly*4 );
				}
				
				uvrgb = work->prim->uvrgb[clock];

				SetStrUV( &uvrgb[msg->message[1]*4], work->tex, msg->message[2], work->divi_w, work->divi_h );
			}
			break;
		  case 3:
			{
				work->poly_flags[msg->message[1]] |= 0x0001;
				work->time[msg->message[1]] = msg->message[2];
				work->to_alpha[msg->message[1]] = (float)msg->message[3];
				work->diff_alpha[msg->message[1]] =
					((float)msg->message[3]-work->alpha[msg->message[1]])/(float)msg->message[2];
				
			}
			break;
		  default:
			printf("gnrl_sprt.c Msg Err!![%d]\n",num);
			break;
		}
		msg--;
	}
	if( AlphaAct( work ) ){
		work->flags = SetPrimAlpha( work->prim, work->flags, work->alpha, work->n_poly );
	}
	
	work->flags &= ~0x0001;
}

static void Die(Work *work)
{
	if(work->prim) GM_FreePrim2(work->prim);
	//printf("die\n");
}


static int InitPolyAddPrim( DG_PRIM2 *prim, DG_TEX *tex, int n_polys, int n_pverts, int divi_w, int divi_h )
{
	int				i,j;
	int				u[2],v[2];
	FVECTOR			*pos = MEM_SCR_POS;
	DG_PRIM2_UVRGB	*uvrgb = MEM_SCR_UV;

	u[0] = FTOI12(0.0f * tex->u_scale + tex->u_offset);
	v[0] = FTOI12(0.0f * tex->v_scale + tex->v_offset);
	u[1] = FTOI12(1.0f * tex->u_scale + tex->u_offset);
	v[1] = FTOI12(1.0f * tex->v_scale + tex->v_offset);
	
	for( i = 0; i < n_polys; i++ ){
		int		color;
		int		init_uv;

		if( GCL_GetNextOption() != 'c' ) return -1;
		color = GCL_GetNextInt()&0xff;
		color |= (GCL_GetNextInt()&0xff)<<8;
		color |= (GCL_GetNextInt()&0xff)<<16;
		color |= (GCL_GetNextInt()&0xff)<<24;

		if( GCL_GetNextOption() != 'i' ) return -1;
		init_uv = GCL_GetNextInt();
		if( GCL_GetNextOption() != 'p' ) return -1;
		for( j = 0; j < n_pverts; j++ ){
			pos->vx = (float)GCL_GetNextInt();//100.0f;
			pos->vy = (float)GCL_GetNextInt();//100.0f;
			pos->vz = (float)GCL_GetNextInt();//100.0f;
			pos->vw = 1.0f;

			uvrgb->r = GET_COL_R(color);
			uvrgb->g = GET_COL_G(color);
			uvrgb->b = GET_COL_B(color);
			uvrgb->a = GET_COL_A(color);
			uvrgb->f = (j<2)?VERT_KICK_CODE:DRAW_KICK_CODE;
			uvrgb->q = 4096;
			uvrgb->u = u[j&1];
			uvrgb->v = v[j>>1];

			pos++;
			uvrgb++;
		}
		SetStrUV( uvrgb-4, tex, init_uv, divi_w, divi_h  );
	}

	TS_Scr_Mem( prim->pos[0], MEM_SCR_POS, sizeof(FVECTOR), n_polys*n_pverts );
	TS_Scr_Mem( prim->pos[1], MEM_SCR_POS, sizeof(FVECTOR), n_polys*n_pverts );
	TS_Scr_Mem( prim->uvrgb[0], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), n_polys*n_pverts );
	TS_Scr_Mem( prim->uvrgb[1], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), n_polys*n_pverts );

	return 0;
}

static int GetResources_Scn( Work *work )
{
//	FVECTOR		pos[4];
	DG_PRIM2	*prim = NULL;
	DG_TEX		*tex = NULL;
	int			tex_code,i;//,color;
	int			mode,prim_mode = DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SHADE;
//	int			w,h,i;


	tex_code = GCL_GetOptionValue( 't', 0 );
	mode = GCL_GetOptionValue( 'm', 0 );
	
	if(GCL_GetOption('d')){
		work->divi_w = GCL_GetNextInt();
		work->divi_h = GCL_GetNextInt();
	}else{
		work->divi_w = 1;
		work->divi_h = 1;
	}

	work->n_poly = GCL_GetOptionValue( 'n', 0 );
	if( work->n_poly > 16 ) work->n_poly = 16;

	work->tex = tex = DG_GetTexture(tex_code);
	if(!tex) { printf("ERR!! NO TEX!! <gnrl_poly.c>\n"); return(-1); }
	if( mode & GPOLY_MODE_FOG ) prim_mode |= DG_PRIM2_FOG;

	prim = work->prim = GM_MakePrim2( prim_mode,
									  1,
									  work->n_poly*4 );
	if(!prim){ printf("ERR!! MAKE PRIM!! <gnrl_poly.c>\n"); return (-1); }
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );
	DG_ConfigPrim2Tex( prim, tex );
	switch(mode & GPOLY_MODE_ALP_CK){
	case GPOLY_MODE_ADD:
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
		break;
	case GPOLY_MODE_SUB:
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
		break;
	case GPOLY_MODE_ALPHA:
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
		break;
	default:
		break;
	}
	
	InitPolyAddPrim( prim, tex, work->n_poly, 4, work->divi_w, work->divi_h );

	for( i = 0; i < 16; i++ ){
		work->alpha[i] = 128.0f;
		work->to_alpha[i] = 128.0f;
		work->diff_alpha[i] = 0.0f;
		work->time[i] = 0;
	}
	return (0);
}

void *NewGeneralPoly_Scn( int name, int map )
{
	Work *work = NULL;

	work = (Work*)GV_NewEffect(GV_ACTOR_USER,sizeof(Work));
	if(work){
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor );
		work->name = name;
		if(GetResources_Scn(work) < 0){
			GV_DestroyActor(work) ;
			return NULL ;
		}
		//printf("new sprite\n");
	}

	return (void *)work ;
}
