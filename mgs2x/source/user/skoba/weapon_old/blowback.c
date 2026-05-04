//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
        blowback.c
        ブローバック

        1999/12/03 M.Matsuzaki
        2001/04/05 S.Kobayashi
*/

#define MATSUZAKI_SOURCE
#include "./matsu.h"
#include "wpeffect.h"
#include        "../../okajima/etc/ok_util.h"
#include "../test/etc.h"

static void BlowBack_Usp(void *);
static void BlowBack_Famas(void *);
static void BlowBack_M92(void *);
static void BlowBack_ShotGun(void *);
static void BlowBack_GLK(void *);
static void BlowBack_ABK(void *);
static void BlowBack_M92_Sea(void *);
static void BlowBack_RGB6(void *);

BLOWBACK_INFO BlowBack_info[]={
        {NULL                                ,0        , 0},
        {BlowBack_Usp                ,5        , 4},        /* USP */
        {BlowBack_Usp                ,5        ,-1},        /* マカロフ */
        {BlowBack_Famas                ,4        ,-1},        /* ファマス */
        {BlowBack_M92                ,50        , 6},        /* M92 */
        {BlowBack_Usp           ,5        , 4},        // SOCOM
        {BlowBack_ShotGun        ,10        ,SHOTGUN_WAIT_COUNT},        /* SHOT GUN SPS */
        {BlowBack_GLK                ,3        , 4},        /* GLK */
        {BlowBack_ABK                ,2        ,-1},        /* アバカン */
        {BlowBack_M92_Sea        ,5        , 4},        /* M92_Sea SilenceなしのM92*/
        {BlowBack_RGB6           ,10        , 4},        /* RGB6*/
        // BP FIX - added NULL function for weapon (see enum in wpeffect.h)
        {NULL                    , 0        , 0 }        /* M4_GRD_HTC */
};

extern void *NewGunSmoke2( FMATRIX * , FVECTOR * , float , int , float );
// sps
extern void CartridgeSPS( FMATRIX * , OBJECT * , CONTROL * );
//      {BlowBack_ShotGun        ,10        ,SHOTGUN_WAIT_COUNT},        /* SHOT GUN SPS */ ショットガンの上にあった

/* ＵＳＰ */
/* ＭＫＲ */
//#define USP_ROT_X     (0.85f)                /* 回転角 */
#define USP_ROT_X       ( ( 7.5f * PI / 360 ) )                /* 回転角 */
#define USP_SLIDE_Y     (50.00f)
static void BlowBack_Usp(void *p){
        WEAPON_EF_CTRL *pctrl=(WEAPON_EF_CTRL *)p;
        FMATRIX *pmat;
        DG_OBJ        *punit = pctrl->pobjs->objs;
        DG_MDL        *pmdl;
        int motion;
        int i;

        motion = DIRECT_TICK( pctrl->motion );
        for(i = 0 ; i < pctrl->pobjs->n_models  ;i++ ,punit++){
                pmdl=punit->model;
                if(pmdl->parent >= 0){
                        pmat =&punit->world;
                        _sceVu0UnitMatrix(pmat);
                        switch(i){
                        case 1:                /* ハンマー */
                                if(pctrl->kind==WEF_ID_MKR)break;                        /* 以下USP , Socom のみ */
                                if((pctrl->flag2 & WPEF_FLG2_SHOTFIRST)){        /* 装備直後 */
                                        if( motion == 0 ){ // 初期化
                                                _sceVu0RotMatrixX( pmat , pmat , 0.0f ); 
                                        }else{
                                                pctrl->flag2 &= (~WPEF_FLG2_SHOTFIRST);
                                        }
                                } else {
                                  // if( pctrl->motion >= 4 ){
                                        _sceVu0RotMatrixX(pmat , pmat , ( USP_ROT_X * ( float )( motion % DIRECT_TICK( 5 ) ) )  );
//                                      _sceVu0RotMatrixX(pmat , pmat , ( USP_ROT_X * ( ( float )( pctrl->motion - 4 ) ) / 2.0f ) );
                                }
                                break;
                        case 2:                /* スライド部分 */
                                if( motion < 0){
                                        pmat->m[3][1] = 0.0f;
                                }else if( motion < DIRECT_TICK( 4 ) ){
                                        pmat->m[3][1] = ( float )( USP_SLIDE_Y / DIRECT_TICK( 4 ) ) * ( float )motion;
                                }else{
                                        pmat->m[3][1] = ( USP_SLIDE_Y );
                                }
                                break;
                        default:
                                break;
                        }
                        pmat->m[3][0] = pmdl->tx;
                        pmat->m[3][1]+= pmdl->ty;
                        pmat->m[3][2] = pmdl->tz;
                        pmat->m[3][3] = 1.0F;
                        _sceVu0MulMatrix( pmat , &(pctrl->pobjs->objs[pmdl->parent].world) , pmat );
                }else{
                        punit->world = *(pctrl->pobjs->root);
                }
        }
}


/* FAMAS */
#define FMS_SLIDE_Y     (32.00f)
static void BlowBack_Famas(void *p)
{
        WEAPON_EF_CTRL *pctrl=(WEAPON_EF_CTRL *)p;
        FMATRIX *pmat;
        DG_OBJ        *punit = pctrl->pobjs->objs;
        DG_MDL        *pmdl;
        int    motion;

        pmdl=punit->model;
        punit->world = *(pctrl->pobjs->root);
        punit++;
        pmdl=punit->model;
        pmat =&punit->world;
        _sceVu0UnitMatrix(pmat);

        motion = DIRECT_TICK( pctrl->motion );
        pmat->m[3][0] = pmdl->tx;
        pmat->m[3][1] = pmdl->ty + ( ( float )( motion % DIRECT_TICK( 4 ) ) * FMS_SLIDE_Y );
        pmat->m[3][2] = pmdl->tz;
        pmat->m[3][3] = 1.0F;
        _sceVu0MulMatrix(pmat,&(pctrl->pobjs->objs->world),pmat);
}

/* アバカン */
#define ABA_SLIDE_Y     (16.00f)
static void BlowBack_ABK( void *p )
{
        WEAPON_EF_CTRL *pctrl=(WEAPON_EF_CTRL *)p;
        FMATRIX *pmat;
        DG_OBJ        *punit = pctrl->pobjs->objs;
        DG_MDL        *pmdl;
        int      motion;

        pmdl=punit->model;
        punit->world = *(pctrl->pobjs->root);
        punit++;
        pmdl=punit->model;
        pmat =&punit->world;
        _sceVu0UnitMatrix(pmat);

        motion = DIRECT_TICK( pctrl->motion );
        pmat->m[3][0] = pmdl->tx;
        pmat->m[3][1] = pmdl->ty + ( ( float )( motion % DIRECT_TICK( 3 ) ) * ABA_SLIDE_Y );
        pmat->m[3][2] = pmdl->tz;
        pmat->m[3][3] = 1.0F;
        _sceVu0MulMatrix( pmat , &(pctrl->pobjs->objs->world) , pmat );
}

/* M4A1 */
#define M4A1_SLIDE_Y    (7.5f)
void SK_BlowBack_M4A1( void *p , char phase ) // ずっとOPEN状態にしたい場合はmotionに15(NTSC)を入れておく
{
        WEAPON_EF_CTRL *pctrl=( WEAPON_EF_CTRL * )p;
        FMATRIX *pmat;
        DG_OBJ        *punit = pctrl->pobjs->objs;
        DG_MDL        *pmdl;
        int      motion;
        int      tmp_motion;

        pmdl = punit->model;
        punit->world = *( pctrl->pobjs->root );
        punit++;
        pmdl = punit->model;
        pmat = &punit->world;
        _sceVu0UnitMatrix( pmat );

        motion = DIRECT_TICK( pctrl->motion );
        tmp_motion = DIRECT_TICK( 16 );
        switch ( phase ){ 
        case 0x0 : // normal_phase
                pmat->m[ 3 ][ 0 ] = pmdl->tx;
                pmat->m[ 3 ][ 1 ] = pmdl->ty;
                pmat->m[ 3 ][ 2 ] = pmdl->tz;
                pmat->m[ 3 ][ 3 ] = 1.0F;
                _sceVu0MulMatrix( pmat , &( pctrl->pobjs->objs->world ) , pmat );
                break;
        case 0x1 : // open phase
                pmat->m[ 3 ][ 0 ] = pmdl->tx;
                pmat->m[ 3 ][ 1 ] = pmdl->ty - ( ( float )( motion % tmp_motion ) * M4A1_SLIDE_Y );
                pmat->m[ 3 ][ 2 ] = pmdl->tz;
                pmat->m[ 3 ][ 3 ] = 1.0F;
                if ( motion % tmp_motion < tmp_motion - 1 ){
                        pctrl->motion++;
                }
                _sceVu0MulMatrix( pmat , &( pctrl->pobjs->objs->world ) , pmat );
                break;
        case 0x2 : // close phase
                pmat->m[ 3 ][ 0 ] = pmdl->tx;
                pmat->m[ 3 ][ 1 ] = pmdl->ty - ( M4A1_SLIDE_Y * DIRECT_TICK( 15 ) ) + ( float )( motion % tmp_motion ) * M4A1_SLIDE_Y;
                pmat->m[ 3 ][ 2 ] = pmdl->tz;
                pmat->m[ 3 ][ 3 ] = 1.0F;
                if ( motion % tmp_motion < tmp_motion - 1 ){
                        pctrl->motion++;
                }
                _sceVu0MulMatrix( pmat , &(pctrl->pobjs->objs->world) , pmat );
                break;
        default : break;
        }
}

// GLK
#define GLK_SLIDE_Y     (20.00f)
#define GLK_ROT_X       (-20.00f)
static void BlowBack_GLK( void *p )
{
        WEAPON_EF_CTRL *pctrl=(WEAPON_EF_CTRL *)p;
        FMATRIX *pmat;
        DG_OBJ        *punit = pctrl->pobjs->objs;
        DG_MDL        *pmdl;
        FVECTOR vec;
        SVECTOR rot;
        int  motion;

        motion = DIRECT_TICK( pctrl->motion );
        pmdl=punit->model;
        punit->world = *(pctrl->pobjs->root);
        pmdl = punit->model;
        pmat = &punit->world;
        //回転
        /*        rot.vx = (float)(pctrl->motion % 4) * GLK_ROT_X;
        rot.vy = rot.vz = 0;
        DG_SetPos( &punit->world );
        DG_RotatePosZYX( &rot );
        DG_GetPos( &punit->world );
        */
        punit++;
        pmdl = punit->model;
        pmat = &punit->world;
        vec.vx = pmdl->tx;
        vec.vy = pmdl->ty + ( ( float )( motion % DIRECT_TICK( 4 ) ) * GLK_SLIDE_Y );
        vec.vz = pmdl->tz;
        vec.vw = 1.0F;
        rot.vx = rot.vy = rot.vz = 0;
        DG_SetPos2( &vec , &rot );
        DG_GetPos( pmat );
        _sceVu0MulMatrix( pmat , &(pctrl->pobjs->objs->world) , pmat );
        // カードリッジどうしようか
}

// RGB6
//#define RGB6_SILINDER_ROT_Y   ( -36.0f * PI / 360.0f )
#define RGB6_ROT_Y      ( 6.0f * PI / 360.0f )
static void BlowBack_RGB6( void *p )
{
        WEAPON_EF_CTRL *pctrl=(WEAPON_EF_CTRL *)p;
        FMATRIX *pmat;
        DG_OBJ        *punit = pctrl->pobjs->objs;
        DG_MDL        *pmdl;
        int  motion;
        int  i;

        motion = DIRECT_TICK( pctrl->motion ); // PAL対応
        for( i = 0 ; i < pctrl->pobjs->n_models ; i++ , punit++ ){
                pmdl = punit->model;
                if( pmdl->parent >= 0){
                        pmat = &punit->world;
                        _sceVu0UnitMatrix( pmat );
                        if ( !( pctrl->flag2 & WPEF_FLG2_RELOAD ) ){
                                switch( i ){
#if 0
                                case 1:        // 軸
                                        if( ( pctrl->flag2 & WPEF_FLG2_SHOTFIRST ) ){        /* 装備直後 */
                                                if( motion == 0 ){
                                                        _sceVu0RotMatrixY( pmat , pmat , 0.0f );
                                                }else{
                                                        pctrl->flag2 &= ( ~WPEF_FLG2_SHOTFIRST );
                                                }
                                        } else         if ( pctrl->flag2 & WPEF_FLG2_SHOTFIRST ){ // リロードフラグがたっていたら将来そうするまだモーションがないから
                                                if ( motion > 5 ){
                                                        _sceVu0RotMatrixY( pmat , pmat , RGB6_SILINDER_ROT_Y * ( float )( DIRECT_TICK( 10 ) - motion ) );
                                                } else {
                                                        _sceVu0RotMatrixY( pmat , pmat , RGB6_SILINDER_ROT_Y * ( float )( motion ) );
                                                }
                                        }
                                        break; 
#endif
                                case 2:        // リボルバー
                                        //回転
                                        _sceVu0RotMatrixY( pmat , pmat , ( float )( DIRECT_TICK( 10 ) - motion ) * RGB6_ROT_Y );
                                        break;
                                default:
                                        break;
                                }
                        }
                        pmat->m[3][0] = pmdl->tx;
                        pmat->m[3][1] = pmdl->ty; // += -> = 2001/06/04 by koba4
                        pmat->m[3][2] = pmdl->tz;
                        pmat->m[3][3] = 1.0F;
                        _sceVu0MulMatrix( pmat , &( pctrl->pobjs->objs[pmdl->parent].world ) , pmat );
                } else {
                        punit->world = *(pctrl->pobjs->root);
                }

        }
}

/* RGB6Reload */
void SK_BlowBack_RGB6( void *p , char phase ) // ずっとOPEN状態にしたい場合はmotionに15(NTSC)を入れておく
{
        WEAPON_EF_CTRL *pctrl=( WEAPON_EF_CTRL * )p;
        FMATRIX *pmat;
        DG_OBJ        *punit = pctrl->pobjs->objs;
        DG_MDL        *pmdl;
        int      motion;
        int      tmp_motion;
        int         i;

//      pmdl = punit->model;
        punit->world = *( pctrl->pobjs->root );
        for ( i = 1 ; i < pctrl->pobjs->n_models ; i ++ ){
                punit++;
                pmdl = punit->model;
                pmat = &punit->world;
                _sceVu0UnitMatrix( pmat );

                pmat->m[ 3 ][ 0 ] = pmdl->tx;
                pmat->m[ 3 ][ 1 ] = pmdl->ty;
                pmat->m[ 3 ][ 2 ] = pmdl->tz;
                pmat->m[ 3 ][ 3 ] = 1.0F;
                if ( i == 1 ){
                        motion = DIRECT_TICK( pctrl->motion );
                        tmp_motion = DIRECT_TICK( 16 ); // 移動frame
                        switch ( phase ){ 
                        case 0x0 : // normal_phase
                                _sceVu0MulMatrix( pmat , &( pctrl->pobjs->objs->world ) , pmat );
                                break;
                        case 0x1 : // open phase
                                //回転
                                _sceVu0RotMatrixY( pmat , pmat , ( float )( motion % tmp_motion ) * RGB6_ROT_Y );
                                if ( motion % tmp_motion < tmp_motion - 1 ){
                                        pctrl->motion++;
                                }
                                _sceVu0MulMatrix( pmat , &( pctrl->pobjs->objs->world ) , pmat );
                                break;
                        case 0x2 : // close phase
                                //回転
                                {
                                        float ftmp;
                                        ftmp = ( float )( ( ( tmp_motion - 1 ) * RGB6_ROT_Y ) - ( ( motion % tmp_motion ) * RGB6_ROT_Y ) );
                                        _sceVu0RotMatrixY( pmat , pmat , ftmp );
                                        if ( motion % tmp_motion < tmp_motion - 1 ){
                                                pctrl->motion++;
                                        }
                                        _sceVu0MulMatrix( pmat , &(pctrl->pobjs->objs->world) , pmat );
                                }
                                break;
                        default : break;
                        }
                }
        }
}

/* Ｍ９２ */
#define M92_ROT_X       (0.9f)
#define M92_SLIDE_Y     (50.00f)
static void BlowBack_M92(void *p){
        WEAPON_EF_CTRL *pctrl=(WEAPON_EF_CTRL *)p;
        FMATRIX *pmat;
        DG_OBJ        *punit = pctrl->pobjs->objs;
        DG_MDL        *pmdl;
        int    motion;
        int i;

        motion = DIRECT_TICK( pctrl->motion );
        for(i = 0 ; i < pctrl->pobjs->n_models  ;i++ ,punit++){
                pmdl=punit->model;
                if(pmdl->parent >= 0){
                        pmat =&punit->world;
                        _sceVu0UnitMatrix(pmat);
                        switch(i){
                        case 1:                /* ハンマー */
                                if((pctrl->flag2 & WPEF_FLG2_SHOTFIRST)){        /* 装備直後 */
                                        if( motion == 0 ){
                                                _sceVu0RotMatrixX(pmat , pmat , M92_ROT_X);
                                        }else{
                                                pctrl->flag2 &= (~WPEF_FLG2_SHOTFIRST);
                                        }
                                }else if( motion > DIRECT_TICK( 15 ) ){
                                        if( motion < DIRECT_TICK( 20 ) ){
                                                _sceVu0RotMatrixX( pmat , pmat , ( M92_ROT_X * ( ( float )( motion - DIRECT_TICK( 15 ) ) ) / ( float )DIRECT_TICK( 5 ) ) );
                                        }else if( motion < DIRECT_TICK( 50 ) ){
                                                _sceVu0RotMatrixX(pmat,pmat,M92_ROT_X);
                                        }
                                }
                                break;
                        case 2:                /* スライド部分 */
                                if( motion < 0 ){
                                        pmat->m[3][1] = 0.0f;
                                }else if( motion < DIRECT_TICK( 16 ) ){
                                        pmat->m[3][1] = ( float )( M92_SLIDE_Y / DIRECT_TICK( 16 ) ) * (float)motion;
                                }else if( motion <= DIRECT_TICK( 22 ) ){
                                        pmat->m[3][1] = ( M92_SLIDE_Y );
                                }else if( motion < DIRECT_TICK( 24 ) ){
                                        pmat->m[3][1] = ( M92_SLIDE_Y / 2.0f ) * (float)( DIRECT_TICK( 24 ) - motion );
                                }
                                break;
                        default:
                                break;
                        }
                        pmat->m[3][0] = pmdl->tx;
                        pmat->m[3][1]+= pmdl->ty;
                        pmat->m[3][2] = pmdl->tz;
                        pmat->m[3][3] = 1.0F;
                        _sceVu0MulMatrix( pmat , &( pctrl->pobjs->objs[pmdl->parent].world ) , pmat );
                }else{
                        punit->world = *(pctrl->pobjs->root);
                }
        }
}

/* Ｍ９２サイレンサー無し */
#define M92_SEA_ROT_X   ( ( 15.f * PI / 360 ) )                /* 回転角 */
#define M92_SEA_SLIDE_Y (50.00f)
static void BlowBack_M92_Sea( void *p )
{
        WEAPON_EF_CTRL *pctrl = (WEAPON_EF_CTRL *)p;
        FMATRIX *pmat;
        DG_OBJ        *punit = pctrl->pobjs->objs;
        DG_MDL        *pmdl;
        int motion;
        int i;

        motion = DIRECT_TICK( pctrl->motion );
        for(i = 0 ; i < pctrl->pobjs->n_models ; i++ , punit++ ){
                pmdl = punit->model;
                if(pmdl->parent >= 0){
                        pmat = &punit->world;
                        _sceVu0UnitMatrix( pmat );
                        switch( i ){
                        case 1 :                /* ハンマー */
                                if((pctrl->flag2 & WPEF_FLG2_SHOTFIRST)){        /* 装備直後 */
                                        if( motion == 0 ){ // 初期化
                                                _sceVu0RotMatrixX( pmat , pmat , 0.0f ); 
                                        }else{
                                                pctrl->flag2 &= (~WPEF_FLG2_SHOTFIRST);
                                        }
                                } else {
                                        _sceVu0RotMatrixX( pmat , pmat , ( M92_SEA_ROT_X * ( float )( motion % DIRECT_TICK( 4 ) ) )  );
                                }
                                break;
                        case 2:                /* スライド部分 */
                                if( motion < 0 ){
                                        pmat->m[ 3 ][ 1 ] = 0.0f;
                                }else if( motion < DIRECT_TICK( 4 ) ){
                                        pmat->m[ 3 ][ 1 ] = ( float )( M92_SEA_SLIDE_Y / DIRECT_TICK( 4 ) ) * ( float )motion;
                                }else{
                                        pmat->m[ 3 ][ 1 ] = ( M92_SEA_SLIDE_Y );
                                }
                                break;
                        default:
                                break;
                        }
                        pmat->m[3][0] = pmdl->tx;
                        pmat->m[3][1]+= pmdl->ty;
                        pmat->m[3][2] = pmdl->tz;
                        pmat->m[3][3] = 1.0F;
                        _sceVu0MulMatrix(pmat , &( pctrl->pobjs->objs[pmdl->parent].world ) , pmat );
                }else{
                        punit->world = *( pctrl->pobjs->root );
                }
        }
}

/* ショットガン ＳＰＳ */
static FVECTOR sps_shift_amo     = {  0.0f,-250.0f, 80.0f, 0.0f };
static void BlowBack_ShotGun(void *p)
{
        WEAPON_EF_CTRL *pctrl;
        FMATRIX world;
        FMATRIX *pmat;
        DG_OBJ        *punit;
        DG_MDL        *pmdl;
        FVECTOR        fvtemp0;
        FVECTOR        fvtemp1;
        FVECTOR speed;
        SVECTOR        svtemp;
        float        len;

        pctrl = (WEAPON_EF_CTRL *)p;
        punit = pctrl->pobjs->objs;
        pmdl  = punit->model;

        punit->world = *(pctrl->pobjs->root);

        DG_COPY_MAT( &world, pctrl->pobjs->root );
        DG_COPY_MAT( &punit->world, &world );

        punit++;
        pmdl=punit->model;
        pmat =&punit->world;
        _sceVu0UnitMatrix(pmat);

        pmat->m[3][0] = pmdl->tx;
        pmat->m[3][1] = pmdl->ty;
        pmat->m[3][2] = pmdl->tz;
        pmat->m[3][3] = 1.0F;
        _sceVu0MulMatrix(pmat,&(pctrl->pobjs->objs->world),pmat);

        /* 左手のモーションと連動するパーツ */
        if( pctrl->flag3 ){
                fvtemp0.vx = 0.0f;
                fvtemp0.vy = 1.0f;
                fvtemp0.vz = 0.0f;
                fvtemp0.vw = 0.0f;
                DG_SetPos( pmat );
                DG_RotVector( &fvtemp0, &fvtemp0, 1 );        // 武器の方向ベクトル

                // 関節間のベクトル
                _sceVu0SubVector( &fvtemp1, (FVECTOR *)pmat->m[3], (FVECTOR *)pctrl->root->objs->objs[HUMAN21_HIDARI_TE].world.m[3] ) ;
                len = _sceVu0InnerProduct( &fvtemp0, &fvtemp1 );

                if( pctrl->count == 0){
                        pctrl->ftemp = len;
                        /* ショットガンコッキングＳＥ */
                        GM_SeSetMode( SD_E_SHTREL01 , (FVECTOR *)pmat->m[3], GM_SEMODE_NORMAL );
                }else{
                        len -= pctrl->ftemp;
                        _sceVu0ScaleVector( &fvtemp1, &fvtemp0, len );
                        _sceVu0SubVector( (FVECTOR *)pmat->m[3], (FVECTOR *)pmat->m[3], &fvtemp1 ) ;
                }
                pctrl->count++;
                if( pctrl->count == 3 ){ // PALの時やや不安
                        OK_DirVecXY( &DG_ZeroVector, &fvtemp0, &svtemp );
                        svtemp.vx += 1024;

                        DG_SetPos( &world );
                        DG_MovePos( &sps_shift_amo );
                        DG_GetPos( &world );

                        fvtemp0.vx =-1.0f;
                        fvtemp0.vy = 0.0f;
                        fvtemp0.vz = 0.0f;
                        fvtemp0.vw = 0.0f;
                        speed.vx = 0.0f;
                        speed.vy = 0.0f;
#if 0	///madakore
                        speed.vz = rnd() * 2.0f;
#else
                        speed.vz = 1.0f;
#endif
                        speed.vw = 1.f;
                        DG_RotVector( &fvtemp0, &fvtemp0, 1 );
                        {
//                              extern void *NewSPS_EjectAmo( FVECTOR *pos, SVECTOR *rot, FVECTOR *vec );
//                              NewSPS_EjectAmo( (FVECTOR *)world.m[3], &svtemp, &fvtemp0 );
                                  CartridgeSPS( &world , NULL , NULL );
                        }

                }
                if( pctrl->count > DIRECT_TICK( 24 ) ) pctrl->flag3 = 0;
        }
}


