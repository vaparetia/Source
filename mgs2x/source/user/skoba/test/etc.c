//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#ifdef PSX2
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libutl.h"

#include "def_dma.h"
#include "utl_dma.h"
#include "../../kira/2D_action/layout_2d.h"
#include "sprite_2d.h"

#include "libdg.h"
#include "libdg.cnf"
#include "libmt.h"
#include "camera.h"

void WorldPos(FVECTOR *pStr , FMATRIX *pWorld , FVECTOR *pPos )
{
	FVECTOR work;

	work = *pPos;
	vu0_Ldv0( &work );
	vu0_Ldm0( pWorld );
	vu0_Mulv0m0v0();
	vu0_Stv0( &work );

	pStr->vx = pWorld->m[ 3 ][ 0 ] + work.vx;
	pStr->vy = pWorld->m[ 3 ][ 1 ] + work.vy;
	pStr->vz = pWorld->m[ 3 ][ 2 ] + work.vz;
}

void mprintf( FMATRIX *fmat )
{
	printf("%6.2f %6.2f %6.2f %6.2f\n",fmat->m[0][0],fmat->m[0][1],fmat->m[0][2],fmat->m[0][3]);
	printf("%6.2f %6.2f %6.2f %6.2f\n",fmat->m[1][0],fmat->m[1][1],fmat->m[1][2],fmat->m[1][3]);
	printf("%6.2f %6.2f %6.2f %6.2f\n",fmat->m[2][0],fmat->m[2][1],fmat->m[2][2],fmat->m[2][3]);
	printf("%6.2f %6.2f %6.2f %6.2f\n",fmat->m[3][0],fmat->m[3][1],fmat->m[3][2],fmat->m[3][3]);
}

void vecprintf( FVECTOR *fvec )
{
	printf("%6.2f %6.2f %6.2f %6.2f\n",fvec->vx,fvec->vy,fvec->vz,fvec->vw);
}

void WorldPos2( FVECTOR *pPos ,  FMATRIX *pWorld )
{
	vu0_Ldv0( pPos );
	vu0_Ldm0( pWorld );
	vu0_Mulv0m0v0();
	vu0_Stv0( pPos );
}

void WorldVec(FVECTOR *pStr , FMATRIX *pWorld , FVECTOR *pPos )
{
	FVECTOR work;

	work = *pPos;
	vu0_Ldv0( &work );
	vu0_Ldm0( pWorld );
	vu0_Mulv0m0v0();
	vu0_Stv0( pStr );
}

/* スクラッチパッドからメインメモリへ転送 */
void _MemCopy( void *dst, void *src, int size, int num )
{
	UTL_StartSprToMem( dst, src, size * num / sizeof(u_long128) );
	UTL_EndSprToMem() ;
}
/* メインメモリからスクラッチパッドへ転送 */
void _MemCopy2( void *dst, void *src, int size, int num )
{
	UTL_StartMemToSpr( dst, src, size * num / sizeof(u_long128) );
	UTL_EndMemToSpr() ;
}

void MulVecInt( FVECTOR *pOut , float *pIn )
{
	pOut->vx *= *pIn;
	pOut->vy *= *pIn;
	pOut->vz *= *pIn;
}

float VecSize( FVECTOR *pVec )
{
	float tmp;

	tmp = pVec->vx * pVec->vx + pVec->vy * pVec->vy + pVec->vz * pVec->vz;

	return ( tmp );  
}

void SK_Err( char *data )
{
	printf("ERR ------> %s load miss prease call Koba4\n!!!!!!!!!!!!\n" , data );
}

void FvectorRndAdd( FVECTOR *out , FVECTOR *in )
{
	FVECTOR ftmp;

	ftmp.vx = rnd() * 10;
	ftmp.vy = rnd() * 10;
	ftmp.vz = rnd() * 10;
	ftmp.vw = 1.0f;
	_sceVu0AddVector( out , in , &ftmp );
}

/* あたりを見ながら進む点（静止） */
int SK_StepCheckHzd( FVECTOR *now, FVECTOR *before, FVECTOR *vec, float decay, float margin, HZX_GROUP_ID map_id )
{
	GM_CameraSet *pCam;
	FVECTOR		local_vec;
	FVECTOR		fvtemp0;
	FVECTOR		fvtemp1;
	FVECTOR		next;
	HZX_SEG		*seg ;
	HZX_FLR		flr[2] ;
	float		len;
	float		ratio;
	int	         flag;
	int			atr[2] ;

	len = GV_VecLen3F( vec );
	if( len == 0.0f ) len = 0.000001f; // 0DIV回避 yano add 2002.03.08
	ratio = margin * 1.41421f / len + 1.0f;
	local_vec.vx = vec->vx  * ratio;
	local_vec.vy = vec->vy  * ratio;
	local_vec.vz = vec->vz  * ratio;
	pCam = GM_GetCurrentCameraSet( 0 );	

	_sceVu0AddVector( &next, before, &local_vec );
	// ハザードチェック：床も見る 
	flag = HZX_OnlineHazardCheck( map_id , before ,	&next ,	HZX_CHK_ALL ,
								  HZX_SEG_NO_SPRAY | HZX_SEG_RECOIL_TYPE ,
								  HZX_FLOOR_NO_SPRAY | HZX_FLOOR_RECOIL_TYPE ) ;
	if( flag==1 ){	/* 壁 */
		HZX_GetOnlinePoint( &fvtemp1 );
		HZX_GetOnlineHazard( flr, atr ) ;
		seg=(HZX_SEG *)flr;
		fvtemp0.vx = seg->p2.z - seg->p1.z ;
		fvtemp0.vy = 0.0f ;
		fvtemp0.vz = seg->p1.x - seg->p2.x ;
		DG_ReflectVector( &fvtemp0, vec, vec );
		vec->vx = 0.f;
		vec->vy *= 0.5f;
		vec->vz = 0.f;
		// カメラにシフトする（メリコミ防止） 
		_sceVu0SubVector( &fvtemp0 , &pCam->position , &pCam->target );
		_sceVu0Normalize( &fvtemp0 , &fvtemp0 );
		_sceVu0ScaleVector( &fvtemp0 , &fvtemp0 , 2000.0f );
		_sceVu0AddVector( &now , &next , &fvtemp0 );
		return ( 1 );
	}else if( flag==2 ){	/* 床 */
		HZX_GetOnlinePoint( &fvtemp1 );
		vec->vx *= 0.8f;
		vec->vy = 0.f;
		vec->vz *= 0.8f;
		_sceVu0SubVector( &fvtemp0 , &pCam->position , &pCam->target );
		_sceVu0Normalize( &fvtemp0 , &fvtemp0 );
		_sceVu0ScaleVector( &fvtemp0 , &fvtemp0 , 2000.0f );
		_sceVu0AddVector( &now , &next , &fvtemp0 );
		return ( 2 );
	} else {
		*now = next;
	}
	return 0;
}

/* あたりを見ながら進む点（半用） */
int SK_StepCheckHzd2( FVECTOR *now, FVECTOR *before, FVECTOR *vec, float decay, float margin, HZX_GROUP_ID map_id , 
					 FVECTOR *f_reflect , FVECTOR *w_reflect )
{
	GM_CameraSet *pCam;
	FVECTOR		local_vec;
	FVECTOR		fvtemp0;
	FVECTOR		fvtemp1;
	FVECTOR		next;
	HZX_SEG		*seg ;
	HZX_FLR		flr[2] ;
	float		len;
	float		ratio;
	int	         flag;
	int			atr[2] ;

	len = GV_VecLen3F( vec );
	if( len == 0.0f ) len = 0.000001f; // 0DIV回避 yano add 2002.03.08
	ratio = margin * 1.41421f / len + 1.0f;
	local_vec.vx = vec->vx  * ratio;
	local_vec.vy = vec->vy  * ratio;
	local_vec.vz = vec->vz  * ratio;
	pCam = GM_GetCurrentCameraSet( 0 );	

#if 0
	printf("next = ");
	vecprintf( &local_vec );
#endif
	_sceVu0AddVector( &next, before, &local_vec );
	// ハザードチェック：床も見る 
	flag = HZX_OnlineHazardCheck( map_id , before ,	&next ,	HZX_CHK_ALL ,
								  HZX_SEG_NO_SPRAY | HZX_SEG_RECOIL_TYPE , 0 );
//								  HZX_FLOOR_NO_SPRAY | HZX_FLOOR_RECOIL_TYPE ) ;
	if( flag==1 ){	/* 壁 */
		HZX_GetOnlinePoint( &fvtemp1 );
		HZX_GetOnlineHazard( flr, atr ) ;
		seg=(HZX_SEG *)flr;
		fvtemp0.vx = seg->p2.z - seg->p1.z ;
		fvtemp0.vy = 0.0f ;
		fvtemp0.vz = seg->p1.x - seg->p2.x ;
		DG_ReflectVector( &fvtemp0, vec, vec );
		_sceVu0MulVector( vec , vec , w_reflect );
		_sceVu0AddVector( &now , &next , &fvtemp0 );
		return ( 1 );
	}else if( flag==2 ){	/* 床 */
		HZX_GetOnlinePoint( &fvtemp1 );
		_sceVu0MulVector( vec , vec , f_reflect );
		_sceVu0AddVector( &now , &next , &fvtemp0 );
		return ( 2 );
	} else {
		*now = next;
	}
	return 0;
}

/* あたりを見ながら進む点（半用） */
int SK_StepCheckHzd3( FVECTOR *now, FVECTOR *before, FVECTOR *vec, float decay, float margin, HZX_GROUP_ID map_id , 
					 FVECTOR *f_reflect , FVECTOR *w_reflect )
{
	GM_CameraSet *pCam;
	FVECTOR		local_vec;
	FVECTOR		fvtemp0;
	FVECTOR		fvtemp1;
	FVECTOR		next;
	HZX_SEG		*seg ;
	HZX_FLR		flr[2] ;
	float		len;
	float		ratio;
	int	         flag;
	int			atr[2] ;

	len = GV_VecLen3F( vec );
	if( len == 0.0f ) len = 0.000001f; // 0DIV回避 yano add 2002.03.08
	ratio = margin * 1.41421f / len + 1.0f;
	local_vec.vx = vec->vx  * ratio;
	local_vec.vy = vec->vy  * ratio;
	local_vec.vz = vec->vz  * ratio;
	pCam = GM_GetCurrentCameraSet( 0 );	

	_sceVu0AddVector( &next, before, &local_vec );
	// ハザードチェック：床も見る 
	flag = HZX_OnlineHazardCheck( map_id , before ,	&next ,	HZX_CHK_ALL ,
								  HZX_SEG_NO_SPRAY | HZX_SEG_RECOIL_TYPE ,
								  HZX_FLOOR_NO_SPRAY | HZX_FLOOR_RECOIL_TYPE ) ;
	if( flag==1 ){	/* 壁 */
		HZX_GetOnlinePoint( &fvtemp1 );
		HZX_GetOnlineHazard( flr, atr ) ;
		seg=(HZX_SEG *)flr;
		fvtemp0.vx = seg->p2.z - seg->p1.z ;
		fvtemp0.vy = 0.0f ;
		fvtemp0.vz = seg->p1.x - seg->p2.x ;
		DG_ReflectVector( &fvtemp0, vec, vec );
		_sceVu0MulVector( vec , vec , w_reflect );
		return ( 1 );
	}else if( flag==2 ){	/* 床 */
		HZX_GetOnlinePoint( &fvtemp1 );
		_sceVu0MulVector( vec , vec , f_reflect );
		return ( 2 );
	}
	return 0;
}


// yano
#ifdef PSX2
// サイトの表示非表示管理 return --> visible = true , invisible = false  
int SightVisibleInvisible( int handle , int strcode )
{
	SPR_OBJ *root;

	root = L2D_GetObject( handle , strcode );
	if ( root == NULL ){
		SK_Err("Visible Invisible No Control\0");
		return ( 0 );
	}
	if ( GM_CheckSightStatus( SGT_Invisible ) ){
		SPR_HIDE( root );
		return ( -1 );
	} else {
		SPR_SHOW( root );
		return ( 1 );
	}
}

// サイトの表示非表示管理(Sthinger 特注) return --> visible = true , invisible = false  
int SthingerSightVisibleInvisible( int handle , int strcode , int flag )
{
	SPR_OBJ *root;

	root = L2D_GetObject( handle , strcode );
	if ( root == NULL ){
		SK_Err("Visible Invisible No Control\0");
		return ( 0 );
	}
	if ( GM_CheckSightStatus( SGT_Invisible ) || flag == 1 ){
		SPR_HIDE( root );
		return ( -1 );
	} else {
		SPR_SHOW( root );
		return ( 1 );
	}
}
#else
// yano --> inlineを外しました。

// サイトの表示非表示管理 return --> visible = true , invisible = false  
int SightVisibleInvisible( int handle , int strcode )
{
	SPR_OBJ *root;

	root = L2D_GetObject( handle , strcode );
	if ( root == NULL ){
		SK_Err("Visible Invisible No Control\0");
		return ( 0 );
	}
	if ( GM_CheckSightStatus( SGT_Invisible ) ){
		SPR_HIDE( root );
		return ( -1 );
	} else {
		SPR_SHOW( root );
		return ( 1 );
	}
}

// サイトの表示非表示管理(Sthinger 特注) return --> visible = true , invisible = false  
int SthingerSightVisibleInvisible( int handle , int strcode , int flag )
{
	SPR_OBJ *root;

	root = L2D_GetObject( handle , strcode );
	if ( root == NULL ){
		SK_Err("Visible Invisible No Control\0");
		return ( 0 );
	}
	if ( GM_CheckSightStatus( SGT_Invisible ) || flag == 1 ){
		SPR_HIDE( root );
		return ( -1 );
	} else {
		SPR_SHOW( root );
		return ( 1 );
	}
}
#endif
//ynao


// 2Dの解放 
void SK_FreeMemory( SPR_OBJ **obj , int num )
{
	int i;

	for ( i = 0 ; i < num ; i++ ){
#if 0
		printf("free num %d\n" , num );
		SK_Err("Non Memory ");
#endif
		if ( obj[ i ] != NULL ){
#if 0
			printf("free %d %p\n" , i , obj[ i ] );
#endif
			SPR_Destroy_2D_Object( obj[ i ] );
		}
	}
}


// String Search Module 
#define ASCCI_MAX (16)
static int SK_StringCmpModule( u_char *code1 , u_char *code2 ) // code1 = 元 , code2 = 比較対象 
{
	int i;

	i = 0;
	while( ( code1[ i ] != '\0' ) && ( code2[ 0 ] != ' ' ) && ( i < ASCCI_MAX ) ){ // 終端コード 0x0 
		printf("%c = %c\n" , code1[ i ] , code2[ i ] );
		if ( code1[ i ] != code2[ i ] ) {
			return ( -1 );
		}
		i++;
	}
	printf("code1 : %d code2 : %d\n" , strlen( code1 ) , strlen( code2 ) );
	if ( ( strlen( code1 ) == strlen( code2 ) ) && ( strlen( code1 ) != 0 ) && ( strlen( code2 ) != 0 ) ){
		return ( 1 ); 
	} else {
		return ( -1 );
	}
}

int SK_StringCmp( u_char *pFirst_name , u_char *pLast_name , u_char *pCode1 , u_char *pCode2 )
{
	int tmp;

	// first name cmp
//	printf("first\n");
	tmp = SK_StringCmpModule( pFirst_name , pCode1 );

	if ( tmp < 0 ){
		return ( -1 );
	}
	
	// last name cmp
//	printf("last\n");
	tmp = SK_StringCmpModule( pLast_name , pCode2 );

	if ( tmp < 0 ){
		return ( -1 );
	} else {
		return ( 1 );
	}
}

int SK_StringCmpSingle( u_char *pFirst , u_char *pCode1 )
{
	int tmp;

	// first name cmp
	tmp = SK_StringCmpModule( pFirst , pCode1 );

	if ( tmp < 0 ){
		return ( -1 );
	} else {
		return ( 1 );
	}
}

void SK_ToUpper( u_char *str1 , u_char *str2 ) // str1 = str2 大文字 ＝ 小文字 
{
	int i;

	i = 0;
	while ( str2[ i ] != '\0' ){
		if ( str2[ i ] >= 0x61 && str2[ i ] <= 0x7a ){
			str1[ i ] = str2[ i ] - 0x20;
		} else if ( str2[ i ] == '|' ){
			str1[ i ] = 0x20;
		} else {
			str1[ i ] = str2[ i ];
		}
		i ++;
	}
}

