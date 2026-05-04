static void NewMakeCorpDynamicFloor( work )
Work	*work ;
{
	FVECTOR	*center ;
	IVECTOR	*pos ;

	center = &work->control.mov ;
	pos = &work->pos[0] ;

	pos->vx = (int)center->vx - 300 ;
	pos->vy = (int)center->vy + 20 ;
	pos->vz = (int)center->vz - 300 ;
	pos++ ;

	pos->vx = (int)center->vx + 300 ;
	pos->vy = (int)center->vy + 20 ;
	pos->vz = (int)center->vz - 300 ;
	pos++ ;

	pos->vx = (int)center->vx + 300 ;
	pos->vy = (int)center->vy + 20 ;
	pos->vz = (int)center->vz + 300 ;
	pos++ ;

	pos->vx = (int)center->vx - 300 ;
	pos->vy = (int)center->vy + 20 ;
	pos->vz = (int)center->vz + 300 ;

	pos = &work->pos[0] ;
	work->d_floor = HZX_AddDynamicFloor( work->control.hzx_id,
							pos, pos+1, pos+2, pos+3,
						     4, DYNAMIC_FLOOR_FLAG ) ;
}
