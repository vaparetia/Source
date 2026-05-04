//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	d_splash_fall.c
	落下水飛沫（シナリオ起動）
	2000/04/14 S.Okajima
	$Id: d_splash_fall.c,v 1.4 2002/11/23 12:24:49 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"libmt.h"
#include	"libutl.h"
#include	"utl_dma.h"
#include	"../etc/ok_util.h"

#define	SCR_POS			(SCRPAD_ADDR)
#define	SCR_VEC			(SCRPAD_ADDR + 0x1000)
#define	SCR_UVS			(SCRPAD_ADDR + 0x2000)

#define	SCR_W0			(SCRPAD_ADDR + 0x3f00)
#define	SCR_W1			(SCRPAD_ADDR + 0x3f10)
#define	SCR_W2			(SCRPAD_ADDR + 0x3f20)

#define	N_VERTS		(64)
#define	N_POLYS		(N_VERTS/4)


#define	BASE_RGB		(255)
#define	MAX_ALPHA		(64)

#define	COUNT_MIN		(24)
#define	COUNT_RND		(16)

#define	SIZE			(140.0f)

#define	ANGLE_LOWER		(64)
#define	ANGLE_UPPER		(256)

#define	SLOW_LIMIT		(0.2f)


extern	SVECTOR	G_wind_rot;
extern float	OK_slow_param;

/*----------------------------------------------------------------*/
typedef	struct	{
	float		count;
	int			count_max;
	FVECTOR		vec[N_VERTS];
} Unit ;

typedef	struct	{
	GV_ACT_EX	actor ;
	int			name;
	int			where;

	int			side;

	DG_PRIM2	*prim ;

	FVECTOR		line0;
	FVECTOR		line1;
	FVECTOR		diff;
	FVECTOR		diff_unit;
	SVECTOR		vec_rot;
	SVECTOR		rot;

	int			repeat_num;
	int			repeat_count;
	float		intense;

	int			base_vol;
	int			track_num;

	int			invisible_flag;

	Unit		unit[0];
} Work ;

/*----------------------------------------------------------------*/
/* 座標計算データのみで不可視 */
static	void	InvisibleVectors( Work *work, int prim_num, int clock )
{
	int		i;
	FVECTOR	*pos,*tmp;

	pos   = work->prim->pos[ clock ];
	pos  += prim_num*N_VERTS;
	for( i=0; i<N_POLYS; i++ ){
#ifdef BP_PSX2_ASM
		asm volatile ("
			lqc2		vf12,0x00(%0)
			sqc2		vf12,0x10(%0)
			sqc2		vf12,0x20(%0)
			sqc2		vf12,0x30(%0)
			"::"r"(pos):"memory"
		);
		pos += 4;
#else
		tmp = pos ;
		DG_COPY_VEC( pos, tmp ); pos++ ;
		DG_COPY_VEC( pos, tmp ); pos++ ;
		DG_COPY_VEC( pos, tmp ); pos++ ;
		DG_COPY_VEC( pos, tmp ); pos++ ;
#endif
	}
}


/*----------------------------------------------------------------*/
/* 座標計算データを初期化する */
static	void	InitVectors( Work *work, int prim_num, int clock )
{
	int		i;
	int		num;
	float	len;
	float	width;
	float	ftemp;
	float	rs,rc;
	SVECTOR	rot;
	FVECTOR	mulv;
	FVECTOR	*pos,*tmp;
	DG_PRIM2_UVRGB	*uvrgb ;
	FVECTOR	*vec;
	FVECTOR	fvcalc[4];
	FVECTOR	*scw_fvtemp0 = (void *)SCR_W0 ;
	FVECTOR	*scw_fvtemp1 = (void *)SCR_W1 ;
	FVECTOR	*scw_fvtemp2 = (void *)SCR_W2 ;
   FVECTOR sin_cos;

	if( OK_slow_param < SLOW_LIMIT ){
		mulv.vx = mulv.vy = mulv.vz = 1.0f / SLOW_LIMIT;
	}else{
		mulv.vx = mulv.vy = mulv.vz = 1.0f / OK_slow_param;
	}

	work->unit[prim_num].count_max = COUNT_MIN + (irnd()>>8)%COUNT_RND;
	work->unit[prim_num].count     = (float)work->unit[prim_num].count_max;

	num   = prim_num*N_VERTS;
	pos   = work->prim->pos[ clock ];
	pos  += num;
	uvrgb = work->prim->uvrgb[ clock ];
	uvrgb+= num;
	vec   = work->unit[prim_num].vec;
	rot.vz = 0;

	DG_COPY_VEC( scw_fvtemp0, &work->line0 );
	DG_COPY_VEC( scw_fvtemp1, &work->diff );
	for( i=0; i<N_POLYS; i++ ){
#if 0
		ratio = rnd();
		center.vx = work->line0.vx + work->diff.vx * ratio;
		center.vy = work->line0.vy + work->diff.vy * ratio;
		center.vz = work->line0.vz + work->diff.vz * ratio;
		DG_COPY_VEC( pos, &center );	pos++;
		DG_COPY_VEC( pos, &center );	pos++;
		DG_COPY_VEC( pos, &center );	pos++;
		DG_COPY_VEC( pos, &center );	pos++;
#else
#ifdef BP_PSX2_ASM
		scw_fvtemp2->vx = rnd();
		asm volatile ("
			lqc2		vf12,0x00(%1)
			lqc2		vf13,0x00(%2)
			lqc2		vf14,0x00(%3)

			vadda.xyz	ACC, vf12,vf0 	
			vmaddx.xyz	vf13,vf13,vf14

			vmove.xyz	vf8 ,vf13
			vmove.xyz	vf9 ,vf13
			vmove.xyz	vf10,vf8
			vmove.xyz	vf11,vf9

			sqc2		vf8 ,0x00(%0)
			sqc2		vf9 ,0x10(%0)
			sqc2		vf10,0x20(%0)
			sqc2		vf11,0x30(%0)
			"::"r"(pos),"r"(scw_fvtemp0),"r"(scw_fvtemp1),"r"(scw_fvtemp2):"memory"
		);
//		pos += 4;
#else
		scw_fvtemp2->vx = rnd();
		scw_fvtemp1->vx= scw_fvtemp2->vx*work->diff.vx + scw_fvtemp0->vx ;
		scw_fvtemp1->vy= scw_fvtemp2->vx*work->diff.vy + scw_fvtemp0->vy ;
		scw_fvtemp1->vz= scw_fvtemp2->vx*work->diff.vz + scw_fvtemp0->vz ;
		tmp = pos ;
		DG_COPY_VEC( tmp, scw_fvtemp1 ); tmp++ ;
		DG_COPY_VEC( tmp, scw_fvtemp1 ); tmp++ ;
		DG_COPY_VEC( tmp, scw_fvtemp1 ); tmp++ ;
		DG_COPY_VEC( tmp, scw_fvtemp1 );
#endif
#endif


		len   = -work->intense * SIZE * (rnd() + 0.2f);
		width = len * 0.8f;

		ftemp = frnd()*PI;
      BP_SinCosFast(&sin_cos, ftemp);
		rs = sin_cos.vx * width;
		rc = sin_cos.vy * width;
		ftemp = len*0.55f;
		fvcalc[0].vx= 0.0f;
		fvcalc[0].vy= 0.0f;
		fvcalc[0].vz=len;

		fvcalc[1].vx= rs;
		fvcalc[1].vy= rc;
		fvcalc[1].vz= ftemp;

		fvcalc[2].vx=-rs;
		fvcalc[2].vy=-rc;
		fvcalc[2].vz= ftemp;

		fvcalc[3].vx= 0.0f;
		fvcalc[3].vy= 0.0f;
		fvcalc[3].vz= len*0.1f;
/*
		rot.vx = 0;
		rot.vy = 0;
		rot.vz = irnd()%4096;
		DG_SetPos2( &DG_ZeroVector, &rot );
		DG_RotVector( fvcalc, fvcalc, 4 );
*/
		rot.vx = work->vec_rot.vx - irnd()%128 - 128;
		rot.vy = work->vec_rot.vy + irnd()%256  - 128;
		DG_SetPos2( &DG_ZeroVector, &rot );
		DG_RotVector( fvcalc, vec, 4 );
#ifdef BP_PSX2_ASM
		asm volatile ("
			lqc2		vf12,0x00(%0)
			lqc2		vf13,0x00(%1)
			lqc2		vf14,0x10(%0)
			lqc2		vf15,0x10(%1)
			lqc2		vf16,0x20(%0)
			lqc2		vf17,0x20(%1)
			lqc2		vf18,0x30(%0)
			lqc2		vf19,0x30(%1)
			lqc2		vf20,0x00(%2)

			vmul.xyz	vf13, vf13, vf20

			vadd.xyz	vf12, vf12, vf13
			vadd.xyz	vf14, vf14, vf15
			vadd.xyz	vf16, vf16, vf17
			vadd.xyz	vf18, vf18, vf19

			sqc2		vf12,0x00(%0)
			sqc2		vf14,0x10(%0)
			sqc2		vf16,0x20(%0)
			sqc2		vf18,0x30(%0)
			"::"r"(pos),"r"(vec),"r"(&mulv):"memory"
		);
#else
			vec[0].vx *= mulv.vx ;
			vec[0].vy *= mulv.vy ;
			vec[0].vz *= mulv.vz ;

			pos[0].vx += vec[0].vx ;
			pos[0].vy += vec[0].vy ;
			pos[0].vz += vec[0].vz ;

			pos[1].vx += vec[1].vx ;
			pos[1].vy += vec[1].vy ;
			pos[1].vz += vec[1].vz ;

			pos[2].vx += vec[2].vx ;
			pos[2].vy += vec[2].vy ;
			pos[2].vz += vec[2].vz ;

			pos[3].vx += vec[3].vx ;
			pos[3].vy += vec[3].vy ;
			pos[3].vz += vec[3].vz ;

#endif
		pos += 4;

#if 0
		(vec++)->vw = P_GRAVITY*1.7f;
		(vec++)->vw = P_GRAVITY*1.7f;
		(vec++)->vw = P_GRAVITY*1.7f;
		(vec++)->vw = P_GRAVITY*1.7f;
#else
		(vec++)->vw = P_GRAVITY;
		(vec++)->vw = P_GRAVITY;
		(vec++)->vw = P_GRAVITY;
		(vec++)->vw = P_GRAVITY;
#endif

		uvrgb->a = MAX_ALPHA;
		uvrgb++;
	}
}

/*----------------------------------------------------------------*/
/* ポリゴンの座標データを更新する */
static	void	UpdateVectors( Work *work, int prim_num, int clock, FVECTOR *slow )
{
	int	j;
	int	num;
	FVECTOR			*d_pos;
   FVECTOR        *pos;
	DG_PRIM2_UVRGB	*uvrgb;
	FVECTOR			*vec;
	u_char	col;
	Unit			*unit;

	unit = &work->unit[prim_num];
	col=(u_char)( (int)(unit->count) * MAX_ALPHA / unit->count_max );

	work->unit[prim_num].count -= 1.0f;
	if( work->unit[prim_num].count < 0.0f ) work->unit[prim_num].count = 0.0f;

	num   = prim_num*N_VERTS;
	d_pos = work->prim->pos[1-clock] + num;
   pos   = work->prim->pos[clock] + num;
   // also works as a precache
   memcpy(pos, d_pos, 4*N_POLYS*sizeof(FVECTOR));
	uvrgb = work->prim->uvrgb[clock];
   uvrgb += num;
	vec   = unit->vec;
	for( j=0; j<N_POLYS; j++ )
   {
		/* α */
		uvrgb->a = col;	uvrgb++;
		uvrgb->a = col;	uvrgb++;
		uvrgb->a = col;	uvrgb++;
		uvrgb->a = col;	uvrgb++;

		pos[0].vx += vec[0].vx*slow->vx;
      pos[0].vy += vec[0].vy*slow->vy;
      pos[0].vz += vec[0].vz*slow->vz;

      pos[1].vx += vec[1].vx*slow->vx;
      pos[1].vy += vec[1].vy*slow->vy;
      pos[1].vz += vec[1].vz*slow->vz;

      pos[2].vx += vec[2].vx*slow->vx;
      pos[2].vy += vec[2].vy*slow->vy;
      pos[2].vz += vec[2].vz*slow->vz;

      pos[3].vx += vec[3].vx*slow->vx;
      pos[3].vy += vec[3].vy*slow->vy;
      pos[3].vz += vec[3].vz*slow->vz;

		vec[0].vy += vec[0].vw ;
		vec[1].vy += vec[1].vw ;
		vec[2].vy += vec[2].vw ;
		vec[3].vy += vec[3].vw ;

      pos += 4;
		vec += 4;
	}
}

/*----------------------------------------------------------------*/
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

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	FVECTOR	fvtemp;
	FVECTOR	slow;
	float	ftemp;
	int		i;
	int		clock;
	int		repeat_count;
	int		put_flag;
	Unit	*unit;

	CheckMesgParam( work );

	fvtemp.vx = 0.0f;
	fvtemp.vz = 0.0f;
	fvtemp.vy = 4096.0f;
	OK_DirectionSmoother( &work->rot, &G_wind_rot, 0.993f );
	DG_SetPos2( &DG_ZeroVector, &work->rot );
	DG_PutVector( &fvtemp, &fvtemp, 1 );
	fvtemp.vy = 0.0f;
	_sceVu0Normalize( &fvtemp, &fvtemp );
	work->intense = _sceVu0InnerProduct( &work->diff_unit, &fvtemp );
	work->intense = ( work->intense > 0.0f )?work->intense:-work->intense*0.1f;
	work->intense = ( work->intense < 1.0f )?work->intense:1.0f;

	repeat_count = (int)((float)work->repeat_num * work->intense);
	repeat_count = ( repeat_count < work->repeat_num )? repeat_count: work->repeat_num;

	if( repeat_count < work->repeat_count ){
		work->repeat_count--;
	}else if( repeat_count > work->repeat_count ){
		work->repeat_count+=2;
		if( work->repeat_count > repeat_count  ) work->repeat_count = repeat_count;
	}

	if( work->invisible_flag ){
		DG_InvisiblePrim2( work->prim );
		return;
	}else{
		DG_VisiblePrim2( work->prim );
	}

	work->base_vol = (int)(80.0f * work->intense + 20.0f);

	ftemp = ( OK_slow_param > SLOW_LIMIT )? OK_slow_param: SLOW_LIMIT;
	slow.vx = ftemp;
	slow.vy = ftemp;
	slow.vz = ftemp;
	slow.vw = ftemp;

	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;
	unit = work->unit;
	put_flag = 0;
	if( GM_CheckGameStatus( STATE_DEMO ) ){
		work->repeat_count = work->repeat_num;
		work->intense = 1.0f;
		for ( i = 0 ; i < work->repeat_count ; i++ ){
			if( unit->count<=0.0f ){
				InitVectors( work, i, clock );
			}else{
				UpdateVectors( work, i, clock, &slow );
			}
			unit++;
		}
	}else{
		for ( i = 0 ; i < work->repeat_count ; i++ ){
			if( unit->count<=0.0f ){
				if(!put_flag ){
					put_flag = 1;
					InitVectors( work, i, clock );
				}else{
					InvisibleVectors( work, i, clock );
				}
			}else{
				UpdateVectors( work, i, clock, &slow );
			}
			unit++;
		}
	}

	for (  ; i < work->repeat_num ; i++ ){
		if( unit->count<=0.0f ){
			InvisibleVectors( work, i, clock );
		}else{
			UpdateVectors( work, i, clock, &slow );
		}
		unit++;
	}



}


static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}


/* 初期設定値を取得 */
static	int	GetOptionValue( Work *work )
{
	FVECTOR	fvtemp;

	if ( GCL_GetOption( 'l' ) != NULL ){
		work->line0.vx = (float)GCL_GetNextInt();
		work->line0.vy = (float)GCL_GetNextInt();
		work->line0.vz = (float)GCL_GetNextInt();
		work->line1.vx = (float)GCL_GetNextInt();
		work->line1.vy = (float)GCL_GetNextInt();
		work->line1.vz = (float)GCL_GetNextInt();
	}else{
		return -1;
	}
	work->diff.vx = work->line1.vx - work->line0.vx;
	work->diff.vy = work->line1.vy - work->line0.vy;
	work->diff.vz = work->line1.vz - work->line0.vz;

	work->side = 0;
	if ( GCL_GetOption( 's' ) != NULL ){
		work->side = GCL_GetNextInt();
	}

	if( work->side==0 ){	/* 右回転 */
		fvtemp.vx =  work->diff.vz;
		fvtemp.vy =  0.0f;
		fvtemp.vz = -work->diff.vx;
	}else{	/* 左回転 */
		fvtemp.vx = -work->diff.vz;
		fvtemp.vy =  0.0f;
		fvtemp.vz =  work->diff.vx;
	}
	_sceVu0Normalize( &work->diff_unit, &fvtemp );
	OK_DirVecXY( &DG_ZeroVector, &fvtemp, &work->vec_rot );


	if ( GCL_GetOption( 't' ) != NULL ){
		work->track_num = GCL_GetNextInt();
	}else{
		work->track_num = -1;
	}

	return 0;
}

/*----------------------------------------------------------------*/
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGB	*uvrgb0 ;
	DG_PRIM2_UVRGB	*uvrgb1 ;
	int		i, k ;

	DG_ConfigPrim2Tex( prim, tex );
//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	uvrgb0 = prim->uvrgb[ 0 ] ;
	uvrgb1 = prim->uvrgb[ 1 ] ;
	for ( i = 0 ; i < work->repeat_num ; i++ ){
		for ( k = 0 ; k < N_POLYS ; k++ ){
			uvrgb1->u = uvrgb0->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb1->v = uvrgb0->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb1->q = uvrgb0->q = 4096 ;
			uvrgb1->f = uvrgb0->f = 0x8fff ;
			uvrgb1->r = uvrgb0->r = BASE_RGB;
			uvrgb1->g = uvrgb0->g = BASE_RGB;
			uvrgb1->b = uvrgb0->b = BASE_RGB;
			uvrgb1->a = uvrgb0->a = MAX_ALPHA ;
			uvrgb1++; uvrgb0++;

			uvrgb1->u = uvrgb0->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb1->v = uvrgb0->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb1->q = uvrgb0->q = 4096 ;
			uvrgb1->f = uvrgb0->f = 0x8fff ;
			uvrgb1->r = uvrgb0->r = BASE_RGB;
			uvrgb1->g = uvrgb0->g = BASE_RGB;
			uvrgb1->b = uvrgb0->b = BASE_RGB;
			uvrgb1->a = uvrgb0->a = MAX_ALPHA ;
			uvrgb1++; uvrgb0++;

			uvrgb1->u = uvrgb0->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb1->v = uvrgb0->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb1->q = uvrgb0->q = 4096 ;
			uvrgb1->f = uvrgb0->f = 0x0fff ;
			uvrgb1->r = uvrgb0->r = BASE_RGB;
			uvrgb1->g = uvrgb0->g = BASE_RGB;
			uvrgb1->b = uvrgb0->b = BASE_RGB;
			uvrgb1->a = uvrgb0->a = MAX_ALPHA ;
			uvrgb1++; uvrgb0++;

			uvrgb1->u = uvrgb0->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb1->v = uvrgb0->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb1->q = uvrgb0->q = 4096 ;
			uvrgb1->f = uvrgb0->f = 0x0fff ;
			uvrgb1->r = uvrgb0->r = BASE_RGB;
			uvrgb1->g = uvrgb0->g = BASE_RGB;
			uvrgb1->b = uvrgb0->b = BASE_RGB;
			uvrgb1->a = uvrgb0->a = MAX_ALPHA ;
			uvrgb1++; uvrgb0++;
		}
	}


	return 1;
}

static int GetResources( Work *work, int name, int where )
{
	int		i;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	work->name  = name;
	work->where = where;

	work->invisible_flag = 0;

	if( GetOptionValue( work ) < 0 ) return -1;

	tex = DG_GetTexture( 6684577 /*"splash05_alp"*/ );
//	tex = DG_GetTexture( 5850884 /*"drop05"*/ );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, work->repeat_num, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );
	GM_GroupPrim2( prim, work->where ) ;

	for( i=0; i<work->repeat_num; i++ ){
		InitVectors( work, i, 0 );
		InitVectors( work, i, 1 );
	}
	DG_VisiblePrim2( work->prim ) ;

	work->base_vol = 0;

	if( work->track_num != -1   &&  !GM_CheckGameStatus( STATE_DEMO ) ){
		extern int		BGM_SetFader( int track, int *base_vol, int status,
					FVECTOR *pos, SVECTOR *size, HZX_GROUP_ID group_id );
		FVECTOR	fvtemp;
		SVECTOR	svtemp;

		fvtemp.vx = (work->line0.vx + work->line1.vx) * 0.5f;
		fvtemp.vy = (work->line0.vy + work->line1.vy) * 0.5f;
		fvtemp.vz = (work->line0.vz + work->line1.vz) * 0.5f;

		svtemp.vx = (short) DG_FABS( (work->line0.vx - work->line1.vx)*0.5f );
		svtemp.vy = (short) DG_FABS( (work->line0.vy - work->line1.vy)*0.5f );
		svtemp.vz = (short) DG_FABS( (work->line0.vz - work->line1.vz)*0.5f );
		if( svtemp.vx < 100 ) svtemp.vx = 100;
		if( svtemp.vy < 3000 ) svtemp.vy = 3000;
		if( svtemp.vz < 100 ) svtemp.vz = 100;

		BGM_SetFader( work->track_num, &work->base_vol, 0x00000000,
                            &fvtemp, &svtemp, GM_GetHzxGroupID( where ) );
	}

	return 0 ;
}


#ifdef KP_WINDOWS
static void DmyAct( Work *work ){ CheckMesgParam(work) ; }
static void DmyDie( Work *work ){}
#endif

void *NewSplashFall_Scn( int name, int where )
{
	Work		*work ;
	int			buf_size;
	int			unit_num;



	OPERATOR() ;

	unit_num=1;
	if ( GCL_GetOption( 'n' ) != NULL ) {
		unit_num = GCL_GetNextInt() ;
	}


	buf_size = sizeof( Work ) + sizeof( Unit ) * unit_num;
	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, buf_size ) ;
	if ( work != NULL ) {
#ifndef KP_WINDOWS
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->repeat_count = work->repeat_num = unit_num;
		work->where = where;

		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
#else
		if ( DG_GetRainEffectLevel() ) {
			GV_SetActor( &( work->actor ), Act, Die ) ;
			GV_ActorEX( &work->actor )

			work->repeat_count = work->repeat_num = unit_num;
			work->where = where;

			if ( GetResources( work, name, where ) < 0 ) {
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
