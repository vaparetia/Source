/*
	prim2.c
	チャンネル処理ユニット／新プリミティブ処理ルーチン

	1999/12/22 K.Takabe
	$Id: particle.cpp,v 1.1.1.3 2002/11/19 11:42:17 Yoshizawa1 Exp $

*/
/*
	チャンネル処理ユニット／新プリミティブ処理ルーチン



	void DG_Prim2Chanl( DG_CHANL *cp, int which)
	DG_CHANL	*cp;		チャンネル構造体
	int		which;		ダブルバッファ選択

	キューされたプリミティブ２のセットアップ、ソートなどを行なう


	------------------------------------------------

	DG_PRIM2 *DG_MakePrim2( int flag, int n_prims, int n_verts, int chanl)
	int		flag;		生成フラグ
	int		n_prims;	プリミティブ数
	int		n_verts;	１プリミティブ中に含まれる頂点数
	int		chanl;		対象チャンネル

		プリミティブ２オブジェクトの生成を行なう
		ソートはプリミティブ単位で各プリミティブのＺ中央値が参照される
		プリミティブ中に含まれる頂点数の最大はプリミティブの種類によって
		異なる（ライン、ポリゴン：６４・スプライト：３２・回転スプライト：１６）


	void DG_FreePrim2( DG_PRIM2 *prim)
	DG_PRIM2	*prim;	オブジェクトハンドル

		プリミティブ２オブジェクトの開放


	------------------------------------------------

	void DG_ConfigPrim2Tex( DG_PRIM2 *prim, DG_TEX *tex)
	DG_PRIM2	*prim;	オブジェクトハンドル
	DG_TEX		*tex;	テクスチャ

		プリミティブにテクスチャを設定する



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
// パーティクル(ポイントスプライト)
//-----------------------------------------------------------------------------

#ifdef KP_XBOX
// XBOXではテクスチャステージが違う。なんでだ(^^;
#define DG_PARTICLE_TEXTURESTAGE 3
#else
#define DG_PARTICLE_TEXTURESTAGE 0
#endif

//#define N_PARTICLE_VBUF   16
#define N_PARTICLE_VBUF   32
#define N_PARTICLE_POINT  2048
static int DG_ParticleVbufID;
static LPDIRECT3DVERTEXBUFFER8 DG_ParticleVbuf[N_PARTICLE_VBUF];

void DG_InitParticleVertexBuffer(void)
{
	int i;
	DG_ParticleVbufID = 0;
	for (i = 0; i < N_PARTICLE_VBUF; i++) {
		DG_CreateVertexBuffer(
				N_PARTICLE_POINT * sizeof(DG_POINTVERTEX),
				D3DUSAGE_DYNAMIC,
				D3DFVF_POINTVERTEX,
				D3DPOOL_DEFAULT,
				&DG_ParticleVbuf[i]);
	}
}

void DG_ReleaseParticleVertexBuffer(void)
{
	int i;
	DG_ParticleVbufID = 0;
	for (i = 0; i < N_PARTICLE_VBUF; i++) {
		if (DG_ParticleVbuf[i]) {
			DG_ParticleVbuf[i]->Release();
			DG_ParticleVbuf[i] = NULL;
		}
	}
}

void DG_FreeParticle(DG_PARTICLE *p)
{
    if (p == NULL) return;
    GV_DelayedFree(p);
}

static inline DWORD FtoDW( FLOAT f ) { return *((DWORD*)&f); }

static void ChainParticleObj(DG_PARTICLE *p)
{
	LPDIRECT3DVERTEXBUFFER8 vbuf;
	DG_POINTVERTEX          *pv;
	int                     size;
	int                     n_verts;

	n_verts = p->n_verts[p->buffer_clock];
	if (n_verts > p->n_verts_max) {
		//printf("Warning!! particle buffer over %d\n", n_verts);
		n_verts = p->n_verts_max;
	} else if (n_verts <= 0) {
		return;
	}

	do {
		vbuf = DG_ParticleVbuf[DG_ParticleVbufID];
#ifdef KP_XBOX
		if (vbuf->IsBusy()) {
			printf("Busy %d\n", DG_ParticleVbufID);
			DG_ParticleVbufID = (DG_ParticleVbufID + 1) & (N_PARTICLE_VBUF - 1);
			continue;
		}
#endif
		DG_ParticleVbufID = (DG_ParticleVbufID + 1) & (N_PARTICLE_VBUF - 1);
		break;
	} while (1);

	size = sizeof(DG_POINTVERTEX) * n_verts;
	vbuf->Lock(0, size, (unsigned char **)&pv, 0);
	memcpy(pv, p->point[p->buffer_clock], size);
	vbuf->Unlock();
	
	DG_SetTransform(D3DTS_WORLD, &p->world);	

	DG_SetRenderState(D3DRS_POINTSPRITEENABLE, TRUE);
	DG_SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	// スケール計算
	if (p->flag & DG_PARTICLE_SCALEENABLE) {
		DG_SetRenderState(D3DRS_POINTSCALEENABLE, TRUE);
	} else {
		DG_SetRenderState(D3DRS_POINTSCALEENABLE, FALSE);
	}
	// テクスチャ指定
	if (p->flag & DG_PARTICLE_TEX) {
		DG_SetTexture(DG_PARTICLE_TEXTURESTAGE, p->tex->tex_trans.ptex);
	} else {
		DG_SetTexture(DG_PARTICLE_TEXTURESTAGE, NULL);
	}
#ifdef KP_XBOX
    DG_SetTextureStageState(DG_PARTICLE_TEXTURESTAGE, D3DTSS_COLOROP, D3DTOP_MODULATE2X);
    DG_SetTextureStageState(DG_PARTICLE_TEXTURESTAGE, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    DG_SetTextureStageState(DG_PARTICLE_TEXTURESTAGE, D3DTSS_COLORARG2, D3DTA_CURRENT);
    DG_SetTextureStageState(DG_PARTICLE_TEXTURESTAGE, D3DTSS_ALPHAOP,   D3DTOP_MODULATE4X);
    DG_SetTextureStageState(DG_PARTICLE_TEXTURESTAGE, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    //DG_SetTextureStageState(DG_PARTICLE_TEXTURESTAGE, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
	DG_SetTextureStageState(DG_PARTICLE_TEXTURESTAGE, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
#endif
	DG_SetSemitransAdd();
	
    DG_SetRenderState(D3DRS_POINTSIZE,     FtoDW(p->size));
    DG_SetRenderState(D3DRS_POINTSIZE_MIN, FtoDW(p->size_min));
    DG_SetRenderState(D3DRS_POINTSCALE_A,  FtoDW(p->scale_a));
    DG_SetRenderState(D3DRS_POINTSCALE_B,  FtoDW(p->scale_b));
    DG_SetRenderState(D3DRS_POINTSCALE_C,  FtoDW(p->scale_c));

	DG_SetStreamSource(0, vbuf, sizeof(DG_POINTVERTEX));
	DG_SetVertexShader(D3DFVF_POINTVERTEX);
	DG_DrawPrimitive(D3DPT_POINTLIST, 0, n_verts);

	DG_SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	DG_SetRenderState(D3DRS_POINTSPRITEENABLE, FALSE);
	DG_SetRenderState(D3DRS_POINTSCALEENABLE, FALSE);
	DG_SetTexture(DG_PARTICLE_TEXTURESTAGE, NULL);
#ifdef KP_XBOX
	DG_SetTextureStageState(DG_PARTICLE_TEXTURESTAGE, D3DTSS_COLOROP, D3DTOP_DISABLE);
	DG_SetTextureStageState(DG_PARTICLE_TEXTURESTAGE, D3DTSS_ALPHAOP, D3DTOP_DISABLE);	
#endif
}

void DG_ConfigParticleTex(DG_PARTICLE *p, DG_TEX *tex)
{
	if (p == NULL) return;
	p->tex = tex;
}

void DG_SetParticleSize(DG_PARTICLE *p, float size)
{
	ASSERT(size >= 0.0f);
	p->size = size;
}

void DG_SetParticleSizeMin(DG_PARTICLE *p, float size)
{
	ASSERT(size >= 0.0f);
	p->size_min = size;
}

void DG_SetParticleNum(DG_PARTICLE *p, int n_verts)
{
	//ASSERT(n_verts <= N_PARTICLE_POINT);
	if (n_verts > N_PARTICLE_POINT) {
		n_verts = N_PARTICLE_POINT;
	}
	p->n_verts[p->buffer_clock] = n_verts;
}

DG_POINTVERTEX *DG_GetParticleBuf(DG_PARTICLE *p)
{
	return p->point[p->buffer_clock];
}

void DG_SwitchBuffParticle(DG_PARTICLE *p)
{
    p->buffer_clock = 1 - p->buffer_clock;
}


// POINTSCALE_A, POINTSCALE_B, POINTSCALE_C に渡す値。
// それぞれのパラメータをA,B,Cと置くと、
// ポイントスプライトのサイズ S は、次の式で計算されます。
// S = V * Size * sqrt(1 / (A + B * d + C * d^2))
//
// V    ... ビューポート高さ
// Size ... D3DRS_POINTSIZEで規定されるサイズ
// d    ... 視点からの距離

void DG_SetParticleScaleParam(DG_PARTICLE *p, float a, float b, float c)
{
	ASSERT(a >= 0.0f && b >= 0.0f && c >= 0.0f);
	p->scale_a = a;
	p->scale_b = b;
	p->scale_c = c;
}

// n_verts には想定される最大サイズを入れておく
// DG_MakeParticle 直後は p->n_verts = 0; になっているため、描画されない
DG_PARTICLE *DG_MakeParticle(int flag, int n_verts, int chanl)
{
	int size;
	DG_PARTICLE    *p;
	DG_POINTVERTEX *point;
	
	size = sizeof(DG_PARTICLE) + sizeof(DG_POINTVERTEX) * n_verts * 2;

	p = (DG_PARTICLE *)GV_Malloc(size);
	GV_ZeroMemory(p, size);

	p->flag = flag;
	p->world = DG_UnitMatrix;
	p->chanl = chanl;
	p->n_verts[0] = p->n_verts[1] = 0;
	p->n_verts_max = n_verts;
	point = (DG_POINTVERTEX *)&p[1];
	p->point[0] = point;
	p->point[1] = point + n_verts;

	p->scale_a = 1.0f;
	p->scale_b = 0.0f;
	p->scale_c = 0.0f;
	p->size = 1.0f;
	p->size_min = 0.0f;
	return p;	
}

void DG_ParticleChanl(DG_CHANL *cp, int which)
{
	int i;
	int max_objs;
	int invisible_flag;
	DG_OBJ_BUFFER *obj_buff;
	DG_PARTICLE   *particle, **que;

	if (cp->obj_queue == NULL) return;
	obj_buff = &cp->obj_queue->particle_buffer;
	que = (DG_PARTICLE **)obj_buff->queue;
	invisible_flag = DG_PARTICLE_INVISIBLE0 << cp->chanl_num;
	
	//printf("Particle Chanl %d\n", obj_buff->n_queue);
	max_objs = 0;
	for (i = obj_buff->n_queue; i > 0; i--, que++) {
		particle = *que;
		if (particle->flag & invisible_flag) continue;
		max_objs++;
	}
	if (max_objs == 0) return;

	// とりあえずソートも何もしない
	DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	DG_SetRenderState(D3DRS_LIGHTING, FALSE);
	DG_SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);

	DG_SetTransform(D3DTS_VIEW, &cp->eye_inv);
	DG_SetTransform(D3DTS_PROJECTION, &cp->pers);

	DG_BeginScene();
    DG_SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	que = (DG_PARTICLE **)obj_buff->queue;	
	for (i = obj_buff->n_queue; i > 0; i--, que++) {
		particle = *que;
		ChainParticleObj(particle);
	}
    DG_SetRenderState(D3DRS_ZWRITEENABLE, TRUE);	
	DG_EndScene();
}
