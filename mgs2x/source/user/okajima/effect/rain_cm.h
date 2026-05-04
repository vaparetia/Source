/*
	rain_cm.h
	カメラ画面に飛び散る雨などの共通ワーク
	2000/03/30 S.Okajima
	$Id: rain_cm.h,v 1.1.1.3 2002/11/19 11:47:09 Yoshizawa1 Exp $
*/

typedef	struct _rain_cm_Work	{
	GV_ACT_EX		actor ;
	int			name ;
	int			where ;

	/* カメラレンズ水滴に影響する光源 */
	int			lit_tex_pos_num;
	FVECTOR		lit_tex_pos[LIGHT_POS_NUM_MAX];

	DG_PRIM2	*prim[MAX_TEX_NUM * ADD_SUB] ;
	int			count[MAX_TEX_NUM * N_VERTS2 * N_PRIMS2];
	int			size[MAX_TEX_NUM * N_VERTS2 * N_PRIMS2];
	float		pos_x[MAX_TEX_NUM * N_VERTS2 * N_PRIMS2];
	float		pos_y[MAX_TEX_NUM * N_VERTS2 * N_PRIMS2];

	float		screen_near_x;
	float		screen_near_y;

	float		purupuru_x[PURUPURU_PAT];
	float		purupuru_y[PURUPURU_PAT];

	int			puru_count ;
	int			puru_flag ;

	int			time;

	int			presence_light;
	int			cycle;

	int			life;

	int			invisible_flag;

	int			always_flag;

	int			splash_on_flag;

} Work ;

