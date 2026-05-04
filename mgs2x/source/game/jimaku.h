/*
	字幕表示用関数

	$Id: jimaku.h,v 1.1.1.3 2002/11/19 11:41:52 Yoshizawa1 Exp $
*/

void *NewJimaku( int name, int map );
void GM_JimakuSetZoom( int rate );
void GM_JimakuSetPosY( int posy );
void GM_JimakuHide( void );
void GM_JimakuShow( int stream_id, char *mes );
void *GM_JimakuDaemonStart( void );
int GM_JimakuGetStreamID( void );
void GM_JimakuDisable( void );
