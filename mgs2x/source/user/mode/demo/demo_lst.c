//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   demolst.c

   2001.07.31 T.Morita
   $Id: demo_lst.c,v 1.1.1.3 2002/11/19 11:45:08 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libdma.h>
#include <libdev.h>
#endif

#include <gameheader.h>

#include "libdemo.h"


int DM_n_RenameList = 0 ; /* リネームリストの長さ */
int DM_RenameFrom[DM_MAX_RENAME] ;
int DM_RenameTo[DM_MAX_RENAME] ;
int DM_RenameDispFlag[DM_MAX_RENAME] ;/* 描画フラグ */

int DM_n_EquipList = 0 ; /* 装備リストの長さ */
int DM_EquipTo[DM_MAX_RENAME] ;
int DM_EquipFlag[DM_MAX_RENAME] ;


void DM_RefreshRenameList( void )
{
    int i ;

    DM_n_RenameList = DM_n_EquipList = 0 ;

    for( i=DM_MAX_RENAME ; --i>=0 ; ) {
	DM_RenameFrom[i] = 0 ;
	DM_RenameTo[i] = 0 ;
	DM_EquipTo[i] = 0 ;
	DM_EquipFlag[i] = 0 ;
	DM_RenameDispFlag[i] = 0 ;
    }
}

void DM_AddRenameList( int ext, int flag )
{
    int id ;

    ext  -= 'a' ;
    ext <<= 24 ;
    while( DM_n_RenameList < DM_MAX_RENAME )
    {
	DM_RenameDispFlag[DM_n_RenameList] = flag ;
	id = GCL_GetNextInt() | ext ;
	DM_RenameFrom[DM_n_RenameList] = id ;
	id = GCL_GetNextInt() | ext ;
	DM_RenameTo[DM_n_RenameList]   = id ;
	if ( GV_GetCache( id ) )
	    DM_n_RenameList++ ;
	if ( !GCL_NextStr() )
	    break ;
    }
}

void DM_AddObjChange( int flag )
{
    int id, ext ;

    while( DM_n_RenameList < DM_MAX_RENAME )
    {
	DM_RenameDispFlag[DM_n_RenameList] = flag ;

	id  = GCL_GetNextInt() ;
	ext = GCL_GetNextInt() ;
	ext = (ext==113171 ? 'k' : ext==107309 ? 'e' : 'a') -'a' ;
	DM_RenameFrom[DM_n_RenameList] = id | ((ext | 0x80) << 24) ;

	id  = GCL_GetNextInt() ;
	ext = GCL_GetNextInt() ;
	ext = (ext==113171 ? 'k' : ext==107309 ? 'e' : 'a') -'a' ;
	DM_RenameTo[DM_n_RenameList++] = id | (ext << 24) ;

	if ( !GCL_NextStr() )
	    break ;
    }
}

void DM_AddEquipFlag()
{
    while( DM_n_EquipList < DM_MAX_RENAME )
    {
	DM_EquipTo[DM_n_EquipList]   = GCL_GetNextInt() ;
	DM_EquipFlag[DM_n_EquipList] = GCL_GetNextInt() ;
	DM_n_EquipList++ ;

	if ( !GCL_NextStr() )
	    break ;
    }
}


void DM_AddArkmsChange( int flag )
{
    int id, ext ;
    int mode, type ;

    if ( DM_n_RenameList < DM_MAX_RENAME &&
	 DM_n_EquipList  < DM_MAX_RENAME )
    {
	DM_RenameDispFlag[DM_n_RenameList] = flag ;

	id  = GCL_GetNextInt() ;
	ext = GCL_GetNextInt() ;
	ext = (ext==113171 ? 'k' : 'e') -'a' ;
	DM_RenameFrom[DM_n_RenameList] = id | ((ext | 0x80) << 24) ;

	mode = GCL_GetNextInt() & 0x00ff ;
	type = GCL_GetNextInt() & 0xffff ;
	DM_RenameTo[DM_n_RenameList] = mode | (type << 8) ;
	DM_n_RenameList++ ;

	DM_EquipTo[DM_n_EquipList]   = mode ;
	DM_EquipFlag[DM_n_EquipList] = GCL_GetNextInt() ;
	DM_n_EquipList++ ;
    }
}

/* リネームリストに描画フラグを加える */



/*

   ローポリ劇場用モデルの入れ替えをさせるテーブルつくり

   各関数内でGCL_GetNext???()が呼び出される

*/
int NewCom_AddRenameList()
{
	int flag ;

    flag = GCL_GetOptionValue( 'f', 0 ) ;

#ifdef DEBUG_MODE
    printf( "NewCom_AddRenameList %x %d\n", GCL_GetOption( 'c' ), flag ) ;
#endif


    if ( GCL_GetOption( 'E' ) != NULL )
	DM_AddRenameList( 'e', flag ) ;

    if ( GCL_GetOption( 'K' ) != NULL )
	DM_AddRenameList( 'k', flag ) ;

    /* ローポリ劇場用モデルの変更 */
    if ( GCL_GetOption( 'c' ) != NULL )
	DM_AddObjChange( flag ) ;

    /* 装備フラグを変更する場合 */
    if ( GCL_GetOption( 'e' ) != NULL )
	DM_AddEquipFlag() ;

    /* ARKMSに変更する場合 */
    if ( GCL_GetOption( 'a' ) != NULL )
	DM_AddArkmsChange( flag ) ;

    return 1 ;
}
