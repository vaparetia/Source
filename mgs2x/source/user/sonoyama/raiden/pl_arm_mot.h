/*
   pl_arm_mot.h
   プレイヤー主観腕モーション定義

   2000/10/22	M.Sonoyama
   $Id: pl_arm_mot.h,v 1.1.1.3 2002/11/19 11:50:57 Yoshizawa1 Exp $
*/

/* モーションリスト */
enum {
	PAnon_idle = 0,
	PAnon_combo,
	PAm9_ready,
	PAm9_fire,
	PAm9_reload,
	PAm9_wall,
	PAm9_ready_cr,
	PAm9_fire_cr,
	PAm9_reload_cr,
	PAm9_wall_cr,
	/* 10 */
	PAgm_fire_start,
	PAgm_fire_min,
	PAgm_fire_max,
	PAgm_fire_5pose,
	PAgm_fire_start_g,
	PAgm_fire_min_g,
	PAgm_fire_max_g,
	PAgm_fire_5pose_g,
	PAm4_ready,
	PAm4_fire,
	/* 20 */
	PAm4_reload,
	PAm4_release,
	PAm4_wall,
	PAm4_ready_cr,
	PAm4_fire_cr,
	PAm4_reload_cr,
	PAm4_release_cr,
	PAm4_wall_cr,
	PArgb_ready,
	PArgb_fire,
	/* 30 */
	PArgb_reload,
	PArgb_wall,
	PArgb_ready_cr,
	PArgb_fire_cr,
	PArgb_reload_cr,
	PArgb_wall_cr,
	PAc4_wall,
	PAc4_floor,
	PAc4_floor_cr,
	PAclay_floor,
	PAspr_ready,
	/* 40 */
	PAspr_ready_cr,
	PAspr_fire,
	PAmic_ready,
	PAmic_ready_cr,
	PAblade_fire,
	PAspr_fire_empty,
	PAspr_fire_empty_cr,
	PAcombo_p,
	PAak_reload,
	PAak_reload_cr,
	PAspr_fire_cr,
	PAspr_wall,
	PAspr_wall_cr,
	PAhfb_combo,
	PAnkt_strike,
	PAnkd_combo,
	PAm4a_combo,
	MAX_PLARM_MOTIONS
} ;

#define	PA_NOACT	MAX_PLARM_MOTIONS

/*-------- 武器ごとに変更 --------*/

static	short	NONESet[] = {
	PAnon_idle, 
	PA_NOACT,
	PA_NOACT,
	PA_NOACT,
	PA_NOACT,
	PA_NOACT,
	PAnon_idle,
	PA_NOACT,
	PA_NOACT,
	PA_NOACT,
	PA_NOACT,
	PA_NOACT
} ;

static	short	M9Set[] = {
	PAnon_idle,
	PAm9_ready,
	PAm9_fire,
	PAm9_reload,
	PA_NOACT,
	PAm9_wall,
	PAnon_idle,
	PAm9_ready_cr,
	PAm9_fire_cr,
	PAm9_reload_cr,
	PA_NOACT,
	PAm9_wall_cr,
} ;

static	short	USPSet[] = {
	PAnon_idle,
	PAm9_ready,
	PAm9_fire,
	PAm9_reload,
	PA_NOACT,
	PAm9_wall,
	PAnon_idle,
	PAm9_ready_cr,
	PAm9_fire_cr,
	PAm9_reload_cr,
	PA_NOACT,
	PAm9_wall_cr,
} ;

static	short	GrenadeSet[] = {
	PAnon_idle,
	PAgm_fire_start,
	PAgm_fire_min,
	PAgm_fire_max,
	PAgm_fire_5pose,
	PAgm_fire_start_g,
	PAgm_fire_min_g,
	PAgm_fire_max_g,
	PAgm_fire_5pose_g
} ;

static	short	M4Set[] = {
	PAnon_idle,
	PAm4_ready,
	PAm4_fire,
	PAm4_reload,
	PAm4_release,
	PAm4_wall,
	PAnon_idle,
	PAm4_ready_cr,
	PAm4_fire_cr,
	PAm4_reload_cr,
	PAm4_release_cr,
	PAm4_wall_cr,
} ;

static	short	AKSSet[] = {
	PAnon_idle,
	PAm4_ready,
	PAm4_fire,
	PAak_reload,
	PAm4_release,
	PAm4_wall,
	PAnon_idle,
	PAm4_ready_cr,
	PAm4_fire_cr,
	PAak_reload_cr,
	PAm4_release_cr,
	PAm4_wall_cr,
} ;

static	short	SPRAYSet[] = {
	PAnon_idle,	
	PAspr_ready,
	PAspr_fire,
	PAspr_fire_empty,
	PA_NOACT,
	PAspr_wall,
	PAnon_idle,	
	PAspr_ready_cr,
	PAspr_fire_cr,
	PAspr_fire_empty_cr,
	PA_NOACT,
	PAspr_wall_cr
} ;

static	short	MICSet[] = {
	PAnon_idle,	
	PAmic_ready,
	PA_NOACT,
	PA_NOACT,
	PA_NOACT,
	PAspr_wall,
	PA_NOACT,
	PAmic_ready_cr,
	PA_NOACT,
	PA_NOACT,
	PA_NOACT,
	PAspr_wall_cr
} ;

static	short	RGBSet[] = {
	PAnon_idle,
	PArgb_ready,
	PArgb_fire,
	PArgb_reload,
	PA_NOACT,
	PAm4_wall,
	PAnon_idle,
	PArgb_ready_cr,
	PArgb_fire_cr,
	PArgb_reload_cr,
	PA_NOACT,
	PAm4_wall_cr,
} ;

#define	SOCOMSet		M9Set
#define	FAMASSet		M4Set
#define	STINGERSet		M9Set
#define	NIKITASet		M9Set

static	short	*ChangeSets[] = {
	NONESet, M9Set, USPSet, SOCOMSet, NONESet, 
	RGBSet, NIKITASet, STINGERSet, NONESet, NONESet, 
	GrenadeSet, GrenadeSet, MICSet, NONESet, SPRAYSet, 
	AKSSet, GrenadeSet, GrenadeSet,	M4Set, NONESet, 
	MICSet, NONESet
} ;

/*-------- 共通 --------*/

static	short	SharedSet[] = {
	PAnon_combo,
	PAhfb_combo,
	PAnkt_strike,
	PAnkd_combo,
	PAm4a_combo
} ;

typedef	struct	{
	short	*change ;
	short	*attack ;
	short	*shared ;
} ARM_MOTION_SET ;





