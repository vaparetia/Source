/********************************************************************************/
/*	har_kasacka.c								*/
/*	カサッカ								*/
/*	2001/05/03 H.Satoyoshi							*/
/*	$Id: */
/********************************************************************************/

#define SET_ITEMBOX(_pos_s, _vx, _vy, _vz)\
{\
    DG_SetPos(&work->item_box_mat);	\
    tmpvec.vx = (_vx);			\
    tmpvec.vy = (_vy);			\
    tmpvec.vz = (_vz);			\
    DG_MovePos(&tmpvec);		\
    DG_GetPos(&(_pos_s)->world);	\
}
/********************************************************************************/
/*	Program									*/
/********************************************************************************/


extern void MAO_MakeBezierMatrix( FMATRIX* pmatBezier, FVECTOR* pvec1, FVECTOR* pvec2, FVECTOR* pvec3);
extern void MAO_Bezier3InterpQuick( FVECTOR* pvecRes, FMATRIX* pmatBezier, float fRate);

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	int init_itembox						*/
/*	引数:	Work	*work							*/
/*	    :	int	where							*/
/*	説明:	アイテムボックスをいい感じにする				*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void cordinate_itembox(Kas_Work *work, float vx, float vy, float vz, short num){
    FVECTOR tmpvec;
    DG_COMDL *com;
    tmpvec.vx = vx;
    tmpvec.vy = vy;
    tmpvec.vz = vz;
    tmpvec.vw = 1.0f;

    DG_SetPos(&work->body.objs->objs[0].world);
    DG_MovePos(&tmpvec);
    DG_GetPos(&work->item_box_mat);

    com = work->kak_itembox[0+num];

    SET_ITEMBOX(com->pos, 0.0f, 0.0f, 275.0f);

    com = work->kak_itembox[1+num];
    SET_ITEMBOX(com->pos, 112.0f, 25.0f, 0.0f);

    com = work->kak_itembox[2+num];
    SET_ITEMBOX(com->pos, -112.0f, 25.0f, 0.0f);

    com = work->kak_itembox[3+num];
    SET_ITEMBOX(com->pos, 0.0f, -100.0f, 0.0f);

    com = work->kak_itembox[4+num];
    SET_ITEMBOX(com->pos, 0.0f, 0.0f, -275.0f);

}


static int PassMotion( MT3_CONTROL *mt3_ctrl )
{
	float left ;
	left = mt3_ctrl->play_time / mt3_ctrl->motion_time_base ;
	return (int)left ;
}


#define	MAX_FLAME (maxflame)	
/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	int drop_cnt_itembox2						*/
/*	引数:	Work	*work							*/
/*	説明:	投げられアイテムボックス動かす					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void drop_cnt_itembox2(Kas_Work *work){
    DG_COMDL *com;
    FVECTOR tmpvec;
    static int flame;
    static int maxflame;
    static FVECTOR	start_speed;
    static FVECTOR	oldpos;



    if (PassMotion(work->hako.m_ctrl->mt3_ctrl) == 1){
	work->hako_shift = DG_ZeroVector;
	KAS_ITEM_FLAG = 1;
    }
    if (PassMotion(work->hako.m_ctrl->mt3_ctrl) == 51){
	FVECTOR	tmpvec;

	_sceVu0SubVector(&tmpvec, &work->item_aim_pos, &work->hako.objs->objs[0].world.m[3]);
	//	MAX_FLAME = fabs(GV_VecLen3F(&tmpvec)/150.0f);	//	11/5 for PAL
		MAX_FLAME = fabs(GV_VecLen3F(&tmpvec)/30.0f);

	work->item_aim_pos.vy += 3000.0f;
	GM_ActMotion (&work->hako);

	flame = 1;
    }

    // *********まだモーション
    //    if (PassMotion(work->hako.m_ctrl->mt3_ctrl) < 70){
    if (PassMotion(work->hako.m_ctrl->mt3_ctrl) < 51){
	FVECTOR	shift = {485.0f, -290.0f, -70.0f, 1.0f};

	_sceVu0CopyMatrix (&work->hako.objs->world, &DG_UnitMatrix);
	_sceVu0CopyMatrix (&work->hako.objs->objs[0].world, &DG_UnitMatrix);
	GM_ActMotion (&work->hako);
	work->hako_step.vy = work->hako_step.vw;
	_sceVu0AddVector( &work->hako_shift,&work->hako_shift,&work->hako_step);

	_sceVu0CopyMatrix (&work->hako.objs->world, &work->snake.objs->world);
	DG_SetPos( &work->hako.objs->world );
	DG_MovePos( &shift );
	DG_MovePos( &work->hako_shift );
	DG_GetPos( &work->hako.objs->world);
	DG_GetPos(&work->hako.objs->objs[0].world);

	GM_ActObject2 (&work->hako);
	DG_GetPos(&work->item_box_mat);

	oldpos.vx = work->item_box_mat.m[3][0]-oldpos.vx;
	oldpos.vy = work->item_box_mat.m[3][1]-oldpos.vy;
	oldpos.vz = work->item_box_mat.m[3][2]-oldpos.vz;

#ifdef DEBUG_MODE
	if (HAR_ItemDbg==ON){
	    printf ("%f  %f  %f  := %d\n",oldpos.vx, oldpos.vy, oldpos.vz, (int)_FVecLen3( &oldpos));
	}
#endif

	start_speed = oldpos;

//	11/5 for PAL
	_sceVu0ScaleVector(&start_speed, &start_speed, 0.2f);


	oldpos = *((FVECTOR*)work->item_box_mat.m[3]);
	work->item_roty = 0;
    }
    // *********落下開始
    else {
	if (KAS_ITEM_FLAG != 0){
	    FVECTOR	tmpvec;


	    if (flame > MAX_FLAME){

		work->hako.objs->objs[0].world.m[3][1] -= 200;
		
		if (work->item_aim_pos.vy == -1000.0f){
		    if (work->hako.objs->objs[0].world.m[3][1] < -4000.0f){
			work->hako.objs->objs[0].world.m[3][1] = -4000.0f;
			printf ("Item Down Flag = 2\n");
			KAS_ITEM_FLAG = 2;
			NewLandingSmoke((FVECTOR*)work->hako.objs->world.m[3], 500.0f);
		    }
		}
		else {
		    if (work->hako.objs->objs[0].world.m[3][1] < -1000.0f){
			work->hako.objs->objs[0].world.m[3][1] = -1000.0f;
			printf ("Item Flag = 2\n");
			KAS_ITEM_FLAG = 2;
			NewLandingSmoke((FVECTOR*)work->hako.objs->world.m[3], 500.0f);
		    }
		}
	    }
	    else {
		//		FVECTOR	endspeed = {0.0f, -200.0f, 0.0f, 1.0f};//	11/5 for PAL
		FVECTOR	endspeed = {0.0f, -40.0f, 0.0f, 1.0f};
		MT_HermiteLerpVecScale( &tmpvec, &oldpos, &work->item_aim_pos,
					&start_speed, &endspeed,
					1.0f*flame/MAX_FLAME, 1.0f*MAX_FLAME);
#ifdef DEBUG_MODE		
		if (HAR_ItemDbg==ON){
		    View_HermiteRout( &oldpos, &work->item_aim_pos,
				      &start_speed, &endspeed, MAX_FLAME, 30);
		}
#endif
		
		//		flame ++;//	11/5 for PAL
				flame = flame + TIME_BASE;
		
		work->hako.objs->objs[0].world.m[3][0] = tmpvec.vx;
		work->hako.objs->objs[0].world.m[3][1] = tmpvec.vy;
		work->hako.objs->objs[0].world.m[3][2] = tmpvec.vz;
		tmpvec.vw = 1.0f;
	    }
	    
	    {
		SVECTOR tmprot = {0, 0, 0, 0};
		if (work->item_roty < 150){ 
		    work->item_roty += TIME_BASE;
		}
		tmprot.vy = work->item_roty;
		DG_SetPos(&work->hako.objs->objs[0].world);
		DG_RotatePos(&tmprot);
		DG_GetPos(&work->hako.objs->objs[0].world);
		DG_GetPos(&work->hako.objs->world);
		DG_GetPos(&work->item_box_mat);
	    }
	}
    }

    
    // *****各パーツの位置
    com = work->kak_itembox[0];
    SET_ITEMBOX(com->pos, 0.0f, 0.0f, 275.0f);
    com = work->kak_itembox[1];
    SET_ITEMBOX(com->pos, 112.0f, 25.0f, 0.0f);
    com = work->kak_itembox[2];
    SET_ITEMBOX(com->pos, -112.0f, 25.0f, 0.0f);
    com = work->kak_itembox[3];
    SET_ITEMBOX(com->pos, 0.0f, -100.0f, 0.0f);
    com = work->kak_itembox[4];
    SET_ITEMBOX(com->pos, 0.0f, 0.0f, -275.0f);
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	int move_itembox						*/
/*	引数:	Work	*work							*/
/*	    :	int	where							*/
/*	説明:	アイテムボックス動かす						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void move_itembox(Kas_Work *work){

    cordinate_itembox(work, -403.0f, -1874.0f, -700.0f,  5);
    cordinate_itembox(work, -150.0f, -1874.0f, -700.0f, 10);
    cordinate_itembox(work,  103.0f, -1874.0f, -700.0f, 15);
    cordinate_itembox(work, -276.0f, -1615.0f, -700.0f, 20);
    cordinate_itembox(work,  -23.0f, -1615.0f, -700.0f, 25);

}



/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	int init_itembox						*/
/*	引数:	Work	*work							*/
/*	    :	int	where							*/
/*	説明:	イニシャライズ				*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void init_itembox(Kas_Work *work, int where)
{
    DG_DEF		*def;
    DG_COMDL_POS	*pos_s;
    short		loop, loop1;
    
    //表示するモデルをセット
    def = (DG_DEF*)GV_GetCache( GV_CacheID( GV_StrCode("kck_release_case_cm"), 'k' ) );
    
    if ( def == NULL ) {
	printf( "moel not found in har_kasacka \n" ) ;
	ASSERT( 0 ) ;	    
    }
    //printf ("start\n");
    for (loop1=0; loop1<6; loop1++){
	for (loop=0; loop<5; loop++){
	    work->kak_itembox[loop+loop1*5] = DG_MakeComdl( def->models[loop].packs, DG_COMDL_SEMITRANS, 1, 0 );
	    
	    if ( work->kak_itembox[loop+loop1*5] == NULL ) {
		printf( "------------------------------------warning %d : model make objs failed in harrier kak\n", loop) ;
	    }
	    DG_QueueComdlObjs( work->kak_itembox[loop+loop1*5] );
	    GM_GroupObject(work->kak_itembox[loop+loop1*5], where);
	}
	//printf (".\n");
    }
    //printf ("end\n");

    for (loop=0; loop<30; loop++){
	pos_s = work->kak_itembox[loop]->pos;
	pos_s->color.vx = pos_s->color.vy = pos_s->color.vz = 128;
	pos_s->color.vw = 128;
    }
}
















