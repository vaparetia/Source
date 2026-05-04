//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#include <stdio.h>

#ifdef KP_XBOX
//void *NewSTG_SmokeBlurEffect() { return NULL ; }
void *HZX_OnlineDebugFlag() { return NULL ; }/* ? */
void *NewVramInitialize(){ return NULL; }/* ? */
void *NewScreenShotControl() { return ( void * )1 ; }/* ? */
void *DG_OnePieceSkip() { return NULL ; }/* デバック用関数 */
/* mpeg */
#if 0
void *NewEnding() {
	if( GCL_GetOption( 'p' ) ){
		void *command;
		if ( (command = ( void * )GCL_GetNextInt()) ) {
			return UTL_DelayedExecCommand( command, NULL, 2 ) ;
		}
	}
	return NULL ;
}/* mpeg */
void *NewMpegPssMovieStr() {/* mpeg */
	if( GCL_GetOption( 'p' ) ){
		void *command;
		if ( (command = ( void * )GCL_GetNextInt()) ) {
			return UTL_DelayedExecCommand( command, NULL, 2 ) ;
		}
	}
	return NULL ;
}
#endif

//void *NewIceEffect() { return NULL ; }
//void *C4MAN_GetMap() { return NULL ; }
//void *C4MAN_GetObjs() { return NULL ; }
//void *C4MAN_GetWorld() { return NULL ; }
//void *NewUSPLight() { return NULL ; }
#endif



#ifdef KP_XBOX

/* 使っているところは、修正必要!! */
void *DG_MakeLoadImagePacket( void *tag_addr, int fmt, int width, int height, int vram_addr, int vram_width, void *tex_addr ) { return tag_addr ; } ;/* 使っているところは、修正必要!! */
void *FlushCache() { return NULL ; }/* 使っているところは、修正必要!! */
//void *DG_FreeMoveReplacePacket() { return NULL ; }/* 使っているところは、修正必要!! */
//void *DG_ResetMoveReplaceTexture() { return NULL ; }/* 使っているところは、修正必要!! */
//void *DG_SetMoveReplaceTexture() { return NULL ; }/* 使っているところは、修正必要!! */
//void *DG_MakeMoveReplacePacket() { return NULL ; }/* 使っているところは、修正必要!! */
int DG_GIFTAG_MENU_SPRITE ;/* 使っているところは、修正必要!! */
int DG_GIFTAG_MENU_LINE ;
#endif

#if 1 //BP_RENDER (normally defined by including prim.c in project)
sceGifTag	DG_GIFTAG_MENU_SPRITE = {
   0,1,0/*pad*/,0,1,SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, 0, 1, 0, 0, 0, 1, 0, 0),SCE_GIF_REGLIST,6,
   GS_REGS_PRIM,GS_REGS_RGBA,GS_REGS_UV,GS_REGS_XYZF2,GS_REGS_UV,GS_REGS_XYZF2,
   0,0,0,0,0,0,0,0,0,0
};
#endif
