/*
  MPEG表示部分ヘッダ
  
  2002/07/11 M.Kobayashi
  $Id: mpegstr.h,v 1.3 2002/11/23 12:46:54 Yoshizawa1 Exp $
  
 */

#ifndef __MPEGSTR_H__
#define __MPEGSTR_H__

// Movie をリクエストする 最後にリクエストしたものが有効
// 		top_pos        : ストリームデータの位置
//		width,height   : ムービーの幅、高さ
//		x,y	           : ムービーの表示位置	
//		repeat         : 真なら繰返し再生する			  
//		pad_cancel     : パッドでキャンセル可能ならそのボタンを入れる		
//		disp_width
//		disp_height    : 表示幅・高さ
void RequestMovieStream( int top_pos, int width, int height,
						 int x, int y, 
						 int repeat, int pad_cancel, 
						 int disp_width, int disp_height );

// Request 中のものを含めてムービーをキャンセルする
void CancelMovieStream( void );

int GetMovieStreamTime();
void MovieStream_KillCreditsMoviePlayer();

#ifdef KP_XBOX
typedef struct {
	int start;			// 開始絶対時間(1/300)
	int width;
	int height;
	int x;
	int y;
} MPEG_MV_INFO;

extern void *NewMpegPssMovieStrProg( MPEG_MV_INFO *pMvInfo,
									 int pos, int proc, int repeat, u_short cancel );
#endif //KP_XBOX

#endif //__MPEGSTR_H__
