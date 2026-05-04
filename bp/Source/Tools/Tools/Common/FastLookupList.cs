using System;
using System.Collections.Generic;
using System.Text;
using System.Collections.ObjectModel;

namespace Tools.Common
{
   public class FastLookupList<Key,Value> : IEnumerable<Value>, System.Collections.IEnumerable, ICloneable
   {
      private List<Value> mList;
      private SortedDictionary<Key, Value> mDict;

      public FastLookupList()
      {
         mList = new List<Value>();
         mDict = new SortedDictionary<Key, Value>();
      }

      public FastLookupList( FastLookupList<Key,Value> src )
      {
         mList = new List<Value>( src.mList );
         mDict = new SortedDictionary<Key, Value>();

         foreach ( KeyValuePair<Key,Value> kv in src.mDict )
         {
            mDict.Add( kv.Key, kv.Value );
         }
      }

      #region IEnumerable<Value> Members

      IEnumerator<Value> IEnumerable<Value>.GetEnumerator()
      {
         return mList.GetEnumerator();
      }

      #endregion

      #region IEnumerable Members

      System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
      {
         return mList.GetEnumerator();
      }

      #endregion

      #region ICloneable Members

      public object Clone()
      {
         return new FastLookupList<Key, Value>( this );
      }

      #endregion

      public void Add( Key key, Value value )
      {
         if ( !mDict.ContainsKey( key ) )
         {
            mList.Add( value );
            mDict[key] = value;
         }
         else
         {
            throw new System.ArgumentException( "Key already exists in list", "key" );
         }
      }

      public void Remove( Key key )
      {
         Value value;

         if ( mDict.TryGetValue( key, out value ) )
         {
            mDict.Remove( key );
            mList.Remove( value );
         }
      }

      public bool ContainsKey( Key key )
      {
         return mDict.ContainsKey( key );
      }

      public Value Get( Key key )
      {
         return mDict[key];
      }

      public Value this[ Key key ]
      {
         get
         {
            return mDict[key];
         }
      }

      public void SetExistingValue( Key key, Value value)
      {
         Value oldValue;

         if ( mDict.TryGetValue( key, out oldValue ) )
         {
            int index = mList.FindIndex( new Predicate<Value>( delegate( Value v ) { return (Object) v == (Object) oldValue; } ) );
            mList[index] = value;
            mDict[key] = value;
         }
         else
         {
            throw new System.ArgumentOutOfRangeException( "key" );
         }

      }

      public ReadOnlyCollection<Value> AsReadOnly()
      {
         return mList.AsReadOnly();
      }

      public IEnumerable<Key> Keys
      {
         get { return mDict.Keys; }
      }

      public int Count
      {
         get { return mList.Count;  }
      }

   }
}
