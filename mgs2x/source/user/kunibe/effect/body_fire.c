//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
body_fire.c
体燃え
2001/04/25 Yuuta Kunibe	
$Id: body_fire.c,v 1.1.1.3 2002/11/19 11:44:33 Yoshizawa1 Exp $
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


#define N_PRIMS		(12)
#define	N_VERTS		(16)
#define	N_SPRT		(N_PRIMS*N_VERTS)

#define	FIRE_TEX	( GV_StrCode( "blood_2bw_msk" ) )

#define	COL_R		(72)
#define	COL_G		(24)
#define	COL_B		(16)
#define ALPHA		(20)

#define ALPHA_MIN	(40)
#define	ALPHA_WIDTH	(16)

#define	FIRE_MAX_SIZE	(200.0f)
#define	FIRE_MIN_SIZE	(50.0f)

#define	LIFE		(300)

#define WIND_MAX 	(20.f)

#define	SPRAY_LENGTH	(3000.0f)



#define	N_PRIMS2	(3)
#define	N_VERTS2	(16)
#define	N_POWDER	(N_PRIMS2*N_VERTS2)

#define	COL_POWDER	(128)
#define	POWDER_ALPHA	(32)
#define	POWDER_ALPHA_WIDTH	(16)

#define	POWDER_TEX	( GV_StrCode( "bombpowder7_msk" ) )



enum {
   BODY,
   L_ARM,
   R_ARM,
   L_LEG,
   R_LEG,
};


typedef struct {

   FVECTOR	vec;
   float	rad;
   float	rot;
   float	rot_add;

   int		node;
   float	param;
   float	rnd_x;
   float	rnd_z;

} FIRE_PARAM;


typedef	struct	{

   GV_ACT_EX	actor;
   int		name;

   DG_PRIM2	*prim;

   FIRE_PARAM	param[N_SPRT];

   OBJECT	*body;
   int		part;

   float	size;
   float	radius;

   float	initsize;
   float	rising_spd;
   FVECTOR	wind;

   int		ext_flag;		/* 消火フラグ */
   int		proc;			/* 消火プロック */

   int 	life;
   int		cnt;


   DG_PRIM2	*prim_powder;
   FVECTOR	vec_powder[N_POWDER];
   int		id;
   int		powder_cnt;

   int		se_count;

} Work;



static void RecieveMessage( Work *work )
{

   GV_MSG*	msg;
   int 	n_msg;


   /* メッセージ取得 */
   n_msg = GV_ReceiveMessage( work->name, &msg ); 

   /* メッセージ反映 */
   while ( n_msg-- > 0 ){

      switch ( msg->message[ 0 ] ) {
      case 0:			/* fade_in, fade_out */
         if ( work->cnt < LIFE  + N_PRIMS*N_VERTS - 60 ) {
            work->cnt = LIFE + N_PRIMS*N_VERTS - 60;
            printf("body_fire-------------extra_extinguish!!\n");
         }
         break;
      }
      msg++;
   }

}


// has a maximum error of 13%
// goes through
static inline float _approxcos(float x)
{
   //return BP_Cos(x);
   static float const skTwoOverPiPow4 = 2.0f/(PI*PI*PI*PI);
   static float const skFourOverPiPow2 = 4.0f/(PI*PI);

   float x2 = x*x;
   return x2*(skTwoOverPiPow4*x2 - skFourOverPiPow2) + 1.0f;
}


static int ActFire( Work *work )
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

   FVECTOR		vectmp;

   int			alpha_flag;

   int			node;    
   FVECTOR		init_pos;

   int			invisible_num;

   /* 操作する頂点バッファ取得 */
   clock = work->prim->buffer_clock;

   pos		= work->prim->pos[clock];
   pos_pre	= work->prim->pos[1-clock];
   uvrgbwh	= work->prim->uvrgb[clock];
   uvrgbwh_pre = work->prim->uvrgb[1-clock];
   param 	= work->param;


   invisible_num = 0;


   if ( !( work->cnt & 3 ) ) {
      alpha_flag = 1;
   }
   else {
      alpha_flag = 0;
   }



   for ( i = 0 ; i < N_SPRT ; i++ ) {

      /* スプライト更新 */
      if ( uvrgbwh_pre->a > ALPHA_MIN ) {

         /* 体付随炎 */
         if ( i&1 ) {

            _sceVu0ScaleVector( &vectmp, (FVECTOR *)work->body->objs->objs[param->node].world.m[3], param->param );

            if ( param->node == 2 ) {
               _sceVu0ScaleVector( &init_pos, (FVECTOR *)work->body->objs->objs[11].world.m[3], ( 1.0f-param->param ) );
            }
            else {
               _sceVu0ScaleVector( &init_pos, (FVECTOR *)work->body->objs->objs[param->node+1].world.m[3], ( 1.0f-param->param ) );
            }

            _sceVu0AddVector( &init_pos, &init_pos, &vectmp );

            if ( work->part == BODY ) {
               init_pos.vx += param->rnd_x;//150.0f*frnd();
               init_pos.vz += param->rnd_z;//150.0f*frnd();
            }

            DG_COPY_VEC( pos, &init_pos );

         }
         /* 舞い上がる炎 */
         else {

            /* 位置更新 */
            _sceVu0AddVector( pos, pos_pre, &param->vec );	    

            /* 速度更新 */
            param->vec.vx = 12.5f * frnd();
            param->vec.vz = 12.5f * frnd();

         }

         /* 幅,高さ更新 */
         // use an approximate cos
         // rot is guaranteed to be within -PI and PI :)
         {
            short wh = (short)(param->rad * _approxcos(param->rot));
            uvrgbwh->w = wh;
            uvrgbwh->h = wh;
         }


         /* 角度更新 */
         param->rot += param->rot_add;
         if ( param->rot > PI ) {
            param->rot -= TPI;
         }
         else if ( param->rot < -PI ) {
            param->rot += TPI;
         }


         if ( uvrgbwh->a > ALPHA_MIN + 5 ) {

            param->rad *= 0.985f;

            if ( !work->ext_flag ) {
               param->vec.vy += 0.30f;
            }

            if ( alpha_flag ) {
               uvrgbwh->a = uvrgbwh_pre->a - 1;
            }
            else {
               uvrgbwh->a = uvrgbwh_pre->a;
            }

         }
         else {

            param->rad *= 0.70f;
            if ( !work->ext_flag ) {
               param->vec.vy += 0.10f;
            }
            uvrgbwh->a = uvrgbwh_pre->a - 1;

         }
      }
      /* スプライト初期化 */
      else {

         if ( i > work->cnt - LIFE ) {

            /* 位置初期化 */
            switch ( work->part ) {
            case BODY:
               node = 0 + irnd()%3;		/* 胴 */
               break;
            case L_ARM:
               node = 7 + irnd()%3;		/* 左腕 */
               break;
            case R_ARM:
               node = 3 + irnd()%3;		/* 右腕 */
               break;
            case L_LEG:
               node = 17 + irnd()%3;		/* 左足 */
               break;
            case R_LEG:
               node = 13 + irnd()%3;		/* 右足 */
               break;
            default:
               node = 0;
               ASSERT(0);
               break;
            }

            ftmp = 0.50f + frnd()*0.50f;

            param->node  = node;
            param->param = ftmp;

            _sceVu0ScaleVector( &vectmp, (FVECTOR *)work->body->objs->objs[node].world.m[3], ftmp );
            if ( node == 2 ) {
               _sceVu0ScaleVector( &init_pos, (FVECTOR *)work->body->objs->objs[11].world.m[3], ( 1.0f-ftmp ) );
            }
            else {
               _sceVu0ScaleVector( &init_pos, (FVECTOR *)work->body->objs->objs[node+1].world.m[3], ( 1.0f-ftmp ) );
            }

            _sceVu0AddVector( &init_pos, &init_pos, &vectmp );
            if ( work->part == BODY ) {
               param->rnd_x = 150.0f*frnd();
               param->rnd_z = 150.0f*frnd();
               init_pos.vx += param->rnd_x;
               init_pos.vz += param->rnd_z;
            }

            DG_COPY_VEC( pos, &init_pos );

            /* パラメータ初期化 */
            ftmp = ( 0.5f + frnd() * 0.50f );
            param->rad = work->size * ( 0.5f + ftmp );
            param->rot = PI / 2.0f;

            param->vec.vx = work->rising_spd * 25.0f / 30.0f * frnd();
            param->vec.vy = work->rising_spd * ( 1.0f - ftmp ) / 1.0f + work->rising_spd * 2.f / 3.f; 
            param->vec.vz = work->rising_spd * 25.0f / 30.0f * frnd();
            _sceVu0ScaleVector( &param->vec, &param->vec, 0.50f );

            /* 幅,高さ,アルファ初期化 */
            cos = _approxcos( param->rot );
            uvrgbwh->w = (short)( param->rad * cos );
            uvrgbwh->h = (short)( param->rad * cos );
            uvrgbwh->a = ALPHA_MIN + irnd()%ALPHA_WIDTH;

         }
         else {
            uvrgbwh->w = uvrgbwh->h = 0;
            uvrgbwh->a = 0;
            invisible_num++;
         }

      }

      pos++;
      pos_pre++;
      uvrgbwh++;
      uvrgbwh_pre++;
      param++;

   }


   if ( work->part == BODY ) {
      if ( work->cnt - LIFE < N_SPRT * 3 / 4 ) {
         if ( work->se_count <= 0 ) {
            GM_SeSetMode( SD_A_FIREBOK1, &GM_PlayerPosition, GM_SEMODE_NORMAL );
            work->se_count = DIRECT_TICK( 6 + irnd()%10 );
         }
         else {
            work->se_count--;
         }
      }
   }


   if ( invisible_num >= N_SPRT ) {
      return 0;
   }
   else {
      return 1;
   }


}




static int ActPowder( Work *work )
{

   int			i;
   int	  		clock;

   FVECTOR  		*pos;
   FVECTOR  		*pos_pre;
   DG_PRIM2_UVRGBWH 	*uvrgbwh;
   DG_PRIM2_UVRGBWH 	*uvrgbwh_pre;
   FVECTOR		*vec;

   int			invisible_num;


   /* 操作する頂点バッファ取得 */
   clock = work->prim_powder->buffer_clock;

   pos		= work->prim_powder->pos[clock];
   pos_pre	= work->prim_powder->pos[1-clock];
   uvrgbwh	= work->prim_powder->uvrgb[clock];
   uvrgbwh_pre = work->prim_powder->uvrgb[1-clock];
   vec         = work->vec_powder;


   invisible_num = 0;


   for ( i = 0 ; i < N_POWDER ; i++ ) {

      if ( uvrgbwh_pre->a > 0 ) {
         _sceVu0AddVector( pos, pos_pre, vec );

         pos->vx += vec->vy * 0.50f * frnd();
         pos->vz += vec->vy * 0.50f * frnd();

         vec->vx *= 0.99f;
         vec->vz *= 0.99f;

         uvrgbwh->a = uvrgbwh_pre->a - 1;

      }
      else {
         invisible_num++;
         uvrgbwh->a = 0;
      }

      pos++;
      pos_pre++;
      vec++;
      uvrgbwh++;
      uvrgbwh_pre++;

   }



   if ( work->cnt < LIFE ) {

      if ( work->cnt&1 ) {

         pos     = &work->prim_powder->pos[clock][work->id];
         uvrgbwh = &((DG_PRIM2_UVRGBWH *)work->prim_powder->uvrgb[clock])[work->id];
         vec     = &work->vec_powder[work->id];

         DG_COPY_VEC( pos, (FVECTOR *)work->body->objs->objs[2].world.m[3] );
         pos->vx += work->radius * frnd();
         pos->vz += work->radius * frnd();
         pos->vy += frnd()*1500.0f;

         vec->vy = 100.0f + frnd() * 25.0f;
         vec->vx += vec->vy * 0.20f * frnd();
         vec->vz += vec->vy * 0.20f * frnd();       	

         uvrgbwh->a = POWDER_ALPHA + irnd()%POWDER_ALPHA_WIDTH;

      }

   }

   if ( ++work->id >= N_POWDER ) {
      work->id = 0;
   }

   if ( invisible_num >= N_POWDER ) {
      return 0;
   }
   else {
      return 1;
   }


}




/* アクト関数 */
static void Act( Work *work )
{

   int ret_fire, ret_powder;
  
   RecieveMessage( work );

   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }

   // ARM optimization - was updating twice
#if 0
   ActFire( work );
   ActPowder( work );
#endif

   /* 炎更新 */
   ret_fire = ActFire( work );
   /* 火の粉更新 */
   ret_powder = ActPowder( work );
   
   if ( !ret_fire && !ret_powder ) {
      GV_DestroyActor( work );
   }
   else {    
      work->cnt++;
   }

}




static void Die( Work *work )
{
   /* メモリ解放 */
   if ( work->prim ) work->prim = OK_FreePrim2( work->prim );
   if ( work->prim_powder ) work->prim_powder = OK_FreePrim2( work->prim_powder );

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
   float cos;


   prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
   if ( prim == NULL ) {
      return 0;
   }

   tex = DG_GetTexture( FIRE_TEX );


   prim->raise = 0;

   DG_ConfigPrim2Tex( prim, tex );
   DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );



   pos     = (FVECTOR *)prim->pos[0];
   uvrgbwh = (DG_PRIM2_UVRGBWH *)prim->uvrgb[0];
   param	= work->param;

   for ( i = 0 ; i < N_SPRT ; i++ ){

      DG_COPY_VEC( pos, &DG_ZeroVector );

      switch ( irnd() & 3 ) {
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
      uvrgbwh->a  = 32 + (irnd() & 31);

      param->rad = work->size * ( 1.0f + frnd() * 0.50f );
      param->rot = PI * frnd();
      param->rot_add = TPI * 0.05f * frnd();//TPI * 0.1f * frnd();
      {
         short wh = (short)(param->rad * _approxcos(param->rot));
         uvrgbwh->w = wh;
         uvrgbwh->h = wh;
      }

      DG_COPY_VEC( &param->vec, &DG_ZeroVector );
      param->vec.vx = 100.f * frnd();
      param->vec.vy = 150.f * frnd() + 200.f; 
      param->vec.vz = 100.f * frnd();

      pos++;
      uvrgbwh++;
      param++;

   }	

   memcpy(prim->pos[1], prim->pos[0], sizeof(FVECTOR)*N_SPRT);
   memcpy(prim->uvrgb[1], prim->uvrgb[0], sizeof(DG_PRIM2_UVRGBWH)*N_SPRT);

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
   if ( prim == NULL ) {
      return 0;
   }

   tex = DG_GetTexture( POWDER_TEX );


   prim->raise = 0;

   DG_ConfigPrim2Tex( prim, tex );
   DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );



   pos     = (FVECTOR *)prim->pos[0];
   uvrgbwh = (DG_PRIM2_UVRGBWH *)prim->uvrgb[0];
   vec     = work->vec_powder;


   for ( i = 0 ; i < N_POWDER ; i++ ){

      DG_COPY_VEC( pos, &DG_ZeroVector );

      switch ( i & 3 ) {
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
      uvrgbwh->a  = 64;

      uvrgbwh->w = 0;//work->size;
      uvrgbwh->h = 0;//work->size;

      DG_COPY_VEC( vec, &DG_ZeroVector );

      pos++;
      uvrgbwh++;
      vec++;

   }	

   memcpy(prim->pos[1], prim->pos[0], sizeof(FVECTOR)*N_POWDER);
   memcpy(prim->uvrgb[1], prim->uvrgb[0], sizeof(DG_PRIM2_UVRGBWH)*N_POWDER);

   prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

   return 1;

}



static int GetResources( Work *work, OBJECT *body, int part )
{

   work->name = GM_PLAYER_CHAR_BODY_FLAME;

   work->body = body;
   work->part = part;

   work->life = LIFE;
   work->cnt  = 0;
   work->ext_flag = 0;

   work->size       = FIRE_MAX_SIZE;
   work->radius     = 200.0f;
   work->rising_spd = 10.0f;

   work->powder_cnt = 5;
   work->id = 0;

   DG_COPY_VEC( &work->wind, &DG_ZeroVector );

   work->se_count = 0;

   if ( !( InitPacket( work ) ) ) {
      return (-1);
   }
   if ( !( InitPacket2( work ) ) ) {
      return (-1);
   }

   return 0;

}



/*
NewRisingSmoke
FVECTOR* pos : 発生位置
*/
void *NewBodyFire( OBJECT *body, int part )
{
   Work	*work;

   work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

   if ( work != NULL ) {
      GV_SetActor( &( work->actor ), Act, Die );
      GV_ActorEX( &work->actor );
      if ( GetResources( work, body, part ) < 0 ) {
         GV_DestroyActor( work );
         return NULL;
      }
   }

   return (void *)work ;

}
