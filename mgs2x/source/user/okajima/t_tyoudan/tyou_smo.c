/*
	跳弾時のスモーク :   (未使用)
	２Dアニメスクリプト
	1999/08/26  H.Tanaka
	2000/10/18 S.Okajima
	$Id: tyou_smo.c,v 1.1.1.3 2002/11/19 11:47:47 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
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
#include	"mgs_type.h"
#include	"../anime/anime.h"

/*跳弾時の煙*/
static ANIMATION	anm_tyoudansmoke_form = {
	16238439 /*"smoke_msk"*/,		/* テクスチャ番号 */
	8,4,			/* テクスチャ分割数 */
	30,				/* アニメパターン数 */
	10,				/* 表示優先 */
	400, 400,			/* サイズ */
	16,			/* 輝度値 */
	1,                      /* 減算半透明 */
	0,
	NULL,			/* スクリプト前処理 */
	(char[]){0x00,0x27,1,0x00,0x05,0x01,0xFE,0x0A,0xFF,0x38,0xFF,0x38,0x0B,0x00,0x05,0x08,0x02,0x02,0x02,0x01,0xFF,0x02,0x00,0x01,0x0C,0x0B,0x00,0x19,0x01,0xFF,0x0A,0x00,0x14,0x00,0x14,0x08,0xFF,0xFF,0xFF,0x02,0x00,0x01,0x0C,0x0E}
} ;

    
void  *AN_Tyoudan_Smoke(world)
FMATRIX *world ;
{
    ANIMATION   *anm ;
    PRESCRIPT   pre ;
    FVECTOR     *pos,shift ;
    FMATRIX     mat ;
  
    pos = &(pre.pos) ;
   
    shift.vx = 0.0F ;
    shift.vy = -20.0F ;
    shift.vz = 0.0F ;
    DG_SetPos(world) ;
    DG_MovePos(&shift) ;
    DG_GetPos(&mat);
    pos->vx = mat.m[3][0];
    pos->vy = mat.m[3][1];
    pos->vz = mat.m[3][2];
    pos->vw = 1.0F ;

    pre.speed.vx = 0.0F;
    pre.speed.vy = 0.0F;
    pre.speed.vz = 0.0F;

    pre.scr_num = 0;
    anm = &anm_tyoudansmoke_form ;
    anm->pre_script = &pre ;
    NewAnime(NULL, 0, anm);
}
