//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   varsys.c
   振動アーカイブシステム

   2000/10/12	M.Sonoyama
   $Id: varsys.c,v 1.3 2002/11/23 11:01:04 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

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

#include "BP_EndianSupport.h"

/*----------------------------------------------------------------*/

extern void	*NewPadVibration3( u_char *, int * ) ;

static	u_char	*SearchVibElem( VAR_DEF *def, int vib, int vls )
{
	u_char	*data = NULL ;
	int		i ;

	if ( vls ) {
		if ( vib < 0 || vib >= def->n_elems ) return NULL ;
		data = def->elems[ vib ] ;
	} else {
		for ( i = 0; i < def->n_elems; i ++ ) {
			if ( vib == def->elemNo[ i ] ) {
				data = def->elems[ i ] ;
				break ;
			}
		}

	}
	return data ;
}

static	u_char	*SearchVibElemName( VAR_DEF *def, int name )
{
	u_char	*data = NULL ;
	int		i ;

	if ( def->name_id == NULL ) return NULL ;
	for ( i = 0; i < def->n_elems; i ++ ) {
		if ( name == def->name_id[ i ] ) {
			data = def->elems[ i ] ;
			break ;
		}
	}
	return data ;
}

/*----------------------------------------------------------------*/

VAR_DEF		*GM_GetVibArc( int file )
{
	int			i ;
	VAR_DEF		*def ;

	def = ( VAR_DEF * )GV_GetCache( GV_CacheID( file, 'v' ) ) ;
	if ( def == NULL ) return NULL ;
	if ( BP_LE_SwapSInt( def->magic_le ) != VAR_MAGIC ) {
		/* 単一のvibファイルである可能性が高い */
		return NULL ;
	}
	if ( BP_LE_SwapUShort( def->version_le ) != VAR_VERSION ) {
		printf( "var file error : invalid version %d\n", BP_LE_SwapSInt( def->version_le ) ) ;
		return NULL ;
	}

	if ( !( BP_LE_SwapUShort( def->flag_le ) & GM_VAR_FLAG_INITIALIZE ) ) {
      BP_LE_SwapSInt_Inp( &def->n_elems );
      BP_LE_SwapPtr_Inp( &( def->elemNo ) );
      BP_LE_SwapPtr_Inp( &( def->name_id ) );
      BP_LE_SwapPtr_Inp( &( def->elems  ) );

		if ( BP_LE_SwapUShort( def->flag_le ) & GM_VAR_FLAG_VLS ) {
			def->elemNo = NULL ;
		} else {
			def->elemNo = ( u_short * )( ( int )def + ( int )def->elemNo ) ;

         BP_LE_SwapUShortArray_Inp( def->elemNo, def->n_elems );
		}

      if ( def->name_id != NULL ) {
			def->name_id = ( int * )( ( int )def + ( int )def->name_id ) ;

         BP_LE_SwapSIntArray_Inp( def->name_id, def->n_elems );
		}
		def->elems = ( u_char ** )( ( int )def + ( int )def->elems ) ;
		for ( i = 0; i < def->n_elems; i ++ ) {
         BP_LE_SwapPtr_Inp( &( def->elems[i]) );

			def->elems[ i ] = ( u_char * )( ( int )def + ( int )def->elems[ i ] ) ;
		}	
	}
	def->flag_le |= BP_LE_SwapUShort( GM_VAR_FLAG_INITIALIZE );
//	printf( "vibarc %d:%x\n", file, def ) ;
	return def ;
}

u_char		*GM_FindVibData( int name )
{
	u_char		*res ;
	VAR_DEF		*def ;

	res = ( u_char * )GV_GetCache( GV_CacheID( name, 'v' ) ) ;
	if ( res != NULL ) return res ;
#if 1
	/* なかったら cache.var と resident.var から探す */
	def = GM_GetVibArc( GV_StrCode( "cache" ) ) ;
	if ( def != NULL ) {
		res = SearchVibElemName( def, name ) ;
		if ( res != NULL ) {
			return res ;
		}
	}
	def = GM_GetVibArc( GV_StrCode( "resident" ) ) ;
	if ( def != NULL ) {
		res = SearchVibElemName( def, name ) ;
		if ( res != NULL ) return res ;
	}
#endif
	return NULL ;
}


int		GM_InitVarControl( GV_ACT_EX *actor, VAR_CONTROL *vctrl, int file )
{
	ASSERT( ( ( GV_ACT * )actor )->class & GV_CLASS_EX ) ;
	GV_ZeroMemory( vctrl, sizeof( VAR_CONTROL ) ) ;
	if ( ( vctrl->def = GM_GetVibArc( file ) ) == NULL ) {
		printf( "warning : var file %x does not exist.\n" ) ;
	}
	vctrl->actor = actor ;
	vctrl->vctrl[ 0 ] = VAR_FLAG_IDLE ;
	vctrl->vctrl[ 1 ] = VAR_FLAG_IDLE ;
	vctrl->active = 0 ;
	vctrl->m_ctrl = NULL ;
	vctrl->vibact = NULL ;
	return 0 ;
}

void	GM_ConfigVctrlMctrl( VAR_CONTROL *vctrl, MOTION_CONTROL *m_ctrl ) 
{
	vctrl->m_ctrl = m_ctrl ;
}

void	GM_ActVctrl( VAR_CONTROL *vctrl )
{
	int		active ;

	active = vctrl->active ;
	if ( vctrl->vib >= 0 && MT_CHECK_END( vctrl->m_ctrl, 0 ) ) {
		GM_VctrlSetVibration( vctrl, vctrl->vib, 0 ) ;
	}
}

void	GM_VctrlSetVibration( VAR_CONTROL *vctrl, int vib, int flag )
{
	int			vls, active, rest ;
	VAR_DEF		*def ;
	u_char		*data ;

	def = vctrl->def ;
	if ( def == NULL ) {
		vctrl->vib = -1 ;
		vctrl->vibact = NULL ;
		return ;
	}

	vls = ( BP_LE_SwapUShort( def->flag_le )& GM_VAR_FLAG_VLS ) ? 1 : 0 ;
	if ( vib >= 0 ) {
		data = SearchVibElem( def, vib, vls ) ;
		if ( data == NULL ) {
//			printf( "warning : vibration data is not exist : %d\n", vib ) ;
            goto vib_stop_all ;
		}
		rest = vctrl->active ;
		active = vctrl->active = 1 - vctrl->active ;
		if ( vctrl->vctrl[ rest ] & VAR_FLAG_PLAYING ) {
			/* 可動中のものを止める */
			vctrl->vctrl[ rest ] |= VAR_FLAG_PLAY_STOP ;
//			vctrl->vctrl[ rest ] &= ~VAR_FLAG_LOOP ;
		}
		vctrl->vctrl[ active ] = VAR_FLAG_PLAYING | flag ;
		vctrl->vib = vib ;
#ifdef DEBUG_MODE
		printf( "vib start %d:%d\n", vib, vls ) ;
#endif
		vctrl->vibact = NewPadVibration3( data, &vctrl->vctrl[ active ] ) ;
		if ( vctrl->vibact != NULL ) GV_SetActorChild( vctrl->actor, vctrl->vibact ) ;
	} else {
vib_stop_all :
		/* 可動中のものを止める */
		if ( vctrl->vctrl[ 0 ] & VAR_FLAG_PLAYING ) {
//			vctrl->vctrl[ 0 ] |= VAR_FLAG_PLAY_STOP ;
			vctrl->vctrl[ 0 ] &= ~VAR_FLAG_LOOP ;
		}
		if ( vctrl->vctrl[ 1 ] & VAR_FLAG_PLAYING ) {
//			vctrl->vctrl[ 1 ] |= VAR_FLAG_PLAY_STOP ;
			vctrl->vctrl[ 1 ] &= ~VAR_FLAG_LOOP ;
		}
        vctrl->vibact = NULL ;
		vctrl->vib = -1 ;
	}
}

void	GM_VctrlStopVibration( VAR_CONTROL *vctrl )
{
	/* 可動中のものを強制的に止める */
	if ( vctrl->vctrl[ 0 ] & VAR_FLAG_PLAYING ) {
		vctrl->vctrl[ 0 ] |= VAR_FLAG_PLAY_STOP ;
	}
	if ( vctrl->vctrl[ 1 ] & VAR_FLAG_PLAYING ) {
		vctrl->vctrl[ 1 ] |= VAR_FLAG_PLAY_STOP ;
	}
}

void	GM_VctrlSetVibrationScale( VAR_CONTROL *vctrl, float scale )
{
	//printf( "[%d] set vib scale\n", GV_Time ) ;
	if ( vctrl->vibact != NULL ) {
		GM_SetVibrationScale( vctrl->vibact, scale ) ;
	}
}

void	GM_VctrlSetFlag( VAR_CONTROL *vctrl, int flag )
{
	vctrl->vctrl[ vctrl->active ] |= flag ;
}

void	GM_VctrlResetFlag( VAR_CONTROL *vctrl, int flag )
{
	vctrl->vctrl[ vctrl->active ] &= ~flag ;
}

void	GM_VarSetVibration( VAR_DEF *def, int file, int vib, int *flag )
{
	VAR_DEF		*def2 ;
	u_char		*script ;

	if ( def == NULL ) {
		def2 = GM_GetVibArc( file ) ;
	} else {
		def2 = def ;
	}
	ASSERT( def2 != NULL ) ;
	ASSERT( vib >= 0 && vib < def2->n_elems ) ;
	script = def2->elems[ vib ] ;
	if ( flag != NULL ) *flag = VAR_FLAG_PLAYING ;
	NewPadVibration3( script, flag ) ;
}
