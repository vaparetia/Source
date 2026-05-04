/*
  utl_eft_bound.h
  エフェクトバウンドチェック

  2001/06/08 T. Morita
  $Id: utl_eft_bound.h,v 1.1.1.3 2002/11/19 11:43:00 Yoshizawa1 Exp $
*/

/*
  リストやバウンドの初期化
*/
extern void UTL_EFT_InitBound() ;
 
/*
  バウンドの個数を返します
*/
extern int UTL_EFT_NBound() ;

/*
  バウンドの情報を返します
*/
extern FVECTOR *UTL_EFT_GetBound( int idx ) ;

/*
  コールバック関数を返します
*/
extern void *UTL_EFT_GetFunc( int idx ) ;

/*
  全てのコールバック関数を実行します。
*/
void UTL_EFT_ExecCallback() ;

/*
  リストからコールバック関数を削除します。

  返り値は,現在のリストの総数です(０もありえます！！)。
  失敗した場合は,-1を返します。
*/
extern int UTL_EFT_DelCallback( void *p ) ;

/*
  リストからコールバック関数を追加します。

  返り値は,現在のリストの総数です(０もありえます！！)。
  失敗した場合は,-1を返します。
*/
extern int UTL_EFT_AddCallback( void *f, void *p ) ;

/*
  与えられた位置のバウンドチェックします。
  成功した場合は,そのインデックスを返します。

  返り値は,バウンドのインデックスです(０もありえます！！)。
  失敗した場合は,-1を返します。
*/
extern int UTL_EFT_CheckBound( FVECTOR *pos ) ;

/*
  バウンドを追加します。
  同じフレームの場合のみ追加していき,
  異なったフレームの場合はクリアされ,コールバックが実行されます。

  返り値は,バウンドの総数です。
*/
extern int UTL_EFT_AddBound( FVECTOR *p0, FVECTOR *p1, int flag ) ;



/*

サンプルプログラムです。
こんな感じで組んで見て下さい。

static void Die( Work *work )
{
    UTL_EFT_DelCallback( work ) ;
}

static void BoundCallBack( Work *work )
{
    int i ;
    for( i=0 ; i<work->n_effect ; i++ )
	if ( UTL_EFT_CheckBound( &work->pos[i]) >= 0 )
	    work->alpha[i] = 0 ;
}

static void GetResources( Work *work )
{
    UTL_EFT_AddCallback( BoundCallBack, work ) ;
    
}

 */
