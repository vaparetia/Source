//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ipupanel.c
	ＩＰＵストリーム貼り付けパネル

	2001/07/26 K.Takabe
	$Id: ipupanel.c,v 1.7 2002/12/17 08:47:14 takaki Exp $

*/
/*

   １６ビットのムービーイメージをテクスチャとして転送するために通常の
   部分テクスチャ転送パケットを準備し、自前で無理やり１６ビットイメージ
   転送用に書き換えることでパッチ曲面オブジェクトに１６ビットイメージ
   テクスチャを貼り付ける


chara ＩＰＵ表示パネル[NewIPUDisplayPanelSet] $s:name \
	-tex $s:ＴＲＩ＿ＩＤ $s:テクスチャ \
	-uv $w:Ｕ０ $w:Ｖ０ $w:Ｕ１ $w:Ｖ１ \
	-num $w:表示枚数 \
	-pos $v:左上座標 $v:右上座標 $v:左下座標 $v:右下座標 ...12
// テクスチャはこのキャラ専用にダミーのＴＲＩを用意する必要がある
// ＵＶ値の指定は左上と右下を４０９６を１．０とみなして設定する
// -uv 0,512,4096,3584


*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"def_dma.h"
#include	"libmt.h"
#include	"gameheader.h"
#include	"debugmenu.h"

#include "../other/vec_util.h"

/* ---------------------------------------------------------------- */
	/*
		外部参照
	*/
/* ---------------------------------------------------------------- */
	/*
		補助マクロ
	*/


/* ---------------------------------------------------------------- */
	/*
		プログラム使用定数の定義
	*/

/* ---------------------------------------------------------------- */

/* ---------------------------------------------------------------- */

extern DG_TEX *DG_SpotLightBaseTexture;

/* ---------------------------------------------------------------- */
	/*
		プログラムワーク
	*/
typedef	struct	{
	GV_ACT_EX	actor ;
	int			map ;
	int			name ;

	int			tri_id ;
	int			tex_id ;
	DG_TEX		*tex ;

	FMATRIX		light[2] ;
	DG_PATCH	*patch ;

#ifdef PSX2
	DG_TEX_LOADREPLACE		*tex_replace[2] ;
#else
#ifndef KP_WINDOWS
	DG_TEX_LIN				lin_tex ;
#endif
#endif
#ifndef KP_WINDOWS
	DG_TEX		*ipu_tex ;
#else
	DG_TEX_LIN	*ipu_tex ;	// Windows版では諸事情でDE_TEX_LIN形式を使用
#endif

	int			light_enable ;
	int			n_panel ;
	int			tex_replace_clock ;
	FVECTOR		uv[4] ;

} Work ;

/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
	/*
		プログラム使用サブルーチン
	*/
#if 0
/* テクスチャ情報をリージョンリピートテクスチャに設定する */
static void TextureRepeatSet( DG_TEX *tex )
{
	int		tw, th, w, h, tx, ty, ofx, ofy, tmp ;
	tw = ( tex->tex_trans.tex0.data >> 26 ) & 0x0f ;
	th = ( tex->tex_trans.tex0.data >> 30 ) & 0x0f ;
	w = 1 << tw ;
	h = 1 << th ;
	tmp = w * tex->u_scale ;
	tx = 1 ; while ( tx < tmp ) tx <<= 1 ;
	tmp = h * tex->v_scale ;
	ty = 1 ; while ( ty < tmp ) ty <<= 1 ;
	ofx = w * tex->u_offset ;
	ofx &= ~( tx - 1 );
	ofy = h * tex->v_offset ;
	ofy &= ~( ty - 1 );
	tex->tex_trans.clamp.data = SCE_GS_SET_CLAMP( 3, 3, tx-1, ofx, ty-1, ofy );
	tex->u_scale = (float)tx / w ;
	tex->v_scale = (float)ty / h ;
}
/* テクスチャのスケーリング値を取得する */
static void GetTextureScale( DG_TEX *tex, FVECTOR *scale )
{
	int		tw, th, w, h, tx, ty, tmp ;
	tw = ( tex->tex_trans.tex0.data >> 26 ) & 0x0f ;
	th = ( tex->tex_trans.tex0.data >> 30 ) & 0x0f ;
	w = 1 << tw ;
	h = 1 << th ;
	tmp = w * tex->u_scale ;
	tx = 1 ; while ( tx < tmp ) tx <<= 1 ;
	tmp = h * tex->v_scale ;
	ty = 1 ; while ( ty < tmp ) ty <<= 1 ;
	scale->vx = 1.0f / ( w / tx ) ;
	scale->vy = 1.0f / ( h / ty ) ;
}
#endif

/* ムービーイメージを部分テクスチャ転送パケットに乗せる */
#ifdef PSX2
static void SetLoadImage( DG_TEX_LOADREPLACE *tex_replace, void *addr )
{
	/* テクセル転送用パケットに自前でムービーイメージ転送パケットを設定する */
	DG_MakeLoadImagePacket( &tex_replace->texel_load, SCE_GS_PSMCT16,
						   256, 256,
						   TEXTURE_TOP_PAGE(), DRAW_WIDTH, addr );
	/* clutは必要ないので代わりに終端タグを設定する */
	tex_replace->clut_load.dmatag0.qwc = DMATAG_SET_QWC( DMATAG_ID_RET, 0 );
	tex_replace->clut_load.dmatag0.addr = NULL ;
	tex_replace->clut_load.dmatag0.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	tex_replace->clut_load.dmatag0.vifcode[1] = SCE_VIF1_SET_NOP( 0 );
}
#endif

/* ---------------------------------------------------------------- */

/* ---------------------------------------------------------------- */
	/*
		プログラムメイン処理
	*/
static void Act( Work *work )
{

	/* バッファ切り替え */
	DG_SwitchBuffPatch( work->patch );


	//if ( work->light_enable ){
	//	DG_GetLightMatrix( &DG_ZeroVector, work->light );
	//}

   if ( DG_SpotLightBaseTexture != NULL ) {
      work->patch->tex = DG_SpotLightBaseTexture;
      work->patch->flag &= ~DG_PATCH_INVISIBLE ;
   } else {
      work->patch->tex = NULL;
      work->patch->flag |= DG_PATCH_INVISIBLE ;
   }


   /*
#ifdef PSX2
	if ( DG_SpotLightBaseImage != NULL ){
		if ( work->tex_replace_clock != -1 ){
			DG_ResetLoadReplaceTexture( work->tex_replace[ work->tex_replace_clock ] );
		} else {
			work->tex_replace_clock = 0 ;
		}
		work->tex_replace_clock = 1 - work->tex_replace_clock ;
		SetLoadImage( work->tex_replace[ work->tex_replace_clock ], DG_SpotLightBaseImage );
		DG_SetLoadReplaceTexture( work->tex_replace[ work->tex_replace_clock ] );
		work->patch->flag &= ~DG_PATCH_INVISIBLE ;
	} else {
		work->patch->flag |= DG_PATCH_INVISIBLE ;
	}
#else
#ifndef KP_WINDOWS
	if ( DG_SpotLightBaseImage != NULL ){
		DG_MakeLinerTexture2( &work->lin_tex, 256, 256, DG_TEXLIN_FORMAT_A8R8G8B8, DG_SpotLightBaseImage );
		work->ipu_tex->tex_trans.ptex = &work->lin_tex.d3dtexture ;
		work->patch->flag &= ~DG_PATCH_INVISIBLE ;
	} else {
		work->patch->flag |= DG_PATCH_INVISIBLE ;
	}
#else
	work->ipu_tex    = DG_SpotLightBaseTexture ;
	work->patch->tex = DG_SpotLightBaseTexture ;
	if ( DG_SpotLightBaseTexture != NULL ){
		work->patch->flag &= ~DG_PATCH_INVISIBLE ;
	} else {
		work->patch->flag |= DG_PATCH_INVISIBLE ;
	}
#endif
#endif*/

}

/* ---------------------------------------------------------------- */
	/*
		プログラム終了処理
	*/
static void Die( Work *work )
{
   /*
	int		i ;
	for ( i = 0 ; i < 2 ; i++ ){
#ifdef PSX2
		if ( work->tex_replace[i] != NULL ){
			if ( work->tex_replace_clock == i ){
				DG_ResetLoadReplaceTexture( work->tex_replace[i] );
			}
			DG_FreeLoadReplacePacket( work->tex_replace[i] );
		}
#endif
	}
#ifndef KP_WINDOWS
	if ( work->ipu_tex != NULL ){
		GV_DelayedFree( work->ipu_tex );
	}
#endif*/
   work->ipu_tex = NULL;
	DG_DequeuePatchObjs( work->patch );
	DG_FreePatch( work->patch );
}

/* ---------------------------------------------------------------- */
	/*
		ワーク初期化処理
	*/
static int GetResources( Work *work, int name, int where )
{
	DG_PATCH	*patch ;
	DG_PATCH_PARTS	*parts ;
	DG_PATCH_VERT	*verts0, *verts1 ;
	int		i, j ;

	work->name = name ;
	work->map = where ;
	GM_CurrentMap = where ;

	work->light[1].m[3][0] = 128 ;
	work->light[1].m[3][1] = 128 ;
	work->light[1].m[3][2] = 128 ;
	work->light[1].m[3][3] = 128 ;

	work->n_panel = 1 ;
	if ( GCL_GetOption( 'n' ) != NULL ){
		work->n_panel = GCL_GetNextInt() ;
	}

	if ( GCL_GetOption( 't' ) != NULL ){
		work->tri_id = GCL_GetNextInt() ;
		work->tex_id = GCL_GetNextInt() ;
		work->tex = DG_GetTexture2( work->tri_id, work->tex_id );
	}

	if ( GCL_GetOption( 'u' ) != NULL ){
		float	u0, v0, u1, v1 ;
		u0 = (float)GCL_GetNextInt() / 4096.0f ;
		v0 = (float)GCL_GetNextInt() / 4096.0f ;
		u1 = (float)GCL_GetNextInt() / 4096.0f ;
		v1 = (float)GCL_GetNextInt() / 4096.0f ;
		work->uv[0].vx = u0 ;
		work->uv[0].vy = v0 ;
		work->uv[0].vz = 1.0f ;
		work->uv[0].vw = 128.0f ;
		work->uv[1].vx = u1 ;
		work->uv[1].vy = v0 ;
		work->uv[1].vz = 1.0f ;
		work->uv[1].vw = 128.0f ;
		work->uv[2].vx = u0 ;
		work->uv[2].vy = v1 ;
		work->uv[2].vz = 1.0f ;
		work->uv[2].vw = 128.0f ;
		work->uv[3].vx = u1 ;
		work->uv[3].vy = v1 ;
		work->uv[3].vz = 1.0f ;
		work->uv[3].vw = 128.0f ;
	}

   // BP - Simple set the ipu texture to be the shared splot light base texture.
	work->ipu_tex = DG_SpotLightBaseTexture;

	work->patch = patch = DG_MakePatch( 0, work->n_panel, work->n_panel * 4 );
	patch->verts[1] = patch->verts[0] ;	/* 面倒なので強制的にシングルバッファモードに */
	if ( patch == NULL ) return ( -1 );
	patch->light = work->light ;
	GM_GroupObject( patch, work->map );
	DG_QueuePatchObjs( patch );
	DG_ConfigPatchLOD( patch, 0 );	/* ちょっと極端すぎかも・・・ */
	patch->tri_id = work->tri_id ;

	patch->tex = work->ipu_tex;

	for ( i = 0 ; i < work->n_panel ; i++ ){
		parts = &patch->parts[ i ] ;
		parts->v_index[0] = i * 4 + 0 ;
		parts->v_index[1] = i * 4 + 1 ;
		parts->v_index[2] = i * 4 + 2 ;
		parts->v_index[3] = i * 4 + 3 ;
		parts->parts_index[0] = -1 ;
		parts->parts_index[1] = -1 ;
		parts->parts_index[2] = -1 ;
		parts->parts_index[3] = -1 ;
		parts->flag = 0 ;
	}

	if ( GCL_GetOption( 'p' ) != NULL ){
		for ( i = 0 ; i < work->n_panel ; i++ ){
			FVECTOR		ds0, ds1, dt0, dt1 ;
			parts = &patch->parts[ i ];
			verts0 = patch->verts[0] + i * 4 ;
			verts1 = patch->verts[1] + i * 4 ;
			for ( j = 0 ; j < 4 ; j++ ){
				verts0[j].pos.vx = GCL_GetNextInt();
				verts0[j].pos.vy = GCL_GetNextInt();
				verts0[j].pos.vz = GCL_GetNextInt();
				verts0[j].pos.vw = 1.0f ;
				verts0[j].uv = work->uv[j] ;
			}
			/* パッチ曲面描画用差分値の設定 */
			GTE_SubVector( &ds0, &verts0[1].pos, &verts0[0].pos );
			GTE_SubVector( &ds1, &verts0[3].pos, &verts0[2].pos );
			GTE_SubVector( &dt0, &verts0[2].pos, &verts0[0].pos );
			GTE_SubVector( &dt1, &verts0[3].pos, &verts0[1].pos );
			verts0[0].pos_ds = ds0 ;
			verts0[0].pos_dt = dt0 ;
			verts0[1].pos_ds = ds0 ;
			verts0[1].pos_dt = dt1 ;
			verts0[2].pos_ds = ds1 ;
			verts0[2].pos_dt = dt0 ;
			verts0[3].pos_ds = ds1 ;
			verts0[3].pos_dt = dt1 ;
			/* バウンディングの設定 */
			GTE_MaxVector( &parts->max, &verts0[0].pos, &verts0[1].pos );
			GTE_MinVector( &parts->min, &verts0[0].pos, &verts0[1].pos );
			GTE_MaxVector( &parts->max, &parts->max, &verts0[2].pos );
			GTE_MinVector( &parts->min, &parts->min, &verts0[2].pos );
			GTE_MaxVector( &parts->max, &parts->max, &verts0[3].pos );
			GTE_MinVector( &parts->min, &parts->min, &verts0[3].pos );
			if ( i == 0 ){
				patch->max = parts->max ;
				patch->min = parts->min ;
			} else {
				GTE_MaxVector( &patch->max, &patch->max, &parts->max );
				GTE_MinVector( &patch->min, &patch->min, &parts->min );
			}
#if 0
			/* ＵＶ値の設定 */
			verts0[0].uv.vx = 0.0f ;
			verts0[0].uv.vy = 0.0f ;
			verts0[0].uv.vz = 1.0f ;
			verts0[0].uv.vw = 128.0f ;
			verts0[1].uv.vx = 1.0f ;
			verts0[1].uv.vy = 0.0f ;
			verts0[1].uv.vz = 1.0f ;
			verts0[1].uv.vw = 128.0f ;
			verts0[2].uv.vx = 0.0f ;
			verts0[2].uv.vy = 1.0f ;
			verts0[2].uv.vz = 1.0f ;
			verts0[2].uv.vw = 128.0f ;
			verts0[3].uv.vx = 1.0f ;
			verts0[3].uv.vy = 1.0f ;
			verts0[3].uv.vz = 1.0f ;
			verts0[3].uv.vw = 128.0f ;
#endif
		}
	}

	if ( GCL_GetOption( 'l' ) != NULL ){
		work->light_enable = 1 ;
		//DG_GetLightMatrix( &DG_ZeroVector, work->light );
		work->light[0] = DG_LightMatrix ;
		work->light[1] = DG_ColorMatrix ;
	}

	/* テクスチャ入れ替え開始 */
#ifdef PSX2
	work->tex_replace[0] = DG_MakeLoadReplacePacket( work->tri_id, work->tex_id, NULL, NULL );
	work->tex_replace[1] = DG_MakeLoadReplacePacket( work->tri_id, work->tex_id, NULL, NULL );
	//SetLoadImage( DG_TEX_LOADREPLACE *tex_replace, void *addr )
	//DG_SetLoadReplaceTexture( work->tex_replace );
#endif

	work->tex_replace_clock = -1 ;

	work->patch->flag |= DG_PATCH_INVISIBLE ;
#ifndef PSX2
	/* XBOXにて線形テクスチャを使用すると不具合が起きるためテクスチャラップを禁止する */
	work->patch->flag |= DG_PATCH_NO_WRAP ;
#endif
	//for ( i = 0 ; i < 65536 ; i++ ) work->data_buffer[ i ] = i ;

	return (0);
}


/* ---------------------------------------------------------------- */
	/*
		シナリオ起動インターフェイス
	*/
void *NewIPUDisplayPanelSet( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}


/* ---------------------------------------------------------------- */
