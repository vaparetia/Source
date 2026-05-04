/*
	facepack.c
	圧縮型固定モーションパックデータ管理ルーチン

	2000/10/02 K.Takabe
	$Id: facepack.c,v 1.1.1.3 2002/11/19 11:42:51 Yoshizawa1 Exp $

*/
/*


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

#include	"libgv.h"
#include	"libdg.h"
#include	"libmt.h"
#include	"utl_dma.h"
#include	"def_dma.h"


static float MFtoF( unsigned short mf );

/*----------------------------------------------------------------*/

static void UncompressMotion( FVECTOR *qrots, FVECTOR *trans, unsigned short *buffer, int n_joints )
{
	int		i ;
	float	d = 1.0f / 32767.0f ;
	for ( i = 0 ; i < n_joints ; i++ ){
		qrots->vx = (*(short*)buffer++) * d ;
		qrots->vy = (*(short*)buffer++) * d ;
		qrots->vz = (*(short*)buffer++) * d ;
		qrots->vw = (*(short*)buffer++) * d ;
		trans->vx = MFtoF( *buffer++ );
		trans->vy = MFtoF( *buffer++ );
		trans->vz = MFtoF( *buffer++ );
		qrots++ ;
		trans++ ;
	}
}

/* 圧縮型モーションパックデータから指定ＩＤのモーションデータを取得 */
void MT_DecodeFpkMotion( FVECTOR *qrots, FVECTOR *trans, void *pack_data, int id )
{
	FPK_HEADER		*header ;
	FPK_DATALIST	*list ;
	int				i ;

	header = pack_data ;
	list = header->list ;
	for ( i = header->n_datas ; i > 0 ; list++, i++ ){
		if ( list->id == id ){
			break ;
		}
	}
	if ( i == 0 ){
		printf("facepack.c: data not found!!\n");
		return ;
	}

	UncompressMotion( qrots, trans, list->data, list->n_joints );
}



/* ---------------------------------------------------------------- */
/* ================================================================ */
/*

	１６ビット浮動小数点フォーマット

	seeeeeffffffffff
	||    |
	||    +- 10bit 仮数部
	||
	|+- 5bit 指数（バイアス値 N の下駄履き指数）
	|
	+- 1bit 符合

*/

/* 指数のバイアス値（2^0となる指数値） */
#define E_BIAS	(8)		/* メタルギアでは大きい値を重視する */

#if 0
/* 単精度浮動小数点から１６ビット浮動小数点へ変換 */
static unsigned short FtoMF( float f )
{
	unsigned int	data, s_dat, e_dat, f_dat ;
	unsigned short	mf ;

	data = *(unsigned int*)&f ;
	/* 符合取りだし */
	s_dat = ( data & 0x80000000 ) >> 16 ;
	/* 指数取りだし */
	e_dat = ( data & 0x7f800000 ) ;
	if ( e_dat != 0 ){
		if ( e_dat <= ( 127 - E_BIAS ) << 23 ){
			//printf("convert error!!\n");
			return ( 0x0000 | s_dat );
		}
		if ( e_dat > ( 127 - E_BIAS + 31 ) << 23 ){
			//printf("convert error!!\n");
			return ( 0x7fff | s_dat );
		}
		e_dat -= ( 127 - E_BIAS ) << 23 ;
		e_dat >>= 13 ;
	}
	/* 仮数部取りだし */
	f_dat = ( data & 0x007fffff ) >> 13 ;

	/* ビット合成 */
	mf = s_dat | e_dat | f_dat ;

	return ( mf );
}
#endif

/* １６ビット浮動小数点から単精度浮動小数点へ変換 */
static float MFtoF( unsigned short mf )
{
	unsigned int	s_dat, e_dat, f_dat ;
	float			f ;

	/* 符合取りだし */
	s_dat = ( mf & 0x8000 ) << 16 ;
	/* 指数取りだし */
	e_dat = ( mf & 0x7c00 ) ;
	if ( e_dat != 0 ){
		e_dat += ( 127 - E_BIAS ) << 10 ;
		e_dat <<= 13 ;
	}
	/* 仮数部取りだし */
	f_dat = ( mf & 0x03ff ) << 13 ;
	/* ビット合成 */
	*(unsigned int*)&f = s_dat | e_dat | f_dat ;

	return ( f );
}


