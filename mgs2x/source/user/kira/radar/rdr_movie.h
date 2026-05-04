#ifndef _rdr_movie_h_
#define _rdr_movie_h_

#include "sprite_2d.h"

#define RDR_STREAM_CH  GM_STREAM_CHANNEL_1
/*
 * レーダーエリア動画サイズ
 */
/* サイズはムービーのピクセル数 */
#define RDR_DEF_CHANL    4
#define RDR_DEF_MOVIE_W  128.0F
#define RDR_DEF_MOVIE_H  128.0F

/* 表示領域サイズ */
#define RDR_DEF_X0      ((float)RADAR_WINDOW_X0)
#define RDR_DEF_Y0      (((float)RADAR_WINDOW_Y0 * 384.0F)/(float)DRAW_HEIGHT)
#define RDR_DEF_WIDTH   ((float)RADAR_WINDOW_W)
#define RDR_DEF_HEIGHT  (((float)RADAR_WINDOW_H * 384.0F) / (float)DRAW_HEIGHT)



#define RDR_OPT_data   'd'
#define RDR_OPT_chanl  'c'
#define RDR_OPT_rgba   'r'
#define RDR_OPT_pos    'p'
#define RDR_OPT_size   's'
#define RDR_OPT_movie  'm'
#define RDR_OPT_finaly 'f'

void   RDR_MovieForceBreak(void);
void * NewRadarMovie(int name, int where);


#endif /* _rdr_movie_h_ */

