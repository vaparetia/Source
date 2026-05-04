// ----------------------------------------------------------------------
// FlatListRemapping
// Currently, just remaps texture file names to the flat list
// ----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace LayoutExtract2
{
   public class FlatListRemapping
   {
      static string GetString(System.IO.BinaryReader br)
      {
         string retStr = "";
         char x = '\0';
         do
         {
            x = br.ReadChar();
            if (x == '\0') break;
            if (x == '/') x = '\\';
            retStr += x;
         }
         while (true);
         return retStr;
      }

      static String FixSlashes( String s )
      {
         return s.Replace( '/', '\\' );
      }

      public FlatListRemapping()
      {
         string repository = System.Environment.GetEnvironmentVariable("BPE_REPOSITORY");
         if (!String.IsNullOrEmpty(repository))
         {
            mRemap = new Dictionary<string, string>();

            String[] remappingEntries = System.IO.File.ReadAllLines(
               repository + "\\bp_flatlisttexturemapping.txt",
               Encoding.ASCII );

            int line = 0;
            if ( remappingEntries[0] != "flatmapv2" )
            {
               throw new Exception( "Flat list is wrong format" );
            }

            foreach ( String entry in remappingEntries )
            {
               ++line;
               if ( line == 1 )
               {
                  // First line is a dummy entry for the version
                  continue;
               }
               String[] entryPair = FixSlashes( entry ).Split( ' ' );

               if ( entryPair.Length > 0 )
               {
                  if ( entryPair.Length != 2 )
                  {
                     throw new Exception( String.Format( "Malformed remapping entry on line {0} - '{1}'", line, entry ) );
                  }

                  string key = entryPair[1] + ".tga";
                  if (!mRemap.ContainsKey(key))
                  {
                     mRemap.Add(key, entryPair[0].ToLower());
                  }
               }
            }
         }
      }

      public string GetRemappedName(uint tex_strcode)
      {
         string name = String.Format("{0:x8}.tga", tex_strcode);
         if (mRemap.ContainsKey(name))
         {
            name = mRemap[name];
         }
         return name;
      }

      private Dictionary<string, string> mRemap;
   }
}
