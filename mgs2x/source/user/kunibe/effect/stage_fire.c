//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    stage_fire.c
    汎用置き炎
    2001/04/25 Yuuta Kunibe	
    $Id: stage_fire.c,v 1.1.1.3 2002/11/19 11:44:52 Yoshizawa1 Exp $
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
#include	"../../morita/emma/include/emma_com.h"




#define N_PRIMS		(5)
#define	N_VERTS		(16)
#define	N_SPRT		(N_PRIMS*N_VERTS)

#define	FIRE_TEX        ( GV_StrCode( "blood_2bw_msk" ) )

#define COL_R		(79)
#define COL_G		(19)
#define COL_B		(13)
#define ALPHA		(20)

#define ALPHA_MIN	(40)
#define	ALPHA_WIDTH	(16)


#define	FIRE_MAX_SIZE	(500.0f)
#define	FIRE_MIN_SIZE	(50.0f)


#define SCR_POS 	(SCRPAD_ADDR)
#define SCR_UVS 	(SCRPAD_ADDR+0x2000)

#define WIND_MAX 	(20.f)

#define	SPRAY_LENGTH	(3000.0f)
#define	INNER_LEVEL	(-0.95f)


#define	N_PRIMS2	(3)
#define	N_VERTS2	(16)
#define	N_POWDER	(N_PRIMS2*N_VERTS2)

#define	COL_POWDER	(128)
#define	POWDER_ALPHA	(32)
#define	POWDER_ALPHA_WIDTH	(16)

#define	POWDER_TEX	( GV_StrCode( "bombpowder7_msk" ) )


#define	FIRE_SEG_FLAG	(HZX_SEG_NO_BULLET|HZX_SEG_NO_MISSILE|HZX_SEG_NO_C4|HZX_SEG_NO_RECOIL|HZX_SEG_NO_HARITSUKI|HZX_SEG_NO_DISP_RADAR|HZX_SEG_NO_BULLETHOLE|HZX_SEG_NO_ENEMY_EYES|HZX_SEG_NO_KNOCK_SE)

#define	SEG_HALF_WIDTH	(250)
#define	SEG_HEIGHT	(1300)
#define	SEG_OFFSET_Y	(250)

#define	SE_RANGE2	(3000.0f*3000.0f)

#define WIND_MAX 	(20.f)


/* グローバル風パラメータ */
extern FVECTOR G_wind;
extern SVECTOR G_wind_rot;
extern int G_wind_intense;
extern int G_wind_intense_max;
extern FMATRIX G_wind_matrix;


extern void *NewSolBodyFlame( DG_OBJS *body, int cv2_id );
extern void *NewBodyFire( OBJECT *body, int part );
extern void *NewBookFire( FMATRIX *mat );
extern void *PutCBoxBreakBody( FMATRIX *world, int map ) ;
extern int  FireBoxFlag;


int	RaidenFireFlag = 0;
int	EmmaFireFlag = 0;


enum {
    FIRE_NORMAL		= 0x00,    
    FIRE_INVISIBLE	= 0x01,
    FIRE_KILL		= 0x02,
};

enum {
    FIRE_STATE_FIRE		= 0,
    FIRE_STATE_EXTINGUISH	= 1,
    FIRE_STATE_ENDLESS		= 2,
};

static int STAGE_FIRE_SE[] =  {
    SD_A_FIRERED1,
    SD_A_FIRERED2,
    SD_A_FIRERED3,
    SD_A_FIRERED4,
    SD_A_FIRESMK1,
    SD_A_FIRESMK2,
    SD_A_FIRESMK3,
    SD_A_FIRESMK4,
};

typedef struct {

    FVECTOR	vec;
    float	rad;
    float	rot;
    float	rot_add;

} FIRE_PARAM;


typedef	struct	{

    GV_ACT_EX	actor;
    int		name;
    int		where;

    DG_PRIM2	*prim;

    FIRE_PARAM	param[N_SPRT];

    FVECTOR*	pos;
    FVECTOR	position;
    float	size;
    float	radius;

    float	initsize;
    float	rising_spd;
    FVECTOR	wind;

    int		ext_flag;		/* 消火フラグ */
    int		proc;			/* 消火プロック */
    char	*block;			
    
    int		cnt;


    DG_PRIM2	*prim_powder;
    FVECTOR	vec_powder[N_POWDER];
    int		id;
    int		powder_cnt;



    /* シナリオ指定モード */
    int		mode;


    /* 侵入禁止壁 */
    int		seg_flag;
    int		x_width;
    int		z_width;
    HZX_D_SEGMENT	*segment[4];


    /* 体燃えカウンタ */
    int		raiden_fire_count;
    int		emma_fire_count;    


    /* ダメージモーション発動用ターゲット */
    /* ライデン用 */
    TARGET	 	pl_offense;
    POWER_TARGET 	pl_pt_offense;
    /* エマ用 */
    TARGET	 	em_offense;
    POWER_TARGET 	em_pt_offense;

    int			sd_id;
    int			sd_count;
    
    int mDamageTick;

} Work;





/* シナリオメッセージ受信関数 */
static void FireRecieveMessage( Work* work )
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
	case FIRE_NORMAL:		
	    work->mode = FIRE_NORMAL;
	    break;
	case FIRE_INVISIBLE:
	    work->mode = FIRE_INVISIBLE;
	    break;
	case FIRE_KILL:
	    work->mode = FIRE_KILL;
	    break;
	default:	    
	    break;
	}
	msg++;
    }

}



/* プレイヤー消火特殊処理メッセージ受信関数 */
static void ExtraRecieveMessage( Work *work )
{

    GV_MSG*	msg;
    int 	n_msg;


    /* メッセージ取得 */
    n_msg = GV_ReceiveMessage( GM_PLAYER_CHAR_BODY_FLAME, &msg ); 

    /* メッセージ反映 */
    while ( n_msg-- > 0 ){

	switch ( msg->message[ 0 ] ) {
	case 0:			/* fade_in, fade_out */
	    work->raiden_fire_count = 0;
	    RaidenFireFlag = 0;
	    printf("stage_fire-------------extra_extinguish!!\n");
	    break;
	}
	msg++;
    }

}




static void SearchBookFire( Work *work )
{

    GM_BOMB	*this;
    GM_BOMB	*list;
    FVECTOR	vectmp;
    FMATRIX	mat;


    list = GM_BombList.next;

    while( list != NULL ) {

	this = list;
	list = list->next;

	switch ( this->weapon ) {
	case WP_Book:
	    _sceVu0SubVector( &vectmp, this->mov, &work->position );
	    if ( vectmp.vx * vectmp.vx + vectmp.vz * vectmp.vz < work->radius * work->radius * 4.0f ) {

		if ( !( this->flag & GM_BMB_FLAG_DESTROY ) ) {
		    this->flag |= GM_BMB_FLAG_DESTROY;	
		    DG_COPY_MAT( &mat, &DG_UnitMatrix );
		    DG_COPY_VEC( (FVECTOR *)mat.m[3], this->mov );
		    NewBookFire( &mat );
		}
		break;
	    }

	}

    }

}



static void ActFire( Work *work )
{
    int			i;
    int	  		clock;

    FVECTOR  		*pos;
    FVECTOR  		*pos_pre;
    DG_PRIM2_UVRGBWH 	*uvrgbwh;
    DG_PRIM2_UVRGBWH 	*uvrgbwh_pre;
    FIRE_PARAM		*param;

    float		ftmp;
    float		cos;

    FMATRIX		*root;
    static FVECTOR	NozzlePoint = { 13.0f, -323.0f, 61.0f, 1.0f };
    FVECTOR		from;
    FVECTOR		vectmp;

    int			alpha_flag;

    GCL_ARGS 		arg;
    int 		buffer[ 3 ];

    float 		x, z;



    /* 操作する頂点バッファ取得 */
    clock = work->prim->buffer_clock;

    pos		= work->prim->pos[clock];
    pos_pre	= work->prim->pos[1-clock];
    uvrgbwh	= work->prim->uvrgb[clock];
    uvrgbwh_pre = work->prim->uvrgb[1-clock];
    param 	= work->param;

   if ( !( work->cnt%4 ) )
   {
   	alpha_flag = 1;
   }
   else
   {
   	alpha_flag = 0;
   }

    /* 消火 & プレイヤー燃え処理 */
    root = NULL;

    if ( work->ext_flag != FIRE_STATE_EXTINGUISH ) {

	/* 冷却スプレー消火 */
	if ( GM_WeaponFire == WP_ColdSpray ) {

	    if ( GM_PlayerSubWeaponBody != NULL ) {

		/* 冷却スプレーのマトリクス取得 */
		root = GM_PlayerSubWeaponBody->objs->root;

		/* ノズルの絶対位置取得 */
		DG_SetPos( root );
		DG_PutVector( &NozzlePoint, &from, 1 );

		/* 炎パラメータの減衰 */
		/* 距離判定 */
		_sceVu0SubVector( &vectmp, &work->position, &from );
		if ( _sceVu0InnerProduct( &vectmp, &vectmp ) < SPRAY_LENGTH * SPRAY_LENGTH ) {	

		    _sceVu0Normalize( &vectmp, &vectmp );

		    /* 角度判定 */
		    if ( _sceVu0InnerProduct( &vectmp, (FVECTOR *)root->m[1] ) < INNER_LEVEL ) {	

			/* 消火中 */
			if ( work->size > FIRE_MIN_SIZE ) {
			    work->size       *= 0.98f;
			    work->rising_spd *= 0.96f;
			}
			/* 消火完了 -> 消える炎なら消す */
			else if ( work->ext_flag == FIRE_STATE_FIRE ) {
			    work->ext_flag   = FIRE_STATE_EXTINGUISH;
			    work->rising_spd = 2.0f;
			    work->sd_id += 4;		/* 音声変更 */

			    /* 壁消去 */
			    if ( work->seg_flag ) {
				for ( i = 0 ; i < 4 ; i++ ) {
				    HZX_RemoveDynamicSegment( work->segment[ i ] ) ;
				}
			    }

			    /* 消火PROC呼び出し */
			    if ( work->proc != 0 ) {
				arg.argc = 3;
				arg.argv = buffer;
				buffer[ 0 ] = (int)work->position.vx;
				buffer[ 1 ] = (int)work->position.vy;
				buffer[ 2 ] = (int)work->position.vz;

				GCL_ExecProc( work->proc, &arg );
			    }
			    /* 消火BLOCK呼び出し */
			    if ( work->block != NULL ) {
				arg.argc = 3;
				arg.argv = buffer;
				buffer[ 0 ] = (int)work->position.vx;
				buffer[ 1 ] = (int)work->position.vy;
				buffer[ 2 ] = (int)work->position.vz;

				GCL_ExecBlock( work->block, &arg );
			    }

			}
		    }
		}
	    }
	}
	/* 炎復活 */
	else {

	    if ( work->size < FIRE_MAX_SIZE ) {
		work->size = work->size * 0.80f + FIRE_MAX_SIZE * 0.20f;
	    }

	    if ( work->rising_spd < 30.0f ) {
		work->rising_spd = work->rising_spd * 0.90f + 30.0f * 0.10f;
	    }

	}

	/*炎エリアに本が置かれたら炎呼んで本消す */
	SearchBookFire( work );
	
	/* プレイヤー燃え呼び出し */
	if ( !( GM_PlayerStatus & ( PLAYER_ROLLING | PLAYER_DAMAGED ) ) ) {
	    if ( RaidenFireFlag == 0 ) {
		x = GM_PlayerControl->mov.vx - work->position.vx ;
		z = GM_PlayerControl->mov.vz - work->position.vz ;
		
		if ( x*x + z*z < work->radius * work->radius * 4.0f ) {

		    for ( i = 0 ; i < 5 ; i++ ) {
			NewBodyFire( GM_PlayerBody, i );
		    }
		    RaidenFireFlag = 1;
		    work->raiden_fire_count = 300;

		    /* ダメージモーション発動 */
		    GM_MoveTarget( &work->pl_offense, &GM_PlayerPosition );
		    GM_PutTarget( &work->pl_offense );

		    /* ダンボール装備中処理 */
		    //if ( GM_PlayerStatus & PLAYER_CB_BOX ) {
		    // 条件変更 2001.08.31
		    if ( GM_ItemTypes[PL_GetPlayerItem()] & IT_TYPE_CBBOX ) {			
			/* ダンボール燃えフラグ立てておく */
			FireBoxFlag = 1;
			/* ダンボール壊れ呼び出し */
			PutCBoxBreakBody( &GM_PlayerBody->objs->world, GM_PlayerControl->map );
			/* ダンボール消滅 ->> 100減らせばいいでしょう */
			GM_DecrementItem( PL_GetPlayerItem(), 100 );	
		    }			

		}
	    }
	}

	/* エマ燃え呼び出し */
	if ( EMA_CommandGetControl() != NULL ) {
	    if ( EmmaFireFlag == 0 ) {
		extern void *NewEmmaBodyFire( OBJECT *body, int se_flag );
		CONTROL	*emma_control = EMA_CommandGetControl();
		int	se_flag;

		x = emma_control->mov.vx - work->position.vx ;
		z = emma_control->mov.vz - work->position.vz ;
		if ( x*x + z*z < work->radius * work->radius * 4.0f ) {
		    for ( i = 0 ; i < 5 ; i++ ) {
			if ( i == 0 ) {
			    se_flag = 1;
			}
			else {
			    se_flag = 0;
			}
			NewEmmaBodyFire( emma_control->object, se_flag );
		    }
		    EmmaFireFlag = 1;
		    work->emma_fire_count = 300;

		    /* ダメージモーション発動 */
		    GM_MoveTarget( &work->em_offense, &emma_control->mov );
		    GM_PutTarget( &work->em_offense );

		}
	    }
	}
	
    }

    // debug segment view
    /*if ( work->seg_flag && work->ext_flag != FIRE_STATE_EXTINGUISH ) {
	for ( i = 0 ; i < 4 ; i++ ) {
	    HZX_ViewDynamicSegment( work->segment[ i ] ) ;
	}
    }*/
    
   for ( i = 0 ; i < N_SPRT ; i++ )
   {
	   /* スプライト更新 */
      if ( uvrgbwh_pre->a > ALPHA_MIN )
      {
         /* 位置更新 */
	      _sceVu0AddVector( pos, pos_pre, &param->vec );	    

	      /* 速度更新 */
	      param->vec.vx = 12.5f * frnd();
	      param->vec.vz = 12.5f * frnd();

	      /* 幅,高さ更新 */
	      cos = vu0_Cos( param->rot );
	      uvrgbwh->w = (short)( param->rad * cos );
	      uvrgbwh->h = (short)( param->rad * cos );


	      /* 角度更新 */
	      param->rot += param->rot_add;
	      if ( param->rot > PI )
         {
            param->rot -= TPI;
	      }
	      else if ( param->rot < -PI )
         {
		      param->rot += TPI;
	      }

         if ( uvrgbwh->a > ALPHA_MIN - 5 )
         {
            param->rad *= 0.985f;

            if ( work->ext_flag != FIRE_STATE_EXTINGUISH )
            {
   		       param->vec.vy += 0.5f;
	   	   }

		      if ( alpha_flag )
            {
               uvrgbwh->a = uvrgbwh_pre->a - 1;
		      }
		      else
            {
   		       uvrgbwh->a = uvrgbwh_pre->a;
	   	   }
	      }
	      else
         {
   		   param->rad *= 0.80f;
   		   if ( work->ext_flag != FIRE_STATE_EXTINGUISH )
            {
               param->vec.vy += 0.50f;
   		   }
		      uvrgbwh->a = uvrgbwh_pre->a - 1;
	      }
         /* 冷却スプレーで空中炎消える */
         if ( root != NULL )
         {
            if ( GM_WeaponFire == WP_ColdSpray )
            {
               _sceVu0SubVector( &vectmp, pos, &from );
		         if ( _sceVu0InnerProduct( &vectmp, &vectmp ) < SPRAY_LENGTH * SPRAY_LENGTH )
               {
			         _sceVu0Normalize( &vectmp, &vectmp );
			         if ( _sceVu0InnerProduct( &vectmp, (FVECTOR *)root->m[1] ) < INNER_LEVEL )
                  {
                     param->rad *= 0.90f;
			         }
		         }
		      }
         }
   	}
      /* スプライト初期化 */
	   else
      {
         if ( work->ext_flag != FIRE_STATE_EXTINGUISH || work->sd_id < 0 )
         {
            /* 位置初期化 */
		      DG_COPY_VEC( pos, &work->position );
		      //pos->vx += frnd()*work->radius;
		      //pos->vz += frnd()*work->radius;
		      pos->vx += frnd()*work->size;
		      pos->vz += frnd()*work->size;

            /* パラメータ初期化 */
		      ftmp = ( 0.5f + frnd() * 0.50f );
		      param->rad = work->size * ( 0.5f + ftmp ) * 1.4f;
		      param->rot = PI / 2.0f;

		      param->vec.vx = work->rising_spd * 25.0f / 30.0f * frnd();
		      param->vec.vy = work->rising_spd * ( 1.0f - ftmp ) / 1.0f + work->rising_spd * 2.f / 3.f; 
		      param->vec.vz = work->rising_spd * 25.0f / 30.0f * frnd();
		      _sceVu0ScaleVector( &param->vec, &param->vec, 0.50f );

		      /* 幅,高さ,アルファ初期化 */
		      cos = vu0_Cos( param->rot );
		      uvrgbwh->w = (short)( param->rad * cos );
		      uvrgbwh->h = (short)( param->rad * cos );
		      uvrgbwh->a = ALPHA_MIN + irnd()%ALPHA_WIDTH;
	      }
	      else
         {
		      if ( uvrgbwh->a > 0 )
            {
               uvrgbwh->w = 0;
		         uvrgbwh->h = 0;
		         uvrgbwh->a = 0;
		      }
         }
	   }
      pos++;
      pos_pre++;
      uvrgbwh++;
      uvrgbwh_pre++;
      param++;
   }
}




static void ActPowder( Work *work )
{

    int			i;
    int	  		clock;

    FVECTOR  		*pos;
    FVECTOR  		*pos_pre;
    DG_PRIM2_UVRGBWH 	*uvrgbwh;
    DG_PRIM2_UVRGBWH 	*uvrgbwh_pre;
    FVECTOR		*vec;

    float		intense;
    float		ftmp;
    FVECTOR		local_wind;
    FVECTOR		vectmp;

    /* 操作する頂点バッファ取得 */
    clock = work->prim_powder->buffer_clock;

    pos		= work->prim_powder->pos[clock];
    pos_pre	= work->prim_powder->pos[1-clock];
    uvrgbwh	= work->prim_powder->uvrgb[clock];
    uvrgbwh_pre = work->prim_powder->uvrgb[1-clock];
    vec         = work->vec_powder;


    /* 風計算 */
    intense = WIND_MAX / (float)G_wind_intense_max;
    _sceVu0ScaleVector( &local_wind, &G_wind, intense );
    local_wind.vy = 0.f;
    work->wind.vx = work->wind.vx * 0.98f + local_wind.vx * 0.02f;
    work->wind.vy = work->wind.vy * 0.98f + local_wind.vy * 0.02f;
    work->wind.vz = work->wind.vz * 0.98f + local_wind.vz * 0.02f;



    for ( i = 0 ; i < N_POWDER ; i++ ) {

	if ( uvrgbwh_pre->a > 0 ) {
	    _sceVu0AddVector( pos, pos_pre, vec );

	    pos->vx += vec->vy * 0.50f * frnd();
	    pos->vz += vec->vy * 0.50f * frnd();

	    ftmp = 1.0f + frnd() * 0.2f;
	    _sceVu0ScaleVector( &vectmp, &work->wind, ftmp );
	    _sceVu0AddVector( pos, pos, &vectmp );
		
	    vec->vx *= 0.99f;
	    vec->vz *= 0.99f;

	    uvrgbwh->a = uvrgbwh_pre->a - 1;

	}
	else {
	    uvrgbwh->a = 0;
	}
	
	pos++;
	pos_pre++;
	vec++;
	uvrgbwh++;
	uvrgbwh_pre++;

    }
	
        

    if ( !(work->cnt%2) ) {

	pos     = &work->prim_powder->pos[clock][work->id];
	uvrgbwh = &((DG_PRIM2_UVRGBWH *)work->prim_powder->uvrgb[clock])[work->id];
	vec     = &work->vec_powder[work->id];
    
	DG_COPY_VEC( pos, &work->position );
	pos->vx += work->radius * frnd();
	pos->vz += work->radius * frnd();
	pos->vy += 750.0f + frnd()*750.0f;
    
	vec->vy = 50.0f + frnd() * 12.5f;
	vec->vx += vec->vy * 0.20f * frnd();
	vec->vz += vec->vy * 0.20f * frnd();       	

	uvrgbwh->a = POWDER_ALPHA + irnd()%POWDER_ALPHA_WIDTH;

    }
        
    if ( ++work->id >= N_POWDER ) {
	work->id = 0;
    }
    
}




/* ＳＥ呼び出し */
static void FireCallSE( Work *work )
{
    if ( work->sd_count <= 0 ) {
	GM_SeSetMode( STAGE_FIRE_SE[work->sd_id], &work->position, GM_SEMODE_NORMAL );
	work->sd_count = DIRECT_TICK( 6 + irnd()%10 );
    }
    else {
	work->sd_count--;
    }
    
}



/* アクト関数 */
static void Act( Work *work )
{

    FVECTOR	vec;
    FVECTOR	m_pos;
    float	temp_x,temp_y;
    float	len2;


    /* メッセージ受信 */
    FireRecieveMessage( work );


    /* se_idが設定されていればＳＥ呼び出しチェック ( demo ->> se_id == -1 ) */
    if ( work->sd_id >= 0 ) {
	/* ＳＥ呼び出しレンジ内でＳＥ呼び出し */
	_sceVu0SubVector( &vec, &GM_PlayerPosition, &work->position );    
	if ( _sceVu0InnerProduct( &vec, &vec ) < SE_RANGE2 ) {
	    FireCallSE( work );
	}
    }


    /* クリッピングしない処理 */
    if ( work->emma_fire_count > 0 ) {
	if ( !(work->mDamageTick & 0xf) ) {
	    EMA_CommandGiveDamage( 1 );
	}
	if ( --work->emma_fire_count == 0 ) {
	    EmmaFireFlag = 0;
	}
	//printf("emma_count : %d\n",work->emma_fire_count);
    }

    if ( work->raiden_fire_count > 0 ) {

	/* 体力減らす */
	if ( !(work->mDamageTick & 0xf) ) {
		GM_VitalityAdjust = -1;
	}

	if ( --work->raiden_fire_count == 0 ) {
	    RaidenFireFlag = 0;
	}
	else {
	    /* 体燃え特殊処理メッセージ受信 */
	    ExtraRecieveMessage( work );
	}

    }

    ++work->mDamageTick;

    if ( work->mode == FIRE_INVISIBLE ) {

	DG_InvisiblePrim2( work->prim );
	DG_InvisiblePrim2( work->prim_powder );
	return;

    }
    else {

	/* 透視変換した座標でクリッピング */
	DG_SetPos( &DG_Chanls[0].eye_pers );
	DG_PutVector( &work->position, &m_pos, 1 );

	temp_x = fpu_Abs( m_pos.vx / m_pos.vw );
	temp_y = fpu_Abs( m_pos.vy / m_pos.vw );

	_sceVu0SubVector( &vec, (FVECTOR *)DG_Chanls[0].eye.m[3], &work->position );
	len2 = vec.vx * vec.vx + vec.vz *vec.vz;
	
	if ( len2 >= 2000.0f*2000.0f && ( m_pos.vz >= m_pos.vw || temp_x > 1.5f ) ) { //|| temp_y > 2.0f ) {
	    DG_InvisiblePrim2( work->prim );
	    DG_InvisiblePrim2( work->prim_powder );
	    return ;
	}

    }

    // AS_HALFSPEED_PARTICLE
    //AR_PARTICLE_HALF
    if (!DG_SwitchBuffPrim2( work->prim ))
    {
       return;
    }
    
    DG_VisiblePrim2( work->prim );
    DG_VisiblePrim2( work->prim_powder );

    /* 炎更新 */
    ActFire( work );
    /* 火の粉更新 */
    ActPowder( work );    

    work->cnt++;

    if ( work->mode == FIRE_KILL ) {
	GV_DestroyActor( work );
    }
}




static void Die( Work *work )
{
    /* メモリ解放 */
    work->prim = OK_FreePrim2( work->prim );
    work->prim_powder = OK_FreePrim2( work->prim_powder );

    if ( work->emma_fire_count > 0 && EmmaFireFlag == 1 ) {
	EmmaFireFlag = 0;
    }

    if ( work->raiden_fire_count > 0 && RaidenFireFlag == 1 ) {
	RaidenFireFlag = 0;
    }

    if ( work->seg_flag && work->ext_flag != FIRE_STATE_EXTINGUISH ) {
	int i;
	for ( i = 0 ; i < 4 ; i++ ) {
	    HZX_RemoveDynamicSegment( work->segment[ i ] ) ;
	}
    }
}




/* 炎初期化関数 */
static int InitPacket( Work *work )
{

	int			i;
	DG_PRIM2		*prim;
	DG_TEX			*tex;
	FVECTOR			*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	FIRE_PARAM		*param;



	prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );

	tex = DG_GetTexture( FIRE_TEX );

	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );



	pos     = SCR_POS;
	uvrgbwh = SCR_UVS;
	param	= work->param;

	for ( i = 0 ; i < N_SPRT ; i++ ){

	    DG_COPY_VEC( pos, &DG_ZeroVector );

	    switch ( irnd()%4 ) {
	    case 0:
		uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
		uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
		uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
		uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
		break;
	    case 1:
		uvrgbwh->u0 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
		uvrgbwh->v0 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
		uvrgbwh->u1 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
		uvrgbwh->v1 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
		break;
	    case 2:
		uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
		uvrgbwh->v0 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
		uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
		uvrgbwh->v1 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
		break;
	    case 3:
		uvrgbwh->u0 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
		uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
		uvrgbwh->u1 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
		uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
		break;
	    }

	    uvrgbwh->q0 = 4096;
	    uvrgbwh->f0 = 0x0fff;
	    uvrgbwh->q1 = 4096;
	    uvrgbwh->f1 = 0x0fff;

	    uvrgbwh->r  = COL_R;
	    uvrgbwh->g  = COL_G + irnd()%COL_G;
	    uvrgbwh->b  = COL_B;
	    uvrgbwh->a  = 32 + irnd()%32;

	    param->rad = 0;
	    param->rot = 0;
	    param->rot_add = TPI * 0.0125f * frnd();
	    uvrgbwh->w = 0;
	    uvrgbwh->h = 0;

	    DG_COPY_VEC( &param->vec, &DG_ZeroVector );
	    param->vec.vx = 100.f * frnd();
	    param->vec.vy = 150.f * frnd() + 200.f; 
	    param->vec.vz = 100.f * frnd();

	    pos++;
	    uvrgbwh++;
	    param++;

	}	

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_SPRT );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_SPRT );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_SPRT );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_SPRT );

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	return 1;

}



/* 火の粉初期化関数 */
static int InitPacket2( Work *work )
{

	int			i;
	DG_PRIM2		*prim;
	DG_TEX			*tex;
	FVECTOR			*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	FVECTOR			*vec;



	prim = work->prim_powder = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS2, N_VERTS2 );

	tex = DG_GetTexture( POWDER_TEX );


	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );


	pos     = SCR_POS;
	uvrgbwh = SCR_UVS;
	vec     = work->vec_powder;


	for ( i = 0 ; i < N_POWDER ; i++ ){

	    DG_COPY_VEC( pos, &DG_ZeroVector );

	    switch ( i%4 ) {
	    case 0:
		uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
		uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
		uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
		uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
		break;
	    case 1:
		uvrgbwh->u0 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
		uvrgbwh->v0 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
		uvrgbwh->u1 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
		uvrgbwh->v1 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
		break;
	    case 2:
		uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
		uvrgbwh->v0 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
		uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
		uvrgbwh->v1 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
		break;
	    case 3:
		uvrgbwh->u0 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
		uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
		uvrgbwh->u1 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
		uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
		break;
	    }

	    uvrgbwh->q0 = 4096;
	    uvrgbwh->f0 = 0x0fff;
	    uvrgbwh->q1 = 4096;
	    uvrgbwh->f1 = 0x0fff;

	    uvrgbwh->r  = COL_POWDER;
	    uvrgbwh->g  = COL_POWDER;
	    uvrgbwh->b  = COL_POWDER;
	    uvrgbwh->a  = 0;

	    uvrgbwh->w = work->size;
	    uvrgbwh->h = work->size;

	    DG_COPY_VEC( vec, &DG_ZeroVector );

	    pos++;
	    uvrgbwh++;
	    vec++;

	}	

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_POWDER );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_POWDER );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_POWDER );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_POWDER );

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	return 1;

}



static void InitSegment( Work *work ) {

    IVECTOR		iv[ 4 ];
    HZX_GROUP_ID	hzx_id, id;
    int			gn;    

    id = GM_GetHzxGroupID( work->where );
    gn = GV_GetNo( id );
    hzx_id = GV_GetBit( gn );


    iv[0].vx = (int)work->position.vx + work->x_width;
    iv[0].vy = (int)work->position.vy - SEG_OFFSET_Y;
    iv[0].vz = (int)work->position.vz + work->z_width;
    iv[0].vw = SEG_HEIGHT;
    
    iv[1].vx = (int)work->position.vx - work->x_width;
    iv[1].vy = (int)work->position.vy - SEG_OFFSET_Y;
    iv[1].vz = (int)work->position.vz + work->z_width;
    iv[1].vw = SEG_HEIGHT;    
    
    work->segment[ 0 ]
	= HZX_AddDynamicSegment( hzx_id, &iv[0], &iv[1], FIRE_SEG_FLAG );
    
    iv[0].vx = (int)work->position.vx + work->x_width;
    iv[0].vy = (int)work->position.vy - SEG_OFFSET_Y;
    iv[0].vz = (int)work->position.vz - work->z_width;
    iv[0].vw = SEG_HEIGHT;
    
    iv[1].vx = (int)work->position.vx - work->x_width;
    iv[1].vy = (int)work->position.vy - SEG_OFFSET_Y;
    iv[1].vz = (int)work->position.vz - work->z_width;
    iv[1].vw = SEG_HEIGHT;    
    
    work->segment[ 1 ]
	= HZX_AddDynamicSegment( hzx_id, &iv[0], &iv[1], FIRE_SEG_FLAG );
    
    iv[0].vx = (int)work->position.vx + work->x_width;
    iv[0].vy = (int)work->position.vy - SEG_OFFSET_Y;
    iv[0].vz = (int)work->position.vz + work->z_width;
    iv[0].vw = SEG_HEIGHT;
    
    iv[1].vx = (int)work->position.vx + work->x_width;
    iv[1].vy = (int)work->position.vy - SEG_OFFSET_Y;
    iv[1].vz = (int)work->position.vz - work->z_width;
    iv[1].vw = SEG_HEIGHT;    
    
    work->segment[ 2 ]
	= HZX_AddDynamicSegment( hzx_id, &iv[0], &iv[1], FIRE_SEG_FLAG );
    
    iv[0].vx = (int)work->position.vx - work->x_width;;
    iv[0].vy = (int)work->position.vy - SEG_OFFSET_Y;
    iv[0].vz = (int)work->position.vz + work->z_width;
    iv[0].vw = SEG_HEIGHT;
    
    iv[1].vx = (int)work->position.vx - work->x_width;;
    iv[1].vy = (int)work->position.vy - SEG_OFFSET_Y;
    iv[1].vz = (int)work->position.vz - work->z_width;
    iv[1].vw = SEG_HEIGHT;    
    
    work->segment[ 3 ]
	= HZX_AddDynamicSegment( hzx_id, &iv[0], &iv[1], FIRE_SEG_FLAG );
    

    //HZX_DynamicSegmentSetAttribute( work->segments[ 0 ] ); 

}


/*--- ダメージモーション発動用ターゲット初期化 ---*/
static int InitTarget( Work *work )
{

    GM_SetTarget( &work->pl_offense, TARGET_OFFENSE, 0, PLAYER_SIDE, &DG_ZeroVector, &DG_ZeroVector );
    GM_SetPowerTarget( &work->pl_offense, &work->pl_pt_offense, POWER_ONCE, 255, 0, 1, &DG_ZeroVector );
    GM_SetTargetWeaponType( &work->pl_offense, WP_BULLET | WP_NO_BLOOD );
    GM_MoveTarget( &work->pl_offense, &DG_ZeroVector );

    GM_SetTarget( &work->em_offense, TARGET_OFFENSE, 0, PLAYER_SIDE, &DG_ZeroVector, &DG_ZeroVector );
    GM_SetPowerTarget( &work->em_offense, &work->em_pt_offense, POWER_ONCE, 255, 0, 1, &DG_ZeroVector );
    GM_SetTargetWeaponType( &work->em_offense, WP_BULLET | WP_NO_BLOOD | WP_NOPLAYER );
    GM_MoveTarget( &work->em_offense, &DG_ZeroVector );

    return 0;
    
}



static int GetResourcesScn( Work *work )
{


    work->mode = FIRE_NORMAL;
    
    /* 炎発生位置取得 */
    GCL_GetOption( 'p' );                                 
    work->position.vx = ( float )GCL_GetNextInt();	
    work->position.vy = ( float )GCL_GetNextInt()-200.0f;	
    work->position.vz = ( float )GCL_GetNextInt();
    work->position.vw = 1.0f;

    /* ステータス取得 : 2001.09.01変更 */ 
    if ( GCL_GetOption( 's' ) != NULL ) {

	work->ext_flag = GCL_GetNextInt();

	if ( work->ext_flag == FIRE_STATE_EXTINGUISH ) {
	    work->size = FIRE_MIN_SIZE;
	}
	else {
	    work->size = FIRE_MAX_SIZE;
	}
    }
    else {
	work->ext_flag = FIRE_STATE_FIRE;
	work->size = FIRE_MAX_SIZE;
    }

    /* 消火後でなければその他のパラメータ取得 */
    if ( work->ext_flag != FIRE_STATE_EXTINGUISH ) {

	/* 炎消火PROC取得 */
	work->proc = 0;
	if( GCL_GetOption( 'O' ) != NULL ) {
	    work->proc = GCL_GetNextInt();
	}

	/* 炎消火BLOCK取得 */
	work->block = NULL;
	if( GCL_GetOption( 'e' ) != NULL ){
	    int type, value;
	    GCL_GetNextValue( GCL_NextStr(), &type, &value );
	    ASSERT( type == GCL_BLOCK );
	    work->block = ( char * )value;
	}

	/* 壁フラグ取得 */
	if ( GCL_GetOption( 'f' ) != NULL ){
	    work->seg_flag = GCL_GetNextInt();
	}
	else {
	    work->seg_flag = 0;
	}

	/* 壁パラメータ取得 */
	if ( GCL_GetOption( 'x' ) != NULL ){
	    work->x_width = GCL_GetNextInt();
	}
	else {
	    work->x_width = SEG_HALF_WIDTH;
	}

	if ( GCL_GetOption( 'z' ) != NULL ){
	    work->z_width = GCL_GetNextInt();
	}
	else {
	    work->z_width = SEG_HALF_WIDTH;
	}

	work->sd_id = irnd()%4;
	work->sd_count = 0;
	
    }
    else {
	work->proc = 0;
	work->block = NULL;
	work->seg_flag = 0;
	work->sd_id = 4+irnd()%4;
	work->sd_count = 0;
    }
	

    work->radius     = 500.f;
    work->rising_spd = 30.0f;

    work->powder_cnt = 5;
    work->id = 0;

    work->cnt  = 0;

    work->raiden_fire_count = 0;
    work->emma_fire_count = 0;


    DG_COPY_VEC( &work->wind, &DG_ZeroVector );

    
    InitPacket( work );    
    InitPacket2( work );    


    if ( work->seg_flag ) {
	InitSegment( work );
    }

    InitTarget( work );
    
    return 0;


}



static int GetDemoResources( Work *work, FVECTOR *pos )
{

    /* 炎発生位置取得 */
    DG_COPY_VEC( &work->position, pos );
    work->position.vw = 1.0f;


    work->proc = 0;
    work->block = NULL;

    work->mode = FIRE_NORMAL;
    
    work->seg_flag = 0;

    work->size       = FIRE_MAX_SIZE;
    work->radius     = 500.f;
    work->rising_spd = 30.0f;

    work->powder_cnt = 5;
    work->id = 0;

    work->cnt  = 0;

    /* 消火もないから燃えてる時の特殊処理とばす */
    work->ext_flag = 1;
    work->raiden_fire_count = 0;
    work->emma_fire_count = 0;

    /* デモでは音声呼び出さない */
    work->sd_id = -1;
    work->sd_count = 0;

    DG_COPY_VEC( &work->wind, &DG_ZeroVector );
    
    InitPacket( work );    
    InitPacket2( work );    

    /* 煙との優先で勝つようにraiseあげておく( P046_01_p01.w25a, P080_01_p01.w46a ) */
    work->prim->raise = 10000;//5000;	// 優先修正 2001.09.29
    
    return 0;

}





void *NewStageFireScn( int name, int where )
{
	Work	*work;

	work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {	        
		work->name = name;
	        work->where = where;
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResourcesScn( work ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}




void *NewDemoStageFire( int name, FVECTOR *pos )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {	    
		work->name = name;
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetDemoResources( work, pos ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}

