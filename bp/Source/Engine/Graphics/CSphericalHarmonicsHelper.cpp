//------------------------------------------------------------------------------------------
// CSphericalHarmonicsHelper.cpp
// Bluepoint
// Copyright 2004
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/Graphics/CSphericalHarmonicsHelper.h"

//------------------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------------------

namespace
{
   real32 const sample_random()
   {
      return rand() / static_cast<real32>( RAND_MAX + 1 );
   }
};

//------------------------------------------------------------------------------------------

void CSphericalHarmonicsHelper::SetupSamples( int const sqrtNumSamples,
                                              std::vector< CVector3 > & samples )
{
   int const numBands = 3;

   real32 const oneOverN = 1.0f / sqrtNumSamples;

   for( int a = 0; a < sqrtNumSamples; ++a )
   {
      for( int b = 0; b < sqrtNumSamples; ++b )
      {
         real32 const x = ( a + sample_random() ) * oneOverN;
         real32 const y = ( b + sample_random() ) * oneOverN;
         
         real32 const theta = 2.0f * acosf( sqrtf( 1.0f - x ) );
         real32 const phi = 2.0f * gkPi32 * y;

         CVector3 const direction( sinf( theta ) * cosf( phi ), sinf( theta ) * sinf( phi ), cosf( theta ) );
         
         samples.push_back( direction );
      }
   }
}

//------------------------------------------------------------------------------------------
   
void CSphericalHarmonicsHelper::SampleSphere( IRadianceFunction const & transferFunction,
                                              int const sqrtNumSamples,
                                              CSphericalHarmonics & out )
{
   vector<CVector3> samples;
   SetupSamples( sqrtNumSamples, samples );

   real32 const normalisation = gkPi32 * 16.0f / 17.0f;
   real32 const const1 = powf( 0.282095f, 2.0f ) * normalisation * 1.0f;
   real32 const const2 = powf( 0.488603f, 2.0f ) * normalisation * (2.0f / 3.0f);
   real32 const const3 = powf( 1.092548f, 2.0f ) * normalisation * (1.0f / 4.0f);
   real32 const const4 = powf( 0.315392f, 2.0f ) * normalisation * (1.0f / 4.0f);
   real32 const const5 = powf( 0.546274f, 2.0f ) * normalisation * (1.0f / 4.0f);  

   for( int i = 0; i < samples.size(); ++i )
   {
      CVector3 const & sample = samples[i];

      CVector3 const color = transferFunction.Evaluate( sample );
      
      real32 const value = ( color[kAX] + color[kAY] + color[kAZ] ) / 3.0f;

      out[0] += value * const1;
      
      out[1] += value * const2 * sample[kAX];
      out[2] += value * const2 * sample[kAY];
      out[3] += value * const2 * sample[kAZ];
   
      out[4] += value * const3 * sample[kAX] * sample[kAZ]; 
      out[5] += value * const3 * sample[kAZ] * sample[kAY];
      out[6] += value * const3 * sample[kAY] * sample[kAX];
   
      out[7] += value * const4 * ( 3.0f * sample[kAZ] * sample[kAZ] - 1.0f );
      
      out[8] += value * const5 * ( sample[kAX] * sample[kAX] - sample[kAY] * sample[kAY] );
   }

   out *= 2.0f * gkPi32 / samples.size();
}

//------------------------------------------------------------------------------------------
   
void CSphericalHarmonicsHelper::SampleSphere( IRadianceFunction const & transferFunction,
                                              int const sqrtNumSamples,
                                              CSphericalHarmonics3 & out )
{
   vector<CVector3> samples;
   SetupSamples( sqrtNumSamples, samples );
   
   real32 const normalisation = gkPi32 * 16.0f / 17.0f;
   real32 const const1 = powf( 0.282095f, 2.0f ) * normalisation * 1.0f;
   real32 const const2 = powf( 0.488603f, 2.0f ) * normalisation * (2.0f / 3.0f);
   real32 const const3 = powf( 1.092548f, 2.0f ) * normalisation * (1.0f / 4.0f);
   real32 const const4 = powf( 0.315392f, 2.0f ) * normalisation * (1.0f / 4.0f);
   real32 const const5 = powf( 0.546274f, 2.0f ) * normalisation * (1.0f / 4.0f);  

   for( int i = 0; i < samples.size(); ++i )
   {
      CVector3 const & sample = samples[i];

      CVector3 const value = transferFunction.Evaluate( sample );
      
      out[0] += value * const1;
      
      out[1] += value * const2 * sample[kAX];
      out[2] += value * const2 * sample[kAY];
      out[3] += value * const2 * sample[kAZ];
   
      out[4] += value * const3 * sample[kAX] * sample[kAZ]; 
      out[5] += value * const3 * sample[kAZ] * sample[kAY];
      out[6] += value * const3 * sample[kAY] * sample[kAX];
   
      out[7] += value * const4 * ( 3.0f * sample[kAZ] * sample[kAZ] - 1.0f );
      
      out[8] += value * const5 * ( sample[kAX] * sample[kAX] - sample[kAY] * sample[kAY] );
   }

   out *= 2.0f * gkPi32 / samples.size();
}

//------------------------------------------------------------------------------------------
   
void CSphericalHarmonicsHelper::AddDirectionalLight( CVector3 const & direction,
                                                     CVector3 const & color,
                                                     CSphericalHarmonics3 & out )
{
   real32 const normalisation = gkPi32 * 16.0f / 17.0f;
   real32 const const1 = powf( 0.282095f, 2.0f ) * normalisation * 1.0f;
   real32 const const2 = powf( 0.488603f, 2.0f ) * normalisation * (2.0f / 3.0f);
   real32 const const3 = powf( 1.092548f, 2.0f ) * normalisation * (1.0f / 4.0f);
   real32 const const4 = powf( 0.315392f, 2.0f ) * normalisation * (1.0f / 4.0f);
   real32 const const5 = powf( 0.546274f, 2.0f ) * normalisation * (1.0f / 4.0f);  
   
   out[0] += color * const1;
   
   out[1] += color * const2 * direction[kAX];
   out[2] += color * const2 * direction[kAY];
   out[3] += color * const2 * direction[kAZ];
   
   out[4] += color * const3 * direction[kAX] * direction[kAZ]; 
   out[5] += color * const3 * direction[kAZ] * direction[kAY];
   out[6] += color * const3 * direction[kAY] * direction[kAX];
   
   out[7] += color * const4 * ( 3.0f * direction[kAZ] * direction[kAZ] - 1.0f );
   
   out[8] += color * const5 * ( direction[kAX] * direction[kAX] - direction[kAY] * direction[kAY] );
}

//------------------------------------------------------------------------------------------
   
void CSphericalHarmonicsHelper::AddAmbientLight( CVector3 const & color,
                                                 CSphericalHarmonics3 & out )
{
   out[0] += color;
}

