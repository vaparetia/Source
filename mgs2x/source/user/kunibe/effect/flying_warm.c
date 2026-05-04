//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  flying_warm.c
  電灯に群がる虫
  2001/05/21 Yuuta Kunibe	
  $Id: flying_warm.c,v 1.1.1.3 2002/11/19 11:44:39 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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
#include	"../../okajima/etc/ok_util.h"


#define COLOR			(255)//(128)
#define ALPHA			(64)

#define N_PRIMS			(1)
#define N_VERTS 		(32)
#define	UNIT_NUM		(4)

#define SCR_POS 		(SCRPAD_ADDR)
#define SCR_UVS 		(SCRPAD_ADDR+0x2000)

#define	WARM_CODE		( GV_StrCode("fly_cm") )
#define SIZE			(5)

extern int BP_AdjustTick2(int);
extern float BP_AdjustTick3(float);
#define SPEED			   (BP_AdjustTick3(40.0f))
#define	ROT_PARAM1		(BP_AdjustTick2(20))
#define	ROT_PARAM2		(BP_AdjustTick2(30))

#define	SPRAY_LENGTH		(3000.0f)

#define	SCALE			(0.1f)



// mode
enum {
    WARM_MODE_NORMAL,
    WARM_MODE_CHASE,
    WARM_MODE_CHASE_2,
    WARM_MODE_INVISIBLE,
    WARM_MODE_KILL,
};

// phase
enum {
    WARM_PHASE_FLY,
    WARM_PHASE_DROP,
    WARM_PHASE_DEATH,
};

// message
enum {
    WARM_MSG_NORMAL,
    WARM_MSG_CHASE,
    WARM_MSG_CHASE_2,
    WARM_MSG_INVISIBLE,
    WARM_MSG_KILL,
    WARM_MSG_WALL_ON,
    WARM_MSG_WALL_OFF,
};




typedef struct {

    FMATRIX	mat;
    SVECTOR	rot;
    float	speed;

    int		phase;
    int		life;
    int		b_flag;
    int		count;        

} Warm;



typedef	struct {

    GV_ACT_EX	actor;
    int		name;
    int		code[2];

    DG_COMDL	*comdl;
    int		n_warms;

    FVECTOR	light_pos;

    FVECTOR	bound[2];
    int		bound_flag;

    GV_MSG	*msg;
    int		mode;

    int		ctrl_name;
    CONTROL	*control;

    int		se_count;
    int		cnt;

    Warm	warm[0];

} Work;


static inline void _ScaleMatrix( FMATRIX *out, FMATRIX *world, float r )
{
	FVECTOR temp = { r, r, r, 0.0f };

// yano /*未検証*/
#ifdef BP_PSX2_ASM
	asm ("
	lqc2				vf8,0x00(%2)
	lqc2				vf4,0x00(%1)
	lqc2				vf5,0x10(%1)
	lqc2				vf6,0x20(%1)
	vmul.xyzw			vf4, vf4,vf8
	vmul.xyzw			vf5, vf5,vf8
	vmul.xyzw			vf6, vf6,vf8
	sqc2				vf4,0x00(%0)
	sqc2				vf5,0x10(%0)
	sqc2				vf6,0x20(%0)
	": : "r"(out), "r"(world), "r"(&temp) );//:"memory" );
#else
	_sceVu0ScaleVectorXYZ( (FVECTOR*)out->m[0], (FVECTOR*)world->m[0], r );
	_sceVu0ScaleVectorXYZ( (FVECTOR*)out->m[1], (FVECTOR*)world->m[1], r );
	_sceVu0ScaleVectorXYZ( (FVECTOR*)out->m[2], (FVECTOR*)world->m[2], r );
	_sceVu0CopyVector( (FVECTOR*)out->m[3], (FVECTOR*)world->m[3] );
#endif
}


/*-------- CheckControl : コントロールチェック関数 --------*/
static int CheckControl( int name )
{
	int		i ;
	CONTROL	*control, **w_list ;

	w_list = GM_WhereList ;
	for ( i = GM_N_WhereList ; i > 0 ; w_list++, i-- ){
		control = *w_list ;
		if ( control->name == name ) return (0) ;
	}
	return (1);
}


/*-------- SearchControl : コントロール検索関数 --------*/
static CONTROL* SearchControl( int name )
{
	int		i ;
	CONTROL	*control, **w_list ;

	w_list = GM_WhereList ;
	for ( i = GM_N_WhereList ; i > 0 ; w_list++, i-- ){
		control = *w_list ;
		if ( control->name != name ) continue ;
		return ( control ) ;
	}
	return ( NULL );
}

/*-------- WarmReceiveMessage : メッセージ取得関数 --------*/
static void WarmRecieveMessage( Work* work )
{
    
    GV_MSG*	msg;
    int 	n_msg;


    if ( work->name == 0 ) {
	return;
    }

    /* メッセージ取得 */
    n_msg = GV_ReceiveMessage( work->name, &work->msg ); 

    msg = work->msg;

    /* メッセージ反映 */
    while ( n_msg-- > 0 ){

	switch ( msg->message[ 0 ] ) {
	case WARM_MSG_NORMAL:		
	    //printf("flying_warm.c : receive_message : normal\n");
	    work->mode = WARM_MODE_NORMAL;
	    break;
	case WARM_MSG_CHASE:
	    //printf("flying_warm.c : receive_message : chase\n");
	    work->ctrl_name = msg->message[1];
	    if ( ( work->control = SearchControl( work->ctrl_name ) ) != NULL ) {
		work->mode = WARM_MODE_CHASE;
	    }
	    else {
		//printf("flying_warm.c : search_control_error!!!\n");
	    }
	    break;
	case WARM_MSG_CHASE_2:
	    //printf("flying_warm.c : receive_message : chase\n");
	    work->ctrl_name = msg->message[1];
	    if ( ( work->control = SearchControl( work->ctrl_name ) ) != NULL ) {
		work->mode = WARM_MODE_CHASE_2;
	    }
	    else {
		//printf("flying_warm.c : search_control_error!!!\n");
	    }
	    break;
	case WARM_MSG_INVISIBLE:		
	    //printf("flying_warm.c : receive_message : invisible\n");
	    work->mode = WARM_MODE_INVISIBLE;
	    break;
	case WARM_MSG_KILL:
	    //printf("flying_warm.c : receive_message : kill\n");
	    work->mode = WARM_MODE_KILL;
	    break;
	case WARM_MSG_WALL_ON:
	    work->bound_flag = 1;
	    break;
	case WARM_MSG_WALL_OFF:
	    work->bound_flag = 0;
	    break;
	}
	msg++;
    }

}




/*-------- アクト関数 --------*/
static void Act( Work *work )
{

    int 		i;
    DG_COMDL_POS 	*comdl_pos;
    Warm		*warm;

    FVECTOR		eye_to_light;
    FVECTOR		vectmp;
    SVECTOR		rot;
    float		inner_x, inner_y, inner_z;

    FMATRIX		*root;
    static FVECTOR	NozzlePoint = { 13.0f, -323.0f, 61.0f, 1.0f };
    FVECTOR		from;

    OBJECT		*body;

    float		scale;
    
    FVECTOR		se_pos;
    FVECTOR		se_pos_max;
    FVECTOR		se_pos_min;
    int			se_flag;
    int			n_live;


    DG_COPY_VEC( &se_pos_max, &DG_ZeroVector );
    DG_COPY_VEC( &se_pos_min, &DG_ZeroVector );
    n_live  = 0;
    se_flag = 0;

    /* 主観時はスケール２倍に */
    if ( GM_PlayerStatus & PLAYER_WATCH ) {
	scale = SCALE;
    }
    else {
	scale = SCALE*2.0f;
    }



    warm    = work->warm;

    work->comdl->group_id = GM_GetDGGroupID( GM_CurrentStageMap );
    comdl_pos = work->comdl->pos;


    /* メッセージ取得 */
    WarmRecieveMessage( work );


    /* 追従コントロールチェック */
    if ( work->mode == WARM_MODE_CHASE || work->mode == WARM_MODE_CHASE_2 ) {
	if ( CheckControl( work->ctrl_name ) ) {
	    work->mode = WARM_MODE_NORMAL;
	    body = NULL;
	}
	else {
	    body = work->control->object;
	}
    }
    else {
	body = NULL;
    }    


    //printf("mode : %d\n", work->mode );

    /* デバッグ用chase_message */
    /*{
	GV_MSG		msg;

	if ( GV_PadData[1].press & PAD_X ) {
	    if ( work->mode == WARM_MODE_NORMAL ) {
		work->code[0] = WARM_MSG_CHASE;
		work->code[1] = GV_StrCode( "スネーク" );

		msg.address = work->name;
		msg.message = work->code;
		msg.message_len = 2;
		GV_SendMessage( &msg );

		printf("send_message : chase\n");
	    }
	    else {
		work->code[0] = WARM_MSG_NORMAL;

		msg.address = work->name;
		msg.message = work->code;
		msg.message_len = 1;
		GV_SendMessage( &msg );

		printf("send_message : normal\n");
	    }
	}

	// デバッグ用invisible_message
	if ( GV_PadData[1].press & PAD_Y ) {
	    if ( work->mode == WARM_MODE_INVISIBLE ) {
		work->code[0] = WARM_MSG_NORMAL;

		msg.address = work->name;
		msg.message = work->code;
		msg.message_len = 1;
		GV_SendMessage( &msg );

		printf("send_message : normal\n");
	    
	    }
	    else {
		work->code[0] = WARM_MSG_INVISIBLE;

		msg.address = work->name;
		msg.message = work->code;
		msg.message_len = 1;
		GV_SendMessage( &msg );

		printf("send_message : invisible\n");
	    }
	}

	// デバッグ用kill_message
	if ( GV_PadData[1].press & PAD_B ) {
	    work->code[0] = WARM_MSG_KILL;

	    msg.address = work->name;
	    msg.message = work->code;
	    msg.message_len = 1;
	    GV_SendMessage( &msg );

	    printf("send_message : kill\n");
	}

	// デバッグ用壁判定オフ
	if ( GV_PadData[1].press & PAD_A ) {
	    if ( work->bound_flag == 1 ) {
		work->code[0] = WARM_MSG_WALL_OFF;

		msg.address = work->name;
		msg.message = work->code;
		msg.message_len = 1;
		GV_SendMessage( &msg );

		printf("send_message : wall_off\n");
	    }
	    else {
		work->code[0] = WARM_MSG_WALL_ON;

		msg.address = work->name;
		msg.message = work->code;
		msg.message_len = 1;
		GV_SendMessage( &msg );

		printf("send_message : wall_on\n");
	    }
	}
    }*/


#if 0
    NewBoundingBoxView( &work->bound[0], &work->bound[1], 128, 32, 32 );
#endif	



    /* 冷凍スプレーマズル座標取得 */
    if ( GM_WeaponFire == WP_ColdSpray && GM_PlayerSubWeaponBody != NULL ) {
	root = GM_PlayerSubWeaponBody->objs->root;
	DG_SetPos( root );
	DG_PutVector( &NozzlePoint, &from, 1 );
    }
    else {
	root = NULL;
    }



    if ( work->mode == WARM_MODE_INVISIBLE ) {

	work->comdl->flag |= DG_COMDL_INVISIBLE;

    }
    else {

	work->comdl->flag &= ~DG_COMDL_INVISIBLE;
    
	if ( GM_CheckPlayerStatus( PLAYER_WATCH ) ) {
	    _sceVu0SubVector( &eye_to_light, &work->light_pos, (FVECTOR *)DG_Chanls[0].eye.m[3] );
	    _sceVu0Normalize( &eye_to_light, &eye_to_light );
	}

	for ( i = 0 ; i < work->n_warms ; i++ ) {

	    switch ( warm->phase ) {

	    case WARM_PHASE_FLY:		/* 飛行 */
		DG_SetPos( &warm->mat );
		DG_RotatePos( &warm->rot );
		DG_GetPos( &warm->mat );
	
		_sceVu0ScaleVector( &vectmp, (FVECTOR *)warm->mat.m[2], warm->speed );
		_sceVu0AddVector( (FVECTOR *)warm->mat.m[3], (FVECTOR *)warm->mat.m[3], &vectmp );

		/* 壁当たり */
		if ( work->bound_flag ) {
		    if ( warm->b_flag ) {
			if ( ( warm->mat.m[3][0] <= work->bound[0].vx || warm->mat.m[3][0] >= work->bound[1].vx )
			     || ( warm->mat.m[3][1] <= work->bound[0].vy || warm->mat.m[3][1] >= work->bound[1].vy )
			     || ( warm->mat.m[3][2] <= work->bound[0].vz || warm->mat.m[3][2] >= work->bound[1].vz ) ) {
			    DG_SetPos( &warm->mat );
			    rot.vx = 2048;
			    rot.vy = 0;
			    rot.vz = 0;
			    DG_RotatePos( &rot );
			    DG_GetPos( &warm->mat );
		    
			    _sceVu0ScaleVector( &vectmp, (FVECTOR *)warm->mat.m[2], warm->speed );
			    _sceVu0AddVector( (FVECTOR *)warm->mat.m[3], (FVECTOR *)warm->mat.m[3], &vectmp );
			}
		    }
		    else {
			if ( ( warm->mat.m[3][0] > work->bound[0].vx && warm->mat.m[3][0] < work->bound[1].vx )
			     && ( warm->mat.m[3][1] > work->bound[0].vy && warm->mat.m[3][1] < work->bound[1].vy )
			     && ( warm->mat.m[3][2] > work->bound[0].vz && warm->mat.m[3][2] < work->bound[1].vz ) ) {
			    warm->b_flag = 1;
			}
		    }
		}
		else {
		    warm->b_flag = 0;
		}

	    
		_ScaleMatrix( &comdl_pos->world, &warm->mat, scale );
		if ( work->cnt%2 ) {
		    _sceVu0ScaleVector( (FVECTOR *)comdl_pos->world.m[1], (FVECTOR *)comdl_pos->world.m[1], -1.0f );
		}
		DG_COPY_VEC( (FVECTOR *)comdl_pos->world.m[3], (FVECTOR *)warm->mat.m[3] );
	
		/* 主観で逆光による影になる */
		/*if ( GM_CheckPlayerStatus( PLAYER_WATCH ) ) {
		    _sceVu0SubVector( &vectmp, pos, (FVECTOR *)DG_Chanls[0].eye.m[3] );
		    _sceVu0Normalize( &vectmp, &vectmp );
		    if ( _sceVu0InnerProduct( &vectmp, &eye_to_light ) > 0.985f ) {
			uvrgbwh->r = 0;
			uvrgbwh->g = 0;
			uvrgbwh->b = 0;
		    }
		    else {
			uvrgbwh->r = COLOR;
			uvrgbwh->g = COLOR;
			uvrgbwh->b = COLOR;
		    }
		}
		else {
		    uvrgbwh->r = COLOR;
		    uvrgbwh->g = COLOR;
		    uvrgbwh->b = COLOR;
		}*/


		/* 軌道変更 */
		if ( --warm->count <= 0 ) {

		    /* 目標点設定 */
		    if ( work->mode == WARM_MODE_NORMAL ) {
			_sceVu0SubVector( &vectmp, &work->light_pos, (FVECTOR *)warm->mat.m[3] );
		    }
		    else if ( work->mode == WARM_MODE_CHASE ) {
			switch (irnd()%4) {
			case 0:
			    _sceVu0SubVector( &vectmp, (FVECTOR *)body->objs->objs[0].world.m[3], (FVECTOR *)warm->mat.m[3] );
			    break;
			case 1:
			    _sceVu0SubVector( &vectmp, (FVECTOR *)body->objs->objs[1].world.m[3], (FVECTOR *)warm->mat.m[3] );
			    break;
			case 2:
			    _sceVu0SubVector( &vectmp, (FVECTOR *)body->objs->objs[2].world.m[3], (FVECTOR *)warm->mat.m[3] );
			    break;
			case 3:
			    _sceVu0SubVector( &vectmp, (FVECTOR *)body->objs->objs[12].world.m[3], (FVECTOR *)warm->mat.m[3] );
			    break;
			}
		    }
		    else if ( work->mode == WARM_MODE_CHASE_2 ) {
			_sceVu0SubVector( &vectmp, (FVECTOR *)&work->control->mov, (FVECTOR *)warm->mat.m[3] );
		    }

		    inner_x = _sceVu0InnerProduct( &vectmp, (FVECTOR *)warm->mat.m[0] );
		    inner_y = _sceVu0InnerProduct( &vectmp, (FVECTOR *)warm->mat.m[1] );
		    inner_z = _sceVu0InnerProduct( &vectmp, (FVECTOR *)warm->mat.m[2] );

		    if ( inner_z > 0.f ) {
		    
			/*if ( inner_x > 0.f ) {
			    warm->rot.vy = 20 + irnd()%20;
			}
			else {
			    warm->rot.vy =-20 - irnd()%20;
			}*/
			if ( inner_x > 0.f ) {
			    warm->rot.vy = ROT_PARAM1 + irnd()%ROT_PARAM1;
			}
			else {
			    warm->rot.vy =-ROT_PARAM1 - irnd()%ROT_PARAM1;
			}

			/*if ( inner_y > 0.f ) {
			    warm->rot.vx =-20 - irnd()%20;
			}
			else {
			    warm->rot.vx = 20 + irnd()%20;
			}*/
			if ( inner_y > 0.f ) {
			    warm->rot.vx =-ROT_PARAM1 - irnd()%ROT_PARAM1;
			}
			else {
			    warm->rot.vx = ROT_PARAM1 + irnd()%ROT_PARAM1;
			}

			warm->rot.vz = 0;

		    }
		    else {

			/*if ( inner_x > 0.f ) {
			    warm->rot.vy = 30 + irnd()%30;
			}
			else {
			    warm->rot.vy =-30 - irnd()%30;
			}*/
			if ( inner_x > 0.f ) {
			    warm->rot.vy = ROT_PARAM2 + irnd()%ROT_PARAM2;
			}
			else {
			    warm->rot.vy =-ROT_PARAM2 - irnd()%ROT_PARAM2;
			}

			/*if ( inner_y > 0.f ) {
			    warm->rot.vx =-30 - irnd()%30;
			}
			else {
			    warm->rot.vx = 30 + irnd()%30;
			}*/
			if ( inner_y > 0.f ) {
			    warm->rot.vx =-ROT_PARAM2 - irnd()%ROT_PARAM2;
			}
			else {
			    warm->rot.vx = ROT_PARAM2 + irnd()%ROT_PARAM2;
			}

			warm->rot.vz = 0;

		    }

		    warm->rot.vx = warm->rot.vx;
		    warm->rot.vy = warm->rot.vy;
		    warm->rot.vz = warm->rot.vz;

		    if ( work->mode == WARM_MODE_NORMAL ) {
			warm->count = DIRECT_TICK( 30 + irnd()%30 );
		    }
		    else {
			warm->count = DIRECT_TICK( 15 + irnd()%15 );
		    }

		}


		/* スプレーとの当たり判定 */
		if ( root != NULL ) {
		    _sceVu0SubVector( &vectmp, (FVECTOR *)warm->mat.m[3], &from );

		    if ( _sceVu0InnerProduct( &vectmp, &vectmp ) < SPRAY_LENGTH * SPRAY_LENGTH ) {
			_sceVu0Normalize( &vectmp, &vectmp );
			if ( _sceVu0InnerProduct( &vectmp, (FVECTOR *)root->m[1] ) < -0.90f ) {
			    if ( --warm->life <= 0 ) {
				warm->phase = WARM_PHASE_DROP;
			    }
			}
		    }
		}

		
		if ( work->se_count <= 0 ) {

		    n_live++;

		    if ( se_flag ) {

			// max
			if ( se_pos_max.vx < comdl_pos->world.m[3][0] ) {
			    se_pos_max.vx = comdl_pos->world.m[3][0];
			}
			if ( se_pos_max.vy < comdl_pos->world.m[3][1] ) {
			    se_pos_max.vy = comdl_pos->world.m[3][1];
			}
			if ( se_pos_max.vz < comdl_pos->world.m[3][2] ) {
			    se_pos_max.vz = comdl_pos->world.m[3][2];
			}

			// min
			if ( se_pos_min.vx > comdl_pos->world.m[3][0] ) {
			    se_pos_min.vx = comdl_pos->world.m[3][0];
			}
			if ( se_pos_min.vx > comdl_pos->world.m[3][1] ) {
			    se_pos_min.vy = comdl_pos->world.m[3][1];
			}
			if ( se_pos_min.vx > comdl_pos->world.m[3][2] ) {
			    se_pos_min.vz = comdl_pos->world.m[3][2];
			}

		    }
		    else {
			se_flag = 1;
			DG_COPY_VEC( &se_pos_max, (FVECTOR *)comdl_pos->world.m[3] );
			DG_COPY_VEC( &se_pos_min, (FVECTOR *)comdl_pos->world.m[3] );
		    }

		}
		break;

	    case WARM_PHASE_DROP:		/* 落下 */
		warm->speed *= 0.90f;
		_sceVu0ScaleVector( &vectmp, (FVECTOR *)warm->mat.m[2], warm->speed );
		_sceVu0AddVector( (FVECTOR *)warm->mat.m[3], (FVECTOR *)warm->mat.m[3], &vectmp );
		warm->mat.m[3][0] += frnd()*20.f;
		warm->mat.m[3][1] -= 35.f;
		warm->mat.m[3][2] += frnd()*20.f;

		if ( warm->mat.m[3][1] < work->bound[0].vy + 10.0f ) {
		    warm->mat.m[3][1] = work->bound[0].vy + 10.0f;
		    warm->phase = WARM_PHASE_DEATH;
		}
	    
		DG_COPY_VEC( (FVECTOR *)warm->mat.m[3], (FVECTOR *)warm->mat.m[3] );
		_ScaleMatrix( &comdl_pos->world, &warm->mat, scale );
		DG_COPY_VEC( (FVECTOR *)comdl_pos->world.m[3], (FVECTOR *)warm->mat.m[3] );

		break;

	    case WARM_PHASE_DEATH:		/* 死亡 */
		_ScaleMatrix( &comdl_pos->world, &warm->mat, scale );
		_sceVu0ScaleVector( (FVECTOR *)comdl_pos->world.m[2], (FVECTOR *)comdl_pos->world.m[2], -1.0f );
		DG_COPY_VEC( (FVECTOR *)comdl_pos->world.m[3], (FVECTOR *)warm->mat.m[3] );
		break;

	    }
		
	    comdl_pos++;
	    warm++;

	}

	/* ＳＥ呼び出し */
	if ( work->se_count <= 0 ) {
	    if ( n_live > work->n_warms * 4 / 10 ) {
		/* maxとminの中間点から音発生 */
		_sceVu0AddVector( &se_pos, &se_pos_max, &se_pos_min );
		_sceVu0ScaleVector( &se_pos, &se_pos, 0.5f );

		if ( GM_CurrentStageMap & GM_GetMapIDfromPos( 0, &se_pos ) ) {
#if 0
		    AN_Test_Eye2( &se_pos, 500.f );
#endif			
		    GM_SeSetMode( SD_A_FLYFLY01, &se_pos, GM_SEMODE_NORMAL );
		}
		work->se_count = DIRECT_TICK( 6+irnd()%10 );
	    }
	}
	else {
	    work->se_count--;
	}

    }

    work->cnt++;
    
    if ( work->mode == WARM_MODE_KILL ) {
	GV_DestroyActor( work );
    }
    
}


static void Die(Work *work )
{
    /* メモリ解放 */
    if ( work->comdl ) {
	DG_DequeueComdlObjs( work->comdl );
	DG_FreeComdl( work->comdl );
    }
}


static int InitComdl( Work *work )
{

	int			i;
	DG_DEF			*def;
	DG_COMDL		*comdl;
	DG_COMDL_POS 		*comdl_pos;
	Warm			*warm;

	FVECTOR			pos;
	SVECTOR			rot;



	def = (DG_DEF *)GV_GetCache( GV_CacheID( WARM_CODE, 'k' ) );

	comdl = work->comdl = DG_MakeComdl( def->models[0].packs, DG_COMDL_SEMITRANS, work->n_warms, 0 );
	if( comdl == NULL ) {
	    return 0;
	}
	DG_QueueComdlObjs( comdl );


	work->comdl->group_id = GM_GetDGGroupID( GM_CurrentStageMap );
	    
	
	comdl_pos = comdl->pos;
	warm      = work->warm;	

	pos.vx = ( work->bound[0].vx + work->bound[1].vx ) / 2.0f;
	pos.vy = ( work->bound[0].vy + work->bound[1].vy ) / 2.0f;
	pos.vz = ( work->bound[0].vz + work->bound[1].vz ) / 2.0f;
	pos.vw = 1.0f;

	for ( i = 0 ; i < work->n_warms ; i++ ) {

	    /* コモデルMATRIX初期化 */
	    DG_SetPos( &DG_UnitMatrix );
	    rot.vx = irnd()%4096;
	    rot.vy = irnd()%4096;
	    rot.vz = irnd()%4096;
	    DG_RotatePos( &rot );
	    DG_GetPos( &warm->mat );
	    DG_COPY_VEC( (FVECTOR *)warm->mat.m[3], &pos );

	    _ScaleMatrix( &comdl_pos->world, &warm->mat, SCALE );
	    DG_COPY_VEC( (FVECTOR*)comdl_pos->world.m[3], (FVECTOR *)warm->mat.m[3] );

	    /* コモデル色初期化 */
	    comdl_pos->color.vx = 128.0f;
	    comdl_pos->color.vy = 128.0f;
	    comdl_pos->color.vz = 128.0f;
	    comdl_pos->color.vw = 128.0f;


	    warm->count = 2 + irnd()%60;
	    warm->speed = SPEED * ( frnd()*0.30f + 0.70f );

	    warm->rot.vx = 0;
	    warm->rot.vy = 0;
	    warm->rot.vz = 0;

	    warm->phase = WARM_PHASE_FLY;
	    warm->life = 60 + irnd()%60;
	    warm->b_flag = 1;	    
	    
	    comdl_pos++;
	    warm++;

	}

	return 1;

}



static int GetResourcesScn( Work *work )
{
    
    work->mode = WARM_MODE_NORMAL;
    work->cnt = 0;
    work->se_count = 0;

    GCL_GetOption( 'p' );                                 
    work->light_pos.vx = ( float )GCL_GetNextInt();	
    work->light_pos.vy = ( float )GCL_GetNextInt();	
    work->light_pos.vz = ( float )GCL_GetNextInt();
    work->light_pos.vw = 1.0f;

    GCL_GetOption( 'b' );
    work->bound_flag = GCL_GetNextInt();    
    
    GCL_GetOption( 's' );                                 
    work->bound[0].vx = ( float )GCL_GetNextInt();
    work->bound[0].vy = ( float )GCL_GetNextInt();
    work->bound[0].vz = ( float )GCL_GetNextInt();
    work->bound[0].vw = 1.0f;

    GCL_GetOption( 'l' );                                 
    work->bound[1].vx = ( float )GCL_GetNextInt();
    work->bound[1].vy = ( float )GCL_GetNextInt();
    work->bound[1].vz = ( float )GCL_GetNextInt();
    work->bound[1].vw = 1.0f;
    
    if ( InitComdl( work ) ) {    
		return 0;
	}
	else {
		return -1;
	}

}





static int GetResources( Work *work, FVECTOR *light_pos )
{
    
    work->light_pos = *light_pos;

    work->bound[0].vx = 9800.0f;
    work->bound[0].vy = 0.0f;
    work->bound[0].vz =-19500.0f;
    work->bound[0].vw = 1.0f;

    work->bound[1].vx = 11800.0f;
    work->bound[1].vy = 2800.0f;
    work->bound[1].vz =-11100.0f;
    work->bound[1].vw = 1.0f;

    work->mode = WARM_MODE_NORMAL;    
    
    if ( InitComdl( work ) ) {    
		return 0;
	}
	else {
		return -1;
	}

}




void *NewFlyingWarmScn( int name, int where )
{
	Work	*work;
	int	n_warms;
	int	data_size;

	/* 虫の数取得 */
	GCL_GetOption( 'n' );
	n_warms = GCL_GetNextInt();	

	data_size = sizeof( Work ) + sizeof( Warm ) * n_warms;
	
	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, data_size );

	if ( work != NULL ) {	        
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		work->name = name;
		work->n_warms = n_warms;
		if ( GetResourcesScn( work ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}


void *NewFlyingWarm( FVECTOR *light_pos )
{
	Work	*work;
	int	n_warms;
	int	data_size;

	n_warms = N_PRIMS * N_VERTS;
	data_size = sizeof( Work ) + sizeof( Warm ) * n_warms;
	
	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, data_size );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		work->name = 0;
		work->n_warms = n_warms;
		if ( GetResources( work, light_pos ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}









