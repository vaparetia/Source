/*
	sort.c
	チャンネル処理ユニット／プリミティブソート処理ルーチン

	1999/07/07 K.Takabe
	$Id: sort.c,v 1.1.1.3 2002/11/19 11:42:26 Yoshizawa1 Exp $

*/
/*
	チャンネル処理ユニット／プリミティブソート処理ルーチン
	表示は半透明モデル表示ルーチンで

	void DG_SortChanl( DG_CHANL *cp, int which );
	DG_CHANL	*cp ;		チャンネル構造体
	int		which ;		ダブルバッファ選択

	プリミティブのソートを行う

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

#include	"def_dma.h"

typedef struct {
	void		*ot3[64] ;
	void		*ot2[64] ;
	void		*ot1[64] ;
} SortWork ;

void	*DG_PrimOT[64] ;

/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
	/*
		プリミティブのソートリストを作成する
	*/
static	void	MakeSortListPrim( DG_PRIM_HEADER *prim_header, int raise )
{
	SortWork	*work = (SortWork*)SCRPAD_ADDR ;
	u_int		z ;
	void		**ot ;

	/* ソート用データを作成 */
	z = ( ( (u_int)( prim_header->sort_z - raise ) + 0x7fffff ) >> 6 ) ;/* あらかじめ６４で割っておく */
	prim_header->z = z ; z &= 63 ;
	ot = work->ot1 + z ;
	prim_header->next_addr = *ot ; *ot = prim_header ;
}

	/*
		ソートを行う
	*/
static	void	SortListPrims( void )
{
	SortWork	*work = (SortWork*)SCRPAD_ADDR ;
	int			i ;
	u_int		z ;
	DG_PRIM_HEADER	*header ;
	void		**ot2, **ot3, *next_addr ;

	/* 順番を崩さないように大きい方からソートしていく */
	for ( i = 63 ; i >= 0  ; i-- ){
		next_addr = work->ot1[i] ;
		while ( next_addr != NULL ){
			header = next_addr ;
			next_addr = header->next_addr ;
			z = header->z ; z = ( z >> 6 ) & 63 ;
			ot2 = work->ot2 + z ;
			header->next_addr = *ot2 ;
			*ot2 = header ;
		}
	}

	/* 順番を崩さないように小さい方からソートしていく */
	for ( i = 0 ; i < 64   ; i++ ){
		next_addr = work->ot2[i] ;
		while ( next_addr != NULL ){
			header = next_addr ;
			next_addr = header->next_addr ;
			z = header->z ; z = ( z >> 12 ) & 63 ;
			ot3 = work->ot3 + z ;
			header->next_addr = *ot3 ;
			*ot3 = header ;
		}
	}

	{/* 最終ＯＴをメインメモリに転送する */
		u_long128	*dst, *src ;
		src = (u_long128*)work->ot3 ;
		dst = (u_long128*)DG_PrimOT ;
		for ( i = 64/4 ; i > 0 ; i-- ){
			*dst++ = *src++ ;
		}
	}
}

#if 1
	/*
		ＤＭＡに接続する
	*/
static	void	SortChainPrims( void )
{
	SortWork	*scrpad = (SortWork*)SCRPAD_ADDR ;
	int			i, j, k, size ;
	void		*next_addr ;
	void		*prim_addr ;
	DG_PRIM_HEADER	*header ;
	DG_DMATAG	*tag ;

	for ( k = 63 ; k >= 0  ; k-- ){
		next_addr = scrpad->ot3[k] ;
		while ( next_addr != NULL ){
			header = next_addr ;
			next_addr = header->next_addr ;
			prim_addr = &header[1] ;

			tag = (DG_DMATAG*)DG_CurrentDmaAddr++ ;
			tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CALL, 0 );
			tag->addr = prim_addr ;
			tag->vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
			tag->vifcode[1] = SCE_VIF1_SET_NOP( 0 ) ;
		}

	}

}
#endif
/*----------------------------------------------------------------*/
void DG_SortChanl( DG_CHANL *cp, int which )
{
	extern u_long128	*DG_CurrentDmaAddr ;
	DG_PRIM			**pque ;
	DG_PRIM_PACKET	*packet ;
	DG_DMATAG		*tag ;
	DG_OBJ_BUFFER	*obj_buff ;
	int				i, j, gid, c_gid ;

	if ( cp->chanl_num != 0 && cp->chanl_num != 4 ) return ;
	/*
		スクラッチパッド上のＯＴ初期化
	*/
	memset( ((SortWork*)SCRPAD_ADDR)->ot3, 0, sizeof(int)*64*3 );

	/*
		キュー数のチェック
	*/
	if (  ( cp->obj_queue != NULL ) && ( ( i = cp->obj_queue->prim_buffer.n_queue ) != 0 ) ){
		pque = cp->obj_queue->prim_buffer.queue ;
		c_gid = cp->group_id ;

		for ( ; i > 0 ; -- i ) {
			DG_PRIM		*prim ;
			int		type ;

			prim = *( pque ++ ) ;
			type = prim->type ;
			if ( ( DG_PRIM_INVISIBLE ) & type ) continue ;
			if ( ( gid = prim->group_id ) != 0 && !( gid & c_gid ) ) continue ;

			packet = (DG_PRIM_PACKET*)( (int)prim->packs[ which ] + prim->packet_size * 0 ) ;
			for ( j = prim->n_packet ; j > 0 ; j-- ){
				if ( packet->header.sort_z > prim->near_z ){
					MakeSortListPrim( &packet->header, prim->raise );
				}
				packet = (DG_PRIM_PACKET*)( (int)packet + prim->packet_size );
			}
		}
	}

	SortListPrims();

	DG_OpenDmaTask( DG_OPEN_DMA_VIF1, NULL, 0 );
	SortChainPrims();
	DG_CloseDmaTask();
}
/*----------------------------------------------------------------*/
