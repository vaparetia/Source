/*
	ツール内部でのデータ形式
*/

typedef struct _common_tag {
	struct _common_tag *next;
} COMMON_TAG;

typedef struct {
	COMMON_TAG tag;
	int start;
	int end;
	int name;
	char *string;
} TALK_AREA;

typedef struct {
	COMMON_TAG tag;
	int start;
	int end;
	int name;
	char *string;
} TALK_DIALOG;

typedef struct {
	COMMON_TAG tag;
	int start;
	int value;
	int name;
} TALK_ACTION;

/*
	inf ファイルデータ形式
*/

/* IDタグ定義 */

#define VERSION 0x00010000

#define IDTAG_SIZE	4

#define INF_FILE_HEADER_ID	"SDHD"
#define INF_FILE_FILEINF_ID	"SDIF"
#define INF_FILE_HISTS_ID	"SDHS"
#define INF_FILE_FILEANM_ID	"SDFA"

#define INF_TALK_INF_ID		"SDTI"
#define INF_TALK_AREA_ID	"SDTA"
#define INF_TALK_ACT_ID		"SDTC"
#define INF_TALK_DIALOG_ID	"SDDL"
#define	INF_TALK_BLOCK_ID	"SDBK"

/* データ構造定義 */

typedef struct {
	int id;
	int block_size;
} INF_BLOCK;

// FILE

typedef struct {
	int id;
	int block_size;
	int version;
	int reserved[ 5 ];
} INF_FILE_HEADER;

typedef struct {
	int id;
	int block_size;
	int reserved[ 2 ];
	char name[ 0 ];
} INF_FILE_FILEINF;

typedef struct {
	int id;
	int block_size;
	int hist_max;
	int change_count;
	int rsvd[ 4 ];
	int changes[ 0 ];
} INF_FILE_HISTS;

typedef struct {
	int id;
	int block_size;
	int rsvd[ 2 ];
	char name[ 0 ];
} INF_FILE_FILEANM;

// TALK

typedef struct {
	int id;
	int block_size;
	int userref;
	int rsvd[ 1 ];
	char name[ 0 ];
} INF_TALK_INF;

typedef struct {
	int id;
	int block_size;
	int pos;
	int duration;
	int rsvd[ 4 ];
} INF_TALK_AREA;

typedef struct {
	int id;
	int block_size;
	int pos;
	int duration;
	int attr;
	int param1;
	int param2;
	int rsvd;
	char name[ 0 ];
} INF_TALK_ACT;

typedef struct {
	int id;
	int block_size;
	int pos;
	int duration;
	char name[ 0 ];
	/* この後に char str[ 0 ] */
} INF_TALK_DIALOG;

typedef struct {
	int id;
	int block_size;
	int pos;
	int duration;
	int block_id;
	int tm;
	int rsvd[ 2 ];
} INF_TALK_BLOCK;

/*
	操作用関数
*/

TALK_DIALOG *get_dialog_top( void );
TALK_DIALOG *get_next_dialog( TALK_DIALOG *now );

TALK_ACTION *get_action_top( void );
TALK_ACTION *get_next_action( TALK_ACTION *now );

TALK_AREA *get_talk_area( int start, int end );

int load_inf_file( FILE *fp );


