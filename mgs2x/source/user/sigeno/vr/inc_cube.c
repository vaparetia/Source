
//#define TRG_VIEW 1


static	void	ChildTargCallBack_Parent( TARGET * off, TARGET * def, void * ptr )
{
	Work	*work ;
	char str[24] ;
	work = ( Work * )ptr ;
//	str = "hit" ;
    if ( def->damaged & TARGET_POWER ) {
		if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
#if 0
		 	if( def->weapon_type & (WP_BULLET|WP_M92)){
#else
		 	if( 1){
#endif
//				sprintf(str,"hitpos %d",(int)def->hit.vy) ;
				sprintf(str,"PARENT") ;
			}
		}
	}
	def->weapon_type = 0 ;
}
static	void	ChildTargCallBack_NG( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
	Work	*work ;
//	char str[24] ;
	work = ( Work * )ptr ;
//	str = "hit" ;
    if ( def->damaged & TARGET_POWER ) {
		if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
		 	if( 1){
				work->finish = VR_TrgHitFunc(work->str_work,&def->hit,&def->center,&def->power->force,
					VR_TRG_NG,VR_TRG_TYPE_FIX_CUBE) ;
				work->amb_cnt = VR_TRG_FLS_CNT ;
			}
		}
	}
	def->weapon_type = 0 ;
}


static	void	ChildTargCallBack_01( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
	Work	*work ;
//	char str[24] ;
	work = ( Work * )ptr ;
//	str = "hit" ;
    if ( def->damaged & TARGET_POWER ) {
		if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
			def->power->force = off->power->force ;
//		 	if( def->weapon_type & WP_BULLET){
#if 0
		 	if( def->weapon_type & (WP_BULLET|WP_M92)){
#else
		 	if( 1){
#endif
				work->finish = VR_TrgHitFunc(work->str_work,&def->hit,&def->center,&def->power->force,
					VR_TRG_LEVEL1,VR_TRG_TYPE_FIX_CUBE) ;
				work->vital+= VR_GetPartsDmg(VR_TRG_TYPE_FIX_CUBE,1) ; 
				work->amb_cnt = VR_TRG_FLS_CNT ;
				if(work->vital < VR_CUBE_LIFE_MAX){
					GM_SeSetMode(SD_A_V_MTHIBI,&def->hit,GM_SEMODE_BOMB) ;
				}
			}else if( def->weapon_type & WP_BLAST){
				/*爆発系ヒット*/
			}
		}
	}
	def->weapon_type = 0 ;
}

static	void	ChildTargCallBack_02( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
	Work	*work ;
//	char str[24] ;
	work = ( Work * )ptr ;
//	str = "hit" ;
    if ( def->damaged & TARGET_POWER ) {
		if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
			def->power->force = off->power->force ;
#if 0
		 	if( def->weapon_type & (WP_BULLET|WP_M92)){
#else
		 	if(1){
#endif
				work->finish = VR_TrgHitFunc(work->str_work,&def->hit,&def->center,&def->power->force,
					VR_TRG_LEVEL2,VR_TRG_TYPE_FIX_CUBE) ;
				work->vital+= VR_GetPartsDmg(VR_TRG_TYPE_FIX_CUBE,2) ; 
				work->amb_cnt = VR_TRG_FLS_CNT ;
				if(work->vital < VR_CUBE_LIFE_MAX){
					GM_SeSetMode(SD_A_V_MTHIBI,&def->hit,GM_SEMODE_BOMB) ;
				}
			}else if( def->weapon_type & WP_BLAST){
				/*爆発系ヒット*/
			}
		}
	}
	def->weapon_type = 0 ;
}
static	void	ChildTargCallBack_03( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
	Work	*work ;
//	char str[24] ;
	work = ( Work * )ptr ;
//	str = "hit" ;
    if ( def->damaged & TARGET_POWER ) {
		if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
			def->power->force = off->power->force ;
#if 0
		 	if( def->weapon_type & (WP_BULLET|WP_M92)){
#else
		 	if(1){
#endif
				work->finish = VR_TrgHitFunc(work->str_work,&def->hit,&def->center,&def->power->force,VR_TRG_LEVEL3,VR_TRG_TYPE_FIX_CUBE) ;
				work->vital+= VR_GetPartsDmg(VR_TRG_TYPE_FIX_CUBE,3) ; 
				work->amb_cnt = VR_TRG_FLS_CNT ;
				if(work->vital < VR_CUBE_LIFE_MAX){
					GM_SeSetMode(SD_A_V_MTHIBI,&def->hit,GM_SEMODE_BOMB) ;
				}
			}else if( def->weapon_type & WP_BLAST){
				/*爆発系ヒット*/
			}
		}
	}
	def->weapon_type = 0 ;
}

static void MulFvec(FVECTOR* fv, float scale){
	fv->vx*=scale ;
	fv->vy*=scale ;
	fv->vz*=scale ;
}

static inline void setfvec(FVECTOR *fv,float x,float y,float z){
	fv->vx = x ;
	fv->vy = y ;
	fv->vz = z ;
}
static void SetTargetCube(work)
Work *work;
{
	int flag,map;
	int i;
	FVECTOR	b_size = { BODY_SIZE, BODY_SIZE_Y, BODY_SIZE_Z ,1.0f} ;
	FVECTOR	core_size01[LV1_NUM] ;
	FVECTOR	core_size02[LV2_NUM] ;
	FVECTOR	core_size03[LV3_NUM] ;
	FVECTOR	off_set01[LV1_NUM] ;
	FVECTOR	off_set02[LV2_NUM] ;
	FVECTOR	off_set03[LV3_NUM] ;

	work->trg_lv1_num = LV1_NUM;
	work->trg_lv2_num = LV2_NUM;
	work->trg_lv3_num = LV3_NUM;

	work->core_trg01 = GV_Malloc( sizeof(TARGET)*LV1_NUM ) ;
	work->core_trg02 = GV_Malloc( sizeof(TARGET)*LV2_NUM ) ;
	work->core_trg03 = GV_Malloc( sizeof(TARGET)*LV3_NUM ) ;

	ASSERT(work->core_trg01 != NULL) ;
	ASSERT(work->core_trg02 != NULL) ;
	ASSERT(work->core_trg03 != NULL) ;



	setfvec(&off_set01[0],0.0f,0.0f,0.0f);
	setfvec(&core_size01[0],1000.0f,120.0f,1000.0f);

	setfvec(&off_set02[0],310.0f,385.0f,-385.0f);
	setfvec(&core_size02[0],380.0f,230.0f,230.0f);

	setfvec(&off_set02[1],385.0f,385.0f,-195.0f);
	setfvec(&core_size02[1],230.0f,230.0f,150.0f);

	setfvec(&off_set02[2],385.0f,195.0f,-385.0f);
	setfvec(&core_size02[2],230.0f,150.0f,230.0f);

	setfvec(&off_set02[3],310.0f,385.0f,385.0f);
	setfvec(&core_size02[3],380.0f,230.0f,230.0f);

	setfvec(&off_set02[4],385.0f,385.0f,195.0f);
	setfvec(&core_size02[4],230.0f,230.0f,150.0f);

	setfvec(&off_set02[5],385.0f,195.0f,385.0f);
	setfvec(&core_size02[5],230.0f,150.0f,230.0f);

	setfvec(&off_set02[6],-310.0f,385.0f,385.0f);
	setfvec(&core_size02[6],380.0f,230.0f,230.0f);

	setfvec(&off_set02[7],-385.0f,385.0f,195.0f);
	setfvec(&core_size02[7],230.0f,230.0f,150.0f);

	setfvec(&off_set02[8],-385.0f,195.0f,385.0f);
	setfvec(&core_size02[8],230.0f,150.0f,230.0f);

	setfvec(&off_set02[9],-310.0f,385.0f,-385.0f);
	setfvec(&core_size02[9],380.0f,230.0f,230.0f);

	setfvec(&off_set02[10],-385.0f,385.0f,-195.0f);
	setfvec(&core_size02[10],230.0f,230.0f,150.0f);

	setfvec(&off_set02[11],-385.0f,195.0f,-385.0f);
	setfvec(&core_size02[11],230.0f,150.0f,230.0f);

	setfvec(&off_set02[12],310.0f,-385.0f,-385.0f);
	setfvec(&core_size02[12],380.0f,230.0f,230.0f);

	setfvec(&off_set02[13],385.0f,-385.0f,-195.0f);
	setfvec(&core_size02[13],230.0f,230.0f,150.0f);

	setfvec(&off_set02[14],385.0f,-195.0f,-385.0f);
	setfvec(&core_size02[14],230.0f,150.0f,230.0f);

	setfvec(&off_set02[15],310.0f,-385.0f,385.0f);
	setfvec(&core_size02[15],380.0f,230.0f,230.0f);

	setfvec(&off_set02[16],385.0f,-385.0f,195.0f);
	setfvec(&core_size02[16],230.0f,230.0f,150.0f);

	setfvec(&off_set02[17],385.0f,-195.0f,385.0f);
	setfvec(&core_size02[17],230.0f,150.0f,230.0f);

	setfvec(&off_set02[18],-310.0f,-385.0f,385.0f);
	setfvec(&core_size02[18],380.0f,230.0f,230.0f);

	setfvec(&off_set02[19],-385.0f,-385.0f,195.0f);
	setfvec(&core_size02[19],230.0f,230.0f,150.0f);

	setfvec(&off_set02[20],-385.0f,-195.0f,385.0f);
	setfvec(&core_size02[20],230.0f,150.0f,230.0f);

	setfvec(&off_set02[21],-310.0f,-385.0f,-385.0f);
	setfvec(&core_size02[21],380.0f,230.0f,230.0f);

	setfvec(&off_set02[22],-385.0f,-385.0f,-195.0f);
	setfvec(&core_size02[22],230.0f,230.0f,150.0f);

	setfvec(&off_set02[23],-385.0f,-195.0f,-385.0f);
	setfvec(&core_size02[23],230.0f,150.0f,230.0f);

	setfvec(&off_set03[0],	0.0f,	0.0f,	0.0f) ;
	setfvec(&core_size03[0],1000.0f,1000.0f,1000.0f) ;
	/*ターゲット設定*/
	flag =(TARGET_DEFENSE|TARGET_SEEK|TARGET_ROTATE|TARGET_POWER);

	GM_CurrentMap =	map = work->where ;
	MulFvec(&b_size, VR_TARGET01_SCALE) ;
	GM_SetTarget( &work->b_trg, (flag|TARGET_THROUGH|TARGET_CHILD_ALWAYS|TARGET_LOCKON),map, ENEMY_SIDE, &b_size, &DG_ZeroVector ) ;
    GM_SetTargetWeaponType( &work->b_trg, 0 ) ;
    GM_SetPowerTarget( &work->b_trg, &work->b_power, POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;
	GM_PutTarget( &work->b_trg );

	for(i=0;i<LV1_NUM;i++){
		MulFvec(&core_size01[i], 0.5f*VR_TARGET01_SCALE) ;
		MulFvec(&off_set01[i], VR_TARGET01_SCALE) ;
		GM_SetTarget( &work->core_trg01[i], flag,map, ENEMY_SIDE, &core_size01[i], &off_set01[i] ) ;
	}
	for(i=0;i<LV2_NUM;i++){
		MulFvec(&core_size02[i], 0.5f*VR_TARGET01_SCALE) ;
		MulFvec(&off_set02[i], VR_TARGET01_SCALE) ;
		GM_SetTarget( &work->core_trg02[i], flag,map, ENEMY_SIDE, &core_size02[i], &off_set02[i] ) ;
	}
	for(i=0;i<LV3_NUM;i++){
		MulFvec(&core_size03[i], 0.5f*VR_TARGET01_SCALE) ;
		MulFvec(&off_set03[i], VR_TARGET01_SCALE) ;
		GM_SetTarget( &work->core_trg03[i], flag,map, ENEMY_SIDE, &core_size03[i], &off_set03[i] ) ;
	}

	if(work->type & VR_TARGET_TYPE_NG){
		GM_SetTargetCallBack( &work->b_trg, ChildTargCallBack_NG, work ) ;
		for(i=0;i<LV1_NUM;i++){
			GM_SetTargetCallBack( &work->core_trg01[i], ChildTargCallBack_NG, work ) ;
		}
		for(i=0;i<LV1_NUM;i++){
			GM_SetTargetCallBack( &work->core_trg02[i], ChildTargCallBack_NG, work ) ;
		}
		for(i=0;i<LV1_NUM;i++){
			GM_SetTargetCallBack( &work->core_trg03[i], ChildTargCallBack_NG, work ) ;
		}
	}else {
		GM_SetTargetCallBack( &work->b_trg, ChildTargCallBack_Parent, work ) ;
		for(i=0;i<LV1_NUM;i++){
			GM_SetTargetCallBack( &work->core_trg01[i], ChildTargCallBack_01, work ) ;
		}
		for(i=0;i<LV2_NUM;i++){
			GM_SetTargetCallBack( &work->core_trg02[i], ChildTargCallBack_02, work ) ;
		}
		for(i=0;i<LV3_NUM;i++){
			GM_SetTargetCallBack( &work->core_trg03[i], ChildTargCallBack_03, work ) ;
		}
	}
	for(i=0;i<LV1_NUM;i++){
		GM_SetTargetWeaponType(&work->core_trg01[i],0);
		GM_SetPowerTarget(&work->core_trg01[i],&work->b_power,POWER_DECREASE,500,0,0,&DG_ZeroVector );
		GM_SetTargetParts( &work->b_trg, &work->core_trg01[i],1, 0 ) ;
	}
	for(i=0;i<LV2_NUM;i++){
		GM_SetTargetWeaponType(&work->core_trg02[i],0);
		GM_SetPowerTarget(&work->core_trg02[i],&work->b_power,POWER_DECREASE,500,0,0,&DG_ZeroVector );
		GM_SetTargetParts( &work->b_trg, &work->core_trg02[i],1, 1 ) ;
	}
	for(i=0;i<LV3_NUM;i++){
		GM_SetTargetWeaponType(&work->core_trg03[i],0);
		GM_SetPowerTarget(&work->core_trg03[i],&work->b_power,POWER_DECREASE,500,0,0,&DG_ZeroVector );
		GM_SetTargetParts( &work->b_trg, &work->core_trg03[i],1, 2 ) ;
	}
#ifdef TRG_VIEW
	if(work->type & VR_TARGET_TYPE_NG){
		NewTargetView( &work->core_trg03[0], 0, 0, 255 ) ;
		NewTargetView( &work->core_trg03[1], 0, 0, 255 ) ;
	}else {
		NewTargetView( &work->b_trg, 0, 255, 0 ) ;

		for(i=0;i<LV1_NUM;i++){
//			NewTargetView( &work->core_trg01[i], 255, 0, 0 ) ;
		}
		for(i=0;i<LV2_NUM;i++){
//			NewTargetView( &work->core_trg02[i], 255, 255, 0 ) ;
		}
		for(i=0;i<LV3_NUM;i++){
//			NewTargetView( &work->core_trg03[i], 255, 255, 255 ) ;
		}
	}
#endif
//	VR_GetTargetLife(VR_TRG_TYPE_FIX_CUBE,&work->trg01_dmg,&work->trg02_dmg,&work->trg03_dmg) ;
}

static void VR_AllTrgSkip(Work *work){
	int	i;
	GM_SetTargetSize( &work->b_trg, &DG_ZeroVector ) ;
	for(i=0;i<work->trg_lv1_num;i++){
		GM_SetTargetSize( &work->core_trg01[i], &DG_ZeroVector ) ;
		work->core_trg01[i].class |= TARGET_SKIP ;
	}
	for(i=0;i<work->trg_lv2_num;i++){
		GM_SetTargetSize( &work->core_trg02[i], &DG_ZeroVector ) ;
		work->core_trg02[i].class |= TARGET_SKIP ;
	}
	for(i=0;i<work->trg_lv3_num;i++){
		GM_SetTargetSize( &work->core_trg03[i], &DG_ZeroVector ) ;
		work->core_trg03[i].class |= TARGET_SKIP ;
	}
	work->b_trg.class |= TARGET_SKIP ;
}
static void VRMoveTrgCube(Work *work ,FVECTOR *t_pos)
{
	FMATRIX	mat;
	int i;
	DG_SetPos2(t_pos,&DG_ZeroSVector);
	DG_GetPos(&mat) ;
	GM_MoveTarget2( &work->b_trg, &mat ) ;
	for(i=0;i<work->trg_lv1_num;i++){
		GM_MoveTarget2( &work->core_trg01[i], &mat ) ;
	}
	for(i=0;i<work->trg_lv2_num;i++){
		GM_MoveTarget2( &work->core_trg02[i], &mat ) ;
	}
	for(i=0;i<work->trg_lv3_num;i++){
		GM_MoveTarget2( &work->core_trg03[i], &mat ) ;
	}
}
