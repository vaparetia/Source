//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_anydust.c
   フォーチュン戦  屑飛び

   2000/12/15 T.Morita
   $Id: brk_anydust.c,v 1.1.1.3 2002/11/19 11:46:13 Yoshizawa1 Exp $
*/
#include "fort_obj.h"


#define  BRK_N_PIECE 64

#define BRK_VEL_R        0.85f
#define BRK_ROT_R        16
#define BRK_BOUNCE       1.68f
#define BRK_BOUNCE_WALL  1.38f
#define BRK_N_SOUND      4

#define ACT_TIME         (5*60*5/TIME_BASE)

typedef struct move_t
{
    FVECTOR pos   ;
    FVECTOR pos_v ;

    short   rot_x,  rot_y  ;
    short   rot_vx, rot_vy ;

    int          flag   ;
    float        push_up ;
    float        scale  ;
    DG_MDL       *model ;
    DG_COMDL_POS *comdl ;
#if MAKING
    void         *wireframe ;
#endif
} MOVE ;

typedef struct work_brk_t
{
    GV_ACT_EX   actor    ;

    DG_COMDL   *piece[2] ;
    HZD_BOX    *hzd      ;
    int         where    ;

    int         flag     ;
    float       gravity  ;
    int         life     ;
    int         act_life ;

    int        *sound      ;
    int         n_sound    ;
    int         sound_tics ;
    int         sound_next ;
    int         sound_no   ;

int count ;

    MOVE       *moves    ;
    int         n_moves  ;
} BrkWork ;


static void RotateMatrixXY( MOVE *p )
{
    int r ;

    r = p->rot_x & 0x0fff ;
    _sceVu0RotMatrixX( &p->comdl->world, &DG_UnitMatrix,
		       (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = p->rot_y & 0x0fff ;
    _sceVu0RotMatrixY( &p->comdl->world, &p->comdl->world,
		       (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
}

static void TransMatrix( MOVE *p, int where )
{
    FMATRIX mtx ;

    /* スケール */
    if ( p->scale != 1.0f )
    {
	_sceVu0CopyMatrix( &mtx, &DG_UnitMatrix ) ;
	mtx.m[X][X] = p->scale ;
	mtx.m[Z][Z] = p->scale ;
	_sceVu0MulMatrix( &p->comdl->world, &p->comdl->world, &mtx ) ;
    }

    /* 位置の更新 */
    _sceVu0CopyVectorXYZ( (FVECTOR *)p->comdl->world.m[W], &p->pos ) ;

    BRK_UTL_ComdlColor2( p->comdl, Z ) ;
    p->comdl->world.m[W][Y] += p->push_up ;
}

static void SetSound( BrkWork *work, FVECTOR *pos )
{
    static int  NextTic[] = { 4, 16, 22, 4, 40, 50 } ;
    int vol, se, pan ;
    float bp_angle;

    if ( work->sound_no < 6 )
	if ( GM_StagePlayTime - work->sound_tics == work->sound_next || !work->sound_next )
	{
	    vol = ((6 - work->sound_no) * 8) & 0x3f ;
	    se  = work->sound[work->sound_no % work->n_sound] ;
	    pan = GM_SeGetPan( pos, GM_SEMODE_NORMAL, &bp_angle ) ;
	    GM_SeSet3D( pan, vol, se, bp_angle ) ;

	    work->sound_tics = GM_StagePlayTime ;
	    work->sound_next = NextTic[work->sound_no++] + (irnd() & 7) ;
	}
}


/***

  物体移動関数

  ***/
/* 当たりを見て移動 */
static int MoveObject( BrkWork *work, MOVE *p )
{
    int flag = 0 ;
    static FVECTOR bounce = { BRK_BOUNCE_WALL, BRK_BOUNCE, BRK_BOUNCE_WALL, 0 } ;
    FVECTOR size ;

    p->pos_v.vy -= work->gravity ;
#if 0
    _sceVu0CopyMatrix( &p->comdl->world, (FMATRIX *)SCRPAD_ADDR + p->rot_x/256*16 + p->rot_y/256 ) ;
    p->rot_x = p->rot_x>2048 ? p->rot_x-4096 : p->rot_x ;
    p->rot_y = p->rot_y>2048 ? p->rot_y-4096 : p->rot_y ;
#else
    RotateMatrixXY( p ) ;
#endif

    BRK_UTL_SizeOfMDL( p->model, &p->comdl->world, &size ) ;
    flag = BRK_CheckHazard( work->hzd, &p->pos, &p->pos_v, &bounce, &size ) ;
    {
//extern int BRK_HZD_Flag ;
//extern HZD_BOX *BRK_HZD_Hazard  ;

//printf( "f(%d %x %x) ", flag,BRK_HZD_Flag, BRK_HZD_Hazard?BRK_HZD_Hazard->flag:0  ) ;
    }
    switch( flag )
    {
    case 3:
    case 1:
        _sceVu0ScaleVector( &p->pos_v, &p->pos_v, BRK_VEL_R + frnd()*0.1f ) ;
	if ( BRK_UTL_PutCenterHazard( work->hzd,
				      &p->pos, &p->pos_v, 3000.0f, 50.0f ) )
	{
//printf( "Illeagal Bounce x%.2f z%.2f\n", p->pos_v.vx,p->pos_v.vz ) ;
	    p->pos_v.vy = 10.0f ;
	    flag = 3 ;
	}
	else
	{
//printf( "(%f %f %f)", p->pos_v.vx, p->pos_v.vy, p->pos_v.vz ) ;
	    flag = (int)(p->pos_v.vy/16.0f) + 2 ;
	    p->rot_vx = ( (p->rot_x/1024 ? 2047 : 0)*(p->rot_x>0 ? 1 : -1) - p->rot_x) / 16 ;
	    if ( flag == 2 )
		if ( !(int)p->pos_v.vx && !(int)p->pos_v.vz )
		    p->rot_vx=0, flag = 1 ;
	}
        break ;
    case 0:
	p->rot_x = (p->rot_x + p->rot_vx)&4095 ;
	p->rot_y = (p->rot_y + p->rot_vy)&4095 ;
	p->rot_x = p->rot_x>2048 ? p->rot_x-4096 : p->rot_x ;
	p->rot_y = p->rot_y>2048 ? p->rot_y-4096 : p->rot_y ;
        _sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ) ;
    }
    TransMatrix( p, work->where ) ;

    return flag ;
}

/* 0°or 180°に倒れ込み */
static int FallDownObject( BrkWork *work, MOVE *p )
{
    int r, flag ;
    FVECTOR size ;

    if ( p->rot_x == 2048 || p->rot_x == -2048 || p->rot_x == 0 )
	flag = 0 ;
    else
    {
	r = p->rot_x + (p->rot_vx += ( p->rot_x/1024 ? 10 : -10) * (p->rot_x>0 ? 1 : -1) ) ;
	if ( (p->rot_x>0 && r<=0) || (p->rot_x<=0 && r>0) )
	    flag = 0, r = 0 ;
	else if ( (!(p->rot_x/2048) && r/2048) || (p->rot_x/2048 && !(r/2048)) )
	    flag = 0, r = p->rot_x>0 ? 2048 : -2048 ;
	else
	    flag = 2 ;
	p->rot_x = r ;
    }
    p->rot_y += p->rot_vy = p->rot_vy * (BRK_ROT_R-1) / BRK_ROT_R ;
    p->pos_v.vy = -0.5f * work->gravity ;
    RotateMatrixXY( p ) ;
    BRK_UTL_SizeOfMDL( p->model, &p->comdl->world, &size ) ;
    _sceVu0ScaleVector( &size, &size, p->scale ) ;
    if ( !BRK_CheckHazard( work->hzd, &p->pos, &p->pos_v, &BRK_HZD_NoBounce, &size ) )
	p->pos.vy += p->pos_v.vy, flag = 1 ;
    TransMatrix( p, work->where ) ;

    return flag ;
}

static void Die( BrkWork *work )
{
#if MAKING
    int  k ;

    for ( k=0; k<work->n_moves ; k++ )
	if ( work->moves[k].wireframe )
	    GV_DestroyOtherActor( work->moves[k].wireframe ) ;
#endif

    if ( work->piece[0] )
	DG_DequeueComdlObjs( work->piece[0] ), DG_FreeComdl( work->piece[0] ) ;
    if ( work->piece[1] )
	DG_DequeueComdlObjs( work->piece[1] ), DG_FreeComdl( work->piece[1] ) ;
    if ( work->moves )
	GV_Free( work->moves ) ;
}

static void Act( BrkWork *work )
{
    int     i ;
    MOVE   *m ;

    if ( work->flag )
    {
	work->flag = 0 ;
	//BRK_MoveHazardToScr( work->hzd ) ;
	for( i=work->n_moves, m=work->moves ; --i>=0 ; m++ )
	{
	    switch( m->flag )
	    {
	    case 2:
//printf( "[%x]MOVE Start\n", work );
		if ( MoveObject( work, m ) == 1 )
		    m->flag = 1 ;
//printf( "[%x]MOVE End\n", work );
		break ;
	    case 1:
		if ( !FallDownObject( work, m ) )
		    m->flag = 0 ;
		break ;
	    case -1:
//printf( "[%x] VANISH %f\n", work, m->pos_v.vy ) ;
		if ( m->comdl->color.vw )
		    if ( --m->comdl->color.vw == 0 ) /* 消えた粒は,止める */
			m->flag = 0 ;
	    case  0:
		break ;
	    default:
		m->flag-- ;
		break ;
	    }

	    /* アクト時間を越えた消す(こいつは2度と復活しない) */
	    if ( work->act_life <= 0 && m->flag > 0 )
		m->flag = -1 ;
	    work->flag |= m->flag ;
	}
	//BRK_ExitHazardToScr() ;

	/* 強制アクト止め */
	if ( work->act_life > 0 )
	    work->act_life-- ;

	/* サウンドを鳴らす */
	if ( work->sound )
	    SetSound( work, &work->moves->pos ) ;
    }

    /*ライフでフェードアウト*/
    if ( work->life > 0 )
	work->life-- ;
    else if ( !work->life )
    {
	int flag = 0 ;

	work->where = 0 ;
	m = work->moves ;
	for( i=work->n_moves ; --i>=0 ; m++ )
	    if ( m->comdl->color.vw )
		flag |= --m->comdl->color.vw ;
	if ( !flag )
	    GV_DestroyActor( work ) ;
    }
}

static int InitMovesParam( BrkWork *work, int j, int n_moves,
			   DG_MDL *mdl, DG_COMDL_POS *comdl,
			   TARGET *trgt, HIDE *hide,
			   float velocity, float scale )
{
    FVECTOR *pos_v ;
    FVECTOR dist ;
    SVECTOR *rot = &hide->rot ;
    DG_OBJS *box =  hide->objs ;
    FVECTOR ofst={ box->def->ux + box->def->lx,
		   box->def->uy + box->def->ly,
		   box->def->uz + box->def->lz, 2.0f } ;
    FVECTOR m_size={ (mdl->ux - mdl->lx)*0.0f,
		     (mdl->uy - mdl->ly)*0.0f,
		     (mdl->uz - mdl->lz)*0.0f, 0.0f } ;
    FVECTOR b_size ;

    scale -= 1.0f ;
    pos_v = trgt ? &trgt->power->force : &DG_ZeroVector ;

    /* 箱の形を計算して出すため */
    _sceVu0ScaleVector( &ofst, &ofst, 0.5f ) ;
    _sceVu0ScaleVector( &b_size, &hide->target.size, 2.0f ) ;
    _sceVu0CopyVector( &ofst, &hide->target.offset ) ;

    while( --j>=0 )
    {
	MOVE *m = &work->moves[n_moves++] ;
	int i = (j/2) % 3 ;

	/*パラメータの初期値*/
	m->comdl   = &comdl[j] ;
	m->model   = mdl ;
	m->push_up = 3.0f*frnd() ;
	m->scale   = 1.0f + scale*rnd() ;

	/*位置はモデルサイズに合わせる（外周に沿わせる取り敢えず
	  ローカルを設定しワールドに変換する） */
	m->pos.vx = (i==0 ? b_size.vx : (b_size.vx - m_size.vx)*frnd()) ;
	m->pos.vy = (i==1 ? b_size.vy : (b_size.vy - m_size.vy)*frnd()) ;
	m->pos.vz = (i==2 ? b_size.vz : (b_size.vz - m_size.vz)*frnd()) ;
	_sceVu0ScaleVector( &m->pos, &m->pos, (j&1 ? -0.5f : 0.5f) ) ;
	_sceVu0AddVector( &m->pos, &m->pos, &ofst ) ;
	m->pos.vw = 1.0f ;
	_sceVu0ApplyMatrix( &m->pos, &box->world, &m->pos ) ;

	/*位置,速度,回転,回転速度などの設定*/
	/*速度はランダムに*/
	if ( !trgt )
	{
	    pos_v = &m->pos_v ;
	    _sceVu0SubVector( pos_v, &m->pos, (FVECTOR*)box->world.m[W] )  ;
	}
	_sceVu0Normalize( &m->pos_v, pos_v ) ;
	m->pos_v.vx += frnd()*0.5f ;
	m->pos_v.vy +=  rnd()*2.0f ;
	m->pos_v.vz += frnd()*0.5f ;
	_sceVu0ScaleVector( &m->pos_v, &m->pos_v, velocity ) ;

	/* 箱に合わせて回転を設定する */
	m->rot_vx = (irnd()&0x70)-64 ;
	m->rot_vy = (irnd()&0x38)-32 ;
	m->rot_x = rot->vx + (i!=1 ? 1024 : 0) * (j&1 ? -1 : 1) ;
	m->rot_y = rot->vy + (i==0 ? 1024 : 0) * (j&1 ? -1 : 1) ;
	RotateMatrixXY( m ) ;
	TransMatrix( m, work->where ) ;

	/*着弾から遠いものは遅れて動き出す。*/
	m->flag = 2 ;
	if ( trgt )
	{
	    _sceVu0SubVector( &dist, &trgt->hit, &m->pos ) ;
	    m->flag += (int)(sceVu0Sqrt( _sceVu0InnerProduct( &dist, &dist ) ) / 200.0f) ;
	}
	m->comdl->color.vw = 128 ;

#if MAKING
	{
	    extern void *NewDrawWirePacks( DG_OBJPACK *packs, int n_packs, FMATRIX *mtx ) ;
	    m->wireframe = NewDrawWirePacks( mdl->packs, 1, &m->comdl->world ) ;
	}
#endif

    }

    return n_moves ;
}


static int InitMovesFlat( BrkWork *work, int j, int n_moves,
			  DG_MDL *mdl, DG_COMDL_POS *comdl,
			  FVECTOR *pos, HIDE *hide,
			  float velocity, float scale )
{
    FVECTOR DownSpeed = { 0.0f,-2000.0f,0.0f,0.0f } ;

    scale -= 1.0f ;
    while( --j>=0 )
    {
	MOVE *m = &work->moves[n_moves++] ;

	/*パラメータの初期値*/
	m->comdl   = &comdl[j] ;
	m->model   = mdl ;
	m->push_up = 3.0f*frnd() ;
	m->scale   = 1.0f + scale*rnd() ;

	DownSpeed.vy = -2000.0f ;
	/*位置はモデルサイズに合わせる（外周に沿わせる取り敢えず
	  ローカルを設定しワールドに変換する） */
	m->pos.vx = pos->vx + 2000.0f * frnd() ;
	m->pos.vz = pos->vz + 5000.0f * rnd()  ;
	m->pos.vy = -45000.0f + m->push_up + (mdl->uy - mdl->ly) * 0.5f ;

	/* 箱に合わせて回転を設定する */
	m->rot_x = j&1 ? 0: 2048 ;
	m->rot_y = irnd() & 4095 ;
	RotateMatrixXY( m ) ;
	TransMatrix( m, work->where ) ;

	/*着弾から遠いものは遅れて動き出す。*/
	m->flag = 0 ;
	m->comdl->color.vw = 128 ;

#if MAKING
	{
	    extern void *NewDrawWirePacks( DG_OBJPACK *packs, int n_packs, FMATRIX *mtx ) ;
	    m->wireframe = NewDrawWirePacks( mdl->packs, 1, &m->comdl->world ) ;
	}
#endif

    }

    return n_moves ;
}

static int GetResources( BrkWork *work, HIDE *hide, TARGET *trgt,
			 int n_dust, int type, int id,
			 float gravity, float velocity, float scale, int life,
			 int *sound, int n_sound )
{
    int      i ;
    int      n_moves ;
    DG_DEF  *def ;
    DG_MDL  *mdl ;

    //if ( GM_GameLevel >= GM_LEVEL_EXTREME )
	n_dust /= 2 ;

    work->where   = hide->work->where ;
    work->hzd     = hide->work->hzd   ;
    work->n_moves = n_dust & 0xfffffffe ;
    work->flag    = 1 ;
    work->gravity = gravity ;
    work->life    = life    ;
    work->act_life = ACT_TIME ;

    work->sound   = sound   ;
    work->n_sound = n_sound ;

    if ( !(work->moves = GV_Malloc( work->n_moves * sizeof(MOVE) )) )
	PERROR( "Can't allocate memory :: NewIronBoxDust\n" ) ;
    n_moves = 0 ;
    for( i=2 ; --i>=0 ; )
    {
	if ( !(def = (DG_DEF *)GV_GetCache( GV_CacheID( id, 'k' ) )) )
	    PERROR( "Can't find COMDEL<%d>(Not in data.cnf) :: NewIronBoxDust\n", id ) ;
	mdl = &def->models[i] ;
	if ( !(work->piece[i] = DG_MakeComdl( mdl->packs, DG_COMDL_SEMITRANS, work->n_moves/2,0 )) )
	    PERROR( "Can't make DG_COMDL(Maybe no memory) :: NewIronBoxDust\n" ) ;
	DG_QueueComdlObjs( work->piece[i] ) ;
	GM_GroupObject( work->piece[i], hide->work->where ) ;

	if ( type )
	    n_moves = InitMovesFlat( work, work->n_moves/2, n_moves,
				     mdl, work->piece[i]->pos,
				     (FVECTOR *)trgt, hide, velocity, scale ) ;
	else
	    n_moves = InitMovesParam( work, work->n_moves/2, n_moves,
				      mdl, work->piece[i]->pos,
				      trgt, hide, velocity, scale ) ;

    }
    return 0 ;
}

static int ReceiveSignal( void *pwork, int signal, int value )
{
    BrkWork *work = pwork ;
    int i ;

    switch( signal )
    {
    case FRT_OBJ_Destroyed:
	/* 床より上にあり,消えていないもののみ動かす */
	if ( work->flag >= 0 )
	    for( i=work->n_moves ; --i>=0 ; )
		if ( work->moves[i].pos.vy > -44900.0f && 
		     work->moves[i].flag >= 0 )
		    work->moves[i].flag = 2 ;
	work->flag = -1 ;
	work->act_life = ACT_TIME ;
	break ;
    default:
	return GV_DefaultSignalFunc( pwork, signal, value ) ;
    }
    return 0 ;
}


static inline void SetActor( BrkWork *work, void *act, void *die,
			     void *parent )
{
    GV_SetActor( &work->actor, act, die ) ;
    GV_ActorEX( &work->actor ) ;
    GV_SetActorChild( parent, work ) ;
    GV_SetActorSignalFunc( work, ReceiveSignal ) ;
}

int NewAnythingDust( HIDE *hide, TARGET *trgt, int n_dust, int id,
		     float gravity, float velocity, int life,
		     int *sound, int n_sound )
{
    BrkWork *work = (BrkWork *)GV_NewActor( GV_ACTOR_USER, sizeof(BrkWork) ) ;

    if ( work != NULL )
    {
	SetActor( work, Act, Die, hide->work ) ;
	if( GetResources( work, hide, trgt,
			  n_dust, 0, id,
			  gravity, velocity, 1.0f, life,
			  sound, n_sound ) < 0 )
	{
	    GV_DestroyActor( work ) ;
	    return 0 ;
	}
	return 1 ;
    }
    return 0 ;
}

int NewAnythingDustScale( HIDE *hide, TARGET *trgt, int n_dust, int id,
			  float gravity, float velocity, float scale,
			  int *sound, int n_sound )
{
    BrkWork *work = (BrkWork *)GV_NewActor( GV_ACTOR_USER, sizeof(BrkWork) ) ;

    if ( work != NULL )
    {
	SetActor( work, Act, Die, hide->work ) ;
	if( GetResources( work, hide, trgt,
			  n_dust, 0, id,
			  gravity, velocity, scale, -1,
			  sound, n_sound ) < 0 )
	{
	    GV_DestroyActor( work ) ;
	    return 0 ;
	}
	return 1 ;
    }
    return 0 ;
}

int NewTankDust( HIDE *hide, TARGET *trgt, int n_dust, int id,
		 float gravity, float velocity, int life,
		 int *sound, int n_sound )
{
    BrkWork *work = (BrkWork *)GV_NewActor( GV_ACTOR_USER, sizeof(BrkWork) ) ;

    if ( work != NULL )
    {
	SetActor( work, Act, Die, hide->work ) ;
	if( GetResources( work, hide, trgt,
			  n_dust, 0, id,
			  gravity, velocity, 1.0f, life,
			  sound, n_sound ) < 0 )
	{
	    GV_DestroyActor( work ) ;
	    return 0 ;
	}
	return 1 ;
    }
    return 0 ;
}

int NewAnythingDustScaleInit( HIDE *hide, FVECTOR *pos,
			      int n_dust, int id, float scale )
{
    BrkWork *work = (BrkWork *)GV_NewActor( GV_ACTOR_USER, sizeof(BrkWork) ) ;

    if ( work != NULL )
    {
	SetActor( work, NULL, Die, hide->work ) ;
	if( GetResources( work, hide, (TARGET *)pos,
			  n_dust, 1, id,
			  0.0f, 0.0f, 1.0f, -1,
			  NULL, 0 ) < 0 )
	{
	    GV_DestroyActor( work ) ;
	    return 0 ;
	}
	return 1 ;
    }
    return 0 ;
}

