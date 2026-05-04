#ifndef _mobile_panel_h_
#define _mobile_panel_h_

#include "BP_BuildDefines.h"
#include "BP_Font.h"

/*
 * 転送端末用テキストパネル
 */
typedef struct MOBILE_MESG_PANEL {
#if BP_USE_NEW_FONT_SYSTEM()
   BP_FONT_VRAMINFO bp_vinfo;
   unsigned int      bp_tex;

#else
   FONT_VRAMINFO vinfo;
   void        * vram;
   int           vramsize;
   DG_TEX_LIN *tex;
   int clut[ 16 ];
#endif

   int           width, height;

   DG_DMAPACK  * dmapack;

   int           panel_dx;
   int           panel_dy;
   int           panel_w;
   int           panel_h;

   void				*packet_mem;
   void                *prim ;
} MOBILE_MESG_PANEL;




#ifndef _mobile_panel_c_
#define EXT  extern
#else
#define EXT
#endif  /* _mobile_panel_c_ */



#undef EXT



void MOBILE_OpenMesgPanel( MOBILE_MESG_PANEL *panel, int width, int height );
void MOBILE_CloseMesgPanel( MOBILE_MESG_PANEL *panel );

void MOBILE_DrawMessage( MOBILE_MESG_PANEL *panel, char *mesg );
void MOBILE_DrawMesgPanel( MOBILE_MESG_PANEL *panel, int x, int y, int w, int h );   // center


#endif /* _mobile_panel_h_ */
