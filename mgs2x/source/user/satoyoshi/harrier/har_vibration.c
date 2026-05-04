/********************************************************************************/
/*	Har_vibration.c								*/
/*	ハリア振動設定								*/
/*	2001/08/06 H.Satoyoshi							*/
/*	$Id: har_vibration.c,v 1.1.1.3 2002/11/19 11:48:25 Yoshizawa1 Exp $		*/
/********************************************************************************/

/********************************************************************************/
/*	include files								*/
/********************************************************************************/

/********************************************************************************/
/*	Define									*/
/********************************************************************************/
#define VIB_BURNING	(1)
#define VIB_MP_EXP	(2)
#define VIB_CL_EXP	(3)
#define VIB_AM_EXP	(4)

/********************************************************************************/
/*	Extern									*/
/********************************************************************************/
extern void *NewPadVibration( char *script, int type );

/********************************************************************************/
/*	Program									*/
/********************************************************************************/

u_char vib_mpexp_data01[] = {
    1, 6, 0, 0
};
u_char vib_mpexp_data02[] = {
    160, 10, 0, 0
};
u_char vib_clexp_data01[] = {
    1, 6, 0, 0
};
u_char vib_clexp_data02[] = {
    255, 8, 64, 16, 0, 0
};
u_char vib_amexp_data01[] = {
    1, 4, 0, 0
};
u_char vib_amexp_data02[] = {
    255, 16, 64, 8, 0, 0
};

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void call_pad_vibration						*/
/*	引数:	振動タイプ							*/
/*	説明:	パッドを振動させる						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void call_pad_vibration(int vib_case)
{
    switch (vib_case){
    case VIB_BURNING:
	GM_PadVibration2 = 96;
	break;
    case VIB_MP_EXP:
	NewPadVibration(vib_mpexp_data01, 1);
	NewPadVibration(vib_mpexp_data02, 2);
	break;
    case VIB_CL_EXP:
	NewPadVibration(vib_clexp_data01, 1);
	NewPadVibration(vib_clexp_data02, 2);
	break;
    case VIB_AM_EXP:
	NewPadVibration(vib_amexp_data01, 1);
	NewPadVibration(vib_amexp_data02, 2);
	break;
    }
}
