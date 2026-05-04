//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/* 

   マンハッタン ブルックリン 用のフォグ
   fog_wall.c

   ????/??/?? H.Tanala
   2000/10/31 T.Shibata		//プリミチブに切り替え

   $Id: fog_wall.c,v 1.1.1.3 2002/11/19 11:47:46 Yoshizawa1 Exp $

*/


#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <math.h>
#endif
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
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif
#include "libutl.h"

#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"

extern int ok_flush_status;

#define	 N_OBJS   1
//#define	 BODY_FLAG  (DG_FLAG_TEXT|DG_FLAG_NOFOG|DG_FLAG_SHADE|DG_FLAG_ONEPIECE)
#define	 BODY_FLAG  (DG_FLAG_ONEPIECE)
#define	 MAX_BRIGHT	 128.0F

typedef  struct
{
	GV_ACT_EX	actor;
	DG_PRIM2	*prim;

} Work ;

static void Act(Work *work)
{
	if( ok_flush_status!=0 ){
		DG_InvisiblePrim2( work->prim ) ;
		return;
	}else{
		DG_VisiblePrim2( work->prim );
	}
}

static void Die(Work *work)
{
	if(work->prim != NULL){
		GM_FreePrim2( work->prim );
	}
}

static  int   GetResources( Work *work, FVECTOR *model_center, 
							SVECTOR *model_rot, FVECTOR *model_scale)
{
	DG_TEX			*tex;
	DG_PRIM2		*prim;
	FMATRIX  		world;
	DG_PRIM2_UVRGB	*uvrgb0,*uvrgb1;
	int				u[3],v[2];
	int				off_u,w,tri_w,i;

	static FVECTOR  init_pos[6] = {
		{ 0.0f, 4000.0f, -5000.0f, 1.0f },
        { 0.0f,    0.0f, -5000.0f, 1.0f },
		{ 0.0f, 4000.0f, -4800.0f, 1.0f },
        { 0.0f,    0.0f, -4800.0f, 1.0f },
		{ 0.0f, 4000.0f,  5000.0f, 1.0f },
        { 0.0f,    0.0f,  5000.0f, 1.0f },
	};
	
	tex = DG_GetTexture(6243702);

	if(!tex){ printf("ERR!! NO TEX!! fog_wall.c\n"); return (-1); }
	DG_GetTexelInfo( &w, NULL, &off_u, NULL, tex );
	tri_w = 1 << ((tex->tex_trans.tex0.data >> 26) & 0x0f);

	u[0] = FTOI12( (0.5f + (float)off_u )/(float)tri_w );
	u[1] = FTOI12( (4.0f + (float)off_u )/(float)tri_w ); 
	u[2] = FTOI12( ((float)w - 0.5f + (float)off_u )/(float)tri_w );

	v[0] = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	v[1] = FTOI12( 1.0f * tex->v_scale + tex->v_offset );

	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SHADE|DG_PRIM2_FOG,
									  1,
									  6 );

	if(!prim){ printf("ERR!! MAKE PRIM2!! win_rain.c\n"); return (-1); }

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x80 ) );
	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );


	DG_SetPos2(model_center,model_rot) ;
	DG_GetPos(&world) ;
	world.m[0][0] *= model_scale->vx;
	world.m[0][1] *= model_scale->vx;
	world.m[0][2] *= model_scale->vx;
	world.m[1][0] *= model_scale->vy;
	world.m[1][1] *= model_scale->vy;
	world.m[1][2] *= model_scale->vy;
	world.m[2][0] *= model_scale->vz;
	world.m[2][1] *= model_scale->vz;
	world.m[2][2] *= model_scale->vz;
	DG_SetPos(&world) ;

	DG_PutVector(init_pos,prim->pos[0],6);
	DG_PutVector(init_pos,prim->pos[1],6);

	uvrgb0 = prim->uvrgb[0];
	uvrgb1 = prim->uvrgb[1];
	for( i = 0; i < 6; i++ ){
		uvrgb0->r = 0x80;
		uvrgb0->g = 0x80;
		uvrgb0->b = 0x80;
		uvrgb0->a = 0x80;
		uvrgb0->q = 4096;
		uvrgb0->f = (i<2)?0x8fff:0x0fff;
		uvrgb0->u = u[i>>1];
		uvrgb0->v = v[i&1];
		*uvrgb1 = *uvrgb0;

		uvrgb0++; uvrgb1++;
	}
	return 0 ;

}

void *NewFog_Wall( FVECTOR *model_center, SVECTOR *model_rot, FVECTOR *model_scale )
{
	Work	 *work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof(Work)) ;

	if(work != NULL){
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor );
		if(GetResources( work,model_center,model_rot,model_scale) < 0){
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}

	return (void *)work ;
}

