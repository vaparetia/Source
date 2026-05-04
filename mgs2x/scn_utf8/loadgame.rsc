/*
	ファイル名		loadgame.rsc
	作成日			2001/08/06
	作者			Y.YANO
	説明			ロードの際に出るメッセージ(多言語対応版)
	$Id: loadgame.rsc,v 1.6 2002/09/25 10:57:04 usr03700 Exp $
*/


resource ロードゲームリソース日本語 {
// 0
メモリーカードが見つからなかった:
	'メモリーカード(PS2)が見つかりません',

// 1
メモリーカードにファイルが無い:
	'ファイルが見つかりません',

// 2
確認:
	'これでよろしいですか？',

// 3
ファイルロード:
	'ファイルのロード中',

// 4
成功:
	'ファイルのロードが正常に終了しました',

// 5
失敗:
#ifdef PSX2
	'ファイルのロード中にエラーが発生しました',
#else
	'%sを読み込めません|'
	'Ａボタンを押して続けてください'
#endif

// 6
はい:
	'Yes'

// 7
いいえ:
	'No'
};

resource ロードゲームリソース英語 {
// 0
メモリーカードが見つからなかった:
	'MEMORY CARD (PS2) not inserted.'

// 1

// これはＺＯＥでは使っていなかったので、作りました。
// これでいいんかいな？

メモリーカードにファイルが無い:
	// 'File not found.'
#ifdef PSX2
	'No MGS2 Substance data found.'
#else
	'No %s found.'
#endif

// 2
確認:
	'Load?'

// 3
ファイルロード:
	'Now loading.'

// 4
成功:
	'Load completed.'

// 5
失敗:
#ifdef PSX2
	'Load failed.'
#else
	'Unable to load %s.|'
	'Press A to continue.'
#endif

// 6
はい:
	'Yes'

// 7
いいえ:
	'No'
};

resource ロードゲームリソースフランス語 {
// 0
メモリーカードが見つからなかった:
	{MEMORY CARD (PS2) non ins&'er&'ee.}

// 1
メモリーカードにファイルが無い:
	// {Fichier non trouv&'e.}
	{Aucune donn&'ee MGS2 trouv&'ee.}

// 2
確認:
	'Charger?'

// 3
ファイルロード:
	'Chargement en cours.'

// 4
成功:
	{Chargement termin&'e.}

// 5
失敗:
#ifdef PSX2
	'Echec du chargement.'
#else
	'Chargement de %s impossible.|'
	'Appuyer sur A pour continuer.'
#endif

// 6
はい:
	'Oui'

// 7
いいえ:
	'Non'
};


resource ロードゲームリソースドイツ語 {
// 0
メモリーカードが見つからなかった:
	'MEMORY CARD (PS2) ist nicht eingelegt.'

// 1
メモリーカードにファイルが無い:
	// 'Datei nicht gefunden.'
	'Keine MGS2-Daten gefunden.'

// 2
確認:
	'Laden?'

// 3
ファイルロード:
	'Datei wird geladen.'

// 4
成功:
	'Datei ist geladen.'

// 5
失敗:
#ifdef PSX2
	'Datei konnte nicht geladen werden.'
#else
	'%s kann nicht geladen werden.|'
	'Weiter mit A-Taste.'
#endif

// 6
はい:
	'Ja'

// 7
いいえ:
	'Nein'
};

resource ロードゲームリソースイタリア語 {
// 0
メモリーカードが見つからなかった:
	'MEMORY CARD (PS2) non inserita.'

// 1
メモリーカードにファイルが無い:
	// 'File non trovato.'
	'Nessun dato MGS2 trovato.'

// 2
確認:
	'Vuoi caricare?'

// 3
ファイルロード:
	'Caricamento in corso.'

// 4
成功:
	'Caricamento completato.'

// 5
失敗:
#ifdef PSX2
	'Caricamento non riuscito.'
#else
	'Impossibile caricare %s.|'
	'Premere A per continuare.'
#endif

// 6
はい:
	{S&`i}

// 7
いいえ:
	'No'
};

resource ロードゲームリソーススペイン語 {
// 0
メモリーカードが見つからなかった:
	'No hay una MEMORY CARD (PS2) introducida.'

// 1
メモリーカードにファイルが無い:
	// 'No se ha encontrado el archivo.'
	{No se ha encontrado informaci&'on sobre MGS2.}

// 2
確認:
	'&~?Cargar?'

// 3
ファイルロード:
	'Cargando.'

// 4
成功:
	'Se ha cargado satisfactoriamente.'

// 5
失敗:
#ifdef PSX2
	'No se ha cargado.'
#else
	'Imposible cargar %s.|'
	'Presionar A para continuar.'
#endif

// 6
はい:
	{S&`i}

// 7
いいえ:
	'No'
};
