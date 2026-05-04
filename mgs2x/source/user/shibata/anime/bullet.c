//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	bullet.c
	２Ｄアニメスクリプト：マズルフラッシュ
	1999/07/07 S.Okajima
	$Id: bullet.c,v 1.1.1.3 2002/11/19 11:48:28 Yoshizawa1 Exp $

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <libvu0.h>
#endif
#include "libutl.h"

#include	"mgs_type.h"
#include	"anime.h"

static char anm_gunflash_form_script[] = {0x00,0x04,1,0x00,0x05,0x02,0x00,0x01,0x0E};

static ANIMATION	anm_gunflash_form = {
	11767764,//GV_StrCode( "fa_fl10_msk" ),		/* テクスチャ番号 */
	2,2,			/* テクスチャ分割数 */
	3,				/* アニメパターン数 */
	300,				/* 表示優先 */
	600, 600,			/* サイズ */
	100,			/* 輝度値 */
	0,                      /* 加算半透明 */
	0,
	NULL,			/* スクリプト前処理 */
	anm_gunflash_form_script
} ;

void	*AN_GunFlash( FMATRIX *world )
{
	ANIMATION		*anm ;
	PRESCRIPT		pre;
	int				r ;

	pre.pos.vx = 5.0f ;
	pre.pos.vy = -500.0f ;
	pre.pos.vz = 80.0f ;

	pre.speed.vx = 0.0f ;
	pre.speed.vy = 0.0f ;
	pre.speed.vz = 0.0f ;
	pre.scr_num = 0 ;
	pre.s_anim = 0 ;

	r = BP_PS2_rand() % 16 ;

	if( r > 4 ){
		pre.s_anim = 2 ;
	}else if( r > 0 ){
		pre.s_anim = 1 ;
	}else{
		pre.s_anim = 0 ;
	}

	anm = &anm_gunflash_form ;
	anm->pre_script = &pre ;

	return (NewAnime( world, 0, anm )) ;
}

