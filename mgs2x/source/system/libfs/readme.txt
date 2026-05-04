LIBFS ファイルモジュール
$Id: readme.txt,v 1.1.1.3 2002/11/19 11:42:40 Yoshizawa1 Exp $

--

ファイル関係の操作はすべてここを介して行なう。

HD/CD上のファイルを同じインターフェースで扱い、
また、ステージデータをアーカイブした形式（圧縮／無圧縮）と
開発用のディレクトリツリー形式の両方で切替えて読み込むことができる。

ファイルの読み込みは開発用のステージディレクトリを除いて、すべて
ファイル番号とセクタ単位のオフセットで行なう。

デバッグ用にUSBへの入出力機能も持つ。

--

FS系関数セッティング

以下、
	CD	CD/DVD
	HDU	PS2外づけ
	HD	開発環境のHD
	USB 開発環境のUSB
と表記。

・実機バージョン(HDU対応)
	elf
		CDから読み込み
	system module
		CDから読み込み
	user module
		CDから読み込み
	stage.dat
		CD/HDUから読み込み
	*.dat
		CD/HDUから読み込み

・プログラマ/シナリオ開発機バージョン
	elf
		HDから読み込み
	system module
		HDから読み込み
	user module
		HDから読み込み
	stage data (File Tree)
		HDから読み込み
	*.dat
		HDから読み込み

・デザイナー/デモプレビューバージョン(DS+HDU)
	booter
		CD/USBから読み込み
	elf
		HDUから読み込み
	system modules
		CDから読み込み
	user modules
		HDUから読み込み（アーカイブ？）
	stage.dat
		HDUから読み込み
	*.dat
		HDUから読み込み
	log
		USBへ書き出し

--
■ モジュールについて

以下の各環境に応じて、IOPのモジュールの構成と読み込みPATHが変わる。
大まかに、HDU, USB, USERとモジュールグループを分けると以下のようになる。

hd(開発環境)
	HDU,USB,USERを、host0:から
	ファイルシステムはhost0:
	ステージはディレクトリ形式

usb(USBファイルシステム)
	HDU,USBをcdrom0:から
	USERをusbfsから
	ファイルシステムはusbfs
	ステージはディレクトリ形式

usbd(USBファイルシステム/デバッグ用)
	HDU,USB,USERをhost0:から
	ファイルシステムはusbfs
	ステージはディレクトリ形式

hdu(PS2HDユニット／プレビュー環境)
	HDUをcdrom0:から
	USB,USERをpfs0:から
	printfはusb
	ファイルシステムはpfs0:
	ステージはパッキング形式

hdud(PS2HDユニット／プレビュー環境/デバッグ用)
	HDU,USB,USERをhost0:から
	printfはusb
	ファイルシステムはpfs0:
	ステージはパッキング形式(BINFIX)

cd(実機)
	HDU,USB,USERを、cdrom0:から
	ファイルシステムはcdbios(pfs0:)
	ステージはパッキング形式

cdd(実機/デバッグ用)
	HDU,USB,USERを、host0:から
	ファイルシステムはcdbios(pfs0:)
	ステージはパッキング形式(BINFIX)

	

