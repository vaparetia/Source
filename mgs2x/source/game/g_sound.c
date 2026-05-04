//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	g_sound.c
	サウンド

	1999/12/02 Y.Korekado
	$Id: g_sound.c,v 1.1.1.3 2002/11/19 11:41:49 Yoshizawa1 Exp $
	
*/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>

#include	"gameheader.h"
#include	"libgv.h"
#include	"libmt.h"
#include	"libutl.h"
#include	"camera.h"

#include "bp_vector.h"

extern float bp_cli_float_param; //used for passing along 3d angle

//#define PRINT_DEBUG	(1)

/* 旧：PAN	20(左３２)～ 3F(左１) ～ 00(右１) ～ 1F(右３１) */
/* 新：PAN	0(左３２)～ 1F(左１) ～ 20(右１) ～ 3F(右３１) */
/* VOL	0 ～ 3F */

#define	NEW_VOLUME	(1)

typedef	struct	{
	int	pan ;	/* パン */
	int	vol ;	/* ボリューム */
   float bp_angle;
} SEPARAM ;

//#define CAMERA_POS	(1)	//常にカメラ中心に計算
#define PAN_NEW (1)

#ifdef DEBUG_MODE
int	GM_SE_TYPE = 0 ;
#else
int	GM_SE_TYPE = 0 ;
#endif

static int	sng_code=0 ;	// 現在なっている曲番号

#if 0
 各キャラで管理するように
1fで0x10値が変化するとノイズが発生する
static	u_cahr	bgm_fader_vol[32] ;	/* 音量補完用 */
static	u_cahr	bgm_fader_pan[32] ;	/* パン補完用 */
#endif
/* -------------------------------------------------------*/
#define PLAYER_MIC_READY ((Ply_GetPlayerWeapon() == WP_Mic)||(Ply_GetPlayerWeapon() == WP_DemoMic))

/* -------------------------------------------------------*/
static float dir_from_matrix_point( FVECTOR* sound_pos, float * const pOutBPAngle ) ;

float bp_pan64_to_angle( const int p )
{
   //This obviously only returns an angle in the front half...
   float bp_angle = (float)( p - 0x20 ) * (M_PI/2) / 0x20;
   return bp_angle;
}

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

//#ifdef PRINT_DEBUG
#if 0
printf("koreeeeeeeeeeeeeeeeeee call se code 0x%x\n",code ) ;
#endif

	/* 擬音（SE）だけ鳴らさない　*/
	if ( ((code & 0xff000000) == 0) && GM_CheckGameStatus( STATE_DEMO ) ) return ; 

//printf(" korekorekroekroe1 [%x]\n", code ) ;
//	code = 0x7b622a ;
	sd_set_cli( code ) ;
//printf(" korekorekroekroe2 \n" ) ;
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

static	inline	void	SE_SET( int p, int l, int n, float bp_angle )
{
   bp_cli_float_param = bp_angle;
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

static	int	GetPan2 ( dir, r, k ) 
int	dir ;
int	r ;
float	k ;
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
#if 0
	dir = GV_VecDir2( &vec ) ;
	dir = dir - (cam->rotate.vy - 1024) ;
	pan = GetPan2 ( dir, r, volcurve->pan ) ;
#else
	{
	    dir = 0x20 * dir_from_matrix_point(pos, &separam->bp_angle);
	    pan = 0x20 + dir;
	    if (pan >= 0x40){
			pan = 0x3f;
	    } else if ( pan < 0 ) {
			pan = 0x00;
		}
	    //	    printf ("Pan %d\n", dir);
	}
#endif

	separam->vol = vol ;
	separam->pan = pan ;

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
static float dir_from_matrix_point(FVECTOR* sound_pos, float * const pOutBPAngle){
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

    if( pOutBPAngle )
    {
       //BP - get 3D angle around yaw.
       *pOutBPAngle = atan2f( pany, panx );

       if( *pOutBPAngle != *pOutBPAngle )
       {
          printf("SOUND ERROR: Invalid 3d angle: %f (panx: %f pany: %f)\n", *pOutBPAngle, panx, pany);
#ifndef GOLD_VERSION
          // AS(JM) - Intentionally cause a crash in non-ship builds in w32a
          if ( !strcmp( GM_GetArea(), "w32a" ) )
          {
            HANGUP();
          }
#endif
          *pOutBPAngle = 0;
       }
    }

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
	    dir = 0x20 * dir_from_matrix_point(pos, &separam->bp_angle);
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
	separam->pan = pan ;

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

//printf("dir[%d] pan[0x%x] vol[%d] \n\n",dir, pan , vol ) ;

	if ( vol <= 0 )  return -1 ;

	return 0 ;
}

#else
static	int	SetSParam_Mic( pos, len, separam, mode )
FVECTOR	*pos ;
int		len ;
SEPARAM	*separam ;
int		mode ;
{
	FVECTOR		vec ;
	int			dir, dir_x, dis, vol, r, pan, diff_dir, max_dis, min_dis, n_vol ;
	int			max_dis_in, min_dis_in, max_dis_out, min_dis_out, base_max_vol, base_min_vol ;
	float 		k, adj, max_vol, v_len, f ;
	GM_CameraSet	*cam ;

	cam = GM_GetCurrentCamera( 0 ) ;	/* 子画面は１*/

	/* ＶＯＬ計算 */
#ifdef CAMERA_POS
    _sceVu0SubVector( &vec, pos, &cam->position ) ;
#else
#ifdef DEBUG_MODE
	if( GM_SE_TYPE ) {
		_sceVu0SubVector( &vec, pos, &GM_PlayerPosition ) ;
	} else {
	    _sceVu0SubVector( &vec, pos, &cam->position ) ;
	}
#else
    _sceVu0SubVector( &vec, pos, &cam->position ) ;
#endif
#endif
    dis = (int)GV_VecLen3F( &vec ) ;
	dis -= len ;
	if ( dis < 0 ) dis = 0 ;
	if ( dis > GM_SE_MIN_DIS_CAMPOS ) dis = GM_SE_MIN_DIS_CAMPOS ;

	base_max_vol = GM_MAX_VOL ;
	base_min_vol = 0 ;
	if ( mode == GM_SEMODE_MIC ) {
		base_max_vol /= 2 ;
	}
	if ( mode == GM_SEMODE_BOMB ) {
		base_min_vol = 0x10 ;
	}

	/* ＰＡＮ計算 */
	dir = GV_VecDir2( &vec ) ;

#if 0	/* 狭指向性マイク 角度０度 */
{
	FVECTOR *p, *v1, *v2 ;
    FVECTOR	d0, d1, d2, d3, d4 ;

	p = &cam->position ;
	v1 = &cam->target ;
	v2 = pos ;

    _sceVu0SubVector( &d2, p, v1 ) ;
    _sceVu0SubVector( &d1, v2, v1 ) ;
    _sceVu0Normalize( &d1, &d1 ) ;
    _sceVu0OuterProduct( &d3, &d1, &d2 ) ;
    _sceVu0OuterProduct( &d4, &d1, &d3 ) ;
    v_len =  GV_VecLen3F( &d4 ) ;

    _sceVu0SubVector( &d0, p, v2 ) ;
    _sceVu0Normalize( &d0, &d0 ) ;
    _sceVu0Normalize( &d2, &d2 ) ;
    f = _sceVu0InnerProduct( &d0, &d2) ;
}

printf("point to vector [%f] f[%f]\n",v_len, f ) ;

	max_vol = base_max_vol ;

	n_vol = (int)( (GM_SE_MIN_DIS_MIC_VEC - GM_SE_MAX_DIS_MIC_VEC) / (max_vol) ) ;
	vol = MakeVolumeEx( v_len, GM_SE_MAX_DIS_MIC_VEC, GM_SE_MIN_DIS_MIC_VEC, n_vol, max_vol ) ;
printf("max_vol[%f] n_vol[%d]\n",max_vol, n_vol ) ;
#else
	/* 鋭指向性マイク 角度 0～MIC_IN_COS ～ MIC_OUT_COS */
{
	FVECTOR *p, *v1, *v2 ;
    FVECTOR	d1, d2 ;
    float f_dir ;

	p = &cam->position ;
	v1 = &cam->target ;
	v2 = pos ;

    _sceVu0SubVector( &d1, p, v1 ) ;
    _sceVu0SubVector( &d2, p, v2 ) ;
    _sceVu0Normalize( &d1, &d1 ) ;
    _sceVu0Normalize( &d2, &d2 ) ;
    f = _sceVu0InnerProduct( &d1, &d2) ;

	max_dis_in = GM_SE_MAX_DIS_MIC_IN ;
	min_dis_in = GM_SE_MIN_DIS_MIC_IN ;
	max_dis_out = GM_SE_MAX_DIS_MIC_OUT ;
	min_dis_out = GM_SE_MIN_DIS_MIC_OUT ;

	switch ( mode ) {
		case GM_SEMODE_MIC :
			max_dis_in = 500 ;
			min_dis_in = 2500 ;
			max_dis_out = 0 ;
			min_dis_out = 500 ;
			break ;
		case GM_SEMODE_BOMB :
			max_dis_out = 0 ;
			min_dis_out = GM_SE_MIN_DIS_CAMPOS_BOMB ;	/* 少しは鳴るように */
			break ;
	}
	if ( f > mic_in_cos ) {
		max_dis = max_dis_in ;
		min_dis = min_dis_in ;
		max_vol = (float)base_max_vol ;
	} else if ( f > mic_out_cos ) {
		k = ( mic_in_cos - f ) / (mic_in_cos - mic_out_cos) ;
		adj = (((float)max_dis_in - (float)max_dis_out)*k) ;
		max_dis = max_dis_in - (int)adj ;
		adj = (((float)min_dis_in - (float)min_dis_out)*k) ;
		min_dis = min_dis_in - (int)adj ;
		max_vol = (float)base_max_vol*( 1 - (k*7/8)) ;
	} else {
		max_dis = max_dis_out ;
		min_dis = min_dis_out ;
		max_vol = (float)base_max_vol / 8 ;
	}
	n_vol = (int)( (min_dis - max_dis) / (max_vol) ) ;
printf("f[%.2f] dis[%d], max_dis[%d] min_dis[%d] max_vol[%.2f] n_vol[%d]\n",f, dis, max_dis,min_dis,max_vol, n_vol ) ;
	vol = MakeVolumeEx( dis, max_dis, min_dis, n_vol, max_vol ) ;
}
#endif

	/* 主観でのステレオ効果をはっきりさせるため、距離を長くして計算する。*/
//	dis *= 4 ;
	dis *= 2 ;
	if ( dis > min_dis ) dis = min_dis ;

//printf("dir[%d] - camera[%d] = cma->dir[%d] ",dir, cam->rotate.vy, dir - (cam->rotate.vy ) ) ;
	dir = dir - (cam->rotate.vy - 1024) ;
	r = GM_MAX_PAN * dis / min_dis ;
	pan = GetPan ( dir, r ) ;

	separam->vol = vol ;
	separam->pan = pan ;

printf("dir[%d] pan[0x%x] vol[%d] \n\n",dir, pan , vol ) ;

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

	if ( vol == 0 ) return -1 ;

	return 0 ;
}

static	int	SetSParam_CamTrg( pos, len, separam )
FVECTOR	*pos ;		/* 発生位置 */
int		len ;		/* ＭＡＸ範囲 */
SEPARAM	*separam ;
{
	FVECTOR	vec ;
	int	dir, dis, vol, r, pan ;
	GM_CameraSet	*cam ;

	cam = GM_GetCurrentCamera( 0 ) ;	/* 子画面は１*/

	/* ＶＯＬ計算 */
#ifdef CAMERA_POS
    _sceVu0SubVector( &vec, pos, &cam->position ) ;
#else
#ifdef DEBUG_MODE
	if( GM_SE_TYPE ) {
		_sceVu0SubVector( &vec, pos, &GM_PlayerPosition ) ;
	} else {
	    _sceVu0SubVector( &vec, pos, &cam->position ) ;
	}
#else
	_sceVu0SubVector( &vec, pos, &cam->position ) ;
#endif
#endif
    dis = (int)GV_VecLen3F( &vec ) ;
	dis -= len ;	/* 最大音量がなる範囲分は差し引く */
	if ( dis < 0 ) dis = 0 ;
	if ( dis > GM_SE_MIN_DIS_CAMTRG ) dis = GM_SE_MIN_DIS_CAMTRG ;

	vol = MakeVolume( dis, GM_SE_MAX_DIS_CAMTRG, GM_SE_MIN_DIS_CAMTRG, GM_SE_VOL_CAMTRG ) ;

	/* ＰＡＮ計算 */
	dir = GV_VecDir2( &vec ) ;
	dir = dir - (cam->rotate.vy - 1024) ;
	r = GM_MAX_PAN * dis / GM_SE_MIN_DIS_CAMTRG ;
	pan = GetPan ( dir, r ) ;

	separam->vol = vol ;
	separam->pan = pan ;

	if ( vol == 0 ) return -1 ;

	return 0 ;
}

static	int	SetSParam_CamTrgBomb( pos, len, separam )
FVECTOR	*pos ;		/* 発生位置 */
int		len ;		/* ＭＡＸ範囲 */
SEPARAM	*separam ;
{
	FVECTOR	vec ;
	int	dir, dis, vol, r, pan ;
	GM_CameraSet	*cam ;

	cam = GM_GetCurrentCamera( 0 ) ;	/* 子画面は１*/

	/* ＶＯＬ計算 */
#ifdef CAMERA_POS
    _sceVu0SubVector( &vec, pos, &cam->position ) ;
#else
#ifdef DEBUG_MODE
	if( GM_SE_TYPE ) {
		_sceVu0SubVector( &vec, pos, &GM_PlayerPosition ) ;
	} else {
	    _sceVu0SubVector( &vec, pos, &cam->position ) ;
	}
#else
	_sceVu0SubVector( &vec, pos, &cam->position ) ;
#endif
#endif
    dis = (int)GV_VecLen3F( &vec ) ;
	dis -= len ;	/* 最大音量がなる範囲分は差し引く */
	if ( dis < 0 ) dis = 0 ;
	if ( dis > GM_SE_MIN_DIS_CAMTRG_BOMB ) dis = GM_SE_MIN_DIS_CAMTRG_BOMB ;

	vol = MakeVolume( dis, GM_SE_MAX_DIS_CAMTRG_BOMB, GM_SE_MIN_DIS_CAMTRG_BOMB, GM_SE_VOL_CAMTRG_BOMB ) ;

	/* ＰＡＮ計算 */
	dir = GV_VecDir2( &vec ) ;
	dir = dir - (cam->rotate.vy - 1024) ;
	r = GM_MAX_PAN * dis / GM_SE_MIN_DIS_CAMTRG_BOMB ;
	pan = GetPan ( dir, r ) ;

	separam->vol = vol ;
	separam->pan = pan ;

	if ( vol == 0 ) return -1 ;

	return 0 ;
}


static	int	SetSParam_CamPos( pos, len, separam )
FVECTOR	*pos ;
int		len ;
SEPARAM	*separam ;
{
	FVECTOR		vec ;
	int			dir, dis, vol, r, pan ;
	GM_CameraSet	*cam ;

	cam = GM_GetCurrentCamera( 0 ) ;	/* 子画面は１*/

	/* ＶＯＬ計算 */
#ifdef CAMERA_POS
    _sceVu0SubVector( &vec, pos, &cam->position ) ;
#else
#ifdef DEBUG_MODE
	if( GM_SE_TYPE ) {
		_sceVu0SubVector( &vec, pos, &GM_PlayerPosition ) ;
	} else {
	    _sceVu0SubVector( &vec, pos, &cam->position ) ;
	}
#else
	_sceVu0SubVector( &vec, pos, &cam->position ) ;
#endif
#endif
    dis = (int)GV_VecLen3F( &vec ) ;
	dis -= len ;
	if ( dis < 0 ) dis = 0 ;
	if ( dis > GM_SE_MIN_DIS_CAMPOS ) dis = GM_SE_MIN_DIS_CAMPOS ;

	vol = MakeVolume( dis, GM_SE_MAX_DIS_CAMPOS, GM_SE_MIN_DIS_CAMPOS, GM_SE_VOL_CAMPOS ) ;

	/* 主観でのステレオ効果をはっきりさせるため、距離を長くして計算する。*/
//	dis *= 4 ;
	dis *= 2 ;
	if ( dis > GM_SE_MIN_DIS_CAMPOS ) dis = GM_SE_MIN_DIS_CAMPOS ;

	/* ＰＡＮ計算 */
	dir = GV_VecDir2( &vec ) ;
	dir = dir - (cam->rotate.vy - 1024) ;
	r = GM_MAX_PAN * dis / GM_SE_MIN_DIS_CAMPOS ;
	pan = GetPan ( dir, r ) ;

	separam->vol = vol ;
	separam->pan = pan ;

	if ( vol <= 0 )  return -1 ;

	return 0 ;
}


static	int	SetSParam_CamPosBomb( pos, len, separam )
FVECTOR	*pos ;
int		len ;
SEPARAM	*separam ;
{
	FVECTOR		vec ;
	int			dir, dis, vol, r, pan ;
	GM_CameraSet	*cam ;

	cam = GM_GetCurrentCamera( 0 ) ;	/* 子画面は１*/

	/* ＶＯＬ計算 */
#ifdef CAMERA_POS
    _sceVu0SubVector( &vec, pos, &cam->position ) ;
#else
#ifdef DEBUG_MODE
	if( GM_SE_TYPE ) {
		_sceVu0SubVector( &vec, pos, &GM_PlayerPosition ) ;
	} else {
	    _sceVu0SubVector( &vec, pos, &cam->position ) ;
	}
#else
	_sceVu0SubVector( &vec, pos, &cam->position ) ;
#endif
#endif
    dis = (int)GV_VecLen3F( &vec ) ;
	dis -= len ;
	if ( dis < 0 ) dis = 0 ;
	if ( dis > GM_SE_MIN_DIS_CAMPOS_BOMB ) dis = GM_SE_MIN_DIS_CAMPOS_BOMB ;

	vol = MakeVolume( dis, GM_SE_MAX_DIS_CAMPOS_BOMB, GM_SE_MIN_DIS_CAMPOS_BOMB, GM_SE_VOL_CAMPOS_BOMB ) ;
	if ( vol <= 0 )  return -1 ;

	/* 主観でのステレオ効果をはっきりさせるため、距離を長くして計算する。*/
	dis *= 4 ;
	if ( dis > GM_SE_MIN_DIS_CAMPOS_BOMB ) dis = GM_SE_MIN_DIS_CAMPOS_BOMB ;

	/* ＰＡＮ計算 */
	dir = GV_VecDir2( &vec ) ;
	dir = dir - (cam->rotate.vy - 1024) ;
	r = GM_MAX_PAN * dis / GM_SE_MIN_DIS_CAMPOS_BOMB ;
	pan = GetPan ( dir, r ) ;

	separam->vol = vol ;
	separam->pan = pan ;

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
	if ( GM_CheckPlayerStatus(PLAYER_BEHIND|PLAYER_INTRUDE|PLAYER_WATCH) ) {
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
	VOLUMEPARAM		tmpvolparam ;
	GM_CameraSet	*cam ;

	cam = GM_GetCurrentCamera( 0 ) ;	/* 子画面は１*/

#if 0
	/* 集音マイク */
	if ( PLAYER_MIC_READY ) {
		return SetSParam_Mic( pos, 0, sp, GM_SEMODE_NORMAL ) ;
	}

	if ( GM_GameStatus & STATE_PLAY_DEMO ) {
		tmpvolparam = NormalVolParam.demo ;
	} else if ( GM_CheckPlayerStatus(PLAYER_BEHIND|PLAYER_INTRUDE|PLAYER_WATCH) ) {
		tmpvolparam = NormalVolParam.ownview ;
	} else {
		tmpvolparam = NormalVolParam.normal ;
	}

	return NewSetSeParam( pos, &tmpvolparam, sp ) ;
#else

	if ( GM_CheckPlayerStatus(PLAYER_BEHIND|PLAYER_INTRUDE|PLAYER_WATCH) ) {
		return SetSParam_CamTrg2( pos, &cam->position, GM_SE_MAX_DIS_CAMTRG, sp, 0, min_dis*6/5,  GM_MAX_VOL  ) ;
	}

	return SetSParam_CamTrg2( pos, &cam->target, GM_SE_MAX_DIS_CAMTRG, sp, max_dis, min_dis,  GM_MAX_VOL  ) ;
#endif
}

/* 俯瞰ではかすかに、主観だと良く聞こえる */
static int MakeSeSilent( pos, sp, max_dis, min_dis )
FVECTOR		*pos ;
SEPARAM		*sp ;
int			max_dis ;
int			min_dis ;
{
	GM_CameraSet	*cam ;

	cam = GM_GetCurrentCamera( 0 ) ;	/* 子画面は１*/

	if ( GM_CheckPlayerStatus(PLAYER_BEHIND|PLAYER_INTRUDE|PLAYER_WATCH) ) {
//		return SetSParam_CamPos( pos, GM_SE_MAX_DIS_CAMPOS, sp ) ;
		return SetSParam_CamTrg2( pos, &cam->position, GM_SE_MAX_DIS_CAMPOS, sp, 0, min_dis*6/5,  GM_MAX_VOL ) ;
	}

#ifdef CAMERA_POS
	return SetSParam_CamTrg2( pos, &cam->position, GM_SE_MAX_DIS_CAMTRG, sp, max_dis, min_dis,  GM_MAX_VOL/2 ) ;
#else
#ifdef DEBUG_MODE
	if( GM_SE_TYPE ) {
		return SetSParam_CamTrg2( pos, &GM_PlayerPosition, GM_SE_MAX_DIS_CAMTRG, sp, max_dis, min_dis,  GM_MAX_VOL  ) ;
	} else {
		return SetSParam_CamTrg2( pos, &cam->position, GM_SE_MAX_DIS_CAMTRG, sp, max_dis, min_dis,  GM_MAX_VOL  ) ;
	}
#else
	return SetSParam_CamTrg2( pos, &cam->position, GM_SE_MAX_DIS_CAMTRG, sp, max_dis, min_dis,  GM_MAX_VOL  ) ;
#endif
#endif
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
	if ( GM_CheckPlayerStatus(PLAYER_BEHIND|PLAYER_INTRUDE|PLAYER_WATCH) ) {
		return NewSetSeParam( pos, &NormalVolParam.ownview, sp ) ;
	}

	return NewSetSeParam( pos, &NormalVolParam.normal, sp ) ;
#else
	if ( GM_CheckPlayerStatus(PLAYER_BEHIND|PLAYER_INTRUDE|PLAYER_WATCH) ) {
		return SetSParam_CamPos( pos, GM_SE_MAX_DIS_CAMPOS, sp ) ;
	}

	return SetSParam_CamTrg( pos, GM_SE_MAX_DIS_CAMTRG, sp ) ;
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
	if ( GM_CheckPlayerStatus(PLAYER_BEHIND|PLAYER_INTRUDE|PLAYER_WATCH) ) {
		return NewSetSeParam( pos, &BombVolParam.ownview, sp ) ;
	}

	return NewSetSeParam( pos, &BombVolParam.normal, sp ) ;
#else
	if ( GM_CheckPlayerStatus(PLAYER_BEHIND|PLAYER_INTRUDE|PLAYER_WATCH) ) {
		return SetSParam_CamPosBomb( pos, GM_SE_MAX_DIS_CAMPOS, sp ) ;
	}

	return SetSParam_CamTrgBomb( pos, GM_SE_MAX_DIS_CAMTRG, sp ) ;
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
	if ( GM_CheckPlayerStatus(PLAYER_BEHIND|PLAYER_INTRUDE|PLAYER_WATCH) ) {
		return NewSetSeParam( pos, &RealVolParam.ownview, sp ) ;
	}
#else
	if ( GM_CheckPlayerStatus(PLAYER_BEHIND|PLAYER_INTRUDE|PLAYER_WATCH) ) {
		return SetSParam_CamPos( pos, GM_SE_MAX_DIS_CAMPOS, sp ) ;
	}

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
   sp->bp_angle = 0.f;
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

/* -------------------------------------------------------*/
/* 音量は指定、パンは自動計算 */
/*
void	GM_SeSetArea( pos, se, len )
SVECTOR	*pos ;
int		se;
int		len ;
{
	SEPARAM	separam ;

	if ( SetSoundModeArea( &separam, pos, len ) < 0 ) {
		return ;
	}
	SE_SET( separam.pan, separam.vol, se );
}
*/

void	GM_SdSet( code )
int			code ;
{
	SNG_CHECK( code ) ;

   //BP - this *should* extract the pan from the code if this is a se play
   //command, but the way GM_SdSet is used to play sounds implies that the
   //sound macros ignore the vol and pan settings since they are both 0 wherever
   //it's called which would be the wrong settings.
   //Safest to just zero out bp_angle parameter here.
   bp_cli_float_param = 0.f;
	SD_SET( code ) ;
}

void	GM_SeSet( int p, int l, int n )
{
   if( p != GM_PAN_CENTER )
      BP_TODO_BREAK; //BP - this call needs to be converted over to GM_SeSet3D()

   GM_SeSet3D( p, l, n, 0.f );
}

void	GM_SeSet3D( int p, int l, int n, float bp_angle )
{
   bp_cli_float_param = bp_angle;
	SD_SET( SE_MAKE( p, l, n ) ) ;
}

int		GM_SeGetPan( FVECTOR * pos, int mode, float * pOutBPAngle )
{
	SEPARAM		sp ;

	GetSp( pos, mode, &sp ) ;

   *pOutBPAngle = sp.bp_angle;
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

void	GM_SeGetVolPan( FVECTOR * pos, int mode, int *vol, int *pan, float *bp_angle )
{
	SEPARAM		sp ;

	*vol = 0 ;
	*pan = GM_PAN_CENTER ;
   *bp_angle = 0.f;

	if ( GetSp( pos, mode, &sp ) < 0 ) return ;

	*vol = sp.vol ;
	*pan = sp.pan ;
   *bp_angle = sp.bp_angle;
}

int	GM_GetPanDir( int s_dir )
{
	int	dir ;
	GM_CameraSet	*cam ;

	cam = GM_GetCurrentCamera( 0 ) ;	/* 子画面は１*/

	dir = s_dir - (cam->rotate.vy - 1024) ;
	return GetPan ( dir, GM_MAX_PAN ) ;
}

void	GM_SeGetVolPanFromVolCurves( FVECTOR *pos, int *vol, int *pan, VOLCURVES *volcurves, float * bp_angle )
{
	SEPARAM		sp ;

	sp.vol = *vol = 0 ;
	sp.pan = *pan = GM_PAN_CENTER ;
   sp.bp_angle = *bp_angle = 0.f;
	if ( SeSetFromVolCurve( pos, &sp, volcurves ) < 0 ) return ;

	*vol = sp.vol ;
	*pan = sp.pan ;
   *bp_angle = sp.bp_angle;
}

void	GM_SeSetMode( int se, FVECTOR * pos, int mode )
{
	SEPARAM		sp ;
	
	sp.vol = GM_MAX_VOL ;
	sp.pan = 0 ;
   sp.bp_angle = 0.f;

//if( se!=SD_P_WALL02)return ;

	if ( GetSp( pos, mode, &sp ) < 0 ) return ;
	SE_SET( sp.pan, sp.vol, se, sp.bp_angle ) ;

//#ifdef PRINT_DEBUG
#if 0
if( se >= 729 ) 
printf(" se[%x] vol[%d] pan[%x] \n",se, sp.vol, sp.pan ) ;
#endif
}

void	GM_SeSetEx( se, pos, max_dis, min_dis )
int		se ;
FVECTOR	*pos ;
int		max_dis ;
int		min_dis ;
{
	SEPARAM		sp ;
	
	sp.vol = GM_MAX_VOL ;
	sp.pan = 0 ;
   sp.bp_angle = 0.f;

	if ( MakeSeNormalEx( pos, &sp, max_dis, min_dis ) < 0 ) return ;

	SE_SET( sp.pan, sp.vol, se, sp.bp_angle ) ;

#ifdef PRINT_DEBUG
//printf(" se[%x] vol[%d] pan[%x] \n",se, sp.vol, sp.pan ) ;
#endif
}

void	GM_SeSetFromVolCurve( int se, FVECTOR *pos, VOLCURVES *volcurves )
{
	SEPARAM		sp ;
	
	sp.vol = GM_MAX_VOL ;
	sp.pan = 0 ;
   sp.bp_angle = 0.f;
	
	if ( SeSetFromVolCurve( pos, &sp, volcurves ) < 0 ) return ;

	SE_SET( sp.pan, sp.vol, se, sp.bp_angle ) ;

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
   sp.bp_angle = 0.f;

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
   sp.bp_angle = 0.f;

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

	RealVolParam.normal = real_param ;
	RealVolParam.ownview = real_ownview_param ;
	RealVolParam.demo = real_demo_param ;
}

#if 0
void	GM_BgmFaderInit( )
{
	int i ;
	
	for( i=0; i<32; i++ ) {
		bgm_fader_vol[i] = 0 ;
		bgm_fader_pan[i] = 0x20 ;
	}
}
#endif
/* -------------------------------------------------------*/
extern int *sd_status( void );

int GM_GetSdStatus(  )
{
	volatile int		*ss ;
	int	ss_status ;
	ss = sd_status() ;

	ss_status = *(ss+1) ;
	
	return ss_status ;
}

extern void BP_EndFrame();

#define SD_LOADFLAG	0x80000007
void GM_SdLoadWait(  )
{
   extern	int *sd_status( void ) ;
   volatile int		*ss ;
   ss = sd_status() ;
#if 1 //BP_PS2
   while( (*(ss+1) & SD_LOADFLAG) )
   {
      BP_EndFrame();
   }
#else
#if 0
	while( (*(ss+1) & SD_LOADFLAG) ) {
		printf("sound.c: LOADFLAG[%x]\n",*(ss+1) ) ;
		sceGsSyncV(0) ;
	}
	sd_set_cli( 1 ) ;
#else
	WAIT_HSYNC( 262 );
	WAIT_HSYNC( 262 );
	WAIT_HSYNC( 262 );

	while( (*(ss+1) & SD_LOADFLAG) ) WAIT_HSYNC( 262 );
#endif
#endif
}

int GM_SdStatus(  )
{
	extern	int *sd_status( void ) ;
	int *ss ;
	
	ss = sd_status() ;

	if( *(ss+1) & 0x80000008 ) return GM_SDATATUS_READY ;		/* 準備中 */
	if( GM_SdStatusCount[0] != GM_SdStatusCount[1] 
		|| GM_SdStatusCount[0] != *(ss+2) ) return GM_SDSTATUS_PLAY ;	/* 再生中 */

	return GM_SDSTATUS_IDLE ;	/* アイドル状態 */
}

void KariLoad()
{
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
   float bp_angle;

    if ( GCL_GetOption( 's' ) == NULL ) return -1 ;
	se = GCL_GetNextInt() ;

	v = GCL_GetOptionValue( 'v', GM_MAX_VOL ) ;
	p = GCL_GetOptionValue( 'p', GM_PAN_CENTER ) ;

   //BP - they never seem to use anything other than center pan (0x20) for this in scripting,
   //which presumably means they're all supposed to be ambient.
   //We'll have to catch this on the other end based on se ID to cause them to spread
   //rather than wind up in the center channel due to 3D pan.
   //ALSO this may be indirectly used by NewEMA_CommandGetMiceVolPan()...need to test!
   bp_angle = bp_pan64_to_angle( p );
	GM_SeSet3D( p, v, se, bp_angle ) ;

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
