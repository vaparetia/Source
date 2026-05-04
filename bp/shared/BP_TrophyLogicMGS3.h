//----------------------------------------------------------------------------
// BP_TrophyLogicMGS3.h
//
// Custom game logic for unlocking trophies
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

   //----------------------------------------------------------------------------

   void bp_trophy_begin_stream( int top );
   void bp_trophy_end_stream( int top );
   void bp_trophy_stream_update( const char * const streamName, const int streamTick, const float manualZoom, const float fpvRotX, const float fpvRotY, const int fpv );

   //custom events

   // Defined in inventory.c due to annoying gae dependencies
   void BP_Trophy_MGS3_CamoCheck( unsigned int bonusFlags );
   void BP_Trophy_MGS3_FaceCheck( unsigned int bonusFlags );

   //----------------------------------------------------------------------------

#ifdef __cplusplus
};
#endif
