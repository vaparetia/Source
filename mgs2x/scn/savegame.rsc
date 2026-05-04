/*
	ファイル名		savegame.rsc
	作成日			2001/10/22
	作者			Y.Yano
	説明			セーブの際に出るメッセージ(多言語対応版)
	$Id: savegame.rsc,v 1.6 2002/10/02 05:54:47 usr03700 Exp $
*/


resource セーブゲームリソース日本語 {
// 0:
メモリーカードが見つからなかった:
	'メモリーカード(PS2)が見つかりません'

// 1:
空き容量が足りない:
#ifdef PSX2
	'メモリーカード(PS2)の空き容量が足りません'
#else
	'ハードディスクの空き容量が足りません'
#endif

// 2:
新規確認:
	'新規にデータをセーブします|' \
	'よろしいですか？'

// 3:
上書き確認:
	'データを上書きセーブします|' \
	'よろしいですか？'

// 4:
フォーマット確認:
	'メモリーカード(PS2)はフォーマットされていません|' \
	'メモリーカード(PS2)をフォーマットして|' \
	'セーブします よろしいですか？'

// 5:
ファイルセーブ:
	'ファイルのセーブ中'

// 6:
フォーマット:
	'フォーマット中'

// 7:
成功:
	'ファイルのセーブが終了しました'

// 8:
失敗:
	'ファイルのセーブ中にエラーが発生しました'

// 9:
新規確認2:
	'新規にデータをセーブします。よろしいですか？'

// 10:
上書き確認2:
	'データを上書きセーブします。よろしいですか？'

// 11:
フォーマット失敗:
	'フォーマット中にエラーが発生しました'

// 12
はい:
	'Yes'

// 13
いいえ:
	'No'

// 14
パッド抜け:
	'コントローラを接続し直し|'
	'START ボタンを押して続けてください'
};

resource セーブゲームリソース英語 {
// 0:
メモリーカードが見つからなかった:
	'MEMORY CARD (PS2) not inserted.'

// 1:
空き容量が足りない:
#ifdef PSX2
	'Insufficient space on the MEMORY CARD (PS2).'
#else
	'Insufficient free blocks on the hard disk.'
#endif

// 2:
新規確認:
	// 'Create new save file. OK?'
	'Create new save data. OK?'

// 3:
上書き確認:
	'Overwrite?'

// 4:
フォーマット確認:
#ifdef PSX2
	'MEMORY CARD (PS2) not formatted.|'
	'Format and save?'
#else
	'There are too many saved games on the hard disk.'
#endif

// 5:
ファイルセーブ:
//	'Saving. Do not remove the Memory Card (PS2).'
	'Saving.'

// 6:
フォーマット:
//	'Formatting. Do not remove the Memory Card (PS2).'
	'Formatting.'

// 7:
成功:
	'Save completed.'

// 8:
失敗:
	'Save failed.'

// 9:
新規確認2:
	// 'Create new save file. OK?'
	'Create new save data. OK?'

// 10:
上書き確認2:
	'Overwrite?'

// 11:
フォーマット失敗:
	'Format failed.'

// 12
はい:
	'Yes'

// 13
いいえ:
	'No'
#ifdef _XBOX
// 14
パッド抜け:
	'Please reconnect the controller|'
	' and press START to continue'
#endif
};


resource セーブゲームリソースフランス語 {
// 0:
メモリーカードが見つからなかった:
	{MEMORY CARD (PS2) non ins&'er&'ee.}

// 1:
空き容量が足りない:
#ifdef PSX2
	{Pas assez d'espace sur la MEMORY CARD (PS2).}
#else
	{Pas assez d'espace sur la disque dur.}
#endif

// 2:
新規確認:
	// {Cr&'eer un nouveau fichier de sauvegarde. OK?}
	'Cr&\'eer nouvelles donn&\'ees de sauvegarde. OK?'

// 3:
上書き確認:
	'Ecraser?'

// 4:
フォーマット確認:
	'MEMORY CARD (PS2) non format&\'ee.|'
	'Formater et sauvegarder?'

// 5:
ファイルセーブ:
//	'Sauvegarde en cours. Ne retirez pas la MEMORY CARD (PS2).'
	'Sauvegarde en cours.'

// 6:
フォーマット:
//	'Formatage en cours. Ne retirez pas la MEMORY CARD (PS2).'
	'Formatage en cours.'

// 7:
成功:
	{Sauvegarde termin&'ee.}

// 8:
失敗:
	'Echec de la sauvegarde.'

// 9:
新規確認2:
	// {Cr&'eer un nouveau fichier de sauvegarde. OK?}
	'Cr&\'eer nouvelles donn&\'ees de sauvegarde. OK?'

// 10:
上書き確認2:
	'Ecraser?'

// 11:
フォーマット失敗:
	'Echec du formatage.'

// 12
はい:
	'Oui'

// 13
いいえ:
	'Non'

#ifdef _XBOX
// 14
パッド抜け:
	'Pour continuer, reconnecter la|'
	'manette et appuyer sur START.'
#endif
};


resource セーブゲームリソースドイツ語 {
// 0:
メモリーカードが見つからなかった:
	'MEMORY CARD (PS2) nicht eingelegt.'

// 1:
空き容量が足りない:
#ifdef PSX2
	'Nicht genug Speicherplatz auf der MEMORY CARD (PS2).'
#else
	'Nicht genug Speicherplatz auf der Festplatte.'
#endif

// 2:
新規確認:
	// 'Neue Speicherdatei erstellen?'
	'Neue Speicherdatei f&~ur Spieldaten erstellen?'

// 3:
上書き確認:
	'&~Uberschreiben?'

// 4:
フォーマット確認:
	'MEMORY CARD (PS2) nicht formatiert.|'
	'Formatieren und speichern?'

// 5:
ファイルセーブ:
//	'Speichert. MEMORY CARD (PS2) nicht entfernen.'
	'Speichert.'

// 6:
フォーマット:
//	'Formatiert. MEMORY CARD (PS2) nicht entfernen.'
	'Formatiert.'

// 7:
成功:
	'Speichern beendet.'

// 8:
失敗:
	'Speichern mi&~slungen.'

// 9:
新規確認2:
	// 'Neue Speicherdatei erstellen?'
	'Neue Speicherdatei f&~ur Spieldaten erstellen?'

// 10:
上書き確認2:
	'&~Uberschreiben?'

// 11:
フォーマット失敗:
	'Formatieren mi&~slungen.'

// 12
はい:
	'Ja'

// 13
いいえ:
	'Nein'

#ifdef _XBOX
// 14
パッド抜け:
	'Bitte den Controller wieder|'
	'anschlie&~sen und START dr&~ucken.'
#endif
};


resource セーブゲームリソースイタリア語 {
// 0:
メモリーカードが見つからなかった:
	'MEMORY CARD (PS2) non inserita.'

// 1:
空き容量が足りない:
#ifdef PSX2
	'Spazio memoria insufficiente sulla MEMORY CARD (PS2).'
#else
	'Spazio memoria insufficiente sulla disco rigido.'
#endif
// 2:
新規確認:
	// 'Crea nuovo file da salvare. OK?'
	'Crea nuovi dati da salvare. OK?'

// 3:
上書き確認:
	'Vuoi sovrascrivere?'

// 4:
フォーマット確認:
	'MEMORY CARD (PS2) non formattata.|'
	'Vuoi formattare e salvare?'

// 5:
ファイルセーブ:
//	'Salvataggio in corso. Non estrarre la MEMORY CARD (PS2).'
	'Salvataggio in corso.'

// 6:
フォーマット:
//	'Formattazione in corso. Non estrarre la MEMORY CARD (PS2).'
	'Formattazione in corso.'

// 7:
成功:
	'Salvataggio completato.'

// 8:
失敗:
	'Salvataggio non riuscito.'

// 9:
新規確認2:
	// 'Crea nuovo file da salvare. OK?'
	'Crea nuovi dati da salvare. OK?'

// 10:
上書き確認2:
	'Vuoi sovrascrivere?'

// 11:
フォーマット失敗:
	'Formattazione non riuscita.'

// 12
はい:
	{S&`i}

// 13
いいえ:
	'No'

#ifdef _XBOX
// 14
パッド抜け:
	'Ricollegare il controller|'
	'e premere START per continuare.'
#endif
};

resource セーブゲームリソーススペイン語 {
// 0:
メモリーカードが見つからなかった:
	'No hay una MEMORY CARD (PS2) introducida.'

// 1:
空き容量が足りない:
#ifdef PSX2
	'No hay suficiente espacio libre en la MEMORY CARD (PS2).'
#else
	'No hay suficiente espacio libre en la disco de memoria.'
#endif

// 2:
新規確認:
	// 'Crear nuevo archivo para guardar. &~?OK?'
	'Crear nueva informaci&\'on para guardar. &~?OK?'

// 3:
上書き確認:
	'&~?Sobrescribir?'

// 4:
フォーマット確認:
	'La MEMORY CARD (PS2) no est&\'a formateada.|'
	'&~?Formatear y guardar?'

// 5:
ファイルセーブ:
//	'Guardando. No extraigas la MEMORY CARD (PS2).'
	'Guardando.'

// 6:
フォーマット:
//	'Formateando. No extraigas la MEMORY CARD (PS2).'
	'Formateando.'

// 7:
成功:
	'Archivo guardado.'

// 8:
失敗:
	'Error al guardar.'

// 9:
新規確認2:
	// 'Crear nuevo archivo para guardar. &~?OK?'
	'Crear nueva informaci&\'on para guardar. &~?OK?'

// 10:
上書き確認2:
	'&~?Sobrescribir?'

// 11:
フォーマット失敗:
	'No se ha formateado.'

// 12
はい:
	{S&`i}

// 13
いいえ:
	'No'

#ifdef _XBOX
// 14
パッド抜け:
	'Volver a conectar el controlador|'
	'y presionar START para continuar.'
#endif
};
