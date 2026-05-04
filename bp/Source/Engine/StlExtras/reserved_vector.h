//----------------------------------------------------------------------------
// reserved_vector.h
// 2005

//----------------------------------------------------------------------------

#ifndef __RESERVED_VECTOR_H__
#define __RESERVED_VECTOR_H__

//----------------------------------------------------------------------------

namespace bpe
{
   template <class T, int Capacity> class reserved_vector
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

      enum
      {
         static_capacity = Capacity
      };

   public:
      reserved_vector()
      :  mSize( 0 )
      {
      }

      explicit reserved_vector( size_type const size, value_type const &def )
      :  mSize( size )
      {
         BPE_ASSERT( size >= 0, "Size must not be negative!" );
         BPE_ASSERT( size <= Capacity, "Size can't exceed capacity!" );

#if BPE_TARGET == BPE_TARGET_WIN32
         _STDEXT unchecked_uninitialized_fill_n( data(), size, def );
#else
         std::uninitialized_fill_n( data(), size, def );
#endif
      }

      reserved_vector ( reserved_vector<T,Capacity> const &copy )
      :  mSize( copy.mSize )
      {
#if BPE_TARGET == BPE_TARGET_WIN32
         _STDEXT unchecked_uninitialized_copy( copy.data(), copy.data() + mSize, data(), std::allocator<T>());
#else
         std::uninitialized_copy( copy.data(), copy.data() + mSize, data());
#endif
      }

      template <class It>
      explicit reserved_vector( It const first, 
                                It const last )
      : mSize( 0 )
      {
         // allocate
         mSize = distance( first, last );

         BPE_ASSERT( mSize <= Capacity, "Size exceeds capacity!" );
         std::uninitialized_copy( first, last, data() );
      }

      ~reserved_vector()
      {
         destroy_all();
      }

      size_type const capacity() const
      {
         return Capacity;
      }
      
      size_type const back_capacity() const
      {
         return capacity() - mSize;
      }
      
      iterator const begin()
      {
         return data();
      }
      
      const_iterator const begin() const
      {
         return data();
      }
      
      iterator const end()
      {
         return data() + mSize;
      }
      
      const_iterator const end() const
      {
         return data() + mSize;
      }
      
      size_type const size() const
      {
         return mSize;
      }
      
      size_type const max_size() const
      {
         return Capacity;
      }
      
      bool const empty() const
      {
         return mSize == 0;
      }
      
      pointer const data()
      {
         return reinterpret_cast<pointer>(reinterpret_cast<void *>(mData) );
      }
      
      const_pointer const data() const
      {
         return const_cast<const_pointer const>(reinterpret_cast<T const * const>(mData) );
      }
      
      reference front()
      {
         return at(0);
      }
      
      const_reference front() const
      {
         return at(0);
      }
      
      reference back()
      {
         return at( mSize-1 );
      }
      
      const_reference back() const
      {
         return at( mSize-1 );
      }

      reference at( size_type const pos )
      {
         BPE_ASSERT( pos >= 0 && pos < mSize, "Invalid position!" );
         return data() [ pos ];
      }

      reference nc_at( size_type const pos )
      {
         BPE_ASSERT( pos >= 0 && pos < mSize, "Invalid position!" );
         return data() [ pos ];
      }

      const_reference at( size_type const pos ) const
      {
         BPE_ASSERT( pos >= 0 && pos < mSize, "Invalid position!" );
         return data() [ pos ];
      }

      reference operator[] ( size_type const pos )
      {
         BPE_ASSERT( pos >= 0 && pos < mSize, "Invalid position!" );
         return data() [ pos ];
      }

      const_reference operator[] ( size_type const pos ) const
      {
         BPE_ASSERT( pos >= 0 && pos < mSize, "Invalid position!" );
         return data() [ pos ];
      }

      void push_back( T const &what )
      {
         BPE_ASSERT( mSize + 1 <= Capacity, "Can't push_back past Capacity!\n" );

         std::_Construct(data() + mSize, what );
         mSize++;
      }

      iterator erase( iterator it )
      {
         BPE_ASSERT( it >= begin() && it < end(), "Can only erase items IN this" );

         if( it >= begin() && it < end() )
         {
            for( iterator assigning = it; assigning < end() - 1; ++assigning )
            {
               *assigning = * ( assigning + 1 );
            }

            std::_Destroy( end() - 1 );

            --mSize;
            return it; // pointer to data remains the same
         }
         else
         {
            // Invalid data was passed.  Let's gracefully get out of this
            return end();
         }
      }

      reserved_vector<T, Capacity> &operator= ( reserved_vector<T,Capacity> const &copy )
      {
         // self-check
         if( this != &copy )
         {
            // clear ourself
            destroy_all();

            // reserve and copy
#if BPE_TARGET == BPE_TARGET_WIN32
            _STDEXT unchecked_uninitialized_copy( copy.data(), copy.data() + copy.mSize, data() );
#else
            std::uninitialized_copy( copy.data(), copy.data() + copy.mSize, data() );
#endif
            mSize = copy.mSize;
         }

         // and return self
         return *this;
      }

      void resize( size_type const n, value_type const &def )
      {
         if( mSize == n )
         {
            return;
         }
         else if( mSize > n )
         {
            BPE_ASSERT( n >= 0, "Size must be greater than or equal to 0" );

            std::allocator<T> alloc_rv;
            std::_Destroy_range( begin() + n, end(), alloc_rv );
         }
         else
         {
            // mSize < n, we're sizing up
            BPE_ASSERT ( n <= Capacity, "Size exceeds capacity!" );

#if BPE_TARGET == BPE_TARGET_WIN32
            _STDEXT unchecked_uninitialized_fill_n( data() + mSize, n-mSize, def );
#else
            std::uninitialized_fill_n( data() + mSize, n-mSize, def );
#endif
         }

         mSize = n;
      }

      void pop_back()
      {
         BPE_ASSERT( mSize > 0, "Tried to pop from an empty vector!" );

         std::_Destroy( data() + mSize - 1 );
         mSize--;
      }

      void clear()
      {
         destroy_all();
         mSize = 0;
      }

   private:
      void destroy_all()
      {
         std::allocator<T> alloc_rv;
         std::_Destroy_range( data(), data() + mSize, alloc_rv );
      }

   private:
      size_type   mSize;
      char        mData[ sizeof(value_type) * Capacity ];
   };
}

#endif
