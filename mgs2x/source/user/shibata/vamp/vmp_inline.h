/*
    vmp_inline.h

	$Id: vmp_inline.h,v 1.1.1.3 2002/11/19 11:48:57 Yoshizawa1 Exp $

*/
#ifndef VAMP_INLINE_H
#define VAMP_INLINE_H

static inline float _Vu0VecLenXYZ( FVECTOR *vec )
{
	float	ans;
	
#ifdef BP_PSX2_ASM
	asm volatile ("
    lqc2		vf4, 0x00(%1)
    vmul.xyz    vf5, vf4, vf4
    vmulax.w	ACC, vf0, vf5x
    vmadday.w	ACC, vf0, vf5y
    vmaddz.w	vf5, vf0, vf5z
	vsqrt		Q, vf5w
	vwaitq
	vaddq.x		vf6, vf0, Q
    qmfc2.i		$8,vf6
    sw			$8,0(%0)
	": : "r"(&ans), "r"(vec) : "$8", "memory" );
#else
	ans = GV_VecLen3F( vec ) ;
#endif

	return (ans);
}

static inline float _Vu0VecLenXZ( FVECTOR *vec )
{
	float	ans;

#ifdef BP_PSX2_ASM	
	asm volatile ("
    lqc2		vf4, 0x00(%1)
    vmul.xyz    vf5, vf4, vf4
    vmulax.w	ACC, vf0, vf5x
    vmaddz.w	vf5, vf0, vf5z
	vsqrt		Q, vf5w
	vwaitq
	vaddq.x		vf6, vf0, Q
    qmfc2.i		$8,vf6
    sw			$8,0(%0)
	": : "r"(&ans), "r"(vec) : "$8", "memory" );
#else
	float a ; 

	a = (vec->vx*vec->vx) + (vec->vz*vec->vz) ;
	ans = bp_sqrtf ( a ) ;  //BP_MATH - emulate PS2 sqrtf
#endif

	return (ans);
}

static inline void _Vu0GetCenterVec2( FVECTOR *center, FVECTOR *vec0, FVECTOR *vec1 )
{
#ifdef BP_PSX2_ASM
	asm volatile ("
    lqc2		vf4, 0x00(%1)
    lqc2		vf5, 0x00(%2)
	qmtc2.ni	%3,  vf3

    vsub.xyzw   vf6, vf5, vf4
	vmulx.xyzw	vf7, vf6, vf3
	vadd.xyzw	vf8, vf4, vf7

    sqc2		vf8, 0x00(%0)
	
	": : "r"(center), "r"(vec0), "r"(vec1), "r"(0.5f) : "$8", "memory" );
#else
	FVECTOR tmp ;
	
	_sceVu0SubVector( &tmp, vec1, vec0 );
	_sceVu0ScaleVector( &tmp, &tmp, 0.5f );
	_sceVu0AddVector( center, vec0, &tmp );
#endif
}

static inline void _Vu0GetScaleVec2( FVECTOR *center, FVECTOR *vec0, FVECTOR *vec1, float len )
{
#ifdef BP_PSX2_ASM
	asm volatile ("
    lqc2		vf4, 0x00(%1)
    lqc2		vf5, 0x00(%2)
	qmtc2.ni	%3,  vf3

    vsub.xyzw   vf6, vf5, vf4
	vmulx.xyzw	vf7, vf6, vf3
	vadd.xyzw	vf8, vf4, vf7

    sqc2		vf8, 0x00(%0)
	
	": : "r"(center), "r"(vec0), "r"(vec1), "r"(len) : "$8", "memory" );
#endif
}

static inline void _RotTrans( FVECTOR *out, FMATRIX *world, FVECTOR *in )
{
#ifdef BP_PSX2_ASM
	asm ("
	lqc2				vf8,0x00(%2)
	lqc2				vf4,0x00(%1)
	lqc2				vf5,0x10(%1)
	lqc2				vf6,0x20(%1)
	lqc2				vf7,0x30(%1)
	vmulax.xyzw			ACC, vf4,vf8
	vmadday.xyzw		ACC, vf5,vf8
	vmaddaz.xyzw		ACC, vf6,vf8
	vmaddw.xyzw			vf8, vf7,vf8
	sqc2				vf8,0x00(%0)
	": : "r"(out), "r"(world), "r"(in) );//:"memory" );
#else
	DG_SetPos( world ) ;
	DG_RotVectorW(in, out, 1 ) ;
#endif
}

static inline void VMP_ResetCtrlHeight( CONTROL *ctrl, OBJECT *body, float height )
{
	MOTION_CONTROL	*m_ctrl ;

	m_ctrl = body->m_ctrl ;

	/* 高さ調整 */
	//height = m_ctrl->mt3_ctrl[ 0 ].move->step.vw ;
	m_ctrl->height = height ;
	m_ctrl->old_height = height ;
	body->height = height ;
	ctrl->height = height ;

	ctrl->hzx_base = ctrl->mov.vy - height ;

	/* ルート位置調整 */
	m_ctrl->root_height = m_ctrl->root_old_height = m_ctrl->mt3_ctrl[ 0 ].root->step.vw ;

	/* step.vyは0に */
	ctrl->step.vy = 0.0F ;

	//VMP_PRINTF("aaaaaaaaa	%f\n",height);
}

//じゅーりょくー
static inline void VMP_Gravitation( CONTROL *ctrl, OBJECT *body, float old, int status,
									FVECTOR *diff, FVECTOR *step_scl, float height )
{
	if ( !(ctrl->skip_flag & CTRL_SKIP_FLR_CHECK) ){
		if ( ctrl->grounded & 1 ) ctrl->step.vy = 0.0F;
		ctrl->step.vy -= 16.0F;
	} else {
		// Y移動量はモーションに任せる
		ctrl->step.vy = body->height - old;
	}

	ctrl->height = body->height;

	if( CHECK_FLAG( status, VMP_STATUS_RESET_H ) ){
		VMP_ResetCtrlHeight( ctrl, body, height );
	}

	if( CHECK_FLAG( status, VMP_STATUS_MUL_STEP) ){
		_sceVu0MulVector( &ctrl->step, &ctrl->step, step_scl );
	}

	if( CHECK_FLAG( status, VMP_STATUS_ADD_STEP ) ){
		//if( 0 && diff->vy < 0.0f ){
		//	PRINT_PFVEC(0,diff);
		//}
		_sceVu0AddVector( &ctrl->step, &ctrl->step, diff );
	}
}


//コントロールの基本処理
static inline void VMP_ControlAct( CONTROL *ctrl, OBJECT *body, FMATRIX *lights,
								   float old, int status, FVECTOR *diff, FVECTOR *step_scl, float height )
{
    GM_ActMotion( body );
	VMP_Gravitation( ctrl, body, old, status, diff, step_scl, height );
	GM_ActControl( ctrl );
	GM_ActObject2( body );
	
	DG_GetLightMatrix( &ctrl->mov, lights );
    /* ＳＥ変換用 */
    //MT_SetMotionSeTable( entk->act->body->m_ctrl, GM_CurrentMap, (entk->id%4)+1, 
	//					 ( entk->ctrl->flr_atrs[ 0 ] & 0xf0000000 ) >> 28, 0 );
}



static inline void VMP_ActStatusCheck( CONTROL *ctrl, OBJECT *body )
{
	int correct;

	/* ターンフラグ */
	if( ( correct = body->m_ctrl->rot_correct ) ){
		VMP_PRINTF("correct[%d] rot[%d]\n",correct, ctrl->rot.vy);
		ctrl->rot.vy += correct;
		ctrl->rot.vy &=4095;
		ctrl->turn.vy = ctrl->rot.vy ;

		/* X軸方向の傾きは体が正面を向いている場合しかしない */
		if ( correct > 1024 || correct < -1024) {
			ctrl->rot.vx = ctrl->turn.vx = - ctrl->turn.vx ;
		}
		body->m_ctrl->rot_correct = 0;
	}
}

//IKとやら
static inline void VMP_IkControl( void *ik_work, int status )
{
	TAKABE_UtilPuppetIK_AutoConfigOfEnemey( ik_work, -1 );
	TAKABE_ActPuppetIK( ik_work );
}

static inline void VMP_ClearMotionStatus( Work *work )
{
	work->think_mul_step.vx = 1.0f;
	work->think_mul_step.vy = 1.0f;
	work->think_mul_step.vz = 1.0f;
	work->think_mul_step.vw = 1.0f;

	//work->think_mot_step.vx = 0.0f;
	//work->think_mot_step.vy = 0.0f;
	//work->think_mot_step.vz = 0.0f;
	//work->think_mot_step.vw = 0.0f;
	
	//MOTION_CONTROL	*m_ctrl = body->m_ctrl;
	work->target.class &= ~TARGET_SKIP;
	work->body.flag &= ~OBJECT_MOTIONSTEP_THROUGH;
	work->control.skip_flag &= ~CTRL_SKIP_HZX;
	work->body.m_ctrl->flag &= ~(MT_FLAG_REVERSAL1|MT_FLAG_REVERSAL2);
}

//モーションセット基本
static inline void VMP_SetActMotion( Work *work, int mot_num )
{
	MOTION_CONTROL	*m_ctrl = work->body.m_ctrl;
	
	VMP_PRINTF("set_mot [%4d]\n",mot_num);
	VMP_ClearMotionStatus( work );
	work->active_mot = mot_num;
	GM_ConfigObjectAction( &work->body, 0, mot_num, 0, 0x1fffff, ACT_INTERP_DEF );
	VMP_ActStatusCheck( &work->control, &work->body );
	MT_SetMotionSpeed( m_ctrl, (float)TIME_BASE );
}

//モーションセット基本
static inline void VMP_SetActMotionNoIntrp( Work *work, int mot_num )
{
	MOTION_CONTROL	*m_ctrl = work->body.m_ctrl;

	VMP_PRINTF("set_mot [%4d]\n",mot_num);
	VMP_ClearMotionStatus( work );
	work->active_mot = mot_num;
	GM_ConfigObjectAction( &work->body, 0, mot_num, 0, 0x1fffff, 0 );
	VMP_ActStatusCheck( &work->control, &work->body );
	MT_SetMotionSpeed( m_ctrl, (float)TIME_BASE );
}

//モーションセット基本
static inline void VMP_SetActMotionObj( OBJECT *body, int mot_num, float time_base )
{
	MOTION_CONTROL	*m_ctrl = body->m_ctrl;

	//VMP_ClearMotionStatus( work );
	VMP_PRINTF("set_mot [%4d]\n",mot_num);
	GM_ConfigObjectAction( body, 0, mot_num, 0, 0x1fffff, ACT_INTERP_DEF );
	MT_SetMotionSpeed( m_ctrl, time_base );
}

//モーションセットチェック基本
static inline void VMP_SetActMotionCheck( Work *work, int mot_num )
{
	MOTION_CONTROL	*m_ctrl = work->body.m_ctrl;

	if( work->active_mot == mot_num ) return;
	VMP_ClearMotionStatus( work );
	work->active_mot = mot_num;
	VMP_PRINTF("set_mot [%4d]\n",mot_num);
	GM_ConfigObjectAction( &work->body, 0, mot_num, 0, 0x1fffff, ACT_INTERP_DEF );
	VMP_ActStatusCheck( &work->control, &work->body );
	MT_SetMotionSpeed( m_ctrl, (float)TIME_BASE );
}

//モーションセット
static inline void VMP_SetActMotionEx( Work *work, int mot_num, int layer, int mask, int time )
{
	MOTION_CONTROL	*m_ctrl = work->body.m_ctrl;

	//work->active_mot = mot_num;
	VMP_ClearMotionStatus( work );
	VMP_PRINTF("set_mot [%4d]\n",mot_num);
	GM_ConfigObjectAction( &work->body, layer, mot_num, time*5, mask, ACT_INTERP_DEF );
	VMP_ActStatusCheck( &work->control, &work->body );
	MT_SetMotionSpeed( m_ctrl, (float)TIME_BASE );
}

//モーションタイムベースセット
static inline void VMP_SetActMotionTimeBase( Work *work, float time_base )
{
	MOTION_CONTROL	*m_ctrl = work->body.m_ctrl;
	MT_SetMotionSpeed( m_ctrl, time_base );
}


//モーションアクトセット
static inline void VMP_SetActCall( Work *work, void *call )
{
	work->ActFunc_call = call;
	work->act_timer = -1;
	
	//work->think_mot_step.vx = 0.0f;
	//work->think_mot_step.vy = 0.0f;
	//work->think_mot_step.vz = 0.0f;
	//work->think_mot_step.vw = 0.0f;
}

//次のモーションとモーションアクトセット
static inline void VMP_SetActCallMotion( Work *work, int mot_num, void *call )
{
	//if( CHECK_FLAG( work->status, VMP_STATUS_NO_SETACT ) && mot_num != VMP_MOT_KNF_GUNDAM ){
	//	VMP_PRINTF( "Cannot set act[%d]\n", mot_num );
	//	return;
	//}
	work->next_mot = mot_num;
	VMP_SetActCall( work, call );
}

//ターゲット移動
static inline void VMP_MoveTarget( Work *work, int *joint_num )
{
	TARGET		*deftrg, *child;
	int			i,*joint_index = joint_num;
	CONTROL		*ctrl = &work->control;
	OBJECT		*body = &work->body;
	
	deftrg = &work->target;
	child = work->def_child;

	//親
	GM_MoveTarget( deftrg, &ctrl->mov );
	//攻撃（セクシー）
	//GM_MoveTarget2( &work->knf_trgt, &(BODYWORLD( body, 6 )) );

	for( i = 0; i < CHILD_TARGET_NUM; i++ ){
		//子供
		GM_MoveTarget2( child, &(BODYWORLD( body, *joint_index )) ) ;
		child++;
		joint_index++;
	}

}

//ダメージフラグクリア
static inline void VMP_DamageFlagClear( Work *work )
{
	TARGET		*deftrg = &work->target;
	TARGET		*child = work->def_child;	
	int			i ;
	
	deftrg->weapon_type = 0 ;
	deftrg->damaged = 0 ;

	for( i = 0; i < CHILD_TARGET_NUM; i++ ) {
		child->weapon_type = 0;
		child->damaged = 0;
		child++;
	}
}

// ダメージを受けたチャイルドターゲット番号を返す
static inline int VMP_ChildTargetCheck( TARGET *target )
{
	TARGET		*def_child = target;
	int			i;
	
	for( i = 0; i < CHILD_TARGET_NUM; i++ ){
		if(TARGET_POWER & def_child->damaged) return i;
		def_child++;
	}

	return -1;
}

static inline void VMP_ClearAdjust( OBJECT *body, int joint )
{
	body->m_ctrl->adjust_flag &= ~(1<<joint);
}

static inline void VMP_SetAdjustWork( Work *work, SVECTOR *to, int joint )
{	
	work->adjust_flags |= (1<<joint);
	work->to_adjust[joint] = *to;
}

static inline void VMP_SetAdjustBody( Work *work )
{
	MOTION_CONTROL	*m_ctrl;
	SVECTOR			*from,*to;
	int				i;

	from = work->from_adjust;
	to = work->to_adjust;
	m_ctrl = work->body.m_ctrl;
	m_ctrl->adjust_flag = 0;

	for( i = 0; i < HUMAN_JOINT; i++ ){
		FVECTOR			quat0;//,quat1;
		long64			*check;

		if( work->adjust_flags & (1<<i) ){
			GV_NearExp4PV( from, to, 3 );
			check = (long64*)from;

			if( ((*check)&(~I64(0))) == 0 ){
				work->adjust_flags &= ~(1<<i);
			}
			m_ctrl->adjust_flag |= (1LL /* BP - Added LL because adjust_flag is a uint64 */ <<i);

			if( i == 12 ){
				GM_RotToQuatXAfterY( from, &quat0 );
			}else{
				if( from->vy != 0 ){
					GM_RotToQuat( from, &quat0 );
				} else {
					GM_RotToQuatXAfterY( from, &quat0 );
				}
			}
			
			DG_COPY_VEC( &m_ctrl->adjust[i], &quat0 );
		}
		from++;
		to++;
	}
}

static inline void VMP_SetThinkSub( Work *work, short nxt_sub, short nxt_lcl )
{
	work->think_pre_sub = work->think_sub;
	work->think_sub = nxt_sub;
	work->think_local = nxt_lcl;
	work->think_timer = -1;
}

static inline void VMP_SetThinkMulStep( Work *work, float x, float y, float z )
{
	work->think_mul_step.vx = x;
	work->think_mul_step.vy = y;
	work->think_mul_step.vz = z;
}

static inline int VMP_StartStream( Work *work, int vox_num )
{
	if( vox_num >= work->n_vox ){
		VMP_PRINTF( "ERR!! vox_num[%d]\n", vox_num );
		return -1;
	}
	if( work->vox_timer >= 0 ){
		VMP_PRINTF( "now streaming\n" );
		return -1;
	}
	if ( work->vox_id[vox_num] < 0 ){
		VMP_PRINTF( "no streaming\n", vox_num );
		return -1;
	}
	work->stream_handler = GM_VoxStream( work->vox_id[vox_num], 0 );//GM_STREAM_PLAY_WAIT );
	//VMP_PRINTF("stream_handler %d\n",work->stream_handler);

	if( work->stream_handler < 0 ){
		VMP_PRINTF( "no handler\n", vox_num );
		return -1;
	}
	// 音声再生
	// いいタイミングでスタートしてください
	// 多重呼びしないように注意してください
	//if( GM_StreamStatus( work->stream_handler ) == GM_STREAM_STATE_END ) {
		GM_StreamStart( work->stream_handler );
		work->pre_vox = work->now_vox;
		work->now_vox = vox_num;
		work->vox_timer = work->vox_len[vox_num];
		VMP_PRINTF("stream start\n");
		return 0;
	//}
	//return -1;
}

static inline void VMP_AdjustPiku( Work *work )
{
	static short asjpiku_buff[]={ 32, 64, 128, 192, 256, 128} ;
	SVECTOR	rot ;
	OBJECT *body = &work->body;

	if( --work->piku_timer < 0 ){ work->piku_timer = 0; return; }
	rot.vx = -asjpiku_buff[ work->piku_timer ] ;
	rot.vy = 0 ;
	rot.vz = 0 ;
	GM_AdjustRotBody( body, &rot, 1  ) ;
	rot.vx = asjpiku_buff[ work->piku_timer ] ;
	GM_AdjustRotBody( body, &rot, 11  ) ;
	GM_AdjustRotBody( body, &rot, 3  ) ;
	GM_AdjustRotBody( body, &rot, 7  ) ;
}

static inline void VMP_KnfObjMesg( int name, int com, int data )
{
//mesg マルチウェイト髪の毛モデル $s:名前 visible[0] $i:フラグ	
	GV_MSG 	msg;
	int msg_data[2];
	msg.address = name;
	msg.message = msg_data;
	msg.message_len = 2;
	msg_data[0] = com;
	msg_data[1] = data;
	GV_SendMessage( &msg );

	
}
#endif
