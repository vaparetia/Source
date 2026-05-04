/*
	m_weight.c
	マルチウェイトエンベロープ補助ルーチン

	1999/09/07 K.Takabe
	$Id: m_weight.c,v 1.1.1.3 2002/11/19 11:42:12 Yoshizawa1 Exp $

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

#include	"libdg.h"
#include	"libdg.cnf"
#include	"def_dma.h"


extern qword MultiWeightEnvelopeVu0 ;
extern qword MultiWeightEnvelopeVu0_Func ;


/* ---------------------------------------------------------------- */
	/*
		マルチウェイト計算マイクロプログラム読み込み
	*/
void DG_StartMultiWeightSupport( void )
{
	DG_DmaStartCheck( 0 );
	DPUT_D0_MADR( ((DG_DMATAG*)MultiWeightEnvelopeVu0)->addr ) ;
	DPUT_D0_QWC( ( ((DG_DMATAG*)MultiWeightEnvelopeVu0)->qwc ) & 0x7fff ) ;
	//DPUT_D0_CHCR( 0x0141 ) ; /* STR:1 TIE:0 TTE:1 ASP:0 MOD:0 DIR:1 */
	asm volatile( "sync.l;sw %1, 0(%0)" :: "r"(D0_CHCR),"r"(0x141) );
	DG_WaitDma( 0 );
}


/* ---------------------------------------------------------------- */
	/*
		マルチウェイト計算マトリクスの読み込み
	*/
void DG_StoreSkeletonMatrix( FMATRIX *mats, int num )
{
	/* vu0 mem にストアする（最大６４マトリクスまで） */
#if 0
	static struct _dma_header{
		DG_DMATAG	tag ;
	} DMA_HEADER ALIGN16 ;
	struct _dma_header *header ALIGN16;

	header = GV_UNCASHEA( &DMA_HEADER ) ;
	DG_DmaStartCheck( 0 );
	header->tag.qwc = DMATAG_SET_QWC( DMATAG_ID_REFE, num * 4 );
	header->tag.addr = (void*)mats ;
	header->tag.vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 1, 0 ) ;
	header->tag.vifcode[1] = SCE_VIF1_SET_UNPACK( 0x00, num * 4, VIF_DATA128, 0 ) ;
	asm("sync.l");
	DPUT_D0_TADR( (void*)&DMA_HEADER ) ;
	DPUT_D0_CHCR( 0x0145 ) ; /* STR:1 TIE:0 TTE:1 ASP:0 MOD:1 DIR:1 */
	DG_WaitDma( 0 );
#else
	asm("ctc2	$0,$vi1");
	while( num ){
		asm("
			lqc2		vf4,0(%0)
			lqc2		vf5,16(%0)
			lqc2		vf6,32(%0)
			lqc2		vf7,48(%0)
			vsqi		vf4,(vi1++)
			vsqi		vf5,(vi1++)
			vsqi		vf6,(vi1++)
			vsqi		vf7,(vi1++)
		"::"r"(mats));
		mats++ ;
		num-- ;
	}
#endif
}



/* ---------------------------------------------------------------- */
/* 頂点に対してマルチウェイト計算を行う */
void DG_TransMultiWeightVertex( FVECTOR *verts, FVECTOR *weight, char *mat_id, int n_verts )
{
}

/* 法線に対してマルチウェイト計算を行う */
void DG_TransMultiWeightNormal( FVECTOR *norms, FVECTOR *weight, char *mati_d, int n_verts )
{
}

/* 頂点・法線に対してマルチウェイト計算を行う */
void DG_TransMultiWeightVertexNormal( FVECTOR *res_verts, FVECTOR *res_norms, FVECTOR *verts, FVECTOR *norms, FVECTOR *weight, char *mat_id, int n_verts )
{
	struct _work {
		FVECTOR		trans_verts[256] ;
		FVECTOR		trans_norms[256] ;
		FVECTOR		weight[256] ;
		int			index[256] ;
	} *work ;
	int		count, i ;
	u_long128	*copy_dst, *copy_src ;
	FVECTOR		*tv, *tn, *w ;
	int		*index ;

	work = (void*)SCRPAD_ADDR ;
	while ( n_verts > 0 ){
		count = n_verts ;
		if ( count > 256 ) count = 256 ;
		n_verts -= 256 ;

		/* ウェイト値をメモリからスクラッチパッドへコピー */
		copy_dst = (u_long128*)work->weight ;
		copy_src = (u_long128*)weight ;
		for ( i = count ; i > 0 ; i -= 4 ){
			copy_dst[0] = copy_src[0] ;
			copy_dst[1] = copy_src[1] ;
			copy_dst[2] = copy_src[2] ;
			copy_dst[3] = copy_src[3] ;
			copy_dst += 4 ;
			copy_src += 4 ;
		}
		weight += count ;

		/* マトリクスインデックスをメモリからスクラッチパッドへコピー */
		copy_dst = (u_long128*)work->index ;
		copy_src = (u_long128*)mat_id ;
		for ( i = count ; i > 0 ; i -= 16 ){
			copy_dst[0] = copy_src[0] ;
			copy_dst[1] = copy_src[1] ;
			copy_dst[2] = copy_src[2] ;
			copy_dst[3] = copy_src[3] ;
			copy_dst += 4 ;
			copy_src += 4 ;
		}
		mat_id += count * 4 ;

		/* Ｖｕ０マイクロプログラムを用いてマルチウェイト計算 */
		index = work->index ;
		w = work->weight ;
		tv = work->trans_verts ;
		tn = work->trans_norms ;
		asm ("
			lq			$12,0(%0)
			lq			$13,0(%1)
			lq			$14,0(%2)
			lw			$15,0(%3)
			qmtc2.i		$12,vf1
			qmtc2		$13,vf2
			qmtc2		$14,vf3
			sll			$12,$15,2
			andi		$12,$12,0xfc
			srl			$13,$15,6
			andi		$13,$13,0xfc
			srl			$14,$15,14
			andi		$14,$14,0xfc
			srl			$15,$15,22
			andi		$15,$15,0xfc
			ctc2		$12,$vi1
			ctc2		$13,$vi2
			ctc2		$14,$vi3
			ctc2		$15,$vi4
			vcallms		0x20
		"::"r"(verts++),"r"(norms++),"r"(w++),"r"(index++):
		"$12","$13","$14","$15","$16" );
		for ( i = count - 1; i > 0 ; i -= 1 ){
			asm ("
				lq			$12,0(%0)		# 予めレジスタに読み込んでおく
				lq			$13,0(%1)		#
				lq			$14,0(%2)		#
				lw			$15,0(%3)		#
				qmtc2.i		$12,vf1			# インタロックありで終了までウェイト
				qmtc2		$13,vf2			# 
				qmtc2		$14,vf3			# 
				sll			$12,$15,2		# char×４をマトリクスアドレスに分解して設定
				andi		$12,$12,0xfc	# 
				srl			$13,$15,6		# 
				andi		$13,$13,0xfc	# 
				srl			$14,$15,14		# 
				andi		$14,$14,0xfc	# 
				srl			$15,$15,22		# 
				andi		$15,$15,0xfc	# 
				ctc2		$12,$vi1		# 
				ctc2		$13,$vi2		# 
				ctc2		$14,$vi3		# 
				ctc2		$15,$vi4		# 
				sqc2		vf12,0(%4)
				sqc2		vf13,0(%5)
				vcallms		0x20
			"::"r"(verts++),"r"(norms++),"r"(w++),"r"(index++),"r"(tv++),"r"(tn++):
			"$12","$13","$14","$15","$16" );
		}
		asm ("
			qmtc2.i		$0,vf1
			sqc2		vf12,0(%0)
			sqc2		vf13,0(%1)
		"::"r"(tv++),"r"(tn++) );

		/* 計算した結果をメモリへ書き戻す */
		copy_dst = (u_long128*)res_verts ;
		copy_src = (u_long128*)work->trans_verts ;
		for ( i = count ; i > 0 ; i -= 4 ){
			copy_dst[0] = copy_src[0] ;
			copy_dst[1] = copy_src[1] ;
			copy_dst[2] = copy_src[2] ;
			copy_dst[3] = copy_src[3] ;
			copy_dst += 4 ;
			copy_src += 4 ;
		}
		res_verts += count ;

		copy_dst = (u_long128*)res_norms ;
		copy_src = (u_long128*)work->trans_norms ;
		for ( i = count ; i > 0 ; i -= 4 ){
			copy_dst[0] = copy_src[0] ;
			copy_dst[1] = copy_src[1] ;
			copy_dst[2] = copy_src[2] ;
			copy_dst[3] = copy_src[3] ;
			copy_dst += 4 ;
			copy_src += 4 ;
		}
		res_norms += count ;

	}
}
