using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Security.Cryptography;

namespace LayoutExtract2
{
   public class LayoutUtil
   {
      public class Exception : System.Exception
      {
         public Exception( String s ) : base( s )
         {
         }
      }

      #region Private Fields
      private static MD5 sMD5 = MD5.Create();
      private static int sLogStreamIndent = 0;
      #endregion

      public static WordLists WordLists = new WordLists(new System.Random());
      public static FlatListRemapping FlatListRemapping = new FlatListRemapping();
      public static System.Drawing.Color mBackgroundColor = System.Drawing.Color.Gray;

      public static bool LogLoads = false;

      public static void LogStreamIndentIn() { sLogStreamIndent++; }
      public static void LogStreamIndentOut() { sLogStreamIndent--; }
      
      public static void LogStreamLoad( BinaryReader br, String logEntry )
      {
         if ( LogLoads )
         {
            // NOTE - IF YOU WANT TO HAVE THE STREAM POSITION IN THERE, PUT A {1} in the string below

            Console.WriteLine( "{0} {2}", LogStreamIndentText, br.BaseStream.Position, logEntry );
         }
      }
      public static void LogStreamObject( BinaryReader by, object o )
      {
         LogStreamLoad( by, o.GetType().ToString() );
      }
      public static void LogStreamAddl( params object[] objs )
      {
         if ( LogLoads )
         {
            Console.Write( "{0}... ", LogStreamIndentText );
            foreach ( object o in objs )
            {
               Console.Write( "{0} ", o.ToString() );
            }
            Console.WriteLine();
         }
      }

      public static String MD5Hash( byte[] bytesin )
      {
         sMD5.Initialize();
         byte[] bytes = sMD5.ComputeHash( bytesin );

         String ret = "";

         foreach ( byte b in bytes )
         {
            ret += b.ToString( "x2" );
         }

         return ret;
      }
      public static void Reset(System.Random wordListRandom)
      {
         WordLists.Reset(wordListRandom);
      }
      public static void RecreateWordLists(System.Random wordListRandom)
      {
         WordLists = new WordLists(wordListRandom);
      }

      public static uint MakeStrCode(string str)
      {
         uint id = 0;
         foreach (char c in str.ToCharArray())
         {
            id = (id << 5) | (id >> (24 - 5));
            id += c;
            id &= 0x00ffffff;
         }
         return (id == 0) ? 1 : id;
      }
      public static string MakePrintable(string str)
      {
         str = str.Replace("\a", "\\a");
         str = str.Replace("\b", "\\b");
         str = str.Replace("\f", "\\f");
         str = str.Replace("\n", "\\n");
         str = str.Replace("\r", "\\r");
         str = str.Replace("\t", "\\t");
         str = str.Replace("\v", "\\v");
         str = str.Replace("\'", "\\'");
         str = str.Replace("&", "&amp;");
         return str;
      }
      public static string MakeUnprintable(string str)
      {
         str = str.Replace("\\a", "\a");
         str = str.Replace("\\b", "\b");
         str = str.Replace("\\f", "\f");
         str = str.Replace("\\n", "\n");
         str = str.Replace("\\r", "\r");
         str = str.Replace("\\t", "\t");
         str = str.Replace("\\v", "\v");
         str = str.Replace("\\'", "\'");
         return str;
      }

      #region Private Methods
      private static String LogStreamIndentText
      {
         get
         {
            String s = "";
            for (int i = 0; i < sLogStreamIndent; ++i)
            {
               s += "- ";
            }

            return s;
         }
      }
      #endregion
   }

   // UNUSED STUFF FROM HERE

   public class LayoutUnit
   {
      public LayoutUnit(BinaryReader br)
      {
         usUnitNum = br.ReadUInt16();
         usVertexCount = br.ReadUInt16();
      }
      public uint usUnitNum;
      public uint usVertexCount;
   }
}
