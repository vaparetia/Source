/*
  cdcobj.c
  source/game/object.c の、無線特化版。
  無線の場合はファイル空間が異なるため、こちらの関数を用いる。

  2001/04/06  Y.Kira
  $Id: cdcobj.c,v 1.1.1.3 2002/11/19 11:45:05 Yoshizawa1 Exp $
*/
/*
  基本的にファイルを参照して構造体を作成する段階が異なるわけで、
  作成されてからの操作(開放以外)は共通とすることが出来る。
 */
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

#include "libgv.h"
#include "libdg.h"
#include "libmt.h"
#include "libhzx.h"
#include "g_struct.h"
#include "g_extern.h"
#include "g_macro.h"
#include "codecmtn.h"
#include "cdc_load.h"

/*----------------------------------------------------------------*/
void CDC_InitObject( OBJECT *object, int model, int flag )
{
  DG_DEF *def ;
  
  /*
    model == 0 は禁止
  */
  ASSERT( model != 0 ) ;

  /* 骨格のロード(通常ファイル空間から) */
  def = (DG_DEF*)GV_GetCache( GV_CacheID( model, 'k' ) );

  /* 見付からない場合は、無線ファイル空間から探す */
  if(NULL == def)
    def = (DG_DEF *)CDC_GetFileEntry(model, 'k');
 
#ifdef DEBUG_MODE
  if ( def == NULL ) {
    printf( "model %d not found\n", model ) ;
    ASSERT( 0 ) ;	    
  }
#endif
  object->objs = DG_MakeObjs( def, flag, 0 );
#ifdef DEBUG_MODE
  if ( object->objs == NULL ) {
    printf( "warning : model %x make objs failed\n", model ) ;
  }
#endif
  DG_QueueObjs( object->objs );
  object->flag = 0 ;
  object->map_name = 0x7fffffff ;
}

void CDC_FreeObject( OBJECT *object )
{
  if ( object->m_ctrl != NULL ){
    CDC_FreeMotion( object->m_ctrl );
    object->m_ctrl = NULL ;
  }
  if ( object->objs != NULL ){
    DG_DequeueObjs( object->objs ) ;
    DG_FreeObjs( object->objs );
    object->objs = NULL ;
  }
  if ( object->evmobj != NULL ){
    DG_DequeueEvmObj( object->evmobj );
    DG_FreeEvmObj( object->evmobj );
    object->evmobj = NULL ;
  }
} 


void CDC_ConfigObjectEvm( OBJECT *object, int model, int flag, int chanl )
{
  EVM_DEF		*def ;
  DG_EVMOBJ	*evmobj ;
  
  /*
    if ( ( def = GV_GetCache( GV_CacheID( model, 'e' ) ) ) == NULL )
    ASSERT( 0 ) ;
  */
  def = CDC_LoadEvmDef(model);
  ASSERT(def != NULL);
  
  object->evmobj = evmobj = DG_MakeEvmObj( def, flag, chanl );
  if ( evmobj != NULL ) DG_QueueEvmObj( evmobj );
}

void CDC_ConfigObjectMotion( OBJECT *object,
			     int n_layer, int motion, int flag )
{
  if ( object->m_ctrl != NULL ){
    CDC_FreeMotion( object->m_ctrl );
  }
  if ( n_layer <= 0 ) n_layer = 1 ;
  object->m_ctrl = CDC_InitMotion( object->objs, n_layer, motion, flag );
}
