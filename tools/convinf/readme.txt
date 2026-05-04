confinf

XXで作成したinfファイルの情報を読み取り、
字幕情報を作成する。

infload.cでinfファイルの情報を読みとる。

MGS1では口パク情報を同時に作成し、その中に話者情報も
入っていたが、今回は口パク情報は別に作成されるので、
字幕情報にも話者情報を入れるようにする。

出力される字幕情報は、最大KBYTEを指定し、そのサイズに収まるよう
グルーピングされる。

-- packet header
int type;
int size;
int time;
int option;
-- caption header
int caption_size
-- table_offset;
{
	int start_count;	// 1/300
	int end_count;		// 1/300
	int name;
	int len;			// string len
	char string[ len ];
} [];
-- font_offset;
char font[];
--







