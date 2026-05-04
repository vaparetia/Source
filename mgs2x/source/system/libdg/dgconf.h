/*
	libdg.cnf
	£Ä£Ç¥é¥¤¥Ö¥é¥êÆâÉô¥³¥ó¥Õ¥£¥®¥å¥ì¡¼¥·¥ç¥ó

	1999/07/07 K.Takabe
	$Id: libdg.cnf,v 1.1.1.3 2002/11/19 11:42:09 Yoshizawa1 Exp $

*/

#ifndef __LIBDG_CNF__
#define __LIBDG_CNF__

/*
	£Ä£Ç¥é¥¤¥Ö¥é¥êÆâÉô¥³¥ó¥Õ¥£¥®¥å¥ì¡¼¥·¥ç¥ó

	¡Ê³°ÉôÄê¿ô¤Ë¤Ä¤¤¤Æ¤Ï libdg.h ¤ò»²¾È¡Ë
*/

/*----------------------------------------------------------------*/

	/*
		¥Ç¥Õ¥©¥ë¥È¥Õ¥ì¡¼¥à¥ì¡¼¥È
	*/
#define	FRAME_RATE	(1)

	/*
		¼Â¸³ÍÑ²èÌÌ¥â¡¼¥É
	*/
//#define HALFRESO_NI		/* 32bit 512x224¥Î¥ó¥¤¥ó¥¿¡¼¥ì¡¼¥¹ */
//#define HALFRESO_FFI		/* 32bit 512x224¥Õ¥ê¥Ã¥«¡¼¥Õ¥ê¡¼ */
//#define HIGHRESO_I		/* 32bit 512x224(448) */
#define HIGHRESO_FFI		/* 32bit 512x448¥Õ¥ê¥Ã¥«¡¼¥Õ¥ê¡¼ */

#define BP_HEIGHT_ADJUST_FACTOR  2
	/*
		ÉÁ²èÎÎ°è
	*/
#define	BUFFER_WIDTH	(512)			/* ³ÎÊÝ¤¹¤ë¥Ð¥Ã¥Õ¥¡¤ÎÉý */
#define	BUFFER_HEIGHT	(256*BP_HEIGHT_ADJUST_FACTOR)			/* ³ÎÊÝ¤¹¤ë¥Ð¥Ã¥Õ¥¡¤Î¹â¤µ */
#define	DRAW_DEPTH		(24)			/* £Ú¥Ð¥Ã¥Õ¥¡¤Î¥Ó¥Ã¥È¿ô */
#define DRAW_Z_MIN		(4096)			/* £ÚÃÍºÇ¾® */
#define DRAW_Z_MAX		(16777216-4096)	/* £ÚÃÍºÇÂç */

extern int BP_BASE_TICK();
#ifndef PAL
   #define	DRAW_WIDTH		(512)			/* É½¼¨ÎÎ°è */
   #define	DRAW_HEIGHT		(224*BP_HEIGHT_ADJUST_FACTOR)			/* É½¼¨ÎÎ°è */
   #define PIXEL_ASPECT	(1.121f)		/* ¥Ô¥¯¥»¥ë¥¢¥¹¥Ú¥¯¥È */
   #define BASE_TICK		(BP_BASE_TICK())				/* NTSC 1/300¤Î»þ¤Îtick */
#else
   #define	DRAW_WIDTH		(512)			/* É½¼¨ÎÎ°è */
   #define	DRAW_HEIGHT		(256*BP_HEIGHT_ADJUST_FACTOR)			/* É½¼¨ÎÎ°è */
   #define PIXEL_ASPECT	(1.281f)		/* ¥Ô¥¯¥»¥ë¥¢¥¹¥Ú¥¯¥È */
   #define BASE_TICK		(BP_BASE_TICK())				/* PAL  1/300¤Î»þ¤Îtick */
#endif

#define DRAW_CENTER_X	(2048)			/* ²èÌÌÃæ¿´¤ÎºÂÉ¸ */
#define DRAW_CENTER_Y	(2048)			/* ²èÌÌÃæ¿´¤ÎºÂÉ¸ */

	/*
		£Ú¥¯¥ê¥Ã¥×ÎÎ°è¡Ê¸«¤¨¤ëÎÎ°è¡Ë
	*/
#define DRAW_NEAR_CLIP	(50.0F)
#define DRAW_FAR_CLIP	(65536.0F*20.0F )

#ifdef HIGHRESO_FFI
   #undef  DRAW_HEIGHT
   #undef  BUFFER_HEIGHT

   #ifndef PAL
      #define	DRAW_HEIGHT		(224*2)			/* É½¼¨ÎÎ°è */
      #define	BUFFER_HEIGHT	(256*BP_HEIGHT_ADJUST_FACTOR)			/* É½¼¨ÎÎ°è */
   #else
      #define	DRAW_HEIGHT		(256*2)			/* É½¼¨ÎÎ°è */
      #define	BUFFER_HEIGHT	(256*BP_HEIGHT_ADJUST_FACTOR)			/* É½¼¨ÎÎ°è */
   #endif
#endif

#define DRAW_Z_SCALE	( ( DRAW_Z_MAX - DRAW_Z_MIN ) / 2.0f )
#define DRAW_Z_OFFSET	( DRAW_Z_MIN + ( DRAW_Z_MAX - DRAW_Z_MIN ) / 2.0f )

	/*
		É½¼¨ÎÎ°è
	*/
#ifndef PAL
   #define	DISPLAY_WIDTH	(512)
   #define	DISPLAY_HEIGHT	(240*BP_HEIGHT_ADJUST_FACTOR)
   #define	DISPLAY_SHIFT	(512)
   #define	DISPLAY_CUTOFF	(DISPLAY_HEIGHT-DRAW_HEIGHT)
   #define	ASPECT_RATIO	58.0F/64.0F						/* 50/55 */
#else
   #define	DISPLAY_WIDTH	(512)
   #define	DISPLAY_HEIGHT	(256*BP_HEIGHT_ADJUST_FACTOR)
   #define	DISPLAY_SHIFT	(512)
   #define	DISPLAY_CUTOFF	(DISPLAY_HEIGHT-DRAW_HEIGHT)
   #define	ASPECT_RATIO	66.0F/64.0F						/* 50/55 */
#endif


	/*
		£Ð£Ó£²²èÌÌ¥â¡¼¥É´ØÏ¢
	*/

extern float BP_GetScreenAspectX();
extern float BP_GetScreenAspectY();

#define TARGET_ASPECT_X       (0.75f)

#define ASPECT_X() BP_GetScreenAspectX()
#define ASPECT_Y() BP_GetScreenAspectY()

#ifndef HIGHRESO_FFI
   /* ÄÌ¾ï¹â²òÁüÅÙ¡Ê£³£²¥Ó¥Ã¥È¥«¥é¡¼¡Ë */

   #define COLOR_DEPTH		(32)
   #define BUFFER_PAGE(_p)	( BUFFER_WIDTH*BUFFER_HEIGHT*COLOR_DEPTH/32*(_p) )
   #define ZBUFFER_PAGE()	( BUFFER_PAGE(3) )
   #define FRAME_BUFFER_COLOR_MODE()	( SCE_GS_PSMCT32 )
   #define Z_BUFFER_COLOR_MODE()		( SCE_GS_PSMZ24 )
   #define TEXTURE_TOP_PAGE()	( BUFFER_PAGE(4) )
#else
   /* ¥Õ¥ê¥Ã¥«¡¼¥Õ¥ê¡¼¹â²òÁüÅÙ¡Ê£³£²¥Ó¥Ã¥È¥«¥é¡¼¡Ë */

   #define COLOR_DEPTH		(32)
   #define BUFFER_PAGE(_p)	( BUFFER_WIDTH*BUFFER_HEIGHT*COLOR_DEPTH*2/32*(_p) )
   #define ZBUFFER_PAGE()	( BUFFER_PAGE(3) )
   #define FRAME_BUFFER_COLOR_MODE()	( SCE_GS_PSMCT32 )
   #define Z_BUFFER_COLOR_MODE()		( SCE_GS_PSMZ24 )
   #define TEXTURE_TOP_PAGE()	( BUFFER_PAGE(2) )
#endif

/*----------------------------------------------------------------*/

	/*
		¥Á¥ã¥ó¥Í¥ë¥­¥å¡¼¥µ¥¤¥º
	*/
#define	CHANL_QUEUE	(8)
#define	CHANL0_QUEUE	(256)
#define	CHANL1_QUEUE	(0)



/*----------------------------------------------------------------*/


/*----------------------------------------------------------------*/


/*----------------------------------------------------------------*/

#endif
