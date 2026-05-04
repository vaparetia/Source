//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  solidus_snakearm_flow.c
  ソリダス蛇手オーラ
  2001/04/16 Yuuta Kunibe	
  $Id: solidus_snakearm_flow.c,v 1.1.1.3 2002/11/19 11:44:51 Yoshizawa1 Exp $
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


#define N_PRIMS			(15*5*2)

#define	SHIFT			(200.0f)

#define SCR_POS 		(SCRPAD_ADDR)
#define SCR_UVS 		(SCRPAD_ADDR+0x2000)

#define SIZE			(400)

#define COL_R			(20)
#define COL_G			(44)
#define COL_B			(42)
#define ALPHA			(8)

#define LIFE			(60)

/* 蛇手関節 */
#define	NODE_POINT_NUM		(5)

#define	LEFT_FIRST_NODE		(16)
#define	LEFT_LAST_NODE		(1)
#define	RIGHT_FIRST_NODE	(32)
#define	RIGHT_LAST_NODE		(17)


#define	NORMAL_COL_R		(64)
#define	NORMAL_COL_G		(64)
#define	NORMAL_COL_B		(64)

#define	DAMAGE_COL_R		(255)//(128)
#define	DAMAGE_COL_G		(128)//(64)
#define	DAMAGE_COL_B		(32) //(16)



/* ソリダス蛇手プラズマ */
extern void *NewSolidusSnakearmPlasma( DG_EVMOBJ *evmobj, FVECTOR *color, int life );
extern void *NewPolyPlasma( FVECTOR *from, FVECTOR *middle, FVECTOR *to, float width, FVECTOR *color );
/*-------- 汎用火花 (足火花に使用) --------*/
extern void *NewSpark2( int n_packets, FMATRIX *matrix,
			float min_speed, float speed_wide, float gravity,
			SVECTOR *rot,SVECTOR *rot_wide,
			FVECTOR *color, float length, int count ) ;
extern float SOL_LifePercentage();     


typedef	struct	{

    GV_ACT_EX	actor;
    int		name;

    DG_PRIM2	*prim;

    DG_EVMOBJ	*evmobj;
    FVECTOR	*pos;

    int		id[N_PRIMS];
    int 	count;

    int 	*flag;
    int		demo_flag;

    int		alpha;

    FVECTOR	normal_color;
    FVECTOR	damage_color;
    int		plasma_count;
    int		plasma_flag;
    
} Work;





static void RecieveMessage( Work *work )
{
    GV_MSG*	msg;
    int 	n_msg;


    if ( work->name == 0 ) {
		return;
    }

    /* メッセージ取得 */
    n_msg = GV_ReceiveMessage( work->name, &msg ); 

    /* メッセージ反映 */
    while ( n_msg-- > 0 ){

		switch ( msg->message[ 0 ] ) {
		case 0:			// fade_in
			work->demo_flag = 1;
			break;
		case 1:			// fade_out
			work->demo_flag = 0;
			break;
		case 2:			// flow_on
			work->alpha = ALPHA;
			work->demo_flag = 1;			
			break;
		case 3:			// flow_off
			work->alpha = 0;
			work->demo_flag = 0;			
			break;
		case 4:			// plasma_on
			work->plasma_flag = 1;
			break;
		case 5:			// plasma_off
			work->plasma_flag = 0;
			break;
		}
		msg++;
    }

	return;
}



static void Act( Work *work )
{

    int			i;
    int	  		clock;
    FVECTOR  		*pos;
    DG_PRIM2_UVRGBWH	*uvrgbwh;

    int 		node_id;
    FVECTOR		node1;
    FVECTOR		node2;

    int			life;

	float		life_rate;
	int 		damage_count;


    if ( work->evmobj == NULL ) {
		GV_DestroyActor( work );
		return;
    }

    /*{
	extern float	OK_slow_param;
	
	if ( GV_PadData[1].press & PAD_L1 ) {
	    if ( OK_slow_param == 1.0f ) {
		OK_slow_param = 0.0f;
	    }
	    else {
		OK_slow_param = 1.0f;
	    }
	}
	else if ( GV_PadData[1].status & PAD_L2 ) {
	    if ( OK_slow_param > 0.025f ) {
		OK_slow_param -= 0.025f;
	    }
	    else {
		OK_slow_param = 0.0f;
	    }
	}
	    
	printf("slow_param : %f\n", OK_slow_param );

    }*/    


    RecieveMessage( work );

    
    if ( !( work->count % 4 ) ) {
		if ( *work->flag ) {
			if ( work->alpha < ALPHA ) {
				work->alpha++;
			}
		}
		else {
			if ( work->alpha > 0 ) {
				work->alpha--;
			}
		}
    }


    
    /* 操作する頂点バッファ取得 */
    DG_SwitchBuffPrim2( work->prim );
    clock = work->prim->buffer_clock;
    pos = work->prim->pos[clock];
    uvrgbwh = work->prim->uvrgb[clock];

    if ( work->alpha > 0 ) {

		DG_VisiblePrim2( work->prim );

		for ( i = 0 ; i < N_PRIMS ; i++ ) {

			if ( !( work->id[i] % NODE_POINT_NUM ) ) {		/* 関節に配置 */

				/* 関節ID取得 */
				node_id = work->id[i] / NODE_POINT_NUM;

				/* 関節相対位置取得 */
				node1.vx = work->evmobj->def->skeleton[ node_id ].rt_tx;
				node1.vy = work->evmobj->def->skeleton[ node_id ].rt_ty;
				node1.vz = work->evmobj->def->skeleton[ node_id ].rt_tz;
		
				/* 関節位置絶対化 */
				DG_SetPos( &work->evmobj->matrix[ work->evmobj->use_buffer ][ node_id ] );
				DG_PutVector( &node1, &node1, 1 ) ;	


				/* 位置更新 */
				DG_COPY_VEC( pos, &node1 );

			}
			else {						/* 関節間に配置 */

				/* 関節ID取得 */
				node_id = work->id[i] / NODE_POINT_NUM;

				/* 関節相対位置取得 */
				node1.vx = work->evmobj->def->skeleton[ node_id ].rt_tx;
				node1.vy = work->evmobj->def->skeleton[ node_id ].rt_ty;
				node1.vz = work->evmobj->def->skeleton[ node_id ].rt_tz;

				/* 関節位置絶対化 */
				DG_SetPos( &work->evmobj->matrix[ work->evmobj->use_buffer ][ node_id ] );
				DG_PutVector( &node1, &node1, 1 ) ;
				_sceVu0ScaleVector( &node1, &node1,
									(float)( NODE_POINT_NUM - work->id[i] % NODE_POINT_NUM ) / (float)NODE_POINT_NUM );

				node_id += 1;
	
				/* 関節相対位置取得 */	
				node2.vx = work->evmobj->def->skeleton[ node_id ].rt_tx;
				node2.vy = work->evmobj->def->skeleton[ node_id ].rt_ty;
				node2.vz = work->evmobj->def->skeleton[ node_id ].rt_tz;

				/* 関節位置絶対化 */
				DG_SetPos( &work->evmobj->matrix[ work->evmobj->use_buffer ][ node_id ] );
				DG_PutVector( &node2, &node2, 1 ) ;	
				_sceVu0ScaleVector( &node2, &node2,
									(float)( work->id[i] % NODE_POINT_NUM ) / (float)NODE_POINT_NUM );

				_sceVu0AddVector( pos, &node1, &node2 );

			}

			uvrgbwh->a = work->alpha;

			/* スプライトループ */
			if ( i < N_PRIMS/2 ) {	/* node : 16 ->> 1 */
				if ( --work->id[i] <= LEFT_LAST_NODE * NODE_POINT_NUM ) {
					work->id[i] = LEFT_FIRST_NODE * NODE_POINT_NUM;
				}
			}
			else {			/* node : 32 ->> 17 */
				if ( --work->id[i] <= RIGHT_LAST_NODE * NODE_POINT_NUM ) {
					work->id[i] = RIGHT_FIRST_NODE * NODE_POINT_NUM;
				}
			}

			pos++;
			uvrgbwh++;
    
		}


		/* プラズマ呼び出し */
		if ( work->alpha == ALPHA && work->plasma_flag ) {			
			if ( work->plasma_count == 0 ) {
				life = 5 + irnd()%15;
				NewSolidusSnakearmPlasma( work->evmobj, &work->normal_color, life );
				work->plasma_count = 5 + irnd()%5;
			}
			else {
				work->plasma_count--;
			}
		}

    }
    else {

		DG_InvisiblePrim2( work->prim );

		/* ゲーム中のダメージエフェクト */
		if ( work->name == 0 ) {

			//printf("life %f, count %d\n", SOL_LifePercentage(), work->plasma_count );
			if ( ( life_rate = SOL_LifePercentage() ) < 80.0f ) {				

				if ( work->plasma_count == 0 ) {

					if ( ( work->count / 30 )%2 ) {

						NewSolidusSnakearmPlasma( work->evmobj, &work->damage_color, 5 );
						GM_SeSetMode( SD_E_HIBANA02,
									  (FVECTOR *)work->evmobj->matrix[work->evmobj->use_buffer][1+irnd()%32].m[3],
									  GM_SEMODE_NORMAL );

						{
							FMATRIX mat;
							FVECTOR	color;
							SVECTOR	rot,rot_wide;
							int		spark_node;
							FVECTOR	spark_pos;
							int		i;

							// テンプライト
							_sceVu0ScaleVector( &spark_pos,
												(FVECTOR *)work->evmobj->matrix[work->evmobj->use_buffer][0].m[2],
												-1000.0f );
							_sceVu0AddVector( &spark_pos, &spark_pos,
											  (FVECTOR *)work->evmobj->matrix[work->evmobj->use_buffer][0].m[3] );

							DG_SetTmpLight2 (
											 &spark_pos,
											 (  750.0f + 200.f * frnd() ),
											 ( 1500.0f + 200.f * frnd() ),
											 0x8f | 0x2f<<8 | 0x0f<<16,
											 LIT_FLAG_CHARAONLY );
		    
							// ランダムな関節から火花呼び出す
							for ( i = 0 ; i < 10 ; i++ ) {

								spark_node = 1+irnd()%32;
								spark_pos.vx = work->evmobj->def->skeleton[ spark_node ].rt_tx;
								spark_pos.vy = work->evmobj->def->skeleton[ spark_node ].rt_ty;
								spark_pos.vz = work->evmobj->def->skeleton[ spark_node ].rt_tz;
		
								DG_SetPos( &work->evmobj->matrix[ work->evmobj->use_buffer ][ spark_node ] );
								DG_PutVector( &spark_pos, &spark_pos, 1 ) ;	

								DG_COPY_MAT( &mat, &DG_UnitMatrix );
								spark_pos.vw = 1.0f;
								DG_COPY_VEC( (FVECTOR *)mat.m[3], &spark_pos );


								rot.vx = irnd()%4096-2048;
								rot.vy = irnd()%4096-2048;
								rot.vz = irnd()%4096-2048;
								rot_wide.vx = 1500;
								rot_wide.vy = 1500;
								rot_wide.vz = 1;
								color.vx = 255.0F;
								color.vy = 128.0F;
								color.vz = 32.0F;
								color.vw = 50.0F;		    
								NewSpark2( 5,
										   &mat,
										   1.0F,
										   50.0F + 10.f * frnd(),
										   2.0F,
										   &rot, &rot_wide,
										   &color,
										   0.5F,
										   10 + irnd()%10 );
							}

						}


						damage_count = (int)( ( life_rate - 40.0f ) / 40.0f * 30.0f );
						if ( damage_count < 0 ) {
							damage_count = 0;
						}
						damage_count += 5;
						work->plasma_count = damage_count + irnd()%( damage_count*2 );

					}

				}
				else {					
					work->plasma_count--;
				}

			}

		}

    }


    work->count++;

}


static void Die(Work *work )
{
    if ( work->prim ) work->prim = OK_FreePrim2( work->prim );
}


/* プリミティブ初期化関数 */
static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{

	int			i;
	FVECTOR			*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh;


	prim->raise = 0;


	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );


	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;

	for ( i = 0 ; i < N_PRIMS ; i++ ) {
	
	    DG_COPY_VEC( pos, &DG_ZeroVector );

	    uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset );
	    uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset );
	    uvrgbwh->q0 = 4096;
	    uvrgbwh->f0 = 0x0fff;
	
	    uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset );
	    uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset );
	    uvrgbwh->q1 = 4096;
	    uvrgbwh->f1 = 0x0fff;

	    uvrgbwh->r  = COL_R;
	    uvrgbwh->g  = COL_G;
	    uvrgbwh->b  = COL_B;
	    uvrgbwh->a  = 0;
	
	    uvrgbwh->w  = uvrgbwh->h  = SIZE * ( 0.8f + frnd()*0.2f );

	    work->id[i] = i + 6;
	    if ( i >= N_PRIMS/2 ) {
			work->id[i] += 5;
	    }
	    
	    pos++;
	    uvrgbwh++;

	}
	
	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	return 1;

}



static int GetResources( Work *work, DG_EVMOBJ *evmobj )
{
    
    DG_PRIM2		*prim ;
    DG_TEX		*tex ;

    work->evmobj = evmobj;

    work->alpha  = 0;
    work->count  = 0;
    work->plasma_count = 0;
	work->plasma_flag  = 1;

    /* ノーマルプラズマ色 */
    work->normal_color.vx = NORMAL_COL_R;
    work->normal_color.vy = NORMAL_COL_G;
    work->normal_color.vz = NORMAL_COL_B;

    /* ダメージプラズマ色 */
    work->damage_color.vx = DAMAGE_COL_R;
    work->damage_color.vy = DAMAGE_COL_G;
    work->damage_color.vz = DAMAGE_COL_B;
    
    prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, 1 );
	if ( prim == NULL ) {
		return -1;
	}

    tex = DG_GetTexture( GV_StrCode( "w_bonbori" ) );
    
    InitPacket( work, prim, tex );    
    
    return 0;
}


void *NewSolidusSnakearmFlow( DG_EVMOBJ *evmobj, int *flag )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		work->name = 0;
		work->flag = flag;
		if ( GetResources( work, evmobj ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}



void *NewDemoSolidusSnakearmFlow( int name, DG_EVMOBJ *evmobj )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		work->name = name;
		work->demo_flag = 0;
		work->flag = &work->demo_flag;
		if ( GetResources( work, evmobj ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}





