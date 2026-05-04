#if 1 	//JAPANESE_BP_IGNORE()から修正
#define	NOT_FINE_STATE	(ACT_STATUS_DEATH|ACT_STATUS_UNREAL)
#else
#define	NOT_FINE_STATE	(ACT_STATUS_FAINT|ACT_STATUS_DEATH|ACT_STATUS_UNREAL)
#endif

int ENE_CLBoundCheckFineEne( HZX_CLE_AREA *area )
{
	COMMANDER	*com ;
	E_GROUP		*gp ;
	E_UNIT		*un ;
	ENETHINK	*entk ;
	int			g, u, i, num ;
	FVECTOR		*pos ;

	com = &Comm ;
	num = 0 ;

	for ( g=0; g<com->enemys.group_num; g++ ) {
		gp = com->enemys.group[ g ] ;
		for ( u=0; u<gp->unit_num; u++ ) {
			un = gp->unit[ u ] ;
			for ( i=0; i<un->enemy_num; i++ ) {
				if ( (entk = un->entk[ i ]) == NULL ) continue ;
				if ( entk->status & ENE_STATUS_EVER_UNREAL ) continue ;
				if ( !(entk->act->status & NOT_FINE_STATE) ) {
					pos = &entk->ctrl->mov ;
					if ( pos->vx < area->b1.vx || pos->vx > area->b2.vx ) continue ;
					if ( pos->vy < area->b1.vy || pos->vy > area->b2.vy ) continue ;
					if ( pos->vz < area->b1.vz || pos->vz > area->b2.vz ) continue ;
					return 1 ;
				}
			}
		}
	}

	return 0 ;
}


int ENE_CLBoundCheck( FVECTOR *pos )
{
	HZX_CLE_AREA *area ;
	int i ;

	area = HZX_CurrentHzx->def->cle_areas ;
	i = COM_GetClearingOfset( ) ;
	for ( ; i<HZX_CurrentHzx->def->n_clears; i++,area++ ) {
		if ( Comm.cle_area_status[ i ] & CLE_AREA_ST_SKIP ) continue ;
		if ( !(Comm.cle_area_status[ i ] & CLE_AREA_ST_ENECHK_SKIP) ) {
			if ( ENE_CLBoundCheckFineEne( area ) ) continue ;
		}

		if ( pos->vx < area->b1.vx || pos->vx > area->b2.vx ) continue ;
		if ( pos->vy < area->b1.vy || pos->vy > area->b2.vy ) continue ;
		if ( pos->vz < area->b1.vz || pos->vz > area->b2.vz ) continue ;
		return i ;
	}
	return -1 ;
}


HZX_CLE_AREA *ENE_GetClearArea( int n )
{
	HZX_CLE_AREA	*area ;
	
	area = HZX_CurrentHzx->def->cle_areas ;
	area += n ;

	return area ;
}

HZX_CLE_ROOT *ENE_GetClearRoot( HZX_CLE_AREA *area, int n )
{
	HZX_CLE_ROOT	*root ;
	
	root = area->roots ;
	root += n ;

	return root ;
}

HZX_CLE_PTP *ENE_GetClearPoint( HZX_CLE_ROOT *root, int n )
{
	HZX_CLE_PTP	*point ;
	
	point = root->points ;
	point += n ;

	return point ;
}

HZX_CLE_PTP *ENE_GetClearPoint2( int area_n, int root_n, int point_n ) 
{
	HZX_CLE_AREA	*area ;
	HZX_CLE_ROOT	*root ;
	HZX_CLE_PTP		*point ;
	
	area = HZX_CurrentHzx->def->cle_areas ;
	area += area_n ;
	root = area->roots ;
	root += root_n ;
	point = root->points ;
	point += point_n ;
printf(" clear date [%d] [%d] [%d] points[%d]\n",area_n, root_n, point_n,root->n_points ) ;

	return point ;
}

#if 0
/* クリアリングデータ初期化 */
static	void	SetupClearings( HZX_CLE_AREA *area, int n, void *def ) 
{
    HZX_CLE_ROOT	*root ;
    int			m, i, j ;

	i = 0 ;
	j = 0 ;
    while( -- n >= 0 ) {
//		area->roots = ( HZX_CLE_ROOT * )( ( void * )def + ( u_int )area->roots ) ; 
		area->roots = &(hzx_cle_root[i]) ; 
		m = area->n_root ;
		root = area->roots ;
		while( -- m >= 0 ) {
//		    root->points = ( HZX_CLE_PTP * )( ( void * )def + ( u_int )root->points ) ;
		    root->points = &(hzx_cle_ptp[ j ]) ;
		    j += root->n_points ;
		    root ++ ;
		}
		area ++ ;
		i += m ;
    }
}
#endif

void	MakeClearing(  )
{

#if 0
	HZX_DEF		*def ;
	HZX_CLE_AREA *a ;
	HZX_CLE_ROOT *root ;
	HZX_CLE_PTP *ptp ;
	int	i,j,k ;

	def = HZX_CurrentHzx->def ;

	printf("CLEARING DATA\n");
	printf("ALEAR NUM[%d] \n",def->n_clears) ;

	a = def->cle_areas ;
	for ( i=0; i<def->n_clears; i++ ) {
		printf("ALEAR [%d] \n",i) ;
		printf("b1[%f][%f][%f] \n",a->b1.vx,a->b1.vy,a->b1.vz) ;
		printf("b2[%f][%f][%f] \n",a->b2.vx,a->b2.vy,a->b2.vz) ;
		root = a->roots ;
		printf("ROOT NUM[%d] \n",a->n_root) ;
		for ( j=0; j<a->n_root; j++ ) {
			printf("ROOT [%d] \n",j) ;
			printf("PTP NUM [%d] \n",root->n_points) ;
			ptp = root->points ;
			for ( k=0; k<root->n_points; k++ ) {
				printf("POINT [%d] \n",k) ;
				printf(" act[%d] time[%d] dir[%d] con[%d]\n",ptp->act,ptp->time,ptp->dir,ptp->pad ) ;
				
				ptp++ ;
			}
			root ++ ;
		}
		a++ ;
	}
	printf("CLEARING DATASET END  \n") ;
#endif
}

static int GetClearingProc( cle_proc )
CLEARING_PROC	*cle_proc ;
{
	int	num ;

	num = 0 ;
	while ( GCL_NextStr() != NULL ){
		cle_proc->area = GCL_GetNextInt( ) + COM_GetClearingOfset( ) ;
		cle_proc->route = GCL_GetNextInt( ) ;
		cle_proc->point = GCL_GetNextInt( ) ;
		cle_proc->proc = GCL_GetNextInt( ) ;
		
		cle_proc++ ;
		num++ ;
	}

	return num ;
	
}
