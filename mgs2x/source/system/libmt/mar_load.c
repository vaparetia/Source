//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	mar_load.c
	モーション読み込み＆初期化ルーチン

	1999/07/07 K.Takabe
	$Id: mar_load.c,v 1.1.1.3 2002/11/19 11:42:52 Yoshizawa1 Exp $

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

#include "BP_EndianSupport.h"

#ifdef KP_XBOX
#include <xtl.h>
#endif

#include	"libgv.h"
#include	"libdg.h"
#include	"libmt.h"

#define MAR_MAGIC_ID	(0x6152414d)	/* "MARa" */
#define MAR_PHYSICS_HEADER_LE ( (('P')<<0) | (('h')<<8) | (('y')<<16) | (('s')<<24) )

/*----------------------------------------------------------------*/

	/*
		モーションアーカイブファイルの取得
	*/
MAR_HEADER	*MT_GetMotionArchives( int motion_id )
{
	MAR_HEADER	*mar ;
	mar = GV_GetCache( GV_CacheID( motion_id, 'm' ) );
	if ( mar == NULL ) return (NULL);
	if ( mar->mar_id != MAR_MAGIC_ID ) return (NULL) ;
	return ( mar ) ;
}

static void EndianSwapMarHeader( MAR_HEADER *pData )
{
   BP_LE_SwapSInt_Inp( &pData->mar_id );
   BP_LE_SwapSInt_Inp( &pData->joints );
   BP_LE_SwapSInt_Inp( &pData->n_motion );
   BP_LE_SwapSInt_Inp( &pData->header_size );
}

static void EndianSwapMt3FileHeader( MT3_FILE_HEADER *pData )
{
   int i;
   int n_rots;

   BP_LE_SwapSInt_Inp( &pData->name_id );
   BP_LE_SwapSInt_Inp( &pData->flag );
   BP_LE_SwapSInt_Inp( &pData->motion_base_tick );
   BP_LE_SwapSInt_Inp( &pData->motion_length );
   BP_LE_SwapPtr_Inp( &pData->archives_data );
   BP_LE_SwapSInt_Inp( &pData->archives_size );
   BP_LE_SwapSInt_Inp( &pData->rot_units );
   BP_LE_SwapSInt_Inp( &pData->root_offset );
   BP_LE_SwapSInt_Inp( &pData->move_offset );
   BP_LE_SwapSInt_Inp( &pData->turn_offset );
   BP_LE_SwapSInt_Inp( &pData->fix_offset );
//   BP_LE_SwapSInt_Inp( &pData->turn_offset );

   // If this flag is set, rots_offset has twice as much 
   // to swap as you'd expect
   if ( pData->flag & MT3_FLAG_TRANS_PART )
   {
      n_rots = 2 * pData->rot_units;
   }
   else
   {
      n_rots = pData->rot_units;
   }

   BP_LE_SwapSIntArray_Inp( pData->rots_offset, n_rots );
}

/*----------------------------------------------------------------*/
	/*
		モーションデータの初期化
	*/
int MT_LoadInitMar( void *buf, int id )
{
	MAR_HEADER	*mar ;
	MT3_FILE_HEADER	*mt3 ;
	int			i ;

	if ( id & GV_CACHEID_RESIDENT ) return (1);
	mar = (MAR_HEADER*)buf ;

   // BP - This is a weird format.  Basically, if the mar header doesn't start with MAR_MAGIC_ID,
   // then this MAR file is just one MT3.  Otherwise it's an array of MT3 headers.  This means
   // that we can't swap the mar header until we determine if it's a MAR header

   if ( BP_LE_SwapUInt( *((unsigned *) buf ) ) == MAR_PHYSICS_HEADER_LE )
   {
      // BP HACK - .mdl files go through this path as well
      // These files are not MAR or MT3 files, and should not be endian swapped by 
      // these means, so PUNT

      return 1;
   }

	if ( BP_LE_SwapSInt( mar->mar_id ) != MAR_MAGIC_ID ){
		/* mt3単体ファイルの初期化 */
		mt3 = buf ;
      EndianSwapMt3FileHeader( mt3 );
		mt3->archives_data = (void*)( (int)mt3 + (int)mt3->archives_data ) ;
      // Now we'll endian swap the archives data
      BP_LE_SwapSShortArray_Inp( mt3->archives_data, mt3->archives_size );
		return (1) ;
	}

   EndianSwapMarHeader( mar );

	/* marの初期化 */
	for ( i = 0 ; i < mar->n_motion ; i++ ){
      BP_LE_SwapPtr_Inp( &( mar->data_table[i].addr ) );
      BP_LE_SwapSInt_Inp( &( mar->data_table[i].name_id ) );

      mt3 = (MT3_FILE_HEADER*)( (int)mar->data_table[ i ].addr + (int)buf + mar->header_size ) ;
      EndianSwapMt3FileHeader( mt3 );
		mar->data_table[ i ].addr = mt3 ;
		mar->data_table[ i ].name_id = mt3->name_id ;
		mt3->archives_data = (void*)( (int)mt3->archives_data + (int)mt3 );
      // Now we'll endian swap the archives data
      BP_LE_SwapSShortArray_Inp( mt3->archives_data, mt3->archives_size );
	}
	return (1);
}

static void EndianSwapSarHeader( SAR_HEADER *pData )
{
   BP_LE_SwapSInt_Inp( &pData->format_id );
   BP_LE_SwapSInt_Inp( &pData->n_datas );
   BP_LE_SwapSInt_Inp( &pData->data_size );
   BP_LE_SwapSIntArray_Inp( &( pData->pad[0] ), 5 );
}

static void EndianSwapSarList( SAR_LIST *pData )
{
   BP_LE_SwapSInt_Inp( &pData->id );
   BP_LE_SwapSInt_Inp( &pData->offset );
   BP_LE_SwapSInt_Inp( &pData->size );
   BP_LE_SwapSInt_Inp( &pData->local_header_size );
}

/*----------------------------------------------------------------*/
	/*
		シーケンスデータの初期化
	*/
int MT_LoadInitSar( void *buf, int id )
{
	SAR_HEADER	*sar ;
	SAR_LIST	*list ;
	unsigned char	*datas ;
	int			i ;

	if ( id & GV_CACHEID_RESIDENT ) return (1);
	sar = (SAR_HEADER*)buf ;
   EndianSwapSarHeader( sar );
	list = sar->list ;
	datas = (unsigned char*)&list[ sar->n_datas ] ;
	/* sarの初期化 */
	for ( i = 0 ; i < sar->n_datas ; i++ ){
		EndianSwapSarList( list );
      list->offset = (int)( datas + list->offset );
		list++ ;
	}

	return (1);
}

//----------------------------------------------------------------------------

static void EndianSwapFarHeader( FAR_HEADER *pData )
{
   BP_LE_SwapSInt_Inp( &( pData->format_type ) );
   BP_LE_SwapSInt_Inp( &( pData->id ) );
   BP_LE_SwapSInt_Inp( &( pData->n_joints ) );
   BP_LE_SwapSInt_Inp( &( pData->pad0[0] ) );
   BP_LE_SwapSInt_Inp( &( pData->max_pattern ) );
   BP_LE_SwapSInt_Inp( &( pData->max_mask ) );
   BP_LE_SwapSInt_Inp( &( pData->max_motion ) );
   BP_LE_SwapSInt_Inp( &( pData->pad1[0] ) );
   BP_LE_SwapPtr_Inp( &( pData->pattern_table ) );
   BP_LE_SwapSInt_Inp( &( pData->mask_offset ) );
   BP_LE_SwapPtr_Inp( &( pData->motion_table ) );
   BP_LE_SwapSInt_Inp( &( pData->pad2[0] ) );
}

/*----------------------------------------------------------------*/
	/*
		フェイスアニメーション用データの初期化
	*/
int MT_LoadInitFar( void *buf, int id )
{
	int			i ;

	if ( id & GV_CACHEID_RESIDENT ) return (1);
	if ( BP_LE_SwapSInt( *(int*)buf ) == FAR_FORMAT_TYPE ){
		FAR_HEADER		*header ;
		FAR_MOTION_INFO	*info ;

printf("init far\n");
		header = (FAR_HEADER*)buf ;

      EndianSwapFarHeader( header );

		header->pattern_table = (void*)( (int)header->pattern_table + (int)buf );
		header->motion_table = (void*)( (int)header->motion_table + (int)buf );
		info = header->motion_table ;
		for ( i = 0 ; i < header->max_motion ; info++, i++ ){
         int farIndex;
         FAR_MOTION_DATA *data;

         BP_LE_SwapSInt_Inp( &( info->id ) );
         BP_LE_SwapSInt_Inp( &( info->max_frame ) );
         BP_LE_SwapPtr_Inp( &( info->data ) );
         BP_LE_SwapSInt_Inp( &( info->pad ) );

         info->data = (void*)( (int)info->data + (int)buf );

         data = info->data;
         while (data->time != 0xffff)
         {
            BP_LE_SwapUShort_Inp( &data->time );
            ++data;
         }
		}

      BP_LE_SwapFloatArray_Inp( &( header->pattern_table->trans.vx ), 8*header->max_pattern*header->n_joints );

   } else if ( BP_LE_SwapSInt( *(int*)buf ) == FPK_FORMAT_TYPE ){
		FPK_HEADER	*header ;
		FPK_DATALIST	*list ;
#if JACK
      HANGUP(); // Hit FPK file?  I thought these were the stuff of legends.
#endif

printf("init fpk\n");
		header = (FPK_HEADER*)buf ;

      BP_LE_SwapSInt_Inp( &( header->format_type ) );
      BP_LE_SwapSInt_Inp( &( header->n_datas ) );
      BP_LE_SwapSInt_Inp( &( header->size ) );
      BP_LE_SwapSInt_Inp( &( header->data_offset ) );

      list = header->list ;
		for ( i = 0 ; i < header->n_datas ; list++, i++ ){
         BP_LE_SwapSInt_Inp( &( list->id ) );
         BP_LE_SwapPtr_Inp( &( list->data ) );
         BP_LE_SwapSInt_Inp( &( list->data_size ) );
         BP_LE_SwapSInt_Inp( &( list->n_joints ) );

         list->data = (short*)( (int)list->data + (int)buf + header->data_offset );

         // The data here is 7x shorts per joint.  I would go by data_size, but I 
         // can't find a place that reliably uses it, and I sure can't find a file
         BP_LE_SwapSShortArray_Inp( list->data, 7 * list->n_joints );
		}
#if 0
		{
			FVECTOR	qrots[55], trans[55] ;
			MT_DecodeFpkMotion( qrots, trans, buf, GV_StrCode("sna_face_speech_a") );
			for ( i = 0 ; i < 32 ; i++ ){
				printf("%d %f %f %f %f, %f %f %f\n", i, 
					   qrots[i].vx, qrots[i].vy, qrots[i].vz, qrots[i].vw,
					   trans[i].vx, trans[i].vy, trans[i].vz );
			}
		}
#endif
	}


	return (1);
}

static void swap_floats_and_inc( char **ppData, int byte_size )
{
   BP_LE_SwapFloatArray_Inp( (float *) (*ppData), byte_size / 4 );
   (*ppData) += byte_size;
}

void RMT_VerifyEndianSwap( RMT_FILE_HEADER *pData )
{
   char *data_char;
   // We keep pData->flags unswapped
   if ( !( BP_LE_SwapSInt( pData->flags_le ) & RMT_FLAG_ENDIAN_SWAPPED ) )
   {
      pData->flags_le |= BP_LE_SwapSInt( RMT_FLAG_ENDIAN_SWAPPED );

      // The rest of the struct is ints, so let's just cheat
      BP_LE_SwapSIntArray_Inp( ((int *) pData ) + 1, ( sizeof( *pData ) / sizeof(int) ) - 1 );

      // Now we need to swap the actual data
      data_char = (char *)( pData + 1 );

      swap_floats_and_inc( &data_char, pData->move_size );
      swap_floats_and_inc( &data_char, pData->rots_size );
      swap_floats_and_inc( &data_char, pData->trans_size );
   }
}


/*----------------------------------------------------------------*/
	/*
		ＬＩＢＭＴ関連初期化
	*/
void MT_Initialize( void )
{
	/*
		ロード初期化ルーチンの登録
	*/
	GV_SetLoader( 'm', (GV_LOADFUNC)MT_LoadInitMar );
	GV_SetLoader( 's', (GV_LOADFUNC)MT_LoadInitSar );
	GV_SetLoader( 'f', (GV_LOADFUNC)MT_LoadInitFar );


	/*
		効果音変換リスト初期化
	*/
	MT_InitExchangeSeList();
}
