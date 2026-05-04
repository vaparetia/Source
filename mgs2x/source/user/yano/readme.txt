メタルギアソリッド2(PS2)
ランダムファイル　　

作成者:矢野悠子
作成日:2001.11.01

<プログラム一覧　ディレクトリ階層>

user/yano/object/ kan.c
		  float_dust.c

	 /effect/ kan_splash2.c
		  kan_splash3.c
		  glass_mist.c
		  glass_mist2.c
		  duct_sight.c
		  duct_sight2.c
		  goruruko.c

	 /etc/    yn_utl.c

system/libutl/	  jpeg.h
	       	  jpeg_common.c
	       	  jpeg_encode.c
               	  jpeg_decode.c


<プログラム内容>
/* kan.c */
水中ステージにある、浮いている缶、沈んでいる空き缶、浮いているヒヨコの動きを制御。

/* float_dust.c */
水中ステージにある、水中に漂うペットボトル、紙コップ、バルカン人形の動きを制御。

/* kan_splash.c */
浮いている缶を撃ったときに出る最初の水しぶき。

/* kan_splash2.c */
浮いている缶を撃ったときに出る水しぶき。第二段階。

/* kan_splash3.c */
浮いている缶を撃ったときに出る水しぶき。第三段階。

/* glass_mist.c */
スプレーをガラスや鏡に向けて吹くと曇るエフェクトで、glass_mist2.c　の呼び出しだけ行っている。

/* glass_mist2.c */
上記のプログラムの実体。ポリゴンの貼り付けを行っている。

/* duct_sight.c */
没。

/* duct_sight2.c */
ダクトに入っときに画面の外周が暗くなるエフェクト。

/* gorururko.c */
ゴルルコ兵装時主観のときのマスクのエフェクト。

/* yn_utl.c */
よく使う関数をまとめておいてある。

/* jpeg.h */
写真データ圧縮プログラムのヘッダファイル。

/* jpeg_encode.c */
写真データ圧縮プログラムの圧縮ルーチン。

/* jpeg_decode.c */
写真データ圧縮プログラムの解凍ルーチン。

/* jpeg_common.c */
jpeg_encode.c, jpeg_decode.cに共通のプログラム。


以上。