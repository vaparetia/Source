/*
	prim2.c
	チャンネル処理ユニット／新プリミティブ処理ルーチン

	1999/12/22 K.Takabe
	$Id: prim2.cpp,v 1.1.1.3 2002/11/19 11:42:20 Yoshizawa1 Exp $

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

// XBOX追加・PRIM2専用VertexBufferの確保

// 各プリミティブの最大数
//#define N_BUF          (2048) // 最大数(2のn乗が条件)
#define N_BUF          (1024) // 最大数(2のn乗が条件)
//#define N_BUF_COLOR    (N_BUF)
//#define N_BUF_TEXTURE  (N_BUF)

// 各頂点バッファに含まれる最大頂点数
// SPRTの最大数を基準に決める
#define N_SPRT_MAX          (32)
#define N_VERTEX_MAX        (N_SPRT_MAX * 4)
//#define N_VERTEX_COLOR      (N_VERTEX_MAX)
//#define N_VERTEX_TEXTURE    (N_VERTEX_MAX)

// 各インデックスバッファに含まれる最大頂点数
#define N_INDEX_MAX         (N_SPRT_MAX * 6)

// 頂点バッファの種類
enum {
	DG_VERTEX_COLOR,    // 頂点 + 色
	DG_VERTEX_TEXTURE,  // 頂点 + 色 + テクスチャ
};

// 頂点バッファの状態フラグ
enum {
	DG_VBUF_FREE         = 0x00,
	DG_VBUF_LOCKED       = 0x01,
	DG_VBUF_USED         = 0x02,
	DG_VBUF_DELAYED_FREE = 0x04,
};

// 頂点バッファ/インデックスバッファ管理用構造体
typedef struct tagDG_VERTEXBUFFER {
	struct tagDG_VERTEXBUFFER *next; // Delayed Free List作成時に使用する
	unsigned int              flag;
	unsigned short			nRef;	// 	リファレンスカウンタ
	LPDIRECT3DVERTEXBUFFER8   vbuf;
	LPDIRECT3DINDEXBUFFER8    ibuf;

	unsigned short ReleaseBuf(unsigned int flag){
		if( nRef > 0 ) {
			nRef--;
			if(nRef == 0) this->flag = flag;
		} else {
			this->flag = flag;
		}
		return nRef;
	}
} DG_VERTEXBUFFER;

// 頂点バッファ管理情報
// size = stride * n_verts なので冗長。
typedef struct tagDG_VERTEXBUFFER_INFO {
	int size;               // 頂点バッファ全体のサイズ
	unsigned int fvf;       // FVFフラグ
	int stride;             // 頂点一個のサイズ
	int n_verts;            // 頂点バッファに含まれる頂点数
	int n_index;            // インデックスバッファに含まれる頂点数
	int n_buf;              // 頂点バッファの数

	int count;              // 割り当て済み頂点バッファの数	
	DG_VERTEXBUFFER *delayed_free_list;   // Delayed Freeする頂点バッファへのポインタ
	DG_VERTEXBUFFER dg_vbuf[N_BUF];
} DG_VERTEXBUFFER_INFO;


// PRIM2用頂点バッファ情報
static DG_VERTEXBUFFER_INFO DG_VertexBufInfo[] = {
	{N_VERTEX_MAX * sizeof(DG_PRIM2VERTEX), // size
	 D3DFVF_PRIM2VERTEX,              // fvf
	 sizeof(DG_PRIM2VERTEX),          // stride
	 N_VERTEX_MAX,                    // n_verts
	 N_INDEX_MAX,                     // n_index
	 N_BUF,                           // n_buf
	 0,                               // count
	 NULL,
	},
#if 0
	// 煩雑なので、DG_VERTEX_COLORとDG_VERTEX_TEXTURE分割を止める
	// DG_VERTEX_TEXTURE 用
	{N_VERTEX_TEXTURE * sizeof(DG_PRIM2VERTEX), // size
	 D3DFVF_PRIM2VERTEX,                // fvf
	 sizeof(DG_PRIM2VERTEX),            // stride
	 N_VERTEX_TEXTURE,                  // n_verts
	 N_INDEX_MAX,                       // n_index
	 N_BUF_TEXTURE,                     // n_buf
	 0,                                 // count
	 NULL,
	},
#endif
};

// Delayed Free 用の頂点リスト先頭
static DG_VERTEXBUFFER *DG_DelayedFreeVertexBuf = NULL;

// VertexBufferを確保する
static void CreateVertexBuffer(DG_VERTEXBUFFER_INFO *info)
{
	int i, n, size;
	DG_VERTEXBUFFER *dg_vbuf;
	
	// 頂点バッファ管理領域を初期化
	size = sizeof(DG_VERTEXBUFFER) * N_BUF;
	GV_ZeroMemory(info->dg_vbuf, size);

	dg_vbuf = info->dg_vbuf;
	n = info->n_buf;	
	for (i = 0; i < n; i++, dg_vbuf++) {
		DG_CreateVertexBuffer(
				info->size,
				D3DUSAGE_DYNAMIC, // この辺のフラグはXboxでは無視される
				info->fvf,
				D3DPOOL_DEFAULT,
				&dg_vbuf->vbuf);
		DG_CreateIndexBuffer(
				info->n_index * sizeof(short),
				D3DUSAGE_DYNAMIC,
				D3DFMT_INDEX16,   // 16bit index
				D3DPOOL_DEFAULT,
				&dg_vbuf->ibuf);
		//printf("vbuf = %p, *buf = %p\n", buf, *buf);
	}
}

// PRIM2で使用するVertexBuffer/IndexBufferをあらかじめ確保する
void DG_InitPrim2VertexBuffer(void)
{
	//int i;
	//for (i = 0; i < DG_VERTEX_MAX; i ++) {
	CreateVertexBuffer(&DG_VertexBufInfo[0]);
	//}
}

// VertexBufferを解放する
static void ReleaseVertexBuffer(DG_VERTEXBUFFER_INFO *info)
{
	int i, n;
	DG_VERTEXBUFFER *dg_vbuf;

	n = info->n_buf;
	dg_vbuf = info->dg_vbuf;
	for (i = n; i > 0; i--, dg_vbuf++) {
		if (dg_vbuf->vbuf != NULL) {
			dg_vbuf->vbuf->Release(); // 頂点バッファを開放
		}
		if (dg_vbuf->ibuf != NULL) {
			dg_vbuf->ibuf->Release(); // インデックスバッファを開放
		}
	}
}

// PRIM2で使用するVertexBufferを全て解放する
void DG_ReleasePrim2VertexBuffer(void)
{
	//int i;
	//for (i = 0; i < DG_VERTEX_MAX; i++) {
	ReleaseVertexBuffer(&DG_VertexBufInfo[0]);
	//}
}

// Delayed Free された頂点バッファを解放する
void DG_CleanPrim2VertexBuffer(void)
{
	DG_VERTEXBUFFER *dg_vbuf, *dg_vbuf_old;
	DG_VERTEXBUFFER_INFO *info = DG_VertexBufInfo;
	//int i;
	
	//for (i = 0; i < DG_VERTEX_MAX; i++, info++) {
	dg_vbuf = info->delayed_free_list;
	info->delayed_free_list = NULL;
		
	while (dg_vbuf != NULL) {
		if (!(dg_vbuf->flag & DG_VBUF_DELAYED_FREE)) {
			ASSERT(0);
		}
		dg_vbuf->flag = DG_VBUF_FREE;
		dg_vbuf_old = dg_vbuf;
		dg_vbuf = dg_vbuf->next;
		dg_vbuf_old->next = NULL;
		//info->count--; // 割り当て合計数を減らす
		//ASSERT(info->count >= 0);
	}
	//}
}

// 頂点バッファを Delayed Free する
static inline void DG_DelayedFreeVertexBuffer(DG_PRIM2 *prim)
{
	int i;
	DG_VERTEXBUFFER      *dg_vbuf;
	DG_VERTEXBUFFER      *next;
	DG_VERTEXBUFFER_INFO *info;
	DG_PRIM2_PACKET      *packet;
	int id;
	
	// バッファ1個目
	packet = prim->packet[0];
	for (i = 0; i < prim->n_prims; i++, packet++) {
		id = packet->verts_buf_id;		
		if (id >= 0) {
			info = &DG_VertexBufInfo[0];
			dg_vbuf = &info->dg_vbuf[id];
			if( dg_vbuf->ReleaseBuf(DG_VBUF_DELAYED_FREE) == 0 ){
				// Delayed Free リストに繋げる
				next = info->delayed_free_list;
				info->delayed_free_list = dg_vbuf;
				dg_vbuf->next = next;
			}
		}
	}

	// シングルバッファならここでおしまい
    if (prim->flag & DG_PRIM2_SINGLEBUFF) return;

	// バッファ2個目
	packet = prim->packet[1];
	for (i = 0; i < prim->n_prims; i++, packet++) {	
		id = packet->verts_buf_id;
		if (id >= 0) {
			info = &DG_VertexBufInfo[0];
			dg_vbuf = &info->dg_vbuf[id];
			if(dg_vbuf->ReleaseBuf(DG_VBUF_DELAYED_FREE) == 0){
				// Delayed Free リストに繋げる
				next = info->delayed_free_list;
				info->delayed_free_list = dg_vbuf;
				dg_vbuf->next = next;
			}
		}
	}
}


// 頂点バッファ/インデックスバッファを取得する
// 返り値はバッファ管理構造体のID
static int DG_GetVertexBufferID(DG_VERTEXBUFFER_INFO *info)
{
	DG_VERTEXBUFFER         *dg_vbuf;

	do {
		dg_vbuf = &info->dg_vbuf[info->count];
#ifdef KP_XBOX
		if (dg_vbuf->flag == DG_VBUF_FREE && !dg_vbuf->vbuf->IsBusy()) break;		
#else
		if (dg_vbuf->flag == DG_VBUF_FREE) break;		
#endif
		//info->count = (info->count + 1) % N_BUF;
		info->count = (info->count + 1) & (N_BUF - 1);
	} while (1);
	dg_vbuf->flag = DG_VBUF_USED;
	return info->count;

#if 0
	int i;
	DG_VERTEXBUFFER *dg_vbuf;
	
	// 空いている領域を返す
	dg_vbuf = info->dg_vbuf;
	for (i = 0; i < info->n_buf; i++, dg_vbuf++) {
		if (dg_vbuf->flag == DG_VBUF_FREE) {
#ifdef KP_XBOX
			//Xbox専用関数/バッファ使用中チェック
			if (dg_vbuf->vbuf->IsBusy()) {
				printf("Busy %d\n", i);
				continue;
			}
#endif
			dg_vbuf->flag = DG_VBUF_USED;
			return i;
		}
	}
#endif	
	ASSERT(0);
	return -1;
}

// PRIM2のパケットに頂点バッファ/インデックスバッファを割り当てる
static void DG_Prim2AllocVertexBuffer(int flag, DG_PRIM2_PACKET *packet)
{
	int n_verts_max;
	int n_index_max;
	DG_VERTEXBUFFER_INFO *info;

	// テクスチャのON,OFFで頂点バッファフォーマットが違う
	if (flag & DG_PRIM2_TEX) {
		packet->verts_type = DG_VERTEX_TEXTURE;
	} else {
		packet->verts_type = DG_VERTEX_COLOR;
	}
	info = &DG_VertexBufInfo[0];

	// packet->sizeに頂点バッファをロックするサイズを入れておく
	switch (flag & DG_PRIM2_TYPEMASK) {
	case DG_PRIM2_POLY:
	case DG_PRIM2_CULLPOLY:
		n_verts_max = packet->n_verts;
		n_index_max = packet->n_verts * 2; /* 本当は 5 / 3 */
		break;
	case DG_PRIM2_LINE:
		// LINEリストにして描いてしまう。
		// 最大で、n 個の頂点から 2 * (n - 1) 本の線を描く可能性がある。
		n_verts_max = packet->n_verts;
		n_index_max = 2 * (packet->n_verts - 1);
		break;
	case DG_PRIM2_SPRT:
	case DG_PRIM2_RSPRT:
		n_verts_max = packet->n_verts * 4; // 1頂点から4点座標を生成する
		n_index_max = packet->n_verts * 6; // インデックスは6点になる
		break;
	case DG_PRIM2_VLINE:
	case DG_PRIM2_IVLINE:
		n_verts_max = packet->n_verts;
		n_index_max = packet->n_verts; // インデックスバッファは使用しないが、一応。
		ASSERT(packet->n_verts % 2 == 0);
		packet->n_prims = packet->n_verts / 2;
		packet->d3dtype = D3DPT_LINELIST;
		break;
	case DG_PRIM2_VTRIANGLE:
	case DG_PRIM2_IVTRIANGLE:
		n_verts_max = packet->n_verts;
		n_index_max = packet->n_verts;
		ASSERT(packet->n_verts % 3 == 0);
		packet->n_prims = packet->n_verts / 3;
		packet->d3dtype = D3DPT_TRIANGLELIST;
		break;
	case DG_PRIM2_VPOLY:
	case DG_PRIM2_IVPOLY:
		n_verts_max = packet->n_verts;
		n_index_max = packet->n_verts;
		packet->n_prims = packet->n_verts - 2;
		packet->d3dtype = D3DPT_TRIANGLESTRIP;
		break;
	default:
		printf("??? Unknown Primitive\n");
		ASSERT(0);
		break;
	}

	// 頂点バッファをロックするサイズ
	packet->size = n_verts_max * info->stride;
	if (n_verts_max > N_VERTEX_MAX) {
		printf("PRIM2: Vertex Number Over ... %d\n", n_verts_max);
		ASSERT(0);
	}

	// インデックスバッファをロックするサイズ
	packet->index_size = n_index_max * sizeof(short);
	if (n_index_max > N_INDEX_MAX) {
		printf("PRIM2: Index Number Over ... %d\n", n_index_max);
		ASSERT(0);
	}
	// 頂点バッファ割り当て
	//packet->verts_buf_id = DG_GetVertexBufferID(info);
	packet->verts_buf_id = -1;
	//printf("verts_buf_id = %d\n", packet->verts_buf_id);
}

//-----------------------------------------------------------------------------



#define ALIGNSIZE1(_n)   (_n)
#define ALIGNSIZE16(_n)  (((_n)+15)&0xfffffff0)
#define ALIGNSIZE64(_n)  (((_n)+63)&0xffffffc0)
#define ALIGNSIZE128(_n) (((_n)+127)&0xffffff80)
#define SIZEOF_WORD(_v)	 (sizeof(_v) / sizeof(int))
#define SIZEOF_QWORD(_v) (sizeof(_v) / 16)

#define USE_ABE	1
#define USE_AA1	0

// スクラッチパッド割り当て
typedef struct {
    MATRIX    eye_pers;
    MATRIX    eye_pers2;
    MATRIX    pers;
    MATRIX    world;
    MATRIX    screen;
	MATRIX    eye;
    VECTOR    bound[8];
    VECTOR    scale;
    VECTOR    tmp_vec;
    void      *ot3[64];
    void      *ot2[64];
    void      *ot1[64];
} ScrpadWork;

#define SCRPAD      ((ScrpadWork*)SCRPAD_ADDR)
#define EYE_MAT     (&(SCRPAD->eye))
#define PERS_MAT    (&(SCRPAD->pers))
#define EYE_PERS    (&(SCRPAD->eye_pers))
#define EYE_PERS2   (&(SCRPAD->eye_pers2))
#define WORLD       (&(SCRPAD->world))
#define BOUNDS      (SCRPAD->bound)
#define SCALE       (&SCRPAD->scale)
#define SORT_Z      (SCRPAD->sort_z)
#define TMP_VEC     (SCRPAD->tmp_vec)
#define OT1         (SCRPAD->ot1)
#define OT2         (SCRPAD->ot2)
#define OT3         (SCRPAD->ot3)

//-----------------------------------------------------------------------------

extern "C" {
void *DG_Prim2OT[64];
}

//-----------------------------------------------------------------------------

// プリミティブのソートリストを作成する
// SCRPAD の ot1 に接続する
static void MakeSortListPrim(DG_PRIM2_PACKET *packet, int raise)
{
    ScrpadWork *work = (ScrpadWork *)SCRPAD_ADDR;
    u_int      z;
    void       **ot;

    /* ソート用データを作成 */
    z = ((u_int)(packet->sort_z + 0x7fffff - raise) >> 6);/* あらかじめ６４で割っておく */
    packet->sort_z = z;
	z &= 63;
    ot = work->ot1 + z;  // ot1 の z に接続
    packet->next_addr = *ot;
	*ot = packet;
}

// ソートを行う
static void SortListPrims(void)
{
    ScrpadWork      *work = (ScrpadWork *)SCRPAD_ADDR;
    int             i;
    u_int           z;
    DG_PRIM2_PACKET *packet;
    void            **ot2, **ot3, *next_addr;

    /* 順番を崩さないように大きい方からソートしていく */
    for (i = 63; i >= 0 ; i --) {
        next_addr = work->ot1[i];
        while (next_addr != NULL) {
            packet = (DG_PRIM2_PACKET *)next_addr;
            next_addr = packet->next_addr;
            z = packet->sort_z;
			z = (z >> 6) & 63;
            ot2 = work->ot2 + z;
            packet->next_addr = *ot2;
            *ot2 = packet;
        }
    }

    /* 順番を崩さないように小さい方からソートしていく */
    for (i = 0; i < 64; i ++) {
        next_addr = work->ot2[i];
        while (next_addr != NULL) {
            packet = (DG_PRIM2_PACKET *)next_addr;
            next_addr = packet->next_addr;
            z = packet->sort_z;
			z = (z >> 12) & 63;
            ot3 = work->ot3 + z;
            packet->next_addr = *ot3;
            *ot3 = packet;
        }
    }

    {/* 最終ＯＴをメインメモリに転送する */
#if 0
        u_long128 *dst, *src;
        src = (u_long128 *)work->ot3;
        dst = (u_long128 *)DG_Prim2OT;
        for (i = 64 / 4; i > 0; i--){
            *dst ++ = *src ++;
        }
#else
        u_long64 *dst, *src;
        src = (u_long64 *)work->ot3;
        dst = (u_long64 *)DG_Prim2OT;
        for (i = 64 / 4 * 2; i > 0; i --){
            *dst++ = *src++;
        }
#endif
    }
}


// プリミティブパケット用に頂点バッファを確保→ロック
// ロックした頂点バッファを返す
// ※頂点バッファをダブルバッファで持ってみたが、
//   ダブルバッファにしてもロックの時にブロックされる…。
DG_PRIM2VERTEX *DG_LockPrimPack(DG_PRIM2_PACKET *packet)
{
	short new_id = DG_PRIM2_PACKET::AllocBuffer();
	packet->CopyBuffer(new_id);
	packet->SetBuffer(new_id);
	return packet->LockVBuf();
	
#if 0	
	DG_VERTEXBUFFER         *dg_vbuf, *prev_dg_vbuf;
	DG_VERTEXBUFFER_INFO    *info;
	LPDIRECT3DVERTEXBUFFER8 vbuf, prev_vbuf;
	int  prev_id;
	void *pv, *prev_pv;
	HRESULT hr;

	// 新規に頂点バッファを割り当てる		
	info = &DG_VertexBufInfo[0];
	prev_id = packet->verts_buf_id;	
	packet->verts_buf_id = DG_GetVertexBufferID(info);
	dg_vbuf = &info->dg_vbuf[packet->verts_buf_id];	
	
	//printf("Lock %d\n", packet->verts_buf_id);
#ifdef KP_XBOX
	// Xboxの場合は、バッファ使用中テストが出来る。
	// ※使用中の場合 Lock でブロックされてしまう。
	if (dg_vbuf->vbuf->IsBusy()) {
		printf("PRIM2 Vertex Buffer %d is Busy ....\n", packet->verts_buf_id);
		ASSERT(0);
	}
#endif
	vbuf = dg_vbuf->vbuf;

	hr = vbuf->Lock(0, packet->size, (unsigned char **)&pv, 0);
	if (FAILED(hr)) {
		printf("Lock failed\n");
		ASSERT(0);
	}

	if (prev_id >= 0) {
		// 前回割り当て済みのバッファがあれば
		// 内容をコピーする
		prev_dg_vbuf = &info->dg_vbuf[prev_id];
		prev_vbuf = prev_dg_vbuf->vbuf;
		prev_vbuf->Lock(0, packet->size, (unsigned char **)&prev_pv, D3DLOCK_READONLY);
		memcpy(pv, prev_pv, packet->size);
		prev_vbuf->Unlock();
		prev_dg_vbuf->flag = DG_VBUF_FREE;
		//printf("Free %d\n", prev_id);
	}
	dg_vbuf->flag |= DG_VBUF_LOCKED;
	return (DG_PRIM2VERTEX *)pv;
#endif	
}

// ロック後は必ずUnlockする必要あり
void DG_UnlockPrimPack(DG_PRIM2_PACKET *packet)
{
	packet->UnlockVBuf();
}

///////////////////// DG_PRIM2_PACKET 関数
short DG_PRIM2_PACKET::AllocBuffer(void)
{
	DG_VERTEXBUFFER_INFO    *info;
	short	new_id;
	
	// 新規に頂点バッファを割り当てる		
	info = &DG_VertexBufInfo[0];
	new_id = DG_GetVertexBufferID(info);
	//	SetBuffer(new_id);
	
	return new_id;
}

void DG_PRIM2_PACKET::CopyBuffer(short idbuf)
{	// 現在のバッファを idbuf にコピーする
	DG_VERTEXBUFFER_INFO    *info = &DG_VertexBufInfo[0];
	if( idbuf >= 0 ) {
		if(idbuf != verts_buf_id && verts_buf_id >= 0){
			// 内容をコピーする
			LPDIRECT3DVERTEXBUFFER8 pVbufDst = info->dg_vbuf[idbuf].vbuf;
			LPDIRECT3DVERTEXBUFFER8 pVbufSrc = info->dg_vbuf[verts_buf_id].vbuf;
			
			DG_PRIM2VERTEX* pvDst;
			DG_PRIM2VERTEX* pvSrc;
			
			if( FAILED( pVbufDst->Lock(0, 0, (unsigned char **)&pvDst, 0) ) ) HANGUP();
			if( FAILED( pVbufSrc->Lock(0, 0, (unsigned char **)&pvSrc, D3DLOCK_READONLY) ) ) HANGUP();

			memcpy(pvDst, pvSrc, info->size);	// 全部コピーする必要はないのだが。。。
			pVbufDst->Unlock();
			pVbufSrc->Unlock();
			
			if( type >= DG_PRIM2_IVLINE ){
				// インデクスバッファもコピーする
				LPDIRECT3DINDEXBUFFER8 pIbufDst = info->dg_vbuf[idbuf].ibuf;
				LPDIRECT3DINDEXBUFFER8 pIbufSrc = info->dg_vbuf[verts_buf_id].ibuf;
			
				short* piDst;
				short* piSrc;
				
				if( FAILED( pIbufDst->Lock(0, 0, (unsigned char **)&piDst, 0) ) ) HANGUP();
				if( FAILED( pIbufSrc->Lock(0, 0, (unsigned char **)&piSrc, D3DLOCK_READONLY) ) ) HANGUP();

				memcpy(piDst, piSrc, info->n_index * sizeof(short));	// 全部コピーする必要はないのだが。。。
				pIbufDst->Unlock();
				pIbufSrc->Unlock();
			}
		}
	}
}

void DG_PRIM2_PACKET::SetBuffer(short idbuf)
{
	DG_VERTEXBUFFER_INFO    *info = &DG_VertexBufInfo[0];
	if(idbuf >= 0) {
		info->dg_vbuf[idbuf].nRef++;
	}
	if( verts_buf_id >= 0){
		info->dg_vbuf[verts_buf_id].ReleaseBuf(DG_VBUF_FREE);
	}
	verts_buf_id = idbuf;
}

DG_PRIM2VERTEX* DG_PRIM2_PACKET::LockVBuf(short id)
{ // 頂点バッファのロック
	DG_VERTEXBUFFER_INFO    *info;
	void *pv;
	info = &DG_VertexBufInfo[0];

	LPDIRECT3DVERTEXBUFFER8 vbuf = info->dg_vbuf[id].vbuf;
	if( FAILED( vbuf->Lock(0, 0, (unsigned char **)&pv, 0) ) ) HANGUP();
	info->dg_vbuf[id].flag |= DG_VBUF_LOCKED;
	return (DG_PRIM2VERTEX*)pv;
}

LPDIRECT3DVERTEXBUFFER8 DG_PRIM2_PACKET::GetVBuf(void)
{
	if( verts_buf_id < 0 ) return NULL;
	return DG_VertexBufInfo[0].dg_vbuf[ verts_buf_id ].vbuf;
}

LPDIRECT3DINDEXBUFFER8 DG_PRIM2_PACKET::GetIBuf(void)
{
	if( verts_buf_id < 0 ) return NULL;
	return DG_VertexBufInfo[0].dg_vbuf[ verts_buf_id ].ibuf;
}

void 	DG_PRIM2_PACKET::UnlockVBuf(short id)
{	// 頂点バッファのアンロック
	DG_VERTEXBUFFER         *dg_vbuf;
	DG_VERTEXBUFFER_INFO    *info;
	LPDIRECT3DVERTEXBUFFER8 vbuf;

	ASSERT(id >= 0);
	info = &DG_VertexBufInfo[0];
	dg_vbuf = &info->dg_vbuf[id];
	vbuf = dg_vbuf->vbuf;
	if (FAILED(vbuf->Unlock())) {
		printf("????? vertex buffer Unlock failed\n");
		ASSERT(0);
	}
	dg_vbuf->flag &= ~DG_VBUF_LOCKED;
	dg_vbuf->flag |= DG_VBUF_USED;	
}

short* DG_PRIM2_PACKET::LockIBuf(short id)
{	// インデックスバッファのロック
	DG_VERTEXBUFFER_INFO    *info;
	void *pi;
	info = &DG_VertexBufInfo[0];

	LPDIRECT3DINDEXBUFFER8 ibuf = info->dg_vbuf[id].ibuf;
	if( FAILED( ibuf->Lock(0, 0, (unsigned char **)&pi, 0) ) ) HANGUP();
	return (short*)pi;
}

void 	DG_PRIM2_PACKET::UnlockIBuf(short id)
{	// インデックスバッファのアンロック
	DG_VERTEXBUFFER         *dg_vbuf;
	DG_VERTEXBUFFER_INFO    *info;
	LPDIRECT3DINDEXBUFFER8 ibuf;

	ASSERT(id >= 0);
	info = &DG_VertexBufInfo[0];
	dg_vbuf = &info->dg_vbuf[id];
	ibuf = dg_vbuf->ibuf;
	if (FAILED(ibuf->Unlock())) {
		printf("????? index buffer Unlock failed\n");
		ASSERT(0);
	}
}


void DG_PRIM2_PACKET::SetDrawPrim( u_short MinIndex, u_short nPrims , u_short d3dType )
{	// 描画を登録する
	this->MinIndex = MinIndex;
	if(d3dType != 0) this->d3dtype = d3dType;
	if(nPrims != 0) {
		n_prims = nPrims;
		switch(this->d3dtype){
		case D3DPT_POINTLIST:
		case D3DPT_LINELOOP:
			NumVertices = nPrims;
			break;
		case D3DPT_LINELIST:
			NumVertices = nPrims * 2;
			break;
		case D3DPT_LINESTRIP:
			NumVertices = nPrims + 1;
			break;
		case D3DPT_TRIANGLELIST:
			NumVertices = nPrims * 3;
			break;
		case D3DPT_TRIANGLESTRIP:
		case D3DPT_TRIANGLEFAN:
			NumVertices = nPrims + 2;
			break;
		//
		}
	}
}

void DG_PRIM2_PACKET::SetDrawIndexedPrim( u_short MinIndex, u_short NumVertices,
										  u_short StartIndex, u_short nPrims , u_short d3dType )
{	// 描画を登録する
	this->MinIndex = MinIndex;
	this->StartIndex = StartIndex;
	if(d3dType != 0) this->d3dtype = d3dType;
	if(nPrims != 0) {
		n_prims = nPrims;
		if( NumVertices == 0){
			switch(this->d3dtype){
			case D3DPT_POINTLIST:
			case D3DPT_LINELOOP:
				NumVertices = nPrims;
				break;
			case D3DPT_LINELIST:
				NumVertices = nPrims * 2;
				break;
			case D3DPT_LINESTRIP:
				NumVertices = nPrims + 1;
				break;
			case D3DPT_TRIANGLELIST:
				NumVertices = nPrims * 3;
				break;
			case D3DPT_TRIANGLESTRIP:
			case D3DPT_TRIANGLEFAN:
				NumVertices = nPrims + 2;
				break;
				//
			}
		}
	}
	if( NumVertices != 0 ) this->NumVertices = NumVertices;
}
	



/////////////////////

// SPRT 接続処理
static void WriteSprtPacks(DG_PRIM2_PACKET *packet, int which, int stride,
						   LPDIRECT3DVERTEXBUFFER8 vbuf, LPDIRECT3DINDEXBUFFER8 ibuf)
{
	int i;
	int n_index = 0;
	
	if (packet->verts_type == DG_VERTEX_TEXTURE) {
		// テクスチャ有り
		DG_PRIM2_UVRGBWH *uvrgbwh = (DG_PRIM2_UVRGBWH *)packet->uvrgb_addr;
		VECTOR           *pos = packet->pos_addr;
		DG_PRIM2VERTEX   *pv;
		short            *pi;
		DWORD            color;
		
		// 頂点バッファをロックして転送する
		vbuf->Lock(0,                     // ロックする頂点データへのオフセット(byte)
				   packet->size,          // ロックするサイズ(byte)
				   (unsigned char **)&pv, // 頂点データが格納されるメモリ
				   0                      // ロックフラグ
				   );

		// インデックスバッファをロックして転送する
		ibuf->Lock(0,                     // ロックする頂点データへのオフセット(byte)
				   packet->index_size,    // ロックするサイズ(byte)
				   (unsigned char **)&pi, // 頂点データが格納されるメモリ
				   0                      // ロックフラグ
				   );
		color = D3DCOLOR_RGBA(uvrgbwh->r, uvrgbwh->g, uvrgbwh->b, uvrgbwh->a);
		
		// 1つのデータから4頂点を作成する
		for (i = packet->n_verts; i > 0; i--, pos++, uvrgbwh++) {
			// 右下
			pv[0].v.x = pos->vx + uvrgbwh->w;
			pv[0].v.y = pos->vy + uvrgbwh->h;
			pv[0].v.z = pos->vz;
			pv[0].rgba = color;
			pv[0].tu0 = uvrgbwh->u1 / (float)uvrgbwh->q1;
			pv[0].tv0 = uvrgbwh->v1 / (float)uvrgbwh->q1;

			// 右上
			pv[1].v.x = pos->vx + uvrgbwh->w;
			pv[1].v.y = pos->vy - uvrgbwh->h;
			pv[1].v.z = pos->vz;
			pv[1].rgba = color;
			pv[1].tu0 = uvrgbwh->u1 / (float)uvrgbwh->q1;
			pv[1].tv0 = uvrgbwh->v0 / (float)uvrgbwh->q0;

			// 左下
			pv[2].v.x = pos->vx - uvrgbwh->w;
			pv[2].v.y = pos->vy + uvrgbwh->h;
			pv[2].v.z = pos->vz;
			pv[2].rgba = color;
			pv[2].tu0 = uvrgbwh->u0 / (float)uvrgbwh->q0;
			pv[2].tv0 = uvrgbwh->v1 / (float)uvrgbwh->q1;

			// 左上
			pv[3].v.x = pos->vx - uvrgbwh->w;
			pv[3].v.y = pos->vy - uvrgbwh->h;
			pv[3].v.z = pos->vz;
			pv[3].rgba = color;
			pv[3].tu0 = uvrgbwh->u0 / (float)uvrgbwh->q0;
			pv[3].tv0 = uvrgbwh->v0 / (float)uvrgbwh->q0;
			pv += 4;

			pi[0] = n_index;
			pi[1] = n_index + 1;
			pi[2] = n_index + 2;
			pi[3] = n_index + 3;
			pi[4] = n_index + 3;
			pi[5] = n_index + 4;
			n_index += 4;
			pi += 6;
		}
	} else if (packet->verts_type == DG_VERTEX_COLOR) {
		// テクスチャ無し
		DG_PRIM2_UVRGBWH *uvrgbwh = (DG_PRIM2_UVRGBWH *)packet->uvrgb_addr;
		VECTOR         *pos = packet->pos_addr;
		DG_PRIM2VERTEX *pv;
		short          *pi;
		DWORD          color;
		
		// 頂点バッファをロックして転送する
		vbuf->Lock(0,                     // ロックする頂点データへのオフセット(byte)
				   packet->size,          // ロックするサイズ(byte)
				   (unsigned char **)&pv, // 頂点データが格納されるメモリ
				   0                      // ロックフラグ(D3DLOCK_DISCARD推奨)
				   );
		// インデックスバッファをロックして転送する
		ibuf->Lock(0,                     // ロックする頂点データへのオフセット(byte)
				   packet->index_size,    // ロックするサイズ(byte)
				   (unsigned char **)&pi, // 頂点データが格納されるメモリ
				   0                      // ロックフラグ(D3DLOCK_DISCARD推奨)
				   );
		// 1つのデータから4頂点を作成する
		color = D3DCOLOR_RGBA(uvrgbwh->r, uvrgbwh->g, uvrgbwh->b, uvrgbwh->a);
		for (i = packet->n_verts; i > 0; i --, pos ++, uvrgbwh ++) {
			// 右下
			pv[0].v.x = pos->vx + uvrgbwh->w;
			pv[0].v.y = pos->vy + uvrgbwh->h;
			pv[0].v.z = pos->vz;
			pv[0].rgba = color;

			// 右上
			pv[1].v.x = pos->vx + uvrgbwh->w;
			pv[1].v.y = pos->vy - uvrgbwh->h;
			pv[1].v.z = pos->vz;
			pv[1].rgba = color;

			// 左下
			pv[2].v.x = pos->vx - uvrgbwh->w;
			pv[2].v.y = pos->vy + uvrgbwh->h;
			pv[2].v.z = pos->vz;
			pv[2].rgba = color;

			// 左上
			pv[3].v.x = pos->vx - uvrgbwh->w;
			pv[3].v.y = pos->vy - uvrgbwh->h;
			pv[3].v.z = pos->vz;
			pv[3].rgba = color;
			pv += 4;

			pi[0] = n_index;
			pi[1] = n_index + 1;
			pi[2] = n_index + 2;
			pi[3] = n_index + 3;
			pi[4] = n_index + 3;
			pi[5] = n_index + 4;
			n_index += 4;
			pi += 6;
		}
	}
	ibuf->Unlock();
	vbuf->Unlock();
	DG_SetStreamSource(0, vbuf, stride);
	DG_SetIndices(ibuf, 0);
	DG_DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, // プリミティブの種類
							0,                   // 最小 index 
							n_index,             // 頂点数
							0,                   // index 開始点
							n_index / 2);        // プリミティブ数
}

// RSPRT 接続処理
static void WriteRSprtPacks(DG_PRIM2_PACKET *packet, int which, int stride,
							LPDIRECT3DVERTEXBUFFER8 vbuf, LPDIRECT3DINDEXBUFFER8 ibuf)
{
	int i;
	int n_index = 0;
	
	if (packet->verts_type == DG_VERTEX_TEXTURE) {
		// テクスチャ有り
		DG_PRIM2_UVRGBWH *uvrgbwh = (DG_PRIM2_UVRGBWH *)packet->uvrgb_addr;
		VECTOR           *pos = packet->pos_addr;
		DG_PRIM2VERTEX   *pv; // 頂点バッファデータ
		short            *pi; // インデックスバッファデータ
		DWORD            color;
		
		// 頂点バッファをロックして転送する
		vbuf->Lock(0,                     // ロックする頂点データへのオフセット(byte)
				   packet->size,          // ロックするサイズ(byte)
				   (unsigned char **)&pv, // 頂点データが格納されるメモリ
				   0                      // ロックフラグ(D3DLOCK_DISCARD推奨)
				   );
		// インデックスバッファをロックして転送する
		ibuf->Lock(0,                     // ロックする頂点データへのオフセット(byte)
				   packet->index_size,    // ロックするサイズ(byte)
				   (unsigned char **)&pi, // 頂点データが格納されるメモリ
				   0                      // ロックフラグ(D3DLOCK_DISCARD推奨)
				   );
		color = D3DCOLOR_RGBA(uvrgbwh->r, uvrgbwh->g, uvrgbwh->b, uvrgbwh->a);
		// 1つのデータから4頂点を作成
		for (i = packet->n_verts; i > 0; i--, pos++, uvrgbwh++) {
			// 右下
			pv[0].v.x = pos->vx + uvrgbwh->w;
			pv[0].v.y = pos->vy + uvrgbwh->h;
			pv[0].v.z = pos->vz;
			pv[0].rgba = color;
			pv[0].tu0 = uvrgbwh->u1 / (float)uvrgbwh->q1;
			pv[0].tv0 = uvrgbwh->v1 / (float)uvrgbwh->q1;

			// 右上
			pv[1].v.x = pos->vx + uvrgbwh->h;
			pv[1].v.y = pos->vy - uvrgbwh->w;
			pv[1].v.z = pos->vz;
			pv[1].rgba = color;
			pv[1].tu0 = uvrgbwh->u1 / (float)uvrgbwh->q1;
			pv[1].tv0 = uvrgbwh->v0 / (float)uvrgbwh->q0;

			// 左下
			pv[2].v.x = pos->vx - uvrgbwh->h;
			pv[2].v.y = pos->vy + uvrgbwh->w;
			pv[2].v.z = pos->vz;
			pv[2].rgba = color;
			pv[2].tu0 = uvrgbwh->u0 / (float)uvrgbwh->q0;
			pv[2].tv0 = uvrgbwh->v1 / (float)uvrgbwh->q1;

			// 左上
			pv[3].v.x = pos->vx - uvrgbwh->w;
			pv[3].v.y = pos->vy - uvrgbwh->h;
			pv[3].v.z = pos->vz;
			pv[3].rgba = color;
			pv[3].tu0 = uvrgbwh->u0 / (float)uvrgbwh->q0;
			pv[3].tv0 = uvrgbwh->v0 / (float)uvrgbwh->q0;
			pv += 4;
			
			// インデックスバッファに座標セット
			pi[0] = n_index;
			pi[1] = n_index + 1;
			pi[2] = n_index + 2;
			pi[3] = n_index + 3;
			pi[4] = n_index + 3;
			pi[5] = n_index + 4;
			n_index += 4;
			pi += 6;
		}
	} else if (packet->verts_type == DG_VERTEX_COLOR) {
		// テクスチャ無し
		DG_PRIM2_UVRGBWH *uvrgbwh = (DG_PRIM2_UVRGBWH *)packet->uvrgb_addr;
		VECTOR         *pos = packet->pos_addr;
		DG_PRIM2VERTEX *pv;
		short          *pi;		
		DWORD          color;
		
		// 頂点バッファをロックして転送する
		vbuf->Lock(0,                     // ロックする頂点データへのオフセット(byte)
				   packet->size,          // ロックするサイズ(byte)
				   (unsigned char **)&pv, // 頂点データが格納されるメモリ
				   0                      // ロックフラグ(D3DLOCK_DISCARD推奨)
				   );
		// インデックスバッファをロックして転送する
		ibuf->Lock(0,                     // ロックする頂点データへのオフセット(byte)
				   packet->index_size,    // ロックするサイズ(byte)
				   (unsigned char **)&pi, // 頂点データが格納されるメモリ
				   0                      // ロックフラグ(D3DLOCK_DISCARD推奨)
				   );
		color = D3DCOLOR_RGBA(uvrgbwh->r, uvrgbwh->g, uvrgbwh->b, uvrgbwh->a);
		// 1つのデータから4頂点を作成
		for (i = packet->n_verts; i > 0; i--, pos++, uvrgbwh++) {
			// 右下
			pv[0].v.x = pos->vx + uvrgbwh->w;
			pv[0].v.y = pos->vy + uvrgbwh->h;
			pv[0].v.z = pos->vz;
			pv[0].rgba = color;

			// 右上
			pv[1].v.x = pos->vx + uvrgbwh->h;
			pv[1].v.y = pos->vy - uvrgbwh->w;
			pv[1].v.z = pos->vz;
			pv[1].rgba = color;

			// 左下
			pv[2].v.x = pos->vx - uvrgbwh->h;
			pv[2].v.y = pos->vy + uvrgbwh->w;
			pv[2].v.z = pos->vz;
			pv[2].rgba = color;

			// 左上
			pv[3].v.x = pos->vx - uvrgbwh->w;
			pv[3].v.y = pos->vy - uvrgbwh->h;
			pv[3].v.z = pos->vz;
			pv[3].rgba = color;

			// インデックスバッファに座標セット
			pi[0] = n_index;
			pi[1] = n_index + 1;
			pi[2] = n_index + 2;
			pi[3] = n_index + 3;
			pi[4] = n_index + 3;
			pi[5] = n_index + 4;
			n_index += 4;
			pi += 6;
		}
	}
	ibuf->Unlock();
	vbuf->Unlock();
	DG_SetStreamSource(0, vbuf, stride);	
	DG_SetIndices(ibuf, 0);
	DG_DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, // プリミティブの種類
							0,                   // 最小 index 
							n_index,             // 頂点数
							0,                   // index 開始点
							n_index / 2);        // プリミティブ数
}

// 線を描画する
// IndexBufferにして LINELISTで描く。
static void WriteLinePacks(DG_PRIM2_PACKET *packet, int which, int stride,
						   LPDIRECT3DVERTEXBUFFER8 vbuf, LPDIRECT3DINDEXBUFFER8 ibuf)
{
	HRESULT hr;
	short index;
	int i;
	int n_line;  // 描画するラインの数

	unsigned char *pv;    // 頂点バッファ要素へのポインタ
	short         *pi;    // インデックスバッファ要素へのポインタ
	VECTOR         *pos = packet->pos_addr;
	DG_PRIM2_UVRGB *uvrgb = (DG_PRIM2_UVRGB *)packet->uvrgb_addr;	

	if (vbuf == NULL || ibuf == NULL) return;
	
	// 頂点バッファをロックする
	hr = vbuf->Lock(0,                     // ロックする頂点データへのオフセット(byte)
					packet->size,          // ロックするサイズ(byte)
					(unsigned char **)&pv, // 頂点データが格納されるメモリ
					0                      // ロックフラグ(D3DLOCK_DISCARD推奨)
					);
	if (FAILED(hr)) { // 失敗されても困るが…
		printf("LinePacks: VertexBuffer Lock Failed\n");
		return;
	}
	// インデックスバッファをロックして転送する
	ibuf->Lock(0,                     // ロックする頂点データへのオフセット(byte)
			   packet->index_size,    // ロックするサイズ(byte)
			   (unsigned char **)&pi, // 頂点データが格納されるメモリ
			   0                      // ロックフラグ(D3DLOCK_DISCARD推奨)
			   );
	if (FAILED(hr)) {
		printf("LinePacks: IndexBuffer Lock Failed\n");
		return;
	}

	// テクスチャ無し
	if (packet->verts_type == DG_VERTEX_COLOR) {
		DG_PRIM2VERTEX *verts = (DG_PRIM2VERTEX *)pv;
		// 頂点バッファにデータコピー
		index = 0;
		n_line = 0;
		for (i = packet->n_verts ; ;) {
			verts->v.x = pos->vx;
			verts->v.y = pos->vy;
			verts->v.z = pos->vz;
			verts->rgba = D3DCOLOR_RGBA(uvrgb->r, uvrgb->g, uvrgb->b, uvrgb->a);
			verts ++;
			pos ++;
			uvrgb ++;

			if (-- i <= 0) break; // 最後の頂点ならここでおしまい

			// 次の頂点に描画キックフラグが立っていれば登録する
			if (!(uvrgb->f & 0x8000)) {
				*pi ++ = index ++;
				*pi ++ = index;
				n_line ++;
			} else {
				index ++;
			}
		}
	} else {
		DG_PRIM2VERTEX *verts = (DG_PRIM2VERTEX *)pv;		
		// 頂点バッファにデータコピー
		index = 0;
		n_line = 0;
		for (i = packet->n_verts ; ;) {
			verts->v.x = pos->vx;
			verts->v.y = pos->vy;
			verts->v.z = pos->vz;
			verts->rgba = D3DCOLOR_RGBA(uvrgb->r, uvrgb->g, uvrgb->b, uvrgb->a);
			verts->tu0 = uvrgb->u / (float)uvrgb->q;
			verts->tv0 = uvrgb->v / (float)uvrgb->q;
			verts ++;
			pos ++;
			uvrgb ++;

			if (-- i <= 0) break; // 最後の頂点ならここでおしまい

			// 次の頂点に描画キックフラグが立っていれば登録する
			if (!(uvrgb->f & 0x8000)) {
				*pi++ = index++;
				*pi++ = index;
				n_line++;
			} else {
				index++;
			}
		}
	}
	ibuf->Unlock();
	vbuf->Unlock();	
	DG_SetStreamSource(0, vbuf, stride);
	DG_SetIndices(ibuf, 0);
	DG_DrawIndexedPrimitive(D3DPT_LINELIST,  // プリミティブの種類
							0,               // 最小 index 
							packet->n_verts, // 頂点数
							0,               // index 開始点
							n_line);         // プリミティブ数
}

// ポリゴンを描画する
// IndexBufferを使用して頂点キック/描画キックフラグに対応する
static void WritePolyPacks(DG_PRIM2_PACKET *packet, int which, int stride,
						   LPDIRECT3DVERTEXBUFFER8 vbuf, LPDIRECT3DINDEXBUFFER8 ibuf)
{
	HRESULT hr;
	short index;
	int i;
	int n_poly;  // 描画するポリゴンの数

	unsigned char *pv;    // 頂点バッファ要素へのポインタ
	short         *pi;    // インデックスバッファ要素へのポインタ
	VECTOR         *pos = packet->pos_addr;
	DG_PRIM2_UVRGB *uvrgb = (DG_PRIM2_UVRGB *)packet->uvrgb_addr;	

	// 頂点バッファをロックする
	hr = vbuf->Lock(0,                     // ロックする頂点データへのオフセット(byte)
					0,//packet->size,      // ロックするサイズ(byte)
					(unsigned char **)&pv, // 頂点データが格納されるメモリ
					0                      // ロックフラグ(XboxではDISCARD無効)
					);
	if (FAILED(hr)) { // 失敗されても困るが…
		printf("PolyPacks: VertexBuffer Lock Failed\n");
		return;
	}
	// インデックスバッファをロックする
	hr = ibuf->Lock(0,                     // ロックする頂点データへのオフセット(byte)
					packet->index_size,    // ロックするサイズ(byte)
					(unsigned char **)&pi, // 頂点データが格納されるメモリ
					0                      // ロックフラグ(XboxではDISCARD無効)
					);
	if (FAILED(hr)) {
		printf("PolyPacks: IndexBuffer Lock Failed\n");
		return;
	}

	// テクスチャ無し
	if (packet->verts_type == DG_VERTEX_COLOR) {
		DG_PRIM2VERTEX *verts = (DG_PRIM2VERTEX *)pv;
		// 頂点バッファにデータコピー
		index = 0;
		n_poly = 0;

		*pi ++ = index ++; // 最初の点をインデックスに入れる
		for (i = packet->n_verts; ;) {
			verts->v.x = pos->vx;
			verts->v.y = pos->vy;
			verts->v.z = pos->vz;
			verts->rgba = D3DCOLOR_RGBA(uvrgb->r, uvrgb->g, uvrgb->b, uvrgb->a);
			verts ++;
			pos ++;
			uvrgb ++;

			if (-- i <= 0) break;

			if (!(uvrgb->f & 0x8000)) {
				// 描画キックならそのまま繋げる。
				*pi ++ = index ++;
				n_poly ++;
			} else if ((uvrgb - 1)->f & 0x8000) {
				// 一個前が頂点キックならそのまま繋げる。ポリゴン数は増えない。
				*pi ++ = index ++;
			} else {
				// それ以外は頂点を増やしてストリップを切る
				*pi ++ = index - 1;
				*pi ++ = index;
				*pi ++ = index ++;
				n_poly += 4; // このポリゴンは見えない
			}
		}
	} else {
		DG_PRIM2VERTEX *verts = (DG_PRIM2VERTEX *)pv;		
		index = 0;
		n_poly = 0;

		// 頂点バッファにデータコピー
		*pi ++ = index ++; // 最初の点をインデックスに入れる
		for (i = packet->n_verts; ;) {
			verts->v.x = pos->vx;
			verts->v.y = pos->vy;
			verts->v.z = pos->vz;
			verts->rgba = D3DCOLOR_RGBA(uvrgb->r, uvrgb->g, uvrgb->b, uvrgb->a);
			verts->tu0 = uvrgb->u / (float)uvrgb->q;
			verts->tv0 = uvrgb->v / (float)uvrgb->q;
			verts ++;
			pos ++;
			uvrgb ++;

			if (-- i <= 0) break;

			if (!(uvrgb->f & 0x8000)) {
				// 描画キックならそのまま繋げる。
				*pi ++ = index ++;
				n_poly ++;
			} else if ((uvrgb - 1)->f & 0x8000) {
				// 一個前が頂点キックならそのまま繋げる。ポリゴン数は増えない。
				*pi ++ = index ++;
			} else {
				// それ以外は頂点を増やしてストリップを切る
				*pi ++ = index - 1;
				*pi ++ = index;
				*pi ++ = index ++;
				n_poly += 4; // このポリゴンは見えない
			}
		}
	}
	ibuf->Unlock();
	vbuf->Unlock();
	DG_SetStreamSource(0, vbuf, stride);
	DG_SetIndices(ibuf, 0);
	DG_DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, // プリミティブの種類
							0,                   // 最小 index 
							packet->n_verts,     // 頂点数
							0,                   // index 開始点
							n_poly);             // プリミティブ数
}

// 頂点バッファ直接指定版 PRIM2 描画関数
static void WriteVertexPacks(DG_PRIM2_PACKET *packet, int stride,
							 LPDIRECT3DVERTEXBUFFER8 vbuf)
{
	ASSERT(packet->n_prims > 0);
	if (packet->n_prims <= 0) return;
	DG_SetStreamSource(0, vbuf, stride);	
	DG_DrawPrimitive((D3DPRIMITIVETYPE)packet->d3dtype, packet->MinIndex, packet->n_prims);
}

static void WriteIndexPacks(DG_PRIM2_PACKET *packet, int stride,
							 LPDIRECT3DVERTEXBUFFER8 vbuf,
							 LPDIRECT3DINDEXBUFFER8 ibuf)
{
	ASSERT(packet->n_prims > 0);
	if (packet->n_prims <= 0) return;
	DG_SetStreamSource(0, vbuf, stride);	
	DG_SetIndices(ibuf, 0);
	DG_DrawIndexedPrimitive((D3DPRIMITIVETYPE)packet->d3dtype,
							packet->MinIndex, packet->NumVertices,
							packet->StartIndex, packet->n_prims );
}

// DMA に接続する
// XBOX では DrawPrimitive 発行処理
int DG_WritePrimPacks(DG_PRIM2_PACKET *packet, DG_CHANL *cp, int which)
{
    DG_PRIM2                *prim;
	DG_VERTEXBUFFER         *dg_vbuf;
	DG_VERTEXBUFFER_INFO    *info;
	LPDIRECT3DINDEXBUFFER8  ibuf;
	LPDIRECT3DVERTEXBUFFER8 vbuf;

    prim = (DG_PRIM2 *)packet->prim;

	// コールバックが指定されていたら、システムでは一切処理しない
	if (prim->exec_func) { 
		prim->exec_func(cp, which, packet, prim->extend_data);
		return 0;
	}

	info = &DG_VertexBufInfo[0];
	// 頂点バッファが割り当てられていない
	if (packet->verts_buf_id < 0) {
		// DG_PRIM2_V* ならこの時点で割り当てられているはず
		if (packet->type >= DG_PRIM2_VLINE) return 0;
		// 頂点バッファをその都度割り当てる
		packet->verts_buf_id = DG_GetVertexBufferID(info);
		dg_vbuf = &info->dg_vbuf[packet->verts_buf_id];
		dg_vbuf->nRef++;
	}else{
		//printf("info->count = %d\n", info->count);
		dg_vbuf = &info->dg_vbuf[packet->verts_buf_id];
	}
	vbuf = dg_vbuf->vbuf;
	ibuf = dg_vbuf->ibuf;

	// PRIM2用テクスチャの設定
	DG_SetTexture(0, prim->tex_trans.ptex);
	
	// PRIM2用の描画環境設定
	DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	DG_SetRenderState(D3DRS_LIGHTING, FALSE);
	DG_SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);

	// 各PRIM2固定の設定
	if (prim->flag & DG_PRIM2_ON_CAMERA) {
		DG_SetTransform(D3DTS_WORLD, EYE_MAT);
	} else {
		DG_SetTransform(D3DTS_WORLD, &prim->world);
	}

	//半透明モードの指定
	if (prim->flag & DG_PRIM2_ALPHA) {
		DG_SetRenderState(D3DRS_ZWRITEENABLE, FALSE);		
		DG_SetAlphaMode(prim->tex_trans.alpha.data & SCE_GS_ALPHA_MASK);
	} else {
		DG_SetAlphaMode(0);
	}
	DG_SetVertexShader(info->fvf);
	switch (packet->type) {
	case DG_PRIM2_SPRT:
		WriteSprtPacks(packet, which, info->stride, vbuf, ibuf);
		packet->verts_buf_id = -1;
		dg_vbuf->ReleaseBuf(DG_VBUF_FREE);
		break;
	case DG_PRIM2_RSPRT:
		WriteRSprtPacks(packet, which, info->stride, vbuf, ibuf);
		packet->verts_buf_id = -1;
		dg_vbuf->ReleaseBuf(DG_VBUF_FREE);
		break;
	case DG_PRIM2_LINE:
		WriteLinePacks(packet, which, info->stride, vbuf, ibuf);
		packet->verts_buf_id = -1;
		dg_vbuf->ReleaseBuf(DG_VBUF_FREE);
		break;
	case DG_PRIM2_POLY:
	case DG_PRIM2_CULLPOLY:		
		WritePolyPacks(packet, which, info->stride, vbuf, ibuf);
		packet->verts_buf_id = -1;
		dg_vbuf->ReleaseBuf(DG_VBUF_FREE);
		break;

		/* Xbox追加 */
	case DG_PRIM2_VLINE:
	case DG_PRIM2_VTRIANGLE:
	case DG_PRIM2_VPOLY:
		WriteVertexPacks(packet, info->stride, vbuf);
		break;
	case DG_PRIM2_IVLINE:
	case DG_PRIM2_IVTRIANGLE:
	case DG_PRIM2_IVPOLY:
		WriteIndexPacks(packet, info->stride, vbuf, ibuf);
		break;
	default:
		printf("PRIM2: Unknown Prim Type ... %d\n", packet->type);
		break;
	}
	// Z更新フラグを元に戻す
	if (prim->flag & DG_PRIM2_ALPHA) {
		DG_SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	}
	return 0;
}

//-----------------------------------------------------------------------------

// prim2.cpp 内部で使用するテンポラリ行列
// _SetMatrix でセットされる。
static MATRIX PrimMatrix = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f,
};

static inline void _SetMatrix(MATRIX  *m)
{
	PrimMatrix = *m;
}

static void CalcSortZ(DG_PRIM2_PACKET *packet)
{
	int i;
	VECTOR *verts;
	VECTOR vecTmp;
	float  sum_w;
    register float d;
	
	ASSERT(packet->n_verts != 0);

	verts = packet->pos_addr; // 位置
	sum_w = 0.0f;
	for (i = packet->n_verts; i > 0; i--, verts++) {
		verts->vw = 1.0f;
		// 位置座標をPrimMatrixで変換
		D3DXVec4Transform(&vecTmp, verts, &PrimMatrix);
		sum_w += vecTmp.vw; // w の値を加算していく
	}

    d = 1.0f / (float)packet->n_verts;
	d *= sum_w;
	packet->sort_z = DG_FTOI(d);
}


/* バウンディングチェック */
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

// プリミティブソート処理ステージ
void DG_Prim2Chanl(DG_CHANL *cp, int which)
{
    DG_OBJ_QUEUE    *queue;
    DG_OBJ_BUFFER   *obj_buff;
    DG_PRIM2        **pque, *prim2;
    DG_PRIM2_PACKET *packet;
    int             i, j, gid, c_gid, invisible_flag;
    ScrpadWork      *scrpad = (ScrpadWork *)SCRPAD_ADDR;
	
    if ((queue = cp->obj_queue) == NULL) return;
    obj_buff = &queue->prim2_buffer;

	// キュー数のチェック
    if ((i = obj_buff->n_queue) == 0) return;
	
	// スクラッチパッド上のＯＴ初期化
    GV_ZeroMemory(scrpad->ot3, sizeof(int) * 64 * 3);

	// パース変換パラメータ／クリッピング領域を設定
    pque = (DG_PRIM2 **)obj_buff->queue;
    c_gid = cp->group_id;

	*EYE_MAT = cp->eye;
    *PERS_MAT = cp->raise_pers;
    *EYE_PERS = cp->raise_eye_pers;
    *EYE_PERS2 = cp->raise_eye_pers2;
    SCALE->vx = (float)cp->width / 2;
    SCALE->vy = (float)cp->height / 2;
    invisible_flag = DG_PRIM2_INVISIBLE0 << cp->chanl_num;

	for (; i > 0; i--, pque++) {
        prim2 = *pque;
		if (prim2->flag & invisible_flag) continue;
		if (prim2->flag & DG_PRIM2_SHADOWVOL ) continue;

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
        scrpad->world = prim2->world;

        // 変換基準チェック
        if (prim2->flag & DG_PRIM2_ON_CAMERA) {
			// カメラ相対  (position * pers)
            scrpad->screen = scrpad->pers;
        } else {
            // 通常計算    (position * world * eye_inv * pers)
			D3DXMatrixMultiply(&scrpad->screen, &scrpad->world, &scrpad->eye_pers);
        }
        prim2->screen = scrpad->screen;
#if 0	// 使わない？	M.K
        prim2->prim_param[which].screen = scrpad->screen;
#endif		

        _SetMatrix(&scrpad->screen);
        // バウンディングチェック
        if (prim2->flag & DG_PRIM2_BOUNDCHECK) {
            if (BoundCheck(&scrpad->screen, &prim2->bound_min) == 2) {
				//printf("boundcheck failed\n");
                continue;
            }
        }

        packet = prim2->packet[prim2->buffer_clock];

		// コールバック関数がある場合には、
		// ソート用z値をあらかじめ設定しておく必要がある
		if (prim2->exec_func) {
			for (j = prim2->n_prims; j > 0; j--, packet++) {
				// 非表示フラグチェック
				if (packet->flag & invisible_flag) continue;
				// work->ot1 に接続する
				MakeSortListPrim(packet, prim2->raise);
			}
		} else {
			for (j = prim2->n_prims; j > 0; j--, packet++) {
				// 非表示フラグチェック
				if (packet->flag & invisible_flag) continue;
				// scrpad->screenを使ってソート用Ｚ値計算
				CalcSortZ(packet);
				// work->ot1 に接続する
				MakeSortListPrim(packet, prim2->raise);
			}
		}
    }

    /* 全プリミティブパケットのソートを行なう */
    SortListPrims();
}

//-----------------------------------------------------------------------------

DG_PRIM2 *DG_MakePrim2(int flag, int n_prims, int n_verts, int chanl)
{
    DG_PRIM2        *prim2;
    DG_PRIM2_PACKET *packet;
    VECTOR          *pos;
    void            *uvrgb;
    int             type, size, n_buffer, uvrgb_size, packet_flag;
    int             i, j;

    packet_flag = flag & 0xffff;
    flag |= DG_PRIM2_INVISIBLE2 | DG_PRIM2_INVISIBLE3;
    type = flag & DG_PRIM2_TYPEMASK;

    if (flag & DG_PRIM2_SINGLEBUFF) {
		n_buffer = 1;
	} else {
		n_buffer = 2;
	}

    if (type < DG_PRIM2_SPRT) {
		uvrgb_size = sizeof(DG_PRIM2_UVRGB);
	} else {
		uvrgb_size = sizeof(DG_PRIM2_UVRGBWH);
	}

	// メモリサイズの計算
    size = sizeof(DG_PRIM2)
			+ sizeof(DG_PRIM2_PACKET) * n_prims * n_buffer
			+ sizeof(VECTOR) * n_prims * n_verts * n_buffer
			+ uvrgb_size * n_prims * n_verts * n_buffer;

	prim2 = (DG_PRIM2 *)GV_Malloc(size);
    if (prim2 == NULL) {
		printf("PRIM2 : malloc failed\n");
		return NULL;
	}
    //GV_ZeroMemory(prim2, sizeof(DG_PRIM2)); // < MGS2のソースはこっち。
    GV_ZeroMemory(prim2, size);	// ???? 原因不明不具合一時的回避の為 ???? 

    // 構造体の初期化
    prim2->flag = flag;
    prim2->world = DG_UnitMatrix;
    prim2->group_id = 0;
    prim2->chanl = chanl;
    prim2->type = type;
    prim2->n_prims = n_prims;
    prim2->packet_verts = n_verts;
	prim2->exec_func = NULL;
	
	// パケットは prim2 に続いて確保される
    packet = (DG_PRIM2_PACKET *)&prim2[1];

    // ポインタの設定 (シングルバッファの場合は同じ場所を指す)
	prim2->packet[0] = &packet[n_prims * (0)];
    prim2->packet[1] = &packet[n_prims * (n_buffer - 1)];

    pos = (VECTOR *)&packet[n_prims * (n_buffer)];
    prim2->pos[0] = &pos[n_prims * n_verts * (0)];
    prim2->pos[1] = &pos[n_prims * n_verts * (n_buffer - 1)];

    uvrgb = (void *)&pos[n_prims * n_verts * (n_buffer)];
    prim2->uvrgb[0] = (void *)((int)uvrgb + uvrgb_size * n_prims * n_verts * (0)); 
    prim2->uvrgb[1] = (void *)((int)uvrgb + uvrgb_size * n_prims * n_verts * (n_buffer - 1)); 

    /* パケットデータの初期化 */
    //for (i = 0; i < 2; i ++) {  // オリジナルはこっち
	for (i = 0; i < n_buffer; i ++) {	
        packet = prim2->packet[i];
        pos = prim2->pos[i];
        uvrgb = prim2->uvrgb[i];
        for (j = 0; j < n_prims; j++) {
            packet->prim = prim2;
            packet->type = type;
            packet->flag = packet_flag;
            packet->n_verts = n_verts;
            packet->pos_addr = pos;
            packet->uvrgb_addr = uvrgb;
            uvrgb = (void *)((int)uvrgb + uvrgb_size * n_verts);

			// 頂点バッファ割り当て & 頂点バッファ関連パラメータをセット
			DG_Prim2AllocVertexBuffer(flag, packet);
			packet->MinIndex = 0;
			packet->NumVertices = (u_short)n_verts;
			packet->StartIndex = 0;
			
			packet++;
            pos += n_verts;
		}
    }

    // 描画環境の初期化(テクスチャ以外)
#if 0
	// このあたりは厄介だなぁ・・・
    prim = SCE_GS_SET_PRIM(0,
                           ((flag & DG_PRIM2_SHADE) != 0),
                           ((flag & DG_PRIM2_TEX) != 0),
                           ((flag & DG_PRIM2_FOG) != 0),
                           ((flag & DG_PRIM2_ALPHA) != 0),
                           ((flag & DG_PRIM2_ANTIALIASING) != 0),
                           0, 0, 0);
#endif
#if 0	// 使わない？ M.K
    {
        DG_PRIM2_PARAM *param;
		
        param = &prim2->prim_param[0];
        param->n_verts = n_verts;
        param->flag = 0;
		prim2->prim_param[1] = *param;
    }
#endif	

    /* デフォルトテクスチャ設定(ＤＭＡパケット初期化のため) */
    return prim2;
}

void DG_FreePrim2(DG_PRIM2 *prim)
{
    if (prim == NULL) return;
    GV_DelayedFree(prim);
	// XBOX追加/頂点バッファの Delayed Free
	DG_DelayedFreeVertexBuffer(prim);
}

//-----------------------------------------------------------------------------
       
// プリミティブのテクスチャを設定する

static const DG_TEX_TRANS dg_tex_trans_null = {
	//0, 0, 0, 0,
	{0,0},{0,0},{0,0},{0,0},
	{0.0f, 0.0f, 0.0f, 0.0f},
	{0.0f, 0.0f, 0.0f, 0.0f},
	NULL,
};

void DG_ConfigPrim2Tex(DG_PRIM2 *prim, DG_TEX *tex)
{
    if (prim == NULL || !(prim->flag & DG_PRIM2_TEX)) {
		prim->tex_trans = dg_tex_trans_null;		
	} else {
		prim->tex_trans = tex->tex_trans;
	}
}

//-----------------------------------------------------------------------------

#ifdef DEBUG_MODE

DG_PRIM2 *DG_MakePrim2D(int flag, int n_prims, int n_verts, int chanl, char *fname)
{
    DG_PRIM2 *prim;
    prim = DG_MakePrim2(flag, n_prims, n_verts, chanl);
	if (prim != NULL) {
		prim->fname = fname;
	}
    return prim;
}

//#include "g_macro.h"

// 機能追加型 DG_MakePrim2()
extern "C" DG_PRIM2 *GM_MakePrim2ChanlD(int type, int n_prims, int n_verts, int chanl, char *fname)
{
    DG_PRIM2        *prim;
    //extern int GM_CurrentMap;

    prim = DG_MakePrim2D(type, n_prims, n_verts, chanl, fname);
    if (prim != NULL) {
        if (DG_QueuePrim2(prim) == - 1) {
			// もともとのソースでもコメントアウト
			//DG_FreePrim2(prim);
			//return NULL;
        }
		// こっちはとりあえずコメントアウト
        //if (chanl != DG_CHANL_MENU) GM_GroupPrim2(prim, GM_CurrentMap);
    }
    return prim;
}

// 機能追加型 DG_MakePrim2()   (３Ｄチャンネル固定)
extern "C" DG_PRIM2 *GM_MakePrim2D(int type, int n_prims, int n_verts, char *fname)
{
	return GM_MakePrim2ChanlD(type, n_prims, n_verts, 0, fname);
}
#endif
