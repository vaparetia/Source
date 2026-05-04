//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    デモエフェクトコントロール管理
	eft_con.c
	2001/04/12 T.Shibata

	$Id: eft_con.c,v 1.1.1.3 2002/11/19 11:45:10 Yoshizawa1 Exp $
*/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <mgs_type.h>
#include <libgv.h>
#include <libdg.h>
#include <libfs.h>
#include <stream.h>
#include <gameheader.h>
#include <camera.h>

#include "libdemo.h"
#define		N_LIST_KEY	(16)

static EFTCONTROL *EftConList[N_LIST_KEY];
//static int EftConListNum[N_LIST_KEY];

//リスト初期化
void DM_EftControlClearList( void )
{
	int i;
	for( i = 0; i < N_LIST_KEY; i++ ){
		EftConList[i] = NULL;
		//EftConListNum[i] = 0;
	}
	printf( "EFTLIST EftConList Init\n" );
}

//EftControlにnameを入れてリストに追加
void DM_EftControlAddList( int name, EFTCONTROL *add )
{
	add->name = name;
	add->next = NULL;

	add->next = EftConList[name&0x0f];
	EftConList[name&0x0f] = add;

	//EftConListNum[name&0x0f]++;

	printf( "EFTLIST add list[%d]< %x >\n", name&0x0f, name );
}
//リストから削除
void DM_EftControlDelList( EFTCONTROL *del )
{
	EFTCONTROL *list;

	//EftConListNum[del->name&0x0f]--;

	if( EftConList[del->name&0x0f] == del ){
		EftConList[del->name&0x0f] = del->next;
		
		printf( "EFTLIST sub list top[%d]< %x >\n", del->name&0x0f, del->name );
		return;
	}

	list = EftConList[del->name&0x0f];

	for( ; list->next; list = list->next ){
		if( list->next == del ){
			list->next = del->next;
			printf( "EFTLIST sub list center[%d]< %x >\n", del->name&0x0f, del->name );
			return;
		}
	}
}
//リストから検索
EFTCONTROL *DM_GetEftControl( int name )
{
	EFTCONTROL *list;

	list = EftConList[name&0x0f];

	for( ; list; list = list->next ){
		if( list->name == name ){
			return list;
		}
	}
	printf("EFTLIST ERR!! No EftControl[%d]< %x >\n",name&0x0f,name);
#if 0
	{
		int i;
		for( i = 0; i < N_LIST_KEY; i++ ){
			printf("%d< %d >\n",i,EftConListNum[i]);
		}
	}
#endif
	return NULL;
}

