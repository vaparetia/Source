using System;
using System.Collections.Generic;
using System.IO;
using System.Text.RegularExpressions;

public class StringIdGatherer
{
   public static string CreateStringIdsFile()
   {
      string stringIdDataFile = GetDataFileName();
      PerforceCmd.CheckoutFile(stringIdDataFile, false, true);
      Dictionary<uint, string> dic = StringIdGatherer.GatherStringIds();
      TextWriter tw = new StreamWriter(stringIdDataFile);
      foreach (KeyValuePair<uint, string> stringId in dic)
      {
         tw.WriteLine("{0} 0x{1:x8}", stringId.Value, stringId.Key);
      }
      tw.Close();
      PerforceCmd.AddFile(stringIdDataFile, true);

      return stringIdDataFile;
   }

   public static Dictionary<uint, string> LoadStringIds()
   {
      Dictionary<uint, string> dic = new Dictionary<uint, string>();
      string stringIdDataFile = GetDataFileName();
      if (File.Exists(stringIdDataFile))
      {
         TextReader tr = new StreamReader(stringIdDataFile);
         while (tr.Peek() != -1)
         {
            string[] stringID = tr.ReadLine().Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);
            uint strcode = 0;
            UInt32.TryParse(stringID[1].Substring(2), System.Globalization.NumberStyles.HexNumber, null, out strcode);
            if (!dic.ContainsKey(strcode))
            {
               dic.Add(strcode, stringID[0]);
            }
         }
      }
      return dic;
   }

   private static string GetDataFileName()
   {
      string stringIdDataFile = Environment.GetEnvironmentVariable("MGS_ROOT") + "\\GameData\\EditorSupport\\StringIDs.txt";
      return stringIdDataFile;
   }
   
   private static Dictionary<uint, string> GatherStringIds()
   {
      string codeRootDir = Environment.GetEnvironmentVariable("MGS_ROOT") + "\\mgs2x\\source";
      List<string> sourceFiles = GetFilesRecursive(codeRootDir, "*.c*");
      sourceFiles.AddRange(GetFilesRecursive(codeRootDir, "*.h"));

      Dictionary<uint, string> dic = new Dictionary<uint, string>();
      Regex[] regex = new Regex[4];
      regex[0] = 
      // matches #define Blah_01 ([0x]666) /* Hello_01 blah blah blah */
      regex[0] = new Regex(@"^#define\s+([A-Za-z0-9_]*)\s+\(*\s*(0[xX][0-9a-fA-F]+)\s*\)*\s+/\*\s+""*([A-Za-z0-9_]*)""*\s+");
      regex[1] = new Regex(@"^#define\s+([A-Za-z0-9_]*)\s+\(*\s*([0-9]+)\s*\)*\s+/\*\s+""*([A-Za-z0-9_]*)""*\s+");
      // matches #define Hello_01 ([0x]666)
      regex[2] = new Regex(@"^#define\s+([A-Za-z0-9_]*)\s+\(*\s*(0[xX][0-9a-fA-F]+)");
      regex[3] = new Regex(@"^#define\s+([A-Za-z0-9_]*)\s+\(*\s*([0-9]+)");
      // matches GV_StrCode( "Hello_01" )
      Regex superAltRegex = new Regex(@"GV_StrCode\s*\(\s*""([A-Za-z0-9_]*)""\s*\)");

      foreach (string sourceFile in sourceFiles)
      {
         StreamReader streamReader = new StreamReader(sourceFile);
         while (!streamReader.EndOfStream)
         {
            MatchCollection matchCollection = null;
            string text = streamReader.ReadLine();
            foreach (Regex r in regex)
            {
               matchCollection = r.Matches(text);
               if (matchCollection.Count > 0)
               {
                  break;
               }
            }
            if (matchCollection.Count == 0)
            {
               MatchCollection matchCollectionSuper = superAltRegex.Matches(text);
               if (matchCollectionSuper.Count > 0)
               {
                  foreach (Match match in matchCollectionSuper)
                  {
                     string stringId = match.Groups[1].Value;
                     uint strcode = StrCode(stringId);
                     if (!dic.ContainsKey(strcode))
                     {
                        dic.Add(strcode, stringId);
                     }
                  }
               }
            }
            foreach (Match match in matchCollection)
            {
               string strcodeStr = match.Groups[2].Value;
               uint strcode = 0;
               if (strcodeStr.StartsWith("0x", StringComparison.CurrentCultureIgnoreCase))
               {
                  UInt32.TryParse(strcodeStr.Substring(2), System.Globalization.NumberStyles.HexNumber, null, out strcode);
               }
               else
               {
                  UInt32.TryParse(strcodeStr, out strcode);
               }
               string stringID = match.Groups[1].Value;
               if (match.Groups.Count == 4)
               {
                  stringID = match.Groups[3].Value;
                  if (StrCode(stringID) != strcode)
                  {
                     stringID = match.Groups[1].Value;
                  }
               }
               if (!String.IsNullOrEmpty(stringID))
               {
                  if (!dic.ContainsKey(strcode))
                  {
                     dic.Add(strcode, stringID);
                  }
               }
            }
         }
         streamReader.Close();
      }
      return dic;
   }

   private static List<string> GetFilesRecursive(string initialDirectory, string filter)
   {
      // 1.
      // Store results in the file results list.
      List<string> result = new List<string>();

      // 2.
      // Store a stack of our directories.
      Stack<string> stack = new Stack<string>();

      // 3.
      // Add initial directory.
      stack.Push(initialDirectory);

      // 4.
      // Continue while there are directories to process
      while (stack.Count > 0)
      {
         // A.
         // Get top directory
         string directory = stack.Pop();

         try
         {
            // B
            // Add all files at this directory to the result List.
            result.AddRange(Directory.GetFiles(directory, filter));

            // C
            // Add all directories at this directory.
            foreach (string childDirectory in Directory.GetDirectories(directory))
            {
               stack.Push(childDirectory);
            }
         }
         catch
         {
            // D
            // Could not open the directory
         }
      }
      return result;
   }

   private static uint StrCode(String name)
   {
      uint id = 0;
      foreach (char c in name)
      {
         if (c != '\0')
         {
            id = (id << 5) | (id >> 19);
            id = (id + c) & 0xffffff;
         }
      }
      return id;
   }
}
