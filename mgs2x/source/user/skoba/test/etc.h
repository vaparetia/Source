#ifndef _ETC_H_
#define _ETC_H_

// define 
#define SCR_SPACE (4096) //  
#define SCR_FVECTOR_MAX ( SCR_SPACE / sizeof( FVECTOR ) ) // 最大 
#define SCR_UVRGBWH_MAX ( SCR_SPACE / sizeof( DG_PRIM2_UVRGBWH ) ) // 最大 

#define SE_SEL()		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_CUR01)
#define SE_START()		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_START01)
#define SE_OK()			GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_WIN01)
#define SE_CANCEL()		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_V_CANS02)
#define SE_RGB_TUNE()   GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_R_TUNE01)
#define SE_WINDOW()     GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_CUR01) // SD_S_W2NCLS01 
#define SE_LINE()       GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_CUR01)
#define SE_TYPE()       GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_TYPING03)
#define SE_WINOPEN()	GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_WINOPN01)
#define SE_WINOPEN_R()	GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_WINOPNR1)
#define SE_WINCLOSE()	GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_WINCLS01)
#define SE_WINCLOSE_L()	GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_WINCLSL1)
#define SE_WINCLOSE_R()	GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_WINCLSR1)

// extern 
extern void WorldPos(FVECTOR * , FMATRIX * , FVECTOR * );
extern void WorldVec(FVECTOR * , FMATRIX * , FVECTOR * );
extern void _MemCopy( void * , void * , int , int );
extern void _MemCopy2( void * , void * , int , int );
extern void MulVecInt( FVECTOR * , float * );
extern void WorldPos2( FVECTOR * ,  FMATRIX * );
extern void mprintf( FMATRIX * );
extern void vecprintf( FVECTOR * );
extern void SK_Err( char * );
extern void FvectorRndAdd( FVECTOR * , FVECTOR * );
extern void SK_FreeMemory( SPR_OBJ **obj , int num );
extern void SK_ToUpper( u_char *str1 , u_char *str2 );
extern int SK_StepCheckHzd( FVECTOR * , FVECTOR * , FVECTOR * , float , float , HZX_GROUP_ID );
extern int SK_StepCheckHzd2( FVECTOR * , FVECTOR * , FVECTOR * , float , float , HZX_GROUP_ID , FVECTOR * , FVECTOR * );
extern int SK_StepCheckHzd3( FVECTOR * , FVECTOR * , FVECTOR * , float , float , HZX_GROUP_ID , FVECTOR * , FVECTOR * );
extern int SK_StringCmp( u_char *pFirst_name , u_char *pLast_name , u_char *pCode1 , u_char *pCode2 );
extern int SK_StringCmpSingle( u_char *pFirst , u_char *pCode1 );
extern int SightVisibleInvisible( int handle , int strcode );
extern int SthingerSightVisibleInvisible( int handle , int strcode , int flag );

// asm 

#ifdef PS2_ASM
static inline void MulVecVec(const FVECTOR * const pOut , const FVECTOR * const pIn )
{
	// vu 版 
    asm volatile ("
    lqc2	vf2,0x00(%0)
    lqc2	vf3,0x00(%1)
	VMUL.xyzw  vf1xyzw, vf2xyzw, vf3xyzw
	sqc2	vf1,0x00(%0)
    " : : "r"(pOut), "r"(pIn) );

}

static inline void _RotTrans( FVECTOR *out, FMATRIX *world , FVECTOR *in )
{
	asm ("
	lqc2				vf8,0x00(%2)
	lqc2				vf4,0x00(%1)
	lqc2				vf5,0x10(%1)
	lqc2				vf6,0x20(%1)
	lqc2				vf7,0x30(%1)
	vmulax.xyzw			ACC, vf4,vf8
	vmadday.xyzw		ACC, vf5,vf8
	vmaddaz.xyzw		ACC, vf6,vf8
	vmaddw.xyzw			vf8, vf7,vf8
	sqc2				vf8,0x00(%0)
	": : "r"(out), "r"(world), "r"(in) );//:"memory" ); 
}
#endif

/*
static inline void MulVecVec(const FVECTOR * const pOut )
{
	// vu 版 
    asm volatile ("
    lqc2	vf1,0x00(%0)
	VMADD.x  accx, vf1x, vf1x
	VMADD.y  accy, vf1y, vf1y
	VMADD.z  accz, vf1z, vf1z
	sqc2	vf1x,0x00(%0)
    " : : "r"(pOut) );
}
*/
#endif
