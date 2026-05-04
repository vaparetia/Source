/*--------------------------------------------------------------*/
/*	wpad_config_equ.h											*/
/*					入力コンフィグ関係テーブル					*/
/*						2002/11/09	Takaki Eiji					*/
/*--------------------------------------------------------------*/

	/*----------------------------------------------------------*/
	/*	X-Box Button											*/
	/*----------------------------------------------------------*/
#ifdef	GV_XBOX_BUTTON_EQU

GV_XBOX_BUTTON_EQU( U,	U )		//		(0x00001000)
GV_XBOX_BUTTON_EQU( D,	D )		//		(0x00004000)
GV_XBOX_BUTTON_EQU( L,	L )		//		(0x00008000)
GV_XBOX_BUTTON_EQU( R,	R )		//		(0x00002000)
GV_XBOX_BUTTON_EQU( A,	B )		//		(0x00000020)
GV_XBOX_BUTTON_EQU( B,	A )		//		(0x00000040)
GV_XBOX_BUTTON_EQU( X,	Y )		//		(0x00000010)
GV_XBOX_BUTTON_EQU( Y,	X )		//		(0x00000080)

GV_XBOX_BUTTON_EQU( L1,	L1 )	//		(0x00000004)
GV_XBOX_BUTTON_EQU( R1,	R1 )	//		(0x00000008)
GV_XBOX_BUTTON_EQU( L2,	L2 )	//		(0x00000001)
GV_XBOX_BUTTON_EQU( R2,	R2 )	//		(0x00000002)

GV_XBOX_BUTTON_EQU( STA,STA )	//		(0x00000800)
GV_XBOX_BUTTON_EQU( SEL,SEL )	//		(0x00000100)

GV_XBOX_BUTTON_EQU( AL,	AL )	//		(0x00000200)
GV_XBOX_BUTTON_EQU( AR,	AR )	//		(0x00000400)

GV_XBOX_BUTTON_EQU( EX1,EX1 )	//		(0x00010000) 特殊ボタン(トグル型主観)
GV_XBOX_BUTTON_EQU( EX2,EX2 )	//		(0x00020000) 特殊ボタン(ゆっくり押し)
GV_XBOX_BUTTON_EQU( EX3,EX3 )	//		(0x00040000) 特殊ボタン(弱押し)

GV_XBOX_BUTTON_EQU( RU,	RU )	//		(0x00100000) 右スティックのエミュレート用
GV_XBOX_BUTTON_EQU( RD,	RD )	//		(0x00400000) 右スティックのエミュレート用
GV_XBOX_BUTTON_EQU( RL,	RL )	//		(0x00800000) 右スティックのエミュレート用
GV_XBOX_BUTTON_EQU( RR,	RR )	//		(0x00200000) 右スティックのエミュレート用

#endif /* defined(GV_XBOX_BUTTON_EQU) */

#ifdef	GV_XBOX_PRESS_EQU

GV_XBOX_PRESS_EQU( R )
GV_XBOX_PRESS_EQU( L )
GV_XBOX_PRESS_EQU( U )
GV_XBOX_PRESS_EQU( D )
GV_XBOX_PRESS_EQU( X )
GV_XBOX_PRESS_EQU( A )
GV_XBOX_PRESS_EQU( B )
GV_XBOX_PRESS_EQU( Y )
GV_XBOX_PRESS_EQU( L1 )
GV_XBOX_PRESS_EQU( R1 )
GV_XBOX_PRESS_EQU( L2 )
GV_XBOX_PRESS_EQU( R2 )

#endif /* defined(GV_XBOX_PRESS_EQU) */

	/*----------------------------------------------------------*/
	/*	Keyboard												*/
	/*----------------------------------------------------------*/
#ifdef	GV_KBCFG_EQU

GV_KBCFG_EQU( ESCAPE )
GV_KBCFG_EQU( 1 ) //
GV_KBCFG_EQU( 2 ) //             0x03
GV_KBCFG_EQU( 3 ) //             0x04
GV_KBCFG_EQU( 4 ) //             0x05
GV_KBCFG_EQU( 5 ) //             0x06
GV_KBCFG_EQU( 6 ) //             0x07
GV_KBCFG_EQU( 7 ) //             0x08
GV_KBCFG_EQU( 8 ) //             0x09
GV_KBCFG_EQU( 9 ) //             0x0A
GV_KBCFG_EQU( 0 ) //             0x0B
GV_KBCFG_EQU( MINUS ) //         0x0C    /* - on main keyboard */
GV_KBCFG_EQU( EQUALS ) //        0x0D
GV_KBCFG_EQU( BACK ) //          0x0E    /* backspace */
GV_KBCFG_EQU( TAB ) //            0x0F
GV_KBCFG_EQU( Q ) //              0x10
GV_KBCFG_EQU( W ) //              0x11
GV_KBCFG_EQU( E ) //              0x12
GV_KBCFG_EQU( R ) //              0x13
GV_KBCFG_EQU( T ) //              0x14
GV_KBCFG_EQU( Y ) //              0x15
GV_KBCFG_EQU( U ) //              0x16
GV_KBCFG_EQU( I ) //              0x17
GV_KBCFG_EQU( O ) //              0x18
GV_KBCFG_EQU( P ) //              0x19
GV_KBCFG_EQU( LBRACKET ) //       0x1A
GV_KBCFG_EQU( RBRACKET ) //       0x1B
GV_KBCFG_EQU( RETURN ) //         0x1C    /* Enter on main keyboard */
GV_KBCFG_EQU( LCONTROL ) //       0x1D
GV_KBCFG_EQU( A ) //              0x1E
GV_KBCFG_EQU( S ) //              0x1F
GV_KBCFG_EQU( D ) //              0x20
GV_KBCFG_EQU( F ) //              0x21
GV_KBCFG_EQU( G ) //              0x22
GV_KBCFG_EQU( H ) //              0x23
GV_KBCFG_EQU( J ) //              0x24
GV_KBCFG_EQU( K ) //              0x25
GV_KBCFG_EQU( L ) //              0x26
GV_KBCFG_EQU( SEMICOLON ) //      0x27
GV_KBCFG_EQU( APOSTROPHE ) //     0x28
GV_KBCFG_EQU( GRAVE ) //          0x29    /* accent grave */
GV_KBCFG_EQU( LSHIFT ) //         0x2A
GV_KBCFG_EQU( BACKSLASH ) //      0x2B
GV_KBCFG_EQU( Z ) //              0x2C
GV_KBCFG_EQU( X ) //              0x2D
GV_KBCFG_EQU( C ) //              0x2E
GV_KBCFG_EQU( V ) //              0x2F
GV_KBCFG_EQU( B ) //              0x30
GV_KBCFG_EQU( N ) //              0x31
GV_KBCFG_EQU( M ) //              0x32
GV_KBCFG_EQU( COMMA ) //          0x33
GV_KBCFG_EQU( PERIOD ) //         0x34    /* . on main keyboard */
GV_KBCFG_EQU( SLASH ) //          0x35    /* / on main keyboard */
GV_KBCFG_EQU( RSHIFT ) //         0x36
GV_KBCFG_EQU( MULTIPLY ) //       0x37    /* * on numeric keypad */
GV_KBCFG_EQU( LMENU ) //          0x38    /* left Alt */
GV_KBCFG_EQU( SPACE ) //          0x39
GV_KBCFG_EQU( CAPITAL ) //        0x3A
GV_KBCFG_EQU( F1 ) //             0x3B
GV_KBCFG_EQU( F2 ) //             0x3C
GV_KBCFG_EQU( F3 ) //             0x3D
GV_KBCFG_EQU( F4 ) //             0x3E
GV_KBCFG_EQU( F5 ) //             0x3F
GV_KBCFG_EQU( F6 ) //             0x40
GV_KBCFG_EQU( F7 ) //             0x41
GV_KBCFG_EQU( F8 ) //             0x42
GV_KBCFG_EQU( F9 ) //             0x43
GV_KBCFG_EQU( F10 ) //            0x44
GV_KBCFG_EQU( NUMLOCK ) //        0x45
GV_KBCFG_EQU( SCROLL ) //         0x46    /* Scroll Lock */
GV_KBCFG_EQU( NUMPAD7 ) //        0x47
GV_KBCFG_EQU( NUMPAD8 ) //        0x48
GV_KBCFG_EQU( NUMPAD9 ) //        0x49
GV_KBCFG_EQU( SUBTRACT ) //        0x4A    /* - on numeric keypad */
GV_KBCFG_EQU( NUMPAD4 ) //        0x4B
GV_KBCFG_EQU( NUMPAD5 ) //        0x4C
GV_KBCFG_EQU( NUMPAD6 ) //        0x4D
GV_KBCFG_EQU( ADD ) //            0x4E    /* + on numeric keypad */
GV_KBCFG_EQU( NUMPAD1 ) //        0x4F
GV_KBCFG_EQU( NUMPAD2 ) //        0x50
GV_KBCFG_EQU( NUMPAD3 ) //        0x51
GV_KBCFG_EQU( NUMPAD0 ) //        0x52
GV_KBCFG_EQU( DECIMAL ) //       0x53    /* . on numeric keypad */
GV_KBCFG_EQU( OEM_102 ) //        0x56    /* <> or \| on RT 102-key keyboard (Non-U.S.) */
GV_KBCFG_EQU( F11 ) //            0x57
GV_KBCFG_EQU( F12 ) //            0x58
GV_KBCFG_EQU( F13 ) //            0x64    /*                     (NEC PC98) */
GV_KBCFG_EQU( F14 ) //            0x65    /*                     (NEC PC98) */
GV_KBCFG_EQU( F15 ) //            0x66    /*                     (NEC PC98) */
GV_KBCFG_EQU( KANA ) //           0x70    /* (Japanese keyboard)            */
GV_KBCFG_EQU( ABNT_C1 ) //        0x73    /* /? on Brazilian keyboard */
GV_KBCFG_EQU( CONVERT ) //        0x79    /* (Japanese keyboard)            */
GV_KBCFG_EQU( NOCONVERT ) //      0x7B    /* (Japanese keyboard)            */
GV_KBCFG_EQU( YEN ) //            0x7D    /* (Japanese keyboard)            */
GV_KBCFG_EQU( ABNT_C2 ) //        0x7E    /* Numpad . on Brazilian keyboard */
GV_KBCFG_EQU( NUMPADEQUALS ) //   0x8D    /* = on numeric keypad (NEC PC98) */
GV_KBCFG_EQU( PREVTRACK ) //      0x90    /* Previous Track (DIK_CIRCUMFLEX on Japanese keyboard) */
GV_KBCFG_EQU( AT ) //             0x91    /*                     (NEC PC98) */
GV_KBCFG_EQU( COLON ) //          0x92    /*                     (NEC PC98) */
GV_KBCFG_EQU( UNDERLINE ) //      0x93    /*                     (NEC PC98) */
GV_KBCFG_EQU( KANJI ) //          0x94    /* (Japanese keyboard)            */
GV_KBCFG_EQU( STOP ) //           0x95    /*                     (NEC PC98) */
GV_KBCFG_EQU( AX ) //             0x96    /*                     (Japan AX) */
GV_KBCFG_EQU( UNLABELED ) //      0x97    /*                        (J3100) */
GV_KBCFG_EQU( NEXTTRACK ) //      0x99    /* Next Track */
GV_KBCFG_EQU( NUMPADENTER ) //    0x9C    /* Enter on numeric keypad */
GV_KBCFG_EQU( RCONTROL ) //       0x9D
GV_KBCFG_EQU( MUTE ) //           0xA0    /* Mute */
GV_KBCFG_EQU( CALCULATOR ) //     0xA1    /* Calculator */
GV_KBCFG_EQU( PLAYPAUSE ) //      0xA2    /* Play / Pause */
GV_KBCFG_EQU( MEDIASTOP ) //      0xA4    /* Media Stop */
GV_KBCFG_EQU( VOLUMEDOWN ) //     0xAE    /* Volume - */
GV_KBCFG_EQU( VOLUMEUP ) //       0xB0    /* Volume + */
GV_KBCFG_EQU( WEBHOME ) //        0xB2    /* Web home */
GV_KBCFG_EQU( NUMPADCOMMA ) //    0xB3    /* , on numeric keypad (NEC PC98) */
GV_KBCFG_EQU( DIVIDE ) //         0xB5    /* / on numeric keypad */
GV_KBCFG_EQU( SYSRQ ) //          0xB7
GV_KBCFG_EQU( RMENU ) //          0xB8    /* right Alt */
GV_KBCFG_EQU( PAUSE ) //          0xC5    /* Pause */
GV_KBCFG_EQU( HOME ) //           0xC7    /* Home on arrow keypad */
GV_KBCFG_EQU( UP ) //             0xC8    /* UpArrow on arrow keypad */
GV_KBCFG_EQU( PRIOR ) //          0xC9    /* PgUp on arrow keypad */
GV_KBCFG_EQU( LEFT ) //           0xCB    /* LeftArrow on arrow keypad */
GV_KBCFG_EQU( RIGHT ) //          0xCD    /* RightArrow on arrow keypad */
GV_KBCFG_EQU( END ) //            0xCF    /* End on arrow keypad */
GV_KBCFG_EQU( DOWN ) //           0xD0    /* DownArrow on arrow keypad */
GV_KBCFG_EQU( NEXT ) //           0xD1    /* PgDn on arrow keypad */
GV_KBCFG_EQU( INSERT ) //         0xD2    /* Insert on arrow keypad */
GV_KBCFG_EQU( DELETE ) //         0xD3    /* Delete on arrow keypad */
GV_KBCFG_EQU( LWIN ) //           0xDB    /* Left Windows key */
GV_KBCFG_EQU( RWIN ) //           0xDC    /* Right Windows key */
GV_KBCFG_EQU( APPS ) //           0xDD    /* AppMenu key */
GV_KBCFG_EQU( POWER ) //          0xDE    /* System Power */
GV_KBCFG_EQU( SLEEP ) //          0xDF    /* System Sleep */
GV_KBCFG_EQU( WAKE ) //           0xE3    /* System Wake */
GV_KBCFG_EQU( WEBSEARCH ) //      0xE5    /* Web Search */
GV_KBCFG_EQU( WEBFAVORITES ) //   0xE6    /* Web Favorites */
GV_KBCFG_EQU( WEBREFRESH ) //     0xE7    /* Web Refresh */
GV_KBCFG_EQU( WEBSTOP ) //        0xE8    /* Web Stop */
GV_KBCFG_EQU( WEBFORWARD ) //     0xE9    /* Web Forward */
GV_KBCFG_EQU( WEBBACK ) //        0xEA    /* Web Back */
GV_KBCFG_EQU( MYCOMPUTER ) //     0xEB    /* My Computer */
GV_KBCFG_EQU( MAIL ) //           0xEC    /* Mail */
GV_KBCFG_EQU( MEDIASELECT ) //    0xED    /* Media Select */

#endif	/* defined(GV_KBCFG_EQU) */

	/*----------------------------------------------------------*/
	/*	PAD														*/
	/*----------------------------------------------------------*/
#ifdef	GV_PADCFG_ANA_EQU

	/*--			定義名,	構造体メンバ名	--*/
GV_PADCFG_ANA_EQU(	X,		lX	)	/* x-axis position              */
GV_PADCFG_ANA_EQU(	Y,		lY	)	/* y-axis position              */
GV_PADCFG_ANA_EQU(	Z,		lZ	)	/* z-axis position              */
GV_PADCFG_ANA_EQU(	Rx,		lRx	)	/* x-axis rotation              */
GV_PADCFG_ANA_EQU(	Ry,		lRy	)	/* y-axis rotation              */
GV_PADCFG_ANA_EQU(	Rz,		lRz	)	/* z-axis rotation              */
GV_PADCFG_ANA_EQU(	Slider0,rglSlider[0])	/* extra axes positions */
GV_PADCFG_ANA_EQU(	Slider1,rglSlider[1])	/* extra axes positions */

#endif	/* defined(GV_PADCFG_ANA_EQU) */
\