/********************************************************************************/
/*	Har_main.c								*/
/*	ハリアメイン *NewHarrier						*/
/*	2001/01/23 H.Satoyoshi							*/
/*	$Id: har_pmove.c,v 1.1.1.3 2002/11/19 11:48:24 Yoshizawa1 Exp $		*/
/********************************************************************************/
/********************************************************************************/
/*	Program									*/
/********************************************************************************/

#define		HMK2_TYPE_RED_AT_BIG	(0x80008020)	//! 赤 大
#define		HMK2_TYPE_RED_AT	(0x80008000)	//! 赤		(前回  0)

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void select_root						*/
/*	引数:	Work	*work							*/
/*	説明:	ルート選択							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void select_root(Work *work){
    switch (RAND(4)){
	
    case 0:
	work->gun_attack_millor_flg &= ~MILLOR_X_FLAG;
	work->gun_attack_millor_flg &= ~MILLOR_Z_FLAG;
	//	printf ("Attack                                          0\n");
	break;
	
    case 1:
	work->gun_attack_millor_flg |=  MILLOR_X_FLAG;
	work->gun_attack_millor_flg &= ~MILLOR_Z_FLAG;
	//	printf ("Attack                                          1\n");
	break;
	
    case 2:
	work->gun_attack_millor_flg |=  MILLOR_X_FLAG;
	work->gun_attack_millor_flg |=  MILLOR_Z_FLAG;
	//	printf ("Attack                                          2\n");
	break;
	
    case 3:
	work->gun_attack_millor_flg &= ~MILLOR_X_FLAG;
	work->gun_attack_millor_flg |=  MILLOR_Z_FLAG;
	//	printf ("Attack                                          3\n");
	break;    
	
    }
}


/*******************************<Global function>********************************/
/*	名前:	void Har_ControlPmove						*/
/*	引数:	Work	*work							*/
/*	説明:	プログラム移動のコントロール					*/
/********************************************************************************/
int Har_ControlPmove(Work *work)
{
    
    switch (HAR_MODE_L2){	// 上段機銃攻撃
	
    case H_MOD_ATACK_UP:
	if (atack_normal_up(work) == ON){
	    return 1;
	}
	break;
    }
    return 0;
}



