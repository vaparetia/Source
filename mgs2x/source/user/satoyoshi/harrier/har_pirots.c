/********************************************************************************/
/*	har_pilots.c								*/
/*	ハリアのパイロット							*/
/*	2001/03/23 H.Satoyoshi							*/
/*	$Id: har_pirots.c,v 1.1.1.3 2002/11/19 11:48:24 Yoshizawa1 Exp $		*/
/********************************************************************************/

/********************************************************************************/
/*	define      								*/
/********************************************************************************/

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void Die							*/
/*	引数:	Work	*work							*/
/*	説明:	終了処理:ワーク解放    						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void Pil_Die(Pil_Work *work)
{
    GM_FreeObject(&(work->body));
    GM_FreeObject(&(work->body2));
    GM_FreeObject(&(work->canp));
    GM_FreeObject(&(work->canp_br));
    GM_FreeObject(&(work->frame));
}
#define M_PI 3.14159265358979323846264338327950288419716939937510f
/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void Act							*/
/*	引数:	Work	*work							*/
/*	説明:	指定のオブジェに追従						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void Pil_Act(Pil_Work *work)
{
    OBJECT	*tmp;
    SVECTOR	pl_rot;

    tmp = work->oya_control->object;

    if (HAR_Canp_Break == ON){
	work->canp_br.objs->flag &= ~DG_FLAG_INVISIBLE;
	work->canp.objs->flag |= DG_FLAG_INVISIBLE;
    }
    else {
	work->canp.objs->flag &= ~DG_FLAG_INVISIBLE;
	work->canp_br.objs->flag |= DG_FLAG_INVISIBLE;
    }

    if ( PL_GetPlayerItem() == IT_Thermal){
	work->canp.objs->flag |= DG_FLAG_INVISIBLE;
	work->canp_br.objs->flag |= DG_FLAG_INVISIBLE;
    }
    //    else {
    //	work->canp.objs->flag &= ~DG_FLAG_INVISIBLE;
    //    }

    //マトリクスの計算 libDGを使用
    _sceVu0CopyMatrix( &work->body.objs->world, &tmp->objs->world );
    _sceVu0CopyMatrix( &work->canp.objs->world, &tmp->objs->world );
    _sceVu0CopyMatrix( &work->canp.objs->objs[0].world, &tmp->objs->world );

    // 壊れキャノピー
    _sceVu0CopyMatrix( &work->canp_br.objs->world, &tmp->objs->world );
    _sceVu0CopyMatrix( &work->canp_br.objs->objs[0].world, &tmp->objs->world );
    _sceVu0CopyMatrix( &work->canp_br.objs->objs[1].world, &tmp->objs->world );
    {
	FVECTOR	shift;

	shift.vx = -439.0f;
	shift.vy = 831.0f;
	shift.vz = 4571.0f;
	DG_SetPos(&tmp->objs->world);
	DG_MovePos(&shift);
	DG_GetPos( &work->canp_br.objs->objs[3].world );

	shift.vx = -292.0f;
	shift.vy = 1356.0f;
	shift.vz = 3180.0f;
	DG_SetPos(&tmp->objs->world);
	DG_MovePos(&shift);
	DG_GetPos( &work->canp_br.objs->objs[5].world );
    }

    _sceVu0CopyMatrix( &work->frame.objs->world, &tmp->objs->world );
    _sceVu0CopyMatrix( &work->frame.objs->objs[0].world, &tmp->objs->world );

    //	非表示フラグの設定
    work->body.objs->flag |= tmp->objs->flag & DG_FLAG_INVISIBLE;
    work->body2.objs->flag |= tmp->objs->flag & DG_FLAG_INVISIBLE;
    work->canp.objs->flag |= tmp->objs->flag & DG_FLAG_INVISIBLE;
    work->frame.objs->flag |= tmp->objs->flag & DG_FLAG_INVISIBLE;

    //パイロット１
    DG_SetPos(&tmp->objs->world);
    DG_GetPos(&work->body.objs->world);
    DG_GetPos(&work->body.objs->objs[0].world);
    DG_MovePos( &work->body.objs->objs[1].trans);
    DG_GetPos(&work->body.objs->objs[1].world);

    {
	//	FVECTOR	verts[2];
	FMATRIX	tmpmat;
	FVECTOR	tmppos;
	_sceVu0InversMatrix(&tmpmat, &work->body.objs->objs[1].world);
	_sceVu0ApplyMatrix(&tmppos, &tmpmat, &GM_PlayerFindPos);

	Dir_from_2Vec(&DG_ZeroVector, &tmppos, &pl_rot);
	DG_SetPos(&work->body.objs->objs[1].world);
	
	if ((4096-750 > pl_rot.vx)&&(pl_rot.vx > 2048)){
	    pl_rot.vx = 4096-750;
	}
	else if (( 2048 >= pl_rot.vx)&&(pl_rot.vx > 400)){
	    pl_rot.vx = 400;
	}
	if ((4096-750 > pl_rot.vy)&&(pl_rot.vy > 2048)){
	    pl_rot.vy = 4096-750;
	}
	else if ((2048 >= pl_rot.vy)&&(pl_rot.vy > 750)){
	    pl_rot.vy = 750;
	}
	DG_RotatePos(&pl_rot);
	//	_sceVu0CopyVector(&verts[0], &GM_PlayerFindPos);
	//	_sceVu0CopyVector(&verts[1], (FVECTOR*)work->body.objs->objs[1].world.m[3]);
	//	NewLineView(verts, 1, 255,0,0);
    }
    DG_GetPos(&work->body.objs->objs[1].world);

    //パイロット２
    DG_SetPos(&tmp->objs->world);
    DG_GetPos(&work->body2.objs->world);
    DG_GetPos(&work->body2.objs->objs[0].world);
    DG_MovePos( &work->body2.objs->objs[1].trans);
    DG_GetPos(&work->body2.objs->objs[1].world);

    {
	//	FVECTOR	verts[2];
	FMATRIX	tmpmat;
	FVECTOR	tmppos;
	_sceVu0InversMatrix(&tmpmat, &work->body2.objs->objs[1].world);
	_sceVu0ApplyMatrix(&tmppos, &tmpmat, &GM_PlayerFindPos);

	Dir_from_2Vec(&DG_ZeroVector, &tmppos, &pl_rot);
	DG_SetPos(&work->body2.objs->objs[1].world);

	if ((4096-750 > pl_rot.vx)&&(pl_rot.vx > 2048)){
	    pl_rot.vx = 4096-750;
	}
	else if ((2048 >= pl_rot.vx)&&(pl_rot.vx > 400)){
	    pl_rot.vx = 400;
	}
	if ((4096-750 > pl_rot.vy)&&(pl_rot.vy > 2048)){
	    pl_rot.vy = 4096-750;
	}
	else if ((2048 >= pl_rot.vy)&&(pl_rot.vy > 750)){
	    pl_rot.vy = 750;
	}

	DG_RotatePos(&pl_rot);
	//	_sceVu0CopyVector(&verts[0], &GM_PlayerFindPos);
	//	_sceVu0CopyVector(&verts[1], (FVECTOR*)work->body2.objs->objs[1].world.m[3]);
	//	NewLineView(verts, 1, 255,0,0);
    }
    DG_GetPos(&work->body2.objs->objs[1].world);

}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	int GetResources						*/
/*	引数:	Work	*work							*/
/*	    :	int	name							*/
/*	    :	int	where							*/
/*	説明:	ワークの確保＆イニシャライズ					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int Pil_GetResources(Pil_Work *work, Work *oya_work)
{
    work->oya_control = &oya_work->control;	//被装備オブジェクト登録

	if(GM_VRStatus & GM_VR_SNAKETALES) {
	    GM_InitObject(&work->body,
			  GV_StrCode("hri_pilot_gbs2"),DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_IRREACTION);
	    GM_InitObject(&work->body2,
			  GV_StrCode("hri_pilot_gbs1"),DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_IRREACTION);
	} else {
	    GM_InitObject(&work->body,
			  GV_StrCode("hri_pilot_sol"),DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_IRREACTION);
	    GM_InitObject(&work->body2,
			  GV_StrCode("hri_pilot_vmp"),DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_IRREACTION);
	}
    GM_InitObject(&work->canp,
		  GV_StrCode("hri_canopy_def_mt"  ),DG_FLAG_SHADE|DG_FLAG_SEMITRANS|DG_FLAG_FINISHCALC|DG_FLAG_FORCEMSAA);
    GM_InitObject(&work->canp_br,
		  GV_StrCode("hri_canopy_high_brk_50hlf_mt"),
		  DG_FLAG_SHADE|DG_FLAG_SEMITRANS|DG_FLAG_FINISHCALC|DG_FLAG_FORCEMSAA);
    GM_InitObject(&work->frame,
		  GV_StrCode("hri_canopy_frame_def"  ),DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_IRREACTION);

    HAR_Canp_Break = OFF;

    return 1;
}

/*******************************<Global function>********************************/
/*	名前:	void *NewHarPilots						*/
/*	引数:	Work *object							*/
/*		short num							*/
/*	説明:	パイロットを表示						*/
/********************************************************************************/
void *NewHarPilots(Work *object)
{
    Pil_Work *work ;
    work = (Pil_Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Pil_Work ), 255 ) ;
    if(work!=NULL) {
	GV_SetActor(&(work->actor),Pil_Act,Pil_Die) ;
	GV_ActorEX (&work->actor ) ;
	if(!Pil_GetResources(work, object)){
	    GV_DestroyActor(work) ;
	    return NULL ;
	}
    }
    return (void *)work ;
}





