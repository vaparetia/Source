//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	testeye.c
	２Ｄアニメスクリプト：座標明示光点
	1999/07/07 S.Okajima
	$Id: testeye.c,v 1.1.1.3 2002/11/19 11:48:29 Yoshizawa1 Exp $

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

static char anm_test_eye_form_script[] = {0x00,0x2F,4,0x00,0x0B,0x00,0x13,0x00,0x20,0x00,0x2D,0x0B,0x00,0x08,0x02,0x00,0x01,0x0C,0x0E,0x0A,0xFF,0xCE,0xFF,0xCE,0x0B,0x00,0x08,0x02,0x00,0x01,0x0C,0x0E,0x0A,0xFF,0x9C,0xFF,0x9C,0x0B,0x00,0x08,0x02,0x00,0x01,0x0C,0x0E,0x0A,0xFF,0x6A,0xFF,0x6A,0x0B,0x00,0x08,0x02,0x00,0x01,0x0C,0x0E};
static ANIMATION	anm_test_eye_form = {
	6715088,//GV_StrCode( "rcm_l_msk" ),		/* テクスチャ番号 */
	1,1,			/* テクスチャ分割数 */
	1,				/* アニメパターン数 */
	230,				/* 表示優先 */
	300, 300,			/* サイズ */
	128,			/* 輝度値 */
	0,                      /* 加算半透明 */
	0,
	NULL,			/* スクリプト前処理 */
	anm_test_eye_form_script
} ;

static char anm_test_eye2_form_script[] = {0x00,0x08,1,0x00,0x05,0x0B,0x00,0x01,0x02,0x00,0x01,0x0C,0x0E};
ANIMATION	anm_test_eye2_form = {
	6715088,//GV_StrCode( "rcm_l_msk" ),		/* テクスチャ番号 */
	1,1,			/* テクスチャ分割数 */
	1,				/* アニメパターン数 */
	500,				/* 表示優先 */
	20, 20,			/* サイズ */
	128,			/* 輝度値 */
	0,                      /* 加算半透明 */
	0,
	NULL,			/* スクリプト前処理 */
	anm_test_eye2_form_script
} ;



void	AN_Test_Eye( FVECTOR *mov )
{
	ANIMATION		*anm ;
	PRESCRIPT		pre;

	pre.pos = *mov ;

	pre.speed.vx = 0.0f ;
	pre.speed.vy = 0.0f ;
	pre.speed.vz = 0.0f ;

	pre.scr_num = 1 ;
	pre.s_anim = 0 ;

	anm = &anm_test_eye_form ;
	anm->pre_script = &pre ;
	NewAnime( NULL, 0, anm ) ;
}

void	AN_Test_Eye2( FVECTOR *mov, int size )
{
	ANIMATION		*anm ;
	PRESCRIPT		pre;

	pre.pos = *mov;

	pre.speed.vx = 0.0f ;
	pre.speed.vy = 0.0f ;
	pre.speed.vz = 0.0f ;

	pre.scr_num = size ;
	pre.s_anim = 0 ;

	anm = &anm_test_eye_form ;
	anm->pre_script = &pre ;
	NewAnime( NULL, 0, anm ) ;
}

void	AN_Test_Eye3( FVECTOR *mov )
{
	ANIMATION		*anm ;
	PRESCRIPT		pre;

	pre.pos = *mov ;

	pre.speed.vx = 0.0f ;
	pre.speed.vy = 0.0f ;
	pre.speed.vz = 0.0f ;

	pre.scr_num = 0 ;
	pre.s_anim = 0 ;

	anm = &anm_test_eye2_form ;
	anm->pre_script = &pre ;
	NewAnime( NULL, 0, anm ) ;
}
