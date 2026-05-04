/*
	g_soundx.c
	サウンド

	1999/12/02 Y.Korekado
	2002/02/22(Xbox発売日) M.Kobayashi

	g_sound.c のＡＰＩをそのままうつす
	ただし３Ｄの音減衰空間マップとかはおそらくＸｂｏｘで調整しなおしになるので、
	とりあえず移植しない

	Obstruction,
	Occlusion の実験を行った後、カーブの調整の予定 : March SDK ではDSOUNDにカーブ調整ＡＰＩがあるらしい
	
	$Id: g_soundx.c,v 1.7 2002/12/26 11:04:53 Yoshizawa1 Exp $
	
*/
#include	"gameheader.h"
#include	"libutl.h"
#include	"camera.h"

#if 1 //BP_TODO

void	GM_SdSet( int code )
{
   BP_BREAK;
}

void	GM_SeSet( int p, int l, int n )
{
   BP_BREAK;
}

void	GM_InitVolumParam( )
{
   BP_BREAK;
}

void	GM_SdUpdate( void )
{
   BP_BREAK;
}

HZX_ZONE_ADD	GM_CurrentAddr = GM_INVALID_ADDR;

#else

extern int PL_SubjectMove;


//	DSI3DL2BUFFER の一部 { lDirect, lDirectHF, lRoom, lRoomHF }
#define		DEF_DSI3DL2BUFFER			{ 0, 0, 0, 0 }

#define		DEF_OBSTRUCTION_HFLEVEL		(-1800)
#define		DEF_OBSTRUCTION_LFRATIO 	0.66f
#define		DEF_OCCLUSION_HFLEVEL		(-1800)
#define		DEF_OCCLUSION_LFRATIO 		0.66f
//#define		DEF_LISTENER_ENV			{ 0, 0, 0.f, 1.f, 0.5f, -10000, 0.02f, -10000, 0.04f, 100.f, 100.f, 5000.f }
//デフォルト値は sound モジュール側で設定するようにした

#include	"sd_ee.h"

//////////////////////// Xbox 追加
HZX_ZONE_ADD	GM_CurrentAddr = GM_INVALID_ADDR;
static HZX_ZONE_ADD		addr_effect_listener = GM_INVALID_ADDR;	// 障害遮蔽計算用リスナーのアドレス
static FVECTOR			vpos_effect_listener;					// 障害遮蔽計算用リスナーの位置
//@static DSI3DL2LISTENER	env_listener;					// 環境パラメータリスナー側
static DSI3DL2BUFFER	env_source;						// 環境パラメータ音源側
														// ただしObstruction, Occlusion はマテリアルを示す
// 距離を定数倍するマクロ
#define DISTANCE_WEIGHT_CONSTANT	0.5f
//#define GV_VecLen3F( pvec ) 	( DISTANCE_WEIGHT_CONSTANT * GV_VecLen3F( pvec ) )
														// 数種類用意する必要があるかも
//#define PRINT_DEBUG	(1)

/* 旧：PAN	20(左３２)～ 3F(左１) ～ 00(右１) ～ 1F(右３１) */
/* 新：PAN	0(左３２)～ 1F(左１) ～ 20(右１) ～ 3F(右３１) */
/* VOL	0 ～ 3F */

#define	NEW_VOLUME	(1)


typedef	struct	{
	int	pan ;	/* パン */
	FVECTOR	pos;	// 位置
	int	vol ;	/* ボリューム */
} SEPARAM ;

//#define CAMERA_POS	(1)	//常にカメラ中心に計算
#define PAN_NEW (1)

#ifdef DEBUG_MODE
int	GM_SE_TYPE = 0 ;
#else
int	GM_SE_TYPE = 0 ;
#endif

static int	sng_code=0 ;	// 現在なっている曲番号

/* -------------------------------------------------------*/
#define PLAYER_MIC_READY ((Ply_GetPlayerWeapon() == WP_Mic)||(Ply_GetPlayerWeapon() == WP_DemoMic))

static	int	VectorCheck( FVECTOR* pv )
{
	if( _finite( (double)( pv->vx + pv->vy + pv->vz ) )) return TRUE;
	return FALSE;
}


/* -------------------------------------------------------*/
static float dir_from_matrix_point( FVECTOR* sound_pos ) ;
/* -------------------------------------------------------*/
static	inline	float __RcosF( d )
int	d ;
{
	float f, t ;

	f = (float)M_PI * (float)d / 2048.0F ;
	t = cosf( f ) ;

	return t ;
}
/* -------------------------------------------------------*/
static	void	SNG_CHECK( code )
int			code ;
{
	if ( code >= SNG_PLAY_01 && code <= SNG_PLAY_08 ) {
		sng_code = code ;
	}
	if ( (code >= SNG_FOUTS_SS && code <= SNG_FOUTS_L) 
		|| (code == SNG_STOP) ) {
		sng_code = 0 ;
	}
}


int	GM_GetSngCode( void )
{
//printf("g_sound.c: playin song code [%x] \n",sng_code ) ;
	return sng_code ;
}
/* -------------------------------------------------------*/
static	inline	void	SD_SET( code )
int			code ;
{
	extern	int	sd_set_cli( int ) ;

	/* 擬音（SE）だけ鳴らさない　*/
	if ( ((code & 0xff000000) == 0) && GM_CheckGameStatus( STATE_DEMO ) ) return ; 

	sd_set_cli( code ) ;
}

static	inline	int	SE_MAKE( p, l, n )
int			p, l, n ;
{
	p &= 0x3f;
	l &= 0xff;
	if ( l > 0x3f ) l = 0x3f ;
	n &= 0x7ff;	/* 0～2048 */

	return (p << 18) | (l << 12) | n ;
}

static	inline	void	SE_SET( p, l, n )
int			p, l, n ;
{
	SD_SET( SE_MAKE( p, l, n ) ) ;
}


static	int	GetPan ( dir, r ) 
int	dir ;
int	r ;
{
	int	pan ;

	dir &= 4095 ;
	pan =(int)(__RcosF( dir ) * (float)r ) ;

	if ( pan > GM_MAX_PAN ) pan = GM_MAX_PAN ;
	if ( pan < -GM_MAX_PAN ) pan = -GM_MAX_PAN ;

#ifdef PAN_NEW
	pan += 0x1f ;
#else
	if( pan < 0 ){				/* 左 */
		pan = 0x3F + pan ;
	}
#endif
	
	return pan ;
}

static	int	GetPan2 (
		int	dir ,
		int	r ,
		float	k )
{
	int	pan ;
	float	tmp ;

	dir &= 4095 ;
	pan =(int)(__RcosF( dir ) * (float)r ) ;

	tmp = (float)pan * k ;
	pan = (int)tmp ;

	if ( pan > GM_MAX_PAN ) pan = GM_MAX_PAN ;
	if ( pan < -GM_MAX_PAN ) pan = -GM_MAX_PAN ;

#ifdef PAN_NEW
	pan += 0x1f ;
#else
	if( pan < 0 ){				/* 左 */
		pan = 0x3F + pan ;
	}
#endif
	
	return pan ;
}

static inline int MakeVolume( int dis, int max_d, int min_d, int n_vol )
{
	int vol ;

	vol = 0x0 ;
	if( dis < max_d ){
		vol = GM_MAX_VOL ;
	}else {
		/* おおよそ画面外で音が消えるように */
		vol = GM_MAX_VOL  - (( dis - max_d ) / n_vol );
	}
	if ( vol < 0 ) vol = 0;

	return vol ;
}

static inline int MakeVolumeEx( int dis, int max_d, int min_d, int n_vol, int max )
{
	int vol ;

	vol = 0x0 ;
	if( dis < max_d ){
		vol = max ;
	}else {
		/* おおよそ画面外で音が消えるように */
		vol = max  - (( dis - max_d ) / n_vol );
	}
	if ( vol < 0 ) vol = 0;

	return vol ;
}

static void	SeSetAddress( int se, FVECTOR *pvpos, HZX_ZONE_ADD addr, int vol );

/* -------------------------------------------------------*/
#define GM_SE_MAX_DIS_MIC_VEC	(200)			/* 最大音量でなる距離 */
#define GM_SE_MIN_DIS_MIC_VEC	(400)			/* 聞こえなくなる距離 */

#define SE_COS_10	(0.9848077530122f)
#define SE_COS_15	(0.9659258262891f)
#define SE_COS_20	(0.9396926207859f)
#define SE_COS_30	(0.8660254037844f)
#define SE_COS_45	(0.7071067811865f)
#define SE_COS_60	(0.5f)
#define SE_COS_75	(0.2588190451025f)
#define SE_COS_80	(0.1736481776669f)
#define SE_COS_85	(0.08715574274766f)
#define SE_COS_90	(0.0f)

#define MIC_IN_COS	SE_COS_10
#define MIC_OUT_COS	SE_COS_20

//static	int	mic_in_cos = MIC_IN_COS ;
//static	int	mic_out_cos = MIC_OUT_COS ;
#if 1
/* 基本カーブはこっちを使う */
typedef struct {
	float	in_cos ;		/* 有効角度 */
	short	in_near_dis ;	/* 内側計算最小距離(俯瞰時はプレイヤーの距離） */
	short	in_far_dis ;	/* 内側計算最大距離 or near_dis からの距離 */
	short	in_near_vol ;	/* 内側最小距離時音量 */
	short	in_center_vol ;	/* 内側中心距離時音量 */
	short	in_far_vol ;	/* 内側最大距離時音量 */

	float	out_cos ;		/* 減衰最大角 */
	short	out_near_dis ;	/* 外側計算最小距離 */
	short	out_far_dis ;	/* 外側計算最大距離 */
	short	out_near_vol ;	/* 外側最小距離時音量 */
	short	out_center_vol ;/* 外側中心距離時音量 */
	short	out_far_vol ;	/* 外側最大距離時音量 */

	float	pan ;			/* パン係数 */
} VOLUMEPARAM ;

typedef struct {
	VOLUMEPARAM		normal ;
	VOLUMEPARAM		ownview ;
	VOLUMEPARAM		demo ;		/* ターゲット固定 */
} VOLPARAMS ;


#define	VOL_CONST	1.1885f		// 10 ** (150 / 2000)
static void ConvertBombVolParam( VOLUMEPARAM* pprm )
{	// Xbox の場合Bomb のボリューム調整を PS2 と変える
	float new_far_vol, new_far_dis;

	// in_cos
	// far ボリュームの計算
	new_far_vol = pprm->in_far_vol * VOL_CONST ;
	// far 距離の計算
	new_far_dis = ( new_far_vol - pprm->in_near_vol ) * (float)( pprm->in_far_dis - pprm->in_near_dis )
			/ (float)( pprm->in_far_vol - pprm->in_near_vol ) + (float) pprm->in_near_dis;
	pprm->in_far_vol = (short) new_far_vol;
	pprm->in_far_dis = (short) new_far_dis;

	// out_cos
	// far ボリュームの計算
	new_far_vol = pprm->out_far_vol * VOL_CONST ;
	// far 距離の計算
	new_far_dis = ( new_far_vol - pprm->out_near_vol ) * (float)( pprm->out_far_dis - pprm->out_near_dis )
			/ (float)( pprm->out_far_vol - pprm->out_near_vol ) + (float) pprm->out_near_dis;
	pprm->out_far_vol = (short) new_far_vol;
	pprm->out_far_dis = (short) new_far_dis;

	printf("Bomb param in_far_vol %d, in_far_dis %d\n", pprm->in_far_vol, pprm->in_far_dis );
	printf("Bomb param out_far_vol %d, out_far_dis %d\n", pprm->out_far_vol, pprm->out_far_dis );
}

#ifdef NEW_VOLUME
static	VOLUMEPARAM normal_param = {
	SE_COS_75,
	6000, 12000, 0x3F, 0x18, 0x00,
	SE_COS_90,
	5000, 10000, 0x3f, 0x18, 0x00,
	1.0f
} ;
static	VOLUMEPARAM normal_ownview_param = {
	SE_COS_60,
	0, 10000, 0x3F, 0x3F, 0x00,
	SE_COS_90,
	0, 9800, 0x3f, 0x3F, 0x00,
	1.0f
} ;
static	VOLUMEPARAM normal_demo_param = {
	SE_COS_75,
	6000, 12000, 0x3F, 0x18, 0x00,
	SE_COS_90,
	5000, 10000, 0x3f, 0x18, 0x00,
	1.0f
} ;

static	VOLUMEPARAM bomb_param = {
	SE_COS_75,
	6000, 14000, 0x3F, 0x38, 0x1f,
	SE_COS_90,
	5000, 12000, 0x3F, 0x38, 0x1f,
	1.0f
} ;
static	VOLUMEPARAM bomb_ownview_param = {
	SE_COS_60,
	0, 10000, 0x3F, 0x3F, 0x1f,
	SE_COS_90,
	0, 9800, 0x3f, 0x3F, 0x1f,
	1.0f
} ;
static	VOLUMEPARAM bomb_demo_param = {
	SE_COS_75,
	6000, 14000, 0x3F, 0x38, 0x1f,
	SE_COS_90,
	5000, 12000, 0x3F, 0x38, 0x1f,
	1.0f
} ;

static	VOLUMEPARAM real_param = {
	SE_COS_75,
	6000, 12000, 0x3F, 0x18, 0x00,
	SE_COS_90,
	5000, 10000, 0x3f, 0x18, 0x00,
	1.0f
} ;
static	VOLUMEPARAM real_ownview_param = {
	SE_COS_60,
	0, 10000, 0x2F, 0x2F, 0x00,
	SE_COS_90,
	0, 9800, 0x2f, 0x2F, 0x00,
	1.0f
} ;
static	VOLUMEPARAM real_demo_param = {
	SE_COS_75,
	6000, 12000, 0x2F, 0xf, 0x00,
	SE_COS_90,
	5000, 10000, 0x2f, 0xf, 0x00,
	1.0f
} ;


static	VOLPARAMS	NormalVolParam ;
static	VOLPARAMS	BombVolParam ;
static	VOLPARAMS	RealVolParam ;

/*音のモード*/
enum {
	VOLPARAM_SEMODE_NORMAL,	/* 俯瞰時はカメラターゲット、主観時はカメラ位置を中心に計算 */
	VOLPARAM_GM_SEMODE_BOMB,		/* 遠くても必ず少しは聞こえる */
	VOLPARAM_GM_SEMODE_REAL,		/* 主観やビハインドになったときだけ聞こえる */
} ;
/*画面の種類*/
enum {
	NORMAL_VIEW,	/*通常画面*/
	OWN_VIEW,		/*主観モード画面*/
	DEMO_VIEW,		/*デモ画面*/
} ;

int	GM_SetVolumeParam( void )
{
	int 	value, val2 ;
	VOLPARAMS	*vlp ;
	VOLUMEPARAM	*volparam ;
	int bConv = FALSE;

	if ( GCL_NextStr() == NULL ) return -1 ;
	value = GCL_GetNextInt() ;

	vlp = NULL ;
	switch( value ) {
		case GM_SEMODE_NORMAL :
printf("Set Volume Param Normal ");
			vlp = &NormalVolParam ;
			break ;
		case GM_SEMODE_BOMB :
printf("Set Volume Param Bomb ");
			vlp = &BombVolParam ;
			bConv = TRUE;
			break ;
		case GM_SEMODE_REAL :
printf("Set Volume Param Real ");
			vlp = &RealVolParam ;
			break ;
		default :
printf("Set Volume Param Other ");
			vlp = &NormalVolParam ;
			break ;
	}

	if ( GCL_NextStr() == NULL ) return -1 ;
	val2 = GCL_GetNextInt() ;

	volparam = NULL ;
	switch( val2 ) {
		case NORMAL_VIEW :
printf("Normal");
			volparam = &vlp->normal ;
			break ;
		case OWN_VIEW :
printf("Own");
			volparam = &vlp->ownview ;
			break ;
		case DEMO_VIEW :
printf("Demo");
			volparam = &vlp->demo ;
			break ;
		default :
printf("Other");
			volparam = &vlp->normal ;
			break ;
	}
printf("\n");

	/* 有効角度 */
	if ( GCL_NextStr() == NULL ) return -1 ;
	value = GCL_GetNextInt() ;
	volparam->in_cos = cosf( (float)value ) ;
//	volparam->in_cos =__RcosF( value ) ;

	/* 内側計算最小距離 */
	if ( GCL_NextStr() == NULL ) return -1 ;
	volparam->in_near_dis = GCL_GetNextInt() ;

	/* 内側計算最大距離 */
	if ( GCL_NextStr() == NULL ) return -1 ;
	volparam->in_far_dis = GCL_GetNextInt() ;

	/* 内側最小距離時音量 */
	if ( GCL_NextStr() == NULL ) return -1 ;
	volparam->in_near_vol = GCL_GetNextInt() ;

	/* 内側中心距離時音量 */
	if ( GCL_NextStr() == NULL ) return -1 ;
	volparam->in_center_vol = GCL_GetNextInt() ;

	/* 内側最大距離時音量 */
	if ( GCL_NextStr() == NULL ) return -1 ;
	volparam->in_far_vol = GCL_GetNextInt() ;

	/* 減衰最大角 */
	if ( GCL_NextStr() == NULL ) return -1 ;
	value = GCL_GetNextInt() ;
	volparam->in_cos = cosf( (float)value ) ;
//	volparam->out_cos =__RcosF( value ) ;

	/* 外側計算最小距離 */
	if ( GCL_NextStr() == NULL ) return -1 ;
	volparam->out_near_dis = GCL_GetNextInt() ;

	/* 外側計算最大距離 */
	if ( GCL_NextStr() == NULL ) return -1 ;
	volparam->out_far_dis = GCL_GetNextInt() ;

	/* 外側最小距離時音量 */
	if ( GCL_NextStr() == NULL ) return -1 ;
	volparam->out_near_vol = GCL_GetNextInt() ;

	/* 外側中心距離時音量 */
	if ( GCL_NextStr() == NULL ) return -1 ;
	volparam->out_center_vol = GCL_GetNextInt() ;

	/* 外側最大距離時音量 */
	if ( GCL_NextStr() == NULL ) return -1 ;
	volparam->out_far_vol = GCL_GetNextInt() ;

	/* パン係数 */
	if ( GCL_NextStr() == NULL ) return -1 ;
	value = GCL_GetNextInt() ;
	volparam->pan = (float)value / 100 ;

	if( bConv ) {
		ConvertBombVolParam( volparam );
	}

	return 0 ;
}

/* -------------------------------------------------------*/
void	GM_SetVolCurveInDisFromPos( FVECTOR *pos, int *from_indis, int *to_indis, int dev )
{
	GM_CameraSet	*cam ;
	int	dis, i ;
	FVECTOR vec ;

	cam = GM_GetCurrentCamera( 0 ) ;	/* 子画面は１*/

	/* 中心位置とカメラの距離 */
    _sceVu0SubVector( &vec, &cam->position, pos ) ;
	dis = (int)GV_VecLen3F( &vec ) ;

	to_indis[0] = ( dis < from_indis[0] ) ? from_indis[0] : dis ;

	for( i=1; i<dev; i++ ) {
		to_indis[i] = to_indis[0] + (from_indis[i] - from_indis[0]) ;
	}
}

static	int	NewSetSeParamFromVolCurve( pos, volcurve, separam )
FVECTOR	*pos ;		/* 発生位置 */
VOLUMECURVE	*volcurve ;
SEPARAM	*separam ;
{

	FVECTOR		vec ;
	int			dir, dis, vol, r, pan, n_vol, i ;
	float 		k, adj ;
	int			far_dis, near_dis,far_vol, near_vol ;
	int			tmp_dis[MAX_VOL_CURVE_DEV] ;
	GM_CameraSet	*cam ;

	cam = GM_GetCurrentCamera( 0 ) ;	/* 子画面は１*/

	/* カメラからの距離 */
    _sceVu0SubVector( &vec, pos, &cam->position ) ;
    dis = (int)GV_VecLen3F( &vec ) ;

	far_dis = near_dis = far_vol = near_vol = 0 ;

	/* 角度 0～MIC_IN_COS ～ MIC_OUT_COS */
	{
		FVECTOR *p, *v1, *v2 ;
	    FVECTOR	d1, d2 ;
	    float 	f ;

		p = &cam->position ;
		v1 = &cam->target ;
		v2 = pos ;

//printf("cam:[%.1f] [%.1f] [%.1f]\n",p->vx,p->vy,p->vz ) ;
//printf("trg:[%.1f] [%.1f] [%.1f]\n",v1->vx,v1->vy,v1->vz ) ;
//printf("pos:[%.1f] [%.1f] [%.1f]\n",v2->vx,v2->vy,v2->vz ) ;

	    _sceVu0SubVector( &d1, p, v1 ) ;
	    _sceVu0SubVector( &d2, p, v2 ) ;
	    _sceVu0Normalize( &d1, &d1 ) ;
	    _sceVu0Normalize( &d2, &d2 ) ;
	    f = _sceVu0InnerProduct( &d1, &d2 ) ;

		if ( f > volcurve->in_cos ) {
			for( i=0; i<volcurve->dev; i++ ) {
				if ( (dis < volcurve->in_dis[i]) || (i==(volcurve->dev-1)) ) {
					near_dis = (i==0)? 0 :volcurve->in_dis[i-1] ;
					far_dis = volcurve->in_dis[i] ;
					near_vol = volcurve->in_vol[i] ;
					far_vol = volcurve->in_vol[i+1] ;
					break ;
				}
			}
		} else if ( f > volcurve->out_cos ) {
			adj = volcurve->in_cos - volcurve->out_cos ;
			ASSERT( adj > 0.0f ) ;

			if ( adj < 0.0000001 ) {
				k = 0.0f ;
			} else {
				k = ( volcurve->in_cos - f ) / adj ;
			}

			for( i=0; i<volcurve->dev; i++ ) {
				adj = ((float)(volcurve->in_dis[i]) - (float)(volcurve->out_dis[i])) * k ;
				tmp_dis[i] = volcurve->in_dis[i] - (int)adj ;

				if ( (dis < tmp_dis[i])  || (i==(volcurve->dev-1)) ) {
					near_dis = (i==0)? 0 :tmp_dis[i-1] ;
					far_dis = tmp_dis[i] ;
					adj = ((float)volcurve->in_vol[i] - (float)volcurve->out_vol[i]) * k ;
					near_vol = volcurve->in_vol[i] - (int)adj ;
					adj = ((float)volcurve->in_vol[i+1] - (float)volcurve->out_vol[i+1]) * k ;
					far_vol = volcurve->in_vol[i+1] - (int)adj ;
					break ;
				}
			}
		} else {
			for( i=0; i<volcurve->dev; i++ ) {
				if ( (dis < volcurve->out_dis[i]) || (i==(volcurve->dev-1))  ) {
					near_dis = (i==0)? 0 :volcurve->out_dis[i-1] ;
					far_dis = volcurve->out_dis[i] ;
					near_vol = volcurve->out_vol[i] ;
					far_vol = volcurve->out_vol[i+1] ;
					break ;
				}
			}
		}

		if ( dis > far_dis ) dis = far_dis ;
		if ( (near_vol - far_vol) == 0 ) {
			vol = near_vol ;
		} else {
			n_vol = (int)( (far_dis - near_dis) / (near_vol - far_vol) ) ;
			if ( n_vol == 0 ) n_vol = 1 ;
			vol = MakeVolumeEx( dis, near_dis, far_dis, n_vol, near_vol ) ;
		}
//printf("f[%.2f] dis[%d] near[%d] far[%d] near_vol[%d] f_vol[%d] vol[%d]\n",
//		f, dis, near_dis,far_dis,near_vol, far_vol ,vol ) ;
	}

	/* ＰＡＮ計算 */
	//距離によってパン効果が薄くなるのを防止する実験
	{
		int dd ;

		dd = ( dis > 3000 ) ? 3000 : dis ;
		r = GM_MAX_PAN * dd / 3000 ;
	}
	{
	    dir = (int)( 0x20 * dir_from_matrix_point(pos)) ;
	    pan = 0x20 + dir;
	    if (pan >= 0x40){
			pan = 0x3f;
	    } else if ( pan < 0 ) {
			pan = 0x00;
		}
	    //	    printf ("Pan %d\n", dir);
	}

	separam->vol = vol ;
	separam->pan = pan ;
	separam->pos = *pos;

	if ( vol == 0 ) return -1 ;

	return 0 ;
}





/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void getdirect_from_matrix_point				*/
/*	引数:	FMATRIX* camera_mat						*/
/*		FVECTOR* sound_pos						*/
/*	返値:	4096角度							*/
/*	説明:	カメラのmatrixと音の発生源から角度を求める			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static float dir_from_matrix_point(FVECTOR* sound_pos){
    FVECTOR		tmpvec;
    FVECTOR		tmppos;
    float		pany;
    float		panx;
    GM_CameraSet	*cam ;
    float		num;	

    cam = GM_GetCurrentCamera( 0 );
    
    //主観とフカンの間には補間を入れる Satoyoshi 2001/09/17
    if ( (GM_PlayerSubjectCamera[0] != NULL) && (GM_GetNextCamera(0) != NULL) )
    {
		float dist0, dist1;
		FVECTOR	shift;
		GM_CameraSet	*cam_first = GM_PlayerSubjectCamera[0];
		GM_CameraSet	*cam_next = GM_GetNextCamera(0);
		GM_CameraSet	*cam_now = GM_GetCurrentCamera(0);
	
		_sceVu0SubVector( &shift, &(cam_first->position),
						  &(cam_next->position));
		dist0 =  GV_VecLen3F( &shift ) ;

		_sceVu0SubVector( &shift, &(cam_first->position),
						  &(cam_now->position));
		dist1 =  GV_VecLen3F( &shift ) ;

		if ( (cam_first->position.vx == cam_next->position.vx ) &&
			 (cam_first->position.vy == cam_next->position.vy ) &&
			 (cam_first->position.vy == cam_next->position.vy ) ){

			num = 1.0f-(100.0f)/(dist1+100.0f);
		}
		else {
			num = (dist1+0.1f)/(dist0+0.1f);
		}

		if (num > 1.0f){
			num = 1.0f;
		}
		//printf("%f\n", num);

		tmppos.vx = ( cam->position.vx + (cam->target.vx*num) )/(1.0f+num);
		tmppos.vy = ( cam->position.vy + (cam->target.vy*num) )/(1.0f+num);
		tmppos.vz = ( cam->position.vz + (cam->target.vz*num) )/(1.0f+num);
		tmppos.vw = 1.0f;

    }
    else {
		tmppos.vx = cam->position.vx;
		tmppos.vy = cam->position.vy;
		tmppos.vz = cam->position.vz;
		tmppos.vw = 1.0f;
		num = 1.0f;
    }

    //    sat_viewp(&tmppos, 0, 0, 250);

    _sceVu0SubVector(&tmpvec, sound_pos, &tmppos);
    _sceVu0Normalize(&tmpvec, &tmpvec);
    pany = _sceVu0InnerProduct(&tmpvec, DG_Chanl(0)->eye.m[0]);
    panx = _sceVu0InnerProduct(&tmpvec, DG_Chanl(0)->eye.m[2]);

    //    sat_viewp(sound_pos, 250, 0, 0);

    //    printf("%f %f:%.0f %.0f %.0f\n",pany, panx,
    //	   sound_pos->vx, sound_pos->vy, sound_pos->vz);


    {
		float	pan_center;

		if (pany > 0.0f){
			pan_center = (pany+(pany*pany*2.0f))/3.0f;
		}
		else {
			pan_center = (pany-(pany*pany*2.0f))/3.0f;
		}

		pany = (num*pany) + ((1.0f-num)*pan_center);
    }
    
    
    /*{
	float pan = 32+(32*pany);
	printf("P:%d  %f  %f\n",(int)pan, pany, num);
    }*/
    return pany;
}




static	int	NewSetSeParam( pos, volparam, separam )
FVECTOR	*pos ;		/* 発生位置 */
VOLUMEPARAM	*volparam ;
SEPARAM	*separam ;
{

	FVECTOR		vec ;
	int			dir, dis, vol, r, pan, pl_dis, n_vol ;
	float 		k, adj ;
	int			far_dis, near_dis,far_vol, near_vol, a, b ;
	GM_CameraSet	*cam ;

	cam = GM_GetCurrentCamera( 0 ) ;	/* 子画面は１*/

	if ( GM_GameStatus & STATE_PLAY_DEMO ) {	/* デモ */
		pl_dis = volparam->in_near_dis ;
	} else {
		/* プレイヤーとカメラの距離 仮 */
	    _sceVu0SubVector( &vec, &cam->position, &GM_PlayerPosition ) ;
		pl_dis = (int)GV_VecLen3F( &vec ) ;
	}

	/* ＶＯＬ計算 */
    _sceVu0SubVector( &vec, pos, &cam->position ) ;
    dis = (int)GV_VecLen3F( &vec ) ;

	/* 角度 0～MIC_IN_COS ～ MIC_OUT_COS */
	{
		FVECTOR *p, *v1, *v2 ;
	    FVECTOR	d1, d2 ;
	    float 	f ;

		p = &cam->position ;
		v1 = &cam->target ;
		v2 = pos ;

//printf("cam:[%.1f] [%.1f] [%.1f]\n",p->vx,p->vy,p->vz ) ;
//printf("trg:[%.1f] [%.1f] [%.1f]\n",v1->vx,v1->vy,v1->vz ) ;
//printf("pos:[%.1f] [%.1f] [%.1f]\n",v2->vx,v2->vy,v2->vz ) ;

	    _sceVu0SubVector( &d1, p, v1 ) ;
	    _sceVu0SubVector( &d2, p, v2 ) ;
	    _sceVu0Normalize( &d1, &d1 ) ;
	    _sceVu0Normalize( &d2, &d2 ) ;
	    f = _sceVu0InnerProduct( &d1, &d2 ) ;

		if ( f > volparam->in_cos ) {
//			if ( pl_dis < volparam->out_near_dis ) pl_dis = volparam->in_near_dis ;
			if ( pl_dis < volparam->in_near_dis ) pl_dis = volparam->in_near_dis ;
			if ( dis < pl_dis ) {	/* プレイヤー（ターゲット位置）よりカメラよりなら */
				near_dis = 0 ;
				far_dis = pl_dis ;
				near_vol = volparam->in_near_vol ;
				far_vol = volparam->in_center_vol ;
			} else {
				near_dis = pl_dis ;
				far_dis = pl_dis + (volparam->in_far_dis-volparam->in_near_dis) ;	/* 俯瞰の */
				near_vol = volparam->in_center_vol ;
				far_vol = volparam->in_far_vol ;
			}
		} else if ( f > volparam->out_cos ) {
			adj = volparam->in_cos - volparam->out_cos ;
			ASSERT( adj > 0.0f ) ;

			if ( adj < 0.0000001 ) {
				k = 0.0f ;
			} else {
				k = ( volparam->in_cos - f ) / adj ;
			}

			adj = ((float)(pl_dis) - (float)(volparam->out_near_dis)) * k ;
			pl_dis = pl_dis - (int)adj ;

			adj = ((float)(volparam->in_near_dis) - (float)(volparam->out_near_dis)) * k ;
			a = volparam->in_near_dis - (int)adj ;

			if ( pl_dis < a ) pl_dis = a ;

			if ( dis < pl_dis ) {	/* プレイヤー（ターゲット位置）よりカメラよりなら */
				near_dis = 0 ;
				far_dis = pl_dis ;
				adj = ((float)volparam->in_near_vol - (float)volparam->out_near_vol) * k ;
				near_vol = volparam->in_near_vol - (int)adj ;
				adj = ((float)volparam->in_center_vol - (float)volparam->out_center_vol) * k ;
				far_vol = volparam->in_center_vol - (int)adj ;
			} else {
				near_dis = pl_dis ;

				a = (volparam->in_far_dis - volparam->in_near_dis) ;
				b = (volparam->out_far_dis - volparam->out_near_dis) ;
				adj = ((float)a - (float)b) * k ;
				far_dis = pl_dis + a - (int)adj ;

				adj = ((float)volparam->in_center_vol - (float)volparam->out_center_vol) * k ;
				near_vol = volparam->in_center_vol - (int)adj ;
				adj = ((float)volparam->in_far_vol - (float)volparam->out_far_vol) * k ;
				far_vol = volparam->in_far_vol - (int)adj ;
			}
		} else {
			if ( dis < volparam->out_near_dis ) {
				near_dis = 0 ;
				far_dis = volparam->out_near_dis ;
				near_vol = volparam->out_near_vol ;
				far_vol = volparam->out_center_vol ;
			} else {
				near_dis = volparam->out_near_dis ;
				far_dis = volparam->out_far_dis ;
				near_vol = volparam->out_center_vol ;
				far_vol = volparam->out_far_vol ;
			}
		}

		if ( dis > far_dis ) dis = far_dis ;

		if ( (near_vol - far_vol) == 0 ) {
			vol = near_vol ;
		} else {
			n_vol = (int)( (far_dis - near_dis) / (near_vol - far_vol) ) ;
			if ( n_vol == 0 ) n_vol = 1 ;
			vol = MakeVolumeEx( dis, near_dis, far_dis, n_vol, near_vol ) ;
		}

//printf("f[%.2f] pl[%d] dis[%d] near[%d] far[%d] near_vol[%d] f_vol[%d] vol[%d]\n",
//		f, pl_dis, dis, near_dis,far_dis,near_vol, far_vol ,vol ) ;
	}

	/* ＰＡＮ計算 */
	/* ＰＡＮ計算 */
#if 1	//距離によってパン効果が薄くなるのを防止する実験
	{
		int dd ;

		dd = ( dis > 3000 ) ? 3000 : dis ;
		r = GM_MAX_PAN * dd / 3000 ;
	}


#if 0	// Satoyoshi 
	dir = GV_VecDir2( &vec ) ;
	dir = dir - (cam->rotate.vy - 1024) ;
	pan = GetPan ( dir, r ) ;
	dir_from_matrix_point(pos);
#else
	{
	    dir = (int)(0x20 * dir_from_matrix_point(pos));
	    pan = 0x20 + dir;
	    if (pan >= 0x40){
			pan = 0x3f;
	    } else if ( pan < 0 ) {
			pan = 0x00;
	    }
	    //	    printf ("Pan %d\n", dir);
	}
#endif




#else
	dir = GV_VecDir2( &vec ) ;
	dir = dir - (cam->rotate.vy - 1024) ;
	r = GM_MAX_PAN * dis / GM_SE_MIN_DIS_CAMTRG ;
	pan = GetPan ( dir, r ) ;
#endif
	
	separam->vol = vol ;
	separam->pan = pan;
	separam->pos = *pos;

	if ( vol == 0 ) return -1 ;

	return 0 ;
}


/* -------------------------------------------------------*/
#endif

static	VOLUMEPARAM normal_micparam = {
	SE_COS_10,
	4000, 12000, 0x3F, 0x00, 0x00,
	SE_COS_20,
	0, 2000, 0x08, 0x00, 0x00,
	1.0f
} ;
static	VOLUMEPARAM mic_micparam = {
	SE_COS_10,
	4000, 12000, 0x2F, 0x00, 0x00,
	SE_COS_20,
	0, 2000, 0x02, 0x00, 0x00,
	1.0f
} ;
static	VOLUMEPARAM bomb_micparam = {
	SE_COS_10,
	4000, 12000, 0x3F, 0x00, 0x1f,
	SE_COS_60,
	0, 2000, 0x1F, 0x00, 0x08,
	1.0f
} ;

/* -------------------------------------------------------*/

int	GM_SetMicParam( void )
{
	int 	value ;
	VOLUMEPARAM	*micparam ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	value = GCL_GetNextInt() ;

	micparam = NULL ;
	switch( value ) {
		case GM_SEMODE_MIC :
			micparam = &mic_micparam ;
			break ;
		case GM_SEMODE_BOMB :
			micparam = &bomb_micparam ;
			break ;
		default :
			micparam = &normal_micparam ;
			break ;
	}
	/* 有効角度 */
	if ( GCL_NextStr() == NULL ) return -1 ;
	value = GCL_GetNextInt() ;
	micparam->in_cos =__RcosF( value ) ;

	/* 内側計算最小距離 */
	if ( GCL_NextStr() == NULL ) return -1 ;
	micparam->in_near_dis = GCL_GetNextInt() ;

	/* 内側計算最大距離 */
	if ( GCL_NextStr() == NULL ) return -1 ;
	micparam->in_far_dis = GCL_GetNextInt() ;

	/* 内側最小距離時音量 */
	if ( GCL_NextStr() == NULL ) return -1 ;
	micparam->in_near_vol = GCL_GetNextInt() ;

	/* 内側最大距離時音量 */
	if ( GCL_NextStr() == NULL ) return -1 ;
	micparam->in_far_vol = GCL_GetNextInt() ;

	/* 減衰最大角 */
	if ( GCL_NextStr() == NULL ) return -1 ;
	value = GCL_GetNextInt() ;
	micparam->out_cos =__RcosF( value ) ;

	/* 外側計算最小距離 */
	if ( GCL_NextStr() == NULL ) return -1 ;
	micparam->out_near_dis = GCL_GetNextInt() ;

	/* 外側計算最大距離 */
	if ( GCL_NextStr() == NULL ) return -1 ;
	micparam->out_far_dis = GCL_GetNextInt() ;

	/* 外側最小距離時音量 */
	if ( GCL_NextStr() == NULL ) return -1 ;
	micparam->out_near_vol = GCL_GetNextInt() ;

	/* 外側最大距離時音量 */
	if ( GCL_NextStr() == NULL ) return -1 ;
	micparam->out_far_vol = GCL_GetNextInt() ;

	/* パン係数 */
	if ( GCL_NextStr() == NULL ) return -1 ;
	value = GCL_GetNextInt() ;
	micparam->pan = (float)value / 100 ;

	return 0 ;
}




static	int	SetSParam_Mic( pos, len, separam, mode )
FVECTOR	*pos ;
int		len ;
SEPARAM	*separam ;
int		mode ;
{
	FVECTOR		vec ;
	int			dir, dis, vol, r, pan, n_vol ;
	float 		k, adj, f ;
	int			far_dis, near_dis,far_vol, near_vol ;
	GM_CameraSet	*cam ;
	VOLUMEPARAM	*micparam ;

	switch( mode ) {
		case GM_SEMODE_MIC :
			micparam = &mic_micparam ;
			break ;
		case GM_SEMODE_BOMB :
			micparam = &bomb_micparam ;
			break ;
		default :
			micparam = &normal_micparam ;
			break ;
	}

	cam = GM_GetCurrentCamera( 0 ) ;	/* 子画面は１*/

	/* ＶＯＬ計算 */
    _sceVu0SubVector( &vec, pos, &cam->position ) ;
    dis = (int)GV_VecLen3F( &vec ) ;
	dis -= len ;
	if ( dis < 0 ) dis = 0 ;

	/* 鋭指向性マイク 角度 0～MIC_IN_COS ～ MIC_OUT_COS */
	{
		FVECTOR *p, *v1, *v2 ;
	    FVECTOR	d1, d2 ;

		p = &cam->position ;
		v1 = &cam->target ;
		v2 = pos ;

	    _sceVu0SubVector( &d1, p, v1 ) ;
	    _sceVu0SubVector( &d2, p, v2 ) ;
	    _sceVu0Normalize( &d1, &d1 ) ;
	    _sceVu0Normalize( &d2, &d2 ) ;
	    f = _sceVu0InnerProduct( &d1, &d2) ;

		if ( f > micparam->in_cos ) {
			far_dis = micparam->in_far_dis ;
			near_dis = micparam->in_near_dis ;
			far_vol = micparam->in_far_vol ;
			near_vol = micparam->in_near_vol ;
		} else if ( f > micparam->out_cos ) {
			adj = micparam->in_cos - micparam->out_cos ;
			ASSERT( adj > 0.0f ) ;

			if ( adj < 0.0000001 ) {
				k = 0.0f ;
			} else {
				k = ( micparam->in_cos - f ) / adj ;
			}

			adj = (((float)micparam->in_far_dis - (float)micparam->out_far_dis)*k) ;
			far_dis = micparam->in_far_dis - (int)adj ;

			adj = (((float)micparam->in_near_dis - (float)micparam->out_near_dis)*k) ;
			near_dis = micparam->in_near_dis - (int)adj ;

			adj = ((float)micparam->in_far_vol - (float)micparam->out_far_vol) * k ;
			far_vol = micparam->in_far_vol - (int)adj ;

			adj = ((float)micparam->in_near_vol - (float)micparam->out_near_vol) * k ;
			near_vol = micparam->in_near_vol - (int)adj ;
		} else {
			far_dis = micparam->out_far_dis ;
			near_dis = micparam->out_near_dis ;
			far_vol = micparam->out_far_vol ;
			near_vol = micparam->out_near_vol ;
		}

		if ( dis > far_dis ) dis = far_dis ;
		n_vol = (int)( (far_dis - near_dis) / (near_vol - far_vol) ) ;

		vol = MakeVolumeEx( dis, near_dis, far_dis, n_vol, near_vol ) ;

//	printf("f[%.2f] dis[%d], near_dis[%d] far_dis[%d] near_vol[%d] far_vol[%d]\n",f, dis, near_dis,far_dis,near_vol, far_vol ) ;
	}

	/* パン計算 */
	dir = GV_VecDir2( &vec ) ;

	/* 主観でのステレオ効果をはっきりさせるため、距離を長くして計算する。*/
	dis *= 4 ;
	if ( dis > far_dis ) dis = far_dis ;

//printf("dir[%d] - camera[%d] = cma->dir[%d] ",dir, cam->rotate.vy, dir - (cam->rotate.vy ) ) ;
	dir = dir - (cam->rotate.vy - 1024) ;
	r = GM_MAX_PAN * dis / far_dis ;
	pan = GetPan2 ( dir, r, micparam->pan ) ;

	separam->vol = vol ;
	separam->pan = pan ;
	separam->pos = *pos ;

//printf("dir[%d] pan[0x%x] vol[%d] \n\n",dir, pan , vol ) ;

	if ( vol <= 0 )  return -1 ;

	return 0 ;
}

#endif

static	int	SetSParam_CamTrg2( pos, trg, len, separam, max_dis, min_dis, max_vol)
FVECTOR	*pos ;		/* 発生位置 */
FVECTOR	*trg ;		/* 到達位置 */
int		len ;		/* ＭＡＸ範囲 */
SEPARAM	*separam ;
int			max_dis ;
int			min_dis ;
int			max_vol ;
{
	FVECTOR	vec ;
	int	dir, dis, vol, r, pan ;
	int	vol_unit ;
	GM_CameraSet	*cam ;

	cam = GM_GetCurrentCamera( 0 ) ;	/* 子画面は１*/

	vol_unit = ( (min_dis - max_dis) / max_vol ) ;
	
	/* ＶＯＬ計算 */
//    _sceVu0SubVector( &vec, pos, &cam->target ) ;
    _sceVu0SubVector( &vec, pos, trg ) ;
    dis = (int)GV_VecLen3F( &vec ) ;
	dis -= len ;	/* 最大音量がなる範囲分は差し引く */
	if ( dis < 0 ) dis = 0 ;
	if ( dis > min_dis ) dis = min_dis ;

	vol = MakeVolumeEx( dis, max_dis, min_dis, vol_unit ,max_vol ) ;

	/* ＰＡＮ計算 */
	dir = GV_VecDir2( &vec ) ;
	dir = dir - (cam->rotate.vy - 1024) ;
	r = GM_MAX_PAN * dis / min_dis ;
//printf("r=%d dis=%d dir=%d min_dis=%d\n",r,dis,dir,min_dis) ;
	pan = GetPan ( dir, r ) ;
//printf("pan=%d\n",pan) ;

	separam->vol = vol ;
	separam->pan = pan ;
	separam->pos = *pos;

	if ( vol == 0 ) return -1 ;

	return 0 ;
}

/* -------------------------------------------------------*/
static	int	SeSetFromVolCurve( FVECTOR *pos, SEPARAM *sp, VOLCURVES *volcurves ) {
	/* 集音マイク */
	if ( PLAYER_MIC_READY ) {
		return NewSetSeParamFromVolCurve( pos, volcurves->mic, sp ) ;
	}
	/* デモ */
	if ( GM_GameStatus & STATE_PLAY_DEMO ) {
		return NewSetSeParamFromVolCurve( pos, volcurves->demo, sp ) ;
	}
	/* 主観 */
	if ( GM_CheckPlayerStatus(PLAYER_BEHIND|PLAYER_INTRUDE|PLAYER_WATCH)
		 || PL_SubjectMove != 0 ) {
		return NewSetSeParamFromVolCurve( pos, volcurves->ownview, sp ) ;
	}

	return NewSetSeParamFromVolCurve( pos, volcurves->normal, sp ) ;
}

static int MakeSeNormalEx( pos, sp, max_dis, min_dis )
FVECTOR		*pos ;
SEPARAM		*sp ;
int			max_dis ;
int			min_dis ;
{
	GM_CameraSet	*cam ;

	cam = GM_GetCurrentCamera( 0 ) ;	/* 子画面は１*/

	if ( GM_CheckPlayerStatus(PLAYER_BEHIND|PLAYER_INTRUDE|PLAYER_WATCH)
		 || PL_SubjectMove != 0 ) {
		return SetSParam_CamTrg2( pos, &cam->position, GM_SE_MAX_DIS_CAMTRG, sp, 0, min_dis*6/5,  GM_MAX_VOL  ) ;
	}

	return SetSParam_CamTrg2( pos, &cam->target, GM_SE_MAX_DIS_CAMTRG, sp, max_dis, min_dis,  GM_MAX_VOL  ) ;
}

static int MakeSeNormal( pos, sp )
FVECTOR		*pos ;
SEPARAM		*sp ;
{
	/* 分岐チェック順序大事！！ */

	/* 集音マイク */
	if ( PLAYER_MIC_READY ) {
		return SetSParam_Mic( pos, 0, sp, GM_SEMODE_NORMAL ) ;
	}
#ifdef NEW_VOLUME
	/* デモ */
	if ( GM_GameStatus & STATE_PLAY_DEMO ) {
		return NewSetSeParam( pos, &NormalVolParam.demo, sp ) ;
	}
	/* 主観 */
	if ( GM_CheckPlayerStatus(PLAYER_BEHIND|PLAYER_INTRUDE|PLAYER_WATCH)
		 || PL_SubjectMove != 0 ) {
		return NewSetSeParam( pos, &NormalVolParam.ownview, sp ) ;
	}

	return NewSetSeParam( pos, &NormalVolParam.normal, sp ) ;
#else
	error;
#endif
}

static int MakeSeBomb( pos, sp )
FVECTOR		*pos ;
SEPARAM		*sp ;
{
	/* 分岐チェック順序大事！！ */

	/* 集音マイク */
	if ( PLAYER_MIC_READY ) {
		return SetSParam_Mic( pos, 0, sp, GM_SEMODE_BOMB ) ;
	}
#ifdef NEW_VOLUME
	/* デモ */
	if ( GM_GameStatus & STATE_PLAY_DEMO ) {
		return NewSetSeParam( pos, &BombVolParam.demo, sp ) ;
	}
	/* 主観 */
	if ( GM_CheckPlayerStatus(PLAYER_BEHIND|PLAYER_INTRUDE|PLAYER_WATCH)
		 || PL_SubjectMove != 0 ) {
		return NewSetSeParam( pos, &BombVolParam.ownview, sp ) ;
	}

	return NewSetSeParam( pos, &BombVolParam.normal, sp ) ;
#else
	error;
#endif
}

static int MakeSeReal( pos, sp )
FVECTOR		*pos ;
SEPARAM		*sp ;
{
	if ( PLAYER_MIC_READY ) {
		return SetSParam_Mic( pos, 0, sp, GM_SEMODE_REAL ) ;
	}
#ifdef NEW_VOLUME
	/* 分岐チェック順序大事！！ */

	/* 主観 */
	if ( GM_CheckPlayerStatus(PLAYER_BEHIND|PLAYER_INTRUDE|PLAYER_WATCH) 
		 || PL_SubjectMove != 0 ) {
		return NewSetSeParam( pos, &RealVolParam.ownview, sp ) ;
	}
#else
	error;
#endif

	return -1 ;
}

static int MakeSeMic( pos, sp )
FVECTOR		*pos ;
SEPARAM		*sp ;
{
	if ( PLAYER_MIC_READY && !(GM_CheckGameStatus( STATE_BIG_SNORE )) ) {
		return SetSParam_Mic( pos, 0, sp, GM_SEMODE_MIC ) ;
	}
	return -1 ;
}

static	int		GetSp( pos, mode, sp )
FVECTOR	*pos ;
int		mode ;
SEPARAM	*sp ;
{
	sp->vol = GM_MAX_VOL ;
	sp->pan = 0 ;
	if ( pos != NULL ) {
		switch ( mode ) {
			case GM_SEMODE_NORMAL :
				/* 俯瞰時はカメラターゲット、主観時はカメラ位置を中心に計算 */
				if ( MakeSeNormal( pos, sp ) < 0 ) return -1 ;
				break ;
			case GM_SEMODE_BOMB :
				/* 遠くても必ず少しは聞こえる */
				if ( MakeSeBomb( pos, sp ) < 0 ) return -1 ;
				break ;
			case GM_SEMODE_REAL :
				/* 主観やビハインドになったときだけ聞こえる */
				if ( MakeSeReal( pos, sp ) < 0 ) return -1 ;
				break ;
			case GM_SEMODE_CAMPOS :
				/* 常にカメラ位置中心に計算 */
	if ( MakeSeNormal( pos, sp ) < 0 ) return -1 ;
				break ;
			case GM_SEMODE_CAMTRG :
				/* 常にカメラターゲット中心に計算 */
	if ( MakeSeNormal( pos, sp ) < 0 ) return -1 ;
				break ;

			case GM_SEMODE_MIC :
				if ( MakeSeMic( pos, sp ) < 0 ) return -1 ;
				break ;
		}
	}
	return 0 ;
}

void	GM_SdSet( code )
int			code ;
{
	SNG_CHECK( code ) ;
	SD_SET( code ) ;
}

void	GM_SeSet( p, l, n )
int			p, l, n ;
{
	SD_SET( SE_MAKE( p, l, n ) ) ;
}

int		GM_SeGetPan( pos, mode )
FVECTOR	*pos ;
int		mode ;
{
	SEPARAM		sp ;

	GetSp( pos, mode, &sp ) ;

	return sp.pan ;
}
int		GM_SeGetVol( pos, mode )
FVECTOR	*pos ;
int		mode ;
{
	SEPARAM		sp ;

	if ( GetSp( pos, mode, &sp ) < 0 ) return 0 ;

	return sp.vol ;
}

void	GM_SeGetVolPan( pos, mode, vol, pan )
FVECTOR	*pos ;
int		mode ;
int		*vol ;
int		*pan ;
{
	SEPARAM		sp ;

	*vol = 0 ;
	*pan = GM_PAN_CENTER ;
	if ( GetSp( pos, mode, &sp ) < 0 ) return ;

	*vol = sp.vol ;
	*pan = sp.pan ;
}

int	GM_GetPanDir( int s_dir )
{
	int	dir ;
	GM_CameraSet	*cam ;

	cam = GM_GetCurrentCamera( 0 ) ;	/* 子画面は１*/

	dir = s_dir - (cam->rotate.vy - 1024) ;
	return GetPan ( dir, GM_MAX_PAN ) ;
}

void	GM_SeGetVolPanFromVolCurves( FVECTOR *pos, int *vol, int *pan, VOLCURVES *volcurves )
{
	SEPARAM		sp ;

	sp.vol = *vol = 0 ;
	sp.pan = *pan = GM_PAN_CENTER ;
	*pan = GM_PAN_CENTER ;
	if ( SeSetFromVolCurve( pos, &sp, volcurves ) < 0 ) return ;

	*vol = sp.vol ;
	*pan = sp.pan ;
}

void	GM_SeSetMode(
		int		se ,
		FVECTOR	*pos ,
		int		mode )
{
	int z;
	int hzx_id;
	SEPARAM		sp ;
	HZX_ZONE_ADD addr;

	if( GM_CurrentAddr != GM_INVALID_ADDR ) {
		GM_SeSetModeAddr( se, pos, mode, GM_CurrentAddr );
		return;
	}
	if( mode == GM_SEMODE_BOMB ) {
		// Bomb 系は障害計算しない
		GM_SeSetModeAddr( se, pos, mode, GM_INVALID_ADDR );
		return;
	}
	
	sp.vol = GM_MAX_VOL ;
	sp.pan = 0 ;
	if ( GetSp( pos, mode, &sp ) < 0 ) return ;

	hzx_id = HZX_GetHzxIDbyZone( 0, pos, &z );
	addr = HZX_AddressNo( GV_GetNo( hzx_id ), z, z );
	SeSetAddress( se, &sp.pos, addr, GM_ConvertVol( sp.vol ) );
}

void	GM_SeSetModeAddr(
		int		se ,
		FVECTOR	*pos ,
		int		mode , HZX_ZONE_ADD addr )
{
	SEPARAM		sp ;
	
	sp.vol = GM_MAX_VOL ;
	sp.pan = 0;
	if ( GetSp( pos, mode, &sp ) < 0 ) return ;

	SeSetAddress( se, &sp.pos, addr, GM_ConvertVol( sp.vol ) );
}

void	GM_SeSetEx(
		int		se ,
		FVECTOR	*pos ,
		int		max_dis ,
		int		min_dis )
{
	int z;
	int hzx_id;
	SEPARAM		sp ;
	HZX_ZONE_ADD addr;
	
	if( GM_CurrentAddr != GM_INVALID_ADDR ) {
		GM_SeSetExAddr( se, pos, max_dis, min_dis, GM_CurrentAddr );
		return;
	}
	
	sp.vol = GM_MAX_VOL ;
	sp.pan = 0 ;
	if ( MakeSeNormalEx( pos, &sp, max_dis, min_dis ) < 0 ) return ;

	hzx_id = HZX_GetHzxIDbyZone( 0, pos, &z );
	addr = HZX_AddressNo( GV_GetNo( hzx_id ), z, z );
	SeSetAddress( se, &sp.pos, addr, GM_ConvertVol( sp.vol ) );
#ifdef PRINT_DEBUG
//printf(" se[%x] vol[%d] pan[%x] \n",se, sp.vol, sp.pan ) ;
#endif
}

void	GM_SeSetExAddr(
		int		se ,
		FVECTOR	*pos ,
		int		max_dis ,
		int		min_dis, HZX_ZONE_ADD addr )
{
	SEPARAM		sp ;
	
	sp.vol = GM_MAX_VOL ;
	sp.pan = 0 ;
	if ( MakeSeNormalEx( pos, &sp, max_dis, min_dis ) < 0 ) return ;

	SeSetAddress( se, &sp.pos, addr, GM_ConvertVol( sp.vol ) );
#ifdef PRINT_DEBUG
//printf(" se[%x] vol[%d] pan[%x] \n",se, sp.vol, sp.pan ) ;
#endif
}

void	GM_SeSetFromVolCurve( int se, FVECTOR *pos, VOLCURVES *volcurves )
{
	int z;
	int hzx_id;
	SEPARAM		sp ;
	HZX_ZONE_ADD addr;
	
	if( GM_CurrentAddr != GM_INVALID_ADDR ) {
		GM_SeSetFromVolCurveAddr( se, pos, volcurves, GM_CurrentAddr );
		return;
	}
	
	sp.vol = GM_MAX_VOL ;
	sp.pan = 0 ;
	
	if ( SeSetFromVolCurve( pos, &sp, volcurves ) < 0 ) return ;

	hzx_id = HZX_GetHzxIDbyZone( 0, pos, &z );
	addr = HZX_AddressNo( GV_GetNo( hzx_id ), z, z );
	SeSetAddress( se, &sp.pos, addr, GM_ConvertVol( sp.vol ) );
#ifdef PRINT_DEBUG
//printf(" se[%x] vol[%d] pan[%x] \n",se, sp.vol, sp.pan ) ;
#endif
}

void	GM_SeSetFromVolCurveAddr( int se, FVECTOR *pos, VOLCURVES *volcurves, HZX_ZONE_ADD addr )
{
	SEPARAM		sp ;
	
	sp.vol = GM_MAX_VOL ;
	sp.pan = 0 ;
	
	if ( SeSetFromVolCurve( pos, &sp, volcurves ) < 0 ) return ;

	SeSetAddress( se, &sp.pos, addr, GM_ConvertVol( sp.vol ) );
#ifdef PRINT_DEBUG
//printf(" se[%x] vol[%d] pan[%x] \n",se, sp.vol, sp.pan ) ;
#endif
}

void GM_InitMixConvFader( )
{
	int i ;
	
	for( i=0; i<GM_MAX_TRACK; i++ ) {
		GM_MixConvFader( i, GM_PAN_CENTER, GM_MAX_VOL ) ;
	}
}

void GM_MixConvFader( track, pan, vol )
int track ;
int pan ;
int vol ;
{
	int code ;

	code = 0xfb000000 ;
#ifdef KP_WINDOWS
	if(track<0x10){
		track+=0x10;
	}
#endif
	code |= track << 16 ;
	code |= pan << 8 ;
	code |= vol ;

#ifdef PRINT_DEBUG
printf("code %8x\n",code ) ;
#endif
	GM_SdSet( code ) ;
}

void GM_MixConvFrequencyFader( int track, int freq, int pan, int vol )
{
	int code ;

	code = 0xf8000000 ;

	code |= track << 19 ;
	code |= freq << 12 ;
	code |= pan << 6 ;
	code |= vol ;

#ifdef PRINT_DEBUG
printf(" frequency code %8x\n",code ) ;
#endif
	GM_SdSet( code ) ;
}

void	GM_BgmFaderMode( track, pos, mode )
int		track ;
FVECTOR	*pos ;
int		mode ;
{
	SEPARAM		sp ;

	sp.vol = GM_MAX_VOL ;
	sp.pan = 0 ;

//	if ( GetSp( pos, mode, &sp ) < 0 ) return ;
	/* Vol＝０でもフェーダーに送る */
	GetSp( pos, mode, &sp ) ;

	GM_MixConvFader( track, sp.pan, sp.vol ) ;

//printf("BGM FADER track[%x] vol[%d] pan[%x] \n",track, sp.vol, sp.pan ) ;
}

void	GM_BgmFaderEx( track, pos, max_dis, min_dis )
int		track ;
FVECTOR	*pos ;
int		max_dis ;
int		min_dis ;
{
	SEPARAM		sp ;
	
	sp.vol = 0 ;
	sp.pan = GM_PAN_CENTER ;

//	if ( MakeSeNormalEx( pos, &sp, max_dis, min_dis ) < 0 ) return ;
	/* Vol＝０でもフェーダーに送る */
//	MakeSeSilent( pos, &sp, max_dis, min_dis ) ;
	MakeSeNormalEx( pos, &sp, max_dis, min_dis ) ;

//	if( sp.pan <= 0x1f ) sp.pan += 0x20 ;
//	else sp.pan -= 0x20 ;

	GM_MixConvFader( track, sp.pan, sp.vol ) ;
//printf("BGM FADER track[%x] vol[%d] pan[%x] \n",track, sp.vol, sp.pan ) ;
}

int	GM_SoundDistance( FVECTOR *pos )
{
	GM_CameraSet	*cam ;
	FVECTOR		vec ;

	cam = GM_GetCurrentCamera( 0 ) ;	/* 子画面は１*/
    _sceVu0SubVector( &vec, pos, &cam->position ) ;
    return (int)GV_VecLen3F( &vec ) ;
}

void	GM_InitVolumParam( )
{
	NormalVolParam.normal = normal_param ;
	NormalVolParam.ownview = normal_ownview_param ;
	NormalVolParam.demo = normal_demo_param ;

	BombVolParam.normal = bomb_param ;
	BombVolParam.ownview = bomb_ownview_param ;
	BombVolParam.demo = bomb_demo_param ;

	ConvertBombVolParam( &BombVolParam.normal );
	ConvertBombVolParam( &BombVolParam.ownview );
	ConvertBombVolParam( &BombVolParam.demo );

	RealVolParam.normal = real_param ;
	RealVolParam.ownview = real_ownview_param ;
	RealVolParam.demo = real_demo_param ;

	// Obstruction, Occlusion パラメータリセットもやっておく
	{
		DSI3DL2BUFFER	ds3db = DEF_DSI3DL2BUFFER;
		// DSI3DL2LISTENER	ds3dl = DEF_LISTENER_ENV;
		// デフォルト値は sound モジュール側で設定するようにした
		ds3db.flRoomRolloffFactor = 0.f;
		ZeroMemory( &ds3db.Obstruction, sizeof( DSI3DL2OBSTRUCTION ) );
		ZeroMemory( &ds3db.Occlusion, sizeof( DSI3DL2OCCLUSION ) );
//		ZeroMemory( &ds3db,  sizeof( ds3db ) );
		GM_SetSourceEnv( &ds3db );

		GM_SetObstructionMaterial( DEF_OBSTRUCTION_HFLEVEL,
								   DEF_OBSTRUCTION_LFRATIO );
		GM_SetOcclusionMaterial( DEF_OCCLUSION_HFLEVEL,
								 DEF_OCCLUSION_LFRATIO );
		//	sd_3dlst_seti3dl2( &ds3dl );

		addr_effect_listener = GM_INVALID_ADDR;
		GM_CurrentAddr = GM_INVALID_ADDR;

	}
}

/* -------------------------------------------------------*/
int GM_GetSdStatus(  )
{
	volatile int		*ss ;
	int	ss_status ;
	ss = sd_status() ;

	ss_status = *(ss+1) ;
	
	return ss_status ;
}

#define SD_LOADFLAG	0x80000007
void GM_SdLoadWait(  )
{
	volatile int		*ss ;
	ss = sd_status();

	while( (*(ss+1) & SD_LOADFLAG) );
	printf("setend\n");
}

int GM_SdStatus(  )
{
	int *ss ;
	
	ss = sd_status();

	if( *(ss+1) & 0x80000008 ) return GM_SDATATUS_READY ;		/* 準備中 */
	if( GM_SdStatusCount[0] != GM_SdStatusCount[1] 
		|| GM_SdStatusCount[0] != *(ss+2) ) return GM_SDSTATUS_PLAY ;	/* 再生中 */

	return GM_SDSTATUS_IDLE ;	/* アイドル状態 */
}

void KariLoad()
{
#if 0	///madakore
	extern	int	sd_set_cli( int ) ;

	GM_SdLoadWait(  ) ;
	printf( "send SE[0xFE00007F]\n" );
	sd_set_cli( 0xFE00007F );	/* 常駐 */

	GM_SdLoadWait(  ) ;
	printf( "send SE[0x0200007E]\n" );
	sd_set_cli( 0x0200007E );	/* ＳＥ */

	GM_SdLoadWait(  ) ;
	printf( "send WAVE[0xFE00007E]\n" );
	sd_set_cli( 0xFE00007E );	/* 敵兵音声波形 */

	sd_set_cli( SE_REVERB_ON );	/* リバーブ */

	GM_SdLoadWait(  ) ;
	printf( "send BGM[0x01000010]\n" );
	sd_set_cli( 0x01000010 );	/* BGM */

	GM_SdLoadWait(  ) ;
#endif
}

/* -------------------------------------------------------*/
int	GM_SeSetModeGcl( void )
{
	int	vec[ 3 ], se, mode ;
	FVECTOR	pos ;

    if ( GCL_GetOption( 's' ) == NULL ) return -1 ;
	se = GCL_GetNextInt() ;

    if ( GCL_GetOption( 'p' ) == NULL ) return -1 ;
	GCL_GetNextIV( vec ) ;
	pos.vx = (float)vec[0] ;
	pos.vy = (float)vec[1] ;
	pos.vz = (float)vec[2] ;

    if ( GCL_GetOption( 'm' ) == NULL ) return -1 ;
	mode = GCL_GetNextInt() ;

printf(" gcl call se code[%d] mode[%d]\n",se,mode) ;
	GM_SeSetMode( se, &pos, mode ) ;

	return 1 ;
}

int	GM_SetNoiseGcl( void )
{
	int	vec[ 3 ], noise, map ;
	FVECTOR	pos ;

    if ( GCL_GetOption( 'n' ) == NULL ) return -1 ;
	noise = GCL_GetNextInt() ;

    if ( GCL_GetOption( 'p' ) == NULL ) return -1 ;
	GCL_GetNextIV( vec ) ;
	pos.vx = (float)vec[0] ;
	pos.vy = (float)vec[1] ;
	pos.vz = (float)vec[2] ;

    if ( GCL_GetOption( 'm' ) == NULL ) return -1 ;
	map = GCL_GetNextInt() ;

printf(" gcl call set noise[%d] map[%d]\n",noise,map) ;
	map = GM_GetMapID( map ) ;
	GM_SetNoise( noise, &pos, map ) ;

	return 1 ;
}

int	GM_SeSetVolPanGcl( void )
{
	int	se, v, p ;

    if ( GCL_GetOption( 's' ) == NULL ) return -1 ;
	se = GCL_GetNextInt() ;

	v = GCL_GetOptionValue( 'v', GM_MAX_VOL ) ;
	p = GCL_GetOptionValue( 'p', GM_PAN_CENTER ) ;

	GM_SeSet( p, v, se ) ;

	return 1 ;
}

int	GM_SdSetGcl( void )
{
	int	code ;
	
    if ( GCL_GetOption( 'c' ) == NULL ) return -1 ;
	code = GCL_GetNextInt() ;

	GM_SdSet( code ) ;
	
	return 1 ;
}


int	GM_LoadPack( void )
{
	extern void FS_LoadSoundPak( int ) ;
	int	pak ;
	
    if ( GCL_GetOption( 'p' ) == NULL ) return -1 ;

	pak = GCL_GetNextInt() ;
	FS_LoadSoundPak( pak ) ;
	GM_SdLoadWait() ;

	printf( "GM_LoadPack:load pak %x\n", pak ) ;
	return 1 ;
}

//////////////////////// Xbox 追加
//@02/11/28(木)@ 読んでない！！
//@void	GM_SetListenerEnv( DSI3DL2LISTENER* pEnv )
//@{	// リスナー環境エフェクトの設定
//@	env_listener = *pEnv;
//@	sd_3dlst_seti3dl2( pEnv );
//@}

void	GM_SetSourceEnv( DSI3DL2BUFFER* pEnv )
{	// 音源エフェクトの設定	（マテリアルも設定される）
	env_source = *pEnv;
}

void	GM_SetObstructionMaterial( LONG lHFLevel, float flLFRatio )
{	// 障害物マテリアルの設定
	env_source.Obstruction.lHFLevel = lHFLevel;
	env_source.Obstruction.flLFRatio = flLFRatio;
}

void	GM_SetOcclusionMaterial( LONG lHFLevel, float flLFRatio )
{	// 遮蔽物マテリアルの設定
	env_source.Occlusion.lHFLevel = lHFLevel;
	env_source.Occlusion.flLFRatio = flLFRatio;
}

// Obstruction, Occlusion 計算
#define MAX_CALC_CORNER		4		// 計算する最大コーナー数
static float  CalcNearestDist( FVECTOR* pv0, FVECTOR* pv1, HZX_ZON* pZone )
{	// 直線 pv0-pv1 に最も近い pZone の頂点を pvAns に返す。xz平面の２Ｄ計算。
	// pZone を直線が通るならば 0.f を返す

	float a, b, c;
	float fmin = FLOAT_MAX;
	float fDiv;	
	int i;
	int sign = 0;

	// 直線の式 ax + bz + c = 0
	a = pv1->vz - pv0->vz;
	b = pv0->vx - pv1->vx;
	c = pv1->vx * pv0->vz - pv0->vx * pv1->vz;

	fDiv = 1.f / bp_sqrtf( a * a + b * b );   //BP_MATH - emulate PS2 sqrtf

	a *= fDiv;
	b *= fDiv;
	c *= fDiv;

	// 距離に比例した値を求めて最近点を出す
	for ( i = 0 ; i < 4; i++ ) {
		float x = ( ( i & 1 ) ? -pZone->w : pZone->w ) + pZone->x;
		float z = ( ( i & 2 ) ? -pZone->h : pZone->h ) + pZone->z;
		float fDist = a * x + b * z + c;
		int cursign = (fDist > 0.f ? 1 : -1 );

		if( cursign * sign < 0 ) return 0.f;
		sign = cursign;

//		fDist = fabs( fDist );
		*(int*)&fDist = (*(int*)&fDist) & 0x7fffffff;	// abs
		if( fDist < fmin ) {
			fmin = fDist;
		}

	}
	return fmin;
}

static float ObstructionFactorFunc( float fDist )
{	// コーナーへの距離を入力して減衰率を得る
	// 適当に線形関数をつなぐ
	// （本当は x>500 で 1 - exp( -(x - 500) * 距離調整係数) とかやったほうがきれい）
	if( fDist < 500.f ) return 0.f;
	if( fDist < 1000.f ) return ( fDist - 500.f ) * 0.001f;
	if( fDist < 11000.f ) return 0.5f + ( fDist - 1000.f ) * 5.0e-5f;
	return 1.f;
}

static	float CalcObstructionFactor( FVECTOR* pvListener, HZX_ZONE_ADD addrListener,
									 FVECTOR* pvSource, HZX_ZONE_ADD addrSource )
{	// リスナーと音源の位置、ゾーンより障害係数を計算
	HZX_ZONE_ADD	nextaddr, corneraddr ;
	HZX_ZONE_ADD	prevaddr;
	int nCorner = 0;
	float	fObstructFactor = 0.f;

	if( addrListener == GM_INVALID_ADDR
		|| addrSource == GM_INVALID_ADDR ) return 0.f;

	// コーナー位置を計算
	// source 位置からたどっていく
	nextaddr = corneraddr = prevaddr = addrSource;
	while ( HZX_ReadOnlinInfo( corneraddr, addrListener ) ) {
		HZX_ZON* pZone;
		float	fDist;
		do {	// 現コーナーから見えなくなるまで隣を探す
			prevaddr = nextaddr;
			nextaddr = HZX_NextZoneCrossGroup2( nextaddr, addrListener ) ;
		} while ( !HZX_ReadOnlinInfo( corneraddr, nextaddr )
				  && prevaddr != nextaddr );	// 到達不能な場合 prevaddr == nextaddr となる
		
		corneraddr = prevaddr;	// コーナーから見える限界の場所
		pZone = HZX_GetZoneFromAdd( corneraddr );

		fDist = CalcNearestDist( pvListener, pvSource, pZone );

		fObstructFactor = 1.f - ( 1.f - fObstructFactor ) * ( 1.f - ObstructionFactorFunc( fDist ) ) ;
		
		if( prevaddr == nextaddr ) break;
		if( ++nCorner >=  MAX_CALC_CORNER ) break;
	}
	// 一応範囲内に押し込める
	if( fObstructFactor > 1.f ) fObstructFactor = 1.f;
	if( fObstructFactor < 0.f ) fObstructFactor = 0.f;

	return fObstructFactor;
}

static inline BOOL IsOccluded( int addrListener, int addrSource )
{	// 遮蔽されているかどうか判定
	if( addrListener == GM_INVALID_ADDR || addrSource == GM_INVALID_ADDR ) return FALSE;
	if( HZX_ZoneMapNo( addrListener ) == HZX_ZoneMapNo( addrSource ) ) return FALSE;
	if( !(GM_GetMapIDfromHzxGroupID( HZX_ZoneGroupID( addrSource ) )
		  & GM_GetMapIDfromChanlDisp( 0 )) ) return TRUE;
	return FALSE;
}


int	GM_ConvertVol( int vol )
{	// PSX2 0-63 のボリュームを mB に変換
	// # log10 使いたくない。。。 > TODO:表にする
	if( vol == 0 ) return DSBVOLUME_MIN;
	return (int)( 20.f * (float)log10( (double)(vol / 63.f) ) * 100.f);
}

int	GM_ConvertFreq( int freq )
{	// PSX2 0-127 の周波数変化を変換
	return (freq - 0x40) * ( 2048 / 64 );
}

static void	SeSetAddress( int se, FVECTOR *pvpos, HZX_ZONE_ADD addr, int vol )
{	// アドレスをユーザからもらう SeSet
	DSI3DL2BUFFER	ds3db	= env_source;
	float fObstructFactor;

	/* 擬音（SE）だけ鳴らさない　*/
	if ( ((se & 0xff000000) == 0) && GM_CheckGameStatus( STATE_DEMO ) ) return ;

	if( !VectorCheck( pvpos ) ) return;

	fObstructFactor = CalcObstructionFactor( &vpos_effect_listener, addr_effect_listener,
											 pvpos, addr );
	if( se == 0 ) {
		printf("GM_SeSet* : se = 0\n" );
		return;
	}
	ds3db.Obstruction.lHFLevel = (LONG)( ds3db.Obstruction.lHFLevel * fObstructFactor);
	ds3db.Obstruction.flLFRatio = fObstructFactor;
	if( !IsOccluded( addr_effect_listener, addr ) ) {
		ds3db.Occlusion.lHFLevel = 0;
		ds3db.Occlusion.flLFRatio = 0.f;
	}

	// パラメータを設定して音を鳴らす
	{
		u_char hn = sd_3d_get_handle();
		sd_3dsrc_setvol( hn, vol );
		sd_3dsrc_setpos( hn, (D3DXVECTOR3*)pvpos );
		sd_3dsrc_seti3dl2( hn, &ds3db );
		sd_3d_play_se( hn, se );
		sd_3d_release_handle( hn , 0);
	}
}

void	GM_SdUpdate( void )
{
#if 1
	// リスナー＝カメラの位置の更新
	FMATRIX	mat;
	mat = DG_Chanl(0)->eye;
	if( VectorCheck( (FVECTOR*)&mat.m[3][0] ) ){
		sd_3dlst_setpos( (D3DXVECTOR3*)&mat.m[3][0] );
	}
	// MGS のカメラはＹ軸が下を向いている
	mat.m[1][0] = -mat.m[1][0];
	mat.m[1][1] = -mat.m[1][1];
	mat.m[1][2] = -mat.m[1][2];

	if( VectorCheck( (FVECTOR*)&mat.m[2][0] ) && VectorCheck( (FVECTOR*)&mat.m[1][0] ) ) {
		sd_3dlst_setdir( (D3DXVECTOR3*)&mat.m[2][0],
						 (D3DXVECTOR3*)&mat.m[1][0] );
#if 0
		printf("%f, %f, %f, %f, %f, %f\n", mat.m[2][0], mat.m[2][1], mat.m[2][2],
			   mat.m[1][0], mat.m[1][1], mat.m[1][2] );
#endif		
			   

	}
#if 0
	{	// カメラの最近ゾーンで計算する場合
		int z;
		int hzx_id = HZX_GetHzxIDbyZone( 0, &pw->vecPosAbs, &z );
		addr_effect_listener = HZX_AddressNo( GV_GetNo( hzx_id ), z, z );
		vpos_effect_listener = pCam->position;
	}
#else
	if( !(GM_GameStatus & STATE_PLAY_DEMO) /*GM_PlayerBody != NULL*/  ) {	// プレイヤーのゾーンで計算する場合
		addr_effect_listener = GM_PlayerAddress;
		vpos_effect_listener = GM_PlayerPosition;
	} else {
		addr_effect_listener = GM_INVALID_ADDR;
	}
#endif
	
#else
	// プレイヤー相対計算
	if( !(GM_GameStatus & STATE_PLAY_DEMO) /* GM_PlayerBody != NULL */ ) {
		if( VectorCheck( (FVECTOR*)&GM_PlayerBody->objs->world.m[2][0] )
			&& VectorCheck( (FVECTOR*)&GM_PlayerBody->objs->world.m[1][0] ) ) {
			sd_3dlst_setdir( (FVECTOR*)&GM_PlayerBody->objs->world.m[2][0],
							 (FVECTOR*)&GM_PlayerBody->objs->world.m[1][0] );
		}
		addr_effect_listener = GM_PlayerAddress;
		if( VectorCheck( &GM_PlayerPosition ) ) {
			sd_3dlst_setpos( &GM_PlayerPosition );
			vpos_effect_listener = GM_PlayerPosition;
		}
	} else {
		addr_effect_listener = GM_INVALID_ADDR;
	}
#endif
}

#endif //BP