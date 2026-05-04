//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	blood_sp.c
	負傷したキャラが垂らした後、床に拡がる血
	1999/09/22 S.Okajima
	$Id: blood_sp.c,v 1.1.1.3 2002/11/19 11:46:59 Yoshizawa1 Exp $
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
#include	"blood.h"
#include	"../etc/ok_util.h"

//#define	COLOR_R			(16)
//#define	COLOR_G			(4)
//#define	COLOR_B			(4)
#define	COLOR_R2			(32)
#define	COLOR_G2			(12)
#define	COLOR_B2			(12)


extern void OK_DirVecXY( FVECTOR *from, FVECTOR *to, SVECTOR *rot );
extern void *NewBloodSpread( int name, int map );
extern int CheckRainBoundary( FVECTOR *pos );

static	int	OK_blood_spread_se_count = 0;
static	float Ratio = 0.0f;

/*----------------------------------------------------------------*/
#define	MAX_TEX		(5)

//#define	RAISE			(-4000)
#define	RAISE			(-60000)

/* 固定 */
#define	MAX_TEX			(5)
//#define	N_VERTS			(64)
#define	N_VERTS			(32)
#define	N_PRIMS			(2)
#define	N_POLYS			(N_VERTS*N_PRIMS/4)

//#define	COLOR_A			(120)
#define	COLOR_A			(100)
#define	DECAY_START		(60)

//#define	SHIFT_CENTER	(1.0f)
#define	SHIFT_CENTER	(16.0f)

#define	R_POS		(1)
#define	R_UVS		(1)

#define	SCR_LENGTH	( 0x4000 )
#define	MAX_FVECTOR	( SCR_LENGTH/16 )

#define	PARAM1		(MAX_FVECTOR / (R_POS + R_UVS  +  R_POS + R_UVS))
#define	PARAM2		(PARAM1 * 16)

#define	SCR_POS0	(SCRPAD_ADDR)
#define	SCR_POS1	(SCR_POS0 + PARAM2)
#define	SCR_UVS0	(SCR_POS1 + PARAM2 )
#define	SCR_UVS1	(SCR_UVS0 + PARAM2 + PARAM2 )

typedef	struct	{
	DG_PRIM2			*prim ;
	int 				sc_life[N_POLYS];
	int 				sc_life_max[N_POLYS];
	FVECTOR				sc_pos[N_POLYS*4];
} Unit ;

typedef	struct	{
	GV_ACT_EX		actor ;
	int			map ;

	int			init_check_count;

	int			prim_count;
	int			next_number;

	ENEFINDLIST	blood_efl ;
	ENEFIND		enefind[ N_POLYS*MAX_TEX ] ;
	int			list_parts_flag[ N_POLYS*MAX_TEX ];
	int			list_flag;
	int			before_num;

	Unit		unit[MAX_TEX];
} Work ;

Work	*OK_BLOOD_SPREAD_WORK = NULL;

/*----------------------------------------------------------------*/
// 床面を消しても血が残る問題用
static void BoundCallBack( Work *work )
{
	if ( !GM_CheckGameStatus( STATE_DEMO ) ) {
		work->init_check_count = 2;
	}
}

/*----------------------------------------------------------------*/
// 床面を消しても血が残る問題用
static void CheckAndClear( Work *work, int clock )
{
	int i,j ;
	int	flag;
	FVECTOR	*keep_p;
	FVECTOR	*fv_p;
	FVECTOR	fvtemp;

	work->init_check_count--;

	for( i=0; i<MAX_TEX; i++ ){
		fv_p = work->unit[i].prim->pos[clock];
		for( j=0; j<N_POLYS; j++ ){
			keep_p = fv_p;
			flag = 0;
			if ( UTL_EFT_CheckBound( fv_p++ ) >= 0 ){	/*バウンドに入る？*/
				flag = 1;
			}
			if ( UTL_EFT_CheckBound( fv_p++ ) >= 0 ){	/*バウンドに入る？*/
				flag = 1;
			}
			if ( UTL_EFT_CheckBound( fv_p++ ) >= 0 ){	/*バウンドに入る？*/
				flag = 1;
			}
			if ( UTL_EFT_CheckBound( fv_p++ ) >= 0 ){	/*バウンドに入る？*/
				flag = 1;
			}
			if( flag ){
				DG_COPY_VEC( &fvtemp, keep_p++ );
				DG_COPY_VEC( keep_p++, &fvtemp );
				DG_COPY_VEC( keep_p++, &fvtemp );
				DG_COPY_VEC( keep_p++, &fvtemp );
			}
		}
	}
}

/*----------------------------------------------------------------*/
static void EneListFree( Work *work )
{
	if( work->list_flag ){
		GM_FreeEneFindList( &work->blood_efl );
		work->list_flag = 0;
	}
}

/*----------------------------------------------------------------*/
static void EneListSet( Work *work )
{
	if( !work->list_flag ){
		GM_InitEneFindList( &work->blood_efl, EF_LIST_TYPE_LINK|EF_LIST_TYPE_BLOOD ) ;
		GM_PutEneFindList( &work->blood_efl ) ;
		work->list_flag = 1;
	}
}

/*----------------------------------------------------------------*/
static void EneListPartFree( Work *work, int num )
{
	int	*flag;

	flag = &work->list_parts_flag[ num ];
	if( (*flag) && (work->list_flag) ){
		GM_FreeEneFindFromList( &work->blood_efl, &work->enefind[ num ] ) ;
		*flag = 0;
	}
}

/*----------------------------------------------------------------*/
static void EneListPartSet( Work *work, FVECTOR *fvtemp, int addr, int num )
{
	int	*flag;
	int		type;

	if( GM_CheckPlayerStatus(PLAYER_NARROW) ) return ;

	type = 0;
	if( GM_CheckPlayerStatus(PLAYER_INTRUDE) ) type |= EF_TYPE_INTRUDE;
	if( GM_CheckPlayerStatus(PLAYER_LOCKER ) ) type |= EF_TYPE_LOCKER;

	flag = &work->list_parts_flag[ num ];
	if( !(*flag) && (work->list_flag) ){
		GM_SetEneFind( &work->enefind[ num ], fvtemp, addr, type ) ;
		GM_PutEneFindToList( &work->blood_efl, &work->enefind[ num ] ) ;
		*flag = 1;
	}
}

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int	i,j;
	int		alpha=0;
	FVECTOR	*pos;
	FVECTOR	*sc_pos;
	DG_PRIM2_UVRGB		*uvrgb ;
	int		*life;
	int		*life_max;
	Unit	*unit;
	int		clock;

	unit  = work->unit;
   //AR_PARTICLE_HALF
   if(!AS_WillPrimBuffSwitch())
   {
      return;
   }
	clock = 1 - unit->prim->buffer_clock;

	if( work->init_check_count ) CheckAndClear( work, 1-clock );

	for(i=0; i<MAX_TEX; i++){
		DG_SwitchBuffPrim2( unit->prim );
		GM_GroupPrim2( unit->prim, GM_CurrentStageMap ) ;
		life     = unit->sc_life;
		life_max = unit->sc_life_max;
		uvrgb  = (DG_PRIM2_UVRGB *)unit->prim->uvrgb[clock];
		pos    = unit->prim->pos[clock];
		sc_pos = unit->sc_pos;
		for ( j=0; j<N_POLYS; j++ ){
			if( (*life) > 0 ){
				if( (*life_max) - (*life) < 2 ){
					DG_COPY_VEC( pos, sc_pos );	pos++;	sc_pos++;
					DG_COPY_VEC( pos, sc_pos );	pos++;	sc_pos++;
					DG_COPY_VEC( pos, sc_pos );	pos++;	sc_pos++;
					DG_COPY_VEC( pos, sc_pos );	pos++;	sc_pos++;
				}else{
					pos    += 4;
					sc_pos += 4;
				}
				(*life)--;
				if( (*life) < DECAY_START ){
					alpha = COLOR_A * (*life) / DECAY_START;
				}else{
					alpha = COLOR_A;
				}
			}else{
				EneListPartFree( work, j*MAX_TEX + i );
				alpha = 0;
				pos    += 4;
				sc_pos += 4;
			}
			uvrgb->a = alpha;	uvrgb++;
			uvrgb->a = alpha;	uvrgb++;
			uvrgb->a = alpha;	uvrgb++;
			uvrgb->a = alpha;	uvrgb++;
			life++;
			life_max++;
		}
		unit++;
	}

	if( OK_blood_spread_se_count > 0 ){
		OK_blood_spread_se_count--;
	}
}

static void Die( Work *work )
{
	int	i;

	UTL_EFT_DelCallback( work ) ;

	EneListFree( work );

	for( i=0; i<MAX_TEX; i++ ){
		work->unit[i].prim = OK_FreePrim2( work->unit[i].prim );
	}
	OK_BLOOD_SPREAD_WORK = NULL;
}

/*----------------------------------------------------------------*/
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR				*pos ;
	DG_PRIM2_UVRGB		*uvrgb ;
	int		i, k ;

	DG_ConfigPrim2Tex( prim, tex );
	if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/04/03 */
	{
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	}
	else
	{
//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 2, 0x00 ) );
	}

	prim->raise = RAISE;

	pos     = SCR_POS0 ;
	uvrgb   = SCR_UVS0 ;
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		for ( k = 0 ; k < N_POLYS ; k++ ){
			pos->vx = 0.0f;
			pos->vy = 0.0f;
			pos->vz = 0.0f;
			pos++;

			pos->vx = 0.0f;
			pos->vy = 0.0f;
			pos->vz = 0.0f;
			pos++;

			pos->vx = 0.0f;
			pos->vy = 0.0f;
			pos->vz = 0.0f;
			pos++;

			pos->vx = 0.0f;
			pos->vy = 0.0f;
			pos->vz = 0.0f;
			pos++;


			uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x8fff ;
			uvrgb->r = COLOR_R2 ;
			uvrgb->g = COLOR_G2 ;
			uvrgb->b = COLOR_B2 ;
			uvrgb->a = COLOR_A ;
			uvrgb++;

			uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x8fff ;
			uvrgb->r = COLOR_R2 ;
			uvrgb->g = COLOR_G2 ;
			uvrgb->b = COLOR_B2 ;
			uvrgb->a = COLOR_A ;
			uvrgb++;

			uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x0fff ;
			uvrgb->r = COLOR_R2 ;
			uvrgb->g = COLOR_G2 ;
			uvrgb->b = COLOR_B2 ;
			uvrgb->a = COLOR_A ;
			uvrgb++;

			uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x0fff ;
			uvrgb->r = COLOR_R2 ;
			uvrgb->g = COLOR_G2 ;
			uvrgb->b = COLOR_B2 ;
			uvrgb->a = COLOR_A ;
			uvrgb++;
		}
	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS0, sizeof(FVECTOR),        N_VERTS * N_PRIMS ) ;
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS0, sizeof(FVECTOR),        N_VERTS * N_PRIMS ) ;
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS0, sizeof(DG_PRIM2_UVRGB), N_VERTS * N_PRIMS ) ;
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS0, sizeof(DG_PRIM2_UVRGB), N_VERTS * N_PRIMS ) ;

	return 1;
}
/*----------------------------------------------------------------*/
static int GetResources( Work *work )
{
	int	i,j;
	DG_PRIM2		*prim ;
	DG_TEX		*tex[MAX_TEX] ;

#if 0
	tex[0] = DG_GetTexture( 10984814 /*"chi01_msk"*/ );
	tex[1] = DG_GetTexture( 12033390 /*"chi02_msk"*/ );
	tex[2] = DG_GetTexture( 13081966 /*"chi03_msk"*/ );
	tex[3] = DG_GetTexture( 14130542 /*"chi04_msk"*/ );
	tex[4] = DG_GetTexture( 15179118 /*"chi05_msk"*/ );
#else
	if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/04/03 */
	{
	tex[0] = DG_GetTexture(  9373287 /*"vr2_chi01_alp"*/ );
	tex[1] = DG_GetTexture( 10421863 /*"vr2_chi02_alp"*/ );
	tex[2] = DG_GetTexture( 11470439 /*"vr2_chi03_alp"*/ );
	tex[3] = DG_GetTexture( 12519015 /*"vr2_chi04_alp"*/ );
	tex[4] = DG_GetTexture( 13567591 /*"vr2_chi05_alp"*/ );
	}
	else
	{
	tex[0] = DG_GetTexture( 10972307 /*"chi01_alp"*/ );
	tex[1] = DG_GetTexture( 12020883 /*"chi02_alp"*/ );
	tex[2] = DG_GetTexture( 13069459 /*"chi03_alp"*/ );
	tex[3] = DG_GetTexture( 14118035 /*"chi04_alp"*/ );
	tex[4] = DG_GetTexture( 15166611 /*"chi05_alp"*/ );
	}
#endif


	for( i=0; i<MAX_TEX; i++ ){
		prim = work->unit[i].prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
		if(prim==NULL){
			printf("null prim\n");
			return -1;
		}
		InitPacket2( work, prim, tex[i] );

		for(j=0; j<N_POLYS; j++){
			work->unit[i].sc_life[j]      = 0;
			work->unit[i].sc_life_max[j]  = 1;
		}
	}

	work->prim_count=0;
	work->next_number=work->prim_count+1;

	work->list_flag  = 0;

	UTL_EFT_AddCallback( BoundCallBack, work ) ;

	return 0 ;
}

/* floor に プリミティブを張る */
static	void	SetPrims( FVECTOR *prim_pos, FVECTOR *point_pos, SVECTOR *point_rot, float sp_size )
{
	float	shift;
	SVECTOR	temp_rot;
	FVECTOR	size[4];
//	Unit	*unit;

//	unit  = OK_BLOOD_SPREAD_WORK->unit;
//	unit += OK_BLOOD_SPREAD_WORK->prim_count;

	temp_rot.vx=0;
	temp_rot.vy=0;
	temp_rot.vz=(short)(irnd() % 4096);

	if( point_rot->vx > 2048 ){
		shift =  SHIFT_CENTER;
	}else{
		shift = -SHIFT_CENTER;
	}

	size[0].vx = - sp_size;
	size[0].vy =   sp_size;
	size[0].vz =   shift;
	size[1].vx =   sp_size;
	size[1].vy =   sp_size;
	size[1].vz =   shift;
	size[2].vx = - sp_size;
	size[2].vy = - sp_size;
	size[2].vz =   shift;
	size[3].vx =   sp_size;
	size[3].vy = - sp_size;
	size[3].vz =   shift;

	DG_SetPos2( &DG_ZeroVector, &temp_rot );
	DG_PutVector( size, prim_pos, 4 );
	DG_SetPos2( point_pos, point_rot );
	DG_PutVector( prim_pos, prim_pos, 4 );
}

void	BloodSpread( FVECTOR *pos, SVECTOR *rot, int life, float size, int addr )
{
	Work	*work;
	int	num0;
	int	num1;
	FVECTOR	fvtemp0;
	float	len;
	float	calc_ratio=0.0f;

	life = DIRECT_TICK( life );
	if( life <= 0 ) life = 1;

	if( OK_BLOOD_SPREAD_WORK==NULL ){
		if( NewBloodSpread( 0, 0 )==NULL ){
			printf("OK_BLOOD_SPREAD_WORK was NULL\n");
			return;
		}
	}
	work = OK_BLOOD_SPREAD_WORK;

	if( addr!=-1 ){
		/* 敵発見用リンクリスト登録 */
		EneListSet( work );
		/* 敵発見用リンクリストに個々のデータをセット */
		EneListPartFree( work, work->prim_count );
		EneListPartSet( work, pos, addr, work->prim_count );
	}



	num0 = work->prim_count % MAX_TEX;
	num1 = work->prim_count / MAX_TEX;
	DG_COPY_VEC( &fvtemp0, &work->unit[num0].sc_pos[num1*4] );

	work->prim_count++;
	if( work->prim_count  >= MAX_TEX*N_POLYS ) work->prim_count=0;
	work->next_number=work->prim_count+1;
	if( work->next_number >= MAX_TEX*N_POLYS ) work->next_number=0;

	_sceVu0SubVector( &fvtemp0, &fvtemp0, pos ) ;
	len = GV_VecLen3F( &fvtemp0 );

	num0 = work->prim_count % MAX_TEX;
	num1 = work->prim_count / MAX_TEX;

	if( CheckRainBoundary( pos ) ) life /= 4;

	work->unit[num0].sc_life[num1]     = life;
	work->unit[num0].sc_life_max[num1] = life;

	if( len < size*3.0f ){	//1.414f*2.0f 以上
		Ratio *= 1.05f;
		if( Ratio > 2.0f ) Ratio = 2.0f;
		calc_ratio = Ratio*rnd();
	}else{
		Ratio  = 1.0f;
		calc_ratio = Ratio;
	}

	SetPrims( &work->unit[num0].sc_pos[num1*4], pos, rot, size*calc_ratio*0.75f );




	if( OK_blood_spread_se_count <= 0 ){
		OK_blood_spread_se_count = 10;
		GM_SeSetMode( SD_E_BLOOD_S1, pos, GM_SEMODE_NORMAL );	/* ぽた血音 */
	}

//printf(":::::%d %d\n",work->prim_count,work->next_number);

}


void *NewBloodSpread( int name, int map )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		OK_BLOOD_SPREAD_WORK = work;
	}
	return (void *)work ;
}

