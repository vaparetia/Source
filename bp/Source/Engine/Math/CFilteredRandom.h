/*************************************************
  
  File: FilteredRandom.h

  Author: Steve Rabin
  Last Revision: August 1st, 2003

  This code is from the article "Filtered Randomness for AI
  Decisions and Game Logic" in the book AI Game Programming Wisdom 2 
  and may be used freely in any commercial software product.
  
  Updates to this source code can be found at www.aiwisdom.com.
  Feedback can be sent to steve@aiwisdom.com.

**************************************************/


#ifndef __FILTEREDRANDOM_H__
#define __FILTEREDRANDOM_H__

#define FR_CHANCE_HISTORY_LENGTH    20
#define FR_RANGE_HISTORY_LENGTH     10
#define FR_REAL_HISTORY_LENGTH      5
#define FR_GAUSSIAN_HISTORY_LENGTH  5

class CRandom;

//
// Produces filtered random Boolean numbers given a chance
//
class CFilteredRandomChance
{
public:

	CFilteredRandomChance( void );
	~CFilteredRandomChance( void ) {}

	bool Generate(CRandom & random, float chance);

protected:
	
	bool mHistory[FR_CHANCE_HISTORY_LENGTH];

};

//
// Produces filtered random numbers in the range [0,range]
//
class CFilteredRandomRange
{
public:

	CFilteredRandomRange( void );
	~CFilteredRandomRange( void ) {}

	int Generate(CRandom & random, int range);

	//Example: repeating length of 2 allows the sequence 348823 but not 3488823 
	void SetRepeatingRunLength( unsigned int length ) { mRepeatingRunLength = length; }

protected:

	unsigned int mRepeatingRunLength;
	int mHistory[FR_RANGE_HISTORY_LENGTH];

};


//
// Produces filtered random numbers in the range [0,1]
//
class CFilteredRandomReal
{
public:

	CFilteredRandomReal( void );
	~CFilteredRandomReal( void ) {}

	float Generate(CRandom & random);

protected:

	bool mChange;
	float mHistory[FR_REAL_HISTORY_LENGTH];
	CFilteredRandomRange mFilteredRandomRange;

};


//
// Produces filtered random numbers with a Gaussian distribution in the range [0,1]
//
class CFilteredRandomGaussian
{
public:

	CFilteredRandomGaussian( void );
	~CFilteredRandomGaussian( void ) {}

	float Generate(CRandom & random);

protected:

	float mNextValue;
	float mHistory[FR_GAUSSIAN_HISTORY_LENGTH];

};


#endif // __FILTEREDRANDOM_H__ 
