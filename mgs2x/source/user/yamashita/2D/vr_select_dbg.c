//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * vr_select - vr_select_dbg.c
 * ＶＲセレクト
 * 2002/08/01 S.Yamashita
 * $Id: vr_select_dbg.c,v 1.3 2002/11/23 12:46:56 Yoshizawa1 Exp $
 */

/*******************************************************************************
 * include
 */

#include "vr_select.h"

#include "mode/menu/xtextscn.h"

/*******************************************************************************
 * extern
 */
extern void MENU_ClearTextTexture(void *work);
extern void MENU_PutTextScreen(void *work, int x1, int y1, int x2, int y2, int u1, int v1, int u2, int v2, int col);

/*******************************************************************************
 * public
 */
/*******************************************************************************
 * Ｌ１ボタンが押された
 */
void VRSEL_Dbg_L1(
	VRSEL_WORK *work)	/* ワーク */
{
#ifdef DEBUG_MODE
	if(MSN_2DSTATUS & MSN_2DSTAT_DEBUGMODE_ON)
	{
		_page--;
		if(_page < 0)
			_page = VRSEL_MAX_PAGES - 1;
		_item = 0;

		/* バッファのクリア */
		MENU_ClearTextTexture(_text_work);
	}
#endif
}
/*******************************************************************************
 * Ｒ１ボタンが押された
 */
void VRSEL_Dbg_R1(
	VRSEL_WORK *work)	/* ワーク */
{
#ifdef DEBUG_MODE
	if(MSN_2DSTATUS & MSN_2DSTAT_DEBUGMODE_ON)
	{
		_item--;
		if(_item < 0)
		{
			switch(_page)
			{
			case 0: _item = VRSEL_PAGE0_MAX_ITEMS - 1; break;
			case 1: _item = VRSEL_PAGE1_MAX_ITEMS - 1; break;
			case 2: _item = VRSEL_PAGE2_MAX_ITEMS - 1; break;
			case 3: _item = VRSEL_PAGE3_MAX_ITEMS - 1; break;
			case 4: _item = VRSEL_PAGE4_MAX_ITEMS - 1; break;
			}
		}

		/* バッファのクリア */
		MENU_ClearTextTexture(_text_work);
	}
#endif
}
/*******************************************************************************
 * Ｌ２ボタンが押された
 */
void VRSEL_Dbg_L2(
	VRSEL_WORK *work)	/* ワーク */
{
#ifdef DEBUG_MODE
	if(MSN_2DSTATUS & MSN_2DSTAT_DEBUGMODE_ON)
	{
		_page++;
		if(_page >= VRSEL_MAX_PAGES)
			_page = 0;
		_item = 0;

		/* バッファのクリア */
		MENU_ClearTextTexture(_text_work);
	}
#endif
}
/*******************************************************************************
 * Ｒ２ボタンが押された
 */
void VRSEL_Dbg_R2(
	VRSEL_WORK *work)	/* ワーク */
{
#ifdef DEBUG_MODE
	if(MSN_2DSTATUS & MSN_2DSTAT_DEBUGMODE_ON)
	{
		_item++;
		switch(_page)
		{
		case 0: if(_item >= VRSEL_PAGE0_MAX_ITEMS) _item = 0; break;
		case 1: if(_item >= VRSEL_PAGE1_MAX_ITEMS) _item = 0; break;
		case 2: if(_item >= VRSEL_PAGE2_MAX_ITEMS) _item = 0; break;
		case 3: if(_item >= VRSEL_PAGE3_MAX_ITEMS) _item = 0; break;
		case 4: if(_item >= VRSEL_PAGE4_MAX_ITEMS) _item = 0; break;
		}

		/* バッファのクリア */
		MENU_ClearTextTexture(_text_work);
	}
#endif
}
/*******************************************************************************
 * Ｌ３ボタンが押された
 */
void VRSEL_Dbg_L3(
	VRSEL_WORK *work)	/* ワーク */
{
#ifdef DEBUG_MODE
	MSN_2DSTATUS ^= MSN_2DSTAT_DEBUGMODE_ON;

	/* バッファのクリア */
	MENU_ClearTextTexture(_text_work);

	if(MSN_2DSTATUS & MSN_2DSTAT_DEBUGMODE_ON)
	{
		/* 背景スプライトの表示 */
		_background->sprite.head.head.flags &= ~SPR_FLAG_HIDDEN;
	}
	else
	{
		/* 背景スプライトの消去 */
		_background->sprite.head.head.flags |= SPR_FLAG_HIDDEN;
	}
#endif
}
/*******************************************************************************
 * Ｒ３ボタンが押された
 */
void VRSEL_Dbg_R3(
	VRSEL_WORK *work)	/* ワーク */
{
#ifdef DEBUG_MODE
#endif
}
/*******************************************************************************
 * 毎フレーム処理
 */
void VRSEL_Dbg_Act(
	VRSEL_WORK *work)	/* ワーク */
{
#ifdef DEBUG_MODE
	int  i;
	int  data[5];
	char buf[256];
	int  stage_id;

	if(MSN_2DSTATUS & MSN_2DSTAT_DEBUGMODE_ON)
	{
		/* 背景スプライトの表示 */
		_background->sprite.head.head.flags &= ~SPR_FLAG_HIDDEN;

		/* テキスト */
		memset(buf, 0x00, sizeof(buf));

		/* Page:Item */
		sprintf(buf, "%d : %d", _page + 1, _item + 1);
		MENU_CreateTextTexture(_text_work, TEXT_X(27), TEXT_Y(1), TEXT_W*32, TEXT_H, 0, 0, buf);

		/* Modified */
		if(*_MSN_SAVE_DATA_FLAG & MSN_DFLAG_MODIFIED_DATA)
		{
			sprintf(buf, "*");
			MENU_CreateTextTexture(_text_work, TEXT_X(30), TEXT_Y(1), TEXT_W*32, TEXT_H, 0, 0, buf);
		}

		switch(_page)
		{
/* page 0 */
		case 0:
			/* メニュー */
			for(i = 0; i < VRSEL_PAGE0_MAX_ITEMS; i++)
			{
				if(i == _item) sprintf(buf, ">>>");
				else           sprintf(buf, "___");
				switch(i)
				{
				case 0: strcat(buf, "SET ALL STAGES PLAYABLE"); break;
				case 1: strcat(buf, "VR SELECT STATUS"); break;
				case 2: strcat(buf, "CURRENT STAGE DATA"); break;
				case 3: strcat(buf, "MISSIONS DATA"); break;
				default:
					break;
				}
				MENU_CreateTextTexture(_text_work, TEXT_X(1), TEXT_Y(i+2), TEXT_W*32, TEXT_H, 0, 0, buf);
			}

			switch(_item)
			{
/* item 0:0 */
			case 0:
				/* Message */
				if(*_MSN_SAVE_DATA_FLAG & MSN_DFLAG_MODIFIED_DATA)
				{
					sprintf(buf, "Now, all stages are playable.");
					MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(7), TEXT_W*32, TEXT_H, 0, 0, buf);

				}
				else
				{
					sprintf(buf, "Press R3 to");
					MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(7), TEXT_W*32, TEXT_H, 0, 0, buf);
					sprintf(buf, "set all stages playable.");
					MENU_CreateTextTexture(_text_work, TEXT_X(20), TEXT_Y(8), TEXT_W*32, TEXT_H, 0, 0, buf);

					if(GV_PadDataDirect[0].press & PAD_AR)
					{
						/* ステージリストの設定 */
						VRSEL_SetStageList(work, 1);
						*_MSN_SAVE_DATA_FLAG |= MSN_DFLAG_MODIFIED_DATA;
					}
				}
				break;
/* item 0:1 */
			case 1:
				/* Phase */
				sprintf(buf, "Phase: %d", work->phase);
				MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(7), TEXT_W*32, TEXT_H, 0, 0, buf);
				/* Count */
				sprintf(buf, "Count: %05d", work->count);
				MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(8), TEXT_W*32, TEXT_H, 0, 0, buf);
				/* Count2 */
				sprintf(buf, "Count2: %05d", work->count2);
				MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(9), TEXT_W*32, TEXT_H, 0, 0, buf);

				/* Key Config Select Set */
				sprintf(buf, "Key Config Select Set: %d", work->keyconsel_set);
				MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(10), TEXT_W*32, TEXT_H, 0, 0, buf);
				/* Key Config Select Ret */
				sprintf(buf, "Key Config Select Ret: %d", work->keyconsel_ret);
				MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(11), TEXT_W*32, TEXT_H, 0, 0, buf);
				/* Key Config Select Ret */
				sprintf(buf, "VR Clear Code Status: %d", work->vrclc_status);
				MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(12), TEXT_W*32, TEXT_H, 0, 0, buf);

				/* Flag */
				sprintf(buf, "Flag: 0x%04x", work->flag);
				MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(13), TEXT_W*32, TEXT_H, 0, 0, buf);

				/* Player */
				sprintf(buf, "Player: %d", work->player);
				MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(14), TEXT_W*32, TEXT_H, 0, 0, buf);
				/* Missions */
				sprintf(buf, "Missions: %d", work->missions);
				MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(15), TEXT_W*32, TEXT_H, 0, 0, buf);
				/* Mode */
				sprintf(buf, "Mode: %d", work->mode);
				MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(16), TEXT_W*32, TEXT_H, 0, 0, buf);
				/* Weapon */
				sprintf(buf, "Weapon: %d", work->weapon);
				MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(17), TEXT_W*32, TEXT_H, 0, 0, buf);
				/* Level */
				sprintf(buf, "Level: %d", work->level);
				MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(18), TEXT_W*32, TEXT_H, 0, 0, buf);
				break;
/* item 0:2 */
			case 2:
				stage_id = VRSEL_GetStageID(work);

				if(stage_id == -1)
				{
					/* Message */
					sprintf(buf, "NOT AVAILABLE");
					MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(7), TEXT_W*32, TEXT_H, 0, 0, buf);
				}
				else
				{
					/* Clear Flag */
					sprintf(buf, "Clear Flag: %d", MSN_GET_CLEARFLAG(stage_id));
					MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(7), TEXT_W*32, TEXT_H, 0, 0, buf);

					/* 1st Score */
					sprintf(buf, "1st Score: %d", MSN_GET_HISCORE2(stage_id, 1));
					MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(8), TEXT_W*32, TEXT_H, 0, 0, buf);
					/* 1st Default Flag */
					sprintf(buf, "1st Overwrite Flag: %d", ((MSN_GET_HISCORE(stage_id, 1) & MSN_OVERWRITEFLAG) != 0));
					MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(9), TEXT_W*32, TEXT_H, 0, 0, buf);
					/* 2nd Score */
					sprintf(buf, "2nd Score: %d", MSN_GET_HISCORE2(stage_id, 2));
					MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(10), TEXT_W*32, TEXT_H, 0, 0, buf);
					/* 2nd Default Flag */
					sprintf(buf, "2nd Overwrite Flag: %d", ((MSN_GET_HISCORE(stage_id, 2) & MSN_OVERWRITEFLAG) != 0));
					MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(11), TEXT_W*32, TEXT_H, 0, 0, buf);
					/* 3rd Score */
					sprintf(buf, "3rd Score: %d", MSN_GET_HISCORE2(stage_id, 3));
					MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(12), TEXT_W*32, TEXT_H, 0, 0, buf);
					/* 3rd Default Flag */
					sprintf(buf, "3rd Overwrite Flag: %d", ((MSN_GET_HISCORE(stage_id, 3) & MSN_OVERWRITEFLAG) != 0));
					MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(13), TEXT_W*32, TEXT_H, 0, 0, buf);

					Msn_Get1stClearData(stage_id, &data[0], &data[1], &data[2], &data[3], &data[4]);
					/* 1st Time Score */
					sprintf(buf, "1st Time Score: %d", data[0]);
					MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(14), TEXT_W*32, TEXT_H, 0, 0, buf);
					/* 1st Bullet Score */
					sprintf(buf, "1st Bullet Score: %d", data[1]);
					MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(15), TEXT_W*32, TEXT_H, 0, 0, buf);
					/* 1st Sneaking Score */
					sprintf(buf, "1st Sneaking Score: %d", data[2]);
					MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(16), TEXT_W*32, TEXT_H, 0, 0, buf);
					/* 1st No Kill Score */
					sprintf(buf, "1st No Kill Score: %d", data[3]);
					MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(17), TEXT_W*32, TEXT_H, 0, 0, buf);
					/* Random Seed */
					sprintf(buf, "Random Seed: %d", data[4]);
					MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(18), TEXT_W*32, TEXT_H, 0, 0, buf);

					/* clear code */
					{
						int length;
						unsigned char clear_code[32];

						memset(clear_code, 0x00, sizeof(clear_code));
						length = Msn_GetClearCode(stage_id, clear_code, 20);

						for(i = 0; i < 20; i++)
						{
							clear_code[i] += 'A';
						}
						sprintf(buf, "Clear Code: %c%c%c%c %c%c%c%c %c%c%c%c %c%c%c%c %c%c%c%c (%d)",
							clear_code[ 0], clear_code[ 1], clear_code[ 2], clear_code[ 3], clear_code[ 4],
							clear_code[ 5], clear_code[ 6], clear_code[ 7], clear_code[ 8], clear_code[ 9],
							clear_code[10], clear_code[11], clear_code[12], clear_code[13], clear_code[14],
							clear_code[15], clear_code[16], clear_code[17], clear_code[18], clear_code[19],
							length);
						MENU_CreateTextTexture(_text_work, TEXT_X(5), TEXT_Y(20), TEXT_W*32, TEXT_H, 0, 0, buf);
					}
				}
				break;
/* item 0:3 */
			case 3:
				/* Name */
				memset(buf, 0x00, sizeof(buf));
				sprintf(buf, "Name: ");
				strncat(buf, (char*)GM_MyName, sizeof(buf) - (strlen("Name: ") + 1));
				MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(7), TEXT_W*32, TEXT_H, 0, 0, buf);
				/* Save Data Version */
				sprintf(buf, "Save Data Version: %d", *_MSN_SAVE_DATA_VERSION);
				MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(8), TEXT_W*32, TEXT_H, 0, 0, buf);
				/* Save Data Size */
				sprintf(buf, "Save Data Size: %d", *_MSN_SAVE_DATA_SIZE);
				MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(9), TEXT_W*32, TEXT_H, 0, 0, buf);
				/* Save Data Flag */
				sprintf(buf, "Save Data Flag: 0x%08d", *_MSN_SAVE_DATA_FLAG);
				MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(10), TEXT_W*32, TEXT_H, 0, 0, buf);
				/* Retry Count */
				sprintf(buf, "Retry Count: %d", MSN_RETRY_COUNT);
				MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(11), TEXT_W*32, TEXT_H, 0, 0, buf);
				/* Continue Count */
				sprintf(buf, "Continue Count: %d", MSN_CONTINUE_COUNT);
				MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(12), TEXT_W*32, TEXT_H, 0, 0, buf);
				/* 2D Status */
				sprintf(buf, "2D Status: 0x%08x", MSN_2DSTATUS);
				MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(13), TEXT_W*32, TEXT_H, 0, 0, buf);
				/* Stage ID */
				sprintf(buf, "Stage ID: %d", MSN_STAGE_ID);
				MENU_CreateTextTexture(_text_work, TEXT_X(16), TEXT_Y(14), TEXT_W*32, TEXT_H, 0, 0, buf);
				break;
			default:
				break;
			}

			break;



/* page 1 */
		case 1:
			/* メニュー */
			for(i = 0; i < VRSEL_PAGE1_MAX_ITEMS; i++)
			{
				if(i == _item) sprintf(buf, ">>>");
				else           sprintf(buf, "___");
				switch(i)
				{
				case 0: strcat(buf, "SNEAKING MODE - SNEAKING DATA"); break;
				case 1: strcat(buf, "SNEAKING MODE - ELIMINATE ALL DATA"); break;
				default:
					break;
				}
				MENU_CreateTextTexture(_text_work, TEXT_X(1), TEXT_Y(i+2), TEXT_W*32, TEXT_H, 0, 0, buf);
			}

			switch(_item)
			{
/* item 1:0-1 */
			case 0:
			case 1:
				sprintf(buf, "No__RAI__NIN__X-R__SNA__PLI__TUX__PRE");
				MENU_CreateTextTexture(_text_work, TEXT_X(5), TEXT_Y(8), TEXT_W*32, TEXT_H, 0, 0, buf);

				for(i = 0; i < SNEAKING_STAGE_NUM; i++)
				{
					sprintf(buf, "%02d___%02d___%02d___%02d___%02d___%02d___%02d___%02d", i + 1,
						_sneaking_stage[(int)_item][i][0],
						_sneaking_stage[(int)_item][i][1],
						_sneaking_stage[(int)_item][i][2],
						_sneaking_stage[(int)_item][i][3],
						_sneaking_stage[(int)_item][i][4],
						_sneaking_stage[(int)_item][i][5],
						_sneaking_stage[(int)_item][i][6]); 
					MENU_CreateTextTexture(_text_work, TEXT_X(5), TEXT_Y(9 + i), TEXT_W*32, TEXT_H, 0, 0, buf);
				}
				break;
			default:
				break;
			}

			break;



/* page 2 */
		case 2:
			/* メニュー */
			for(i = 0; i < VRSEL_PAGE2_MAX_ITEMS; i++)
			{
				if(i == _item) sprintf(buf, ">>>");
				else           sprintf(buf, "___");
				switch(i)
				{
				case 0: strcat(buf, "WEAPON MODE - HANDGUN DATA"); break;
				case 1: strcat(buf, "WEAPON MODE - ASSAULT RIFLE DATA"); break;
				case 2: strcat(buf, "WEAPON MODE - C4 / CLAYMORE DATA"); break;
				case 3: strcat(buf, "WEAPON MODE - GRENADE DATA"); break;
				case 4: strcat(buf, "WEAPON MODE - PSG-1 DATA"); break;
				case 5: strcat(buf, "WEAPON MODE - STINGER DATA"); break;
				case 6: strcat(buf, "WEAPON MODE - NIKITA DATA"); break;
				case 7: strcat(buf, "WEAPON MODE - HF. BLADE DATA"); break;
				default:
					break;
				}
				MENU_CreateTextTexture(_text_work, TEXT_X(1), TEXT_Y(i+2), TEXT_W*32, TEXT_H, 0, 0, buf);
			}

			switch(_item)
			{
/* item 2:0-7 */
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
				sprintf(buf, "No__RAI__NIN__X-R__SNA__PLI__TUX__PRE");
				MENU_CreateTextTexture(_text_work, TEXT_X(5), TEXT_Y(13), TEXT_W*32, TEXT_H, 0, 0, buf);

				for(i = 0; i < WEAPON_STAGE_NUM; i++)
				{
					sprintf(buf, "%02d___%02d___%02d___%02d___%02d___%02d___%02d___%02d", i + 1,
						_weapon_stage[(int)_item][i][0],
						_weapon_stage[(int)_item][i][1],
						_weapon_stage[(int)_item][i][2],
						_weapon_stage[(int)_item][i][3],
						_weapon_stage[(int)_item][i][4],
						_weapon_stage[(int)_item][i][5],
						_weapon_stage[(int)_item][i][6]); 
					MENU_CreateTextTexture(_text_work, TEXT_X(5), TEXT_Y(14 + i), TEXT_W*32, TEXT_H, 0, 0, buf);
				}
				break;
			default:
				break;
			}

			break;



/* page 3 */
		case 3:
			/* メニュー */
			for(i = 0; i < VRSEL_PAGE3_MAX_ITEMS; i++)
			{
				if(i == _item) sprintf(buf, ">>>");
				else           sprintf(buf, "___");
				switch(i)
				{
				case 0: strcat(buf, "FIRST PERSON VIEW MODE DATA"); break;
				case 1: strcat(buf, "VARIETY MODE DATA"); break;
				case 2: strcat(buf, "STREAKING MODE DATA"); break;
				default:
					break;
				}
				MENU_CreateTextTexture(_text_work, TEXT_X(1), TEXT_Y(i+2), TEXT_W*32, TEXT_H, 0, 0, buf);
			}

			switch(_item)
			{
/* item 3:0 */
			case 0:
				sprintf(buf, "No__RAI__NIN__X-R__SNA__PLI__TUX__PRE");
				MENU_CreateTextTexture(_text_work, TEXT_X(5), TEXT_Y(8), TEXT_W*32, TEXT_H, 0, 0, buf);

				for(i = 0; i < FPV_STAGE_NUM; i++)
				{
					sprintf(buf, "%02d___%02d___%02d___%02d___%02d___%02d___%02d___%02d", i + 1,
						_fpv_stage[i][0],
						_fpv_stage[i][1],
						_fpv_stage[i][2],
						_fpv_stage[i][3],
						_fpv_stage[i][4],
						_fpv_stage[i][5],
						_fpv_stage[i][6]); 
					MENU_CreateTextTexture(_text_work, TEXT_X(5), TEXT_Y(9 + i), TEXT_W*32, TEXT_H, 0, 0, buf);
				}
				break;
/* item 3:1 */
			case 1:
				sprintf(buf, "No__RAI__NIN__X-R__SNA__PLI__TUX__PRE");
				MENU_CreateTextTexture(_text_work, TEXT_X(5), TEXT_Y(8), TEXT_W*32, TEXT_H, 0, 0, buf);

				for(i = 0; i < VARIETY_STAGE_NUM; i++)
				{
					sprintf(buf, "%02d___%02d___%02d___%02d___%02d___%02d___%02d___%02d", i + 1,
						_variety_stage[i][0],
						_variety_stage[i][1],
						_variety_stage[i][2],
						_variety_stage[i][3],
						_variety_stage[i][4],
						_variety_stage[i][5],
						_variety_stage[i][6]); 
					MENU_CreateTextTexture(_text_work, TEXT_X(5), TEXT_Y(9 + i), TEXT_W*32, TEXT_H, 0, 0, buf);
				}
				break;
/* item 3:2 */
			case 2:
				sprintf(buf, "No__RAI__NIN__X-R__SNA__PLI__TUX__PRE");
				MENU_CreateTextTexture(_text_work, TEXT_X(5), TEXT_Y(8), TEXT_W*32, TEXT_H, 0, 0, buf);
				sprintf(buf, "01___-1___-1___%02d___-1___-1___-1___-1", _streaking_stage); 
				MENU_CreateTextTexture(_text_work, TEXT_X(5), TEXT_Y(9), TEXT_W*32, TEXT_H, 0, 0, buf);
			default:
				break;
			}

			break;



/* page 4 */
		case 4:
			/* メニュー */
			for(i = 0; i < VRSEL_PAGE4_MAX_ITEMS; i++)
			{
				if(i == _item) sprintf(buf, ">>>");
				else           sprintf(buf, "___");
				switch(i)
				{
				case 0: strcat(buf, "BOMB DISPOSAL MODE DATA"); break;
				case 1: strcat(buf, "ELIMINATE MODE DATA"); break;
				case 2: strcat(buf, "HOLD UP MODE DATA"); break;
				case 3: strcat(buf, "PHOTOGRAPH MODE DATA"); break;
				default:
					break;
				}
				MENU_CreateTextTexture(_text_work, TEXT_X(1), TEXT_Y(i+2), TEXT_W*32, TEXT_H, 0, 0, buf);
			}

			switch(_item)
			{
/* item 4:0 */
			case 0:
				sprintf(buf, "No__RAI__NIN__X-R__SNA__PLI__TUX__PRE");
				MENU_CreateTextTexture(_text_work, TEXT_X(5), TEXT_Y(8), TEXT_W*32, TEXT_H, 0, 0, buf);

				for(i = 0; i < BOMB_STAGE_NUM; i++)
				{
					sprintf(buf, "%02d___%02d___%02d___%02d___%02d___%02d___%02d___%02d", i + 1,
						_bomb_stage[i][0],
						_bomb_stage[i][1],
						_bomb_stage[i][2],
						_bomb_stage[i][3],
						_bomb_stage[i][4],
						_bomb_stage[i][5],
						_bomb_stage[i][6]); 
					MENU_CreateTextTexture(_text_work, TEXT_X(5), TEXT_Y(9 + i), TEXT_W*32, TEXT_H, 0, 0, buf);
				}
				break;
/* item 4:1 */
			case 1:
				sprintf(buf, "No__RAI__NIN__X-R__SNA__PLI__TUX__PRE");
				MENU_CreateTextTexture(_text_work, TEXT_X(5), TEXT_Y(8), TEXT_W*32, TEXT_H, 0, 0, buf);

				for(i = 0; i < ELIMINATE_STAGE_NUM; i++)
				{
					sprintf(buf, "%02d___%02d___%02d___%02d___%02d___%02d___%02d___%02d", i + 1,
						_eliminate_stage[i][0],
						_eliminate_stage[i][1],
						_eliminate_stage[i][2],
						_eliminate_stage[i][3],
						_eliminate_stage[i][4],
						_eliminate_stage[i][5],
						_eliminate_stage[i][6]); 
					MENU_CreateTextTexture(_text_work, TEXT_X(5), TEXT_Y(9 + i), TEXT_W*32, TEXT_H, 0, 0, buf);
				}
				break;
/* item 4:2 */
			case 2:
				sprintf(buf, "No__RAI__NIN__X-R__SNA__PLI__TUX__PRE");
				MENU_CreateTextTexture(_text_work, TEXT_X(5), TEXT_Y(8), TEXT_W*32, TEXT_H, 0, 0, buf);

				for(i = 0; i < HOLDUP_STAGE_NUM; i++)
				{
					sprintf(buf, "%02d___%02d___%02d___%02d___%02d___%02d___%02d___%02d", i + 1,
						_holdup_stage[i][0],
						_holdup_stage[i][1],
						_holdup_stage[i][2],
						_holdup_stage[i][3],
						_holdup_stage[i][4],
						_holdup_stage[i][5],
						_holdup_stage[i][6]); 
					MENU_CreateTextTexture(_text_work, TEXT_X(5), TEXT_Y(9 + i), TEXT_W*32, TEXT_H, 0, 0, buf);
				}
				break;
/* item 4:3 */
			case 3:
				sprintf(buf, "No__RAI__NIN__X-R__SNA__PLI__TUX__PRE");
				MENU_CreateTextTexture(_text_work, TEXT_X(5), TEXT_Y(8), TEXT_W*32, TEXT_H, 0, 0, buf);

				for(i = 0; i < PHOTOGRAPH_STAGE_NUM; i++)
				{
					sprintf(buf, "%02d___%02d___%02d___%02d___%02d___%02d___%02d___%02d", i + 1,
						_photograph_stage[i][0],
						_photograph_stage[i][1],
						_photograph_stage[i][2],
						_photograph_stage[i][3],
						_photograph_stage[i][4],
						_photograph_stage[i][5],
						_photograph_stage[i][6]); 
					MENU_CreateTextTexture(_text_work, TEXT_X(5), TEXT_Y(9 + i), TEXT_W*32, TEXT_H, 0, 0, buf);
				}
				break;
			default:
				break;
			}

			break;



		default:
			break;
		}

		/* 画面出力 */
		MENU_PutTextScreen(_text_work, 0, 0, VRDEF_SCREEN_X, VRDEF_SCREEN_Y, 0, 0, VRSEL_TEXT_BUF_X, VRSEL_TEXT_BUF_Y, 0x60203ce0);

		if(GV_PadDataDirect[0].press)
		{
			/* バッファのクリア */
			MENU_ClearTextTexture(_text_work);
		}
	}
	else if(*_MSN_SAVE_DATA_FLAG & MSN_DFLAG_MODIFIED_DATA)
	{
		/* Modified */
		sprintf(buf, "*");
		MENU_CreateTextTexture(_text_work, TEXT_X(30), TEXT_Y(1), TEXT_W*32, TEXT_H, 0, 0, buf);

		/* 画面出力 */
		MENU_PutTextScreen(_text_work, 0, 0, VRDEF_SCREEN_X, VRDEF_SCREEN_Y, 0, 0, VRSEL_TEXT_BUF_X, VRSEL_TEXT_BUF_Y, 0x60203ce0);
	}
#endif
}
