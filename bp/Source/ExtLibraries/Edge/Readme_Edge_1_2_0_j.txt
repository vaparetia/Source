SCE CONFIDENTIAL
PlayStation(R)Edge 1.2.0
Copyright (C) 2010 Sony Computer Entertainment Inc.
All Rights Reserved.

======================================================================
このパッケージには、以下のツール、およびドキュメントが含まれています。
- Edge Geometry
- Edge Animation
- Edge Zlib
- Edge LZMA
- Edge LZO
- Edge DXT
- Edge Post
- SPA

======================================================================

----------------------------------------------------------------------
パッケージ構成
----------------------------------------------------------------------
cell\
|---Readme_Edge_X_X_X_j.txt                 このファイル
|---host-common
|    |---contrib                            FCOLLADAのインストールターゲット
|    |    +---FColladaBuild                 FCOLLADAプロジェクト管理用フォルダ
|    |---include
|    |    +---edge
|    |         |---libedgeanimtool          Edge Animツール用共通ヘッダ
|    |         +---libedgegeomtool          Edge Geomツール用共通ヘッダ
|    +---src
|         +---edge
|              |---edgeanimcompiler         Edge Anim COLLADAコンパイラ
|              |---edgegeomcompiler         Edge Geom COLLADAコンパイラ
|              |---geomtoolsample           Edge Geom ツールサンプル
|              |---libedgeanimtool          Edge Animツールライブラリ
|              |---libedgegeomtool          Edge Geomツールライブラリ
|              +---shared                   Edgeツール共通ヘッダ
|---host-win32
|    |---bin                                Windows用ビルド済み実行バイナリ(SPAツール)
|    +---lib                                Windows用ビルド済みライブラリ
|---license
|    +---others               
|         |---FCollada.txt
|         +---PlayStation_Edge_Terms_and_Conditions_j.txt ライセンス事項
|---samples
|    +---edge                               Edgeサンプルディレクトリ
|---SDK_doc
|    +---en(jp)
|         +---pdf
|              +---edge                     ドキュメント
+---target
     |---common
     |    +---include
     |         +---edge
     |              |---edge_*.h            Edgeランタイム共通ヘッダ
     |              |---anim                Edge Anim共通ヘッダ
     |              |---geom                Edge Geom共通ヘッダ
     |              |---lzma                Edge LZMA共通ヘッダ
     |              |---lzo                 Edge LZO 共通ヘッダ
     |              |---post                Edge Post共通ヘッダ
     |              +---zlib                Edge Zlib共通ヘッダ
     |---ppu
     |    |---include
     |    |    +---edge
     |    |         |---anim                Edge Anim PPEヘッダ
     |    |         |---lzma                Edge LZMA PPEヘッダ
     |    |         |---lzo                 Edge LZO  PPEヘッダ
     |    |         |---post                Edge Post PPEヘッダ
     |    |         +---zlib                Edge Zlib PPEヘッダ
     |    |---lib                           PPE用ライブラリ
     |    +---src
     |         +---edge
     |              |---anim                Edge Anim PPEソースコード
     |              |---lzma                Edge LZMA PPEソースコード
     |              |---lzo                 Edge LZO  PPEソースコード
     |              |---post                Edge Post PPEソースコード
     |              +---zlib                Edge Zlib PPEソースコード
     |---spu
     |    |---include
     |    |    +---edge
     |    |         |---anim                Edge Anim SPEヘッダ
     |    |         |---dxt                 Edge DXT  SPEヘッダ
     |    |         |---geom                Edge Geom SPEヘッダ
     |    |         |---lzma                Edge LZMA SPEヘッダ
     |    |         |---lzo                 Edge LZO  SPEヘッダ
     |    |         |---post                Edge Post SPEヘッダ
     |    |         +---zlib                Edge Zlib SPEヘッダ
     |    |---lib                           Edge SPE用ライブラリ
     |    +---src
     |         +---edge
     |              |---anim                Edge Anim SPEソースコード
     |              |---dxt                 Edge DXT  SPEソースコード
     |              |---geom                Edge Geom SPEソースコード
     |              |---lzma                Edge LZMA SPEソースコード
     |              |---lzma-inflate-task   Edge LZMA SPURS解凍タスクソースコード
     |              |---lzo                 Edge LZO  SPEソースコード
     |              |---lzo1x-deflate-task  Edge LZO  SPURS圧縮タスクソースコード
     |              |---lzo1x-inflate-task  Edge LZO  SPURS解凍タスクソースコード
     |              |---post                Edge Post SPEソースコード
     |              |---zlib                Edge Zlib SPEソースコード
     |              |---zlib-deflate-task   Edge Zlib SPURS圧縮タスクソースコード
     |              +---zlib-inflate-task   Edge Zlib SPURS解凍タスクソースコード
     |---reference
     |    +---src
     |        +---edge
     |              |---anim                Edge Anim参考用ソースコード
     |
     +---windows
          |---include
          |    +---edge
          |         +---anim                Edge Anim Windows環境向けヘッダ
          +---src
               +---edge
                    +---anim                Edge Anim Windows環境向けソースコード


--------------------------------------------------------------------
新規機能
--------------------------------------------------------------------

 Edge Animation
 - EdgeAnimCommandOpに新しいダミーコマンドを追加
   ユーザがこの値から独自のopcodeを追加して頂ければ、今までの、全体的に
   opcodeのデフォルト有効範囲を変更する様な作り方と比べ、潜在的な不具合を
   引き起こす可能性が無くなります。

 - EdgeAnimSkeletonにlocomotionBoneへのindexが新たに追加されました
   バージョンタグもこれに伴い更新されました。

 - EdgeAnimAnimationにrotation及びtranslation等、locomotionアニメーションの
   デルタ情報へのオフセットを追加。歩いたり走ったりするようなアニメーションなど
   で利用できます。
   これに伴いアニメーションデータのバージョンタグも更新されました。
   
 - 主にEdgeAnimSkeletonとEdgeAnimAnimation等のデータ構造に対し、
   カスタムデータテーブルを使用する時の具体例として、edgeanimcompilerツールに
   新しいオプション'-jointnames'を追加しました。

 - _edgeAnimMirrorJoints()でジョイントマッピングを行う時の補助として、
   ヘルパー関数edgeAnimCreateMirrorPairSpec()を追加しました。
   この関数に関する説明は今回のリファレンスドキュメントに掲載されていませんが、
   具体的にはOverviewドキュメントのチャプタ8、ミラーリングの節で説明した
   スペックの作成をより直感的に行えるようにする為の関数となります。
   
 - SPU向けのedgeanimライブラリにSPA最適化版の_edgeAnimEvaluateSTと
   _edgeAnimEvaluateUserの実装が追加されました。

 - C++版の参考実装はcell\target\reference\src\edge\anim\に移しました。
 
 - Win32ターゲットがwindowsターゲットにリネームされました。
   関連するファイル   cell\target\win32\include\edge\anim\edgeanim_win32.hも
   cell\target\windows\include\edge\anim\edgeanim_windows.hにリネームされました。
   edgeanim_win32.vcprojプロジェクトファイルに関してはそのままにしています。

 - Windowsターゲット向けにSSE2イントリンジックで最適化した実装を以下のパスに
   追加しました。
   cell\target\windows\src\edge\anim\
   これらの実装に関して、その入力/出力データのアラインメントはSPU用の実装と
   一致しています。デバッグ版ではアラインメントの不正を検出できます。
   これに伴い、win32版edgeanimライブラリはデフォルトでSSE2イントリンジックを
   使用するように変更されました。

 - windows向けSSE2ターゲットでedgeAnimEvaluateJoint及び
   edgeAnimEvaluateUserChannelヘルパー関数をサポートするようになりました。

 Edge Geometry

 - EdgeGeomはGPUと同じアルゴリズムでsub-pixelトライアングルカリングを行なうよう
   になりました。これによりカリングの精度が大幅に改善され、一般的なシーンでの
   テストでは約10-25%の改善がありました。

 Edge Tools

 - edgeanimcompilerにオプション-locoJoint <name>を追加しました。
   これにより、対応するスケルトン、或いはアニメーションリソース内の
   ジョイントインデックス及びlocomotionデルタを保存することが出来ます。

 - libEdgeAnimToolでのExtractSkeletonはスケルトン構造内のバージョンタグから、
   正しいエンディアンを判断するようになりました。これにより明示的に指定する
   必要が無くなりました。

 - LibEdgeAnimToolでのExtractSkeletonはペアレントインデックスとスケール補正
   フラグを入力バイナリより取り出すようになりました。これにより、スケルトン
   構造内から取り出されないデータは、カスタムデータのみとなりました。

 - LibEdgeAnimToolでのComputePeriodは行列形式のperiod hintsを
   受け入れるようになりました。より詳細な説明はリファレンス、或いは
   ソースコード内のコメントを参照してください。

 - edgeGeomPartitioner()は入力をシリアライズ化し、スタンドアローンなC++行列
   として出力出来るようになりました。内部用ヘルパー関数としてまとめましたので、
   パーティショナー関連の問題を調査時にも利用出来ます。

 Edge Post

 - 今回のリリースより、SPUでのMorphological Anti-Aliasing (MLAA)の実装を
   提供し始めました。MLAAはイメージベースアンチエイリアシングを行なう
   ポストプロセスであり、今回提供したSPUの実装はsingle-sampleで生成した
   フレームバッファを、GPUと並行で処理できるようになっております。
   これによりアンチエイリアシングの処理はGPUから切り離す事ができます。

 - MLAAの組み込み方法を示すサンプルを追加しました。
   このサンプルはassetフォルダ内にある全てのイメージをロードし、
   MLAA処理を施します。このサンプルを使い、ゲームのスクリーンショットから
   実際にMLAAをかけた結果を簡単に確認する事が出来ます。
   
 - edgepostサンプルを改善し、MLAA適用時のより複雑なシナリオを表現するように
   なりました。MLAAを始め、被写界深度、モーションブラー、ブルーム等のエフェクト
   の組み合わせが参考になります。
 
 - 同じメモリ上にpseudo-transposeを行なうサンプルとして、
   Young-van-Vliet recursive gaussian blurを実装したサンプルプログラムを
   追加しました。
 
--------------------------------------------------------------------
仕様変更
--------------------------------------------------------------------

 Edge Animation

 - ローカル座標からワールド座標の変換関数内で、ペアレントによるスケール補正は、
   ペアレントのワールドスペーススケールを使うより、ローカルスペースでのスケール
   を使用するように変更しました。
   これにより、入力と出力のジョイント行列を同じメモリ上のもので指定する事が
   できなくなりました。（エラー検知の為のアサートも追加されました）

 Edge Geometry

 - edgeGeomGetSpuVertexFormat()とedgeGeomSetSpuVertexFormat()関数は出力先として、
   今までは内部でメモリを確保していましたが、新たに引数で対象構造体へのポインタ
   を指定できるようになりました。

 - 1.1.xより廃止予定となった構造体メンバー及び関数の引数等を削除しました。
   - m_cullingFlavor (EdgeGeomPartitionerInputのメンバー変数)
   - m_canBeOccluded (EdgeGeomPartitionerInputのメンバー変数)
   - cullType (edgeGeomGetScratchBufferSizeInQwordsへの引数)
   - canOcclude (edgeGeomGetScratchBufferSizeInQwordsへの引数)
   - m_cullType (EdgeGeomSegmentFormatのメンバー変数)
   - m_canBeOccluded (EdgeGeomSegmentFormatのメンバー変数)
   - numOneDimensionalTriangles (EdgeGeomCullingResultsのメンバー変数)
   - numZeroAreaTriangles (EdgeGeomCullingResultsのメンバー変数)

 Edge Samples

 - locomotion-sampleサンプルはアニメーション内のスケルトンとアニメーションに
   含まれるlocomotionジョイントインデックスとデルタ情報を使用するように
   変更しました。
   (今まではルートジョイントを仮定し、初期化値でデルタの算出を行なっていました)

 Edge Tools

 - 1.1.0より廃止予定となったホストツール向けライブラリlibedgeanimと
   libedgegeomのヘッダ及びソースコードを削除しました。
   
 Edge Zlib

 - 関数"longest_match"をSPUアセンブラで最適化しました。Level 9の圧縮に於いて、
   入力データ依存ですが、Edge Zlibのスループットが約9%～84%改善されました。
   Level 1の圧縮に関しては、約4%～13%の改善となります。

 - Inflateタスク内の"_edgeZlib_inflate_fast_spu_asm"で使用された
  "_edgeZlibFalseStore"を".data"セクションより".bss"セクションへ移動しました。

 Edge Post

 - edgePostFP16toFloatsを書き直し、入力・出力に同じ場所を指定されても動作
   出来るようになりました。

 
--------------------------------------------------------------------
不具合修正
--------------------------------------------------------------------

 Edge Animation

 - _edgeAnimMirrorJointsのリファレンス用C++実装はSPUコードと同じように
   ジョイントのウェートを取り換えるようになりました。

 - _edgeAnimEvaluateのリファレンス用C++実装は64-bitアドレス対応になりました。

 - _edgeAnimEvaluateのリファレンス用C++実装は各チャンネルのデータがbitpackedか
   どうかの検出をする様になりました。

 - edgeanim_spu.hとedgeanim_ppu.hから、edgeanim_common.h内で既に定義された
   ジョイントインデックスを取得する関数に関する重複した宣言を削除しました。

 Edge Geometry

 - edgeGeomEndCommandBufferHole()は実際に出力する前にHoleのサイズが十分にあるか
   どうかの検出を行なう様に修正しました。

 - edgeGeomBeginCommandBufferHole()内の潜在的なI/O Bufferへのバッファ
   オーバーラン問題を修正しました。

 - ツールで提供したedgeGeomCalculateDefaultOutputSize()とランタイムの
   edgeGeomOutputVertexes()関数が同じアルゴリズムで出力バッファサイズの計算を
   行なう様に修正しました。

 Edge Tools

 - 効果がなかったedgeGeomMakeBlendShapeBuffer()内のアサートを修正しました。

 - 1ストリーム内で複数のFixed-pointを使用した頂点アトリビュートがあった時、
   不正なFixed-pointオフセットを生成する問題を修正しました。


 Edge Post

 - edgePostFP16toFloatsからのデータの出力順序が入力時の順序と異なる問題を
   修正しました。
  
 - モーションブラーエフェクトにバンディングを引き起こす不具合を修正しました。
 
 - edgepostサンプルで、edgepost effect内で定義したctor/dtorが呼ばれない
   不具合を修正しました。
 
 Edge Zlib

 - kEdgeZlibDeflateTask_DeflateStoreCompressedWithHeaderを設定し圧縮を
   行った場合、出力のチェックサムがゼロになり、そのデータを通常の"zlib"で
   伸張時にチェックサムテストで失敗してしまう問題を、正しいAdler-32
   チェックサムをセットする様に修正しました。

---------------------------------------------------------------------
既知の不具合・制限事項
---------------------------------------------------------------------

Edge Tools

 - edgegeomcompiler及びedgeanimcompilerをビルドするとき
   FColladaライブラリが必要です。下記のアドレスからダウンロードできます。
   http://sourceforge.net/projects/colladamaya
   これを下記のパスへコピーしてください。
   cell\host-common\contrib\FCollada
   FColladaを使用した為、edgegeomcompilerとedgeanimcompilerは
   Windows環境でしか動作しません。
   今回のリリースではFCollada 3.05Bとの動作チェックはしております。
   なお、FColladaはWindows環境のVisual Studioでビルドする必要があります。

Edge Geometry 

 - "libedgegeomtool.vs8.vcproj"プロジェクト内にビルドターゲット"PS3 Debug|x64"
   及び"PS3 Release|x64"がありますが、正しく動作しません。代りに
   "PS3 Debug|Win32"と"PS3 Release|Win32"を使ってください。

Edge Samples

 - サンプルのビルドはPlayStation(R)3 Programmer Tool Runtime Library内に
   含まれた下記のサンプルライブラリに依存しています。サンプルのビルドを
   行う前にこれらのライブラリのビルドを先に行って下さい。
   $(SCE_PS3_ROOT)/samples/fw
   $(SCE_PS3_ROOT)/samples/sdk/graphics/gcm
   $(SCE_PS3_ROOT)/samples/common/gtf

----------------------------------------------------------------------
使用許諾・制限
----------------------------------------------------------------------
このソフトウェアの使用許諾、使用制限は貴社と当社(株式会社ソニー・
コンピュータエンタテインメント)との間に締結されている契約に準じます。

----------------------------------------------------------------------
商標に関する注意書き
----------------------------------------------------------------------
"PlayStation"、"PS3"、"RSX"および"Cell Broadband Engine"は、
株式会社ソニー･コンピュータエンタテインメントの商標または登録商標です。
パッケージ内の本文中に記載されている会社名、製品名、サービス名は、
一般に各社の商標または登録商標です。
なお、パッケージ内の本文中に (R)、(TM)、(SM)マークは
明記していない場合があります。


PlayStation(R)Edgeを使って開発する場合はcell\license\others以下の
PlayStation_Edge_Terms_and_Conditions_*.txtをご一読ください。

cell\host-win32\bin\*.exeは、FCollada SDKを使用しています。
cell\license\othersフォルダ内のFCollada.txtを参照してください。


