/*
	main.h
	プログラム用ヘッダ

 */

/* ---------------------------------------------------------------- */
/* 処理できる最大データ数 */
#define MAX_DATAS	(32)

/* コマンドライン入力データワーク */
typedef struct _option{
	int		flag ;
	int		n_datas ;
	int		base_tick ;
	int		frame_skip ;
	struct _input_data{
		int			strcode ;
		char		filename[256] ;
		MTN_MOTION	*motion ;
	} input_data[MAX_DATAS];
	char		out_filename[256];
}PROGRAM_OPTION ;

enum {
	PROGRAM_FLAG_NEWVERTION				= 0x0001,
	PROGRAM_FLAG_DUMP					= 0x0002,
};

/* ---------------------------------------------------------------- */
extern PROGRAM_OPTION	Option ;

/* ---------------------------------------------------------------- */
