
#define WIDTH   600.0f
#define LENGTH 1200.0f
#define LENGTH_OF_CUBE 1341.64078f
//#define ANGLE_OF_CUBE  (int)(63.4368f*4096/360)
#define ANGLE_OF_CUBE  (int)((26.5658f+0.0f)*4096/360)


#define	RAIDEN_NAME	(4595192) /* GV_StrCode( "raiden" ) */
//#define	MOTION_NAME	GV_StrCode( "mg2ene" )
#define MOTION_NAME     (4595192)  /* GV_StrCode( "raiden" ) */
#define	MODEL_NAME	31645/*GV_StrCode( "out_cold" )*/
#define	OBJECT_FLAG	(DG_FLAG_SHADE | DG_FLAG_FINISHCALC)
#define	ONE_CLOCK	(300 / 60)

#define	DEKU_NAME	GV_StrCode( "マン" ) 

/* 重力 */
#define	GRAVITY		(96.0F)


typedef	struct	_Work {
    GV_ACT		actor ;
    CONTROL		control ;
    OBJECT		body ;
    FMATRIX		lights[ 2 ] ;

    TARGET		def ;
    TARGET		hzx[4] ;
    TARGET              hit[2] ;

    int			motion ;
    int			flag ;
    int			address ;
    int			data2 ;

    FVECTOR mov ;
    int     turn ;
    int pad[3] ;
} Work ;

struct seg_ext_t
{
    HZX_SEG *seg ;
    float cntr_x ;
    float cntr_z ;

    float asn ;
    float acs ;

    float leng ;
    int   ang ;

    FVECTOR step ; /* new position */
    int     turn ; /* new turn     */
    int     det  ;
} ;










enum {
    naked_pose,
    naked_run,
    gun_pose,
    gun_ready_pose,
    gun_fire,
    naked_squat,
    naked_hofuku_start,
    naked_hofuku_pose,
    naked_hofuku_front_move,
    naked_hofuku_back_move,
    naked_pose_fromhofuku,
    naked_wall_pose,
    naked_wall_squat_pose,
    naked_wall_move_l,
    naked_wall_move_r,
    naked_damage05_head,
    naked_damage06_body,
    naked_damage_dead,
    naked_wall_knock_L,
    naked_wall_knock_R,
    raiden_dam_blow1,
    raiden_dam_blow2,
    raiden_dam_blow3,
    test_walk,
    test_run,
    test_dash,
    hang_pose,
    hang_tie,
    hang_kill,
    MAX_MOTIONS
} ;
