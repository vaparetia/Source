//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	line_box.c
	Sthinger 用 Line Box
	2001/05/19 S.Kobayashi
	$Id: line_box.c,v 1.4 2002/11/23 12:28:14 Yoshizawa1 Exp $

*/
#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>


#include	"gameheader.h"
#include	"libmt.h"
#include    "libutl.h"
#include	"../test/etc.h"
#include	"../../okajima/etc/ok_util.h"

#define LINE_FLAG ( DG_PRIM2_LINE | DG_PRIM2_SHADE | DG_PRIM2_ANTIALIASING )
#define N_VERTS ( 10 + 8 ) 

#define	RGB_SMOKE		(255)
#define	ALPHA_MIN		(6.0f)
#define	ALPHA_RND		(6.0f)
#define	ALPHA_SUB		( (float)(ALPHA_MIN+ALPHA_RND)/(float)(N_PRIMS*N_VERTS)*MULTIPLE )

#define	RAISE_SMOKE			(1000)

#define MEM_ADDR1	((void*)( SCRPAD_ADDR + 0x0000 ))
#define MEM_ADDR2	((void*)( SCRPAD_ADDR + 0x2000 ))

#define WIND_MAX ( 5.f )

#define VERTEX_PLUS ( 5 )


/*----------------------------------------------------------------*/
typedef	struct	{
	int			count;
	FVECTOR		vec;
	float		alpha;
	float       alpha_sub;
	float       sign; // 回転の方向
} Unit ;

typedef	struct _work {
	GV_ACT_EX		actor ;

	CONTROL		*pCtrl;
	FVECTOR		shift;
	FVECTOR     power;
	FVECTOR		before_pos;

	DG_PRIM2	*prim ;
	DG_TEX      *tex;
	int         num;
	float       alpha;
	int        disp_num; //ポーズ中を考慮して	
	void (*act)( struct _work * );
} Work ;

static float sign_x[ N_VERTS ] = { -1 , -1 , 1 , 1 }; 
static float sign_y[ N_VERTS ] = {  1 , -1 ,-1 , 1 }; 
static int point[5] = { 0 , 1 , 2 , 3 , 0 };

extern void	DG_TransPersOne( FVECTOR * , FVECTOR * );

/*----------------------------------------------------------------*/
// ミサイルに数字を付ける 
static void MissileNum( Work *pWork , FVECTOR *pPos )
{
	FVECTOR disp_pos;
	FVECTOR shift;

	if ( pPos == NULL ){
		return; 
	}
	shift = DG_ZeroVector;
	shift.vx = 40.0f;
	shift.vy = 40.0f;
	DG_TransPersOne( &disp_pos , pPos );
	disp_pos.vx += shift.vx;
	disp_pos.vy += shift.vy;
	disp_pos.vy = disp_pos.vy * 384.0F / ( float )DRAW_HEIGHT;

	MENU_S_Locate( disp_pos.vx , disp_pos.vy , 2 );
	MENU_S_Color( 200 , 240 , 120 , 32 );
    if ( !( GV_PauseLevel & ( GV_PAUSE_PAUSE | GV_PAUSE_MENU ) ) ){
		pWork->disp_num = irnd() % 10000;
	}
	MENU_S_Printf( "%4d" , pWork->disp_num );
}

static void NormalAct( Work *pWork )
{
	// 位置の更新
	DG_PRIM2_UVRGB *uvrgb; // Line用
	FMATRIX   fmtmp;
	FVECTOR    *pos;
	FVECTOR dx , dy , dz;
	FVECTOR   fvtmp;
	int       clock;
	int           i;

	DG_VisiblePrim2( pWork->prim ) ;
	GM_GroupPrim2( pWork->prim, GM_CurrentStageMap ) ;

	DG_SwitchBuffPrim2( pWork->prim );
	clock = pWork->prim->buffer_clock;

	pos = pWork->prim->pos[ clock ];
	uvrgb = pWork->prim->uvrgb[ clock ];
	// matrixの算出
	DG_SetPos2( &pWork->pCtrl->mov , &pWork->pCtrl->rot );
	DG_GetPos( &fmtmp );
	// 縦横default の算出
	DG_COPY_VEC( &dx , ( FVECTOR * )&fmtmp.m[ 0 ] );
	DG_COPY_VEC( &dy , ( FVECTOR * )&fmtmp.m[ 2 ] );
	DG_COPY_VEC( &dz , ( FVECTOR * )&fmtmp.m[ 1 ] );
	// scale
	_sceVu0ScaleVector( &dx , &dx , 300.0f );
	_sceVu0ScaleVector( &dy , &dy , 300.0f );
	_sceVu0ScaleVector( &dz , &dz , 300.0f );
	// 比率計算
	{
		float raute;

		if ( GM_PlayerSubWeaponBody != NULL ){
			_sceVu0SubVector( &fvtmp , &pWork->pCtrl->mov , ( FVECTOR * )GM_PlayerSubWeaponBody->objs->world.m[ 3 ] );
			raute = GV_VecLen3F( &fvtmp );
			raute /= 10000.0f;
			if ( raute < 1.0f ){
				raute = 1.0f;
			} else {
				_sceVu0ScaleVector( &dx , &dx , raute );
				_sceVu0ScaleVector( &dy , &dy , raute );
				_sceVu0ScaleVector( &dz , &dz , raute );
			}
		}
	}
	for ( i = 0 ; i < N_VERTS - 8 ; i++ ){
		{
			FVECTOR fvtmp;
			int      itmp;
			float    sign_z;      

			itmp = i % 5;
			DG_COPY_VEC( ( pos + i ) , ( FVECTOR * )fmtmp.m[ 3 ] );
			_sceVu0ScaleVector( &fvtmp , &dx , sign_x[ point[ itmp ] ] ); 
			_sceVu0AddVector( ( pos + i ) , ( pos + i ) , &fvtmp );
			_sceVu0ScaleVector( &fvtmp , &dy , sign_y[ point[ itmp ] ] ); 
			_sceVu0AddVector( ( pos + i ) , ( pos + i ) , &fvtmp );
			sign_z = i < 5 ? 1 : -1;
			_sceVu0ScaleVector( &fvtmp , &dz , sign_z ); 
			_sceVu0AddVector( ( pos + i ) , ( pos + i ) , &fvtmp );
		}
	}
	{
		int itmp;

		itmp = 0;
		i = N_VERTS - 8;
		while ( i < N_VERTS ){
			DG_COPY_VEC( ( pos + i ) , ( pos + itmp ) );
			i++;
			DG_COPY_VEC( ( pos + i ) , ( pos + itmp + VERTEX_PLUS ) );
			i++;
			itmp++;
		}
	}
	MissileNum( pWork , pos );
}

static void Act( Work *pWork )
{
	pWork->act( pWork );
}

static void Die( Work *pWork )
{
	if ( pWork->prim != NULL ){
		pWork->prim = OK_FreePrim2( pWork->prim );
	}
}

#define DG_COPY_VEC_2(a, b)	\
{						\
   (a)->x = (b)->x ;	\
   (a)->y = (b)->y ;	\
   (a)->z = (b)->z ;	\
   (a)->w = (b)->w ;	\
}

static int InitPacket( Work *pWork, DG_PRIM2 *prim )
{
	FVECTOR dx , dy , dz;
	FVECTOR *pos;
	FMATRIX fmtmp;
	DG_PRIM2_UVRGB *uvrgb; // Line用
	float sign_z;
	int i , j;

	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	pos = MEM_ADDR1 ;
	uvrgb = MEM_ADDR2 ;

	// matrixの算出
	DG_SetPos2( &pWork->pCtrl->mov , &pWork->pCtrl->rot );
	DG_GetPos( &fmtmp );
	// 縦横default の算出
	DG_COPY_VEC( &dx , ( FVECTOR * )&fmtmp.m[ 0 ] );
	DG_COPY_VEC( &dy , ( FVECTOR * )&fmtmp.m[ 2 ] );
	DG_COPY_VEC( &dz , ( FVECTOR * )&fmtmp.m[ 1 ] );
	// scale
	_sceVu0ScaleVector( &dx , &dx , 300.0f );
	_sceVu0ScaleVector( &dy , &dy , 300.0f );
	_sceVu0ScaleVector( &dz , &dz , 300.0f );
	j = 0;
	for ( i = 0 ; i < N_VERTS - 8 ; i++ ){
		// 場所の算出
		{
			FVECTOR fvtmp;
			int      itmp;

			itmp = i % 5;
			DG_COPY_VEC_2( ( pos + i ) , ( FVECTOR * )fmtmp.m[ 3 ] );
			_sceVu0ScaleVector( &fvtmp , &dx , sign_x[ point[ itmp ] ] ); 
			_sceVu0AddVector( ( pos + i ) , ( pos + i ) , &fvtmp );
			_sceVu0ScaleVector( &fvtmp , &dy , sign_y[ point[ itmp ] ] ); 
			_sceVu0AddVector( ( pos + i ) , ( pos + i ) , &fvtmp );
			sign_z = i < 5 ? 1 : -1;
			_sceVu0ScaleVector( &fvtmp , &dz , sign_z ); 
			_sceVu0AddVector( ( pos + i ) , ( pos + i ) , &fvtmp );
		}
		uvrgb->q = 4096 ;
		if ( i % 5 ){
			uvrgb->f = 0x0fff ; // kick
		} else {
			uvrgb->f = 0x8fff ; // strip
		}
		uvrgb->r = 200;
		uvrgb->g = 240;
		uvrgb->b = 120;
		uvrgb->a = 16;/* アンチエイリアスを使用するときには１２８にする */
		uvrgb++;
	}
	{
		int itmp;

		itmp = 0;
		i = N_VERTS - 8;
		while ( i < N_VERTS ){
			DG_COPY_VEC( ( pos + i ) , ( pos + itmp ) );
			uvrgb->f = 0x8fff ; // strip
			uvrgb->r = 200;
			uvrgb->g = 240;
			uvrgb->b = 120;
			uvrgb->a = 16;/* アンチエイリアスを使用するときには１２８にする */
			uvrgb++;
			i++;
			DG_COPY_VEC( ( pos + i ) , ( pos + itmp + VERTEX_PLUS ) );
			uvrgb->f = 0x0fff ; // kick
			uvrgb->r = 200;
			uvrgb->g = 240;
			uvrgb->b = 120;
			uvrgb->a = 32;/* アンチエイリアスを使用するときには１２８にする */
			uvrgb++;
			i++;
			itmp++;
		}
	}
	_MemCopy( prim->pos[ 0 ],  MEM_ADDR1, sizeof(FVECTOR), N_VERTS );
	_MemCopy( prim->uvrgb[ 0 ], MEM_ADDR2, sizeof(DG_PRIM2_UVRGB), N_VERTS );
	_MemCopy( prim->pos[ 1 ],  MEM_ADDR1, sizeof(FVECTOR), N_VERTS );
	_MemCopy( prim->uvrgb[ 1 ], MEM_ADDR2, sizeof(DG_PRIM2_UVRGB), N_VERTS );

	return ( 0 );
}

/*----------------------------------------------------------------*/
static int GetResources( Work *pWork )
{
	DG_PRIM2	*prim ;

	prim = pWork->prim = GM_MakePrim2( LINE_FLAG , 1, N_VERTS );
	if( prim == NULL ){
		return ( -1 );
	}
	if ( InitPacket( pWork, prim ) < 0 ){
		return ( -1 );
	}
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	pWork->disp_num = 0;
	return 0 ;
}

void *NewSKLineBox( CONTROL *pCtrl , float alpha )
{
	Work		*pWork ;

	pWork = (Work *)GV_NewEffect( GV_ACTOR_AFTER2, sizeof( Work ) );
	if ( pWork != NULL ) {
		GV_SetActor( &( pWork->actor ), Act , Die ) ;
		GV_ActorEX( &pWork->actor );
		pWork->pCtrl = pCtrl;
		pWork->alpha = alpha;
		pWork->act = ( void * )NormalAct;
		if ( GetResources( pWork ) < 0 ) {
			GV_DestroyActor( pWork ) ;
			return NULL ;
		}
	}
	return (void *)pWork ;
}
