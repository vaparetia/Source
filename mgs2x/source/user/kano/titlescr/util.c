//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

  ユーティリティ
  2002/06/14 M.Kobayashi
  $Id: util.c,v 1.1.1.3 2002/11/19 11:43:42 Yoshizawa1 Exp $

*/

#include "titlescr.h"
#include "../../yamashita/2D/msn.h"



float TTL_CalcVRAchievementRatio( void )
{
	int i, j, k;
	int nstage = 0;
	int nclear = 0;

	for( i = 0 ; i < MSN_MISSION_MAX ; i++ ) {
		for( j = 0 ; j < MSN_MODE_MAX ; j++ ) {
			for( k = 0 ; k < MSN_WEAPON_MAX; k++ ) {
				int nclearlocal;
				nstage += Msn_GetStageCount( i, j, k, MSN_PLAYER_ALL, &nclearlocal );
				nclear += nclearlocal;
			}
		}
	}
	return nclear * 100.f / (float)nstage;
}

int BP_TTL_CalcTotalVRStageCount( void )
{
   int i, j, k;
   int nstage = 0;
   int nclear = 0;

   for( i = 0 ; i < MSN_MISSION_MAX ; i++ ) {
      for( j = 0 ; j < MSN_MODE_MAX ; j++ ) {
         for( k = 0 ; k < MSN_WEAPON_MAX; k++ ) {
            int nclearlocal;
            nstage += Msn_GetStageCount( i, j, k, MSN_PLAYER_ALL, &nclearlocal );
            nclear += nclearlocal;
         }
      }
   }
   //511 :)
   return nstage;
}

float Transfarring_CalcVRAchievementRatio( const char* vrScoreDataBuffer )
{
   int i, j, k;
   int nstage = 0;
   int nclear = 0;

   for( i = 0 ; i < MSN_MISSION_MAX ; i++ ) {
      for( j = 0 ; j < MSN_MODE_MAX ; j++ ) {
         for( k = 0 ; k < MSN_WEAPON_MAX; k++ ) {
            int nclearlocal;
            nstage += Transfarring_Msn_GetStageCount( vrScoreDataBuffer, i, j, k, MSN_PLAYER_ALL, &nclearlocal );
            nclear += nclearlocal;
         }
      }
   }
   return nclear * 100.f / (float)nstage;
}

// command セーブ用参照変数登録[ComSetReferenceData] $r:変数 ...

GCL_VAR_REF	TTL_ReferenceVariable[ TTL_MAX_VARREF ];

int ComSetReferenceData( void )
{
	int i;
	for( i = 0 ; i < TTL_MAX_VARREF ; i ++ ) {
		if( GCL_NextStr() == NULL ) break;
		GCL_GetNextVarRef( &TTL_ReferenceVariable[i] );
		printf("ComSetReferenceData: %d\n", GCL_ReadVarRef( &TTL_ReferenceVariable[i], 0 ));
	}
	return 1;
}

unsigned int BP_GetGclVarUnsignedInt( const unsigned int code )
{
   char codeBuf[4] = { (code>>24), (code>>16)&0xff, (code>>8)&0xff, code&0xff };
   int varType = 0;
   int varValue = 0;
   GCL_GetVar( codeBuf, &varType, &varValue );
   return (unsigned int)varValue;
}

unsigned int BP_GetSnakeTalesClearGcl()
{
   unsigned int st_clear_flag = 0;
   int i;

   const unsigned char skSnakeTalesClearVarCodes[MCMAN_MAX_TALES][4] =
   {
      { 0x14, 0x05, 0x01, 0xDD },  //テイルズＡクリアしたフラグ
      { 0x14, 0x06, 0x01, 0xDD },  //テイルズＢクリアしたフラグ
      { 0x14, 0x07, 0x01, 0xDD },  //テイルズＣクリアしたフラグ
      { 0x14, 0x00, 0x02, 0x62 },  //テイルズＤクリアしたフラグ
      { 0x14, 0x01, 0x02, 0x62 },  //テイルズＥクリアしたフラグ
   };
   for( i = 0 ; i < MCMAN_MAX_TALES ; i++ )
   {
      int varType = 0;
      int varValue = 0;
      GCL_GetVar( (char*)skSnakeTalesClearVarCodes[i], &varType, &varValue );
      if( varValue )
      {
         st_clear_flag |= (1<<i);
      }
   }
   return st_clear_flag;
}

unsigned int BP_GetMissionNumberGcl()
{
   return BP_GetGclVarUnsignedInt( 0x1200022c ); //ミッション番号
}

unsigned int BP_GetSelectedPlayerGcl()
{
   return BP_GetGclVarUnsignedInt( 0x16000234 ); //選択プレイヤー
}

unsigned int BP_GetDemoTheaterFlagGcl()
{
   return BP_GetGclVarUnsignedInt( 0x140001DD ); //ローポリ劇場フラグ
}
