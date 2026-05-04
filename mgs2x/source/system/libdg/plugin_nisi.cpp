/*
	plugin_nisi.cpp
	ＤＭＡパケット接続型オブジェクト管理ルーチン

	2001/07/30 NISINO Motoaki
	$Id: plugin_nisi.cpp,v 1.1.1.3 2002/11/19 11:42:19 Yoshizawa1 Exp $

*/

#if 0

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>

#include	"libdg.h"
#include	"libdg.cnf"

#include	"def_dma.h"

#else

#ifdef KP_XBOX
#include <xtl.h>
#else
#include <d3dx8.h>
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <xgraphics.h>

#include "libgv.h"
#include "libdg.h"

#include	"gameheader.h"

#include "dmapack.h"

#include "..\..\user\nisi\test\nisi.h"


#endif


/* ---------------------------------------------------------------- */
	/*
		プラグイン固有設定
	*/

/* メインチャンネルプラグイン用設定 */
	/* プラグイン固有ＩＤ */
#define PLUGIN_ID		(4827819)	/* "dmapack" */
	/* プラグイン初期化フラグ */
#define PLUGIN_FLAG		(DG_PLUGIN_FLAG_OBJBUFFER|DG_PLUGIN_FLAG_ENABLE|DG_PLUGIN_FLAG_ENABLEMENU)
	/* プラグイン実行フェーズ指定 */
#define PLUGIN_PHASE	(DG_PLUGIN_PHASE_FIRST|DG_PLUGIN_PHASE_NORMAL|DG_PLUGIN_PHASE_AFTER|DG_PLUGIN_PHASE_LAST)
	/* プラグインプライオリティ */
#define PLUGIN_PRIO		(DG_PLUGIN_PRIO_NORMAL)
	/* 最大使用オブジェクトキューサイズ */
#define PLUGIN_USE_QUEUE	(64)


#define SAFE_DELETE(p)       { delete (p);     (p)=NULL; }
#define SAFE_DELETE_ARRAY(p) { delete[] (p);   (p)=NULL; }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }


/*
typedef struct _WORK{
	
	DG_OBJS *objs2 ;
	FVECTOR vector2[21];

}WORK;
static WORK work;
*/


struct CUSTOMVERTEX
{
    FLOAT x, y, z, rhw; // The transformed position for the vertex
    DWORD color;        // The vertex color
    float tu,tv;
};
static CUSTOMVERTEX VertTex[10];


struct VERT2D
{
    FLOAT x, y, z, rhw; // The transformed position for the vertex
    DWORD color;        // The vertex color
    //float tu,tv;
};
static VERT2D vert2d[10];




// A structure for our custom vertex type
struct VERTEX3
{
    FLOAT x, y, z;      // The untransformed, 3D position for the vertex
    DWORD color;        // The vertex color
    float tu,tv;
};

// A structure for our custom vertex type
struct VERTEX3_G
{
    FLOAT x, y, z;      // The untransformed, 3D position for the vertex
    DWORD color;        // The vertex color
};



// Our custom FVF, which describes our custom vertex structure
#define D3DFVF_CUSTOMVERTEX3 (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

#define D3DFVF_VERT2D (D3DFVF_XYZRHW|D3DFVF_DIFFUSE)
//#define D3DFVF_VERT2D (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)


#define D3DFVF_VERTEX_G ( D3DFVF_XYZ | D3DFVF_DIFFUSE )


LPDIRECT3DVERTEXBUFFER8 g_pVB        = NULL; // Buffer to hold vertices
LPDIRECT3DVERTEXBUFFER8 g_pVB_g      = NULL; // Buffer to hold vertices

LPDIRECT3DINDEXBUFFER8  g_pIB        = NULL;

LPDIRECT3DTEXTURE8  m_pSpotLightTexture = NULL;   // procedurally generated spotlight texture
LPDIRECT3DTEXTURE8  g_pZStencil		= NULL;   // procedurally generated spotlight texture

//extern CUSTOMVERTEX *VertTest;
extern IDirect3DBaseTexture8* pTexture;
extern DrawTest_cnt;



//extern LPDIRECT3DVERTEXBUFFER8  vb;



/* ---------------------------------------------------------------- */
	/*
		プラグイン固有変数
	*/

	/* プラグイン初期化フラグ */
static int	PluginStartFlag = 0 ;
static DG_PLUGIN	Plugin ;

/* ---------------------------------------------------------------- */
	/*
		プラグイン内部使用関数
	*/

static void PluginActor( DG_CHANL *cp, int which, DG_OBJ_BUFFER *obj_buff, int status );


//-----------------------------------------------------------------------------
// Name: InitGeometry()
// Desc: Creates the scene geometry
//-----------------------------------------------------------------------------


HRESULT InitGeometry()
{
    // Initialize three vertices for rendering a triangle
    //..ふなむし
    VERTEX3 g_Vertices[] =
    {
        { -10.0f, 10.0f, 0.0f, 0xffff0000, 0.0f, 0.0f}, // x, y, z, color tu tv
        {  10.0f, 10.0f, 0.0f, 0xff0000ff, 0.1f, 0.0f},
        {  10.0f,-10.0f, 0.0f, 0xffffffff, 0.1f, 0.1f},
        { -10.0f,-10.0f, 0.0f, 0xffffffff, 0.1f, 0.1f},
    };

    // Create the vertex buffer.
    if( FAILED( g_pd3dDevice->CreateVertexBuffer( 4*sizeof(VERTEX3),
                                                  D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX3,
                                                  D3DPOOL_MANAGED, &g_pVB ) ) )
        return E_FAIL;

    // Fill the vertex buffer.
    VERTEX3* pVertices;
    if( FAILED( g_pVB->Lock( 0, 0, (BYTE**)&pVertices, 0 ) ) )
        return E_FAIL;

    memcpy( pVertices, g_Vertices, 4*sizeof(VERTEX3) );
    g_pVB->Unlock();

    return S_OK;
}


#define MESH_SIZE		10
#define MESH_W			200.0f
#define MESH_H			200.0f

#define CENTER_POS_X	(MESH_SIZE * MESH_W / 2.0f - MESH_W/2.0f)
#define CENTER_POS_Z	(MESH_SIZE * MESH_H / 2.0f - MESH_H/2.0f)



#define NUM_INDICES		(4 * (MESH_SIZE - 1) * (MESH_SIZE - 1))



HRESULT InitGround(void){
	
	int i,j,col_f=0;;
    // Create the vertex buffer.
    if( FAILED( g_pd3dDevice->CreateVertexBuffer(
		(2 * MESH_SIZE + 2 * MESH_SIZE) * sizeof(VERTEX3_G),	0, 0, 0, &g_pVB_g ) ) 
	)
	ASSERT(0);
	
    // Fill the vertex buffer.
    VERTEX3_G* pVert;
    if( FAILED( g_pVB_g->Lock( 0, 0, (BYTE**)&pVert, 0 ) ) )
        return E_FAIL;
	
	
	for(j = 0; j < MESH_SIZE; j++){
		pVert->x = 0.0f - CENTER_POS_X ;
		pVert->y = 0.0f;
		pVert->z = j * MESH_H - CENTER_POS_Z;
		pVert->color = 0xffffffff;
		pVert++;
		col_f++;
		pVert->x = (MESH_SIZE-1) * MESH_W - CENTER_POS_X;
		pVert->y = 0.0f;
		pVert->z = j * MESH_H - CENTER_POS_Z;
		pVert->color = 0xff000000;
		pVert++;
		col_f++;
	}
	for(i = 0; i < MESH_SIZE; i++){
		pVert->x = i * MESH_W - CENTER_POS_X;
		pVert->y = 0.0f;
		pVert->z = 0.0f - CENTER_POS_Z;
		pVert->color = 0xffffffff;
		pVert++;
		col_f++;
		pVert->x = i * MESH_W - CENTER_POS_X;
		pVert->y = 0.0f;
		pVert->z = (MESH_SIZE-1) * MESH_H - CENTER_POS_Z;
		pVert->color = 0xff000000;
		pVert++;
		col_f++;
	}
	g_pVB_g->Unlock();
	
	
	dprint(col_f);

	/*	
	//..インデックスバッファ作成
	WORD* pIndices;
	if( FAILED( g_pd3dDevice->CreateIndexBuffer( 
		NUM_INDICES * sizeof(WORD),
		0, 
		D3DFMT_INDEX16,		//..無視される
		D3DPOOL_DEFAULT, 	//..無視される
		&g_pIB ) ) 
	)	ASSERT(0);
	
	

	g_pIB->Lock( 0, NUM_INDICES * sizeof(WORD), (BYTE**)&pIndices, 0 );

	for( DWORD y=0; y<MESH_SIZE-1; y++ ){
		for( DWORD x=0; x<MESH_SIZE-1; x++ ){
			*pIndices++ = (WORD)( (x+0) + (y+0)*MESH_SIZE );
			*pIndices++ = (WORD)( (x+1) + (y+0)*MESH_SIZE );
			*pIndices++ = (WORD)( (x+1) + (y+1)*MESH_SIZE );
			*pIndices++ = (WORD)( (x+0) + (y+1)*MESH_SIZE );
			
			dprint((x+0) + (y+0)*MESH_SIZE );
			dprint((x+1) + (y+0)*MESH_SIZE );
			dprint((x+1) + (y+1)*MESH_SIZE );
			dprint((x+0) + (y+1)*MESH_SIZE );
		}
	}
	g_pIB->Unlock();
	*/
	
}



//-----------------------------------------------------------------------------
// Name: SetupMatrices()
// Desc: Sets up the world, view, and projection transform matrices.
//-----------------------------------------------------------------------------
VOID SetupMatrices()
{
    D3DXMATRIX matScale;
    //D3DXMatrixScaling( &matScale, 2.0f, 2.0f, 2.0f );


    // For our world matrix, we will just rotate the object about the y-axis.
    D3DXMATRIX matWorld;
    D3DXMatrixRotationY( &matWorld, timeGetTime()/2000.0f );

    //D3DXMatrixMultiply( &matWorld, &matWorld, &matScale );
        
    g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

    D3DXMATRIX matView;


    D3DXMatrixLookAtLH( &matView, &D3DXVECTOR3( 0.0f, 500.0f,2000.0f ),
                                  &D3DXVECTOR3( 0.0f, 0.0f, 0.0f ),
                                  &D3DXVECTOR3( 0.0f, 1.0f, 0.0f ) );

    g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, 1.0f, 1.0f, 10000.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );
}

D3DXMATRIX matWorld_w;
D3DXMATRIX matView_w;
D3DXMATRIX matProj_w;

VOID GetMatrices()
{
    g_pd3dDevice->GetTransform( D3DTS_WORLD, &matWorld_w );
    g_pd3dDevice->GetTransform( D3DTS_VIEW, &matView_w );
    g_pd3dDevice->GetTransform( D3DTS_PROJECTION, &matProj_w );
}

VOID ResetMatrices()
{
    g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld_w );
    g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView_w );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj_w );
}




//-----------------------------------------------------------------------------
// Name: XBUtil_SwizzleTexture2D()
// Desc: Swizzles a 2D texture before it gets unlocked. Note: this operation
//       can be very slow.
//-----------------------------------------------------------------------------
VOID XBUtil_SwizzleTexture2D( D3DLOCKED_RECT* pLock, const D3DSURFACE_DESC* pDesc )
{
    DWORD dwPixelSize   = XGBytesPerPixelFromFormat( pDesc->Format );
    DWORD dwTextureSize = pDesc->Width * pDesc->Height * dwPixelSize;

    BYTE* pSrcBits = new BYTE[ dwTextureSize ];
    memcpy( pSrcBits, pLock->pBits, dwTextureSize );
    
    XGSwizzleRect( pSrcBits, 0, NULL, pLock->pBits,
                  pDesc->Width, pDesc->Height, 
                  NULL, dwPixelSize );

    SAFE_DELETE_ARRAY( pSrcBits );
}


//-----------------------------------------------------------------------------
// Name: CreateSpotLightTexture()
// Desc: 
//-----------------------------------------------------------------------------

#define TEXTURE_W	256
#define TEXTURE_H	256


HRESULT CreateSpotLightTexture( BOOL bBorder )
{
    // Release the old texture
    //SAFE_RELEASE( m_pSpotLightTexture );

    // Create a new texture
	if( FAILED(g_pd3dDevice->CreateTexture( 
		TEXTURE_W, 
		TEXTURE_H, 
		1, 
		D3DUSAGE_RENDERTARGET, 		// Usage (無視)
		D3DFMT_A8R8G8B8, 
		D3DPOOL_DEFAULT, 
		&m_pSpotLightTexture 
	))){
		HANGUP();
	}

    D3DLOCKED_RECT lock;
    m_pSpotLightTexture->LockRect( 0, &lock, NULL, 0L );

    D3DSURFACE_DESC desc;
    m_pSpotLightTexture->GetLevelDesc( 0, &desc );
    DWORD* pBits    = (DWORD*)lock.pBits;
    DWORD* pLine    = (DWORD*)lock.pBits;
    DWORD  dwStride = lock.Pitch / sizeof(DWORD);


    if( bBorder )
    {
        // Draw border around spotlight texture
        DWORD* pLineX1 = (DWORD*)pBits + dwStride;
        DWORD* pLineX2 = (DWORD*)pBits + dwStride * (128 - 2);
        DWORD* pLineY1 = (DWORD*)pBits + 1;
        DWORD* pLineY2 = (DWORD*)pBits + dwStride - 2;

        for( DWORD x = 0; x < 128; x++ )
        {
            if( x != 0 && x != 127 )
            {
                *pLineX1 = *pLineX2 = 0xffffffff;
                *pLineY1 = *pLineY2 = 0xffffffff;
            }

            pLineX1 += 1;
            pLineX2 += 1;
            pLineY1 += dwStride;
            pLineY2 += dwStride;
        }
    }

    // Swizzle and unlock the texture
    //XBUtil_SwizzleTexture2D( &lock, &desc );
    m_pSpotLightTexture->UnlockRect(0);

    return S_OK;
}

HRESULT CreateZStencilBuffer( void )
{

    // Create a new texture
	if( FAILED(g_pd3dDevice->CreateTexture( 
	//if( FAILED(g_pd3dDevice->CreateDepthStencilSurface( 
		TEXTURE_W,
		TEXTURE_H,
		1,
		D3DUSAGE_DEPTHSTENCIL ,		// Usage (無視)
		D3DFMT_LIN_D24S8, 
		D3DPOOL_DEFAULT, 
		&g_pZStencil
	))){
		HANGUP();
	}

#if 0
    D3DLOCKED_RECT lock;
    g_pZStencil->LockRect( 0, &lock, NULL, 0L );

    D3DSURFACE_DESC desc;
    g_pZStencil->GetLevelDesc( 0, &desc );
    DWORD* pBits    = (DWORD*)lock.pBits;
    DWORD* pLine    = (DWORD*)lock.pBits;
    DWORD  dwStride = lock.Pitch / sizeof(DWORD);


    if( bBorder )
    {
        // Draw border around spotlight texture
        DWORD* pLineX1 = (DWORD*)pBits + dwStride;
        DWORD* pLineX2 = (DWORD*)pBits + dwStride * (128 - 2);
        DWORD* pLineY1 = (DWORD*)pBits + 1;
        DWORD* pLineY2 = (DWORD*)pBits + dwStride - 2;

        for( DWORD x = 0; x < 128; x++ )
        {
            if( x != 0 && x != 127 )
            {
                *pLineX1 = *pLineX2 = 0xffffffff;
                *pLineY1 = *pLineY2 = 0xffffffff;
            }

            pLineX1 += 1;
            pLineX2 += 1;
            pLineY1 += dwStride;
            pLineY2 += dwStride;
        }
    }

    // Swizzle and unlock the texture
    XBUtil_SwizzleTexture2D( &lock, &desc );
    g_pZStencil->UnlockRect(0);
#endif
    return S_OK;
}




/* ---------------------------------------------------------------- */
	/*
		プラグイン起動
	*/


#define TEX_X		64
#define TEX_Y		64
#define TEX_W		128
#define TEX_H		128
	
void Plugin_nisi(void)
{
	int i;

	for(i=0; i<4; i++){
		VertTex[i].rhw = 1.0f;
		VertTex[i].color = D3DCOLOR_RGBA(255,255,255,255);
	}
	
	VertTex[0].x = TEX_X;
	VertTex[0].y = TEX_Y;
	VertTex[0].z = 0.5f;
	VertTex[1].x = TEX_X + TEX_W;
	VertTex[1].y = TEX_Y;
	VertTex[1].z = 0.5f;
	VertTex[2].x = TEX_X + TEX_W;
	VertTex[2].y = TEX_Y + TEX_H;
	VertTex[2].z = 0.5f;
	VertTex[3].x = TEX_X ;
	VertTex[3].y = TEX_Y + TEX_H;
	VertTex[3].z = 0.5f;

	VertTex[0].tu = 0.0f;
	VertTex[0].tv = 0.0f;
	VertTex[1].tu = 1.0f;
	VertTex[1].tv = 0.0f;
	VertTex[2].tu = 1.0f;
	VertTex[2].tv = 1.0f;
	VertTex[3].tu = 0.0f;
	VertTex[3].tv = 1.0f;
	
	CreateSpotLightTexture( 1 );
	CreateZStencilBuffer();
	

	for(i=0; i<4; i++){
		vert2d[i].rhw = 1.0f;
		vert2d[i].color = D3DCOLOR_RGBA(255,255,255,255);
	}
	
	vert2d[0].x = TEX_X;
	vert2d[0].y = TEX_Y;
	vert2d[0].z = 0.5f;
	vert2d[1].x = TEX_X + TEX_W;
	vert2d[1].y = TEX_Y;
	vert2d[1].z = 0.5f;
	vert2d[2].x = TEX_X + TEX_W;
	vert2d[2].y = TEX_Y + TEX_H;
	vert2d[2].z = 0.5f;
	vert2d[3].x = TEX_X ;
	vert2d[3].y = TEX_Y + TEX_H;
	vert2d[3].z = 0.5f;

/*
	vert2d[0].tu = 0.0f;
	vert2d[0].tv = 0.0f;
	vert2d[1].tu = 1.0f;
	vert2d[1].tv = 0.0f;
	vert2d[2].tu = 1.0f;
	vert2d[2].tv = 1.0f;
	vert2d[3].tu = 0.0f;
	vert2d[3].tv = 1.0f;
*/
	
	InitGeometry();
	InitGround();

	if ( PluginStartFlag != 0 ) return ;

	/* プラグイン作成 */
	DG_MakePlugin( 
		&Plugin, PLUGIN_ID, PLUGIN_FLAG, PLUGIN_PHASE, 
		PLUGIN_PRIO, PluginActor, PLUGIN_USE_QUEUE
	);
	
	DG_AddPlugin( &Plugin );

	PluginStartFlag = 1 ;
	DG_PrivilegeMode = 0 ;
	
}

	/*
		プラグイン終了
	*/
void DG_DeletePlugin_nisi( void )
{
	while ( PluginStartFlag != 0 ){
		DG_DeletePlugin( &Plugin );
		DG_FreePlugin( &Plugin );
	}
	PluginStartFlag = 0 ;
}





/* ---------------------------------------------------------------- */
	/*
		プラグイン実行アクター
		暫定版
	*/

static void PluginActor( DG_CHANL *cp, int which, DG_OBJ_BUFFER *obj_buff, int status )
{
	LPDIRECT3DSURFACE8 pBackSurface;
	LPDIRECT3DSURFACE8 pBackDepthStencil;
	LPDIRECT3DSURFACE8 pTargetSurface;
	LPDIRECT3DSURFACE8 pTargetDepthStencil;

	// バックバッファの情報を保存
	g_pd3dDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pBackSurface );
	g_pd3dDevice->GetDepthStencilSurface( &pBackDepthStencil );
	
	// レンダリングターゲットの変更
	m_pSpotLightTexture->GetSurfaceLevel( 0, &pTargetSurface );	//..テクスチャ
	g_pZStencil->GetSurfaceLevel( 0, &pTargetDepthStencil );
	
	//..						 (pRenderTarget,  pNewZStencil) 
	g_pd3dDevice->SetRenderTarget( pTargetSurface, pTargetDepthStencil );
	pTargetSurface->Release(); // 使い終わったらReleaseする
	pTargetDepthStencil->Release();

	//..Z値更新あり
	DG_SetRenderState(D3DRS_ZENABLE, TRUE);
	DG_SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	DG_SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL );	

	// Z値更新無し
	//DG_SetRenderState(D3DRS_ZENABLE, FALSE);
	//DG_SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	//DG_SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);

    // Turn off culling, so we see the front and back of the triangle
    g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

    // Turn off D3D lighting, since we are providing our own vertex colors
    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

	//..現在のワールド、カメラ座標取得
	GetMatrices();
	
	DWORD vertex_shader;
	g_pd3dDevice->GetVertexShader(&vertex_shader);
	
	//----------------------------------------------------------


    // Begin the scene
    g_pd3dDevice->BeginScene();

    // Clear the backbuffer to a black color
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL , 
                         D3DCOLOR_XRGB(0x40,0x40,0x40), 1.0f, 0 );


	//..テクスチャレンダリングターゲット用行列をセット
    SetupMatrices();

#if 0
	//..2001/10/11	
	{
	//..テクスチャつき3Dプリミティブ描画    
	//..テクスチャ変更	ふなむし
	DG_TEX	*tex=NULL;
	tex = DG_GetTexture( GV_StrCode( "funa_alp_ovl" ) );

	if(tex != NULL){
		g_pd3dDevice->SetTexture(0,tex->tex_trans.ptex);
		VERTEX3* pVertices;
		if( FAILED( g_pVB->Lock( 0, 0, (BYTE**)&pVertices, 0 ) ) )
	        return ;
		
		pVertices[0].tu = tex->u_offset;
		pVertices[0].tv = tex->v_offset;
		pVertices[1].tu = tex->u_offset + tex->u_scale ;
		pVertices[1].tv = tex->v_offset;
		pVertices[2].tu = tex->u_offset + tex->u_scale ;
		pVertices[2].tv = tex->v_offset + tex->v_scale ;
		pVertices[3].tu = tex->u_offset;
		pVertices[3].tv = tex->v_offset + tex->v_scale ;
		
	    //memcpy( pVertices, g_Vertices, 4*sizeof(CUSTOMVERTEX) );
	    g_pVB->Unlock();	
	}
	
	g_pd3dDevice->SetStreamSource( 0, g_pVB, sizeof(VERTEX3) );
	g_pd3dDevice->SetVertexShader( D3DFVF_CUSTOMVERTEX3 );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, 2 );
	}
#endif

	//..地面
	g_pd3dDevice->SetTexture(0,NULL);
    g_pd3dDevice->SetStreamSource( 0, g_pVB_g, sizeof(VERTEX3_G) );
	g_pd3dDevice->SetVertexShader( D3DFVF_VERTEX_G );
	
	g_pd3dDevice->DrawPrimitive( D3DPT_LINELIST, 0, (4 * MESH_SIZE )/2 );

/*	
	g_pd3dDevice->SetIndices( g_pIB, 0 );
	g_pd3dDevice->DrawIndexedPrimitive( 
										D3DPT_LINESTRIP,
										//D3DPT_QUADLIST, 
										0, 
										NUM_INDICES-1, 
										0, 
										NUM_INDICES-1 );
										//NUM_INDICES/4 );
*/


#if 1
	//..モデルオブジェクト
	extern DG_OBJS *objs2 ;
	extern FVECTOR vector2[21];

	if(objs2!=NULL)
	{

		//printf("TEST\n");
		
		DG_OBJS *objs;	//..引数
		
		DG_OBJ        *obj;
		DG_OBJ_PACKET *pkt;
		int           i, j;
		DG_VBUF_INFO  *info;

		DG_INDEX_INFO *index;

		objs=objs2;

		obj = objs->objs;
		pkt = obj->packets;
		//info = pkt->vbuf_info;
		index = pkt->index;
		info = pkt->index->vbuf_info;
		
		DG_SetStreamSource(0, info->vbuf, info->stride);
		DG_SetVertexShader(info->fvf);
		DG_SetIndices(info->ibuf, 0);
		DG_SetTexture(0, pkt->ptex); 	// TODO: マルチテクスチャ管理
		//DG_SetTexture(0, NULL);
		for (i = objs->n_models; i > 0; i--, obj++) {	
			DG_SetTransform(D3DTS_WORLD, &obj->world);
			if (obj->parent != - 1) {
				DG_SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_1WEIGHTS);
				DG_SetTransform(D3DTS_WORLD1, &obj->inv_mat);
			} else {
				DG_SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
			}
			pkt = obj->packets;
			
			//..描画
			for (j = obj->n_packs; j > 0; j --, pkt++, index++) {
				DG_DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,
										index->min,
										index->num,
										index->start,
										index->prims);
			}
		}
	}
#endif


#if 0	
	//..2Dプリミティブ
	//..SetTexture(0,NULL)にしないとうつらない
	g_pd3dDevice->SetTexture(0,NULL);
	g_pd3dDevice->SetVertexShader( D3DFVF_VERT2D );
	g_pd3dDevice->DrawPrimitiveUP( 
	    	D3DPT_TRIANGLEFAN, 2,vert2d, sizeof(VERT2D)  );
#endif
	
	DG_EndScene();

	//----------------------------------------------------------
	
    
	//..元に戻す
	g_pd3dDevice->SetVertexShader(vertex_shader);	
	ResetMatrices();
	
	
	// バックバッファに戻す
	DG_SetRenderTarget(pBackSurface, pBackDepthStencil);
	pBackSurface->Release();
	pBackDepthStencil->Release();

	// Z値更新有りに戻す
	DG_SetRenderState(D3DRS_ZENABLE, TRUE);
	DG_SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	DG_SetRenderState(D3DRS_ZFUNC, D3DCMP_GREATEREQUAL);	
	//DG_SetTexture(0, DG_ShadowTexture);
	//g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
	
	//..レンダリングされたターゲットのテクスチャを描画
	if(m_pSpotLightTexture){
		g_pd3dDevice->SetTexture(0,m_pSpotLightTexture);
		g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		g_pd3dDevice->DrawPrimitiveUP( 
		   	D3DPT_TRIANGLEFAN, 2,VertTex, sizeof(CUSTOMVERTEX)  );
	}

}




/* ---------------------------------------------------------------- */

