/*
	conv.c
	.de2ファイル 圧縮処理
	$Id: conv.c,v 1.5 2001/08/07 11:31:49 usr02774 Exp $

 */


#include <stdio.h>
#include <stdlib.h>

typedef struct {
	float	vx, vy, vz, vw ;
} FVECTOR ;
typedef struct {
	short	vx, vy, vz, pad ;
} SVECTOR ;
#include "fmt_demo.h"

#include "stream.h"

/* ---------------------------------------------------------------- */
/* プロトタイプ宣言 */
void InitMotionCache( void );
void FreeMotionCache( void );
DEMO_MOTION* GetMotionCache( DEMO_MOTION *motion );
static unsigned short FtoMF( float f );
static float MFtoF( unsigned short mf );
void *MotionPacketUncompress( DEMO_MOTION *packet );



/* ---------------------------------------------------------------- */
/* １ストリーミング分作業バッファ */
static int		FrameBufferSize ;
static char	*FrameBuffer ;
static char	*CopyBuffer ;

/* ---------------------------------------------------------------- */
static dump_b( void *p_data, int size )
{
	unsigned char	*data = p_data ;
	int				i ;
	for ( i = 0 ; i < size ; i++ ){
		if ( ( i & 15 ) == 0 ) printf("%08x:", data );
		printf("%02x ", *data & 0xff );
		data++ ;
		if ( ( i & 15 ) == 15 ) printf("\n");
	}
	if ( i != 0 ) printf("\n");
}
static dump_h( void *p_data, int size )
{
	unsigned short	*data = p_data ;
	int				i ;
	for ( i = 0 ; i < size ; i++ ){
		if ( ( i & 7 ) == 0 ) printf("%08x:", data );
		printf("%04x ", *data & 0xffff );
		data++ ;
		if ( ( i & 7 ) == 7 ) printf("\n");
	}
	if ( i != 0 ) printf("\n");
}
static dump_w( void *p_data, int size )
{
	unsigned int	*data = p_data ;
	int				i ;
	for ( i = 0 ; i < size ; i++ ){
		if ( ( i & 7 ) == 0 ) printf("%08x:", data );
		printf("%08x ", *data & 0xffffffff );
		data++ ;
		if ( ( i & 7 ) == 7 ) printf("\n");
	}
	if ( i != 0 ) printf("\n");
}
/* ---------------------------------------------------------------- */
	/*
		モーションキャッシュ関連ルーチン
	*/

#define MAX_MOTIONS		(256)
static int		MaxMotions = 0 ;
static DEMO_MOTION	*MotionList[MAX_MOTIONS] ;
static int		MaxMotionsDecode = 0 ;
static DEMO_MOTION	*MotionListDecode[MAX_MOTIONS] ;


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
#if 0
		if ( motion->motion_type & DEMO_MOTION_ROT ){
			size += sizeof(FVECTOR) * motion->n_joints ;
		}
		if ( motion->motion_type & DEMO_MOTION_TRANS ){
			size += sizeof(FVECTOR) * motion->n_joints ;
		}
#else
		size += sizeof(FVECTOR) * motion->n_joints * 2 ;
#endif
		MotionList[ MaxMotions++ ] = prev_motion = malloc( size );
		*prev_motion = *motion ;
		prev_motion->motion_type = DEMO_MOTION_ROT|DEMO_MOTION_TRANS ;
		vec = prev_motion->motion ;
		for ( i = 0 ; i < motion->n_joints ; i++ ){
#if 0
			if ( prev_motion->motion_type & DEMO_MOTION_ROT ){
				vec->vx = 0.0f ; vec->vy = 0.0f ; vec->vz = 0.0f ; vec->vw = 1.0f ;
				vec++ ;
			}
			if ( prev_motion->motion_type & DEMO_MOTION_TRANS ){
				vec->vx = 0.0f ; vec->vy = 0.0f ; vec->vz = 0.0f ; vec->vw = 1.0f ;
				vec++ ;
			}
#else
			vec->vx = 0.0f ; vec->vy = 0.0f ; vec->vz = 0.0f ; vec->vw = 1.0f ;
			vec++ ;
			vec->vx = 0.0f ; vec->vy = 0.0f ; vec->vz = 0.0f ; vec->vw = 1.0f ;
			vec++ ;
#endif
		}
		prev_motion->header.size = size ;
		printf("create new motion buffer\n");
	}

	return ( prev_motion );
}

void InitMotionCacheDecode( void )
{
	MaxMotionsDecode =  0 ;
}

void FreeMotionCacheDecode( void )
{
	int			i ;
	for ( i = 0 ; i < MaxMotionsDecode ; i++ ){
		free( MotionListDecode[ i ] );
	}
}

DEMO_MOTION* GetMotionCacheDecode( DEMO_MOTION *motion )
{
	DEMO_MOTION		*prev_motion = NULL ;
	int				i, size ;

	/* 同じモーションを検索 */
	for ( i = 0 ; i < MaxMotionsDecode ; i++ ){
		if ( MotionListDecode[i]->object_id == motion->object_id ){
			prev_motion = MotionListDecode[i] ;
			break ;
		}
	}
	/* 該当モーションがない場合には新規作成 */
	if ( prev_motion == NULL ){
		FVECTOR		*vec ;

		size = sizeof(DEMO_MOTION) ;
#if 0
		if ( motion->motion_type & DEMO_MOTION_ROT ){
			size += sizeof(FVECTOR) * motion->n_joints ;
		}
		if ( motion->motion_type & DEMO_MOTION_TRANS ){
			size += sizeof(FVECTOR) * motion->n_joints ;
		}
#else
		size += sizeof(FVECTOR) * motion->n_joints * 2 ;
#endif
		MotionListDecode[ MaxMotionsDecode++ ] = prev_motion = malloc( size );
		*prev_motion = *motion ;
		prev_motion->motion_type = DEMO_MOTION_ROT|DEMO_MOTION_TRANS ;
		vec = prev_motion->motion ;
		for ( i = 0 ; i < motion->n_joints ; i++ ){
#if 0
			if ( prev_motion->motion_type & DEMO_MOTION_ROT ){
				vec->vx = 0.0f ; vec->vy = 0.0f ; vec->vz = 0.0f ; vec->vw = 1.0f ;
				vec++ ;
			}
			if ( prev_motion->motion_type & DEMO_MOTION_TRANS ){
				vec->vx = 0.0f ; vec->vy = 0.0f ; vec->vz = 0.0f ; vec->vw = 1.0f ;
				vec++ ;
			}
#else
			vec->vx = 0.0f ; vec->vy = 0.0f ; vec->vz = 0.0f ; vec->vw = 1.0f ;
			vec++ ;
			vec->vx = 0.0f ; vec->vy = 0.0f ; vec->vz = 0.0f ; vec->vw = 1.0f ;
			vec++ ;
#endif
		}
		prev_motion->header.size = size ;
		printf("create new motion buffer(%08x)\n", prev_motion );
	}

	return ( prev_motion );
}

/* ---------------------------------------------------------------- */
	/*
		モーションパケット圧縮処理
		（確保済メモリ領域dst_motionに対して圧縮モーションをsrc_motionから生成）
	*/
#define QUAT_INT(_f)	((int)((_f) * 16383.0f))
void MotionPacketCompress( DEMO_MOTION *dst_motion, DEMO_MOTION *org_motion )
{
	FVECTOR		*src, *old ;
	short		*dst, data, *last_flag = NULL;
	int			count, i, flag, size ;
	SVECTOR		srot, strans, last_srot, last_strans ;
	DEMO_MOTION	*old_motion ;
	DEMO_MOTION	*src_motion ;
#if 0
	/* すでに圧縮モーションデータであればコピーして終了 */
	if ( src_motion->motion_type == DEMO_MOTION_COMPRESS ){
		printf("compressed data...\n");
		memcpy( dst_motion, src_motion, src_motion->header.size );
		return ;
	}
#endif
	src_motion = MotionPacketUncompress( org_motion );


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
			if ( QUAT_INT(old->vx) == QUAT_INT(src->vx) ) flag |= MOTIONDIET_ROT_X ;
			srot.vx = (int)( src->vx * 16383.0f );
			/* Y */
			if ( QUAT_INT(old->vy) == QUAT_INT(src->vy) ) flag |= MOTIONDIET_ROT_Y ;
			srot.vy = (int)( src->vy * 16383.0f );
			/* Z */
			if ( QUAT_INT(old->vz) == QUAT_INT(src->vz) ) flag |= MOTIONDIET_ROT_Z ;
			srot.vz = (int)( src->vz * 16383.0f );
			/* W */
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
			if ( FtoMF(old->vx) == FtoMF(src->vx) ) flag |= MOTIONDIET_TRANS_X ;
			strans.vx = FtoMF( src->vx );
			/* Y */
			if ( FtoMF(old->vy) == FtoMF(src->vy) ) flag |= MOTIONDIET_TRANS_Y ;
			strans.vy = FtoMF( src->vy );
			/* Z */
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
	printf("\n");

	//free( src_motion );
}

void *MotionPacketUncompress( DEMO_MOTION *packet )
{
	int			i ;
	const FVECTOR	zero_vector = {0,0,0,1};
	DEMO_MOTION	*expand_motion = NULL ;

	/* 展開ワークの新規作成 */
	expand_motion = GetMotionCacheDecode( packet );

	/* 無圧縮用モーション用処理 */
	if ( packet->motion_type != DEMO_MOTION_COMPRESS ){
		FVECTOR		*src, *dst ;

		/* モーション以外のデータをコピー */
		expand_motion->pos = packet->pos ;
		expand_motion->rot = packet->rot ;

		src = packet->motion ;
		dst = expand_motion->motion ;
		for ( i = 0 ; i < packet->n_joints ; i++ ){
			if ( packet->motion_type & DEMO_MOTION_ROT ){
				*dst++ = *src++ ;
			} else {
				*dst++ = zero_vector ;
			}
			if ( packet->motion_type & DEMO_MOTION_TRANS ){
				*dst++ = *src++ ;
			} else {
				*dst++ = zero_vector ;
			}
		}
		return ( expand_motion ) ;
	}

	{/* モーション展開処理 */
		FVECTOR		*dst ;
		short		*src, flag, *data ;
		int			count ;

		/* モーション以外のデータをコピー */
		expand_motion->pos = packet->pos ;
		expand_motion->rot = packet->rot ;

		src = (short*)packet->motion ;
		dst = expand_motion->motion ;

		flag = *src ;			/* 展開フラグ */
		count = *src & 0xff ;	/* 展開数 */
		for ( i = 0 ; i < packet->n_joints ; i++ ){
			data = &src[ 1 ];
			if ( !( flag & MOTIONDIET_ROT_X ) ){
				dst->vx = ( (float)*data++ ) / 16383.0f ;
			}
			if ( !( flag & MOTIONDIET_ROT_Y ) ){
				dst->vy = ( (float)*data++ ) / 16383.0f ;
			}
			if ( !( flag & MOTIONDIET_ROT_Z ) ){
				dst->vz = ( (float)*data++ ) / 16383.0f ;
			}
			if ( !( flag & MOTIONDIET_ROT_W ) ){
				dst->vw = ( (float)*data++ ) / 16383.0f ;
			}
			//MT_QuatNormalize( dst, dst );
			dst++ ;
			if ( !( flag & MOTIONDIET_TRANS_X ) ){
				dst->vx = MFtoF( *data ) ;
				data++ ;
			}
			if ( !( flag & MOTIONDIET_TRANS_Y ) ){
				dst->vy = MFtoF( *data ) ;
				data++ ;
			}
			if ( !( flag & MOTIONDIET_TRANS_Z ) ){
				dst->vz = MFtoF( *data ) ;
				data++ ;
			}
			dst++ ;
			if ( --count <= 0 ){
				src = data ;
				flag = *src ;			/* 展開フラグ */
				count = *src & 0xff ;	/* 展開数 */
			}
		}
	}

	return ( expand_motion );

}
/* ---------------------------------------------------------------- */
	/*
		モーションパケット圧縮処理
		（確保済メモリ領域dst_motionに対して圧縮モーションをsrc_motionから生成）
	*/
//#define QUAT_INT(_f)	((int)((_f) * 32767.0f))
#define QUAT_INT(_f)	((int)((_f) * 16383.0f))
void *MotionPacketCompress2( void *write_buffer, DEMO_MOTION_INFO *info, DEMO_MOTION *src_motion )
{
	FVECTOR		*src, *old ;
	short		*dst, data, *last_flag = NULL;
	int			count, i, flag, size ;
	SVECTOR		srot, strans, last_srot, last_strans ;
	DEMO_MOTION	*old_motion ;

	/* 関節情報以外のデータをコピーする */
	memcpy( write_buffer, &src_motion->pos, sizeof(FVECTOR) );
	write_buffer = (void*)( (int)write_buffer + sizeof(FVECTOR) );
	memcpy( write_buffer, &src_motion->rot, sizeof(SVECTOR) );
	write_buffer = (void*)( (int)write_buffer + sizeof(SVECTOR) );

	old_motion = GetMotionCache( src_motion );
	src = src_motion->motion ;
	dst = (short*)write_buffer ;
	old = old_motion->motion ;
	count = 0 ;
	for ( i = 0 ; i < src_motion->n_joints ; i++ ){
		/* フラグの初期化 */
		flag = 0 ;
		/* 回転データの整数化＆省略フラグチェック */
		if ( src_motion->motion_type & DEMO_MOTION_ROT ){
			/* X */
			if ( QUAT_INT(old->vx) == QUAT_INT(src->vx) ) flag |= MOTIONDIET_ROT_X ;
			srot.vx = QUAT_INT(src->vx);
			/* Y */
			if ( QUAT_INT(old->vy) == QUAT_INT(src->vy) ) flag |= MOTIONDIET_ROT_Y ;
			srot.vy = QUAT_INT(src->vy);
			/* Z */
			if ( QUAT_INT(old->vz) == QUAT_INT(src->vz) ) flag |= MOTIONDIET_ROT_Z ;
			srot.vz = QUAT_INT(src->vz);
			/* W */
			if ( QUAT_INT(old->vw) == QUAT_INT(src->vw) ) flag |= MOTIONDIET_ROT_W ;
			srot.pad = QUAT_INT(src->vw);
			/* ポインタインクリメント */
			src++ ;
			old++ ;
		} else {
			flag |= MOTIONDIET_ROT_X | MOTIONDIET_ROT_Y | MOTIONDIET_ROT_Z | MOTIONDIET_ROT_W ;
		}
		/* 移動量データの整数化＆省略フラグチェック */
		if ( src_motion->motion_type & DEMO_MOTION_TRANS ){
			/* X */
			if ( FtoMF(old->vx) == FtoMF(src->vx) ) flag |= MOTIONDIET_TRANS_X ;
			strans.vx = FtoMF( src->vx );
			/* Y */
			if ( FtoMF(old->vy) == FtoMF(src->vy) ) flag |= MOTIONDIET_TRANS_Y ;
			strans.vy = FtoMF( src->vy );
			/* Z */
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
					//printf(".");
					continue ;
				}
			}
		}

		//if ( flag & 0xff00 )	printf("+");
		//else					printf("*");
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
	//printf("\n");

	/* 最終モーションを保存する */
	memcpy( old_motion, src_motion, src_motion->header.size );

	return ( dst );
}

/* ---------------------------------------------------------------- */
	/*
		.de2ファイルの圧縮処理
	*/
int DietDe2( char *out_filename, char *in_filename )
{
	FILE	*ifp, *ofp ;
	int		end_flag = 0, motion_flag ;
	/* new motion compress */
	int		n_motions ;
	DEMO_MOTION_INFO	motion_infos[MAX_MOTIONS] ;
	char	*spool_buffer, *write_buffer ;
	int		spool_buffer_offset ;

	/* ファイルのオープン処理 */
	if ( ( ifp = fopen( in_filename, "rb" ) ) == NULL ){
		printf("in-file open error!! (%s)\n", in_filename );
		return ( -1 );
	}
	if ( ( ofp = fopen( out_filename, "wb" ) ) == NULL ){
		printf("out-file open error!! (%s)\n", out_filename );
		fclose( ifp );
		return ( -1 );
	}
	/* １フレーム分のデータを貯めておくためのバッファを確保 */
	FrameBuffer = malloc( 1 * 1024 * 1024 );
	memset( FrameBuffer, 0, 1 * 1024 * 1024 );
	CopyBuffer = malloc( 1 * 1024 * 1024 );
	memset( CopyBuffer, 0, 1 * 1024 * 1024 );
	/* new motion compress */
	spool_buffer = malloc( 1024 * 128 );
	memset( spool_buffer, 0, 1024 * 128 );

	/* モーションキャッシュ初期化 */
	InitMotionCache();
	InitMotionCacheDecode();

	/* メインループ */
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
		/* new motion compression initialize */
		n_motions = 0 ;
		write_buffer = spool_buffer ;
		spool_buffer_offset = 0 ;
		motion_flag = 0 ;
		while ( 1 ){
			int		packet_type ;
			//printf("packet type:%08x\n", src_packet->packet_type );
			packet_type = src_packet->packet_type & 0x0000ffff ;
			switch ( packet_type ){
			case DEMO_PACKET_MOTION:
#if 0
				MotionPacketCompress( (DEMO_MOTION*)dst_packet, (DEMO_MOTION*)src_packet );
				//printf("%d to %d \n", src_packet->size, dst_packet->size );
				if ( src_packet->size < dst_packet->size ){
					DEMO_MOTION	*motion ;
					motion = (DEMO_MOTION*)src_packet ;
					printf("src:%d %d %d %04x\n",
						   src_packet->size,
						   motion->object_id, motion->n_joints, motion->motion_type );
					dump_h( motion->motion, 16 );
					motion = (DEMO_MOTION*)dst_packet ;
					printf("dst:%d %d %d %04x\n",
						   dst_packet->size,
						   motion->object_id, motion->n_joints, motion->motion_type );
					dump_h( motion->motion, 16 );
				}
#else
				{
					DEMO_MOTION		*motion_packet ;
					motion_packet = MotionPacketUncompress( (DEMO_MOTION*)src_packet );
					motion_infos[ n_motions ].object_id = motion_packet->object_id ;
					motion_infos[ n_motions ].offset = spool_buffer_offset ;
					motion_infos[ n_motions ].start_joint = motion_packet->start_joint ;
					motion_infos[ n_motions ].n_joints = motion_packet->n_joints ;
					motion_infos[ n_motions ].frame = 1 ;
					motion_infos[ n_motions ].flag = DEMO_MOTION2_MOV|DEMO_MOTION2_ROT|DEMO_MOTION2_ROTS|DEMO_MOTION2_TRANS ;
					write_buffer = MotionPacketCompress2( write_buffer, &motion_infos[ n_motions ], motion_packet );
					//free( motion_packet );
					spool_buffer_offset = (int)write_buffer - (int)spool_buffer ;
				}
				n_motions++ ;
				motion_flag = 1 ;
				src_packet = (DEMO_PACKET*)( (int)src_packet + src_packet->size );
				continue ;
#endif
				break ;
			default:
				if ( motion_flag ){
					DEMO_MOTION_PACK	*motion_packet ;
					int			i, size ;
					size = sizeof(DEMO_MOTION_PACK) + sizeof(DEMO_MOTION_INFO) * n_motions ;
					for ( i = 0 ; i < n_motions ; i++ ){
						motion_infos[ i ].offset += size ;
					}
					motion_packet = (DEMO_MOTION_PACK*)dst_packet ;
					dst_packet->packet_type = DEMO_PACKET_MOTION_PACK | DEMO_PACKET_NOSKIP ;
					dst_packet->size = ( size + spool_buffer_offset + 15 ) & ~15 ;
					dst_packet->id = 0 ;
					dst_packet->reserved = 0 ;
					motion_packet->n_lists = n_motions ;
					motion_packet->data_offset = size ;
					memcpy( motion_packet->lists, motion_infos, sizeof(DEMO_MOTION_INFO) * n_motions );
					memcpy( (void*)((int)motion_packet + size ), spool_buffer, spool_buffer_offset );
					printf("%d motions , size %d(0x%04x)\n", n_motions, dst_packet->size, dst_packet->size );

					FrameBufferSize += dst_packet->size ;
					dst_packet = (DEMO_PACKET*)( (int)dst_packet + dst_packet->size );
					motion_flag = 0 ;
				}
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

	/* モーションキャッシュ解放 */
	FreeMotionCacheDecode();
	FreeMotionCache();


	/* 終了処理 */
	free( FrameBuffer );
	free( CopyBuffer );
	free( spool_buffer );

	fclose( ofp );
	fclose( ifp );

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




