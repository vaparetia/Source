//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	trample_slater.c
	潰れたふなむし
	
	2001/06/06 T.Shibata
	
	$Id: trample_slater.c,v 1.1.1.3 2002/11/19 11:48:50 Yoshizawa1 Exp $

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
#include "libutl.h"

#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"def_dma.h"
#include	"../util/ts_util.h"

#define CLOCK_COUNT	(BP_BASE_TICK())

#define FABS(_x) ({float _a = _x;asm("abs.s %0,%1":"=f"(_a):"f"(_a));(_a);})
#define		PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)



#define TEX_CODE_BASE (15965844)	//(GV_StrCode("hnm_death_00_ovl_alp"))

#define		N_FUNAS			(128)
#define		N_FUNAS_VERTS	(4)

#define		N_ALLVERTS		(N_FUNAS*N_FUNAS_VERTS)
#define		N_VERTS	(64)
#define		N_PRIMS	(N_ALLVERTS/N_VERTS)

typedef	struct {
	GV_ACT_EX		actor;
	int				name;
	int				map;
	int				flags;
	DG_PRIM2		*prim;

	char			init_flags[N_FUNAS];
	FVECTOR			init_pos[N_ALLVERTS];
	int				index;
} Work ;

static Work *TrmplSlater = NULL;

#if 0
static int CheckMesgParam( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;

	mes_num = GV_ReceiveMessage( work->name, &msg );

	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		default:
			break;
		}
		msg--;
	}

	return 0;
}
#endif
void SetTrmplSlater( FVECTOR *center, FMATRIX *root, float size_tmp )
{
	Work *work = TrmplSlater;
//	DG_PRIM2	*prim = work->prim;
	FVECTOR		*pos;
	FMATRIX		world;
	float		size = 80.0f;
//	SVECTOR		rot = {0,0,0,0};
#if 0
	FVECTOR		init_pos[4] = {
		{  size, 1.0f,  size, 1.0f },
		{ -size, 1.0f,  size, 1.0f },
		{  size, 1.0f, -size, 1.0f },
		{ -size, 1.0f, -size, 1.0f },
	};
#else
	short		rot_y = (irnd()>>7)%4096;
	FVECTOR		init_pos[4] = {
		{ size*TS_COSs( rot_y ), 1.0f, size*TS_SINs( rot_y ), 1.0f },
		{ size*TS_COSs( (rot_y+1024)%4096 ), 1.0f, size*TS_SINs( (rot_y+1024)%4096 ), 1.0f },
		{ size*TS_COSs( (rot_y+3072)%4096 ), 1.0f, size*TS_SINs( (rot_y+3072)%4096 ), 1.0f },
		{ size*TS_COSs( (rot_y+2048)%4096 ), 1.0f, size*TS_SINs( (rot_y+2048)%4096 ), 1.0f },
	};
#endif
	int se_table[2] = {
		SD_A_FNAFOOT1, //船虫踏み潰す１
		SD_A_FNAFOOT2, //船虫踏み潰す２(２種ランダム)
	};

	DG_COPY_MAT( &world, root );
	DG_COPY_VEC( (FVECTOR*)world.m[3], center );
	work->flags = 1;
	pos = &work->init_pos[work->index*N_FUNAS_VERTS];

	//TS_VecToRot( &rot, norm );
	//rot.vz += irnd()%2048;
	DG_SetPos( &world );
	DG_PutVector( init_pos, pos, 4 );
	work->init_flags[work->index] = 1;

	GM_SeSetMode( se_table[(irnd()>>17)&1], center, GM_SEMODE_BOMB );
//printf("aaa\n");
	work->index++;
	if( work->index >= N_FUNAS ) work->index = 0;
}

static void Act( Work *work )
{
	DG_PRIM2	*prim = work->prim;
	FVECTOR		*pos;
	FVECTOR		*init_pos = work->init_pos;

	int i;

	if( !work->flags ) return;

	DG_SwitchBuffPrim2( prim );
	pos = prim->pos[prim->buffer_clock];

	for( i = 0; i < N_FUNAS; i++ ){
		if( work->init_flags[i] ){

			DG_COPY_MAT( pos, init_pos );
			work->init_flags[i] <<= 1;
			if(work->init_flags[i] == 4)work->init_flags[i] = 0;
		}
		pos+=4; init_pos+=4;
	}
	work->flags = 0;
}

static void Die( Work *work )
{
	if(work->prim)GM_FreePrim2(work->prim);
	TrmplSlater = NULL;
}

static void *InitFunaPrim( int tec_code, int n_prims, int n_verts )
{
	int					i,j;
	DG_PRIM2 			*prim;
	DG_TEX				*tex;
	FVECTOR				*pos0, *pos1;
	DG_PRIM2_UVRGB		*uvrgb0, *uvrgb1;
	int					u[4],v[2];
	int					tw,th,off_u,off_v,tex_w,tex_h;

	tex = DG_GetTexture(tec_code);
	if(!tex){ printf("ERR!! NO TEX!! trample_slater.c\n"); return (NULL); }
	
	prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SHADE,
						 n_prims,
						 n_verts );
	if(!prim){ printf("ERR!! MAKE PRIM2!! trample_slater.c\n"); return (NULL); }

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x80 ) );

	// まだチャンネル１～３は無いが取りあえず フラグ立て
	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	DG_GetTexelInfo( &tex_w, &tex_h, &off_u, &off_v, tex );
	tw = ( tex->tex_trans.tex0.data >> 26 ) & 0x0f ;
	th = ( tex->tex_trans.tex0.data >> 30 ) & 0x0f ;
	tw = 1 << tw ;
	th = 1 << th ;

	u[0] = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	u[1] = FTOI12( ((float)(off_u + tex_w/2)-0.5f)/(float)tw );
	u[2] = FTOI12( ((float)(off_u + tex_w/2)+0.5f)/(float)tw );
	u[3] = FTOI12( 1.0f * tex->u_scale + tex->u_offset );

	v[0] = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	v[1] = FTOI12( 1.0f * tex->v_scale + tex->v_offset );

	pos0 = prim->pos[0];
	pos1 = prim->pos[1];
	uvrgb0 = prim->uvrgb[0];
	uvrgb1 = prim->uvrgb[1];
	for( i = 0; i < n_prims; i++ ){
		for( j = 0; j < n_verts; j++ ){
			DG_COPY_VEC( pos0, &DG_ZeroVector );
			DG_COPY_VEC( pos1, &DG_ZeroVector );


			uvrgb0->r = 0x60;
			uvrgb0->g = 0x60;
			uvrgb0->b = 0x60;
			uvrgb0->a = 0x40;
			uvrgb0->q = 4096;
			uvrgb0->f = (j<2)?VERT_KICK_CODE:DRAW_KICK_CODE;
		
			uvrgb0->u = u[(j&1)+2*(i&1)];
			uvrgb0->v = v[j/2];

			*uvrgb1 = *uvrgb0;
			pos0++; pos1++;
			uvrgb0++; uvrgb1++;
		}
	}

	return prim;
}

//static int GetOptions( Work *work )
//{
//}

static int GetResources( Work *work )
{
	work->prim = InitFunaPrim( TEX_CODE_BASE, N_FUNAS, N_FUNAS_VERTS );
	if(!work->prim) return -1;
	work->index = 0;
	return 0;
}

void *NewTrampleSlater( int name, int map )
{
	Work		*work ;

	if( TrmplSlater ) return NULL;

	//work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	work = GV_CreateActor( GV_ACTOR_EFFECT, GV_CLASS_EFFECT, sizeof( Work ), 0 );
//	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;
		work->name = name;
		work->map = map;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
		TrmplSlater = work;
	}
	
	return (void *)work ;
}

