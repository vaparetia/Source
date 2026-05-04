/********************************************************************************/
/*	Har_message.c								*/
/*	^$B%O%j%"%a%C%;!<%87O^(B */
/*	2001/01/23 H.Satoyoshi							*/
/*	$Id: har_message.c,v 1.1.1.3 2002/11/19 11:48:22 Yoshizawa1 Exp $		*/
/********************************************************************************/

static GV_MSG tmp_gv_msg;

static int	messs[5];
static int	messt[5];

void send_player_message()
{	
    int loop;

    for (loop=0; loop<5; loop++){
	NewBodyFire(GM_PlayerBody, loop);
    }
    tmp_gv_msg.address = GM_PlayerControl->name;
    tmp_gv_msg.message_len = 5;
    tmp_gv_msg.message = messs;
    messs[0] = 19;

    switch( GM_GameLevel ){
    case GM_LEVEL_VERYEASY:
	messs[1] = 30;
	break;
    case ST_LEV_EASY:
	messs[1] = 30;
	break;
    case ST_LEV_NORMAL:
	messs[1] = 35;
	break;

    case ST_LEV_UPNORM:
	messs[1] = 37;
	break;


    case GM_LEVEL_HARD:
	messs[1] = 40;
	break;
    case GM_LEVEL_E_EXTREME:
    case GM_LEVEL_EXTREME:
	messs[1] = 50;
	break;
    }


    messs[3] = 500;
    //	messs[3] = 320;

    if (GM_PlayerPosition.vx<=-1600.0f){
	messs[2] = 30;
	messs[4] = 0;
    }
#if 0
    else if ( (-1250.0f<=GM_PlayerPosition.vx) && (GM_PlayerPosition.vx<=1250.0f) &&
	 (-154750.0f<=GM_PlayerPosition.vz) && (GM_PlayerPosition.vz<=-152251.0f) ){

	if (GM_PlayerPosition.vx > 0.0f){
	    messs[2] = 30;
	}
	else {
	    messs[2] = -30;
	}

	if (GM_PlayerPosition.vz > -154500.0f){
	    messs[4] = 30;
	}
	else {
	    messs[4] = -30;
	}
    }
#endif
    else {
	messs[2] = messs[4] = 0;
    }


    GV_SendMessage( &tmp_gv_msg );
printf ("player down (%d, %d, %d)\n",messs[2], messs[3], messs[4]);
}


void send_wave_message()
{
    tmp_gv_msg.address = GV_StrCode("plant_sea");
    tmp_gv_msg.message_len = 2;
    tmp_gv_msg.message = messs;
    messs[0] = 5;
    if (messs[1] == 1){
	messs[1] = 0;

printf ("Wave Off \n");

    }
    else {
	messs[1] = 1;

printf ("Wave ON \n");

    }

    GV_SendMessage( &tmp_gv_msg );
}

void send_wavetex_message()
{
    tmp_gv_msg.address = GV_StrCode("plant_sea");
    tmp_gv_msg.message_len = 2;
    tmp_gv_msg.message = messt;
    messt[0] = 99;
    if (messt[1] == 1){
	messt[1] = 0;
printf ("Tex Off \n");
    }
    else {
	messt[1] = 1;
printf ("Tex ON \n");
    }

    GV_SendMessage( &tmp_gv_msg );
}




/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	^$BL>A0^(B:	void call_event_proc						*/
/*	^$B0z?t^(B:	Work	*work							*/
/*	^$B@bL@^(B:	^$B%$%Y%s%H4XO"%W%m%C%/^(B						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void har_call_event_proc(Work *work, int num)
{
    if (work->GAME_OVER_FLAG == ON){
	return;
    }

    if ( work->event_proc_id != 0 ){
	GCL_ARGS	args;
	int		event_num = num;
	
	args.argc = 1;
	args.argv = &event_num;
	
	GCL_ExecProc(work->event_proc_id, &args);
    }
    work->event_id = num;	//^$B%$%Y%s%HHV9f$rJ]B8^(B

    printf ("						Event proc call %d \n", num);
}
