/*
  MPEG表示部分ヘッダ
  
  2002/07/11 M.Kobayashi
  $Id: mpegstrx.h,v 1.1.1.1 2002/08/02 02:40:23 yoshizawa1 Exp $
  
 */

typedef struct {
	int start;		// 開始絶対時間(1/300)
	int width;
	int height;
	int x;
	int y;
} MPEG_MV_INFO;

extern void *NewMpegPssMovieStrProg( MPEG_MV_INFO *pMvInfo,
									 int pos, int proc, int repeat, u_short cancel );
