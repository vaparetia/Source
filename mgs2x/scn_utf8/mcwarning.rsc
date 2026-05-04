/*
	ファイル名		mcwarning.rsc
	作成日			2002/06/08
	作者			Y.YANO
	説明			起動直後に出るメモリーカード警告メッセージ(多言語対応版)
	$Id: mcwarning.rsc,v 1.8 2002/10/21 10:01:05 usr01475 Exp $
*/


resource メモリーカード警告メッセージ日本語 {
// 0
空き無し:
	//'メモリーカード(PS2)に|'
	//'空き容量が足りません|'
	//'セーブファイルの保存には、|'
	//'?キロバイト以上の|'
	//'空き容量が必要です|'
	//'このままゲームを開始しますか？'
#ifdef PSX2
	'新規にセーブファイルを作成するためには｜'
	'メモリーカード(PS2)に空き容量が足りません|'
	'作成には?キロバイト以上の空き容量が必要です|'
	'このままゲームを開始しますか？'
#else
	'ハードディスクに空き容量が|'
	'足りません。セーブファイルの保存には、|'
	'?ブロック以上の空き容量が必要です。|'
	'このままゲームを開始しますか？'
#endif

// 1
カード無し:
	//'メモリーカード(PS2)が|'
	//'ささっていません|'
	//'セーブファイルの保存には、|'
	//'?キロバイト以上の|'
	//'空き容量が必要です|'
	//'このままゲームを開始しますか？'
	'メモリーカード(PS2)がささっていません。|'
	'セーブファイルの保存には、|'
	'?キロバイト以上の空き容量が必要です。|'
	'このままゲームを開始しますか？'

// 2
エラー:
	//'メモリーカード(PS2)検出で|'
	//'エラーが発生しました|'
	//'セーブファイルの保存には、|'
	//'?キロバイト以上の|'
	//'空き容量が必要です|'
	//'このままゲームを開始しますか？'
	'メモリーカード(PS2)検出でエラーが発生|'
	'しました。セーブファイルの保存には、|'
	'?キロバイト以上の空き容量が必要です。|'
	'このままゲームを開始しますか？'

// 3
はい:
	'はい'

// 4
いいえ:
	'いいえ'
};


resource メモリーカード警告メッセージ英語 {
// 0
空き無し:
#ifdef PSX2
	'There is insufficient space|'
	'on the MEMORY CARD (PS2).'
	'At least ?KB of free space|'
	'is needed to save your game.|'
	'Will you start the game as it is?'
#else
#if 0
	'Your Xbox doesn\'t have|'
	'enough free blocks to save games.|'
	'Press A to continue without saving|'
	'or B to free more blocks.'
#else
	'There are insufficient free blocks|'
	'on the hard disk. At least 20 blocks|'
	'are needed to save your game.|'
	'You have %d free %s and|'
	'you need %d more %s to be freed.|'
	'Press A to continue without saving|'
	'or B to free more blocks.'
#endif
#endif

// 1
カード無し:
	'There is no MEMORY CARD (PS2)|'
	'inserted.|'
	'At least ?KB of free space|'
	'is needed to save your game.|'
	'Will you start the game as it is?'

// 2
エラー:
	'There was an error when detecting|'
	'the MEMORY CARD (PS2).|'
	'At least ?KB of free space|'
	'is needed to save your game.|'
	'Will you start the game as it is?'

// 3
はい:
	'Yes'

// 4
いいえ:
	'No'
};

resource メモリーカード警告メッセージフランス語 {
// 0
空き無し:
#ifdef PSX2
	'Il n\'y a pas assez d\'espace sur la|'
	'MEMORY CARD (PS2).|'
	'Un minimum de ?Ko d\'espace libre|'
	'est requis pour sauvegarder la partie.|'
	'Voulez-vous commencer &\`a jouer?'
#else
//	'Il n\'y a pas assez d\'espace sur la|'
//	'disque dur. Un minimum de 5 blocs d\'espace|'
//	'libre est requis pour sauvegarder la partie.|'
//	'Appuyer sur A pour jouer sans sauvegarder.|'
//	'Appuyer sur B pour lib&\'erer des blocs.'

	'Il n\'y a pas assez de blocs libres sur|'
	'le disque dur. Il faut au moins 20 blocs|'
	'pour enregistrer une partie.|'
	'Vous avez %d bloc%s libre%s mais il faut|'
	'%d bloc%s libre%s en plus. Appuyez sur|'
	'A pour continuer sans enregistrer|'
	'ou sur B pour lib&\'erer plus de blocs.'
#endif

// 1
カード無し:
	'Aucune MEMORY CARD (PS2)|'
	'n\'est ins&\'er&\'ee.|'
	'Un minimum de ?Ko d\'espace libre|'
	'est requis pour sauvegarder la partie.|'
	'Voulez-vous commencer &\`a jouer?'

// 2
エラー:
	'Une erreur s\'est produite pendant|'
	'la d&\'etection de la MEMORY CARD (PS2).|'
	'Un minimum de ?Ko d\'espace libre|'
	'est requis pour sauvegarder la partie.|'
	'Voulez-vous commencer &\`a jouer?'

// 3
はい:
	'Oui'

// 4
いいえ:
	'Non'
};


resource メモリーカード警告メッセージドイツ語 {
// 0
空き無し:
#ifdef PSX2
	'Nicht genug Speicherplatz auf der|'
	'MEMORY CARD (PS2).|'
	'Zum Speichern sind mind.|'
	'?KB freier Speicherplatz n&~otig.|'
	'M&~ochten Sie das Spiel so beginnen?'
#else
//	'Nicht genug Speicherplatz auf der|'
//	'Festplatte. Zum Speichern sind mind.|'
//	'5 Bl&~ocke freier Speicherplatz n&~otig.|'
//	'A dr&~ucken, um ohne zu speichern weiterzuspielen,|'
//	'B dr&~ucken, um f&~ur mehr freie Bl&~ocke zu sorgen.'
	
	'Nicht gen&~ugend freie Bl&~ocke auf der|'
	'Festplatte. Mindestens 20 Bl&~ocke sind|'
	'zum Speichern des Spiels erforderlich.|'
	'Sie haben %d freie%s und m&~ussen %d|'
	'mehr %s freimachen. A dr&~ucken,|'
	'um ohne Speichern fortzufahren oder B,|'
	'um mehr Bl&~ocke freizumachen.'
#endif

// 1
カード無し:
	'Es ist keine MEMORY CARD (PS2)|'
	'eingelegt.|'
	'Zum Speichern sind mind.|'
	'?KB freier Speicherplatz n&~otig.|'
	'M&~ochten Sie das Spiel so beginnen?'

// 2
エラー:
	'Fehler beim Lesen der|'
	'MEMORY CARD (PS2).|'
	'Zum Speichern sind mind.|'
	'?KB freier Speicherplatz n&~otig.|'
	'M&~ochten Sie das Spiel so beginnen?'

// 3
はい:
	'Ja'

// 4
いいえ:
	'Nein'
};


resource メモリーカード警告メッセージイタリア語 {
// 0
空き無し:
#ifdef PSX2
	'Spazio memoria insufficiente|'
	'sulla MEMORY CARD (PS2).|'
	'Per salvare il gioco occorrono|'
	'almeno ?KB di memoria libera.|'
	'Vuoi iniziare il gioco cos&`i com\'&`e?'
#else
//	'Spazio memoria insufficiente sulla|'
//	'disco rigido. Per salvare il gioco occorrono|'
//	'almeno 5 blocchi di memoria libera.|'
//	'Premere A per continuare senza salvare oppure|'
//	'B per liberare dei blocchi.'

	'Blocchi liberi insufficienti sul disco|'
	'rigido. Per salvare il gioco sono|'
	'necessari almeno 20 blocchi.|'
	'%s disponibil%s %d blocc%s liber%s.|'
	'Occorre liberare ancora %d blocc%s.|'
	'Premere A per continuare senza salvare|'
	'o B per liberare pi&`u blocchi.'
#endif

// 1
カード無し:
	'Nessuna MEMORY CARD (PS2)|'
	'inserita.|'
	'Per salvare il gioco occorrono|'
	'almeno ?KB di memoria libera.|'
	'Vuoi iniziare il gioco cos&`i com\'&`e?'

// 2
エラー:
	'Errore nel rilevamento della|'
	'MEMORY CARD (PS2).|'
	'Per salvare il gioco occorrono|'
	'almeno ?KB di memoria libera.|'
	'Vuoi iniziare il gioco cos&`i com\'&`e?'

// 3
はい:
	'S&`i'

// 4
いいえ:
	'No'
};


resource メモリーカード警告メッセージスペイン語 {
// 0
空き無し:
#ifdef PSX2
	'No hay suficiente espacio libre en la|'
	'MEMORY CARD (PS2).|'
	'Para guardar el juego necesitas al menos|'
	'?KB libres. &~?Quieres comenzar a jugar|'
	'de todas las formas?'
#else
//	'No hay suficiente espacio libre en la|'
//	'disco de memoria. Para guardar el juego|'
//	'necesitas al menos 5 bloques libres.|'
//	'Presionar A para continuar sin guardar &\'o|'
//	'B para liberar m&\'as bloques.'

	'No hay suficientes bloques libres en|'
	'el disco de memoria. Se necesitan al|'
	'menos 20 bloques para guardar el juego.|'
	'Tienes %d bloque%s libre%s y necesitas|'
	'liberar %d bloque%s m&\'as. Pulsa el bot&\'on|'
	'A para continuar sin guardar o el|'
	'bot&\'on B para liberar m&\'as bloques.|'
#endif

// 1
カード無し:
	'No hay una MEMORY CARD (PS2)|'
	'introducida.|'
	'Para guardar el juego necesitas al menos|'
	'?KB libres. &~?Quieres comenzar a jugar|'
	'de todas las formas?'

// 2
エラー:
	'Ha habido un error al detectar la|'
	'MEMORY CARD (PS2).|'
	'Para guardar el juego necesitas al menos|'
	'?KB libres. &~?Quieres comenzar a jugar|'
	'de todas las formas?'

// 3
はい:
	'S&`i'

// 4
いいえ:
	'No'
};
