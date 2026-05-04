//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	set_t_fc.c
	カメラに平行にテクスチャセット（クロスフェード）
	1999/11/09 S.Okajima
	$Id: set_t_fc.c,v 1.1.1.3 2002/11/19 11:47:12 Yoshizawa1 Exp $
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
#include	"../etc/ok_util.h"

extern int ok_flush_status;
extern float ok_focus_z_far;
extern void OK_LightPos_PutCameraDrop( FVECTOR *fvec );

#define	DUMMY_Z			(5000.0f)
#define	RATIO_MAX		(64.0f)
#define	RATIO_MAX_CORE	(128.0f)

#define	RGB_0			(40)
#define	RGB_1			(128)
#define	RGB_CORE		(120)

/* ok_focus_z_far よりどれだけ奥までに遷移するか */
#define	FADE_RANGE_FAR		(8000.0f)
/* ok_focus_z_far よりどれだけ手前から遷移するか */
#define	FADE_RANGE_NEAR		(-700.0f)

#define	R_POS		(1)
#define	R_UVS		(2)
#define	R_DAT		( R_POS +   R_UVS + R_UVS + R_UVS )

#define N_SET		(3)
#define N_PRIMS2	(1)
#define N_VERTS2	(16)

#define	SCR_POS			(SCRPAD_ADDR)
#define	SCR_UVS0		(SCRPAD_ADDR + 0x1000)
#define	SCR_UVS1		(SCRPAD_ADDR + 0x2000)
#define	SCR_UVS_CORE	(SCRPAD_ADDR + 0x3000)

#define	RAISE_SHIFT		(110.0f)

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;
	int		name;
	int		where;

	int		invisible_flag;

	FVECTOR		center[N_VERTS2];

	int			verts_num;
	DG_PRIM2	*prim0 ;
	DG_PRIM2	*prim1 ;
	DG_PRIM2	*prim_core ;
	float		zoom_max;

	float		radius0 ;
	float		radius1 ;
	float		radius_core ;

	TARGET tgt[N_VERTS2];
	POWER_TARGET pow[N_VERTS2];

	TARGET tgt_bomb[N_VERTS2];
	POWER_TARGET pow_bomb[N_VERTS2];

	int			flag[N_VERTS2];
} Work ;

enum {
	REQ_OFF=0,
	REQ_ON,
};
/*----------------------------------------------------------------*/
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex, FVECTOR *center )
{
	FVECTOR				*pos ;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		i, k ;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos     = SCR_POS ;
	uvrgbwh = SCR_UVS0 ;
	for ( i = 0 ; i < N_PRIMS2 ; i++ ){
		for ( k = 0 ; k < work->verts_num ; k++ ){
			DG_COPY_VEC( pos, center );

			uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh->q0 = 4096 ;
			uvrgbwh->q1 = 4096 ;
			uvrgbwh->f0 = 0x0fff ;
			uvrgbwh->f1 = 0x0fff ;

			uvrgbwh->w = 0;
			uvrgbwh->h = 0;

			uvrgbwh->r = 0 ;
			uvrgbwh->g = 0 ;
			uvrgbwh->b = 0 ;
			uvrgbwh->a = 0 ;

			center++;
			pos ++ ;		/* 同一プリミティブではデータは連続している */
			uvrgbwh ++ ;
		}
	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS,  sizeof(FVECTOR),          work->verts_num * N_PRIMS2 );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS,  sizeof(FVECTOR),          work->verts_num * N_PRIMS2 );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS0, sizeof(DG_PRIM2_UVRGBWH), work->verts_num * N_PRIMS2 );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS0, sizeof(DG_PRIM2_UVRGBWH), work->verts_num * N_PRIMS2 );

	return 1;
}

/*----------------------------------------------------------------*/
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
		  default:
			break;
		}
		msg--;
	}
}


/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int	i, clock, num_verts;
	FVECTOR		disp_center;
	float	physic_raise;
	float	change_start;
	float	change_end;
	float	len;
	float	zoom;
	float	radius0_x;
	float	radius0_y;
	float	radius1_x;
	float	radius1_y;
	float	cf,sf;
	float	ratio;
	float	screen_near_x;
	float	screen_near_y;
	float	angle;

	FVECTOR		*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh0		;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1		;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh_core	;	/* スプライト用 */

	CheckMesgParam( work );

	if( ok_flush_status!=0 || work->invisible_flag==1 ){
		DG_InvisiblePrim2( work->prim0 ) ;
		DG_InvisiblePrim2( work->prim1 ) ;
		DG_InvisiblePrim2( work->prim_core ) ;
		return;
	}else{
		DG_VisiblePrim2( work->prim0 ) ;
		DG_VisiblePrim2( work->prim1 ) ;
		DG_VisiblePrim2( work->prim_core ) ;
	}

	angle = DG_Chanls->screen;

	DG_COPY_VEC( &disp_center, (FVECTOR *)DG_Chanls->eye.m[3] );



	if( ok_focus_z_far!=0.0f ){
		change_start = ok_focus_z_far - FADE_RANGE_NEAR;
		if(change_start < 1.0f) change_start = 1.0f;
		change_end   = ok_focus_z_far + FADE_RANGE_FAR;
	}else{
		/* キャラ：Ｚ制御が起動していないとき */
		change_start = DUMMY_Z - FADE_RANGE_NEAR;
		if(change_start < 1.0f) change_start = 1.0f;
		change_end   = DUMMY_Z + FADE_RANGE_FAR;
	}

//画角とぼやけ具合をリンク
	change_start *= (angle*0.5f);
	change_end   *= (angle*0.5f);

//	len=GV_VecLen3F( &disp_center ) * (1.0f/2048.0f) + angle*0.08f;	/* 対象はなんでもいい？ */
	len=GV_VecLen3F( &disp_center ) * (1.0f/1800.0f) + angle*0.08f;	/* 対象はなんでもいい？ */
	cf = cosf( len );
	sf = sinf( len );
	radius0_x = work->radius0 * cf;
	radius0_y = work->radius0 * sf;
	radius1_x = work->radius1 * cf;
	radius1_y = work->radius1 * sf;

	DG_SwitchBuffPrim2( work->prim0 );
	DG_SwitchBuffPrim2( work->prim1 );
	DG_SwitchBuffPrim2( work->prim_core );
	clock = work->prim0->buffer_clock;
	num_verts = work->verts_num * N_PRIMS2;
	OK_Mem_Scr( SCR_POS,      work->center,                  sizeof(FVECTOR),          num_verts ) ;
	OK_Mem_Scr( SCR_UVS0,     work->prim0->uvrgb[clock],     sizeof(DG_PRIM2_UVRGBWH), num_verts ) ;
	OK_Mem_Scr( SCR_UVS1,     work->prim1->uvrgb[clock],     sizeof(DG_PRIM2_UVRGBWH), num_verts ) ;
	OK_Mem_Scr( SCR_UVS_CORE, work->prim_core->uvrgb[clock], sizeof(DG_PRIM2_UVRGBWH), num_verts ) ;

	pos          = SCR_POS;
	uvrgbwh0     = SCR_UVS0;
	uvrgbwh1     = SCR_UVS1;
	uvrgbwh_core = SCR_UVS_CORE;

	screen_near_x = (ASPECT_X()                                           ) * angle;
	screen_near_y = (ASPECT_Y() * DG_Chanls->width / DG_Chanls->height) * angle;

	for(i=0; i<work->verts_num; i++){
		/* 座標を透視変換する */
		DG_SetPos( &DG_Chanls->eye_pers_no_offset );
		DG_PutVector( pos, pos, 1 );

		if( pos->vz < pos->vw  &&  pos->vw > RAISE_SHIFT + 51.0f ){

			if(pos->vw > change_end){
				ratio=RATIO_MAX;
			}else if(pos->vw > change_start ){
				ratio=(pos->vw - change_start)*(float)RATIO_MAX/(change_end - change_start);
			}else{
				ratio=0.0f;		/* フラグとしても使用 */
			}

//if(GV_Time%120==0)printf("%f:%f:%f:%f:\n",pos->vx,pos->vy,pos->vz,pos->vw);

//			pos->vw = (pos->vw  > 0.0f)? pos->vw: -pos->vw;
			physic_raise = pos->vw - RAISE_SHIFT;
#if 1
			pos->vz = physic_raise;
			pos->vx = physic_raise * pos->vx / screen_near_x / pos->vw;
			pos->vy = physic_raise * pos->vy / screen_near_y / pos->vw;
#else
			pos->vz = physic_raise;
			ftemp0 = pos->vx / pos->vw;
			ftemp1 = pos->vy / pos->vw;
			if( ftemp0 > 2.0f || ftemp0 < -2.0f || ftemp1 > 2.0f || ftemp1 < -2.0f ){
				continue;
			}
			pos->vx = physic_raise * pos->vx / screen_near_x / pos->vw;
			pos->vy = physic_raise * pos->vy / screen_near_y / pos->vw;
#endif

			/* カメラ座標をワールド座標に変換 */
			DG_SetPos( &DG_Chanls->eye );
			DG_PutVector( pos, pos, 1 );

			zoom = work->zoom_max
			              + (1.0f-work->zoom_max) * (RATIO_MAX - ratio) / RATIO_MAX;

			physic_raise *= (1.0f/2048.0f);

			uvrgbwh0->w     = (int)(radius0_x * zoom);
			uvrgbwh0->h     = (int)(radius0_y * zoom);
			uvrgbwh0->r = uvrgbwh0->g = uvrgbwh0->b = RGB_0;
			uvrgbwh1->w     = (int)(radius1_x * zoom);
			uvrgbwh1->h     = (int)(radius1_y * zoom);
			uvrgbwh1->r = uvrgbwh1->g = uvrgbwh1->b = RGB_1;
			uvrgbwh_core->w = uvrgbwh_core->h = (int)(work->radius_core * zoom);
			uvrgbwh_core->r = uvrgbwh_core->g = uvrgbwh_core->b = RGB_CORE;

			if(work->flag[i]){
//				uvrgbwh0->a     = (int)(ratio*0.8f);
				uvrgbwh0->a     = (int)(ratio*2.0f);
				uvrgbwh1->a     = (int)(RATIO_MAX-ratio);
				uvrgbwh_core->a = (int)(RATIO_MAX_CORE*(RATIO_MAX-ratio)/RATIO_MAX);
			}else{
				uvrgbwh0->a     = 0;
				uvrgbwh1->a     = 0;
				uvrgbwh_core->a = 0;
			}
		}else{
			uvrgbwh0->w = 0;
			uvrgbwh0->h = 0;
			uvrgbwh1->w = 0;
			uvrgbwh1->h = 0;
		}


		pos++;
		uvrgbwh0++;
		uvrgbwh1++;
		uvrgbwh_core++;
	}

	OK_Scr_Mem( work->prim0->pos[clock],       SCR_POS,      sizeof(FVECTOR),          num_verts ) ;
	OK_Scr_Mem( work->prim1->pos[clock],       SCR_POS,      sizeof(FVECTOR),          num_verts ) ;
	OK_Scr_Mem( work->prim0->uvrgb[clock],     SCR_UVS0,     sizeof(DG_PRIM2_UVRGBWH), num_verts ) ;
	OK_Scr_Mem( work->prim1->uvrgb[clock],     SCR_UVS1,     sizeof(DG_PRIM2_UVRGBWH), num_verts ) ;
	OK_Scr_Mem( work->prim_core->uvrgb[clock], SCR_UVS_CORE, sizeof(DG_PRIM2_UVRGBWH), num_verts ) ;

//	uvrgbwh0     = work->prim0->uvrgb[clock];
//printf("e:%x:%d %d %d\n",uvrgbwh0,uvrgbwh0->w,uvrgbwh0->h,uvrgbwh0->a);

}

static void Die( Work *work )
{
        int i ;

	work->prim0 = OK_FreePrim2( work->prim0 );
	work->prim1 = OK_FreePrim2( work->prim1 );
	work->prim_core = OK_FreePrim2( work->prim_core );

	for ( i=work->verts_num ; --i>=0 ; ){
		if( work->flag[i] == 1 ){
			GM_FreeTarget( &work->tgt[i] ) ;
			GM_FreeTarget( &work->tgt_bomb[i] ) ;
		}
	}
}

static void TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
	Work        *work ;
	extern void *NewBreakLight( FVECTOR *, FVECTOR *, int  ) ;
	int         i ;

	work = (Work *)ptr ;
	if ( def->damaged & TARGET_POWER ){
		for ( i=0; i<work->verts_num; i++ ){
			if( work->flag[i]==1 ){
				if( (&work->tgt[i] == def      && (def->weapon_type & (WP_BULLET|WP_M92)))
				 || (&work->tgt_bomb[i] == def && (def->weapon_type & WP_BLAST))  ){
					GM_FreeTarget( &work->tgt[i] ) ;
					GM_FreeTarget( &work->tgt_bomb[i] ) ;
					work->flag[i]=0;
					if( off->power!=NULL ){
						NewBreakLight( &def->offset, &off->power->force, work->where ) ;
					}else{
						NewBreakLight( &def->offset, &DG_ZeroVector, work->where ) ;
					}
					GM_SeSetMode( SD_A_LAMP02, &def->hit, GM_SEMODE_BOMB ) ;
					GM_ClearTargetDamage( &work->tgt[i] ) ;
					GM_ClearTargetDamage( &work->tgt_bomb[i] ) ;

//					DG_DestroyLightSphere( &work->center[i], 500.0f ) ;

				}
			}
		}
	}
}


static int GetResources( Work *work, int name, int where )
{
	int	i;
	int	tex_name;
	FVECTOR		fvtemp[2];
	DG_PRIM2	*prim ;
	DG_TEX		*tex0 ;
	DG_TEX		*tex1 ;
	DG_TEX		*tex_core ;

	work->name = name;
	work->where = where;

	tex0=NULL ;
	tex1=NULL ;
	tex_core=NULL ;

	if ( GCL_GetOption( 't' ) != NULL ) {
		if( GCL_NextStr() != NULL ){
			tex_name       = GCL_GetNextInt();
			tex0 = DG_GetTexture( tex_name );
		}
	}

	if ( GCL_GetOption( 'u' ) != NULL ) {
		if( GCL_NextStr() != NULL ){
			tex_name       = GCL_GetNextInt();
			tex1 = DG_GetTexture( tex_name );
		}
	}

	if ( GCL_GetOption( 'v' ) != NULL ) {
		if( GCL_NextStr() != NULL ){
			tex_name       = GCL_GetNextInt();
			tex_core = DG_GetTexture( tex_name );
		}
	}


	if ( GCL_GetOption( 's' ) != NULL ) {
		if( GCL_NextStr() != NULL ){
			work->radius0     = (float)GCL_GetNextInt()*2.0f ;
			work->radius1     = (float)GCL_GetNextInt()*2.0f ;
			work->radius_core = (float)GCL_GetNextInt()*2.0f ;
		}
	}

	work->verts_num=0;
	if ( GCL_GetOption( 'p' ) != NULL ) {
		for(i=0; i<N_VERTS2; i++){
			if( GCL_NextStr() != NULL ){
				work->center[i].vx   = (float)GCL_GetNextInt() ;
				work->center[i].vy   = (float)GCL_GetNextInt() ;
				work->center[i].vz   = (float)GCL_GetNextInt() ;

/*
				{
					void *NewSideFlare_WorldAuto(
						FVECTOR *pos_world,	// 死ぬまで参照し続ける
						u_char	max_alpha
					);
					NewSideFlare_WorldAuto(
						&work->center[i],	// 死ぬまで参照し続ける
						64
					);
				}
*/

				OK_LightPos_PutCameraDrop( &work->center[i] );

				fvtemp[0].vx = fvtemp[0].vz = fvtemp[0].vy = 90.0F ;
				GM_SetTarget( &work->tgt[i], TARGET_DEFENSE|TARGET_POWER, where, BOTH_SIDE, &fvtemp[0], &work->center[i] ) ;
				GM_SetPowerTarget( &work->tgt[i], &work->pow[i], POWER_ONCE, 255, 0, 0, &work->center[i] ) ;
				GM_SetTargetCallBack( &work->tgt[i], TargetCallBack, work ) ;
				GM_PutTarget( &work->tgt[i] ) ;
//				NewTargetView( &work->tgt[i], 255, 0, 0 ) ;

				fvtemp[0].vx = fvtemp[0].vz = fvtemp[0].vy = 1500.0F ;
				GM_SetTarget( &work->tgt_bomb[i], TARGET_DEFENSE|TARGET_POWER|TARGET_THROUGH, where, BOTH_SIDE, &fvtemp[0], &work->center[i] ) ;
				GM_SetPowerTarget( &work->tgt_bomb[i], &work->pow_bomb[i], POWER_ONCE, 255, 0, 0, &work->center[i] ) ;
				GM_SetTargetCallBack( &work->tgt_bomb[i], TargetCallBack, work ) ;
				GM_PutTarget( &work->tgt_bomb[i] ) ;
//				NewTargetView( &work->tgt_bomb[i], 0, 0, 255 ) ;

				work->flag[i]=1;
			}else{
				break;
			}
		}
		work->verts_num=i;
	}

	work->zoom_max=1.0f;
	if ( GCL_GetOption( 'z' ) != NULL ) {
		if( GCL_NextStr() != NULL ){
			work->zoom_max = (float)GCL_GetNextInt() / 4096.0f;
		}
	}

	/* 回転スプライト */
	prim = work->prim0 = GM_MakePrim2(
				DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
				N_PRIMS2,
				work->verts_num );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex0, work->center );
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	/* 回転スプライト */
	prim = work->prim1 = GM_MakePrim2(
				DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
				N_PRIMS2,
				work->verts_num );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex1, work->center );
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	/* 固定スプライト */
	prim = work->prim_core = GM_MakePrim2(
				DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
				N_PRIMS2,
				work->verts_num );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex_core, work->center );
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	return 0 ;
}

void *NewPutWorldTexFadeCamera(  int name, int where  )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, name, where  ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
