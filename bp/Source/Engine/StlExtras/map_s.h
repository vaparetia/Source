//------------------------------------------------------------------------------------------
// map_s.h
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include <map>

//------------------------------------------------------------------------------------------

class CInputStream;
class COutputStream;

//------------------------------------------------------------------------------------------

namespace bpe
{
   template <class X, class Y> class map_s : public std::map<X, Y>
   {
   public:
      typedef typename std::map<X, Y>::const_iterator const_iterator;

      map_s()
      : std::map<X, Y>()
      {
      }

      map_s( CInputStream & inStream )
      {
         int const size = inStream.ReadUint32();

         for( int i = 0; i < size; ++i )
         {
            X x = inStream.Get(TType<X>());
            Y y = inStream.Get(TType<Y>());
            insert( make_pair(x, y) );
         }
      }
   
      void PutTo( COutputStream & outStream ) const
      {
         outStream.Put( static_cast<uint32>( this->size() ) );

         for ( const_iterator current = this->begin(); current != this->end(); ++current )
         {
            outStream.Put( current->first );
            outStream.Put( current->second );
         }
      }
   };

} // end namespace bpe

//------------------------------------------------------------------------------------------


