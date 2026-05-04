hdudown

PS2デバッギングステーションにつながっているハードディスクユニットに
USB, PCMCIAのNIC経由でデータをダウンロードするプログラム。
以下のような流れで、ダウンロード／実行を行なう

キーパッドチェック
	L1 + ○が押されていればHDをフォーマット
	×が押されていればダウンロードせず最初に見つけたパーティションから起動

ネットワークチェック
	メモリカード上のファイルKCEJ-NETCONFを読み込み、ホストを決定する。
	KCEJ-NETCONFには複数のホストを書け、選択できる。

ダウンロード
	hdinst.cnfスクリプトファイルを読み込み、それにしたがってダウンロード
	タイムスタンプチェックを行ない、同じファイルはダウンロードしない

実行
	HDからhdboot.cnfを読み込んでBOOT2セクションのELFを起動


このディスクには最低限HDDユニットを起動し、
しかもUSBFSをマウントする必要があるため、HDDの起動用のIPKはCD上にある。

---
hdinst.cnfファイルの文法
#はコメント行

mount hdd0:mgs2,fpwd,rpwd,1G,PFS
		# マウント。SCEライブラリにそのまま渡される。

dir  pfs0:/mgs2
		# ディレクトリ作成。すでにある場合は無視。
dir  pfs0:/modules

file ./modules/modhdu.ipk	pfs0:/modules/modhdu.ipk
		# ファイルのダウンロード

zfile ./vox.z		pfs0:/mgs2/vox.dat
		# zlibencによって圧縮されたファイルのダウンロード

exec pfs0:/hdboot.elf
		# 実行
---
メモリーカードに保存するKCEJ-NETCONFの内容は以下の通り。

--- old version
netfs0:$host:sinonome.konami:49275:11475:10000:/usr/local4/worm/mgs2/cdrom.img
---

netfs0:$host:サーバー名:ポート番号(49275):ユーザーID:グループID:ディレクトリ

である。余分なスペース、改行を入れてはいけない。

複数のサーバーを切替えることもできる。
--
: worm sinonome
worm:netfs0:$host:sinonome.konami:49275:11475:10000:/usr/local4/worm/mgs2/cdrom.img
sinonome:netfs0:$host:sinonome.konami:49275:11475:10000:/usr/local2/hdimage/mgs2/cdrom.img
--
一行目の行頭が':'であるなら、セレクトタグの設定になる。
現在の設定では16こまでセレクトが可能。
その下にタグ:に続けて、上と同様のサーバー設定を書く。
余分なスペース、改行を入れてはいけない。
