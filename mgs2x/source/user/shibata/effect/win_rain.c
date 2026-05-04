//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	win_rain.c
	窓雨
	
	2000/10/19 T.Shibata
	
	$Id: win_rain.c,v 1.1.1.3 2002/11/19 11:48:42 Yoshizawa1 Exp $

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
#include "BP_RenderObj.h"

#define CLOCK_COUNT	(BP_BASE_TICK())

#define FABS(_x) ({float _a = _x;asm("abs.s %0,%1":"=f"(_a):"f"(_a));(_a);})
#define		PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)

//  １ベースとなる付着雨の帯
//  ２大きな流れ
//  ３水飛沫状のおかず
//  ４雨粒
//  ５流水
//  ６流水ぼかし

#define TEX_CODE_BASE (15090113)	//(GV_StrCode("wrain01_add_alp_ovl"))
#define TEX_CODE_FLW0 (15094209)	//(GV_StrCode("wrain02_add_alp_ovl"))
#define TEX_CODE_DECO (15098305)	//(GV_StrCode("wrain03_add_alp_ovl"))
#define TEX_CODE_DROP (15102401)	//(GV_StrCode("wrain04_add_alp_ovl"))
#define TEX_CODE_FLW1 (15106497)	//(GV_StrCode("wrain05_add_alp_ovl"))
#define TEX_CODE_FLW2 (15110593)	//(GV_StrCode("wrain06_add_alp_ovl"))

//#define TEX_CODE_BASE (6715088)	//(GV_StrCode(""))

#define		N_BUMPS	(2)
#define		N_PRIMS	(3)

#define N_ALLPRIMS	(N_BUMPS+N_PRIMS)

typedef	struct	{
	GV_ACT_EX		actor;
	FMATRIX			world;
	FVECTOR			center;
	SVECTOR			rot;
	int				width;
	int				height;
	int				name;
	int				flag;
	int				map;
	int				mode;

	float			add_vx[N_BUMPS];
	FVECTOR			verts[N_BUMPS][2][6];
	FVECTOR			shift[N_BUMPS];
	float			eheehe[N_BUMPS];
	

	float			min_v[N_ALLPRIMS];
	float			len_v[N_ALLPRIMS];
	float			now_v[N_ALLPRIMS];
	float			diff_v[N_ALLPRIMS];
	float			alpha[N_ALLPRIMS];
	float			add_alpha[N_ALLPRIMS];

	DG_PRIM2		*add[N_BUMPS];
	DG_PRIM2		*sub[N_BUMPS];

	DG_PRIM2		*prim[N_PRIMS];
} Work ;

static int CheckMesgParam( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;

	mes_num = GV_ReceiveMessage( work->name, &msg );

	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		default:
			break;
		}
		msg--;
	}

	return 0;
}

static void TexFlow_Act( FVECTOR *pos, DG_PRIM2_UVRGB *uvrgb,
						 FVECTOR *center, FVECTOR *shift, SVECTOR *rot, FVECTOR *init_pos,
						 float ratio, float scale, float off,
						 int alpha, int mode, int sub )
{	

	if(center){
		FVECTOR		fvtemp;
		if(sub){
			init_pos[2].vy = init_pos[3].vy = ratio * init_pos[0].vy + (1.0f - ratio) * init_pos[5].vy;
		}else{
			init_pos[2].vy = init_pos[3].vy = ratio * init_pos[0].vy + (1.0f - ratio) * init_pos[5].vy;
		}
		DG_SetPos2( &DG_ZeroVector, rot);
		DG_RotVector( shift, &fvtemp, 1);
		_sceVu0AddVector( &fvtemp, &fvtemp, center );
		DG_SetPos2( &fvtemp, rot);
		DG_PutVector( init_pos,pos,6);
	}else{
		//if(sub){
			pos[2].vy = pos[3].vy = ratio * pos[0].vy + (1.0f - ratio) * pos[5].vy;//-2.0f;
		//}else{
			pos[2].vy = pos[3].vy = ratio * pos[0].vy + (1.0f - ratio) * pos[5].vy;//+2.0f;
		//}
	}


	if(!mode){
		uvrgb[0].v = uvrgb[1].v = FTOI12(ratio * scale + off);
		uvrgb[2].v = uvrgb[3].v = FTOI12(off+scale);
		uvrgb[4].v = uvrgb[5].v = FTOI12((1.0f - ratio ) * scale + off);
	}else{
		uvrgb[0].v = uvrgb[1].v = FTOI12((1.0f - ratio ) * scale + off);
		uvrgb[2].v = uvrgb[3].v = FTOI12(off);
		uvrgb[4].v = uvrgb[5].v = FTOI12(ratio * scale + off);
	}
	uvrgb[0].a = alpha;
	uvrgb[1].a = alpha;
	uvrgb[2].a = alpha;
	uvrgb[3].a = alpha;
	uvrgb[4].a = alpha;
	uvrgb[5].a = alpha;
}


static void Act( Work *work )
{
	int			i,clock;
	DG_PRIM2	*prim;
	DG_PRIM2_UVRGB		*uvrgb;
	FVECTOR				*pos;
	float				alpha;
	
	CheckMesgParam( work );

	if( (work->map & GM_CurrentStageMap) != 0 ){
		if(!work->flag){
			for( i = 0; i < N_BUMPS; i++ ){
				DG_VisiblePrim2( work->add[i] ) ;
				DG_VisiblePrim2( work->sub[i] ) ;
			}
		}
		for( i = 0; i < N_PRIMS; i++ ){
			DG_VisiblePrim2( work->prim[i] ) ;
		}
	}else{
		if(!work->flag){
			for( i = 0; i < N_BUMPS; i++ ){
				DG_InvisiblePrim2( work->add[i] ) ;
				DG_InvisiblePrim2( work->sub[i] ) ;
			}
		}
		for( i = 0; i < N_PRIMS; i++ ){
			DG_InvisiblePrim2( work->prim[i] ) ;
		}
		return;
	}

	for( i = 0; i < N_PRIMS; i++ ){
		alpha = work->alpha[i];
		if(alpha > 1.0f) alpha = 2.0f - alpha;

		prim = work->prim[i];
		DG_SwitchBuffPrim2( prim );
		clock = prim->buffer_clock;
		uvrgb = prim->uvrgb[clock];
		pos = prim->pos[clock];
		TexFlow_Act( pos, uvrgb, NULL, NULL, NULL, NULL,
					 work->now_v[i], work->len_v[i],
					 work->min_v[i], (int)(12.0f*alpha)+16, work->mode&(1<<i), 0 );


		work->now_v[i] -= work->diff_v[i];// * rnd() + 1.0f/96.0f;
		if( work->now_v[i] < 0.0f ){
			work->now_v[i] = 1.0f + work->now_v[i];
			work->mode ^= (1<<i);
			//work->diff_v[i] = 1.0f/96.0f;//-1.0f/128.0f * rnd() + 1.0f/64.0f;
		}

		work->alpha[i] += work->add_alpha[i];
		if(work->alpha[i] > 2.0f){
			work->alpha[i] = 0.0f;//work->alpha[i] - 2.0f;
			work->add_alpha[i] = -1.0f/96.0f*rnd() + 1.0f/48.0f;
			//work->diff_v[i] = -1.0f/128.0f*rnd() + 1.0f/64.0f;
		}

		//NewLineView( pos, 6, 0x80, 0x20, 0x20 ); 
		
	}
if(!work->flag){
	for( ; i < N_ALLPRIMS; i++ ){
		alpha = work->alpha[i];
		if(alpha > 1.0f) alpha = 2.0f - alpha;

		prim = work->add[i-N_PRIMS];
		DG_SwitchBuffPrim2( prim );
		clock = prim->buffer_clock;
		uvrgb = prim->uvrgb[clock];
		pos = prim->pos[clock];
		TexFlow_Act( pos, uvrgb, &work->center, &work->shift[i-N_PRIMS], &work->rot, work->verts[i-N_PRIMS][0],
					 work->now_v[i], work->len_v[i],
					 work->min_v[i], (int)(128.0f*alpha), work->mode&(1<<i), 0 );

		prim = work->sub[i-N_PRIMS];
		DG_SwitchBuffPrim2( prim );
		clock = prim->buffer_clock;
		uvrgb = prim->uvrgb[clock];
		pos = prim->pos[clock];
		TexFlow_Act( pos, uvrgb, &work->center, &work->shift[i-N_PRIMS], &work->rot, work->verts[i-N_PRIMS][1],
					 work->now_v[i], work->len_v[i],
					 work->min_v[i], (int)(128.0f*alpha), work->mode&(1<<i), 1 );


		work->now_v[i] -= work->diff_v[i];// * rnd() + 1.0f/96.0f;
		if( work->now_v[i] < 0.0f ){
			work->now_v[i] = 1.0f + work->now_v[i];
			work->mode ^= (1<<i);
			//work->diff_v[i] = 1.0f/96.0f;//-1.0f/128.0f * rnd() + 1.0f/64.0f;
		}

		work->alpha[i] += work->add_alpha[i];
		work->shift[i-N_PRIMS].vx += work->add_vx[i-N_PRIMS];
		if(work->alpha[i] > 2.0f){
			work->alpha[i] = 0.0f;//work->alpha[i] - 2.0f;
			work->add_alpha[i] = -1.0f/32.0f*rnd() + 1.0f/16.0f;
						
			work->diff_v[i] = -1.0f/256.0f*rnd() + 1.0f/128.0f;

			DG_COPY_VEC(&work->shift[i-N_PRIMS],&DG_ZeroVector);
			work->shift[i-N_PRIMS].vx = (float)work->width*0.5*frnd();
			work->shift[i-N_PRIMS].vy = (float)work->width*0.5*frnd();
			work->add_vx[i-N_PRIMS] = frnd()*3.0f;
		}
		//NewLineView( pos, 6, 0x80, 0x20, 0x20 ); 

	}
}

	//NewLineView( pos, 6, 0x80, 0x20, 0x20 ); 
	//NewBoundingBoxView( &work->verts[0], &work->verts[5], 0x80, 0x20, 0x20 );
}

static void Die( Work *work )
{
	int i;

	for( i = 0; i < N_BUMPS; i++ ){
		if(work->add[i])GM_FreePrim2(work->add[i]);
		if(work->sub[i])GM_FreePrim2(work->sub[i]);
	}
	
	for( i = 0; i < N_PRIMS; i++ ){
		if(work->prim[i])GM_FreePrim2(work->prim[i]);
	}
}

static int GetOptions( Work *work )
{
	int		c=0,check = 0;

	while( (c = GCL_GetNextOption()) ){
		switch(c){
		case 'c':
			//printf("center\n");
			work->center.vx = (float)GCL_GetNextInt() ;
			work->center.vy = (float)GCL_GetNextInt() ;
			work->center.vz = (float)GCL_GetNextInt() ;
			work->center.vw = 1.0f ;
			check |= 1;
			break;
		case 's':
			//printf("size\n");
			work->width = (float)GCL_GetNextInt() ;
			work->height = (float)GCL_GetNextInt() ;
			//work->width = (float)GCL_GetNextInt() ;
			check |= (1<<1);
			break;
		case 'r':
			//printf("rot\n");
			work->rot.vx = GCL_GetNextInt();
			work->rot.vy = GCL_GetNextInt();
			work->rot.vz = GCL_GetNextInt();
			work->rot.pad = 0;
			check |= (1<<2);
			break;
		case 'm':
			work->flag = GCL_GetNextInt();
			break;
		default:
			break;
		}
	}

   // BP_WARNING The following 3 ifs do not work due to !a&b needing to be !(a&b)
	if((!check & (1<<0))) printf("GCL:ERR!::c_light_spot::set option 'c'\n");
	if((!check & (1<<1))) printf("GCL:ERR!::c_light_spot::set option 's'\n");
	if((!check & (1<<2))) printf("GCL:ERR!::c_light_spot::set option 'r'\n");	
	 
	return (check==0);
}

static int as_get_addl_prim_flags()
{
   // AS(JM) - There are certain areas where win_rain does not go into the MSAA buffer.
   if ( BP_Obj_CheckInsideAreaWhereWindowRainNoMSAA() )
   {
      return DG_PRIM2_NOMSAA;
   }
   else
   {
      return 0;
   }
}

static void *InitFlowPrim( int tec_code, int mode, FVECTOR *init_pos, int a )
{
	int					i;
	DG_PRIM2 			*prim;
	DG_TEX				*tex;
	FVECTOR				*pos0, *pos1;
	DG_PRIM2_UVRGB		*uvrgb0, *uvrgb1;
	int					u0,v0,u1,v1;

	tex = DG_GetTexture(tec_code);
	if(!tex){ printf("ERR!! NO TEX!! cgrt.c\n"); return (NULL); }
	
	prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SHADE | as_get_addl_prim_flags(),
						 1,
						 6 );
	if(!prim){ printf("ERR!! MAKE PRIM2!! win_rain.c\n"); return (NULL); }

	DG_ConfigPrim2Tex( prim, tex );
	if(mode){
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x80 ) );
	}else{
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x80 ) );
	}
	// まだチャンネル１～３は無いが取りあえず フラグ立て
	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	if(mode){
		prim->raise = 20000*a + 10000;
		//prim->raise = 20000*a + 20000;
	}else{
		prim->raise = 20000*a + 20000;
		//prim->raise = 20000*a + 10000;
	}
//	printf("raise[%4d]\n",prim->raise);
	u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );

	pos0 = prim->pos[0];
	pos1 = prim->pos[1];
	uvrgb0 = prim->uvrgb[0];
	uvrgb1 = prim->uvrgb[1];

	for( i = 0; i < 6; i++ ){
		DG_COPY_VEC( pos0, &init_pos[i] );
		DG_COPY_VEC( pos1, &init_pos[i] );

		uvrgb0->a = 0x80;
		
		if(mode){
			uvrgb0->r = 0x70;
			uvrgb0->g = 0x70;
			uvrgb0->b = 0x70;
		}else{
			uvrgb0->r = 0x70;
			uvrgb0->g = 0x70;
			uvrgb0->b = 0x70;
		}

		uvrgb0->q = 4096;
		uvrgb0->f = (i<2)?VERT_KICK_CODE:DRAW_KICK_CODE;
		
		uvrgb0->u = (i&1)?u1:u0;
		uvrgb0->v = 0;//FTOI12((float)(i/2) * 1.0f/2.0f * tex->v_scale + tex->v_offset );

		*uvrgb1 = *uvrgb0;
		pos0++; pos1++;
		uvrgb0++; uvrgb1++;
	}

	return prim;
}
static int tex_code[]={

	TEX_CODE_BASE,
	TEX_CODE_BASE,
	TEX_CODE_FLW2,

	TEX_CODE_DROP,
	TEX_CODE_DROP,
};

static int GetResources_S( Work *work )
{
	FVECTOR 			verts_add[6],verts_sub[6];
	FVECTOR				*pverts_add,*pverts_sub;
	int					i,j,k = N_BUMPS;
	DG_TEX				*tex;

	if(GetOptions( work )) return -1;

	DG_SetPos2(&work->center,&work->rot);

	for( k = 0; k < N_PRIMS; k++ ){
		pverts_add = verts_add;

		for( i = 0; i < 3; i++ ){
			for( j = 0; j < 2; j++ ){
				pverts_add->vx = (float)(work->width) * ((j&1)?1.0f:-1.0f);
				if( i == 1 ){
					pverts_add->vy = 0.0f;
				}else{
					pverts_add->vy = (float)(work->height) * ((i&2)?-1.0f:1.0f);
				}
				pverts_add->vz = 0.0f;//-20.0f;//-40.0f*(float)k-40.0f;
				pverts_add->vw = 1.0f;
				pverts_add++;
			}
		}
		pverts_add = verts_add;

		DG_PutVector(pverts_add,pverts_add,6);

		work->prim[k] = InitFlowPrim( tex_code[k], 1, verts_add, k );

		tex = DG_GetTexture(tex_code[k]);
		if(!tex){ printf("ERR!! NO TEX!! cgrt.c\n"); return (-1); }
		
		work->min_v[k] = 0.0f * tex->v_scale + tex->v_offset;
		work->len_v[k] = 1.0f * tex->v_scale + tex->v_offset - work->min_v[k];
		work->now_v[k] = 0.0f;
		work->diff_v[k] = -1.0f/128.0f*rnd() + 1.0f/64.0f;

		work->alpha[k] = rnd() * 2.0f;
		work->add_alpha[k] = -1.0f/128.0f*rnd() + 1.0f/64.0f;

		//DG_InvisiblePrim2(work->prim[k]);
	}

if(!work->flag){
	for( ; k < N_ALLPRIMS; k++ ){
		pverts_add = work->verts[k-N_PRIMS][0];//verts_add;
		pverts_sub = work->verts[k-N_PRIMS][1];//verts_sub;

		for( i = 0; i < 3; i++ ){
			for( j = 0; j < 2; j++ ){
				pverts_add->vx = (float)(work->width<<1) * ((j&1)?1.0f:-1.0f)+1.0f;
				pverts_sub->vx = (float)(work->width<<1) * ((j&1)?1.0f:-1.0f)-1.0f;
				if( i == 1 ){
					pverts_add->vy = 0.0f;
					pverts_sub->vy = 0.0f;
				}else{
					pverts_add->vy = (float)(work->height<<1) * ((i&2)?-1.0f:1.0f)+1.0f;
					pverts_sub->vy = (float)(work->height<<1) * ((i&2)?-1.0f:1.0f)-1.0f;
				}
				pverts_add->vz = 0.0f;//-20.0f;//-40.0f*(float)k-40.0f;
				pverts_sub->vz = 0.0f;//-20.0f;//-40.0f*(float)k-40.0f;
				pverts_add->vw = 1.0f;
				pverts_sub->vw = 1.0f;
				pverts_add++;
				pverts_sub++;
			}
		}
		pverts_add = work->verts[k-N_PRIMS][0];//verts_add;
		pverts_sub = work->verts[k-N_PRIMS][1];//verts_sub;

		DG_PutVector(pverts_add,verts_add,6);
		DG_PutVector(pverts_sub,verts_sub,6);

		work->add[k-N_PRIMS] = InitFlowPrim( tex_code[k], 1, verts_add, k );
		work->sub[k-N_PRIMS] = InitFlowPrim( tex_code[k], 0, verts_sub, k );

		tex = DG_GetTexture(tex_code[k]);
		if(!tex){ printf("ERR!! NO TEX!! cgrt.c\n"); return (-1); }
		
		work->min_v[k] = 0.0f * tex->v_scale + tex->v_offset;
		work->len_v[k] = 1.0f * tex->v_scale + tex->v_offset - work->min_v[k];
		work->now_v[k] = 0.0f;
		work->diff_v[k] = -1.0f/256.0f*rnd() + 1.0f/128.0f;

		work->alpha[k] = rnd() * 2.0f;
		work->add_alpha[k] = -1.0f/64.0f*rnd() + 1.0f/32.0f;

		DG_COPY_VEC(&work->shift[k-N_PRIMS],&DG_ZeroVector);
		work->shift[k-N_PRIMS].vx = (float)work->width*0.5*frnd();
		work->shift[k-N_PRIMS].vy = (float)work->width*0.5*frnd();
		work->add_vx[k-N_PRIMS] = frnd()*2.0f;
		
		//DG_InvisiblePrim2(work->add[k]);
		//DG_InvisiblePrim2(work->sub[k]);
		
	}
}
	return 0;
}

static int GetResources( Work *work )
{
	return 0;
}

void *NewWindowRain( int name, int map )
{
	Work		*work ;
		
	work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
//	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;
		work->name = name;
		work->map = map;
		if ( GetResources_S( work ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}
	
	return (void *)work ;
}

void *NewWindowRain_Demo( int name, int map )
{
	Work		*work ;
		
	work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
//	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;
		work->name = name;
		work->map = map;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}
	
	return (void *)work ;
}
