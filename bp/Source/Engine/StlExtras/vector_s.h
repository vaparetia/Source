//------------------------------------------------------------------------------------------
// vector_s.h
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include <vector>

//------------------------------------------------------------------------------------------

class CInputStream;
class COutputStream;

//------------------------------------------------------------------------------------------
#if !defined(SPU)

namespace bpe
{
   template <class X> class vector_s : public std::vector<X>
   {
      typedef std::vector<X> inherited;
   public:
      typedef typename inherited::const_iterator const_iterator;

      vector_s()
      {
      }

      vector_s( inherited const & other )
         : inherited( other )
      {
      }

      vector_s( CInputStream & inStream )
      {
         int const size = inStream.ReadUint32();

         this->reserve( size );

         for( int i = 0; i < size; ++i )
         {
            this->push_back( inStream.Get( TType<X>() ) );
         }
      }

      // Allows you to build vector after default construction without a copy
      void ConstructFromStream( CInputStream & inStream )
      {
         // Reset any allocated memory
         *this = vector_s<X>();

         int const size = inStream.ReadUint32();

         this->reserve( size );

         for( int i = 0; i < size; ++i )
         {
            this->push_back( inStream.Get( TType<X>() ) );
         }
      }

      void PutTo( COutputStream & outStream ) const
      {
         outStream.WriteUint32(this->size());

         for( const_iterator current = this->begin(); current != this->end(); ++current )
         {
            outStream.Put(*current);
         }
      }
   };

} // end namespace std

#else //#if !defined(SPU)
// SPU version that is intended to overlay on regular vector
namespace bpe
{
   template <class T>   class vector_s
   {
   public:
      typedef T value_type;
      typedef value_type &reference;
      typedef value_type const &const_reference;
      typedef value_type *pointer;
      typedef value_type const  *const_pointer;

      typedef int size_type;

      typedef pointer iterator;
      typedef const_pointer const_iterator;

   public:
      vector_s()
         : _Myfirst(0)
         , _Mylast(0)
         , _Myend(0)
      {
      }

      ~vector_s()
      {
         BPE_ASSERTA( "No vector_s destructor on SPU");
      }

      size_type const capacity() const
      {
		   return (_Myfirst == 0 ? 0 : _Mylast - _Myfirst);
      }
      
      size_type const back_capacity() const
      {
         return capacity() - size();
      }
      
      iterator const begin()
      {
         return _Myfirst;
      }
      
      const_iterator const begin() const
      {
         return _Myfirst;
      }
      
      iterator const end()
      {
         return _Mylast;
      }
      
      const_iterator const end() const
      {
         return _Mylast;
      }
      
      size_type const size() const
      {
		   return (_Myfirst == 0 ? 0 : _Mylast - _Myfirst);
      }
      
      size_type const max_size() const
      {
		   return (_Myfirst == 0 ? 0 : _Mylast - _Myfirst);
      }
      
      bool const empty() const
      {
         return size() == 0;
      }
      
      pointer const data()
      {
         return _Myfirst;
      }
      
      const_pointer const data() const
      {
         return _Myfirst;
      }
      
      reference front()
      {
         return *_Myfirst;
      }
      
      const_reference front() const
      {
         return *_Myfirst;
      }
      
      reference back()
      {
         return *_Mylast;
      }
      
      const_reference back() const
      {
         return *_Mylast;
      }

      reference at( size_type const pos )
      {
         BPE_ASSERT( pos >= 0 && pos < size(), "Invalid position!" );
         return *(_Myfirst + pos);
      }

      reference nc_at( size_type const pos )
      {
         BPE_ASSERT( pos >= 0 && pos < size(), "Invalid position!" );
         return *(_Myfirst + pos);
      }

      const_reference at( size_type const pos ) const
      {
         BPE_ASSERT( pos >= 0 && pos < size(), "Invalid position!" );
         return *(_Myfirst + pos);
      }

      reference operator[] ( size_type const pos )
      {
         BPE_ASSERT( pos >= 0 && pos < size(), "Invalid position!" );
         return *(_Myfirst + pos);
      }

      const_reference operator[] ( size_type const pos ) const
      {
         BPE_ASSERT( pos >= 0 && pos < size(), "Invalid position!" );
         return *(_Myfirst + pos);
      }

      void push_back( T const &what )
      {
         BPE_ASSERTA( "No vector_s::push_back on SPU!");
      }

      iterator erase( iterator it )
      {
         BPE_ASSERTA( "No vector_s::erase on SPU!");
         return end();
      }

      vector_s<T> &operator= ( vector_s<T> const &copy )
      {
         BPE_ASSERTA( "No vector_s::operator= on SPU!");
         return *this;
      }

      void resize( size_type const n, value_type const &def )
      {
         BPE_ASSERTA( "No vector_s::resize on SPU!");
      }

      void pop_back()
      {
         BPE_ASSERTA( "No vector_s::pop_back on SPU!");
      }

      void clear()
      {
         BPE_ASSERTA( "No vector_s::clear on SPU!");
      }

   private:
      uint32   _Pad1;      // Padding added so it matches std::vector (w/o _HAS_ITERATOR_DEBUGGING)
      pointer  _Myfirst;	// pointer to beginning of array
	   pointer  _Mylast;    // pointer to current end of sequence
	   pointer  _Myend;	   // pointer to end of array
   };
}

#endif //!defined(SPU)
//------------------------------------------------------------------------------------------

