//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	foot_print.c
	足跡

	2000/06/21 S.Okajima
	$Id: foot_print.c,v 1.1.1.3 2002/11/19 11:47:06 Yoshizawa1 Exp $
*/

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
#include <math.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"gameheader.h"
#include	"libmt.h"
#include	"utl_dma.h"
#include	"../etc/ok_util.h"

extern float GM_WaterLevel;

/*----------------------------------------------------------------*/
enum {
	PAT_NOTHING = -1,
	PAT_WATER,
	PAT_SNOW,
	PAT_FLOUR,
};

/*----------------------------------------------------------------*/
#define	TRAP_OUTSIDE	(0x839ecc)		/* "puddle" */
#define	TRAP_INSIDE		(0x3b4657)		/* "snow" */
#define	TRAP_INSIDEOUT	(0x97cb18)		/* "flour" */

#define	N_VERTS		(64)
#define	N_POLYS		(N_VERTS/4)
#define	N_PRIMS		(8)
#define	TOTAL_POLYS		(N_PRIMS*N_POLYS)


#define	ALPHA_DIV		(4)
#define	ALPHA_SUB		(1.0f / (float)ALPHA_DIV)

//#define	BASE_RGB		(255)
#define	BASE_RGB		(0)
#define	MIN_ALPHA		(32)
#define	MAX_ALPHA		(255-MIN_ALPHA)
#define	DECAY_START		(256)

#define	SCR_POS			(SCRPAD_ADDR)
#define	SCR_VEC			(SCRPAD_ADDR + 0x1000)
#define	SCR_UVS			(SCRPAD_ADDR + 0x2000)

#define	N_FOOT			(2)
#define	FOOT_SIZE_SIDE		(70.0f)
#define	FOOT_SIZE_HEIGHT	(150.0f)

#define	NARR_LIMIT	(300.0f)
#define	STEP_LIMIT	(150.0f)

static ALIGN16_PRE int ALIGN16_POST foot_joint[N_FOOT] = { 16, 20 };
static ALIGN16_PRE FVECTOR ALIGN16_POST size_left[4] = {
	{   FOOT_SIZE_SIDE,   FOOT_SIZE_HEIGHT, 0.0f, 1.0f },
	{ - FOOT_SIZE_SIDE,   FOOT_SIZE_HEIGHT, 0.0f, 1.0f },
	{   FOOT_SIZE_SIDE, - FOOT_SIZE_HEIGHT, 0.0f, 1.0f },
	{ - FOOT_SIZE_SIDE, - FOOT_SIZE_HEIGHT, 0.0f, 1.0f }
};
static ALIGN16_PRE FVECTOR ALIGN16_POST size_right[4] = {
	{ - FOOT_SIZE_SIDE,   FOOT_SIZE_HEIGHT, 0.0f, 1.0f },
	{   FOOT_SIZE_SIDE,   FOOT_SIZE_HEIGHT, 0.0f, 1.0f },
	{ - FOOT_SIZE_SIDE, - FOOT_SIZE_HEIGHT, 0.0f, 1.0f },
	{   FOOT_SIZE_SIDE, - FOOT_SIZE_HEIGHT, 0.0f, 1.0f }
};




typedef	struct	{
	GV_ACT_EX		actor ;
	int			map ;

	OBJECT		*body;
	CONTROL		*control;
	int			*flag;

	int			top_num;
	int			activate_flag;

	int			time;
	int			decay_time;

	DG_PRIM2	*prim ;

	ENEFINDLIST	asiato_efl ;
	ENEFIND		enefind[ TOTAL_POLYS ] ;
	int			list_flag;
	int			list_parts_flag[ TOTAL_POLYS ];

	int			delay_times;
	int			current_trap;
	int			before_trap;
	int			current_rest_time;

} Work ;

/*----------------------------------------------------------------*/
static void EneListFree( Work *work )
{
	if( work->list_flag ){
		GM_FreeEneFindList( &work->asiato_efl );
		work->list_flag = 0;
	}
}

/*----------------------------------------------------------------*/
static void EneListSet( Work *work )
{
	if( !work->list_flag ){
		GM_InitEneFindList( &work->asiato_efl, EF_LIST_TYPE_LINK|EF_LIST_TYPE_FOOT ) ;
		GM_PutEneFindList( &work->asiato_efl ) ;
		work->list_flag = 1;
	}
}

/*----------------------------------------------------------------*/
static void EneListPartFree( Work *work )
{
	int	*flag;

	flag = &work->list_parts_flag[ work->top_num ];
	if( (*flag) && (work->list_flag) ){
		GM_FreeEneFindFromList( &work->asiato_efl, &work->enefind[ work->top_num ] ) ;
		*flag = 0;
	}
}

/*----------------------------------------------------------------*/
static void EneListPartSet( Work *work, FVECTOR *fvtemp, int addr )
{
	int	*flag;

	flag = &work->list_parts_flag[ work->top_num ];
	if( !(*flag) && (work->list_flag) ){
		GM_SetEneFind( &work->enefind[ work->top_num ], fvtemp, addr, EF_TYPE_FOOTSTAMP ) ;
		GM_PutEneFindToList( &work->asiato_efl, &work->enefind[ work->top_num ] ) ;
		*flag = 1;
//AN_Test_Eye2( fvtemp, 2 );
	}
}


/*----------------------------------------------------------------*/
/* floor についての rotを得る */
static	void	CalcLocalRot_Floor( HZX_FLR *floor, SVECTOR *rot )
{
	FVECTOR	to;

	/* 法線収得 */
	to.vx=floor->p1.h;
	to.vy=floor->p3.h;
	to.vz=floor->p2.h;

	OK_DirVecXY( &DG_ZeroVector, &to, rot );
}

/* floor に プリミティブを張る */
static void SetPrims( Work *work, FVECTOR *prim_pos, FVECTOR *point_pos, SVECTOR *point_rot, short rot_y, int left )
{
	SVECTOR	temp_rot;

	temp_rot.vx=0;
	temp_rot.vy=0;
	temp_rot.vz=rot_y;

	DG_SetPos2( &DG_ZeroVector, &temp_rot );
	if( left == 1 ){
		DG_PutVector( size_left, prim_pos, 4 );
	}else{
		DG_PutVector( size_right, prim_pos, 4 );
	}
	DG_SetPos2( point_pos, point_rot );
	DG_PutVector( prim_pos, prim_pos, 4 );

	prim_pos[0].vw = (float)( work->time );
	if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/04/02 */
	{
	prim_pos[1].vw = (float)( (128-MIN_ALPHA) * work->current_rest_time / work->time + MIN_ALPHA );
	}
	else
	{
	prim_pos[1].vw = (float)( MAX_ALPHA * work->current_rest_time / work->time + MIN_ALPHA );
	}

//printf("%f %f\n",prim_pos[0].vw,prim_pos[1].vw);

/*
printf("0:%f %f %f\n",prim_pos[0].vx,prim_pos[0].vy,prim_pos[0].vz);
printf("1:%f %f %f\n",prim_pos[1].vx,prim_pos[1].vy,prim_pos[1].vz);
printf("2:%f %f %f\n",prim_pos[2].vx,prim_pos[2].vy,prim_pos[2].vz);
printf("3:%f %f %f\n",prim_pos[3].vx,prim_pos[3].vy,prim_pos[3].vz);
*/

}

/*----------------------------------------------------------------*/
static int FadePrim( Work *work, int clock )
{
	int		i;
	int		count;
	int		alpha;
	FVECTOR	*fvtemp0;
	FVECTOR	*fvtemp1;
	FVECTOR	*fvtemp2;
	DG_PRIM2_UVRGB	*uvrgb0 ;
	DG_PRIM2_UVRGB	*uvrgb1 ;

	fvtemp0 = work->prim->pos[   clock ];
	fvtemp1 = work->prim->pos[ 1-clock ];
	uvrgb0 = work->prim->uvrgb[   clock ];
	uvrgb1 = work->prim->uvrgb[ 1-clock ];
	OK_Mem_Scr( SCR_POS, fvtemp1, sizeof(FVECTOR), N_PRIMS*N_VERTS );
	OK_Mem_Scr( SCR_UVS, uvrgb1, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
	fvtemp2 = fvtemp1 = SCR_POS;
	fvtemp2++;
	uvrgb1  = SCR_UVS;
	count = 0;
	for ( i = 0 ; i < TOTAL_POLYS-1 ; i++ ){
		if( fvtemp1->vw > work->decay_time ){
			fvtemp1->vw -= 1.0f;
			alpha = (int)fvtemp2->vw;
		}else{
			if( fvtemp2->vw > 0.0f ){
				fvtemp2->vw -= ALPHA_SUB;
				alpha = (int)(fvtemp2->vw);
			}else{
				fvtemp2->vw = 0.0f;
				alpha = 0;
				count++;
				if( work->list_parts_flag[i] ){
					GM_FreeEneFindFromList( &work->asiato_efl, &work->enefind[i] ) ;
					work->list_parts_flag[i] = 0;
				}
			}
		}

		(uvrgb1++)->a = alpha ;
		(uvrgb1++)->a = alpha ;
		(uvrgb1++)->a = alpha ;
		(uvrgb1++)->a = alpha ;
		fvtemp1+=4;
		fvtemp2+=4;
	}

	OK_Scr_Mem( uvrgb0,  SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
	OK_Scr_Mem( fvtemp0, SCR_POS, sizeof(FVECTOR), N_PRIMS*N_VERTS );

//printf("[%d][%d]\n",count,TOTAL_POLYS);

	if( count == TOTAL_POLYS-1 ) return 0;
	return 1;
}

/*----------------------------------------------------------------*/
static short CalcRotY( FMATRIX *mat )
{
	FVECTOR	fvtemp;
	float	ftemp;
	short stemp;

	fvtemp.vx = 1.0f;
	fvtemp.vy = 0.0f;
	fvtemp.vz = 0.0f;
	fvtemp.vw = 1.0f;

	DG_SetPos( mat );
	DG_RotVector( &fvtemp, &fvtemp, 1 );
//printf("foot_printf.c:fvtemp %f %f %f\n",fvtemp.vx, fvtemp.vy, fvtemp.vz);
	ftemp = atan2f( fvtemp.vx, fvtemp.vz ) ;
	stemp = ( 4095 & ( short )( ( ftemp * 2048.0f / PI - 1024.0f ) + 0.5f ) );
//printf("foot_printf.c:stemp %d\n",stemp);
	return stemp; /* 四捨五入 */
}


/*----------------------------------------------------------------*/
static int CheckTrap( CONTROL *ctrl )
{
	u_int	*inside ;
	u_int	ui_temp ;
	int		n_inside ;

	n_inside = ctrl->evt.n_inside ;
	inside = ctrl->evt.inside ;

//if(GV_Time%120==0)printf("trap::%d\n",( *inside & 0x00ffffff ));

	if ( GM_PlayerStatus & (PLAYER_ROLLING) ) return -1 ;

	while ( -- n_inside >= 0 ) {
		ui_temp = ( *inside & 0x00ffffff );
		if ( ui_temp == TRAP_OUTSIDE ){
			return 0 ;
		}else if ( ui_temp == TRAP_INSIDE ){
			return 1 ;
		}else if ( ui_temp == TRAP_INSIDEOUT ){
			return 2 ;
		}
		inside ++ ;
	}
	return -1 ;
}

/*----------------------------------------------------------------*/

#pragma optimize( "", off )

static void Act( Work *work )
{
	FMATRIX	*mat;
	int	i;
	int	foot;
	int	floor_flag;
	int	clock;
	float	ftemp;
	float	ftemp2;
	SVECTOR	floor_rot;
	FVECTOR	fvtemp;
	FVECTOR	*pos;
	HZX_FLR		flr[2];
	int			flr_atrs[2];

//printf("(*work->flag):%x\n",(*work->flag));
	if( (*work->flag)&0x08 ){
		(*work->flag) = 0;
		work->current_rest_time = 0;
	}

	work->current_rest_time--;
	if( work->current_rest_time < 0 ) work->current_rest_time = 0;
	work->current_trap = CheckTrap( work->control );

//if(GV_Time%120==0)printf("current_trap:%d\n",work->current_trap);

	if( work->before_trap != work->current_trap ){
		work->delay_times = 2;
		if( work->current_trap == PAT_NOTHING ){
			if( work->before_trap ==  PAT_WATER ){			// 水溜まり後
				work->current_rest_time = work->time;
			}else if( work->before_trap ==  PAT_SNOW ){		// 雪原後
				work->current_rest_time = 0;
			}else if( work->before_trap ==  PAT_FLOUR ){	// 小麦粉後
				work->current_rest_time = work->time;
			}
		}
		work->before_trap = work->current_trap;
	}
	if( work->current_trap == PAT_WATER ){				// 現在水溜まり
		work->current_rest_time = 0;
	}else if( work->current_trap == PAT_SNOW ){			// 現在雪原
		work->current_rest_time = work->time;
	}else if( work->current_trap == PAT_FLOUR ){		// 現在小麦粉
		work->current_rest_time = work->time;
	}


	if( *work->flag  &&  work->current_rest_time > 0 ) work->activate_flag = 1;

	if( !work->activate_flag ){
		DG_InvisiblePrim2( work->prim ) ;
		return;
	}
	DG_VisiblePrim2( work->prim ) ;

   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = work->prim->buffer_clock;
	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;
	work->activate_flag = FadePrim( work, clock );

	if( work->current_rest_time <= 0 ) return;

	for( i=0; i<N_FOOT; i++ ){
		if( (*work->flag) & (i+1) ){
			if( work->delay_times > 0 ){
				work->delay_times--;
				break;
			}

			foot = foot_joint[i];
			mat = &work->body->objs->objs[foot].world;

			DG_COPY_VEC( &fvtemp ,(FVECTOR *)mat->m[3]);
			fvtemp.vy += 100.0f;
			floor_flag = HZX_LevelHazardCheck(
	                               work->control->hzx_id,
	                               &fvtemp,
	                               HZX_CHK_F_FLOOR,
//	                               HZX_FLOOR_NO_PLAYER );
	                               HZX_FLOOR_NO_BULLET | HZX_FLOOR_RECOIL_TYPE );
			if( floor_flag & 1 ){
				float	bound_min_x;
				float	bound_min_z;
				float	bound_max_x;
				float	bound_max_z;

				HZX_GetLevelHazard( flr, flr_atrs );
				work->activate_flag = 1;
				CalcLocalRot_Floor( flr, &floor_rot );
#if 1 //yano 2002.02.26
				ftemp = HZX_GetFloorLevel();
#else
				HZX_GetLevelHeight( ftemp ); /*floor_rotのポインターを壊しています。*/
#endif

				if( ftemp > GM_WaterLevel ){ // 水面下には付けない
					fvtemp.vy = ftemp + 2.0f;

					bound_min_x = DG_MIN( flr->p1.x,   flr->p2.x );
					bound_min_x = DG_MIN( bound_min_x, flr->p3.x );
					bound_min_x = DG_MIN( bound_min_x, flr->p4.x );
					bound_min_z = DG_MIN( flr->p1.z,   flr->p2.z );
					bound_min_z = DG_MIN( bound_min_z, flr->p3.z );
					bound_min_z = DG_MIN( bound_min_z, flr->p4.z );
					bound_max_x = DG_MAX( flr->p1.x,   flr->p2.x );
					bound_max_x = DG_MAX( bound_max_x, flr->p3.x );
					bound_max_x = DG_MAX( bound_max_x, flr->p4.x );
					bound_max_z = DG_MAX( flr->p1.z,   flr->p2.z );
					bound_max_z = DG_MAX( bound_max_z, flr->p3.z );
					bound_max_z = DG_MAX( bound_max_z, flr->p4.z );

					// 階段等の狭い場所では狭い幅の真ん中に置く
					ftemp  = bound_max_x - bound_min_x;
					ftemp2 = bound_max_z - bound_min_z;
					if( DG_MIN( ftemp, ftemp2 ) < NARR_LIMIT ){
						if( ftemp > ftemp2 ){
							fvtemp.vz = ( bound_max_z + bound_min_z ) * 0.5f;
						}else{
							fvtemp.vx = ( bound_max_x + bound_min_x ) * 0.5f;
						}
					}else{
					// はみ出しチェック
//printf("a:%f::%f %f::%f\n",fvtemp.vx,bound_max_x,bound_min_x,STEP_LIMIT);

						if( fvtemp.vx > bound_max_x - STEP_LIMIT ){
//printf("b:\n");
							fvtemp.vx = bound_max_x - STEP_LIMIT;
						}else if( fvtemp.vx < bound_min_x + STEP_LIMIT ){
//printf("c:\n");
							fvtemp.vx = bound_min_x + STEP_LIMIT;
						}
//printf("d:%f::%f %f::%f\n",fvtemp.vz,bound_max_z,bound_min_z,STEP_LIMIT);
						if( fvtemp.vz > bound_max_z - STEP_LIMIT ){
//printf("e:\n");
							fvtemp.vz = bound_max_z - STEP_LIMIT;
						}else if( fvtemp.vz < bound_min_z + STEP_LIMIT ){
//printf("f:\n");
							fvtemp.vz = bound_min_z + STEP_LIMIT;
						}
					}


					SetPrims( work, &work->prim->pos[clock][work->top_num*4], &fvtemp, &floor_rot, CalcRotY( mat ), i+1 );

					if( !((*work->flag)&0x04) ){
						/* 敵発見用リンクリスト登録 */
						EneListSet( work );
						/* 敵発見用リンクリストに個々のデータをセット */
						EneListPartFree( work );
						EneListPartSet( work, &fvtemp, work->control->addr );
					}

					work->top_num++;
					if( work->top_num >= TOTAL_POLYS-1 ) work->top_num = 0;
				}

			}
		}
	}
	(*work->flag) &= ~(0x03);


	// 一つ遠くに設定
	_sceVu0ScaleVector( &fvtemp, (FVECTOR *)DG_Chanls->eye.m[2], 80000.0f );
	pos = work->prim->pos[clock];
	pos+= (TOTAL_POLYS-1)*4;
	DG_COPY_VEC( pos++, &fvtemp );
	DG_COPY_VEC( pos++, &fvtemp );
	DG_COPY_VEC( pos++, &fvtemp );
	DG_COPY_VEC( pos++, &fvtemp );
	pos = work->prim->pos[1-clock];
	pos+= (TOTAL_POLYS-1)*4;
	DG_COPY_VEC( pos++, &fvtemp );
	DG_COPY_VEC( pos++, &fvtemp );
	DG_COPY_VEC( pos++, &fvtemp );
	DG_COPY_VEC( pos++, &fvtemp );



}

#pragma optimize( "", on )


static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
	EneListFree( work );
}

/*----------------------------------------------------------------*/
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR			*pos ;
	DG_PRIM2_UVRGB	*uvrgb ;
	int		i, k ;

	DG_ConfigPrim2Tex( prim, tex );
	if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/04/02 */
	{
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	}
	else
	{
//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	}

	pos   = SCR_POS ;
	uvrgb = SCR_UVS ;
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		for ( k = 0 ; k < N_POLYS ; k++ ){
			DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
			DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
			DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
			DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;

			uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x8fff ;
			uvrgb->r = BASE_RGB ;
			uvrgb->g = BASE_RGB ;
			uvrgb->b = BASE_RGB ;
			uvrgb->a = 0 ;
			uvrgb++;

			uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x8fff ;
			uvrgb->r = BASE_RGB ;
			uvrgb->g = BASE_RGB ;
			uvrgb->b = BASE_RGB ;
			uvrgb->a = 0 ;
			uvrgb++;

			uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x0fff ;
			uvrgb->r = BASE_RGB ;
			uvrgb->g = BASE_RGB ;
			uvrgb->b = BASE_RGB ;
			uvrgb->a = 0 ;
			uvrgb++;

			uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x0fff ;
			uvrgb->r = BASE_RGB ;
			uvrgb->g = BASE_RGB ;
			uvrgb->b = BASE_RGB ;
			uvrgb->a = 0 ;
			uvrgb++;

			if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/04/02 */
			{
				(uvrgb - 4)->r = (uvrgb - 4)->g = (uvrgb - 4)->b = 
				(uvrgb - 3)->r = (uvrgb - 3)->g = (uvrgb - 3)->b = 
				(uvrgb - 2)->r = (uvrgb - 2)->g = (uvrgb - 2)->b = 
				(uvrgb - 1)->r = (uvrgb - 1)->g = (uvrgb - 1)->b = 128 ;
			}
		}
	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS);
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS);
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);

	return 1;
}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/04/02 */
	{
	tex = DG_GetTexture( 1882897 /*"vr2_footstamp1_add_alp"*/ );
	}
	else
	{
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
//	tex = DG_GetTexture( 6689720 /*"asiato"*/ );
//	tex = DG_GetTexture( 2476846 /*"fstamp01_alp"*/ );
//	tex = DG_GetTexture( 3525422 /*"fstamp02_alp"*/ );
	tex = DG_GetTexture( 4573998 /*"fstamp03_alp"*/ );
	}

	if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/06/25 */
	{
	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	}
	else
	{
	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_FOG, N_PRIMS, N_VERTS );
	}
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );

	work->delay_times = 0;
	work->top_num = 0;
	work->activate_flag = 0;
	work->before_trap = -1;

	return 0 ;
}

/*
起動時のflagの内容は意味無し。

起動後
flag:0x00:何も起こらない
flag:0x01:右足の真下に足跡（ENE_FIND付ける）
flag:0x02:左足の真下に足跡（ENE_FIND付ける）
flag:0x03:両足の真下に足跡（ENE_FIND付ける）

flag:0x05右足の真下に足跡（ENE_FIND付けない）
flag:0x06:左足の真下に足跡（ENE_FIND付けない）
flag:0x07:両足の真下に足跡（ENE_FIND付けない）

flag:0x08:リセット（新たにトラップに入るまで足跡付けない）

time:出てから完全に消えるまでの時間（６０で１秒）

*/
void *NewFootPrint( OBJECT *body, CONTROL *control, int *flag, int time )
{
	int			i;
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->list_flag = 0;
		for( i=0; i<TOTAL_POLYS; i++ ){
			work->list_parts_flag[i] = 0;
		}
		work->body = body;
		work->control = control;
		work->flag = flag;

		work->time = DIRECT_TICK( time );
		if( work->time <= 0 ) work->time = 1;
//		work->decay_time = work->time / ALPHA_DIV;
		work->decay_time = work->time;


		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
