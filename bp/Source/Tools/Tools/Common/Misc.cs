using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Windows.Forms;
using System.Drawing;

namespace Tools.Common
{
   public class Misc
   {
      /// <summary>
      /// Converts a string that contains four characters into a fourCC type identifier
      /// </summary>
      /// <param name="typeFourCC"></param>
      /// <returns></returns>
      static public uint GetFourCCFromString( string typeFourCC )
      {
         if( typeFourCC.Length == 4 )
         {
            uint fourCC =  (uint) (typeFourCC[0] << 24 | typeFourCC[1] << 16 | typeFourCC[2] << 8 | typeFourCC[3]);
            return fourCC;
         }
         else
         {
            throw new Exception("FourCC's are supposed to be exactly 4 characters long");
         }
      }

      /// <summary>
      /// Converts a fourCC identifier into a four character string
      /// </summary>
      /// <param name="fourCC"></param>
      /// <returns></returns>
      static public string GetStringFromFourCC(uint fourCC)
      {
         char[] temp = new char[] { (char)( ( fourCC & 0xff000000 ) >> 24 ), (char)( ( fourCC & 0x00ff0000 ) >> 16 ), (char)( ( fourCC & 0x0000ff00 ) >> 8 ), (char)( fourCC & 0x000000ff ) };
         return new string(temp);
      }
      
      /// <summary>
      /// Robust string->bool converter. Returns true for 'true' / 'True' / '1'. '0' / 'false' / 'False' returns false.  
      /// Anything else (including null) returns defaultValue.
      /// </summary>
      static public bool ParseBool(String boolValue, bool defaultValue)
      {
         if ( String.IsNullOrEmpty( boolValue ) )
         {
            return defaultValue;
         }
         else if (boolValue == "1")
         {
            return true;
         }
         else if (boolValue == "0")
         {
            return false;
         }
         else
         {
            try
            {
               return bool.Parse(boolValue);
            }
            catch (FormatException)
            {
               return defaultValue;
            }
         }
      }

      /// <summary>
      /// Calls ParseBool with a default value of false
      /// </summary>
      static public bool ParseBool(String boolValue)
      {
         return ParseBool(boolValue, false);
      }

      public enum EComparisonType
      {
         kBool,
         kInt,
         kFloat,
         kString
      }

      public static bool CompareStringsAsType(string value1, string value2, EComparisonType compareType)
      {
         switch (compareType)
         {
            case EComparisonType.kBool:
               return Tools.Common.Misc.ParseBool(value1) == Tools.Common.Misc.ParseBool(value2);

            case EComparisonType.kFloat:
               return value1.Length > 0 && value2.Length > 0 && (Double.Parse(value1) == Double.Parse(value2));

            case EComparisonType.kInt:
            case EComparisonType.kString:
               return value1 == value2;

            default:
               return false;
         }
      }

      public static bool CompareStringArrays(List<string> lhs, List<string> rhs, bool ignoreCase)
      {
         if (lhs == null && rhs == null)
            return true;

         if( lhs != null && rhs != null && lhs.Count == rhs.Count )
         {
            for( int i = 0; i < lhs.Count; ++i )
            {
               if (string.Compare(lhs[i], rhs[i]) != 0)
                  return false;
            }

            return true;
         }

         return false;
      }

      public static Comparison<string> MakeCaseInsensitiveComparer()
      {
         return new Comparison<string>( delegate( string lhs, string rhs ) { return String.Compare( lhs, rhs, true ); } );
      }

      static public void DestroyControls( Control.ControlCollection controls )
      {
         // First arrange the controls to dispose
         List<Control> toDispose = new List<Control>();
         foreach ( Control control in controls )
         {
            toDispose.Add( control );
         }

         if ( toDispose.Count > 0 )
         {
            // Now destroy them going backwards
            toDispose.Reverse();
            foreach ( Control control in toDispose )
            {
               control.Dispose();
            }
         }

         if ( controls.Count > 0 )
         {
            System.Diagnostics.Debug.Fail( "Control Collection should be empty" );
         }
      }
      /// <summary>
      /// Clamps passed value to lay between min and max
      /// </summary>
      /// <param name="value"></param>
      /// <param name="min"></param>
      /// <param name="max"></param>
      /// <returns></returns>
      public static int Clamp(int value, int min, int max)
      {
         return System.Math.Max(min, System.Math.Min(max, value));
      }

      /// <summary>
      /// Clamps passed value to lay between min and max
      /// </summary>
      /// <param name="value"></param>
      /// <param name="min"></param>
      /// <param name="max"></param>
      /// <returns></returns>
      public static float Clamp(float value, float min, float max)
      {
         return System.Math.Max(min, System.Math.Min(max, value));
      }

      public static float Lerp(float src, float dst, float t)
      {
         return src + (dst - src) * t;
      }

      public static Color Lerp(Color src, Color dst, float t)
      {
         return Color.FromArgb((int)Lerp(src.A, dst.A, t), (int)Lerp(src.R, dst.R, t), (int)Lerp(src.G, dst.G, t), (int)Lerp(src.B, dst.B, t));
      }

      public static String FindCommonStringBeginningAndReplaceWithEllipsis( String root, String str )
      {
         int numCharactersInCommon = 0;
         int shortestLength = Math.Min( root.Length, str.Length );

         for ( numCharactersInCommon = 0; numCharactersInCommon < shortestLength; ++numCharactersInCommon )
         {
            if ( root[numCharactersInCommon] != str[numCharactersInCommon] )
            {
               break;
            }
         }

         // If it's worth putting ellipsis, and the result would not just BE ellipsis, do it
         if ( numCharactersInCommon > 3 && numCharactersInCommon < str.Length )
         {
            return "..." + str.Substring( numCharactersInCommon );
         }
         else
         {
            return str;
         }
      }

      public static string ConcatStringArrayWithSeparator(string[] stringArray, string separator)
      {
         string output = "";

         foreach(string current in stringArray)
         {
            if( !String.IsNullOrEmpty(output) )
            {
               output += separator;
            }
            
            output += current;
         }

         return output;
      }

      public class EmptyEnumerator<T> : IEnumerator<T>
      {
         public T Current
         {
            get { throw new ArgumentOutOfRangeException( "EmptyEnumerator has no current elements" ); }
         }
         public void Dispose()
         {
         }
         object System.Collections.IEnumerator.Current
         {
            get { return this.Current; }
         }

         public bool MoveNext()
         {
            return false;
         }

         public void Reset()
         {
         }
      }

      public class EmptyEnumerable<T> : IEnumerable<T>
      {

         public IEnumerator<T> GetEnumerator()
         {
            return new EmptyEnumerator<T>();
         }

         System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
         {
            return this.GetEnumerator();
         }
      }
   }
}
