/*
	conv.c
	.mtn to .mst 変換プログラム
	$Id: conv.c,v 1.4 2001/07/27 07:48:01 usr02774 Exp $

 */


#include <stdio.h>
#include <stdlib.h>

#if 0
typedef struct {
	float	vx, vy, vz, vw ;
} FVECTOR ;
typedef struct {
	short	vx, vy, vz, pad ;
} SVECTOR ;
#endif
#include "gte.h"
#include "quat.h"
#include "fmt_demo.h"

#include "stream.h"
#include "motion.h"
#include "main.h"


/* ---------------------------------------------------------------- */
/* プロトタイプ宣言 */
void InitMotionCache( void );
void FreeMotionCache( void );
DEMO_MOTION* GetMotionCache( DEMO_MOTION *motion );
static unsigned short FtoMF( float f );
static float MFtoF( unsigned short mf );



/* ---------------------------------------------------------------- */
/* １ストリーミング分作業バッファ */
static int		FrameBufferSize ;
static char	*FrameBuffer ;
static char	*CopyBuffer ;

/* ---------------------------------------------------------------- */
	/*
		モーションキャッシュ関連ルーチン
	*/

#define MAX_MOTIONS		(256)
static int		MaxMotions = 0 ;
static DEMO_MOTION	*MotionList[MAX_MOTIONS] ;


void InitMotionCache( void )
{
	MaxMotions =  0 ;
}

void FreeMotionCache( void )
{
	int			i ;
	for ( i = 0 ; i < MaxMotions ; i++ ){
		free( MotionList[ i ] );
	}
}

DEMO_MOTION* GetMotionCache( DEMO_MOTION *motion )
{
	DEMO_MOTION		*prev_motion = NULL ;
	int				i, size ;

	/* 同じモーションを検索 */
	for ( i = 0 ; i < MaxMotions ; i++ ){
		if ( MotionList[i]->object_id == motion->object_id ){
			prev_motion = MotionList[i] ;
			break ;
		}
	}
	/* 該当モーションがない場合には新規作成 */
	if ( prev_motion == NULL ){
		FVECTOR		*vec ;

		size = sizeof(DEMO_MOTION) ;
		if ( motion->motion_type & DEMO_MOTION_ROT ){
			size += sizeof(FVECTOR) * motion->n_joints ;
		}
		if ( motion->motion_type & DEMO_MOTION_TRANS ){
			size += sizeof(FVECTOR) * motion->n_joints ;
		}
		MotionList[ MaxMotions++ ] = prev_motion = malloc( size );
		*prev_motion = *motion ;
		vec = prev_motion->motion ;
		for ( i = 0 ; i < motion->n_joints ; i++ ){
			if ( motion->motion_type & DEMO_MOTION_ROT ){
				vec->vx = 0.0f ; vec->vy = 0.0f ; vec->vz = 0.0f ; vec->vw = 1.0f ;
				vec++ ;
			}
			if ( motion->motion_type & DEMO_MOTION_TRANS ){
				vec->vx = 0.0f ; vec->vy = 0.0f ; vec->vz = 0.0f ; vec->vw = 1.0f ;
				vec++ ;
			}
		}
		printf("create new motion buffer\n");
	}

	return ( prev_motion );
}

/* ---------------------------------------------------------------- */
	/*
		モーションパケット圧縮処理
		（確保済メモリ領域dst_motionに対して圧縮モーションをsrc_motionから生成）
	*/
void MotionPacketCompress( DEMO_MOTION *dst_motion, DEMO_MOTION *src_motion )
{
	FVECTOR		*src, *old ;
	short		*dst, data, *last_flag = NULL;
	int			count, i, flag, size ;
	SVECTOR		srot, strans, last_srot, last_strans ;
	DEMO_MOTION	*old_motion ;

	/* すでに圧縮モーションデータであればコピーして終了 */
	if ( src_motion->motion_type == DEMO_MOTION_COMPRESS ){
		printf("compressed data...\n");
		memcpy( dst_motion, src_motion, src_motion->header.size );
		return ;
	}

	/* 関節情報以外のデータをコピーする */
	memcpy( dst_motion, src_motion, sizeof(DEMO_MOTION) );

	old_motion = GetMotionCache( src_motion );
	src = src_motion->motion ;
	dst = (short*)dst_motion->motion ;
	old = old_motion->motion ;
	count = 0 ;
	for ( i = 0 ; i < dst_motion->n_joints ; i++ ){
		/* フラグの初期化 */
		flag = 0 ;
		/* 回転データの整数化＆省略フラグチェック */
		if ( src_motion->motion_type & DEMO_MOTION_ROT ){
			/* X */
#define QUAT_INT(_f)	((int)((_f) * 16383.0f))
			//if ( old->vx == src->vx ) flag |= MOTIONDIET_ROT_X ;
			if ( QUAT_INT(old->vx) == QUAT_INT(src->vx) ) flag |= MOTIONDIET_ROT_X ;
			srot.vx = (int)( src->vx * 16383.0f );
			/* Y */
			//if ( old->vy == src->vy ) flag |= MOTIONDIET_ROT_Y ;
			if ( QUAT_INT(old->vy) == QUAT_INT(src->vy) ) flag |= MOTIONDIET_ROT_Y ;
			srot.vy = (int)( src->vy * 16383.0f );
			/* Z */
			//if ( old->vz == src->vz ) flag |= MOTIONDIET_ROT_Z ;
			if ( QUAT_INT(old->vz) == QUAT_INT(src->vz) ) flag |= MOTIONDIET_ROT_Z ;
			srot.vz = (int)( src->vz * 16383.0f );
			/* W */
			//if ( old->vw == src->vw ) flag |= MOTIONDIET_ROT_W ;
			if ( QUAT_INT(old->vw) == QUAT_INT(src->vw) ) flag |= MOTIONDIET_ROT_W ;
			srot.pad = (int)( src->vw * 16383.0f );
			/* ポインタインクリメント */
			src++ ;
			old++ ;
		} else {
			flag |= MOTIONDIET_ROT_X | MOTIONDIET_ROT_Y | MOTIONDIET_ROT_Z | MOTIONDIET_ROT_W ;
		}
		/* 移動量データの整数化＆省略フラグチェック */
		if ( src_motion->motion_type & DEMO_MOTION_TRANS ){
			/* X */
			//if ( old->vx == src->vx ) flag |= MOTIONDIET_TRANS_X ;
			if ( FtoMF(old->vx) == FtoMF(src->vx) ) flag |= MOTIONDIET_TRANS_X ;
			strans.vx = FtoMF( src->vx );
			/* Y */
			//if ( old->vy == src->vy ) flag |= MOTIONDIET_TRANS_Y ;
			if ( FtoMF(old->vy) == FtoMF(src->vy) ) flag |= MOTIONDIET_TRANS_Y ;
			strans.vy = FtoMF( src->vy );
			/* Z */
			//if ( old->vz == src->vz ) flag |= MOTIONDIET_TRANS_Z ;
			if ( FtoMF(old->vz) == FtoMF(src->vz) ) flag |= MOTIONDIET_TRANS_Z ;
			strans.vz = FtoMF( src->vz );
			/* ポインタインクリメント */
			src++ ;
			old++ ;
		} else {
			flag |= MOTIONDIET_TRANS_X | MOTIONDIET_TRANS_Y | MOTIONDIET_TRANS_Z ;
		}

		/* バッファへの書き込み処理 */
		if ( last_flag != NULL ){
			if ( ( *last_flag & 0xff00 ) == ( flag & 0xff00 ) ){
				int		f = 0 ;
				/* フラグが同じだった場合にチェック */
				if ( !( flag & MOTIONDIET_ROT_X ) ) f += last_srot.vx != srot.vx ;
				if ( !( flag & MOTIONDIET_ROT_Y ) ) f += last_srot.vy != srot.vy ;
				if ( !( flag & MOTIONDIET_ROT_Z ) ) f += last_srot.vz != srot.vz ;
				if ( !( flag & MOTIONDIET_ROT_W ) ) f += last_srot.pad != srot.pad ;
				if ( !( flag & MOTIONDIET_TRANS_X ) ) f += last_strans.vx != strans.vx ;
				if ( !( flag & MOTIONDIET_TRANS_Y ) ) f += last_strans.vy != strans.vy ;
				if ( !( flag & MOTIONDIET_TRANS_Z ) ) f += last_strans.vz != strans.vz ;
				if ( f == 0 ){
					*last_flag += 1 ;
					printf(".");
					continue ;
				}
			}
		}

		if ( flag & 0xff00 )	printf("+");
		else					printf("*");
		last_flag = dst ;
		*dst = 1 ;
		*dst |= flag ;
		dst++ ;
		if ( !( flag & MOTIONDIET_ROT_X ) ) *dst++ = srot.vx ;
		if ( !( flag & MOTIONDIET_ROT_Y ) ) *dst++ = srot.vy ;
		if ( !( flag & MOTIONDIET_ROT_Z ) ) *dst++ = srot.vz ;
		if ( !( flag & MOTIONDIET_ROT_W ) ) *dst++ = srot.pad ;
		if ( !( flag & MOTIONDIET_TRANS_X ) ) *dst++ = strans.vx ;
		if ( !( flag & MOTIONDIET_TRANS_Y ) ) *dst++ = strans.vy ;	
		if ( !( flag & MOTIONDIET_TRANS_Z ) ) *dst++ = strans.vz ;
		last_srot = srot ;
		last_strans = strans ;
	}

	/* 最終モーションを保存する */
	memcpy( old_motion, src_motion, src_motion->header.size );

	/* サイズ計算 */
	size = (int)dst - (int)dst_motion->motion ;
	size = ( size + 15 ) & 0xffffff0 ;
	dst_motion->header.packet_type |= DEMO_PACKET_NOSKIP ;
	dst_motion->header.size = size + sizeof(DEMO_MOTION) ;
	dst_motion->motion_type = DEMO_MOTION_COMPRESS ;
	printf("(%d,%d)\n", size, dst_motion->header.size);
}

/* ---------------------------------------------------------------- */
	/*
		.mtnファイルの圧縮＆ストリーム化処理
	*/
int Convert( char *out_filename, PROGRAM_OPTION *option )
{
	static FVECTOR		zero_vec = {0,0,0,1};
	static SVECTOR		zero_svec = {0,0,0,1};
	FILE	*ofp ;
	int		end_flag = 0, i, j, max_frame = 0, n_motions ;
	FVECTOR	qrots[128], trans[128] ;

	/* ファイルのオープン処理 */
	n_motions = option->n_datas ;
	for ( i = 0 ; i < n_motions ; i++ ){
		MTN_MOTION	*motion ;
		motion = option->input_data[i].motion = MTN_OpenMTNFile( option->input_data[i].filename );
		if ( option->input_data[i].motion == NULL ){
			printf("file open error!!(%s)\n", option->input_data[i].filename );
			return ( -1 );
		}
		if ( motion->header.motion_length > max_frame ) max_frame = motion->header.motion_length ;
	}
	if ( ( ofp = fopen( out_filename, "wb" ) ) == NULL ){
		printf("out-file open error!! (%s)\n", out_filename );
		return ( -1 );
	}
	/* １フレーム分のデータを貯めておくためのバッファを確保 */
	FrameBuffer = malloc( 2 * 1024 * 1024 );
	memset( FrameBuffer, 0, 2 * 1024 * 1024 );
	CopyBuffer = malloc( 2 * 1024 * 1024 );
	memset( CopyBuffer, 0, 2 * 1024 * 1024 );

	/* モーションキャッシュ初期化 */
	InitMotionCache();

	/* メインループ */
	for ( i = 0 ; i < max_frame ; i++ ){
		STREAM_TAG	stream_tag ;
		MTN_MOTION	*motion ;
		DEMO_PACKET	*dst_packet ;

		stream_tag.type = 0 ;
		stream_tag.size = 0 ;
		stream_tag.time = i * option->base_tick ;/* NTSC = 5, PAL = 6 */
		stream_tag.option = 0 ;

		FrameBufferSize = 0 ;
		dst_packet = (DEMO_PACKET*)FrameBuffer ;
		/* 各モーションの書きだし処理 */
		for ( j = 0 ; j < n_motions ; j++ ){
			int			k, size ;
			FVECTOR		mov ;
			DEMO_MOTION	*packet ;

			motion = option->input_data[j].motion ;
			if ( i >= motion->header.motion_length ) continue ;
			/* 無圧縮モーションパケットの生成 */
			size = sizeof(DEMO_MOTION) + sizeof(FVECTOR) * 2 * motion->header.motion_joints ;
			packet = malloc( size );
			packet->header.packet_type = DEMO_PACKET_MOTION ;
			packet->header.size = size ;
			packet->header.id = j ;
			packet->header.reserved = 0 ;
			packet->object_id = option->input_data[j].strcode ;
			packet->motion_type = DEMO_MOTION_ROTTRANS ;
			packet->start_joint = 0 ;
			packet->n_joints = motion->header.motion_joints ;
			packet->pos = mov ;
			packet->rot = zero_svec ;
			packet->pad[0] = 0 ;
			packet->pad[1] = 0 ;
			/* モーションデータの設定 */
			MTN_GetMotionData( motion, i, &mov, trans, qrots );
			for ( k = 0 ; k < motion->header.motion_joints ; k++ ){
				packet->motion[ k * 2 + 0 ] = qrots[k] ;
				packet->motion[ k * 2 + 1 ] = trans[k] ;
			}
			MotionPacketCompress( (DEMO_MOTION*)dst_packet, (DEMO_MOTION*)packet );
			FrameBufferSize += dst_packet->size ;
			dst_packet = (DEMO_PACKET*)( (int)dst_packet + dst_packet->size );
		}
		{/* 終端パケットの書き込み */
			DEMO_PACKET	packet ;
			packet.packet_type = DEMO_PACKET_TERMINATE_FRAME ;
			packet.size = sizeof(DEMO_PACKET) ;
			packet.id = 0 ;
			packet.reserved = 0 ;
			memcpy( dst_packet, &packet, packet.size );
			FrameBufferSize += dst_packet->size ;
			dst_packet = (DEMO_PACKET*)( (int)dst_packet + dst_packet->size );
		}
		stream_tag.size = FrameBufferSize + sizeof(STREAM_TAG) ;
		fwrite( &stream_tag, sizeof(STREAM_TAG), 1, ofp );
		fwrite( FrameBuffer, 1, FrameBufferSize, ofp );
	}
#if 0
	while ( feof( ifp ) == 0 && end_flag == 0 ){
		STREAM_TAG	stream_tag ;
		DEMO_PACKET	*src_packet, *dst_packet ;

		/* ストリームデータ読み込み */
		fread( &stream_tag, sizeof(STREAM_TAG), 1, ifp );
		fread( CopyBuffer, 1, stream_tag.size - sizeof(STREAM_TAG), ifp );
		printf("stream data (id:%02x time:%d size:%d option:%08x)\n",
			   stream_tag.type, stream_tag.time, stream_tag.size, stream_tag.option );

#if 0
		/* ストリームデータ解析 */
		if ( stream_tag.type != CHANK_TYPE_DEMO ){
			printf("not demo stream data (%02x)\n", stream_tag.type );
			fwrite( &stream_tag, sizeof(STREAM_TAG), 1, ofp );
			fwrite( CopyBuffer, 1, stream_tag.size - sizeof(STREAM_TAG), ofp );
			if ( stream_tag.type == CHANK_TYPE_END ){
				printf("stream data end");
				break ;
			}
			continue ;
		}
		//printf("demo stream data (%02x)\n", stream_tag.type );
#endif

		/* デモパケット解析 */
		FrameBufferSize = 0 ;
		src_packet = (DEMO_PACKET*)CopyBuffer ;
		dst_packet = (DEMO_PACKET*)FrameBuffer ;
		while ( 1 ){
			int		packet_type ;
			//printf("packet type:%08x\n", src_packet->packet_type );
			packet_type = src_packet->packet_type & 0x0000ffff ;
			switch ( packet_type ){
			case DEMO_PACKET_MOTION:
				MotionPacketCompress( (DEMO_MOTION*)dst_packet, (DEMO_MOTION*)src_packet );
				break ;
			default:
				memcpy( dst_packet, src_packet, src_packet->size );
				break ;
			}
			FrameBufferSize += dst_packet->size ;
			dst_packet = (DEMO_PACKET*)( (int)dst_packet + dst_packet->size );
			src_packet = (DEMO_PACKET*)( (int)src_packet + src_packet->size );
			if ( packet_type == DEMO_PACKET_TERMINATE_SEQUENCE ) end_flag = 1 ;
			if ( packet_type == DEMO_PACKET_TERMINATE_FRAME ) break ;
		}
		stream_tag.size = FrameBufferSize + sizeof(STREAM_TAG) ;
		fwrite( &stream_tag, sizeof(STREAM_TAG), 1, ofp );
		fwrite( FrameBuffer, 1, FrameBufferSize, ofp );
		
	}
#endif

	/* モーションキャッシュ解放 */
	FreeMotionCache();

	/* 終了処理 */
	free( FrameBuffer );
	free( CopyBuffer );
	for ( i = 0 ; i < option->n_datas ; i++ ){
		if ( option->input_data[i].motion != NULL ){
			MTN_FreeMotion( option->input_data[i].motion );
		}
	}
	fclose( ofp );

}



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




