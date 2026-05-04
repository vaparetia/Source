/*  */
/* 画像転送端末で使用する StrCode 群 */
/*  */

#define PHOTO_Console	0x1b638a	/* "console" */

/* 表示する l2d の名称 */
#define NAME_densou_title	0xad278c	/* "densou_title" */
#define NAME_densou_main	0x39c7db	/* "densou_main" */
#define NAME_puppet	0x324d1b	/* "papet" */

#define STAT_default	0x927bc4	/* "default" */
#define ACT_DefaultAction	0x8a3fb	/* "DefaultAction" */

/* papet.l2d のアクション */
#define ACT_enter	0x88dd18	/* "enter" */
#define ACT_exit	0x346d94	/* "exit" */


/* densou_title.l2d のアクション */
#define ACT_openTitle	0xe8f806	/* "openTitle" */
#define ACT_idleTitle	0xe299b4	/* "idleTitle" */
#define ACT_setTitle	0x7a1012	/* "setTitle" */

/* densou_main.l2d のオブジェクト */
#define OBJ_ROOT	0x2a4634	/* "ROOT" */
#define OBJ_Prog	0x29d647	/* "Prog" */
#define OBJ_PhotoArea	0x65b1b0	/* "PhotoArea" */
#define OBJ_TextArea	0x650f38	/* "TextArea" */
#define OBJ_dot	0x19e54	/* "dot" */
#define OBJ_progress	0xae4037	/* "progress" */

/* OK サイン表示のオブジェクト名称 */
#define OBJ_lit1	0x37b2b1	/* "lit1" */
#define OBJ_lit2	0x37b2b2	/* "lit2" */
#define OBJ_lit3	0x37b2b3	/* "lit3" */
#define OBJ_lit4	0x37b2b4	/* "lit4" */
#define OBJ_lit5	0x37b2b5	/* "lit5" */
#define OBJ_lit6	0x37b2b6	/* "lit6" */


/* 撮影枚数表示のオブジェクト名称 */
#define OBJ_shot1	0x65cab8	/* "shot1" */
#define OBJ_shot2	0x65cab9	/* "shot2" */
#define OBJ_shot3	0x65caba	/* "shot3" */
#define OBJ_shot4	0x65cabb	/* "shot4" */
#define OBJ_shot5	0x65cabc	/* "shot5" */
#define OBJ_shot6	0x65cabd	/* "shot6" */


/* densou_main.l2d のアクション */
#define ACT_openUpload	0x8b0cc3	/* "openUpload" */
#define ACT_closeUpload	0x2a7c0f	/* "closeUpload" */
#define ACT_openViewer	0x17ad53	/* "openViewer" */
#define ACT_KeyWait	0xbd2525	/* "KeyWait" */
#define ACT_closeViewer	0xb71c9e	/* "closeViewer" */

/* densou_main.l2d から送られるシグナル */
#define SIG_progress_start	0xfba164	/* "progress_start" */
#define SIG_showPuppet	0xa8a9c7	/* "showPapet" */
#define SIG_hidePuppet	0x9daee3	/* "hidePapet" */
#define SIG_showPhoto	0xac27a2	/* "showPhoto" */
#define SIG_SE	0xaa5	/* "SE" */

/* プログレスバーのキー */
#define KEY_progress0	0xc80725	/* "progress0" */
#define KEY_progress100	0x1ca150	/* "progress100" */

/* 撮影枚数表示のキー */
#define KEY_hideShot	0x7e8962	/* "hideShot" */
#define KEY_showShot	0x9ee139	/* "showShot" */
#define KEY_selShot	0xeb4be4	/* "selShot" */

/* OK サイン点灯/非点灯キー */
#define KEY_litON	0xf65a34	/* "litON" */
#define KEY_litOFF	0xcb45e4	/* "litOFF" */



/*  */
/* テキスト端末関連 */
/*  */
#define SCRN_densou_log	0x69cbf9	/* "densou_log" */
#define TEXT_photo_term	0x97389f	/* "photo_term" */
#define TEXT_photo_term_us	0x5156af	/* "photo_term_us" */
#define TEXT_photo_font	0x906026	/* "photo_font" */
#define TEXT_densou_font_alp_ovl	0xb592cb	/* "densou_font_alp_ovl" */
