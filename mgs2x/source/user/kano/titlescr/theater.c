//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	theater.c
		ローポリ劇場選択画面

	2001/09/28  K.Kano
	$Id: theater.c,v 1.1.1.3 2002/11/19 11:43:41 Yoshizawa1 Exp $
*/


#include "titlescr.h"

#include "font.h"

#include "subtitle.h"

#include "BP_BuildDefines.h"
#include "BP_Font.h"
#include "BP_TrophyLogicMGS2.h"

#define N_DEMOS		8
#define N_CASTS		4


typedef struct {
	GV_ACT_EX actor;
	int name;

	int proc;
	int ans;

	int busy_flag;
	int l2d_strcode;
	int l2d_handle;
	int action_strcode;

	int step;
	int sub_step;

	int key_status;
	int key_press;
	int key_autostatus;
	int key_count;

	int demo_cursor;
	int cast_cursor;
	int cast_item_cursor[N_CASTS];
	int wait_count;

	int start_mode;

	int sound_flag;

	void *strman;
	int explain_flag;
	int disp_width;

	int enselcast_flags[N_DEMOS][N_CASTS];

#if 0

	struct {
		int part_code;
		int key0_code;
		int key1_code;
		float p;
		int count;
	} key_anim[ANIM_WORK_SIZE];

	float t_y_pos;
	int y_pos_count;

	struct {
		int spr_code;
		int t_alpha;
		int count;
	} alphaanim[2];

	int base_v[N_OPTION_ITEMS],base_pv[N_OPTION_ITEMS];

#endif

	int explain_resource;

} Work;


#define CODE_RARROW						0x00d535f0		/* right_arrow */
#define CODE_LARROW						0x00332429		/* left_arrow */

#define CODE_KEY_RARROW_HIGHLIGHT		0x005b4db4		/* preRightHigh */
#define CODE_KEY_RARROW_NORMAL			0x0062eb3a		/* preRightLow */
#define CODE_KEY_LARROW_HIGHLIGHT		0x000d2d2a		/* preLeftHigh */
#define CODE_KEY_LARROW_NORMAL			0x00107a36		/* preLeftLow */


#define CODE_PIC_TITLE					0x0089e81e		/* demo_titleSC */

#define CODE_KEY_DISP_TITLE1			0x004a27f1		/* preview_sw1 */
#define CODE_KEY_DISP_TITLE2			0x004a27f2		/* preview_sw2 */
#define CODE_KEY_DISP_TITLE3			0x004a27f3		/* preview_sw3 */
#define CODE_KEY_DISP_TITLE4			0x004a27f4		/* preview_sw4 */
#define CODE_KEY_DISP_TITLE5			0x004a27f5		/* preview_sw5 */
#define CODE_KEY_DISP_TITLE6			0x004a27f6		/* preview_sw6 */
#define CODE_KEY_DISP_TITLE7			0x004a27f7		/* preview_sw7 */
#define CODE_KEY_DISP_TITLE8			0x004a27f8		/* preview_sw8 */


#define CODE_PIC_TITLE_LIGHT			0x00bc50ac		/* demo_switch-1 */

#define CODE_KEY_TITLE_HIGHLIGHT		0x001579b6		/* preCenterHigh */
#define CODE_KEY_TITLE_NORMAL			0x0070bc9a		/* preCenterLow */


#define CODE_PICPANEL1					0x0099c062		/* preview1 */
#define CODE_PICPANEL2					0x0099c063		/* preview2 */

#define CODE_SPR_PICPANEL1				0x0099c062		/* preview1 */
#define CODE_SPR_PICPANEL2				0x0099c063		/* preview2 */

#define CODE_KEY_DISP_PIC1				0x0099c062		/* preview1 */
#define CODE_KEY_DISP_PIC2				0x0099c063		/* preview2 */
#define CODE_KEY_DISP_PIC3				0x0099c064		/* preview3 */
#define CODE_KEY_DISP_PIC4				0x0099c065		/* preview4 */
#define CODE_KEY_DISP_PIC5				0x0099c066		/* preview5 */
#define CODE_KEY_DISP_PIC6				0x0099c067		/* preview6 */
#define CODE_KEY_DISP_PIC7				0x0099c068		/* preview7 */
#define CODE_KEY_DISP_PIC8				0x0099c069		/* preview8 */

#define CODE_SHOW_DEMOSEL				0x00d5bc46		/* openScene */
#define CODE_HIDE_DEMOSEL				0x0032b7c1		/* closeScene */
#define CODE_SHOW_CASTSEL				0x0006a7a5		/* openCast */
#define CODE_HIDE_CASTSEL				0x00d98f80		/* closeCast */


#define CODE_PLAY_BUTTON				0x004d5e47		/* play-hi */

#define CODE_KEY_PLAY_HIGHLIGHT			0x00b94d11		/* playHigh */
#define CODE_KEY_PLAY_NORMAL			0x004ddb35		/* playLow */


#define CODE_SHUFFLE_BUTTON				0x00bf03ca		/* shuffle-hi */

#define CODE_KEY_SHUFFLE_HIGHLIGHT		0x00edfd7f		/* shuffleHigh */
#define CODE_KEY_SHUFFLE_NORMAL			0x006a18aa		/* shffleLow */


#define CODE_CAST_POINTER				0x003ddd36		/* arrow_cursor */
#define CODE_DISP_CAST_POINTER1			0x0004f737		/* arrow_1-1 */
#define CODE_DISP_CAST_POINTER2			0x0004fb37		/* arrow_2-1 */
#define CODE_DISP_CAST_POINTER3			0x0004ff37		/* arrow_3-1 */
#define CODE_DISP_CAST_POINTER4			0x00050337		/* arrow_4-1 */
#define CODE_DISP_CAST_POINTER5			0x00050737		/* arrow_5-1 */
#define CODE_DISP_CAST_POINTER6			0x00050b37		/* arrow_6-1 */

#define CODE_KEY_CAST_LINE1				0x00f644d7		/* line1 */
#define CODE_KEY_CAST_LINE2				0x00f644d8		/* line2 */
#define CODE_KEY_CAST_LINE3				0x00f644d9		/* line3 */
#define CODE_KEY_CAST_LINE4				0x00f644da		/* line4 */
#define CODE_KEY_CAST_LINE5				0x00f644db		/* line5 */
#define CODE_KEY_CAST_LINE6				0x00f644dc		/* line6 */


#define CODE_CAST_TITLE					0x0089e61c		/* demo_titleCA */

#define CODE_KEY_CAST_TITLE1			0x00744217		/* demo1 */
#define CODE_KEY_CAST_TITLE2			0x00744218		/* demo2 */
#define CODE_KEY_CAST_TITLE3			0x00744219		/* demo3 */
#define CODE_KEY_CAST_TITLE4			0x0074421a		/* demo4 */
#define CODE_KEY_CAST_TITLE5			0x0074421b		/* demo5 */
#define CODE_KEY_CAST_TITLE6			0x0074421c		/* demo6 */
#define CODE_KEY_CAST_TITLE7			0x0074421d		/* demo7 */
#define CODE_KEY_CAST_TITLE8			0x0074421e		/* demo8 */
#define CODE_KEY_CAST_TITLE9			0x0074421f		/* demo9 */
#define CODE_KEY_CAST_TITLE10			0x0088431e		/* demo10 */


#define CODE_DST6_CAST					0x007d7343		/* chg_line6-1 */

#define CODE_KEY_DST6_SNAKE				0x00789343		/* chgSnake6 */
#define CODE_KEY_DST6_OTHEROT			0x0053174f		/* chgOcelot6 */
#define CODE_KEY_DST6_GOLCOMMAND		0x00c0ec07		/* chgGolhei6 */
#define CODE_KEY_DST6_OLGA				0x002adb29		/* chgOlga6 */
#define CODE_KEY_DST6_GOL				0x00c6dc07		/* chgGoltai6 */
#define CODE_KEY_DST6_SCOTT				0x00667d89		/* chgScot6 */
#define CODE_KEY_DST6_RAIDEN			0x008eef4b		/* chgRaiden6 */
#define CODE_KEY_DST6_VAMP				0x00957509		/* chgVamp6 */
#define CODE_KEY_DST6_SEALS				0x00e89902		/* chgSeals6 */
#define CODE_KEY_DST6_PRISKIN			0x00d0d0e8		/* chgPriskin6 */
#define CODE_KEY_DST6_FORTUNE			0x00e5931a		/* chgFortune6 */
#define CODE_KEY_DST6_SOLIDUS			0x002d98ce		/* chgSolidus6 */
#define CODE_KEY_DST6_ROSE				0x005c8ba9		/* chgRose6 */
#define CODE_KEY_DST6_MGS1_SNAKE		0x00dd860f		/* chgQsnake6 */
#define CODE_KEY_DST6_MGS1_OTHEROT		0x00f170db		/* chgQocelot6 */
#define CODE_KEY_DST6_BUSINESS_MAN		0x002e6c61		/* chgRiman6 */
#define CODE_KEY_DST6_OL				0x00b4ca82		/* chgOl6 */
#define CODE_KEY_DST6_OBA_CHAN			0x00992dec		/* chgOba6 */
#define CODE_KEY_DST6_GOLHITEQ			0x008e07ab		/* chgGolhi6 */
#define CODE_KEY_DST6_NINJA				0x002f0eb9		/* chgNinja6 */
/* 2002.09.10 新規追加 */
#define CODE_KEY_DST6_OTACON		  	0x000e96b1		/* chgOtacon6 */
#define CODE_KEY_DST6_MERYL				0x00f14c16		/* chgMeryl6 */
#define CODE_KEY_DST6_TAXEDO_SNAKE		0x000375ed		/* chgSnake_txd6 */
#define CODE_KEY_DST6_HIPOLY_MGS1_SNAKE	0x00b7e99f		/* chgSnake_hi6 */
#define CODE_KEY_DST6_NINJA_RAIDEN		0x0084925c		/* chgRaiden_ninja6 */

#define CODE_KEY_DST6_QUESTION			0x009a744c		/* chgQst6 */


#define CODE_DST5_CAST					0x007d6f43		/* chg_line5-1 */

#define CODE_KEY_DST5_SNAKE				0x00789342		/* chgSnake5 */
#define CODE_KEY_DST5_OTHEROT			0x0053174e		/* chgOcelot5 */
#define CODE_KEY_DST5_GOLCOMMAND		0x00c0ec06		/* chgGolhei5 */
#define CODE_KEY_DST5_OLGA				0x002adb28		/* chgOlga5 */
#define CODE_KEY_DST5_GOL				0x00c6dc06		/* chgGoltai5 */
#define CODE_KEY_DST5_SCOTT				0x00667d88		/* chgScot5 */
#define CODE_KEY_DST5_RAIDEN			0x008eef4a		/* chgRaiden5 */
#define CODE_KEY_DST5_VAMP				0x00957508		/* chgVamp5 */
#define CODE_KEY_DST5_SEALS				0x00e89901		/* chgSeals5 */
#define CODE_KEY_DST5_PRISKIN			0x00d0d0e7		/* chgPriskin5 */
#define CODE_KEY_DST5_FORTUNE			0x00e59319		/* chgFortune5 */
#define CODE_KEY_DST5_SOLIDUS			0x002d98cd		/* chgSolidus5 */
#define CODE_KEY_DST5_ROSE				0x005c8ba8		/* chgRose5 */
#define CODE_KEY_DST5_MGS1_SNAKE		0x00dd860e		/* chgQsnake5 */
#define CODE_KEY_DST5_MGS1_OTHEROT		0x00f170da		/* chgQocelot5 */
#define CODE_KEY_DST5_BUSINESS_MAN		0x002e6c60		/* chgRiman5 */
#define CODE_KEY_DST5_OL				0x00b4ca81		/* chgOl5 */
#define CODE_KEY_DST5_OBA_CHAN			0x00992deb		/* chgOba5 */
#define CODE_KEY_DST5_GOLHITEQ			0x008e07aa		/* chgGolhi5 */
#define CODE_KEY_DST5_NINJA				0x002f0eb8		/* chgNinja5 */
/* 2002.09.10 新規追加 */
#define CODE_KEY_DST5_OTACON		  	0x000e96b0		/* chgOtacon5 */
#define CODE_KEY_DST5_MERYL				0x00f14c15		/* chgMeryl5 */
#define CODE_KEY_DST5_TAXEDO_SNAKE		0x000375ec		/* chgSnake_txd5 */
#define CODE_KEY_DST5_HIPOLY_MGS1_SNAKE	0x00b7e99e		/* chgSnake_hi5 */
#define CODE_KEY_DST5_NINJA_RAIDEN		0x0084925b		/* chgRaiden_ninja5 */

#define CODE_KEY_DST5_QUESTION			0x009a744b		/* chgQst5 */


#define CODE_DST4_CAST					0x007d6b43		/* chg_line4-1 */

#define CODE_KEY_DST4_SNAKE				0x00789341		/* chgSnake4 */
#define CODE_KEY_DST4_OTHEROT			0x0053174d		/* chgOcelot4 */
#define CODE_KEY_DST4_GOLCOMMAND		0x00c0ec05		/* chgGolhei4 */
#define CODE_KEY_DST4_OLGA				0x002adb27		/* chgOlga4 */
#define CODE_KEY_DST4_GOL				0x00c6dc05		/* chgGoltai4 */
#define CODE_KEY_DST4_SCOTT				0x00667d87		/* chgScot4 */
#define CODE_KEY_DST4_RAIDEN			0x008eef49		/* chgRaiden4 */
#define CODE_KEY_DST4_VAMP				0x00957507		/* chgVamp4 */
#define CODE_KEY_DST4_SEALS				0x00e89900		/* chgSeals4 */
#define CODE_KEY_DST4_PRISKIN			0x00d0d0e6		/* chgPriskin4 */
#define CODE_KEY_DST4_FORTUNE			0x00e59318		/* chgFortune4 */
#define CODE_KEY_DST4_SOLIDUS			0x002d98cc		/* chgSolidus4 */
#define CODE_KEY_DST4_ROSE				0x005c8ba7		/* chgRose4 */
#define CODE_KEY_DST4_MGS1_SNAKE		0x00dd860d		/* chgQsnake4 */
#define CODE_KEY_DST4_MGS1_OTHEROT		0x00f170d9		/* chgQocelot4 */
#define CODE_KEY_DST4_BUSINESS_MAN		0x002e6c5f		/* chgRiman4 */
#define CODE_KEY_DST4_OL				0x00b4ca80		/* chgOl4 */
#define CODE_KEY_DST4_OBA_CHAN			0x00992dea		/* chgOba4 */
#define CODE_KEY_DST4_GOLHITEQ			0x008e07a9		/* chgGolhi4 */
#define CODE_KEY_DST4_NINJA				0x002f0eb7		/* chgNinja4 */
/* 2002.09.10 新規追加 */
#define CODE_KEY_DST4_OTACON		  	0x000e96af		/* chgOtacon4 */
#define CODE_KEY_DST4_MERYL				0x00f14c14		/* chgMeryl4 */
#define CODE_KEY_DST4_TAXEDO_SNAKE		0x000375eb		/* chgSnake_txd4 */
#define CODE_KEY_DST4_HIPOLY_MGS1_SNAKE	0x00b7e99d		/* chgSnake_hi4 */
#define CODE_KEY_DST4_NINJA_RAIDEN		0x0084925a		/* chgRaiden_ninja4 */

#define CODE_KEY_DST4_QUESTION			0x009a744a		/* chgQst4 */


#define CODE_DST3_CAST					0x007d6743		/* chg_line3-1 */

#define CODE_KEY_DST3_SNAKE				0x00789340		/* chgSnake3 */
#define CODE_KEY_DST3_OTHEROT			0x0053174c		/* chgOcelot3 */
#define CODE_KEY_DST3_GOLCOMMAND		0x00c0ec04		/* chgGolhei3 */
#define CODE_KEY_DST3_OLGA				0x002adb26		/* chgOlga3 */
#define CODE_KEY_DST3_GOL				0x00c6dc04		/* chgGoltai3 */
#define CODE_KEY_DST3_SCOTT				0x00667d86		/* chgScot3 */
#define CODE_KEY_DST3_RAIDEN			0x008eef48		/* chgRaiden3 */
#define CODE_KEY_DST3_VAMP				0x00957506		/* chgVamp3 */
#define CODE_KEY_DST3_SEALS				0x00e898ff		/* chgSeals3 */
#define CODE_KEY_DST3_PRISKIN			0x00d0d0e5		/* chgPriskin3 */
#define CODE_KEY_DST3_FORTUNE			0x00e59317		/* chgFortune3 */
#define CODE_KEY_DST3_SOLIDUS			0x002d98cb		/* chgSolidus3 */
#define CODE_KEY_DST3_ROSE				0x005c8ba6		/* chgRose3 */
#define CODE_KEY_DST3_MGS1_SNAKE		0x00dd860c		/* chgQsnake3 */
#define CODE_KEY_DST3_MGS1_OTHEROT		0x00f170d8		/* chgQocelot3 */
#define CODE_KEY_DST3_BUSINESS_MAN		0x002e6c5e		/* chgRiman3 */
#define CODE_KEY_DST3_OL				0x00b4ca7f		/* chgOl3 */
#define CODE_KEY_DST3_OBA_CHAN			0x00992de9		/* chgOba3 */
#define CODE_KEY_DST3_GOLHITEQ			0x008e07a8		/* chgGolhi3 */
#define CODE_KEY_DST3_NINJA				0x002f0eb6		/* chgNinja3 */
/* 2002.09.10 新規追加 */
#define CODE_KEY_DST3_OTACON		  	0x000e96ae		/* chgOtacon3 */
#define CODE_KEY_DST3_MERYL				0x00f14c13		/* chgMeryl3 */
#define CODE_KEY_DST3_TAXEDO_SNAKE		0x000375ea		/* chgSnake_txd3 */
#define CODE_KEY_DST3_HIPOLY_MGS1_SNAKE	0x00b7e99c		/* chgSnake_hi3 */
#define CODE_KEY_DST3_NINJA_RAIDEN		0x00849259		/* chgRaiden_ninja3 */

#define CODE_KEY_DST3_QUESTION			0x009a7449		/* chgQst3 */


#define CODE_DST2_CAST					0x007d6343		/* chg_line2-1 */

#define CODE_KEY_DST2_SNAKE				0x0078933f		/* chgSnake2 */
#define CODE_KEY_DST2_OTHEROT			0x0053174b		/* chgOcelot2 */
#define CODE_KEY_DST2_GOLCOMMAND		0x00c0ec03		/* chgGolhei2 */
#define CODE_KEY_DST2_OLGA				0x002adb25		/* chgOlga2 */
#define CODE_KEY_DST2_GOL				0x00c6dc03		/* chgGoltai2 */
#define CODE_KEY_DST2_SCOTT				0x00667d85		/* chgScot2 */
#define CODE_KEY_DST2_RAIDEN			0x008eef47		/* chgRaiden2 */
#define CODE_KEY_DST2_VAMP				0x00957505		/* chgVamp2 */
#define CODE_KEY_DST2_SEALS				0x00e898fe		/* chgSeals2 */
#define CODE_KEY_DST2_PRISKIN			0x00d0d0e4		/* chgPriskin2 */
#define CODE_KEY_DST2_FORTUNE			0x00e59316		/* chgFortune2 */
#define CODE_KEY_DST2_SOLIDUS			0x002d98ca		/* chgSolidus2 */
#define CODE_KEY_DST2_ROSE				0x005c8ba5		/* chgRose2 */
#define CODE_KEY_DST2_MGS1_SNAKE		0x00dd860b		/* chgQsnake2 */
#define CODE_KEY_DST2_MGS1_OTHEROT		0x00f170d7		/* chgQocelot2 */
#define CODE_KEY_DST2_BUSINESS_MAN		0x002e6c5d		/* chgRiman2 */
#define CODE_KEY_DST2_OL				0x00b4ca7e		/* chgOl2 */
#define CODE_KEY_DST2_OBA_CHAN			0x00992de8		/* chgOba2 */
#define CODE_KEY_DST2_GOLHITEQ			0x008e07a7		/* chgGolhi2 */
#define CODE_KEY_DST2_NINJA				0x002f0eb5		/* chgNinja2 */
/* 2002.09.10 新規追加 */
#define CODE_KEY_DST2_OTACON		  	0x000e96ad		/* chgOtacon2 */
#define CODE_KEY_DST2_MERYL				0x00f14c12		/* chgMeryl2 */
#define CODE_KEY_DST2_TAXEDO_SNAKE		0x000375e9		/* chgSnake_txd2 */
#define CODE_KEY_DST2_HIPOLY_MGS1_SNAKE	0x00b7e99b		/* chgSnake_hi2 */
#define CODE_KEY_DST2_NINJA_RAIDEN		0x00849258		/* chgRaiden_ninja2 */

#define CODE_KEY_DST2_QUESTION			0x009a7448		/* chgQst2 */


#define CODE_DST1_CAST					0x007d5f43		/* chg_line1-1 */

// #define CODE_KEY_DST1_SNAKE				0x0078933e		/* chgSnake1 */
#define CODE_KEY_DST1_SNAKE				0x0084e3be		/* oriSnake1 */
#define CODE_KEY_DST1_OTHEROT			0x0053174a		/* chgOcelot1 */
#define CODE_KEY_DST1_GOLCOMMAND		0x00c0ec02		/* chgGolhei1 */
#define CODE_KEY_DST1_OLGA				0x002adb24		/* chgOlga1 */
#define CODE_KEY_DST1_GOL				0x00c6dc02		/* chgGoltai1 */
#define CODE_KEY_DST1_SCOTT				0x00667d84		/* chgScot1 */
#define CODE_KEY_DST1_RAIDEN			0x008eef46		/* chgRaiden1 */
#define CODE_KEY_DST1_VAMP				0x00957504		/* chgVamp1 */
#define CODE_KEY_DST1_SEALS				0x00e898fd		/* chgSeals1 */
#define CODE_KEY_DST1_PRISKIN			0x00d0d0e3		/* chgPriskin1 */
#define CODE_KEY_DST1_FORTUNE			0x00e59315		/* chgFortune1 */
#define CODE_KEY_DST1_SOLIDUS			0x002d98c9		/* chgSolidus1 */
#define CODE_KEY_DST1_ROSE				0x005c8ba4		/* chgRose1 */
#define CODE_KEY_DST1_MGS1_SNAKE		0x00dd860a		/* chgQsnake1 */
#define CODE_KEY_DST1_MGS1_OTHEROT		0x00f170d6		/* chgQocelot1 */
#define CODE_KEY_DST1_BUSINESS_MAN		0x002e6c5c		/* chgRiman1 */
#define CODE_KEY_DST1_OL				0x00b4ca7d		/* chgOl1 */
#define CODE_KEY_DST1_OBA_CHAN			0x00992de7		/* chgOba1 */
#define CODE_KEY_DST1_GOLHITEQ			0x008e07a6		/* chgGolhi1 */
#define CODE_KEY_DST1_NINJA				0x002f0eb4		/* chgNinja1 */
/* 2002.09.10 新規追加 */
#define CODE_KEY_DST1_OTACON		  	0x000e96ac		/* chgOtacon1 */
#define CODE_KEY_DST1_MERYL				0x00f14c11		/* chgMeryl1 */
#define CODE_KEY_DST1_TAXEDO_SNAKE		0x000375e8		/* chgSnake_txd1 */
#define CODE_KEY_DST1_HIPOLY_MGS1_SNAKE	0x00b7e99a		/* chgSnake_hi1 */
#define CODE_KEY_DST1_NINJA_RAIDEN		0x00849257		/* chgRaiden_ninja1 */

#define CODE_KEY_DST1_QUESTION			0x009a7447		/* chgQst1 */


#define CODE_SRC6_CAST					0x00bf7374		/* ori_line6-1 */

#define CODE_KEY_SRC6_SNAKE				0x0084e3c3		/* oriSnake6 */
#define CODE_KEY_SRC6_OTHEROT			0x00dd2750		/* oriOcelot6 */
#define CODE_KEY_SRC6_GOLCOMMAND		0x004afc09		/* oriGolhei6 */
#define CODE_KEY_SRC6_OLGA				0x002b3dad		/* oriOlga6 */
#define CODE_KEY_SRC6_GOL				0x0050ec09		/* oriGoltai6 */
#define CODE_KEY_SRC6_SCOTT				0x0066e00d		/* oriScot6 */
#define CODE_KEY_SRC6_RAIDEN			0x0018ff4d		/* oriRaiden6 */
#define CODE_KEY_SRC6_VAMP				0x0095d78d		/* oriVamp6 */
#define CODE_KEY_SRC6_SEALS				0x00f4e982		/* oriSeals6 */
#define CODE_KEY_SRC6_PRISKIN			0x0012d11a		/* oriPriskin6 */
#define CODE_KEY_SRC6_FORTUNE			0x0027934c		/* oriFortune6 */
#define CODE_KEY_SRC6_SOLIDUS			0x006f98ff		/* oriSolidus6 */
#define CODE_KEY_SRC6_ROSE				0x005cee2d		/* oriRose6 */
#define CODE_KEY_SRC6_MGS1_SNAKE		0x00679611		/* oriQsnake6 */
#define CODE_KEY_SRC6_MGS1_OTHEROT		0x0033710d		/* oriQocelot6 */
#define CODE_KEY_SRC6_BUSINESS_MAN		0x003abce1		/* oriRiman6 */
#define CODE_KEY_SRC6_OL				0x0055ca9b		/* oriOl6 */
#define CODE_KEY_SRC6_OBA_CHAN			0x00b93100		/* oriOba6 */
#define CODE_KEY_SRC6_GOLHITEQ			0x009a582b		/* oriGolhi6 */
#define CODE_KEY_SRC6_NINJA				0x003b5f39		/* oriNinja6 */


#define CODE_SRC5_CAST					0x00bf6f74		/* ori_line5-1 */

#define CODE_KEY_SRC5_SNAKE				0x0084e3c2		/* oriSnake5 */
#define CODE_KEY_SRC5_OTHEROT			0x00dd274f		/* oriOcelot5 */
#define CODE_KEY_SRC5_GOLCOMMAND		0x004afc08		/* oriGolhei5 */
#define CODE_KEY_SRC5_OLGA				0x002b3dac		/* oriOlga5 */
#define CODE_KEY_SRC5_GOL				0x0050ec08		/* oriGoltai5 */
#define CODE_KEY_SRC5_SCOTT				0x0066e00c		/* oriScot5 */
#define CODE_KEY_SRC5_RAIDEN			0x0018ff4c		/* oriRaiden5 */
#define CODE_KEY_SRC5_VAMP				0x0095d78c		/* oriVamp5 */
#define CODE_KEY_SRC5_SEALS				0x00f4e981		/* oriSeals5 */
#define CODE_KEY_SRC5_PRISKIN			0x0012d119		/* oriPriskin5 */
#define CODE_KEY_SRC5_FORTUNE			0x0027934b		/* oriFortune5 */
#define CODE_KEY_SRC5_SOLIDUS			0x006f98fe		/* oriSolidus5 */
#define CODE_KEY_SRC5_ROSE				0x005cee2c		/* oriRose5 */
#define CODE_KEY_SRC5_MGS1_SNAKE		0x00679610		/* oriQsnake5 */
#define CODE_KEY_SRC5_MGS1_OTHEROT		0x0033710c		/* oriQocelot5 */
#define CODE_KEY_SRC5_BUSINESS_MAN		0x003abce0		/* oriRiman5 */
#define CODE_KEY_SRC5_OL				0x0055ca9a		/* oriOl5 */
#define CODE_KEY_SRC5_OBA_CHAN			0x00b930ff		/* oriOba5 */
#define CODE_KEY_SRC5_GOLHITEQ			0x009a582a		/* oriGolhi5 */
#define CODE_KEY_SRC5_NINJA				0x003b5f38		/* oriNinja5 */


#define CODE_SRC4_CAST					0x00bf6b74		/* ori_line4-1 */

#define CODE_KEY_SRC4_SNAKE				0x0084e3c1		/* oriSnake4 */
#define CODE_KEY_SRC4_OTHEROT			0x00dd274e		/* oriOcelot4 */
#define CODE_KEY_SRC4_GOLCOMMAND		0x004afc07		/* oriGolhei4 */
#define CODE_KEY_SRC4_OLGA				0x002b3dab		/* oriOlga4 */
#define CODE_KEY_SRC4_GOL				0x0050ec07		/* oriGoltai4 */
#define CODE_KEY_SRC4_SCOTT				0x0066e00b		/* oriScot4 */
#define CODE_KEY_SRC4_RAIDEN			0x0018ff4b		/* oriRaiden4 */
#define CODE_KEY_SRC4_VAMP				0x0095d78b		/* oriVamp4 */
#define CODE_KEY_SRC4_SEALS				0x00f4e980		/* oriSeals4 */
#define CODE_KEY_SRC4_PRISKIN			0x0012d118		/* oriPriskin4 */
#define CODE_KEY_SRC4_FORTUNE			0x0027934a		/* oriFortune4 */
#define CODE_KEY_SRC4_SOLIDUS			0x006f98fd		/* oriSolidus4 */
#define CODE_KEY_SRC4_ROSE				0x005cee2b		/* oriRose4 */
#define CODE_KEY_SRC4_MGS1_SNAKE		0x0067960f		/* oriQsnake4 */
#define CODE_KEY_SRC4_MGS1_OTHEROT		0x0033710b		/* oriQocelot4 */
#define CODE_KEY_SRC4_BUSINESS_MAN		0x003abcdf		/* oriRiman4 */
#define CODE_KEY_SRC4_OL				0x0055ca99		/* oriOl4 */
#define CODE_KEY_SRC4_OBA_CHAN			0x00b930fe		/* oriOba4 */
#define CODE_KEY_SRC4_GOLHITEQ			0x009a5829		/* oriGolhi4 */
#define CODE_KEY_SRC4_NINJA				0x003b5f37		/* oriNinja4 */


#define CODE_SRC3_CAST					0x00bf6774		/* ori_line3-1 */

#define CODE_KEY_SRC3_SNAKE				0x0084e3c0		/* oriSnake3 */
#define CODE_KEY_SRC3_OTHEROT			0x00dd274d		/* oriOcelot3 */
#define CODE_KEY_SRC3_GOLCOMMAND		0x004afc06		/* oriGolhei3 */
#define CODE_KEY_SRC3_OLGA				0x002b3daa		/* oriOlga3 */
#define CODE_KEY_SRC3_GOL				0x0050ec06		/* oriGoltai3 */
#define CODE_KEY_SRC3_SCOTT				0x0066e00a		/* oriScot3 */
#define CODE_KEY_SRC3_RAIDEN			0x0018ff4a		/* oriRaiden3 */
#define CODE_KEY_SRC3_VAMP				0x0095d78a		/* oriVamp3 */
#define CODE_KEY_SRC3_SEALS				0x00f4e97f		/* oriSeals3 */
#define CODE_KEY_SRC3_PRISKIN			0x0012d117		/* oriPriskin3 */
#define CODE_KEY_SRC3_FORTUNE			0x00279349		/* oriFortune3 */
#define CODE_KEY_SRC3_SOLIDUS			0x006f98fc		/* oriSolidus3 */
#define CODE_KEY_SRC3_ROSE				0x005cee2a		/* oriRose3 */
#define CODE_KEY_SRC3_MGS1_SNAKE		0x0067960e		/* oriQsnake3 */
#define CODE_KEY_SRC3_MGS1_OTHEROT		0x0033710a		/* oriQocelot3 */
#define CODE_KEY_SRC3_BUSINESS_MAN		0x003abcde		/* oriRiman3 */
#define CODE_KEY_SRC3_OL				0x0055ca98		/* oriOl3 */
#define CODE_KEY_SRC3_OBA_CHAN			0x00b930fd		/* oriOba3 */
#define CODE_KEY_SRC3_GOLHITEQ			0x009a5828		/* oriGolhi3 */
#define CODE_KEY_SRC3_NINJA				0x003b5f36		/* oriNinja3 */


#define CODE_SRC2_CAST					0x00bf6374		/* ori_line2-1 */

#define CODE_KEY_SRC2_SNAKE				0x0084e3bf		/* oriSnake2 */
#define CODE_KEY_SRC2_OTHEROT			0x00dd274c		/* oriOcelot2 */
#define CODE_KEY_SRC2_GOLCOMMAND		0x004afc05		/* oriGolhei2 */
#define CODE_KEY_SRC2_OLGA				0x002b3da9		/* oriOlga2 */
#define CODE_KEY_SRC2_GOL				0x0050ec05		/* oriGoltai2 */
#define CODE_KEY_SRC2_SCOTT				0x0066e009		/* oriScot2 */
#define CODE_KEY_SRC2_RAIDEN			0x0018ff49		/* oriRaiden2 */
#define CODE_KEY_SRC2_VAMP				0x0095d789		/* oriVamp2 */
#define CODE_KEY_SRC2_SEALS				0x00f4e97e		/* oriSeals2 */
#define CODE_KEY_SRC2_PRISKIN			0x0012d116		/* oriPriskin2 */
#define CODE_KEY_SRC2_FORTUNE			0x00279348		/* oriFortune2 */
#define CODE_KEY_SRC2_SOLIDUS			0x006f98fb		/* oriSolidus2 */
#define CODE_KEY_SRC2_ROSE				0x005cee29		/* oriRose2 */
#define CODE_KEY_SRC2_MGS1_SNAKE		0x0067960d		/* oriQsnake2 */
#define CODE_KEY_SRC2_MGS1_OTHEROT		0x00337109		/* oriQocelot2 */
#define CODE_KEY_SRC2_BUSINESS_MAN		0x003abcdd		/* oriRiman2 */
#define CODE_KEY_SRC2_OL				0x0055ca97		/* oriOl2 */
#define CODE_KEY_SRC2_OBA_CHAN			0x00b930fc		/* oriOba2 */
#define CODE_KEY_SRC2_GOLHITEQ			0x009a5827		/* oriGolhi2 */
#define CODE_KEY_SRC2_NINJA				0x003b5f35		/* oriNinja2 */


#define CODE_SRC1_CAST					0x00bf5f74		/* ori_line1-1 */

#define CODE_KEY_SRC1_SNAKE				0x0084e3be		/* oriSnake1 */
#define CODE_KEY_SRC1_OTHEROT			0x00dd274b		/* oriOcelot1 */
#define CODE_KEY_SRC1_GOLCOMMAND		0x004afc04		/* oriGolhei1 */
#define CODE_KEY_SRC1_OLGA				0x002b3da8		/* oriOlga1 */
#define CODE_KEY_SRC1_GOL				0x0050ec04		/* oriGoltai1 */
#define CODE_KEY_SRC1_SCOTT				0x0066e008		/* oriScot1 */
#define CODE_KEY_SRC1_RAIDEN			0x0018ff48		/* oriRaiden1 */
#define CODE_KEY_SRC1_VAMP				0x0095d788		/* oriVamp1 */
#define CODE_KEY_SRC1_SEALS				0x00f4e97d		/* oriSeals1 */
#define CODE_KEY_SRC1_PRISKIN			0x0012d115		/* oriPriskin1 */
#define CODE_KEY_SRC1_FORTUNE			0x00279347		/* oriFortune1 */
#define CODE_KEY_SRC1_SOLIDUS			0x006f98fa		/* oriSolidus1 */
#define CODE_KEY_SRC1_ROSE				0x005cee28		/* oriRose1 */
#define CODE_KEY_SRC1_MGS1_SNAKE		0x0067960c		/* oriQsnake1 */
#define CODE_KEY_SRC1_MGS1_OTHEROT		0x00337108		/* oriQocelot1 */
#define CODE_KEY_SRC1_BUSINESS_MAN		0x003abcdc		/* oriRiman1 */
#define CODE_KEY_SRC1_OL				0x0055ca96		/* oriOl1 */
#define CODE_KEY_SRC1_OBA_CHAN			0x00b930fb		/* oriOba1 */
#define CODE_KEY_SRC1_GOLHITEQ			0x009a5826		/* oriGolhi1 */
#define CODE_KEY_SRC1_NINJA				0x003b5f34		/* oriNinja1 */
#define CODE_KEY_SRC1_DAMMY				0x00000000	/* 数合わせのダミーです */

#define ARROW_FLUSH_COUNT				DIRECT_TICK(2)
#define TITLE_FLUSH_COUNT				DIRECT_TICK(5)


static const struct {
	int name;
	int picpanel;
	int pic;
	int hide_picpanel;
} demotitle[]={
	{
		CODE_KEY_DISP_TITLE1,
		CODE_PICPANEL1,
		CODE_KEY_DISP_PIC1,
		CODE_SPR_PICPANEL2,
	},
	{
		CODE_KEY_DISP_TITLE2,
		CODE_PICPANEL1,
		CODE_KEY_DISP_PIC2,
		CODE_SPR_PICPANEL2,
	},
	{
		CODE_KEY_DISP_TITLE3,
		CODE_PICPANEL1,
		CODE_KEY_DISP_PIC3,
		CODE_SPR_PICPANEL2,
	},
	{
		CODE_KEY_DISP_TITLE4,
		CODE_PICPANEL1,
		CODE_KEY_DISP_PIC4,
		CODE_SPR_PICPANEL2,
	},
	{
		CODE_KEY_DISP_TITLE5,
		CODE_PICPANEL1,
		CODE_KEY_DISP_PIC5,
		CODE_SPR_PICPANEL2,
	},
	{
		CODE_KEY_DISP_TITLE6,
		CODE_PICPANEL2,
		CODE_KEY_DISP_PIC6,
		CODE_SPR_PICPANEL1,
	},
	{
		CODE_KEY_DISP_TITLE7,
		CODE_PICPANEL2,
		CODE_KEY_DISP_PIC7,
		CODE_SPR_PICPANEL1,
	},
	{
		CODE_KEY_DISP_TITLE8,
		CODE_PICPANEL2,
		CODE_KEY_DISP_PIC8,
		CODE_SPR_PICPANEL1,
	},
};


#define REPEAT_FIRST		20
#define REPEAT_NEXT			1

#define DISP_CHANL			TITLE_CHANL
#define MENU_PRIORITY		TITLE_PRI_NORMAL

#define SEL_ALPHA		128
#define UNSEL_ALPHA		52


#define EXPLAIN_RESOURCE		(work->explain_resource)	/* ローポリ劇場解説 */

#define J_EXPLAIN_RESOURCE		0x00e48056		/* ローポリ劇場解説日本語 */
#define E_EXPLAIN_RESOURCE		0x000f8aca		/* ローポリ劇場解説英語 */
#define F_EXPLAIN_RESOURCE		0x00eb9707		/* ローポリ劇場解説フランス語 */
#define G_EXPLAIN_RESOURCE		0x00763e6e		/* ローポリ劇場解説ドイツ語 */
#define I_EXPLAIN_RESOURCE		0x002a3087		/* ローポリ劇場解説イタリア語 */
#define S_EXPLAIN_RESOURCE		0x00e07b42		/* ローポリ劇場解説スペイン語 */

static const int explain_resource_name[]={
	E_EXPLAIN_RESOURCE,
	F_EXPLAIN_RESOURCE,
	G_EXPLAIN_RESOURCE,
	I_EXPLAIN_RESOURCE,
	S_EXPLAIN_RESOURCE,
	0, /* 韓国語 */
	J_EXPLAIN_RESOURCE,
};



#define FONT_WIDTH				SUBTITLE_FONT_WIDTH
#define FONT_HEIGHT				SUBTITLE_FONT_HEIGHT
#define FONT_DISP_WIDTH			SUBTITLE_FONT_DISP_WIDTH
#define FONT_DISP_HEIGHT		SUBTITLE_FONT_DISP_HEIGHT
#define LINE_SPACE				RUBI_SIZE_H
#define LINE_HEIGHT				(FONT_HEIGHT+LINE_SPACE)

#define EXPLAIN_U				0
#define EXPLAIN_V				4
#define EXPLAIN_WIDTH			(30*FONT_WIDTH)
#define EXPLAIN_HEIGHT			(1*LINE_HEIGHT+DOWN_MARGINE)

#define EXPLAIN_X				68
#define EXPLAIN_Y				296
#define EXPLAIN_W				(EXPLAIN_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define EXPLAIN_H				(EXPLAIN_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)

#define TEXTCOLOR				0x80555a50

#define EXPLAIN_RECT			0x009abd00		/* msgRect */


enum {
	INIT_STEP=0,
	DEMO_SELECT_STEP,
	CAST_SELECT_STEP,
	EXIT_STEP,
};


enum {
	CAST_CODE_NONE=-1,

	CAST_CODE_RAIDEN=0,
	CAST_CODE_SOLIDUS,
	CAST_CODE_SNAKE,
	CAST_CODE_OTHEROT,
	CAST_CODE_VAMP,
	CAST_CODE_FORTUNE,
	CAST_CODE_ROSE,
	CAST_CODE_MGS1_SNAKE,
	CAST_CODE_MGS1_OTHEROT,
	CAST_CODE_BUSINESS_MAN,
	CAST_CODE_OL,
	CAST_CODE_OBACHAN,
	CAST_CODE_HIGHTEQ,

	CAST_CODE_NINJA,

	/* 2002.09.10 新規追加分キャラ */
	CAST_CODE_OTACON,
	CAST_CODE_MERYL,
	CAST_CODE_TAXEDO_SANKE,
	CAST_CODE_HIPOLY_MGS1_SNAKE,
	CAST_CODE_NINJA_RAIDEN,
	
	CAST_CODE_MAX,

	EXCAST_CODE_GOLCOMMAND=CAST_CODE_MAX,
	EXCAST_CODE_OLGA,
	EXCAST_CODE_GOL,
	EXCAST_CODE_SCOTT,
	EXCAST_CODE_SEALS,
	EXCAST_CODE_PRISKIN,

	EXCAST_CODE_QUESTION,

	EXCAST_CODE_MAX,
};


#define DEF_ENSELCAST	(((1<<CAST_CODE_MAX)-1)|(1<<EXCAST_CODE_QUESTION))


static const struct {
	int title;
	int n_casts;
	int defcast[N_CASTS];
	int enselcast[N_CASTS];
	char *name;
} demodata[]={
	{
		CODE_KEY_CAST_TITLE1,
		2,
		{
			CAST_CODE_SNAKE,
			CAST_CODE_OTHEROT,
			CAST_CODE_NONE,
			CAST_CODE_NONE,
		},
		{
			DEF_ENSELCAST,
			DEF_ENSELCAST,
			DEF_ENSELCAST,
			DEF_ENSELCAST,
		},
		"DEMO 1",
	},
	{
		CODE_KEY_CAST_TITLE2,
		2,
		{
			CAST_CODE_SNAKE,
			EXCAST_CODE_GOLCOMMAND,
			CAST_CODE_NONE,
			CAST_CODE_NONE,
		},
		{
			DEF_ENSELCAST,
			DEF_ENSELCAST|(1<<EXCAST_CODE_GOLCOMMAND),
			DEF_ENSELCAST,
			DEF_ENSELCAST,
		},
		"DEMO 2",
	},
	{
		CODE_KEY_CAST_TITLE3,
		3,
		{
			CAST_CODE_SNAKE,
			EXCAST_CODE_OLGA,
			EXCAST_CODE_GOL,
			CAST_CODE_NONE,
		},
		{
			DEF_ENSELCAST,
			DEF_ENSELCAST|(1<<EXCAST_CODE_OLGA),
			DEF_ENSELCAST|(1<<EXCAST_CODE_GOL),
			DEF_ENSELCAST,
		},
		"DEMO 3",
	},
	{
		CODE_KEY_CAST_TITLE4,
		4,
		{
			CAST_CODE_SNAKE,
			CAST_CODE_OTHEROT,
			EXCAST_CODE_SCOTT,
			EXCAST_CODE_GOL,
		},
		{
			DEF_ENSELCAST,
			DEF_ENSELCAST,
			DEF_ENSELCAST|(1<<EXCAST_CODE_SCOTT),
			DEF_ENSELCAST|(1<<EXCAST_CODE_GOL),
		},
		"DEMO 4",
	},
	{
		CODE_KEY_CAST_TITLE5,
		4,
		{
			CAST_CODE_RAIDEN,
			CAST_CODE_VAMP,
			EXCAST_CODE_SEALS,
			EXCAST_CODE_PRISKIN,
		},
		{
			DEF_ENSELCAST,
			DEF_ENSELCAST,
			DEF_ENSELCAST|(1<<EXCAST_CODE_SEALS),
			DEF_ENSELCAST|(1<<EXCAST_CODE_PRISKIN),
		},
		"DEMO 5",
	},
	{
		CODE_KEY_CAST_TITLE6,
		4,
		{
			CAST_CODE_RAIDEN,
			CAST_CODE_VAMP,
			CAST_CODE_FORTUNE,
			EXCAST_CODE_SEALS,
		},
		{
			DEF_ENSELCAST,
			DEF_ENSELCAST,
			DEF_ENSELCAST,
			DEF_ENSELCAST|(1<<EXCAST_CODE_SEALS),
		},
		"DEMO 6",
	},
	{
		CODE_KEY_CAST_TITLE7,
		1,
		{
			CAST_CODE_SOLIDUS,
			CAST_CODE_NONE,
			CAST_CODE_NONE,
			CAST_CODE_NONE,
		},
		{
			DEF_ENSELCAST,
			DEF_ENSELCAST,
			DEF_ENSELCAST,
			DEF_ENSELCAST,
		},
		"DEMO 7",
	},
	{
		CODE_KEY_CAST_TITLE8,
		2,
		{
			CAST_CODE_RAIDEN,
			CAST_CODE_SOLIDUS,
			CAST_CODE_NONE,
			CAST_CODE_NONE,
		},
		{
			DEF_ENSELCAST,
			DEF_ENSELCAST,
			DEF_ENSELCAST,
			DEF_ENSELCAST,
		},
		"DEMO 8",
	},
};

static const int arrow_point[]={
	CODE_KEY_CAST_LINE1,
	CODE_KEY_CAST_LINE2,
	CODE_KEY_CAST_LINE3,
	CODE_KEY_CAST_LINE4,
	CODE_KEY_CAST_LINE5,
	CODE_KEY_CAST_LINE6,
};

static const int disp_arrow_point[]={
	CODE_DISP_CAST_POINTER1,
	CODE_DISP_CAST_POINTER2,
	CODE_DISP_CAST_POINTER3,
	CODE_DISP_CAST_POINTER4,
	CODE_DISP_CAST_POINTER5,
	CODE_DISP_CAST_POINTER6,
};

static const int cast_panel_srcname[]={
	CODE_SRC1_CAST,
	CODE_SRC2_CAST,
	CODE_SRC3_CAST,
	CODE_SRC4_CAST,
	CODE_SRC5_CAST,
	CODE_SRC6_CAST,
};

static const int cast_panel_dstname[]={
	CODE_DST1_CAST,
	CODE_DST2_CAST,
	CODE_DST3_CAST,
	CODE_DST4_CAST,
	CODE_DST5_CAST,
	CODE_DST6_CAST,
};

static int cast_key_srcname[][EXCAST_CODE_MAX]={
	{
		CODE_KEY_SRC1_RAIDEN,
		CODE_KEY_SRC1_SOLIDUS,
		CODE_KEY_SRC1_SNAKE,
		CODE_KEY_SRC1_OTHEROT,
		CODE_KEY_SRC1_VAMP,
		CODE_KEY_SRC1_FORTUNE,
		CODE_KEY_SRC1_ROSE,
		CODE_KEY_SRC1_MGS1_SNAKE,
		CODE_KEY_SRC1_MGS1_OTHEROT,
		CODE_KEY_SRC1_BUSINESS_MAN,
		CODE_KEY_SRC1_OL,
		CODE_KEY_SRC1_OBA_CHAN,
		CODE_KEY_SRC1_GOLHITEQ,

		CODE_KEY_SRC1_NINJA,
		/* 新規追加分のダミー */
		CODE_KEY_SRC1_DAMMY,
		CODE_KEY_SRC1_DAMMY,
		CODE_KEY_SRC1_DAMMY,
		CODE_KEY_SRC1_DAMMY,
		CODE_KEY_SRC1_DAMMY,

		CODE_KEY_SRC1_GOLCOMMAND,
		CODE_KEY_SRC1_OLGA,
		CODE_KEY_SRC1_GOL,
		CODE_KEY_SRC1_SCOTT,
		CODE_KEY_SRC1_SEALS,
		CODE_KEY_SRC1_PRISKIN,
	},
	{
		CODE_KEY_SRC2_RAIDEN,
		CODE_KEY_SRC2_SOLIDUS,
		CODE_KEY_SRC2_SNAKE,
		CODE_KEY_SRC2_OTHEROT,
		CODE_KEY_SRC2_VAMP,
		CODE_KEY_SRC2_FORTUNE,
		CODE_KEY_SRC2_ROSE,
		CODE_KEY_SRC2_MGS1_SNAKE,
		CODE_KEY_SRC2_MGS1_OTHEROT,
		CODE_KEY_SRC2_BUSINESS_MAN,
		CODE_KEY_SRC2_OL,
		CODE_KEY_SRC2_OBA_CHAN,
		CODE_KEY_SRC2_GOLHITEQ,

		CODE_KEY_SRC2_NINJA,
		/* 新規追加分のダミー */
		CODE_KEY_SRC1_DAMMY,
		CODE_KEY_SRC1_DAMMY,
		CODE_KEY_SRC1_DAMMY,
		CODE_KEY_SRC1_DAMMY,
		CODE_KEY_SRC1_DAMMY,

		CODE_KEY_SRC2_GOLCOMMAND,
		CODE_KEY_SRC2_OLGA,
		CODE_KEY_SRC2_GOL,
		CODE_KEY_SRC2_SCOTT,
		CODE_KEY_SRC2_SEALS,
		CODE_KEY_SRC2_PRISKIN,
	},
	{
		CODE_KEY_SRC3_RAIDEN,
		CODE_KEY_SRC3_SOLIDUS,
		CODE_KEY_SRC3_SNAKE,
		CODE_KEY_SRC3_OTHEROT,
		CODE_KEY_SRC3_VAMP,
		CODE_KEY_SRC3_FORTUNE,
		CODE_KEY_SRC3_ROSE,
		CODE_KEY_SRC3_MGS1_SNAKE,
		CODE_KEY_SRC3_MGS1_OTHEROT,
		CODE_KEY_SRC3_BUSINESS_MAN,
		CODE_KEY_SRC3_OL,
		CODE_KEY_SRC3_OBA_CHAN,
		CODE_KEY_SRC3_GOLHITEQ,

		CODE_KEY_SRC3_NINJA,
		/* 新規追加分のダミー */
		CODE_KEY_SRC1_DAMMY,
		CODE_KEY_SRC1_DAMMY,
		CODE_KEY_SRC1_DAMMY,
		CODE_KEY_SRC1_DAMMY,
		CODE_KEY_SRC1_DAMMY,

		CODE_KEY_SRC3_GOLCOMMAND,
		CODE_KEY_SRC3_OLGA,
		CODE_KEY_SRC3_GOL,
		CODE_KEY_SRC3_SCOTT,
		CODE_KEY_SRC3_SEALS,
		CODE_KEY_SRC3_PRISKIN,
	},
	{
		CODE_KEY_SRC4_RAIDEN,
		CODE_KEY_SRC4_SOLIDUS,
		CODE_KEY_SRC4_SNAKE,
		CODE_KEY_SRC4_OTHEROT,
		CODE_KEY_SRC4_VAMP,
		CODE_KEY_SRC4_FORTUNE,
		CODE_KEY_SRC4_ROSE,
		CODE_KEY_SRC4_MGS1_SNAKE,
		CODE_KEY_SRC4_MGS1_OTHEROT,
		CODE_KEY_SRC4_BUSINESS_MAN,
		CODE_KEY_SRC4_OL,
		CODE_KEY_SRC4_OBA_CHAN,
		CODE_KEY_SRC4_GOLHITEQ,

		CODE_KEY_SRC4_NINJA,
		/* 新規追加分のダミー */
		CODE_KEY_SRC1_DAMMY,
		CODE_KEY_SRC1_DAMMY,
		CODE_KEY_SRC1_DAMMY,
		CODE_KEY_SRC1_DAMMY,
		CODE_KEY_SRC1_DAMMY,

		CODE_KEY_SRC4_GOLCOMMAND,
		CODE_KEY_SRC4_OLGA,
		CODE_KEY_SRC4_GOL,
		CODE_KEY_SRC4_SCOTT,
		CODE_KEY_SRC4_SEALS,
		CODE_KEY_SRC4_PRISKIN,
	},
	{
		CODE_KEY_SRC5_RAIDEN,
		CODE_KEY_SRC5_SOLIDUS,
		CODE_KEY_SRC5_SNAKE,
		CODE_KEY_SRC5_OTHEROT,
		CODE_KEY_SRC5_VAMP,
		CODE_KEY_SRC5_FORTUNE,
		CODE_KEY_SRC5_ROSE,
		CODE_KEY_SRC5_MGS1_SNAKE,
		CODE_KEY_SRC5_MGS1_OTHEROT,
		CODE_KEY_SRC5_BUSINESS_MAN,
		CODE_KEY_SRC5_OL,
		CODE_KEY_SRC5_OBA_CHAN,
		CODE_KEY_SRC5_GOLHITEQ,

		CODE_KEY_SRC5_NINJA,
		/* 新規追加分のダミー */
		CODE_KEY_SRC1_DAMMY,
		CODE_KEY_SRC1_DAMMY,
		CODE_KEY_SRC1_DAMMY,
		CODE_KEY_SRC1_DAMMY,
		CODE_KEY_SRC1_DAMMY,

		CODE_KEY_SRC5_GOLCOMMAND,
		CODE_KEY_SRC5_OLGA,
		CODE_KEY_SRC5_GOL,
		CODE_KEY_SRC5_SCOTT,
		CODE_KEY_SRC5_SEALS,
		CODE_KEY_SRC5_PRISKIN,
	},
	{
		CODE_KEY_SRC6_RAIDEN,
		CODE_KEY_SRC6_SOLIDUS,
		CODE_KEY_SRC6_SNAKE,
		CODE_KEY_SRC6_OTHEROT,
		CODE_KEY_SRC6_VAMP,
		CODE_KEY_SRC6_FORTUNE,
		CODE_KEY_SRC6_ROSE,
		CODE_KEY_SRC6_MGS1_SNAKE,
		CODE_KEY_SRC6_MGS1_OTHEROT,
		CODE_KEY_SRC6_BUSINESS_MAN,
		CODE_KEY_SRC6_OL,
		CODE_KEY_SRC6_OBA_CHAN,
		CODE_KEY_SRC6_GOLHITEQ,

		CODE_KEY_SRC6_NINJA,
		/* 新規追加分のダミー */
		CODE_KEY_SRC1_DAMMY,
		CODE_KEY_SRC1_DAMMY,
		CODE_KEY_SRC1_DAMMY,
		CODE_KEY_SRC1_DAMMY,
		CODE_KEY_SRC1_DAMMY,

		CODE_KEY_SRC6_GOLCOMMAND,
		CODE_KEY_SRC6_OLGA,
		CODE_KEY_SRC6_GOL,
		CODE_KEY_SRC6_SCOTT,
		CODE_KEY_SRC6_SEALS,
		CODE_KEY_SRC6_PRISKIN,
	},
};

static int cast_key_dstname[][EXCAST_CODE_MAX]={
	{
		CODE_KEY_DST1_RAIDEN,
		CODE_KEY_DST1_SOLIDUS,
		CODE_KEY_DST1_SNAKE,
		CODE_KEY_DST1_OTHEROT,
		CODE_KEY_DST1_VAMP,
		CODE_KEY_DST1_FORTUNE,
		CODE_KEY_DST1_ROSE,
		CODE_KEY_DST1_MGS1_SNAKE,
		CODE_KEY_DST1_MGS1_OTHEROT,
		CODE_KEY_DST1_BUSINESS_MAN,
		CODE_KEY_DST1_OL,
		CODE_KEY_DST1_OBA_CHAN,
		CODE_KEY_DST1_GOLHITEQ,

		CODE_KEY_DST1_NINJA,
		/* 2002.09.10 add */
		CODE_KEY_DST1_OTACON,
		CODE_KEY_DST1_MERYL,
		CODE_KEY_DST1_TAXEDO_SNAKE,
		CODE_KEY_DST1_HIPOLY_MGS1_SNAKE,
		CODE_KEY_DST1_NINJA_RAIDEN,

		CODE_KEY_DST1_GOLCOMMAND,
		CODE_KEY_DST1_OLGA,
		CODE_KEY_DST1_GOL,
		CODE_KEY_DST1_SCOTT,
		CODE_KEY_DST1_SEALS,
		CODE_KEY_DST1_PRISKIN,
		CODE_KEY_DST1_QUESTION,
	},
	{
		CODE_KEY_DST2_RAIDEN,
		CODE_KEY_DST2_SOLIDUS,
		CODE_KEY_DST2_SNAKE,
		CODE_KEY_DST2_OTHEROT,
		CODE_KEY_DST2_VAMP,
		CODE_KEY_DST2_FORTUNE,
		CODE_KEY_DST2_ROSE,
		CODE_KEY_DST2_MGS1_SNAKE,
		CODE_KEY_DST2_MGS1_OTHEROT,
		CODE_KEY_DST2_BUSINESS_MAN,
		CODE_KEY_DST2_OL,
		CODE_KEY_DST2_OBA_CHAN,
		CODE_KEY_DST2_GOLHITEQ,

		CODE_KEY_DST2_NINJA,
		/* 2002.09.10 add */
		CODE_KEY_DST2_OTACON,
		CODE_KEY_DST2_MERYL,
		CODE_KEY_DST2_TAXEDO_SNAKE,
		CODE_KEY_DST2_HIPOLY_MGS1_SNAKE,
		CODE_KEY_DST2_NINJA_RAIDEN,

		CODE_KEY_DST2_GOLCOMMAND,
		CODE_KEY_DST2_OLGA,
		CODE_KEY_DST2_GOL,
		CODE_KEY_DST2_SCOTT,
		CODE_KEY_DST2_SEALS,
		CODE_KEY_DST2_PRISKIN,
		CODE_KEY_DST2_QUESTION,
	},
	{
		CODE_KEY_DST3_RAIDEN,
		CODE_KEY_DST3_SOLIDUS,
		CODE_KEY_DST3_SNAKE,
		CODE_KEY_DST3_OTHEROT,
		CODE_KEY_DST3_VAMP,
		CODE_KEY_DST3_FORTUNE,
		CODE_KEY_DST3_ROSE,
		CODE_KEY_DST3_MGS1_SNAKE,
		CODE_KEY_DST3_MGS1_OTHEROT,
		CODE_KEY_DST3_BUSINESS_MAN,
		CODE_KEY_DST3_OL,
		CODE_KEY_DST3_OBA_CHAN,
		CODE_KEY_DST3_GOLHITEQ,

		CODE_KEY_DST3_NINJA,
		/* 2002.09.10 add */
		CODE_KEY_DST3_OTACON,
		CODE_KEY_DST3_MERYL,
		CODE_KEY_DST3_TAXEDO_SNAKE,
		CODE_KEY_DST3_HIPOLY_MGS1_SNAKE,
		CODE_KEY_DST3_NINJA_RAIDEN,

		CODE_KEY_DST3_GOLCOMMAND,
		CODE_KEY_DST3_OLGA,
		CODE_KEY_DST3_GOL,
		CODE_KEY_DST3_SCOTT,
		CODE_KEY_DST3_SEALS,
		CODE_KEY_DST3_PRISKIN,
		CODE_KEY_DST3_QUESTION,
	},
	{
		CODE_KEY_DST4_RAIDEN,
		CODE_KEY_DST4_SOLIDUS,
		CODE_KEY_DST4_SNAKE,
		CODE_KEY_DST4_OTHEROT,
		CODE_KEY_DST4_VAMP,
		CODE_KEY_DST4_FORTUNE,
		CODE_KEY_DST4_ROSE,
		CODE_KEY_DST4_MGS1_SNAKE,
		CODE_KEY_DST4_MGS1_OTHEROT,
		CODE_KEY_DST4_BUSINESS_MAN,
		CODE_KEY_DST4_OL,
		CODE_KEY_DST4_OBA_CHAN,
		CODE_KEY_DST4_GOLHITEQ,

		CODE_KEY_DST4_NINJA,
		/* 2002.09.10 add */
		CODE_KEY_DST4_OTACON,
		CODE_KEY_DST4_MERYL,
		CODE_KEY_DST4_TAXEDO_SNAKE,
		CODE_KEY_DST4_HIPOLY_MGS1_SNAKE,
		CODE_KEY_DST4_NINJA_RAIDEN,

		CODE_KEY_DST4_GOLCOMMAND,
		CODE_KEY_DST4_OLGA,
		CODE_KEY_DST4_GOL,
		CODE_KEY_DST4_SCOTT,
		CODE_KEY_DST4_SEALS,
		CODE_KEY_DST4_PRISKIN,
		CODE_KEY_DST4_QUESTION,
	},
	{
		CODE_KEY_DST5_RAIDEN,
		CODE_KEY_DST5_SOLIDUS,
		CODE_KEY_DST5_SNAKE,
		CODE_KEY_DST5_OTHEROT,
		CODE_KEY_DST5_VAMP,
		CODE_KEY_DST5_FORTUNE,
		CODE_KEY_DST5_ROSE,
		CODE_KEY_DST5_MGS1_SNAKE,
		CODE_KEY_DST5_MGS1_OTHEROT,
		CODE_KEY_DST5_BUSINESS_MAN,
		CODE_KEY_DST5_OL,
		CODE_KEY_DST5_OBA_CHAN,
		CODE_KEY_DST5_GOLHITEQ,

		CODE_KEY_DST5_NINJA,
		/* 2002.09.10 add */
		CODE_KEY_DST5_OTACON,
		CODE_KEY_DST5_MERYL,
		CODE_KEY_DST5_TAXEDO_SNAKE,
		CODE_KEY_DST5_HIPOLY_MGS1_SNAKE,
		CODE_KEY_DST5_NINJA_RAIDEN,

		CODE_KEY_DST5_GOLCOMMAND,
		CODE_KEY_DST5_OLGA,
		CODE_KEY_DST5_GOL,
		CODE_KEY_DST5_SCOTT,
		CODE_KEY_DST5_SEALS,
		CODE_KEY_DST5_PRISKIN,
		CODE_KEY_DST5_QUESTION,
	},
	{
		CODE_KEY_DST6_RAIDEN,
		CODE_KEY_DST6_SOLIDUS,
		CODE_KEY_DST6_SNAKE,
		CODE_KEY_DST6_OTHEROT,
		CODE_KEY_DST6_VAMP,
		CODE_KEY_DST6_FORTUNE,
		CODE_KEY_DST6_ROSE,
		CODE_KEY_DST6_MGS1_SNAKE,
		CODE_KEY_DST6_MGS1_OTHEROT,
		CODE_KEY_DST6_BUSINESS_MAN,
		CODE_KEY_DST6_OL,
		CODE_KEY_DST6_OBA_CHAN,
		CODE_KEY_DST6_GOLHITEQ,

		CODE_KEY_DST6_NINJA,
		/* 2002.09.10 add */
		CODE_KEY_DST6_OTACON,
		CODE_KEY_DST6_MERYL,
		CODE_KEY_DST6_TAXEDO_SNAKE,
		CODE_KEY_DST6_HIPOLY_MGS1_SNAKE,
		CODE_KEY_DST6_NINJA_RAIDEN,

		CODE_KEY_DST6_GOLCOMMAND,
		CODE_KEY_DST6_OLGA,
		CODE_KEY_DST6_GOL,
		CODE_KEY_DST6_SCOTT,
		CODE_KEY_DST6_SEALS,
		CODE_KEY_DST6_PRISKIN,
		CODE_KEY_DST6_QUESTION,
	},
};


/* ------------------------------------------------------------------------ */


static void ShuffleCast0(Work *work,int i)
{
	int enselcast=work->enselcast_flags[work->demo_cursor][i];
	int cast_array[EXCAST_CODE_MAX];
	int n_cast_array=0;
	int j;

	for(j=0;j<EXCAST_CODE_MAX-1;j++){
		if(enselcast & (1<<j)){
			cast_array[n_cast_array]=j;
			n_cast_array++;
		}
	}

	if(n_cast_array==0){

#ifdef DEBUG_MODE
		printf("0000\n");
#endif

		return;
	}

	if(n_cast_array==1){

#ifdef DEBUG_MODE
		printf("1111\n");
#endif

		work->cast_item_cursor[i]=cast_array[0];
	}
	else{
		int pre_value=work->cast_item_cursor[i];

		do{
			work->cast_item_cursor[i]=cast_array[(irnd()>>16) % n_cast_array];
		} while(work->cast_item_cursor[i]==pre_value);
	}

	// TEST
	// work->cast_item_cursor[i]=demodata[work->demo_cursor].defcast[i];
}

static void ShuffleCast(Work *work)
{
	int i;
	for(i=0;i<demodata[work->demo_cursor].n_casts;i++){
		// ShuffleCast0(work,i);
		work->cast_item_cursor[i]=EXCAST_CODE_QUESTION;
	}
}

static void CallLocalProc(Work *work)
{
	if(work->proc!=0 && work->proc!=1){
		extern void init_rnd( int x );
		GCL_ARGS arg;
		int argv[N_PROC_ARGS];

		init_rnd(GV_Time);

		arg.argc=sizeof(argv)/sizeof(argv[0]);
		arg.argv=argv;

		argv[0]=work->demo_cursor+1;
		if(work->ans==-1){
			argv[0]=-1;
		}

		if(work->cast_item_cursor[0]==EXCAST_CODE_QUESTION){
			ShuffleCast0(work,0);
		}
		if(work->cast_item_cursor[0]==demodata[work->demo_cursor].defcast[0]){
			argv[1]=0;
			// argv[1]=work->cast_item_cursor[0]+1;
		}
		else{
			argv[1]=work->cast_item_cursor[0]+1;
		}
		if(argv[1]<0 || argv[1]>=CAST_CODE_MAX+1){
			argv[1]=0;
		}

		if(work->cast_item_cursor[1]==EXCAST_CODE_QUESTION){
			ShuffleCast0(work,1);
		}
		if(work->cast_item_cursor[1]==demodata[work->demo_cursor].defcast[1]){
			argv[2]=0;
			// argv[2]=work->cast_item_cursor[1]+1;
		}
		else{
			argv[2]=work->cast_item_cursor[1]+1;
		}
		if(argv[2]<0 || argv[2]>=CAST_CODE_MAX+1){
			argv[2]=0;
		}

		if(work->cast_item_cursor[2]==EXCAST_CODE_QUESTION){
			ShuffleCast0(work,2);
		}
		if(work->cast_item_cursor[2]==demodata[work->demo_cursor].defcast[2]){
			argv[3]=0;
			// argv[3]=work->cast_item_cursor[2]+1;
		}
		else{
			argv[3]=work->cast_item_cursor[2]+1;
		}
		if(argv[3]<0 || argv[3]>=CAST_CODE_MAX+1){
			argv[3]=0;
		}

		if(work->cast_item_cursor[3]==EXCAST_CODE_QUESTION){
			ShuffleCast0(work,3);
		}
		if(work->cast_item_cursor[3]==demodata[work->demo_cursor].defcast[3]){
			argv[4]=0;
			// argv[4]=work->cast_item_cursor[3]+1;
		}
		else{
			argv[4]=work->cast_item_cursor[3]+1;
		}
		if(argv[4]<0 || argv[4]>=CAST_CODE_MAX+1){
			argv[4]=0;
		}

#ifdef DEBUG_MODE
		printf("theater : %d %d %d %d\n",argv[1],argv[2],argv[3],argv[4]);
#endif

      bp_trophy_demo_theater_play( N_PROC_ARGS, argv );
		GCL_ExecProc(work->proc,&arg);
	}
}

static void ProgMessage(Work *work)
{

#if 0
    GV_MSG *msg;
    int n_msg;

    n_msg=GV_ReceiveMessage(work->name,&msg);

    while(n_msg>0){
		int command=*(msg->message+0);
		int arg=*(msg->message+1);

		msg++;
		n_msg--;
    }
#endif

}

static void Key(Work *work)
{
	int status=GV_PadDataDirect[0].status & (PAD_U|PAD_D|PAD_L|PAD_R|PAD_L1|PAD_R1|PAD_OK|PAD_CANCEL);
	int press=GV_PadDataDirect[0].press & (PAD_U|PAD_D|PAD_L|PAD_R|PAD_L1|PAD_R1|PAD_OK|PAD_CANCEL);

	work->key_press=press;

	if(status && status==work->key_status){
		if(work->key_count>REPEAT_FIRST){
			work->key_count-=REPEAT_NEXT;
			work->key_autostatus=work->key_status;
			work->key_count = 0;
		}
		else work->key_autostatus=0;

		work->key_count++;
	}
	else{
		work->key_autostatus=work->key_status=status;
		work->key_count=0;
	}
}


/* ------------------------------------------------------------------------ */


#if 0


#define START_LEFT		30
#define START_UPPER		30

#define FONT_WIDTH		9
#define LINE_HEIGHT		17


static const char *disp_castname[]={
	"RAIDEN",
	"SOLIDUS",
	"SNAKE",
	"OTHEROT",
	"VAMP",
	"FORTUNE",
	"ROSE",
	"MGS1 SNAKE",
	"MGS1 OTHEROT",
	"BUSINESS MAN",
	"OL",
	"OBA-CHAN",
	"HIGHTEQ",

	"GOL-COMMAND",
	"OLGA",
	"GOL",
	"SCOTT",
	"SEALS",
	"PRISKIN",

	"QUESTION",
};

static void DispDemoname(Work *work)
{
	int i;
	int x,y;

	x=START_LEFT;
	y=START_UPPER;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("DEMO SELECT");

	x+=FONT_WIDTH*2;
	y+=LINE_HEIGHT;
	for(i=0;i<N_DEMOS;i++){
		DEBUG_Locate(x,y+i*LINE_HEIGHT,MENU_MODE_NORMAL);
		DEBUG_Printf(demodata[i].name);
	}

	x-=FONT_WIDTH*2;
	DEBUG_Locate(x,y+work->demo_cursor*LINE_HEIGHT,MENU_MODE_NORMAL);
	DEBUG_Printf(">>");
}

static void DispCastname(Work *work)
{
	int i;
	int x,y;

	x=START_LEFT;
	y=START_UPPER;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("CAST SELECT");

	y+=LINE_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("DEMO NAME : %s",demodata[work->demo_cursor].name);

	x+=FONT_WIDTH*2;
	y+=LINE_HEIGHT;
	for(i=0;i<demodata[work->demo_cursor].n_casts;i++){
		DEBUG_Locate(x,y+i*LINE_HEIGHT,MENU_MODE_NORMAL);
		DEBUG_Printf(disp_castname[demodata[work->demo_cursor].defcast[i]]);

		DEBUG_Locate(x+FONT_WIDTH*12,y+i*LINE_HEIGHT,MENU_MODE_NORMAL);
		DEBUG_Printf("--> %s",disp_castname[work->cast_item_cursor[i]]);
	}

	x-=FONT_WIDTH*2;
	DEBUG_Locate(x,y+work->cast_cursor*LINE_HEIGHT,MENU_MODE_NORMAL);
	DEBUG_Printf(">>");
}

#endif



static void ChangeSelectedDemo(Work *work)
{
	SPR_OBJ *spr;
	void *part;

	part=L2D_GetParts(work->l2d_handle,CODE_PIC_TITLE);
	if(part!=NULL){
		L2D_MorfObject(part,
					   demotitle[work->demo_cursor].name,
					   demotitle[work->demo_cursor].name,1.0f);
	}

	part=L2D_GetParts(work->l2d_handle,demotitle[work->demo_cursor].picpanel);
	if(part!=NULL){
		L2D_MorfObject(part,
					   demotitle[work->demo_cursor].pic,
					   demotitle[work->demo_cursor].pic,1.0f);
	}

	spr=L2D_GetObject(work->l2d_handle,demotitle[work->demo_cursor].hide_picpanel);
	if(spr!=NULL) SPR_HIDE(spr);
}

static void TitleHighlight(Work *work)
{
	void *part;

	part=L2D_GetParts(work->l2d_handle,CODE_PIC_TITLE_LIGHT);
	if(part!=NULL){
		L2D_MorfObject(part,
					   CODE_KEY_TITLE_HIGHLIGHT,
					   CODE_KEY_TITLE_HIGHLIGHT,1.0f);
	}
}

static void TitleNormal(Work *work)
{
	void *part;

	part=L2D_GetParts(work->l2d_handle,CODE_PIC_TITLE_LIGHT);
	if(part!=NULL){
		L2D_MorfObject(part,
					   CODE_KEY_TITLE_NORMAL,
					   CODE_KEY_TITLE_NORMAL,1.0f);
	}
}

static void RightArrowHighlight(Work *work)
{
	void *part;

	part=L2D_GetParts(work->l2d_handle,CODE_RARROW);
	if(part!=NULL){
		L2D_MorfObject(part,
					   CODE_KEY_RARROW_HIGHLIGHT,
					   CODE_KEY_RARROW_HIGHLIGHT,1.0f);
	}
}

static void RightArrowNormal(Work *work)
{
	void *part;

	part=L2D_GetParts(work->l2d_handle,CODE_RARROW);
	if(part!=NULL){
		L2D_MorfObject(part,
					   CODE_KEY_RARROW_NORMAL,
					   CODE_KEY_RARROW_NORMAL,1.0f);
	}
}

static void LeftArrowHighlight(Work *work)
{
	void *part;

	part=L2D_GetParts(work->l2d_handle,CODE_LARROW);
	if(part!=NULL){
		L2D_MorfObject(part,
					   CODE_KEY_LARROW_HIGHLIGHT,
					   CODE_KEY_LARROW_HIGHLIGHT,1.0f);
	}
}

static void LeftArrowNormal(Work *work)
{
	void *part;

	part=L2D_GetParts(work->l2d_handle,CODE_LARROW);
	if(part!=NULL){
		L2D_MorfObject(part,
					   CODE_KEY_LARROW_NORMAL,
					   CODE_KEY_LARROW_NORMAL,1.0f);
	}
}


static void CreateExplain(Work *work,int index)
{
	if(!work->explain_flag){
		char *str;

printf("get res [%d]\n",EXPLAIN_RESOURCE);
		str=BP_GCL_LOOKUP_NEW_FONT_STRING((char *)GetLocalResource(EXPLAIN_RESOURCE,index));

		MENU_ClearTextTexture(work->strman);

		work->disp_width=MENU_CreateTextTexture(work->strman,
												EXPLAIN_U,EXPLAIN_V,
												EXPLAIN_WIDTH,EXPLAIN_HEIGHT,
												0,0,0,str)-EXPLAIN_U;
	}

	work->explain_flag++;
}

static void PrintExplain(Work *work)
{
	if(work->explain_flag>0){
		SPR_OBJ *spr;
		int x;

		work->explain_flag=1;

		spr=L2D_GetObject(work->l2d_handle,EXPLAIN_RECT);
		if(spr!=NULL){
			x=(int)((spr->box.rect.begin.x+spr->box.rect.end.x)/2.0f);
		}
		else{
			x=DRAW_WIDTH/2;
		}

#ifdef PSX2
#ifdef NTSC
		x-=(work->disp_width/2)*FONT_DISP_WIDTH/FONT_WIDTH+FONT_DISP_WIDTH/2;
#endif

#ifdef PAL
		x-=(work->disp_width/2)*FONT_DISP_WIDTH/FONT_WIDTH+FONT_DISP_WIDTH/4;
#endif
#else //XBOX
		if( GM_Language == GM_LANG_JAPANESE ){
			x-=(work->disp_width/2)*FONT_DISP_WIDTH/FONT_WIDTH+FONT_DISP_WIDTH/2;
		} else {
			x-=(work->disp_width/2)*FONT_DISP_WIDTH/FONT_WIDTH+FONT_DISP_WIDTH/4;
		}
#endif

		MENU_PutTextScreen(work->strman,
						   x,EXPLAIN_Y,
						   x+EXPLAIN_W,
						   EXPLAIN_Y+EXPLAIN_H,
						   EXPLAIN_U,EXPLAIN_V,
						   EXPLAIN_U+EXPLAIN_WIDTH,
						   EXPLAIN_V+EXPLAIN_HEIGHT,
						   TEXTCOLOR);
	}
}

static int demoSelect(Work *work)
{

#if 1

	switch(work->sub_step){
	case 0:
		if(work->busy_flag) break;

		ChangeSelectedDemo(work);
		work->action_strcode=CODE_SHOW_DEMOSEL;
		work->explain_flag=0;

		work->sound_flag=0;

		work->wait_count=0;

		work->sub_step++;

		SE_EXPANDLINE();
		break;
	case 1:
		if(work->busy_flag) break;

		PrintExplain(work);

		if(work->wait_count>0){
			work->wait_count--;
			break;
		}

		CreateExplain(work,work->demo_cursor);

		RightArrowNormal(work);
		LeftArrowNormal(work);

		if(work->key_press & PAD_CANCEL){
			work->ans=-1;
			work->sub_step++;

			return 0;
		}
		else if(work->key_press & PAD_OK){
			work->ans=1;
			work->sub_step++;
			work->wait_count=TITLE_FLUSH_COUNT;

			TitleHighlight(work);
			return 0;
		}

		switch(work->key_autostatus & (PAD_L|PAD_R)){
		case PAD_L:
			work->demo_cursor--;
			if(work->demo_cursor<0){
				work->demo_cursor=N_DEMOS-1;
			}

			work->wait_count=ARROW_FLUSH_COUNT;
			work->explain_flag=0;

			LeftArrowHighlight(work);
			ChangeSelectedDemo(work);

			SE_SEL();
			break;
		case PAD_R:
			work->demo_cursor++;
			if(work->demo_cursor>=N_DEMOS){
				work->demo_cursor=0;
			}

			work->wait_count=ARROW_FLUSH_COUNT;
			work->explain_flag=0;

			RightArrowHighlight(work);
			ChangeSelectedDemo(work);

			SE_SEL();
			break;
		}
		break;

	case 2:
		if(work->wait_count>0){
			work->wait_count--;
			break;
		}

		TitleNormal(work);

		work->action_strcode=CODE_HIDE_DEMOSEL;
		work->sub_step++;

		if(work->ans==1) SE_OK();
		else SE_CANCEL();
		break;
	case 3:
		if(work->busy_flag) break;
		return work->ans;
	}

#else

	DispDemoname(work);

	if(work->key_press & PAD_CANCEL) return -1;
	else if(work->key_press & PAD_OK) return 1;

	switch(work->key_autostatus & (PAD_U|PAD_D)){
	case PAD_U:
		work->demo_cursor--;
		if(work->demo_cursor<0){
			work->demo_cursor=N_DEMOS-1;
		}
		break;
	case PAD_D:
		work->demo_cursor++;
		if(work->demo_cursor>=N_DEMOS){
			work->demo_cursor=0;
		}
		break;
	}

#endif

	return 0;
}


static void CastTitleChange(Work *work)
{
	void *part;

	part=L2D_GetParts(work->l2d_handle,CODE_CAST_TITLE);
	if(part!=NULL){
		L2D_MorfObject(part,
					   demodata[work->demo_cursor].title,
					   demodata[work->demo_cursor].title,1.0f);
	}
}

static void SrcCastsChange(Work *work)
{
	int i;

	for(i=0;i<demodata[work->demo_cursor].n_casts;i++){
		int cast_num=demodata[work->demo_cursor].defcast[i];
		void *part;

		part=L2D_GetParts(work->l2d_handle,cast_panel_srcname[i]);
		if(part!=NULL){
			L2D_MorfObject(part,
						   cast_key_srcname[i][cast_num],
						   cast_key_srcname[i][cast_num],1.0f);
		}
	}
}

static void ItemDispControl(Work *work)
{
	int i;

	for(i=0;i<demodata[work->demo_cursor].n_casts;i++){
		SPR_OBJ *spr;

		spr=L2D_GetObject(work->l2d_handle,disp_arrow_point[i]);
		if(spr!=NULL) SPR_SHOW(spr);

		spr=L2D_GetObject(work->l2d_handle,cast_panel_srcname[i]);
		if(spr!=NULL) SPR_SHOW(spr);

		spr=L2D_GetObject(work->l2d_handle,cast_panel_dstname[i]);
		if(spr!=NULL) SPR_SHOW(spr);
	}

	for( ;i<sizeof(disp_arrow_point)/sizeof(disp_arrow_point[0]);i++){
		SPR_OBJ *spr;

		spr=L2D_GetObject(work->l2d_handle,disp_arrow_point[i]);
		if(spr!=NULL) SPR_HIDE(spr);

		spr=L2D_GetObject(work->l2d_handle,cast_panel_srcname[i]);
		if(spr!=NULL) SPR_HIDE(spr);

		spr=L2D_GetObject(work->l2d_handle,cast_panel_dstname[i]);
		if(spr!=NULL) SPR_HIDE(spr);
	}
}

static void DstCastChange(Work *work,int index)
{
	int cast_num=work->cast_item_cursor[index];
	void *part;

printf("name[%d]\n",cast_panel_dstname[index]);
	part=L2D_GetParts(work->l2d_handle,cast_panel_dstname[index]);
	if(part!=NULL){
printf("chg! cast_num[%d]index[%d]\n",cast_num,index);		
		L2D_MorfObject(part,
					   cast_key_dstname[index][cast_num],
					   cast_key_dstname[index][cast_num],1.0f);
	}
}

static void DstCastsChange(Work *work)
{
	int i;
	for(i=0;i<demodata[work->demo_cursor].n_casts;i++){
		DstCastChange(work,i);
	}
}

static void ShuffleButtonHighlight(Work *work)
{
	void *part;

	part=L2D_GetParts(work->l2d_handle,CODE_SHUFFLE_BUTTON);
	if(part!=NULL){
		L2D_MorfObject(part,
					   CODE_KEY_SHUFFLE_HIGHLIGHT,
					   CODE_KEY_SHUFFLE_HIGHLIGHT,1.0f);
	}
}

static void ShuffleButtonNormal(Work *work)
{
	void *part;

	part=L2D_GetParts(work->l2d_handle,CODE_SHUFFLE_BUTTON);
	if(part!=NULL){
		L2D_MorfObject(part,
					   CODE_KEY_SHUFFLE_NORMAL,
					   CODE_KEY_SHUFFLE_NORMAL,1.0f);
	}
}

static void PlayButtonHighlight(Work *work)
{
	void *part;

	part=L2D_GetParts(work->l2d_handle,CODE_PLAY_BUTTON);
	if(part!=NULL){
		L2D_MorfObject(part,
					   CODE_KEY_PLAY_HIGHLIGHT,
					   CODE_KEY_PLAY_HIGHLIGHT,1.0f);
	}
}

static void PlayButtonNormal(Work *work)
{
	void *part;

	part=L2D_GetParts(work->l2d_handle,CODE_PLAY_BUTTON);
	if(part!=NULL){
		L2D_MorfObject(part,
					   CODE_KEY_PLAY_NORMAL,
					   CODE_KEY_PLAY_NORMAL,1.0f);
	}
}

static void DispCastArrow(Work *work)
{
	int n_casts=demodata[work->demo_cursor].n_casts;
	SPR_OBJ *spr;
	int i;

	for(i=0;i<sizeof(arrow_point)/sizeof(arrow_point[0]);i++){
		spr=L2D_GetObject(work->l2d_handle,cast_panel_srcname[i]);
		if(spr!=NULL) spr->sprite.col.a=UNSEL_ALPHA;

		spr=L2D_GetObject(work->l2d_handle,cast_panel_dstname[i]);
		if(spr!=NULL) spr->sprite.col.a=UNSEL_ALPHA;
	}

	if(work->cast_cursor<n_casts){
		void *part;

		part=L2D_GetParts(work->l2d_handle,CODE_CAST_POINTER);
		if(part!=NULL){
			L2D_MorfObject(part,
						   arrow_point[work->cast_cursor],
						   arrow_point[work->cast_cursor],1.0f);
		}

		spr=L2D_GetObject(work->l2d_handle,cast_panel_srcname[work->cast_cursor]);
		if(spr!=NULL) spr->sprite.col.a=SEL_ALPHA;

		spr=L2D_GetObject(work->l2d_handle,cast_panel_dstname[work->cast_cursor]);
		if(spr!=NULL) spr->sprite.col.a=SEL_ALPHA;

		ShuffleButtonNormal(work);
		PlayButtonNormal(work);
	}
	else{
		SPR_OBJ *spr;

		spr=L2D_GetObject(work->l2d_handle,CODE_CAST_POINTER);
		if(spr!=NULL) SPR_HIDE(spr);

		if(work->cast_cursor==n_casts){
			ShuffleButtonHighlight(work);
			PlayButtonNormal(work);
		}
		else{
			ShuffleButtonNormal(work);
			PlayButtonHighlight(work);
		}
	}
}

static int castSelect(Work *work)
{

#if 1

	switch(work->sub_step){
	case 0:
		if(work->busy_flag) break;

		work->action_strcode=CODE_SHOW_CASTSEL;
		work->explain_flag=0;
		CastTitleChange(work);
		SrcCastsChange(work);
		DstCastsChange(work);
		DispCastArrow(work);

		work->sound_flag=0;

		work->wait_count=0;

		work->sub_step++;

		SE_EXPANDLINE();
		break;
	case 1:
		ItemDispControl(work);

		if(work->busy_flag) break;

		PrintExplain(work);

		if(work->wait_count>0){
			work->wait_count--;
			break;
		}

		{
			int n_casts=demodata[work->demo_cursor].n_casts;
			int n_casts2=n_casts+2;

			if(work->cast_cursor<n_casts){
				CreateExplain(work,N_DEMOS+0);
			}
			else{
				CreateExplain(work,N_DEMOS+1+work->cast_cursor-n_casts);
			}

			if(work->key_press & PAD_CANCEL){
				work->ans=-1;
				work->sub_step++;

				return 0;
			}
			else if(work->key_press & PAD_OK){
				if(work->cast_cursor==n_casts){
					work->cast_cursor=n_casts+1;
					work->explain_flag=-1;

					ShuffleCast(work);
					DstCastsChange(work);
					DispCastArrow(work);

					SE_DECIDE();
				}
				else if(work->cast_cursor==n_casts+1){
					work->ans=1;
					work->sub_step++;
				}
				else{
					work->cast_cursor=n_casts+1;
					work->explain_flag=-1;

					DispCastArrow(work);

					SE_SEL();
				}
				return 0;
			}

			switch(work->key_autostatus & (PAD_U|PAD_D)){
			case PAD_U:
				work->cast_cursor--;
				if(work->cast_cursor<0){
					work->cast_cursor=n_casts2-1;
				}
				DispCastArrow(work);

				if(work->cast_cursor==n_casts2-1 ||
				   work->cast_cursor>=n_casts-1){

					work->explain_flag=-1;
				}

				SE_SEL();
				return 0;
			case PAD_D:
				work->cast_cursor++;
				if(work->cast_cursor>=n_casts2){
					work->cast_cursor=0;
				}
				DispCastArrow(work);

				if(work->cast_cursor==0 ||
				   work->cast_cursor>=n_casts){

					work->explain_flag=-1;
				}

				SE_SEL();
				return 0;
			}

			if(work->cast_cursor<n_casts){
				int enselcast=work->enselcast_flags[work->demo_cursor][work->cast_cursor];
				int cur0=work->cast_item_cursor[work->cast_cursor];
				int cur1=cur0;

				switch(work->key_autostatus & (PAD_L|PAD_R)){
				case PAD_L:
					do{
						cur1--;
						if(cur1<0){
							cur1=EXCAST_CODE_MAX-1;
						}
					} while(!(enselcast & (1<<cur1)));
					break;
				case PAD_R:
					do{
						cur1++;
						if(cur1>=EXCAST_CODE_MAX){
							cur1=0;
						}
					} while(!(enselcast & (1<<cur1)));
					break;
				}

				if(cur0!=cur1){
printf("cur0 %d::cur1 %d\n",cur0,cur1);
					work->cast_item_cursor[work->cast_cursor]=cur1;
					DstCastChange(work,work->cast_cursor);
					DispCastArrow(work);

					SE_SWITCHOPT();
				}
			}
		}
		break;

	case 2:
		ItemDispControl(work);

		if(work->wait_count>0){
			work->wait_count--;
			break;
		}

		work->action_strcode=CODE_HIDE_CASTSEL;
		work->sub_step++;

		ShuffleButtonNormal(work);
		PlayButtonNormal(work);

		if(work->ans==1) SE_OK();
		else SE_CANCEL();
		break;
	case 3:
		ItemDispControl(work);

		if(work->busy_flag) break;
		return work->ans;
	}

#else

	int n_casts=demodata[work->demo_cursor].n_casts;


	DispCastname(work);

	if(work->key_press & PAD_CANCEL) return -1;
	else if(work->key_press & PAD_OK) return 1;

	switch(work->key_autostatus & (PAD_U|PAD_D)){
	case PAD_U:
		work->cast_cursor--;
		if(work->cast_cursor<0){
			work->cast_cursor=n_casts-1;
		}
		return 0;
	case PAD_D:
		work->cast_cursor++;
		if(work->cast_cursor>=n_casts){
			work->cast_cursor=0;
		}
		return 0;
	}

	switch(work->key_autostatus & (PAD_L|PAD_R)){
	case PAD_L:
		work->cast_item_cursor[work->cast_cursor]--;
		if(work->cast_item_cursor[work->cast_cursor]<0){
			work->cast_item_cursor[work->cast_cursor]=CAST_CODE_MAX-1;
		}
		break;
	case PAD_R:
		work->cast_item_cursor[work->cast_cursor]++;
		if(work->cast_item_cursor[work->cast_cursor]>=CAST_CODE_MAX){
			work->cast_item_cursor[work->cast_cursor]=0;
		}
		break;
	}

#endif

	return 0;
}

static void ResetCastItemCursor(Work *work)
{
	int i;

	work->cast_cursor=0;

	for(i=0;i<N_CASTS;i++){
		work->cast_item_cursor[i]=demodata[work->demo_cursor].defcast[i];
	}
}

static void Step(Work *work)
{
	switch(work->step){
	case INIT_STEP:
		work->step++;

	case DEMO_SELECT_STEP:
		switch(demoSelect(work)){
		case 1:
			work->step=CAST_SELECT_STEP;
			work->sub_step=0;
			ResetCastItemCursor(work);
			break;
		case -1:
			work->step=EXIT_STEP;
			work->sub_step=0;
			work->ans=-1;
			break;
		}
		break;
	case CAST_SELECT_STEP:
		switch(castSelect(work)){
		case 1:
			work->step=EXIT_STEP;
			work->sub_step=0;
			work->ans=1;
			break;
		case -1:
			work->step=DEMO_SELECT_STEP;
			work->sub_step=0;
			break;
		}
		break;
	case EXIT_STEP:
		GV_DestroyActor(work);
		break;
	}
}


static void Act(Work *work)
{
	if(work->l2d_handle>=0){
		if(L2D_ActionStatus(work->l2d_handle)==L2D_STAT_ACK){
			work->busy_flag=0;
			if(work->action_strcode!=0){
				int stat=L2D_EvokeAction(work->l2d_handle,work->action_strcode);

#ifdef DEBUG_MODE
				printf("L2D Stat = %d\n",stat);
#endif

				work->action_strcode=0;
				work->busy_flag=1;
			}
		}
		else{
			work->busy_flag=1;
		}

		// KeyAnim(work);
	}
	else{
		work->busy_flag=0;
	}


	ProgMessage(work);

	Key(work);
	Step(work);
}

static void Die(Work *work)
{
	CallLocalProc(work);

	if(work->strman!=NULL){
		GV_DestroyOtherActor(work->strman);
		work->strman=NULL;
	}
	if(work->l2d_handle>=0){
		L2D_ReleaseLayout(work->l2d_handle);
		work->l2d_handle=-1;
	}
}


/* ------------------------------------------------------------------------ */


static void LayoutSignalFunc(Work *work,int sign,int value)
{
	switch(sign){
	case CODE_SOUND:
		if(work->sound_flag) SE_EXPANDLINE();
		else{
			SE_WINOPEN();
			work->sound_flag=1;
		}
		break;
	}
}

/* オプションの評価 */
static void GetOptionValue(Work *work)
{
	int i,j;

	work->step=0;
	work->sub_step=0;

	work->l2d_strcode=0;
	work->l2d_handle=-1;
	work->proc=0;
	work->start_mode=0;

	work->demo_cursor=0;
	work->cast_cursor=0;

	for(i=0;i<N_DEMOS;i++){
		for(j=0;j<N_CASTS;j++){
			work->enselcast_flags[i][j]=demodata[i].enselcast[j];
		}
	}

    if(GCL_GetOption('d')!=NULL){
		work->l2d_strcode=GCL_GetNextInt();
    }
    if(GCL_GetOption('p')!=NULL){
		work->proc=GCL_GetNextInt();
    }
    if(GCL_GetOption('s')!=NULL){
		int demonum;

		demonum=work->demo_cursor=GCL_GetNextInt()-1;

		for(i=0;i<N_CASTS;i++){
			work->cast_item_cursor[i]=GCL_GetNextInt();

			if(work->cast_item_cursor[i]>0){
				work->cast_item_cursor[i]--;
			}
			else{
				work->cast_item_cursor[i]=demodata[demonum].defcast[i];
			}
		}

		work->step=CAST_SELECT_STEP;
    }
	if(GCL_GetOption('f')!=NULL){
		for(i=0;i<N_DEMOS;i++){
			for(j=0;j<N_CASTS;j++){
				int flag=GCL_GetNextInt();
				work->enselcast_flags[i][j]&=~flag;
			}
		}
	}
}

/* 資源を獲得 */
static int GetResources(Work *work)
{
	GetOptionValue(work);

#if 1
	work->l2d_handle=L2D_LoadLayout(work->l2d_strcode,DISP_CHANL,MENU_PRIORITY,0);

	if(work->l2d_handle<0){
#ifdef DEBUG_MODE
		printf("Select Scr : L2D Init Error = %d\n",work->l2d_handle);
#endif

		ASSERT(0);
	}
	else{
		/* シグナルハンドラの設定 */
		L2D_SetSignalHandle(work->l2d_handle,work,(void (*)(void *,int,int))LayoutSignalFunc);

		/* layoutの初期化 */
		L2D_EvokeAction(work->l2d_handle,CODE_DEFAULTACTION);
	}
#endif

    return 1;
}

/* 初期化部メイン */
void *NewTheaterScr(int name,int where)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work));

    if(work!=NULL) {
		work->name=name;

		work->l2d_handle=-1;
		work->busy_flag=0;
		work->action_strcode=0;

		work->ans=0;

		work->strman=NULL;

		work->explain_resource=explain_resource_name[ GM_Language-GM_LANG_ENGLISH ];

		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));
		GV_SetActorMessageKill(&(work->actor),name);

		if(!GetResources(work)){
			GV_DestroyActor(work);
			return NULL ;
		}
		if((work->strman=NewTextScreenControl())==NULL){
			GV_DestroyActor(work);
			return NULL;
		}

		// InitMenu(work);

		GM_SetGameStatus(STATE_PAUSE_DISABLE);
    }
    return (void *)work ;
}
