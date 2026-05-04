using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Xml;
using System.Security.Cryptography;
using System.Diagnostics;

//////////////////////////////////////////////////////////////////////////
// AssetToLib
// 
// Takes in a list of textures in an XML format, then reads them in as if 
// they are binary data and constructs a .c file with the data embedded in
// it and a header file that can be included. Then the c data can be included
// directly into the target executable, and all assets will be loaded directly.
//
// Xml Format
// <AssetToLib header="AssetHeader.h">
//    <Asset name="asset_1">_win\SomeFile1.txt</Asset>
//    <Asset name="asset_2">_win\SomeFile2.txt</Asset>
//    <Asset name="asset_3">_win\SomeFile3.txt</Asset>
// </AssetToLib>
//////////////////////////////////////////////////////////////////////////

namespace AssetToLib
{
   class Program
   {
      static Boolean CPP_SOURCE = true;

      static void Log(String inMsg)
      {
         System.Console.WriteLine(inMsg);
         System.Diagnostics.Debug.WriteLine(inMsg);
      }

      static int Main(string[] args)
      {
         if (args.Length == 0)
         {
            Log("AssetToLib [/c] <FILE1> <FILE2> <FILE3> ...");
            Log("");
            Log("Either generates a layout file based on the files passed, or generates the source files based on the layout files passed. If no switch is passed, then the parameter list is compiled into a layout file.");
            Log("The output file is defaulted to AssetLayout.xml and is located in the working directory. All file locations in the layout file are relative to this location.");
            Log("/c   Generate source files based on passed layout files.");
            Log("");
            Log("E.g. AssetToLib /c Layout.xml");
            Log("E.g. AssetToLib asset1.txt asset2.txt _win/asset3.txtr");

            return 1;
         }
         else if (args[0].ToLower() == "/c" || args[0].ToLower() == "-c")
         {
            bool bFirst = true;
            foreach (String arg in args)
            {
               // Skip the initial switch
               if (bFirst)
               {
                  bFirst = false;
                  continue;
               }

               try
               {
                  ExportFilesFromXml(arg);
               }
               catch (System.Exception ex)
               {
                  Log("Failed to export layout file " + arg + "\n" + ex.Message);
                  Log( ex.StackTrace.ToString() );

                  return 1;
               }
            }
         }
         else
         {
            StringBuilder xmlTempFile = new StringBuilder();
            xmlTempFile.AppendFormat("<assets>\n   <header file=\"Source\\Assets.h\">\n");
            foreach (String arg in args)
            {
               if (Directory.Exists(arg))
               {
                  String[] filesInDirectory = Directory.GetFiles(arg, "*", SearchOption.AllDirectories);
                  foreach (String file in filesInDirectory)
                  {
                     String cName = file.Replace("\\", "_").Replace("/", "_").Replace(".", "_").Replace("-", "_");
                     String platform = (file.Contains("\\_win\\")) ? " platform=\"WIN32\"" : "";
                     platform = (file.Contains("\\_ps3\\")) ? " platform=\"PS3\"" : platform;
                     platform = (file.Contains("\\_360\\")) ? " platform=\"X360\"" : platform;
                     xmlTempFile.AppendFormat("      <asset name=\"{0}\" compress=\"true\"{2}>{1}</asset>\n", cName, file, platform);
                  }
               }
               else if (File.Exists(arg))
               {
                  String cName = arg.Replace("\\", "_").Replace("/", "_").Replace(".", "_").Replace("-", "_");
                  String platform = (arg.Contains("\\_win\\")) ? " platform=\"WIN32\"" : "";
                  platform = (arg.Contains("\\_ps3\\")) ? " platform=\"PS3\"" : platform;
                  platform = (arg.Contains("\\_360\\")) ? " platform=\"X360\"" : platform;
                  xmlTempFile.AppendFormat("      <asset name=\"{0}\" compress=\"true\"{2}>{1}</asset>\n", cName, arg, platform);
               }
            }
            xmlTempFile.Append("   </header>\n</assets>");

            File.WriteAllText("AssetLayout.xml", xmlTempFile.ToString());
         }

         return 0;
      }

      static void ExportFilesFromXml(String inXmlFile)
      {
         String sourceDirectory = Path.GetDirectoryName(inXmlFile);
         if (!String.IsNullOrEmpty(sourceDirectory))
            sourceDirectory += "\\";
         
         XmlDocument xmlDoc = new XmlDocument();
         try
         {
            if (File.Exists(inXmlFile))
               xmlDoc.Load(inXmlFile);
            else
            {
               Log("Failed to find file " + inXmlFile);
               return;
            }
         }
         catch (System.Exception ex)
         {
            Log("Failed to load Xml File!\n" + ex.Message);
            return;
         }

         XmlElement headNode = xmlDoc.DocumentElement;
         foreach (XmlElement header in headNode)
         {
            Dictionary<String, String> filesToProcess = new Dictionary<String, String>();
            Dictionary<String, String> filesToPlatform = new Dictionary<String, String>();
            List<String> filesToCompress = new List<String>();
            String headerFile = "";

            /// We'll use a list of asset files to cook instead of long string to avoid the 
            /// 2080 byte string length limitation for XP
            List<String> filesToCook = new List<String>();

            headerFile = header.GetAttribute("file");
            String headerFileNoPath = Path.GetFileName(headerFile);

            foreach (XmlElement childNode in header)
            {
               String cName = childNode.GetAttribute("name");
               
               String file = childNode.InnerText.Replace("\\", "/");
               
               filesToProcess.Add(cName, file);

               if (childNode.HasAttribute("compress"))
               {
                  String compress = childNode.GetAttribute("compress");
                  if (compress.ToLower() == "true")
                     filesToCompress.Add(cName);
               }

               String platform = null;
               if (childNode.HasAttribute("platform"))
               {
                  platform = childNode.GetAttribute("platform").ToUpper();
               }
               filesToPlatform.Add(cName, platform);

               if (childNode.HasAttribute("source"))
               {
                  String source = childNode.GetAttribute("source").ToLower();
                  source = source.Replace("\\", "/").ToLower();
                  filesToCook.Add(source);
               }
            }

            {
			   // Generate multiple command lines for the input files
               List<String> commandLines = new List<String>();

               commandLines.Add( "" );

               foreach ( String srcAsset in filesToCook )
               {
                  if ( ( commandLines[commandLines.Count - 1 ] != "" ) &&
                       ( commandLines[commandLines.Count - 1].Length + srcAsset.Length >= 512 ) )
                  {
                     // If our resultant command line would be too long, let's add a new one
                     commandLines.Add( "" );
                  }

                  commandLines[commandLines.Count - 1] += srcAsset + " ";
               }

               foreach ( String cookAssetFileList in commandLines )
               {
                  if ( cookAssetFileList == "" )
                  {
                     continue;
                  }

                  // Create a new process to cook assets.
                  // Don't create a new window as this is really distracting 
                  // Disable multi threaded cooks as MT cooks cause AssetTool to never exit when trying to re-direct output
                  // TODO: Fix this!

                  ProcessStartInfo startInfo = new ProcessStartInfo("AssetTool.exe", cookAssetFileList);                     
                  startInfo.CreateNoWindow = true;
                  startInfo.UseShellExecute = false;
                  startInfo.RedirectStandardOutput = true;
                  Process proc = Process.Start(startInfo);
                  // Write output to console (read first before wait to prevent a deadlock)
                  // Not reading error stream to avoid deadlock (tricker to read 2 streams, see docs)
                  Console.Write(proc.StandardOutput.ReadToEnd());
                  proc.WaitForExit();                  
                  // Clear assets to cook list
               }            
            }

            String directory = Path.GetDirectoryName(headerFile);
            if (!String.IsNullOrEmpty(directory) && !Directory.Exists(directory))
            {
               Directory.CreateDirectory(directory);
            }
            directory += "\\";

            List<String> toRemove = new List<String>();
            foreach (KeyValuePair<String, String> fileToProcess in filesToProcess)
               try
               {
                  if (!WriteOutFileToCFile(directory, headerFileNoPath, fileToProcess.Key, sourceDirectory + fileToProcess.Value, filesToCompress.Contains(fileToProcess.Key), filesToPlatform[fileToProcess.Key]))
                     toRemove.Add(fileToProcess.Key);
               }
               catch (System.Exception ex)
               {
                  Log("Failed to write out file: " + fileToProcess.Key + ".\n" + ex.Message);
               }

            foreach (String toRem in toRemove)
               filesToProcess.Remove(toRem);

            WriteOutHeaderFile(headerFile, filesToProcess, inXmlFile);
            WriteOutSourceFile(Path.ChangeExtension(headerFile, (CPP_SOURCE) ? ".cpp" : ".c"), headerFileNoPath, filesToProcess, inXmlFile);
         }
      }

      static void CopyStream(Stream input, Stream output)
      {
         byte[] buffer = new byte[32768];
         while (true)
         {
            int read = input.Read(buffer, 0, buffer.Length);
            if (read <= 0) return;
            output.Write(buffer, 0, read);
         }
      }

      static Boolean WriteOutFileToCFile(String inDirectory, String inHeaderFile, String inTargetCName, String inSourceDataFile, Boolean inCompress, String inPlatform)
      {
         String fileName = inDirectory + inTargetCName + ((CPP_SOURCE) ? ".cpp" : ".c");
         Log("Writing out " + fileName + "...");

         FileStream inputFileStream = File.OpenRead(inSourceDataFile);
         if (inputFileStream == null)
         {
            Log("FAILED TO OPEN FILE!!!");
            return false;
         }

         Int32 totalSize = (Int32)inputFileStream.Length;
         Byte[] inputBuffer = new Byte[totalSize];
         if (totalSize != inputFileStream.Read(inputBuffer, 0, totalSize))
         {
            Log("FAILED TO READ FILE!!!");
            return false;
         }
         inputFileStream.Close();

         MemoryStream compressedStream = null;
         if (inCompress && totalSize > 0)
         {
            Int64 outputSize = totalSize * 4;
            Byte[] outputBuffer = new Byte[outputSize];
            CZLib.Compress(outputBuffer, ref outputSize, inputBuffer, inputBuffer.Length);

            compressedStream = new MemoryStream(outputBuffer, 0, (Int32)outputSize, false);
         }
         else
         {
            compressedStream = new MemoryStream(inputBuffer, 0, totalSize, false);
         }
         Int32 compressedSize = (Int32)compressedStream.Length;

         // Force not compressed if the compression makes it larger.... :(
         if (compressedSize > totalSize)
         {
            compressedStream = new MemoryStream(inputBuffer, 0, totalSize, false);
            compressedSize = totalSize;
            inCompress = false;
         }

         compressedStream.Position = 0;

         StringBuilder fileData = new StringBuilder();
         fileData.Append("//----------------------------------------------------------------------------\n");
         fileData.Append("// AUTOMATICALLY GENERATED CODE!!! DO NOT MODIFY!!!\n");
         fileData.Append("// Generated from: " + inSourceDataFile + "\n");
         fileData.Append("//----------------------------------------------------------------------------\n\n");
         fileData.Append("#ifdef AST_USE_STDAFX\n   #include \"StdAfx.h\"\n#endif // AST_USE_STDAFX\n");
         fileData.Append("#include \"" + inHeaderFile + "\"\n\nunsigned char " + inTargetCName + "[] = {");

         if (!String.IsNullOrEmpty(inPlatform))
            fileData.Append("\n#if BPE_TARGET == BPE_TARGET_" + inPlatform + "");

         Byte[] buffer = new Byte[32768];
         Boolean bContinueReading = true;
         while (bContinueReading)
         {
            Int32 start = 0;
            Int32 readin = compressedStream.Read(buffer, start, 32768);

            for (Int32 ii = 0; ii < readin; ++ii)
            {
               if (ii % 16 == 0)
                  fileData.Append("\n");
               fileData.AppendFormat("0x{0:x2},", buffer[ii]);
            }

            if (readin < 32768)
               break;
         }

         Log(String.Format("Size Change {0}% ({1} to {2}) bytes", (Int32)(100 * ((float)compressedSize / (float)totalSize)), totalSize, compressedSize));

         if (!String.IsNullOrEmpty(inPlatform))
            fileData.Append("\n#endif");

         fileData.Append("\n0x00 };\n\n");

         fileData.Append("//----------------------------------------------------------------------------\n\n");
         fileData.Append("unsigned char const * ast_get_" + inTargetCName + "__data() { return " + inTargetCName + "; }\n");
         fileData.Append("unsigned long         ast_get_" + inTargetCName + "__length() { return " + totalSize + "; }\n");
         fileData.Append("unsigned long         ast_get_" + inTargetCName + "__compressed_length() { return " + compressedSize + "; }\n");
         fileData.Append("bool                  ast_get_" + inTargetCName + "__is_compressed() { return " + ((inCompress) ? "true" : "false") + "; }\n");

         fileData.Append("\n");
         fileData.Append("//----------------------------------------------------------------------------\n\n");


         // Check to see if the data has changed...Don't update if it hasn't
         MD5 md5 = MD5.Create();
         MemoryStream memStream = new MemoryStream(ASCIIEncoding.Default.GetBytes(fileData.ToString()), false);
         byte[] hash = md5.ComputeHash(memStream);

         String hashString = "// Hash: ";
         for (Int32 ii = 0; ii < hash.Length; ++ii)
            hashString += hash[ii].ToString();
         hashString += "\n";
         fileData.Insert(0, hashString);

         if (File.Exists(fileName))
         {
            FileStream fileStream = File.OpenRead(fileName);
            if (fileStream.Length == fileData.Length)
            {
               Byte[] fileHashArray = new Byte[hashString.Length];
               fileStream.Read(fileHashArray, 0, hashString.Length);
               String fileHashArrayString = ASCIIEncoding.Default.GetString(fileHashArray);
               if (hashString == fileHashArrayString)
               {
                  Log("File is the same. Not changing anything.");
                  return true;
               }
            }
            fileStream.Close();
         }

         File.WriteAllText(fileName, fileData.ToString());

         return true;
      }

      static void WriteOutHeaderFile(String inHeaderFile, Dictionary<String, String> inFilesProcessed, String inSourceFile)
      {
         StringBuilder fileData = new StringBuilder();
         fileData.Append("#pragma once\n\n");
         fileData.Append("//----------------------------------------------------------------------------\n");
         fileData.Append("// AUTOMATICALLY GENERATED CODE!!! DO NOT MODIFY!!!\n");
         fileData.Append("// Generated from: " + inSourceFile + "\n");
         fileData.Append("//----------------------------------------------------------------------------\n\n");

         fileData.Append("void                  ast_cleanup();\n");
         fileData.Append("void                  ast_set_file_decompression(void (*decompress)(void* inSrcData, int inSrcSize, void* inDestData, int inDestSize));\n");
         fileData.Append("unsigned long         ast_open_file_index(char const * inFileName);\n");
         fileData.Append("int                   ast_get_file_index(char const * inFileName);\n");
         fileData.Append("void                  ast_close_file_index(unsigned long inFileIndex);\n");
         fileData.Append("unsigned char const * ast_get_file_data(unsigned long inFileIndex);\n");
         fileData.Append("unsigned long         ast_get_file_length(unsigned long inFileIndex);\n\n");

         fileData.Append("//----------------------------------------------------------------------------\n\n");

         foreach (String cName in inFilesProcessed.Keys)
         {
            fileData.Append("// " + cName + "\n");
            fileData.Append("unsigned char const * ast_get_" + cName + "__data();\n");
            fileData.Append("unsigned long         ast_get_" + cName + "__length();\n");
            fileData.Append("unsigned long         ast_get_" + cName + "__compressed_length();\n");
            fileData.Append("bool                  ast_get_" + cName + "__is_compressed();\n\n");
            fileData.Append("//----------------------------------------------------------------------------\n\n");
         }


         // Check to see if the data has changed...Don't update if it hasn't
         MD5 md5 = MD5.Create();
         MemoryStream memStream = new MemoryStream(ASCIIEncoding.Default.GetBytes(fileData.ToString()), false);
         byte[] hash = md5.ComputeHash(memStream);

         String hashString = "// Hash: ";
         for (Int32 ii = 0; ii < hash.Length; ++ii)
            hashString += hash[ii].ToString();
         hashString += "\n";
         fileData.Insert(0, hashString);

         if (File.Exists(inHeaderFile))
         {
            FileStream fileStream = File.OpenRead(inHeaderFile);
            if (fileStream.Length == fileData.Length)
            {
               Byte[] fileHashArray = new Byte[hashString.Length];
               fileStream.Read(fileHashArray, 0, hashString.Length);
               String fileHashArrayString = ASCIIEncoding.Default.GetString(fileHashArray);
               if (hashString == fileHashArrayString)
               {
                  Log("File is the same. Not changing anything.");
                  return;
               }
            }
            fileStream.Close();
         }

         File.WriteAllText(inHeaderFile, fileData.ToString());
      }

      static void WriteOutSourceFile(String inTargetFile, String inHeaderFile, Dictionary<String, String> inFilesProcessed, String inSourceFile)
      {
         Tools.AssetSystem.Manager assetManager = new Tools.AssetSystem.Manager();

         StringBuilder fileData = new StringBuilder();
         fileData.Append("//----------------------------------------------------------------------------\n");
         fileData.Append("// AUTOMATICALLY GENERATED CODE!!! DO NOT MODIFY!!!\n");
         fileData.Append("// Generated from: " + inSourceFile + "\n");
         fileData.Append("//----------------------------------------------------------------------------\n\n");
         fileData.Append("#ifdef AST_USE_STDAFX\n   #include \"StdAfx.h\"\n#endif // AST_USE_STDAFX\n");
         fileData.Append("#include \"" + inHeaderFile + "\"\n");
         fileData.Append("#include <string.h>\n");
         fileData.Append("#include <ctype.h>\n\n");

         fileData.Append("#define NUM_FILES " + inFilesProcessed.Count + "\n\n");
         fileData.Append("//----------------------------------------------------------------------------\n\n");

         // Uncompressed file data storage location
         fileData.Append("static char const * g_asset_files[] = {\n");
         foreach (String fName in inFilesProcessed.Values)
         {
//            string repositoryPath = assetManager.GetRepositoryRelativePath(fName);
//            fileData.Append("   \"" + repositoryPath + "\",\n");
            fileData.Append( String.Format( "   \"{0}\",\n", fName.ToLower() ) );
         }
         fileData.Append("   NULL\n};\n\n");
         fileData.Append("//----------------------------------------------------------------------------\n\n");

         // Function pointers to get compressed file data
         fileData.Append("static unsigned char const * (*g_asset_file_data_tbl[])() = {\n");
         foreach (String cName in inFilesProcessed.Keys)
         {
            fileData.Append("   " + "ast_get_" + cName + "__data" + ",\n");
         }
         fileData.Append("   NULL\n};\n\n");
         fileData.Append("//----------------------------------------------------------------------------\n\n");

         // Function pointers to get uncompressed file length
         fileData.Append("static unsigned long (*g_asset_file_length_tbl[])() = {\n");
         foreach (String cName in inFilesProcessed.Keys)
         {
            fileData.Append("   " + "ast_get_" + cName + "__length" + ",\n");
         }
         fileData.Append("   NULL\n};\n\n");
         fileData.Append("//----------------------------------------------------------------------------\n\n");

         // Function pointers to get compressed file length
         fileData.Append("static unsigned long (*g_asset_file_compressed_length_tbl[])() = {\n");
         foreach (String cName in inFilesProcessed.Keys)
         {
            fileData.Append("   " + "ast_get_" + cName + "__compressed_length" + ",\n");
         }
         fileData.Append("   NULL\n};\n\n");
         fileData.Append("//----------------------------------------------------------------------------\n\n");

         // Function pointers to get whether the file is compressed or not
         fileData.Append("static bool (*g_asset_file_is_compressed_tbl[])() = {\n");
         foreach (String cName in inFilesProcessed.Keys)
         {
            fileData.Append("   " + "ast_get_" + cName + "__is_compressed" + ",\n");
         }
         fileData.Append("   NULL\n};\n\n");
         fileData.Append("//----------------------------------------------------------------------------\n\n");

         // Temporary storage for the uncompressed file data
         fileData.Append("static unsigned char const * g_open_asset_file_data[] = {\n   ");
         foreach (String cName in inFilesProcessed.Keys)
         {
            fileData.Append("NULL, ");
         }
         fileData.Append("\n   NULL\n};\n\n");
         fileData.Append("//----------------------------------------------------------------------------\n\n");

         // Allows library user to set the decompression function
         fileData.Append("static void (*g_decompress_func)(void* inSrcData, int inSrcSize, void* inDestData, int inDestSize) = NULL;\n\n");
         fileData.Append("//----------------------------------------------------------------------------\n\n");
         fileData.Append("void ast_set_file_decompression(void (*decompress)(void* inSrcData, int inSrcSize, void* inDestData, int inDestSize))\n");
         fileData.Append("{\n");
         fileData.Append("   g_decompress_func = decompress;\n");
         fileData.Append("}\n\n");
         fileData.Append("//----------------------------------------------------------------------------\n\n");

         // Close any remaining open file handles and release memory
         fileData.Append("void ast_cleanup()\n");
         fileData.Append("{\n");
         fileData.Append("   int index;\n");
         fileData.Append("   \n");
         fileData.Append("   for (index=0; index<NUM_FILES; ++index)\n");
         fileData.Append("      if (g_open_asset_file_data[index] != NULL)\n");
         fileData.Append("         ast_close_file_index(index);\n");
         fileData.Append("}\n\n");
         fileData.Append("//----------------------------------------------------------------------------\n\n");

         // Get the file index for the file name
         fileData.Append("int ast_get_file_index(char const * inFileName)\n");
         fileData.Append("{\n");
         fileData.Append("   char lcFileName[1024];\n");
         fileData.Append("   int inFileNameLen;\n");
         fileData.Append("   int index;\n");
         fileData.Append("   \n");
         fileData.Append("   inFileNameLen = strlen(inFileName);\n");
         fileData.Append("   for (index=0; index<inFileNameLen; ++index )\n");
         fileData.Append("   {\n");
         fileData.Append("      lcFileName[index] = tolower(inFileName[index]);\n");
         fileData.Append("      if (lcFileName[index] == '\\\\')\n");
         fileData.Append("         lcFileName[index] = '/';\n");
         fileData.Append("   }\n");
         fileData.Append("   lcFileName[index] = 0;\n");
         fileData.Append("   \n");
         fileData.Append("   for (index=0; index<NUM_FILES; ++index)\n");
         fileData.Append("      if (0 == strcmp(lcFileName, g_asset_files[index]))\n");
         fileData.Append("         break;\n");
         fileData.Append("   if (index == NUM_FILES)\n");
         fileData.Append("      return -1;\n");
         fileData.Append("   return index;\n");
         fileData.Append("}\n\n");
         fileData.Append("//----------------------------------------------------------------------------\n\n");

         // Opens and decompresses (if a decompression function was set) then returns an index to access the decompressed data.
         fileData.Append("unsigned long ast_open_file_index(char const * inFileName)\n");
         fileData.Append("{\n");
         fileData.Append("   int index;\n");
         fileData.Append("   \n");
         fileData.Append("   index = ast_get_file_index(inFileName);\n");
         fileData.Append("   if (index == -1)\n");
         fileData.Append("      return -1;\n");
         fileData.Append("   \n");
         fileData.Append("   if (g_open_asset_file_data[index] != NULL)\n");
         fileData.Append("      return index;\n");
         fileData.Append("   \n");
         fileData.Append("   if (g_decompress_func == NULL || !g_asset_file_is_compressed_tbl[index]())\n");
         fileData.Append("   {\n");
         fileData.Append("      g_open_asset_file_data[index] = g_asset_file_data_tbl[index]();\n");
         fileData.Append("   }\n");
         fileData.Append("   else\n");
         fileData.Append("   {\n");
         fileData.Append("      g_open_asset_file_data[index] = new unsigned char[g_asset_file_length_tbl[index]()];\n");
         fileData.Append("      g_decompress_func((void*)g_asset_file_data_tbl[index](), g_asset_file_compressed_length_tbl[index](), (void*)g_open_asset_file_data[index], g_asset_file_length_tbl[index]());\n");
         fileData.Append("   }\n");
         fileData.Append("   return index;\n");
         fileData.Append("}\n\n");
         fileData.Append("//----------------------------------------------------------------------------\n\n");

         // Releases the temporary memory the file was decompressed into
         fileData.Append("void ast_close_file_index(unsigned long inFileIndex)\n");
         fileData.Append("{\n");
         fileData.Append("   if (inFileIndex >= NUM_FILES)\n");
         fileData.Append("      return;\n");
         fileData.Append("   if (g_decompress_func != NULL && g_asset_file_is_compressed_tbl[inFileIndex]())\n");
         fileData.Append("      delete [] g_open_asset_file_data[inFileIndex];\n");
         fileData.Append("   g_open_asset_file_data[inFileIndex] = NULL;\n");
         fileData.Append("}\n\n");
         fileData.Append("//----------------------------------------------------------------------------\n\n");

         // Gets the decompressed data
         fileData.Append("unsigned char const * ast_get_file_data(unsigned long inFileIndex)\n");
         fileData.Append("{\n");
         fileData.Append("   if (inFileIndex >= NUM_FILES)\n");
         fileData.Append("      return NULL;\n");
         fileData.Append("   return g_open_asset_file_data[inFileIndex];\n");
         fileData.Append("}\n\n");
         fileData.Append("//----------------------------------------------------------------------------\n\n");

         // Gets the decompressed data size
         fileData.Append("unsigned long ast_get_file_length(unsigned long inFileIndex)\n");
         fileData.Append("{\n");
         fileData.Append("   if (inFileIndex >= NUM_FILES)\n");
         fileData.Append("      return (unsigned long)-1;\n");
         fileData.Append("   return g_asset_file_length_tbl[inFileIndex]();\n");
         fileData.Append("}\n\n");

         fileData.Append("//----------------------------------------------------------------------------\n\n");


         // Check to see if the data has changed...Don't update if it hasn't
         MD5 md5 = MD5.Create();
         MemoryStream memStream = new MemoryStream(ASCIIEncoding.Default.GetBytes(fileData.ToString()), false);
         byte[] hash = md5.ComputeHash(memStream);

         String hashString = "// Hash: ";
         for (Int32 ii = 0; ii < hash.Length; ++ii)
            hashString += hash[ii].ToString();
         hashString += "\n";
         fileData.Insert(0, hashString);

         if (File.Exists(inTargetFile))
         {
            FileStream fileStream = File.OpenRead(inTargetFile);
            if (fileStream.Length == fileData.Length)
            {
               Byte[] fileHashArray = new Byte[hashString.Length];
               fileStream.Read(fileHashArray, 0, hashString.Length);
               String fileHashArrayString = ASCIIEncoding.Default.GetString(fileHashArray);
               if (hashString == fileHashArrayString)
               {
                  Log("File is the same. Not changing anything.");
                  return;
               }
            }
            fileStream.Close();
         }

         File.WriteAllText(inTargetFile, fileData.ToString());
      }
   }
}
