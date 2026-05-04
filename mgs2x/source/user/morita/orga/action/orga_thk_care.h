/*
   orga_thk_care.h
   オルガ 思考関数 警戒モード

   2000/01/05 T.Morita
   $Id: orga_thk_care.h,v 1.1.1.3 2002/11/19 11:46:23 Yoshizawa1 Exp $
*/

static int ORG_ActThinkToBeCarefull( Work *work, void (***list)( Work * ) )
{
    if ( work->flag & ORGA_F_OFFENCE )
	ChangeFlag( work, ORGA_F_NONE, ORGA_F_OFFENCE| ORGA_F_MODIST ) ;
    else
	ChangeFlag( work, ORGA_F_ENB_SAFE| ORGA_F_OFFENCE, ORGA_F_MODIST ) ;/* 攻撃モードへ */

    if ( work->flag & ORGA_F_DANGR_STAND && !(work->stll_mtn && ORGA_F_IS_BEHIND) )
	*list++ = ORG_ActionHalfStandPeeping ;
    else if ( work->flag & ORGA_F_DANGR_LEFT )
	*list++ = ORG_ActListModistPeepingLeft[ work->flag & ORGA_F_AIMED_NEAR ?
					       work->stll_mtn+4 : work->stll_mtn ] ;
    else if ( work->flag & ORGA_F_DANGR_RIGHT )
	*list++ = ORG_ActListModistPeepingRight[ work->flag & ORGA_F_AIMED_NEAR ?
					       work->stll_mtn+4 : work->stll_mtn ] ;
    if ( work->procs[ORGA_P_PEEP] )
	GCL_ExecProc( work->procs[ORGA_P_PEEP], NULL ) ;/*プロック実行*/
    *list = NULL ;

    return 1 ;
}
