#ifndef _command_h_
#define _command_h_

/*
 * デリミタ, 空白, クォート文字の設定
 */
#define CMD_DELIM   "{(,!)}\n"
#define CMD_BLANK   " \t"
#define CMD_QUOTE   "'\""

/* トークン取得一時バッファのサイズ */
#define CMD_TKNBUF   256

/* 解釈ステータス */
#define CMD_ERR_SYNTAX     -1
#define CMD_ERR_NO_MEMORY  -2
#define CMD_ERR_DUPLICATED -3

/* 分岐テーブル用構造体 */
typedef struct cmdIndex {
  char * cmd;
  int (*func)(char * cmd, tknFILE * tkn, lblBlock * block, FILE * wfp);
} cmdIndex;


#ifdef _command_c_
#define EXT
#else
#define EXT extern
#endif /* _command_c_ */

/*
 * 命令解釈モジュールで使用する大域変数
 */
EXT lblBlock * cmdLabel;  /* ラベル管理ブロック */
EXT int        cmdStatus; /* コマンド解釈結果のステータス一時保持用 */

#undef EXT

void * cmdMalloc(size_t size);
char * cmdStrDup(char * str);
char * cmdGetCommand(tknFILE * tkn);
char * cmdGetToken(tknFILE * tkn, tknStat stat);
int    cmdConvert(tknFILE * tkn,
		  cmdIndex * cmd_list, int (*func)(char * token));
int    cmdInit(char * src, char * dst);
int    cmdConvMain(char * src, char * dst);

#endif /* _command_h_ */
