//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	blood.c
	２Ｄアニメスクリプト：血
	1999/07/07 S.Okajima
	$Id: blood2.c,v 1.1.1.3 2002/11/19 11:48:28 Yoshizawa1 Exp $

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

static char anm_blood_mist_form_script[] = {0x00,0x32,1,0x00,0x05,0x02,0x00,0x01,0x0B,0x00,0x01,0x05,0x00,0x00,0x00,0x0A,0x00,0x00,0x0A,0x01,0x90,0x01,0x90,0x08,0xFE,0xFE,0xFE,0x02,0x00,0x01,0x0C,0x0B,0x00,0x02,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x0A,0xFF,0xCE,0xFF,0xCE,0x08,0xFB,0xFB,0xFB,0x02,0x00,0x01,0x0C,0x0E} ;
static ANIMATION        anm_blood_mist_form = {
    4445730,//GV_StrCode( "blood_2_msk" ),
    1,1,
    1,
    500,
    200,200,
    20,
    0,                           /* 加算半透明 */
    0,
    NULL,
    anm_blood_mist_form_script
} ;

void    *AN_Blood_Mist(pos)
FVECTOR    *pos ;
{
    ANIMATION          *anm ;
    PRESCRIPT          pre  ;
    FVECTOR            *speed ;

	if( GM_Configuration & GM_CONFIG_BLOOD_OFF ) return NULL;

    pre.pos.vx = pos->vx ;
    pre.pos.vy = pos->vy ;
    pre.pos.vz = pos->vz ;
    speed = &(pre.speed);
    speed->vx  = 0.0F;
    speed->vy  = 0.0F;
    speed->vz  = 0.0F;
    pre.scr_num = 0 ;
    pre.s_anim = 0 ;
    anm = &anm_blood_mist_form ;
    anm->pre_script = &pre ;
    return( NewAnime( NULL, 0, anm )) ;
}


static char anm_blood_mist_form_fog_script[] = {0x00,0x32,1,0x00,0x05,0x02,0x00,0x01,0x0B,0x00,0x01,0x05,0x00,0x00,0x00,0x0A,0x00,0x00,0x0A,0x01,0x90,0x01,0x90,0x08,0xFE,0xFE,0xFE,0x02,0x00,0x01,0x0C,0x0B,0x00,0x02,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x0A,0xFF,0xCE,0xFF,0xCE,0x08,0xFB,0xFB,0xFB,0x02,0x00,0x01,0x0C,0x0E} ;

static ANIMATION        anm_blood_mist_form_fog = {
    4445730,//GV_StrCode( "blood_2_msk" ),
    1,1,
    1,
    500,
    200,200,
    20,
    AN_PRIMTYPE_FOG,      /* 加算半透明 */
    0,
    NULL,
    anm_blood_mist_form_fog_script
} ;

void    *AN_Blood_Mist_Fog(pos)
FVECTOR    *pos ;
{
    ANIMATION          *anm ;
    PRESCRIPT          pre  ;
    FVECTOR            *speed ;

	if( GM_Configuration & GM_CONFIG_BLOOD_OFF ) return NULL;

    pre.pos.vx = pos->vx ;
    pre.pos.vy = pos->vy ;
    pre.pos.vz = pos->vz ;
    speed = &(pre.speed);
    speed->vx  = 0.0F;
    speed->vy  = 0.0F;
    speed->vz  = 0.0F;
    pre.scr_num = 0 ;
    pre.s_anim = 0 ;
    anm = &anm_blood_mist_form_fog ;
    anm->pre_script = &pre ;
    return( NewAnime( NULL, 0, anm )) ;
}

    
