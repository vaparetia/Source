using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Runtime.InteropServices;

namespace Tools.Common
{
   static public class FileUtils
   {
      /// <summary>
      /// Opens a Text stream reader, but never throws an exception.  Only returns 
      /// null on failure
      /// </summary>
      /// <param name="fileName">File to open</param>
      /// <returns>StreamReader to the file, or null on failure</returns>
      static public System.IO.StreamReader OpenTextSafe(string fileName)
      {
         try
         {
            return System.IO.File.OpenText(fileName);
         }
         catch (System.Exception)
         {
            return null;
         }
      }

      /// <summary>
      /// If any args start with @, inserts those args in the resultant args array
      /// </summary>
      /// <param name="args">Incoming original arguments</param>
      /// <returns>Arguments returned from file</returns>
      static public string[] ExpandArgsFromArgsFiles(string[] args)
      {
         List<string> outArgs = new List<String>();

         foreach (string arg in args)
         {
            if (arg != String.Empty)
            {
               if (arg.StartsWith("@"))
               {
                  // We're taking input from a file.  Take each arg per line
                  using (System.IO.StreamReader fileStream = OpenTextSafe(arg.Substring(1)))
                  {
                     if (fileStream == null)
                     {
                        // If we couldn't find the file, then just add the argument with the @
                        // in there.
                        outArgs.Add(arg);
                     }
                     else
                     {
                        while (!fileStream.EndOfStream)
                        {
                           string line = fileStream.ReadLine().Trim();

                           if (line != "")
                           {
                              outArgs.Add(line);
                           }
                        }
                     }
                  }
               }
               else
               {
                  outArgs.Add(arg);
               }
            }
         }

         return outArgs.ToArray();
      }

      /// <summary>
      /// Builds a relative path to a file given a root path.
      /// e.g. Given
      /// string mainDirPath = "c:\dev\next\source\gamecomponents"
      /// string absoluteFilePath = "c:\dev\next\source\gameobjects\gamecontroller\CComponentGameControllerBlastFactor.cpp"
      /// It will return:
      /// "..\gameobjects\gamecontroller\ComponentGameControllerBlastFactor.cpp"
      /// Useful for creating vcproj 'RelativePaths'.
      /// </summary>      
      public static string MakeRelativePath(string mainDirPath, string absoluteFilePath)
      {
         string[] firstPathParts = mainDirPath.Trim(Path.DirectorySeparatorChar).Split(Path.DirectorySeparatorChar);
         string[] secondPathParts = absoluteFilePath.Trim(Path.DirectorySeparatorChar).Split(Path.DirectorySeparatorChar);

         int sameCounter = 0;
         for (int i = 0; i < Math.Min(firstPathParts.Length, secondPathParts.Length); i++)
         {
            if (!firstPathParts[i].ToLower().Equals(secondPathParts[i].ToLower()))
            {
               break;
            }
            sameCounter++;
         }

         if (sameCounter == 0)
         {
            return absoluteFilePath;
         }

         string newPath = String.Empty;
         for (int i = sameCounter; i < firstPathParts.Length; i++)
         {
            if (i > sameCounter)
            {
               newPath += Path.DirectorySeparatorChar;
            }
            newPath += "..";
         }
         if (newPath.Length == 0)
         {
            newPath = ".";
         }
         for (int i = sameCounter; i < secondPathParts.Length; i++)
         {
            newPath += Path.DirectorySeparatorChar;
            newPath += secondPathParts[i];
         }
         return newPath;
      }

      static public bool ChangeFileAttributes(String fileName, System.IO.FileAttributes mask, System.IO.FileAttributes toSet)
      {
         if (System.IO.File.Exists(fileName))
         {
            System.IO.FileAttributes attributes = System.IO.File.GetAttributes(fileName);
            System.IO.FileAttributes newAttributes = (attributes & (~mask)) | toSet;

            if (attributes != newAttributes)
            {
               System.IO.File.SetAttributes(fileName, newAttributes);
            }

            return true;
         }
         else
         {
            return false;
         }
      }

      static public String GetBluepointAppdataFolder()
      {
         String appDataFolder = System.Environment.GetEnvironmentVariable("APPDATA");
         return appDataFolder + "\\Bluepoint Games";
      }

      /// <summary>
      /// Gets the size of a file on disk
      /// </summary>
      /// <param name="fileName">File whose size to get</param>
      /// <returns>The file size or -1 on failure</returns>
      static public Int64 GetFileSize(string fileName)
      {
         if (fileName.IndexOfAny(new char[] { '*', '?' }) != -1)
         {
            // Don't accept wildcards
            return -1;
         }

         WindowsNative.WIN32_FIND_DATA wfd = new WindowsNative.WIN32_FIND_DATA();
         using (SafeFindHandle found = WindowsNative.FindFirstFile(fileName, wfd))
         {

            if (!found.IsInvalid)
            {
               return (((long)wfd.nFileSizeHigh << 32) | (wfd.nFileSizeLow & ((long)0xffffffffL)));
            }
         }

         return -1;
      }

      /// <summary>
      /// Pass in two files, if file2 is newer, or the only one that exists, it is returned.
      /// Otherwise (if file1 is newer, only it exists, or neither exist) file1 is returned
      /// </summary>
      /// <param name="file1"></param>
      /// <param name="file2"></param>
      /// <returns></returns>
      public static string ChooseNewerFile(string file1, string file2)
      {
         bool fileExists1 = File.Exists(file1);
         bool fileExists2 = File.Exists(file2);

         // If both files exist chose the newer one
         if (fileExists1 && fileExists2)
         {
            DateTime fileTime1 = File.GetLastWriteTime(file1);
            DateTime fileTime2 = File.GetLastWriteTime(file2);

            // Compare file time
            if (fileTime1.CompareTo(fileTime2) > 0)
            {
               return file1;
            }
            else
            {
               return file2;
            }
         }
         // If only file 2 exists, return that one
         else if (fileExists2)
         {
            return file2;
         }
         // If only file 1 exists (or none), return the first one
         else
         {
            return file1;
         }
      }

      [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
      internal struct SHFILEOPSTRUCT
      {
         public IntPtr hwnd;
         public UInt32 wFunc;
         [MarshalAs(UnmanagedType.LPWStr)]
         public String pFrom;
         [MarshalAs(UnmanagedType.LPWStr)]
         public String pTo;
         public UInt16 fFlags;
         public Int32 fAnyOperationsAborted;
         public IntPtr hNameMappings;
         [MarshalAs(UnmanagedType.LPWStr)]
         public String lpszProgressTitle;
      }

      [DllImport("shell32.dll", CharSet = CharSet.Unicode)]
      internal static extern Int32 SHFileOperation(ref SHFILEOPSTRUCT lpFileOp);
      internal const int FO_DELETE = 3;
      internal const int FOF_ALLOWUNDO = 0x40;
      internal const int FOF_NOCONFIRMATION = 0x10;

      /// <summary>
      /// Sends the specified file/folder to the recycle bin
      /// </summary>
      public static void RecycleFile(string sPath)
      {
         List<string> files = new List<string>();
         files.Add(sPath);

         RecycleFiles(files);
      }

      public static void RecycleFiles(List<string> files)
      {
         string fromPath = "";

         foreach(string file in files)
         {
            fromPath += file.Replace('/', '\\') + '\0';
         }

         fromPath += '\0';

         SHFILEOPSTRUCT shf = new SHFILEOPSTRUCT();
         shf.wFunc = (uint)FO_DELETE;
         shf.fFlags = (ushort)FOF_ALLOWUNDO | FOF_NOCONFIRMATION;
         shf.pFrom = fromPath;
         shf.fAnyOperationsAborted = 0;
         shf.hNameMappings = IntPtr.Zero;

         SHFileOperation(ref shf);
      }

      /// <summary>
      /// Transforms a file path into a lower case string with consistent slashes.
      /// 
      /// You should not use this to make paths human-readable, but rather to make
      /// it easy to compare file path names.
      /// </summary>
      /// <param name="fileName">File name to normalize</param>
      /// <returns>Lower case and consistently "slashed" name</returns>
      static public String NormalizeFileName(String fileName)
      {
         if (String.IsNullOrEmpty(fileName))
         {
            return "";
         }

         String initialFixedName = fileName.ToLower().Replace('/', '\\');

         // If we have a double backslash (and are not starting with one due to UNC)
         // then we need to remove them
         if (initialFixedName.IndexOf("\\\\", 1) != -1)
         {
            bool wasSlash = false;
            String slashFixedName = "";
            // Remove duplicate slashes
            // Since filenames can start with "\\", always just write the first character
            slashFixedName += initialFixedName[0];
            for (int i = 1; i < initialFixedName.Length; ++i)
            {
               char ch = initialFixedName[i];
               if (ch == Path.DirectorySeparatorChar)
               {
                  if (!wasSlash)
                  {
                     slashFixedName += ch;
                  }

                  wasSlash = true;
               }
               else
               {
                  wasSlash = false;
                  slashFixedName += ch;
               }
            }

            return slashFixedName;
         }
         else
         {
            return initialFixedName;
         }
      }
   }
}
