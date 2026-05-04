//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	negafilter.c
	全画面ネガ
	2002/05/07
	$Id: negafilter.c,v 1.1.1.3 2002/11/19 11:49:48 Yoshizawa1 Exp $
*/
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>


#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#endif

#include	"gameheader.h"
#include	"libutl.h"

#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

//テクスチャ
//(A-B)*C>>7+D
#define UWAGAKI (SCE_GS_SET_ALPHA(2,2,0,0,0))
#define KASAN (SCE_GS_SET_ALPHA(0,2,0,1,0))
#define KASAN2 (SCE_GS_SET_ALPHA(0,2,2,1,64))
#define ALPHA_HALF (SCE_GS_SET_ALPHA(0,1,0,1,0x80))
#define GENSAN (SCE_GS_SET_ALPHA(2,0,0,1,0))
//#define NEGA	(SCE_GS_SET_ALPHA(0,1,0,2,0))
//#define FIX_NEGA	(SCE_GS_SET_ALPHA(0,1,2,1,255)) //FIX APHA
#define SOURCE_NEGA	(SCE_GS_SET_ALPHA(0,1,0,1,0))	//source alpha

#define PRI_R		(63)
#define PRI_G		(63)
#define PRI_B		(63)

#define N_POLY		(1)

#define SC_X (640)
#define SC_Y (480)



//#define PRIM_FLAG	(DG_PRIM2_LINE|DG_PRIM2_ALPHA|DG_PRIM2_TEX)
#define PRIM_FLAG	(DG_PRIM2_LINE|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ANTIALIASING|DG_PRIM2_ALPHA)
#define POLY_FLAG	(DG_MENU2_PRIM_POLY | DG_PRIM2_SHADE | DG_PRIM2_ALPHA )

//DG_PRIM2_CULLPOLY
#define CYCLE (8)
typedef	struct	{
	GV_ACT_EX	actor ;
//	DG_PRIM2	*prim ;
    DG_MENU2_PRIM	*m_prim ;

	float	rate ;
	int		flag ;
	int		cnt ;
} Work ;



static void SetAlpha( Work *work){
	int		i ;

    DG_MENU2_PRIM	*prim ;
    DG_MENU2_PACKET	*packs ;


	prim = work->m_prim ;

	for(i= 0 ;i<2;i++){
		packs = ( DG_MENU2_PACKET * )prim->packet[ i ] ;
		packs->alpha.data = SCE_GS_SET_ALPHA(0,1,2,1,work->cnt) ;
	}
}
static void InitPacket( Work *work){
	int		i,j,k;
	FVECTOR		*pos;
//	DG_PRIM2	*prim ;
//	DG_PRIM2_UVRGB	*uvrgb;
	DG_TEX		*tag_tex ;

    DG_MENU2_PRIM	*prim ;
    DG_MENU2_PACKET	*packs ;
    DG_MENU2_POLY_G	*poly ;


	prim = work->m_prim ;


	/* テクスチャ取得 */
//	tag_tex = DG_GetTexture( 8617368 );
//	DG_ConfigPrim2Tex( work->m_prim, tag_tex );


	DG_InvisibleMenu2Prim( work->m_prim ) ;




	for(i= 0 ;i<2;i++){
		packs = ( DG_MENU2_PACKET * )prim->packet[ i ] ;
		packs->alpha.data = SOURCE_NEGA ;
		poly = ( DG_MENU2_POLY_G * )packs->data ;

		for(k= 0 ; k<N_POLY;k++){
//			poly->prim.ABE = 0 ;
//			poly->prim.IIP = 0 ;

			DG_MENU2_SET_RGBA1( poly, PRI_R, PRI_G, PRI_B, 64 ) ;
			DG_MENU2_SET_RGBA2( poly, PRI_R, PRI_G, PRI_B, 64 ) ;
			DG_MENU2_SET_RGBA3( poly, PRI_R, PRI_G, PRI_B, 64 ) ;
			DG_MENU2_SET_RGBA4( poly, PRI_R, PRI_G, PRI_B, 64 ) ;

			DG_MENU2_SET_XY1( poly, 0, 0 ) ;
			DG_MENU2_SET_XY2( poly, SC_X, 0 ) ;
			DG_MENU2_SET_XY3( poly, 0, SC_Y ) ;
			DG_MENU2_SET_XY4( poly, SC_X, SC_Y ) ;

			poly++;
		}
	}

}
static void SetSourceAlpha( Work *work){
	int		i,j;

    DG_MENU2_PACKET	*packs ;
    DG_MENU2_POLY_G	*poly ;


	for(i= 0 ;i<2;i++){
		packs = ( DG_MENU2_PACKET * )work->m_prim->packet[ i ] ;
		poly = ( DG_MENU2_POLY_G * )packs->data ;
		for(j= 0 ; j<N_POLY;j++){
//			DG_MENU2_SET_RGBA1( poly, PRI_R, PRI_G, PRI_B, 0 ) ;
//			DG_MENU2_SET_RGBA2( poly, PRI_R, PRI_G, PRI_B, 0 ) ;
			DG_MENU2_SET_RGBA1( poly, PRI_R, PRI_G, PRI_B, work->cnt ) ;
			DG_MENU2_SET_RGBA2( poly, PRI_R, PRI_G, PRI_B, work->cnt ) ;
			DG_MENU2_SET_RGBA3( poly, PRI_R, PRI_G, PRI_B, work->cnt ) ;
			DG_MENU2_SET_RGBA4( poly, PRI_R, PRI_G, PRI_B, work->cnt ) ;
			poly++;
		}
	}

}
static	void	Act( work )
Work		*work ;
{


	DG_VisibleMenu2Prim( work->m_prim ) ;

//	SetAlpha( work ) ;
	SetSourceAlpha( work ) ;


#if 1
	if(GV_PadData[ 1 ].status & PAD_U){
		work->cnt += 4; 
	}
	if(GV_PadData[ 1 ].status & PAD_D){
		work->cnt -= 4; 
	}
#else
	work->cnt+=4 ;
#endif

	work->cnt &= 255 ;

}

static	void	Die( work )
Work		*work ;
{

	if(work->m_prim != NULL) DG_FreeMenu2Prim( work->m_prim ) ;
}




/*----------------------------------------------------------------*/
static	int	GetResources( Work	*work ){

//	work->prim = GM_MakePrim2( PRIM_FLAG, 1, N_VERTS );
	work->m_prim = DG_MakeMenu2Prim( POLY_FLAG, N_POLY, 0 ) ;
	work->cnt = 0 ;

	InitPacket( work );

	return 0 ;
}
void	*NewNegaFilter( int flag,float rate ,float height,int time_a,int time_b)
{
	Work		*work ;
	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), 0 ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		work->flag = flag ;
		if ( GetResources( work) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}

