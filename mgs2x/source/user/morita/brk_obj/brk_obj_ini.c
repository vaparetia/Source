//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_obj_ini.c
   オブジェクト壊れ

   1999/12/13 T. Morita
   $Id: brk_obj_ini.c,v 1.1.1.3 2002/11/19 11:45:37 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "brk_object.h"



void BRK_OBJ_InitEneFind( Work *work )
{
    int grp, zone ;
    int addr ;
    extern void HZX_Pos2Zone( FVECTOR *pos, int *g, int *z ) ;

    HZX_Pos2Zone( &work->obj->pos, &grp, &zone ) ;
    addr = HZX_AddressNo( grp, zone, zone ) ;
    GM_SetEneFind( &work->ene_find, &work->obj->pos, addr, EF_TYPE_NO_FIND ) ;
    GM_PutEneFind( &work->ene_find ) ;
}

void BRK_OBJ_InitBroken( Work *work )
{
    if ( GCL_GetOptionValue( 'B', 0 ) )
	BRK_OBJ_AlreadyMessy( work ) ;
}


static int BRK_OBJ_InitTarget( TARGET *t, POWER_TARGET *p, int map, int flag,
			       float *uv, float *lv,
			       FVECTOR *pos,
			       void (*callback)( TARGET *off, TARGET *def, void *ptr ), void *ptr )
{
    FVECTOR t_size, t_pos ;
    FVECTOR u = { uv[X],uv[Y],uv[Z],0 }, l = { lv[X],lv[Y],lv[Z],0 } ;

    _sceVu0SubVector( &t_size, &u, &l ) ;
    _sceVu0ScaleVector( &t_size, &t_size, 0.5f ) ;
    _sceVu0AddVector( &t_pos, &u, &l ) ;
    _sceVu0ScaleVector( &t_pos , &t_pos , 0.5f ) ;

    GM_SetTarget( t, TARGET_DEFENSE|TARGET_POWER|TARGET_SEEK|flag,
		  map, BOTH_SIDE, &t_size, &t_pos ) ;
    GM_SetPowerTarget( t, p, POWER_DECREASE, GM_Vitality, 0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( t, callback, ptr ) ;
    GM_PutTarget( t ) ;
    GM_MoveTargetMap( t, pos, map ) ;

    //NewTargetView( t,  200, 50, 32 ) ;

    return 0 ;
}


int BRK_OBJ_InitHazard( Work *work, int where )
{
    int hzd_id ;

    hzd_id = GCL_GetOptionValue( 'A', BRK_HZD_W01D_STORERAGE ) ;
    work->hzx = GM_GetHzxGroupID( where ) ;
    if ( !(work->hzd = BRK_InitHazard( hzd_id )) )
	return -1 ;
    return  0 ;
}

static void *BRK_OBJ_SetHazard( int flag, FVECTOR *pos,
				float *uv, float *lv, DG_OBJS *objs )
{
    FVECTOR t_size, t_pos ;
    FVECTOR u = { uv[X],uv[Y],uv[Z],0 }, l = { lv[X],lv[Y],lv[Z],0 } ;

    _sceVu0SubVector( &t_size, &u, &l ) ;
    _sceVu0ScaleVector( &t_size, &t_size, 0.5f ) ;
    _sceVu0AddVector( &t_pos, &u, &l ) ;
    _sceVu0ScaleVector( &t_pos , &t_pos , 0.5f ) ;

    return BRK_MakeHazard( flag|BRK_HZD_ROTATE,
			   &objs->world, NULL, &t_size, &t_pos, NULL ) ;
}

int BRK_OBJ_InitBox( Work *work, int map )
{
    DG_DEF  *def ;
    BOX     *b ;
    IVECTOR  buf ;
    int      i ;

    /* プロックIDの取得 */
    work->proc_id = GCL_GetOptionValue( 'C', 0 ) ;

    if ( !(i = GCL_GetOptionValue( 'b', 0 )) )
	return 1 ;
    if ( !(def = (DG_DEF*)GV_GetCache( GV_CacheID( i, 'k' )) ) )
	PERROR( "Model not found :: NewPutBreakObject\n" ) ;
    if ( !(b = work->box = GV_Malloc( sizeof(BOX) )) )
	PERROR( "No Memory for <work->box> :: NewPutBreakObject\n" ) ;
    b->hzd  = NULL ;
    b->objs = NULL ;/*これらがないと正常にDieされない可能性がある*/

    if ( GCL_GetOption( 'g' ) )
    {
        GCL_GetIV( GCL_NextStr(), (int *)&buf ) ;
        vu0_IV0toFV( &buf, &b->pos ) ;
	GCL_GetSV( GCL_NextStr(), (short*)&b->brot ) ;
    }
    else
	PERROR( "Position not specified(-geometry option missing) :: NewPutBreakObject\n" ) ;

    if ( !(b->objs = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_FINISHCALC, 0 )) )
	PERROR( "Can't Make DG_OBJS(May be no memory) :: NewPutBreakObject\n" ) ;

    /* マトリックスの初期化 */
    RotateMatrix( &b->objs->world, &DG_UnitMatrix, &b->brot ) ;
    TransMatrix( &b->objs->world, &b->pos ) ;

    /* モデルの初期化 */
    if ( !(work->flag & BRK_F_NO_BOXSHOW) )
    {
	DG_QueueObjs( b->objs ) ;
	//DG_GetLightMatrixFix( &b->pos, b->lights ) ;
	DG_SetLightMatrix( b->objs, b->lights ) ;

	/* マトリックスの初期化 */
	for ( i=b->objs->n_models ; --i>=0 ; )
	{
	    FVECTOR pos ;

	    b->objs->objs[i].world = b->objs->world ;
	    pos.vx = b->objs->def->models[i].tx + b->objs->def->tx ;
	    pos.vy = b->objs->def->models[i].ty + b->objs->def->ty ;
	    pos.vz = b->objs->def->models[i].tz + b->objs->def->tz ;
	    _sceVu0AddVector( &pos, &b->pos, &pos ) ;
	    TransMatrix( &b->objs->objs[i].world, &pos ) ;
	    b->rot[i] = b->rot_v[i] = 0 ;
	}

	/* うまく表示するようバウンダリを調整 */
	def->ux += 500.0f, def->uy += 500.0f, def->uz += 500.0f ;
	def->lx -= 500.0f, def->ly -= 500.0f, def->lz -= 500.0f ;
    }

    if ( !(work->flag & BRK_F_NO_BOXTRG) )
    {
	/* モデル情報よりターゲットを設定 */
	BRK_OBJ_InitTarget( &b->target, &b->power, map,
			    work->flag & BRK_F_NO_BOXSHOW ? TARGET_SKIP : 0,
			    &def->models->ux, &def->models->lx,
			    &b->pos, /*BRK_OBJ_BoxTargetCallBack*/NULL, work ) ;

	/* モデル情報より当たりを設定 */
	if ( !(b->hzd = BRK_OBJ_SetHazard( work->n_obj ? BRK_HZD_Y_BOX : BRK_HZD_OUTSIDE,
					   &b->pos, &def->models->ux, &def->models->lx, b->objs )) )
	    PERROR( "Put hazard failed :: NewPutBreakObject\n" ) ;
    }

    /* 各変数の初期化 */
    b->vitality = BRK_OBJ_BOXVITALITY ;
    b->act      = NULL ;
    work->flag |= BRK_F_INSIDE_BOX ;

    return 0 ;
}



int BRK_OBJ_InitObject( BRK_OBJ *p, DG_DEF *def, int flag, void *act )
{
    int      i ;
    DG_OBJS *objs = p->objs ;
    union light_t *light = &p->work->light ;
    FVECTOR pos ;

    p->pos_v = DG_ZeroVector ;
    p->rot_v = DG_ZeroSVector ;
    p->act   = act ;
    if ( !(p->objs = DG_MakeObjs( def, flag, 0 )) )
	PERROR( "Can't Make DG_OBJS(May be no memory) :: NewPutBreakObject\n" ) ;
    DG_QueueObjs( p->objs ) ;
    GM_GroupObjs( p->objs, p->work->where ) ;
    if ( flag & DG_FLAG_PAINT )
	DG_MakePreshade( p->objs, light->def ) ;
    else
	DG_SetLightMatrix( p->objs, light->mtx ) ;

    RotateMatrix( &p->objs->world, &DG_UnitMatrix, &p->rot ) ;
    TransMatrix( &p->objs->world, &p->pos ) ;
    /* マトリックスの初期化 */
    if ( flag & DG_FLAG_FINISHCALC )
	for ( i=p->objs->n_models ; --i>=0 ; )
	{
	    p->objs->objs[i].world = p->objs->world ;
	    pos.vx = p->objs->def->models[i].tx + p->objs->def->tx ;
	    pos.vy = p->objs->def->models[i].ty + p->objs->def->ty ;
	    pos.vz = p->objs->def->models[i].tz + p->objs->def->tz ;
	    TransMatrix( &p->objs->objs[i].world, &pos ) ;
	}

    if ( objs )
    {
	if ( flag & DG_FLAG_PAINT )
	    DG_FreePreshade( objs ) ;
	DG_DequeueObjs( objs ) ;
	DG_FreeObjs( objs );
    }
    return  0 ;
}


static DG_COMDL *GetComdl( Work *work, int i, DG_DEF *def, int n_piece )
{
    DG_COMDL *comdl ;

    comdl = DG_MakeComdl( def->models[i].packs,
			   DG_COMDL_NOFOG|DG_COMDL_SEMITRANS,
			   n_piece, 0 ) ;
    if ( comdl )
    {
	DG_QueueComdlObjs( comdl ) ;
	GM_GroupObject( comdl, work->where ) ;
	/* 子画面には見えないようにする */
	comdl->flag |= (DG_FLAG_INVISIBLE1|
			DG_FLAG_INVISIBLE2|
			DG_FLAG_INVISIBLE3) ;
    }
    return comdl ;
}

static int BRK_OBJ_InitPieces( Work *work, DG_DEF *def, FVECTOR *pos )
{
    int i ;
    DG_COMDL_POS *pos_s, *pos_l ;
    PIECE *p ;
    FMATRIX light[2] ;
    FVECTOR c = { 0.5f, 0.5f, 0.5f, 1.0f } ;
    int idx1, idx2 ;

    pos_l = pos_s = NULL ;
    idx1 = -1 ;
    idx2 = -1 ;
    for( i=0 ; i<(int)def->n_x_models ; i++ )
	if ( def->models[i].n_packs > 0 )
	{
	    if ( idx1<0 )
		idx1 = i ;
	    else if ( idx2<0 )
		idx2 = i ;
	}

    if ( idx1 >= 0 )
    {
	work->comdl[0] = GetComdl( work, idx1, def,
				   BRK_N_PIECE/(idx2<0? 1 : 2) ) ;
	pos_l = work->comdl[0]->pos ;
	pos_s = work->comdl[0]->pos ;
    }
    if ( idx2 >= 0 )
    {
	work->comdl[1] = GetComdl( work, idx2, def, BRK_N_PIECE/2 ) ;
	pos_s = work->comdl[1]->pos ;
    }
printf( "pos%x %x\n",  pos_l, pos_s ) ;
    if ( pos_l && pos_s )
    {
	DG_GetLightMatrix( pos, light ) ;
	c.vx *= GV_VecLen3F( (FVECTOR*)&light[0].m[X] ) ;
	c.vy *= GV_VecLen3F( (FVECTOR*)&light[0].m[Y] ) ;
	c.vz *= GV_VecLen3F( (FVECTOR*)&light[0].m[Z] ) ;
	_sceVu0ApplyMatrix( &c, &light[1], &c ) ;

	p = work->piece ;
	for ( i=BRK_N_PIECE/2 ; --i>=0 ; pos_s++, pos_l++ )
	{
	    (p++)->comdl = pos_s ;
	    (p++)->comdl = pos_l ;
	    pos_l->world.m[W][W] = pos_s->world.m[W][W] = 0.0f ;
	    pos_l->color.vx = pos_s->color.vx = (int)c.vx ;
	    pos_l->color.vy = pos_s->color.vy = (int)c.vy ;
	    pos_l->color.vz = pos_s->color.vz = (int)c.vz ;
	    pos_l->color.vw = pos_s->color.vw = 0 ;
	}
    }
    work->n_piece = 0 ;

    return 0 ;
}

int BRK_OBJ_GetOptions( Work *work, int where )
{
    int     i, flag ;
    IVECTOR buf ;
    char   *c ;
    DG_DEF *m_def, *p_def, *c_def=NULL ;
    BRK_OBJ    *o ;
    FVECTOR pos ;

    if ( GCL_GetOption( 'm' ) )
    {
	i = GCL_GetNextInt() ;
	if ( !(m_def = (DG_DEF*)GV_GetCache( GV_CacheID( i, 'k' ) )) )
	    PERROR( "Model<%d> not found !! (before break KMS missing) :: NewPutBreakObject\n", i ) ;

	i = GCL_GetNextInt() ;
	if ( !(p_def = (DG_DEF*)GV_GetCache( GV_CacheID( i, 'k' ) )) )
	    PERROR( "Break Model<%d> not found !! (after break KMS missing) :: NewPutBreakObject\n", i ) ;

	if ( GCL_NextStr() )
	{
	    i = GCL_GetNextInt() ;
	    if ( !(c_def = (DG_DEF*)GV_GetCache( GV_CacheID( i, 'k' ) )) )
		PERROR( "CoModel<%d> not found !! (dust KMS missing) :: NewPutBreakObject\n", i ) ;
	}
	p_def->ux = BRK_BOUND_X ; p_def->lx = -BRK_BOUND_X ;
	p_def->uy = BRK_BOUND_Y ; p_def->ly = -BRK_BOUND_Y ;
	p_def->uz = BRK_BOUND_Z ; p_def->lz = -BRK_BOUND_Z ;
	work->part_def = p_def ;
    }
    else
	PERROR( "No models specified!! (-m option missing) :: NewPutBreakObject\n" ) ;

    work->brk_se_id = GCL_GetOptionValue( 'E', SD_A_RICVGT02 ) ;
    work->drp_se_id = GCL_GetOptionValue( 'S', SD_A_VGTFAL02 ) ;
    work->dust_mode = GCL_GetOptionValue( 'd', -1 ) ; /* 岡嶋さんキャラ呼出モード指定 */
    work->flag   = GCL_GetOptionValue( 'f', 0  ) ;
    work->stable = (float)GCL_GetOptionValue( 's', 50 ) ;
    work->n_foot = GCL_GetOptionValue( 'h', 0  ) ;
    work->n_obj  = GCL_GetOptionValue( 'n', 0  ) ;

    if ( !(work->flag & BRK_F_BRK_SPREAD) )
    {
	work->r_size.vx = (m_def->ux - m_def->lx)*0.5f ;
	work->r_size.vy = (m_def->uy - m_def->ly)*0.5f ;
	work->r_size.vz = (m_def->uz - m_def->lz)*0.5f ;
    }
    else
    {
	work->r_size.vx = 40.0f ;
	work->r_size.vy = 40.0f ;
	work->r_size.vz = 40.0f ;
    }

    if ( GCL_GetOption( 'e' ) )
    {
	work->explode = (float)GCL_GetNextInt()/10.0f ;
	work->pop_up  = (float)GCL_GetNextInt()       ;
    }
    else
	work->explode = 1.0f, work->pop_up = 30.0f ;

    if ( (i = GCL_GetOptionValue( 'l', 0 )) )
    {
        flag = DG_FLAG_PAINT ;
	if ( !(work->light.def = (LIT_DEF *)GV_GetCache( GV_CacheID( i, 'l' ) )) )
	    PERROR( "Light not found :: NewPutBreakObject\n" ) ;
    }
    else
	flag = DG_FLAG_SHADE ;

    if ( BRK_OBJ_InitBox( work, where ) < 0 )
	return -1 ;

    if ( !(work->obj = GV_Malloc( sizeof(BRK_OBJ) * (work->n_obj + work->n_foot * 2) )) )
	PERROR( "Cannot alloc memory (work->obj) :: NewPutBreakObject\n" ) ;
    for ( i=work->n_obj, o=work->obj ; --i>=0 ; o++ )
	o->objs = NULL ;

    if ( GCL_GetOption( 'p' ) )
	for ( i=work->n_obj, o=work->obj ; --i>=0 ; o++ )
	{
	    if ( !(c = GCL_NextStr()) )
		PERROR( "No position data (-p option):: NewPutBreakObject\n" ) ;
	    GCL_GetIV( c, (int *)&buf ) ;
	    vu0_IV0toFV( &buf, &o->pos ) ;
	    if ( !(c = GCL_NextStr()) )
		PERROR( "No rotation data (-p option):: NewPutBreakObject\n" ) ;
	    GCL_GetSV( c, (short*)&o->rot ) ;
	    if ( work->flag & BRK_F_INSIDE_BOX )
	    {
		o->rot.vx += work->box->brot.vx ;
		o->rot.vy += work->box->brot.vy ;
		o->rot.vz += work->box->brot.vz ;
		o->pos.vw =1.0f ;
		_sceVu0ApplyMatrix( &o->pos, &work->box->objs->world, &o->pos ) ;
	    }
	    _sceVu0AddVector( &pos, &pos, &o->pos ) ;/*オブジェクトの中心を求めるため*/

	    o->n_parts = 0 ;
	    o->work = work ;
	    if ( BRK_OBJ_InitObject( o, m_def, flag|DG_FLAG_ONEPIECE, BRK_OBJ_ActMove ) )
		PERROR( "InitObject Error :: NewPutBreakObject\n" ) ;
	    BRK_OBJ_InitTarget( &o->target, &o->power, where, 0,
				&m_def->ux, &m_def->lx, &o->pos, BRK_OBJ_TargetCallBack, o ) ;
	    if ( !(work->flag & BRK_F_NO_BOXSHOW) )
		DG_InvisibleObjs( o->objs ) ;
	}
    else
	PERROR( "No position specified!! (-p option missing) :: NewPutBreakObject\n" ) ;
    work->foot = o ;

    if ( flag == DG_FLAG_SHADE )
    {
		if ( work->n_obj ) {
			_sceVu0ScaleVector( &pos, &pos, 1.0f/work->n_obj ) ;
			DG_GetLightMatrixFix( &pos, work->light.mtx ) ;
		} else {
		}
    }

    if ( c_def )
	if ( BRK_OBJ_InitPieces( work, c_def, &pos ) )
	    PERROR( "InitPieces Error :: NewPutBreakObject\n" ) ;

    return 0 ;
}
