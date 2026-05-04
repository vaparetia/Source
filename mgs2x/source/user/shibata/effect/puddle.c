//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    puddle.c
	水溜り管理人
	2000/10/20 T.Shibata
	
	$Id: puddle.c,v 1.1.1.3 2002/11/19 11:48:38 Yoshizawa1 Exp $

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



/*

chara 水溜り管理[NewPuddle] $s:name \
	-n_obj $i:波紋発生キャラ数 \
	-obj   $s:キャラネーム ... 

mesg 水溜り管理 $s:名前 入る[0] $s:キャラ名
mesg 水溜り管理 $s:名前 出る[1] $s:キャラ名

例）
	chara 水溜り管理 水溜り管理人 \
		-n_obj	4 \
		-obj	d:PLAYER \
				警備兵:01 \
				攻撃兵:01 \
				攻撃兵:02 

	trap puddle ？ \
	-mask ？ \
	-exec {
		if($3 == 入る){
			mesg 水溜り管理 水溜り管理人 入る $2
		}else{
			mesg 水溜り管理 水溜り管理人 出る $2
		}
	}
*/

#define MAX_PUDDLE_OBJS		(32)

#define PDLOBJ_TYPE_HUMAN	(0x8000)
#define PDLOBJ_TYPE_ACTIVE	(0x4000)

#define PDLOBJ_TYPE_RFOOT	(0x0010)
#define PDLOBJ_TYPE_LFOOT	(0x0020)
#define PDLOBJ_TYPE_RHAND	(0x0040)
#define PDLOBJ_TYPE_LHAND	(0x0080)

#define N_POLYS		(32)
#define	N_ALLVERTS	(N_POLYS<<2)

#define N_VERTS		(64)
#define N_PRIMS		(N_ALLVERTS/64)

#ifdef BP_PS2
#define			MEM_SCR_POS0	((void*)(SCRPAD_ADDR))
#define			MEM_SCR_POS1	((void*)(MEM_SCR_POS0 + sizeof(FVECTOR)*N_ALLVERTS))
#define			MEM_SCR_UV0		((void*)(MEM_SCR_POS1 + sizeof(FVECTOR)*N_ALLVERTS))
#define			MEM_SCR_UV1		((void*)(MEM_SCR_UV0 + sizeof(DG_PRIM2_UVRGB)*N_ALLVERTS))
#else
#define			MEM_SCR_POS0	((void*)(SCRPAD_ADDR))
#define			MEM_SCR_POS1	((void*)((char *)MEM_SCR_POS0 + sizeof(FVECTOR)*N_ALLVERTS))
#define			MEM_SCR_UV0		((void*)((char *)MEM_SCR_POS1 + sizeof(FVECTOR)*N_ALLVERTS))
#define			MEM_SCR_UV1		((void*)((char *)MEM_SCR_UV0 + sizeof(DG_PRIM2_UVRGB)*N_ALLVERTS))
#endif


#define			ADD_DIFF		(5.0f)
#define			INIT_ALPHA		(128)
#define			SUB_ALPHA		(4)

typedef struct {
	int		name;
	int		type;
	int		*map;
	u_char	*grounded;

	float	*levels;
	FVECTOR	*pos;
	FVECTOR	*step;
	DG_OBJS	*chara_objs;
} PUDDLE_OBJCT;

typedef struct {
	GV_ACT_EX		actor ;
	int				name;
	int				map;
	int				n_objcts;
	int				n_active;
	
	DG_PRIM2		*prim[2];
	int				n_used;
	int				pad;
	
	FVECTOR			data[N_POLYS];
	float			diff_x[N_POLYS];
	float			diff_y[N_POLYS];

	PUDDLE_OBJCT	pdl_objs[MAX_PUDDLE_OBJS];
} Work;

enum {
	MSG_PDL_IN = 0,		//進入
	MSG_PDL_OUT,		//撤退
	MSG_PDL_ADD,		//オブジェクト追加
	MSG_PDL_DEL,		//オブジェクト削除
};

static CONTROL* SearchControl( Work *work, int name )
{
	int		i ;
	CONTROL	*control, **w_list ;

	w_list = GM_WhereList ;
	for ( i = GM_N_WhereList ; i > 0 ; w_list++, i-- ){
		control = *w_list ;
		if ( control->name != name ) continue ;
		if ( ( control->map & work->map ) == 0 ) continue ;
		return ( control ) ;
	}
	printf("puddle.c: search faild control !!\n");
	return ( NULL );
}

static PUDDLE_OBJCT* SearchPdlObj( Work *work, int name )
{
	PUDDLE_OBJCT	*pdl_obj = work->pdl_objs;
	int				i;
	
	for( i = 0; i < MAX_PUDDLE_OBJS; i++ ){
		if(name == pdl_obj->name) {
			//printf("aaaaaaaaaaaaaaaaaaaaaaaa[%d]\n",i);
			return pdl_obj;
		}
		pdl_obj++;
	}

	return NULL;
}

static int CheckMesgParam( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num,name;
	PUDDLE_OBJCT	*pdl_obj;

	mes_num=GV_ReceiveMessage( work->name, &msg );

	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		case MSG_PDL_IN:
			name = msg->message[1];
			pdl_obj = SearchPdlObj( work, name );
			if(pdl_obj && !(pdl_obj->type & PDLOBJ_TYPE_ACTIVE)){
				DG_OBJ *obj = pdl_obj->chara_objs->objs;

				pdl_obj->type |= PDLOBJ_TYPE_ACTIVE;

				if( obj[16].world.m[3][1] > pdl_obj->levels[0]+50.0f ){
					pdl_obj->type |= PDLOBJ_TYPE_RFOOT;
				}
				if( obj[20].world.m[3][1] > pdl_obj->levels[0]+50.0f ){
					pdl_obj->type |= PDLOBJ_TYPE_LFOOT;
				}

				if( obj[6-1].world.m[3][1] > pdl_obj->levels[0]+20.0f ){
					pdl_obj->type |= PDLOBJ_TYPE_RHAND;
				}
				if( obj[10-1].world.m[3][1] > pdl_obj->levels[0]+20.0f ){
					pdl_obj->type |= PDLOBJ_TYPE_LHAND;
				}

				printf("padule start[%x]\n",pdl_obj->name);
				work->n_active++;
			}
			break;
		case MSG_PDL_OUT:
			name = msg->message[1];
			pdl_obj = SearchPdlObj( work, name );
			if(pdl_obj && (pdl_obj->type & PDLOBJ_TYPE_ACTIVE)){
				pdl_obj->type &= ~(PDLOBJ_TYPE_ACTIVE);
				pdl_obj->type &= ~(PDLOBJ_TYPE_RFOOT|PDLOBJ_TYPE_RFOOT);
				printf("padule end[%x]\n",pdl_obj->name);
				work->n_active--;
			}
			break;
		case MSG_PDL_ADD:

			break;
		case MSG_PDL_DEL:

			break;
		default:
			printf("puddle.c Msg Err!![%d]\n",num);
			break;
		}
		msg--;
	}

	return (work->n_active+work->n_used);
}

static int HamonPrim_Act(Work *work)
{
	int			i,n_used = 0,clock = work->prim[0]->buffer_clock;
	FVECTOR		*data = work->data;
	FVECTOR		center_add,center_sub;
	float		size;

	FVECTOR			*pos_add = MEM_SCR_POS0;
	FVECTOR			*pos_sub = MEM_SCR_POS1;
	DG_PRIM2_UVRGB	*uvrgba = MEM_SCR_UV0;

//	TS_Mem_Scr( MEM_SCR_POS0, work->prim[0]->pos[1-clock], sizeof(FVECTOR), N_ALLVERTS );
//	TS_Mem_Scr( MEM_SCR_POS1, work->prim[1]->pos[1-clock], sizeof(FVECTOR), N_ALLVERTS );
	TS_Mem_Scr( MEM_SCR_UV0, work->prim[0]->uvrgb[1-clock], sizeof(DG_PRIM2_UVRGB), N_ALLVERTS );

//	TS_Mem_Scr( MEM_SCR_UV0, work->prim[1]->uvrgb[1-clock], sizeof(DG_PRIM2_UVRGB), N_ALLVERTS );


	for( i = 0; i < N_POLYS; i++ ){
	//	printf("data[%d][%f]\n",i,data->vw);
		if(data->vw < -128.0f){
			//動いてません
			uvrgba[0].a = 0;
			uvrgba[1].a = 0;
			uvrgba[2].a = 0;
			uvrgba[3].a = 0;
			DG_COPY_VEC(&pos_add[0],&DG_ZeroVector);
			DG_COPY_VEC(&pos_add[1],&DG_ZeroVector);
			DG_COPY_VEC(&pos_add[2],&DG_ZeroVector);
			DG_COPY_VEC(&pos_add[3],&DG_ZeroVector);
			DG_COPY_VEC(&pos_sub[0],&DG_ZeroVector);
			DG_COPY_VEC(&pos_sub[1],&DG_ZeroVector);
			DG_COPY_VEC(&pos_sub[2],&DG_ZeroVector);
			DG_COPY_VEC(&pos_sub[3],&DG_ZeroVector);
			data->vw = -1024.0f;
		}else if(data->vw < 0.0f){
			uvrgba[0].a = 0;
			uvrgba[1].a = 0;
			uvrgba[2].a = 0;
			uvrgba[3].a = 0;
			DG_COPY_VEC(&pos_add[0],&DG_ZeroVector);
			DG_COPY_VEC(&pos_add[1],&DG_ZeroVector);
			DG_COPY_VEC(&pos_add[2],&DG_ZeroVector);
			DG_COPY_VEC(&pos_add[3],&DG_ZeroVector);
			DG_COPY_VEC(&pos_sub[0],&DG_ZeroVector);
			DG_COPY_VEC(&pos_sub[1],&DG_ZeroVector);
			DG_COPY_VEC(&pos_sub[2],&DG_ZeroVector);
			DG_COPY_VEC(&pos_sub[3],&DG_ZeroVector);
			data->vw += 1.0f;
		}else{
			//初期化
			if(data->vw == 0.0f){
				//printf("data[%d][%f]\n",i,data->vw);
				work->diff_x[i] = rnd();
				work->diff_y[i] = 1.0f - work->diff_x[i];
				data->vw = 50.0f;
				uvrgba[0].a = INIT_ALPHA;
				uvrgba[1].a = INIT_ALPHA;
				uvrgba[2].a = INIT_ALPHA;
				uvrgba[3].a = INIT_ALPHA;
				DG_COPY_VEC(&pos_add[0],&DG_ZeroVector);
				DG_COPY_VEC(&pos_add[1],&DG_ZeroVector);
				DG_COPY_VEC(&pos_add[2],&DG_ZeroVector);
				DG_COPY_VEC(&pos_add[3],&DG_ZeroVector);
				DG_COPY_VEC(&pos_sub[0],&DG_ZeroVector);
				DG_COPY_VEC(&pos_sub[1],&DG_ZeroVector);
				DG_COPY_VEC(&pos_sub[2],&DG_ZeroVector);
				DG_COPY_VEC(&pos_sub[3],&DG_ZeroVector);
			}else{
				
				if(uvrgba[0].a <= SUB_ALPHA){
					//終了処理
					data->vw = -256.0f;
					DG_COPY_VEC(&center_add,&DG_ZeroVector);
					DG_COPY_VEC(&center_sub,&DG_ZeroVector);
					size = 0.0f;
					
					uvrgba[0].a = 0;
					uvrgba[1].a = 0;
					uvrgba[2].a = 0;
					uvrgba[3].a = 0;
					
				}else{
					uvrgba[0].a -= SUB_ALPHA;
					uvrgba[1].a -= SUB_ALPHA;
					uvrgba[2].a -= SUB_ALPHA;
					uvrgba[3].a -= SUB_ALPHA;
					//通常
					DG_COPY_VEC(&center_add,data);
					DG_COPY_VEC(&center_sub,data);
					center_add.vw = 1.0f;
					center_sub.vw = 1.0f;
					data->vw += rnd()*4.0f+2.0f;
					size = data->vw;

					work->diff_x[i] *= 1.05f;
					work->diff_y[i] *= 1.05f;
					center_add.vx += work->diff_x[i];
					center_add.vy += work->diff_y[i];
					center_sub.vx -= work->diff_x[i];
					center_sub.vy -= work->diff_y[i];
					//AN_Test_Eye2( &center_add,2);
				}

				pos_add[0].vx = center_add.vx - size;
				pos_add[0].vz = center_add.vz - size;
				pos_add[1].vx = center_add.vx + size;
				pos_add[1].vz = center_add.vz - size;
				pos_add[2].vx = center_add.vx - size;
				pos_add[2].vz = center_add.vz + size;
				pos_add[3].vx = center_add.vx + size;
				pos_add[3].vz = center_add.vz + size;

				pos_add[0].vy = center_add.vy;
				pos_add[0].vw = 1.0f;
				pos_add[1].vy = center_add.vy;
				pos_add[1].vw = 1.0f;
				pos_add[2].vy = center_add.vy;
				pos_add[2].vw = 1.0f;
				pos_add[3].vy = center_add.vy;
				pos_add[3].vw = 1.0f;


				pos_sub[0].vx = center_sub.vx - size;
				pos_sub[0].vz = center_sub.vz - size;
				pos_sub[1].vx = center_sub.vx + size;
				pos_sub[1].vz = center_sub.vz - size;
				pos_sub[2].vx = center_sub.vx - size;
				pos_sub[2].vz = center_sub.vz + size;
				pos_sub[3].vx = center_sub.vx + size;
				pos_sub[3].vz = center_sub.vz + size;

				pos_sub[0].vy = center_sub.vy;
				pos_sub[0].vw = 1.0f;
				pos_sub[1].vy = center_sub.vy;
				pos_sub[1].vw = 1.0f;
				pos_sub[2].vy = center_sub.vy;
				pos_sub[2].vw = 1.0f;
				pos_sub[3].vy = center_sub.vy;
				pos_sub[3].vw = 1.0f;

			}
			
			n_used++;
		}

		pos_add += 4;
		pos_sub += 4;
		uvrgba += 4;
		data++;
	}


	TS_Scr_Mem( work->prim[0]->pos[clock], MEM_SCR_POS0, sizeof(FVECTOR), N_ALLVERTS );
	TS_Scr_Mem( work->prim[1]->pos[clock], MEM_SCR_POS1, sizeof(FVECTOR), N_ALLVERTS );
	TS_Scr_Mem( work->prim[0]->uvrgb[clock], MEM_SCR_UV0, sizeof(DG_PRIM2_UVRGB), N_ALLVERTS );
	TS_Scr_Mem( work->prim[1]->uvrgb[clock], MEM_SCR_UV0, sizeof(DG_PRIM2_UVRGB), N_ALLVERTS );
	
	return n_used;
}

static FVECTOR *GetVoidData( Work *work ) 
{
	FVECTOR	*data = work->data;
	int i;
	for( i = 0; i < N_POLYS; i++ ){
		if(data->vw <= -1024.0f){
			//printf("init[%d]\n",i);
			return data;
		}
		data++;
	}
	printf("puddle prim full\n");
	return NULL;
}

static int SetHamonPrim( Work *work, FVECTOR *pos, PUDDLE_OBJCT *pdl_obj, float data_vw )
{
	FVECTOR	*data;

	data = GetVoidData( work );
	if(data){
		data->vx = pos->vx;
		data->vz = pos->vz;
		data->vy = pdl_obj->levels[0]+10.0f;
		//AN_Test_Eye2(data,2);
		data->vw = data_vw;
		work->n_used++;
	}
	return 0;
}

static void Act(Work *work)
{
	int				i = 0;
	PUDDLE_OBJCT	*pdl_obj = work->pdl_objs;
//	FVECTOR			*data = NULL;

	if(!CheckMesgParam( work )){
//		GV_WaitMessage( work, work->name );
//		return;
	}
	
	DG_SwitchBuffPrim2( work->prim[0] );
	DG_SwitchBuffPrim2( work->prim[1] );

	if(work->n_used) work->n_used = HamonPrim_Act(work);
	
//	while( i < work->n_active ){
	for( i = 0; i < MAX_PUDDLE_OBJS; i++ ){
//		printf("[%d]\n",i);
		if( pdl_obj->name && 
			pdl_obj->type & PDLOBJ_TYPE_ACTIVE 
			&& (*pdl_obj->grounded)&1){

			if(pdl_obj->type & PDLOBJ_TYPE_HUMAN ){
				DG_OBJ *obj = pdl_obj->chara_objs->objs;
				FVECTOR	fvtemp0,fvtemp1;

				DG_COPY_VEC(&fvtemp0,obj[16].world.m[3]);
				DG_COPY_VEC(&fvtemp1,obj[20].world.m[3]);
					
				//右足
				if(fvtemp0.vy > pdl_obj->levels[0]+50.0f){
					if( !(pdl_obj->type & PDLOBJ_TYPE_RFOOT) ){
						pdl_obj->type |= PDLOBJ_TYPE_RFOOT;
						SetHamonPrim( work, &fvtemp0, pdl_obj,  0.0f );
						SetHamonPrim( work, &fvtemp0, pdl_obj, -12.0f );
					}
				}else{
					if( pdl_obj->type & PDLOBJ_TYPE_RFOOT ){
						pdl_obj->type &= ~(PDLOBJ_TYPE_RFOOT);
						SetHamonPrim( work, &fvtemp0, pdl_obj,  0.0f );
						SetHamonPrim( work, &fvtemp0, pdl_obj, -12.0f );
					}
				}
				
				
				//左足
				if(fvtemp1.vy > pdl_obj->levels[0]+50.0f){
					if( !(pdl_obj->type & PDLOBJ_TYPE_LFOOT) ){
						pdl_obj->type |= PDLOBJ_TYPE_LFOOT;
						SetHamonPrim( work, &fvtemp1, pdl_obj,  0.0f );
						SetHamonPrim( work, &fvtemp1, pdl_obj, -12.0f );
					}
				}else{
					if( pdl_obj->type & PDLOBJ_TYPE_LFOOT ){
						pdl_obj->type &= ~(PDLOBJ_TYPE_LFOOT);
						SetHamonPrim( work, &fvtemp1, pdl_obj,  0.0f );
						SetHamonPrim( work, &fvtemp1, pdl_obj, -12.0f );
					}
				}
				
				if(pdl_obj->pos->vy < pdl_obj->levels[0]+500.0f){
					//倒れ
					if((irnd()>>16)%24==0){
						DG_COPY_VEC(&fvtemp0,obj[(irnd()>>16)%21].world.m[3]);
						SetHamonPrim( work, &fvtemp0, pdl_obj,  0.0f );
						SetHamonPrim( work, &fvtemp0, pdl_obj, -12.0f );
					}
					
					DG_COPY_VEC(&fvtemp0,obj[6-1].world.m[3]);
					DG_COPY_VEC(&fvtemp1,obj[10-1].world.m[3]);
					//右手
					if(fvtemp0.vy > pdl_obj->levels[0]+20.0f){
						if( !(pdl_obj->type & PDLOBJ_TYPE_RHAND) ){
							pdl_obj->type |= PDLOBJ_TYPE_RHAND;
							SetHamonPrim( work, &fvtemp0, pdl_obj,  0.0f );
							SetHamonPrim( work, &fvtemp0, pdl_obj, -12.0f );
						}
					}else{
						if( pdl_obj->type & PDLOBJ_TYPE_RHAND ){
							pdl_obj->type &= ~(PDLOBJ_TYPE_RHAND);
							SetHamonPrim( work, &fvtemp0, pdl_obj,  0.0f );
							SetHamonPrim( work, &fvtemp0, pdl_obj, -12.0f );
						}
					}
				
				
					//左手
					if(fvtemp1.vy > pdl_obj->levels[0]+20.0f){
						if( !(pdl_obj->type & PDLOBJ_TYPE_LHAND) ){
							pdl_obj->type |= PDLOBJ_TYPE_LHAND;
							SetHamonPrim( work, &fvtemp1, pdl_obj,  0.0f );
							SetHamonPrim( work, &fvtemp1, pdl_obj, -12.0f );
						}
					}else{
						if( pdl_obj->type & PDLOBJ_TYPE_LHAND ){
							pdl_obj->type &= ~(PDLOBJ_TYPE_LHAND);
							SetHamonPrim( work, &fvtemp1, pdl_obj,  0.0f );
							SetHamonPrim( work, &fvtemp1, pdl_obj, -12.0f );
						}
					}
					
				}

			}
		}
		pdl_obj++;
	}

	
}

static void Die(Work *work)
{
	if(work->prim[0])GM_FreePrim2(work->prim[0]);
	if(work->prim[1])GM_FreePrim2(work->prim[1]);
}

static void GetOptions( Work *work )
{
	int				n_objcts,i = 0;
	PUDDLE_OBJCT	*pdl_obj = work->pdl_objs;
	CONTROL			*control;

	n_objcts = GCL_GetOptionValue( 'n', 0 );
	
	if( n_objcts && GCL_GetOption( 'o' ) != NULL){
		
		while( i < MAX_PUDDLE_OBJS && i < n_objcts ){

			pdl_obj->name = GCL_GetNextInt();
			control = SearchControl( work, pdl_obj->name );
			if(control){

				pdl_obj->chara_objs = control->object->objs;
				pdl_obj->map = &control->map;
				pdl_obj->grounded = &control->grounded;
				pdl_obj->levels = control->levels;
				pdl_obj->pos = &control->mov;
				pdl_obj->step = &control->step;

				if(pdl_obj->chara_objs->def->n_models == 21)
					pdl_obj->type = PDLOBJ_TYPE_HUMAN;
				else
					pdl_obj->type = 0;

				work->n_objcts++;
				pdl_obj++;
			}
			i++;
		}
	}

}

static int GetResources( Work *work )
{
	DG_TEX			*tex;
	DG_PRIM2		*prim;
	FVECTOR			*pos = MEM_SCR_POS0;
	DG_PRIM2_UVRGB	*uvrgba = MEM_SCR_UV0;
	int				u[2],v[2];
	int				i,j;

	GetOptions( work );

	//tex = DG_GetTexture(7560498);	//6
	//tex = DG_GetTexture(7429426);	//4
	//tex = DG_GetTexture(7494962);	//5

	//tex = DG_GetTexture(7691570); //8
	tex = DG_GetTexture(7744599); //9

	if(!tex){ printf("ERR!! NO TEX!! cgrt.c\n"); return (-1); }
	
	u[0] = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	v[0] = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	u[1] = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	v[1] = FTOI12( 1.0f * tex->v_scale + tex->v_offset );

	for( i = 0; i < 2; i++ ){
		prim = work->prim[i] = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SHADE,
											 N_PRIMS,
											 N_VERTS );
		if(!prim){ printf("ERR!! MAKE PRIM2!! win_rain.c\n"); return (-1); }

		DG_ConfigPrim2Tex( prim, tex );
		if(!i){
			DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x80 ) );
		}else{
			DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x80 ) );
		}
		prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );
		prim->raise = i*500 + 500;
	}

	//DG_InvisiblePrim2(work->prim[0]);
	//DG_InvisiblePrim2(work->prim[1]);

	for( i = 0; i < (N_ALLVERTS>>2); i++ ){
		work->data[i].vw = -1024.0f;
		for( j = 0; j < 4; j++ ){
			DG_COPY_VEC(pos,&DG_ZeroVector);
			uvrgba->r = 0x80;
			uvrgba->g = 0x80;
			uvrgba->b = 0x80;
			uvrgba->a = 0;
			
			uvrgba->f = (j<2)?VERT_KICK_CODE:DRAW_KICK_CODE;
			uvrgba->q = 4096;
			uvrgba->u = u[j&1];
			uvrgba->v = v[j>>1];
			uvrgba++;
			pos++;
		}
	}

	TS_Scr_Mem( work->prim[0]->pos[0], MEM_SCR_POS0, sizeof(FVECTOR), N_ALLVERTS );
	TS_Scr_Mem( work->prim[0]->pos[1], MEM_SCR_POS0, sizeof(FVECTOR), N_ALLVERTS );
	TS_Scr_Mem( work->prim[1]->pos[0], MEM_SCR_POS0, sizeof(FVECTOR), N_ALLVERTS );
	TS_Scr_Mem( work->prim[1]->pos[1], MEM_SCR_POS0, sizeof(FVECTOR), N_ALLVERTS );

	TS_Scr_Mem( work->prim[0]->uvrgb[0], MEM_SCR_UV0, sizeof(DG_PRIM2_UVRGB), N_ALLVERTS );
	TS_Scr_Mem( work->prim[0]->uvrgb[1], MEM_SCR_UV0, sizeof(DG_PRIM2_UVRGB), N_ALLVERTS );
	TS_Scr_Mem( work->prim[1]->uvrgb[0], MEM_SCR_UV0, sizeof(DG_PRIM2_UVRGB), N_ALLVERTS );
	TS_Scr_Mem( work->prim[1]->uvrgb[1], MEM_SCR_UV0, sizeof(DG_PRIM2_UVRGB), N_ALLVERTS );

	return 0;
}

void *NewPuddle( int name, int map )
{
	Work *work = NULL;

	work = (Work*)GV_NewEffect(GV_ACTOR_EFFECT,sizeof(Work));
	if(work){
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor );
		
		work->name = name;
		work->map = map;
		if(GetResources( work ) < 0){
			GV_DestroyActor(work) ;
			return NULL ;
		}
		//printf("new sprite\n");
	}

	return (void *)work ;
}
