/*

	preview.h
	デザイナープレビュー環境用：カメラ
	1999/07/07 S.Okajima
	$Id: preview.h,v 1.1.1.3 2002/11/19 11:43:31 Yoshizawa1 Exp $

*/


#ifndef _preview_h_
#define _preview_h_


#include	"preview_def.h"


typedef	struct	{
	GV_ACT		actor ;

	SAVE_AREA	save;

	void		*human_work;
	void		*object_work;
	void		*fobj_work;

	void		*camera_work;
	void		*light_work;
	void		*particle_work;
	void		*vib_work;
	void		*sky_work;
        void            *layout_work;

	int			first_in_flag;
	int			key_count;
	int			pad;
	GM_CameraSet		*camera ;

	DG_PRIM		*prim ;
	DG_TEX		*tex ;

	DG_OBJS		*yajirushi_objs ;

	unsigned char	data_filename[MAX_FILE_NUM][FILE_NAME_LEN];
	unsigned char	data_objects_name[MAX_FILE_NUM][FILE_NAME_LEN];
	int			max_num_files;

	int			tri_top;
	int			tri_num;

	int			kms_top;
	int			kms_num;

	int			cv2_top;
	int			cv2_num;

	int			tex_width;
	int			tex_height;

	int			human_num;

	int			motion_num_old;
	int			motion_interp_old;
	int			motion_stop_flag;

	int			dummy[256];
	int			objects_num;
	int			dummy2[256];
	DG_DEF		*objects_def[ MAX_OBJECTS ];
	DG_OBJS		*objects[ MAX_OBJECTS ] ;
	FMATRIX		objects_lights[MAX_OBJECTS][ 2 ] ;

	int			font_color;

#if 0
	/* 追加  99/12/10  Ken Kano */
	void		*objchange[3];
	int		objchange_index;
	DG_DEF		*objchange_def[MAX_OBJCHANGE_PATTERN];
	int		objchange_def_size;

	void		*va;
	char		*va_filename[MAX_VA_PATTERN];
	int		va_filename_size;
#endif

	int			cam_disp_flag;

	FVECTOR		forVRSlit;

} Work ;


#endif
