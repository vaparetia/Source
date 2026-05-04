  
  テクスチャフォーマットXTI(directX-Texture-Image)に関して
  2000/04/13 F.Miyauchi
  $Id: xti_readme.txt,v 1.1 2002/03/08 07:23:38 usr02774 Exp $

1.概要

    複数枚のテクスチャを1枚にまとめたものです。
    PS2版MGS2では、'TRI'にあたります。
    実験のため、linux上で動作するツールを「とりあえず」作成しました。


2.コンバート方法

    ex7:/usr/local/develop/usr/mgsx/tools/makexti
    を使用します。bmp結合アルゴリズム(by M.Nisino)と
    libMDU のテクスチャ読み込みルーチンを強引にくっつけた
    「とりあえず」版のツールです。
    対応するテクスチャフォーマットはlibMDU に依存します。

    makexti -out output -in inputfiles....

    例) > makexti -out sna_def -in *.bmp *.pic
         -> カレントディレクトリの *.bmp, *.pic を読み込んで
            sna_def.xti を作成します。

3.データフォーマット

    --------------------------
    TRI_FILEHEADER
    --------------------------
    DG_TEX[テクスチャの数]
    --------------------------
    IMAGE DATA(現在は24bitBMP)
    --------------------------

    TRI_FILEHEADER, DG_TEX は実機と整合性を取るようにして下さい。
    PS2固有のデータがかなり残っているので、XBOX側は変更の可能性大です。

    /* PS2版と全く同じ */
    typedef struct _tri_fileheader {
        unsigned int tex_offset;    /* テクスチャデータ開始オフセット(byte) */
        unsigned int tex_size;      /* テクスチャデータサイズ(byte) */
        unsigned int clut_offset;   /* ＣＬＵＴデータ開始オフセット(未使用) */
        unsigned int clut_size;     /* ＣＬＵＴデータサイズ(未使用) */
        unsigned int n_textures;    /* テクスチャデータ数 */
        unsigned int compress_flag; /* 圧縮フラグ(未使用) */
        unsigned int texel_addr;    /* テクセルデータへのポインタ(未使用) */
        unsigned int clut_addr;     /* ＣＬＵＴデータへのポインタ(未使用) */
    } TRI_FILEHEADER;

    /* PS2版とサイズが違うので注意！*/
    typedef struct _dg_tex {
        float               u_offset;   /* 使用テクスチャの開始Ｕ座標 */
        float               v_offset;   /* 使用テクスチャの開始Ｖ座標 */
        float               u_scale;    /* 使用テクスチャのスケール補正値 */
        float               v_scale;    /* 使用テクスチャのスケール補正値 */
        unsigned int        tex_id;     /* テクスチャＩＤ */
        unsigned int        tri_id;     /* テクスチャアーカイバＩＤ */
        int                 flag;       /* テクスチャフラグ(未使用) */
        int                 pad[19];    /* pad, DG_TEX_TRANS の大きさ */
  } DG_TEX ;


4.その他メモ

    XBOX側では、
   「IMAGE DATA をそのまま D3DXCreateTextureFromFileInMemory();に渡す」
    という適当な実装になっています。
    従って、D3DXCrea.. で読めるファイル形式であれば、
    フォーマットは何でも構いません。
    ※将来的には、compress_flag に画像フォーマット用のフラグを入れます(多分)。

    「モデルより先」に読み込んでおかないと、モデルのUV補正が出来ないため
    モデル読み込みに失敗します。
