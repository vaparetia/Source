/*************************************************
  
  File: FilteredRandom.cpp

  Author: Steve Rabin
  Last Revision: August 1st, 2003

  This code is from the article "Filtered Randomness for AI
  Decisions and Game Logic" in the book AI Game Programming Wisdom 2 
  and may be used freely in any commercial software product.
  
  Updates to this source code can be found at www.aiwisdom.com.
  Feedback can be sent to steve@aiwisdom.com.

**************************************************/

#include "StdAfx.h"
#include "CFilteredRandom.h"

#include "Engine/Math/CRandom.h"

#include <math.h>

//Uncomment the next two lines to print out diagnostics
//#define FR_CHANCE_DIAGNOSTICS
//#define FR_RANGE_DIAGNOSTICS
//#define FR_GAUSSIAN_DIAGNOSTICS

#define FLOATING_POINT_ROUNDOFF_ERROR 0.000001f
#define INVALID_NEXT_VALUE_GAUSSIAN 100

namespace
{
   const bool InitialHistoryChance[FR_CHANCE_HISTORY_LENGTH] = {0,1,1,1,0,0,1,1,0,0,0,1,0,1,0,0,1,1,0,0};
   const unsigned char InitialHistoryRange[FR_RANGE_HISTORY_LENGTH] =   {3,8,0,6,7,9,7,0,3,5};
   const float InitialHistoryReal[FR_REAL_HISTORY_LENGTH] = {0.9f,0.3f,0.1f,0.4f,0.6f};
   const float InitialHistoryGaussian[FR_GAUSSIAN_HISTORY_LENGTH] = {0.9f,0.3f,0.6f,0.4f,0.8f};

   //These tables were carefully hand-tuned to ensure that random chances were within 1% of requested chance.

   //                                         1  2  3  4  5  6  7  8  9 10
   const unsigned int MaxAlternations[50] = { 2, 4, 4, 4, 4, 4, 4, 4, 4, 6,  //0.01 through 0.10
                                              6, 6, 6, 6, 6, 8, 8, 8, 8, 8,  //0.11 through 0.20
                                             10,10,10,10,10,12,12,12,12,12,  //0.21 through 0.30
                                             12,12,12,12,12,12,12,12,12,12,  //0.31 through 0.40
                                             12,12,12,12,12,14,14,14,14,14 };//0.41 through 0.50

   const unsigned int MinAlternations[50] = { 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,  //0.01 through 0.10
                                              1, 1, 1, 4, 4, 4, 4, 4, 4, 4,  //0.11 through 0.20
                                              4, 4, 4, 6, 6, 6, 6, 6, 6, 6,  //0.21 through 0.30
                                              8, 8, 8, 8, 8, 8, 8, 8, 8, 8,  //0.31 through 0.40
                                              8, 8,10,10,10,12,12,12,12,12 };//0.41 through 0.50

   const unsigned int MaxTrueRun[50] =      { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  //0.01 through 0.10
                                              2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  //0.11 through 0.20
                                              2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  //0.21 through 0.30
                                              2, 2, 2, 2, 3, 3, 3, 3, 3, 3,  //0.31 through 0.40
                                              3, 3, 3, 3, 3, 3, 3, 3, 3, 3 };//0.41 through 0.50

   const unsigned int MaxFalseRun[50] =     {25,25,25,25,25,25,25,15,15,15,  //0.01 through 0.10
                                             14,14,14,12,12,12,12,10,10,10,  //0.11 through 0.20
                                             10,10,10,10,10,10,10, 7, 7, 7,  //0.21 through 0.30
                                              7, 7, 7, 7, 7, 6, 6, 5, 5, 5,  //0.31 through 0.40
                                              5, 5, 4, 4, 4, 4, 4, 3, 3, 3 };//0.41 through 0.50
}

CFilteredRandomChance::CFilteredRandomChance( void )
{
	for( int i=0; i<FR_CHANCE_HISTORY_LENGTH; i++ )
	{
		mHistory[i] = InitialHistoryChance[i];
	}
}


bool CFilteredRandomChance::Generate( CRandom & random, float chance )
{
	int i, alternations = 0;
	float adjchance = chance;
	int adjchance_int = (int)((chance*100.0f) + 0.5f) - 1;
	bool flip = false;

	BPE_ASSERT( chance >= 0.0f, "CFilteredRandomChance::Generate - chance arg must be >= 0.0" );
	BPE_ASSERT( chance <= 1.0f, "CFilteredRandomChance::Generate - chance arg must be <= 1.0" );

	for( i=0; i<FR_CHANCE_HISTORY_LENGTH-1; i++ )
	{	//move history down
		mHistory[i] = mHistory[i+1];
	}

	//Take care of non-random chances
	if( chance <= 0.0f ) {
		mHistory[FR_CHANCE_HISTORY_LENGTH-1] = false;
		return( mHistory[FR_CHANCE_HISTORY_LENGTH-1] );
	}
	else if( chance >= 1.0f ) {
		mHistory[FR_CHANCE_HISTORY_LENGTH-1] = true;
		return( mHistory[FR_CHANCE_HISTORY_LENGTH-1] );
	}

	if( chance > 0.5f )
	{
		adjchance = 1.0f - chance - FLOATING_POINT_ROUNDOFF_ERROR;
		adjchance_int = (int)(((1.0f - chance)*100.0f) + 0.5f) - 1;
		flip = true;
	}
	
	if( adjchance_int < 0 )
	{
		adjchance_int = 0;
	}

	//Get random value based on chance
	mHistory[FR_CHANCE_HISTORY_LENGTH-1] = (random.RandomFloat() < adjchance);

	{	//enforce the correct number of alterations
		int max, min;

		for( i=0; i<FR_CHANCE_HISTORY_LENGTH-1; i++ )
		{	//count alternations
			if( mHistory[i] != mHistory[i+1] ) {
				alternations++;
			}
		}

		max = MaxAlternations[adjchance_int];
		min = MinAlternations[adjchance_int];

		if( alternations > max )
		{	//if too many alternations, make the new value the same as the last
			#ifdef FR_CHANCE_DIAGNOSTICS
			printf("-");
			#endif

			mHistory[FR_CHANCE_HISTORY_LENGTH-1] = mHistory[FR_CHANCE_HISTORY_LENGTH-2];
		}
		else if( alternations < min )
		{	//if not enough alternations, make the new value the opposite fo the last
			#ifdef FR_CHANCE_DIAGNOSTICS
			printf("-");
			#endif

			mHistory[FR_CHANCE_HISTORY_LENGTH-1] = !mHistory[FR_CHANCE_HISTORY_LENGTH-2];
		}
	}


	{	//eliminate implausible runs
		unsigned int run = 1;	//the first element starts as a run of 1

		for( i=FR_CHANCE_HISTORY_LENGTH-2; i>=0; i-- )
		{	//count size of the most recent run
			if( mHistory[i] == mHistory[i+1] ) {
				run++;
			}
			else {
				break;
			}
		}
		
		if( mHistory[FR_CHANCE_HISTORY_LENGTH-1] && run > MaxTrueRun[adjchance_int] )
		{
			#ifdef FR_CHANCE_DIAGNOSTICS
			printf(",");
			#endif

			mHistory[FR_CHANCE_HISTORY_LENGTH-1] = 0;
		}
		else if( !mHistory[FR_CHANCE_HISTORY_LENGTH-1] && run > MaxFalseRun[adjchance_int] )
		{
			#ifdef FR_CHANCE_DIAGNOSTICS
			printf(",");
			#endif

			mHistory[FR_CHANCE_HISTORY_LENGTH-1] = 1;
		}	
	}

	//eliminate repeating motifs of size 4, like 01110111 where 0111 is the motif
	if( chance >= 0.4f && chance <= 0.6f )
	{	//enforce for around the 50% chance case
		if( mHistory[FR_CHANCE_HISTORY_LENGTH-1] == mHistory[FR_CHANCE_HISTORY_LENGTH-5] &&
			mHistory[FR_CHANCE_HISTORY_LENGTH-2] == mHistory[FR_CHANCE_HISTORY_LENGTH-6] &&
			mHistory[FR_CHANCE_HISTORY_LENGTH-3] == mHistory[FR_CHANCE_HISTORY_LENGTH-7] &&
			mHistory[FR_CHANCE_HISTORY_LENGTH-4] == mHistory[FR_CHANCE_HISTORY_LENGTH-8] )
		{
			#ifdef FR_CHANCE_DIAGNOSTICS
			printf(".");
			#endif
			
			if( mHistory[FR_CHANCE_HISTORY_LENGTH-1] == 0 ) {
				mHistory[FR_CHANCE_HISTORY_LENGTH-1] = 1;
			}
			else {
				mHistory[FR_CHANCE_HISTORY_LENGTH-1] = 0;
			}
		}
	}

	//eliminate 111000 and 000111 pattern
	if( chance >= 0.4f && chance <= 0.6f )
	{	//enforce for around the 50% chance case
		if( (mHistory[FR_CHANCE_HISTORY_LENGTH-1] == 0 && mHistory[FR_CHANCE_HISTORY_LENGTH-4] == 1 &&
			 mHistory[FR_CHANCE_HISTORY_LENGTH-2] == 0 && mHistory[FR_CHANCE_HISTORY_LENGTH-5] == 1 &&
			 mHistory[FR_CHANCE_HISTORY_LENGTH-3] == 0 && mHistory[FR_CHANCE_HISTORY_LENGTH-6] == 1) ||
			(mHistory[FR_CHANCE_HISTORY_LENGTH-1] == 1 && mHistory[FR_CHANCE_HISTORY_LENGTH-4] == 0 &&
			 mHistory[FR_CHANCE_HISTORY_LENGTH-2] == 1 && mHistory[FR_CHANCE_HISTORY_LENGTH-5] == 0 &&
			 mHistory[FR_CHANCE_HISTORY_LENGTH-3] == 1 && mHistory[FR_CHANCE_HISTORY_LENGTH-6] == 0) )
		{
			#ifdef FR_CHANCE_DIAGNOSTICS
			printf("^");
			#endif			

			if( mHistory[FR_CHANCE_HISTORY_LENGTH-1] == 0 ) {
				mHistory[FR_CHANCE_HISTORY_LENGTH-1] = 1;
			}
			else {
				mHistory[FR_CHANCE_HISTORY_LENGTH-1] = 0;
			}
		}
	}


	if( flip ) {	//requested percentage (chance) is above 0.50, so flip result
		return( !mHistory[FR_CHANCE_HISTORY_LENGTH-1] );
	}
	else {
		return( mHistory[FR_CHANCE_HISTORY_LENGTH-1] );
	}
}






CFilteredRandomRange::CFilteredRandomRange( void )
{
	mRepeatingRunLength = 1;

	for( int i=0; i<FR_RANGE_HISTORY_LENGTH; i++ ) {
		mHistory[i] = InitialHistoryRange[i];
	}
}


int CFilteredRandomRange::Generate( CRandom & random, int range )
{
	int i;
	int escape = 0;			//Escapes while loop if sequence becomes overconstrained
	bool change = true;		//If the sequence was alterred and must be reexamined by each rule

   BPE_ASSERT( range > 2, "CFilteredRandomRange::Generate - range arg must be >= 3" );
	BPE_ASSERT( range < 256, "CFilteredRandomRange::Generate - range arg must be < 256" );

	for( i=0; i<FR_RANGE_HISTORY_LENGTH-1; i++ )
	{	//move history down
		mHistory[i] = mHistory[i+1];
	}

	while( change && escape < 50 )
	{
		change = false;
		escape++;

		//Get random value based on chance
		mHistory[FR_RANGE_HISTORY_LENGTH-1] = random.Random16() % range;


		{	//Allow correct number of repeating numbers as specified
			int i = 2;
			unsigned int runlength = 1;
			while( mHistory[FR_RANGE_HISTORY_LENGTH-1] == mHistory[FR_RANGE_HISTORY_LENGTH-i] )
			{
				runlength++;
				if( runlength > mRepeatingRunLength )
				{
					#ifdef FR_RANGE_DIAGNOSTICS
					printf( "(%d)", mHistory[FR_RANGE_HISTORY_LENGTH-1] );
					#endif

					change = true;
					break;
				}

				i++;
				if( i > FR_RANGE_HISTORY_LENGTH )
				{
					break;
				}
			}
			if( change ) {
				continue;
			}
		}
		
		//Check for more than 3 in the last 10
		if( range >= 8 )
		{	
			int count = 1;
			for( i=1; i<10; i++ )
			{
				if( mHistory[FR_RANGE_HISTORY_LENGTH-1-i] == mHistory[FR_RANGE_HISTORY_LENGTH-1] ) {
					count++;
				}
			}
			if( count > 3 ) {
				#ifdef FR_RANGE_DIAGNOSTICS
				printf( "[%d]", mHistory[FR_RANGE_HISTORY_LENGTH-1] );
				#endif

				change = true;
				continue;
			}
		}
		else if (range >= 5 )
		{	//Check for more than 4 in the last 10
			int count = 1;
			for( i=1; i<10; i++ )
			{
				if( mHistory[FR_RANGE_HISTORY_LENGTH-1-i] == mHistory[FR_RANGE_HISTORY_LENGTH-1] ) {
					count++;
				}
			}
			if( count > 4 ) {
				#ifdef FR_RANGE_DIAGNOSTICS
				printf( "[%d]", mHistory[FR_RANGE_HISTORY_LENGTH-1] );
				#endif

				change = true;
				continue;
			}
		}
		else
		{	//Check for more than 5 in the last 10
			int count = 1;
			for( i=1; i<10; i++ )
			{
				if( mHistory[FR_RANGE_HISTORY_LENGTH-1-i] == mHistory[FR_RANGE_HISTORY_LENGTH-1] ) {
					count++;
				}
			}
			if( count > 5 ) {
				#ifdef FR_RANGE_DIAGNOSTICS
				printf( "[%d]", mHistory[FR_RANGE_HISTORY_LENGTH-1] );
				#endif
				
				change = true;
				continue;
			}
		}


		//Check for more than 2 in a counting sequence
		if( range > 6 )
		{	
			if( (mHistory[FR_RANGE_HISTORY_LENGTH-1] + 1 == mHistory[FR_RANGE_HISTORY_LENGTH-2] &&
				 mHistory[FR_RANGE_HISTORY_LENGTH-1] + 2 == mHistory[FR_RANGE_HISTORY_LENGTH-3]) ||
				(mHistory[FR_RANGE_HISTORY_LENGTH-1] == mHistory[FR_RANGE_HISTORY_LENGTH-2] + 1  &&
				 mHistory[FR_RANGE_HISTORY_LENGTH-1] == mHistory[FR_RANGE_HISTORY_LENGTH-3] + 2)
				)
			{
				#ifdef FR_RANGE_DIAGNOSTICS				
				printf( "{%d}", mHistory[FR_RANGE_HISTORY_LENGTH-1] );
				#endif
				
				change = true;	
				continue;
			}
		}
		else
		{	//Check for more than 3 in a counting sequence
			if( (mHistory[FR_RANGE_HISTORY_LENGTH-1] + 1 == mHistory[FR_RANGE_HISTORY_LENGTH-2] &&
				 mHistory[FR_RANGE_HISTORY_LENGTH-1] + 2 == mHistory[FR_RANGE_HISTORY_LENGTH-3] &&
				 mHistory[FR_RANGE_HISTORY_LENGTH-1] + 3 == mHistory[FR_RANGE_HISTORY_LENGTH-4]) ||
				(mHistory[FR_RANGE_HISTORY_LENGTH-1] == mHistory[FR_RANGE_HISTORY_LENGTH-2] + 1 &&
				 mHistory[FR_RANGE_HISTORY_LENGTH-1] == mHistory[FR_RANGE_HISTORY_LENGTH-3] + 2 &&
				 mHistory[FR_RANGE_HISTORY_LENGTH-1] == mHistory[FR_RANGE_HISTORY_LENGTH-4] + 3)
				)
			{
				#ifdef FR_RANGE_DIAGNOSTICS				
				printf( "{%d}", mHistory[FR_RANGE_HISTORY_LENGTH-1] );
				#endif				

				change = true;
				continue;
			}
		}


		//Check for no more than 4 in a row being at the bottom of the range or the top of the range
		if( range > 8 )
		{
			if( (mHistory[FR_RANGE_HISTORY_LENGTH-1] < range/2 &&
				 mHistory[FR_RANGE_HISTORY_LENGTH-2] < range/2 &&
				 mHistory[FR_RANGE_HISTORY_LENGTH-3] < range/2 &&
				 mHistory[FR_RANGE_HISTORY_LENGTH-4] < range/2 &&
				 mHistory[FR_RANGE_HISTORY_LENGTH-5] < range/2) ||
				(mHistory[FR_RANGE_HISTORY_LENGTH-1] >= range/2 &&
				 mHistory[FR_RANGE_HISTORY_LENGTH-2] >= range/2 &&
				 mHistory[FR_RANGE_HISTORY_LENGTH-3] >= range/2 &&
				 mHistory[FR_RANGE_HISTORY_LENGTH-4] >= range/2 &&
				 mHistory[FR_RANGE_HISTORY_LENGTH-5] >= range/2) )
			{
				#ifdef FR_RANGE_DIAGNOSTICS
				printf( "$%d$", mHistory[FR_RANGE_HISTORY_LENGTH-1] );
				#endif

				change = true;
				continue;
			}

		}


		//Check for two pairs right next to each other (like 2255)
		if( mHistory[FR_RANGE_HISTORY_LENGTH-1] == mHistory[FR_RANGE_HISTORY_LENGTH-2] &&
			mHistory[FR_RANGE_HISTORY_LENGTH-3] == mHistory[FR_RANGE_HISTORY_LENGTH-4] )
		{
			#ifdef FR_RANGE_DIAGNOSTICS
			printf( "#%d#", mHistory[FR_RANGE_HISTORY_LENGTH-1] );
			#endif
			
			change = true;
			continue;
		}


		//Check for a motif of 2 repeating immediately
		if( range > 3 )
		{
			if( mHistory[FR_RANGE_HISTORY_LENGTH-1] == mHistory[FR_RANGE_HISTORY_LENGTH-3] &&
				mHistory[FR_RANGE_HISTORY_LENGTH-2] == mHistory[FR_RANGE_HISTORY_LENGTH-4] )
			{
				#ifdef FR_RANGE_DIAGNOSTICS
				printf( "-%d-", mHistory[FR_RANGE_HISTORY_LENGTH-1] );
				#endif

				change = true;
				continue;
			}
		}


		//Check for a motif (or mirror motif) of 3 repeating in the last 10
		if( range > 5 )
		{	
			for( i=3; i<7; i++ )
			{
				if( (mHistory[FR_RANGE_HISTORY_LENGTH-1] == mHistory[FR_RANGE_HISTORY_LENGTH-1-i] &&
					 mHistory[FR_RANGE_HISTORY_LENGTH-2] == mHistory[FR_RANGE_HISTORY_LENGTH-1-i-1] &&
					 mHistory[FR_RANGE_HISTORY_LENGTH-3] == mHistory[FR_RANGE_HISTORY_LENGTH-1-i-2]) ||
					(mHistory[FR_RANGE_HISTORY_LENGTH-1] == mHistory[FR_RANGE_HISTORY_LENGTH-1-i-2] &&
					 mHistory[FR_RANGE_HISTORY_LENGTH-2] == mHistory[FR_RANGE_HISTORY_LENGTH-1-i-1] &&
					 mHistory[FR_RANGE_HISTORY_LENGTH-3] == mHistory[FR_RANGE_HISTORY_LENGTH-1-i]) )
				{
					#ifdef FR_RANGE_DIAGNOSTICS
					printf( "*%d*", mHistory[FR_RANGE_HISTORY_LENGTH-1] );
					#endif

					change = true;
					break;
				}
			}
			if( change ) {
				continue;
			}
		}

	}

	return( mHistory[FR_RANGE_HISTORY_LENGTH-1] );

}






CFilteredRandomReal::CFilteredRandomReal( void )
{
	for( int i=0; i<FR_REAL_HISTORY_LENGTH; i++ )
	{
		mHistory[i] = InitialHistoryReal[i];
	}
}


float CFilteredRandomReal::Generate(CRandom & random)
{
	int i;
	int escape = 0;
	
	for( i=0; i<FR_REAL_HISTORY_LENGTH-1; i++ )
	{	//Move history down
		mHistory[i] = mHistory[i+1];
	}

	mChange = true;
	while( mChange && escape < 50 )
	{
		mChange = false;
		escape++;

		//Get the whole number from a filtered random source
		float whole = (float)mFilteredRandomRange.Generate(random, 10);

		//Allow a repeating run of two
		mFilteredRandomRange.SetRepeatingRunLength(2);

		//Let the fractional be completely random
		float fractional = random.RandomFloat();
		
		//Combine the whole and fractional
		float candidate = whole + fractional;

		//Move the number into the [0,1] range
		candidate = candidate * 0.1f;

		if( candidate > 1.0f )
		{	//Because of floating-point round-off, ensure upper bound
			candidate = 1.0f;
		}

		mHistory[FR_REAL_HISTORY_LENGTH-1] = candidate;

		{	//Check that the last 3 numbers were more than 0.1 away from each other
			float diff1_2 = mHistory[FR_REAL_HISTORY_LENGTH-1] - mHistory[FR_REAL_HISTORY_LENGTH-2];
			float diff1_3 = mHistory[FR_REAL_HISTORY_LENGTH-1] - mHistory[FR_REAL_HISTORY_LENGTH-3];
			float diff2_3 = mHistory[FR_REAL_HISTORY_LENGTH-2] - mHistory[FR_REAL_HISTORY_LENGTH-3];

			if( (diff1_2 <= 0.1f && diff1_2 >= -0.1f) &&
				(diff1_3 <= 0.1f && diff1_3 >= -0.1f) &&
				(diff2_3 <= 0.1f && diff2_3 >= -0.1f) )
			{
				mChange = true;
				continue;
			}
		}

		{	//Check that the last 2 numbers are more than 0.02 away from each other
			float diff =  mHistory[FR_REAL_HISTORY_LENGTH-1] - mHistory[FR_REAL_HISTORY_LENGTH-2];
			if( diff <= 0.02f && diff >= -0.02f )
			{
				mChange = true;
				continue;
			}
		}


		{	//Check that the last 5 numbers don't make an increasing/decreasing sequence
			if( (mHistory[FR_REAL_HISTORY_LENGTH-1] > mHistory[FR_REAL_HISTORY_LENGTH-2] &&
				 mHistory[FR_REAL_HISTORY_LENGTH-2] > mHistory[FR_REAL_HISTORY_LENGTH-3] &&
				 mHistory[FR_REAL_HISTORY_LENGTH-3] > mHistory[FR_REAL_HISTORY_LENGTH-4] &&
				 mHistory[FR_REAL_HISTORY_LENGTH-4] > mHistory[FR_REAL_HISTORY_LENGTH-5]) ||
				(mHistory[FR_REAL_HISTORY_LENGTH-1] < mHistory[FR_REAL_HISTORY_LENGTH-2] &&
				 mHistory[FR_REAL_HISTORY_LENGTH-2] < mHistory[FR_REAL_HISTORY_LENGTH-3] &&
				 mHistory[FR_REAL_HISTORY_LENGTH-3] < mHistory[FR_REAL_HISTORY_LENGTH-4] &&
				 mHistory[FR_REAL_HISTORY_LENGTH-4] < mHistory[FR_REAL_HISTORY_LENGTH-5]) )
			{
				mChange = true;
				continue;
			}
		}
		
	}

	return( mHistory[FR_REAL_HISTORY_LENGTH-1] );
}







CFilteredRandomGaussian::CFilteredRandomGaussian(void)
{
	mNextValue = INVALID_NEXT_VALUE_GAUSSIAN;

	for( int i=0; i<FR_GAUSSIAN_HISTORY_LENGTH; i++ )
	{
		mHistory[i] = InitialHistoryGaussian[i];
	}
}



float CFilteredRandomGaussian::Generate(CRandom & random)
{
	int i;
	bool change = true;

	for( i=0; i<FR_GAUSSIAN_HISTORY_LENGTH-1; i++ )
	{	//Move history down
		mHistory[i] = mHistory[i+1];
	}

	while( change )
	{
		change = false;

		if( mNextValue == INVALID_NEXT_VALUE_GAUSSIAN )
		{
			//Gaussian random number generator adapted from Everett Carter's 
			//article "Generating Gaussian Random Numbers" (www.taygeta.com/random/gaussian.html)

			float x1, x2, w, y1, y2;

			do {
				x1 = 2.0f*random.RandomFloat() - 1.0f;
				x2 = 2.0f*random.RandomFloat() - 1.0f;
				w = x1 * x1 + x2 * x2;
			} while ( w >= 1.0 );

			w = (float)sqrtf( (-2.0f * log10( w ) ) / w );


			//Generate two random numbers at a time, store one for
			//the next time Generate is called.
			y1 = (x1 * w) / 1.5f;
			y2 = (x2 * w) / 1.5f;
			if( y1 > 1.0f ) { y1 = 1.0f; }
			if( y1 < -1.0f ) { y1 = -1.0f; }
			if( y2 > 1.0f ) { y2 = 1.0f; }
			if( y2 < -1.0f ) { y2 = -1.0f; }
			mNextValue = y2;	//The stored random value for the next time
			mHistory[FR_GAUSSIAN_HISTORY_LENGTH-1] = y1;
		}
		else
		{
			mHistory[FR_GAUSSIAN_HISTORY_LENGTH-1] = mNextValue;
			mNextValue = INVALID_NEXT_VALUE_GAUSSIAN;
		}

		{	//Check that the there are not more than 3 numbers in a row above or below zero.
			if( (mHistory[FR_GAUSSIAN_HISTORY_LENGTH-1] < 0.0f &&
				 mHistory[FR_GAUSSIAN_HISTORY_LENGTH-2] < 0.0f &&
				 mHistory[FR_GAUSSIAN_HISTORY_LENGTH-3] < 0.0f &&
				 mHistory[FR_GAUSSIAN_HISTORY_LENGTH-4] < 0.0f) ||
				(mHistory[FR_GAUSSIAN_HISTORY_LENGTH-1] > 0.0f &&
				 mHistory[FR_GAUSSIAN_HISTORY_LENGTH-2] > 0.0f &&
				 mHistory[FR_GAUSSIAN_HISTORY_LENGTH-3] > 0.0f &&
				 mHistory[FR_GAUSSIAN_HISTORY_LENGTH-4] > 0.0f) )
			{
				#ifdef FR_GAUSSIAN_DIAGNOSTICS
				printf("a");
				#endif

				change = true;
				continue;
			}
		}

		{	//Check that the last 3 numbers were more than 0.1 away from each other
			double diff1_2 = mHistory[FR_GAUSSIAN_HISTORY_LENGTH-1] - mHistory[FR_GAUSSIAN_HISTORY_LENGTH-2];
			double diff1_3 = mHistory[FR_GAUSSIAN_HISTORY_LENGTH-1] - mHistory[FR_GAUSSIAN_HISTORY_LENGTH-3];
			double diff2_3 = mHistory[FR_GAUSSIAN_HISTORY_LENGTH-2] - mHistory[FR_GAUSSIAN_HISTORY_LENGTH-3];

			if( (diff1_2 <= 0.1f && diff1_2 >= -0.1f) &&
				(diff1_3 <= 0.1f && diff1_3 >= -0.1f) &&
				(diff2_3 <= 0.1f && diff2_3 >= -0.1f) )
			{
				#ifdef FR_GAUSSIAN_DIAGNOSTICS
				printf("b");
				#endif

				change = true;
				continue;
			}
		}

		{	//Check that the last 2 numbers are more than 0.02 away from each other
			double diff =  mHistory[FR_GAUSSIAN_HISTORY_LENGTH-1] - mHistory[FR_GAUSSIAN_HISTORY_LENGTH-2];
			if( diff <= 0.02f && diff >= -0.02f )
			{
				#ifdef FR_GAUSSIAN_DIAGNOSTICS
				printf("c");
				#endif

				change = true;
				continue;
			}
		}

		{	//Check that the last 5 numbers don't make an increasing/decreasing sequence
			if( (mHistory[FR_GAUSSIAN_HISTORY_LENGTH-1] > mHistory[FR_GAUSSIAN_HISTORY_LENGTH-2] &&
				 mHistory[FR_GAUSSIAN_HISTORY_LENGTH-2] > mHistory[FR_GAUSSIAN_HISTORY_LENGTH-3] &&
				 mHistory[FR_GAUSSIAN_HISTORY_LENGTH-3] > mHistory[FR_GAUSSIAN_HISTORY_LENGTH-4] &&
				 mHistory[FR_GAUSSIAN_HISTORY_LENGTH-4] > mHistory[FR_GAUSSIAN_HISTORY_LENGTH-5]) ||
				(mHistory[FR_GAUSSIAN_HISTORY_LENGTH-1] < mHistory[FR_GAUSSIAN_HISTORY_LENGTH-2] &&
				 mHistory[FR_GAUSSIAN_HISTORY_LENGTH-2] < mHistory[FR_GAUSSIAN_HISTORY_LENGTH-3] &&
				 mHistory[FR_GAUSSIAN_HISTORY_LENGTH-3] < mHistory[FR_GAUSSIAN_HISTORY_LENGTH-4] &&
				 mHistory[FR_GAUSSIAN_HISTORY_LENGTH-4] < mHistory[FR_GAUSSIAN_HISTORY_LENGTH-5]) )
			{
				#ifdef FR_GAUSSIAN_DIAGNOSTICS
				printf("d");
				#endif

				change = true;
				continue;
			}
		}

	}

	return( mHistory[FR_GAUSSIAN_HISTORY_LENGTH-1] );
}

