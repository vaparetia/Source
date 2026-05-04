/*
   orga_wait.h
   オルガ 待ち行動処理関数群

   2000/01/13 T.Morita
   $Id: orga_dbug.h,v 1.1.1.3 2002/11/19 11:46:22 Yoshizawa1 Exp $
*/


static int ORG_DebugMoveAction( Work *work, void (***list)( Work * ) )
{
    extern int ORG_DbgAction ;

    if ( ORG_DbgAction )
    {
	switch( ORG_DbgAction )
	{
	case  1: *list = ORG_ActionRolloutRun ;	     break ;
	case  2: *list = ORG_ActionCrowRun ;	     break ;
	case  3: *list = ORG_ActionCrowRun ;	     break ;
	case  4: *list = ORG_ActionTumbleRun ;	     break ;
	case  5: *list = ORG_ActionTumbleRun ;	     break ;
	case  6: *list = ORG_ActionTumbleDash ;	     break ;
	case  7: *list = ORG_ActionTumbleDash ;	     break ;
	case  8: *list = ORG_ActionAimmingDashWithRage ; break ;
	case  9: *list = ORG_ActionAimmingDash ;	     break ;
	case 10: *list = ORG_ActionAimmingDashWithRage ; break ;
	case 11: *list = ORG_ActionAimmingDash ;	     break ;
	case 12: *list = ORG_ActionRunSliding ;	     break ;
	case 13: *list = ORG_ActionRunSliding ;	     break ;
	case 14: *list = ORG_ActionRolloutRun ;	     break ;
	case 15: *list = ORG_ActionRolloutRun ;	     break ;
	}
	return 0 ;
    }
    return 1 ;
}



static int ORG_DebugAttackAction( Work *work, void (***list)( Work * ) )
{
    extern int hid_id ;

    extern int ORG_DbgAction ;
    if ( ORG_DbgAction )
    {
	int j=0 ;
	work->weap_blt = work->weap_max ; /* blowback release */
	switch( ORG_DbgAction )
	{
	case  1: j=0x00; break ; /*ORG_ActionRolloutRun ;         */ 
	case  2: j=0x80; break ; /*ORG_ActionCrowRun ;            */ 
	case  3: j=0x01; break ; /*ORG_ActionCrowRun ;            */ 
	case  4: j=0x80; break ; /*ORG_ActionTumbleRun ;          */ 
	case  5: j=0x01; break ; /*ORG_ActionTumbleRun ;          */ 
	case  6: j=0x80; break ; /*ORG_ActionTumbleDash ;         */ 
	case  7: j=0x01; break ; /*ORG_ActionTumbleDash ;         */ 
	case  8: j=0x00; break ; /*ORG_ActionAimmingDashWithRage ;*/ 
	case  9: j=0x00; break ; /*ORG_ActionAimmingDash ;        */
	case 10: j=0x01; break ; /*ORG_ActionAimmingDashWithRage ;*/ 
	case 11: j=0x01; break ; /*ORG_ActionAimmingDash ;        */
	case 12: j=0x80; break ; /*ORG_ActionRunSliding ;        */
	case 13: j=0x01; break ; /*ORG_ActionRunSliding ;        */
	case 14: j=0x80; break ; /*ORG_ActionRolloutRun ;        */
	case 15: j=0x01; break ; /*ORG_ActionRolloutRun ;        */
	}
	work->hide_spot = &work->hide_pool[j&7] ;
	work->trgt_pos = j&0x80 ? &work->hide_spot->right : &work->hide_spot->left ;
	if ( work->hide_spot->height < ORGA_DANGER_HEIGHT )
	    work->flag |= ORGA_F_DANGR_STAND ;
	work->control.mov = *work->trgt_pos ;

	*list = ORG_ActionAbort, ChangeFlag( work, ORGA_F_NONE, ORGA_F_OFFENCE ) ;
	return 0 ;
    }

    if ( work->weap_blt <=0 && hid_id )
	ChangeFlag( work, ORGA_F_NONE, ORGA_F_ENB_AVOID| ORGA_F_ENB_AIM ),
	    *list = ORG_ActListReload[ (int)work->stll_mtn ] ;
    else
	switch ( hid_id )
	{
	case 4: case 5: case 6:
	    if ( work->weap_blt <=0 )
		*list = ORG_ActionStandReload ;
	    else 
		work->weap_blt = work->weap_max,
		    *list = ORG_ActionMachineGunFellow ;
	    ChangeFlag( work, ORGA_F_OFFENCE, ORGA_F_MODIST ) ;
	    return 0 ;

	case 7:
	    work->vitality = ORGA_MAX_VITALITY/2-1 ;
	    if ( work->trgt_pos != &ORG_ShootHoloPos )
		*list = ORG_ActionAbort, ChangeFlag( work, ORGA_F_SRCH_HIDE, ORGA_F_OFFENCE ) ;
	    else
		*list = ORG_ActionOpenHolo,
		    ChangeFlag( work, ORGA_F_NOMISS_SHOT, ORGA_F_ENB_AVOID|ORGA_F_OFFENCE ) ;
	    return 0 ;

	case 8:
	    if ( work->hide_spot->id == 0 )
		*list = ORG_ActionBreakSpotLight ;
	    return 0 ;
	}
    return 1 ;
}


static int ORG_DebugMaskID( void  )
{
    extern int hid_id ;
    int mask ;

    switch( hid_id )
    {
    case 1: case 4:  case 8:
	mask = 0x00000030 ; break ;
    case 2: case 5:
	mask = 0x0000000c ; break ;
    case 3: case 6:
	mask = 0x00000003 ; break ;
    case 9:
	mask = 0x0000003c ; break ;
    default:
	mask = 0x0000003f ; break ;
    }
    return mask ;
}
