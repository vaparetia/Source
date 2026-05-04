//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	discovery.c
   
	2000/10/18 T.Shibata

	$Id: discovery.c,v 1.1.1.3 2002/11/19 11:48:46 Yoshizawa1 Exp $
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
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"../util/ts_util.h"
/* ------------------------------------------------------ */
           /* 
               補助マクロ
           */

#define   FTOI12(_f)        ( DG_FTOI( ( (float)(_f)*4096.0f) ) )
/* ---------------------------------------------------------------- */
	/*
		プログラム使用定数の定義
	*/

#define		N_PRIMS		(1)
#define		N_VERTS		(1)
#define		PRIM_TYPE	(DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA)

#define		TEX_CODE 	(9300995)	// kirari_msk
//#define		TEX_CODE 	(13276037)	// kirari

#define		SHIFT_X	(0.0f)
#define		SHIFT_Y	(37.5f)
#define		SHIFT_Z	(124.5f) //(117.5)

#define		SIZE_W	(100.0f)
#define		SIZE_H	(25.0f)

#define		LIFE	(8)

typedef struct {
    GV_ACT_EX		actor;

    DG_PRIM2	    *prim;
    FMATRIX	    	*world;

	short			u0[3];
	short			u1[3];

    short			timer;
	short			now_anime;
} Work;

static FVECTOR		init_pos[4] = {
	{ SHIFT_X - SIZE_W, SHIFT_Y - SIZE_H, SHIFT_Z, 1.0f },
	{ SHIFT_X + SIZE_W, SHIFT_Y - SIZE_H, SHIFT_Z, 1.0f },
	{ SHIFT_X - SIZE_W, SHIFT_Y + SIZE_H, SHIFT_Z, 1.0f },
	{ SHIFT_X + SIZE_W, SHIFT_Y + SIZE_H, SHIFT_Z, 1.0f },
};

static  void  Act(Work *work)
{
	int					clock;
	FVECTOR				*pos;
	DG_PRIM2_UVRGB		*uvrgb;
	
	DG_VisiblePrim2(work->prim);
	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;

	pos = work->prim->pos[clock];
	uvrgb = work->prim->uvrgb[clock];

	DG_SetPos( work->world );
	DG_PutVector( init_pos, pos, 4 );

	if(work->timer == 2 || work->timer == 3){
		uvrgb[0].u = work->u0[1];
		uvrgb[1].u = work->u1[1];
		uvrgb[2].u = work->u0[1];
		uvrgb[3].u = work->u1[1];
      // BP_WARNING - The below if is never true.  Not fixing.
	}else if(work->timer == 4 && work->timer == 5 ){
		uvrgb[0].u = work->u0[2];
		uvrgb[1].u = work->u1[2];
		uvrgb[2].u = work->u0[2];
		uvrgb[3].u = work->u1[2];
	}
#if 0
	if( work->timer % 3 == 0 ){
		uvrgb[0].u = work->u0[work->now_anime];
		uvrgb[1].u = work->u1[work->now_anime];
		uvrgb[2].u = work->u0[work->now_anime];
		uvrgb[3].u = work->u1[work->now_anime];
	}else if( work->timer % 3 == 1){
		uvrgb[0].u = work->u0[work->now_anime];
		uvrgb[1].u = work->u1[work->now_anime];
		uvrgb[2].u = work->u0[work->now_anime];
		uvrgb[3].u = work->u1[work->now_anime];
		work->now_anime++;
	}
#endif
	//AN_Test_Eye2(pos,2);

	if(++work->timer >= LIFE ){
		DG_InvisiblePrim2(work->prim);
		GV_DestroyActor( work );
	}
}

static  void  Die(Work *work)
{
    if(work->prim != NULL) GM_FreePrim2(work->prim) ;
}
     

static int GetResources( Work *work )
{
	DG_TEX				*tex ;
	DG_PRIM2			*prim ;
	DG_PRIM2_UVRGB		*uvrgb0,*uvrgb1;
	int					add_u,off_u,tri_w,i;

	work->timer = 0;
	work->now_anime = 0;

	if(work->world == NULL)
	    return -1 ;

	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SHADE,
									  1, 4);
	if(prim == NULL) return -1 ;
	tex = DG_GetTexture(TEX_CODE);
	if(tex == NULL) return -1 ;
	DG_ConfigPrim2Tex(prim,tex) ;
	DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 )) ;

	tri_w = 1 << ((tex->tex_trans.tex0.data >> 26) & 0x0f);
	DG_GetTexelInfo( &add_u, NULL, &off_u, NULL, tex);
	add_u /= 3;

	work->u0[0] = FTOI12(((float)(off_u + 0) + 0.5f)/(float)tri_w);
	work->u0[1] = FTOI12(((float)(off_u + add_u) + 0.5f)/(float)tri_w);
	work->u0[2] = FTOI12(((float)(off_u + add_u*2) + 0.5f)/(float)tri_w);

	work->u1[0] = FTOI12(((float)(off_u + add_u) - 0.5f)/(float)tri_w);
	work->u1[1] = FTOI12(((float)(off_u + add_u*2) - 0.5f)/(float)tri_w);
	work->u1[2] = FTOI12(((float)(off_u + add_u*3) - 0.5f)/(float)tri_w);

	//pos init
	DG_SetPos(work->world);
	DG_PutVector(init_pos,prim->pos[0],4);
	DG_PutVector(init_pos,prim->pos[1],4);

	uvrgb0 = prim->uvrgb[0];
	uvrgb1 = prim->uvrgb[1];
	for( i = 0; i < 4; i++ ){
		uvrgb0->u = (i&1)?work->u1[0]:work->u0[0];
		uvrgb0->v = (i<2)?FTOI12(tex->v_offset):FTOI12(tex->v_scale + tex->v_offset);

		uvrgb0->f = (i<2)?VERT_KICK_CODE:DRAW_KICK_CODE;
		uvrgb0->q = 4096;

		uvrgb0->r = 196 - 32;
		uvrgb0->g = 196 - 32;
		uvrgb0->b = 128 - 16;
		uvrgb0->a = 128;
	
		*uvrgb1 = *uvrgb0;

		uvrgb1++; uvrgb0++;
		
	}
	return 0;
}


void *NewDiscovery(FMATRIX *world)
{
	Work		*work ;

	//printf("キュピーン\n");
	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) 
	{
	    work->world = world ;
	    GV_SetActor( &( work->actor ), Act, Die ) ;
	    GV_ActorEX(&work->actor) ;
	    if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
	    }
	}
	return (void *)work ;
}
