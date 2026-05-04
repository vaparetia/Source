/********************************************************************************/
/*	structure      								*/
/********************************************************************************/

#ifndef __def_dev_ubcamera_h__
#define __def_dev_ubcamera_h__

#define SetRGBA(_r, _g, _b, _a, _p)\
{					\
     (_p)->vx = (_r);	\
     (_p)->vy = (_g);	\
     (_p)->vz = (_b);	\
     (_p)->vw = (_a);	\
}

typedef struct {
    GV_ACT_EX	actor ;
    OBJECT		body ;
    CONTROL		ctrl ;
    FMATRIX		lights[2] ;
    FVECTOR		camera_pos;	/*カメラ座標*/
    SVECTOR		rot;		/*土台方向*/
    SVECTOR		hrot;
    int			map ;
    int			name ;
    int			alert_time;
    
    /*****ターゲット*****/
    TARGET		h_trg;
    TARGET		lens_trg;
    /*****ボンボリ用*****/
    FVECTOR	        rgba ;
    int		        b_mode ;
    FVECTOR		b_pos ;
    void	        *b_work ;
    int			mode;
    int			timer;
    u_short		n_msg;
    GV_MSG		*msg;
}Work;

static Work work;

#endif
