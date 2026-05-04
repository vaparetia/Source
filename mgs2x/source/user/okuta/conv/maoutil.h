/*
    maoutil.h
    便利関数用ヘッダ
    2001/04/13 Masafumi Okuta
    $Id: maoutil.h,v 1.1.1.3 2002/11/19 11:47:48 Yoshizawa1 Exp $
*/
#ifndef __MAO_UTIL_H__
#define __MAO_UTIL_H__

extern int _FVecLen3( FVECTOR* );
extern int _FVecDir2( FVECTOR* );

void 	MAO_GetDiffVec3( FVECTOR*, int*, int*, FVECTOR*, FVECTOR* );
void	MAO_DbgPlayerPosDump(void);
int 	MAO_FlatCheck( FVECTOR*, FVECTOR*, FVECTOR*);
int 	MAO_BoundCheck( FVECTOR*, FVECTOR*, FVECTOR*);
int 	MAO_SphereCheck(FVECTOR*, FVECTOR*, int);
void 	MAO_DbgDumpVector( FVECTOR*);
void 	MAO_DbgDumpSVector( SVECTOR*);
void 	MAO_DbgDumpMatrix( FMATRIX*);
void 	MAO_DbgDrawMatrix( FMATRIX*, float);
void 	MAO_DbgDrawBox( FVECTOR*, float, float, float, u_char, u_char, u_char);
int 	MAO_DbgDrawTrap( int nTrap, u_char r, u_char g, u_char b);
int 	MAO_DbgDrawZone( int hzx_id, int nZone, u_char r, u_char g, u_char b);
int	MAO_GetDirZone2( FVECTOR*, int, int);
int 	MAO_GetDirZoneToZone(int, int);

// スクラッチパッド関連
void Mao_CopyMemToScr( void*, void*, int, int);
void Mao_CopyScrToMem( void*, void*, int, int);

extern void UTL_StartSprToMem( void *mem_addr, void *spr_addr, int size );	
extern void UTL_StartMemToSpr( void *spr_addr, void *mem_addr, int size );    
extern void UTL_EndSprToMem( void );

int MAO_ChkZoneInZ2Z( HZX_GROUP_ID hzx_id, int nZone1, int nZone2, int nAimzone);

// プリミティブ系
extern DG_PRIM2* MAO_FreePrim2( DG_PRIM2* prim );
extern int MAO_SetRGBA( u_char R, u_char G, u_char B, u_char A);

void* MAO_MemoryFullCreate( void );
void  MAO_MemoryFullDestroy( void );


// yano gcc記述削除しました。
#ifdef PSX2

#ifdef DEBUG_MODE
#define MAO_PRINTF( ... )  { printf( __VA_ARGS__); printf("-----------> %s line:%d\n", __FILE__, __LINE__ );  }
#else
#define MAO_PRINTF( ... )
#endif

#else

#ifdef DEBUG_MODE
#define MAO_PRINTF printf
#else
#define MAO_PRINTF
#endif

#endif
// yano

#endif // __MAO_UTIL_H__






