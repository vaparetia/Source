/********************************************************************************/
/*	har_kasacka.c								*/
/*	カサッカ								*/
/*	2001/05/03 H.Satoyoshi							*/
/*	$Id: */
/********************************************************************************/

/********************************************************************************/
/*	Program									*/
/********************************************************************************/
//#define		GRAV	(20.0f)
#define		GRAV	(9.0f)
#define		TIME	(st_time)
static float		st_time;

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	get_speed							*/
/*	引数:	Kas_Work	*work						*/
/*		short		num	グレネード番号				*/
/*	説明:	グレネードの速度を得る						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void get_speed(Kas_Work *work, short num){

    FVECTOR	distance;
    float	lx, ly;
    _sceVu0SubVector(&distance, &work->aimtarget,	// とりあえず狙いはプレイヤ
			     (FVECTOR*)work->grn_sel[num].objs->world.m[3]);
    lx = bp_sqrtf( (distance.vx * distance.vx * 1.0f) +  //BP_MATH - emulate PS2 sqrtf
		      (distance.vz * distance.vz * 1.0f) ); 

    //    st_time = lx / 1500.0f + 6.0f;
    st_time = lx / 700.0f + 15.0f;

    ly = distance.vy;
    work->grn_speed[num].vx = lx*distance.vx/lx/TIME;
    work->grn_speed[num].vz = lx*distance.vz/lx/TIME;
    work->grn_speed[num].vy = ly/TIME + GRAV*TIME/2;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	get_speed							*/
/*	引数:	Kas_Work	*work						*/
/*		short		num	グレネード番号				*/
/*	説明:	グレネードの速度を得る						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void get_grn_aim(Kas_Work *work){
    FVECTOR	distance;
    static	FVECTOR	tmppos;
    float	lx, ly;
    FVECTOR	verts[2];
    FVECTOR	verts0[2];

    _sceVu0SubVector(&distance, &work->aimtarget,	// とりあえず狙いはプレイヤ
			     (FVECTOR*)work->snake.objs->objs[HUMAN21_MIGI_TE].world.m[3]);
    lx = bp_sqrtf( (distance.vx * distance.vx * 1.0f) +  //BP_MATH - emulate PS2 sqrtf
		      (distance.vz * distance.vz * 1.0f) ); 
    st_time = lx / 1500.0f + 6.0f;
    ly = distance.vy;
    tmppos.vx = lx*distance.vx/lx;
    tmppos.vz = lx*distance.vz/lx;
    tmppos.vy = ly + GRAV*TIME*TIME/2;

    //printf("(%6.0f, %6.0f, %6.0f) -",tmppos.vx, tmppos.vy, tmppos.vz);

    _sceVu0AddVector(&tmppos, &tmppos, (FVECTOR*)work->snake.objs->objs[HUMAN21_KUBI].world.m[3]);

    //printf(" (%6.0f, %6.0f, %6.0f)\n",tmppos.vx, tmppos.vy, tmppos.vz);

	    _sceVu0CopyVector(&verts[0], (FVECTOR*)work->snake.objs->objs[HUMAN21_MIGI_TE].world.m[3]);
	    _sceVu0CopyVector(&verts0[0], (FVECTOR*)work->snake.objs->objs[HUMAN21_MIGI_TE].world.m[3]);
	    _sceVu0CopyVector(&verts[1], &tmppos);
	    _sceVu0CopyVector(&verts0[1], &GM_PlayerFindPos);
	    //	    NewLineView(verts, 1, 250, 50, 50);
	    //	    NewLineView(verts0, 1, 250, 50, 50);

    tmppos.vx -= work->control.step.vx;
    tmppos.vy -= work->control.step.vy;
    tmppos.vz -= work->control.step.vz;
    
    work->homing_p = &tmppos;
}

extern void NewSK_Easy_Smoke2Normal( FMATRIX *world, FVECTOR *shift, char *status );
/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void shoot_grn							*/
/*	引数:	Work	*work							*/
/*	説明:	グレネードの発射						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void shoot_grn(Kas_Work *work){
    short	loop;
    for (loop=0; loop<SNAKE_GRND_NUM; loop++){
	if (!(CHECK_GRN_ACTV(loop))){
	    {
		FMATRIX	tmpmat;
		FVECTOR	tmpvec = {0.0f, -10.0f, 0.0f, 1.0f};

		{
		    _sceVu0CopyMatrix (&tmpmat, &work->m4gl.objs->world);
		    _sceVu0CopyVector((FVECTOR*)tmpmat.m[3], &DG_ZeroVector);
		    DG_SetPos(&tmpmat);
		    DG_MovePos(&tmpvec);
		    DG_GetPos(&tmpmat);
		    _sceVu0CopyVector(&work->grn_speed[loop], (FVECTOR*)tmpmat.m[3]);
		}

		DG_SetPos( &work->m4gl.objs->world  ) ;
		DG_GetPos( &work->grn_sel[loop].objs->world ) ;
		DG_GetPos( &work->grn_sel[loop].objs->objs[0].world ) ;

		work->grn_flg[loop] = 0;
		work->grn_hit[loop] = OFF;
		NewSK_Easy_Smoke2Normal(&work->grn_sel[loop].objs->world, NULL, &work->grn_flg[loop]);
		GM_SeSetMode (SD_A_KA_GRE01, (FVECTOR *)work->m4gl.objs->world.m[3], GM_SEMODE_BOMB);
		get_speed(work, loop);		//速度の計算
	    }
	    SET_GRN_ACTV(loop);
	    return;
	}
    }
}

extern void *NewBlast3( FVECTOR*, int, int, int, int, int, int, int);
/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void fly_grn							*/
/*	引数:	Work	*work							*/
/*	説明:	グレネードのあたりチェック					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int grn_hit_check(Kas_Work *work, int num, FVECTOR* frompos, FVECTOR* topos){
    FVECTOR hitpos;

    if (work->grn_hit[num] == ON){
	NewBlast3( frompos, BOTH_SIDE, 1000, 
		   3000, 40, FNT_BLAST, WP_Grenade, BLAST_TYPE_NO_NOISE|BLAST_TYPE_NO_PLAYER ) ;
	work->grn_hit[num] = OFF;
	return 1;
    }


    if (( fabs(work->control.mov.vx-topos->vx) > 3000.0f )||
	( fabs(work->control.mov.vz-topos->vz) > 3000 )){
	GM_MoveOnlineTarget( &work->grn_tgt[num], frompos, topos );
	GM_PutTarget( &work->grn_tgt[num] );
	//	printf ("AT OK\n");
    }
    
    if ( HZX_OnlineHazardCheck(0, frompos, topos, HZX_CHK_ALL|HZX_CHK_RECOIL_TYPE_ONLY,
			       (HZX_SEG_NO_BULLET&HZX_SEG_NO_MISSILE),
			       (HZX_SEG_NO_BULLET&HZX_SEG_NO_MISSILE)) ){
	
	HZX_GetOnlinePoint(&hitpos);

	hitpos.vx = (hitpos.vx*2.0f+frompos->vx)/3.0f;
	hitpos.vy = (hitpos.vy*2.0f+frompos->vy)/3.0f;
	hitpos.vz = (hitpos.vz*2.0f+frompos->vz)/3.0f;
	NewBlast3( &hitpos, BOTH_SIDE, 1000, 
		   3000, 40, FNT_BLAST, WP_Grenade, BLAST_TYPE_NO_NOISE|BLAST_TYPE_NO_PLAYER ) ;
	return 1;
    }

    return 0;
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void fly_grn							*/
/*	引数:	Work	*work							*/
/*	説明:	グレネードの飛翔						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void fly_grn(Kas_Work *work){
    short	loop;
    static FVECTOR	verts[4][2];

    for (loop=0; loop<SNAKE_GRND_NUM; loop++){
	
	if ((CHECK_GRN_ACTV(loop))){
	    
	    //	    DG_SetPos( &work->grn_sel[loop].objs->world ) ;
	    //	    DG_MovePos( &work->grn_speed[loop] );
	    //	    DG_GetPos( &work->grn_sel[loop].objs->world ) ;

	    _sceVu0AddVector((FVECTOR*)work->grn_sel[loop].objs->world.m[3], 
			     (FVECTOR*)work->grn_sel[loop].objs->world.m[3], &work->grn_speed[loop]);

	    work->grn_sel[loop].objs->world.m[3][3] = 1.0f;
	    work->grn_speed[loop].vy -= GRAV;

	    {
		SVECTOR	tmprot;
		FVECTOR oldpos;
		_sceVu0CopyVector(&verts[loop][0], &verts[loop][1]);
		_sceVu0CopyVector(&verts[loop][1], (FVECTOR*)work->grn_sel[loop].objs->world.m[3]);

		Dir_from_2Vec(&verts[loop][0], &verts[loop][1], &tmprot);

		tmprot.vx -= 1024;

		oldpos = *((FVECTOR*)work->grn_sel[loop].objs->objs[0].world.m[3]);

		DG_SetPos2( (FVECTOR*)work->grn_sel[loop].objs->world.m[3], &tmprot);
		DG_GetPos( &work->grn_sel[loop].objs->world ) ;
		DG_GetPos( &work->grn_sel[loop].objs->objs[0].world ) ;

		// ***グレネード弾の当たりチェック
		if (grn_hit_check(work, loop, &oldpos, (FVECTOR*)work->grn_sel[loop].objs->world.m[3]) == 1){
		    SET_GRN_NONACTV(loop);
		    work->grn_flg[loop] = 1;
		}
	    }

	    //	    NewLineView(verts[loop], 1, 250, 50, 50);
	    
	    if (work->grn_sel[loop].objs->world.m[3][1] < -30000.0f){
	    	SET_GRN_NONACTV(loop);
		work->grn_flg[loop] = 1;
	    }
	}
    }
}


