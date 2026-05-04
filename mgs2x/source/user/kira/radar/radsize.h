/**********
***********/

/*
   仮想座標系(x:512 y:384)
*/
/*なし*/

/*
   実座標系(PS2::x:512 y:448 / PS2_PAL::x:512 y:512 / XBOX::x:640 y:448)
*/
/* 実座標から2Dsprite系の仮想座標への変換スケール */




#define SC_RATE_W			(512.0f/DRAW_WIDTH)
#define SC_RATE_H			(384.0f/DRAW_HEIGHT)
//#define RADAR_WINDOW_W		((int)(120.0f*((SC_RATE_W/SC_RATE_H)/(1280.0f/720.0f))))          // adjust radar_window_w back into 4:3 ratio. only needed for the width
#define RADAR_WINDOW_W		(((int)(120.0f*TARGET_ASPECT_X))+1)          // adjust radar_window_w back into 4:3 ratio. only needed for the width


#define RADAR_WINDOW_H		((int)(105*DRAW_HEIGHT/448))
#define TOP_SIZE_H_SC		((int)(17*DRAW_HEIGHT/448))

/* 念のため互換性を持たせる */
#if defined(PSX2) && !defined(PAL)
//#undef RADAR_WINDOW_H
//#define RADAR_WINDOW_H		(104)	/* 本来なら105になる */
#endif


#if 0
#ifndef PAL
#define RADAR_WINDOW_H		104
#define SC_RATE_H		(0.8571F)
#define TOP_SIZE_H_SC	(17)
#else
#define RADAR_WINDOW_H		120
#define SC_RATE_H		(0.75F)
#define TOP_SIZE_H_SC	(18)
#endif
#endif

#ifdef PSX2
#define RADAR_OFFSET_X ((int)(39*DRAW_WIDTH/512))	//(16)
#define RADAR_OFFSET_Y ((int)(38*DRAW_HEIGHT/448))	//(12)
#else
#define RADAR_OFFSET_X ((int)(16*DRAW_WIDTH/512))	//(16)
#define RADAR_OFFSET_Y ((int)(20*DRAW_HEIGHT/448))	//(12)
#endif

#define TOP_SIZE_H (14)

#define BOTOM_SIZE_H (12.0F)
#define BOTOM_SIZE_H_LEVEL ((BOTOM_SIZE_H-SC_RATE_H)+0.5f)

#define RADAR_WINDOW_X0		(DRAW_WIDTH - RADAR_OFFSET_X - RADAR_WINDOW_W )
#define RADAR_WINDOW_X1		(DRAW_WIDTH - RADAR_OFFSET_X)
#define RADAR_WINDOW_Y0		(RADAR_OFFSET_Y+TOP_SIZE_H_SC)
#define RADAR_WINDOW_Y1		(RADAR_WINDOW_Y0 + RADAR_WINDOW_H)

// by koba4
#define SK_RADAR_WINDOW_W ( RADAR_WINDOW_W )
#define SK_RADAR_WINDOW_H ( RADAR_WINDOW_H )


#if 0
----------------------------------------------
座標系が入り混じっていたので、、、、
2002.04.09yano
----------------------------------------------

#define RADAR_WINDOW_W		120

//256/224= 1.142857
#ifndef PAL
#define RADAR_WINDOW_H		104
#define SC_RATE_H		(0.8571F)
#define TOP_SIZE_H_SC	(17)
#else
#define RADAR_WINDOW_H		120
#define SC_RATE_H		(0.75F)
#define TOP_SIZE_H_SC	(18)
#endif

#define RADAR_OFFSET_X (16)
#define RADAR_OFFSET_Y (12)

#define TOP_SIZE_H (14)

#define BOTOM_SIZE_H (12.0F)
#define BOTOM_SIZE_H_LEVEL (BOTOM_SIZE_H-SC_RATE_H)

#define RADAR_WINDOW_X0		(DRAW_WIDTH - RADAR_OFFSET_X - RADAR_WINDOW_W )
#define RADAR_WINDOW_X1		(DRAW_WIDTH - RADAR_OFFSET_X)
#define RADAR_WINDOW_Y0		(RADAR_OFFSET_Y+TOP_SIZE_H_SC)
#define RADAR_WINDOW_Y1		(RADAR_WINDOW_Y0 + RADAR_WINDOW_H)

// by koba4
#define SK_RADAR_WINDOW_W ( RADAR_WINDOW_W )
#define SK_RADAR_WINDOW_H ( RADAR_WINDOW_H )
#endif

