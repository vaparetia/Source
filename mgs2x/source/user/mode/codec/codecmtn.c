//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
 * codecmtn.c
 * 無線用モーションロード/初期化関数
 *
 * 2000/11/03  Y.Kira
 * $Id: codecmtn.c,v 1.1.1.3 2002/11/19 11:45:01 Yoshizawa1 Exp $
 */
#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#ifdef KP_XBOX
#include <xtl.h>
#endif

#include "gameheader.h"
#include "codec.h"

#include "libfs.h"
#include "libdg.h"
#include "libmt.h"
#include "libutl.h"
#include "fmt_demo.h"

#define _codecmtn_c_
#include "codecmtn.h"
#include "cdc_load.h"
#include "codecmem.h"
#include "codec_config.h"

#ifdef PSX2
#ifndef _DEBUG_
#define DBG(...)
#else
#define DBG(...)   printf(__VA_ARGS__)
#endif /* _DEBUG_ */
#endif
#ifdef KP_XBOX
#define DBG
#endif


#define MALLOC(size)   codecMalloc((size))
#define FREE(ptr)      codecDelayedFree((ptr))

/*
#define MALLOC(SIZE)   GV_Malloc((size))
#define FREE(ptr)      GV_Free((ptr))
*/

void* CDC_InitSequence( int n_layer, int id, int flag )
{
	SAR_CONTROL *sar_ctrl ;
	SAR_HEADER	*header ;
	int			size, i ;

	header = CDC_GetFileEntry(id, 's');
	if ( header == NULL ) return( NULL ) ;

	size = sizeof(SAR_CONTROL) + sizeof(SAR_INST) * n_layer ;
	sar_ctrl = MALLOC( size );
	DBG("CDC_InitSequence(): MALLOC: %p (size + %d)\n", sar_ctrl, size);
	GV_ZeroMemory( sar_ctrl, size );

	sar_ctrl->header = header ;
	sar_ctrl->n_instance = n_layer ;

	/* データタイプチェック */
	if ( header->format_id == SAR_FORMATID_SEV ) sar_ctrl->type = 1 ;

	for ( i = 0 ; i < n_layer ; i++ ) sar_ctrl->instance[i].base_tick = TIME_BASE ;

	return ( sar_ctrl );
}

void CDC_FreeSequence( SAR_CONTROL *sar_ctrl )
{
	if ( sar_ctrl == NULL ) return ;
	FREE( sar_ctrl );
	DBG("CDC_FreeSequence(): FREE: %p\n", sar_ctrl);
}


	/*
		モーションアーカイブファイルの取得
	*/
MAR_HEADER	*CDC_GetMotionArchives( int motion_id )
{
	MAR_HEADER	*mar ;
	mar = CDC_GetFileEntry(motion_id, 'm');

	if ( mar == NULL ) return (NULL);
	if ( mar->mar_id != MAR_MAGIC_ID ) return (NULL) ;
	return ( mar ) ;
}

	/*
		モーション用ワークの初期化
	*/
void *CDC_InitMotion( DG_OBJS *objs, int n_layer, int motion, int flag )
{
  MOTION_CONTROL	*m_ctrl ;
  int		n_joints, size, i ;
  char	*mem ;
  MAR_HEADER		*mar_header ;
  
  if ( objs == NULL ){
    DBG("motion init faild!! -- no object\n");
    return ( NULL ) ;
  }
  
  /* モーションデータの取得 */
  mar_header = CDC_GetMotionArchives( motion );
  if ( mar_header == NULL ){
    DBG("motion init faild!! -- no motions file\n");
    return ( NULL ) ;
  }
  
  n_joints = mar_header->joints ;
  
  /* メモリの確保 */
  size = ( sizeof(MOTION_CONTROL)
	   + sizeof(MT3_CONTROL) * n_layer
	   + sizeof(MT3_MOVE_SEGMENT) * n_layer
	   + sizeof(MT3_MOVE_SEGMENT2) * n_layer
	   + sizeof(MT3_ROOT_SEGMENT) * n_layer
	   + sizeof(MT3_FIX_SEGMENT) * n_layer
	   + sizeof(MT3_TURN_SEGMENT) * n_layer
	   + sizeof(MT3_JOINT_SEGMENT) * n_joints * n_layer
	   + sizeof(FVECTOR) * n_joints * 4 /* rots + adjust + abs_rots + old_abs_rots */
	   );
  mem = MALLOC( size );
  DBG("CDC_InitMotion(): MALLOC: %p(size = %d)\n", mem, size);
  if(NULL == mem) return NULL;
  GV_ZeroMemory( mem, size );
  size = 0 ;
  m_ctrl = mem ;
  size += sizeof(MOTION_CONTROL) ;
  m_ctrl->mt3_ctrl = mem + size ;
  size += sizeof(MT3_CONTROL) * n_layer ;
  m_ctrl->rots = mem + size ;
  size += sizeof(FVECTOR) * n_joints ;
  m_ctrl->adjust = mem + size ;
  size += sizeof(FVECTOR) * n_joints ;
  m_ctrl->abs_rots = mem + size ;
  size += sizeof(FVECTOR) * n_joints ;
  m_ctrl->old_abs_rots = mem + size ;
  size += sizeof(FVECTOR) * n_joints ;
  
  /* ワークの初期化 */
  m_ctrl->motion_arc = mar_header ;
  m_ctrl->n_layer = n_layer ;
  m_ctrl->n_joints = n_joints ;
  m_ctrl->flag = flag ;
  for ( i = 0 ; i < n_layer ; i++ ){
    if ( i == 0 ){
      /* 第１レイヤのモーションのみ有効 */
      m_ctrl->mt3_ctrl[i].move = mem + size ; size += sizeof(MT3_MOVE_SEGMENT);
      m_ctrl->mt3_ctrl[i].move2 = mem + size ; size += sizeof(MT3_MOVE_SEGMENT2);
      m_ctrl->mt3_ctrl[i].root = mem + size ; size += sizeof(MT3_ROOT_SEGMENT);
      m_ctrl->mt3_ctrl[i].fix = mem + size ; size += sizeof(MT3_FIX_SEGMENT);
      m_ctrl->mt3_ctrl[i].turn = mem + size ; size += sizeof(MT3_TURN_SEGMENT);
    }
    m_ctrl->mt3_ctrl[i].motion_arc = m_ctrl->motion_arc ;
    m_ctrl->mt3_ctrl[i].joints = mem + size ; size += sizeof(MT3_JOINT_SEGMENT) * n_joints ;
    m_ctrl->mt3_ctrl[i].rots = m_ctrl->rots ;
    m_ctrl->mt3_ctrl[i].motion_num = -1 ;
    m_ctrl->mt3_ctrl[i].play_time_base = (float)TIME_BASE ;
  }
  
  /* ＳＥシーケンス設定 */
  m_ctrl->sar_ctrl = CDC_InitSequence( n_layer, motion, 0 );
  m_ctrl->se_table_id = -1 ;
  if ( m_ctrl->sar_ctrl != NULL ){
    m_ctrl->sar_ctrl->param1 = (int)objs ;
  }
  return (m_ctrl);
}

	/*
		モーション用ワークを開放する
	*/
void CDC_FreeMotion( MOTION_CONTROL *m_ctrl )
{
	/* ＳＥシーケンス設定 */
  CDC_FreeSequence( m_ctrl->sar_ctrl );
  FREE( m_ctrl );
  DBG("CDC_FreeMotion(): FREE: %p\n", m_ctrl);
} 
