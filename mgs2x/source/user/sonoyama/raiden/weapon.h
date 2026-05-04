/*
   weapon.h
   Éð´ï´Ø·¸ÄêµÁ¥Õ¥¡¥¤¥ë

   1999/07/19 M.Sonoyama
   $Id: weapon.h,v 1.1.1.3 2002/11/19 11:51:03 Yoshizawa1 Exp $
*/

extern	WEAPON	NewUsp, NewM92, NewFamas, NewSocom ;
extern	WEAPON	NewC4Bomb, NewPsg1, NewChaff, NewStun, NewMagazine ;
extern	WEAPON	NewClaymore, NewGrenade, NewRGB6, NewNikita, NewStinger ;
extern	WEAPON	NewColdSpray, NewSpp1M, NewMic ;
extern	WEAPON	NewAks, NewM4, NewPsg1T, NewBook ;

PL_WeaponSet	PL_WeaponSets[] = {
    { NULL,   	NULL, 	NONE_TYPE, 	NULL, NULL },			/* ÁÇ¼ê */
    { NewM92,   NULL, 	M92_TYPE,	ShootBullet, NULL },	/* ¥Ù¥ì¥Ã¥¿ */
    { NewUsp,   NULL,	USP_TYPE,	ShootBullet, NULL },	/* £Õ£Ó£Ð */
    { NewSocom, NULL,	SOCOM_TYPE,	ShootBullet, NULL },	/* £Ó£Ï£Ã£Ï£Í */
    { NewPsg1,  ShootPsg1,	PSG_TYPE, NULL, ReleasePsg1 },		/* £Ð£Ó£Ç¡Ý£± */
    { NewRGB6, 	NULL, 	RGB_TYPE, 	ShootBullet, NULL },	/* £Ò£Ç£Â¡²£¶ */
    { NewNikita,NULL, 	NIKITA_TYPE, 	ShootNikita, NULL },	/* ¥Ë¥­¡¼¥¿ */
    { NewStinger, ShootStinger, STG_TYPE, 	NULL, NULL },	/* ¥¹¥Æ¥£¥ó¥¬¡¼ */
    { NewClaymore,NULL, CLAY_TYPE, 	SetClaymore, NULL },	/* ¥¯¥ì¥¤¥â¥¢ */
    { NewC4Bomb,NULL, 	C4_TYPE, 	SetC4BombCheck, NULL },	/* £Ã£´ */
    { NewChaff, NULL, 	CHAFF_TYPE, 	ThrowGrenade, NULL },	/* ¥Á¥ã¥Õ */
    { NewStun, 	NULL, 	STUN_TYPE, 	ThrowGrenade, NULL },	/* ¥¹¥¿¥ó */
    { NewMic,   	  	SetMic, 	MIC_TYPE, 		NULL, NULL },	      	/* ¥Þ¥¤¥¯ */
    { NULL, 	NULL, 	BLADE_TYPE, NULL, NULL },		/* ¥Ö¥ì¡¼¥É */
    { NewColdSpray,   	JetSpray, 	SPRAY_TYPE, 	NULL, NULL },		/* Åà·ë¥¹¥×¥ì¡¼ */
    { NewAks, NULL, 	AKS_TYPE, ShootBullet, NULL },		/* £Á£Ë£Ó */
    { NewMagazine, NULL, MAGAZINE_TYPE, ThrowGrenade, NULL },	/* ¥Þ¥¬¥¸¥ó */
    { NewGrenade, NULL, GRENADE_TYPE, ThrowGrenade, NULL },	/* ¥°¥ì¥Í¡¼¥É */
    { NewM4, NULL,	M4_TYPE,	ShootBullet, NULL },		/* £Í£´ */
    { NewPsg1T,  ShootPsg1,	PSG_TYPE, NULL, ReleasePsg1 },		/* £Ð£Ó£Ç¡Ý£±¡ÊËã¿ì¡Ë */
    { NewMic,    SetMic, 	MIC_TYPE, NULL, NULL },	      	/* ÆÃ¼ì¥Þ¥¤¥¯ */
	{ NewBook, NULL, BOOK_TYPE, SetBook, NULL }		/* »¨»ï */
} ;

