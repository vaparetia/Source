/*
	tst_clut.c
	モノクロＣＬＵＴエフェクト実験

	1999/11/15 K.Takabe
	$Id: tst_clut.c,v 1.1.1.3 2002/11/19 11:51:30 Yoshizawa1 Exp $

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"def_dma.h"
#include	"gameheader.h"


#define ALIGNSIZE1(_n) (_n)
#define ALIGNSIZE16(_n) (((_n)+15)&0xfffffff0)
#define ALIGNSIZE64(_n) (((_n)+63)&0xffffffc0)
#define ALIGNSIZE128(_n) (((_n)+127)&0xffffff80)


/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT		actor ;
	int			time ;
	void		*tmp_clut_addr ;
	int			n_list ;
} Work ;

/* ---------------------------------------------------------------- */

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	int		i, j ;
	DG_TEXTURE_LIST		**list ;
	void				*mem ;

	/* カウンタチェック */
	if ( work->time == 0 ){
		GV_DestroyActor( work );
		return ;
	}
	work->time-- ;

	/* 初期化したメモリを一時ＣＬＵＴアドレスに設定する */
	mem = work->tmp_clut_addr ;
	list = DG_TextureList ;
	for ( i = DG_MaxTextures ; i > 0 ; i-- ){
		(*list)->tmp_clut_image = mem ;
		mem = (void*)( (int)mem + (*list)->header->clut_size * 64 * 4 ) ;
		list++ ;
	}
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	GV_Free( work->tmp_clut_addr );
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, int time )
{
	int		all_clut_size = 0 ;
	int		i, j ;
	DG_TEXTURE_LIST		*list, **list_p ;
	void				*mem ;
	TransDataInfo		*datainfo ;

	work->time = time ;

	/* ＣＬＵＴの全サイズを求める（ここで言うサイズは６４ワード単位なので注意！！） */
	list_p = DG_TextureList ;
	for ( i = DG_MaxTextures ; i > 0 ; i-- ){
		list = *list_p;
		all_clut_size += ( list->header->clut_size + 2047 ) & 0xfffff800 ;
		list_p++ ;
	}

	/* １２８バイトにアラインしたメモリを確保 */
	mem = work->tmp_clut_addr = GV_AllocMemory( GV_NORMAL_MEMORY, NULL, ALIGNSIZE128( all_clut_size*64*4 ) , 128 );
	if ( work->tmp_clut_addr == NULL ) return ( -1 );

	list_p = DG_TextureList ;
	for ( i = DG_MaxTextures ; i > 0 ; i-- ){
		/* モノクロＣＬＵＴの生成 */
		list = *list_p;
#if 1
		/* 遅いバージョン */
		unsigned int		*clut, *org_clut ;
		clut = mem ;
		org_clut = list->clut_image ;
		for ( j = list->header->clut_size * 64 ; j > 0 ; j-- ){
			unsigned int	tmp ;
			if ( (*clut) & 0x00ffffff ){
				/* 色付きＣＬＵＴ */
				tmp = ( ( *org_clut ) & 255 ) + ( ( *org_clut >> 8 ) & 255 ) + ( ( *org_clut >> 16 ) & 255 ) ;
				tmp = ( tmp * ( 256/3 ) ) >> 8 ;
				/* アルファはオリジナルのものを使用し、ＲＧＢ値のみを変更する */
				*clut = ( *org_clut & 0xff000000 ) | ( tmp << 16 ) | ( tmp << 8 ) | tmp ;
			} else {
				/* 透明ＣＬＵＴ */
				*clut = *org_clut ;
			}
			org_clut++ ;
			clut++ ;
		}
#else
		/* 速いバージョン */
		/* source/takabe/effect1/grayclut.cを参照 */
#endif
		mem = (void*)( (int)mem + list->header->clut_size * 64 * 4 ) ;
		list_p++ ;
	}
	FlushCache( 0 );

	return (0);
}


/* ---------------------------------------------------------------- */
void *NewGrayClut( int time )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, time ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
