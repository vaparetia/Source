//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	windnois.c
	風音

	2000/06/28 Y.Korekado
	$Id: windnois.c,v 1.1.1.3 2002/11/19 11:44:29 Yoshizawa1 Exp $
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

#include	"gameheader.h"
#include	"korekado/conv/define.h"
#include	"camera.h"
/*----------------------------------------------------------------*/


/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;

	char	upwind_pan ;
	char	downwind_pan ;
	char	earwind_l_pan ;
	char	earwind_r_pan ;

	char	wind_vol ;
	char	earwind_vol ;
	char	micwind_vol ;
	char	c_box_vol ;

	int		stage ;		//(0:tanker,rain 1:plant,fine)
	int		mode ;
} Work ;

/*----------------------------------------------------------------*/
#define PRIO	0x40	/* 敵兵より後に処理 */



#define	UPWIND		0x04	/* 風上の音 */
#define	DOWNWIND	0x05	/* 風下の音 */
#define	EARWIND_L	0x0A 	/* 主観耳風ボーボー音(左) */
#define	EARWIND_R	0x0B 	/* 主観耳風ボーボー音(右) */
#define	C_BOX_1		0x0C 	/* ダンボール音１|プラントはマイク風 */
#define	C_BOX_2		0x0D 	/* ダンボール音２|プラントはマイク風 */

#define	MIC_L		0x0C 	/* プラントはマイク風 */
#define	MIC_R		0x0D 	/* プラントはマイク風 */

#define PAN_INTERP_DOWN	(1)
#define PAN_INTERP	(2)
#define VOL_INTERP	(4)

#define WIND_SPEED_MAX	(500.0f)
#define WIND_SPEED_BASE	(100.0f)


enum {
	WEATHER_RAIN,
	WEATHER_FAIN,
} ;
/*----------------------------------------------------------------*/
extern	FVECTOR	G_wind ;				/* 風速（単位フレームに移動する距離） */
extern	SVECTOR	G_wind_rot ;			/* 風向 */
extern	int		G_wind_intense ;		/* 強さ */
extern	int		G_wind_intense_max ;	/* 強さ */
extern	int		G_wind_sw ;				/* 突風発生中に立つ */
extern	FMATRIX	G_wind_matrix ;
/*----------------------------------------------------------------*/


static	inline	float __RcosF( d )
int	d ;
{
	float f, t ;

	f = (float)M_PI * (float)d / 2048.0F ;
	t = cosf( f ) ;

	return t ;
}

static void WindNoise( Work *work )
{
	extern void OK_GetLocalWindDirection( FVECTOR *pos, SVECTOR *output ) ;
	GM_CameraSet	*cam ;
	int	pan, dir, p, vol, box_vol, panl, panr, c_box ;
	float	v, f ;

//G_wind_rot.vy = 1024 ;

	cam = GM_GetCurrentCamera( 0 ) ;	/* メイン画面のみ */
//	OK_GetLocalWindDirection( FVECTOR *pos, SVECTOR *output )

	f = ((float)G_wind_intense - WIND_SPEED_BASE) / WIND_SPEED_MAX ;
	v = f * 0x17 ;
	vol = 0x28 + (int)v ;
	if ( vol > 0x3f ) vol = 0x3f ;
	box_vol = vol ;
	if ( work->wind_vol-VOL_INTERP > vol ) {
		work->wind_vol -= VOL_INTERP ;
	} else if( work->wind_vol+VOL_INTERP < vol ) {
		work->wind_vol += VOL_INTERP ;
	} else {
		work->wind_vol = vol ;
	}

//printf(" wind dir[%d] speed [%d/%d] vol[%x]\n", G_wind_rot.vy, G_wind_intense,G_wind_intense_max, work->wind_vol ) ;
	c_box = 1 ;
	if ( work->stage == WEATHER_RAIN ) {
		/* ダンボール音 */
		if ( (GM_PlayerStatus&PLAYER_CB_BOX)&&(GM_PlayerStatus&PLAYER_WATCH) ) {
			if( GM_PlayerControl->level_found & 0x2 ) {
				box_vol = 0 ;
			}
			c_box = 2 ;
		} else {
			box_vol = 0 ;
		}
		if ( work->c_box_vol-VOL_INTERP > box_vol ) {
			work->c_box_vol -= VOL_INTERP ;
		} else if( work->c_box_vol+VOL_INTERP < box_vol ) {
			work->c_box_vol += VOL_INTERP ;
		} else {
			work->c_box_vol = box_vol ;
		}
		GM_MixConvFader( C_BOX_1, 0x20, work->c_box_vol ) ;
		GM_MixConvFader( C_BOX_2, 0x20, work->c_box_vol ) ;
	}


	/* 風下 */
	dir = G_wind_rot.vy - (cam->rotate.vy - 1024) ;
	dir &= 4095 ;
	p =(int)(__RcosF( dir ) * 31.0f ) ;
	pan = 0x20 + p ;
	if( work->downwind_pan-PAN_INTERP_DOWN > pan ) {
		work->downwind_pan -= PAN_INTERP_DOWN ;
	} else if( work->downwind_pan+PAN_INTERP_DOWN < pan ) {
		work->downwind_pan += PAN_INTERP_DOWN ;
	} else {
		work->downwind_pan = pan ;
	}
	GM_MixConvFader( DOWNWIND, work->downwind_pan, work->wind_vol/c_box ) ;

//printf("down pan[%x] ", work->downwind_pan ) ;

	/* 風上 */
	dir = (G_wind_rot.vy+2048) - (cam->rotate.vy - 1024) ;
	dir &= 4095 ;
	p =(int)(__RcosF( dir ) * 31.0f ) ;
	pan = 0x20 + p ;
//printf("p[%d] ", p) ;
	if( work->upwind_pan-PAN_INTERP > pan ) {
		work->upwind_pan -= PAN_INTERP ;
	} else if( work->upwind_pan+PAN_INTERP < pan ) {
		work->upwind_pan += PAN_INTERP ;
	} else {
		work->upwind_pan = pan ;
	}
	GM_MixConvFader( UPWIND, work->upwind_pan, work->wind_vol/c_box ) ;

//printf(" up pan[%x] \n", work->upwind_pan ) ;


	/* 主観耳風音 */
	dir = (G_wind_rot.vy+2048 - (cam->rotate.vy - 1024)) & 4095 ;

	if ( GM_PlayerStatus & (PLAYER_INTRUDE|PLAYER_WATCH) ) {
		vol = (int)(16.0f * f) ;
		if( vol > 16.0f ) vol = 16.0f ;
		if ( dir < 1024 ) {
			v = __RcosF( dir ) * 16.0f  ;
			vol += 0x2f - (int)v ; 
		} else if ( dir < 2048 ) {
			v = __RcosF( dir ) * 16.0f  ;
			vol += 0x2f + (int)v ; 
		} else {
			vol += 0x1f ; 
		}
	} else {
		vol = 0 ; 
	}

	if ( work->earwind_vol-VOL_INTERP > vol ) {
		work->earwind_vol -= VOL_INTERP ;
	} else if( work->earwind_vol+VOL_INTERP < vol ) {
		work->earwind_vol += VOL_INTERP ;
	} else {
		work->earwind_vol = vol ;
	}

	if( PL_GetPlayerWeapon( ) != WP_Mic ) {
		vol = 0 ; 
	}

	if ( work->micwind_vol-VOL_INTERP > vol ) {
		work->micwind_vol -= VOL_INTERP ;
	} else if( work->micwind_vol+VOL_INTERP < vol ) {
		work->micwind_vol += VOL_INTERP ;
	} else {
		work->micwind_vol = vol ;
	}
//printf("wind dir[%d] speed [%d/%d] vol[%x]\n", G_wind_rot.vy,G_wind_intense,G_wind_intense_max, work->earwind_vol ) ;

	if ( dir < 1024 ) {
		p =(int)(__RcosF( dir ) * 23.0f ) ;
		panr = 0x28 + p ;
		p =(int)(__RcosF( dir ) * 24.0f ) ;
		panl = 0x18 + p ;
	} else if ( dir < 2048 ) {
		p =(int)(__RcosF( dir ) * 24.0f ) ;
		panr = 0x28 + p ;
		p =(int)(__RcosF( dir ) * 24.0f ) ;
		panl = 0x18 + p ;
	} else if ( dir < 3072 ) {
		p =(int)(__RcosF( dir ) * 31.0f ) ;
		panr = 0x2f + p ;
		p =(int)(__RcosF( dir ) * 16.0f ) ;
		panl = 0x10 + p ;
	} else {
		p =(int)(__RcosF( dir ) * 16.0f ) ;
		panr = 0x2f + p ;
		p =(int)(__RcosF( dir ) * 32.0f ) ;
		panl = 0x10 + p ;
	}

	GM_MixConvFader( EARWIND_R, panr, work->earwind_vol/c_box ) ;
	GM_MixConvFader( EARWIND_L, panl, work->earwind_vol/c_box ) ;
	if ( work->stage == WEATHER_FAIN ) {
		/* マイクボーボー音 */
		GM_MixConvFader( MIC_R, panr, work->micwind_vol/c_box ) ;
		GM_MixConvFader( MIC_L, panl, work->micwind_vol/c_box ) ;
	}
}

static	void	Act( work )
Work		*work ;
{
	switch ( work->mode ) {
		case ALERT_MODE_SNEAK :
			if ( GM_AlertMode == ALERT_MODE_SNEAK ) {
				WindNoise( work ) ;
			} else {
//				GM_InitMixConvFader( ) ;
				work->mode = GM_AlertMode ;
			}
		break ;
		default :
			if ( GM_AlertMode == ALERT_MODE_SNEAK ) {
				work->mode = GM_AlertMode ;
			}
		break ;
	}
}

static	void	Die( work )
Work		*work ;
{
}

/*----------------------------------------------------------------*/
static	int	GetResources( work )
Work	*work ;
{

	work->stage = GCL_GetOptionValue( 's', 0 ) ;

	work->mode = ALERT_MODE_SNEAK ;
	work->upwind_pan = 0x20 ;
	work->downwind_pan = 0x20 ;
	work->earwind_l_pan = 0x20 ;
	work->earwind_r_pan = 0x20 ;

	work->wind_vol = 0x20 ;
	work->earwind_vol = 0x00 ;
	work->micwind_vol = 0x20 ;
	work->c_box_vol = 0x0 ;

	return 0 ;
}

void	*NewWindNoise( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &( work->actor ) ) ;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}


#if 0

#define HARRIER_DOPPLER	0x12	/* ハリヤー ドップラー */
#define 


static void HarrierDoppler( FVECTOR *pos )
{

	GM_BgmFaderEx( HARRIER_DOPPLER, pos, 1000, 10000 ) ;
}



＜ＢＧＭ呼びＳＥ、ボリュームオートメーションリスト＞
①トラック１６(0x10)「ハリアータービン」
最遠      ０（  ０％）   ＶＯＬ     ０（  ０％）
      １５０（５９％）              ０（  ０％）
      ２１０（８２％）          １００（３９％）
      ２３０（９０％）          ２５５（１００％）
最接近２５５（１００）          ２５５（１００％）

②トラック１７(0x11)「ハリアーホバリング」
最遠      ０（  ０％）   ＶＯＬ     ０（  ０％）
      ２１０（８２％）              ０（  ０％）
      ２３０（９０％）          ２５５（１００％）
最接近２５５（１００）          ２５５（１００％）

③トラック１８(0x12)「ハリアードップラー」
最遠      ０             ＶＯＬ     ０（  ０％）
      ２１０（８２％）          １５０（５９％）
      ２３０（９０％）          ２５５（１００％）
最接近２５５（１００）          １５０（５９％）
      
④トラック１９(0x13)「カサッカ羽音」
最遠      ０（  ０％）   ＶＯＬ    ０（  ０％）
      １５０（５９％）             ０（  ０％）
      ２１０（８２％）           ８０（３１％）
      ２３０（９０％）         ２５５（１００％）
最接近２５５（１００）         ２５５（１００％）

⑤トラック２０(0x14)「カサッカタービン」
最遠      ０（  ０％）    ＶＯＬ    ０（  ０％）
      １９０（７４％）              ０（  ０％）
      ２３０（９０％）          ２５５（１００％）
      ２５５（１００）          ２５５（１００％）

#endif
