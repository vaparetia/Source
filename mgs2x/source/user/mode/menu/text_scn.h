#ifndef __TEXT_SCN__H
#define __TEXT_SCN__H

/* プリミティブ表示コントロール構造体 */
typedef struct xtextscn_PRIM_CONTROL {
   DG_DMAPACK		*dmapack ;
   int				tri_id ;			/* 使用TRI */
   DG_TEXTURE_LIST	*tex_list ;			/* TRIデータポインタ */
   int				current_offset ;	/* 書き込みデータ最終位置オフセット（1qword単位） */
   int				end_offset ;		/* 書き込みデータ限界位置オフセット（1qword単位） */
   u_long128		*buffer[2] ;		/* 転送バッファ */
   u_long128		*current_buffer ;	/* 最終書き込み位置アドレス */
} TextScn_PRIM_CONTROL ;


typedef struct xtextscn_Work {
   GV_ACT_EX	actor ;
   TextScn_PRIM_CONTROL	text_ctrl ;	/* 説明分表示用（テクスチャ転送パケットなども含む） */

#if BP_USE_NEW_FONT_SYSTEM()
   BP_FONT_VRAMINFO bp_vinfo;
   struct _SBP_BufferedTexture* pbp_BufferedTexture;
   int bufferedTextFlag;
#else
   FONT_VRAMINFO	vinfo;			/* フォント展開ワーク */
   void			*vram ;			/* フォント展開領域 */
   int				vram_size ;
   DG_TEX_LIN		*tex ;
#endif
   int				vram_width ;	/* フォント展開領域幅 */
   int				vram_height ;	/* フォント展開領域高さ */
   char			*last_message ;	/* フォント展開領域に書き込まれているメッセージへのポインタ */
   int				flag ;			/* 各種フラグ */

   void			*imagetop;			// Add K.Uehara 2002/06/25
} TextScn_Work ;

#endif //__TEXT_SCN__H