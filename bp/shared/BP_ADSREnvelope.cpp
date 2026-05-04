//----------------------------------------------------------------------------
// BP_ADSREnvelope.h
// Multiplatform PS2 ADSR envelope emulation support
//----------------------------------------------------------------------------

#include "Engine/StdAfx.h"

#include "Engine/System/CStopWatch.h"
#include "Engine/Graphics/CColor.h"
#include "BP_RendererDebug.h"

#include "BP_BuildDefines.h"

#include "BP_ADSREnvelope.h"

#include "MGS_Common.h"

#define SD_ADSR_S_LINEAR_INC    (0<<13)
#define SD_ADSR_S_LINEAR_DEC    (2<<13)
#define SD_ADSR_S_EXP_INC       (4<<13)
#define SD_ADSR_S_EXP_DEC       (6<<13)

//----------------------------------------------------------------------------

// Magic numbers that mean the envelope does not change over time in the corresponding part
static const int skInfinityIndex_AS = 127;
static const int skInfinityIndex_R = 31;

//----------------------------------------------------------------------------

//0->1 time
static const double skPlusLinTable[] =
{
   0.05,// (msec)
   0.06,
   0.07,
   0.09,
   0.10,
   0.12,
   0.15,
   0.18,
   0.21,
   0.24,
   0.29,
   0.36,
   0.41,
   0.48,
   0.58,
   0.73,
   0.83,
   0.97,
   1.2,
   1.5,
   1.7,
   1.9,
   2.3,
   2.9,
   3.3,
   3.9,
   4.6,
   5.8,
   6.6,
   7.7,
   9.3,
   12,
   13,
   15,
   19,
   23,
   27,
   31,
   37,
   46,
   53,
   62,
   74,
   93,
   0.11*1000,
   0.12*1000,
   0.15*1000,
   0.19*1000,
   0.21*1000,
   0.25*1000,
   0.30*1000,
   0.37*1000,
   0.42*1000,
   0.50*1000,
   0.59*1000,
   0.74*1000,
   0.85*1000,
   0.99*1000,
   1.2*1000,
   1.5*1000,
   1.7*1000,
   2.0*1000,
   2.4*1000,
   3.0*1000,
   3.4*1000,
   4.0*1000,
   4.8*1000,
   5.9*1000,
   6.8*1000,
   7.9*1000,
   9.5*1000,
   12*1000,
   14*1000,
   16*1000,
   19*1000,
   24*1000,
   27*1000,
   32*1000,
   38*1000,
   48*1000,
   54*1000,
   63*1000,
   76*1000,
   95*1000,
   109*1000,
   127*1000,
   152*1000,
   190*1000,
   218*1000,
   254*1000,
   304*1000,
   380*1000,
   436*1000,
   508*1000,
   608*1000,
   760*1000,
   872*1000,
   1016*1000,
   1216*1000,
   1520*1000,
   1744*1000,
   2032*1000,
   2432*1000,
   3040*1000,
   3488*1000,
   4064*1000,
   4864*1000,
   6080*1000
};

//1->0 time
static const double skMinusLinTable[] =
{
   0.04,// (msec)
   0.05,
   0.06,
   0.07,
   0.09,
   0.10,
   0.12,
   0.15,
   0.18,
   0.21,
   0.24,
   0.29,
   0.36,
   0.41,
   0.48,
   0.58,
   0.73,
   0.83,
   0.97,
   1.2,
   1.5,
   1.7,
   1.9,
   2.3,
   2.9,
   3.3,
   3.9,
   4.6,
   5.8,
   6.6,
   7.7,
   9.3,
   12,
   13,
   15,
   19,
   23,
   27,
   31,
   37,
   46,
   53,
   62,
   74,
   93,
   0.11*1000,
   0.12*1000,
   0.15*1000,
   0.19*1000,
   0.21*1000,
   0.25*1000,
   0.30*1000,
   0.37*1000,
   0.42*1000,
   0.50*1000,
   0.59*1000,
   0.74*1000,
   0.85*1000,
   0.99*1000,
   1.2*1000,
   1.5*1000,
   1.7*1000,
   2.0*1000,
   2.4*1000,
   3.0*1000,
   3.4*1000,
   4.0*1000,
   4.8*1000,
   5.9*1000,
   6.8*1000,
   7.9*1000,
   9.5*1000,
   12*1000,
   14*1000,
   16*1000,
   19*1000,
   24*1000,
   27*1000,
   32*1000,
   38*1000,
   48*1000,
   54*1000,
   63*1000,
   76*1000,
   95*1000,
   109*1000,
   127*1000,
   152*1000,
   190*1000,
   218*1000,
   254*1000,
   304*1000,
   380*1000,
   436*1000,
   508*1000,
   608*1000,
   760*1000,
   872*1000,
   1016*1000,
   1216*1000,
   1520*1000,
   1744*1000,
   2032*1000,
   2432*1000,
   3040*1000,
   3488*1000,
   4064*1000,
   4864*1000
};

const double skPlusExpTable[] =
{
   0.09,// (msec)
   0.11,
   0.13,
   0.16,
   0.18,
   0.21,
   0.25,
   0.32,
   0.36,
   0.42,
   0.51,
   0.64,
   0.73,
   0.85,
   1.0,
   1.3,
   1.5,
   1.7,
   2.0,
   2.5,
   2.9,
   3.4,
   4.1,
   5.1,
   5.8,
   6.8,
   8.1,
   10,
   12,
   14,
   16,
   20,
   23,
   27,
   33,
   41,
   46,
   54,
   65,
   81,
   93,
   0.11*1000,
   0.13*1000,
   0.16*1000,
   0.19*1000,
   0.22*1000,
   0.26*1000,
   0.33*1000,
   0.37*1000,
   0.43*1000,
   0.52*1000,
   0.65*1000,
   0.74*1000,
   0.87*1000,
   1.0*1000,
   1.3*1000,
   1.5*1000,
   1.7*1000,
   2.1*1000,
   2.6*1000,
   3.0*1000,
   3.5*1000,
   4.2*1000,
   5.2*1000,
   5.9*1000,
   6.9*1000,
   8.3*1000,
   10*1000,
   12*1000,
   14*1000,
   17*1000,
   21*1000,
   24*1000,
   28*1000,
   33*1000,
   42*1000,
   48*1000,
   55*1000,
   67*1000,
   83*1000,
   95*1000,
   111*1000,
   133*1000,
   166*1000,
   190*1000,
   222*1000,
   266*1000,
   333*1000,
   380*1000,
   444*1000,
   532*1000,
   666*1000,
   760*1000,
   888*1000,
   1064*1000,
   1332*1000,
   1520*1000,
   1776*1000,
   2128*1000,
   2664*1000
};

//1->0.1 time
const double skMinusExpTable[] =
{
   0.07,// (msec)
   0.09,
   0.11,
   0.14,
   0.18,
   0.21,
   0.25,
   0.31,
   0.39,
   0.45,
   0.53,
   0.64,
   0.81,
   0.93,
   1.1,
   1.3,
   1.6,
   1.9,
   2.2,
   2.6,
   3.3,
   3.8,
   4.4,
   5.3,
   6.7,
   7.6,
   8.9,
   11,
   13,
   15,
   18,
   21,
   27,
   31,
   36,
   43,
   53,
   61,
   71,
   86,
   0.11*1000,
   0.12*1000,
   0.14*1000,
   0.17*1000,
   0.21*1000,
   0.24*1000,
   0.29*1000,
   0.34*1000,
   0.43*1000,
   0.49*1000,
   0.57*1000,
   0.68*1000,
   0.86*1000,
   0.98*1000,
   1.1*1000,
   1.4*1000,
   1.7*1000,
   2.0*1000,
   2.3*1000,
   2.7*1000,
   3.4*1000,
   3.9*1000,
   4.6*1000,
   5.5*1000,
   6.8*1000,
   7.8*1000,
   9.1*1000,
   11*1000,
   14*1000,
   16*1000,
   18*1000,
   22*1000,
   27*1000,
   31*1000,
   36*1000,
   44*1000,
   55*1000,
   63*1000,
   73*1000,
   88*1000,
   109*1000,
   125*1000,
   146*1000,
   175*1000,
   219*1000,
   250*1000,
   292*1000,
   350*1000,
   438*1000,
   500*1000,
   584*1000,
   700*1000,
   876*1000,
   1000*1000,
   1168*1000,
   1400*1000,
   1752*1000,
   2000*1000,
   2336*1000,
   2800*1000,
   3504*1000,
   4000*1000,
   4672*1000,
   5600*1000,
   7008*1000,
   8000*1000,
   9344*1000,
   11200*1000
};

//1->0.1 time
const double skDecayRate[] =
{
   0.07,// (msec)
   0.18,
   0.39,
   0.81,
   1.6,
   3.3,
   6.7,
   13,
   27,
   53,
   0.11*1000,
   0.21*1000,
   0.43*1000,
   0.86*1000,
   1.7*1000,
   3.4*1000
};

//1->0 time
const double skReleaseTime_MinusLin[] =
{
   0.04,// (msec)
   0.09,
   0.18,
   0.36,
   0.73,
   1.5,
   2.9,
   5.8,
   12,
   23,
   46,
   93,
   0.19*1000,
   0.37*1000,
   0.74*1000,
   1.5*1000,
   3.0*1000,
   5.9*1000,
   12*1000,
   24*1000,
   48*1000,
   95*1000,
   190*1000,
   380*1000,
   760*1000,
   1520*1000,
   3040
};

//1->0.1 time
const double skReleaseTime_MinusExp[] =
{
   0.07,// (msec)
   0.18,
   0.39,
   0.81,
   1.6,
   3.3,
   6.7,
   13,
   27,
   53,
   0.11*1000,
   0.21*1000,
   0.43*1000,
   0.86*1000,
   1.7*1000,
   3.4*1000,
   6.8*1000,
   14*1000,
   27*1000,
   55*1000,
   109*1000,
   219*1000,
   438*1000,
   876*1000,
   1752*1000,
   3504*1000,
   7008*1000,
};

//----------------------------------------------------------------------------

static double get_table_value( const double * const table, int index, const int tableCount )
{
   if( index < 0 )
      BP_BREAK;
   //Many of the tables do not have entries for the full range of values.  Just clamp.
   if( index >= tableCount )
   {
      //N.B. these cases are illegal, as values over those listed in the table are reserved,
      //but they come up anyway as in w02a of MGS2.  There's no good way to know exactly
      //what the PS2 did in this case, so take the highest value in the table.
      index = tableCount-1;
   }
   //convert to seconds.
   double value = table[index] / 1000.0;
   return value;
}

static float get_pseudo_exp_inc_dy( const float y, const float dx, const double tableValue )
{
   //"linear volume increment is lowered in increment rate when 75% of maximum value is exceeded."
   const float kSlopeChangeThreshold = 0.75f;
   const float kSteepSlopeScale = 2.f;
   float time0To1 = (float)tableValue;
   float linearSlope = 1.f / time0To1;
   float preSlope = kSteepSlopeScale * linearSlope;
   if( y < kSlopeChangeThreshold )
   {
      //use steep slope.
      float dy = dx * preSlope;
      return dy;
   }
   else
   {
      //from here, use gentle slope that intersects y=1 at the same place as the linear slope.
      float postSlope = (1.f-kSlopeChangeThreshold) / ( ( 1.f-kSlopeChangeThreshold/kSteepSlopeScale ) * tableValue );
      float dy = dx * postSlope;
      return dy;
   }
}

static float get_exp_dec_dy( const float y, const float dx, const double tableValue )
{
   //Eyeballing the curve(s) in fig. 2.7, using the following for exponential decrease function:
   //y = -0.25 * ln(x/tableValue) + 0.1
   //dy/dx = -0.25 * 1/(x/tableValue)
   //x = (e^(-4(y-0.1))) * tableValue.

   const double kE = 2.718281828459;
   //where we currently are in the curve
   double x = pow(kE, -4.0 * (y - 0.1) ) * tableValue;
   //how much to drop envelope for this timestep
   float dy = (float)( (-0.25)*(double)dx / x );
   return dy;
}

//----------------------------------------------------------------------------

void BP_InitADSREnvelope( SADSREnvelopeState * const pEnvelope )
{
   pEnvelope->mADSRState = kADSRState_Attack;
   pEnvelope->mInitEnvelopeTimeUs = CStopWatch::gGlobalTime.GetElapsedMicroSeconds();
   pEnvelope->mCurrEnvelopeTimeElapsedMs = 0;
   pEnvelope->mEnvelopeScale = 0.f;
}

extern void BP_UpdateADSREnvelope( SADSREnvelopeState * const pEnvelope )
{
   const EADSRState prevADSRState = pEnvelope->mADSRState;
   const uint64 newTimeUs = CStopWatch::gGlobalTime.GetElapsedMicroSeconds();
   const int msElapsed = (int)((newTimeUs - pEnvelope->mInitEnvelopeTimeUs)/1000);

   //update values every n ms (increment fixed for consistent results across different BASE_TICK)
   const int kUpdateIncrementMs = 10;

   int attackMode = pEnvelope->mADSR1 & 0x8000;
   int attackRate = ( pEnvelope->mADSR1 & ( 0x7f << 8 ) ) >> 8;
   int decayRate = ( pEnvelope->mADSR1 & ( 0xf << 4 ) ) >> 4;
   int sustainLevel = pEnvelope->mADSR1 & 0xf;

   int sustainMode = pEnvelope->mADSR2 & 0xe000;
   int sustainRate = ( pEnvelope->mADSR2 & ( 0x7f << 6 ) ) >> 6;
   int releaseMode = pEnvelope->mADSR2 & 0x20;
   int releaseRate = pEnvelope->mADSR2 & 0x1f;

   double sustainFraction = (double)(sustainLevel+1) / 16;

   //Ensure that the first update call actually updates the envelope
   //by letting envelope time get up to < kUpdateIncrementMs ahead

   // Since we update sound on a separate thread at the correct frequency, just do a single iteration on Vita
   for( ; pEnvelope->mCurrEnvelopeTimeElapsedMs < msElapsed + kUpdateIncrementMs; pEnvelope->mCurrEnvelopeTimeElapsedMs += kUpdateIncrementMs )
   {
      const float dt = (float)kUpdateIncrementMs/1000.f;   //seconds

      //Tick ADSR envelope.
      switch (pEnvelope->mADSRState)
      {
      case kADSRState_Off:
         break;
      case kADSRState_Attack:
         {
            if( attackRate != skInfinityIndex_AS )
            {
               float dy;

               if( attackMode )
               {
                  //pseudo exponential increment (+exp)
                  const double attackTime0To1 = get_table_value( skPlusExpTable, attackRate, BPE_ARRAY_SIZE( skPlusExpTable ) );
                  dy = get_pseudo_exp_inc_dy( pEnvelope->mEnvelopeScale, dt, attackTime0To1 );
               }
               else
               {
                  //linear increment (+lin)
                  const float attackTime0To1 = (float)(get_table_value( skPlusLinTable, attackRate, BPE_ARRAY_SIZE( skPlusLinTable ) ));
                  const float slope = 1.f / attackTime0To1;
                  dy = dt * slope;
               }

               pEnvelope->mEnvelopeScale += dy;
               if( pEnvelope->mEnvelopeScale >= 1.f )
               {
                  //Attack is finished-- move on to decay.
                  pEnvelope->mEnvelopeScale = 1.f;
                  pEnvelope->mADSRState = kADSRState_Decay;
               }
            } //if (!infinity attack)
         }
         break;
      case kADSRState_Decay:
         {
            //always -exp
            if( sustainLevel == 15 )
            {
               //No decay; immediately advance to sustain. (4.1.7 of SPU manual)
               pEnvelope->mADSRState = kADSRState_Sustain;
            }
            else
            {
               //decay to sustain fraction then advance
               double decayTime1To0Pt1 = get_table_value( skDecayRate, decayRate, BPE_ARRAY_SIZE( skDecayRate ) );
               float dy = get_exp_dec_dy( pEnvelope->mEnvelopeScale, dt, decayTime1To0Pt1 );
               pEnvelope->mEnvelopeScale += dy;
               if( pEnvelope->mEnvelopeScale <= sustainFraction )
               {
                  pEnvelope->mEnvelopeScale = sustainFraction;
                  pEnvelope->mADSRState = kADSRState_Sustain;
               }
            }
         }
         break;
      case kADSRState_Sustain:
         {
            if( sustainRate != skInfinityIndex_AS )
            {
               switch( sustainMode )
               {
               default:
                  BP_BREAK;
               case SD_ADSR_S_LINEAR_INC:
                  {
                     //linear increment (+lin)
                     float sustainTime0To1 = (float)(get_table_value( skPlusLinTable, sustainRate, BPE_ARRAY_SIZE( skPlusLinTable ) ));
                     float slope = 1.f / sustainTime0To1;
                     float dy = dt * slope;
                     pEnvelope->mEnvelopeScale += dy;
                     if( pEnvelope->mEnvelopeScale >= 1.f )
                     {
                        pEnvelope->mEnvelopeScale = 1.f;
                     }
                  }
                  break;
               case SD_ADSR_S_LINEAR_DEC:
                  {
                     //linear decrement (-lin)
                     const float sustainTime1To0 = (float)(get_table_value( skMinusLinTable, sustainRate, BPE_ARRAY_SIZE( skMinusLinTable ) ));
                     const float slope = 1.f / sustainTime1To0;
                     const float dy = dt * slope;
                     pEnvelope->mEnvelopeScale -= dy;
                     if( pEnvelope->mEnvelopeScale <= 0.f )
                     {
                        pEnvelope->mEnvelopeScale = 0.f;
                     }
                  }
                  break;
               case SD_ADSR_S_EXP_INC:
                  {
                     //pseudo exponential increment (+exp)
                     const double sustainTime0To1 = get_table_value( skPlusExpTable, sustainRate, BPE_ARRAY_SIZE( skPlusExpTable ) );
                     const float dy = get_pseudo_exp_inc_dy( pEnvelope->mEnvelopeScale, dt, sustainTime0To1 );
                     pEnvelope->mEnvelopeScale += dy;
                     if( pEnvelope->mEnvelopeScale >= 1.f )
                     {
                        pEnvelope->mEnvelopeScale = 1.f;
                     }
                  }
                  break;
               case SD_ADSR_S_EXP_DEC:
                  {
                     //exponential decrement (-exp)
                     double sustainTime1To0Pt1 = get_table_value( skMinusExpTable, sustainRate, BPE_ARRAY_SIZE( skMinusExpTable ) );
                     float dy = get_exp_dec_dy( pEnvelope->mEnvelopeScale, dt, sustainTime1To0Pt1 );
                     pEnvelope->mEnvelopeScale += dy;
                     if( pEnvelope->mEnvelopeScale <= 0.f )
                     {
                        pEnvelope->mEnvelopeScale = 0.f;
                     }
                  }
                  break;
               }
            }//if (!infinity sustain)
         }
         break;
      case kADSRState_Release:
         {
            if( releaseRate != skInfinityIndex_R )
            {
               if( releaseMode )
               {
                  //exponential decrement (-exp)
                  double releaseTime1To0Pt1 = get_table_value( skReleaseTime_MinusExp, releaseRate, BPE_ARRAY_SIZE( skReleaseTime_MinusExp ) );
                  float dy = get_exp_dec_dy( pEnvelope->mEnvelopeScale, dt, releaseTime1To0Pt1 );
                  pEnvelope->mEnvelopeScale += dy;
                  if( pEnvelope->mEnvelopeScale <= 0.f )
                  {
                     //Envelope is done.
                     pEnvelope->mEnvelopeScale = 0.f;
                     pEnvelope->mADSRState = kADSRState_Off;
                  }
                  
               }
               else
               {
                  //linear decrement (-lin)
                  float releaseTime1To0 = (float)(get_table_value( skReleaseTime_MinusLin, releaseRate, BPE_ARRAY_SIZE( skReleaseTime_MinusLin ) ));
                  float slope = 1.f / releaseTime1To0;
                  float dy = dt * slope;
                  pEnvelope->mEnvelopeScale -= dy;
                  if( pEnvelope->mEnvelopeScale <= 0.f )
                  {
                     //Envelope is done.
                     pEnvelope->mEnvelopeScale = 0.f;
                     pEnvelope->mADSRState = kADSRState_Off;
                  }
               }
            }//if (!infinity release)
         }
         break;
      }
      if( prevADSRState != kADSRState_Off )
      {
#if 0
         const char kADSRChar[] = { '0', 'A', 'D', 'S', 'R' };
         printf("ENV %p: %04x %04x %c: %d: %f\n", pEnvelope, pEnvelope->mADSR1, pEnvelope->mADSR2, kADSRChar[pEnvelope->mADSRState], pEnvelope->mCurrEnvelopeTimeElapsedMs, pEnvelope->mEnvelopeScale );
#endif
      }
   }
}
