using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using TextToolData;
using System.IO;
using System.Xml;

namespace TextToolData
{
   public enum TextPlatform
   {
      Default,
      X360,
      PS3,
      Vita,

      KJP_Default,
      KJP_X360,
      KJP_PS3,
      KJP_Vita,

   };

   public abstract class Misc
   {
      public static TextPlatform ParseTextPlatform(string value)
      {
         switch(value.ToLower())
         {
            case "x360":
               return TextPlatform.X360;
            
            case "kjp_x360":
               return TextPlatform.KJP_X360;

            case "ps3":
               return TextPlatform.PS3;

            case "kjp_ps3":
               return TextPlatform.KJP_PS3;

            case "vita":
               return TextPlatform.Vita;

            case "kjp_vita":
               return TextPlatform.KJP_Vita;

            case "kjp_default":
               return TextPlatform.KJP_Default;

            case "default":
            default:
               return TextPlatform.Default;
         }
      }
      
      public static string RemoveExtraNewlines(string inputString)
      {
         string output = inputString;

         while (output.StartsWith("\n"))
            output = output.Substring(1);

         while (output.EndsWith("\n"))
            output = output.Substring(0, output.Length - 1);

         output = output.TrimEnd();
         return output;
      }
   };

   public struct StringFileInfo
   {
      public string FileName;
      public int StringCount;

      public StringFileInfo(string fileName, int stringCount)
      {
         FileName = fileName;
         StringCount = stringCount;
      }
   }

   public struct StringFileRef
   {
      public int FileIdx;
      public int FileStringIdx;

      public StringFileRef(int fileIdx, int stringIdx)
      {
         FileIdx = fileIdx;
         FileStringIdx = stringIdx;
      }
   }

   public class StringEntry
   {
      public string Hash { get; private set; }
      public string String { get; private set; }
      public string OutputFile { get; private set; }

      public int mLanguage;
      public List<StringFileRef> Files = new List<StringFileRef>();

      public StringEntry(string entry, string hash, int language, String outputFile)
      {
         String = entry;
         Hash = hash;
         mLanguage = language;
         OutputFile = outputFile;
      }
   }

   public class StringOverride
   {
      public string Hash;
      public string OriginalString;
      public string Info;
      public String OutputFile { get; private set; }

      public SortedDictionary<TextPlatform, string> OverrideString;

      public StringOverride( String outputFile )
      {
         OverrideString = new SortedDictionary<TextPlatform, string>();
         OutputFile = outputFile;
      }

      public static TextPlatform GetKJPEquivalent(TextPlatform platform)
      {
         switch(platform)
         {
            case TextPlatform.Default:
               return TextPlatform.KJP_Default;
            
            case TextPlatform.PS3:
               return TextPlatform.KJP_PS3;
            
            case TextPlatform.X360:
               return TextPlatform.KJP_X360;

            case TextPlatform.Vita:
               return TextPlatform.KJP_Vita;
            
            default:
               return platform;
         }
      }

      public bool GetFinalGameString(TextPlatform platform, out string output)
      {
         TextPlatform kjpFallback = GetKJPEquivalent(platform);

         // If we're asking for a BP platform type, check them first.
         if (platform != kjpFallback)
         {
            // Try exact platform first.
            if (OverrideString.TryGetValue(platform, out output))
               return true;

            if (platform == TextPlatform.Vita)
            {
               if (OverrideString.TryGetValue(TextPlatform.PS3, out output))
                  return true;
            }
            
            // Fallback to BP default.
            if (OverrideString.TryGetValue(TextPlatform.Default, out output))
               return true;
         }

         // Try KJP platform next.
         if (OverrideString.TryGetValue(kjpFallback, out output))
            return true;

         if (kjpFallback == TextPlatform.KJP_Vita)
         {
            if (OverrideString.TryGetValue(TextPlatform.KJP_PS3, out output))
               return true;
         }

         // Last fallback, KJP Default.
         if (OverrideString.TryGetValue(TextPlatform.KJP_Default, out output))
            return true;
         
         return false;
      }

      public bool GetOverride(TextPlatform platform, out string output)
      {
         switch(platform)
         {
            case TextPlatform.KJP_Default:
            case TextPlatform.KJP_PS3:
            case TextPlatform.KJP_X360:
            case TextPlatform.KJP_Vita:
               throw new InvalidOperationException("Trying to use wrong 'GetOverride' function.");
         }

         if (OverrideString.TryGetValue(platform, out output))
            return true;

         if (OverrideString.TryGetValue(TextPlatform.Default, out output))
            return true;

         return false;
      }

      public bool CanRemove()
      {
         return OverrideString.Count == 0 && string.IsNullOrEmpty(Info);
      }

      static public bool IsOverrideReadonly(TextPlatform platform)
      {
         switch(platform)
         {
            case TextPlatform.KJP_Default:
            case TextPlatform.KJP_PS3:
            case TextPlatform.KJP_X360:
            case TextPlatform.KJP_Vita:
               return true;

            default:
               return false;
         }
      }
   }

   public class StringDB
   {
      enum FileType
      {
         Caption,
         StringTable,
         Invalid
      }

      Backend mBackend;

      public SortedDictionary<int, StringFileInfo> mFiles;
      public List<StringEntry> mUnifiedEntries;
      Dictionary<string, StringOverride> mStringMapping;

      public StringDB(Backend backend)
      {
         mBackend = backend;
         mUnifiedEntries = new List<StringEntry>();
         mFiles = new SortedDictionary<int, StringFileInfo>();
         mStringMapping = new Dictionary<string, StringOverride>();
      }

      List<int> tempUnknownCodes = new List<int>();

      public void UnifyStrings(string rootPath, List<string> files)
      {
         Dictionary<string, StringEntry> unifyHelper = new Dictionary<string, StringEntry>();

         for( int fileIdx = 0; fileIdx < files.Count; ++fileIdx )
         {
            string filePath = files[fileIdx];
            string shortFilePath = filePath.Substring(rootPath.Length + 1);
            shortFilePath = shortFilePath.Replace('\\', '/');

            FileType fileType = FileType.Invalid;

            switch (Path.GetExtension(filePath.ToLower()))
            {
               case ".cap":
                  fileType = FileType.Caption;
                  break;
               case ".strings":
                  fileType = FileType.StringTable;
                  break;
            }

            Console.WriteLine("Unifying ({0}/{1}): {2}...", fileIdx + 1, files.Count, shortFilePath);

            XmlDocument doc = new XmlDocument();
            doc.Load(filePath);

            switch(fileType)
            {
               case FileType.Caption:
                  {
                     XmlNodeList stringElements = doc.SelectNodes("//Caption");

                     mFiles[fileIdx] = new StringFileInfo(shortFilePath, stringElements.Count);

                     for(int strIdx = 0; strIdx < stringElements.Count; ++strIdx)
                     {
                        XmlElement stringElement = (XmlElement)stringElements[strIdx];
                        int languageId = int.Parse(stringElement.GetAttribute("lang"));
                        AddString(stringElement, languageId, fileIdx, strIdx, unifyHelper);
                     }
                  }
                  break;
               
               case FileType.StringTable:
                  {
                     XmlNodeList stringElements = doc.SelectNodes("//String");

                     mFiles[fileIdx] = new StringFileInfo(shortFilePath, stringElements.Count);

                     for (int strIdx = 0; strIdx < stringElements.Count; ++strIdx)
                     {
                        XmlElement stringElement = (XmlElement)stringElements[strIdx];
                        AddString(stringElement, -1, fileIdx, strIdx, unifyHelper);
                     }
                  }
                  break;
            }
         }
         
         mUnifiedEntries = new List<StringEntry>(unifyHelper.Values);
      }

      private void AddString(XmlElement stringElement, int language, int fileId, int stringIdx, Dictionary<string, StringEntry> unifyHelper)
      {
         string decodedString = mBackend.DecodeString(stringElement, null, tempUnknownCodes, null);

         string hash = Convert.ToBase64String(new System.Security.Cryptography.MD5CryptoServiceProvider().ComputeHash(Encoding.UTF8.GetBytes(decodedString)));

         if (tempUnknownCodes.Count > 0)
            throw new Exception("Unsupported characters found in string");

         StringEntry entry;

         if (!unifyHelper.TryGetValue(decodedString, out entry))
         {
            entry = new StringEntry(decodedString, hash, language, "");
            unifyHelper[decodedString] = entry;
         }

         StringFileRef fileRef = new StringFileRef(fileId, stringIdx);

         if (!entry.Files.Contains(fileRef))
            entry.Files.Add(fileRef);
      }

      public void LoadDB(string path)
      {
         mFiles.Clear();
         mUnifiedEntries.Clear();

         XmlDocument doc = new XmlDocument();
         doc.Load(path);

         foreach(XmlElement fileElement in doc.SelectNodes("//File"))
         {
            int fileId = int.Parse(fileElement.GetAttribute("id"));
            int stringCount = int.Parse(fileElement.GetAttribute("stringCount"));
            string filePath = fileElement.InnerText;
            mFiles[fileId] = new StringFileInfo(filePath, stringCount);
         }

         foreach(XmlElement stringElement in doc.SelectNodes("//String"))
         {
            int language = int.Parse(stringElement.GetAttribute("lang"));
            string text = stringElement.SelectSingleNode("./Text").InnerText;
            text = text.Replace("\r", "");

            string hash = stringElement.GetAttribute("hash");

            StringEntry entry = new StringEntry(text, hash, language, "");
            
            foreach(XmlElement fileElement in stringElement.SelectNodes("./FileRef"))
            {
               StringFileRef fileRef = new StringFileRef(int.Parse(fileElement.GetAttribute("id")), int.Parse(fileElement.GetAttribute("idx")));
               entry.Files.Add(fileRef);
            }

            mUnifiedEntries.Add(entry);
         }
      }

      public IEnumerable<String> GetUniqueOutputs()
      {
         Dictionary<String,bool> uniqueOutputs = new Dictionary<String, bool>();

         foreach ( var entry in mUnifiedEntries )
         {
            if ( entry.OutputFile != "" )
            {
               uniqueOutputs[entry.OutputFile] = true;
            }
         }

         return uniqueOutputs.Keys;
      }

      private static IEnumerable<String> UnpackCustomDBString( String str )
      {
         // If a string contains $LANG$, replaces it six times with each of the six supported languages

         if ( str.Contains( "$LANG$" ) )
         {
            foreach ( String lang in new String[] { "EN", "FR", "SP", "IT", "GR", "JP" } )
            {
               yield return str.Replace( "$LANG$", lang );
            }
         }
         else
         {
            yield return str;
         }
      }

      public void AddCustomDB( string path )
      {
         XmlDocument doc = new XmlDocument();
         const int kFirstCustomFileId = 1000000;
         int currentCustomFileId = kFirstCustomFileId;
         doc.Load( path );

         // First, let's get the unique categories and register them as files
         var categoryToFileId = new Dictionary<String, int>();

         foreach ( XmlNode node in doc.SelectNodes( "StringCustomOverrides/String/Category" ) )
         {
            String categoryName = node.InnerText;
            if ( categoryName != "" && !categoryToFileId.ContainsKey( categoryName ) )
            {
               categoryToFileId.Add( categoryName, currentCustomFileId );
               mFiles[currentCustomFileId] = new StringFileInfo( categoryName, 1 );
               ++currentCustomFileId;
            }
         }


         foreach ( XmlElement stringEntry in doc.SelectNodes( "StringCustomOverrides/String" ) )
         {
            String xmlText = stringEntry.SelectSingleNode( "./Text" ).InnerText.Replace( "\r", "" );
            string outputFile = stringEntry.GetAttribute( "output" );

            if ( outputFile != "" && !categoryToFileId.ContainsKey( outputFile ) )
            {
               categoryToFileId.Add( outputFile, currentCustomFileId );
               mFiles[currentCustomFileId] = new StringFileInfo( outputFile, 1 );
               ++currentCustomFileId;
            }

            foreach ( String text in UnpackCustomDBString( xmlText ) )
            {
               string hash = Convert.ToBase64String( new System.Security.Cryptography.MD5CryptoServiceProvider().ComputeHash( Encoding.UTF8.GetBytes( text ) ) );
               var entry = new StringEntry( text, hash, 0 /* lang */, outputFile );

               foreach ( XmlElement categoryElement in stringEntry.SelectNodes( "./Category" ) )
               {
                  String categoryName = categoryElement.InnerText;
                  if ( categoryName != "" )
                  {
                     entry.Files.Add( new StringFileRef( categoryToFileId[categoryName], 0 ) );
                  }
               }

               if ( outputFile != "" )
               {
                  entry.Files.Add( new StringFileRef( categoryToFileId[outputFile], 0 ) );
               }

               mUnifiedEntries.Add( entry );
            }
         }
      }

      public void SaveDB(string path)
      {
         XmlDocument doc = new XmlDocument();
         XmlElement root = doc.CreateElement("StringDB");
         doc.AppendChild(root);

         foreach(KeyValuePair<int, StringFileInfo> file in mFiles)
         {
            XmlElement fileElement = doc.CreateElement("File");
            root.AppendChild(fileElement);
            fileElement.SetAttribute("id", file.Key.ToString());
            fileElement.SetAttribute("stringCount", file.Value.StringCount.ToString());
            fileElement.InnerText = file.Value.FileName;
         }

         foreach (StringEntry entry in mUnifiedEntries)
         {
            XmlElement stringElement = doc.CreateElement("String");
            root.AppendChild(stringElement);

            stringElement.SetAttribute("lang", entry.mLanguage.ToString());
            stringElement.SetAttribute("hash", entry.Hash);

            {
               XmlElement textElement = doc.CreateElement("Text");
               textElement.InnerText = entry.String;
               stringElement.AppendChild(textElement);
            }

            foreach (StringFileRef fileId in entry.Files)
            {
               XmlElement fileElement = doc.CreateElement("FileRef");
               fileElement.SetAttribute("id", fileId.FileIdx.ToString());
               fileElement.SetAttribute("idx", fileId.FileStringIdx.ToString());
               stringElement.AppendChild(fileElement);
            }
         }

         doc.Save(path);
      }

      public void ClearRemapping()
      {
         mStringMapping.Clear();
      }

      public void AddRemapping(string path, string outputFile )
      {
         if (File.Exists(path))
         {
            XmlDocument doc = new XmlDocument();
            doc.Load(path);
         
            foreach (XmlElement entry in doc.SelectNodes("//String"))
            {
               StringOverride stringOverride = new StringOverride( outputFile );

               XmlElement hash = (XmlElement)entry.SelectSingleNode("./Hash");
               stringOverride.Hash = hash.InnerText;

               XmlElement info = (XmlElement)entry.SelectSingleNode("./Info");
               if( info != null )
               {
                  stringOverride.Info = info.InnerText;
               }

               XmlElement original = (XmlElement)entry.SelectSingleNode("./Original");
               stringOverride.OriginalString = original.InnerText.Replace("\r", "");

               foreach (XmlElement replacement in entry.SelectNodes("./Replacement"))
               {
                  TextPlatform platform = Misc.ParseTextPlatform(replacement.GetAttribute("platform"));
                  stringOverride.OverrideString[platform] = replacement.InnerText.Replace("\r", "");
               }

               mStringMapping.Add( stringOverride.Hash, stringOverride );
            }
         }
      }

      public void SaveRemapping(string path, string outputTag )
      {
         XmlDocument doc = new XmlDocument();
         XmlElement root = doc.CreateElement("StringOverrideTable");
         doc.AppendChild(root);

         foreach (KeyValuePair<string, StringOverride> item in mStringMapping)
         {
            if ( item.Value.OutputFile != outputTag )
            {
               continue;
            }

            XmlElement stringElement = doc.CreateElement("String");
            root.AppendChild(stringElement);

            XmlElement hash = doc.CreateElement("Hash");
            hash.InnerText = item.Value.Hash;
            stringElement.AppendChild(hash);

            if( !string.IsNullOrEmpty(item.Value.Info) )
            {
               XmlElement info = doc.CreateElement("Info");
               info.InnerText = item.Value.Info;
               stringElement.AppendChild(info);
            }

            XmlElement original = doc.CreateElement("Original");
            original.InnerText = item.Value.OriginalString;
            stringElement.AppendChild(original);

            foreach(KeyValuePair<TextPlatform, string> replacementItem in item.Value.OverrideString)
            {
               XmlElement replacement = doc.CreateElement("Replacement");
               replacement.SetAttribute("platform", replacementItem.Key.ToString());
               replacement.InnerText = replacementItem.Value;
               stringElement.AppendChild(replacement);
            }
         }

         doc.Save(path);
      }

      public bool HasOverride(StringEntry entry)
      {
         return mStringMapping.ContainsKey(entry.Hash);
      }

      public bool HasOverride(StringEntry entry, TextPlatform platform)
      {
         StringOverride stringOverride;
         
         if( mStringMapping.TryGetValue(entry.Hash, out stringOverride))
         {
            return stringOverride.OverrideString.ContainsKey(platform);
         }

         return false;
      }

      public string ApplyOriginalCodeOverrides(string input)
      {
         string output = input;

         output = output.Replace("L1", "[L1]");
         output = output.Replace("L2", "[L2]");
         output = output.Replace("L3", "[L3]");
         output = output.Replace("R1", "[R1]");
         output = output.Replace("R2", "[R2]");
         output = output.Replace("R3", "[R3]");

         return output;
      }

      public string ApplyKeywordReplacement(string input)
      {
         string output = input;

         output = output.Replace("[CROSS]", "#H{e080}");
         output = output.Replace("[A]", "#H{e080}");

         output = output.Replace("[CIRCLE]", "#H{e081}");
         output = output.Replace("[B]", "#H{e081}");

         output = output.Replace("[SQUARE]", "#H{e082}");
         output = output.Replace("[X]", "#H{e082}");

         output = output.Replace("[TRIANGLE]", "#H{e083}");
         output = output.Replace("[Y]", "#H{e083}");

         output = output.Replace("[L1]", "#H{e084}");
         output = output.Replace("[LB]", "#H{e084}");
         
         output = output.Replace("[L2]", "#H{e085}");
         output = output.Replace("[LT]", "#H{e085}");
         
         output = output.Replace("[L3]", "#H{e086}");
         output = output.Replace("[LS]", "#H{e086}");
         
         output = output.Replace("[R1]", "#H{e087}");
         output = output.Replace("[RB]", "#H{e087}");
         
         output = output.Replace("[R2]", "#H{e088}");
         output = output.Replace("[RT]", "#H{e088}");
         
         output = output.Replace("[R3]", "#H{e089}");
         output = output.Replace("[RS]", "#H{e089}");

         output = output.Replace("[OK]", "#H{e08a}");
         output = output.Replace("[CANCEL]", "#H{e08b}");

         return output;
      }

      public string ConvertKeywordsToX360(string input)
      {
         string output = input;

         output = output.Replace("[CROSS]", "[A]");
         output = output.Replace("[CIRCLE]", "[B]");
         output = output.Replace("[SQUARE]", "[X]");
         output = output.Replace("[TRIANGLE]", "[Y]");
         output = output.Replace("[L1]", "[LB]");
         output = output.Replace("[L2]", "[LT]");
         output = output.Replace("[L3]", "[LS]");
         output = output.Replace("[R1]", "[RB]");
         output = output.Replace("[R2]", "[RT]");
         output = output.Replace("[R3]", "[RS]");

         return output;
      }

      public string ConvertKeywordsToPS3(string input)
      {
         string output = input;

         output = output.Replace("[A]", "[CROSS]");
         output = output.Replace("[B]", "[CIRCLE]");
         output = output.Replace("[X]", "[SQUARE]");
         output = output.Replace("[Y]", "[TRIANGLE]");
         output = output.Replace("[LB]", "[L1]");
         output = output.Replace("[LT]", "[L2]");
         output = output.Replace("[LS]", "[L3]");
         output = output.Replace("[RB]", "[R1]");
         output = output.Replace("[RT]", "[R2]");
         output = output.Replace("[RS]", "[R3]");

         return output;

      }

      public StringOverride GetOverrideEntry(StringEntry entry)
      {
         StringOverride overrideEntry;

         if (mStringMapping.TryGetValue(entry.Hash, out overrideEntry))
         {
            return overrideEntry;
         }
         
         return null;
      }

      public void GetFinalGameString(StringEntry entry, TextPlatform platform, out string output)
      {
         StringOverride overrideEntry;

         if (mStringMapping.TryGetValue(entry.Hash, out overrideEntry))
         {
            if( overrideEntry.GetFinalGameString(platform, out output) )
               return;
         }

         output = ApplyOriginalCodeOverrides(entry.String);
      }

      public void GetFinalGameStringOnlyOverride( StringEntry entry, TextPlatform platform, out string output )
      {
         StringOverride overrideEntry;

         output = null;

         if ( mStringMapping.TryGetValue( entry.Hash, out overrideEntry ) )
         {
            overrideEntry.GetFinalGameString( platform, out output );
         }
      }

      public StringOverride SetStringOverride(StringEntry entry, TextPlatform platform, string remappedString)
      {
         StringOverride overrideEntry;

         if (!mStringMapping.TryGetValue(entry.Hash, out overrideEntry))
         {
            overrideEntry = new StringOverride( entry.OutputFile );
            overrideEntry.Hash = entry.Hash;
            mStringMapping[entry.Hash] = overrideEntry;
         }

         overrideEntry.OriginalString = entry.String;
         overrideEntry.OverrideString[platform] = remappedString;

         return overrideEntry;
      }

      public void RemoveOverride(StringEntry entry, TextPlatform platform)
      {
         StringOverride overrideEntry;

         if (mStringMapping.TryGetValue(entry.Hash, out overrideEntry))
         {
            overrideEntry.OverrideString.Remove(platform);

            if (overrideEntry.CanRemove())
               mStringMapping.Remove(entry.Hash);
         }
      }

      public void WriteOverrideTable(TextPlatform platform, string path)
      {
         UTF8Encoding encoding = new UTF8Encoding(false);

         using (StreamWriter writer = new StreamWriter(path, false, encoding))
         {
            writer.WriteLine("//----------------------------------------------------------------------------");
            writer.WriteLine("// Automatically generated by TextTool, do not modify.");
            writer.WriteLine("//----------------------------------------------------------------------------");
            writer.WriteLine("");
            writer.WriteLine("#include \"Engine/Stdafx.h\"");
            writer.WriteLine("");
            writer.WriteLine("//----------------------------------------------------------------------------");
            writer.WriteLine("");
            writer.WriteLine("struct SBP_StringOverride");
            writer.WriteLine("{");
            writer.WriteLine("   uint32               mHash;");
            writer.WriteLine("   unsigned const char* mpString;");
            writer.WriteLine("};");
            writer.WriteLine("");

            // Write out strings
            {
               int stringIdx = 0;

               foreach (StringOverride item in mStringMapping.Values)
               {
                  string replacementString;
                  if (item.GetFinalGameString(platform, out replacementString))
                  {
                     writer.WriteLine("/*");
                     writer.WriteLine("----- Original -----");
                     writer.WriteLine(item.OriginalString);
                     writer.WriteLine("----- Replacement -----");
                     writer.WriteLine(replacementString);
                     writer.WriteLine("*/");

                     writer.WriteLine("unsigned char const kString{0}[] =", stringIdx);
                     writer.WriteLine("{");
                     writer.Write("   ");

                     replacementString = ApplyKeywordReplacement(replacementString);

                     byte[] replacementUtf8 = mBackend.ConvertStringToUTF8(replacementString);

                     for (int i = 0; i < replacementUtf8.Length; ++i)
                     {
                        if (i != 0)
                           writer.Write(", ");

                        writer.Write("0x{0:x2}", replacementUtf8[i]);
                     }

                     if (replacementUtf8.Length > 0 )
                        writer.WriteLine(", 0x00");
                     else
                        writer.WriteLine("0x00");

                     writer.WriteLine("};");
                     writer.WriteLine("");
                  }

                  ++stringIdx;
               }
            }

            writer.WriteLine("");
            writer.WriteLine("SBP_StringOverride const kStringOverrides[] =");
            writer.WriteLine("{");

            // Write out mapping table
            {
               int stringIdx = 0;

               foreach (StringOverride item in mStringMapping.Values)
               {
                  string dummy;
                  if (item.GetFinalGameString(platform, out dummy))
                  {
                     string currentInGameString = item.OriginalString;
                     currentInGameString = ApplyOriginalCodeOverrides(currentInGameString);
                     currentInGameString = ApplyKeywordReplacement(currentInGameString);

                     byte[] utf8Characters = mBackend.ConvertStringToUTF8(currentInGameString);

                     uint hash = Tools.Common.CRC32.CalculateCRC(utf8Characters);

                     writer.WriteLine(String.Format("   {{ 0x{0:x8}, kString{1} }}, ", hash, stringIdx));
                  }

                  stringIdx++;
               }
            }

            writer.WriteLine("};");
            writer.WriteLine("");
            writer.WriteLine("unsigned const char* BP_LookupStringOveride(uint32 const hash)");
            writer.WriteLine("{");
            writer.WriteLine("   for( int i = 0; i < BPE_ARRAY_SIZE(kStringOverrides); ++i )");
            writer.WriteLine("   {");
            writer.WriteLine("      if( kStringOverrides[i].mHash == hash )");
            writer.WriteLine("         return kStringOverrides[i].mpString;");
            writer.WriteLine("   }");
            writer.WriteLine("");
            writer.WriteLine("   return NULL;");
            writer.WriteLine("}");
         }
      }

      public StringEntry FindString(string originalText)
      {
         string searchString = Misc.RemoveExtraNewlines(originalText);

         foreach(StringEntry entry in mUnifiedEntries)
         {
            string entryString = Misc.RemoveExtraNewlines(entry.String);

            if (entryString == searchString)
               return entry;
         }

         return null;
      }

      public void RemoveAllKJPOverrides()
      {
         foreach(StringOverride overrideEntry in mStringMapping.Values)
         {
            overrideEntry.OverrideString.Remove(TextPlatform.KJP_Default);
            overrideEntry.OverrideString.Remove(TextPlatform.KJP_PS3);
            overrideEntry.OverrideString.Remove(TextPlatform.KJP_X360);
            overrideEntry.OverrideString.Remove(TextPlatform.KJP_Vita);
         }
      }

      public void CullEmptyOverrides()
      {
         List<string> toRemove = new List<string>();

         foreach (KeyValuePair<string, StringOverride> item in mStringMapping)
         {
            if (item.Value.CanRemove())
               toRemove.Add(item.Key);
         }

         foreach(string removeItem in toRemove)
         {
            mStringMapping.Remove(removeItem);
         }
      }
   }
}
