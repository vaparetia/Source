USB file transfar
	2000/05/01	K.Uehara
	$Id: readme.txt,v 1.4 2001/02/14 01:20:41 usr01475 Exp $

USBリンクケーブルを使って、PCと相互にファイルをやりとりする。
プロトコルは独自プロトコル。
sceOpen, sceReadと同じインターフェースを持つ。
IOPからは、pcOpen, pcReadなど、pc*という形で呼出し可能。
EEからはusbOpenという形で呼び出す。

FS対応
SCEから「質問」で手に入れたAddDrv系の関数を使って
usb0:として実装。
fs.hは、SCEから入手したもの。ライブラリのバージョンアップによっては、
うまく動作しなくなるかも。

usbfs.h			ヘッダ
usbfs.a			EE呼出し用ライブラリ
usbfs.irx		IOPモジュール
nousbfs.irx		USBFSを使用しない時のダミーモジュール

サポートしているリンクケーブルは2000/05/02現在、以下。
	NEC PK-UP010	(AN2720)
	ELECOM UC-NBB	(PL2302)

各関数(EE/IOPとも)
プロトコルは暫定

C: クライアントからサーバーへ。
S: サーバーからクライアントへ。

S:レスポンスで、Error(NG)は、
01 01 er 00 00 00 00 00 00

usbInit( char *id )
PCとのリンクを初期化する。idは５バイト以下の文字列

C: 01 01 id id id id id id	: init packet
S: 01 00 00 00 00 00 00 00  : OK

usbOpen( char *filename, int mode )

C: 01 02 md 00 fl fl fl fl  : open ( md = mode, fl = filename length )
S: 01 00 00 00 00 00 00 00  : OK
C: filename( len = fl )
S: 01 00 fd 00 00 00 00 00  : OK ( return fd )

usbClose( int fd )

C: 01 03 fd 00 00 00 00 00  : close ( fd )
S: 01 00 00 00 00 00 00 00  : OK

usbRead( int fd, void *buffer, int size )

C: 01 04 fd 00 sz sz sz sz : read
S: 01 00 00 00 sz sz sz sz : OK send start ( sz = size )
C: 01 00 00 00 00 00 00 00 : OK
S: data(size)
C: 01 00 00 00 00 00 00 00 : OK
S: 01 00 00 00 00 00 00 00 : OK

usbWrite( int fd, void *buffer, int size )

C: 01 05 fd 00 sz sz sz sz : write
S: 01 00 00 00 00 00 00 00 : OK recieve start
C: data(size)
S: 01 00 00 00 00 00 00 00 : OK

usbLseek( int fd, int offset, int where )

C: 01 06 fd wh of of of of : seek
S: 01 00 00 00 ps ps ps ps : OK

// Dopen系

usbDopen( char *dirname )

C: 01 07 00 00 fl fl fl fl  : dopen ( md = mode, fl = filename length )
S: 01 00 00 00 00 00 00 00  : OK
C: filename( len = fl )
S: 01 00 fd 00 00 00 00 00  : OK ( return fd )

usbDread( int fd, struct sce_dirent *buf )

C: 01 08 fd 00 sz sz sz sz  : dread ( sz = sizeof( sce_dirent ) )
S: 01 00 ct 00 00 00 00 00  : 終了の場合はct = 0;
S: data (sz)                : Errorのときは送られない

usbDclose( int fd )
C: 01 09 fd 00 00 00 00 00  : dclose
S: 01 00 00 00 00 00 00 00  : OK

// usbGetStat

usbGetStat( char *name )
C: 01 07 00 00 fl fl fl fl  : getstat ( md = mode, fl = filename length )
S: 01 00 00 00 00 00 00 00  : OK
C: filename( len = fl )
S: 01 00 00 00 sz sz sz sz  : OK ( return size )
S: data (sz)                : Errorのときは送られない

// PUTS

usbPuts( char *mesg )
C: 01 10 00 00 sz sz sz sz  : puts ( sz = len )
C: string( sz )             : 流しっぱなし
