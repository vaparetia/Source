/*
   brk_fed_ini.c
   雑誌壊れ 初期化

   2000/06/20 T. Morita
   $Id: brk_fed_ini.c,v 1.1.1.3 2002/11/19 11:45:27 Yoshizawa1 Exp $
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

#include "brk_fedralex.h"
#include "../brk_utl/brk_utl.x"


void BRK_FED_InitEneFind( Work *work )
{
    int grp, zone ;
    int addr ;
    extern void HZX_Pos2Zone( FVECTOR *pos, int *g, int *z ) ;

    HZX_Pos2Zone( &work->fedral->target.center, &grp, &zone ) ;
    addr = HZX_AddressNo( grp, zone, zone ) ;
    GM_SetEneFind( &work->ene_find, &work->fedral->target.center, addr, EF_TYPE_NO_FIND ) ;
    GM_PutEneFind( &work->ene_find ) ;
}

void BRK_FED_InitBroken( Work *work )
{
    if ( GCL_GetOptionValue( 'B', 0 ) )
	BRK_FED_AlreadyMessy( work ) ;
}


/*  uvいじり */
void BRK_FED_ChangeBook( FEDRAL *p )
{
    int            j, k, l ;
    DG_DEF	  *def;
    DG_MDL	  *mdl;
    DG_MDLPACK	  *mdlpack;
    DG_TEX	  *tex;
    DG_OBJ	  *obj;
    DG_OBJ_PACKET *obj_packet;
    short	  *uv_read,*uv_write;
    short	   add_v;

    obj = p->objs->objs ;
    def = p->objs->def  ;
    mdl = def->models   ;

    for( j = 0; j < def->n_models; j++ )
    {
	DG_MakeAnimVertsBuffer( &p->v_anime, obj, DG_VANIME_UVS|DG_VANIME_SINGLE ) ;
	mdlpack    = mdl->packs   ;
	obj_packet = obj->packets ;
	uv_write   = obj->uvs[0]  ;

	/* この分だけUV値のVをずらす */
	tex = (DG_TEX*)mdlpack->tex_id[0] ;
	add_v = DG_FTOI( ((tex->v_scale * (float)p->fed_type ) / 9.0f) * 4096.0f ) ;

	for( k=0 ; k<mdl->n_packs ; k++ )
	{
	    uv_read = mdlpack->uvs[0] ;

	    /* UVデータ生成 */
	    for( l=mdlpack->n_verts ; --l>=0 ; uv_read+=2, uv_write+=2 )
	    {
		uv_write[0] = uv_read[0] ;
		uv_write[1] = uv_read[1] + add_v ;
	    }
	    uv_write = (short*)((long128*)obj->uvs[0] + obj_packet->uvs_offset[0]) ;
	    mdlpack++ ;
	    obj_packet++ ;
	}
	mdl++ ;
	obj++ ;
    }
}


int BRK_FED_InitTarget( FEDRAL *fed, int where )
{
    TARGET       *t = &fed->target ;
    POWER_TARGET *p = &fed->power  ;
    FVECTOR t_size, t_pos ;
    FVECTOR u = { fed->objs->def->ux, fed->objs->def->uy, fed->objs->def->uz, 0 } ;
    FVECTOR l = { fed->objs->def->lx, fed->objs->def->ly, fed->objs->def->lz, 0 } ;

    _sceVu0SubVector( &t_size, &u, &l ) ;
    _sceVu0ScaleVector( &t_size, &t_size, 0.5f ) ;
    _sceVu0AddVector( &t_pos , &u, &l ) ;
    _sceVu0ScaleVector( &t_pos , &t_pos , 0.5f ) ;

    GM_SetTarget( t, TARGET_DEFENSE|TARGET_POWER|TARGET_ROTATE,
		  where, BOTH_SIDE, &t_size, &t_pos ) ;
    GM_SetPowerTarget( t, p, POWER_DECREASE, GM_Vitality, 0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( t, BRK_FED_TargetCallBack, fed ) ;
    GM_PutTarget( t ) ;
    GM_MoveTarget2Map( t, &fed->objs->world, where ) ;
    //NewTargetView( t,  200, 50, 32 ) ;

    return 0 ;
}


int BRK_FED_InitHazard( Work *work )
{
    int hzd_id = GCL_GetOptionValue( 'h', BRK_HZD_W01A1_TV_SPACE2 ) ;

    if ( !(work->hzd = BRK_InitHazard( hzd_id )) )
	PERROR( "Undefined Hazard ID!! :: NewPutFedralObject\n" ) ;

    return 0 ;
}



int BRK_FED_InitPiece( Work *work, int where )
{
    int     i ;
    PIECE  *p ;
    DG_COMDL_POS *pos_s, *pos_l ;

    if ( work->p_def )
    {
        if ( !(work->piece_l = DG_MakeComdl( work->p_def->models[0].packs,
                                             DG_COMDL_SEMITRANS, BRK_FED_N_PIECE/2, 0)) )
            PERROR( "Can't Make DG_COMDL(May be no memory) :: NewPutFedralObject\n" ) ;
        DG_QueueComdlObjs( work->piece_l ) ;
	GM_GroupObject( work->piece_l, where ) ;
	work->piece_l->flag |= DG_FLAG_INVISIBLE1|DG_FLAG_INVISIBLE2|DG_FLAG_INVISIBLE3 ;

        if ( !(work->piece_s = DG_MakeComdl( work->p_def->models[1].packs,
                                             DG_COMDL_SEMITRANS, BRK_FED_N_PIECE/2, 0)) )
            PERROR( "Can't Make DG_COMDL(May be no memory) :: NewPutFedralObject\n" ) ;
        DG_QueueComdlObjs( work->piece_s ) ;
	GM_GroupObject( work->piece_s, where ) ;
	work->piece_s->flag |= DG_FLAG_INVISIBLE1|DG_FLAG_INVISIBLE2|DG_FLAG_INVISIBLE3 ;

        pos_l = work->piece_l->pos ;
        pos_s = work->piece_s->pos ;
        for ( i=BRK_FED_N_PIECE, p=work->piece ; --i>=0 ; p++ )
        {
            p->pos  = i&1 ? pos_l++ : pos_s++ ;
            p->flag = 0 ;
        }
    }
    work->n_piece = BRK_FED_INACTIVE ;

    return 0 ;
}


#define FTOI12(_f)      ( DG_FTOI( ( (float)(_f)*4096.0f) ) )

DG_PRIM2 *BRK_FED_InitPrims( FMATRIX *root )
{
    int       i, rgba ;
    DG_TEX   *tex = DG_GetTexture( 14845995 /*GV_StrCode( "dankon2_alp" )*/ ) ;
    DG_PRIM2       *p     ;
    DG_PRIM2_UVRGB *uvrgb ;
    FVECTOR        *pos   ;

    if ( (p = GM_MakePrim2( DG_PRIM2_POLY| DG_PRIM2_TEX| DG_PRIM2_SHADE|
			    DG_PRIM2_ALPHA| DG_PRIM2_SINGLEBUFF,
			    BRK_FED_N_SCAR, 4 )) )
    {
	rgba  = 0x00808080 ;
	DG_ConfigPrim2Tex( p, tex ) ;
	DG_SetPrim2Alpha( p, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) ) ;
	p->root = root ;

	uvs = p->uvrgb[0] ;
	pos = p->pos  [0] ;
	for ( i=BRK_FED_N_SCAR ; --i>=0 ; uvs+=4, pos+=4 )
	{
	    uvs[0].u = uvs[2].u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	    uvs[1].u = uvs[3].u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
	    uvs[0].v = uvs[1].v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
	    uvs[2].v = uvs[3].v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
	    uvs[0].q = uvs[1].q = uvs[2].q = uvs[3].q = 4096 ;
	    uvs[0].f = uvs[1].f = 0x8fff ;
	    uvs[2].f = uvs[3].f = 0x0fff ;
	    uvs[0].r = uvs[1].r = uvs[2].r = uvs[3].r = (rgba >>  0) & 0xff ;
	    uvs[0].g = uvs[1].g = uvs[2].g = uvs[3].g = (rgba >>  8) & 0xff ;
	    uvs[0].b = uvs[1].b = uvs[2].b = uvs[3].b = (rgba >> 16) & 0xff ;
	    uvs[0].a = uvs[1].a = uvs[2].a = uvs[3].a = (rgba >> 24) & 0xff ;


	    _sceVu0CopyVector( &pos[0], &DG_ZeroVector ) ;
	    _sceVu0CopyVector( &pos[1], &DG_ZeroVector ) ;
	    _sceVu0CopyVector( &pos[2], &DG_ZeroVector ) ;
	    _sceVu0CopyVector( &pos[3], &DG_ZeroVector ) ;
	}
    }
    return p ;
}



int BRK_FED_GetOptions( Work *work, int where )
{
    int       i   ;
    FEDRAL *p   ;
    IVECTOR   buf ;
    DG_DEF  *def ;
    LIT_DEF *lit ;


    /* プロックIDの取得 */
    work->proc_id = GCL_GetOptionValue( 'C', 0 ) ;

    if ( !GCL_GetOption( 'b' ) )
	PERROR( "No Litter Model ID!!(-break option maybe missing) :: NewPutFedralObject\n" ) ;
    if ( !(work->p_def = (DG_DEF*)GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ))) )
	PERROR( "No Dust KMS-MODEL (not in data.cnf) :: NewPutFedralObject\n" ) ;

    if ( !(work->n_fedral = GCL_GetOptionValue( 'n', 0 ) ) )
	PERROR( "number must not be Zero(-number option maybe missing) :: NewPutFedralObject\n" ) ;
    if ( !(p = work->fedral = GV_Malloc( sizeof(FEDRAL) * work->n_fedral )) )
	PERROR( "Cannot allocate memory for <work->fedral> :: NewPutFedralObject\n" ) ;
    for ( i=work->n_fedral ; --i>=0 ; p++ )
    {
	_sceVu0CopyVector( &p->mov.pos  , &DG_ZeroVector ) ;
	_sceVu0CopyVector( &p->mov.pos_v, &DG_ZeroVector ) ;
	p->mov.rot_x  = p->mov.rot_y  = 0 ;
	p->mov.rot_vx = p->mov.rot_vy = 0 ;
	p->objs     = NULL ; /* malloc からの初期化なので NULL にする */
	p->scar     = NULL ; /* malloc からの初期化なので NULL にする */
	p->n_scar   =  0   ;
	p->fed_type =  0   ;
	p->y_offset = 8.0f * (float)(i%5);
	p->work     = work ;
	p->flag     = -1   ;

	/* ひとまずNULLに初期化しておく */
	p->v_anime.mem[0] = NULL ;
    }

    /* モデルIDを取得 */
    if ( !GCL_GetOption( 'm' ) )
	PERROR( "No Model ID!!(-model option maybe missing) :: NewPutFedralObject\n" ) ;
    if ( !(def = (DG_DEF*)GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ))) )
	PERROR( "No Fedral KMS-MODEL (not in data.cnf) :: NewPutFedralObject\n" ) ;

    /* ライトデータを取得 */
    lit = GM_GetMap( where )->light ;
    if ( GCL_GetOption( 'l' ) )
	if ( !(lit = GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'l' ))) )
	    PERROR( "Cannot find Light data (not in data.cnf) :: NewPutFedralObject\n" ) ;

    p = work->fedral ;
    while( (i = GCL_GetNextOption()) )
        switch( i )
        {
	case 't':
	    p->fed_type = GCL_GetNextInt();
	    if( p->fed_type < 0 || 9 <= p->fed_type )
		p->fed_type = 0 ;

	    /* モデルを作る */
	    if ( !(p->objs = DG_MakeObjs( def, DG_FLAG_PAINT| DG_FLAG_ONEPIECE, 0 )) )
		PERROR( "Can't initialize DG_OBJS(Maybe no memory) :: NewPutFedralObject\n" ) ;
	    DG_QueueObjs( p->objs ) ;
	    GM_GroupObjs( p->objs, where ) ;

	    /* テクスチャーのUVを変える */
	    BRK_FED_ChangeBook( p ) ;
	    break ;

	case 'r':
	    p->mov.rot_x = 1024 ;
	    p->mov.rot_y = GCL_GetNextInt() ;
	    break ;

	case 'p':
	    GCL_GetIV( GCL_NextStr(), (int  *)&buf ) ;
	    vu0_IV0toFV( &buf, &p->mov.pos ) ;
	    break ;

	case 'H':
	    p->y_offset =  GCL_GetNextInt() * 4.0f + 1.0f ;

	    RotateMatrixZY( &p->objs->world, &DG_UnitMatrix, p->mov.rot_x, p->mov.rot_y ) ;
	    TransMatrix( &p->objs->world, &p->mov.pos ) ;
	    DG_MakePreshade( p->objs, lit ) ;

	    if ( !(p->scar = BRK_FED_InitPrims( &p->objs->world )) )
		PERROR( "Can't initialize DG_PRIM(Maybe no memory) :: NewPutFedralObject\n" ) ;
	    BRK_FED_InitTarget( p, work->where ) ;
	    p++ ;
	    break ;
	}
    work->n_fedral |= BRK_FED_INACTIVE ;

    return 0 ;
}
