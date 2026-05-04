

                        フォーチュンの仕様

          $Id: readme.txt,v 1.1.1.3 2002/11/19 11:46:07 Yoshizawa1 Exp $


[仕様]

基本的には、下のフェーズを条件が満たすと、次のフェーズへ移行する仕組み
になっています。この他、特別アクションの４つ攻撃は、フェーズに関係なく
条件を満たせば優先的に発動する。

   狙いは、狙い始めてから、銃を止めるまでの時間が
   　　EXTREME　 間隔時間の0
   　　HARD以上　間隔時間の4/5
   　　NORMAL　　間隔時間の4/5 - TIME_BASE
   　　EASY以下　間隔時間の4/5 - TIME_BASE*2
   のようになっています。時間が短いほど、プレーヤーが移動していた場合に
   あたりにくくなります。
   さらにEXTREMEは、２連射です。

   プレーヤーの受けるダメージは、難易度に関係なく同じです。

   -----------------ここから特別アクション--------------------
   　　FRT_F_STAND_ATTK         禁止区域侵入攻撃（フォーチュン側に入る） 
   　　FRT_F_HIDING_ATTK        全地帯攻撃（両脇の爆発タンクの後ろあたり） 
   　　FRT_F_GASTANK_ATTK       ガスタンクを撃つ（３秒以上隠れている） 
   　　FRT_F_MOVE_ATTK          強制移動（メッセージ） 

   -----------------ここからフェーズ攻撃----------------------
   フェーズ 1　　　　あまり撃たない（15秒間たったら　次のフェーズへ） 
   フェーズ 2　　　　壁や天井の電灯を落す （プレイヤーに最も近い３つ落としたら次へ） 
   フェーズ 3　　　　前列壊す（決められた箱４つが全て壊れたら次のフェーズへ）
   フェーズ 4　　　　フォークリフト壊す（壊れたら　次のフェーズへ）
   フェーズ 5　　　　物を壊しつつライデンを追う（２０秒間たったら次、エレベータランプが点灯）
   　 HARD以上          狙いがかなり正確
   　 VERYHARD          0秒間隔で撃つ
   　 HARD,NORM,EASY    1秒間隔で撃つ
   　 VERYEASY          2秒間隔で撃つ
   フェーズ 6　　　　ライデンを本気で追う（１５秒たったら次へ）
   　 HARD以上          0秒間隔で撃つ
   　 NORMAL以下　　　　1秒間隔で撃つ
   フェーズ 7　　　　最後の爆発物を壊す　（爆発ドラム缶を破壊したら、次へ）
   　 全ての難易度      0秒間隔で撃つ
   フェーズ 8　　　　瓦礫落し　（瓦礫を撃って落としたら次へ）
   フェーズ 9　　　　ゲームクリア 



--------------------------------------------------------------------
[プログラムの内訳]

オルガと思考体系が全く同一である。

説明しやすいように言葉を定義しておく。
        思考アクト    : まとまりのある複数のモーションを再生する行動単位。
        思考アクション: 思考アクトの集まり。

フォーチュン自信は、思考アクト関数を書き変えることで思考変化を行なって
いる。実際には、攻撃思考アクト,移動思考アクトの２つの思考アクトで次に
行なわれるべき思考アクションを選択し、行動を進める。思考アクションは、
基本的に上の２つの何れかの思考アクトが最後に来るようになっている。つま
り、あるアクションの後に思考し、次のアクションを選択する仕掛けになる。

認知部として用意してあるのが FRT_Recognition() である。さまざまな情報
を調べるための物である。フラグによって管理されている。要らない情報は、
思考アクト側でフラグを寝かして管理する。フラグが立っていれば毎フレーム
取りに行く。

fort:
-rw-rw-r--    1 usr04098 user         2664 Oct  5 15:53 fort.c
全体の統括(New???? Act Die GetResource)。
-rw-rw-r--    1 usr04098 user         4555 Oct  5 15:53 fort_act.c
システムに必要な処理を行ない。思考actの履歴をとる関数(ORG_ActThink)な
どもある。think以下のヘッダは全てincludeされている。
-rw-rw-r--    1 usr04098 user         6701 Oct  5 19:19 fort_aim.c
狙いシステムの登録、削除、検索など。
-rw-rw-r--    1 usr04098 user         3251 Oct  5 23:37 fort_clb.c
弾それエフェクトの発動。
-rw-rw-r--    1 usr04098 user         6185 Oct  5 15:53 fort_dsp.c
表示系をまとめている。口パク、ホーミングの上体操作(補間あり)。システム
では、レーダーの色。
-rw-rw-r--    1 usr04098 user         7997 Oct  5 15:53 fort_ini.c
システム(Control,Object)の初期化。ターゲットの設定。装備や付随エフェク
トの起動。
-rw-rw-r--    1 usr04098 user         4896 Oct  5 15:53 fort_msg.c
メッセージ処理。
-rw-rw-r--    1 usr04098 user        11999 Oct 22 10:38 fort_rcg.c
さまざまな情報収集。flag によって収集すべき物を選択できる。特別行動の
設定も行なう。現在の行動をリセットして特別行動(act_flag)をセットする。
-rw-rw-r--    1 usr04098 user          718 Oct  5 15:53 fort_var.c
フォーチュン戦で使うグローバル変数。


include:
-rw-rw-r--    1 usr04098 user        18176 Oct  5 15:53 fort.h
ワークの型。
-rw-rw-r--    1 usr04098 user          164 Mar 16  2001 fort.mh
フォーチュンモーション。
-rw-rw-r--    1 usr04098 user         6224 Oct  5 15:53 fort_flag.h
フォーチュン状態フラグ。
-rw-rw-r--    1 usr04098 user         3598 Oct  5 15:53 fort_se.h
これは、全く使われていない。オルガの名残です。


action:
-rw-rw-r--    1 usr04098 user         3598 Oct  5 15:53 fort_act.h
アクト全体で使う汎用関数。
-rw-rw-r--    1 usr04098 user          142 Oct  5 15:53 fort_dbug.h
デバッグ用。
-rw-rw-r--    1 usr04098 user         3652 Oct  5 15:53 fort_fire.h
攻撃アクト。撃つ、りロードなど。
-rw-rw-r--    1 usr04098 user         2971 Oct  5 15:53 fort_move.h
移動アクト。蟹歩き、歩き、回転など。
-rw-rw-r--    1 usr04098 user         2565 Oct  5 15:53 fort_stll.h
静止アクト。
-rw-rw-r--    1 usr04098 user         2135 Oct  5 15:53 fort_wait.h
待ちアクト。モーション無しアクト。
-rw-rw-r--    1 usr04098 user         9672 Oct  5 15:53 fort_thk_attk.h
思考攻撃アクト。攻撃アクションの選択
-rw-rw-r--    1 usr04098 user         3035 Oct  5 15:53 fort_thk_move.h
思考移動アクト。移動アクションの選択
-rw-rw-r--    1 usr04098 user         6260 Oct  5 15:53 fort_list.h
アクションリスト。


effect:
-rw-rw-r--    1 usr04098 user         9145 Oct  5 19:19 efct_b_line.c
弾それエフェクト。
-rw-rw-r--    1 usr04098 user         6542 Oct  5 15:53 efct_bdy_flame.c
ライデンを包む炎。
-rw-rw-r--    1 usr04098 user         6381 Oct  5 15:53 efct_bullet.c
使われていない。没キャラ。
-rw-rw-r--    1 usr04098 user         4137 Oct  5 15:53 efct_drop.c
影だしキャラ。ドロップシャドウ。
-rw-rw-r--    1 usr04098 user         3414 Oct  5 15:53 efct_elec.c
電気走りエフェクト。没キャラ。
-rw-rw-r--    1 usr04098 user         5201 Oct  5 15:53 efct_elv_btn.c
エレベータの点滅ボタン。
-rw-rw-r--    1 usr04098 user         4586 Oct  5 15:53 efct_falldust.c
天井から降る埃。
-rw-rw-r--    1 usr04098 user        15760 Oct  5 19:19 efct_flame.c
オイルなどの炎。
-rw-rw-r--    1 usr04098 user        10330 Oct  5 19:19 efct_flow.c
フォークリフトが爆発した時に部屋中に溜る煙。
-rw-rw-r--    1 usr04098 user         3556 Oct  5 15:53 efct_heatiron.c
貫通した時に焼けた穴。鉄が焼けたように赤くなってフェードアウトする。
-rw-rw-r--    1 usr04098 user         6237 Oct  5 15:53 efct_oil.c
オイルを広げて、炎エフェクトも呼び出す。炎の当たり判定もここでやる。
-rw-rw-r--    1 usr04098 user         3863 Oct  5 15:53 efct_oilstain.c
広がるオイル。
-rw-rw-r--    1 usr04098 user         4348 Oct  5 15:53 efct_shadow.c
光源が通った際の伸び影。没キャラ。
-rw-rw-r--    1 usr04098 user         5775 Oct  5 15:53 efct_sidedust.c
フォークリフトが倒れた時の埃が舞うエフェクト。
-rw-rw-r--    1 usr04098 user         5092 Oct  5 15:53 efct_smoke.c
煙エフェクト。薄い。
-rw-rw-r--    1 usr04098 user         6513 Oct  5 15:53 efct_spark.c
壊れシステムを使ったエフェクト。弾んだりして消える。
-rw-rw-r--    1 usr04098 user         1116 Oct  5 15:53 efct_vib.c
振動エフェクト。
-rw-rw-r--    1 usr04098 user         8825 Oct  5 15:53 efct_wallscar.c
リニアガンが壁に当たった際の弾痕。


hang_lgt:
-rw-rw-r--    1 usr04098 user        12784 Oct  5 15:53 fort_hang.c
揺れライト。モデルの構造は[タンカー食堂のライト]と同じ。fort_lgt.cを呼
び出す。
-rw-rw-r--    1 usr04098 user         2185 Oct  5 15:53 fort_hang.h
揺れライトのワーク。
-rw-rw-r--    1 usr04098 user        20811 Oct  5 15:53 fort_lgt.c
天井ライト。フォーチュンの弾やグレネードで外れる。外れ落ちたライトは、
攻撃属性を持つ。
-rw-rw-r--    1 usr04098 user         2276 Oct  5 15:53 fort_lgt_src.c
efct_drop.cで使用される光源管理。
-rw-rw-r--    1 usr04098 user        12236 Oct  5 15:53 fort_wall_lgt.c
壁ライト。天井ライト同様落ち、攻撃属性を持つ。
-rw-rw-r--    1 usr04098 user         1846 Oct  5 15:53 fort_wind.c
マグニチュード管理。爆発などによってライトを揺らすため。


hide:
-rw-rw-r--    1 usr04098 user        14938 Oct  5 19:19 brk_anydust.c
汎用 壊れ物破片。モデルなどを指定すると、箱上に飛び散る。
-rw-rw-r--    1 usr04098 user         1657 Oct  5 15:53 brk_container.c
２Ｍコンテナ。基本的にモデル切替え。
-rw-rw-r--    1 usr04098 user         6706 Oct  5 15:53 brk_fl_bwheel.c
フォークリフトの後ろタイヤ。
-rw-rw-r--    1 usr04098 user        10813 Oct  5 15:53 brk_fl_frame.c
フォークリフトの上部フレーム。
-rw-rw-r--    1 usr04098 user         4854 Oct  5 15:53 brk_fl_fwheel.c
フォークリフトの前タイヤ。
-rw-rw-r--    1 usr04098 user         8230 Oct  5 15:53 brk_fl_lift.c
フォークリフトのリフト部分。
-rw-rw-r--    1 usr04098 user        11352 Oct  5 19:19 brk_fl_stick.c
フォークリフトのフレームの支え棒。挙動のみ。
-rw-rw-r--    1 usr04098 user        17291 Oct  5 15:53 brk_forklift.c
フォークリフト。本体の挙動、各種エフェクトの発動など。
-rw-rw-r--    1 usr04098 user         6595 Oct  5 15:53 brk_gas_cap.c
ドラム缶の蓋。
-rw-rw-r--    1 usr04098 user        10782 Oct  5 15:53 brk_gastank.c
ドラム缶。
-rw-rw-r--    1 usr04098 user        10891 Oct  5 15:53 brk_ironbox.c
１Ｍコンテナ。
-rw-rw-r--    1 usr04098 user         2931 Oct  5 15:53 brk_slide.c
ずり落ち。見栄えも良くなく問題が多そうなので、没。
-rw-rw-r--    1 usr04098 user         1730 Oct  5 15:53 brk_woodbox.c
１Ｍ木箱。
-rw-rw-r--    1 usr04098 user         2833 Oct  5 15:53 fort_obj.c
全体の統括(New???? Act Die GetResource)。
-rw-rw-r--    1 usr04098 user        24605 Oct  5 15:53 fort_obj.h
ワークの型。
-rw-rw-r--    1 usr04098 user         3206 Oct  5 15:53 fort_obj_act.c
フォーチュンの視線計算。
-rw-rw-r--    1 usr04098 user         5035 Oct  5 15:53 fort_obj_clb.c
当たった場合の処理。
-rw-rw-r--    1 usr04098 user         3230 Oct  5 15:53 fort_obj_hzx.c
動的当たりを貼る。
-rw-rw-r--    1 usr04098 user         7070 Oct  5 15:53 fort_obj_ini.c
狙いシステムへの登録など各種初期化。
-rw-rw-r--    1 usr04098 user          273 Oct  5 15:53 fort_sig.h
壊れ物とのシグナル通信用のデファイン。


misc:
-rw-rw-r--    1 usr04098 user        10873 Oct  5 15:53 fort_ceil.c
天井の崩れ。3パーツのモデルをモーション再生させて、降らす。
-rw-rw-r--    1 usr04098 user         6245 Oct  5 15:53 fort_ceil_dust.c
天井から降るコモデル。最初は、当たり付きでやっていたのだが重いので見な
い方に変更した。
-rw-rw-r--    1 usr04098 user         4567 Oct  5 15:53 fort_com.c
外部用コマンド。位置、グレネードの位置、フラグなどの取得。
-rw-rw-r--    1 usr04098 user         8690 Oct  5 15:53 fort_dbg.c
数々のデバッグコード。
-rw-rw-r--    1 usr04098 user         1228 Oct  5 15:53 fort_dbgcnf.c
数々のデバッグコード。
-rw-rw-r--    1 usr04098 user         3948 Oct  5 15:53 fort_gate.c
ジャバラシャッター。ダイナミックハザードを付けプレーヤー当たりを持つ。
-rw-rw-r--    1 usr04098 user         4635 Oct  5 15:53 fort_jnt.c
PRIM２で2つの関節を結ぶ汎用なもの。実際には、リニアガンのスリング
-rw-rw-r--    1 usr04098 user         5746 Oct  5 15:53 fort_motion.c
モーション再生君。自分用に汎用的な物が欲しかったため。


weapon:
-rw-rw-r--    1 usr04098 user         5308 Oct  5 15:53 fort_amo.c
カートリッジの挙動。システムの当たりを見ている。
-rw-rw-r--    1 usr04098 user         3195 Oct  5 15:53 fort_bar.c
フォーチュンバリアー。使われていない。
-rw-rw-r--    1 usr04098 user         2182 Oct  5 15:53 fort_blast.c
爆発系のフットバシキャラ。ものによって出すエフェクトを変えている。
-rw-rw-r--    1 usr04098 user         9775 Oct  5 19:19 fort_bul.c
リニアガンの弾。放電効果も一緒に出している。
-rw-rw-r--    1 usr04098 user         7312 Oct  5 15:53 fort_bul2.c
リニアガンの弾。使われていない。
-rw-rw-r--    1 usr04098 user         3296 Oct  5 15:53 fort_hzd.c
システムを使う汎用的な物。
-rw-rw-r--    1 usr04098 user         4162 Oct  5 15:53 fort_laser.c
レーザーポインター。ターゲットに反応して、通さないようにしている。
-rw-rw-r--    1 usr04098 user         7766 Oct 22 15:25 fort_lnr.c
リニアガン。ブローバック処理や弾を出したりする。
-rw-rw-r--    1 usr04098 user         1392 Oct  5 15:53 fort_tmplgt.c
テンプライトキャラ。システム的に重いので没。
-rw-rw-r--    1 usr04098 user         4784 Oct  5 15:53 fort_wep.h
汎用的なヘッダ。
--------------------------------------------------------------------
