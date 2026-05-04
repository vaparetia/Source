PS2HD

PS2HD用のFTPモドキ
EEのルーチンはまだちゃんと動いていないみたいなので、
IOPのプログラムとして実装する。

以下のコマンドを実装
基本的にhdd0:, pfs0:のみを使うようにしてある。
hdd0:,pfs0:は指定しないでいい。

-- パーティション --

allformat	全パーティションのフォーマット
pdir		パーティションリストを出力
pinfo		指定パーティションの情報出力
	pinfo <パーティション名>

create		指定サイズのパーティションを作成
	create "<パーティション名>,read_pass,full_pass,<SIZE>,PFS,name"
remove		パーティションを削除
	remove	"<パーティション名>,full_pass"

format		パーティションをformat
	format	"<パーティション名>,full_pass"

mount		パーティションをpfs0:にマウント、以下のファイルコマンドを使用可能に。
	mount <パーティション名>,read_pass,full_pass rwc
umount		パーティションをpfs0:からアンマウント
	umount


-- ファイル --
dir
mkdir
rmdir

get
	get filename
put
	put filename
copy
	copy filename filaname
delete
	delete filename
rename
	rename filename filename
info
	info filename
cd
	cd directory


