$Id: readme.txt,v 1.3 2002/04/03 04:54:13 usr03700 Exp $ 

2000/10/11  狩野  賢一郎 (KanoKen)


mdl2kcの使い方

		mdl2kc inputfile(*.mdl) [outputfile(*.kms)] [outputfile(*.cv2)]

		inputfile               ... 入力mdlファイル名
		outputfile(*.kms,*.evm) ... 出力kms(evm)ファイル名(省略可)
		outputfile(*.cv2)       ... 出力cv2ファイル名(省略可)

・kms,evmが、実際にPS2で表示に用いられるモデルファイルです。
・cv2は、共有頂点情報と、ストリップ構造との関係を示したインデックスのファイルです。
  特殊処理に使われます。
・出力ファイル名(kms,evm,cv2)が省略された場合、入力ファイルの拡張子をmdlから
  それぞれの拡張子に切替えたものが使われます。

		オプション
		・オプションは、コマンドのどこに置いても構いません。
		  通常は、mdl2kcのすぐ後、入力ファイル名の前か、全てが書かれた後に
		  付加されます。
		  -d : 分割処理の際の単位を指定します。
               例 : -d 3000 5000 3000
		  -s : 全体のスケール値を指定します。
               例 : -s 100
		  -k : モデルがスケルトンモデル(骨構造を利用したモーション再生を行なうモデル)で
		       あることを指定します。
		       例 : -k
		  -l : 出力するファイルの格納先ディレクトリを指定します。
		       mdl2kcは、変換の際に貼られているテクスチャも自動出力します。
		       そのテクスチャの出力先を指定します。
		       例 : -l /usr/local/develop
		  -r : rptファイルの指定をします。rptファイルは、オプション指定も含めた
		       モデルの付加情報の指定用ファイルです。
		       例 : -r sna_def.rpt
		  -f : ストリップの変換を最速モードにします。この変換は、変換速度のみを追求しますので、
		       表示最適化はしません。
		       例 : -f
		  -m : テクスチャの貼り方を、シングルかマルチか選択します。
		       シングルとマルチでは、若干モデルフォーマットに対する制限事項が違いますので、
		       意図して(シングル->マルチとして)変換する際のオプションです。
		       例 : -m 2   ( 0:Auto Select, 1:Single Texture, 2:Multi Texture )
		  -c : テクスチャが制限を超えて貼られているかどうかのチェックを行なわないオプションです。
		       例 : -c
		  -e : evmファイルフォーマットは、座標データとして、二種類の固定小数点が選択できます。
		       このオプションが指定されますと、(1.15.0)が用いられます。
		       例 : -e
		  -u : mdlに含まれるUV値を全て用いるようにします。
		       mdlは、通常、マルチテクスチャでも一組のUV値しか有効でないようです。
		       このオプションは、将来テクスチャごとにUV値が指定できるようになった場合のための
		       拡張用です。
		       例 : -u


プログラムの解説
  mdl2kcは、ほとんどの機能をlibMDUやVRSのライブラリに依存し、mdl2kc2ディレクトリには、
  オプションの処理しかありません。ですので、libMDUのファイルを中心に解説します。

  mdl2kcの処理の流れは、次のようになっています。

    mdlファイル --(0)----(1)--> kmxファイル(中間形式) --(3)---(4)--> kmsファイル
                      |                                        |
                      |                                        ----> cv2ファイル
                      |
                      ---(2)--> evfファイル(中間形式) --------(5)--> evmファイル

    (0) シングルウェイトモデルかマルチウェイトモデルかを判定します。
        シングル ... ポリゴンを含まないオブジェクトが無い。
        マルチ   ... ポリゴンを含むオブジェクト     = 1
                     ポリゴンを含まないオブジェクト = 複数
        上の条件以外では、エラーとなります。
        (Mdl2Evf.c : CountMesh( ...) )

    (1) mdlを、シングルウェイトモデル用の中間形式、kmxに変換します。
        (Mdl2Kmx.c, CheckPrims.c)

    (2) mdlを、マルチウェイトモデル用の中間形式、evfに変換します。
        (Mdl2Evf.c, CheckPrims.c)

    (3) kmxに対し、分割処理を行ないます。
        (DivideKmx.c)

    (4) kmxをkmsに変換します。その際、cv2のための情報も抽出します。
        (Kmx2Kms2.c, DivPrims.c, ToStrip.c)

    (5) evfからevmに変換します。
        (Evf2Evm.c, DivPrims.c, ToStrip.c)

//////////////////////////
2002/03/28 M.Kobayashi
Xbox フォーマット拡張について

１．DG_MDLPACK が下記データのようになりました。evm に関しても同様の変更が加えられています。
・n_verts が u_int -> u_short になって
　u_short n_indices が追加されました。インデックスの数です。
・pad の場所を利用して u_short	*index が入ります。インデックスバッファのポインタです。
　他のポインタと同様、データファイルになったときはファイル先頭を０としたアドレスが入ります。
　

typedef	struct _KMS2_MDLPACK	{
    unsigned int	flag ;		/* 属性フラグ				*/
#if 0
    unsigned int	n_verts ;	/* ポリゴン数				*/
#else
 	unsigned short	n_verts ;		/* 頂点数 */
	unsigned short	n_indices ;		/* インデックス数 */
#endif
    unsigned int	tex_id[3] ;	/* テクスチャＩＤ（３セット）		*/
    short		*verts ;	/* 頂点データ				*/
    short		*norms ;	/* 法線データ				*/
    short		*uvs[3] ;	/* ＵＶデータ（３セット）		*/
    CVECTOR		*rgbs ;		/* プリシェードデータへ（未使用）	*/
	//    int			pad ;		/* リザーブ */
	unsigned short	*index;			/* KMS/EVM用頂点バッファ情報へのポインタ */	
} KMS2_MDLPACK ;

・インデックス付きデータの場合、DG_DEF の u_int data_format の上位ビット部分にフラグ
  MGS_MODEL_FLAG_INDEX が立ちます。
  data_format を直接評価しているところは MGS_MODEL_FORMAT_MASK でマスクする必要があります。
  （実機では以下のマクロを使っています）
   #define DG_GetMdlFormat( _def ) ((_def)->data_format & MGS_MODEL_FORMAT_MASK )

enum {
	MGS_MODEL_NORM			= 13112177,	/* 通常モデル */
	MGS_MODEL_MULTITEX		= 11686819,	/* 通常マルチテクスチャモデル */
	MGS_MODEL_MULTIWEIGHT	= 13335939,	/* マルチウェイトエンベロープモデル */

	MGS_MODEL_FORMAT_MASK	= 0x00ffffff,	/* 24 bit mask 上位ビットは拡張 */

	MGS_MODEL_FLAG_INDEX	= 0x01000000,	/* インデックスつきモデル（Xbox用） */
} ;

２．ツール変更点

mdl2kc にオプション -x をつけることによって、
インデックスデータが付き、１ストリップの長さ制限が取り払われます。
-x がつかないときは従来のＰＳ２データを吐きます。
（フォーマットは上記のものだが n_indices に０が書き込まれる）

この変更のために、
tools/mdl2kc2
tools/libMDU 
が書き換えられています。

また、上記フォーマット対応のため、
tools/evm_conv
tools/kms_util2
が書き換えられました。
