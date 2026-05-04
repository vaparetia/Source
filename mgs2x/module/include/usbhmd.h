/*
	usbkbd.h
		USBキーボード読み取り用モジュール(デバッグ用)
		2000/04/05 K.Uehara
	$Id: usbhmd.h,v 1.1 2002/05/13 02:18:22 usr01475 Exp $
*/

#define SIF_SYSREG_HMD	12		// SIFのSYSREGを使用
#define SIF_SYSREG_HMD2	13		// SIFのSYSREGを使用

typedef struct {
	signed short yaw;
	signed short pitch;
	signed short roll;
	unsigned short button;
} USBHMD_DATA;

// EEからの読み取り

extern inline void usbhmd_read( USBHMD_DATA *data )
{
   BP_TODO_BREAK;
#if 0 //BP_PS2
	int value[2];
	value[0] = sceSifGetSreg( SIF_SYSREG_HMD );
	value[1] = sceSifGetSreg( SIF_SYSREG_HMD2 );
	memcpy( data, value, sizeof( USBHMD_DATA ) );
#endif
}
