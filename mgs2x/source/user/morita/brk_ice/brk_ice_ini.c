//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_ice_ini.c
   アイスクーラー

   1999/12/13 T. Morita
   $Id: brk_ice_ini.c,v 1.1.1.3 2002/11/19 11:45:32 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "brk_icebox.h"
#include "../../shibata/util/ts_util.h"

#if DEBUG_MODE
static int DebugFlag = 0 ;
#endif



void BRK_ICE_InitBroken( Work *work )
{
    if ( GCL_GetOptionValue( 'B', 0 ) )
	BRK_ICE_AlreadyMessy( work ) ;
}


#define FTOI12(_f)      ( DG_FTOI( ( (float)(_f)*4096.0f) ) )
static inline void InitPacket( DG_PRIM2 *p, DG_TEX *tex, int i, u_int rgba )
{
    DG_PRIM2_UVRGB *uvrgb = p->uvrgb[0] ;

    DG_SetPrim2Alpha( p, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) ) ;
    DG_ConfigPrim2Tex( p, tex ) ;

    for ( ; --i>=0 ; uvrgb += 4 )
    {
	uvrgb[0].u = uvrgb[2].u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	uvrgb[1].u = uvrgb[3].u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
	uvrgb[0].v = uvrgb[1].v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
	uvrgb[2].v = uvrgb[3].v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
	uvrgb[0].q = uvrgb[1].q = uvrgb[2].q = uvrgb[3].q = 4096 ;
	uvrgb[0].f = uvrgb[1].f = 0x8fff ;
	uvrgb[2].f = uvrgb[3].f = 0x0fff ;
	uvrgb[0].r = uvrgb[1].r = uvrgb[2].r = uvrgb[3].r = (rgba >>  0) & 0xff ;
	uvrgb[0].g = uvrgb[1].g = uvrgb[2].g = uvrgb[3].g = (rgba >>  8) & 0xff ;
	uvrgb[0].b = uvrgb[1].b = uvrgb[2].b = uvrgb[3].b = (rgba >> 16) & 0xff ;
	uvrgb[0].a = uvrgb[1].a = uvrgb[2].a = uvrgb[3].a = (rgba >> 24) & 0xff ;
    }
}

int BRK_ICE_InitPrims( Work *work )
{
    DG_TEX *t = DG_GetTexture( GV_StrCode( "dankon2_alp" ) ) ;
    DG_PRIM2 *p ;

    if ( !(p = GM_MakePrim2( DG_PRIM2_POLY| DG_PRIM2_TEX| DG_PRIM2_ALPHA| DG_PRIM2_SINGLEBUFF,
			     BRK_N_SCAR, BRK_N_VERTS )) )
	PERROR( "Cannot make DG_PRIM2(Maybe no memory) :: NewPutIceboxObject" ) ;
    InitPacket( p, t, BRK_N_SCAR, 0x00808080 ) ;
    p->root = &work->box.objs->world ;
    work->box.scar = p ;

    return 0 ;
}
/*
int BRK_ICE_InitIceDustPrims( ICEBOX *ice )
{
    DG_PRIM2_UVRGB *u0, *u1 ;
    FVECTOR        *p0, *p1 ;
    int i ;

	
    if ( !(ice->prim = GM_MakePrim2( DG_PRIM2_LINE|DG_PRIM2_ALPHA|DG_PRIM2_SHADE, BRK_N_DUST, 2 )) )
	PERROR( "Cannot make DG_PRIM2(Maybe no memory) :: NewPutIceboxObject" ) ;

    DG_SetPrim2Alpha( ice->prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) ) ;
    u0 = ice->prim->uvrgb[0], u1 = ice->prim->uvrgb[1] ;
    p0 = ice->prim->pos[0]  , p1 = ice->prim->pos[1]   ;

    for ( i=BRK_N_DUST ; --i>=0 ; u0+=2, u1+=2, p0+=2, p1+=2 )
    {
	p1->vx = p0->vx = ((p1+1)->vx = (p0+1)->vx = ice->mov.pos.vx) + frnd()*10.0f ;
	p1->vy = p0->vy = ((p1+1)->vy = (p0+1)->vy = ice->mov.pos.vy) +  rnd()*30.0f ;
	p1->vz = p0->vz = ((p1+1)->vz = (p0+1)->vz = ice->mov.pos.vz) + frnd()*10.0f ;
	p1->vw = p0->vw =  (p1+1)->vw = (p0+1)->vw = 1.0f ;

	u0[0].q = u0[1].q = u1[0].q = u1[1].q = 4096 ;
	u0[0].r = u1[0].r = u0[0].g = u1[0].g = u0[0].b = u1[0].b = u0[0].a = u1[0].a =
	    u0[1].r = u1[1].r = u0[1].g = u1[1].g = u0[1].b = u1[1].b = 128 ;
	u0[1].a = u1[1].a = 0 ;
	u0[0].f = u1[0].f = 0x8fff ;
	u0[1].f = u1[1].f = 0x0fff ;
    }

    return 0 ;
}
*/

//add shibata
int BRK_ICE_InitIceDustComdl( ICEBOX *ice )
{
	int i;
	FVECTOR *dvec = ice->dvec;
	FVECTOR *center = &ice->mov.pos;
	DG_COMDL_POS 	*comdl_pos;
	FVECTOR fvtemp;
	SVECTOR *rot = ice->rot;

	if(!ice->frg_objs){ printf("NULL comdl\n"); return 1; }
	comdl_pos = ice->frg_objs->pos;
	ice->frg_objs->flag &= ~(DG_COMDL_INVISIBLE0);
	ice->ice_scl.vx = (0.25f * fpu_VectorLength2((FVECTOR*)ice->objs->world.m[0]));
	ice->ice_scl.vy = ice->ice_scl.vz = ice->ice_scl.vx;
//printf("scle = %f\n",ice->ice_scl.vx);
//	printf("center vx %f: vy %f: vz %f\n",center->vx,center->vy,center->vz);
	for( i = 0; i < BRK_N_DUST; i++ ){
		dvec->vx = frnd() * 10.0f;
		dvec->vy =  rnd() * 50.0f;
		dvec->vz = frnd() * 10.0f;
		dvec->vw= 0.0f;

		rot->vx = (short)(irnd());
		rot->vy = (short)(irnd());
		rot->vz = (short)(irnd());

		_sceVu0AddVector( &fvtemp, dvec, center ) ;
	//	fpu_AddVectors( &fvtemp, dvec, center );
		DG_SetPos2( &fvtemp, rot ) ;
		DG_GetPos( &comdl_pos->world );
		TS_ScaleMatrix(&comdl_pos->world,&comdl_pos->world,&ice->ice_scl);
		
		comdl_pos->color.vx = 0x80;
		comdl_pos->color.vy = 0x80;
		comdl_pos->color.vz = 0xa0;
		comdl_pos->color.vw = 0x80;

		comdl_pos++;
		dvec++; rot++;
	}
	

	return 0;
}

void BRK_ICE_MakeSizeAndCenter( FVECTOR *t_size, FVECTOR *t_pos, float *uv, float *lv )
{
    FVECTOR u = { uv[X],uv[Y],uv[Z],0 }, l = { lv[X],lv[Y],lv[Z],0 } ;

    _sceVu0SubVector( t_size, &u, &l ) ;
    _sceVu0ScaleVector( t_size, t_size, 0.5f ) ;
    _sceVu0AddVector( t_pos, &u, &l ) ;
    _sceVu0ScaleVector( t_pos , t_pos , 0.5f ) ;
}

int BRK_ICE_InitHazard( Work *work )
{
    if ( !GCL_GetOption( 'h' ) )
	PERROR( "No Icebox Model :: NewPutIceboxObject\n" ) ;
    if ( !(work->box.hzd = BRK_InitHazard( GCL_GetNextInt() )) )
	PERROR( "Ice Hazard wrong ID :: NewPutIceboxObject\n" ) ;
#if 0
    if ( !(work->hzd     = BRK_InitHazard( GCL_GetNextInt() )) )
	PERROR( "Box Hazard wrong ID :: NewPutIceboxObject\n" ) ;
#else
    work->hzd = work->box.hzd ;
#endif
    return 0 ;
}

int BRK_ICE_InitTarget( TARGET *t, POWER_TARGET *p, int map, int flag,
			float *uv, float *lv, FVECTOR *pos,
			void (*callback)( TARGET *off, TARGET *def, void *ptr ), void *ptr )
{
    FVECTOR t_size, t_pos ;

    BRK_ICE_MakeSizeAndCenter( &t_size, &t_pos, uv, lv ) ;
    GM_SetTarget( t, TARGET_DEFENSE|TARGET_POWER|TARGET_SEEK|flag, map, BOTH_SIDE, &t_size, &t_pos ) ;
    GM_SetPowerTarget( t, p, POWER_DECREASE, GM_Vitality, 0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( t, callback, ptr ) ;
    GM_PutTarget( t ) ;
    GM_MoveTarget( t, pos ) ;

#if DEBUG_MODE
    if ( DebugFlag )
	NewTargetView( t,  200, 50, 32 ) ;
#endif

    return 0 ;
}



int BRK_ICE_InitIce( Work *work )
{
    int     i ;
    ICEBOX *p ;

    work->n_ice &= ~BRK_ICE_INACTIVE ; /* 氷のアクトを有効にする */
    if ( !(work->ice = GV_Malloc( sizeof(ICEBOX) * work->n_ice )) )
	PERROR( "Cannot allocate memory for <work->icebox>  :: NewPutIceboxObject\n" ) ;

    for ( i=work->n_ice, p=work->ice ; --i>=0 ; p++ )
	p->objs = NULL,p->frg_objs = NULL;//, p->prim = NULL ; /* mallocからの初期化なのでNULLにする */

    for ( i=work->n_ice, p=work->ice ; --i>=0 ; p++ )
    {
	p->mov.pos.vx = frnd() * work->box.target.size.vx ;
	p->mov.pos.vy = 0 ;
	p->mov.pos.vz = frnd() * work->box.target.size.vz ;
	p->mov.rot_vx = (irnd()&15)-8 ;
	p->mov.rot_vy = (irnd()&15)-8 ;
	p->mov.rot_x  = irnd()&4095 ;
	p->mov.rot_y  = irnd()&4095 ;
	p->work = work ;
	p->flag = -1   ;

	p->sound_flag = 0;
	if ( !(p->objs = DG_MakeObjs( work->i_def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 )) )
	    PERROR( "Can't initialize DG_OBJS(Maybe no memory) :: NewPutIceboxObject\n" ) ;
	DG_SetLightMatrix( p->objs, p->lights ) ;
	DG_QueueObjs( p->objs ) ;
	GM_GroupObjs( p->objs, work->where ) ;
	DG_InvisibleObjs( p->objs ) ;

	//add shibata
//BRK_N_DUST

	p->frg_objs = DG_MakeComdl( work->frg_i_def->models[0].packs, DG_COMDL_SEMITRANS|DG_COMDL_INVISIBLE, BRK_N_DUST, 0 );
	if(!p->frg_objs) PERROR( "Can't initialize DG_COMDL(Maybe no memory) :: NewPutIceboxObject\n" );
	DG_QueueComdlObjs( p->frg_objs );
	GM_GroupObject( p->frg_objs, work->where ) ;
	
    }
    return  0 ;
}

int BRK_ICE_InitBox( Work *work )
{
    BOX    *b = &work->box ;
    int     buf[3] ;

    if ( !GCL_GetOption( 'p' ) )
	PERROR( "No Position (-p option missing) :: NewPutIceboxObject\n" ) ;
    GCL_GetIV( GCL_NextStr(), buf ) ;
    vu0_IV0toFV( (IVECTOR *)buf, &b->mov.pos ) ;
    if ( GCL_GetOption( 'r' ) )
    {
	GCL_GetIV( GCL_NextStr(), buf ) ;
	b->mov.rot_x = buf[X] ;
	b->mov.rot_y = buf[Y] ;
    }
    b->flag = GCL_GetOptionValue( 'f', 0 ) ;
#if DEBUG_MODE
    if ( b->flag )
	DebugFlag = 1 ;
#endif

    /* モデルの初期化 */
    if ( !(b->objs = DG_MakeObjs( work->m_def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 )) )
	PERROR( "Can't initialize DG_OBJS(Maybe box model missing) :: NewPutIceboxObject\n" ) ;
    DG_QueueObjs( b->objs ) ;
    GM_GroupObjs( b->objs, work->where ) ;
    DG_SetLightMatrix( b->objs, b->lights ) ;
    DG_GetLightMatrixFix( &b->mov.pos, b->lights ) ;

#if MAKING
    {
	extern void *NewDrawWireframe( DG_OBJ *obj ) ;
	NewDrawWireframe( &b->objs->objs[0] ) ;

	NewDrawWireRoot( &b->objs->objs[0], &b->objs->world ) ;

    }
#endif

    /* モデル情報よりターゲットを設定 */
    BRK_ICE_InitTarget( &b->target, &b->power, work->where, TARGET_ROTATE,
			&b->objs->def->ux, &b->objs->def->lx,
			&b->mov.pos, BRK_ICE_BoxTargetCallBack, work ) ;
    /* マトリックスの初期化 */
    RotateMatrixXY( &b->objs->world, &DG_UnitMatrix, b->mov.rot_x, b->mov.rot_y ) ;
    TransMatrix( &b->objs->world, &b->mov.pos ) ;

    /* 各変数の初期化 */
    b->non_dmg   = 0 ;
    b->ice_fount = 0 ;
	b->snd_flag = 0;
    return 0 ;
}


int BRK_ICE_InitOpenBox( Work *work )
{
    BOX     *b = &work->box ;
    DG_OBJS *objs = b->objs ;

    /* モデルの初期化 */
    if ( !(b->objs = DG_MakeObjs( work->p_def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 )) )
	PERROR( "Can't initialize DG_OBJS(Maybe box model missing) :: NewPutIceboxObject\n" ) ;
    DG_QueueObjs( b->objs ) ;
    GM_GroupObjs( b->objs, work->where ) ;
    DG_SetLightMatrix( b->objs, b->lights ) ;

    /* マトリックスの初期化 */
    b->objs->world = objs->world ;

    if ( objs )
    {
	DG_DequeueObjs( objs ) ;
	DG_FreeObjs( objs ) ;
    }

    return 0 ;
}


int BRK_ICE_GetOptions( Work *work )
{
    /* プロックIDの取得 */
    work->proc_id = GCL_GetOptionValue( 'C', 0 ) ;

    if ( !(work->n_ice = GCL_GetOptionValue( 'n', 20 )) )
	PERROR( "Number must be more than one :: NewPutIceboxObject\n" ) ;

    if ( !GCL_GetOption( 'm' ) )
	PERROR( "No Icebox Model :: NewPutIceboxObject\n" ) ;
    if ( !(work->m_def = (DG_DEF*)GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ))) )
	PERROR( "Cannot find BOX KMS-MODEL( not in data.cnf ) :: NewPutIceboxObject\n" ) ;
    if ( !(work->p_def = (DG_DEF*)GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ))) )
	PERROR( "Cannot find ICEinBOX KMS-MODEL( not in data.cnf ) :: NewPutIceboxObject\n" ) ;
    if ( !(work->i_def = (DG_DEF*)GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ))) )
	PERROR( "Cannot find ICE KMS-MODEL( not in data.cnf ) :: NewPutIceboxObject\n" ) ;
	
    if ( !(work->frg_i_def = (DG_DEF*)GV_GetCache( GV_CacheID( BRK_DUST_COMDL, 'k' ))) )
	PERROR( "Cannot find ICE KMS-MODEL( not in data.cnf ) :: NewPutIceboxObject\n" ) ;
	

    work->n_ice |= BRK_ICE_INACTIVE ;

    return 0 ;
}

int BRK_ICE_SprayPrimSet( Work *work, FVECTOR *center, FVECTOR *force )
{
	int				i,j;
	DG_PRIM2		*prim = work->spray;
	FVECTOR			*dvec = work->sp_dvec;
	FVECTOR			*lpos = work->sp_lpos;
	FVECTOR			*pos = prim->pos[0];
	DG_PRIM2_UVRGB	*uvrgb = prim->uvrgb[0];
	FMATRIX			world;
	SVECTOR			rot = {0,0,0,0};
	
	work->sp_flag = SPRAY_ACTIVE;
	
	TS_MakeMatrix(&world,force,center);
	
	for( i = 0; i < BRK_N_SPRAY; i++ ){
		dvec[0].vx = 0.0f;
		dvec[0].vy = 0.0f;
		dvec[0].vz = 80.0f * rnd() + 30.0f;
		dvec[1].vx = 20.0f * rnd() + 10.0f;
		dvec[1].vy = 0.0f;
		dvec[1].vz = 40.0f * rnd() + 15.0f;
		dvec[2].vx = -20.0f * rnd() - 10.0f;
		dvec[2].vy = 0.0f;
		dvec[2].vz = 40.0f * rnd() + 15.0f;
		dvec[3].vx = 0.0f;
		dvec[3].vy = 0.0f;
		dvec[3].vz = 20.0f * rnd() + 5.0f;
		
		rot.vx = ((irnd()>>16) % 512) - 256;
		rot.vy = ((irnd()>>16) % 512) - 256;
		rot.vz = ((irnd()>>16) % 256) + 896;

		DG_SetPos(&world);
		DG_RotatePos( &rot );
		DG_RotVector( dvec, dvec, 4 ) ;
		
		for( j = 0; j < 4; j++ ){
			DG_COPY_VEC( lpos, center ) ;
			DG_COPY_VEC( pos, center );
			uvrgb->a = 0x40;
			
			pos++; lpos++;
			uvrgb++;
		}
		dvec+=4;
	}
	
	memcpy( prim->pos[1], prim->pos[0], sizeof(FVECTOR)*N_SP_PRIMS*N_SP_VERTS );
	memcpy( prim->uvrgb[1], prim->uvrgb[0], sizeof(DG_PRIM2_UVRGB)*N_SP_PRIMS*N_SP_VERTS );

	DG_VisiblePrim2( prim ) ;

	return 0;
}

int BRK_ICE_SprayPrimInit( Work *work )
{
	int				i,j;
	DG_PRIM2		*prim;
	DG_TEX			*tex;
	FVECTOR			*pos;
	FVECTOR			*dvec = work->sp_dvec;
//	FVECTOR			*mpos = work->sp_mpos;
	FVECTOR			*lpos = work->sp_lpos;
	DG_PRIM2_UVRGB	*uvrgb;
	u_short			u[2],v[2];
//printf("\n\n Make Ice Spray\n");
	work->sp_flag = 0;
	
	tex = DG_GetTexture( GV_StrCode( "splash05_alp" ) );
	if( tex == NULL ){ printf("ERR!! NO TEX!!\n"); return -1; }
	
	prim = work->spray = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
									   N_SP_PRIMS, N_SP_VERTS );
	if(prim==NULL){ printf("null prim\n"); return -1; }
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) ) ;
    DG_ConfigPrim2Tex( prim, tex ) ;
	
	u[0] = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	v[0] = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	u[1] = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	v[1] = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
	
	pos = prim->pos[0];
	uvrgb = prim->uvrgb[0];

	for( i = 0; i < BRK_N_SPRAY; i++ ){
//		DG_COPY_VEC( mpos, &DG_ZeroVector );
		for( j = 0; j < 4; j++ ){
			DG_COPY_VEC( lpos, &DG_ZeroVector );
			DG_COPY_VEC( pos, &DG_ZeroVector );
			DG_COPY_VEC( dvec, &DG_ZeroVector );
			
			uvrgb->r = 0x80;
			uvrgb->g = 0x80;
			uvrgb->b = 0x80;
			uvrgb->a = 0x0;
			
			uvrgb->q = 4096;
			uvrgb->f = (i<2)?VERT_KICK_CODE:DRAW_KICK_CODE;
			
			uvrgb->u = u[j&1];
			uvrgb->v = v[j/2];

			dvec++;
			pos++; lpos++;
			uvrgb++;
		}
	//	mpos++;
	}
	
	memcpy( prim->pos[1], prim->pos[0], sizeof(FVECTOR)*N_SP_PRIMS*N_SP_VERTS );
	memcpy( prim->uvrgb[1], prim->uvrgb[0], sizeof(DG_PRIM2_UVRGB)*N_SP_PRIMS*N_SP_VERTS );

	DG_InvisiblePrim2( prim ) ;
	
	return 0;
}

