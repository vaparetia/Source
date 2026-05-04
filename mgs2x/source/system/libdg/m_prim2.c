/*
   m_prim2.c

   メニュープリミティブ２

   2000/04/28 M.Sonoyama 
   $Id: m_prim2.c,v 1.1.1.3 2002/11/19 11:42:11 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>

#include	"gameheader.h"
#include	"def_dma.h"
#include	"plugin.h"

/*------------------------------------------------------------*/

sceGifTag	DG_GIFTAG_MENU2_LINE = {
	0, 1, 0, 0, 0, 0, SCE_GIF_REGLIST, 5, 
	GS_REGS_RGBA, GS_REGS_UV, GS_REGS_XYZF2, GS_REGS_UV, GS_REGS_XYZF2
} ;

sceGifTag	DG_GIFTAG_MENU2_LINE_G = {
	0, 1, 0, 0, 0, 0, SCE_GIF_REGLIST, 6, 
	GS_REGS_RGBA, GS_REGS_UV, GS_REGS_XYZF2, 
	GS_REGS_RGBA, GS_REGS_UV, GS_REGS_XYZF2
} ;

sceGifTag	DG_GIFTAG_MENU2_POLY = {
	0, 1, 0, 0, 0, 0, SCE_GIF_REGLIST, 10, 
	GS_REGS_PRIM, GS_REGS_RGBA, 
	GS_REGS_UV, GS_REGS_XYZF2, GS_REGS_UV, GS_REGS_XYZF2, 
	GS_REGS_UV, GS_REGS_XYZF2, GS_REGS_UV, GS_REGS_XYZF2
} ;

sceGifTag	DG_GIFTAG_MENU2_POLY_G = {
	0, 1, 0, 0, 0, 0, SCE_GIF_REGLIST, 13, 
	GS_REGS_PRIM,
	GS_REGS_RGBA, GS_REGS_UV, GS_REGS_XYZF2, 
	GS_REGS_RGBA, GS_REGS_UV, GS_REGS_XYZF2, 
	GS_REGS_RGBA, GS_REGS_UV, GS_REGS_XYZF2, 
	GS_REGS_RGBA, GS_REGS_UV, GS_REGS_XYZF2
} ;

sceGifTag	DG_GIFTAG_MENU2_POLY3 = {
	0, 1, 0, 0, 0, 0, SCE_GIF_REGLIST, 8, 
	GS_REGS_PRIM, GS_REGS_RGBA, 
	GS_REGS_UV, GS_REGS_XYZF2, GS_REGS_UV, GS_REGS_XYZF2, 
	GS_REGS_UV, GS_REGS_XYZF2
} ;

sceGifTag	DG_GIFTAG_MENU2_POLY3_G = {
	0, 1, 0, 0, 0, 0, SCE_GIF_REGLIST, 10, 
	GS_REGS_PRIM,
	GS_REGS_RGBA, GS_REGS_UV, GS_REGS_XYZF2, 
	GS_REGS_RGBA, GS_REGS_UV, GS_REGS_XYZF2, 
	GS_REGS_RGBA, GS_REGS_UV, GS_REGS_XYZF2, 
} ;

sceGifTag	DG_GIFTAG_MENU2_POINT = {
	0, 1, 0, 0, 0, 0, SCE_GIF_REGLIST, 2,
	GS_REGS_RGBA, GS_REGS_XYZF2
} ;

/*---------------------------------------------------------------*/

/*------------------------------------------------------------*/

static	int			ALIGN128( size )
int					size ;
{
	return ( size + sizeof( u_long128 ) - 1 ) / sizeof( u_long128 ) * sizeof( u_long128 ) ;
}

/*------------------------------------------------------------*/

inline	void	DG_InvisibleMenu2Prim( prim )
DG_MENU2_PRIM			*prim ;
{
	prim->dmapack.flag |= DG_DMAPACK_INVISIBLEMENU ;
}

inline	void	DG_VisibleMenu2Prim( prim )
DG_MENU2_PRIM			*prim ;
{
	prim->dmapack.flag &= ~DG_DMAPACK_INVISIBLEMENU ;
}

void 	DG_FreeMenu2Prim( prim )
DG_MENU2_PRIM			*prim ;
{
	DG_DequeueDmapack( &prim->dmapack ) ;
	GV_DelayedFree( prim->packet[ 0 ] ) ;
	GV_DelayedFree( prim ) ;
}

DG_MENU2_PRIM		*DG_MakeMenu2Prim( type, n_prims, prio )
int					type, n_prims, prio ;
{
	DG_MENU2_PRIM	*prim ;
	DG_MENU2_PACKET	*pack ;
	DG_DMAPACK		*dmapack ;
	DG_DMATAG		*tag ;
	sceGifTag		*gtag ;
	sceGsPrim		gprim, *gprim2 ;
	void			*buf ;
	int				shape ;
	int				i, j, size, packet_size, tmp ;
	int				next ;

	prim = GV_Malloc( sizeof( DG_MENU2_PRIM ) ) ;
	if ( prim == NULL ) return NULL ;

	dmapack = &prim->dmapack ;
	dmapack->flag = DG_DMAPACK_MENU ;
	dmapack->phase = DG_DMAPACK_PHASE_AFTER ;
	dmapack->priority = 240 ;
	DG_QueueDmapack( dmapack ) ;
	
	prim->flag = type ;
	prim->n_prims = n_prims ;
	prim->priority = prio ;
	shape = type & DG_MENU2_PRIM_TYPEMASK ;

	next = 0 ;
	switch( shape ) {
	case DG_MENU2_PRIM_LINE :
	case DG_MENU2_PRIM_LINESTRIP :
		if ( type & DG_PRIM2_SHADE ) {
			tmp = sizeof( DG_MENU2_LINE_G ) * n_prims ;
		} else {
			tmp = sizeof( DG_MENU2_LINE ) * n_prims ;
		}
		break ;
	case DG_MENU2_PRIM_POLY :
		if ( type & DG_PRIM2_SHADE ) {
			tmp = sizeof( DG_MENU2_POLY_G ) * n_prims ;
			next = sizeof( DG_MENU2_POLY_G ) ;
		} else {
			tmp = sizeof( DG_MENU2_POLY ) * n_prims ;
			next = sizeof( DG_MENU2_POLY ) ;
		}
		break ;
	case DG_MENU2_PRIM_POLY3 :
		if ( type & DG_PRIM2_SHADE ) {
			tmp = sizeof( DG_MENU2_POLY3_G ) * n_prims ;
			next = sizeof( DG_MENU2_POLY3_G ) ;
		} else {
			tmp = sizeof( DG_MENU2_POLY3 ) * n_prims ;
			next = sizeof( DG_MENU2_POLY3 ) ;
		}
		break ;
	case DG_MENU2_PRIM_SPRT :
		tmp = sizeof( DG_MENU2_SPRT ) * n_prims ;
		break ;
	case DG_MENU2_PRIM_POINT :
		tmp = sizeof( DG_MENU2_POINT ) * n_prims ; 
		break ;
	default :
		return NULL ;
	}

	packet_size = ALIGN128( tmp ) ;
	size = packet_size + sizeof( u_long128 ) * 6 ;

	buf = GV_Malloc( size * 2 ) ;
	if ( buf == NULL ) return ( NULL );
	GV_ZeroMemory( buf, size * 2 ) ;
	dmapack->packet[ 0 ] = buf ;
	dmapack->packet[ 1 ] = buf + size ;
	prim->packet[ 0 ] = buf ;
	prim->packet[ 1 ] = buf + size ;

	GV_ZeroMemory( &gprim, sizeof( sceGsPrim ) ) ;

	for ( i = 0; i < 2; i ++ ) {
		pack = ( DG_MENU2_PACKET * )dmapack->packet[ i ] ;
		tag = &pack->setuptag ;
		tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ) ;
		tag->addr = NULL ;
		tag->vifcode[ 0 ] = SCE_VIF1_SET_NOP( 0 ) ;
		tag->vifcode[ 1 ] = SCE_VIF1_SET_NOP( 0 ) ;
		tag = &pack->dmatag ;
		tag->qwc = DMATAG_SET_QWC( DMATAG_ID_RET, packet_size / sizeof( u_long128 ) + 4 ) ;
		tag->vifcode[ 0 ] = SCE_VIF1_SET_NOP( 0 ) ;
		tag->vifcode[ 1 ] = SCE_VIF1_SET_DIRECT( packet_size / sizeof( u_long128 ) + 4, 0 ) ;
		DG_SET_GIFTAG( &pack->primtag, .FLG = SCE_GIF_PACKED, .NREG = 1, .NLOOP = 2,
					   .REGS0 = GS_REGS_AD, .EOP = 0 ) ;
		pack->prim.reg = SCE_GS_PRIM ;
		/* アルファモードはデフォルト加算 */
		pack->alpha.reg = SCE_GS_ALPHA_1 ;
		pack->alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 ) ; 

		gtag = &pack->giftag ;
		switch( shape ) {
		case DG_MENU2_PRIM_LINE :
		case DG_MENU2_PRIM_LINESTRIP :
			if ( shape == DG_MENU2_PRIM_LINE ) gprim.PRIM = SCE_GS_PRIM_LINE ;
			else							   gprim.PRIM = SCE_GS_PRIM_LINESTRIP ;
			if ( type & DG_PRIM2_SHADE ) {
				*gtag = DG_GIFTAG_MENU2_LINE_G ;
				gprim.IIP = 1 ;
			} else {
				*gtag = DG_GIFTAG_MENU2_LINE ;
			}
			break ;
		case DG_MENU2_PRIM_POLY :
			gprim.PRIM = SCE_GS_PRIM_TRISTRIP ;
			if ( type & DG_PRIM2_SHADE ) {
				*gtag = DG_GIFTAG_MENU2_POLY_G ;
				gprim.IIP = 1 ;
			} else {
				*gtag = DG_GIFTAG_MENU2_POLY ;
			}
			break ;
		case DG_MENU2_PRIM_POLY3 :
			gprim.PRIM = SCE_GS_PRIM_TRI ;
			if ( type & DG_PRIM2_SHADE ) {
				*gtag = DG_GIFTAG_MENU2_POLY3_G ;
				gprim.IIP = 1 ;
			} else {
				*gtag = DG_GIFTAG_MENU2_POLY3 ;
			}
			break ;
		case DG_MENU2_PRIM_SPRT :
			gprim.PRIM = SCE_GS_PRIM_SPRITE ;
			*gtag = DG_GIFTAG_MENU2_SPRT ;
			break ;
		case DG_MENU2_PRIM_POINT :
			gprim.PRIM = SCE_GS_PRIM_POINT ;
			*gtag = DG_GIFTAG_MENU2_POINT ;
		}

		gtag->NLOOP = n_prims ;
		if ( type & DG_PRIM2_TEX ) {
			gprim.TME = 1 ;
			gprim.FST = 1 ;
		}
		if ( type & DG_PRIM2_ALPHA ) gprim.ABE = 1 ;
		if ( type & DG_PRIM2_ANTIALIASING ) gprim.AA1 = 1 ;

		pack->prim.data = SCE_GS_SET_PRIM( gprim.PRIM, gprim.IIP, gprim.TME, 0,
										   gprim.ABE, gprim.AA1, gprim.FST, 0, 0 ) ;

		if ( shape != DG_MENU2_PRIM_LINE &&
			 shape != DG_MENU2_PRIM_SPRT &&
			 shape != DG_MENU2_PRIM_POINT ) {
			void	*ptr ;

			ptr = pack->data ;
			for ( j = 0; j < n_prims; j ++ ) {
				gprim2 = ( sceGsPrim * )ptr ;
				*gprim2 = gprim ;
				ptr += next ;
			}
		}
	}
	return prim ;
}

/*------------------------------------------------------------*/

