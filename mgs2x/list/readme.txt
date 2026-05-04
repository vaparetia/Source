

                          SRCファイルの書き方

1999/08/10 T.Morita
$Id: readme.txt,v 1.33 2001/09/18 11:43:23 usr04098 Exp $


０.始めに

ランタイムアップツールで完了したデータから cdrom.img を作るためのツー
ルである dpack とそれのために必要なSRCファイルについて、説明します。



１.SRCファイルとは？

SRCファイルは、dpack に食わして、自分の欲しいデータの cdrom.img を作る
ためのものです。SRCファイルは、大きく分けて、コマンド群とディレクトリ
群に分けられます。コマンドは、すべて予約語となります。コマンド以外の文
字列は、ディレクトリとして認識されます。基本構造として以下のようになり
ます。

[コマンド]     [引数１] [引数2] ....
[ディレクトリ] [サブディレクトリ または 拡張子付ファイルネーム]

ここでKMSがディレクトリ内に含まれている場合、同じディレクトリ内のDARファ
イル（BMP,PICの集まったもの）からTRI（テクスチャー）を自動的に生成しま
す。一般的にTRIの名前は、サブディレクトリの名前となります。また、ディ
レクトリに含まれるモデルに対し一つのTRIが生成されます。

SRCファイルのベースネーム（拡張子をとった名前）は、ステージ名と同じで
なければなりません。以下でファイル名という場合は、ディレクトリ付きのこ
とを意味します。



２.SRCのコマンド

コマンドには、引数をいくつも必要なものから、引数が無いものまであります。
まずピリオドで始まるコマンドは最初に書いて下さい。

block	.ブロック名
	各SRCファイルには必ずなんらかの名前で入ります。一般に.nocacheや
	.residentなどをブロック名の所に入れることになります。また、ブ
	ロックの切り分けに使われる意味も持っており、data.cnfに一度フラッ
	シュされます。以下に既に決められた意味を持つブロック名です。
		.nocache  w???.binが入る。???はステージ名です。
		.cache    ブロックの最後にsenario.gcxが入る。
		.resident 常駐データが入るブロック。
		.?????    分割ロード時のブロック名になります。

rename      src dst
	srcおよびdstは拡張子付である必要があります。includeするファイ
	ル内でも効果があります。strcode回避コマンド。

texrename   model.kms src.bmp dst.bmp
	model.kms内またはpackall内にある src.bmpをdst.bmpに書き換えます。
	includeするファイル内でも効果があります。モデル名がrenameコマ
	ンドで名前を変えた場合は、変えた名前を指定して下さい。strcode
	回避コマンド。ディレクトリは、指定できませんので、御利用の際は
	ご注意下さい。拡張子は、BMPのみです。

hzd     name
	ステージ名を入れて下さい。

icon    icon/name.cnf
	iconのコンフ名を入れて下さい。コンフ名.icoが生成されます。この
	コンフはlist/icon/の下でCVS管理します。一つのアイコンにつき一
	つのコンフファイルが必要です。コンフファイルの書き方は、狩野さ
	んまで宜しくお願いします。４章にサンプルあります。

mtn	name[.mls or .mar]
	SRCファイルと同じディレクトリにある mls ファイルを参照し mar 
	ファイルを構築します。作られる mar ファイルは name.mar になり
	ます。mls については最終章を参照して下さい。

include file
	ファイルを内挿し、解析します。このfileは、ディレクトリ付で指定
	できます。

lt2	file [dx dy dz]
	lt2のファイルを dx dy dz で分割をします。dx dy dzを指定しなけ
	ればデフォルトの5000 5000 5000が分割数になります。

arkms   dir/file[.kls] [packed]
	複数のモデルからKMSを生成するコマンドです。敵兵のバリエーショ
	ンになどに使われます。
	ディレクトリつきファイル名で指定します。基本的にarkmsで作られ
	る KMS に対してひとつのTRIにまとめなければなりません。なってい
	ない場合には止まります。なお明示的にpackallなどでTRIを作成した
	場合、packedをつけることで、arkmsでのTRI生成を止めることができ
	ます。

kms_archiver  name.zms aaa.kms ....
	複数の同じ形をしたKMSをまとめます。メモリ消費が、かなり軽減さ
	れるため段ボールのようなテクスチャー違いのモデルは必ずやるよう
	にしましょう。モデルの指定後でないとエラーで止まりますので後の
	方に書くことをお勧め致します

anmtex  name.rat dir_or_files
	テクスチャーアニメーションんデータを生成します。必ず名前を入力
	しなければなりません。ディレクトリを指定した場合は、アルファベッ
	ト順に名前が入るのと同値です。例は、下のサンプルにあります。


anm	XSI-file_or_dir [-n] [-N] [-S scale値]
	頂点ストリームデータを作成し、更に必要なKMSおよびCV2を取ってき
	ます。ディレクトリ指定の場合のその下にある全てのXSIファイルを
	ANMに変換します。-n は、KMSおよびCV2を取ってこないオプション
	です。他のコマンドでCV2やKMSを取ってきている場合に付ける必要が
	あります。-Nは、法線のアニメーションデータも生成します。陰影部
	分もアニメーションさせたいときは法線のアニメーションも要ります。
	スケール値は、rptに指定したスケール値を入れて下さい。

gcx	name[.gcx]
	複数のGCXを持つステージに必要。どの場所にかいても良いが、順序
	は senario.gcx の後に指定順に加わる。拡張子は、なくてもOK

l2d	L2D-file
	２Ｄモジュールで必要と思われるo2dを生成し、l2dファイルから必要
	なビットマップをかき集めTRIを生成します。デザイナーには必要な
	テクスチャーをl2dと同じディレクトリのランタイムに上げる必要が
	あります。そのように促して下さい。

cvd     dir or file [vnuVN]
	モデルのディレクトリかディレクトリ付きファイル名(拡張子付き)を
	指定して下さい。ディレクトリの場合、その下にあるCV2をすべてコ
	ピーしてきます。ファイルの場合は、そのファイルのみのCV2ファイ
	ルをコピーしてきます。また必要な情報のみを取ってくるオプション
	を付けることでファイルを小さくすることができます。
		v	頂点情報のみ
		n	法線情報のみ
		u	UV情報のみ
		V	ユーザー頂点情報のみ
		N	ユーザー法線情報のみ
	一度に複数指定することができます。指定順は、順不同です。

pack系のコマンドは、特別に指定されたファイル群をまとめてTRIにしてしま
う機能です。ファイル群に指定できるのは、bmp、dar、ディレクトリの３種類
です。ディレクトリの場合、ディレクトリ以下にあるものすべてをかき集める
対象にします。すでにTRIに入ったファイルは、ファイルの重複を避けるため、
他のTRIに入ることはありません。とりあえずまとめておきたいTRIを作る場合
は、pack系のコマンドを最初の方にやっておくことをお勧めします。

pack_all   pack.tri file1 [file2 ...]
	指定されたファイル群の中から半透明以外のテクスチャーを集めて
	TRIにします。

pack_trn   pack.tri file1 [file2 ...]
	指定されたファイル群の中から半透明のものだけを選んでTRIにまと
	めます。

pack_trnall pack.tri file1 [file2 ...]
	指定されたファイル群を半透明のものとして、すべてTRIにまとめま
	す。

pack_trnflush pack.tri file1 [file2 ...]
	指定されたファイル群の中から半透明のものだけを選んでTRIにまと
	めます。このとき半透明のバッファ内をフラッシュします。もしここ
	に書いた物だけをTRIにまとめたい場合は、必ず先頭で行なうように
	して下さい。


３.ディレクトリ

基本的にディレクトリを指定する場合、そのディレクトリ以下にあるすべての
ファイルを取ってくることになります。サブディレクトリを指定することがで
きます。サブディレクトリが存在しない場合、そのベースネームと同じファイ
ルを取ってくることになります。例えば、前のメタルギアのFIXではLITやHZD
に当たります。



４.例
例の記述をしておきます。参考にして下さい。
------- Begin --------
block .use r_plt0    //レジデントの指定
block .nocache
block .cache         //読み込みステージであること意味する

include         common.src

human           rai_def
human           gbs_def                                //KMSのディレクトリ指定
goods/bottle	bottle00.kms bottle01.kms bottle02.kms //KMSの指定

cvd             human/rai_def	vnN
cvd             human/gbs_def/gbs_skl.cvd Vnv

pack_all        effect2.tri world/w00a/swing.bmp world/w00a/glass/ weapon/usp/usp.dar

lt2             w00a
hzd             w00a
world           w00a

anmtex  wave.rat effect/wave
anmtex  steam.rat effect/steam/steam_b.bmp effect/steam/steam_c.bmp effect/steam/steam_d.bmp

icon		icon/testicon.cnf

effect          flare

rename          rai_def.kms raiden.kms
rename          gbs_def.kms lopryhei.kms
texrename       lopryhei.kms  gbs_uwe_h6.bmp gbs_uwe_h6_x.bmp
texrename       gca_gun.kms   gca_cam06.bmp  gca_cam06_x.bmp
------- End --------



５.dpackについて

dpackに引数を与えずに実行すればHELPが出てきます。これでほとんどの
説明がいりませんが、しておきます。

Usage: dpack  [dir] [file.src] [-options...]
options:
        -t  get data from temporary
        -c 'command' get data as the command specifies
        -d  delete all files in the directory of the destination.
        -log <logfile>  save the warning to logfile.
        -cm2  leave the cm2-file to current dir.

基本的には、上記で説明したSRCファイルを用意し、ディレクトリを指定すれ
ば、そのディレクトリにデータファイルをコピーし、data.cnfを自動生成しま
す。ディレクトリを指定しなければ、コマンドを実行した場所に展開されます。
指定の順序はありません。好きな用にやって構いません。

オプション
	-t 展開元がFIXでなくTemoporaryに代わります。
	-c シングルクォーテーションで囲んだコマンドを送ることができま
	   す。dpack -c 'human raiden'などするとその場にライデンのデー
	   タがその場に展開されます。
	-d 格納先のディレクトリが存在した場合に中身をすべて消すオプショ
	   ンです。
        -log <logfile>  生成ログをファイルに落します。（デバッグに使用）
        -cm2  CM2のファイルをカレントディレクトリに残します。

何か不明な点、バグが出た場合は、すぐにお近くの森田に一報下さい。



５.mls（モーションリスト）について

  モーションリストは、基本的にファイル名の羅列になります。さらにディレ
クトリを跨ることを想定していますので、ディレクトリの下にある場にはディ
レクトリを含めたファイル名での指定になります。

モーションリストから mar を作る makemar もあります。
Usage: makemar list-file[.mls] [output[.mar]] [-options]
Make mar-file from mt3 motion file list.
options:
        -l <dir> directory for input files.
        -e output the enum-file.
        -n  Do not make MAR-file.
        -s <sls-file> Input SLS-file
        -sd SAR-debug mode

このオプション-eを付けることでenum.hを吐くことができます。ファイル名の
拡張子を省いたものが enum のリストになります。拡張表現として、名前を変
更したい時は、ファイル名の後に enum 用の名前を指定することができます。
また名前のところにskipを入れることで、その mt3 は、mar に入れないよう
にすることができます。

記述例です。
----------ここから-----------
gbs/gbs_ak_arml_idle.mt3
gbs/gbs_ak_arms_idle.mt3 skip
gbs/gbs_ak_legs_idle.mt3 call_team
----------ここまで-----------
上記のMLSを加えることで
enum{
gbs_ak_arml_idle
call_team
} ;
の enum ファイルを吐かせることができます。

捕捉ですが、dpack では、内部で、
makemar gbs.mls gbs.mar -l/usr/local/develop/fix/mtn/
を実行しているのと同値であります。




おわるぜ！！ベイベー！！
