//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   wig.c
   各種カツラ

   2001/07/31	M.Sonoyama
   $Id: wig.c,v 1.1.1.3 2002/11/19 11:50:17 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include "BP_Renderer.h"

#include	"gameheader.h"

extern	void *NewEvmHairModel_called(int name,int model_name,int sample_num,
						  OBJECT *target,int objnum,FVECTOR *x,SVECTOR *r,
						  float oval_param,int collision_flag,unsigned char *collision_objs,
						  int visible_flag,int light_flag,int boundmodel_name) ;

typedef	struct	{
	GV_ACT_EX				actor ;
	int						name ;
	int						sample ;
	void					*hair ;
	void					*texreplace ;
} Work ;

#define	TRI_CODE		(3271380)		/* rai_tex_mw.tri */
#define	TRI_CODE_MIRROR	(5668626)		/* rai_shadow.tri */

static	int		MugenTexs[] = {
	0xcfcd5b,		/* rai_hair_bk_base.bmp */
	0xa0519e,		/* rai_hair_bk_edge2_ovl_alp.bmp */
	0xcff7fa,		/* rai_hair_bk_edge_ovl_alp.bmp */
	0x268c0,		/* rai_hair_bk_front_ovl_alp.bmp */
	0xb36926,		/* rai_hair_bk_fside_ovl_alp.bmp */
	0xa26a5c,		/* rai_hair_bk_sbotom_ovl_alp.bmp */
} ;

static	int		BlueTexs[] = {
	0xf7fc98,		/* rai_hair_b_base.bmp */
	0x455786,		/* rai_hair_b_edge2_ovl_alp.bmp */
	0xd202a,		/* rai_hair_b_edge_ovl_alp.bmp */
	0xa76ea7,		/* rai_hair_b_front_ovl_alp.bmp */
	0x586f0e,		/* rai_hair_b_fside_ovl_alp.bmp */
	0x432751,		/* rai_hair_b_sbotom_ovl_alp.bmp */
} ;

static	int		RedTexs[] = {
	0xf7fcb8,		/* rai_hair_r_base.bmp */
	0x45578a,		/* rai_hair_r_edge2_ovl_alp.bmp */
	0x2d202a,		/* rai_hair_r_edge_ovl_alp.bmp */
	0xa76eab,		/* rai_hair_r_front_ovl_alp.bmp */
	0x586f12,		/* rai_hair_r_fside_ovl_alp.bmp */
	0x4327d1,		/* rai_hair_r_sbotom_ovl_alp.bmp */
} ;

static	int		NormalMirrorTexs[] = {
	0xf7fc9c,		/* rai_hair_d_base */
	0x11202a		/* rai_hair_d_edge_ovl_alp */
} ;

static	int		MugenMirrorTexs[] = {
	0xcfcd5b,		/* rai_hair_bk_base */
	0xcff7fa		/* rai_hair_bk_edge_ovl_alp */
} ;

static	int		BlueMirrorTexs[] = {
	0xf7fc98,		/* rai_hair_b_base */
	0xd202a			/* rai_hair_b_edge_ovl_alp */
} ;

static	int		RedMirrorTexs[] = {
	0xf7fcb8,		/* rai_hair_r_base */
	0x2d202a 		/* rai_hair_r_edge_ovl_alp */
} ;

/*----------------------------------------------------------------*/

/* テクスチャ差し替えアクター */

typedef	struct	wig_TR_Work
{
	GV_ACT_EX				actor ;
#if 0 //BP_PS2
	DG_TEX_MOVEREPLACE		*tex_replace[ 8 ] ;
	DG_TEX_MOVEREPLACE		*tex_replace_mirror[ 4 ] ;
#else
   unsigned int bp_original_tex[8];
   unsigned int bp_original_mirror_tex[4];
#endif
} TR_Work ;

static	TR_Work		*TexReplaceWork ;

static	void	TR_Act( TR_Work *work )
{
}

static	void	TR_Die( TR_Work *work )
{
	int			i ;

   int			*src = MugenTexs;
   int         *src2 = NormalMirrorTexs;

	for ( i = 0; i < 6; i ++ ) 
   {
#if 0 //BP_PS2
		if ( work->tex_replace[ i ] != NULL ) 
      {
			DG_ResetMoveReplaceTexture( work->tex_replace[ i ] ) ;
			DG_FreeMoveReplacePacket( work->tex_replace[ i ] ) ;
		}
#else
      // Restore original textures using the backups we made when this object was created.
      DG_TEX* pDest = DG_GetTexture2(TRI_CODE, src[i]);

      BP_ReplaceTexture(work->bp_original_tex[i], pDest->BP_TextureHandle);
      BP_FreeDynamicTexture(work->bp_original_tex[i]);
      work->bp_original_tex[i] = 0;

#endif
	}
	
   for ( i = 0; i < 2; i ++ ) {
#if 0 //BP_PS2
		if ( work->tex_replace_mirror[ i ] != NULL ) {
			DG_ResetMoveReplaceTexture( work->tex_replace_mirror[ i ] ) ;
			DG_FreeMoveReplacePacket( work->tex_replace_mirror[ i ] ) ;
		}
#else
      // Restore original textures using the backups we made when this object was created.
      DG_TEX* pDest = DG_GetTexture2(TRI_CODE, src2[i]);

      BP_ReplaceTexture(work->bp_original_mirror_tex[i], pDest->BP_TextureHandle);
      BP_FreeDynamicTexture(work->bp_original_mirror_tex[i]);
      work->bp_original_mirror_tex[i] = 0;
#endif
	}

	TexReplaceWork = NULL ;
}

static	void	*NewWigTexReplaceActor( int this )
{
	TR_Work		*work ;	
	int			i, *src, *dst = NULL ;
	int			*src2, *dst2 = NULL ;

	/* 複数起動しない */
	if ( TexReplaceWork != NULL ) {
		GV_DestroyOtherActorQuick( TexReplaceWork ) ;
	}
	TexReplaceWork = NULL ;

	work = ( TR_Work * )GV_CreateActor( GV_ACTOR_USER, GV_CLASS_OBJECT,
									    sizeof( TR_Work ), 0 ) ;
	if ( work == NULL ) return NULL ;
	GV_SetActor( &work->actor, TR_Act, TR_Die ) ;
	GV_ActorEX( &work->actor ) ;

	/* テクスチャ差し替え */
	src = MugenTexs ;
	src2 = NormalMirrorTexs ;
	dst2 = MugenMirrorTexs ;

   // Backup original textures, so we can restore them when this object gets destroyed, otherwise the change will STICK.
   {
      for( i = 0; i < 6; ++i )
      {
         DG_TEX* pSource = DG_GetTexture2(TRI_CODE, src[i]);
         work->bp_original_tex[i] = BP_AllocDynamicTexture(16, 16, 0);
         BP_ReplaceTexture(pSource->BP_TextureHandle, work->bp_original_tex[i]);
      }

      for( i = 0; i < 2; ++i )
      {
         DG_TEX* pSource = DG_GetTexture2(TRI_CODE, src2[i]);
         work->bp_original_mirror_tex[i] = BP_AllocDynamicTexture(16, 16, 0);
         BP_ReplaceTexture(pSource->BP_TextureHandle, work->bp_original_mirror_tex[i]);
      }
    }
	
	if ( this == IT_WigA ) {
		dst = BlueTexs ;
		dst2 = BlueMirrorTexs ;
	} else if ( this == IT_WigB ) {
		dst = RedTexs ;
		dst2 = RedMirrorTexs ;
	}

	if ( dst != NULL ) {
		for ( i = 0; i < 6; i ++ ) {
#if 0 //BP_PS2
			work->tex_replace[ i ] = DG_MakeMoveReplacePacket( TRI_CODE, src[ i ], dst[ i ] ) ;
			if ( work->tex_replace[ i ] != NULL ) {
				DG_SetMoveReplaceTexture( work->tex_replace[ i ] ) ;
			}
#else
         // Replace texture
         DG_TEX* pDest = DG_GetTexture2(TRI_CODE, src[i]);
         DG_TEX* pSource = DG_GetTexture2(TRI_CODE, dst[i]);
         BP_ReplaceTexture(pSource->BP_TextureHandle, pDest->BP_TextureHandle);
#endif
		}
	}

	if ( dst2 != NULL ) {
		for ( i = 0; i < 2; i ++ ) {
#if 0 //BP_PS2
			work->tex_replace_mirror[ i ] 
				= DG_MakeMoveReplacePacket( TRI_CODE_MIRROR, src2[ i ], dst2[ i ] ) ;
			if ( work->tex_replace_mirror[ i ] != NULL ) {
				DG_SetMoveReplaceTexture( work->tex_replace_mirror[ i ] ) ;
			}
#else
         // Replace texture
         DG_TEX* pDest = DG_GetTexture2(TRI_CODE, src2[i]);
         DG_TEX* pSource = DG_GetTexture2(TRI_CODE, dst2[i]);
         BP_ReplaceTexture(pSource->BP_TextureHandle, pDest->BP_TextureHandle);
#endif
		}
	}

	TexReplaceWork = work ;
	return work ;
}

void	*NewWigTexReplaceActorScn( int name, int where )
{
	int	this ;

	this = GCL_GetOptionValue( 'i', 0 ) ;
	return NewWigTexReplaceActor( this ) ;
}

/*----------------------------------------------------------------*/

extern void	PL_SendMessage( int to, int *mesg, int v ) ;

/* オリジナル髪の毛消し */
static	inline	void	VisibleOrgHair( int id )
{
	int			buf[ 2 ] ;

	buf[ 0 ] = 0 ;
	buf[ 1 ] = id ;
	PL_SendMessage( GV_StrCode( "ライデン髪の毛" ), buf, 2 ) ;
}

/* オリジナル髪の毛のサンプル番号を取得するメッセージを発行 */
static	inline	void	SendMessageToOrgHair( Work *work )
{
	int			buf[ 2 ] ;

	buf[ 0 ] = 9 ;
	buf[ 1 ] = GV_StrCode( "カツラマネージャー" ) ; 
	PL_SendMessage( GV_StrCode( "ライデン髪の毛" ), buf, 2 ) ;
}

static	inline	void	ChangeSampleNo( Work *work )
{
	int			n_msg, next ;
	int			buf[ 2 ] ;
	GV_MSG		*msg ;

	n_msg = GV_ReceiveMessage( GV_StrCode( "カツラマネージャー" ), &msg ) ;
	if ( n_msg == 0 ) return ;
	while( -- n_msg >= 0 ) {
		next = msg->message[ 1 ] ;
		if ( work->sample != next ) {
			//printf( "change hair sample -> %d\n", next ) ;
			buf[ 0 ] = 1 ;
			buf[ 1 ] = next ;
			PL_SendMessage( work->name, buf, 2 ) ;
			work->sample = next ;
		}
		msg ++ ;
	}
}

/*----------------------------------------------------------------*/

static	void	Act( Work *work )
{
	VisibleOrgHair( 0 ) ;

	SendMessageToOrgHair( work ) ;
	ChangeSampleNo( work ) ;
}

static	void	Die( Work *work )
{
	VisibleOrgHair( 1 ) ;
	if ( work->texreplace != NULL ) {
		GV_ACT		*actor ;

		actor = GV_SearchActor( work->texreplace ) ;
		if ( actor != NULL && actor->name == GV_StrCode( "wigtexreplace" ) ) {
			GV_DestroyOtherActorQuick( work->texreplace ) ;
		}
	}
}

/*----------------------------------------------------------------*/

static	int	GetResources( Work *work, int this )
{
	u_char	colobjs[] = { HUMAN21_MUNE, HUMAN21_KUBI, HUMAN21_ATAMA } ;

	/* テクスチャ差し替え */
	//if ( this != IT_MugenWig ) {
		work->texreplace = NewWigTexReplaceActor( this ) ;
		if ( work->texreplace != NULL ) {
			GV_SetActorChild( work, work->texreplace ) ;
			( ( GV_ACT * )work->texreplace )->name = GV_StrCode( "wigtexreplace" ) ;
		} else {
			return -1 ;
		}
	//}

	work->name = GV_StrCode( "カツラ" ) + this ;
	work->sample = 0 ;
	work->hair = NewEvmHairModel_called( work->name, GV_StrCode( "rai_zura_mugen_mh_mt" ),
									  0, GM_PlayerBody, HUMAN21_ATAMA, NULL, NULL,
									  1.0F, 3, colobjs, 0, 0, GV_StrCode( "rai_hair_bounding" ) ) ;
	if ( work->hair == NULL ) return -1 ;
	GV_SetActorChild( work, work->hair ) ;

	GM_ClearCodeFlag |= GM_CLEAR_SPECIAL_ITEM_USED ;

	switch( this ) {
	case IT_MugenWig :
		GM_ClearCodeFlag |= GM_CLEAR_MUGENWIG_USED ;
		break ;
	case IT_WigA :
		GM_ClearCodeFlag |= GM_CLEAR_WIG_A_USED ;
		break ;
	case IT_WigB :
		GM_ClearCodeFlag |= GM_CLEAR_WIG_B_USED ;
	}

	return 0 ;
}

/*----------------------------------------------------------------*/

static	void	*NewWig( CONTROL *ctrl, OBJECT **body, int **unit, u_int *trigger, int this )
{
	Work		*work ;

	work = ( Work * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT,
									 sizeof( Work ), PLAYER_ITEM_ACTOR_PRIO ) ;
	if ( work == NULL ) return NULL ;
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	if ( GetResources( work, this ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;
}

void	*NewMugenWig( CONTROL *ctrl, OBJECT **body, int **unit, u_int *trigger )
{
	return NewWig( ctrl, body, unit, trigger, IT_MugenWig ) ;
}

void	*NewMugenO2Wig( CONTROL *ctrl, OBJECT **body, int **unit, u_int *trigger )
{
	return NewWig( ctrl, body, unit, trigger, IT_WigA ) ;
}

void	*NewMugenGripWig( CONTROL *ctrl, OBJECT **body, int **unit, u_int *trigger )
{
	return NewWig( ctrl, body, unit, trigger, IT_WigB ) ;
}
