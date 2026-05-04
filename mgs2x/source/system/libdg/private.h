/*
  private.h
  libdg 内部使用ヘッダ
  
  2001/03/22 F.Miyauchi
  $Id: private.h,v 1.1.1.3 2002/11/19 11:42:21 Yoshizawa1 Exp $

*/

#ifndef __PRIVATE_H__
#define __PRIVATE_H__

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// libdg内部使用関数プロトタイプ
//-----------------------------------------------------------------------------

// 2d_prim.cpp
void DG_Init2DPrim(void);
void DG_Draw2DPrim(void);
void DG_Reset2DPrim(void);
void DG_Release2DPrim(void);	
	
// chanl.cpp

// evmobjs.cpp

// light.cpp
void DG_DisableDxLight(void);
void DG_EnableDxLight(void);
	
//prim2.cpp
void DG_InitAlphaMode(void);	
void DG_AlphaBlendDisable(void);

//shdwchin.cpp
void DG_InitShadowVertexBuffer(void);
void DG_ReleaseShadowVertexBuffer(void);

#ifdef __cplusplus
}
#endif

//shadowvl.cpp
extern void DG_ShadowVolChanl(DG_CHANL *cp, int which);


#endif // !__PRIVATE_H__

//-----------------------------------------------------------------------------
