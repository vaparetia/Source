/*
   arm_motion.h 
   ¥é¥¤¥Ç¥ó¡¿¼ç´ÑÏÓ¥â¡¼¥·¥ç¥óÄêµÁ

   1999/11/08 M.Sonoyama
   $Id: arm_motion.h,v 1.1.1.3 2002/11/19 11:50:53 Yoshizawa1 Exp $
*/

/* ¥â¡¼¥·¥ç¥ó¥ê¥¹¥È */
enum {
    dummy,
    dummy2,
    m92_fire,
    m92_ready,
    m92_ready2,
    m92_reload,
    m92_stand,
    usp_fire,
    usp_ready,
    usp_ready2,
    usp_reload,
    usp_stand,
    none_stand,
    m92_crouch_ready,
    m92_crouch_fire,
    m92_crouch_reload,
    fms_stand,
    fms_ready,
    fms_fire,
    fms_fire_end,
    fms_reload,
	fms_wall,
	non_combo_p,
	non_combo,
    MAX_ARM_MOTIONS
};

/*-------- Éð´ïËè¤ËÊÑ¹¹ --------*/

/* ÁÇ¼ê */
static	short	NoneSet[] = {
    none_stand, none_stand, none_stand, none_stand, AM_NOACT, AM_NOACT, 
    none_stand, none_stand, none_stand, none_stand, AM_NOACT,
} ;

/* £Í£¹£² */
static	short	M92Set[] = {
    m92_stand, m92_ready, m92_fire, m92_reload, AM_NOACT, AM_NOACT, 
    none_stand, m92_crouch_ready, m92_crouch_fire, m92_crouch_reload, AM_NOACT,
} ;

/* £Õ£Ó£Ð */
static	short	USPSet[] = {
    m92_stand, m92_ready, m92_fire, m92_reload, AM_NOACT, AM_NOACT, 
    none_stand, m92_crouch_ready, m92_crouch_fire, m92_crouch_reload, AM_NOACT
} ;

/* £Ó£Ï£Ã£Ï£Í */
static	short	SocomSet[] = {
    m92_stand, m92_ready, m92_fire, m92_reload, AM_NOACT, AM_NOACT, 
    none_stand, m92_crouch_ready, m92_crouch_fire, m92_crouch_reload, AM_NOACT
} ;

/* £Æ£Á£Í£Á£Ó */
static	short	FamasSet[] = {
    fms_stand, fms_ready, fms_fire, fms_reload, fms_fire_end, fms_wall,
    fms_stand, m92_crouch_ready, m92_crouch_fire, m92_crouch_reload, fms_fire_end
} ;

/* £Ó£ð£ð£±£Í */
static	short	SppSet[] = {
    m92_stand, m92_ready, m92_fire, m92_reload, AM_NOACT, AM_NOACT, 
    none_stand, m92_crouch_ready, m92_crouch_fire, m92_crouch_reload, AM_NOACT
} ;

/* £Ó£Ð£Ò£Á£Ù */
static	short	SpraySet[] = {
    none_stand, m92_ready, none_stand, none_stand, AM_NOACT, AM_NOACT, 
    none_stand, m92_crouch_ready, none_stand, none_stand, AM_NOACT,
} ;

// 2000.7/11 added by S.Okajima.
#define	STGSet		USPSet
#define	NikitaSet	USPSet
#define	RGBSet		USPSet

static	short	*ChangeSets[] = {
    NoneSet, M92Set, USPSet, SocomSet, FamasSet,
    NoneSet, SppSet, RGBSet, NikitaSet, STGSet,
    NoneSet, NoneSet, NoneSet, NoneSet, NoneSet,
    NoneSet, NoneSet, NoneSet, SpraySet, SpraySet,
    SpraySet, NoneSet, NoneSet, NoneSet, NoneSet
} ;

/*-------- ¶¦ÄÌ --------*/

static	short	SharedSet[] = {
    non_combo_p, non_combo
} ;

typedef	struct {
    short	*change ;
    short	*attack ;
    short	*shared ;
} ARM_MOTION_SET ;

