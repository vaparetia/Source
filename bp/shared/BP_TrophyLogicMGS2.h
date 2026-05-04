//----------------------------------------------------------------------------
// BP_TrophyLogicMGS2.h
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
void bp_trophy_update_stream_pan( int top, float angle, float volume );
void bp_trophy_end_stream( int top );
void bp_trophy_stream_update( const char * const streamName, const int streamTick, const float manualZoom, const float manualX, const float manualY );
void bp_trophy_new2dsprite( unsigned int texCode );
void bp_trophy_vr_mission_clear( const int stage );
void bp_trophy_snake_tales_clear( const int varcode );
void bp_trophy_demo_theater_play( const int argc, const int * const argv );

//custom events
void bp_trophy_skipped_cinematic();
void bp_trophy_destroyed_radio();
void bp_trophy_destroyed_camera();
void bp_trophy_tranqed_enemy();
void bp_trophy_broke_enemy_neck();
void bp_trophy_ko_enemy();
void bp_trophy_steam_enemy();

void bp_trophy_set_tanker_cleared_by_user();
void bp_trophy_set_plant_cleared_by_user();
int bp_trophy_get_tanker_cleared_by_user();
int bp_trophy_get_plant_cleared_by_user();

int bp_trophy_check_warning_cleared_by_user();

//----------------------------------------------------------------------------

#ifdef __cplusplus
};
#endif
