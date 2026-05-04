/*
   utl_lzh.h
   
   2000/01/21 T.Morita
   $Id: utl_lzh.h,v 1.1.1.3 2002/11/19 11:43:00 Yoshizawa1 Exp $
*/


/*

  lzshの圧縮形式の解凍関数です。静的辞書をさらにハフマン符号で圧縮した
 もので,圧縮率は,それなりにいいです。しかし,ハフマン符号の解凍に手間が
 かかるため解凍速度は,速いとはいえません。

  ハンドラーは, LZSS辞書を含んだり,解凍を途中から再開できるよう多くの
  変数を持っていますのでサイズは,少し大きいものと考えて下さい。

*/

/* ハンドラー取得 */
extern void *UTL_LzshDecodeInit( void *in, void *out, int size ) ;
/* ハンドラーに登録した<out>に解凍 */
extern int   UTL_LzshStreamDecode( void *handler ) ;
/* ハンドラーの解放 */
extern void  UTL_LzshDecodeFree( void *handler ) ; 


/* ハンドラーに登録した<out>を先頭に戻す */
extern void  UTL_LzshDecodeRewindOut( void *handler ) ;
/* ハンドラーに登録した<in>を先頭に戻す */
extern void  UTL_LzshDecodeRewindIn( void *handler ) ;
/* ハンドラーに登録した<in>と<out>を先頭に戻す */
extern void  UTL_LzshDecodeRewind( void *handler ) ;
/* ハンドラーに登録した<in>と<out>を先頭に戻す */
extern void  UTL_LzshDecodeChangeIn( void *handler, void *in ) ;


/* 一気にすべて解凍する(ハンドラーは要らない) */
extern int   UTL_LzshDecode( void *in, void *out ) ;


#if 0

/* サンプルプログラム */
struct work_t
{
    ...

    u_int *buffer ;
    void  *lzh    ;
} Work ;

static void Act( Work *work )
{
    /* 毎フレーム少しずつ解凍する場合 */
    if ( UTL_LzshStreamDecode( work->lzh ) )   /* 出力バッファに解凍される   */
	UTL_LzshDecodeRewindOut( work->lzh ) ; /* 出力バッファを先頭に巻戻す */
    else
	UTL_LzshDecodeRewind( work->lzh ) ;    /* 入力バッファおよび出力バッファを先頭に巻戻す */

    ...
}
static void Die( Work *work )
{
    UTL_LzshDecodeFree( work->lzh ) ;/*ハンドラーの解放*/
}
static void GetResources( Work *work, int name, int where )
{
    u_int *in = GV_GetCache( GV_CacheID( GV_StrCode( "horo" ), 'a') ) ;

    /* 出力バッファを用意 */
    work->buffer = GV_Malloc( 1024 ) ;
    /* ハンドラーを取得   */
    work->lzh = UTL_LzshDecodeInit( in,           /* 解凍元バッファ */
				    work->buffer, /* 解凍先バッファ */
				    1024 ) ;      /* 1024バイト毎に解凍が止まる */
}

#endif
