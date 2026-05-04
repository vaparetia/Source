//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	vmp_etc.c
	ヴァンプ　いろいろ
	2001/03/02

	$Id: vmp_etc.c,v 1.1.1.3 2002/11/19 11:48:56 Yoshizawa1 Exp $
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

#include 	"libutl.h"
#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"../../shibata/t_headmark/headmark.h"
#include	"../../kano/attachment/attachment_called.h"
#include	"../../takabe/other/puppetik.h"
#include	"../../kira/face/face.h"
#include	"../util/ts_util.h"

#include	"vamp.h"
#include	"vmp_inline.h"

//体のアジャスト指定
void SetAjustToPlayer( Work *work )
{
	int		rot_y,rot_x,check;
	CONTROL *ctrl = &work->control;
	SVECTOR to1 = { 0, 0, 0, 0 };
	SVECTOR to2 = { 0, 0, 0, 0 };
	SVECTOR to11 = { 0, 0, 0, 0 };
	SVECTOR to12 = { 0, 0, 0, 0 };

	rot_y = (work->think_dir - ctrl->rot.vy)&0x0fff;
	if( rot_y > 2048 ) rot_y -= 4096;
	check = (rot_y<0)?-rot_y:rot_y;
	//VMP_PRINTF("rot_y = %d check = %d ",rot_y,check);

	if( 3 < check && check < 1024-128 ){
		//修正入れて

		if( rot_y < 0 ){
			to1.vy = (rot_y + 128)/3;
			to2.vy = (rot_y + 128)/3;
			to11.vy = (rot_y + 128)/3;
			to12.vy = (rot_y + 128);
		}else{
			to1.vy = (rot_y - 128)/3;
			to2.vy = (rot_y - 128)/3;
			to11.vy = (rot_y - 128)/3;
			to12.vy = (rot_y - 128);
		}
		//VMP_PRINTF("回れー\n");

		//頭のみｘ回転
		rot_x = work->adjust_rot_x - ctrl->rot.vx;
		if( rot_x > 2048 ) rot_x -= 4096;
		check = (rot_x<0)?-rot_x:rot_x;

		//VMP_PRINTF("rot_x = %d rot_x = %d check = %d\n",work->adjust_rot_x,rot_x,check);
		if( 0 < check && check < 512-64 ){
			//to11.vx = rot_x/2;
			to12.vx = rot_x;
		}
	}
	VMP_SetAdjustWork( work, &to1, 1 );
	VMP_SetAdjustWork( work, &to2, 2 );
	VMP_SetAdjustWork( work, &to11, 11 );
	VMP_SetAdjustWork( work, &to12, 12 );
}

void ClearAjustToPlayer( Work *work )
{
	static SVECTOR rot_zero = { 0, 0, 0, 0 };
	VMP_SetAdjustWork( work, &rot_zero, 1 );
	VMP_SetAdjustWork( work, &rot_zero, 2 );
	VMP_SetAdjustWork( work, &rot_zero, 11 );
	VMP_SetAdjustWork( work, &rot_zero, 12 );
}

void GetInitialVelocity( FVECTOR *step, FVECTOR *from, FVECTOR *to, float time )
{
//#define		DIFF_HIGHT	(-2000.0f)
	FVECTOR		fvtemp;

	_sceVu0SubVector( &fvtemp, to, from );
	_sceVu0ScaleVector( &fvtemp, &fvtemp, 1.0f/time );

	fvtemp.vy = 0.0f;//2.0f*DIFF_HIGHT/(time*time);//DIFF_HIGHT/time - P_GRAVITY*time/2.0f;
//	fvtemp.vy = bp_sqrtf( (-2.0f*P_GRAVITY*4000.0f) ); //BP_MATH - emulate PS2 sqrtf
//	VMP_PRINTF("vx %f: vy %f: vz %f:\n",fvtemp.vx,fvtemp.vy,fvtemp.vz);
	fvtemp.vw = 0.0f;
	DG_COPY_VEC( step, &fvtemp );
}

int GetOutRandNum( int from, int to, int out )
{
	int rand_list[32];
	int i,cnt = 0;

	for( i = from; i <= to; i++ ){
		if( i == out ) continue;
		rand_list[cnt++] = i;
	}

	if( cnt == 0 ) return from;

	return rand_list[(irnd()>>12)%cnt];
}


#if 0
//取り合えずバックアップ
//旧ダメージチェック
static int VmpCheckDamage( Work *work )
{
	TARGET	*def;
//	CAPTURE_TARGET	*cap;
	long64	weapon;
	int		dam_child_num;

	def = &work->target;
	weapon = 0 ;


#if 1
	if( TARGET_POWER & def->damaged ){
		weapon = def->weapon_type;

		VMP_PRINTF("damage weapon type [%016lx]  \n",weapon);
		
		VMP_DamageFlagClear( work );
		/* このフレームではダメージを受けない */
		//NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;

		if( weapon & WP_PUNCHR ){
			VMP_SetActCallMotion( work, VMP_MOT_GUNDAM, VMP_ActDamege );
		}else if( weapon & WP_PUNCHL ){
			VMP_SetActCallMotion( work, VMP_MOT_GUNDAM, VMP_ActDamege );
		}else if( weapon & WP_KICK ){
		}else if( weapon & WP_M92 ){
			VMP_SetActCallMotion( work, VMP_MOT_THOROW, VMP_ActKnifeThrowSingleLeft );
		}
		return 1 ;
	}
#endif
	def = work->def_child;
	dam_child_num = VMP_ChildTargetCheck( def );
	if( dam_child_num >= 0 ){
		def += dam_child_num;
		weapon = def->weapon_type;
		VMP_PRINTF("[%2d] damage weapon type [%016lx]  \n",dam_child_num,weapon);
		
		VMP_DamageFlagClear( work );

		if( weapon & WP_PUNCHR ){
			VMP_SetActCallMotion( work, VMP_MOT_GUNDAM, VMP_ActDamege );
		}else if( weapon & WP_PUNCHL ){
			VMP_SetActCallMotion( work, VMP_MOT_GUNDAM, VMP_ActDamege );
		}else if( weapon & WP_KICK ){
		}else if( weapon & WP_M92 ){
			VMP_SetActCallMotion( work, VMP_MOT_THOROW, VMP_ActKnifeThrowSingleLeft );
		}
		return 1 ;
	}

	return 0 ;
}
#endif
