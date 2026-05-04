//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	サウンドストリーミングドライバ

	2000/07/04 K.Uehara
	$Id: sdstream.c,v 1.6 2002/12/01 12:00:53 Yoshizawa1 Exp $
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>
#ifdef PSX2
#include	<eekernel.h>
#include	<eeregs.h>
#include	<libgraph.h>
#endif

#include	"libgv.h"
#include	"libgv.cnf"
#include	"libfs.h"
#include	"libdg.cnf"
#include	"stream.h"
#include	"g_define.h"
#include	"linkvar.h"

#include	"sd_ee.h"
#include	"g_sound.h"

#include	"strctrl.h"

#include "BP_AudioStream.h"
#include "BP_EndianSupport.h"
#include "BP_SoundSupport.h"
#include "BP_TrophyLogicMGS2.h"

#undef KP_XBOX      //BP_SOUND

typedef struct _sdstream_Work {
	GV_ACT_EX actor;
	int type;
	GM_STREAM_CONTROL *ctrl;

	void *stream_h;
	void *sd_ptr;
	int sd_tr_block_count;
	int sd_tr_count;
	int sd_size;
	int sd_ch;
	int sd_duration;
	int sd_fmt;
} Work;

#ifdef DEBUG_MODE
static Work *sd_work;
#endif

static int sd_used_channel = 0;
int pause_channel_flag = 0;

extern GM_STREAM_CONTROL* BP_GetStreamControl( unsigned int stream );

extern int bp_get_str2_stream_surround_type( const int top_pos, const int flag, const int is_stereo );
extern unsigned long se_pant[0x41];

//BP - added these to allow passing along extra data that doesn't fit in their 4-byte command format
extern float bp_cli_float_param;
extern int bp_cli_int_param;
extern void * bp_cli_ptr_param;

static int is_bp_audio[2] = { 0 };           //of the two channels, either could be BP audio
static int const kInvalidVolumes[2][2] = { -1, -1, -1, -1 };

static inline int get_free_channel( void )
{
	if( !( sd_used_channel & 1 ) ){
		sd_used_channel |= 1;
		return 0;
	} else if( !( sd_used_channel & 2 ) ){
		sd_used_channel |= 2;
		return 1;
	}
	return -1;
}

static inline void free_channel( int channel )
{
	sd_used_channel &= ~( 1 << channel );
}

int GM_StreamSdGetChannel( void )
{
	return get_free_channel();
}

int GM_StreamSdGetChannelNo( int no )
{
	if( no < 0 ){
		return get_free_channel();
	}
	if( ( sd_used_channel & ( 1 << no ) ) != 0 ){
		// 使用中
printf( "USED CHANNEL %X %d\n", sd_used_channel, no ) ;
		return -1;
	}
	sd_used_channel |= ( 1 << no );
	return no;
}

void GM_StreamSdFreeChannel( int channel )
{
	free_channel( channel );
}

static void destroy( Work *work );

/* ---------------------------------------------------------------------- */
/*
	tickの更新処理
*/

static void update_tick( Work *work, int *status )
{
	int sd_tick;
	int channel;
	int playing;
   int first_load;
   int bp_audio = is_bp_audio[work->ctrl->sd_channel];

	channel = work->ctrl->sd_channel;

   if( bp_audio )
   {
      int status = BP_AudioStreamGetStatus( work->ctrl->sd_channel );
      playing = (status >= 4);
      first_load = status < 4;
      if( status == 3 ) //KCEJSD_STST_READY
      {
         BP_AudioStreamPlay( work->ctrl->sd_channel );
      }
   }
   else
   {
   	playing = ( status[1] & ( 0x00000020 << work->ctrl->sd_channel ) );
      first_load = ( status[ 1 ] & ( 0x08 << channel ) );
   }

	if( !playing ){
		sd_tick = 0;
	} else {
      if( bp_audio )
      {
         //gets value in ms
         sd_tick = BP_AudioStreamGetPlayTime( work->ctrl->sd_channel );
         //convert to 200ths of a second
         sd_tick /= 5;
      }
      else
      {
   		sd_tick = status[ 4 + channel ];
      }
	}

	if( work->ctrl->tick < 0 && first_load ){
		work->ctrl->tick = 0;
	} else if( sd_tick > 0 ){
		int tick;
		tick = ( (float)sd_tick * 3.0F ) / 2.0F;
		work->ctrl->sd_tick = tick;
		while( ( tick - work->ctrl->tick ) > BASE_TICK ){
			work->ctrl->tick += BASE_TICK;
		}
		if( tick > work->ctrl->tick ){
			work->ctrl->tick += BASE_TICK;
		}
	} else if( work->ctrl->tick > 0 && playing == 0 ){
		/* 一度でもサウンドを再生したが, その後sd_tickが終了した */
		destroy( work );
	}
}

/* ---------------------------------------------------------------------- */
/*
	処理実体
*/

static void Die( Work *work )
{
	if( GV_IsFollowDestroy( work ) || GV_IsStageDestroy( work ) ){
		// 親から強制終了またはステージ終了
		// 強制停止
		pause_channel_flag &= ~( 1 << work->ctrl->sd_channel );
	}
	if( work->ctrl->sd_channel == 0 ){
		sd_set_cli( 0xFF00000A );
	} else {
		sd_set_cli( 0xFF00000E );
	}
   if( is_bp_audio[work->ctrl->sd_channel] )
   {
      BP_AudioStreamStop( work->ctrl->sd_channel, 0 );
      BP_AudioStreamSetVolume_Voice(work->ctrl->sd_channel, kInvalidVolumes, 0.0f, 0.0f );
   }
	work->ctrl->sd_duration = 0;
	work->ctrl->sd_tick = -1;
#ifdef DEBUG_MODE
	sd_work = NULL;
#endif
}

static void DieWait( Work *work )
{
   int playing;
   int bp_audio = is_bp_audio[work->ctrl->sd_channel];

	update_tick( work, sd_status() );

   if( bp_audio )
   {
      playing = BP_AudioStreamGetStatus( work->ctrl->sd_channel ) >= 4;
   }
   else
   {
      playing = ( sd_status()[1] & ( 0x00000020 << work->ctrl->sd_channel ) );
   }

	// なっている間は終了しない。
	if( !playing
//		&& !( pause_channel_flag & ( 1 << work->ctrl->sd_channel ) )
		){
		GV_DestroyActor( work );
#if 0
		if( work->sd_fmt == 1 ){
			sd_set_cli( 0xFF00000E );
		}
#endif
	}
}

static void destroy( Work *work )
{
	GV_ChangeActFunc( work, DieWait );
}

static inline int is_data_set( int play_ofs, Work *work, const int size )
{
   if( is_bp_audio[work->ctrl->sd_channel] )
   {
      //play_ofs is simply the amount of bytes into the stream we're allowed
      //to have buffered at this point
//      printf("is_data_set: %d > %d + %d?\n", play_ofs, work->sd_tr_count, size );
      return play_ofs > work->sd_tr_count + size;
   }
	if( work->sd_ch > 1 ){
		if( play_ofs + 0x4000 * work->sd_ch > work->sd_tr_count ){
			return 1;
		}
	} else {
		if( play_ofs + 0x4000 + 0x800 > work->sd_tr_count ){
			return 1;
		}
	}
	return 0;
}

static void send_sound_data( Work *work, const int *status )
{
	int play_ofs;
	int count;
	int channel;
   void *ptr;
   int size;

	channel = work->ctrl->sd_channel;

   if( is_bp_audio[channel] )
   {
      play_ofs = BP_AudioStreamGetRequest( channel );
   }
   else
   {
	   if( channel == 0 ){
		   play_ofs = status[ 2 ];
		   count = ( status[ 6 ] & 0xFFFF );
	   } else {
		   play_ofs = status[ 3 ];
		   count = ( status[ 7 ] & 0xFFFF );
	   }
   //printf( "COUNT = %d, tr = %d\n", count, work->sd_tr_block_count );
	   if( count != work->sd_tr_block_count ){
		   // ちょっと危険だが。
		   return;
	   }
      play_ofs = play_ofs * work->sd_ch;
   }

   //BP - changed so that it always takes the top packet if there is one,
   //checks the size, and uses that to determine whether to proceed
   //rather than assume that all audio packets are 0x4000 in size as the
   //original ones are.
   if( ( ptr = FS_StreamGetData( work->stream_h, work->type ) ) != NULL ){
      size = FS_StreamGetSize( work->stream_h, ptr );
      if( !is_data_set( play_ofs, work, size ) ){
         //BP - no room to copy out this data.  Put it back for now.
         FS_StreamUngetData( work->stream_h, ptr );
         return;
      }
      else {
         //BP - copy out the data.

#ifdef PSX2
			SyncDCache( ptr, ptr + size );
#endif
#ifdef PSX2
         //BP - note that the old sd_set_cli path assumes that the high 4 bits
         //of pointers are empty and that it's safe to OR with a command code
         //after shifting right 4 bits.  Guess what, not true if you're not on PS2
         //or apparently XBOX1 or Win32. :(
         //I just ignore the parm on the other end and instead set it directly here
         //with an extern global.
#if 1 //BP_PS2
         if( is_bp_audio[channel] )
         {
            int sizeUnaligned; // Armature - Size of the stream packet without padding

#if defined(BP_VITA)
            // On Vita, the sound system expects the size of the packet to not include padding. This is consistent with
            // MGS3 which appears to give the size without padding already. For BP_Audio streams, the option field 
            // contains the size without padding.
            sizeUnaligned = FS_STREAM_GET_OPTION(ptr);
            BP_AudioStreamSetPacket(channel, ptr, sizeUnaligned);
#else
            BP_AudioStreamSetPacket( channel, ptr, size );
#endif
         }
         else
         {
            bp_cli_ptr_param = ptr;
            if( channel == 0 ){
               sd_set_cli( 0xF1000000 | ( (( int )(ptr)&0x0FFFFFFF) >> 4 ) );
            } else {
               sd_set_cli( 0xF3000000 | ( (( int )(ptr)&0x0FFFFFFF) >> 4 ) );
            }
         }
#else
			if( channel == 0 ){
				sd_set_cli( 0xF1000000 | ( ( int )ptr >> 4 ) );
			} else {
				sd_set_cli( 0xF3000000 | ( ( int )ptr >> 4 ) );
			}
#endif
#else		// 24 bit だと足りない
			if( channel == 0 ){
				sd_set_cli( 0xF1000000 | ( ( u_int )ptr >> 16 ) );
				sd_set_cli( 0xF1100000 | ( (( u_int )ptr) & 0xffff ) );
			} else {
				sd_set_cli( 0xF3000000 | ( ( u_int )ptr >> 16 ) );
				sd_set_cli( 0xF3100000 | ( (( u_int )ptr) & 0xffff ) );
			}
#endif			
			work->sd_tr_count += size;
			work->sd_ptr = ptr;
			work->sd_size -= size;
			work->sd_tr_block_count ++;
//@			printf( "GET %X\n", work->ctrl->sd_tick );
      }
   }
   else {
      //BP - no point spamming the terminal any more since we don't even check
      //whether we could copy data across until we've already acquired it.
      //printf( "CAN'T GET %X\n", work->ctrl->sd_tick );
      //			FS_StreamDump( work->stream_h );
      //			HANGUP();
   }
}

static int is_sended_data( Work *work, int *status )
{
   if( is_bp_audio[work->ctrl->sd_channel] )
   {
      //Transfers are synchronous in BP_AudioStreams.
      //(well, so are the below transfers not on PS2...)
      return 1;
   }
   else
   {
	   unsigned int mask = ( ( 0x1000 << work->ctrl->sd_channel ) | 0x80000000 );

	   if( !( status[ 1 ] & mask ) ){
		   return 1;
	   } else {
		   return 0;
	   }
   }
}

static void Act( Work *work )
{
	int *status;

	status = sd_status();

	update_tick( work, status );
	if( work->sd_ptr != NULL ){
		if( is_sended_data( work, status ) ){
			FS_StreamFreeData( work->stream_h, work->sd_ptr );
			work->sd_ptr = NULL;
		} else {
			return;
		}
		if( work->sd_size <= 0 ){
			destroy( work );
			return;
		}
	}
	send_sound_data( work, status );

	/* 300 baseのtickに変換 */
}

static int SoundStreamInit( Work *work )
{
	/*
		この関数が読み込まれている時はすでにメモリ上に
		初期転送分が読み込まれている
	*/

	void *ptr;

	if( ( ptr = FS_StreamGetData( work->stream_h, work->type ) ) != NULL )
   {
      int ch, size, fmt;
      if( is_bp_audio[work->ctrl->sd_channel] )
      {
         BP_AudioStreamHeader *pHeader = ptr;
         int bp_stream_type = 0;
         int bp_surround_type = 0;

         pHeader->mChannels = pHeader->mChannels & 0xffff;

         switch( work->ctrl->tagflag & (0xf0000000&(~TAGFLAG_SOUND_AC3)) )
         {
         case TAGFLAG_SOUND_VAG:
            //original stream was a str2/vag stream.
            bp_stream_type = BP_DOSTREAM_TYPE_VOICE;
            break;
         case TAGFLAG_SOUND_8BIT:
            //original stream was a str3/lnr8 stream.
            bp_stream_type = BP_DOSTREAM_TYPE_DIRECT;
            break;
         default:
            bp_stream_type = BP_DOSTREAM_TYPE_VOICE;
            break;
         }

         bp_surround_type = bp_get_str2_stream_surround_type( work->ctrl->top_pos, work->ctrl->flag, pHeader->mChannels == 2 );
         BP_AudioStreamOpen( work->ctrl->sd_channel, pHeader, bp_stream_type, bp_surround_type, work->ctrl->top_pos );
         ch = pHeader->mChannels;

#if !defined(BP_VITA)
         size = pHeader->mSampleSize;

#if defined(BP_360)
         //N.B. size set in the audio stream header is the total size of xwm data, before padding to 16 byte alignment.
         //This is not the total size of sound packet body data, however, and that's what this is expected to be.
         //Calculate the correct size on the fly here.
         {
            int audioPacketCount = pHeader->mSampleSize / pHeader->mBlockAlign;
            size = audioPacketCount * ( (pHeader->mBlockAlign + 15) & (~15) );
         }
#endif

#else
         size = pHeader->mStreamDataSize;
#endif

         fmt = -1;   //unused
         //BP_TODO: this calculation is wrong unless the stream format is 16-bit LPCM.
         //Doesn't matter though as sd_duration is only ever checked for nonzero.
         work->ctrl->sd_duration = ( (float)size / sizeof(short) ) * 300.0F / ( pHeader->mFrequency * pHeader->mChannels );

         //BP - initialize pan and volume AFTER the stream has been opened so the channel count is correct.
         {
            int currVols[2][2];
            BP_AudioStreamGetVolume_Voice(&currVols[0][0], work->ctrl->sd_channel);

            // Only set the volumes if they have not already been set. The volume is reset when a stream is stopped so if the 
            // game sets the volume before telling the stream to play, don't overwrite the settings.
            if ((!memcmp(&kInvalidVolumes[0][0], &currVols[0][0], sizeof(kInvalidVolumes))) || 
                  bp_surround_type != BP_SURROUND_VOICE_TYPE_SURROUND)
            {
               GM_StreamSdSetPan( work->ctrl->sd_channel, 0x0000203F, 0.f );
            }
         }
      }
      else
      {
         int pitch;
		   unsigned char *p = ptr;

		   size = ( p[ 0 ] << 24 ) | ( p[ 1 ] << 16 ) | ( p[ 2 ] << 8 ) | ( p[ 3 ] );
		   pitch = ( p[ 6 ] << 8 ) | ( p[ 7 ] );
		   ch = p[ 8 ];
		   fmt = p[ 10 ];

		   printf( "size %d pitch %d ch %d fmt %d\n", size, pitch, ch, fmt );

         //BP - sound system also needs to know flag and top_pos to figure out surround settings.
         bp_cli_int_param = work->ctrl->flag;
         bp_cli_ptr_param = (void*)(work->ctrl->top_pos);

		   if( work->ctrl->sd_channel == 0 ){
            
            if( fmt == 1 )
            {
               printf("ERROR: Invalid stream format for channel 0: 0x%8.8x\n", work->ctrl->tagflag);

               // Trying to play wrong format (LNR8/STR3) stream on channel 0, see bp_apply_tagflag_hacks in strctrl.c
               // This happens because the stream "tagflag" gets passed by scripting, and the stream format has changed 
               // between MGS2 SoL and MGS2 Substance but we're still using the same script data.
               BP_BREAK;
            }

			   sd_set_cli( 0xF0000000 | ( size >> 4 ) );
#ifdef PSX2			
   			sd_set_cli( 0xF5000000 | ( fmt << 20 ) | ( ch << 16 ) | pitch );
#else
	   		sd_3d_start_stream( work->ctrl->hnsd3d, 0xF5000000 | ( ch << 16 ) | pitch );
#endif			
		   } else {
			   sd_set_cli( 0xF2000000 | ( size >> 4 ) );
#ifdef PSX2
   			sd_set_cli( 0xF4000000 | ( fmt << 20 ) | ( ch << 16 ) | pitch );
#else
	   		sd_3d_start_stream( work->ctrl->hnsd3d, 0xF4000000 | ( ch << 16 ) | pitch );
#endif			
   		}
#ifdef PSX2		
		   if( fmt == 0 ){
			   work->ctrl->sd_duration = ( size / 16 * 28 ) * 300.0F / ( pitch * ch );
		   } else if( fmt == 1 ){
			   work->ctrl->sd_duration = ( size ) * 300.0F / ( pitch * ch );
		   } else if( fmt == 2 ){
			   work->ctrl->sd_duration = ( size * 3 ) * 300.0F / ( pitch * 2 * 2 );
		   }
#endif
#ifdef KP_XBOX
		   if( fmt == 0x11 ) {
			   // TODO:計算式は嘘なので
			   work->ctrl->sd_duration = ( size / 16 * 28 ) * 300.0F / ( pitch * ch );
		   } else {
			   printf( __FILE__ ": PS2 Format Sound Stream\n");
			   FS_StreamFreeData( work->stream_h, ptr );
			   return -1;
		   }
#endif		
      } //if( is_bp_audio )

printf( "duration = %d (%d)\n", work->ctrl->sd_duration, work->ctrl->sd_duration / 5 );
		work->sd_tr_count = 0;
		work->sd_size = size;
		work->sd_ptr = NULL;
		work->sd_ch = ch;
		work->sd_fmt = fmt;

		sd_status()[ 4 + work->ctrl->sd_channel ] = 0;

		FS_StreamFreeData( work->stream_h, ptr );

		if( pause_channel_flag & ( 1 << work->ctrl->sd_channel ) )
      {
			GM_StreamSdPauseControl( work->ctrl->sd_channel, 1 );
		}
	} else {
		printf( "NO SOUND DATA!!\n" );
		HANGUP();
		return -1;
	}
	{
		/* 初期バッファ転送分の転送 */
		int *status;

		work->sd_tr_block_count = 0;

		status = sd_status();
		send_sound_data( work, status );
	}

	return 0;
}

void *NewStreamSoundDriver( GM_STREAM_CONTROL *ctrl, int type )
{
	Work *work;
	int channel;
	int subtype;

	subtype = ( type >> 16 );
	if( subtype == 1 ){
		// AC3
#ifdef	KP_WINDOWS
		if(0) {
			printf("5.1ch stream is canceled\n");
			return NULL;
		}
#else
#ifdef KP_XBOX
		if( !(XC_AUDIO_FLAGS_BASIC( XGetAudioFlags() ) == XC_AUDIO_FLAGS_SURROUND )) {
			printf("5.1ch stream is canceled\n");
			return NULL;
		}
		printf("5.1ch stream is initialized\n");
#else
		if( ( GM_Configuration & GM_CONFIG_SOUND_5_1CHANL ) == 0 ){
			return NULL;
		}
#endif
#endif			
	} else if( subtype == 2 ){
		// AC3の時の通常チャンネル
#ifdef	KP_WINDOWS
		if(1) {
			printf("5.1ch stream is canceled\n");
			return NULL;
		}
#else
#ifdef KP_XBOX
		
		if( XC_AUDIO_FLAGS_BASIC( XGetAudioFlags() ) == XC_AUDIO_FLAGS_SURROUND ) {
			printf("2ch stream is canceled\n");
			return NULL;
		}
		printf("2ch stream is initialized\n");
#else
		if( ( GM_Configuration & GM_CONFIG_SOUND_5_1CHANL ) ){
			return NULL;
		}
#endif		
#endif
	}

	if( ( channel = ctrl->sd_channel ) < 0 ){
		if( ( channel = get_free_channel() ) < 0 ){
			return NULL;	// NO CHANNEL
		}
	}
	if( ( work = GV_CreateActor( GV_ACTOR_MANAGER, GV_CLASS_SYSTEM
								 , sizeof( Work ), 0xf2 ) ) != NULL ){
		GV_SetActor( &work->actor, Act, Die );
		GV_ActorEX( &work->actor );
		work->ctrl = ctrl;
		work->ctrl->sd_channel = channel;
		work->stream_h = ctrl->stream_h;
		work->type = type;
      if( ctrl->sd_channel < 0 || ctrl->sd_channel > 1 )
         BP_BREAK;
      is_bp_audio[ctrl->sd_channel] = ( type == STREAM_TYPE_MSF || type == STREAM_TYPE_XAUDIO || type == STREAM_TYPE_VITA_AT9) ? 1 : 0;

      gBP_DirectOutputStreamIsDemo = 1;   //no streamed BGM in MGS2!

		ctrl->tick = -1;

		if( SoundStreamInit( work ) < 0 ){
			GV_DestroyActor( work );
			return NULL;
		}
#ifdef DEBUG_MODE
		sd_work = work;
#endif
	}
printf( "NewStreamSoundDriver end\n" );
	return work;
}

void GM_StreamSdSetPan( int channel, int volpan, float bp_angle )
{
   //N.B. this is called in one case before a stream is initialized, so it's safest to send the volume
   //and pan to both interfaces even though it should affect only one stream.
   {
      //Perform same volume calculation as in sd_str2 which is the type replaced by this BP_AudioStream.
      int channel_count = BP_AudioStreamGetChannelCount( channel );
      int is_mono = ( channel_count == 1 );
      int set_vol = volpan & 0x3F;
      int set_pan = (volpan&0x3F00) >> 8;

      int volsStereo[2][2];
      float vol3d;

      // AS - If this function was called before the audio stream has a chance to load, assume it is a mono stream since
      // this is true in most cases where this problem might occur.
      if (channel_count == 0)
         is_mono = 1;

      if( !is_mono )
      {
         //max vol for both L and R.
         volsStereo[0][0] = volsStereo[1][1] = (unsigned int)((((0x7F*set_vol)/0x3F)*se_pant[0x3F])/0x7F);
         //right volume of first channel and left volume of second channel are empty.
         volsStereo[0][1] = volsStereo[1][0] = 0;
      }
      else
      {
         //For mono streams, sd_str2 had a second channel containing the same data as the first.
         //The second channel's volume was 1/3 that of the first.
         volsStereo[0][0] = (unsigned int)((((0x7F*set_vol)/0x3F)*se_pant[0x40-set_pan])/0x7F);
         volsStereo[0][1] = (unsigned int)((((0x7F*set_vol)/0x3F)*se_pant[set_pan])/0x7F);
         volsStereo[1][0] = volsStereo[0][0] / 3;
         volsStereo[1][1] = volsStereo[0][1] / 3;
      }

      // calculate the 3D volume.
      vol3d = (float)set_vol/0x3F;

      //BP - notify the trophy logic of the current event.  This will let trophies be decided based on
      // the volume of a stream.
      {
         GM_STREAM_CONTROL* ctrl = BP_GetStreamControl( channel );
         if ( ctrl )
            bp_trophy_update_stream_pan( ctrl->top_pos, bp_angle, vol3d );
      }

      // apply the volume.
      BP_AudioStreamSetVolume_Voice( channel, volsStereo, bp_angle, vol3d );
   }
   {
      bp_cli_float_param = bp_angle;

      if( channel == 0 ){
         sd_set_cli( 0xFBFE0000 | volpan );
      } else if( channel == 1 ) {
         sd_set_cli( 0xFBFF0000 | volpan );
      }
   }
}

void GM_StreamSdPauseOn( GM_STREAM_CONTROL *ctrl )
{
printf( "GM_StreamSdPauseOn\n" );
	sd_set_cli( 0xFF000019 );
   BP_SetAllDirectOutputStreamPaused( 1 );
}

void GM_StreamSdPauseOff( GM_STREAM_CONTROL *ctrl )
{
printf( "GM_StreamSdPauseOff\n" );
	sd_set_cli( 0xFF00001A );
   BP_SetAllDirectOutputStreamPaused( 0 );
}

void GM_StreamSdPauseControl( int channel, int on_off )
{
	int code = 0;
   int bp_audio = is_bp_audio[channel];

printf( "GM_StreamSdPauseControl %d %d\n", channel, on_off );

	if( on_off ){
		/* PAUSE ON */
		if( channel == 0 ){
			code = 0xFF00001B;
		} else if( channel == 1 ){
			code = 0xFF00001D;
		}
      if( bp_audio )
         BP_AudioStreamPause( channel, 1, BP_AUDIOSTREAM_PAUSE_STREAM );

		pause_channel_flag |= ( 1 << channel );
	} else {
		/* PAUSE OFF */
		if( channel == 0 ){
			code = 0xFF00001C;
		} else {
			code = 0xFF00001E;
		}
      if( bp_audio )
         BP_AudioStreamPause( channel, 0, BP_AUDIOSTREAM_PAUSE_STREAM );

		pause_channel_flag &= ~( 1 << channel );
	}
	if( code != 0 ){
		sd_set_cli( code );
	}
}

/*
	ドライバ登録 
*/

static GM_STREAM_DRIVER driver ;

int GM_StreamSdDriverInit( void )
{
	sd_used_channel = 0;
	pause_channel_flag = 0;

	driver.driver = NewStreamSoundDriver ;
	GM_StreamAddDriver( &driver, CHANK_TYPE_PCM );
	return 0;
}

#ifdef DEBUG_MODE
void GM_StreamSdCancel( void )
{
	// 音声をキャンセルし,それ以降ダミー扱いにする
	if( sd_work != NULL ){
		sd_set_cli( 0xFF00000A );
		sd_set_cli( 0xFF00000E );
		GV_ChangeActFunc( sd_work, GM_StreamDummyAct );
	}
}
#endif
