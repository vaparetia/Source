//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

   メリケン粉の固まりの落ちる奴 2
	2000/04/26 T.Shibata

	$Id: flour_Fall.c,v 1.1.1.3 2002/11/19 11:48:51 Yoshizawa1 Exp $

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

#include	"def_dma.h"
#include	"utl_dma.h"


#define		N_PRIMS		(12)
#define		N_VERTS		(4)	//4 only
#define		GRAVITY		(-1.0f)

#define		COLOR_R		(0x80)
#define		COLOR_G		(0x80)
#define		COLOR_B		(0x80)
#define		ALPHA		(0x80)

#define		SCALE_R (12)
#define		SCALE_X (4.0f)
#define		SCALE_Z (4.0f)

#define		H_MIZUMASI	10.0f
#define		LIFE_TIME (240)
/* 床の厚さ（適当） */
#define		ATSUSA (40.0f)

/* フラグ */
#define		FLAGS_A		(0x0003)	/* 0:前回調べてない 1:床あり 2:天井あり 3:両方 */
#define		FLAGS_FLR	(0x0001)	/*  */
#define		FLAGS_CEL	(0x0002)	/*  */
#define		FLAGS_FALL	(0x0004)	/* 落ちとりまっせ */
#define		FLAGS_FLRFIND	(0x0008)	/* 床発見 */
#define		FLAGS_ALPHA		(0x0010)

/* check scr add over?*/
#ifdef BP_PS2
#define		MEM_SCR0	((void *)( SCRPAD_ADDR))
#define		MEM_SCR1	((void *)( MEM_SCR0 ) + sizeof(FVECTOR)*N_PRIMS*N_VERTS)
#define		MEM_SCR2	((void *)( MEM_SCR1 ) + sizeof(DG_PRIM2_UVRGB)*N_PRIMS*N_VERTS)
#define		MEM_SCR3	((void *)( MEM_SCR2 ) + sizeof(FVECTOR)*N_PRIMS*N_VERTS)
#else
#define		MEM_SCR0	((void *)( SCRPAD_ADDR))
#define		MEM_SCR1	((void *)( (char *)MEM_SCR0 + sizeof(FVECTOR)*N_PRIMS*N_VERTS) )
#define		MEM_SCR2	((void *)( (char *)MEM_SCR1 + sizeof(DG_PRIM2_UVRGB)*N_PRIMS*N_VERTS) )
#define		MEM_SCR3	((void *)( (char *)MEM_SCR2 + sizeof(FVECTOR)*N_PRIMS*N_VERTS) )
#endif

/* extern */
#define 	FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
extern	void  _BigScrCopy( void *dat, void *src, int size, int num ) ;
extern  void  _BigMemCopy( void *dst, void *src, int size, int num ) ;
extern 	void  BIG_VectoMat(FMATRIX *world, FVECTOR *force, FVECTOR *hit, int mode) ;
extern 	void  AN_Test_Eye2( FVECTOR *mov, int size );

typedef struct
{
	GV_ACT_EX		actor ;
	int			map;
	int			name;
	int			timer;
	FVECTOR		center;
	
	DG_PRIM2	*prim;
	
	FVECTOR		m_pos;
	FVECTOR		m_vec;
	
	FVECTOR		pos[N_PRIMS];
	FVECTOR		vec[N_PRIMS];
	float		pre_flr[N_PRIMS];
	int			flags[N_PRIMS];
	float		alpha[N_PRIMS];


	HZX_GROUP_ID	map_id;
	
} Work ;


static void ReSetVertex( FVECTOR *pos, FVECTOR *before_pos, FVECTOR *p_pos,float hight, Work *work, float *len2 )
{
	FVECTOR 		*new_pos = MEM_SCR0;
	FVECTOR			fvtemp = { p_pos->vx - work->m_pos.vx,0,p_pos->vz - work->m_pos.vz };
	int				time = work->timer;
	float			angle;
	float			scale;
	float			u,v;

	*len2 = _sceVu0InnerProduct( &fvtemp, &fvtemp );
	*len2 = bp_sqrtf(*len2);   //BP_MATH - emulate PS2 sqrtf
//	printf("len = %f\n",*len2);

	if( *len2 < 256.0f ){
		scale = 8.0f;
	}else{
		scale = *len2 / 32.0f;
	}

//	printf("scale = %f\n",scale);
	angle = rnd()*PI;
	u = scale * time * cosf(angle);
	v = scale * time * sinf(angle);

	new_pos[0].vx = p_pos->vx + u;
	new_pos[0].vy = hight + H_MIZUMASI;
	new_pos[0].vz = p_pos->vz + v;

	new_pos[1].vx = p_pos->vx - v;
	new_pos[1].vy = hight + H_MIZUMASI;
	new_pos[1].vz = p_pos->vz + u;

	new_pos[2].vx = p_pos->vx + v;
	new_pos[2].vy = hight + H_MIZUMASI;
	new_pos[2].vz = p_pos->vz - u;

	new_pos[3].vx = p_pos->vx - u;
	new_pos[3].vy = hight + H_MIZUMASI;
	new_pos[3].vz = p_pos->vz - v;

	if( *len2 < 16.0f ){
		*len2 = ALPHA;
	}else if( *len2 > 210.0f ) {
		*len2 = 12.0f;
	}else{
		*len2 = (246.0f - *len2) / 4.0f;
	}
//	printf("alpha = %f\n\n",*len2);
//	printf("vx %f,vy %f,vz %f\n",p_pos->vx,hight,p_pos->vz);

	_BigMemCopy(pos,MEM_SCR0,sizeof(FVECTOR),N_VERTS);
	_BigMemCopy(before_pos,MEM_SCR0,sizeof(FVECTOR),N_VERTS);
}

static void Act(Work *work)
{
	int 				j,k;
	FVECTOR 			*pos,*before_pos;
	FVECTOR		  		fvtemp;
	DG_PRIM2_UVRGB		*uvrgb;

	DG_SwitchBuffPrim2( work->prim ) ;
	pos = work->prim->pos[work->prim->buffer_clock];
	before_pos = work->prim->pos[1-work->prim->buffer_clock];
	uvrgb = work->prim->uvrgb[work->prim->buffer_clock];
	
	for( j = 0; j < N_PRIMS ; j++ ){
		if(work->flags[j] & FLAGS_FALL){
			if(j==0){
				_sceVu0AddVector( &work->m_pos, &work->m_pos, &work->m_vec ) ;
				work->m_vec.vy += GRAVITY ;
			
			}
	
			_sceVu0AddVector( &fvtemp, &work->pos[j], &work->vec[j] ) ;
			work->vec[j].vy += GRAVITY ;
//printf("%f %f %f\n",fvtemp.vx,fvtemp.vy,fvtemp.vz);
//AN_Test_Eye2( &fvtemp, 2 );
			{	/* ハザードチェック */
				unsigned int 		init_flr_flag;
				HZX_FLR				flr[2];
				int					flr_atrs[2];
				float				flr_height[2];

				init_flr_flag = HZX_LevelHazardCheck( work->map_id,
													 &fvtemp,
													 HZX_CHK_F_FLOOR,
													 HZX_FLOOR_NO_PLAYER );
			    //printf(" flr_Flag = %x\n",init_flr_flag);
				if( init_flr_flag & FLAGS_FLR){
					HZX_GetLevelHazard( flr, flr_atrs );
                    /* flr_height[2] 0:flr 1:cel*/
					HZX_GetLevelHeight( flr_height );
					work->pre_flr[j] = flr_height[0];
					work->flags[j] |= FLAGS_FLR;

					DG_COPY_VEC(&work->pos[j], &fvtemp);
				}else{
					if(!(work->flags[j] & FLAGS_A)){
						DG_InvisiblePrim2(work->prim) ;
						GV_DestroyActor(work) ;
						printf("Kill flour_Fall\n");
						return;
					} else {
						work->flags[j] |= FLAGS_FLRFIND;
						work->flags[j] &= ~(FLAGS_FALL);
						ReSetVertex( pos, before_pos, &work->pos[j], work->pre_flr[j], work, &work->alpha[j]);
					}
				}
			}
			
			pos+=N_VERTS;
			before_pos+=N_VERTS;
			uvrgb+=N_VERTS;
					
		} else {
			for( k = 0; k < N_VERTS; k++ ){
				if(uvrgb->a - (int)work->alpha[j])
					uvrgb->a++;
				uvrgb++;
			}
			
			pos+=N_VERTS;
			before_pos+=N_VERTS;
			
		}
	}


	work->timer++;
    /*	
	if(work->timer > LIFE_TIME){
		DG_InvisiblePrim2(work->prim) ;
		GV_DestroyActor( work ) ;
	}
	*/

	//printf("act flour_Down\n");
}


static void Die(Work *work)
{
	if(work->prim) GM_FreePrim2(work->prim) ;

}

static void InitWorkData( Work *work,DG_TEX *tex ,FVECTOR *force)
{
	int 				j,k;
	FVECTOR				*pos,fvtemp;
	DG_PRIM2_UVRGB		*uvrgb;
	short				u0,u1,v0,v1;
	float				speed,angle,f_len;

	DG_COPY_VEC(&fvtemp, force);
	fvtemp.vy = 0.0f;
	/* fvtemp の大きさを１にしてから */
	f_len = _sceVu0InnerProduct( &fvtemp, &fvtemp );
	f_len = bp_sqrtf(f_len);   //BP_MATH - emulate PS2 sqrtf
	fvtemp.vx = -(5.0f * fvtemp.vx)/f_len;
	fvtemp.vz = -(5.0f * fvtemp.vz)/f_len;

	_sceVu0AddVector( &work->center, &work->center, &fvtemp );
	DG_COPY_VEC(&work->m_pos,&work->center);
	DG_COPY_VEC(&work->m_vec,&fvtemp);
	/* 床に広がる奴 */	
	pos = MEM_SCR0;
	uvrgb = MEM_SCR1;
	u0 = FTOI12(0.0f * tex->u_scale + tex->u_offset);
	v0 = FTOI12(0.0f * tex->v_scale + tex->v_offset);
	u1 = FTOI12(1.0f * tex->u_scale + tex->u_offset);
	v1 = FTOI12(1.0f * tex->v_scale + tex->v_offset);
	for( j = 0; j < N_PRIMS ; j++ ){		
		    /* vec init */
		speed = SCALE_R * rnd();
		angle = rnd() * PI * 2.0f;
		work->vec[j].vx = speed * cosf(angle);//0.0f;
		work->vec[j].vy = 0.0f;
		work->vec[j].vz = speed * sinf(angle);//0.0f;
		_sceVu0AddVector( &work->vec[j], &work->vec[j], &fvtemp );
		
		    /* pos init */
		DG_COPY_VEC(&work->pos[j], &work->center) ;
		    /* uvrgb init */
		    /*フラグ（描画キック：0x0fff、頂点キックのみ：0x8fff）*/
		uvrgb[0].f = 0x8fff;
		uvrgb[1].f = 0x8fff;
		uvrgb[2].f = 0x0fff;
		uvrgb[3].f = 0x0fff;
		
		uvrgb[0].u = u0;
		uvrgb[0].v = v0;
		uvrgb[1].u = u1;
		uvrgb[1].v = v0;
		uvrgb[2].u = u0;
		uvrgb[2].v = v1;	
		uvrgb[3].u = u1;
		uvrgb[3].v = v1;
		work->flags[j] = FLAGS_FALL;
		for( k = 0; k < N_VERTS ; k++ ){
			uvrgb->q = 4096;
			uvrgb->r = COLOR_R;
			uvrgb->g = COLOR_G;
			uvrgb->b = COLOR_B;
			uvrgb->a = 0;//ALPHA;
			DG_COPY_VEC(pos, &work->center) ;
			pos++;
			uvrgb++;
		}
		
	}
	_BigMemCopy(work->prim->pos[0],MEM_SCR0,sizeof(FVECTOR),N_PRIMS * N_VERTS);
	_BigMemCopy(work->prim->uvrgb[0],MEM_SCR1,sizeof(DG_PRIM2_UVRGB),N_PRIMS * N_VERTS);
	_BigMemCopy(work->prim->pos[1],MEM_SCR0,sizeof(FVECTOR),N_PRIMS * N_VERTS);
	_BigMemCopy(work->prim->uvrgb[1],MEM_SCR1,sizeof(DG_PRIM2_UVRGB),N_PRIMS * N_VERTS);

	
}


static int GetResources( Work *work, int map, int name, FVECTOR *hit ,FVECTOR *force )
{
	DG_PRIM2	*prim;
	DG_TEX		*tex;


//	tex = DG_GetTexture(GV_StrCode("powder01_alp"));
	tex = DG_GetTexture(8949918);
	if(!tex){
		printf(" not texture int flour_Down.c \n");
		return(-1);
	}

	/* 床に広がる奴 */
	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SHADE,
									  N_PRIMS,N_VERTS );
	if(!prim){
		printf("null prim\n");
		return (-1);
	}
	/* まだチャンネル１～３は無いので取りあえず フラグ立て */
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	/* DataInit */
	work->timer = 0;
	work->map = map;
	GM_GroupPrim2( prim, map ) ;
	work->name = name;
	work->center.vx = hit->vx;
	work->center.vy = hit->vy;
	work->center.vz = hit->vz;
	work->map_id = GM_GetHzxGroupID( map );
	InitWorkData( work, tex ,force);

	return (0);
}

/*
    map:
    name:
	hit:	当たった場所
*/

void *NewFlour_Fall( int map, int name, FVECTOR *hit ,FVECTOR *force)
{
	Work *work = NULL;

	//printf("unko");
	work = (Work*)GV_NewActor(GV_ACTOR_USER,sizeof(Work));
	if(work){
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(GetResources(work,map,name,hit,force) < 0){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
