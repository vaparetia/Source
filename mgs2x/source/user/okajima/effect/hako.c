//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	hako.c
	蓋付き段ボール
	2000/02/23 S.Okajima
	$Id: hako.c,v 1.1.1.3 2002/11/19 11:47:06 Yoshizawa1 Exp $

*/

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
#include <libutl.h>

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#ifdef PSX2
#include	"utl_dma.h"
#endif
#include	"../etc/ok_util.h"

#define PURUPURU_TIME		(4)
#define PURU_SHIFT_XZ		(5.0f)
#define PURU_SHIFT_Y		(10.0f)
#define PURU_ROT			(16)


#define PAPER_THIN			(7.0f)
#define PAPER_THIN2			(PAPER_THIN * 2.0f)
#define DANKON_SIZE			(30.0f)
#define DANKON_SIZE_ADJUST	(DANKON_SIZE * 0.6f)
#define	COLOR_R			(64)
#define	COLOR_G			(64)
#define	COLOR_B			(64)
#define	COLOR_A			(128)


#define TARGET_SIZE_X (1000.0f)
#define TARGET_SIZE_Y ( 350.0f)
#define TARGET_SIZE_Z (1000.0f)
#define TARGET_SHIFT_X (   0.0f)
#define TARGET_SHIFT_Y ( 350.0f)
#define TARGET_SHIFT_Z (   0.0f)


#define TARGET_BASE_CHILD_SIZE_X  ( PAPER_THIN)
#define TARGET_BASE_CHILD_SIZE_Y  (  250.0f - DANKON_SIZE_ADJUST)
#define TARGET_BASE_CHILD_SIZE_Z  (  500.0f - DANKON_SIZE_ADJUST)
#define TARGET_BASE_CHILD_SHIFT_X (-PAPER_THIN * 0.5f)
#define TARGET_BASE_CHILD_SHIFT_Y (   0.0f)
#define TARGET_BASE_CHILD_SHIFT_Z (   0.0f)




#define TARGET_BASE_CHILD_SIZE_X  ( PAPER_THIN)
#define TARGET_BASE_CHILD_SIZE_Y  (  250.0f - DANKON_SIZE_ADJUST)
#define TARGET_BASE_CHILD_SIZE_Z  (  500.0f - DANKON_SIZE_ADJUST)
#define TARGET_BASE_CHILD_SHIFT_X (-PAPER_THIN * 0.5f)
#define TARGET_BASE_CHILD_SHIFT_Y (   0.0f)
#define TARGET_BASE_CHILD_SHIFT_Z (   0.0f)




#define TARGET_CHILD0_SIZE_X  ( PAPER_THIN)
#define TARGET_CHILD0_SIZE_Y  ( 125.0f - DANKON_SIZE_ADJUST)
#define TARGET_CHILD0_SIZE_Z  ( 250.0f - DANKON_SIZE_ADJUST)
#define TARGET_CHILD0_SHIFT_X (-PAPER_THIN * 0.5f)
#define TARGET_CHILD0_SHIFT_Y (-125.0f)
#define TARGET_CHILD0_SHIFT_Z (   0.0f)


#define TARGET_CHILD1_SIZE_X  ( PAPER_THIN)
#define TARGET_CHILD1_SIZE_Y  ( 125.0f - DANKON_SIZE_ADJUST)
#define TARGET_CHILD1_SIZE_Z  ( 250.0f - DANKON_SIZE_ADJUST)
#define TARGET_CHILD1_SHIFT_X (-PAPER_THIN * 0.5f)
#define TARGET_CHILD1_SHIFT_Y (-125.0f)
#define TARGET_CHILD1_SHIFT_Z (   0.0f)


#define TARGET_CHILD2a_SIZE_X  ( PAPER_THIN)
#define TARGET_CHILD2a_SIZE_Y  ( 100.0f - DANKON_SIZE_ADJUST)
#define TARGET_CHILD2a_SIZE_Z  ( 500.0f - DANKON_SIZE_ADJUST)
#define TARGET_CHILD2a_SHIFT_X (-PAPER_THIN * 0.5f)
#define TARGET_CHILD2a_SHIFT_Y (-100.0f)
#define TARGET_CHILD2a_SHIFT_Z (   0.0f)

#define TARGET_CHILD2b_SIZE_X  ( PAPER_THIN)
#define TARGET_CHILD2b_SIZE_Y  ( 125.0f - DANKON_SIZE_ADJUST)
#define TARGET_CHILD2b_SIZE_Z  ( 500.0f - DANKON_SIZE_ADJUST)
#define TARGET_CHILD2b_SHIFT_X ( PAPER_THIN * 0.5f)
#define TARGET_CHILD2b_SHIFT_Y (-125.0f)
#define TARGET_CHILD2b_SHIFT_Z (   0.0f)

#define TARGET_CHILD3a_SIZE_X  ( PAPER_THIN)
#define TARGET_CHILD3a_SIZE_Y  ( 100.0f - DANKON_SIZE_ADJUST)
#define TARGET_CHILD3a_SIZE_Z  ( 500.0f - DANKON_SIZE_ADJUST)
#define TARGET_CHILD3a_SHIFT_X (-PAPER_THIN * 0.5f)
#define TARGET_CHILD3a_SHIFT_Y (-100.0f)
#define TARGET_CHILD3a_SHIFT_Z (   0.0f)


#define TARGET_CHILD3b_SIZE_X  ( PAPER_THIN)
#define TARGET_CHILD3b_SIZE_Y  ( 125.0f - DANKON_SIZE_ADJUST)
#define TARGET_CHILD3b_SIZE_Z  ( 500.0f - DANKON_SIZE_ADJUST)
#define TARGET_CHILD3b_SHIFT_X ( PAPER_THIN * 0.5f)
#define TARGET_CHILD3b_SHIFT_Y (-125.0f)
#define TARGET_CHILD3b_SHIFT_Z (   0.0f)





/* 底 */
#define SOKO_SHIFT_X (      0.0f)
#define SOKO_SHIFT_Y (PAPER_THIN)
#define SOKO_SHIFT_Z (      0.0f)
#define SOKO_ROT_X   (      0.0f)
#define SOKO_ROT_Y   (      0.0f)
#define SOKO_ROT_Z   (   1024.0f)


/* 下 */
#define HUTA0_SHIFT_X (   0.0f)
#define HUTA0_SHIFT_Y ( 300.0f)
#define HUTA0_SHIFT_Z ( 500.0f)
#define HUTA0_ROT_X   (   0.0f)
#define HUTA0_ROT_Y   (1024.0f)
#define HUTA0_ROT_Z   (   0.0f)

/* 上 */
#define HUTA1_SHIFT_X (   0.0f)
#define HUTA1_SHIFT_Y ( 300.0f)
#define HUTA1_SHIFT_Z (-500.0f)
#define HUTA1_ROT_X   (   0.0f)
#define HUTA1_ROT_Y   (3072.0f)
#define HUTA1_ROT_Z   (   0.0f)

/* 左 */
#define HUTA2_SHIFT_X (-250.0f)
#define HUTA2_SHIFT_Y ( 300.0f)
#define HUTA2_SHIFT_Z (   0.0f)
#define HUTA2_ROT_X   (   0.0f)
#define HUTA2_ROT_Y   (   0.0f)
#define HUTA2_ROT_Z   (   0.0f)

/* 右 */
#define HUTA3_SHIFT_X ( 250.0f)
#define HUTA3_SHIFT_Y ( 300.0f)
#define HUTA3_SHIFT_Z (   0.0f)
#define HUTA3_ROT_X   (   0.0f)
#define HUTA3_ROT_Y   (2048.0f)
#define HUTA3_ROT_Z   (   0.0f)



//#define LIMIT_ANGLE_MARGIN	(32.0f * TPI / 4096.0f)
#define LIMIT_ANGLE_MARGIN	(0.0f * TPI / 4096.0f)
#define LIMIT_ANGLE_MIN	(   0.0f * TPI / 4096.0f + LIMIT_ANGLE_MARGIN)
#define LIMIT_ANGLE_MAX	(4096.0f * TPI / 4096.0f - LIMIT_ANGLE_MARGIN)


#define STEP_ANGLE	(128.0f * TPI / 4096.0f)
#define DECAY		(0.95f)

#define N_VERTS (64)
#define N_POLYS (N_VERTS/4)
#define N_PRIMS (8)
#define MAX_DANKON	(N_PRIMS * N_POLYS)

#define MAX_TARGET	(9)
#define MAX_HUTA	(4)

extern void DG_FreePreshade( DG_OBJS * );
extern int	DG_MakePreshade( DG_OBJS *, LIT_DEF * ) ;

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;
	int			name ;
	int			where ;

	int			hit_time;

	TARGET		target;
	TARGET		target_child[MAX_TARGET] ;
	FVECTOR		shift[MAX_TARGET] ;
	SVECTOR		rot[MAX_TARGET] ;
	FVECTOR		frot[MAX_TARGET] ;
	FMATRIX		mat[MAX_TARGET] ;				/* ターゲット参照元 */
	FMATRIX		world ;

	float		base_angle[MAX_HUTA] ;
	float		angle[MAX_HUTA] ;
	float		intense[MAX_HUTA] ;
	int			hit_flag[MAX_HUTA];

	DG_OBJS		*objs ;
	FMATRIX		lights[2] ;


	int			dankon_max_num;
	int			dankon_now_num;
	int			dankon_matrix_num[MAX_DANKON];
	FVECTOR		dankon_verts[MAX_DANKON * 4] ;

	DG_PRIM2	*prim ;

	int			open_close_flag;

	int			iwtemp;

} Work ;

/*------------------------------------------------*/
static	void	ChildTargCallBack( TARGET *off, TARGET *def, void *ptr )
{
	Work	*work;
	FMATRIX	mat;
	FVECTOR	fvec;
	FVECTOR	*pfv;
	int	i;
	int	num;
	int	hit;

	work = ( Work * )ptr ;

	hit = 0;
	if ( def->damaged & TARGET_POWER ) {
		for( i=0; i<MAX_TARGET; i++ ){
			if ( &work->target_child[i] == def  &&  (def->weapon_type & (WP_BULLET|WP_M92)) ){
				hit = 1;

				GM_SeSetMode( SD_A_RICDAN01, &def->hit, GM_SEMODE_BOMB ) ;


//printf("%d\n",i);
//				_sceVu0InversMatrix( &mat, &work->mat[i] );
				FastInverseMatrix( &mat, &work->mat[i] );

				DG_SetPos( &mat );
				DG_PutVector( &def->hit, &fvec, 1 );
//printf("a:%f %f %f\n",def->hit.vx,def->hit.vy,def->hit.vz);
//AN_Test_Eye3( &def->hit );
//printf("b:%f %f %f\n",fvec.vx,fvec.vy,fvec.vz);

				num = work->dankon_now_num;
				work->dankon_matrix_num[num] = i;
				pfv = &work->dankon_verts[num*4];
				pfv->vx = fvec.vx;
				pfv->vy = fvec.vy + DANKON_SIZE;
				pfv->vz = fvec.vz - DANKON_SIZE;
				pfv++;
				pfv->vx = fvec.vx;
				pfv->vy = fvec.vy + DANKON_SIZE;
				pfv->vz = fvec.vz + DANKON_SIZE;
				pfv++;
				pfv->vx = fvec.vx;
				pfv->vy = fvec.vy - DANKON_SIZE;
				pfv->vz = fvec.vz - DANKON_SIZE;
				pfv++;
				pfv->vx = fvec.vx;
				pfv->vy = fvec.vy - DANKON_SIZE;
				pfv->vz = fvec.vz + DANKON_SIZE;
				pfv++;
				work->dankon_now_num++;
				if( work->dankon_now_num >= MAX_DANKON ) work->dankon_now_num=0;
				work->dankon_max_num++;
				if( work->dankon_max_num >= MAX_DANKON ) work->dankon_max_num=MAX_DANKON;

				num = work->dankon_now_num;
				work->dankon_matrix_num[num] = i;
				if( fvec.vx > 0.0f ){
					pfv->vx = fvec.vx - PAPER_THIN2;
					pfv->vy = fvec.vy + DANKON_SIZE;
					pfv->vz = fvec.vz - DANKON_SIZE;
					pfv++;
					pfv->vx = fvec.vx - PAPER_THIN2;
					pfv->vy = fvec.vy + DANKON_SIZE;
					pfv->vz = fvec.vz + DANKON_SIZE;
					pfv++;
					pfv->vx = fvec.vx - PAPER_THIN2;
					pfv->vy = fvec.vy - DANKON_SIZE;
					pfv->vz = fvec.vz - DANKON_SIZE;
					pfv++;
					pfv->vx = fvec.vx - PAPER_THIN2;
					pfv->vy = fvec.vy - DANKON_SIZE;
					pfv->vz = fvec.vz + DANKON_SIZE;
					pfv++;
				}else{
					pfv->vx = fvec.vx + PAPER_THIN2;
					pfv->vy = fvec.vy + DANKON_SIZE;
					pfv->vz = fvec.vz - DANKON_SIZE;
					pfv++;
					pfv->vx = fvec.vx + PAPER_THIN2;
					pfv->vy = fvec.vy + DANKON_SIZE;
					pfv->vz = fvec.vz + DANKON_SIZE;
					pfv++;
					pfv->vx = fvec.vx + PAPER_THIN2;
					pfv->vy = fvec.vy - DANKON_SIZE;
					pfv->vz = fvec.vz - DANKON_SIZE;
					pfv++;
					pfv->vx = fvec.vx + PAPER_THIN2;
					pfv->vy = fvec.vy - DANKON_SIZE;
					pfv->vz = fvec.vz + DANKON_SIZE;
					pfv++;
				}
				work->dankon_now_num++;
				if( work->dankon_now_num >= MAX_DANKON ) work->dankon_now_num=0;
				work->dankon_max_num++;
				if( work->dankon_max_num >= MAX_DANKON ) work->dankon_max_num=MAX_DANKON;

				if( i<MAX_HUTA ){
					work->angle[i]    = rnd()*TPI;
					work->intense[i]  = frnd()*PI;
					work->hit_flag[i] = 1;
				}
		    }
		}
	}

	if( hit ) work->hit_time = PURUPURU_TIME;


}

static void InitTarget( Work *work )
{
	FVECTOR	size, offset ;
	int	i ;

	size.vx = TARGET_SIZE_X;
	size.vy = TARGET_SIZE_Y;
	size.vz = TARGET_SIZE_Z;

	offset.vx = TARGET_SHIFT_X;
	offset.vy = TARGET_SHIFT_Y;
	offset.vz = TARGET_SHIFT_Z;

//	GM_SetTarget( &work->target, TARGET_ONLINE|TARGET_THROUGH|TARGET_DEFENSE|TARGET_POWER|TARGET_CHILD, work->where, BOTH_SIDE,  &size, &offset ) ;
//	GM_SetTarget( &work->target, TARGET_ONLINE|TARGET_THROUGH|TARGET_DEFENSE|TARGET_POWER|TARGET_CHILD, 0, BOTH_SIDE,  &size, &offset ) ;
	GM_SetTarget( &work->target, TARGET_THROUGH|TARGET_DEFENSE|TARGET_POWER|TARGET_CHILD|TARGET_CHILD_ALWAYS, 0, BOTH_SIDE,  &size, &offset ) ;
	GM_PutTarget( &work->target ) ;

//	 NewTargetView( &work->target, 255, 0, 0 ) ;

	for( i=0; i<9; i++ ) {
		switch ( i ) {
			case 0:
				size.vx   = TARGET_CHILD0_SIZE_X;
				size.vy   = TARGET_CHILD0_SIZE_Y;
				size.vz   = TARGET_CHILD0_SIZE_Z;
				offset.vx = TARGET_CHILD0_SHIFT_X;
				offset.vy = TARGET_CHILD0_SHIFT_Y;
				offset.vz = TARGET_CHILD0_SHIFT_Z;
				break ;
			case 4:
				size.vx   = TARGET_CHILD0_SIZE_X;
				size.vy   = TARGET_CHILD0_SIZE_Y;
				size.vz   = TARGET_CHILD0_SIZE_Z;
				offset.vx =-TARGET_CHILD0_SHIFT_X;
				offset.vy = TARGET_CHILD0_SHIFT_Y - 50.0f;
				offset.vz = TARGET_CHILD0_SHIFT_Z;
				break ;
			case 1:
				size.vx   = TARGET_CHILD1_SIZE_X;
				size.vy   = TARGET_CHILD1_SIZE_Y;
				size.vz   = TARGET_CHILD1_SIZE_Z;
				offset.vx = TARGET_CHILD1_SHIFT_X;
				offset.vy = TARGET_CHILD1_SHIFT_Y;
				offset.vz = TARGET_CHILD1_SHIFT_Z;
				break ;
			case 5:
				size.vx   = TARGET_CHILD1_SIZE_X;
				size.vy   = TARGET_CHILD1_SIZE_Y;
				size.vz   = TARGET_CHILD1_SIZE_Z;
				offset.vx =-TARGET_CHILD1_SHIFT_X;
				offset.vy = TARGET_CHILD1_SHIFT_Y - 50.0f;
				offset.vz = TARGET_CHILD1_SHIFT_Z;
				break ;
			case 2:
				size.vx   = TARGET_CHILD2a_SIZE_X;
				size.vy   = TARGET_CHILD2a_SIZE_Y;
				size.vz   = TARGET_CHILD2a_SIZE_Z;
				offset.vx = TARGET_CHILD2a_SHIFT_X;
				offset.vy = TARGET_CHILD2a_SHIFT_Y;
				offset.vz = TARGET_CHILD2a_SHIFT_Z;
				break ;
			case 6:
				size.vx   = TARGET_CHILD2b_SIZE_X;
				size.vy   = TARGET_CHILD2b_SIZE_Y;
				size.vz   = TARGET_CHILD2b_SIZE_Z;
				offset.vx = TARGET_CHILD2b_SHIFT_X;
				offset.vy = TARGET_CHILD2b_SHIFT_Y - 50.0f;
				offset.vz = TARGET_CHILD2b_SHIFT_Z;
				break ;
			case 3:
				size.vx   = TARGET_CHILD3a_SIZE_X;
				size.vy   = TARGET_CHILD3a_SIZE_Y;
				size.vz   = TARGET_CHILD3a_SIZE_Z;
				offset.vx = TARGET_CHILD3a_SHIFT_X;
				offset.vy = TARGET_CHILD3a_SHIFT_Y;
				offset.vz = TARGET_CHILD3a_SHIFT_Z;
				break ;
			case 7:
				size.vx   = TARGET_CHILD3b_SIZE_X;
				size.vy   = TARGET_CHILD3b_SIZE_Y;
				size.vz   = TARGET_CHILD3b_SIZE_Z;
				offset.vx = TARGET_CHILD3b_SHIFT_X;
				offset.vy = TARGET_CHILD3b_SHIFT_Y - 50.0f;
				offset.vz = TARGET_CHILD3b_SHIFT_Z;
				break ;
			case 8:	/* 底 */
				size.vx   = TARGET_BASE_CHILD_SIZE_X;
				size.vy   = TARGET_BASE_CHILD_SIZE_Y;
				size.vz   = TARGET_BASE_CHILD_SIZE_Z;
				offset.vx = TARGET_BASE_CHILD_SHIFT_X;
				offset.vy = TARGET_BASE_CHILD_SHIFT_Y;
				offset.vz = TARGET_BASE_CHILD_SHIFT_Z;
				break ;
		}

//		GM_SetTarget( &work->target_child[i], TARGET_ONLINE|TARGET_THROUGH|TARGET_DEFENSE|TARGET_POWER|TARGET_ROTATE, work->where, BOTH_SIDE, &size, &offset ) ;
//		GM_SetTarget( &work->target_child[i], TARGET_ONLINE|TARGET_THROUGH|TARGET_DEFENSE|TARGET_POWER|TARGET_ROTATE, 0, BOTH_SIDE, &size, &offset ) ;
//		GM_SetTarget( &work->target_child[i], TARGET_ONLINE|TARGET_THROUGH|TARGET_DEFENSE|TARGET_POWER|TARGET_ROTATE|TARGET_CHILD_ALWAYS, 0, BOTH_SIDE, &size, &offset ) ;
		GM_SetTarget( &work->target_child[i], TARGET_ONLINE|TARGET_THROUGH|TARGET_DEFENSE|TARGET_POWER|TARGET_ROTATE, 0, BOTH_SIDE, &size, &offset ) ;
		GM_SetTargetCallBack( &work->target_child[i], ChildTargCallBack, work ) ;

#if 0
		if( i<4 ){
			NewTargetView( &work->target_child[i], 0, 255, 0 ) ;
		}else{
			NewTargetView( &work->target_child[i], 0, 0, 255 ) ;
		}
#endif

	}
	GM_SetTargetParts( &work->target, work->target_child, MAX_TARGET, 0 ) ;
}


/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int		i,num;
	FMATRIX	world;
	FMATRIX	*mat;
	FVECTOR	*frot;
	FVECTOR	*pos;
	FVECTOR	*shift;
	FVECTOR	fvtemp;
	SVECTOR	svtemp;
	SVECTOR	*rot;
	DG_OBJ	*objs;
	float	*angle;
	float	*intense;
	float	*base_angle;
	float	ftemp;
	TARGET	*target;

	DG_SwitchBuffPrim2( work->prim );

	intense    = work->intense;
	for(i=0; i<MAX_HUTA; i++){
		if( (*intense)!=0.0f) break;
		intense++;
	}
//	if( i==MAX_HUTA && work->open_close_flag==0) return;
	if( i==MAX_HUTA ) return;

	/* 全体揺らし */
	if( work->hit_time > 0 ){
		DG_SetPos( &work->world ) ;
		fvtemp.vx = frnd() * PURU_SHIFT_XZ;
		fvtemp.vy =  rnd() * PURU_SHIFT_Y;
		fvtemp.vz = frnd() * PURU_SHIFT_XZ;
		svtemp.vx = irnd() % PURU_ROT - PURU_ROT/2;
		svtemp.vy = irnd() % PURU_ROT - PURU_ROT/2;
		svtemp.vz = irnd() % PURU_ROT - PURU_ROT/2;
		DG_MovePos( &fvtemp );
		DG_RotatePos( &svtemp );
		DG_GetPos( &world ) ;
		work->hit_time--;
	}else{
		DG_COPY_MAT( &world, &work->world );
	}

	/* 親ターゲット */
	DG_COPY_MAT( &work->objs->world, &world );
	DG_COPY_MAT( &work->objs->objs[0].world, &world );
	GM_CurrentMap = work->where;	//act control で やるものの代用
	GM_MoveTarget2( &work->target, &world );

	num=0;
	for( i=0; i<MAX_HUTA; i++ ){
		if( work->hit_flag[i] != 0 ){
			num=1;
			break;
		}
	}
	if( num!=0 ){
		angle      = work->angle;
		intense    = work->intense;
		for( i=0; i<MAX_HUTA; i++ ){
			if( work->hit_flag[i] != 0 ){
				work->hit_flag[i]  = 0;
			}else{
				(*angle)   = rnd()*TPI;
				(*intense) = frnd()*PI*0.6f;
			}
			angle++;
			intense++;
		}
	}

//	work->iwtemp = (work->iwtemp + 32)&4095;
//printf("%d\n",work->iwtemp);

	frot       = work->frot;
	frot++;
	angle      = work->angle;
	intense    = work->intense;
	base_angle = work->base_angle;
	mat        = work->mat;
	shift      = work->shift;
	rot        = work->rot;
	target     = work->target_child;
	objs       = work->objs->objs;
	objs++;
	for(i=0; i<MAX_TARGET; i++){
		if( i<MAX_HUTA ){
//printf("%f:%f:%f:\n",(*base_angle),(*intense),cosf( (*angle) ));

			ftemp = (*base_angle) + (*intense) * cosf( (*angle) );
//			ftemp = (float)work->iwtemp * TPI / 4096.0f;

			if( ftemp < LIMIT_ANGLE_MIN ){
				ftemp = LIMIT_ANGLE_MIN;
				(*angle)    = TPI - (*angle);
				(*intense) *= 0.8f;
			}else if( ftemp > LIMIT_ANGLE_MAX ){
				ftemp = LIMIT_ANGLE_MAX;
				(*angle)    = TPI - (*angle);
				(*intense) *= 0.8f;
			}


			if( i==0 ){
				frot->vx    = TPI - ftemp;
				if( frot->vx > PI ) frot->vx -= TPI;
			}else if( i==1 ){
				frot->vx    = ftemp;
				if( frot->vx > PI ) frot->vx -= TPI;
			}else if( i==2 ){
				frot->vz    = TPI - ftemp;
				if( frot->vz > PI ) frot->vz -= TPI;
			}else if( i==3 ){
				frot->vz    = ftemp;
				if( frot->vz > PI ) frot->vz -= TPI;
			}
			(*angle)   += STEP_ANGLE;
			(*intense) *= DECAY;
			DG_SetPos( &objs->world ) ;
			intense++;
			base_angle++;
			angle++;
			frot++;
			objs++;
		}else{
			DG_SetPos( &world ) ;
		}
		DG_MovePos( shift );
		DG_RotatePos( rot );
		DG_GetPos( mat ) ;
		GM_CurrentMap = work->where;	//act control で やるものの代用
		GM_MoveTarget2( target, mat );

		shift++;
		rot++;
		mat++;
		target++;
	}

	num   = work->prim->buffer_clock;
	pos   = work->prim->pos[num];
	shift = work->dankon_verts;
	if( work->dankon_max_num!=0 ){
		for( i=0; i<work->dankon_max_num; i++ ){
			num = work->dankon_matrix_num[i];
			DG_SetPos( &work->mat[num] ) ;
			DG_PutVector( shift, pos, 4 );
			shift+=4;
			pos  +=4;
		}
	}

#if 0
	{/* バウンディングのデバッグ表示 */
		extern void *NewBoundingBoxView_1( FMATRIX *world, float *max, float *min, int color );
		NewBoundingBoxView_1( &work->objs->objs[0].world, &work->objs->def->lx, &work->objs->def->ux, 0x00ff0000 );
		NewBoundingBoxView_1( &work->objs->objs[0].world, &work->objs->objs[0].model->lx, &work->objs->objs[0].model->ux, 0x0000ffff );
		NewBoundingBoxView_1( &work->objs->objs[1].world, &work->objs->objs[1].model->lx, &work->objs->objs[1].model->ux, 0x0000ffff );
		NewBoundingBoxView_1( &work->objs->objs[2].world, &work->objs->objs[2].model->lx, &work->objs->objs[2].model->ux, 0x0000ffff );
		NewBoundingBoxView_1( &work->objs->objs[3].world, &work->objs->objs[3].model->lx, &work->objs->objs[3].model->ux, 0x0000ffff );
		NewBoundingBoxView_1( &work->objs->objs[4].world, &work->objs->objs[4].model->lx, &work->objs->objs[4].model->ux, 0x0000ffff );
	}
#endif

}

static	void	Die( Work *work )
{
	int	i;

	work->prim = OK_FreePrim2( work->prim );
	if( work->objs != NULL ){
		DG_FreePreshade( work->objs );
		DG_DequeueObjs( work->objs );
		DG_FreeObjs( work->objs );
	}

	if(&work->target != NULL) GM_FreeTarget( &work->target ) ;
	for( i=0; i<MAX_TARGET; i++ ){
		if(&work->target_child[i] != NULL) GM_FreeTarget( &work->target_child[i] ) ;
	}
}

/*----------------------------------------------------------------*/
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR				*pos0 ;
	FVECTOR				*pos1 ;
	DG_PRIM2_UVRGB		*uvrgb0 ;
	DG_PRIM2_UVRGB		*uvrgb1 ;
	int		i, k ;

	DG_ConfigPrim2Tex( prim, tex );
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) ;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	pos0 = prim->pos[0] ;
	pos1 = prim->pos[1] ;
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		for ( k = 0 ; k < N_POLYS ; k++ ){
			pos0->vx = 0.0f;
			pos0->vy = 0.0f;
			pos0->vz = 0.0f;
			DG_COPY_VEC( pos1, pos0 );	pos0++;	pos1++;

			pos0->vx = 0.0f;
			pos0->vy = 0.0f;
			pos0->vz = 0.0f;
			DG_COPY_VEC( pos1, pos0 );	pos0++;	pos1++;

			pos0->vx = 0.0f;
			pos0->vy = 0.0f;
			pos0->vz = 0.0f;
			DG_COPY_VEC( pos1, pos0 );	pos0++;	pos1++;

			pos0->vx = 0.0f;
			pos0->vy = 0.0f;
			pos0->vz = 0.0f;
			DG_COPY_VEC( pos1, pos0 );	pos0++;	pos1++;
		}
	}

	uvrgb0 = prim->uvrgb[0] ;
	uvrgb1 = prim->uvrgb[1] ;
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		for ( k = 0 ; k < N_POLYS ; k++ ){
			uvrgb0->u = uvrgb1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb0->v = uvrgb1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb0->q = uvrgb1->q = 4096 ;
			uvrgb0->f = uvrgb1->f = 0x8fff ;
			uvrgb0->r = uvrgb1->r = COLOR_R ;
			uvrgb0->g = uvrgb1->g = COLOR_G ;
			uvrgb0->b = uvrgb1->b = COLOR_B ;
			uvrgb0->a = uvrgb1->a = COLOR_A ;
			uvrgb0++;
			uvrgb1++;

			uvrgb0->u = uvrgb1->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb0->v = uvrgb1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb0->q = uvrgb1->q = 4096 ;
			uvrgb0->f = uvrgb1->f = 0x8fff ;
			uvrgb0->r = uvrgb1->r = COLOR_R ;
			uvrgb0->g = uvrgb1->g = COLOR_G ;
			uvrgb0->b = uvrgb1->b = COLOR_B ;
			uvrgb0->a = uvrgb1->a = COLOR_A ;
			uvrgb0++;
			uvrgb1++;

			uvrgb0->u = uvrgb1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb0->v = uvrgb1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb0->q = uvrgb1->q = 4096 ;
			uvrgb0->f = uvrgb1->f = 0x0fff ;
			uvrgb0->r = uvrgb1->r = COLOR_R ;
			uvrgb0->g = uvrgb1->g = COLOR_G ;
			uvrgb0->b = uvrgb1->b = COLOR_B ;
			uvrgb0->a = uvrgb1->a = COLOR_A ;
			uvrgb0++;
			uvrgb1++;

			uvrgb0->u = uvrgb1->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb0->v = uvrgb1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb0->q = uvrgb1->q = 4096 ;
			uvrgb0->f = uvrgb1->f = 0x0fff ;
			uvrgb0->r = uvrgb1->r = COLOR_R ;
			uvrgb0->g = uvrgb1->g = COLOR_G ;
			uvrgb0->b = uvrgb1->b = COLOR_B ;
			uvrgb0->a = uvrgb1->a = COLOR_A ;
			uvrgb0++;
			uvrgb1++;
		}
	}

	return 1;
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work, int name, int where )
{
	char		*opt ;
	int			i;
	int			vec[3];
	DG_DEF		*def ;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	FVECTOR		pos;
	SVECTOR		rot;
	float	*intense;

	work->name  = name;
	work->where = where;



	intense    = work->intense;
	for(i=0; i<MAX_HUTA; i++){
		(*intense) = 0.0001f;
		intense++;
	}




	if ( ( opt = GCL_GetOption( 'p' ) ) != NULL ){
		GCL_GetIV( opt, vec ) ;
		pos.vx = (float)vec[0] ;
		pos.vy = (float)vec[1] ;
		pos.vz = (float)vec[2] ;
	}else{
		return -1;
	}

	if ( ( opt = GCL_GetOption( 'r' ) ) != NULL ){
		GCL_GetSV( opt, (short *)&rot ) ;
	}else{
		return -1;
	}

	work->open_close_flag = 0;	/* 開け放し */
	if ( GCL_GetOption( 'a' ) != NULL ){
		work->base_angle[0] = (float)GCL_GetNextInt() * TPI / 4096.0f;
		work->base_angle[1] = (float)GCL_GetNextInt() * TPI / 4096.0f;
		work->base_angle[2] = (float)GCL_GetNextInt() * TPI / 4096.0f;
		work->base_angle[3] = (float)GCL_GetNextInt() * TPI / 4096.0f;
	}else{
		work->open_close_flag = 1;	/* 箱閉じ */
		work->base_angle[0] = 3072.0f * TPI / 4096.0f;
		work->base_angle[1] = 3072.0f * TPI / 4096.0f;
		work->base_angle[2] = 3072.0f * TPI / 4096.0f;
		work->base_angle[3] = 3072.0f * TPI / 4096.0f;
	}


	def = (DG_DEF*)GV_GetCache( GV_CacheID( 3510735 /*"hako"*/, 'k' ) ) ;

//	work->objs = DG_MakeObjs( def, DG_FLAG_SHADE, 0 );
	work->objs = DG_MakeObjs( def, DG_FLAG_PAINT, 0 );

	{
		LIT_DEF	*lit_def ;
		lit_def = (LIT_DEF*)GV_GetCache(  GV_CacheID( 3950212 /*"w01d"*/, 'l' ) );
		DG_MakePreshade( work->objs, lit_def );
	}



	/* 蓋の接続ローテーション */
	work->objs->rots = work->frot;

	DG_QueueObjs( work->objs );
	GM_GroupObjs( work->objs, where );

	DG_SetPos2( &pos, &rot ) ;
	DG_GetPos( &work->world ) ;
	DG_PutObjs( work->objs ) ;
	DG_GetLightMatrix( &pos, work->lights );
	DG_SetLightMatrix( work->objs, work->lights );
	work->objs->flag &= ~DG_FLAG_INVISIBLE;

	work->shift[0].vx = 0.0f;
	work->shift[0].vy = 0.0f;
	work->shift[0].vz = 0.0f;
	work->rot[0].vx   = HUTA0_ROT_X;
	work->rot[0].vy   = HUTA0_ROT_Y;
	work->rot[0].vz   = HUTA0_ROT_Z;

	work->shift[1].vx = 0.0f;
	work->shift[1].vy = 0.0f;
	work->shift[1].vz = 0.0f;
	work->rot[1].vx   = HUTA1_ROT_X;
	work->rot[1].vy   = HUTA1_ROT_Y;
	work->rot[1].vz   = HUTA1_ROT_Z;

	work->shift[2].vx = 0.0f;
	work->shift[2].vy = 0.0f;
	work->shift[2].vz = 0.0f;
	work->rot[2].vx   = HUTA2_ROT_X;
	work->rot[2].vy   = HUTA2_ROT_Y;
	work->rot[2].vz   = HUTA2_ROT_Z;

	work->shift[3].vx = 0.0f;
	work->shift[3].vy = 0.0f;
	work->shift[3].vz = 0.0f;
	work->rot[3].vx   = HUTA3_ROT_X;
	work->rot[3].vy   = HUTA3_ROT_Y;
	work->rot[3].vz   = HUTA3_ROT_Z;

	work->shift[4].vx = HUTA0_SHIFT_X;
	work->shift[4].vy = HUTA0_SHIFT_Y;
	work->shift[4].vz = HUTA0_SHIFT_Z;
	work->rot[4].vx   = HUTA0_ROT_X;
	work->rot[4].vy   = HUTA0_ROT_Y;
	work->rot[4].vz   = HUTA0_ROT_Z;

	work->shift[5].vx = HUTA1_SHIFT_X;
	work->shift[5].vy = HUTA1_SHIFT_Y;
	work->shift[5].vz = HUTA1_SHIFT_Z;
	work->rot[5].vx   = HUTA1_ROT_X;
	work->rot[5].vy   = HUTA1_ROT_Y;
	work->rot[5].vz   = HUTA1_ROT_Z;

	work->shift[6].vx = HUTA2_SHIFT_X;
	work->shift[6].vy = HUTA2_SHIFT_Y;
	work->shift[6].vz = HUTA2_SHIFT_Z;
	work->rot[6].vx   = HUTA2_ROT_X;
	work->rot[6].vy   = HUTA2_ROT_Y;
	work->rot[6].vz   = HUTA2_ROT_Z;

	work->shift[7].vx = HUTA3_SHIFT_X;
	work->shift[7].vy = HUTA3_SHIFT_Y;
	work->shift[7].vz = HUTA3_SHIFT_Z;
	work->rot[7].vx   = HUTA3_ROT_X;
	work->rot[7].vy   = HUTA3_ROT_Y;
	work->rot[7].vz   = HUTA3_ROT_Z;

	work->shift[8].vx = SOKO_SHIFT_X;
	work->shift[8].vy = SOKO_SHIFT_Y;
	work->shift[8].vz = SOKO_SHIFT_Z;
	work->rot[8].vx   = SOKO_ROT_X;
	work->rot[8].vy   = SOKO_ROT_Y;
	work->rot[8].vz   = SOKO_ROT_Z;

	for( i=0; i<MAX_HUTA; i++ ){
		work->hit_flag[i] = 0;
	}
	InitTarget( work );



	work->dankon_now_num=0;
	work->dankon_max_num=0;

	tex = DG_GetTexture( 14845995 /*"dankon2_alp"*/ );

	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );

	work->hit_time=0;

	return 0 ;
}

	/*
		シナリオ呼び出し
	*/
void *NewHako_purapura( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
