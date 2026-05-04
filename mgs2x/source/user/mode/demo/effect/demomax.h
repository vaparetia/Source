/*

  マックス　ダイアローグ　ビルトイン関数

  2000/08/23 T.Morita
  $Id: demomax.h,v 1.1.1.3 2002/11/19 11:45:11 Yoshizawa1 Exp $

 */


/*
  エフェクトの各種プロパティを取ってくる（ダイアローグ一切なし）

  int   MaxInputGetKeyRangeMin( char *name, int flag ) ;
  int   MaxInputGetKeyRangeMax( char *name, int flag ) ;
  int   MaxInputGetEffectName( char *name, int flag ) ;
  int   MaxInputGetFrameRate( char *name, int flag ) ;
  IVECTOR MaxInputGetLink( char *name, int flag ) ;
  float MaxInputGetFloat( char *name, int flag ) ;(内部的に使用)
  int   MaxInputGetInt( char *name, int flag ) ;(内部的に使用)

  [引数]
  char *name    ダイアローグ名
  int flag      フラグは無いので０にすること

  [返り値]
  MaxInputGetKeyRangeMinでは,エフェクトのキーフレーム範囲で最初のフレーム番号を返す。
  MaxInputGetKeyRangeMaxでは,エフェクトのキーフレーム範囲で最後のフレーム番号を返す。
  MaxInputGetEffectNameでは,エフェクトの名前をStrCodeで返す。
  MaxInputGetFrameRateでは,MAXの現在のフレームレートを返す。
  IVECTOR MaxInputGetLinkでは,エフェクトのリンク制御によりハンドラID及び関節番号を返す。
       vxにハンドルID、vyに間接番号が入る。MaxInputHumanNum()よりこっちを優先的に使う
       二つ以上モデル関節が要る場合にMaxInputHumanNum()を使うことが望ましい。更に言え
       ば,位置的な親参照には,リンク制御にして下さい。
  float MaxInputGetFloatは,構造体から値をとってくる（内部的に使用）
  int   MaxInputGetIntは,構造体から値をとってくる（内部的に使用）

*/




/*
  スピン付きFVECTORダイアローグ

  FVECTOR MaxInputFVectorSpin( char *name, int flag, float def_x, float def_y, float def_z ) ;

  [引数]
  char *name    ダイアローグ名
  int flag      フラグ
                 ０:コントロールなし（数値入力のみ）
                 １:位置コントロール
                 ２:回転コントロール
                 ３:スケールコントロール
  float def_x   ダイアローグX値のデフォルト値
  float def_y   ダイアローグY値のデフォルト値
  float def_z   ダイアローグZ値のデフォルト値

  [返り値]
  xyzに値が入ったFVECTORを返す。
  回転コントロール時 xyz入力単位は°であるが,出力はπラジアンで入る。

  [機能]
  スピンにより微調整が可能。
  キーボードからの入力が可能。
  flagを0以外に設定することでMAXでのビューポートでのオペレーションが可能。
  スピンがあるため, 入力ウィンドウは縦にならんでいる。
*/
/*
  FVECTORダイアローグ

  FVECTOR MaxInputFVector( char *name, int flag, float def_x, float def_y, float def_z ) ;

  [引数]
  char *name    ダイアローグ名
  int flag      フラグ
                 ０:コントロールなし（数値入力のみ）
                 １:位置コントロール
                 ２:回転コントロール
                 ３:スケールコントロール
  float def_x   ダイアローグX値のデフォルト値
  float def_y   ダイアローグY値のデフォルト値
  float def_z   ダイアローグZ値のデフォルト値

  [返り値]
  xyzに値が入ったFVECTORを返す。
  回転コントロール時 xyz入力単位は°であるが,出力はπラジアンで入る。

  [機能]
  キーボードからの入力が可能。
  flagを0以外に設定することでMAXでのビューポートでのオペレーションが可能。
  入力ウィンドウは横にならんでいる。
*/

/*
  スピン付きSVECTORダイアローグ

  SVECTOR MaxInputSVectorSpin( char *name, int flag, short def_x, short def_y, short def_z ) ;

  [引数]
  char *name    ダイアローグ名
  int flag      フラグ
                 ０:コントロールなし（数値入力のみ）
                 １:位置コントロール
                 ２:回転コントロール
                 ３:スケールコントロール
  float def_x   ダイアローグX値のデフォルト値
  float def_y   ダイアローグY値のデフォルト値
  float def_z   ダイアローグZ値のデフォルト値

  [返り値]
  xyzに値が入ったSVECTORを返す。
  回転コントロール時 xyz入力単位は°であるが,出力は4096ラジアンで入る。

  [機能]
  キーボードからの入力が可能。
  flagを0以外に設定することでMAXでのビューポートでのオペレーションが可能。
  スピンがあるため, 入力ウィンドウは縦にならんでいる。
*/

/*
  SVECTORダイアローグ

  SVECTOR MaxInputSVector( char *name, int flag, short def_x, short def_y, short def_z ) ;

  [引数]
  char *name    ダイアローグ名
  int flag      フラグ
                 ０:コントロールなし（数値入力のみ）
                 １:位置コントロール
                 ２:回転コントロール
                 ３:スケールコントロール
  float def_x   ダイアローグX値のデフォルト値
  float def_y   ダイアローグY値のデフォルト値
  float def_z   ダイアローグZ値のデフォルト値

  [返り値]
  xyzに値が入ったSVECTORを返す。
  回転コントロール時 xyz入力単位は°であるが,出力は4096ラジアンで入る。

  [機能]
  キーボードからの入力が可能。
  flagを0以外に設定することでMAXでのビューポートでのオペレーションが可能。
  入力ウィンドウは横にならんでいる。
*/


/*
  コンボボックス(単一選択)

  int     MaxInputComboBox( char *name, int flag, char *input, char *def_v ) ;

  [引数]
  char *name    ダイアローグ名
  int flag      フラグは無いので０にすること
  char *input   コンボ文字列 (例  "名前１=0&名前２=3&名前３=8")
  char *def_v   デフォルトの選択 (例  "名前３")

  [返り値]
  選択された項目に相当する値が返る。

  [機能]
  プルダウンより項目を選ぶことが出来る。
  既定値の場合,数字で選ぶより文字で選べるので便利
*/


/*
  リストボックス(複数選択可能)

  int     MaxInputListBox( char *name, int flag, char *input, int def_v ) ;

  [引数]
  char *name    ダイアローグ名
  int   flag    フラグは無いので０にすること
  char *input   リスト文字列 (例  "名前１=0x2000&名前２=0x0001&名前３=0x0010")
  int   def_v   デフォルトの選択 (例  0x2010  これだと名前１と名前３が選ばれる)

  [返り値]
  複数選択された項目に相当する値のORが返る。

  [機能]
  ボックスより項目を複数選ぶことが出来る。
  既定値の場合,数字で選ぶより文字で選べるので便利
  フラグなどで利用すると良い
*/

/*
  モデル名と関節名の指定

  IVECTOR MaxInputHumanNum( char *name, int flag ) ;

  [引数]
  char *name    ダイアローグ名
  int flag      ０…２１関節
                １…５３関節

  [返り値]
  xに デモハンドラID(DM_GetObjectHandle()に使う)
  yに 関節番号が入る(-1の時は ルートオブジェクトになる)

  [機能]
  プルダウンより現在読み込まれているモデルを選び,
  その関節を指定することが出来る。ほとんどの場合,
  エフェクトは２１関節なので,５３関節は非実用的？
*/

/*
  現在デモで置いているモデルやエフェクトの名前を取得する

  int     MaxInputSelectName( char *name, int flag ) ;

  [引数]
  char *name    ダイアローグ名
  int flag      フラグは無いので０にすること

  [返り値]
  名前のStrCodeが返る

  [機能]
  プルダウンより現在読み込まれているモデルやエフェクトを選び,
  そのStrCodeを得ることが出来る。現在, 用途的にあまりない。
*/

/*
  Floatスライドバー

  float MaxInputSlideBarF( char *name, int flag, float min, float max, int seg, float def_f ) ;

  [引数]
  char *name    ダイアローグ名
  int flag      フラグは無いので０にすること
  float min     最小値
  float max     最大値
  int seg       メモリ数（１０だと１０個のメモリがつく）
  float def_f   デフォルト値

  [返り値]
  値がfloatで返る

  [機能]
  最大値と最小値の間に値が必ず来る。
  エディットがついているため直に打ち込むことも可能。
*/

/*
  Intスライドバー

  int   MaxInputSlideBarI( char *name, int flag, int min, int max, int seg, int def_i ) ;

  [引数]
  char *name    ダイアローグ名
  int flag      フラグは無いので０にすること
  int min     最小値
  int max     最大値
  int seg       メモリ数（１０だと１０個のメモリがつく）
  int def_f   デフォルト値

  [返り値]
  値がintで返る

  [機能]
  最大値と最小値の間に値が必ず来る。
  エディットがついているため直に打ち込むことも可能。
*/

/*
  カラーセレクション

  IVECTOR MaxInputSelectColor( char *name, int flag, int def_r, int def_g, int def_b, int def_a ) ;

  [引数]
  char *name    ダイアローグ名
  int flag      フラグは無いので０にすること
  int def_r     赤デフォルト値
  int def_g     緑デフォルト値
  int def_b     青デフォルト値
  int def_a     αデフォルト値

  [返り値]
  α付き色をIVECTOR にて返す。
  x に赤（０～２５５）
  y に緑（０～２５５）
  z に青（０～２５５）
  w にα（０～２５５）

  [機能]
  カラーセレクションダイアローグつきなのでRGBが色から選び出せる。
  エディットがついているため直に打ち込むことも可能。
  αは,必ずエディットのみでしか入力できない。
*/

/*
  Float入力

  float MaxInputFloat( char *name, int flag, float def_f ) ;

  [引数]
  char *name    ダイアローグ名
  int   flag    フラグは無いので０にすること
  float def_f   デフォルト値

  [返り値]
  ダイアローグに入力したfloatの値を返す

  [機能]
  ダイアローグに入力したfloatの値を返す。
  エディットしか入力できない。
  桁数のある数字を扱うには,非常に良い
*/

/*
  Int入力

  int   MaxInputInt( char *name, int flag, int def_i ) ;

  [引数]
  char *name    ダイアローグ名
  int flag      フラグは無いので０にすること
  int def_f     デフォルト値

  [返り値]
  ダイアローグに入力したintの値を返す

  [機能]
  ダイアローグに入力したintの値を返す。
  エディットしか入力できない。
  桁数のある数字を扱うには,非常に良い
*/

/*
  文字列入力よりStrCodeを得る

  int   MaxInputStrCode( char *name, int flag, char *def_s, char *add_src ) ;

  [引数]
  char *name    ダイアローグ名
  int flag      フラグは無いので０にすること
  int def_f     デフォルト値(もちろん文字列なので”で囲む必要がある)

  [返り値]
  ダイアローグに入力した文字列のStrCodeを返す

  [機能]
  ダイアローグに入力された文字列からStrCodeを返す。pathつきや拡張子が
  ついても取った状態のStrCodeを返す。
  モデル名入力などに有効である。
*/


/*
  メッセージ(内部的に使用)

  int   MaxInputMessage( char *name, int flag ) ;

  利用厳禁
*/

/*
  文字列入力より文字列を得る

  char  MaxInputString( char *name, int flag ) ;

  [引数]
  char *name    ダイアローグ名
  int flag      フラグは無いので０にすること
  int def_f     デフォルト値(もちろん文字列なので”で囲む必要がある)

  [返り値]
  文字列の先頭文字

  [機能]
  ダイアローグに入力された文字列から文字列（最高127文字まで）を出力する

*/
/*
  POINTダイアローグ

  int MaxInputPoint( char *name, int flag,
                     float def_rx, float def_ry, float def_rz,
                     float def_px, float def_py, float def_pz) ;

  [引数]
  char *name    ダイアローグ名
  int flag      フラグは,４（固定）ポジションコントロール機能
  float def_rx   回転ダイアローグ X値のデフォルト値
  float def_ry   回転ダイアローグ Y値のデフォルト値
  float def_rz   回転ダイアローグ Z値のデフォルト値
  float def_px   位置ダイアローグ X値のデフォルト値
  float def_py   位置ダイアローグ Y値のデフォルト値
  float def_pz   位置ダイアローグ Z値のデフォルト値

  [返り値]
  MAXの名前がStrCodeとして返ってきます。InitControl()のnameに使用して
  下さい。そうすると,毎フレーム mov と rot が変化します。

  [機能]
  キーボードからの入力が可能。リンクしている場合は,数値は,ローカルを入
  力する。flagを0以外に設定することでMAXでのビューポートでのオペレーショ
  ンが可能。入力ウィンドウは横にならんでいる。

  位置は, そのままPS単位（mm）である。回転は, xyz入力単位は360°である
  が, 出力はπラジアンで入る。リンクした場合は,ローカルの座標や回転角
  になる。

  プログラム内でCONTROLを使用していることを前提として作られている。MAX
  の名前をInitControl()の引数に使うわなければ,正常な動作はしない。ゲー
  ムシステム上でCONTROLをフレーム単位で動かすことが出来る。しかし,デモ
  デーモンよりアクターレベルが高い場合は,1フレーム遅れてしまう。

*/





/* ダイアローグなし */
int     MaxInputGetKeyRangeMin( char *name, int flag ) ;
int     MaxInputGetKeyRangeMax( char *name, int flag ) ;
int     MaxInputGetEffectName( char *name, int flag ) ;           
int     MaxInputGetFrameRate( char *name, int flag ) ;
IVECTOR MaxInputGetLink( char *name, int flag ) ; /* vxにハンドル取得ID、vyに間接番号 */
float   MaxInputGetFloat( char *name, int flag ) ;/* 内部的に使用 */
int     MaxInputGetInt( char *name, int flag ) ;  /* 内部的に使用 */


/* ダイアローグあり */
FVECTOR MaxInputFVectorSpin( char *name, int flag, float def_x, float def_y, float def_z ) ;
FVECTOR MaxInputFVector( char *name, int flag, float def_x, float def_y, float def_z ) ;
SVECTOR MaxInputSVectorSpin( char *name, int flag, short def_x, short def_y, short def_z ) ;
SVECTOR MaxInputSVector( char *name, int flag, short def_x, short def_y, short def_z ) ;
int     MaxInputComboBox( char *name, int flag, char *input, char *def_v ) ;
int     MaxInputListBox( char *name, int flag, char *input, int def_v ) ;
IVECTOR MaxInputHumanNum( char *name, int flag ) ;
int     MaxInputSelectName( char *name, int flag ) ;
float   MaxInputSlideBarF( char *name, int flag, float min, float max, int seg, float def_f ) ;
int     MaxInputSlideBarI( char *name, int flag, int min, int max, int seg, int def_i ) ;
IVECTOR MaxInputSelectColor( char *name, int flag, int def_r, int def_g, int def_b, int def_a ) ;
float   MaxInputFloat( char *name, int flag, float def_f ) ;
int     MaxInputInt( char *name, int flag, int def_i ) ;
int     MaxInputStrCode( char *name, int flag, char *def_s, char *add_src ) ;
int     MaxInputMessage( char *name, int flag ) ;
char   *MaxInputString( char *name, int flag ) ;
int     MaxInputPoint( char *name, int flag,
		       float def_rx, float def_ry, float def_rz,
		       float def_px, float def_py, float def_pz ) ;
