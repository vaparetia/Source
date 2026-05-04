/*
	sd_ee.h
		サウンドプログラムEE側インターフェース
		ヘッダファイル ( EE側のプログラムから参照 )

	1999/11/30 K.Uehara
	$Id: sd_ee.h,v 1.3 1999/12/13 12:00:27 usr00339 Exp $
*/

#ifndef	__SD_EE__H__
#define __SD_EE__H__

#ifdef __cplusplus
extern "C" {
#endif

int sd_init( void );
void sd_set_cli( int code );
int *sd_status( void );

#ifdef __cplusplus
}
#endif

#endif /* __SD_EE__H__ */


