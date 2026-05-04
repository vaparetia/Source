//------------------------------------------------------------------------------------------
// CMergeContainer.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "map"

//------------------------------------------------------------------------------------------

template< class T > class CMergeContainer
{
   typedef std::multimap<uint32, int> TFinderHelperMap;
public:

   explicit CMergeContainer( std::vector<T> const & data, bool const buildFinderHelper = false )
   :  mArray( data )
   {
      if( buildFinderHelper )
      {
         for( int i = 0; i < mArray.size(); ++i )
         {
            mFindHelper.insert( TFinderHelperMap::value_type( mArray[i].GetHashValue(), i ) );
         }
      }
   }

   CMergeContainer()
   {
   }

   // TODO: Rename this function so it indicates that it also inserts the elements if not present.
   int const GetIndexForElement( T const & toFind )
   {
      std::pair< TFinderHelperMap::const_iterator, TFinderHelperMap::const_iterator > const range = mFindHelper.equal_range( toFind.GetHashValue() );
      
      for( TFinderHelperMap::const_iterator it = range.first; it != range.second; ++it )
      {
         int const & index = it->second;
         T const &compare = mArray[index];
         if( compare == toFind )
         {
            return index;
         }
      }
   
      // no match found, have to insert it ourselves
      int const index = static_cast<int>( mArray.size() );
      
      mArray.push_back( toFind );
      mFindHelper.insert( TFinderHelperMap::value_type( toFind.GetHashValue(), index ) );
   
      return index;
   }

   std::vector<T> const & GetArray() const { return mArray; }
   void Clear() { mArray.clear(); mFindHelper.clear(); }

   // this will clear out the information used to find common vertices, with it gone, any new vertices added CANT be shared with the old ones
   void ResetFindInfo() { mFindHelper.clear(); }

private:
   std::vector<T>    mArray;
   TFinderHelperMap  mFindHelper;
};

