/*
  Web Ranking で使用する場合の結果出力部分
  漢字は使用せず、
  <tag> : <value>
  形式で出力する。

  PHP からは、popen() でパイプを作成することによって実行、結果を取り込む。

  $Id: display_web.c,v 1.9 2002/11/29 06:25:03 usr10656 Exp $
*/
#include	<stdio.h>
#include	"clearcode.h"
#include	"format.h"

static void DisplayCommon( u_int* data )
{
	printf("platform : ");
	switch( data[ COMMON_PLATFORM ] ) {
	case 0: printf("PS2\n"); break;
	case 1: printf("Xbox\n"); break;
	case 2: printf("PC\n"); break;
	default: printf("unknown\n"); break;
	}
	printf("region : ");
	switch( data[ COMMON_REGION ] ) {
	case 0: printf("Japan\n"); break;
	case 1: printf("USA\n"); break;
	case 2: printf("Europe\n"); break;
	default: printf("unknown\n"); break;
	}
}

void DisplayGame( u_int* data )
{
	printf("mode : main_game\n");
	printf("radar : ");
	switch( data[ GM_RADAR_TYPE ] ) {
	case 0: printf("OFF\n"); break;
	default: printf("ON\n"); break;
	}

	printf("playtime : %d : %d\n",
	       data[ GM_PLAY_TIME ], (data[ GM_PLAY_TIME ] == 0x7fff) ? 1 : 0);

	printf("fire : %d : %d\n",
	       data[ GM_SHOOT ], ( data[ GM_SHOOT ] == 0x3ff) ? 1 : 0);

	printf("damage : %d : %d\n",
	       data[ GM_DAMAGE_RATE ], (data[ GM_DAMAGE_RATE ]==0x1f) ? 1 : 0);

	printf("kill : %d : %d\n",
	       data[ GM_KILL ], ( data[ GM_KILL ] == 0xff) ? 1 : 0);

	printf("alert : %d : %d\n",
	       data[ GM_ALERT ], (data[ GM_ALERT ] ==0xff) ? 1 : 0);


	printf("clearing : %d : %d\n",
	       data[ GM_CLEARING ], (data[ GM_CLEARING ]==0xff) ? 1 : 0);
	
	printf("tanker : %d : %d\n",
	       data[ GM_TANKER_CLEAR ],
	       (data[ GM_TANKER_CLEAR ]==0x3f) ? 1 : 0);

	printf("plant : %d : %d\n",
	       data[ GM_PLANT_CLEAR ],
	       (data[ GM_PLANT_CLEAR ]==0x3f) ? 1 : 0);
	
	printf("continue : %d : %d\n",
	       data[ GM_CONTINUE ], (data[ GM_CONTINUE ]==0x3f) ? 1 : 0);


	printf("ration : %d : %d\n",
	       data[ GM_RATION ], (data[ GM_RATION ]==0x1f) ? 1 : 0);

	printf("save : %d : %d\n",
	       data[ GM_SAVE ], (data[ GM_SAVE ]==0x7f) ? 1 : 0);

	printf("mech : %d : %d\n",
	       data[ GM_CAM_BREAK ],(data[ GM_CAM_BREAK ]==0xff) ? 1 : 0);

	printf("sealouce : ");
	printf("%s\n", ( data[ GM_FUNAMUSHI ] ) ? "yes" : "no");

	printf("special : %s\n", ( data[ GM_SPECIAL_ITEM ] ) ? "yes" : "no");
	
	printf("level : ");
	switch( data[ GM_LEVEL ] ){
	case 0: printf("ve\n"); break;
	case 1: printf("e\n"); break;
	case 2: printf("n\n"); break;
	case 3: printf("h\n"); break;
	case 4: printf("ex\n"); break;
	case 5: printf("e_ex\n"); break;
	}

	printf("chapter : ");
	switch( data[ GM_T_OR_P ] ){
	case 0:	printf("tp\n");	break;
	case 1:	printf("t\n");	break;
	case 2:	printf("p\n"); break;
	case 3:	printf("Unkown\n");	break;
	}

#if 0
	printf("version : ");
	switch( data[ GM_VERSION ] ){
	case 0:	printf("us\n"); break;
	case 1:	printf("jp\n"); break;
	case 2:	printf("eu\n");	break;
	case 3:	printf("Unknown\n"); break;
	}
#else
	if( data[ GM_DATA_MAX + COMMON_PLATFORM ] == 0 )
	  printf("dog_ver : %s\n",
		 (data[ GM_PS2_DOGTAG_2002 ]) ? "2002" : "2001");
#endif
	printf("dogtags : %d\n", data[ GM_DOGTAG ] );

	DisplayCommon( data + GM_DATA_MAX );
}

// #define FPS	60
#define result_print_bstime( title, name )	\
                printf("%s_time : %d\n", title, data[ name ] * TIME_BASE)

#define result_print_bsdie( title, name ) printf( "%s_die : %s\n", title, data[ name ] ? "no" : "yes" );
void DisplayBoss( u_int* data )
{
  int TIME_BASE;
  TIME_BASE = (data[ BS_DATA_MAX + COMMON_REGION ] == 2) ? 6 : 5;

	printf("mode : boss\n");
	result_print_bstime( "orga", BS_OLG_TIME );
	result_print_bstime( "fatman", BS_FAT_TIME );
	result_print_bstime( "harrier", BS_HAR_TIME );
	result_print_bstime( "vamp", BS_VMP_TIME );
	result_print_bstime( "ray", BS_RAY_TIME );
	result_print_bstime( "solidus", BS_SOL_TIME );

	result_print_bsdie( "fatman", BS_FAT_DIE );
	result_print_bsdie( "vamp", BS_VMP_DIE );
	result_print_bsdie( "solidus", BS_SOL_DIE );

	printf("version : %d\n" , data[ BS_VERSION ] );
	printf("level : ");
	switch ( data[ BS_DIFFICULTY ] ){
	case 1 : printf("e"); break;
	case 2 : printf("n"); break;
	case 3 : printf("h"); break;
	}
	printf("\nplayer : %s\n" , data[ BS_PLAYER ] == 0 ? "snake" : "raiden" );
	
	DisplayCommon( data + BS_DATA_MAX );
}

void DisplayBossEx( u_int* data )
{
  int TIME_BASE;
  TIME_BASE = (data[ BE_DATA_MAX + COMMON_REGION ] == 2) ? 6 : 5;

	printf("mode: boss_ex\n");
	result_print_bstime( "orga", BE_OLG_TIME );
	result_print_bstime( "w03", BE_W03_TIME );
	result_print_bstime( "fatman", BE_FAT_TIME );
	result_print_bstime( "harrier", BE_HAR_TIME );
	result_print_bstime( "vamp", BE_VMP_TIME );
	result_print_bstime( "tng", BE_TNG_TIME );
	result_print_bstime( "ray", BE_RAY_TIME );
	result_print_bstime( "solidus", BE_SOL_TIME );

	result_print_bsdie( "w03", BE_W03_DIE );
	result_print_bsdie( "fatman", BE_FAT_DIE );
	result_print_bsdie( "vamp", BE_VMP_DIE );
	result_print_bsdie( "tng", BE_TNG_DIE );
	result_print_bsdie( "solidus", BE_SOL_DIE );

	printf("version : %d\n" , data[ BE_VERSION ] );
	printf("level : ", data[ BE_DIFFICULTY ]);
	switch ( data[ BE_DIFFICULTY ] ){
	case 1 : printf("e"); break;
	case 2 : printf("n"); break;
	case 3 : printf("h"); break;
	}
	printf("\nplayer : %s\n" , data[ BE_PLAYER ] == 0 ? "snake" : "raiden" );
	
	DisplayCommon( data + BE_DATA_MAX );
}

#include	"vrstagename.h"

void DisplayVR( u_int* data )
{
	printf("mode : VR\n");
	printf("codehash : %d\n", data[ VR_NAME ] );
	//	printf("Version %d\n", data[ VR_VER ] );

	printf("stage : %d\n", data[ VR_STAGE ] );

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
	  printf("player : %s\n", player);
	  printf("mission : %s\n", mission);
	  printf("stgmode : %s\n", mode);
	  printf("stgsubmode : %s\n", submode);
	  printf("level : %s\n", level);
	}
	printf("score : %d\n", data[ VR_SCORE ] );
	printf("timescore : %d\n", data[ VR_TIME ] );
	printf("bullets : %d\n", data[ VR_BULLETS ] );
	printf("sneaking : %d\n", data[ VR_SNEAKING ] );
	printf("nokill : %d\n", data[ VR_NO_KILL ] );
	
	DisplayCommon( data + VR_DATA_MAX );
}
