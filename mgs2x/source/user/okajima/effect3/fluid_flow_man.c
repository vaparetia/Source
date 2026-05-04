//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	fluid_flow_man.c
	流体ライン、マネージャ

	2001/06/06 S.Okajima
	$Id: fluid_flow_man.c,v 1.1.1.3 2002/11/19 11:47:31 Yoshizawa1 Exp $
*/



#ifdef PSX2 ///
#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include <libutl.h>
#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"
#include	"../../mode/demo/eft_con.h"

#define STRIP_WIDTH_BASE (0.20f)
#define STRIP_WIDTH_SIN  (0.05f)


/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;
	int			name;

	EFTCONTROL	control;

	float		width ;
	float		height;

	int			on_flag;
	int			count;
	int			col;
} Work ;

/* ---------------------------------------------------------------- */
enum {
	REQ_OFF=0,
	REQ_ON,
	REQ_KILL,
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
			work->on_flag = 0;
			break;
		  case REQ_ON:
			work->on_flag = 1;
			break;
		  case REQ_KILL:
			GV_DestroyActor( work ) ;
			break;
		  default:
			  break;
		}
		msg--;
	}
}

/*----------------------------------------------------------------*/
static void Act( Work *work )
{
	FMATRIX	tmp_world;
	FVECTOR	base[16];
	FVECTOR	strip0[16];
	FVECTOR	strip1[16];
	FVECTOR	fvtemp;
	FVECTOR	fvtemp1;
	FVECTOR	dir_unit;
	FVECTOR	width_dir;
	FVECTOR	width_base;
	FVECTOR	*pfv0;
	FVECTOR	*pfv1;
	FVECTOR	*pfv2;
	float	radius;
	float	angle;
	float	ftemp;
	float	ftemp2;
	float	param;
	float	param1;
	float	width_angle0;
	float	width_angle1;
	float	angle_step0;
	float	angle_step1;
	float	inner;
	HZX_SEG		*seg ;
	HZX_FLR		flr[2] ;
	int			atr[2] ;
	int		i;
	int		map_id;
	int		h_flag;


	CheckMesgParam( work );

	if( !work->on_flag ){
//		GV_DestroyActor( work ) ;
		return;
	}

//	if( !(GV_PadData[ 0 ].status & PAD_X) ) return;

	work->count++;
	if( work->count%2!=0 ) return;


	DM_EftControlMatrix( &work->control, &tmp_world ) ;

	radius = DG_MAX( work->width, work->height ) * 1.2f;

	angle = TPI*rnd();
	fvtemp.vx = radius * sinf(angle);
	fvtemp.vy = radius * cosf(angle);

	width_dir.vz = 0.0f;
	if( fvtemp.vx < -work->width ){
		fvtemp.vy*=-work->width / fvtemp.vx;
		fvtemp.vx =-work->width;
		width_dir.vx = 0.0f;
		width_dir.vy = 1.0f;
		if( fvtemp.vy < -work->height ){
			fvtemp.vx*=-work->height / fvtemp.vy;
			fvtemp.vy =-work->height;
			width_dir.vx = 1.0f;
			width_dir.vy = 0.0f;
		}else if( fvtemp.vy > work->height ){
			fvtemp.vx*= work->height / fvtemp.vy;
			fvtemp.vy = work->height;
			width_dir.vx = 1.0f;
			width_dir.vy = 0.0f;
		}
	}else if( fvtemp.vx > work->width ){
		fvtemp.vy*= work->width / fvtemp.vx;
		fvtemp.vx = work->width;
		width_dir.vx = 0.0f;
		width_dir.vy = 1.0f;
		if( fvtemp.vy < -work->height ){
			fvtemp.vx*=-work->height / fvtemp.vy;
			fvtemp.vy =-work->height;
			width_dir.vx = 1.0f;
			width_dir.vy = 0.0f;
		}else if( fvtemp.vy > work->height ){
			fvtemp.vx*= work->height / fvtemp.vy;
			fvtemp.vy = work->height;
			width_dir.vx = 1.0f;
			width_dir.vy = 0.0f;
		}
	}else if( fvtemp.vy < -work->height ){
		fvtemp.vx*=-work->height / fvtemp.vy;
		fvtemp.vy =-work->height;
		width_dir.vx = 1.0f;
		width_dir.vy = 0.0f;
	}else if( fvtemp.vy > work->height ){
		fvtemp.vx*= work->height / fvtemp.vy;
		fvtemp.vy = work->height;
		width_dir.vx = 1.0f;
		width_dir.vy = 0.0f;
	}
	fvtemp.vz = 0.0f;
	_sceVu0Normalize( &dir_unit, &fvtemp );
	_sceVu0ScaleVector( &fvtemp, &fvtemp, 0.7f+0.2f*frnd() );

	param = 1.00f + 1.0f*rnd();	// ２次曲線の幅比
	param1= 2.00f + 2.0f*rnd();	// ２次曲線の到達距離比

	width_angle0 = TPI*rnd();
	width_angle1 = width_angle0 + PI + PI*0.25f*frnd();
	angle_step0  = TPI*(0.02f + 0.2f*rnd());
	angle_step1  = TPI*(0.03f + 0.2f*rnd());
	for( i=0; i<16; i++ ){
		ftemp = (float)i / 16.0f - 0.25f;
		ftemp2= ftemp*ftemp*radius*param1;

		base[i].vx = fvtemp.vx + dir_unit.vx*ftemp2;
		base[i].vy = fvtemp.vy + dir_unit.vy*ftemp2;
		base[i].vz = ftemp*radius*param;
	}

	DG_SetPos( &tmp_world );
	DG_PutVector( base, base, 16 );
	DG_RotVector( &width_dir, &width_dir, 1 );

	map_id=GM_GetHzxGroupID( GM_CurrentStageMap );


	pfv0 = base;
	pfv1 = base;
	pfv1++;
	for( i=0; i<15; i++ ){
		h_flag = HZX_OnlineHazardCheck( /* ハザードチェック：床も見る */
			map_id,
			pfv0,
			pfv1,
			HZX_CHK_FIX,
			HZX_SEG_NO_BULLET | HZX_SEG_RECOIL_TYPE,
			HZX_FLOOR_NO_BULLET | HZX_FLOOR_RECOIL_TYPE );
		if( h_flag&1 ){	// 壁
			HZX_GetOnlinePoint( &fvtemp );
			_sceVu0SubVector( &fvtemp, &fvtemp, pfv1 );	// 壁の向こう側から交点まで


			HZX_GetOnlineHazard( flr, atr ) ;
			seg=(HZX_SEG *)flr;
			fvtemp1.vx = seg->p2.z - seg->p1.z ;
			fvtemp1.vy = 0.0f ;
			fvtemp1.vz = seg->p1.x - seg->p2.x ;

			_sceVu0Normalize( &fvtemp1, &fvtemp1 );	// 壁に垂直な単位ベクトル
			inner = _sceVu0InnerProduct( &fvtemp1, &fvtemp );
			if( inner > 0.0f ){
				inner+= 1.0f;
			}else{
				inner-= 1.0f;
			}
			_sceVu0ScaleVector( &fvtemp1, &fvtemp1, inner );
			_sceVu0AddVector( pfv1, pfv1, &fvtemp1 );

		}else if( h_flag&2 ){ // 床
			HZX_GetOnlinePoint( &fvtemp );
			if( fvtemp.vy < pfv0->vy ){
				pfv1->vy = fvtemp.vy + 1.0f;
			}else{
				pfv1->vy = fvtemp.vy - 1.0f;
			}
		}

//		AN_Test_Eye2( pfv1, 2 );

		pfv0++;
		pfv1++;
	}

	_sceVu0ScaleVector( &width_base, &width_dir, radius*STRIP_WIDTH_BASE );
	_sceVu0ScaleVector( &width_dir,  &width_dir, radius*STRIP_WIDTH_SIN );

	width_angle0 = TPI*rnd();
	width_angle1 = width_angle0 + PI + PI*0.25f*frnd();
	angle_step0  = TPI*(0.02f + 0.2f*rnd());
	angle_step1  = TPI*(0.03f + 0.2f*rnd());
	pfv0 = strip0;
	pfv1 = strip1;
	pfv2 = base;
	for( i=0; i<16; i++ ){
		ftemp = sinf(width_angle0);
		pfv0->vx = pfv2->vx + width_base.vx + width_dir.vx*ftemp;
		pfv0->vy = pfv2->vy + width_base.vy + width_dir.vy*ftemp;
		pfv0->vz = pfv2->vz + width_base.vz + width_dir.vz*ftemp;

		ftemp = sinf(width_angle1);
		pfv1->vx = pfv2->vx - width_base.vx + width_dir.vx*ftemp;
		pfv1->vy = pfv2->vy - width_base.vy + width_dir.vy*ftemp;
		pfv1->vz = pfv2->vz - width_base.vz + width_dir.vz*ftemp;

		_sceVu0ScaleVector( &width_base, &width_base, 1.1f );

		width_angle0+= angle_step0;
		width_angle1+= angle_step1;
//if(GV_Time%300==0)printf("%f %f %f\n",strip[i].vx,strip[i].vy,strip[i].vz);

		pfv0++;
		pfv1++;
		pfv2++;
	}


/*
{
	extern void *FluidFlowParts( FVECTOR *edge0, FVECTOR *edge1, int num, int life, int col );
	GV_SetActorChild( work,
		FluidFlowParts( strip0, strip1, 16, 300, work->col )
	);
}
*/



}

static void Die( Work *work )
{
	if( work->name != 0 ){
		DM_EftControlDelList( &work->control );
	}
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work )
{
	work->on_flag = 1;
	work->count = 0;

	return 0 ;
}


void *NewFluidFlowMan( int name, float width, float height, int col )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name = name;
		DM_EftControlAddList( name, &work->control );

		work->width  = width;
		work->height = height;
		work->col = col;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

