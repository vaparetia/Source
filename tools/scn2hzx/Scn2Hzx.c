/*
   Scn2Hzx.c
   Scn(VRS) -> Hzx(MGS2) コンバータ
   
   1999/12/16 M.Sonoyama
   
   $Id: Scn2Hzx.c,v 1.44 2002/05/10 07:15:29 usr03635 Exp $
   */

#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<string.h>
#include	<sys/types.h> 
#include	<math.h>

#ifdef _WINDOWS
#include <windows.h>
#include <SFDWIN.h>
#endif
#ifdef _UNIX
#include <SFDUNIX.h>
#endif
#ifdef _LINUX
#include <SFDLINUX.h>
#endif

#include	"Scene2HZX.h"
#include	"HZXStruct2.h"
#include	"fmt_hzx.h"

/*---------------------------------------------------------*/

static	int	MemoryBlock[ 65535 ] ;
static	int	N_MemBlocks = 0 ;
static	int	N_Allocs = 0 ;
static	int	N_Frees = 0 ;
static	int	N_FreeFalses = 0 ;

static	int	DebugFlag = 0 ;

static	int	N_BadZones = 0 ;

int	EasyStrip = 0 ;

static	void	*MyAlloc( size )
int		size ;
{
    void	*buf ;

    buf = malloc( size ) ;
    if ( buf == NULL ) {
		printf( "cannot alloc memory\n" ) ;
		return NULL ;
    }
    MemoryBlock[ N_MemBlocks ] = ( u_int )buf ;
    N_MemBlocks ++ ;
    N_Allocs ++ ;
    return buf ;
}

static	void	MyFree( addr )
void		*addr ;
{
    int		loc, i, c ;

    if ( N_MemBlocks == 0 ) return ;

    loc = ( u_int )addr ;
    c = 0 ;
    for ( i = 0; i < N_MemBlocks; i ++ ) {
		if ( c == 0 && MemoryBlock[ i ] == loc ) {
			free( addr ) ;
			c = 1 ;
			continue ;
		} else if ( c ) {
			MemoryBlock[ i - 1 ] = MemoryBlock[ i ] ;
		}
    }
    if ( c ) {
		N_MemBlocks -- ;
    } else {
		N_FreeFalses ++ ;
    }
    N_Frees ++ ;
}

void		FreeHzxDef( def )
HZD_DEF		*def ;
{
    HZD_GRP	*grp ;
    HZD_BLOCK	*blk ;
    HZD_VuSEG	*seg ;
    int		i, j, k ;

    MyFree( def->patrols ) ;
    MyFree( def->points ) ;
    MyFree( def->cle_areas ) ;
    MyFree( def->cle_roots ) ;
    MyFree( def->cle_points ) ;

    grp = def->groups ;
    for ( i = 0; i < def->n_groups; i ++ ) {
		MyFree( grp->behinds ) ;
		MyFree( grp->zones ) ;
		MyFree( grp->link_zone ) ;
		blk = grp->blocks ;
		for ( j = 0; j < grp->n_blocks; j ++ ) {
			MyFree( blk->traps ) ;
			seg = blk->segs ;
			for ( k = 0; k < blk->n_segs; k ++ ) {
				MyFree( seg->verts ) ;
				seg ++ ;
			}
			if ( blk->n_segs >0 ) MyFree( blk->segs ) ;
			seg = blk->flrs ;
			for ( k = 0; k < blk->n_flrs; k ++ ) {
				MyFree( seg->verts ) ;
				seg ++ ;
			}
			if ( blk->n_flrs > 0 ) MyFree( blk->flrs ) ;
			seg = blk->bul_segs ;
			for ( k = 0; k < blk->n_bul_segs; k ++ ) {
				MyFree( seg->verts ) ;
				seg ++ ;
			}
			if ( blk->n_bul_segs > 0 ) MyFree( blk->bul_segs ) ;
			seg = blk->bul_flrs ;
			for ( k = 0; k < blk->n_bul_flrs; k ++ ) {
				MyFree( seg->verts ) ;
				seg ++ ;
			}
			if ( blk->n_bul_flrs > 0 ) MyFree( blk->bul_flrs ) ;
			blk ++ ;
		}
		MyFree( grp->blocks ) ;
		grp ++ ;
    }
    MyFree( def->groups ) ;
    MyFree( def ) ;
}

/*---------------------------------------------------------*/

static	int	BoundMinX, BoundMinY, BoundMinZ ;
static	int	SizeX, SizeY, SizeZ ;
static	int	DivX, DivY, DivZ ;

static	int	MaxX, MaxY, MaxZ, MinX, MinY, MinZ ;

static	int	Scissor = 0 ;

typedef	struct	{
    int		atr ;
    int		n_points ;
    IVECTOR	*verts ;
} SEGVerts ;

typedef	struct	{
    int		atr ;
    SVECTOR	v[ 3 ] ;
} Triangle ;

typedef	struct	{
    int		atr ;
    SVECTOR	v[ 2 ] ;
    u_char	f1, f2 ;
} Line ;

typedef	struct	{
    int		atr ;
    int		n_points ;
    SVECTOR	v[ 4 ] ;
} Square ;

static	int		N_Triangles ;
static	Triangle	*Triangles ;

static	int		N_Lines ;
static	Line		*Lines ;

static	int		N_Squares ;
static	Square		*Squares ;

/*------------------------------------------------------------------*/

#define BIT_LEN		24
int 	GetStrCode( char *string )
{
    unsigned char c ;
    unsigned char *p ;
    unsigned int id, mask ;

    p = string ;
    id = 0 ;
    mask = ( 1 << BIT_LEN ) - 1 ;
    while( ( c = *( p++ ) ) != '\0' ) {
		id = ( id << 5 ) | ( id >> ( BIT_LEN - 5 ) ) ;
		id += c ;
		id &= mask ;
    }
    if( id == 0 ) id = 1 ;		/* 0 になってしまったら１にする */
    return id;
}

static	int	GetStrCodeTrap( char *string )
{
    unsigned char c ;
    unsigned char *p ;
    unsigned int id, mask ;
    int	     number ;

    p = string ;
    id = 0 ; number = 0 ;
    mask = ( 1 << BIT_LEN ) - 1 ;
    while( ( c = *( p++ ) ) != '\0' ) {
		if ( c == ':' ) {
			sscanf( p, "%x", &number ) ;
			if ( number <= 0 || number > 0xff ) {
				printf( "wrong number %s\n", string ) ;
				number = 0 ;
			}
			break ;
		}
		id = ( id << 5 ) | ( id >> ( BIT_LEN - 5 ) ) ;
		id += c ;
		id &= mask ;
    }
    if( id == 0 ) id = 1 ;		/* 0 になってしまったら１にする */
    id = id | ( ( number & 255 ) << 24 ) ;
    if ( number != 0 ) {
		printf( "%s -> id has number %x\n",  string, id ) ;
    }
    return id;
}

/* ダンプ */
void		DumpDVec( v )
void		*v ;
{
    double	*d ;

    d = ( double * )v ;
    printf( "%.6lf %.6lf %.6lf\n", d[ 0 ], d[ 1 ], d[ 2 ] ) ;
}

static	void	DumpVec( v )
FVECTOR		*v ;
{
    printf( "%.0f %.0f %.0f %.0f\n", v->vx, v->vy, v->vz, v->vw ) ;
}

static	void	DumpSVector( sv )
SVECTOR		*sv ;
{
    printf( "%d %d %d %d\n", sv->vx, sv->vy, sv->vz, sv->vw ) ;
}

static	void	DumpIVector( sv )
IVECTOR		*sv ;
{
    printf( "%d %d %d %d\n", sv->vx, sv->vy, sv->vz, sv->vw ) ;
}

static	void	DumpTriangle( tri )
Triangle	*tri ;
{
    printf( "%d %d %d : ", tri->v[ 0 ].vx, tri->v[ 0 ].vy, tri->v[ 0 ].vz ) ;
    printf( "%d %d %d : ", tri->v[ 1 ].vx, tri->v[ 1 ].vy, tri->v[ 1 ].vz ) ;
    printf( "%d %d %d\n", tri->v[ 2 ].vx, tri->v[ 2 ].vy, tri->v[ 2 ].vz ) ;
}

static	void	DumpSquare( sq )
Square		*sq ;
{
    DumpSVector( &sq->v[ 0 ] ) ;
    DumpSVector( &sq->v[ 1 ] ) ;
    DumpSVector( &sq->v[ 2 ] ) ;
    DumpSVector( &sq->v[ 3 ] ) ;
}

static	void	DumpLine( ln )
Line		*ln ;
{
    printf( "line %d %d %d %d - ", ln->v[ 0 ].vx, ln->v[ 0 ].vy, ln->v[ 0 ].vz, ln->v[ 0 ].vw ) ;
    printf( "%d %d %d %d\n", ln->v[ 1 ].vx, ln->v[ 1 ].vy, ln->v[ 1 ].vz, ln->v[ 1 ].vw ) ;
}

static	void	DumpVuSEGS( segs, n_segs )
HZD_VuSEG	*segs ;
int		n_segs ;
{
    int		i ;
    SVECTOR	*v ;

    if ( segs == NULL ) { 
		printf( "nul\n" ) ; 
		return ; 
    }
    printf( "%d strips\n", n_segs ) ;
    while( -- n_segs >= 0 ) {
		printf( "n_point %d\n", segs->b1.vw ) ;
		printf( "atr %x\n", segs->atr ) ;
		printf( "%d %d %d - %d %d %d : %d\n",
			   segs->b1.vx, segs->b1.vy, segs->b1.vz,
			   segs->b2.vx, segs->b2.vy, segs->b2.vz, segs->b2.vw ) ;
		v = segs->verts ;
		for ( i = 0; i < segs->b1.vw; i ++, v ++ ) {
			DumpSVector( v ) ;
		}
		segs ++ ;
    }
}

static	void	DumpVuSEGS2( b, segs, n_segs )
HZD_BLOCK	*b ;
HZD_VuSEG	*segs ;
int		n_segs ;
{
    int		i ;
    SVECTOR	*v ;

    if ( segs == NULL ) { 
		printf( "nul\n" ) ; 
		return ; 
    }
  printf( "%d strips\n", n_segs ) ;
    while( -- n_segs >= 0 ) {
		printf( "n_point %d\n", segs->b1.vw ) ;
		printf( "atr %x\n", segs->atr ) ;
		printf( "%d %d %d - %d %d %d : %d\n",
			   b->tx + segs->b1.vx, b->ty + segs->b1.vy, b->tz + segs->b1.vz,
			   b->tx + segs->b2.vx, b->ty + segs->b2.vy, b->tz + segs->b2.vz, segs->b2.vw ) ;
		v = segs->verts ;
		for ( i = 0; i < segs->b1.vw; i ++, v ++ ) {
			printf( "%d %d %d %d\n", v->vx + b->tx, v->vy + b->ty, v->vz + b->tz, v->vw ) ;
		}
		segs ++ ;
    }
}

static	void	DumpSEGVerts( sgv )
SEGVerts	*sgv ;
{
    int		i ;
    IVECTOR	*v ;

    printf( "n_point %d\n", sgv->n_points ) ;
    printf( "atr %x\n", sgv->atr ) ;
    v = sgv->verts ;
    for ( i = 0; i < sgv->n_points; i ++, v ++ ) {
		printf( "%d %d %d\n", v->vx, v->vy, v->vz ) ;
    }
}

static	void	DumpSEGVerts2( sgv, bo )
SEGVerts	*sgv ;
BLOCKOBJECT	*bo ;
{
    int		i ;
    IVECTOR	*v ;

    printf( "n_point %d\n", sgv->n_points ) ;
    printf( "atr %x\n", sgv->atr ) ;
    v = sgv->verts ;
    for ( i = 0; i < sgv->n_points; i ++, v ++ ) {
		printf( "%d %d %d\n", v->vx + bo->nxyzCenter.x, 
			   v->vy + bo->nxyzCenter.y, 
			   v->vz + bo->nxyzCenter.z ) ;
    }
}

static	void	DumpHzxTraps( trps, n )
HZD_TRP		*trps ;
int		n ;
{
    printf( "%d traps\n", n ) ;
    while( -- n >= 0 ) {
		printf( "%d %d %d - %d %d %d\n", 
			   trps->b1.vx, trps->b1.vy, trps->b1.vz,
			   trps->b2.vx, trps->b2.vy, trps->b2.vz ) ;
		printf( "name_id %x\n", trps->name_id ) ;
		trps ++ ;
    }
}

static	void	DumpHzxHeader( def )
HZD_DEF		*def ;
{
	int i;
    printf( "version  \t %d\n", def->version ) ;
    printf( "n_groups \t %d\n", def->n_groups ) ;
	//    printf( "n_zones  \t %d\n", def->n_zones ) ;
    printf( "n_patrols\t %d\n", def->n_patrols ) ;
    for(i = 0; i < HZX_VR_CHARACTER_GROUP_MAX; i++)
    {
		printf("vr_pat_offset[%d] = %d\n", i, def->vr_pat_offset[i]);
	}

    printf( "n_clears \t %d\n", def->n_clears ) ;
    for(i = 0; i < HZX_VR_CLEARING_GROUP_MAX; i++)
    {
		printf("vr_clr_offset[%d] = %d\n", i, def->vr_clr_offset[i]);
	}
}

static	void	DumpHzxBlock( blk )
HZD_BLOCK	*blk ;
{
	printf( "block_no[ %d ]\n", blk->block_no ) ;
  printf( "center %d %d %d\n", blk->tx, blk->ty, blk->tz ) ;
  printf( "%d segs(strip) : %d floors(strip) : %d traps\n",
		   blk->n_segs, blk->n_flrs, blk->n_traps ) ;
	printf( "%d bsegs(strip) : %d bfloors(strip)\n",
		   blk->n_bul_segs, blk->n_bul_flrs ) ;
	DumpVuSEGS2( blk, blk->segs, blk->n_segs ) ;
	DumpVuSEGS2( blk, blk->flrs, blk->n_flrs ) ;
  DumpVuSEGS2( blk, blk->bul_segs, blk->n_bul_segs ) ;
    DumpVuSEGS2( blk, blk->bul_flrs, blk->n_bul_flrs ) ;
  DumpHzxTraps( blk->traps, blk->n_traps ) ;
}

static	void	DumpBehind( bhd, n )
HZD_BEHIND	*bhd ;
int		n ;
{
	while( -- n >= 0 ) {
	    printf( "behind[ %d ]\n", n ) ;
	    DumpVec( &( bhd->b1 ) ) ;
	    DumpVec( &( bhd->b2 ) ) ;
	    printf( "%.0f %.0f %.0f %.0f %.0f %.0f\n", bhd->v[ 0 ], bhd->v[ 1 ], bhd->v[ 2 ],
			   bhd->v[ 3 ], bhd->v[ 4 ], bhd->v[ 5 ] ) ;
	    printf( "%.0f %.0f %.0f %.0f %.0f %.0f\n", bhd->right[ 0 ], bhd->right[ 1 ], 
			   bhd->right[ 2 ], bhd->right[ 3 ], bhd->right[ 4 ], bhd->right[ 5 ] ) ;
	    printf( "%.0f %.0f %.0f %.0f %.0f %.0f\n", bhd->left[ 0 ], bhd->left[ 1 ], 
			   bhd->left[ 2 ], bhd->left[ 3 ], bhd->left[ 4 ], bhd->left[ 5 ] ) ;	    
	    bhd ++ ;
	}
}

void		DumpZone( zon, n )
HZD_ZON		*zon ;
int		n ;
{
    int		i, chk ;

	chk = 0 ;
#if 1
    printf( "%d zones\n", n ) ;
#endif
    for ( i = 0; i < n; i ++ ) {
		if ( zon->nears[ 0 ] == 255 ) chk ++ ;
#if 1
		printf( "[ %d ]", i ) ;
		printf( "%f %f %f : %f %f\n", zon->x, zon->z, zon->y, zon->w, zon->h ) ;
		printf( "near %d %d %d %d %d %d\n", zon->nears[ 0 ], zon->nears[ 1 ],
			   zon->nears[ 2 ], zon->nears[ 3 ], zon->nears[ 4 ], zon->nears[ 5 ] ) ;
		printf( "dist %d %d %d %d %d %d\n", zon->dists[ 0 ], zon->dists[ 1 ],
			   zon->dists[ 2 ], zon->dists[ 3 ], zon->dists[ 4 ], zon->dists[ 5 ] ) ;
#ifndef OLD
		printf( "safes %d %d %d %d\n", zon->safes[ 0 ], zon->safes[ 1 ], 
			   zon->safes[ 2 ], zon->safes[ 3 ] ) ;
		printf( "safe_dists %d %d %d %d\n", zon->safe_dists[ 0 ], zon->safe_dists[ 1 ], 
			   zon->safe_dists[ 2 ], zon->safe_dists[ 3 ] ) ;
		printf( "safe_types %x %x %x %x\n", zon->safe_types[ 0 ], zon->safe_types[ 1 ], 
			   zon->safe_types[ 2 ], zon->safe_types[ 3 ] ) ;
#endif
#endif
		zon ++ ;
    }
	if ( n > 0 && chk == n ) {
		fprintf( stdout, "no route\n" ) ;
	}
}

#ifndef OLD
void		DumpRoute( route, n_zones )
u_char		*route ;
int		n_zones ;
{
    int		size, i ;

    if ( n_zones < 2 ) return ;
    size = ( ( n_zones - 1 ) * ( n_zones - 2 ) ) / 2 
		+ ( n_zones - 1 ) ;
    for ( i = 0; i < size; i ++ ) {
		if ( i % 100 == 0 ) printf( "%d:", route[ i ] ) ;
    }
    printf( "\n" ) ;
}

void		DumpOnlineInfo( info, n_zones )
u_int		*info ;
int		n_zones ;
{
    u_int	*buf ;
    int		bitsize, size ;
    int		n, c, x, z, b ;
    int		i, j ;

    bitsize = ( n_zones * n_zones - n_zones ) / 2 ; /* 総ビット数 */
    size = ( bitsize + ( sizeof( int ) * 8 - 1 ) ) / 
		( sizeof( int ) * 8 ) ; /* 総Ｉｎｔ数 */
    buf = info ; 
    z = n_zones - 1 ;
    n = 0 ; b = 0 ;
    for ( i = 0; i < size; i ++ ) {
		x = *( buf + i ) ;
		for ( j = 31; j >= 0; j -- ) {
			c = ( x & ( 1 << j ) ) ? 1 : 0 ;
			printf( "%d", c ) ;
			if ( ++ n == z ) {
				printf( "\n" ) ;
				n = 0 ; z -- ;
			}
			if ( ++ b == bitsize ) break ;
		}
    }
}
#endif

void		DumpLinkZone( link, n )
u_char		*link ;
int		n ;
{
    printf( "%d link zones\n", n ) ;
    while( -- n >= 0 ) {
		printf( "( grp, zone ) = ( %d, %d )\n", link[ 0 ], link[ 1 ] ) ;
		link += 2 ;
    }
}

void		DumpPatrol( pat, n )
HZD_PAT		*pat ;
int		n ;
{
    int		m ;
    HZD_PTP	*ptp ;

    while( -- n >= 0 ) {
		printf( "n_points %d\n", pat->n_points ) ;
		m = pat->n_points ;
		ptp = pat->points ;
		while( -- m >= 0 ) {
			printf( "\t %.0f %.0f %.0f\n", ptp->x, ptp->z, ptp->y ) ;
			printf( "\t act %d : time %d : dir %d\n", ptp->act, ptp->time, ptp->dir ) ;
			printf( "\t group_id %d\n", ptp->group_id ) ;
			ptp ++ ;
		}
		pat ++ ;
    }
}

static	void	DumpClearing( def )
HZD_DEF		*def ;
{
    HZD_CLE_AREA	*area ;
    HZD_CLE_ROOT	*root ;
    HZD_CLE_PTP		*ptp ;
    int			n_areas, n_points, n_root ;

    area = def->cle_areas ;
    n_areas = def->n_clears ;

    printf( "%d clearing areas\n", n_areas ) ;
    while( -- n_areas >= 0 ) {
		printf( "%.0f %.0f %.0f - %.0f %.0f %.0f\n", 
			   area->b1.vx, area->b1.vy, area->b1.vz,
			   area->b2.vx, area->b2.vy, area->b2.vz ) ;
		printf( "name = %d\n", area->name ) ;
		printf( "go_time = %d\n", area->go_time ) ;
		printf( "n_root = %d\n", area->n_root ) ;
		n_root = area->n_root ;
		root = area->roots ;
		while( -- n_root >= 0 ) {
			n_points = root->n_points ;
			printf( "\tn_points = %d\n", n_points ) ;
			ptp = root->points ;
			while( -- n_points >= 0 ) {
				printf( "\t\t %.0f %.0f %.0f\n", ptp->x, ptp->y, ptp->z ) ;
				printf( "\t\t %.0f %.0f %.0f\n", ptp->ax, ptp->ay, ptp->az ) ;
				printf( "\t\t act = %d\n", ptp->act ) ;
				printf( "\t\t time = %d\n", ptp->time ) ;
				printf( "\t\t dir = %d\n", ptp->dir ) ;
				printf( "\t\t group_id %d\n", ptp->group_id ) ;
				ptp ++ ;
			}
			root ++ ;
		}
		area ++ ;
    }
}

static	void	DumpHzxGroup( def )
HZD_DEF			*def ;
{
    int		i, j ;
    HZD_GRP	*grp ;
    HZD_BLOCK	*blk ;

    grp = def->groups ;
#if 1
    printf( "%d groups\n", def->n_groups ) ;
#endif
    for ( i = 0; i < def->n_groups; i ++, grp ++ ) {
		fprintf( stdout, "---- < %d group > ----\n", i ) ;
#if 1
		printf( "bound_min %d %d %d\n", grp->bound_min_x, grp->bound_min_y, grp->bound_min_z ) ;
		printf( "block_size %d %d %d\n", grp->block_size_x, grp->block_size_y, grp->block_size_z ) ;
		printf( "div %d %d %d\n", grp->div_x, grp->div_y, grp->div_z ) ;
		printf( "%d blocks\n", grp->n_blocks ) ;
		DumpBehind( grp->behinds, grp->n_behinds ) ;
#ifndef OLD
		DumpOnlineInfo( grp->online_info, grp->n_zones ) ;
#endif
		DumpLinkZone( grp->link_zone, grp->n_link_zones ) ;
#endif
		DumpZone( grp->zones, grp->n_zones ) ;
#if 1
		DumpRoute( grp->route, grp->n_zones ) ;
		blk = grp->blocks ;
		for ( j = 0; j < grp->n_blocks; j ++, blk ++ ) {
			printf( "block[ %d ]\n", j ) ;
			DumpHzxBlock( blk ) ;
		}
#endif
    }
}

void		DumpHzx( def )
HZD_DEF		*def ;
{
#if 1
  DumpHzxHeader( def ) ;
  DumpPatrol( def->patrols, def->n_patrols ) ;
  DumpClearing( def ) ;
#endif
  DumpHzxGroup( def ) ;
}

/*-------------------------------------------------------------------------*/

static	void	DXYZ2IV( IVECTOR *v, DOUBLEXYZ *dxyz ) 
{
	double		d, adj = 0.0 ;

	d = floor( dxyz->x ) ;
	if ( dxyz->x - d > 0.8 ) {
		adj = 0.2 ;	//printf( "xxx\n" ) ;
	}
	v->vx = ( int )( floor( dxyz->x + adj ) ) ;
	d = floor( dxyz->y ) ;
	if ( dxyz->y - d > 0.8 ) {
		adj = 0.2 ; //printf( "yyy\n" ) ;
	}
	v->vy = ( int )( floor( dxyz->y + adj ) ) ;
	d = floor( dxyz->z ) ;
	if ( dxyz->z - d > 0.8 ) {
		adj = 0.2 ; //printf( "zzz\n" ) ;
	}
	v->vz = ( int )( floor( dxyz->z + adj ) ) ;
}

/*-------------------------------------------------------------------------*/

/* グループの変換 */

/*-------------------------------------------------------------------------*/

/* 頂点の追加 */
static	void		AddVerts( seg, v, kick )
HZD_VuSEG		*seg ;
SVECTOR			*v ;
int			kick ;
{
    SVECTOR		*verts ;

    verts = ( SVECTOR * )MyAlloc( sizeof( SVECTOR ) * ( seg->b1.vw + 1 ) ) ;
    if ( seg->verts != NULL ) {
		memcpy( verts, seg->verts, sizeof( SVECTOR ) * seg->b1.vw ) ;
		MyFree( seg->verts ) ;
    }
    *( verts + seg->b1.vw ) = *v ;
    ( verts + seg->b1.vw )->vw = kick ;
    seg->verts = verts ;
    seg->b1.vw ++ ;
}

/* ライン分割 */
static	void	MakeLine( sgv )
SEGVerts	*sgv ;
{
    Line	*lines, *ll ;
    IVECTOR	*iv ;
    int		i, j, n_classes, c, h ;
    int		*points, *vnum ;
    IVECTOR	*class, *v1, *v2, tmp ;
    float	t ;

    iv = sgv->verts ;
    n_classes = 0 ;
    class = ( IVECTOR * )MyAlloc( sizeof( IVECTOR ) * sgv->n_points ) ;
    points = ( int * )MyAlloc( sizeof( int ) * sgv->n_points ) ;
    vnum = ( int * )MyAlloc( sizeof( int ) * sgv->n_points ) ;
    /* ＸＺが同じものを組にする */
    for ( i = 0; i < sgv->n_points; i ++, iv ++ ) {
		c = 0 ;
		for ( j = 0; j < n_classes; j ++ ) {
			if ( iv->vx == class[ j ].vx &&
				iv->vz == class[ j ].vz ) {
				points[ j ] ++ ;
				c = 1 ;
				if ( iv->vy > class[ j ].vy ) {
					class[ j ].vw = iv->vy - class[ j ].vy ;
				} else {
					class[ j ].vw = class[ j ].vy - iv->vy ;
					class[ j ].vy = iv->vy ;
				}
				break ;
			}
		}
		if ( c == 0 ) {
			class[ n_classes ] = *iv ;
			points[ n_classes ] = 1 ;
			vnum[ n_classes ] = i ;
			n_classes ++ ;
		}
    }
    for ( i = 0; i < n_classes; i ++ ) {
		if ( points[ i ] == 1 ) {
			/* 自分を含まない直線で、ＸＺが同じ点のある
			   ものを探す */
			c = 0 ;
			iv = class + i ;
			for ( j = 0; j < sgv->n_points; j ++ ) {
				if ( j == vnum[ i ] || 
					( ( j + 1 ) % sgv->n_points ) == vnum[ i ] ) continue ; 
				v1 = sgv->verts + j ;
				v2 = sgv->verts + ( ( j + 1 ) % sgv->n_points ) ;
				if ( ( v1->vx <= iv->vx && v2->vx >= iv->vx &&
					  v1->vz <= iv->vz && v2->vz >= iv->vz ) ||
					( v2->vx <= iv->vx && v1->vx >= iv->vx &&
					 v2->vz <= iv->vz && v1->vz >= iv->vz ) ) {
					c = 1 ;
					/* その直線上でのＸＺ点での高さを求める */
					if ( v1->vx == v2->vx ) {
						/* Ｚで求める */
						t = ( float )( iv->vz - v1->vz ) / ( float )( v2->vz - v1->vz ) ;
					} else {
						/* Ｘで求める */
						t = ( float )( iv->vx - v1->vx ) / ( float )( v2->vx - v1->vx ) ;
					}
					h = ( int )( ( float )v1->vy + t * ( float )( v2->vy - v1->vy ) ) ;
					if ( h > iv->vy ) {
						iv->vw = h - iv->vy ;
					} else {
						iv->vw = iv->vy - h ;
						iv->vy = h ;
					}
				}
			}
			if ( c == 0 ) {
				/* 高さ０ */
				iv->vw = 0 ;
			}
		}
    }
    /* 座標順にならべる */
    for ( i = 0; i < n_classes - 1; i ++ ) {
		for ( j = 0; j < n_classes - 1; j ++ ) {	
			if ( class[ j ].vx == class[ j + 1 ].vx ) {
				if ( class[ j ].vz > class[ j + 1 ].vz ) {
					tmp = class[ j ] ;
					class[ j ] = class[ j + 1 ] ;
					class[ j + 1 ] = tmp ;
				}
			} else {
				if ( class[ j ].vx > class[ j + 1 ].vx ) {
					tmp = class[ j ] ;
					class[ j ] = class[ j + 1 ] ;
					class[ j + 1 ] = tmp ;
				}		
			}
		}
    }
    /* ライン登録 */
    lines = Lines + N_Lines ;
    iv = class ;
    for ( i = 0; i < n_classes - 1; i ++ ) {
		lines->atr = sgv->atr ;
		lines->v[ 0 ].vx = iv->vx ;
		lines->v[ 0 ].vy = iv->vy ;
		lines->v[ 0 ].vz = iv->vz ;
		lines->v[ 0 ].vw = iv->vw ;
		lines->v[ 1 ].vx = ( iv + 1 )->vx ;
		lines->v[ 1 ].vy = ( iv + 1 )->vy ;
		lines->v[ 1 ].vz = ( iv + 1 )->vz ;
		lines->v[ 1 ].vw = ( iv + 1 )->vw ;
		/* 使用済みフラグ */
		lines->f1 = lines->f2 = 0 ;

		ll = Lines ;
		for ( j = 0; j < N_Lines; j ++, ll ++ ) {
			if ( ll->v[ 0 ].vx == lines->v[ 0 ].vx &&	
				ll->v[ 0 ].vy == lines->v[ 0 ].vy &&
				ll->v[ 0 ].vz == lines->v[ 0 ].vz &&
				ll->v[ 0 ].vw == lines->v[ 0 ].vw &&
				ll->v[ 1 ].vx == lines->v[ 1 ].vx &&	
				ll->v[ 1 ].vy == lines->v[ 1 ].vy &&
				ll->v[ 1 ].vz == lines->v[ 1 ].vz &&
				ll->v[ 1 ].vw == lines->v[ 1 ].vw &&
				ll->atr == lines->atr ) {
				break ;
			}
		}
		if ( j != N_Lines ) continue ;
		lines ++ ;
		N_Lines ++ ;
    }
    MyFree( class ) ;
    MyFree( points ) ;
    MyFree( vnum ) ;
}

/* ストリップ作成 */
static	void		ChainLineStrip( seg, n_tris, tris, atr, i, j )
HZD_VuSEG		*seg ;
int			n_tris ;
Line			*tris ;
int			atr, i, j ;
{	
    Line	*tri ;
    SVECTOR	*v ;
    int		k, n, c ;

    seg->atr = atr ;
    seg->b1.vw = 0 ;
    seg->verts = NULL ;
    tri = tris + i ;
    tri->f2 = 1 ;
    AddVerts( seg, &tri->v[ j ], tri->v[ j ].vw ) ;
    AddVerts( seg, &tri->v[ ( j + 1 ) % 2 ], tri->v[ ( j + 1 ) % 2 ].vw ) ;
chainlinestriprepeat :
    c = 0 ;
    tri = tris ;
    for ( k = 0; k < n_tris; k ++, tri ++ ) {
		if ( tri->atr != atr || tri->f1 == 1 ||
			tri->f2 == 1 ) continue ;
		v = seg->verts + seg->b1.vw - 1 ;
		n = -1 ;
		if ( tri->v[ 0 ].vx == v->vx && tri->v[ 0 ].vy == v->vy &&
			tri->v[ 0 ].vz == v->vz && tri->v[ 0 ].vw == v->vw ) {
			n = 1 ;
		} else if ( tri->v[ 1 ].vx == v->vx && tri->v[ 1 ].vy == v->vy &&
				   tri->v[ 1 ].vz == v->vz && tri->v[ 1 ].vw == v->vw ) {
			n = 0 ;
		} 
		if ( n < 0 ) continue ;
		AddVerts( seg, &tri->v[ n ], tri->v[ n ].vw ) ;
		tri->f2 = 1 ;
		c = 1 ;
    }
    if ( c ) goto chainlinestriprepeat ;
}

/* １ストリップのバウンディングボックスを設定 */
static	void	MakeLineStripBoundingBox( seg )
HZD_VuSEG	*seg ;
{
    int		i ;
    SVECTOR	*v ;

    seg->b1.vx = seg->b1.vy = seg->b1.vz = 32000 ;
    seg->b2.vx = seg->b2.vy = seg->b2.vz = -32000 ;
    v = seg->verts ;
    for ( i = 0; i < seg->b1.vw; i ++ ) {
		if ( v->vx < seg->b1.vx ) seg->b1.vx = v->vx ;
		if ( v->vy < seg->b1.vy ) seg->b1.vy = v->vy ;
		if ( v->vz < seg->b1.vz ) seg->b1.vz = v->vz ;
		if ( v->vx > seg->b2.vx ) seg->b2.vx = v->vx ;
		if ( v->vy + v->vw > seg->b2.vy ) seg->b2.vy = v->vy + v->vw ;
		if ( v->vz > seg->b2.vz ) seg->b2.vz = v->vz ;
		v ++ ;
    }
}

/* バウンディング生成（床） */
static	void	MakeStripBoundingBox( seg )
HZD_VuSEG	*seg ;
{
    int		i ;
    SVECTOR	*v ;

    seg->b1.vx = seg->b1.vy = seg->b1.vz = 32000 ;
    seg->b2.vx = seg->b2.vy = seg->b2.vz = -32000 ;
    v = seg->verts ;
    for ( i = 0; i < seg->b1.vw; i ++ ) {
		if ( v->vx < seg->b1.vx ) seg->b1.vx = v->vx ;
		if ( v->vy < seg->b1.vy ) seg->b1.vy = v->vy ;
		if ( v->vz < seg->b1.vz ) seg->b1.vz = v->vz ;
		if ( v->vx > seg->b2.vx ) seg->b2.vx = v->vx ;
		if ( v->vy > seg->b2.vy ) seg->b2.vy = v->vy ;
		if ( v->vz > seg->b2.vz ) seg->b2.vz = v->vz ;
		v ++ ;
    }
}

/* ストリップリストに追加 */
static	HZD_VuSEG	*AddSegs( segs, n_segs, seg )
HZD_VuSEG		*segs ;
u_short			*n_segs ;
HZD_VuSEG		*seg ;
{
    HZD_VuSEG		*new, tmp ;
    SVECTOR		*verts ;
    int			i, n_verts, c ;

    i = 0 ; c = 0 ;
	//    printf( "%d verts \n", seg->b1.vw ) ;
    while( 1 ) {
		new = ( HZD_VuSEG * )MyAlloc( sizeof( HZD_VuSEG ) * ( *n_segs + 1 ) ) ;
		if ( new == NULL ) exit( -1 ) ;
		if ( segs != NULL ) {
			memcpy( new, segs, sizeof( HZD_VuSEG ) * ( *n_segs ) ) ;
			MyFree( segs ) ;
		}
#if 1
		if ( seg->b1.vw > HZX_MAX_VUSEG_VERTS ) {
			/* 分割する */
			verts = tmp.verts = 
				( SVECTOR * )MyAlloc( sizeof( SVECTOR ) * HZX_MAX_VUSEG_VERTS ) ;
			tmp.atr = seg->atr ;
			tmp.tag = NULL ;
			tmp.size = 0 ;
			tmp.b2.vw = seg->b2.vw ;
			n_verts = seg->b1.vw - i ; 
			if ( n_verts <= 0 ) {
				MyFree( verts ) ;
				break ;
			}
			if ( n_verts > HZX_MAX_VUSEG_VERTS ) n_verts = HZX_MAX_VUSEG_VERTS ;
			if ( seg->b2.vw != 0 ) n_verts = n_verts / seg->b2.vw * seg->b2.vw ;
			memcpy( verts, seg->verts + i, sizeof( SVECTOR ) * n_verts ) ;
			if ( seg->b2.vw == 0 ) {
				i += n_verts - 1 ; /* 壁は最終頂点を次に残しとかないと */
				tmp.b1.vw = n_verts ;
				MakeLineStripBoundingBox( &tmp ) ;
			} else {
				i += n_verts ;
				tmp.b1.vw = n_verts ;
				MakeStripBoundingBox( &tmp ) ;
			}
			//DumpVuSEGS( tmp, 1 ) ;
			//printf( "div %d verts \n", tmp.b1.vw ) ;
			*( new + *n_segs ) = tmp ;
			*n_segs = *n_segs + 1 ;
			if ( n_verts < HZX_MAX_VUSEG_VERTS ) break ;
		} else {
			*( new + *n_segs ) = *seg ;
			*n_segs = *n_segs + 1 ;
			break ;
		}
#else
		*( new + *n_segs ) = *seg ;
		*n_segs = *n_segs + 1 ;
		break ;
#endif
		segs = new ;
    }
    if ( c ) MyFree( seg->verts ) ;
    return new ;
}

/* １ブロックのデータをストリップリスト化 */
static	void	MakeLineStrip( tris, n_tris, blk )
Line		*tris ;
int		n_tris ;
HZD_BLOCK	*blk ;
{
    HZD_VuSEG	*segs, *bul_segs, seg, max ;
    int		c, i, j, k, maxlen, minlen ;
    u_char	*used ;
    Line	*tri ;
    int		make, start, minstart, n_strips ;

    if ( n_tris == 0 ) {
		blk->n_segs = blk->n_bul_segs = 0 ;
		blk->segs = blk->bul_segs = NULL ;
		return ;
    }
    used = ( u_char * )MyAlloc( sizeof( u_char ) * n_tris ) ; 
    /* ストリップ数が一番少ないのを採用 */
    start = make = minstart = 0 ;
    minlen = 1000000 ;
    if ( EasyStrip ) make = 1 ;
makelinestriprepeat :    
    segs = bul_segs = NULL ;
    max.verts = NULL ;
    blk->n_segs = blk->n_bul_segs = 0 ;
    n_strips = 0 ;
    memset( used, 0x00, sizeof( u_char ) * n_tris ) ; 
    /* 一番長いストリップを採用 */
    for ( i = start; i < n_tris + start; i ++ ) {
		tri = tris + ( i % n_tris ) ;
		if ( tri->f1 != 0 ) continue ;
		maxlen = 0 ;
		c = 0 ;
		for ( j = 0; j < 2; j ++ ) {
			ChainLineStrip( &seg, n_tris, tris, tri->atr, i % n_tris, j ) ;
			if ( seg.b1.vw > maxlen ) {
				maxlen = seg.b1.vw ;
				if ( max.verts != NULL ) MyFree( max.verts ) ;
				max = seg ;  c = 1 ;
				for ( k = 0; k < n_tris; k ++ ) {
					used[ k ] = tris[ k ].f2 ;
					tris[ k ].f2 = 0 ;
				}
			} else {
				for ( k = 0; k < n_tris; k ++ ) {
					tris[ k ].f2 = 0 ;
				}
				MyFree( seg.verts ) ;
			}
			if ( EasyStrip ) break ;
		} 
		if ( c ) {
			n_strips ++ ;
			for ( k = 0; k < n_tris; k ++ ) if ( used[ k ] ) tris[ k ].f1 = 1 ;
			if ( make ) {
				MakeLineStripBoundingBox( &max ) ;
				if ( max.atr & HZX_SEG_RECOIL_TYPE ) {	
					max.atr &= ~HZX_SEG_RECOIL_TYPE ;
					bul_segs = AddSegs( bul_segs, &blk->n_bul_segs, &max ) ;
				} else {
					segs = AddSegs( segs, &blk->n_segs, &max ) ;
				}
				max.verts = NULL ;
			} else {
				MyFree( max.verts ) ;
				max.verts = NULL ;
			}
		}
    }
    if ( make ) goto makelinestripend ;
    for ( i = 0; i < n_tris; i ++ ) tris[ i ].f1 = 0 ;
    if ( n_strips < minlen ) {
		minlen = n_strips ;
		minstart = start ;
    }
    start ++ ;
    if ( start == n_tris ) {
		make = 1 ; start = minstart ;
    }
    goto makelinestriprepeat ;
makelinestripend :
    MyFree( used ) ;
    blk->segs = segs ;
    blk->bul_segs = bul_segs ;
}

/* Ｘ切断 */
static	void	ScissorringX( sgv, x, mode )
SEGVerts	*sgv ;
int		x, mode ;
{
    int		i, j ;
    int		newpointloc[ 32 ], n_newpoints ;
    IVECTOR	*v1, *v2, *newv ;
    IVECTOR	newpoint[ 32 ] ;
    int		n_points, in1, in2, inout[ 32 ] ;
    float	t, adj ;

    n_newpoints = 0 ;
    n_points = 0 ;
    v1 = sgv->verts ;
    memset( inout, 0x00, sizeof( int ) * 16 ) ;
    for ( i = 0; i < sgv->n_points; i ++, v1 ++ ) {
		if ( i == sgv->n_points - 1 ) { 
			v2 = sgv->verts ;
		} else {
			v2 = v1 + 1 ;
		}
		in1 = in2 = 0 ;
		if ( mode == 0 ) {
			if ( v1->vx >= x ) { in1 = inout[ i ] = 1 ; n_points ++ ; }
			if ( v2->vx >= x ) in2 = 1 ;
		} else {
			if ( v1->vx <= x ) { in1 = inout[ i ] = 1 ; n_points ++ ; }
			if ( v2->vx <= x ) in2 = 1 ;
		}
		if ( in1 != in2 ) {
			t = ( float )( x - v1->vx ) / ( float )( v2->vx - v1->vx ) ;
			if ( t != 0.0F && t != 1.0F ) {
				Scissor = 1 ;
				newpoint[ n_newpoints ].vx = x ;
				adj = ( float )( v2->vy - v1->vy ) * t ;
				if ( adj != 0.0F ) {
					adj = ( adj > 0 ) ? adj + 0.01F : adj - 0.01F ;
				}
				newpoint[ n_newpoints ].vy = v1->vy + ( int )adj ;
				adj = ( float )( v2->vz - v1->vz ) * t ;
				if ( adj != 0.0F ) {
					adj = ( adj > 0 ) ? adj + 0.01F : adj - 0.01F ; 
				}
				newpoint[ n_newpoints ].vz = v1->vz + ( int )adj ;
				newpointloc[ n_newpoints ] = i ;
				n_newpoints ++ ;
				n_points ++ ;
			}
		}
    }
    v1 = sgv->verts ;
    v2 = newv = ( IVECTOR * )MyAlloc( sizeof( IVECTOR ) * n_points ) ;
    for ( i = 0; i < sgv->n_points; i ++, v1 ++ ) {
		if ( inout[ i ] == 1 ) *( v2 ++ ) = *v1 ;
		for ( j = 0; j < n_newpoints; j ++ ) {
			if ( newpointloc[ j ] == i ) {
				*( v2 ++ ) = newpoint[ j ] ;
				break ;
			}
		}
    }
    MyFree( sgv->verts ) ;
    sgv->verts = newv ;
    sgv->n_points = n_points ;
}

/* Ｙ切断 */
static	void	ScissorringY( sgv, y, mode )
SEGVerts	*sgv ;
int		y, mode ;
{
    int		i, j ;
    int		newpointloc[ 32 ], n_newpoints ;
    IVECTOR	*v1, *v2, *newv ;
    IVECTOR	newpoint[ 32 ] ;
    int		n_points, in1, in2, inout[ 32 ] ;
    float	t, adj ;

    n_newpoints = 0 ;
    n_points = 0 ;
    v1 = sgv->verts ;
    memset( inout, 0x00, sizeof( int ) * 16 ) ;
    for ( i = 0; i < sgv->n_points; i ++, v1 ++ ) {
		if ( i == sgv->n_points - 1 ) { 
			v2 = sgv->verts ;
		} else {
			v2 = v1 + 1 ;
		}
		in1 = in2 = 0 ;
		if ( mode == 0 ) {
			if ( v1->vy >= y ) { in1 = inout[ i ] = 1 ; n_points ++ ; }
			if ( v2->vy >= y ) in2 = 1 ;
		} else {
			if ( v1->vy <= y ) { in1 = inout[ i ] = 1 ; n_points ++ ; }
			if ( v2->vy <= y ) in2 = 1 ;
		}
		if ( in1 != in2 ) {
			t = ( float )( y - v1->vy ) / ( float )( v2->vy - v1->vy ) ;
			if ( t != 0.0F && t != 1.0F ) {
				Scissor = 1 ;
				newpoint[ n_newpoints ].vy = y ;
				adj = ( float )( v2->vx - v1->vx ) * t ;
				if ( adj != 0.0F ) {
					adj = ( adj > 0 ) ? adj + 0.01F : adj - 0.01F ;
				}
				newpoint[ n_newpoints ].vx = v1->vx + ( int )adj ;
				adj = ( float )( v2->vz - v1->vz ) * t ;
				if ( adj != 0.0F ) {
					adj = ( adj > 0 ) ? adj + 0.01F : adj - 0.01F ; 
				}
				newpoint[ n_newpoints ].vz = v1->vz + ( int )adj ;
				newpointloc[ n_newpoints ] = i ;
				n_newpoints ++ ;
				n_points ++ ;
			}
		}
    }
    v1 = sgv->verts ;
    v2 = newv = ( IVECTOR * )MyAlloc( sizeof( IVECTOR ) * n_points ) ;
    for ( i = 0; i < sgv->n_points; i ++, v1 ++ ) {
		if ( inout[ i ] == 1 ) *( v2 ++ ) = *v1 ;
		for ( j = 0; j < n_newpoints; j ++ ) {
			if ( newpointloc[ j ] == i ) {
				*( v2 ++ ) = newpoint[ j ] ;
				break ;
			}
		}
    }
    MyFree( sgv->verts ) ;
    sgv->verts = newv ;
    sgv->n_points = n_points ;
}

/* Ｚ切断 */
static	void	ScissorringZ( sgv, z, mode )
SEGVerts	*sgv ;
int		z, mode ;
{
    int		i, j ;
    int		newpointloc[ 32 ], n_newpoints ;
    IVECTOR	*v1, *v2, *newv ;
    IVECTOR	newpoint[ 32 ] ;
    int		n_points, in1, in2, inout[ 32 ] ;
    float	t, adj ;

    n_newpoints = 0 ;
    n_points = 0 ;
    v1 = sgv->verts ;
    memset( inout, 0x00, sizeof( int ) * 16 ) ;
    for ( i = 0; i < sgv->n_points; i ++, v1 ++ ) {
		v2 = sgv->verts + ( i + 1 ) % sgv->n_points ;
		in1 = in2 = 0 ;
		if ( mode == 0 ) {
			if ( v1->vz >= z ) { in1 = inout[ i ] = 1 ; n_points ++ ; }
			if ( v2->vz >= z ) in2 = 1 ;
		} else {
			if ( v1->vz <= z ) { in1 = inout[ i ] = 1 ; n_points ++ ; }
			if ( v2->vz <= z ) in2 = 1 ;
		}
		if ( in1 != in2 ) {
			t = ( float )( z - v1->vz ) / ( float )( v2->vz - v1->vz ) ;
			if ( t != 0.0F && t != 1.0F ) {
				Scissor = 1 ;
				newpoint[ n_newpoints ].vz = z ;
				adj = ( float )( v2->vx - v1->vx ) * t ;
				if ( adj != 0.0F ) {
					adj = ( adj > 0 ) ? adj + 0.01F : adj - 0.01F ;
				}
				newpoint[ n_newpoints ].vx = v1->vx + ( int )adj ;
				adj = ( float )( v2->vy - v1->vy ) * t ;
				if ( adj != 0.0F ) {
					adj = ( adj > 0 ) ? adj + 0.01F : adj - 0.01F ; 
				}
				newpoint[ n_newpoints ].vy = v1->vy + ( int )adj ;
				newpointloc[ n_newpoints ] = i ;
				n_newpoints ++ ;
				n_points ++ ;
			}
		}
    }
    v1 = sgv->verts ;
    v2 = newv = ( IVECTOR * )MyAlloc( sizeof( IVECTOR ) * n_points ) ;
    for ( i = 0; i < sgv->n_points; i ++, v1 ++ ) {
		if ( inout[ i ] == 1 ) *( v2 ++ ) = *v1 ;
		for ( j = 0; j < n_newpoints; j ++ ) {
			if ( newpointloc[ j ] == i ) {
				*( v2 ++ ) = newpoint[ j ] ;
				break ;
			}
		}
    }
    MyFree( sgv->verts ) ;
    sgv->verts = newv ;
    sgv->n_points = n_points ;
}

/* 壁の切断 */
static	void	ScissorringSegment( sgv, atr, v1, v2, v3, v4 )
SEGVerts	*sgv ;
IVECTOR		*v1, *v2, *v3, *v4 ;
{
    IVECTOR	*verts, *v ;
    int		i ;

    /* 初期設定 */
    sgv->atr = atr ;
    if ( v3->vx == v4->vx &&
		v3->vy == v4->vy &&
		v3->vz == v4->vz ) {
		sgv->n_points = 3 ;	
    } else {
		sgv->n_points = 4 ;
    }
    sgv->verts = verts = ( IVECTOR * )MyAlloc( sizeof( IVECTOR ) * 4 ) ;
    verts[ 0 ] = *v1 ;
    verts[ 1 ] = *v2 ;
    verts[ 2 ] = *v3 ;
    verts[ 3 ] = *v4 ;
	//if ( DebugFlag ) DumpSEGVerts( sgv ) ;
    v = sgv->verts ;
    for ( i = 0; i < sgv->n_points;  i ++, v ++ ) {
		if ( v->vx < -16000 || v->vx > 16000 ) {
			/* 要Ｘ切断 */
			if ( atr & HZX_FLOOR_STEP ) {
				printf( "warning : step floor is divided : X\n" ) ;
			}
			ScissorringX( sgv, -16000, 0 ) ;
			ScissorringX( sgv, 16000, 1 ) ;
			break ;
		}
    }
	//if ( DebugFlag ) DumpSEGVerts( sgv ) ;
    v = sgv->verts ;
    for ( i = 0; i < sgv->n_points;  i ++, v ++ ) {
		if ( v->vy < -16000 || v->vy > 16000 ) {
			/* 要Ｙ切断 */
			if ( atr & HZX_FLOOR_STEP ) {
				printf( "warning : step floor is divided : Y\n" ) ;
			}
			ScissorringY( sgv, -16000, 0 ) ;
			ScissorringY( sgv, 16000, 1 ) ;	    
			break ;
		}
    }
	//if ( DebugFlag ) DumpSEGVerts( sgv ) ;
    v = sgv->verts ;
    for ( i = 0; i < sgv->n_points;  i ++, v ++ ) {
		if ( v->vz < -16000 || v->vz > 16000 ) {
			/* 要Ｚ切断 */
			if ( atr & HZX_FLOOR_STEP ) {
				printf( "warning : step floor is divided : Z\n" ) ;
			}
			ScissorringZ( sgv, -16000, 0 ) ;
			ScissorringZ( sgv, 16000, 1 ) ;	    
			break ;
		}
    }
	//if ( DebugFlag ) DumpSEGVerts( sgv ) ;
}

/* 誤差の吸収 */
static	void	DelDelta( v1, v2, v3, v4 )
IVECTOR		*v1, *v2, *v3, *v4 ;
{
    int		v ;

    v = v1->vx - v2->vx ; if ( v < 0 ) v = -v ;
    if ( v == 1 ) v2->vx = v1->vx ;
    v = v1->vy - v2->vy ; if ( v < 0 ) v = -v ;
    if ( v == 1 ) v2->vy = v1->vy ;
    v = v1->vz - v2->vz ; if ( v < 0 ) v = -v ;
    if ( v == 1 ) v2->vz = v1->vz ;

    v = v1->vx - v3->vx ; if ( v < 0 ) v = -v ;
    if ( v == 1 ) v3->vx = v1->vx ;
    v = v1->vy - v3->vy ; if ( v < 0 ) v = -v ;
    if ( v == 1 ) v3->vy = v1->vy ;
    v = v1->vz - v3->vz ; if ( v < 0 ) v = -v ;
    if ( v == 1 ) v3->vz = v1->vz ;

    v = v1->vx - v4->vx ; if ( v < 0 ) v = -v ;
    if ( v == 1 ) v4->vx = v1->vx ;
    v = v1->vy - v4->vy ; if ( v < 0 ) v = -v ;
    if ( v == 1 ) v4->vy = v1->vy ;
    v = v1->vz - v4->vz ; if ( v < 0 ) v = -v ;
    if ( v == 1 ) v4->vz = v1->vz ;

    v = v2->vx - v3->vx ; if ( v < 0 ) v = -v ;
    if ( v == 1 ) v3->vx = v2->vx ;
    v = v2->vy - v3->vy ; if ( v < 0 ) v = -v ;
    if ( v == 1 ) v3->vy = v2->vy ;
    v = v2->vz - v3->vz ; if ( v < 0 ) v = -v ;
    if ( v == 1 ) v3->vz = v2->vz ;

    v = v2->vx - v4->vx ; if ( v < 0 ) v = -v ;
    if ( v == 1 ) v4->vx = v2->vx ;
    v = v2->vy - v4->vy ; if ( v < 0 ) v = -v ;
    if ( v == 1 ) v4->vy = v2->vy ;
    v = v2->vz - v4->vz ; if ( v < 0 ) v = -v ;
    if ( v == 1 ) v4->vz = v2->vz ;

    v = v3->vx - v4->vx ; if ( v < 0 ) v = -v ;
    if ( v == 1 ) v4->vx = v3->vx ;
    v = v3->vy - v4->vy ; if ( v < 0 ) v = -v ;
    if ( v == 1 ) v4->vy = v3->vy ;
    v = v3->vz - v4->vz ; if ( v < 0 ) v = -v ;
    if ( v == 1 ) v4->vz = v3->vz ;
}

static	void	DelDelta2( seg, bo )
SEGVerts		*seg ;
BLOCKOBJECT		*bo ;
{
    int		i, j, v, c, k ;
    IVECTOR	*v1, *v2 ;
    IVECTOR	*old, max, min ;

    old = ( IVECTOR * )malloc( sizeof( IVECTOR ) * seg->n_points ) ;
    memcpy( old, seg->verts, sizeof( IVECTOR ) * seg->n_points ) ;
    c = 0 ;

    max = min = *( seg->verts ) ;
    v1 = seg->verts + 1 ;
    for ( i = 1; i < seg->n_points; i ++ ) {
		if ( v1->vx > max.vx ) max.vx = v1->vx ;
		if ( v1->vx < min.vx ) min.vx = v1->vx ;
		if ( v1->vy > max.vy ) max.vy = v1->vy ;
		if ( v1->vy < min.vy ) min.vy = v1->vy ;
		if ( v1->vz > max.vz ) max.vz = v1->vz ;
		if ( v1->vz < min.vz ) min.vz = v1->vz ;
		v1 ++ ;
    }
    
    for ( i = 0; i < seg->n_points - 1; i ++ ) {
		v1 = seg->verts + i ;
		for ( j = i + 1; j < seg->n_points; j ++ ) {
			v2 = seg->verts + j ;	
    
			v = v1->vx - v2->vx ; if ( v < 0 ) v = -v ;
			if ( v == 1 ) { 
				if ( v1->vx == min.vx || v1->vx == max.vx ) {
					v2->vx = v1->vx ; 
				} else {
					v1->vx = v2->vx ; 
				} 
				c = 1 ;
			}
			v = v1->vy - v2->vy ; if ( v < 0 ) v = -v ;
			if ( v == 1 ) { 
				if ( v1->vy == min.vy || v1->vy == max.vy ) {
					v2->vy = v1->vy ; 
				} else {
					v1->vy = v2->vy ; 
				} 
				c = 1 ;
			}
			v = v1->vz - v2->vz ; if ( v < 0 ) v = -v ;
			if ( v == 1 ) { 
				if ( v1->vz == min.vz || v1->vz == max.vz ) {
					v2->vz = v1->vz ; 
				} else {
					v1->vz = v2->vz ; 
				} 
				c = 1 ;
			}
		}
    }
#if 0
    if ( c ) {
		printf( "--- Del Delta ---\n" ) ;
		v1 = old ;
		for ( k = 0; k < seg->n_points; k ++ ) {
			DumpIVector( v1 ) ;
			v1 ++ ;
		}
//		DumpSEGVerts( seg ) ;
		DumpSEGVerts2( seg, bo ) ;
    }
#endif
    free( old ) ;
}

/* 壁データ変換 */
static	void	ConvertSegment( blk, bo )
HZD_BLOCK	*blk ;
BLOCKOBJECT	*bo ;
{
    SEGMENTOBJECT	*so ;
    DOUBLEXYZ		*verts ;
    int		n_segments, atr ;
    SEGVerts	sgv ;
    IVECTOR	v1, v2, v3, v4, vv ;

    N_Lines = 0 ;
    Lines = ( Line * )MyAlloc( sizeof( Line ) * 65535 ) ; /* 適当 */

    n_segments = bo->nHazardCount ;
    so = bo->lpHazard ;
    memset( &sgv, 0x00, sizeof( SEGVerts ) ) ;
    while( -- n_segments >= 0 ) {
		atr = so->nAttribute ;
		verts = so->dxyzVertex ;

		DXYZ2IV( &v1, &verts[ 0 ] ) ;
		DXYZ2IV( &v2, &verts[ 1 ] ) ;
		DXYZ2IV( &v3, &verts[ 2 ] ) ;
		DXYZ2IV( &v4, &verts[ 3 ] ) ;
		v1.vx -= bo->nxyzCenter.x ;
		v1.vy -= bo->nxyzCenter.y ;
		v1.vz -= bo->nxyzCenter.z ;
		v2.vx -= bo->nxyzCenter.x ;
		v2.vy -= bo->nxyzCenter.y ;
		v2.vz -= bo->nxyzCenter.z ;
		v3.vx -= bo->nxyzCenter.x ;
		v3.vy -= bo->nxyzCenter.y ;
		v3.vz -= bo->nxyzCenter.z ;
		v4.vx -= bo->nxyzCenter.x ;
		v4.vy -= bo->nxyzCenter.y ;
		v4.vz -= bo->nxyzCenter.z ;
#if 0
		v1.vx = ( int )floor( verts[ 0 ].x ) - bo->nxyzCenter.x ;
		v1.vy = ( int )floor( verts[ 0 ].y ) - bo->nxyzCenter.y ;
		v1.vz = ( int )floor( verts[ 0 ].z ) - bo->nxyzCenter.z ;
		v2.vx = ( int )floor( verts[ 1 ].x ) - bo->nxyzCenter.x ;
		v2.vy = ( int )floor( verts[ 1 ].y ) - bo->nxyzCenter.y ;
		v2.vz = ( int )floor( verts[ 1 ].z ) - bo->nxyzCenter.z ;
		v3.vx = ( int )floor( verts[ 2 ].x ) - bo->nxyzCenter.x ;
		v3.vy = ( int )floor( verts[ 2 ].y ) - bo->nxyzCenter.y ;
		v3.vz = ( int )floor( verts[ 2 ].z ) - bo->nxyzCenter.z ;
		v4.vx = ( int )floor( verts[ 3 ].x ) - bo->nxyzCenter.x ;
		v4.vy = ( int )floor( verts[ 3 ].y ) - bo->nxyzCenter.y ;
		v4.vz = ( int )floor( verts[ 3 ].z ) - bo->nxyzCenter.z ;
#endif
		DelDelta( &v1, &v2, &v3, &v4 ) ;
		if ( so->nVertexCount == 3 ) {
			if ( v1.vx == v2.vx && 
				v1.vz == v2.vz &&	
				v1.vy > v2.vy ) { vv = v1 ; v1 = v2 ; v2 = vv ; }
			else if ( v3.vx == v2.vx && 
					 v3.vz == v2.vz &&	
					 v3.vy > v2.vy ) { vv = v3 ; v3 = v2 ; v2 = vv ; }
			v4 = v3 ;
		} else {
			if ( v1.vx == v2.vx && 
				v1.vz == v2.vz &&	
				v1.vy > v2.vy ) { vv = v1 ; v1 = v2 ; v2 = vv ; }
			if ( v3.vx == v4.vx && 
				v3.vz == v4.vz &&	
				v4.vy > v3.vy ) { vv = v3 ; v3 = v4 ; v4 = vv ; }
		}
		Scissor = 0 ;
		ScissorringSegment( &sgv, atr, &v1, &v2, &v3, &v4 ) ;
		if ( Scissor ) {
			DelDelta2( &sgv, bo ) ;
			Scissor = 0 ;
		}
		if ( sgv.n_points > 2 ) MakeLine( &sgv ) ;
		MyFree( sgv.verts ) ;
		so ++ ;
    }	
    MakeLineStrip( Lines, N_Lines, blk ) ;


    {
		HZD_VuSEG	*segs, *segs2 ;
		int		n_segs, i, j ;
		SVECTOR		*b1, *b2 ;

		segs = blk->segs ;
		n_segs = blk->n_segs ;
		if ( n_segs > 0 ) {
			for ( i = 0; i < n_segs - 1; i ++ ) {
				b1 = &segs->b1 ; b2 = &segs->b2 ;
				segs2 = segs + 1 ;
				for ( j = i + 1; j < n_segs; j ++ ) {
					if ( segs2->b1.vx >= b1->vx && segs2->b2.vx <= b2->vx &&
						segs2->b1.vy >= b1->vy && segs2->b2.vy <= b2->vy &&
						segs2->b1.vz >= b1->vz && segs2->b2.vz <= b2->vz ) {
//						printf( "[%d]:<%d/%d> segs inside <%d/%d> segs\n", blk->block_no, j, 
//							   segs2->b1.vw, i, b1->vw ) ;
					}
					segs2 ++ ;
				}
				segs ++ ;
			}
		}
    }
    MyFree( Lines ) ;
}

/* フラットチェック */
static	int		CheckFlat( v1, v2, v3 )
SVECTOR			*v1, *v2, *v3 ;
{
    if ( v1->vy == v2->vy &&
		v2->vy == v3->vy ) return 2 ;
    return 1 ;
}

static	int		CheckFlat4( v1, v2, v3, v4 )
SVECTOR			*v1, *v2, *v3, *v4 ;
{
    if ( v1->vy == v2->vy &&
		v2->vy == v3->vy && 
		v3->vy == v4->vy ) return 2 ;
    return 1 ;
}

static	int		CheckRect( v1, v2, v3, v4 )
SVECTOR			*v1, *v2, *v3, *v4 ;
{
    if ( v1->vx == v2->vx && v3->vx == v4->vx &&
		v1->vz == v4->vz && v2->vz == v3->vz ) return 1 ;
    if ( v1->vz == v2->vz && v3->vz == v4->vz &&
		v1->vx == v4->vx && v2->vx == v3->vx ) return 1 ;
    return 0 ;
}

/* 四角形単体分割 */
static	void	MakeSquare( sgv )
SEGVerts	*sgv ;
{
    Square	*sq, *tt ;
    IVECTOR	*iv ;
    int		i, j, n_points ;

    if ( sgv->n_points < 3 ) return ;

    sq = Squares + N_Squares ;
    iv = sgv->verts + 1 ;
    n_points = sgv->n_points ;
next_square :
    sq->atr = sgv->atr ;
    sq->v[ 0 ].vx = sgv->verts->vx ;
    sq->v[ 0 ].vy = sgv->verts->vy ;
    sq->v[ 0 ].vz = sgv->verts->vz ;
    sq->v[ 1 ].vx = iv->vx ;
    sq->v[ 1 ].vy = iv->vy ;
    sq->v[ 1 ].vz = iv->vz ;	
    sq->v[ 2 ].vx = ( iv + 1 )->vx ;
    sq->v[ 2 ].vy = ( iv + 1 )->vy ;
    sq->v[ 2 ].vz = ( iv + 1 )->vz ;	
    sq->v[ 0 ].vw = 1 ;			/* 使用してないよフラグ */
    if ( n_points == 3 ) {
		n_points = 0 ;
		sq->n_points = 3 ;
		if ( CheckFlat( &sq->v[ 0 ], &sq->v[ 1 ], &sq->v[ 2 ] ) == 2 ) {
			sq->atr |= HZX_FLOOR_FLAT ;
		}
		tt = Squares ;
		for ( j = 0; j < N_Squares; j ++, tt ++ ) {
			if ( tt->atr == sq->atr &&
				tt->n_points == sq->n_points &&
				tt->v[ 0 ].vx == sq->v[ 0 ].vx &&
				tt->v[ 0 ].vy == sq->v[ 0 ].vy &&
				tt->v[ 0 ].vz == sq->v[ 0 ].vz &&
				tt->v[ 1 ].vx == sq->v[ 1 ].vx &&
				tt->v[ 1 ].vy == sq->v[ 1 ].vy &&
				tt->v[ 1 ].vz == sq->v[ 1 ].vz &&
				tt->v[ 2 ].vx == sq->v[ 2 ].vx &&
				tt->v[ 2 ].vy == sq->v[ 2 ].vy &&
				tt->v[ 2 ].vz == sq->v[ 2 ].vz ) {
				break ;
			}
		}
		if ( j != N_Squares ) goto make_next_square ;
		sq ++ ;
		N_Squares ++ ;
    } else {
		sq->n_points = 4 ;
		sq->v[ 3 ].vx = ( iv + 2 )->vx ;
		sq->v[ 3 ].vy = ( iv + 2 )->vy ;
		sq->v[ 3 ].vz = ( iv + 2 )->vz ;

		n_points -= 4 ;
		if ( n_points != 0 ) {	
			n_points += 2 ;
			iv += 2 ;
		}

		if ( CheckFlat4( &sq->v[ 0 ], &sq->v[ 1 ], &sq->v[ 2 ], &sq->v[ 3 ] ) == 2 ) {
			sq->atr |= HZX_FLOOR_FLAT ;
		}
		if ( CheckRect( &sq->v[ 0 ], &sq->v[ 1 ], &sq->v[ 2 ], &sq->v[ 3 ] ) ) {
			sq->atr |= HZX_FLOOR_RECT ;
		}
		tt = Squares ;
		for ( j = 0; j < N_Squares; j ++, tt ++ ) {
			if ( tt->atr == sq->atr &&
				tt->n_points == sq->n_points &&
				tt->v[ 0 ].vx == sq->v[ 0 ].vx &&
				tt->v[ 0 ].vy == sq->v[ 0 ].vy &&
				tt->v[ 0 ].vz == sq->v[ 0 ].vz &&
				tt->v[ 1 ].vx == sq->v[ 1 ].vx &&
				tt->v[ 1 ].vy == sq->v[ 1 ].vy &&
				tt->v[ 1 ].vz == sq->v[ 1 ].vz &&
				tt->v[ 2 ].vx == sq->v[ 2 ].vx &&
				tt->v[ 2 ].vy == sq->v[ 2 ].vy &&
				tt->v[ 2 ].vz == sq->v[ 2 ].vz &&
				tt->v[ 3 ].vx == sq->v[ 3 ].vx &&
				tt->v[ 3 ].vy == sq->v[ 3 ].vy &&
				tt->v[ 3 ].vz == sq->v[ 3 ].vz ) {
				break ;
			}
		}
		if ( j != N_Squares ) goto make_next_square ;
		sq ++ ;
		N_Squares ++ ;
    } 
make_next_square :
    if ( n_points == 0 ) return ;
    else {
		goto next_square ;
    }
}

/* 法線長さ32000 */
static	SVECTOR	GetNormal( v1, v2, v3, v4, n )
SVECTOR		*v1, *v2, *v3, *v4 ;
int		n ;
{
    FVECTOR	normal ;
    SVECTOR	res, tmp ;
    float	len ;

    normal.vx = ( v3->vy - v2->vy ) * ( v2->vz - v1->vz ) -
		( v2->vy - v1->vy ) * ( v3->vz - v2->vz ) ;
    normal.vy = ( v3->vz - v2->vz ) * ( v2->vx - v1->vx ) -
		( v2->vz - v1->vz ) * ( v3->vx - v2->vx ) ;
    normal.vz = ( v3->vx - v2->vx ) * ( v2->vy - v1->vy ) -
		( v2->vx - v1->vx ) * ( v3->vy - v2->vy ) ;
    if ( normal.vy < 0.0F ) {
		normal.vx *= -1.0F ;
		normal.vy *= -1.0F ;
		normal.vz *= -1.0F ;
		/* 反転 */
		if ( n == 3 ) {
			tmp = *v1 ; *v1 = *v3 ; *v3 = tmp ;
		} else {
			tmp = *v1 ; *v1 = *v4 ; *v4 = tmp ;
			tmp = *v2 ; *v2 = *v3 ; *v3 = tmp ;
		}
    }
    len = sqrtf( normal.vx * normal.vx +
				normal.vy * normal.vy +
				normal.vz * normal.vz ) ;
    res.vx = ( short )( normal.vx * 32000.0F / len ) ;
    res.vy = ( short )( normal.vy * 32000.0F / len ) ;
    res.vz = ( short )( normal.vz * 32000.0F / len ) ;
    return res ;
}

/* 床、ストリップ化しない。属性のみ */
static	void	MakeFloorGroup( sqs, n_sqs, blk )
Square		*sqs ;
int		n_sqs ;
HZD_BLOCK	*blk ;
{
    HZD_VuSEG	*segs, *bul_segs, seg, max ;
    int		c, i, j, k, maxlen, minlen, flag ;
    Square	*sq ;
    SVECTOR	normal ;

    segs = bul_segs = NULL ;
    blk->n_flrs = blk->n_bul_flrs = 0 ;
    sq = sqs ;
    for( i = 0; i < n_sqs; i ++ ) {
		if ( sq[ i ].v[ 0 ].vw == 0 ) continue ;
		seg.atr = sq[ i ].atr ;
		seg.b1.vw = 0 ;
		seg.b2.vw = sq[ i ].n_points ;
		seg.verts = NULL ;
		sq[ i ].v[ 0 ].vw = 0 ;
		normal = GetNormal( &sq[ i ].v[ 0 ], &sq[ i ].v[ 1 ], &sq[ i ].v[ 2 ], 
						   &sq[ i ].v[ 3 ], sq[ i ].n_points ) ;
		if ( normal.vx == 0 && normal.vy == 0 && normal.vz == 0 ) {
			printf( "warning : invalid floor\n " ) ;
			DumpSquare( &sq[ i ] ) ;
			continue ;
		}
		AddVerts( &seg, &sq[ i ].v[ 0 ], normal.vx ) ;
		AddVerts( &seg, &sq[ i ].v[ 1 ], normal.vy ) ;
		AddVerts( &seg, &sq[ i ].v[ 2 ], normal.vz ) ;
		if ( sq[ i ].n_points == 4 ) {
			AddVerts( &seg, &sq[ i ].v[ 3 ], 0 ) ;
		}
		for( j = i + 1; j < n_sqs; j ++ ) {	
			if ( sq[ j ].atr == seg.atr &&
				sq[ j ].n_points == sq[ i ].n_points ) {
				sq[ j ].v[ 0 ].vw = 0 ;
				normal = GetNormal( &sq[ j ].v[ 0 ], &sq[ j ].v[ 1 ], &sq[ j ].v[ 2 ],
								   &sq[ j ].v[ 3 ], sq[ j ].n_points ) ;
				if ( normal.vx == 0 && normal.vy == 0 && normal.vz == 0 ) {
					printf( "warning : invalid floor\n " ) ;
					DumpSquare( &sq[ j ] ) ;
					continue ;
				}
				AddVerts( &seg, &sq[ j ].v[ 0 ], normal.vx ) ;
				AddVerts( &seg, &sq[ j ].v[ 1 ], normal.vy ) ;
				AddVerts( &seg, &sq[ j ].v[ 2 ], normal.vz ) ;
				if ( sq[ i ].n_points == 4 ) {
					AddVerts( &seg, &sq[ j ].v[ 3 ], 0 ) ;
				}		
			}
		}
		if ( seg.b1.vw > 0 ) {
			MakeStripBoundingBox( &seg ) ;
			if ( seg.atr & HZX_FLOOR_RECOILE_TYPE ) {
				seg.atr &= ~HZX_FLOOR_RECOILE_TYPE ;
				bul_segs = AddSegs( bul_segs, &blk->n_bul_flrs, &seg ) ;
			} else {
				segs = AddSegs( segs, &blk->n_flrs, &seg ) ;
			}
			seg.verts = NULL ;
		}
    }
    blk->flrs = segs ;
    blk->bul_flrs = bul_segs ;
//	DumpVuSEGS( blk->flrs, blk->n_flrs ) ;
}

/* デバッグ様 */
static	int		SameIV( IVECTOR *v1, IVECTOR *v2 )
{
	if ( v1->vx == v2->vx &&
		 v1->vy == v2->vy &&
		 v1->vz == v2->vz ) return 1 ;
	return 0 ;
}

static	int		DebugFloor( BLOCKOBJECT *bo, 
						    IVECTOR *v1, IVECTOR *v2, IVECTOR *v3, IVECTOR *v4, int atr )
{
	IVECTOR	cv1 = { -5480, 9450, -12930 } ;
	IVECTOR	cv2 = { -5920, 9450, -12930 } ;
	IVECTOR	cv3 = { -5920, 9450, -14570 } ;
	IVECTOR	cv4 = { -5480, 9450, -14570 } ;	
	IVECTOR	cv5 = { -11800, 9000, -19800 } ;
	IVECTOR	cv6 = { -1000, 9000, -8550 } ;
	IVECTOR	cv7 = { -1000, 9000, -19800 } ;
	IVECTOR	cv8 = { -11800, 9000, -19800 } ;
	IVECTOR	vv1, vv2, vv3, vv4 ;
	INTXYZ	*c ;

	if ( !( atr & HZX_FLOOR_RECOILE_TYPE ) ) return 1 ;

	c = &bo->nxyzCenter ;
	vv1.vx = v1->vx + c->x ;
	vv1.vy = v1->vy + c->y ;
	vv1.vz = v1->vz + c->z ;
	vv2.vx = v2->vx + c->x ;
	vv2.vy = v2->vy + c->y ;
	vv2.vz = v2->vz + c->z ;
	vv3.vx = v3->vx + c->x ;
	vv3.vy = v3->vy + c->y ;
	vv3.vz = v3->vz + c->z ;
	vv4.vx = v4->vx + c->x ;
	vv4.vy = v4->vy + c->y ;
	vv4.vz = v4->vz + c->z ;
	
//printf( "-----\n" ) ;
//DumpIVector( &vv1 ) ;
//DumpIVector( &vv2 ) ;
//DumpIVector( &vv3 ) ;
//DumpIVector( &vv4 ) ;

	if ( SameIV( &vv1, &cv4 ) ||
		 SameIV( &vv2, &cv3 ) ||
		 SameIV( &vv3, &cv2 ) ||
		 SameIV( &vv4, &cv1 ) ||
		 SameIV( &vv1, &cv5 ) ||		 
		 SameIV( &vv2, &cv6 ) ||		 
		 SameIV( &vv3, &cv7 ) ||		 
		 SameIV( &vv4, &cv8 ) ) {
		printf( "debug floor find!!!!!!!!!!!!!\n" ) ;
//DumpIVector( &vv1 ) ;
//DumpIVector( &vv2 ) ;
//DumpIVector( &vv3 ) ;
//DumpIVector( &vv4 ) ;
		return 1 ;
	}
	return 1 ;
}

/* 床データ変換 */
static	void	ConvertFloor( blk, bo )
HZD_BLOCK	*blk ;
BLOCKOBJECT	*bo ;
{
    SEGMENTOBJECT	*so ;
    DOUBLEXYZ		*verts, dxyz ;
    int		n_floors, atr ;
    SEGVerts	sgv ;
    IVECTOR	v1, v2, v3, v4 ;

    N_Squares = 0 ;
    Squares = ( Square * )MyAlloc( sizeof( Square ) * 65535 ) ; 
    n_floors = bo->nFloorCount ;
    so = bo->lpFloor ;
    memset( &sgv, 0x00, sizeof( SEGVerts ) ) ;
    while( -- n_floors >= 0 ) {
		atr = so->nAttribute ;
		verts = so->dxyzVertex ;
#if 1
		DXYZ2IV( &v1, &verts[ 0 ] ) ;
		DXYZ2IV( &v2, &verts[ 1 ] ) ;
		DXYZ2IV( &v3, &verts[ 2 ] ) ;
		v1.vx -= bo->nxyzCenter.x ;
		v1.vy -= bo->nxyzCenter.y ;
		v1.vz -= bo->nxyzCenter.z ;
		v2.vx -= bo->nxyzCenter.x ;
		v2.vy -= bo->nxyzCenter.y ;
		v2.vz -= bo->nxyzCenter.z ;
		v3.vx -= bo->nxyzCenter.x ;
		v3.vy -= bo->nxyzCenter.y ;
		v3.vz -= bo->nxyzCenter.z ;
#else
		v1.vx = ( int )floor( verts[ 0 ].x ) - bo->nxyzCenter.x ;
		v1.vy = ( int )floor( verts[ 0 ].y ) - bo->nxyzCenter.y ;
		v1.vz = ( int )floor( verts[ 0 ].z ) - bo->nxyzCenter.z ;
		v2.vx = ( int )floor( verts[ 1 ].x ) - bo->nxyzCenter.x ;
		v2.vy = ( int )floor( verts[ 1 ].y ) - bo->nxyzCenter.y ;
		v2.vz = ( int )floor( verts[ 1 ].z ) - bo->nxyzCenter.z ;
		v3.vx = ( int )floor( verts[ 2 ].x ) - bo->nxyzCenter.x ;
		v3.vy = ( int )floor( verts[ 2 ].y ) - bo->nxyzCenter.y ;
		v3.vz = ( int )floor( verts[ 2 ].z ) - bo->nxyzCenter.z ;
#endif
		if ( so->nVertexCount == 3 ) {
			v4 = v3 ;
		} else {
#if 1
			DXYZ2IV( &v4, &verts[ 3 ] ) ;
			v4.vx -= bo->nxyzCenter.x ;
			v4.vy -= bo->nxyzCenter.y ;
			v4.vz -= bo->nxyzCenter.z ;
#else
			v4.vx = ( int )floor( verts[ 3 ].x ) - bo->nxyzCenter.x ;
			v4.vy = ( int )floor( verts[ 3 ].y ) - bo->nxyzCenter.y ;
			v4.vz = ( int )floor( verts[ 3 ].z ) - bo->nxyzCenter.z ;
#endif
		}

//		if ( !DebugFloor( bo, &v1, &v2, &v3, &v4, atr ) ) goto convert_floor_skip ;

		if ( !( atr & HZX_FLOOR_RECOILE_TYPE ) ) {
			DelDelta( &v1, &v2, &v3, &v4 ) ;
		}
		Scissor = 0 ;
		ScissorringSegment( &sgv, atr, &v1, &v2, &v3, &v4 ) ;
		if ( Scissor && !( atr & HZX_FLOOR_RECOILE_TYPE ) ) {
			DelDelta2( &sgv, bo ) ;
			Scissor = 0 ;
		}
		MakeSquare( &sgv ) ;
		MyFree( sgv.verts ) ;
convert_floor_skip :
		so ++ ;
    }	
    MakeFloorGroup( Squares, N_Squares, blk ) ;
    MyFree( Squares ) ;    
}

#define	HZX_N_RTN	(10)
static	int	HZX_ReservedTrapID[] = {
	13300053,	/* no_crouch */
	13033828,	/* enhidden */
	12819940,	/* hidden */
	4702179,	/* wallto */
	3379883,	/* dark */
	9557678,	/* nobeyond */
	4088226,	/* corpshdw */
	5961588,	/* bomblost */
	12767202,	/* wallto_hidden */
	13434271	/* CORPROTTEN */
} ;

static char *HZX_ReservedTrapName[] = {
	"no_crouch",
	"enhidden",
	"hidden",
	"wallto",
	"dark",
	"nobeyond",
	"corpshdw",
	"bomblost",
	"wallto_hidden",
	"CORPROTTEN"
} ;

/* 予約トラップ名チェック */
static	int		CheckTrapName( name )
char			*name ;
{
	int			i, id, *rtid ;
	char		**rtn ;

	id = GetStrCodeTrap( name ) ;
	rtid = HZX_ReservedTrapID ;
	rtn = HZX_ReservedTrapName ;

	for ( i = HZX_N_RTN; i > 0; i --, rtid ++, rtn ++ ) {
		if ( *rtid == id && strcmp( *rtn, name ) ) {
			printf( "Scn2Hzx Error!! : TrapID conflict [%s : %s]\n",
				     name, *rtn ) ;
			exit( -1 ) ;
//			return 1 ;
		}
	}
	return 0 ;
}


/* トラップ変換 */
static	void	ConvertTrap( blk, bo )
HZD_BLOCK	*blk ;
BLOCKOBJECT	*bo ;
{
    int		n_traps, i ;
    HZD_TRP	*trp ;
    TRAPOBJECT	*to ;
    IVECTOR	iv ;

    n_traps = blk->n_traps = bo->nTrapCount ;
    trp = blk->traps = ( HZD_TRP * )MyAlloc( sizeof( HZD_TRP ) * n_traps ) ;
    to = bo->lpTrap ;
    for ( i = 0; i < n_traps; i ++ ) {
		iv.vx = ( int )( floor( to->dxyzBound[ 0 ].x ) - bo->nxyzCenter.x ) ;
		iv.vy = ( int )( floor( to->dxyzBound[ 0 ].y ) - bo->nxyzCenter.y ) ;
		iv.vz = ( int )( floor( to->dxyzBound[ 0 ].z ) - bo->nxyzCenter.z ) ;
		if ( iv.vx < -32000 ) iv.vx = -32000 ;
		if ( iv.vx > 32000 ) iv.vx = 32000 ;
		if ( iv.vy < -32000 ) iv.vy = -32000 ;
		if ( iv.vy > 32000 ) iv.vy = 32000 ;
		if ( iv.vz < -32000 ) iv.vz = -32000 ;
		if ( iv.vz > 32000 ) iv.vz = 32000 ;
		trp->b1.vx = iv.vx ;
		trp->b1.vy = iv.vy ;
		trp->b1.vz = iv.vz ;
		iv.vx = ( int )( floor( to->dxyzBound[ 1 ].x ) - bo->nxyzCenter.x ) ;
		iv.vy = ( int )( floor( to->dxyzBound[ 1 ].y ) - bo->nxyzCenter.y ) ;
		iv.vz = ( int )( floor( to->dxyzBound[ 1 ].z ) - bo->nxyzCenter.z ) ;
		//	iv.vx = ( int )( to->dxyzBound[ 1 ].x - bo->nxyzCenter.x + 0.50F ) ;
		//	iv.vy = ( int )( to->dxyzBound[ 1 ].y - bo->nxyzCenter.y + 0.50F ) ;
		//	iv.vz = ( int )( to->dxyzBound[ 1 ].z - bo->nxyzCenter.z + 0.50F ) ;
		if ( iv.vx < -32000 ) iv.vx = -32000 ;
		if ( iv.vx > 32000 ) iv.vx = 32000 ;
		if ( iv.vy < -32000 ) iv.vy = -32000 ;
		if ( iv.vy > 32000 ) iv.vy = 32000 ;
		if ( iv.vz < -32000 ) iv.vz = -32000 ;
		if ( iv.vz > 32000 ) iv.vz = 32000 ;
		trp->b2.vx = iv.vx ;
		trp->b2.vy = iv.vy ;
		trp->b2.vz = iv.vz ;
		trp->name_id = GetStrCodeTrap( to->szName ) ;
		if ( CheckTrapName( to->szName ) ) trp->b1.vw = 1 ;
		else								 trp->b1.vw = 0 ;
		if ( to->sAttribute & HZX_TRAP_NO_CHECK ) {
			trp->b1.vw |= 0x2 ;
		}
		/* ダークエリアレベルを設定 */
		trp->b1.vw |= ( to->sAttribute & 0x00f0 ) ;
		trp ++ ; to ++ ;
    }
}

/* ブロックデータ変換 */
static	void	ConvertBlock( grp, go )
HZD_GRP		*grp ;
GROUPOBJECT	*go ;
{
    int		n_blocks, i ;
    HZD_BLOCK	*blk ;
    BLOCKOBJECT	*bo ;

    n_blocks = grp->n_blocks = go->nBlockCount ;
    bo = go->lpBlock ;
    blk = grp->blocks = ( HZD_BLOCK * )MyAlloc( sizeof( HZD_BLOCK ) * n_blocks ) ;
    for ( i = 0; i < n_blocks; i ++ ) {
		blk->tx = bo->nxyzCenter.x ;
		blk->ty = bo->nxyzCenter.y ;
		blk->tz = bo->nxyzCenter.z ;
		blk->th = 0 ;
		blk->extension = -1 ;
		blk->n_segs = bo->nHazardCount ;
		blk->n_flrs = bo->nFloorCount ;
		blk->n_bul_segs = blk->n_bul_flrs = 0 ;
		blk->n_traps = bo->nTrapCount ;
		blk->block_no = bo->nBlockNumber ;
		blk->segs = NULL ;
		blk->flrs = NULL ;
		blk->bul_segs = NULL ;
		blk->bul_flrs = NULL ;
		blk->traps = NULL ;	
		ConvertTrap( blk, bo ) ;
		ConvertSegment( blk, bo ) ;
		ConvertFloor( blk, bo ) ;
		blk ++ ; bo ++ ;
    }
}

/* ビハインド変換 */
static	void	ConvertBehind( grp, go )
HZD_GRP		*grp ;
GROUPOBJECT	*go ;
{
    int			n_behinds ;
    int			i ;
    HZD_BEHIND		*bh ;
    CAMERAOBJECT	*co ;

    n_behinds = go->nCameraCount ;
    co = go->lpCamera ;
    bh = grp->behinds = ( HZD_BEHIND * )MyAlloc( sizeof( HZD_BEHIND ) * n_behinds ) ;

printf("bhind count = %d\n", n_behinds);
    for ( i = 0; i < n_behinds; i ++ ) {
		bh->b1.vx = co->dxyzBound[ 0 ].x ;
		bh->b1.vy = co->dxyzBound[ 0 ].y ;
		bh->b1.vz = co->dxyzBound[ 0 ].z ;
		bh->b2.vx = co->dxyzBound[ 1 ].x ;
		bh->b2.vy = co->dxyzBound[ 1 ].y ;
		bh->b2.vz = co->dxyzBound[ 1 ].z ;
		bh->v[ 0 ] = co->dxyzTarget.x ;
		bh->v[ 1 ] = co->dxyzTarget.y ;
		bh->v[ 2 ] = co->dxyzTarget.z ;
		bh->v[ 3 ] = co->dxyzCamera.x ;
		bh->v[ 4 ] = co->dxyzCamera.y ;
		bh->v[ 5 ] = co->dxyzCamera.z ;
		bh->right[ 0 ] = co->dxyzRightTarget.x ;
		bh->right[ 1 ] = co->dxyzRightTarget.y ;
		bh->right[ 2 ] = co->dxyzRightTarget.z ;
		bh->right[ 3 ] = co->dxyzRightCamera.x ;
		bh->right[ 4 ] = co->dxyzRightCamera.y ;
		bh->right[ 5 ] = co->dxyzRightCamera.z ;
		bh->left[ 0 ] = co->dxyzLeftTarget.x ;
		bh->left[ 1 ] = co->dxyzLeftTarget.y ;
		bh->left[ 2 ] = co->dxyzLeftTarget.z ;
		bh->left[ 3 ] = co->dxyzLeftCamera.x ;
		bh->left[ 4 ] = co->dxyzLeftCamera.y ;
		bh->left[ 5 ] = co->dxyzLeftCamera.z ;
		bh->pad[ 0 ] = co->nPadData[ 0 ] ;
		bh->pad[ 1 ] = co->nPadData[ 1 ] ;
		if ( bh->pad[ 0 ] != 0 || bh->pad[ 1 ] != 0 ) {
//			printf( "behind %d %d\n", bh->pad[ 0 ], bh->pad[ 1 ] ) ;
		}
		//printf( "[%d]", i ) ;
		//DumpBehind( bh, 1 ) ;
		bh ++ ; co ++ ;
    }
}

/* グローバルゾーン番号からローカル番号への変換テーブル */
static	int	GN2LN[ 65535 ] ;

/* ゾーン番号をローカルに変換するテーブルを作成 */
static	void	MakeRenumberZoneTable( scn )
SCENE2HZXHEADER	*scn ;
{
    int		n_groups, n_zones ;
    GROUPOBJECT	*go ;
    ZONEOBJECT	*zo ;
    int		i, j ;

    go = scn->lpGroup ;
    n_groups = scn->nGroupCount ;
    for ( i = 0; i < n_groups; i ++ ) {
		n_zones = go->nZoneCount ;
		zo = go->lpZone ;
		for ( j = 0; j < n_zones; j ++ ) {
			GN2LN[ zo->nIncremental ] = j ;
			zo ++ ;
		}
		go ++ ;
    }
}

/* 指定ゾーン番号が何番のグループにあるかを返す */
static	int	ZoneWhereGroup( scn, nz )
SCENE2HZXHEADER	*scn ;
int		nz ;
{
    int		n_groups, n_zones ;
    GROUPOBJECT	*go ;
    ZONEOBJECT	*zo ;
    int		i, j ;

    go = scn->lpGroup ;
    n_groups = scn->nGroupCount ;
    for ( i = 0; i < n_groups; i ++ ) {
		n_zones = go->nZoneCount ;
		zo = go->lpZone ;
		for ( j = 0; j < n_zones; j ++ ) {
			if ( nz == zo->nIncremental ) return i ;
			zo ++ ;
		}
		go ++ ;
    }
    return -1 ;
}

/* ゾーン変換 */
static	int		ConvertZone( scn, grp, go, ro, n_grp ) 
SCENE2HZXHEADER	*scn ;
HZD_GRP		*grp ;
GROUPOBJECT	*go ;
ROUTEOBJECT	*ro ;
int		n_grp ; /* グループ番号 */
{
    int		n_zones, n_linkzones ;
    int		n_routes ;
    HZD_ZON	*zones ;
    ZONEOBJECT	*zo ;
    ROUTECOORDOBJECT	*rco ;
    ROUTEPOLYGONOBJECT	*rpo ;
    int		i, j, no, n_nears, nz, ng ; 
    float	flen ;
    FVECTOR	diff ;
    int		link, c ;
    u_char	*link_zone ;


    n_zones = grp->n_zones = go->nZoneCount ;
    zo = go->lpZone ;
    zones = grp->zones = ( HZD_ZON * )MyAlloc( sizeof( HZD_ZON ) * n_zones ) ;
    /* linkzoneは１６個まで */
    link_zone = grp->link_zone = ( u_char * )MyAlloc( sizeof( u_char ) * 2 * 16 ) ; 
    n_linkzones = 0 ;

	if ( n_zones > 255 ) {
		printf( "scn2hzx : Error! too many zones in one group!\n" ) ;
		exit( -1 ) ;
	}

    for ( i = 0; i < n_zones; i ++ ) {
		n_nears = 0 ;
		link = 0 ;
		zones->flag = zo->nFlag ;
#if 0
		{
			int		w, h ;

			w = ( int )( zo->dxyzVertex[ 2 ].x - zo->dxyzVertex[ 0 ].x ) ;
			h = ( int )( zo->dxyzVertex[ 2 ].z - zo->dxyzVertex[ 0 ].z ) ;
			if ( ( w % 2 ) || ( h % 2 ) ) {
				printf( "scn2hzx : ConvertZone Warning!! - a zone width is odd!\n" ) ;
				printf( "\t%.2lf %.2lf - %.2lf %.2lf\n",
					   zo->dxyzVertex[ 0 ].x, zo->dxyzVertex[ 0 ].z,
					   zo->dxyzVertex[ 2 ].x, zo->dxyzVertex[ 2 ].z ) ;				
				N_BadZones ++ ;
				//exit( -1 ) ;
			}
		}

		zo->dxyzVertex[ 2 ].x = ( double )( ( int )zo->dxyzVertex[ 2 ].x / 2 * 2 ) ;
		zo->dxyzVertex[ 2 ].z = ( double )( ( int )zo->dxyzVertex[ 2 ].z / 2 * 2 ) ;
		zo->dxyzVertex[ 0 ].x = ( double )( ( int )zo->dxyzVertex[ 0 ].x / 2 * 2 ) ;
		zo->dxyzVertex[ 0 ].z = ( double )( ( int )zo->dxyzVertex[ 0 ].z / 2 * 2 ) ;
#endif
		if ( ( fabs( zo->dxyzVertex[ 2 ].x - zo->dxyzVertex[ 0 ].x ) / 2.0 > 30000.0 ) ||
			 ( fabs( zo->dxyzVertex[ 2 ].z - zo->dxyzVertex[ 0 ].z ) / 2.0 > 30000.0 ) ) {
			printf( "scn2hzx : ConvertZone Error!! - a zone is too large!\n" ) ;
			printf( "\t%.2lf %.2lf - %.2lf %.2lf\n",
				   zo->dxyzVertex[ 0 ].x, zo->dxyzVertex[ 0 ].z,
				   zo->dxyzVertex[ 2 ].x, zo->dxyzVertex[ 2 ].z ) ;
			exit( -1 ) ;
		}

		zones->w = fabs( zo->dxyzVertex[ 2 ].x - zo->dxyzVertex[ 0 ].x ) / 2.0F ;
		zones->h = fabs( zo->dxyzVertex[ 2 ].z - zo->dxyzVertex[ 0 ].z ) / 2.0F ;
		for ( j = 0; j < 6; j ++ ) {
			zones->nears[ j ] = 255 ;
			zones->near_flag[ j ] = 0 ;
		}
		no = zo->nIncremental ;
		GN2LN[ no ] = i ;
		if ( ro == NULL ) {
			zones->x = ( zo->dxyzVertex[ 2 ].x + zo->dxyzVertex[ 0 ].x ) / 2.0F ;
			zones->y = ( zo->dxyzVertex[ 2 ].y + zo->dxyzVertex[ 0 ].y ) / 2.0F ;
			zones->z = ( zo->dxyzVertex[ 2 ].z + zo->dxyzVertex[ 0 ].z ) / 2.0F ;
			goto convert_zone_skip ;
		}
		rco = ro->lpRouteCoord ;
		for ( j = 0; j < ro->nVertexCount; j ++ ) {
			if ( rco->nIncremental == no ) {
				zones->x = rco->dxyzVertex.x ;
				zones->y = rco->dxyzVertex.y ;
				zones->z = rco->dxyzVertex.z ;
				break ;
			}
			rco ++ ;
		}
		rpo = ro->lpRoutePolygon ;
		for ( j = 0; j < ro->nPolygonCount; j ++ ) {
			c = 0 ;
			if ( rpo->lpRouteCoord[ 0 ]->nIncremental == no ) {
				c = 1 ;
				nz = rpo->lpRouteCoord[ 1 ]->nIncremental ;
				diff.vx = rpo->lpRouteCoord[ 1 ]->dxyzVertex.x - zones->x ;
				diff.vy = rpo->lpRouteCoord[ 1 ]->dxyzVertex.y - zones->y ;
				diff.vz = rpo->lpRouteCoord[ 1 ]->dxyzVertex.z - zones->z ;
				diff.vx *= diff.vx ;
				diff.vy *= diff.vy ;
				diff.vz *= diff.vz ;
				flen = sqrtf( diff.vx + diff.vy + diff.vz ) / 500.0F + 1.0F ;
				if ( flen > 255.0F ) flen = 255.0F ;
				ng = ZoneWhereGroup( scn, nz ) ;
				if ( n_grp == ng ) {
					if ( n_nears == 5 && link ) {
						printf( "near zone over\n" ) ;
						return -1 ;
					}
					if ( n_nears == 6 ) {
						printf( "near zone over\n" ) ;
						return -1 ;
					}
					zones->nears[ n_nears ] = GN2LN[ nz ] ;
					zones->dists[ n_nears ] = ( u_char )flen ;

					zones->near_flag[ n_nears ] = rpo->nZoneToZoneFlag ;

					n_nears ++ ;
				} else {
					if ( n_nears == 5 ) {
						printf( "no space for link zone\n" ) ;
						return -1 ;
					}
					if ( link ) printf( "this is already link zone set\n" ) ;
					if ( link == 0 ) {
						if ( n_linkzones == 16 ) {
							printf( "link zone over\n" ) ;
							return -1 ;
						}
						link_zone[ 0 ] = n_grp ;
						link_zone[ 1 ] = i ;
						link_zone += 2 ;
						n_linkzones ++ ;
					}
					link = 1 ;
					zones->nears[ 5 ] = GN2LN[ nz ] ;
					zones->dists[ 5 ] = ( u_char )flen ;
					zones->near_flag[ 5 ] = ng ;
					zones->flag |= HZD_ZON_LINK ;
					printf( "link zone found %d %d %d %d\n", n_grp, i, GN2LN[ nz ], ng ) ;
				}
			} else if ( rpo->lpRouteCoord[ 1 ]->nIncremental == no ) {
				c = 1 ;
				nz = rpo->lpRouteCoord[ 0 ]->nIncremental ;
				diff.vx = rpo->lpRouteCoord[ 0 ]->dxyzVertex.x - zones->x ;
				diff.vy = rpo->lpRouteCoord[ 0 ]->dxyzVertex.y - zones->y ;
				diff.vz = rpo->lpRouteCoord[ 0 ]->dxyzVertex.z - zones->z ;
				diff.vx *= diff.vx ;
				diff.vy *= diff.vy ;
				diff.vz *= diff.vz ;
				flen = sqrtf( diff.vx + diff.vy + diff.vz ) / 500.0F + 1.0F ;
				if ( flen > 255.0F ) flen = 255.0F ;
				ng = ZoneWhereGroup( scn, nz ) ;
				if ( n_grp == ng ) {
					if ( n_nears == 5 && link ) {
						printf( "near zone over\n" ) ;
						return -1 ;
					}
					if ( n_nears == 6 ) {
						printf( "near zone over\n" ) ; 
						return -1 ;
					}
					zones->nears[ n_nears ] = GN2LN[ nz ] ;
					zones->dists[ n_nears ] = ( u_char )flen ;

					zones->near_flag[ n_nears ] = rpo->nZoneToZoneFlag ;

					n_nears ++ ;
				} else {
					if ( n_nears == 5 ) {
						printf( "no space for link zone\n" ) ;
						return -1 ;
					}
					if ( link ) printf( "this is already link zone set\n" ) ;
					if ( link == 0 ) {
						if ( n_linkzones == 16 ) {
							printf( "link zone over\n" ) ;
							return -1 ;
						}
						printf( "link zone found %d %d %d %d\n", n_grp, i, GN2LN[ nz ], ng ) ;
						link_zone[ 0 ] = n_grp ;
						link_zone[ 1 ] = i ;
						link_zone += 2 ;
						n_linkzones ++ ;
					}
					link = 1 ;
					zones->nears[ 5 ] = GN2LN[ nz ] ;
					zones->dists[ 5 ] = ( u_char )flen ;
					zones->near_flag[ 5 ] = ng ;
					zones->flag |= HZD_ZON_LINK ;
				}
			} 
			rpo ++ ;
		}
convert_zone_skip :
        //printf( "%d g %x z %d nears %x\n", n_grp, i, n_nears, zones->flag ) ;
		zones ++ ; zo ++ ;
    }
	printf( "%d group has %d linkzones \n", n_grp, n_linkzones ) ;
    grp->n_link_zones = n_linkzones ;
	return 0 ;
}

/* グループの変換 */
static	int		ConvertGroup( scn, def )
SCENE2HZXHEADER	*scn ;
HZD_DEF		*def ;
{
    int		n_groups ;
    GROUPOBJECT	*go ;
    HZD_GRP	*grp ;
    int		i, j ;

    MakeRenumberZoneTable( scn ) ;
    n_groups = def->n_groups = scn->nGroupCount ;
    grp = def->groups = ( HZD_GRP * )MyAlloc( sizeof( HZD_GRP ) * n_groups ) ;
    go = scn->lpGroup ;

	N_BadZones = 0 ;

    for ( i = 0; i < n_groups; i ++ ) {
printf( "-------- group %d/%d\n", i, scn->nGroupCount ) ;
		grp->n_behinds = go->nCameraCount ;
		grp->n_zones = go->nZoneCount ;
		grp->n_link_zones = 0 ;
		grp->behinds = NULL ;
		grp->zones = NULL ;
		grp->link_zone = NULL ;
		grp->bound_min_x = go->nxyzBoundMin.x ;
		grp->bound_min_y = go->nxyzBoundMin.y ;
		grp->bound_min_z = go->nxyzBoundMin.z ;
		grp->block_size_x = go->nxyzSize.x ;
		grp->block_size_y = go->nxyzSize.y ;
		grp->block_size_z = go->nxyzSize.z ;
		grp->div_x = go->nxyzDivideReCalc.x ;
		grp->div_y = go->nxyzDivideReCalc.y ;
		grp->div_z = go->nxyzDivideReCalc.z ;
		grp->n_blocks = go->nBlockCount ;
		grp->n_x_blocks = go->nBlockCount ;
		grp->blocks = NULL ;
		grp->dynamics = NULL ;
printf("convert bihind, in Group-%d\n", i);
		ConvertBehind( grp, go ) ;
		if ( ConvertZone( scn, grp, go, scn->lpRoute, i ) < 0 ) return -1 ;
		ConvertBlock( grp, go ) ;
		grp ++ ; go ++ ;
    }

	printf( "%d BadZone Exists!!\n", N_BadZones ) ;

	return 0 ;
}

/*-------------------------------------------------------------------------*/

/* 敵データの変換 */

/*-------------------------------------------------------------------------*/

/* 巡回ルートの変換 */
static	void	ConvertPatrol( scn, def ) 
SCENE2HZXHEADER	*scn ;
HZD_DEF			*def ;
{
    CHARACTERROUTEOBJECT	*cro ;
    CHARACTERCOORDOBJECT	*cco ;
    HZD_PAT			*pat ;
    HZD_PTP			*ptp ;
    int				n_points, n_patrols ;
    int				i, j ;

    n_patrols = def->n_patrols = scn->nCharacterRouteCount ;    
    n_points = 0 ;
    cro = scn->lpCharacterRoute ;
    for ( i = 0; i < n_patrols; i ++ ) {
		n_points += cro->nCoordCount ;
		cro ++ ;
    }
    cro = scn->lpCharacterRoute ;
    pat = def->patrols = ( HZD_PAT * )MyAlloc( sizeof( HZD_PAT ) * n_patrols ) ;
    ptp = def->points = ( HZD_PTP * )MyAlloc( sizeof( HZD_PTP ) * n_points ) ;
    for ( i = 0; i < n_patrols; i ++ ) {
		cco = cro->lpCharacterCoord ;
		pat->n_points = cro->nCoordCount ;
		pat->pad = cro->nPad ;
		pat->flag = cro->nFlag ;
		pat->points = ptp ;
		for ( j = 0; j < pat->n_points; j ++ ) {
			ptp->x = cco->dxyzVertex.x ;
			ptp->z = cco->dxyzVertex.z ;
			ptp->y = cco->dxyzVertex.y ;
			ptp->ax = cco->dxyzTarget.x ;
			ptp->az = cco->dxyzTarget.z ;
			ptp->ay = cco->dxyzTarget.y ;
			ptp->act = cco->nAction ;
			ptp->time = cco->nTime ;
			ptp->dir = cco->nDirection ;
			ptp->pad = cco->nPad ;
			ptp->flag = cco->nFlag ;
			ptp ++ ; cco ++ ;
		}
		pat ++ ; cro ++ ;
    }

    /* VRオフセット値の格納 */
	for( i = 0; i < HZX_VR_CHARACTER_GROUP_MAX; i++) {
	    def->vr_pat_offset[i] = scn->arynVRCharacterOffset[i];
printf("vr patrols offset[%d] = %d\n", i, def->vr_pat_offset[i]);
	}
}

/* クリアリングデータ変換 */
static	void	ConvertClearing( scn, def )
SCENE2HZXHEADER	*scn ;
HZD_DEF			*def ;
{
    int			n_cle_areas, n_cle_roots, n_cle_points ;
    CLEARINGAREAOBJECT	*cao ;
    CLEARINGROUTEOBJECT	*cro ;
    CLEARINGCOORDOBJECT	*cco ;
    HZD_CLE_AREA	*area ;
    HZD_CLE_ROOT	*root ;
    HZD_CLE_PTP		*ptp ;
    int			i, j, k ;

    n_cle_areas = def->n_clears = scn->nClearingAreaCount ;
    cao = scn->lpClearingArea ;
    n_cle_roots = n_cle_points = 0 ;
    for ( i = 0; i < n_cle_areas; i ++ ) {
		n_cle_roots += cao->nRouteCount ;
		cro = cao->lpClearingRoute ;
		for ( j = 0; j < cao->nRouteCount; j ++ ) {
			n_cle_points += cro->nPoints ;
			cro ++ ;
		}
		cao ++ ;
    }
    area = def->cle_areas = ( HZD_CLE_AREA * )MyAlloc( sizeof( HZD_CLE_AREA ) * n_cle_areas ) ;
    root = def->cle_roots = ( HZD_CLE_ROOT * )MyAlloc( sizeof( HZD_CLE_ROOT ) * n_cle_roots ) ;
    ptp = def->cle_points = ( HZD_CLE_PTP * )MyAlloc( sizeof( HZD_CLE_PTP ) * n_cle_points ) ;
    cao = scn->lpClearingArea ;
    for ( i = 0; i < n_cle_areas; i ++ ) {
		area->b1.vx = cao->dxyzBound[ 0 ].x ;
		area->b1.vy = cao->dxyzBound[ 0 ].y ;
		area->b1.vz = cao->dxyzBound[ 0 ].z ;
		area->b2.vx = cao->dxyzBound[ 1 ].x ;
		area->b2.vy = cao->dxyzBound[ 1 ].y ;
		area->b2.vz = cao->dxyzBound[ 1 ].z ;
		area->name = GetStrCode( cao->szName ) ;
		area->go_time = cao->nTime ;
		area->n_root = cao->nRouteCount ;
		area->roots = root ;
		cro = cao->lpClearingRoute ;
		for ( j = 0; j < area->n_root; j ++ ) {
			root->n_points = cro->nPoints ;
			root->pad = cro->nPad ;
			root->points = ptp ;
			cco = cro->lpClearingCoord ;
			for ( k = 0; k < root->n_points; k ++ ) {
				ptp->x = cco->dxyzVertex.x ;
				ptp->z = cco->dxyzVertex.z ;
				ptp->y = cco->dxyzVertex.y ;
				ptp->ax = cco->dxyzTarget.x ;
				ptp->az = cco->dxyzTarget.z ;
				ptp->ay = cco->dxyzTarget.y ;
				ptp->act = cco->nAction ;
				ptp->time = cco->nTime ;
				ptp->dir = cco->nDirection ;
				ptp->pad = cco->nCondition ;
				ptp ++ ; cco ++ ;
			}
			root ++ ; cro ++ ;
		}
		area ++ ; cao ++ ;
    }

    /* VRオフセット値の格納 */
	for( i = 0; i < HZX_VR_CLEARING_GROUP_MAX; i++) {
		def->vr_clr_offset[i] = scn->arynVRClearingOffset[i];
printf("vr clearing offset[%d] = %d\n", i, def->vr_clr_offset[i]);
	}
}

/* 敵データ変換 */
static	void	ConvertEnemyData( scn, def )
SCENE2HZXHEADER	*scn ;
HZD_DEF			*def ;
{
    /* 巡回データ変換 */
    ConvertPatrol( scn, def ) ;
    /* クリアリングデータ変換 */
    ConvertClearing( scn, def ) ;
}

/* 位置からマップを求める */
static	int	Pos2Map( def, pos ) 
HZD_DEF		*def ;
FVECTOR		*pos ;
{
    int		n_groups, n_zones ;
    int		i, group ;
    HZD_ZON	*zon ;
    HZD_GRP	*grp ;
    float	minlen, len ;

    grp = def->groups ;
    n_groups = def->n_groups ;
    minlen = 1000000.0F ;
    group = -1 ;
    for ( i = 0; i < n_groups; i ++ ) {
		if ( SearchGroupZone( pos, grp, &len ) >= 0 ) {
			if ( minlen > len ) {
				minlen = len ;
				group = i ;
			}
		}
		grp ++ ;
    }
	if ( group >= 0 ) return group ;
    grp = def->groups ;
    n_groups = def->n_groups ;
    minlen = 1000000.0F ;
    for ( i = 0; i < n_groups; i ++ ) {
		if ( NearestGroupZone( pos, grp, &len ) >= 0 ) {
			if ( minlen > len ) {
				minlen = len ;
				group = i ;
			}
		}
		grp ++ ;
    }
    if ( group < 0 ) printf( "warning : point to group / not found group\n" ) ;
    return group ;
}

/* 敵ポイントにマップ設定 */
static	void	SetMapToPoint( def )
HZD_DEF		*def ;
{
    HZD_PTP	*ptp ;
    HZD_PAT	*pat ;
    int		n_patrols, n_points ;
    FVECTOR	pos ;
    HZD_CLE_AREA	*area ;
    HZD_CLE_ROOT	*root ;
    HZD_CLE_PTP		*cptp ;
    int		i, j, k ;
    int		n_cle_areas, n_cle_roots, n_cle_points ;

    n_patrols = def->n_patrols ;
    pat = def->patrols ;
    while( -- n_patrols >= 0 ) {
		ptp = pat->points ;
		n_points = pat->n_points ;
		while( -- n_points >= 0 ) {
			pos.vx = ptp->x ;
			pos.vy = ptp->y ;
			pos.vz = ptp->z ;
			ptp->group_id = Pos2Map( def, &pos ) ;
			if ( ptp->group_id < 0 ) {
				printf( "%d ptp of %d pat\n", 
					   pat->n_points - n_points + 1,
					   def->n_patrols - n_patrols + 1 ) ;
			}
			ptp ++ ;
		}
		pat ++ ;
    }
    n_cle_areas = def->n_clears ;
    area = def->cle_areas ;
    for ( i = 0; i < n_cle_areas; i ++ ) {
		root = area->roots ;
		for ( j = 0; j < area->n_root; j ++ ) {
			cptp = root->points ;
			for ( k = 0; k < root->n_points; k ++ ) {
				pos.vx = cptp->x ;
				pos.vy = cptp->y ;
				pos.vz = cptp->z ;
				cptp->group_id = Pos2Map( def, &pos ) ;		
				if ( cptp->group_id < 0 ) {
					printf( "%d ptp of %d cle_root of %d cle_area\n", k, j, i ) ;
				}
				cptp ++ ; 
			}
			root ++ ; 
		}
		area ++ ; 
    }    
}

/* Scn -> Hzx */
HZD_DEF		*Scn2Hzx( scn )
SCENE2HZXHEADER	*scn ;
{
    HZD_DEF	*def ;

	//    printf( "Converting Scn -> Hzx ..." ) ;
    def = ( HZD_DEF * )MyAlloc( sizeof( HZD_DEF ) ) ;
    if ( def == NULL ) goto hzx_conv_error ;
    memset( def, 0x00, sizeof( HZD_DEF ) ) ;
    def->version = HZX_VERSION ;
    /* 敵兵データの変換 */
    ConvertEnemyData( scn, def ) ;
    /* グループデータ変換 */
    if ( ConvertGroup( scn, def ) < 0 ) goto hzx_conv_error ;
    /* 巡回ポイントにマップ設定 */
    SetMapToPoint( def ) ;
#ifndef OLD
	printf( "makeroute\n" ) ;
    /* ルート情報作成 */
    HZD_MakeRoute( def ) ;
	printf( "makesafezone\n" ) ;
    /* 安全ゾーン設定 */
    HZD_SafeZoneMake( def ) ;
	printf( "make online info\n" ) ;
    HZD_MakeOnlineInfo( def ) ;
#endif
	//    printf( "End.\n" ) ;
    return def ;
hzx_conv_error :
    printf( "error\n" ) ;
    return NULL ;
}

/*--------------------------------------------------------------------------*/

static	u_int	N_Points, HeadAddr ;
static	u_int	BlockAddr, TrapAddr, BehindAddr ;
static	u_int	ZoneAddr, PatAddr, GroupAddr, PtpAddr ;
static	u_int	CleAreaAddr, CleRootAddr, ClePointAddr, N_CleRoots, N_ClePoints ;
static	u_int	VuSegAddr, StripAddr, LinkZoneAddr ;
static	u_int	RouteAddr, OinfoAddr, RouteCourseAddr ;

/* 最初にファイルサイズを計算 */
static	int	CountHzx2Size( def )
HZD_DEF		*def ;
{
    int		size, n ;

    size = sizeof( HZD_DEF ) ;
    size = ( size + 15 ) / 16 * 16 ;
    /* 敵関係 */
    PatAddr = size ;
    size += sizeof( HZD_PAT ) * def->n_patrols ;
    size = ( size + 15 ) / 16 * 16 ;
    CleAreaAddr = size ;
    size += sizeof( HZD_CLE_AREA ) * def->n_clears ;
    size = ( size + 15 ) / 16 * 16 ;
    /* グループ */
    GroupAddr = size ;
    size += sizeof( HZD_GRP ) * def->n_groups ;
    size = ( size + 15 ) / 16 * 16 ;
    /* 巡回ポイント */
    PtpAddr = size ;
    {
		HZD_PAT		*pat ;
		int		n_points ;

		pat = def->patrols ;
		n = def->n_patrols ;
		n_points = 0 ;
		while( -- n >= 0 ) {
			n_points += pat->n_points ;
			pat ++ ;
		}
		N_Points = n_points ;
		size += sizeof( HZD_PTP ) * n_points ;
		size = ( size + 15 ) / 16 * 16 ;
    }
    /* クリアリングデータ */
    CleRootAddr = size ;
    {
		int	n_areas, n_roots, n_points, tmp ;
		HZD_CLE_AREA	*area ;
		HZD_CLE_ROOT	*root ;

		n_areas = def->n_clears ;
		n_roots = n_points = 0 ;
		area = def->cle_areas ;
		while( -- n_areas >= 0 ) {
			n_roots += area->n_root ;
			tmp = area->n_root ;	
			root = area->roots ;
			while( -- tmp >= 0 ) {
				n_points += root->n_points ;
				root ++ ;
			}
			area ++ ;
		}
		N_CleRoots = n_roots ;
		N_ClePoints = n_points ;	
		size += sizeof( HZD_CLE_ROOT ) * n_roots ;
		size = ( size + 15 ) / 16 * 16 ;
		ClePointAddr = size ;
		size += sizeof( HZD_CLE_PTP ) * n_points ;
		size = ( size + 15 ) / 16 * 16 ;
    }
    /* ビハインドデータ */
    BehindAddr = size ;
    {
		HZD_GRP		*grp ;
		int		n_groups ;
	
		n_groups = def->n_groups ;
		grp = def->groups ;
		while( -- n_groups >= 0 ) {
			size += sizeof( HZD_BEHIND ) * grp->n_behinds ;
			grp ++ ;
		}
    }
    /* ゾーンデータ */
    ZoneAddr = size ;
    {
		HZD_GRP		*grp ;
		int		n_groups ;
	
		n_groups = def->n_groups ;
		grp = def->groups ;
		while( -- n_groups >= 0 ) {
			size += sizeof( HZD_ZON ) * grp->n_zones ;
			grp ++ ;
		}
		size = ( size + 15 ) / 16 * 16 ;
    }
#ifndef OLD
    /* ルートデータ */
    RouteAddr = size ;
    {	
		HZD_GRP		*grp ;
		int		n_groups, n_zones, route_size ;

		n_groups = def->n_groups ;
		grp = def->groups ;
		while( -- n_groups >= 0 ) {
			n_zones = grp->n_zones ;
			if ( n_zones > 1 ) {
				route_size = ( ( n_zones - 1 ) * ( n_zones - 2 ) ) / 2 
					+ ( n_zones - 1 ) ;
				size += route_size ;
				size = ( size + 15 ) / 16 * 16 ;
			}
			grp ++ ;
		}
		size = ( size + 15 ) / 16 * 16 ;
    }

    /* オンライン情報データ */
    OinfoAddr = size ;
    {	
		HZD_GRP		*grp ;
		int		n_groups, n_zones, isize ;

		n_groups = def->n_groups ;
		grp = def->groups ;
		while( -- n_groups >= 0 ) {
			n_zones = grp->n_zones ;
			if ( n_zones > 1 ) {
				isize = ( ( ( n_zones * n_zones - n_zones ) / 2 ) + 
						 ( sizeof( u_int ) * 8 - 1 ) ) / ( sizeof( u_int ) * 8 ) ;
				isize *= sizeof( u_int ) ;
				size += isize ;
			}
			grp ++ ;
		}
		size = ( size + 15 ) / 16 * 16 ;
    }
#ifdef ROUTE_COURSE_TEST
    /* ルートコースデータ */
    RouteCourseAddr = size ;
    {	
		HZD_GRP		*grp ;
		int		n_groups, n_zones, route_size ;

		n_groups = def->n_groups ;
		grp = def->groups ;

		while( -- n_groups >= 0 ) {
			n_zones = grp->n_zones ;
			if ( n_zones > 1 ) {
				route_size = ( ( n_zones - 1 ) * ( n_zones - 2 ) ) / 2 
					+ ( n_zones - 1 ) ;
				size += sizeof( u_short ) * route_size ;
				size = ( size + 15 ) / 16 * 16 ;
				if ( grp->courses & HZX_ROOT_COURSE1 ) {
					size += sizeof( u_short ) * route_size ;
					size = ( size + 15 ) / 16 * 16 ;
				}
				if ( grp->courses & HZX_ROOT_COURSE2 ) {
					size += sizeof( u_short ) * route_size ;
					size = ( size + 15 ) / 16 * 16 ;
				}
				if ( grp->courses == (HZX_ROOT_COURSE1|HZX_ROOT_COURSE2) ) {
					size += sizeof( u_short ) * route_size ;
					size = ( size + 15 ) / 16 * 16 ;
				}
			}
			grp ++ ;
		}
		size = ( size + 15 ) / 16 * 16 ;
    }
#endif
#endif
    /* リンクゾーン */
    LinkZoneAddr = size ;
    {
		HZD_GRP		*grp ;
		int		n_groups ;
	
		n_groups = def->n_groups ;
		grp = def->groups ;
		while( -- n_groups >= 0 ) {
			size += sizeof( u_char ) * 2 * grp->n_link_zones ;
			grp ++ ;
		}
		size = ( size + 15 ) / 16 * 16 ;
    }

    /* ブロックデータ */
    BlockAddr = size ;
    {
		HZD_GRP		*grp ;
		int		n_groups ;
	
		n_groups = def->n_groups ;
		grp = def->groups ;
		while( -- n_groups >= 0 ) {
			size += sizeof( HZD_BLOCK ) * grp->n_blocks ;
			grp ++ ;
		}
    }
    /* 壁床データ */
    VuSegAddr = size ;
    {
		HZD_GRP		*grp ;
		HZD_BLOCK	*blk ;
		int		n_groups, n_blocks ;
	
		n_groups = def->n_groups ;
		grp = def->groups ;
		while( -- n_groups >= 0 ) {
			n_blocks = grp->n_blocks ;
			blk = grp->blocks ;
			while( -- n_blocks >= 0 ) {
				size += sizeof( HZD_VuSEG ) * ( blk->n_segs + blk->n_flrs +
											   blk->n_bul_segs + blk->n_bul_flrs ) ;
				blk ++ ;
			}
			grp ++ ;
		}	
    }
    size = ( size + 15 ) / 16 * 16 ;
    /* トラップデータ */
    TrapAddr = size ;
    {
		HZD_GRP		*grp ;
		HZD_BLOCK	*blk ;
		int		n_groups, n_blocks ;
	
		n_groups = def->n_groups ;
		grp = def->groups ;
		while( -- n_groups >= 0 ) {
			n_blocks = grp->n_blocks ;
			blk = grp->blocks ;
			while( -- n_blocks >= 0 ) {
				size += sizeof( HZD_TRP ) * blk->n_traps ;
				blk ++ ;
			}
			grp ++ ;
		}	
    }    
    size = ( size + 15 ) / 16 * 16 ;
    /* ストリップデータ */
    StripAddr = size ;
    {
		HZD_GRP		*grp ;
		HZD_BLOCK	*blk ;
		HZD_VuSEG	*seg ;
		int		n_groups, n_blocks, n_segs ;
	
		n_groups = def->n_groups ;
		grp = def->groups ;
		while( -- n_groups >= 0 ) {
			n_blocks = grp->n_blocks ;
			blk = grp->blocks ;
			while( -- n_blocks >= 0 ) {
				seg = blk->segs ;
				n_segs = blk->n_segs ;
				while( -- n_segs >= 0 ) {
					size += sizeof( SVECTOR ) * seg->b1.vw ;
					size = ( size + 15 ) / 16 * 16 ;
					seg ++ ;
				}
				seg = blk->flrs ;
				n_segs = blk->n_flrs ;
				while( -- n_segs >= 0 ) {
					size += sizeof( SVECTOR ) * seg->b1.vw ;
					size = ( size + 15 ) / 16 * 16 ;
					seg ++ ;
				}
				seg = blk->bul_segs ;
				n_segs = blk->n_bul_segs ;
				while( -- n_segs >= 0 ) {
					size += sizeof( SVECTOR ) * seg->b1.vw ;
					size = ( size + 15 ) / 16 * 16 ;
					seg ++ ;
				}
				seg = blk->bul_flrs ;
				n_segs = blk->n_bul_flrs ;
				while( -- n_segs >= 0 ) {
					size += sizeof( SVECTOR ) * seg->b1.vw ;
					size = ( size + 15 ) / 16 * 16 ;
					seg ++ ;
				}
				blk ++ ;
			}
			grp ++ ;
		}	
    }        
    size = ( size + 15 ) / 16 * 16 ;
    return size ;
}

/* ヘッダ書き込み */
static	void	WriteHzx2Header( def2, def )
HZD_DEF		*def2, *def ;
{
    *def2 = *def ;
    def2->patrols = ( HZD_PAT * )PatAddr ;
    def2->points = ( HZD_PTP * )PtpAddr ;
    def2->cle_areas = ( HZD_CLE_AREA * )CleAreaAddr ;
    def2->cle_roots = ( HZD_CLE_ROOT * )CleRootAddr ;
    def2->cle_points = ( HZD_CLE_PTP * )ClePointAddr ;
    def2->groups = ( HZD_GRP * )GroupAddr ;
}

/* 巡回データ書き込み */
static	void	WriteHzx2Patrol( def2, def ) 
HZD_DEF		*def2, *def ;
{
    HZD_PAT	*pat, *pat2 ;
    int		n, points ;

    n = def->n_patrols ;
    pat = def->patrols ;
    pat2 = ( HZD_PAT * )( HeadAddr + ( u_int )def2->patrols ) ;	
    points = ( u_int )def2->points ;
    while( -- n >= 0 ) {
		*pat2 = *( pat ++ ) ;
		pat2->points = ( HZD_PTP * )points ;
		points += pat2->n_points * sizeof( HZD_PTP ) ;
		pat2 ++ ;
    }
}

/* 巡回ポイントデータ書き込み */
static	void	WriteHzx2Point( def2, def ) 
HZD_DEF		*def2, *def ;
{
    HZD_PTP	*ptp, *ptp2 ;
    int		n ;

    n = N_Points ;
    ptp = def->points ;
    ptp2 = ( HZD_PTP * )( HeadAddr + ( u_int )def2->points ) ;
    while( -- n >= 0 ) *( ptp2 ++ ) = *( ptp ++ ) ;
}

/* クリアリングデータ書き込み */
static	void	WriteHzx2Clearing( def2, def )
HZD_DEF		*def2, *def ;
{
    HZD_CLE_AREA	*a1, *a2 ;
    HZD_CLE_ROOT	*r1, *r2 ;
    HZD_CLE_PTP		*p1, *p2 ;
    u_int		aadr, radr, padr ;
    int			n_areas, n_roots, n_points ;

    aadr = CleAreaAddr ;
    radr = CleRootAddr ;
    padr = ClePointAddr ;

    a1 = def->cle_areas ;
    r1 = def->cle_roots ;
    p1 = def->cle_points ;
    a2 = ( HZD_CLE_AREA * )( HeadAddr + aadr ) ; 
    r2 = ( HZD_CLE_ROOT * )( HeadAddr + radr ) ; 
    p2 = ( HZD_CLE_PTP * )( HeadAddr + padr ) ; 
    
    n_areas = def->n_clears ;
    while( -- n_areas >= 0 ) {
		*a2 = *a1 ;
		a2->roots = ( HZD_CLE_ROOT * )radr ;
		radr += sizeof( HZD_CLE_ROOT ) * a1->n_root ;
		a1 ++ ; a2 ++ ;
    }
    n_roots = N_CleRoots ;
    while( -- n_roots >= 0 ) {
		*r2 = *r1 ;
		r2->points = ( HZD_CLE_PTP * )padr ;
		padr += sizeof( HZD_CLE_PTP ) * r1->n_points ;	
		r1 ++ ; r2 ++ ;
    }
    n_points = N_ClePoints ;
    while( -- n_points >= 0 ) {
		*( p2 ++ ) = *( p1 ++ ) ;
    }
}

/* グループデータ書き込み */
static	void	WriteHzx2Group( def2, def )
HZD_DEF		*def2, *def ;
{
    HZD_GRP	*grp, *grp2 ;
    int		n, cur, bhd, zne, lz, rut, oi, cour ;
    int		n_zones, isize ;

    n = def->n_groups ;
    grp = def->groups ;
    grp2 = ( HZD_GRP * )( HeadAddr + ( u_int )( def2->groups ) ) ;

    cur = BlockAddr ;
    bhd = BehindAddr ;
    zne = ZoneAddr ;
    rut = RouteAddr ;
#ifdef ROUTE_COURSE_TEST
	cour = RouteCourseAddr ;
#endif
    lz = LinkZoneAddr ;
    oi = OinfoAddr ;
    while( -- n >= 0 ) {
		*grp2 = *grp ;
		grp2->blocks = ( HZD_BLOCK * )cur ;
		cur += sizeof( HZD_BLOCK ) * grp->n_blocks ;
		grp2->behinds = ( HZD_BEHIND * )bhd ;
		bhd += sizeof( HZD_BEHIND ) * grp->n_behinds ;
		grp2->zones = ( HZD_ZON * )zne ;
		zne += sizeof( HZD_ZON ) * grp->n_zones ;
#ifndef OLD
		n_zones = grp->n_zones ;

		if ( n_zones > 1 ) {
			grp2->route = ( u_char * )rut ;
			rut += ( ( n_zones - 1 ) * ( n_zones - 2 ) ) / 2 +
				( n_zones - 1 ) ;
			rut = ( rut + 15 ) / 16 * 16 ;
		} else {
			grp2->route = NULL ;
		}

		n_zones = grp->n_zones ;
		if ( n_zones > 1 ) {
			grp2->online_info = ( u_int * )oi ;
			isize = ( ( ( n_zones * n_zones - n_zones ) / 2 ) + 
					 ( sizeof( u_int ) * 8 - 1 ) ) / ( sizeof( u_int ) * 8 ) ;
			isize *= sizeof( u_int ) ;
			oi += isize ;
		} else {
			grp2->online_info = NULL ;
		}

#ifdef ROUTE_COURSE_TEST
		n_zones = grp->n_zones ;
		grp2->courses = grp->courses ;
		if ( n_zones > 1 ) {
			grp2->route_course[0] = ( u_short * )cour ;
			isize = ( ( n_zones - 1 ) * ( n_zones - 2 ) ) / 2 +
				( n_zones - 1 ) ;
			isize *= sizeof( u_short ) ;

			cour += isize ;
			cour = ( cour + 15 ) / 16 * 16 ;
			if ( grp2->courses & HZX_ROOT_COURSE1 ) {
				grp2->route_course[1] = ( u_short * )cour ;
				cour += isize ;
				cour = ( cour + 15 ) / 16 * 16 ;
			} else {
				grp2->route_course[1] = grp2->route_course[0] ;
			}
			if ( grp2->courses & HZX_ROOT_COURSE2 ) {
				grp2->route_course[2] = ( u_short * )cour ;
				cour += isize ;
				cour = ( cour + 15 ) / 16 * 16 ;
			} else {
				grp2->route_course[2] = grp2->route_course[0] ;
			}
			if ( grp2->courses == (HZX_ROOT_COURSE1|HZX_ROOT_COURSE2) ) {
				grp2->route_course[3] = ( u_short * )cour ;
				cour += isize ;
				cour = ( cour + 15 ) / 16 * 16 ;
			} else {
				if ( grp2->courses & HZX_ROOT_COURSE1 ) {
					grp2->route_course[3] = grp2->route_course[1] ;
				} else if ( grp2->courses & HZX_ROOT_COURSE2 ) {
					grp2->route_course[3] = grp2->route_course[2] ;
				} else {
					grp2->route_course[3] = grp2->route_course[0] ;
				}
			}
		} else {
			grp2->route = NULL ;
		}
#endif
#endif
		grp2->link_zone = ( u_char * )lz ;
		lz += sizeof( u_char ) * 2 * grp->n_link_zones ;
		grp ++ ; grp2 ++ ;
    }
}

/* ビハインド書き込み */
static	void	WriteHzx2Behind( def2, def )
HZD_DEF		*def2, *def ;
{
    HZD_GRP	*grp ;
    HZD_BEHIND	*bhd, *bhd2 ;
    int		n, n_behinds ;

    n = def->n_groups ;
    grp = def->groups ;
    bhd2 = ( HZD_BEHIND * )( BehindAddr + HeadAddr ) ;
    while( -- n >= 0 ) {    
		bhd = grp->behinds ;
		n_behinds = grp->n_behinds ;
		while( -- n_behinds >= 0 ) *( bhd2 ++ ) = *( bhd ++ ) ;
		grp ++ ;
    }
}

/* ゾーンデータ書き込み */
static	void	WriteHzx2Zone( def2, def ) 
HZD_DEF		*def2, *def ;
{
    HZD_GRP	*grp ;
    HZD_ZON	*zon, *zon2 ;
    u_char	*lz, *lz2 ;
    u_char	*rut, *rut2 ;
    u_int	*oi, *oi2 ;
    int		n, n_zones, size, isize ;
    u_short	*cour, *cour2 ;

    n = def->n_groups ;
    grp = def->groups ;
    zon2 = ( HZD_ZON * )( ZoneAddr + HeadAddr ) ;
    while( -- n >= 0 ) {    
		zon = grp->zones ;
		n_zones = grp->n_zones ;
		while( -- n_zones >= 0 ) *( zon2 ++ ) = *( zon ++ ) ;
		grp ++ ;
    }
#ifndef OLD
    n = def->n_groups ;
    grp = def->groups ;
    rut2 = ( u_char * )( RouteAddr + HeadAddr ) ;
    while( -- n >= 0 ) {    
		rut = grp->route ;
		n_zones = grp->n_zones ;
		if ( n_zones > 1 ) {
			size = ( ( n_zones - 1 ) * ( n_zones - 2 ) ) / 2 
				+ ( n_zones - 1 ) ;
			memcpy( rut2, rut, size ) ;
			size = ( size + 15 ) / 16 * 16 ;
			rut2 += size ;
		}
		grp ++ ;
    }

    n = def->n_groups ;
    grp = def->groups ;
    oi2 = ( u_int * )( OinfoAddr + HeadAddr ) ;
    while( -- n >= 0 ) {    
		oi = grp->online_info ;
		n_zones = grp->n_zones ;
		if ( n_zones > 1 ) {
			isize = ( ( ( n_zones * n_zones - n_zones ) / 2 ) + 
					 ( sizeof( u_int ) * 8 - 1 ) ) / ( sizeof( u_int ) * 8 ) ;
			isize *= sizeof( u_int ) ;
			memcpy( oi2, oi, isize ) ;
			oi2 += isize / sizeof( u_int ) ;
		}
		grp ++ ;
    }

#ifdef ROUTE_COURSE_TEST
	/* courses */
    n = def->n_groups ;
    grp = def->groups ;
	cour2 = ( u_short * )( RouteCourseAddr + HeadAddr ) ;
    while( -- n >= 0 ) {
		n_zones = grp->n_zones ;
		if ( n_zones > 1 ) {
			int size2 ;

			cour = grp->route_course[0] ;
			size = ( ( n_zones - 1 ) * ( n_zones - 2 ) ) / 2 
				+ ( n_zones - 1 ) ;
			size2 = (size *= sizeof( u_short )) ;
			memcpy( cour2, cour, size2 ) ;
			size = ( size + 15 ) / 16 * 16 ;
			cour2 += (size/2) ;
			if ( grp->courses & HZX_ROOT_COURSE1 ) {
				cour = grp->route_course[1] ;
				memcpy( cour2, cour, size2 ) ;
				cour2 += (size/2) ;
			}
			if ( grp->courses & HZX_ROOT_COURSE2 ) {
				cour = grp->route_course[2] ;
				memcpy( cour2, cour, size2 ) ;
				cour2 += (size/2) ;
			}
			if ( grp->courses == (HZX_ROOT_COURSE1|HZX_ROOT_COURSE2) ) {
				cour = grp->route_course[3] ;
				memcpy( cour2, cour, size2 ) ;
				cour2 += (size/2) ;
			}
		}
		grp ++ ;
    }
#endif





#endif    
    n = def->n_groups ;
    grp = def->groups ;
    lz2 = ( u_char * )( LinkZoneAddr + HeadAddr ) ;
    while( -- n >= 0 ) {    
		lz = grp->link_zone ;
		n_zones = grp->n_link_zones ;
		while( -- n_zones >= 0 ) {
			*( lz2 ++ ) = *( lz ++ ) ;
			*( lz2 ++ ) = *( lz ++ ) ;
		}
		grp ++ ;
    }
}

/* ブロックデータ書き込み */
static	void	WriteHzx2Block( def2, def )
HZD_DEF		*def2, *def ;
{
    HZD_GRP	*grp ;
    HZD_BLOCK	*blk, *blk2 ;
    int		n_groups, n_blocks ;
    int		segaddr, trpaddr ;

    grp = def->groups ;
    n_groups = def->n_groups ;
    blk2 = ( HZD_BLOCK * )( BlockAddr + HeadAddr ) ;
    segaddr = VuSegAddr ;
    trpaddr = TrapAddr ;
    while( -- n_groups >= 0 ) {
		blk = grp->blocks ;
		n_blocks = grp->n_blocks ;
		while( -- n_blocks >= 0 ) {
			*blk2 = *blk ;
			if ( blk->n_segs > 0 ) {
				blk2->segs = ( HZD_VuSEG * )segaddr ;
				segaddr += sizeof( HZD_VuSEG ) * blk->n_segs ;
			} else {
				blk2->segs = NULL ;		
			}
			if ( blk->n_flrs > 0 ) {
				blk2->flrs = ( HZD_VuSEG * )segaddr ;	    
				segaddr += sizeof( HZD_VuSEG ) * blk->n_flrs ;
			} else {
				blk2->flrs = NULL ;
			}
			if ( blk->n_bul_segs > 0 ) {
				blk2->bul_segs = ( HZD_VuSEG * )segaddr ;
				segaddr += sizeof( HZD_VuSEG ) * blk->n_bul_segs ;
			} else {
				blk2->bul_segs = NULL ;		
			}
			if ( blk->n_bul_flrs > 0 ) {
				blk2->bul_flrs = ( HZD_VuSEG * )segaddr ;	    
				segaddr += sizeof( HZD_VuSEG ) * blk->n_bul_flrs ;
			} else {
				blk2->bul_flrs = NULL ;
			}
			if ( blk->n_traps > 0 ) {
				blk2->traps = ( HZD_TRP * )trpaddr ;
				trpaddr += sizeof( HZD_TRP ) * blk->n_traps ; 
			} else {
				blk2->traps = NULL ;
			}
			blk ++ ; blk2 ++ ;
		}
		grp ++ ;
    }
}

/* 壁床書き込み */
static	void	WriteHzx2VuSeg( def2, def )
HZD_DEF		*def2, *def ;
{
    HZD_GRP	*grp ;
    HZD_BLOCK	*blk ;
    HZD_VuSEG	*seg, *seg2 ;
    int		n_groups, n_blocks, n_segs ;
    int	       	stripaddr ;

    grp = def->groups ;
    n_groups = def->n_groups ;
    seg2 = ( HZD_VuSEG * )( VuSegAddr + HeadAddr ) ;
    stripaddr = StripAddr ;
    while( -- n_groups >= 0 ) {
		blk = grp->blocks ;
		n_blocks = grp->n_blocks ;
		while( -- n_blocks >= 0 ) {
			n_segs = blk->n_segs ;
			seg = blk->segs ;
			while( -- n_segs >= 0 ) {
				*seg2 = *seg ;
				seg2->verts = ( SVECTOR * )stripaddr ;
				stripaddr += sizeof( SVECTOR ) * seg->b1.vw ;
				stripaddr = ( stripaddr + 15 ) / 16 * 16 ;
				seg ++ ; seg2 ++ ;
			}
			n_segs = blk->n_flrs ;
			seg = blk->flrs ;
			while( -- n_segs >= 0 ) {
				*seg2 = *seg ;
				seg2->verts = ( SVECTOR * )stripaddr ;
				stripaddr += sizeof( SVECTOR ) * seg->b1.vw ;
				stripaddr = ( stripaddr + 15 ) / 16 * 16 ;
				seg ++ ; seg2 ++ ;
			}
			n_segs = blk->n_bul_segs ;
			seg = blk->bul_segs ;
			while( -- n_segs >= 0 ) {
				*seg2 = *seg ;
				seg2->verts = ( SVECTOR * )stripaddr ;
				stripaddr += sizeof( SVECTOR ) * seg->b1.vw ;
				stripaddr = ( stripaddr + 15 ) / 16 * 16 ;
				seg ++ ; seg2 ++ ;
			}
			n_segs = blk->n_bul_flrs ;
			seg = blk->bul_flrs ;
			while( -- n_segs >= 0 ) {
				*seg2 = *seg ;
				seg2->verts = ( SVECTOR * )stripaddr ;
				stripaddr += sizeof( SVECTOR ) * seg->b1.vw ;
				stripaddr = ( stripaddr + 15 ) / 16 * 16 ;
				seg ++ ; seg2 ++ ;
			}
			blk ++ ;
		}
		grp ++ ;
    }
}

/* トラップ書き込み */
static	void	WriteHzx2Trap( def2, def )
HZD_DEF		*def2, *def ;
{
    HZD_GRP	*grp ;
    HZD_BLOCK	*blk ;
    HZD_TRP	*trp, *trp2 ;
    int		n_groups, n_blocks, n_traps ;

    grp = def->groups ;
    n_groups = def->n_groups ;
    trp2 = ( HZD_TRP * )( TrapAddr + HeadAddr ) ;
    while( -- n_groups >= 0 ) {
		blk = grp->blocks ;
		n_blocks = grp->n_blocks ;
		while( -- n_blocks >= 0 ) {
			n_traps = blk->n_traps ;
			trp = blk->traps ;
			while( -- n_traps >= 0 ) {
				*trp2 = *trp ;
				trp ++ ; trp2 ++ ;
			}
			blk ++ ;
		}
		grp ++ ;
    }
}

/* ストリップ書き込み */
static	void	WriteHzx2Strip( def2, def )
HZD_DEF		*def2, *def ;
{
    HZD_GRP	*grp ;
    HZD_BLOCK	*blk ;
    HZD_VuSEG	*seg ;
    SVECTOR	*v, *v2 ;
    int		n_groups, n_blocks, n_segs, n_points, diff ;

    grp = def->groups ;
    n_groups = def->n_groups ;
    v2 = ( SVECTOR * )( StripAddr + HeadAddr ) ;
    while( -- n_groups >= 0 ) {
		blk = grp->blocks ;
		n_blocks = grp->n_blocks ;
		while( -- n_blocks >= 0 ) {
			n_segs = blk->n_segs ;
			seg = blk->segs ;
			while( -- n_segs >= 0 ) {
				n_points = seg->b1.vw ;
				v = seg->verts ;
				while( -- n_points >= 0 ) {
					*( v2 ++ ) = *( v ++ ) ;
				}
				diff = ( u_int )v2 - HeadAddr ;
				diff = ( diff + 15 ) / 16 * 16 ;
				v2 = ( SVECTOR * )( diff + HeadAddr ) ;
				seg ++ ; 
			}
			n_segs = blk->n_flrs ;
			seg = blk->flrs ;
			while( -- n_segs >= 0 ) {
				n_points = seg->b1.vw ;
				v = seg->verts ;
				while( -- n_points >= 0 ) {
					*( v2 ++ ) = *( v ++ ) ;
				}
				diff = ( u_int )v2 - HeadAddr ;
				diff = ( diff + 15 ) / 16 * 16 ;
				v2 = ( SVECTOR * )( diff + HeadAddr ) ;
				seg ++ ; 
			}
			n_segs = blk->n_bul_segs ;
			seg = blk->bul_segs ;
			while( -- n_segs >= 0 ) {
				n_points = seg->b1.vw ;
				v = seg->verts ;
				while( -- n_points >= 0 ) {
					*( v2 ++ ) = *( v ++ ) ;
				}
				diff = ( u_int )v2 - HeadAddr ;
				diff = ( diff + 15 ) / 16 * 16 ;
				v2 = ( SVECTOR * )( diff + HeadAddr ) ;
				seg ++ ; 
			}
			n_segs = blk->n_bul_flrs ;
			seg = blk->bul_flrs ;
			while( -- n_segs >= 0 ) {
				n_points = seg->b1.vw ;
				v = seg->verts ;
				while( -- n_points >= 0 ) {
					*( v2 ++ ) = *( v ++ ) ;
				}
				diff = ( u_int )v2 - HeadAddr ;
				diff = ( diff + 15 ) / 16 * 16 ;
				v2 = ( SVECTOR * )( diff + HeadAddr ) ;
				seg ++ ; 
			}
			blk ++ ;
		}
		grp ++ ;
    }
}

/* Hzxセーブ */
int		SaveHzx( def, name )
HZD_DEF		*def ;
char		*name ;
{
    FILE	*fp ;
    char	str[ 256 ] ;
    char	*ext ;
    HZD_DEF	*def2 ;
    int		size ;

    size = CountHzx2Size( def ) ;
	//    printf( "size = %d\n", size ) ;
    def2 = ( HZD_DEF * )MyAlloc( size ) ;
    if ( def2 == NULL ) goto hzx2_save_error ;
    HeadAddr = ( u_int )def2 ;
    WriteHzx2Header( def2, def ) ;
    WriteHzx2Patrol( def2, def ) ;
    WriteHzx2Point( def2, def ) ;
    WriteHzx2Clearing( def2, def ) ;
    WriteHzx2Group( def2, def ) ;
    WriteHzx2Behind( def2, def ) ;
    WriteHzx2Zone( def2, def ) ;
    WriteHzx2Block( def2, def ) ;
    WriteHzx2VuSeg( def2, def ) ;
    WriteHzx2Trap( def2, def ) ;
    WriteHzx2Strip( def2, def ) ;

    strcpy( str, name ) ;
    printf( "Save %s\n", str ) ;
    fp = fopen( str, "wb" ) ;
    if ( fp == NULL ) goto hzx2_save_error ;
    fwrite( def2, size, 1, fp ) ;
    fclose( fp ) ;
    MyFree( def2 ) ;
    return 0 ;
hzx2_save_error :
    if ( def2 != NULL ) MyFree( def2 ) ;
    return -1 ;
}

/*--------------------------------------------------------------------------*/

/* Ｈｚｘロード */
HZD_DEF		*LoadHzx( name )
char		*name ;
{
    FILE	*fp ;
    HZD_DEF	*def ;
    int		size, head, m, n, i, j ;
    
    fp = fopen( name, "rb" ) ;
    if ( fp == NULL ) return NULL ;
    fseek( fp, 0, SEEK_END ) ;
    size = ftell( fp ) ;
	//    printf( "size %d\n", size ) ;
    fseek( fp, 0, SEEK_SET ) ;
    def = ( HZD_DEF * )MyAlloc( size ) ;
    if ( def == NULL ) return NULL ;
    head = ( u_int )def ;
    fread( def, size, 1, fp ) ;
    fclose( fp ) ;
    
    def->patrols = ( HZD_PAT * )( head + ( u_int )def->patrols ) ;
    def->points = ( HZD_PTP * )( head + ( u_int )def->points ) ;
    def->cle_areas = ( HZD_CLE_AREA * )( head + ( u_int )def->cle_areas ) ;    
    def->cle_roots = ( HZD_CLE_ROOT * )( head + ( u_int )def->cle_roots ) ;    
    def->cle_points = ( HZD_CLE_PTP * )( head + ( u_int )def->cle_points ) ;    
    def->groups = ( HZD_GRP * )( head + ( u_int )def->groups ) ;
    {
		HZD_PAT		*pat ;

		n = def->n_patrols ;
		pat = def->patrols ;
		while( -- n >= 0 ) {
			pat->points = ( HZD_PTP * )( head + ( u_int )pat->points ) ; 
			pat ++ ;
		}	
	
    }
    {
		HZD_CLE_AREA	*area ;
		HZD_CLE_ROOT	*root ;

		n = def->n_clears ;
		area = def->cle_areas ;
		while( -- n >= 0 ) {
			area->roots = ( HZD_CLE_ROOT * )( head + ( u_int )area->roots ) ; 
			m = area->n_root ;
			root = area->roots ;
			while( -- m >= 0 ) {
				root->points = ( HZD_CLE_PTP * )( head + ( u_int )root->points ) ;
				root ++ ;
			}
			area ++ ;
		}
    }
    {
		HZD_GRP		*grp ;
		HZD_BLOCK	*blk ;
		HZD_VuSEG	*seg ;

		i = def->n_groups ;
		grp = def->groups ;
		while( -- i >= 0 ) {
			if ( grp->n_behinds > 0 ) 
				grp->behinds = ( HZD_BEHIND * )( head + ( u_int )grp->behinds ) ;
			if ( grp->n_zones > 0 ) 
				grp->zones = ( HZD_ZON * )( head + ( u_int )grp->zones ) ;
#ifndef OLD
			if ( grp->n_zones > 1 ) 
				grp->route = ( u_char * )( head + ( u_int )grp->route ) ;
			if ( grp->n_zones > 1 ) 
				grp->online_info = ( u_int * )( head + ( u_int )grp->online_info ) ;
#endif
			if ( grp->n_link_zones > 0 ) 
				grp->link_zone = ( u_char * )( head + ( u_int )grp->link_zone ) ;		
			if ( grp->n_blocks > 0 ) 
				grp->blocks = ( HZD_BLOCK * )( head + ( u_int )grp->blocks ) ;
			j = grp->n_blocks ;
			blk = grp->blocks ;
			while( -- j >= 0 ) {
				if ( blk->n_segs > 0 ) 
					blk->segs = ( HZD_VuSEG * )( head + ( u_int )blk->segs ) ;
				else	
					blk->segs = NULL ;
				if ( blk->n_flrs > 0 ) 
					blk->flrs = ( HZD_VuSEG * )( head + ( u_int )blk->flrs ) ;
				else	
					blk->flrs = NULL ;
				if ( blk->n_bul_segs > 0 ) 
					blk->bul_segs = ( HZD_VuSEG * )( head + ( u_int )blk->bul_segs ) ;
				else	
					blk->bul_segs = NULL ;
				if ( blk->n_bul_flrs > 0 ) 
					blk->bul_flrs = ( HZD_VuSEG * )( head + ( u_int )blk->bul_flrs ) ;
				else	
					blk->bul_flrs = NULL ;
				if ( blk->n_traps > 0 ) {
					blk->traps = ( HZD_TRP * )( head + ( u_int )blk->traps ) ;
				} else	
					blk->traps = NULL ;
				n = blk->n_segs ;
				seg = blk->segs ;
				while( -- n >= 0 ) {
					seg->verts = ( SVECTOR * )( head + ( u_int )seg->verts ) ;
					seg ++ ;
				}
				n = blk->n_flrs ;
				seg = blk->flrs ;
				while( -- n >= 0 ) {
					seg->verts = ( SVECTOR * )( head + ( u_int )seg->verts ) ;
					seg ++ ;
				}
				n = blk->n_bul_segs ;
				seg = blk->bul_segs ;
				while( -- n >= 0 ) {
					seg->verts = ( SVECTOR * )( head + ( u_int )seg->verts ) ;
					seg ++ ;
				}
				n = blk->n_bul_flrs ;
				seg = blk->bul_flrs ;
				while( -- n >= 0 ) {
					seg->verts = ( SVECTOR * )( head + ( u_int )seg->verts ) ;
					seg ++ ;
				}
				blk ++ ;
			}
			grp ++ ;
		}
    }
    return def ;
}
