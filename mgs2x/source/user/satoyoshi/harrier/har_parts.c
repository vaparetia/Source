/********************************************************************************/
/*	har_parts.c								*/
/*	ハリアパーツ								*/
/*	2001/04/19 H.Satoyoshi							*/
/*	$Id: har_parts.c,v 1.1.1.3 2002/11/19 11:48:23 Yoshizawa1 Exp $		*/
/********************************************************************************/

/********************************************************************************/
/*	Program									*/
/********************************************************************************/

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void SetPartsRotete						*/
/*	引数:	FVECTOR	*mv_rot							*/
/*		float	*rot							*/
/*	説明:	クオータニオンン回転角の設定					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void SetPartsRotete(FVECTOR *mv_rot,float *rot){
    _sceVu0Normalize(mv_rot, mv_rot);
    mv_rot->vx = sinf(*rot) * mv_rot->vx;
    mv_rot->vy = sinf(*rot) * mv_rot->vy;
    mv_rot->vz = sinf(*rot) * mv_rot->vz;
    mv_rot->vw = cosf(*rot);
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void Parts_Backnoz						*/
/*	引数:	Work	*work							*/
/*		FVECTOR	*mv_rot							*/
/*	説明:	後ろノズル							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static inline void Parts_Backnoz(Work *work, FVECTOR *mv_rot){
    mv_rot->vx = 1.0f;
    mv_rot->vy = 0.0f;
    mv_rot->vz = 0.0f;
    
    SetPartsRotete(mv_rot,&work->noz_bk_rot);
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void Parts_RFlap						*/
/*	引数:	Work	*work							*/
/*		FVECTOR	*mv_rot							*/
/*	説明:	フラップ右							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static inline void Parts_RFlap(Work *work, FVECTOR *mv_rot){
    mv_rot->vx = -2426.0f+836.0f;
    mv_rot->vy = 379.0f-680.0f;
    mv_rot->vz = -1030.0f+854.0f;
    SetPartsRotete(mv_rot,&work->r_flap_rot);
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void Parts_RElron						*/
/*	引数:	Work	*work							*/
/*		FVECTOR	*mv_rot							*/
/*	説明:	エルロン右							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static inline void Parts_RElron(Work *work, FVECTOR *mv_rot){
    mv_rot->vx = -4024.0f+2622.0f;
    mv_rot->vy = -23.0f-334.0f;
    mv_rot->vz = -1440.0f+1055.0f;
    SetPartsRotete(mv_rot,&work->r_elr_rot);
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void Parts_LFlap						*/
/*	引数:	Work	*work							*/
/*		FVECTOR	*mv_rot							*/
/*	説明:	フラップ左							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static inline void Parts_LFlap(Work *work, FVECTOR *mv_rot){
    mv_rot->vx = 2426.0f-836.0f;
    mv_rot->vy = 379.0f-680.0f;
    mv_rot->vz = -1030.0f+854.0f;
    SetPartsRotete(mv_rot,&work->l_flap_rot);
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void Parts_LElron						*/
/*	引数:	Work	*work							*/
/*		FVECTOR	*mv_rot							*/
/*	説明:	エルロン左							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static inline void Parts_LElron(Work *work, FVECTOR *mv_rot){
    mv_rot->vx = 4024.0f-2622.0f;
    mv_rot->vy = -23.0f-334.0f;
    mv_rot->vz = -1440.0f+1055.0f;
    SetPartsRotete(mv_rot,&work->l_elr_rot);
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void Parts_TaleWing						*/
/*	引数:	Work	*work							*/
/*		FVECTOR	*mv_rot							*/
/*	説明:	尾翼フラップ							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static inline void Parts_TaleWing(Work *work, FVECTOR *mv_rot){
    mv_rot->vx = 0.0f;
    mv_rot->vy = 2359.0f-1175.0f;
    mv_rot->vz = -5835.0f+5528.0f;
    SetPartsRotete(mv_rot,&work->lader_rot);
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void Parts_AirIntake						*/
/*	引数:	Work	*work							*/
/*		FVECTOR	*mv_rot							*/
/*	説明:	エアインテーク							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static inline void Parts_AirIntake(Work *work, FVECTOR *mv_rot){
    mv_rot->vx = 0.0f;
    mv_rot->vy = 0.0f;
    mv_rot->vz = 1.0f;
    work->intake_rot += 0.14;
    SetPartsRotete(mv_rot,&work->intake_rot);
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void Parts_Cover						*/
/*	引数:	Work	*work							*/
/*		FVECTOR	*mv_rot							*/
/*	説明:	ホイールカバー							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static inline void Parts_Cover(Work *work, FVECTOR *mv_rot){
    mv_rot->vx = 1.0f;
    mv_rot->vy = 0.0f;
    mv_rot->vz = 0.0f;
    SetPartsRotete(mv_rot,&work->cover_rot);
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void Parts_Tale2Wing						*/
/*	引数:	Work	*work							*/
/*		FVECTOR	*mv_rot							*/
/*	説明:	水平尾翼							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static inline void Parts_Tale2Wing(Work *work, FVECTOR *mv_rot){
    mv_rot->vx = 1.0f;
    mv_rot->vy = 0.0f;
    mv_rot->vz = 0.0f;
    SetPartsRotete(mv_rot,&work->talewing_rot);
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void Parts_Frontnoz						*/
/*	引数:	Work	*work							*/
/*		FVECTOR	*mv_rot							*/
/*	説明:	前ノズル							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static inline void Parts_Frontnoz(Work *work, FVECTOR *mv_rot){
    mv_rot->vx = 1.0f;
    mv_rot->vy = 0.0f;
    mv_rot->vz = 0.0f;
    SetPartsRotete(mv_rot,&work->noz_fr_rot);
}

/*******************************<Global function>********************************/
/*	名前:	void Har_MoveParts						*/
/*	引数:	Work	*work							*/
/*	説明:	さまざまなパーツを動かす					*/
/********************************************************************************/
void Har_MoveParts(Work *work)
{
    FVECTOR	*vectmp;
    short	loop;
    FMATRIX tmpmat;
    
    vectmp = work->rots;

    DG_SetPos( &work->body.objs->world  ) ;
    
    MT_QuatToMat( &tmpmat, &vectmp[0]);
    _sceVu0MulMatrix(&work->body.objs->objs[0].world, &work->body.objs->world,
		     &tmpmat );

    for (loop=1; loop<11; loop++){
	
	if (loop == 1){				//後ろノズル
	    Parts_Backnoz(work, &vectmp[loop]);
	}
	else if (loop == 2){			//フラップ右
	    Parts_RFlap(work, &vectmp[loop]);
	}
	else if (loop == 3){			//エルロン右
	    Parts_RElron(work, &vectmp[loop]);
	}
	else if (loop == 4){			//フラップ左
	    Parts_LFlap(work, &vectmp[loop]);
	}
	else if (loop == 5){			//エルロン左
	    Parts_LElron(work, &vectmp[loop]);
	}
	else if (loop == 6){			//尾翼フラップ
	    Parts_TaleWing(work, &vectmp[loop]);
	}
	else if (loop == 7){			//エアインテーク
	    Parts_AirIntake(work, &vectmp[loop]);
	}
	else if (loop == 8){			//ホイールカバー
	    Parts_Cover(work, &vectmp[loop]);
	}
	else if (loop == 9){			//水平尾翼
	    Parts_Tale2Wing(work, &vectmp[loop]);
	}
	else if (loop == 10){			//前ノズル
	    Parts_Frontnoz(work, &vectmp[loop]);
	}
	
	MT_QuatToMat( &tmpmat, &vectmp[loop]);
	tmpmat.m[3][0] = work->body.objs->objs[loop].trans.vx;
	tmpmat.m[3][1] = work->body.objs->objs[loop].trans.vy;
	tmpmat.m[3][2] = work->body.objs->objs[loop].trans.vz;
	
	_sceVu0MulMatrix(&work->body.objs->objs[loop].world, &work->body.objs->world,
			 &tmpmat );
    }
}



