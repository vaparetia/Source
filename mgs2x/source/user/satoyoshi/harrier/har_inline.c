/********************************************************************************/
/*	Inline Program									*/
/********************************************************************************/



/**************************<-------local function------>*************************/
/*	^$BL>A0^(B:	void SetRGBA_Char							*/
/*	^$B0z?t^(B:	int r,g,b	RGB^$B$N51EY^(B				       	*/
/*	    :	int a   	^$B&A%A%c%s%M%k51EY^(B			       	*/
/*	^$B@bL@^(B:	RGBA^$B$r%;%C%H$9$k^(B			       			*/
/********************************************************************************/
static inline void SetRGBA_Char(u_char *color, int *rgba){
    *rgba = ((color[3]<<24)|(color[2]<<16)|(color[1]<<8)|(color[0])) ;
}

/**************************<-------local function------>*************************/
/*	^$BL>A0^(B:	void SetRGBA_Char							*/
/*	^$B0z?t^(B:	int r,g,b	RGB^$B$N51EY^(B				       	*/
/*	    :	int a   	^$B&A%A%c%s%M%k51EY^(B			       	*/
/*	^$B@bL@^(B:	RGBA^$B$r%;%C%H$9$k^(B			       			*/
/********************************************************************************/
static inline void SetRGBA_CharR(u_char *color, int *rgba){
    *rgba = ((color[0]<<24)|(color[1]<<16)|(color[2]<<8)|(color[3])) ;
}


/**************************<-------local function------>*************************/
/*	^$BL>A0^(B:	int har_gameover_check_ok							*/
/*	^$B@bL@^(B:	Gameover wo check	       			*/
/********************************************************************************/
static inline int har_gameover_check_ok()
{
    if (GM_IsGameOver()){
	return 0;
    }
    if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_ELUDE_FALL) &&
	 (fabs(GM_PlayerFindPos.vx) > 2500.0f) ){
	return 0;
    }
    return 1;
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	^$BL>A0^(B:	void call_item_proc						*/
/*	^$B0z?t^(B:	Work	*work							*/
/*	^$B@bL@^(B:	^$B%"%$%F%`4XO"%W%m%C%/^(B						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static inline void har_call_item_proc(Work *work){
    if ( work->item_proc_id != 0 ){
	Kas_Work *kwork;
	GCL_ARGS	args;
	int		argv[4];

	if ( GET_KAK_WORK == NULL ){
	    return;
	}

	kwork = GET_KAK_WORK;
	args.argc = 4;

	argv[0] = kwork->item_aim_pos.vx;
	argv[1] = kwork->item_aim_pos.vy-3000.0f;
	argv[2] = kwork->item_aim_pos.vz;

	argv[3] = KAS_ITEM_NUM;
	
	args.argv = argv;
	printf ("**Put Item proc Pos:%d, %d, %d, Num:%d \n",argv[0],argv[1],argv[2],argv[3]);
	kwork->item_reserve_flag = 0;
	GCL_ExecProc(work->item_proc_id, &args);
    }
}

