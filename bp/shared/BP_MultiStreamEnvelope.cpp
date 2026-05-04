#include <stdio.h>
#include <math.h>
#include <string.h>
#include <cell/mstream.h>

#define BP_BREAK asm("trap")

#define BP_ARRAY_LENGTH( arr ) ( sizeof( arr ) / sizeof( *arr ) )

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

//0->1 time
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

const double skPlusExpATable[] =
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

const double skPlusExpSTable[] =
{
   0.08,// (msec)
   0.09,
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
   64,
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
   2128*1000
};

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

//Decay rate
//1->0.1
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

static double get_table_value( const double * const table, int index, const int tableCount )
{
   if( index < 0 )
      BP_BREAK;
   //Many of the tables do not have entries for the full range of values.  Just clamp.
   if( index >= tableCount )
   {
      index = tableCount-1;
   }
   double value = table[index];
   return value > 1 ? value : 1;
}

#define SD_ADSR_S_LINEAR_INC    (0<<13)
#define SD_ADSR_S_LINEAR_DEC    (2<<13)
#define SD_ADSR_S_EXP_INC       (4<<13)
#define SD_ADSR_S_EXP_DEC       (6<<13)

extern "C" void BP_BuildMSEnvelope( const unsigned short adsr1, const unsigned short adsr2, CellMSEnvelope * const pEnvelope );

static void add_point( CellMSEnvelope * const pEnvelope, const int deltaTime, const float value )
{
   if( pEnvelope->NumPoints == CELL_MS_ENVELOPE_MAX_POINTS )
      BP_BREAK;

   const int prevTime = pEnvelope->Point[pEnvelope->NumPoints-1].X;
   int currTime = prevTime + deltaTime;
   if( currTime == prevTime )
   {
      //Be nice to multistream and don't give it two points with the same time.
      ++currTime;
   }
   pEnvelope->Point[pEnvelope->NumPoints].X = currTime;
   pEnvelope->Point[pEnvelope->NumPoints].Y = value;
   ++pEnvelope->NumPoints;
}

void BP_BuildMSEnvelope( const unsigned short adsr1, const unsigned short adsr2, CellMSEnvelope * const pEnvelope )
{
   memset( pEnvelope, 0, sizeof( *pEnvelope ) );

   const double kE = 2.718281828459;

   int attackMode = adsr1 & 0x8000;
   int attackRate = ( adsr1 & ( 0x7f << 8 ) ) >> 8;
   int decayRate = ( adsr1 & ( 0xf << 4 ) ) >> 4;
   int sustainLevel = adsr1 & 0xf;

   int sustainMode = adsr2 & 0xe000;
   int sustainRate = ( adsr2 & ( 0x7f << 6 ) ) >> 6;
   int releaseMode = adsr2 & 0x20;
   int releaseRate = adsr2 & 0x1f;

//   printf("ADSR1: AM %s, AR %d, DR %d, SL %d\n", attackMode ? "+EXP" : "+LIN", attackRate, decayRate, sustainLevel );
//   printf("ADSR2: SM %s, SR %d, RM %s, RR %d\n", sustainMode==SD_ADSR_S_LINEAR_INC ? "+LIN" : sustainMode==SD_ADSR_S_LINEAR_DEC ? "-LIN" : sustainMode==SD_ADSR_S_EXP_INC ? "+EXP" : sustainMode == SD_ADSR_S_EXP_DEC ? "-EXP" : "???", sustainRate, releaseMode ? "-EXP" : "-LIN", releaseRate );
   //Prepare a multistream envelope.
   pEnvelope->Type=CELL_MS_ENVELOPE_LIN;  //linear overall, we'll approximate exponential parts with more points.
   pEnvelope->NumPoints=1; //first point is always (0,0)
   pEnvelope->LoopStart=CELL_MS_ENVELOPE_NO_LOOP;
   pEnvelope->Point[0].X=0;
   pEnvelope->Point[0].Y=0;

   //First, Attack.
   //Time to reach 1.0
   double attackTime;
   if( attackMode )
   {
      //pseudo exponential
      //"linear volume increment is lowered in increment rate when 75% of maximum value is exceeded"
      attackTime = get_table_value( skPlusExpATable, attackRate, BP_ARRAY_LENGTH( skPlusExpATable ) );
      //One point where it hits 0.75
      //To approximate the shape of the pseudo-exponential curve, take a slope of twice the linear slope
      //to reach the intermediate point.
      const double linearSlope = 1;
      const double steepSlope = linearSlope * 2;
      const double intermediateX = attackTime * 0.75 / steepSlope;
      add_point( pEnvelope, (int)intermediateX, 0.75f );
      //Second point where it hits 1
      add_point( pEnvelope, (int)attackTime, 1 );
   }
   else
   {
      //linear
      attackTime = get_table_value( skPlusLinTable, attackRate, BP_ARRAY_LENGTH( skPlusLinTable ) );
      //One point where it hits 1
      add_point( pEnvelope, (int)attackTime, 1 );
   }


   //Next, Decay.
   //See 4.1.7 of SPU manual
   double sustainFraction = (double)(sustainLevel+1) / 16;
   if( sustainLevel == 15 )
   {
      //No decay, just create one more point at the same level right after the attack peak.
      add_point( pEnvelope, pEnvelope->Point[pEnvelope->NumPoints-1].X, 1.0 );
   }
   else
   {
      //Exponential decay.  The table gives the time it takes to exponentially decay from
      //1 to 0.1, so first find the point where it hits the sustain level.
      //Eyeballing the curve in fig. 2.7, using y = -ln(x)*0.25 + 0.1 as the decay function.
      //Inverse is x = e^(-4y-0.1).
      double stopDecayTimeScalar = get_table_value( skDecayRate, decayRate, BP_ARRAY_LENGTH( skDecayRate ) );
      //Generate a fixed number of intermediate points to approximate the curve.
      const int kNumPointsForDecay = 12;
      int lastX = 0;
      for( int i=kNumPointsForDecay-1; i >= 0; --i )
      {
         //Interpolate sustain linearly from 1 to target sustainFraction and find the appropriate x coordinate.
         const double currSustain = sustainFraction + ( 1.0 - sustainFraction ) * i / kNumPointsForDecay;
         double currTime = pow( kE, -4*currSustain - 0.1 );
         currTime *= stopDecayTimeScalar;
         int currX = (int)currTime;
         if( currX == lastX )
         {
            //Don't generate another point if the curve is too steep here.
            continue;
         }

         add_point( pEnvelope, currX, currSustain );
      }
   }

   //Next, Sustain.
   double sustainTime;
   switch( sustainMode )
   {
   default:
      BP_BREAK;
   case SD_ADSR_S_LINEAR_INC:
      {
         //Measure of the time it takes to get from 0 to 1 linearly
         sustainTime = get_table_value( skPlusLinTable, sustainRate, BP_ARRAY_LENGTH( skPlusLinTable ) );
         //How long from the start sustain level?
         if( sustainFraction < 1 )
         {
            sustainTime *= (1.0-sustainFraction);
         }
         else
         {
            //Already at 1.0
            sustainTime = 1.0;
         }

         //Generate the point where it hits the limit.
         add_point( pEnvelope, (int)sustainTime, 1 );
      }
      break;
   case SD_ADSR_S_LINEAR_DEC:
      {
         //Measure of the time it takes to get from 1 to 0 linearly
         sustainTime = get_table_value( skMinusLinTable, sustainRate, BP_ARRAY_LENGTH( skMinusLinTable ) );
         //How long from the start sustain level?
         sustainTime *= sustainFraction;

         //Generate the point where it hits the limit.
         add_point( pEnvelope, (int)sustainTime, 0 );
      }
      break;
   case SD_ADSR_S_EXP_INC:
      {
         //Pseudo exponential increment.
         sustainTime = get_table_value( skPlusExpSTable, sustainRate, BP_ARRAY_LENGTH( skPlusExpSTable ) );
         if( sustainFraction < 1 )
         {
            sustainTime *= (1.0-sustainFraction);
         }
         else
         {
            //Already at full volume.  Override sustain table setting.
            sustainTime = 1.0;
         }
         //Generate the point where it hits 0.75.
         //To approximate the shape of the pseudo-exponential curve, take a slope of twice the linear slope
         //to reach the intermediate point.
         const double linearSlope = 1;
         const double steepSlope = linearSlope * 2;
         const double intermediateX = sustainTime * 0.75 / steepSlope;
         add_point( pEnvelope, (int)intermediateX, 0.75f );

         //Generate the point where it hits 1.
         add_point( pEnvelope, (int)sustainTime, 1 );
      }
      break;
   case SD_ADSR_S_EXP_DEC:
      {
         if( sustainFraction > 0.1 )
         {
            //N.B. this looks like a gentle enough exponential dropoff that we can save some grief by just
            //using a linear dropoff instead.
            sustainTime = get_table_value( skMinusExpTable, sustainRate, BP_ARRAY_LENGTH( skMinusExpTable ) );
            sustainTime *= ( sustainFraction - 0.1 ) / 0.9;

            //Generate the point where it hits 0.1.
            add_point( pEnvelope, (int)sustainTime, 0.1 );
         }
         //else we're already below the range of the table.  Don't need to add more decay from here.
      }
      break;
   }

   //We have no control over release shape via points.
   //Assume we're at the end of sustain for the purposes of this calculation.
   double releaseTime;
   if( releaseMode )
   {
      //exponential
      releaseTime = get_table_value( skReleaseTime_MinusExp, releaseRate, BP_ARRAY_LENGTH( skReleaseTime_MinusExp ) );
      //N.B. the dropoff curve is *not* exponential in our multistream envelope, so bring in the time a bit to better fit
      //this linear shape to the important part of the curve.
      releaseTime *= 0.75;
   }
   else
   {
      //linear
      releaseTime = get_table_value( skReleaseTime_MinusLin, releaseRate, BP_ARRAY_LENGTH( skReleaseTime_MinusLin ) );
   }
   pEnvelope->ReleaseRate=(int)releaseTime;
}

