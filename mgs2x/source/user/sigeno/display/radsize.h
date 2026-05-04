/**********
***********/
/*
   仮想座標系(x:512 y:384)
*/
/*なし*/

/*
   実座標系(PS2::x:512 y:448 / PS2_PAL::x:512 y:512 / XBOX::x:640 y:448)
*/
#ifdef PSX2
#define RADAR_WINDOW_W		120
#define SC_RATE_W			(1.0f)
#else
#define RADAR_WINDOW_W		150
#define SC_RATE_W			(0.8f)
#endif

//256/224= 1.142857
#ifndef AREA_EU_BP_IGNORE()		// #ifndef PAL
#define RADAR_WINDOW_H		104
#define SC_RATE_H		(0.8571F)
#define TOP_SIZE_H_SC	(17)
#else
#define RADAR_WINDOW_H		120
#define SC_RATE_H		(0.75F)
#define TOP_SIZE_H_SC	(18)
#endif

#ifdef PSX2
#define RADAR_OFFSET_X (16)
#else
#define RADAR_OFFSET_X (20)
#endif
#define RADAR_OFFSET_Y (12)

#define TOP_SIZE_H (14)

#define BOTOM_SIZE_H (12.0F)
#define BOTOM_SIZE_H_LEVEL (BOTOM_SIZE_H-SC_RATE_H)


#define RADAR_WINDOW_X0		(DRAW_WIDTH - RADAR_OFFSET_X - RADAR_WINDOW_W )
#define RADAR_WINDOW_X1		(DRAW_WIDTH - RADAR_OFFSET_X)
#define RADAR_WINDOW_Y0		(RADAR_OFFSET_Y+TOP_SIZE_H_SC)
#define RADAR_WINDOW_Y1		(RADAR_WINDOW_Y0 + RADAR_WINDOW_H)

