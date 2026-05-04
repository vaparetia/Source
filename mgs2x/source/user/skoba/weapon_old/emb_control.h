/*
	enb_control.c
	統合薬莢管理（コモデル）
	
	2001/07/23 T.Shibata
	
	$Id: emb_control.h,v 1.1.1.3 2002/11/19 11:50:34 Yoshizawa1 Exp $

*/

#ifndef EMB_CONTROL_H
#define EMB_CONTROL_H

enum {
	AMO_KIND_USP = 0,		//usp
	AMO_KIND_GLK,			//usp
	AMO_KIND_SOCOM,			//usp

	AMO_KIND_FMS,			//fms
	AMO_KIND_P90,			//fms
	AMO_KIND_M4A1,			//fms
	AMO_KIND_ABK,			//fms
	AMO_KIND_AKS,			//fms
	AMO_KIND_PSG,			//fms
	AMO_KIND_SPS,			//sps
	AMO_KIND_MECA,			//fms
	AMO_KIND_M4,			//fms
	
	AMO_KIND_M4GRN,			//demo_m4_grn_emb

	AMO_KIND_MAX,
};

typedef struct _emb_data
{
	FVECTOR		pos;			// 位置 
	FVECTOR		speed;			// スピード 
	SVECTOR		rot ;			// 回転 
	SVECTOR		const_rot;		// 回転スピード  
	short		time;			// 時間 
	short		point_time;		// 何かしらのタイミング 
	u_char		mode;			// モードをチェック(外の人も見ます) 
	u_char		emb_id;			// 弾の種類 
	u_char		mdl_kind;		// モデルの種類 
	u_char		pad0;
	int			map;			// 今いるマップ 

	struct _emb_data	*next;		// 次の弾 
	struct _emb_data	*next_emb;	// 次の弾(種類別) 
} EMB_DATA;

extern EMB_DATA *SetNewEmbData( int emb_id, FVECTOR *pos, FVECTOR *speed, SVECTOR *init_rot, SVECTOR *rot, short point_time );

#endif
