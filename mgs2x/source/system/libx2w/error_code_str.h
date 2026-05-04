/*--------------------------------------------------------------*/
/*	error_code_str.h											*/
/*					Error Code Message Strings					*/
/*--------------------------------------------------------------*/


/*-- 定型単語宣言 ----------------------------------------------*/

#define	FATAL_ERR_STR_JPN	""
#define	FATAL_ERR_STR_ENG	""
#define	FATAL_ERR_STR_GER	""
#define	FATAL_ERR_STR_ITA	""
#define	FATAL_ERR_STR_FRA	""
#define	FATAL_ERR_STR_SPA	""

#define	WARN_STR_JPN	""
#define	WARN_STR_ENG	""
#define	WARN_STR_GER	""
#define	WARN_STR_ITA	""
#define	WARN_STR_FRA	""
#define	WARN_STR_SPA	""
/*--------------------------------------------------------------*/

/*-- 文字列設定用マクロ ----------------------------------------*/

#define	MESSAGE_DEF(id_, jpn_, eng_, ger_, ita_, fra_, spa_)	\
	static	ERROR_CODE_MESS	id_ ## _mess =	\
	{										\
		jpn_, eng_, ger_, ita_, fra_, spa_	\
	} ;
/*--------------------------------------------------------------*/


MESSAGE_DEF(
	X2WERR_ID_CANT_CREATE_D3D8,

	/* JPN */	FATAL_ERR_STR_JPN	"Direct3D8 オブジェクトのインスタンス作成に失敗しました",
	/* ENG */	FATAL_ERR_STR_ENG	"Failed creating Direct3D8 Object.",
	/* GER */	FATAL_ERR_STR_GER	"Failed creating Direct3D8 Object.",
	/* ITA */	FATAL_ERR_STR_ITA	"Failed creating Direct3D8 Object.",
	/* FRA */	FATAL_ERR_STR_FRA	"Failed creating Direct3D8 Object.",
	/* SPA */	FATAL_ERR_STR_SPA	"Failed creating Direct3D8 Object.",
	)

MESSAGE_DEF(
	X2WERR_ID_CANT_CREATE_D3DEVICE8,

	/* JPN */	FATAL_ERR_STR_JPN	"Direct3DDevice8 オブジェクトのインスタンス作成に失敗しました",
	/* ENG */	FATAL_ERR_STR_ENG	"Failed creating Direct3DDevice8 Object.",
	/* GER */	FATAL_ERR_STR_GER	"Failed creating Direct3DDevice8 Object.",
	/* ITA */	FATAL_ERR_STR_ITA	"Failed creating Direct3DDevice8 Object.",
	/* FRA */	FATAL_ERR_STR_FRA	"Failed creating Direct3DDevice8 Object.",
	/* SPA */	FATAL_ERR_STR_SPA	"Failed creating Direct3DDevice8 Object.",
	)

MESSAGE_DEF(
	X2WERR_ID_DISPLAY_FORMAT,

	/* JPN */	"使用できない画面モードです。\nHigh Color(16bit)もしくはTrue Color(32bit)設定にして起動して下い",
	/* ENG */	"This screen mode cannot be used.\n"
				"Activate after setting to High Color(16bit) orTrue Color(32bit).",
	/* GER */	"This screen mode cannot be used.\n"
				"Activate after setting to High Color(16bit) orTrue Color(32bit).",
	/* ITA */	"This screen mode cannot be used.\n"
				"Activate after setting to High Color(16bit) orTrue Color(32bit).",
	/* FRA */	"This screen mode cannot be used.\n"
				"Activate after setting to High Color(16bit) orTrue Color(32bit).",
	/* SPA */	"This screen mode cannot be used.\n"
				"Activate after setting to High Color(16bit) orTrue Color(32bit).",
	)

MESSAGE_DEF(
	X2WERR_ID_OTHER_FATALERROR,

	/* JPN */	"致命的エラーが発生しました",
	/* ENG */	"Critical error",
	/* GER */	"Critical error",
	/* ITA */	"Critical error",
	/* FRA */	"Critical error",
	/* SPA */	"Critical error",
	)


MESSAGE_DEF(
	X2WERR_ID_SET_DISPLAY_16BIT,

	/* JPN */	WARN_STR_JPN	"現在の画面モードではゲームを続ける事は出来ません。\n画面モードをHigh Color(16bit)に変更して下さい",
	/* ENG */	WARN_STR_ENG	"Cannot continue the game with the current screen mode.\n"
								"Change the screen mode to High Color(16bit).",
	/* GER */	WARN_STR_GER	"Cannot continue the game with the current screen mode.\n"
								"Change the screen mode to High Color(16bit).",
	/* ITA */	WARN_STR_ITA	"Cannot continue the game with the current screen mode.\n"
								"Change the screen mode to High Color(16bit).",
	/* FRA */	WARN_STR_FRA	"Cannot continue the game with the current screen mode.\n"
								"Change the screen mode to High Color(16bit).",
	/* SPA */	WARN_STR_SPA	"Cannot continue the game with the current screen mode.\n"
								"Change the screen mode to High Color(16bit).",
	)

MESSAGE_DEF(
	X2WERR_ID_SET_DISPLAY_32BIT,

	/* JPN */	WARN_STR_JPN	"現在の画面モードではゲームを続ける事は出来ません。\n画面モードをTrue Color(32bit)に変更して下さい",
	/* ENG */	WARN_STR_ENG	"Cannot continue the game with the current screen mode.\n"
								"Change the screen mode to True Color(32bit).",
	/* GER */	WARN_STR_GER	"Cannot continue the game with the current screen mode.\n"
								"Change the screen mode to True Color(32bit).",
	/* ITA */	WARN_STR_ITA	"Cannot continue the game with the current screen mode.\n"
								"Change the screen mode to True Color(32bit).",
	/* FRA */	WARN_STR_FRA	"Cannot continue the game with the current screen mode.\n"
								"Change the screen mode to True Color(32bit).",
	/* SPA */	WARN_STR_SPA	"Cannot continue the game with the current screen mode.\n"
								"Change the screen mode to True Color(32bit).",
	)


MESSAGE_DEF(
	X2WERR_ID_ASK_RETRY_RESET_DEVICE,

	/* JPN */	WARN_STR_JPN	"Direct3Dの復旧に失敗しました。\n再試行しますか？",
	/* ENG */	WARN_STR_ENG	"Failed restoring Direct3D. Try again?",
	/* GER */	WARN_STR_GER	"Failed restoring Direct3D. Try again?",
	/* ITA */	WARN_STR_ITA	"Failed restoring Direct3D. Try again?",
	/* FRA */	WARN_STR_FRA	"Failed restoring Direct3D. Try again?",
	/* SPA */	WARN_STR_SPA	"Failed restoring Direct3D. Try again?",
	)

MESSAGE_DEF(
	X2WERR_ID_ASK_RETRY_FILEREAD,

	/* JPN */	WARN_STR_JPN	"ファイル\"%s\"へのアクセスに失敗しました\n再試行しますか？",
	/* ENG */	WARN_STR_ENG	"Failed access to file\"%s\".\nTry again?",
	/* GER */	WARN_STR_GER	"Failed access to file\"%s\".\nTry again?",
	/* ITA */	WARN_STR_ITA	"Failed access to file\"%s\".\nTry again?",
	/* FRA */	WARN_STR_FRA	"Failed access to file\"%s\".\nTry again?",
	/* SPA */	WARN_STR_SPA	"Failed access to file\"%s\".\nTry again?",
	)

MESSAGE_DEF(
	X2WERR_ID_ASK_ERROR_EXIT,

	/* JPN */	WARN_STR_JPN	"ゲームを終了します。\nセーブされていないデータは消えてしまいます。\nよろしいですか？",
	/* ENG */	WARN_STR_ENG	"Ending game.\nData not saved will disappear.\nOK?",
	/* GER */	WARN_STR_GER	"Ending game.\nData not saved will disappear.\nOK?",
	/* ITA */	WARN_STR_ITA	"Ending game.\nData not saved will disappear.\nOK?",
	/* FRA */	WARN_STR_FRA	"Ending game.\nData not saved will disappear.\nOK?",
	/* SPA */	WARN_STR_SPA	"Ending game.\nData not saved will disappear.\nOK?",
	)

MESSAGE_DEF(
	X2WERR_ID_ERROR_ABORT,

	/* JPN */	FATAL_ERR_STR_JPN	"異常終了しました",
	/* ENG */	FATAL_ERR_STR_ENG	"Abnormal shutdown",
	/* GER */	FATAL_ERR_STR_GER	"Abnormal shutdown",
	/* ITA */	FATAL_ERR_STR_ITA	"Abnormal shutdown",
	/* FRA */	FATAL_ERR_STR_FRA	"Abnormal shutdown",
	/* SPA */	FATAL_ERR_STR_SPA	"Abnormal shutdown",
	)

MESSAGE_DEF(
	X2WERR_ID_OUTOFMEMORY,

	/* JPN */	FATAL_ERR_STR_JPN	"記憶領域が不足しています",
	/* ENG */	FATAL_ERR_STR_ENG	"There is insufficient memory area.",
	/* GER */	FATAL_ERR_STR_GER	"There is insufficient memory area.",
	/* ITA */	FATAL_ERR_STR_ITA	"There is insufficient memory area.",
	/* FRA */	FATAL_ERR_STR_FRA	"There is insufficient memory area.",
	/* SPA */	FATAL_ERR_STR_SPA	"There is insufficient memory area.",
	)

MESSAGE_DEF(
	X2WERR_ID_OUTOFVIDEOMEMORY,

	/* JPN */	FATAL_ERR_STR_JPN	"ビデオメモリが不足しています",
	/* ENG */	FATAL_ERR_STR_ENG	"There is insufficient video memory.",
	/* GER */	FATAL_ERR_STR_GER	"There is insufficient video memory.",
	/* ITA */	FATAL_ERR_STR_ITA	"There is insufficient video memory.",
	/* FRA */	FATAL_ERR_STR_FRA	"There is insufficient video memory.",
	/* SPA */	FATAL_ERR_STR_SPA	"There is insufficient video memory.",
	)

MESSAGE_DEF(
	X2WERR_ID_INVALIDCALL,

	/* JPN */	FATAL_ERR_STR_JPN	"ハードウェア構成に問題があります",
	/* ENG */	FATAL_ERR_STR_ENG	"There is a problem with the hardware structure.",
	/* GER */	FATAL_ERR_STR_GER	"There is a problem with the hardware structure.",
	/* ITA */	FATAL_ERR_STR_ITA	"There is a problem with the hardware structure.",
	/* FRA */	FATAL_ERR_STR_FRA	"There is a problem with the hardware structure.",
	/* SPA */	FATAL_ERR_STR_SPA	"There is a problem with the hardware structure.",
	)

MESSAGE_DEF(
	X2WERR_ID_CANT_CREATE_OBJECT,

	/* JPN */	FATAL_ERR_STR_JPN	"%sオブジェクトのインスタンス作成に失敗しました",
	/* ENG */	FATAL_ERR_STR_ENG	"Failed creating %s Object.",
	/* GER */	FATAL_ERR_STR_GER	"Failed creating %s Object.",
	/* ITA */	FATAL_ERR_STR_ITA	"Failed creating %s Object.",
	/* FRA */	FATAL_ERR_STR_FRA	"Failed creating %s Object.",
	/* SPA */	FATAL_ERR_STR_SPA	"Failed creating %s Object.",
	)

MESSAGE_DEF(
	X2WERR_ID_CHECKDISK_FAILED,

	/* JPN */	FATAL_ERR_STR_JPN	"METAL GEAR SOLID 2:SUBSTANCE\nのディスクを挿入してください",
	/* ENG */	FATAL_ERR_STR_ENG	"Please insert \"METAL GEAR SOLID 2:SUBSTANCE\" DISK.",
	/* GER */	FATAL_ERR_STR_GER	"Please insert \"METAL GEAR SOLID 2:SUBSTANCE\" DISK.",
	/* ITA */	FATAL_ERR_STR_ITA	"Please insert \"METAL GEAR SOLID 2:SUBSTANCE\" DISK.",
	/* FRA */	FATAL_ERR_STR_FRA	"Please insert \"METAL GEAR SOLID 2:SUBSTANCE\" DISK.",
	/* SPA */	FATAL_ERR_STR_SPA	"Please insert \"METAL GEAR SOLID 2:SUBSTANCE\" DISK.",
	)


#undef	MESSAGE_DEF

