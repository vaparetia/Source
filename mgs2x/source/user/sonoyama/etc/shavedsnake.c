//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   shavedsnake.c
   髭剃りスネークテクスチャ入れ替え処理

   2001/08/01	M.Sonoyama
   $Id: shavedsnake.c,v 1.1.1.3 2002/11/19 11:50:45 Yoshizawa1 Exp $
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"

static	int		NormalTexs[] = {
	0x7152e3,		/* sna_face_ss01dt.bmp */
	0x6fb16d 		/* sna_face_ss_ovl_sub_alp.bmp */
} ;

static	int		SingleHighTexs[] = {
	0x8d5b4b, /* sna_hi_face01dt.bmp */
	0xd7cd75  /* sna_hi_face_ovl_sub_alp.bmp */
} ;

static	int		MultiWeightTexs[] = {
	0x8d5b4b, /* sna_hi_face01dt.bmp */
	0xd7cd75  /* sna_hi_face_ovl_sub_alp.bmp */	
} ;

static	int		NormalSTexs[] = {
	0x7af846, /* sna_face_shaved_ss.bmp */
	0x49b700  /* sna_shaved_ss_ovl_sub_alp.bmp */
} ;

static	int		SingleHighSTexs[] = {
	0x8ef1bc, /* sna_hi_face_shave.bmp */
	0xef8699  /* sna_face_shaved_ovl_sub_alp.bmp */
} ;

static	int		MultiWeightSTexs[] = {
	0x8ef1bc, /* sna_hi_face_shave.bmp */
	0xef8699  /* sna_face_shaved_ovl_sub_alp.bmp */
} ;

typedef	struct	{
	GV_ACT_EX			actor ;
	int					n_trs ;
	int					n_tri ;
	int					tri[ 16 ] ;
	int					*src[ 16 ] ;
	void				*tex_replace[ 16 ] ;
} Work ;

static	void	Die( Work *work )
{
	int			i ;

	for ( i = 0; i < work->n_trs; i ++ ) {
		DG_ResetMoveReplaceTexture( work->tex_replace[ i ] ) ;
		DG_FreeMoveReplacePacket( work->tex_replace[ i ] ) ;
	}
}

static	void	MakeTexReplace( Work *work, int tricode, int *src, int *dst, int n_texs )
{
	int			i ;

	for ( i = 0; i < work->n_tri; i ++ ) {
		if ( tricode == work->tri[ i ] && 
			 src == work->src[ i ] ) return ;
	}
	work->tri[ i ] = tricode ;
	work->src[ i ] = src ;
	work->n_tri ++ ;

	for ( i = 0; i < n_texs; i ++ ) {
		work->tex_replace[ work->n_trs ] = DG_MakeMoveReplacePacket( tricode, src[ i ], dst[ i ] ) ;
		if ( work->tex_replace[ work->n_trs ] != NULL ) {
			DG_SetMoveReplaceTexture( work->tex_replace[ work->n_trs ] ) ;
			work->n_trs ++ ;
			printf( "shaved tex replace : %x : %x -> %x\n",
				     tricode, src[ i ], dst[ i ] ) ;
		}
	}
}

void	*NewShavedSnakeTexReplaceActorScn( int name, int where )
{
	Work		*work ;
	DG_DEF		*def ;
	EVM_DEF		*evmdef ;

	work = ( Work * )GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work == NULL ) return NULL ;
	GV_SetActor( &work->actor, NULL, Die ) ;

	def = ( DG_DEF * )GV_GetCache( GV_CacheID( GV_StrCode( "sna_def" ), 'k' ) ) ;
	if ( def != NULL ) {
		MakeTexReplace( work, def->texture, NormalTexs, NormalSTexs, 2 ) ;
	}
	def = ( DG_DEF * )GV_GetCache( GV_CacheID( GV_StrCode( "sna_def_sh" ), 'k' ) ) ;
	if ( def != NULL ) {
		MakeTexReplace( work, def->texture, SingleHighTexs, SingleHighSTexs, 2 ) ;
	}
	evmdef = ( EVM_DEF * )GV_GetCache( GV_CacheID( GV_StrCode( "sna_def_mh" ), 'e' ) ) ;
	if ( evmdef != NULL ) {
		MakeTexReplace( work, evmdef->texture, MultiWeightTexs, MultiWeightSTexs, 2 ) ;
	}
	evmdef = ( EVM_DEF * )GV_GetCache( GV_CacheID( GV_StrCode( "sna_def_addhand_mh_mt" ), 'e' ) ) ;
	if ( evmdef != NULL ) {
		MakeTexReplace( work, evmdef->texture, MultiWeightTexs, MultiWeightSTexs, 2 ) ;
	}
	
	if ( work->n_trs > 0 ) return work ;
	GV_DestroyActor( work ) ;
	return ( void * )1 ;
}
