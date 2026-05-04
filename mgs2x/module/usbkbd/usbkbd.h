/*
	usbkbd.h
		USBキーボード読み取り用モジュール(デバッグ用)
		2000/04/05 K.Uehara
	$Id: usbkbd.h,v 1.2 2000/04/05 06:05:00 usr01475 Exp $
*/

#define SIF_SYSREG_USBKBD	9		// SIFのSYSREGを使用

typedef struct {
	unsigned char mask;
	unsigned char key[ 3 ];
} USBKBD_DATA;

// EEからの読み取り

extern inline void usbkbd_read( USBKBD_DATA *data )
{
	extern unsigned int sceSifGetSreg(int);

	*( int * )data = sceSifGetSreg( SIF_SYSREG_USBKBD );
}

/* ---------------------------------------------------------------------- */
/*
	マクロ
*/

#define KBD_A		0x04
#define KBD_B		0x05
#define KBD_C		0x06
#define KBD_D		0x07
#define KBD_E		0x08
#define KBD_F		0x09
#define KBD_G		0x0A
#define KBD_H		0x0B
#define KBD_I		0x0C
#define KBD_J		0x0D
#define KBD_K		0x0E
#define KBD_L		0x0F
#define KBD_M		0x10
#define KBD_N		0x11
#define KBD_O		0x12
#define KBD_P		0x13
#define KBD_Q		0x14
#define KBD_R		0x15
#define KBD_S		0x16
#define KBD_T		0x17
#define KBD_U		0x18
#define KBD_V		0x19
#define KBD_W		0x1A
#define KBD_X		0x1B
#define KBD_Y		0x1C
#define KBD_Z		0x1D
#define KBD_1		0x1E
#define KBD_2		0x1F
#define KBD_3		0x20
#define KBD_4		0x21
#define KBD_5		0x22
#define KBD_6		0x23
#define KBD_7		0x24
#define KBD_8		0x25
#define KBD_9		0x26
#define KBD_0		0x27
#define KBD_RET		0x28
#define KBD_ESC		0x29
#define KBD_BS		0x2A
#define KBD_TAB		0x2B
#define KBD_SPC		0x2C
#define KBD_ZENHAN	0x35

#define KBD_CPS		0x39
#define KBD_F1		0x3A
#define KBD_F2		0x3B
#define KBD_F3		0x3C
#define KBD_F4		0x3D
#define KBD_F5		0x3E
#define KBD_F6		0x3F
#define KBD_F7		0x40
#define KBD_F8		0x41
#define KBD_F9		0x42
#define KBD_F10		0x43
#define KBD_F11		0x44
#define KBD_F12		0x45

#define KBD_PRINT	0x46
#define KBD_SCLOCK	0x47
#define KBD_PAUSE	0x48
#define KBD_INSERT	0x49
#define KBD_HOME	0x4A
#define KBD_PAGEUP	0x4B
#define KBD_DELETE	0x4C
#define KBD_END		0x4D
#define KBD_PAGEDOWN	0x4E

#define KBD_RIGHT	0x4F
#define KBD_LEFT	0x50
#define KBD_DOWN	0x51
#define KBD_UP		0x52

#define KPD_NUMLOCK	0x53
#define KPD_SLASH	0x54
#define KPD_ASTA	0x55
#define KPD_MINUS	0x56
#define KPD_PLUS	0x57
#define KPD_ENTER	0x58
#define KPD_1		0x59
#define KPD_2		0x5A
#define KPD_3		0x5B
#define KPD_4		0x5C
#define KPD_5		0x5D
#define KPD_6		0x5E
#define KPD_7		0x5F
#define KPD_8		0x60
#define KPD_9		0x61
#define KPD_0		0x62
#define KPD_PERIOD	0x63

#define KBD_MASK_LCTRL	0x01
#define KBD_MASK_RCTRL	0x10
#define KBD_MASK_CTRL	0x11
#define KBD_MASK_LSHIFT	0x02
#define KBD_MASK_RSHIFT	0x20
#define KBD_MASK_SHIFT	0x22
#define KBD_MASK_LALT	0x04
#define KBD_MASK_RALT	0x40
#define KBD_MASK_ALT	0x44



