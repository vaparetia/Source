/*
  shadowvl.cpp
  シャドウボリューム処理

  2001/06/18 M.Kobayashi
  $Id: shadowvl.cpp,v 1.1.1.3 2002/11/19 11:42:24 Yoshizawa1 Exp $

 */

#ifdef KP_XBOX
#include <xtl.h>
#else
#include <d3dx8.h>
#endif

#include "libgv.h"
#include "libdg.h"
#include "dgconf.h"
#include "private.h"

//-----------------------------------------------------------------------------
// シャドウパレット

class SHADOWPALLET {
private:
	DWORD		dwSrcBlend;
	DWORD		dwDestBlend;
	DWORD		dwBlendOp;
	D3DCOLOR	rgba;
public:
	DG_PRIM2_PACKET*	pCwPacket;	// ClockWise 用リスト
	DG_PRIM2_PACKET*	pCcwPacket;	// CounterClockWise 用リスト
	void SetParam(DWORD dwSrcBlend, DWORD dwDestBlend, DWORD dwBlendOp, D3DCOLOR rgba){
		// 設定
		this->dwSrcBlend = dwSrcBlend;
		this->dwDestBlend = dwDestBlend;
		this->dwBlendOp = dwBlendOp;
		this->rgba = rgba;
	}
	void DrawStencil(void){	// ステンシルを見ながら画面全体を描く
		//Z テストなし、ステンシル有効
		DG_SetRenderState( D3DRS_ZENABLE,          FALSE );

		// アルファ設定

		DG_SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		DG_SetRenderState(D3DRS_SRCBLEND, dwSrcBlend);
		DG_SetRenderState(D3DRS_DESTBLEND, dwDestBlend);
		DG_SetRenderState(D3DRS_BLENDOP, dwBlendOp);

		DG_SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALL );

		
		// ステンシルテスト条件
		DG_SetRenderState( D3DRS_STENCILREF,  0x1 );
		DG_SetRenderState( D3DRS_STENCILFUNC, D3DCMP_LESSEQUAL );

		// ステンシルはとことんリセットする
		DG_SetRenderState( D3DRS_STENCILZFAIL,  D3DSTENCILOP_ZERO );
		DG_SetRenderState( D3DRS_STENCILFAIL,   D3DSTENCILOP_ZERO );
		DG_SetRenderState( D3DRS_STENCILPASS,   D3DSTENCILOP_ZERO );

		DG_SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
		
		DG_2DCOLORVERTEX v[4];
		v[0].v = D3DXVECTOR4(   0.f, 480.f, 0.0f, 1.0f ); v[0].rgba = rgba;
		v[1].v = D3DXVECTOR4(   0.f,   0.f, 0.0f, 1.0f ); v[1].rgba = rgba;
		v[2].v = D3DXVECTOR4( 640.f, 480.f, 0.0f, 1.0f ); v[2].rgba = rgba;
		v[3].v = D3DXVECTOR4( 640.f,   0.f, 0.0f, 1.0f ); v[3].rgba = rgba;
		DG_SetVertexShader( D3DFVF_2DCOLORVERTEX );
		g_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, v, sizeof(v[0]) );
	}
};
static SHADOWPALLET shadowpallet[DG_MAX_SHADOWPALLET];
void DG_SetShadowPallet(int shadowID, DWORD dwSrcBlend, DWORD dwDestBlend, DWORD dwBlendOp, D3DCOLOR rgba)
{
	if( shadowID >= DG_MAX_SHADOWPALLET ) {
		printf( "DG_SetShadowPallet: too large ID\n");
		return;
	}
	shadowpallet[shadowID].SetParam(dwSrcBlend, dwDestBlend, dwBlendOp, rgba);
}


//-----------------------------------------------------------------------------
static int BoundCheck(MATRIX *mat, VECTOR *bound)
{
	int    i, flag, and_flag;
	float  w;
	VECTOR vec;

	flag = 0;
	and_flag = CLIP_FLAG;
	for (i = 8; i > 0; i --) {
		vec.vx = (i & 0x01) ? bound[0].vx : bound[1].vx;
		vec.vy = (i & 0x02) ? bound[0].vy : bound[1].vy;
		vec.vz = (i & 0x04) ? bound[0].vz : bound[1].vz;
		vec.vw = 1.0f;
		// 透視変換
		D3DXVec4Transform(&vec, &vec, mat);

		// clipw
		w = DG_FABS(vec.vw);
		if (vec.vx >   w) flag |= CLIP_X0_FLAG;
		if (vec.vx < - w) flag |= CLIP_X1_FLAG;
		if (vec.vy >   w) flag |= CLIP_Y0_FLAG;
		if (vec.vy < - w) flag |= CLIP_Y1_FLAG;
		if (vec.vz >   w) flag |= CLIP_Z0_FLAG;
		if (vec.vz < - w) flag |= CLIP_Z1_FLAG;
		and_flag &= flag;
	}
	if (and_flag & CLIP_FLAG) return 2; // 完全に画面外
	return 0; // 画面内
}

//-----------------------------------------------------------------------------
// 頂点バッファ直接指定版 PRIM2 描画関数
static void WriteVertexPacks(DG_PRIM2_PACKET *packet,
							 LPDIRECT3DVERTEXBUFFER8 vbuf)
{
	ASSERT(packet->n_prims > 0);
	if (packet->n_prims <= 0) return;
	DG_SetStreamSource(0, vbuf, sizeof(DG_PRIM2VERTEX));
	DG_DrawPrimitive((D3DPRIMITIVETYPE)packet->d3dtype, packet->MinIndex, packet->n_prims);
}

static void WriteIndexPacks(DG_PRIM2_PACKET *packet,
							 LPDIRECT3DVERTEXBUFFER8 vbuf,
							 LPDIRECT3DINDEXBUFFER8 ibuf)
{
	ASSERT(packet->n_prims > 0);
	if (packet->n_prims <= 0) return;
	DG_SetStreamSource(0, vbuf, sizeof(DG_PRIM2VERTEX));	
	DG_SetIndices(ibuf, 0);
	DG_DrawIndexedPrimitive((D3DPRIMITIVETYPE)packet->d3dtype,
							packet->MinIndex, packet->NumVertices,
							packet->StartIndex, packet->n_prims );
}

// 描画
static int WriteShadowPacks(DG_PRIM2_PACKET *packet)
{
    DG_PRIM2                *prim;
	LPDIRECT3DINDEXBUFFER8  ibuf;
	LPDIRECT3DVERTEXBUFFER8 vbuf;

	// 頂点バッファが割り当てられていない
	if (packet->verts_buf_id < 0) {
		// DG_PRIM2_V* ならこの時点で割り当てられているはず
		return 0;
	}
	vbuf = packet->GetVBuf();
	ibuf = packet->GetIBuf();

	prim = (DG_PRIM2 *)packet->prim;

	
	switch (packet->type) {
	case DG_PRIM2_VLINE:
	case DG_PRIM2_VTRIANGLE:
	case DG_PRIM2_VPOLY:
		WriteVertexPacks(packet, vbuf);
		break;
	case DG_PRIM2_IVLINE:
	case DG_PRIM2_IVTRIANGLE:
	case DG_PRIM2_IVPOLY:
		WriteIndexPacks(packet, vbuf, ibuf);
		break;
	default:
		printf("shadowvol: Can't Render this Prim Type ... %d\n", packet->type);
		break;
	}
	return 0;
}


// シャドウボリューム処理ステージ
void DG_ShadowVolChanl(DG_CHANL *cp, int which)
{
	DG_OBJ_QUEUE    *queue;
	DG_OBJ_BUFFER   *obj_buff;
	DG_PRIM2        **pque, *prim2;
	DG_PRIM2_PACKET* pPacket;
	int             i, j, gid, c_gid, invisible_flag;
	
	if ((queue = cp->obj_queue) == NULL) return;
	obj_buff = &queue->prim2_buffer;

	// キュー数のチェック
	if ((i = obj_buff->n_queue) == 0) return;
	
	// シャドウパレットリセット
	for( j = 0 ; j < DG_MAX_SHADOWPALLET; j++ ){
		shadowpallet[j].pCwPacket = shadowpallet[j].pCcwPacket = NULL;
	}
	
	// パース変換パラメータ／クリッピング領域を設定
	pque = (DG_PRIM2 **)obj_buff->queue;
	c_gid = cp->group_id;

	invisible_flag = DG_PRIM2_INVISIBLE0 << cp->chanl_num;

	// 各種レンダ－ステート設定
	DG_SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
	DG_SetRenderState(D3DRS_LIGHTING, FALSE);
	DG_SetRenderState(D3DRS_FOGENABLE, FALSE);
	DG_SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
	DG_SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT );

	// ステンシル設定
	DG_SetRenderState( D3DRS_ZWRITEENABLE,  FALSE );
	DG_SetRenderState( D3DRS_STENCILENABLE, TRUE );

	DG_BeginScene();

	for (; i > 0; i --, pque ++) {
		prim2 = *pque;
		if (!(prim2->flag & DG_PRIM2_SHADOWVOL)) continue;
		if (prim2->flag & invisible_flag) continue;

        // 表示グループチェック
		gid = prim2->group_id;
        if (gid != 0 && !(gid & c_gid)) {
			printf("group id check failed\n");
			continue;
		}

        // プリミティブ設置マトリクス確定
        if (prim2->root != NULL) {
			prim2->world = *prim2->root;
		}

		// NOTE: PRIM2の変換基準チェック
		// DirectX では、変換に使用する行列を SetTransform で指定する仕様だが、
		// VIEW, PROJECTION, WORLD に分離した形式でしか設定できない。
		//
		// 通常は
		// PROJECTION = cp->pers;
		// VIEW       = cp->eye_inv;
		// である。
		// カメラ相対の時は VIEW = D3DXMatrixIdentity(); でいいのかな？
		// eye_pers = pers * eye;
		
        // 変換基準チェック
        if (prim2->flag & DG_PRIM2_ON_CAMERA) {
			// カメラ相対  (position * pers)
			prim2->screen = cp->pers;
			prim2->world = cp->eye;
		} else {
			// 通常計算    (position * world * eye_inv * pers)
			D3DXMatrixMultiply(&prim2->screen, &prim2->world, &cp->eye_pers);
        }
        // バウンディングチェック
		if (prim2->flag & DG_PRIM2_BOUNDCHECK) {
			if (BoundCheck(&prim2->screen, &prim2->bound_min) == 2) {
				//printf("boundcheck failed\n");
				continue;
            }
		}

		ASSERT( prim2->shadowID < DG_MAX_SHADOWPALLET );
		SHADOWPALLET* pSPallet = &shadowpallet[prim2->shadowID];

		// リストを作る
		pPacket = prim2->packet[prim2->buffer_clock];
		for (j = prim2->n_prims; j > 0; j --, pPacket ++) {
			// 非表示フラグチェック
			if (pPacket->flag & invisible_flag) continue;
			if(pPacket->flag & DG_PRIM2_CW){
				pPacket->next_addr = pSPallet->pCwPacket;
				pSPallet->pCwPacket = pPacket;
			}else{
				pPacket->next_addr = pSPallet->pCcwPacket;
				pSPallet->pCcwPacket = pPacket;
			}
		}
	}

	// 描画
	SHADOWPALLET* pSPallet = &shadowpallet[0];
	for( i = DG_MAX_SHADOWPALLET ; i > 0  ; i--, pSPallet++ ){
		if( pSPallet->pCwPacket == NULL && pSPallet->pCcwPacket == NULL) continue;
		
		//Z テストあり
		DG_SetRenderState( D3DRS_ZENABLE,       TRUE );

		// 画面に描かれないようにする
		DG_SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
		DG_SetRenderState( D3DRS_COLORWRITEENABLE, 0 );
		
		// ステンシルテスト条件
		DG_SetRenderState( D3DRS_STENCILFUNC,   D3DCMP_ALWAYS );

		DG_SetRenderState( D3DRS_STENCILZFAIL,  D3DSTENCILOP_KEEP );
		DG_SetRenderState( D3DRS_STENCILFAIL,   D3DSTENCILOP_KEEP );

		// 頂点シェーダ設定
		DG_SetVertexShader(D3DFVF_PRIM2VERTEX);
	
		// まず表側だけを描いてインクリメント
		DG_PRIM2* pPrimPrev = NULL;
		DG_PRIM2* pPrim;
		DG_SetRenderState( D3DRS_STENCILPASS,      D3DSTENCILOP_INCR );
		DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
		for( pPacket = pSPallet->pCwPacket ; pPacket != NULL ; pPacket = (DG_PRIM2_PACKET*)pPacket->next_addr ){
			if( (pPrim = (DG_PRIM2*)pPacket->prim ) != pPrimPrev ) {
				DG_SetTransform(D3DTS_WORLD, &pPrim->world);
				pPrimPrev = pPrim;
			}
			WriteShadowPacks( pPacket );
		}
		DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		for( pPacket = pSPallet->pCcwPacket ; pPacket != NULL ; pPacket = (DG_PRIM2_PACKET*)pPacket->next_addr ){
			if( (pPrim = (DG_PRIM2*)pPacket->prim ) != pPrimPrev ) {
				DG_SetTransform(D3DTS_WORLD, &pPrim->world);
				pPrimPrev = pPrim;
			}
			WriteShadowPacks( pPacket );
		}

		// 次に裏側だけを描いてデクリメント
		DG_SetRenderState( D3DRS_STENCILPASS,      D3DSTENCILOP_DECR );
		for( pPacket = pSPallet->pCwPacket ; pPacket != NULL ; pPacket = (DG_PRIM2_PACKET*)pPacket->next_addr ){
			if( (pPrim = (DG_PRIM2*)pPacket->prim ) != pPrimPrev ) {
				DG_SetTransform(D3DTS_WORLD, &pPrim->world);
				pPrimPrev = pPrim;
			}
			WriteShadowPacks( pPacket );
		}
		DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
		for( pPacket = pSPallet->pCcwPacket ; pPacket != NULL ; pPacket = (DG_PRIM2_PACKET*)pPacket->next_addr ){
			if( (pPrim = (DG_PRIM2*)pPacket->prim ) != pPrimPrev ) {
				DG_SetTransform(D3DTS_WORLD, &pPrim->world);
				pPrimPrev = pPrim;
			}
			WriteShadowPacks( pPacket );
		}

		// 最後に画面全体に影を塗る
		pSPallet->DrawStencil();
	}

	// Z、ステンシルをもとにもどす
	DG_SetRenderState( D3DRS_ZENABLE,      TRUE );
	DG_SetRenderState( D3DRS_ZWRITEENABLE,  TRUE );
	DG_SetRenderState( D3DRS_STENCILENABLE, FALSE );
	DG_SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );

	DG_SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
	
	DG_SetAlphaMode(0);
	
	DG_EndScene();
}

