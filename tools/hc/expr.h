/* ------------------------- 式の解釈 -------------------------- */

/* 逆ポーランド記法で式を変換し直し、バイトコード化する */

/*----------------------------------------------------------------*/

#define TYPE_UNSET		-1
#define TYPE_VALUE		0
#define TYPE_VARIABLE	1
#define TYPE_ARRAY		2
#define TYPE_OPERAND	3
#define TYPE_BLOCK		4
#define TYPE_STRID		5
#define TYPE_FLOAT		6

	/*
		演算子番号
	*/
enum {
	OP_END,						/* 終端記号	*/
	OP_MNS, OP_NOT, OP_NEG,				/* 単項演算 */
	OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,		/* 四則演算	*/
	OP_LSL, OP_LSR,								/* 論理シフト */
	OP_EQ, OP_NE, OP_LT, OP_LE, OP_GT, OP_GE,	/* 比較演算	*/
	OP_OR, OP_AND, OP_XOR,					/* 論理演算	*/
	OP_OROR, OP_ANDAND,					/* 論理演算	*/
	OP_SET								/* 代入 */
} ;

typedef struct node {
	int type;
	int value;
} NODE;

typedef struct operand {
	int operand;
	int operand_pri;
	struct operand *parent;
	NODE prev;
	NODE next;
} OPERAND;

typedef struct {
	int op_buf_p;
	char *tmp_var_work_p;
	OPERAND *op_buf;
	char *tmp_var_work;
	int term_flag;
	int term_char;
} EXPR_INFO;

#define EXPR_MAX_OP_BUF			256
#define EXPR_MAX_TMP_VER_WORK	(10*1024)

void init_expr( EXPR_INFO *exinfo, char term_char );
void close_expr( EXPR_INFO *exinfo );
OPERAND *set_expr( EXPR_INFO *exinfo );
int calc_expr( EXPR_INFO *exinfo, OPERAND *top, int *value );

int is_expr_num( char *buf, int *value );
unsigned int get_expr_strcode( char *buf );
