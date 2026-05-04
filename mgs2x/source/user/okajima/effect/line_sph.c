//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
﻿/*
	line_sph.c
	線上に並ぶ水飛沫  // wet splash stand in line

	2000/02/15 S.Okajima
	$Id: line_sph.c,v 1.5 2002/11/23 12:09:25 Yoshizawa1 Exp $
*/
#endif

#ifdef PSX2
#include <sys/types.h>
#endif
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

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"../../system/libutl/rand.h"
#include	"../etc/ok_util.h"


extern int ok_flush_status;

#define	N_VERTS		(32)

//#define	P_RGB_MAX		(128)
#define	P_RGB_MAX		(96)
//#define	P_RGB_MAX		(64)

#define	SIZE			(100)

#define	DEFAULT_LENGTH			(25000)
#define	DEFAULT_STABLE_LENGTH	(15000)

#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCRPAD_ADDR + 0x2000)

/* ---------------------------------------------------------------- */
typedef	struct	{
	int			n_prim;
	float		width;
	float		height;
	int			alpha;
	FVECTOR		line_data[2];
	FVECTOR		line_width;
	DG_PRIM2	*prim ;
} Unit ;

typedef	struct	{
	GV_ACT_EX		actor ;
	int			name;
	int			map;

	int			limit_length;
	int			fade_length;

	int			unit_num;

	int			invisible_flag;

	int			unit_visible;

	Unit		unit[0];
} Work ;

/* ---------------------------------------------------------------- */
enum {
	REQ_OFF=0,
	REQ_ON,
	REQ_KILL,
	REQ_PARAM,
	REQ_NO
};

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
		  case GV_MESSAGE_KILL:
			GV_DestroyActor( work ) ;
			break;
		  default:
			break;
		}
		msg--;
	}
}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	int		i,j;
	int		itemp;
	int		clock;
	int		alpha=0;
	float	param0;
	float	param1;
	float	param2;
	FVECTOR	*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	Unit	*unit;
	FVECTOR			line_data ;
	FVECTOR			line_width ;

	CheckMesgParam( work );

	if( ok_flush_status!=0 || work->invisible_flag )
   {
		unit = work->unit;
		for(i=0; i<work->unit_num; i++)
      {
			DG_InvisiblePrim2( unit->prim ) ;
			unit++;
		}
		return;
	}
   else
   {
		unit = work->unit;
		for(i=0; i<work->unit_num; i++)
      {
			DG_VisiblePrim2( unit->prim ) ;
			unit++;
		}
	}
   //AR_PARTICLE_HALF
   if(!AS_WillPrimBuffSwitch())
   {
      return;
   }
	clock = 1 - work->unit[0].prim->buffer_clock;

   if (DG_Arm_SkipThisFrame())
   {
      return;
   }

	DG_SetPos( &DG_Chanls->eye_pers );
	unit = work->unit;
	for(i=0; i<work->unit_num; i++)
   {
		DG_SwitchBuffPrim2( unit->prim );
		alpha  = unit->alpha;
		param1  = unit->width*0.5f;
		param2  = unit->height;
		pos     = unit->prim->pos[clock];
		uvrgbwh = unit->prim->uvrgb[clock];
		itemp = unit->n_prim*N_VERTS;
      line_data  = *unit->line_data ;
      line_width = unit->line_width ;
		for ( j = 0 ; j < itemp ; j++ )
      {
			param0 = rnd();

         pos->vx = line_data.vx + line_width.vx * param0 + param1*frnd();
         pos->vy = line_data.vy + line_width.vy * param0 + param2* rnd();
         pos->vz = line_data.vz + line_width.vz * param0 + param1*frnd();
			uvrgbwh->a = alpha;

			pos++;
			uvrgbwh++;
		}

		unit++;
	}
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	Unit	*unit;
	int	i;

	unit = work->unit;
	for(i=0; i<work->unit_num; i++){
		unit->prim = OK_FreePrim2( unit->prim );
		unit++;
	}
}

/* ---------------------------------------------------------------- */
static void InitPacket2( Unit *unit, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* �スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int		j, k ;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	//-------------------------------
	uvrgbwh0 = prim->uvrgb[ 0 ] ;
	uvrgbwh1 = prim->uvrgb[ 1 ] ;
	for ( j = 0 ; j < unit->n_prim ; j++ ){
		for ( k = 0 ; k < N_VERTS ; k++ ){
			uvrgbwh0->u0 = uvrgbwh1->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh0->v0 = uvrgbwh1->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh0->u1 = uvrgbwh1->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh0->v1 = uvrgbwh1->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh0->q0 = uvrgbwh1->q0 = 4096 ;
			uvrgbwh0->q1 = uvrgbwh1->q1 = 4096 ;
			uvrgbwh0->f0 = uvrgbwh1->f0 = 0x0fff ;
			uvrgbwh0->f1 = uvrgbwh1->f1 = 0x0fff ;

			uvrgbwh0->w = uvrgbwh1->w = SIZE ;
			uvrgbwh0->h = uvrgbwh1->h = SIZE ;

			uvrgbwh0->r = uvrgbwh1->r = P_RGB_MAX ;
			uvrgbwh0->g = uvrgbwh1->g = P_RGB_MAX ;
			uvrgbwh0->b = uvrgbwh1->b = P_RGB_MAX ;
			uvrgbwh0->a = uvrgbwh1->a = unit->alpha ;

			uvrgbwh0++;	uvrgbwh1++;
		}
	}

	GV_ZeroMemory( prim->pos[0], sizeof( FVECTOR ) * unit->n_prim * N_VERTS );
	GV_ZeroMemory( prim->pos[1], sizeof( FVECTOR ) * unit->n_prim * N_VERTS );

}

static int GetResources( Work *work, int name, int where, int unit_num )
{
	int			i ;
	FVECTOR		center;
	FVECTOR		fvtemp0;
	FVECTOR		fvtemp1;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	Unit		*unit;
	float		diff_x;
	float		diff_y;
	float		diff_z;

	work->name = name;
	work->map = where;
	work->unit_num = unit_num;

	work->invisible_flag = 0;
	work->unit_visible = 1;

//	tex = DG_GetTexture( 7745660 /*"splash06_msk"*/ );
	tex = DG_GetTexture( 7733153 /*"splash06_alp"*/ );
//	tex = DG_GetTexture( 6684577 /*"splash05_alp"*/ );
	if ( GCL_GetOption( 'd' ) != NULL ){
		unit = work->unit;
		for(i=0; i<work->unit_num; i++){
			unit->n_prim = GCL_GetNextInt();
#ifdef KP_WINDOWS
			unit->n_prim = ((DWORD)unit->n_prim * DG_GetRainEffectLevel()) >> 8 ;	// 調節
#endif
			unit->width  = (float)GCL_GetNextInt();
			unit->height = (float)GCL_GetNextInt();
			unit->alpha  = GCL_GetNextInt() / 2;
			fvtemp0.vx   = (float)GCL_GetNextInt();
			fvtemp0.vy   = (float)GCL_GetNextInt();
			fvtemp0.vz   = (float)GCL_GetNextInt();
			fvtemp1.vx   = (float)GCL_GetNextInt();
			fvtemp1.vy   = (float)GCL_GetNextInt();
			fvtemp1.vz   = (float)GCL_GetNextInt();

			unit->line_data[0].vx = fvtemp0.vx;
			unit->line_data[1].vx = fvtemp1.vx;
			unit->line_width.vx   = diff_x = fvtemp1.vx - fvtemp0.vx;
			unit->line_data[0].vy = fvtemp0.vy;
			unit->line_data[1].vy = fvtemp1.vy;
			unit->line_width.vy   = diff_y = fvtemp1.vy - fvtemp0.vy;
			unit->line_data[0].vz = fvtemp0.vz;
			unit->line_data[1].vz = fvtemp1.vz;
			unit->line_width.vz   = diff_z = fvtemp1.vz - fvtemp0.vz;
			prim = unit->prim = GM_MakePrim2(  DG_PRIM2_SPRT
			                                  |DG_PRIM2_TEX
			                                  |DG_PRIM2_ALPHA
			                                  |DG_PRIM2_BOUNDCHECK
			                                  |DG_PRIM2_FOG,
			                                  unit->n_prim, N_VERTS );
			if(prim==NULL) return -1;
			InitPacket2( unit, prim, tex );
			DG_VisiblePrim2( prim );

#if 1
			diff_x *= 0.5f;
			diff_y *= 0.5f;
			diff_z *= 0.5f;

			center.vx = fvtemp0.vx + diff_x;
			center.vy = fvtemp0.vy + diff_y;
			center.vz = fvtemp0.vz + diff_z;
			center.vw = 1.0f;					//�重要

			//バウンディングマトリクス
         prim->flag |= DG_PRIM_AS_CUSTOMWORLD;
			DG_COPY_MAT( &prim->as_world, &DG_UnitMatrix );
			DG_COPY_VEC( (FVECTOR *)prim->as_world.m[3], &center );

			//マトリクス相対に変更
			_sceVu0SubVector( &unit->line_data[0], &unit->line_data[0], &center ) ;
			_sceVu0SubVector( &unit->line_data[1], &unit->line_data[1], &center ) ;

//printf("diff half::::%f %f %f\n",diff_x,diff_y,diff_z);


			//相対バウンドボックス
			prim->bound_min.vx = (  ( diff_x < 0.0f )? diff_x: -diff_x  ) - unit->width;
			prim->bound_max.vx = -prim->bound_min.vx;
			prim->bound_min.vy = (  ( diff_y < 0.0f )? diff_y: -diff_y  ) - unit->width;
			prim->bound_max.vy = -prim->bound_min.vy;
			prim->bound_min.vz = (  ( diff_z < 0.0f )? diff_z: -diff_z  ) - unit->width;
			prim->bound_max.vz = -prim->bound_min.vz;

#endif

			unit++;
		}
	}else{
		return -1;
	}

	work->fade_length = DEFAULT_STABLE_LENGTH;
	if ( GCL_GetOption( 'f' ) != NULL ){
		work->fade_length = GCL_GetNextInt();
		if( work->fade_length < DEFAULT_STABLE_LENGTH ) work->fade_length = DEFAULT_STABLE_LENGTH;
	}

	work->limit_length = DEFAULT_LENGTH;
	if ( GCL_GetOption( 'l' ) != NULL ){
		work->limit_length = GCL_GetNextInt();
		if( work->limit_length <= work->fade_length ) work->limit_length = (int) (work->fade_length + 1.0f); //avoiding zero divide
	}


	return (0);
}

/* ---------------------------------------------------------------- */

#ifdef KP_WINDOWS
static void DmyAct( Work *work ){ CheckMesgParam(work) ; }
static void DmyDie( Work *work ){}
#endif

void *NewLineSplash( int name, int where )
{
	Work		*work ;
	int			buf_size;
	int			unit_num;

	OPERATOR() ;

	unit_num=0;
	if ( GCL_GetOption( 'n' ) != NULL ) {
		unit_num = GCL_GetNextInt() ;
	}

	buf_size = sizeof( Work ) + sizeof( Unit ) * unit_num;
	work = (Work *)GV_NewEffect( GV_ACTOR_USER, buf_size ) ;
	if ( work != NULL ) {
#ifndef KP_WINDOWS
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, name, where, unit_num ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
#else
		if ( DG_GetRainEffectLevel() ) {
			GV_SetActor( &( work->actor ), Act, Die ) ;
			GV_ActorEX( &work->actor )
			if ( GetResources( work, name, where, unit_num ) < 0 ) {
				GV_DestroyActor( work ) ;
				return NULL ;
			}
		} else {
			GV_SetActor( &( work->actor ), DmyAct, DmyDie ) ;
			GV_ActorEX( &work->actor )
		}
#endif
	}
	return (void *)work ;
}
