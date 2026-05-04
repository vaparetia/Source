//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	blstsgl.c
	２Ｄアニメスクリプト：爆発
	1999/07/07 S.Okajima
	$Id: blastsgl.c,v 1.1.1.3 2002/11/19 11:48:28 Yoshizawa1 Exp $

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

#include	"libutl.h"
#include	"mgs_type.h"
#include	"anime.h"

static char anm_blast_single_form_script[] = {0x00,0x3B,1,0x00,0x05,0x01,0x00,0x02,0x00,0x01,0x0B,0x00,0x02,0x01,0xFF,0x0A,0x04,0xB0,0x04,0xB0,0x02,0x00,0x01,0x0C,0x0B,0x00,0x06,0x01,0xFF,0x02,0x00,0x01,0x0C,0x0B,0x00,0x07,0x01,0xFF,0x0A,0x00,0x64,0x00,0x64,0x08,0xF8,0xF8,0xF8,0x02,0x00,0x01,0x0A,0x00,0x50,0x00,0x50,0x08,0xF8,0xF8,0xF8,0x02,0x00,0x01,0x0C,0x0E};

static ANIMATION	anm_blast_single_form = {
	4044236,//GV_StrCode( "bomb2_fl" ),		/* テクスチャ番号 */
	4,4,			/* テクスチャ分割数 */
	16,				/* アニメパターン数 */
	2000,				/* 表示優先 */
	1000, 1000,		/* サイズ */
	128,			/* 輝度値 */
	0,                      /* 加算半透明 */
	0,
	NULL,			/* スクリプト前処理 */
   anm_blast_single_form_script
} ;


static char anm_blast_single_form2_script[] = {0x00,0x2A,1,0x00,0x05,0x01,0xFE,0x02,0x00,0x01,0x0B,0x00,0x10,0x01,0xFF,0x0A,0x00,0x14,0x00,0x14,0x08,0x04,0x04,0x04,0x02,0x00,0x01,0x0C,0x0B,0x00,0x10,0x01,0xFF,0x0A,0x00,0x14,0x00,0x14,0x08,0xFC,0xFC,0xFC,0x02,0x00,0x01,0x0C,0x0E};

static ANIMATION	anm_blast_single_form2 = {
	0,//GV_StrCode( "bakuha06_msk" ),		/* テクスチャ番号 */
	4,4,			/* テクスチャ分割数 */
	16,				/* アニメパターン数 */
	2000,				/* 表示優先 */
	100, 100,		/* サイズ */
	8,			/* 輝度値 */
	0,                      /* 加算半透明 */
	0,
	NULL,			/* スクリプト前処理 */
   anm_blast_single_form2_script
} ;

void	*AN_Blast_Single( FVECTOR *pos )
{
	ANIMATION		*anm ;
	PRESCRIPT		pre;

	pre.pos = *pos ;
	pre.speed.vx = 0.0f ;
	pre.speed.vy = 0.0f ;
	pre.speed.vz = 0.0f ;

	pre.s_anim = 0 ;

	anm = &anm_blast_single_form ;
	pre.scr_num = 0 ;

	anm->pre_script = &pre ;

	return (NewAnime( NULL, 0, anm )) ;
}

void	*AN_Blast_Loop( FVECTOR *pos )
{
	ANIMATION		*anm ;
	PRESCRIPT		pre;

	pre.pos = *pos ;
	pre.speed.vx = 0.0f ;
	pre.speed.vy = 0.0f ;
	pre.speed.vz = 0.0f ;

	pre.s_anim = 0 ;

	anm = &anm_blast_single_form2 ;
//	anm = &anm_blast_single_form ;
	pre.scr_num = 0 ;

	anm->pre_script = &pre ;

	return (NewAnime( NULL, 0, anm )) ;
}

