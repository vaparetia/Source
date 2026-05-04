//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_gastank.c
   フォーチュン戦 タンク壊れ

   2000/12/18 T.Morita
   $Id: brk_fl_fwheel.c,v 1.1.1.3 2002/11/19 11:46:13 Yoshizawa1 Exp $
*/
#include "fort_obj.h"

typedef struct work_brk_t
{
    GV_ACT_EX   actor ;

    FVECTOR   pos       ;
    short     rot[XY]   ;
    short     rot_v[XY] ;

    DG_OBJS           *objs  ;
    VERTEX_ANIME_WORK *anime ;

    int         frame ;
    int         flag  ;
    int         where ;
    HIDE       *hide  ;
} BrkWork ;


#define ANGtoDEG(_a) ((_a)*45/8192)
#define DEGtoANG(_a) ((_a)*8192/45)
#define ANGtoRAD(_a) ((_a)*(float)M_PI/32768.0f)

static void Die( BrkWork *work )
{
    if ( work->anime )
	ExitVertexAnimation( work->anime ) ;
    if ( work->objs  )
	DG_DequeueObjs( work->objs ), DG_FreeObjs( work->objs ) ;
}

static inline void RotateTransMatrix( FMATRIX *out,
				      short rot[XYZ], FVECTOR *pos )
{
    _sceVu0RotMatrixY( out, &DG_UnitMatrix, rot[Y]*(float)M_PI/32768.0F ) ;
    _sceVu0RotMatrixX( out, out, rot[X]*(float)M_PI/32768.0F ) ;
    _sceVu0CopyVector( (FVECTOR*)out->m[W], pos ) ;
    out->m[W][W] = 1.0f ;
}


#define FALL_ROT_MAX (1024*16)
#define FALL_ROT_V   16

static void Act( BrkWork *work )
{
    if ( work->flag )
    {
	switch( FRT_OBJ_GetTypeMasked( work->hide ) )
	{
	case FRT_TYP_FORKLIFT_B:
	    work->pos.vy -= 90.0f/work->frame ;
	    SimpleVertexAnimation( work->anime ) ;
	    if ( work->anime->count == 0 )
	    {
printf( "FrontTire B : r(%d %d) p(%f %f %f)\n",
	ANGtoDEG( work->rot[X] ),
	ANGtoDEG( work->rot[Y] ),
	work->pos.vx,
	work->pos.vy,
	work->pos.vz
	) ;
		work->flag = 0 ;
	    }
	    break ;

	case FRT_TYP_FORKLIFT_C:
	    work->rot[X] += work->rot_v[X] += FALL_ROT_V ;
	    if ( work->rot[X] > FALL_ROT_MAX )
	    {
		work->rot[X] = FALL_ROT_MAX ;
		work->rot_v[X] = -work->rot_v[X]/2 ;
		if ( !(work->rot_v[X]/(FALL_ROT_V/2)) )
      {
         // BP_WARNING - This USED to be work->rot[Z] = 0, but
         // that is invalid because rot is really just XY.  So
         // the side effect that we're keeping is that it's work->rot_v[X] = 0
         // work->rot[Z] = 0 ;
		    work->rot_v[X]=0;
          work->flag = 0 ;
      }
	    }
	    break ;

	case FRT_TYP_FORKLIFT_D:
	    break ;
	}
	RotateTransMatrix( &work->objs->world, work->rot, &work->pos ) ;
    }
}

static int GetResources( BrkWork *work, HIDE *hide, FVECTOR *pos,
			 int id, int frame )
{
    CV2_DEF *cdef ;
    DG_DEF  *def  ;
    LIT_DEF *lit_def ;
    int      i    ;

    work->where = hide->work->where ;
    work->hide  = hide ;
    work->flag  = 0 ;
    work->frame = frame ;

    _sceVu0ApplyMatrix( &work->pos, &work->hide->objs->world, pos ) ;
    work->rot[X] = hide->rot.vx ;
    work->rot[Y] = hide->rot.vy ;
    work->rot_v[X] = 300 ;

    if ( !(cdef = GV_GetCache( GV_CacheID( id, 'c' ) )) )
	PERROR( "No CV2(%d) found in data.cnf!! :: NewPutVanimeObject\n", id ) ;
    if ( !(def  = GV_GetCache( GV_CacheID( id, 'k' ) )) )
	PERROR( "No KMS(%d) found in data.cnf!! :: NewPutVanimeObject\n", id ) ;
    def->n_models = def->n_x_models = 1 ;

    if ( !(work->objs = DG_MakeObjs( def, DG_FLAG_PAINT|DG_FLAG_ONEPIECE, 0 )) )
	PERROR( "Can't Create DG_OBJS(Maybe no memory)!! :: NewPutVanimeObject\n" ) ;
    DG_QueueObjs( work->objs ) ;
    RotateTransMatrix( &work->objs->world, work->rot, &work->pos ) ;
    if ( (lit_def = GM_GetMap( hide->work->where )->light) )
	DG_MakePreshade( work->objs, lit_def ) ;

    /* アニメーション初期化 */
    if ( !(work->anime = InitVertexAnimation( work->objs->objs,
					      cdef->models,
					      DG_VANIME_VERTS,
					      cdef->n_models ) ) )
	PERROR( "InitVertexAnimation() returns NULL!! :: NewForkliftRearWheel\n" ) ;

    for ( i=0 ; i<cdef->n_models ; i++ )
	if ( cdef->models[i].n_verts == cdef->models[0].n_verts )
	{
	    work->anime->key[i] = &cdef->models[i] ;
	    work->anime->p[i] = i ? 0.0f : 1.0f ;
	}
	else
	    PERROR( "Animation Vertexies are not same!! :: NewForkliftRearWheel\n" ) ;
    SimpleVertexAnimation( work->anime ) ;
    work->anime->p[0] = 0.0f  ;
    work->anime->p[1] = 1.0f  ;
    work->anime->count = frame ;

    return 0 ;
}

static int ReceiveSignal( void *pwork, int signal, int value )
{
    BrkWork *work = pwork ;

    switch( signal )
    {
    case FRT_OBJ_FlatTire:
	work->flag = 1 ;
	break;
    default:
	return GV_DefaultSignalFunc( pwork, signal, value ) ;
    }
    return 0 ;
}

int NewForkliftFrontWheel( HIDE *hide, FVECTOR *pos, int id, int frame )
{
    BrkWork *work ;

    work = (BrkWork *)GV_NewActor( GV_ACTOR_USER, sizeof(BrkWork) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
	GV_SetActorChild( hide->work, work ) ;
	GV_SetActorSignalFunc( work, ReceiveSignal ) ;

        if( GetResources( work, hide, pos, id, frame ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return 0 ;
        }
	return 1 ;
    }
    return 0 ;
}
