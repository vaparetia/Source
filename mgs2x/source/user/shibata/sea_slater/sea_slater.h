#ifndef SEA_SLATER_H
#define SEA_SLATER_H

#define		N_MAX_CHECK			(4)
#define		N_MAX_BULLET_POS	(16)
#define		N_MAX_BLAST_POS		(16)
#define		BLAST_WAIT_TIME		(120)
#define		N_MAX_REFUGE		(8)

typedef	struct {
	GV_ACT_EX		actor;
	TARGET			def_trgt;
	int				name;
	int				map;
	
	short			act_flags;
	short			main_flags;

	short			n_check;
	short			n_foot;
	short			n_bullet;
	short			n_blast;
	short			pre_n_bullet;
	short			n_refuge;

	FVECTOR			center;
	FVECTOR			wide;
	SVECTOR			rot;

	int				check_name[N_MAX_CHECK];
	short			flags[N_MAX_CHECK];

	float			pre_height[N_MAX_CHECK][2];
	float			*level[N_MAX_CHECK];
	OBJECT			*check_body[N_MAX_CHECK];
	
//	FVECTOR			koshi[N_MAX_CHECK];
	FVECTOR			check_diff[N_MAX_CHECK];
	FVECTOR			check_pos[N_MAX_CHECK];

	FVECTOR			joint_pos[N_MAX_CHECK][4];
	FVECTOR			joint_diff[N_MAX_CHECK][4];
	
	FVECTOR			foot_pos[N_MAX_CHECK*2];	
	FVECTOR			bullet_list[N_MAX_BULLET_POS];
	FVECTOR			blast_list[N_MAX_BLAST_POS];
	short			blast_timer[N_MAX_BLAST_POS];

	FVECTOR			refuge_center[N_MAX_REFUGE][2];
	FVECTOR			refuge_wide[N_MAX_REFUGE][2];

	FMATRIX			root;
	FMATRIX			inv_root;
} AllWork;

extern AllWork *AllSlater_CurrentWork;

extern float GetAllCheckNearPosLen( FVECTOR *out_pos, FVECTOR *out_vec, FVECTOR *in, AllWork* );
extern void AllCheckSltrBlast( FVECTOR *sltr, FVECTOR *wide, AllWork *work, int n_ssltr );

#endif
