/*
  結果表示部分
  EUC/SJIS 両方つくる
  
  $Id: display.c,v 1.8 2002/10/02 06:47:24 usr03700 Exp $

*/

#include	<stdio.h>
#include	"clearcode.h"
#include	"format.h"

static void DisplayCommon( u_int* data )
{
	printf("プラットフォーム : ");
	switch( data[ COMMON_PLATFORM ] ) {
	case 0: printf("PS2\n"); break;
	case 1: printf("Xbox\n"); break;
	default: printf("unknown\n"); break;
	}
	printf("仕向地 : ");
	switch( data[ COMMON_REGION ] ) {
	case 0: printf("Japan\n"); break;
	case 1: printf("USA\n"); break;
	case 2: printf("Europe\n"); break;
	default: printf("unknown\n"); break;
	}
}

void DisplayGame( u_int* data )
{
	printf("**** 本編クリアコードが入力されました ****\n");
	printf("レーダータイプ : ");
#if 0
	switch( data[ GM_RADAR_TYPE ] ) {
	case 0: printf("OFF\n"); break;
	case 1: printf("TYPE 2\n"); break;
	case 2: printf("TYPE 1\n"); break;
	default: printf("Unknown\n"); break;
	}
#else
	switch( data[ GM_RADAR_TYPE ] ) {
	case 0: printf("未使用\n"); break;
	default: printf("使用した\n"); break;
	}
#endif

	{
		int hour,min,sec;
		min= data[ GM_PLAY_TIME ] /4;
		sec=(data[ GM_PLAY_TIME ] %4)*15;
		hour=min/60;
		min%=60;
		if(data[ GM_PLAY_TIME ] ==0x7fff) printf("プレイ時間(15秒単位) : %d:%d:%d 以上\n",hour,min,sec);
		else printf("プレイ時間(15秒単位) : %04d:%02d:%02d\n",hour,min,sec);
	}

	if( data[ GM_SHOOT ] == 0x3ff) printf("発砲数 : %d 以上\n",data[ GM_SHOOT ]);
	else printf("発砲数 : %d\n",data[ GM_SHOOT ]);

	if(data[ GM_DAMAGE_RATE ]==0x1f) printf("ダメージ率 : ゲージ %f 本分以上\n",(float)data[ GM_DAMAGE_RATE ]*0.5f);
	else printf("ダメージ率 : ゲージ %f 本文\n",(float)data[ GM_DAMAGE_RATE ]*0.5f);

	if(data[ GM_ALERT ] ==0xff) printf("危険モード回数 : %d 以上\n",data[ GM_ALERT ]);
	else printf("危険モード回数 : %d\n",data[ GM_ALERT ]);

	if(data[ GM_CLEARING ]==0xff) printf("クリアリング回数 : %d 以上\n",data[ GM_CLEARING ]);
	else printf("クリアリング回数 : %d\n",data[ GM_CLEARING ]);
	
	if(data[ GM_TANKER_CLEAR ]==0x3f) printf("タンカー編クリア回数 : %d 以上\n",data[ GM_TANKER_CLEAR ]);
	else printf("タンカー編クリア回数 : %d\n",data[ GM_TANKER_CLEAR ]);

	if(data[ GM_PLANT_CLEAR ]==0x3f) printf("プラント編クリア回数 : %d 以上\n",data[ GM_PLANT_CLEAR ]);
	else printf("プラント編クリア回数 : %d\n",data[ GM_PLANT_CLEAR ]);
	
	if(data[ GM_CONTINUE ]==0x3f) printf("コンティニュー回数 : %d 以上\n",data[ GM_CONTINUE ]);
	else printf("コンティニュー回数 : %d\n",data[ GM_CONTINUE ]);

	if(data[ GM_RATION ]==0x1f) printf("レーション使用回数 : %d 以上\n",data[ GM_RATION ]);
	else printf("レーション使用回数 : %d\n",data[ GM_RATION ]);

	if(data[ GM_SAVE ]==0x7f) printf("セーブ回数 : %d 以上\n",data[ GM_SAVE ]);
	else printf("セーブ回数 : %d\n",data[ GM_SAVE ]);

	if(data[ GM_CAM_BREAK ]==0xff) printf("カメラ・ガンカメラ・サイファー破壊数 : %d 以上\n",data[ GM_CAM_BREAK ]);
	else printf("カメラ・ガンカメラ・サイファー破壊数 : %d\n",data[ GM_CAM_BREAK ]);

	printf("舟虫がくっついた : ");
	if( data[ GM_FUNAMUSHI ] ) printf("はい\n");
	else printf("いいえ\n");

	printf("特殊アイテム使用 : ");
	if( data[ GM_SPECIAL_ITEM ] ) printf("使用した\n");
	else printf("使用していない\n");
	
	printf("ゲームレベル : ");
	switch( data[ GM_LEVEL ] ){
	case 0: printf("VERY EASY\n"); break;
	case 1: printf("EASY\n"); break;
	case 2: printf("NORMAL\n"); break;
	case 3: printf("HARD\n"); break;
	case 4: printf("EXTREME\n"); break;
	case 5: printf("E-EXTREME\n"); break;
	}

	printf("タンカー編 or プラント編 : ");
	switch( data[ GM_T_OR_P ] ){
	case 0:	printf("Tanker & Plant\n");	break;
	case 1:	printf("Tanker\n");	break;
	case 2:	printf("Plant\n"); break;
	case 3:	printf("Unkown\n");	break;
	}
	
#if 0	// バージョンは未使用扱い
	printf("バージョン : ");
	switch( data[ GM_VERSION ] ){
	case 0:	printf("USA\n"); break;
	case 1:	printf("JAPAN\n"); break;
	case 2:	printf("EUROPE\n");	break;
	case 3:	printf("Unknown\n"); break;
	}
#else
	if( data[ GM_DATA_MAX + COMMON_PLATFORM ] == 0 ) {
		// PS2 のみの仕様
		printf("ドッグタグモード : ");
		if( data[ GM_PS2_DOGTAG_2002 ] ) {
			printf( "2002年バージョン\n" );
		} else {
			printf( "2001年バージョン\n" );
		}
	}
	
#endif
	printf("ドッグタグ回収率 : %d\n", data[ GM_DOGTAG ] );
	DisplayCommon( data + GM_DATA_MAX );

}

//#define FPS	60
#define result_print_bstime( title, name )	printf( title " : %02d:%02d:%02d\n", data[ name ] / (FPS * 60), \
										   (data[ name ] / FPS) % 60, (data[ name ] * 100 / FPS) % 100)
#define result_print_bsdie( title, name ) printf( title " : %s\n", data[ name ] ? "気絶" : "死亡" );
void DisplayBoss( u_int* data )
{
	int FPS;
	// 仕向地によってFPSを変える
	if( data[ BS_DATA_MAX + COMMON_REGION ] == 2 ) {
		FPS = 50;
	} else {
		FPS = 60;
	}
	

	printf("**** ボスサバイバルクリアコードが入力されました ****\n");
	result_print_bstime( "オルガ      ", BS_OLG_TIME );
	result_print_bstime( "ファットマン", BS_FAT_TIME );
	result_print_bstime( "ハリアー    ", BS_HAR_TIME );
	result_print_bstime( "ヴァンプ    ", BS_VMP_TIME );
	result_print_bstime( "レイ        ", BS_RAY_TIME );
	result_print_bstime( "ソリダス    ", BS_SOL_TIME );

	result_print_bsdie( "ファットマン", BS_FAT_DIE );
	result_print_bsdie( "ヴァンプ    ", BS_VMP_DIE );
	result_print_bsdie( "ソリダス    ", BS_SOL_DIE );

	printf("Version %x\n" , data[ BS_VERSION ] );
	printf("難易度 : ");
	switch ( data[ BS_DIFFICULTY ] ){
	case 1 : printf("Easy\n"); break;
	case 2 : printf("Normal\n"); break;
	case 3 : printf("Hard\n"); break;
	}
	printf("Player : %s\n" , data[ BS_PLAYER ] == 0 ? "スネーク" : "ライデン" );
	
	DisplayCommon( data + BS_DATA_MAX );
}

void DisplayBossEx( u_int* data )
{
	int FPS;
	// 仕向地によってFPSを変える
	if( data[ BE_DATA_MAX + COMMON_REGION ] == 2 ) {
		FPS = 50;
	} else {
		FPS = 60;
	}

	printf("**** ボスサバイバルExクリアコードが入力されました ****\n");
	result_print_bstime( "オルガ      ", BE_OLG_TIME );
	result_print_bstime( "Ｗ０３      ", BE_W03_TIME );
	result_print_bstime( "ファットマン", BE_FAT_TIME );
	result_print_bstime( "ハリアー    ", BE_HAR_TIME );
	result_print_bstime( "ヴァンプ    ", BE_VMP_TIME );
	result_print_bstime( "天狗        ", BE_TNG_TIME );
	result_print_bstime( "レイ        ", BE_RAY_TIME );
	result_print_bstime( "ソリダス    ", BE_SOL_TIME );

	result_print_bsdie( "Ｗ０３      ", BE_W03_DIE );
	result_print_bsdie( "ファットマン", BE_FAT_DIE );
	result_print_bsdie( "ヴァンプ    ", BE_VMP_DIE );
	result_print_bsdie( "天狗        ", BE_TNG_DIE );
	result_print_bsdie( "ソリダス    ", BE_SOL_DIE );

	printf("Version %x\n" , data[ BE_VERSION ] );
	printf("難易度 : ");
	switch ( data[ BE_DIFFICULTY ] ){
	case 1 : printf("Easy\n"); break;
	case 2 : printf("Normal\n"); break;
	case 3 : printf("Hard\n"); break;
	}
	printf("Player : %s\n" , data[ BE_PLAYER ] == 0 ? "スネーク" : "ライデン" );
	
	DisplayCommon( data + BE_DATA_MAX );
}

#include	"vrstagename.h"

void DisplayVR( u_int* data )
{
	printf("**** ＶＲクリアコードが入力されました ****\n");
	printf("プレイヤー名ハッシュ値 %x\n", data[ VR_NAME ] );
	//	printf("Version %d\n", data[ VR_VER ] );
	printf("ステージＩＤ %d\n", data[ VR_STAGE ] );
	{
		char player[ VR_STAGENAME_MAX ];
		char mission[ VR_STAGENAME_MAX ];
		char mode[ VR_STAGENAME_MAX ];
		char submode[ VR_STAGENAME_MAX ];
		char level[ VR_STAGENAME_MAX ];

		GetVrStageName( data[ VR_STAGE ],
						data[ VR_DATA_MAX + COMMON_PLATFORM ],
						data[ VR_DATA_MAX + COMMON_REGION ],
						player, mission, mode, submode, level );
		// submode == "" の場合もある
		printf("(%s  %s  %s  %s  %s)\n", player, mission, mode, submode, level );
	}
	printf("スコア %d\n", data[ VR_SCORE ] );
	printf("時間スコア %d\n", data[ VR_TIME ] );
	printf("残弾数スコア %d\n", data[ VR_BULLETS ] );
	printf("隠密度 %d\n", data[ VR_SNEAKING ] );
	printf("不殺 %d\n", data[ VR_NO_KILL ] );
	
	DisplayCommon( data + VR_DATA_MAX );
}
