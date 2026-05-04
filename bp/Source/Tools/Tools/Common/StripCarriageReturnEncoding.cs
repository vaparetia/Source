using System;
using System.Collections.Generic;
using System.Text;

namespace Tools.Common
{
   public class StripCarriageReturnEncoding : System.Text.Encoding
   {
      new static public StripCarriageReturnEncoding ASCII
      {
         get { return new StripCarriageReturnEncoding( Encoding.ASCII ); }
      }

      public StripCarriageReturnEncoding( System.Text.Encoding baseEncoding )
         {
         mEncoder = baseEncoding;
         }

      private char[] StripCarriageReturns( char[] incoming, int index, int count )
      {
         int cr_count = 0;
         for ( int i = index; i < index + count; ++i )
         {
            char c = incoming[i];

            if ( c == '\r' )
            {
               ++cr_count;
            }
         }

         char[] stripped = new char[count - cr_count];

         int write = 0;
         for ( int read = index; read < index + count; ++read )
         {
            if ( incoming[read] != '\r' )
            {
               stripped[write] = incoming[read];
               ++write;
            }
         }

         return stripped;
      }

      public override int GetByteCount( char[] chars, int index, int count )
      {
         return mEncoder.GetByteCount( StripCarriageReturns( chars, index, count ) );
      }

      public override int GetBytes( char[] chars, int charIndex, int charCount, byte[] bytes, int byteIndex )
      {
         char [] stripped = StripCarriageReturns( chars, charIndex, charCount );

         return mEncoder.GetBytes( stripped, 0, stripped.Length, bytes, byteIndex );
      }

      public override int GetCharCount( byte[] bytes, int index, int count )
      {
         char[] chars = mEncoder.GetChars( bytes, index, count );
         int cr_count = 0;
         foreach ( char ch in chars )
         {
            if ( ch == '\r' )
            {
               cr_count++;
            }
         }

         return chars.Length - cr_count;
      }

      public override int GetChars( byte[] bytes, int byteIndex, int byteCount, char[] chars, int charIndex )
      {
         char[] newchars = mEncoder.GetChars( bytes, byteIndex, byteCount );
         newchars = StripCarriageReturns( newchars, 0, newchars.Length );

         for ( int i = 0; i < chars.Length; ++i )
         {
            chars[charIndex + i] = newchars[i];
         }

         return chars.Length;
      }

      public override int GetMaxByteCount( int charCount )
      {
         return mEncoder.GetMaxByteCount( charCount );
      }

      public override int GetMaxCharCount( int byteCount )
      {
         return mEncoder.GetMaxCharCount( byteCount );
      }

      System.Text.Encoding mEncoder;
   }
}
