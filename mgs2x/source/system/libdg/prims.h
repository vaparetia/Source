/*
	prims.h
	各種プリミティブタイプ定義ヘッダ

	1999/07/07 K.Takabe
	$Id: prims.h,v 1.1.1.3 2002/11/19 11:42:21 Yoshizawa1 Exp $

*/

/*----------------------------------------------------------------*/

	/*
		プリミティブデータ要素構造体
	*/
typedef struct {
	u_int	r, g, b, a ;
} DG_RGBA ;
typedef struct {
	u_int	x, y, z, f ;
} DG_XYZF ;
typedef struct {
	float	s, t, q, pad ;
} DG_STQ ;

typedef struct {
	u_char	r, g, b, a, pad[4] ;
} DG_PRIM_RGBA ;
typedef struct {
	u_short	x, y, pad[2] ;
} DG_PRIM_XY ;
typedef struct {
	u_short	u, v, pad[2] ;
} DG_PRIM_UV ;

/*----------------------------------------------------------------*/

	/*
		プリミティブメンバ簡易設定マクロ
	*/
#define DG_SET_RGBA1( _p, _r, _g, _b, _a )	{ (_p)->rgba1.r = _r ; (_p)->rgba1.g = _g ; (_p)->rgba1.b = _b ; (_p)->rgba1.a = _a ;}
#define DG_SET_RGBA2( _p, _r, _g, _b, _a )	{ (_p)->rgba2.r = _r ; (_p)->rgba2.g = _g ; (_p)->rgba2.b = _b ; (_p)->rgba2.a = _a ;}
#define DG_SET_RGBA3( _p, _r, _g, _b, _a )	{ (_p)->rgba3.r = _r ; (_p)->rgba3.g = _g ; (_p)->rgba3.b = _b ; (_p)->rgba3.a = _a ;}
#define DG_SET_RGBA4( _p, _r, _g, _b, _a )	{ (_p)->rgba4.r = _r ; (_p)->rgba4.g = _g ; (_p)->rgba4.b = _b ; (_p)->rgba4.a = _a ;}
#define DG_SET_STQ1( _p, _s, _t, _q ) { (_p)->stq1.s = _s ; (_p)->stq1.t = _t ; (_p)->stq1.q = _q ; }
#define DG_SET_STQ2( _p, _s, _t, _q ) { (_p)->stq2.s = _s ; (_p)->stq2.t = _t ; (_p)->stq2.q = _q ; }
#define DG_SET_STQ3( _p, _s, _t, _q ) { (_p)->stq3.s = _s ; (_p)->stq3.t = _t ; (_p)->stq3.q = _q ; }
#define DG_SET_STQ4( _p, _s, _t, _q ) { (_p)->stq4.s = _s ; (_p)->stq4.t = _t ; (_p)->stq4.q = _q ; }
#define DG_SET_XYZF1( _p, _x, _y, _z, _f ) { (_p)->xyzf1.x = _x ; (_p)->xyzf1.y = _y ; (_p)->xyzf1.z = _z ; (_p)->xyzf1.f = _f ;}
#define DG_SET_XYZF2( _p, _x, _y, _z, _f ) { (_p)->xyzf2.x = _x ; (_p)->xyzf2.y = _y ; (_p)->xyzf2.z = _z ; (_p)->xyzf2.f = _f ;}
#define DG_SET_XYZF3( _p, _x, _y, _z, _f ) { (_p)->xyzf3.x = _x ; (_p)->xyzf3.y = _y ; (_p)->xyzf3.z = _z ; (_p)->xyzf3.f = _f ;}
#define DG_SET_XYZF4( _p, _x, _y, _z, _f ) { (_p)->xyzf4.x = _x ; (_p)->xyzf4.y = _y ; (_p)->xyzf4.z = _z ; (_p)->xyzf4.f = _f ;}


#define DG_SET_POLYR4( _p, _a ) { (_p)->prim = SCE_GS_PRIM_TRISTRIP|((_a)<<6); }
#define DG_SET_POLYG4( _p, _a ) { (_p)->prim = SCE_GS_PRIM_TRISTRIP|SCE_GS_PRIM_IIP|((_a)<<6); }
#define DG_SET_POLYT4( _p, _a ) { (_p)->prim = SCE_GS_PRIM_TRISTRIP|SCE_GS_PRIM_TME|((_a)<<6); }
#define DG_SET_POLYGT4( _p, _a ) { (_p)->prim = SCE_GS_PRIM_TRISTRIP|SCE_GS_PRIM_IIP|SCE_GS_PRIM_TME|((_a)<<6); }


#define DG_SET_XY1( _p, _x, _y ) { (_p)->xy1.x = _x ; (_p)->xy1.y = _y ;}
#define DG_SET_XY2( _p, _x, _y ) { (_p)->xy2.x = _x ; (_p)->xy2.y = _y ;}
#define DG_SET_UV1( _p, _u, _v ) { (_p)->uv1.u = _u ; (_p)->uv1.v = _v ;}
#define DG_SET_UV2( _p, _u, _v ) { (_p)->uv2.u = _u ; (_p)->uv2.v = _v ;}


/*----------------------------------------------------------------*/

	/*
		プリミティブソート用ワーク
	*/
typedef ALIGN16_DECL(struct) _dg_sort_header{
	int	sort_z ;		/* パケットソートＺ座標 */
	void	*next_addr ;		/* ソート用リンクポインタ */
	void	*packet ;		/* プリミティブデータへのポインタ */
	u_int	z ;			/* ソート用 */
} DG_PRIM_HEADER ;

	/*
		プリミティブ初期化ＧＩＦパケット
		（今後メンバを変更する可能性もあるので注意すること）
	*/
typedef ALIGN16_DECL(struct) _dg_prim_init{
	sceGifTag		gif_tag ;
	sceGsAlpha		alpha ;
	u_long64			alpha_addr ;
	sceGsClamp		clamp ;
	u_long64			clamp_addr ;
	sceGsTex0		tex2 ;
	u_long64			tex2_addr ;
	sceGsTex0		tex0 ;
	u_long64			tex0_addr ;
} DG_PRIM_INIT  ;

	/*
		プリミティブパケット
	*/
typedef ALIGN16_DECL(struct) _dg_prim_packet{
	DG_PRIM_HEADER		header ;		/* ソート用ワーク */
	DG_DMATAG		dma_tag ;		/* ＤＭＡタグ */
	DG_PRIM_INIT		prim_init ;		/* プリミティブ初期化ＧＩＦパケット */
	sceGifTag		gif_tag ;		/* 描画ＧＩＦタグ */
	int			prim_top[0] ;	/* 描画ＧＩＦパケットへのポインタ */
} DG_PRIM_PACKET  ;


/*----------------------------------------------------------------*/
/*
	各種プリミティブ定義
*/

	/*
		ライン用パケット構造体
	*/
#if 0
typedef ALIGN16_DECL(struct) _dg_line_r2 {
	DG_RGBA		rgba1 ;
	DG_XYZF		xyzf1 ;
	DG_XYZF		xyzf2 ;
} DG_LINE_R2  ;

typedef ALIGN16_DECL(struct) _dg_line_g2 {
	DG_RGBA		rgba1 ;
	DG_XYZF		xyzf1 ;
	DG_RGBA		rgba2 ;
	DG_XYZF		xyzf2 ;
} DG_LINE_G2  ;

typedef ALIGN16_DECL(struct) _dg_line_t2 {
	DG_STQ		stq1 ;
	DG_RGBA		rgba_dummy ;
	DG_XYZF		xyzf1 ;
	DG_STQ		stq2 ;
	DG_RGBA		rgba1 ;
	DG_XYZF		xyzf2 ;
} DG_LINE_T2  ;

typedef ALIGN16_DECL(struct) _dg_line_gt2 {
	DG_STQ		stq1 ;
	DG_RGBA		rgba1 ;
	DG_XYZF		xyzf1 ;
	DG_STQ		stq2 ;
	DG_RGBA		rgba2 ;
	DG_XYZF		xyzf2 ;
} DG_LINE_GT2  ;

	/*
		３角形ポリゴン用パケット構造体
	*/
typedef ALIGN16_DECL(struct) _dg_poly_r3 {
	DG_RGBA		rgba1 ;
	DG_XYZF		xyzf1 ;
	DG_XYZF		xyzf2 ;
	DG_XYZF		xyzf3 ;
} DG_POLY_R3  ;

typedef ALIGN16_DECL(struct) _dg_poly_g3 {
	DG_RGBA		rgba1 ;
	DG_XYZF		xyzf1 ;
	DG_RGBA		rgba2 ;
	DG_XYZF		xyzf2 ;
	DG_RGBA		rgba3 ;
	DG_XYZF		xyzf3 ;
} DG_POLY_G3  ;

typedef ALIGN16_DECL(struct) _dg_poly_t3 {
	DG_STQ		stq1 ;
	DG_RGBA		rgba_dummy1 ;
	DG_XYZF		xyzf1 ;
	DG_STQ		stq2 ;
	DG_RGBA		rgba_dummy2 ;
	DG_XYZF		xyzf2 ;
	DG_STQ		stq3 ;
	DG_RGBA		rgba1 ;
	DG_XYZF		xyzf3 ;
} DG_POLY_T3  ;

typedef ALIGN16_DECL(struct) _dg_poly_gt3 {
	DG_STQ		stq1 ;
	DG_RGBA		rgba1 ;
	DG_XYZF		xyzf1 ;
	DG_STQ		stq2 ;
	DG_RGBA		rgba2 ;
	DG_XYZF		xyzf2 ;
	DG_STQ		stq3 ;
	DG_RGBA		rgba3 ;
	DG_XYZF		xyzf3 ;
} DG_POLY_GT3  ;

	/*
		４角形ポリゴン用パケット構造体
	*/
typedef ALIGN16_DECL(struct) _dg_poly_r4 {
	u_long64		prim,pad ;
	DG_RGBA		rgba1 ;
	DG_XYZF		xyzf1 ;
	DG_XYZF		xyzf2 ;
	DG_XYZF		xyzf3 ;
	DG_XYZF		xyzf4 ;
} DG_POLY_R4  ;

typedef ALIGN16_DECL(struct) _dg_poly_g4 {
	u_long64		prim,pad ;
	DG_RGBA		rgba1 ;
	DG_XYZF		xyzf1 ;
	DG_RGBA		rgba2 ;
	DG_XYZF		xyzf2 ;
	DG_RGBA		rgba3 ;
	DG_XYZF		xyzf3 ;
	DG_RGBA		rgba4 ;
	DG_XYZF		xyzf4 ;
} DG_POLY_G4  ;

typedef ALIGN16_DECL(struct) _dg_poly_t4 {
	u_long64		prim,pad ;
	DG_STQ		stq1 ;
	DG_RGBA		rgba_dummy1 ;
	DG_XYZF		xyzf1 ;
	DG_STQ		stq2 ;
	DG_RGBA		rgba_dummy2 ;
	DG_XYZF		xyzf2 ;
	DG_STQ		stq3 ;
	DG_RGBA		rgba1 ;
	DG_XYZF		xyzf3 ;
	DG_STQ		stq4 ;
	DG_RGBA		rgba2 ;
	DG_XYZF		xyzf4 ;
} DG_POLY_T4  ;

typedef ALIGN16_DECL(struct) _dg_poly_gt4 {
	u_long64		prim,pad ;
	DG_STQ		stq1 ;
	DG_RGBA		rgba1 ;
	DG_XYZF		xyzf1 ;
	DG_STQ		stq2 ;
	DG_RGBA		rgba2 ;
	DG_XYZF		xyzf2 ;
	DG_STQ		stq3 ;
	DG_RGBA		rgba3 ;
	DG_XYZF		xyzf3 ;
	DG_STQ		stq4 ;
	DG_RGBA		rgba4 ;
	DG_XYZF		xyzf4 ;
} DG_POLY_GT4  ;

	/*
		スプライト用パケット構造体
	*/
typedef ALIGN16_DECL(struct) _dg_sprt {
	DG_STQ		stq1 ;
	DG_RGBA		rgba_dummy ;
	DG_XYZF		xyzf1 ;
	DG_STQ		stq2 ;
	DG_RGBA		rgba1 ;
	DG_XYZF		xyzf2 ;
} DG_SPRT  ;
#endif


	/*
		メニュー用パケット構造体
	*/
typedef ALIGN16_DECL(struct) _dg_menu_sprt {
	u_long64			prim ;
	DG_PRIM_RGBA	rgba1 ;
	DG_PRIM_UV		uv1 ;
	DG_PRIM_XY		xy1 ;
	DG_PRIM_UV		uv2 ;
	DG_PRIM_XY		xy2 ;
} DG_MENU_SPRT  ;

typedef ALIGN16_DECL(struct) _dg_menu_line {
	u_long64			prim ;
	DG_PRIM_RGBA	rgba1 ;
	DG_PRIM_XY		xy1 ;
	DG_PRIM_XY		xy2 ;
} DG_MENU_LINE  ;

typedef ALIGN16_DECL(struct) _dg_menu_line_g {
	u_long64			prim ;
	DG_PRIM_RGBA	rgba1 ;
	DG_PRIM_XY		xy1 ;
	DG_PRIM_RGBA	rgba2 ;
	DG_PRIM_XY		xy2 ;
	u_long64			pad ;
} DG_MENU_LINE_G  ;


/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/

enum {
	GS_REGS_PRIM,
	GS_REGS_RGBA,
	GS_REGS_STQ,
	GS_REGS_UV,
	GS_REGS_XYZF2,
	GS_REGS_XYZ2,
	GS_REGS_TEX0_1,
	GS_REGS_TEX0_2,
	GS_REGS_CLAMP_1,
	GS_REGS_CLAMP_2,
	GS_REGS_FOG,
	GS_REGS_0B_UNDEFINED,
	GS_REGS_XYZF3,
	GS_REGS_XYZ3,
	GS_REGS_AD,
	GS_REGS_NOP,
};

/*----------------------------------------------------------------*/
	/*
		プリミティブ用ＧＩＦタグ初期化定数
	*/
#ifdef __PRIM_C__
sceGifTag	DG_GIFTAG_LINE_R2 = {
	0,1,0/*pad*/,0,1,SCE_GS_SET_PRIM(SCE_GS_PRIM_LINE, 0, 0, 0, 0, 0, 0, 0, 0),SCE_GIF_PACKED,3,
	GS_REGS_RGBA,GS_REGS_XYZF2,GS_REGS_XYZF2,
	0,0,0,0,0,0,0,0,0,0,0,0,0
};
sceGifTag	DG_GIFTAG_LINE_G2 = {
	0,1,0/*pad*/,0,1,SCE_GS_SET_PRIM(SCE_GS_PRIM_LINE, 1, 0, 0, 0, 0, 0, 0, 0),SCE_GIF_PACKED,4,
	GS_REGS_RGBA,GS_REGS_XYZF2,GS_REGS_RGBA,GS_REGS_XYZF2,
	0,0,0,0,0,0,0,0,0,0,0,0
};
sceGifTag	DG_GIFTAG_LINE_T2 = {
	0,1,0/*pad*/,0,1,SCE_GS_SET_PRIM(SCE_GS_PRIM_LINE, 0, 1, 0, 0, 0, 0, 0, 0),SCE_GIF_PACKED,6,
	GS_REGS_STQ,GS_REGS_RGBA,GS_REGS_XYZF2,GS_REGS_STQ,GS_REGS_RGBA,GS_REGS_XYZF2,
	0,0,0,0,0,0,0,0,0,0
};
sceGifTag	DG_GIFTAG_LINE_GT2 = {
	0,1,0/*pad*/,0,1,SCE_GS_SET_PRIM(SCE_GS_PRIM_LINE, 1, 1, 0, 0, 0, 0, 0, 0),SCE_GIF_PACKED,6,
	GS_REGS_STQ,GS_REGS_RGBA,GS_REGS_XYZF2,GS_REGS_STQ,GS_REGS_RGBA,GS_REGS_XYZF2,
	0,0,0,0,0,0,0,0,0,0
};
sceGifTag	DG_GIFTAG_POLY_R3 = {
	0,1,0/*pad*/,0,1,SCE_GS_SET_PRIM(SCE_GS_PRIM_TRI, 0, 0, 0, 0, 0, 0, 0, 0),SCE_GIF_PACKED,4,
	GS_REGS_RGBA,GS_REGS_XYZF2,GS_REGS_XYZF2,GS_REGS_XYZF2,
	0,0,0,0,0,0,0,0,0,0,0,0
};
sceGifTag	DG_GIFTAG_POLY_G3 = {
	0,1,0/*pad*/,0,1,SCE_GS_SET_PRIM(SCE_GS_PRIM_TRI, 1, 0, 0, 0, 0, 0, 0, 0),SCE_GIF_PACKED,6,
	GS_REGS_RGBA,GS_REGS_XYZF2,GS_REGS_RGBA,GS_REGS_XYZF2,GS_REGS_RGBA,GS_REGS_XYZF2,
	0,0,0,0,0,0,0,0,0,0
};
sceGifTag	DG_GIFTAG_POLY_T3 = {
	0,1,0/*pad*/,0,1,SCE_GS_SET_PRIM(SCE_GS_PRIM_TRI, 0, 1, 0, 0, 0, 0, 0, 0),SCE_GIF_PACKED,9,
	GS_REGS_STQ,GS_REGS_RGBA,GS_REGS_XYZF2,GS_REGS_STQ,GS_REGS_RGBA,GS_REGS_XYZF2,
	GS_REGS_STQ,GS_REGS_RGBA,GS_REGS_XYZF2,
	0,0,0,0,0,0,0
};
sceGifTag	DG_GIFTAG_POLY_GT3 = {
	0,1,0/*pad*/,0,1,SCE_GS_SET_PRIM(SCE_GS_PRIM_TRI, 1, 1, 0, 0, 0, 0, 0, 0),SCE_GIF_PACKED,9,
	GS_REGS_STQ,GS_REGS_RGBA,GS_REGS_XYZF2,GS_REGS_STQ,GS_REGS_RGBA,GS_REGS_XYZF2,
	GS_REGS_STQ,GS_REGS_RGBA,GS_REGS_XYZF2,
	0,0,0,0,0,0,0
};
sceGifTag	DG_GIFTAG_POLY_R4 = {
	0,1,0/*pad*/,0,1,SCE_GS_SET_PRIM(SCE_GS_PRIM_TRISTRIP, 0, 0, 0, 0, 0, 0, 0, 0),SCE_GIF_PACKED,6,
	GS_REGS_NOP,
	GS_REGS_RGBA,GS_REGS_XYZF2,GS_REGS_XYZF2,GS_REGS_XYZF2,GS_REGS_XYZF2,
	0,0,0,0,0,0,0,0,0,0
};
sceGifTag	DG_GIFTAG_POLY_G4 = {
	0,1,0/*pad*/,0,1,SCE_GS_SET_PRIM(SCE_GS_PRIM_TRISTRIP, 1, 0, 0, 0, 0, 0, 0, 0),SCE_GIF_PACKED,9,
	GS_REGS_NOP,
	GS_REGS_RGBA,GS_REGS_XYZF2,GS_REGS_RGBA,GS_REGS_XYZF2,GS_REGS_RGBA,GS_REGS_XYZF2,GS_REGS_RGBA,GS_REGS_XYZF2,
	0,0,0,0,0,0,0
};
sceGifTag	DG_GIFTAG_POLY_T4 = {
	0,1,0/*pad*/,0,1,SCE_GS_SET_PRIM(SCE_GS_PRIM_TRISTRIP, 0, 1, 0, 0, 0, 0, 0, 0),SCE_GIF_PACKED,13,
	GS_REGS_NOP,
	GS_REGS_STQ,GS_REGS_RGBA,GS_REGS_XYZF2,GS_REGS_STQ,GS_REGS_RGBA,GS_REGS_XYZF2,
	GS_REGS_STQ,GS_REGS_RGBA,GS_REGS_XYZF2,GS_REGS_STQ,GS_REGS_RGBA,GS_REGS_XYZF2,
	0,0,0
};
sceGifTag	DG_GIFTAG_POLY_GT4 = {
	0,1,0/*pad*/,0,1,SCE_GS_SET_PRIM(SCE_GS_PRIM_TRISTRIP, 1, 1, 0, 0, 0, 0, 0, 0),SCE_GIF_PACKED,13,
	GS_REGS_NOP,
	GS_REGS_STQ,GS_REGS_RGBA,GS_REGS_XYZF2,GS_REGS_STQ,GS_REGS_RGBA,GS_REGS_XYZF2,
	GS_REGS_STQ,GS_REGS_RGBA,GS_REGS_XYZF2,GS_REGS_STQ,GS_REGS_RGBA,GS_REGS_XYZF2,
	0,0,0
};
sceGifTag	DG_GIFTAG_SPRITE = {
	0,1,0/*pad*/,0,1,SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, 0, 1, 0, 0, 0, 0, 0, 0),SCE_GIF_PACKED,6,
	GS_REGS_STQ,GS_REGS_RGBA,GS_REGS_XYZF2,GS_REGS_STQ,GS_REGS_RGBA,GS_REGS_XYZF2,
	0,0,0,0,0,0,0,0,0,0
};

sceGifTag	DG_GIFTAG_MENU_SPRITE = {
	0,1,0/*pad*/,0,1,SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, 0, 1, 0, 0, 0, 1, 0, 0),SCE_GIF_REGLIST,6,
	GS_REGS_PRIM,GS_REGS_RGBA,GS_REGS_UV,GS_REGS_XYZF2,GS_REGS_UV,GS_REGS_XYZF2,
	0,0,0,0,0,0,0,0,0,0
};

sceGifTag	DG_GIFTAG_MENU_LINE = {
	0,1,0/*pad*/,0,1,SCE_GS_SET_PRIM(SCE_GS_PRIM_LINE, 0, 0, 0, 0, 0, 0, 0, 0),SCE_GIF_REGLIST,4,
	GS_REGS_PRIM,GS_REGS_RGBA,GS_REGS_XYZF2,GS_REGS_XYZF2,
	0,0,0,0,0,0,0,0,0,0,0,0
};

sceGifTag	DG_GIFTAG_MENU_LINE_G = {
	0,1,0/*pad*/,0,1,SCE_GS_SET_PRIM(SCE_GS_PRIM_LINE, 1, 0, 0, 0, 0, 0, 0, 0),SCE_GIF_REGLIST,6,
	GS_REGS_PRIM,GS_REGS_RGBA,GS_REGS_XYZF2,GS_REGS_RGBA,GS_REGS_XYZF2,GS_REGS_NOP,
	0,0,0,0,0,0,0,0,0,0
};
#else
extern sceGifTag	DG_GIFTAG_LINE_R2 ;
extern sceGifTag	DG_GIFTAG_LINE_G2 ;
extern sceGifTag	DG_GIFTAG_LINE_T2 ;
extern sceGifTag	DG_GIFTAG_LINE_GT2 ;
extern sceGifTag	DG_GIFTAG_POLY_R3 ;
extern sceGifTag	DG_GIFTAG_POLY_G3 ;
extern sceGifTag	DG_GIFTAG_POLY_T3 ;
extern sceGifTag	DG_GIFTAG_POLY_GT3 ;
extern sceGifTag	DG_GIFTAG_POLY_R4 ;
extern sceGifTag	DG_GIFTAG_POLY_G4 ;
extern sceGifTag	DG_GIFTAG_POLY_T4 ;
extern sceGifTag	DG_GIFTAG_POLY_GT4 ;
extern sceGifTag	DG_GIFTAG_SPRITE ;
extern sceGifTag	DG_GIFTAG_MENU_SPRITE ;
extern sceGifTag	DG_GIFTAG_MENU_LINE ;
extern sceGifTag	DG_GIFTAG_MENU_LINE_G ;
#endif
