//----------------------------------------------------------------------------
// BP_SurroundSound.cpp
//----------------------------------------------------------------------------

#include "Engine/StdAfx.h"

#include "BP_BuildDefines.h"

#include "BP_SurroundSound.h"
#include "BP_Debug.h"

#include "libgraph.h"

//----------------------------------------------------------------------------

namespace
{
   struct SSurroundFilter
   {
      int            mAzimuthPointCount;
      int            mSpeakerCount;
      const float *  mSpeakerCoeffs;
   };
}

//----------------------------------------------------------------------------

static const float skSpeakerCoeffsD7[] =
{
   // FL     FR     FC    LFE     RL     RR     EL     ER
   0.00f, 0.00f, 1.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f,  // 0   - FC
   0.00f, 0.71f, 0.71f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f,  // 15
   0.00f, 1.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f,  // 30  - FR
   0.00f, 0.95f, 0.00f, 0.00f, 0.00f, 0.31f, 0.00f, 0.00f,  // 45
   0.00f, 0.81f, 0.00f, 0.00f, 0.00f, 0.59f, 0.00f, 0.00f,  // 60
   0.00f, 0.59f, 0.00f, 0.00f, 0.00f, 0.81f, 0.00f, 0.00f,  // 75
   0.00f, 0.31f, 0.00f, 0.00f, 0.00f, 0.95f, 0.00f, 0.00f,  // 90
   0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 1.00f, 0.00f, 0.00f,  // 105 - RR
   0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.87f, 0.00f, 0.50f,  // 120
   0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.50f, 0.00f, 0.87f,  // 135
   0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 1.00f,  // 150 - ER
   0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.36f, 0.92f,  // 165
   0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.71f, 0.71f,  // 180
   0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.92f, 0.36f,  // 195
   0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 1.00f, 0.00f,  // 210 - EL
   0.00f, 0.00f, 0.00f, 0.00f, 0.50f, 0.00f, 0.87f, 0.00f,  // 225
   0.00f, 0.00f, 0.00f, 0.00f, 0.87f, 0.00f, 0.50f, 0.00f,  // 240
   0.00f, 0.00f, 0.00f, 0.00f, 1.00f, 0.00f, 0.00f, 0.00f,  // 255 - RL
   0.31f, 0.00f, 0.00f, 0.00f, 0.95f, 0.00f, 0.00f, 0.00f,  // 270
   0.59f, 0.00f, 0.00f, 0.00f, 0.81f, 0.00f, 0.00f, 0.00f,  // 285
   0.81f, 0.00f, 0.00f, 0.00f, 0.59f, 0.00f, 0.00f, 0.00f,  // 300
   0.95f, 0.00f, 0.00f, 0.00f, 0.31f, 0.00f, 0.00f, 0.00f,  // 315
   1.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f,  // 330 - FL
   0.71f, 0.00f, 0.71f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f,  // 345
   0.00f, 0.00f, 1.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f,  // 0   - FC
};

static const SSurroundFilter skFilterD7 =
{
   24,
   8,
   skSpeakerCoeffsD7,
};

//----------------------------------------------------------------------------

static void bp_do_calc_speaker_vols( const SSurroundFilter * const pFilter, float * const outSpeakerVols, float azimuth )
{
   if( azimuth != azimuth )
   {
      BP_BREAK;   //catch bad floats most likely due to uninitialized data for 3d pan
      azimuth = 0.f;
   }

   //Wrap azimuth
   while (azimuth >= (M_PI*2))
   {
      azimuth -= (M_PI*2);
   }
   while (azimuth < 0.f)
   {
      azimuth += (M_PI*2);
   }

   const float azimuth_point_interval = (M_PI*2) / (float)pFilter->mAzimuthPointCount;
   const float azimuth_point = azimuth / azimuth_point_interval;

   int az_index = (int)azimuth_point;

   const int row0 = (az_index * pFilter->mSpeakerCount);
   const int row1 = (((az_index+1)%pFilter->mAzimuthPointCount) * pFilter->mSpeakerCount);

   float azimuth_fraction = azimuth_point - (float)az_index;

   float one_minus_t = (1.0f - azimuth_fraction);
   float t = azimuth_fraction;

   for( int speaker = 0; speaker < pFilter->mSpeakerCount; ++speaker )
   {
      outSpeakerVols[speaker] = ( pFilter->mSpeakerCoeffs[row0 + speaker] * one_minus_t ) + ( pFilter->mSpeakerCoeffs[row1 + speaker] * t );
   }
}

void BP_SurroundSoundCalcVols( float * const outSpeakerVols, const int speakerCount, const float pan3d, const float vol3d )
{
   BPE_ASSERT( speakerCount == 8, "TODO!" );
   const SSurroundFilter * const pFilter = &skFilterD7;

   if( vol3d != 0.f )
   {
      bp_do_calc_speaker_vols(pFilter, outSpeakerVols, pan3d);
      for( int i=0; i < speakerCount; ++i )
      {
         outSpeakerVols[i] *= vol3d;
      }
   }
   else
   {
      for( int i=0; i < speakerCount; ++i )
      {
         outSpeakerVols[i] = 0.f;
      }
   }
}
