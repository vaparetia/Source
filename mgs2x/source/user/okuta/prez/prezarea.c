/*
    prezarea.c
    大統領ニキータイベントエリア設定
    2001/04/24 Masafumi Okuta
    $Id: prezarea.c,v 1.1.1.3 2002/11/19 11:48:09 Yoshizawa1 Exp $
*/
// エリア設定
#define PREZ_AREA_X_1 (530.f)
#define PREZ_AREA_X_2 (3333.f)
#define PREZ_AREA_X_3 (6396.f)

#define PREZ_AREA_Z_1 (-246000.f)
#define PREZ_AREA_Z_2 (-243232.f)
#define PREZ_AREA_Z_3 (-244490.f)	// 電気版前

// エリア取得
static int PREZ_GetArea( FVECTOR* pvec)
{
    if ( pvec->vx < PREZ_AREA_X_1 ){ 		// 左側
	if ( pvec->vz < PREZ_AREA_Z_2 ){
	    return (PREZ_AREA_1);
	}else {
	    return (PREZ_AREA_2);
	}
    }else if ( pvec->vx < PREZ_AREA_X_2 ){ 	// 中央
	if ( pvec->vz < PREZ_AREA_Z_1 ){
	    return (PREZ_AREA_3);
	}else if ( pvec->vz < PREZ_AREA_Z_2 ){
	    return (PREZ_AREA_4);
	}else {
	    return (PREZ_AREA_5);
	}
    }else if ( pvec->vx < PREZ_AREA_X_3 ){ 	// 右側
	    return (PREZ_AREA_6);
    }else {					// 電気版前
	if ( pvec->vz < PREZ_AREA_Z_3 ){
	    return (PREZ_AREA_7);
	}else {
	    return (PREZ_AREA_8);
	}
    }

    return (-1);
}

