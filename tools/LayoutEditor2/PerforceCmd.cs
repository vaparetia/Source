using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.IO;

public class PerforceCmd
{
   #region Private Fields
   private static Boolean mHasShownMessage = false;
   private static List<String> mLastFilePathListUsed = null;
   #endregion

   public static Boolean SyncToHead(List<String> inFilePaths, Boolean showMessageOnFail)
   {
      Int32 exitCode = 0;
      String output = "";
      if (!ExecuteP4Command("sync", inFilePaths, showMessageOnFail, out output, out exitCode))
         return false;
      return true;
   }
   public static Boolean CheckoutFile(List<String> inFilePaths, Boolean inLockFile, Boolean showMessageOnFail)
   {
      Int32 exitCode = 0;
      String output = "";
      if (!ExecuteP4Command("edit", inFilePaths, showMessageOnFail, out output, out exitCode))
         return false;
      if (inLockFile)
         if (!ExecuteP4Command("lock", inFilePaths, showMessageOnFail, out output, out exitCode))
            return false;
      return true;
   }

   public static Boolean CheckoutFile(String fileName, Boolean lockFile, Boolean showMessageOnFail)
   {
      List<String> files = new List<String>();
      files.Add(fileName);
      return CheckoutFile(files, lockFile, showMessageOnFail);
   }

   public static void CheckoutFileThrow(List<String> inFilePaths, Boolean inLockFile)
   {
      if (!CheckoutFile(inFilePaths, inLockFile, true))
      {
         throw new Exception("Perforce Error");
      }
   }

   public static Boolean AddFile(List<String> inFilePaths, Boolean showMessageOnFail)
   {
      Int32 exitCode = 0;
      String output = "";
      if (!ExecuteP4Command("add", inFilePaths, showMessageOnFail, out output, out exitCode))
         return false;

      return true;
   }

   public static Boolean AddFile(String fileName, Boolean showMessageOnFail)
   {
      List<String> files = new List<String>();
      files.Add(fileName);
      return AddFile(files, showMessageOnFail);
   }

   public static void AddFileThrow(List<String> inFilePaths)
   {
      if (!AddFile(inFilePaths, true))
      {
         throw new Exception("Perforce Error");
      }
   }

   public static Boolean GetFilesOpenForEdit(out List<String> outFilePaths, Boolean showMessageOnFail)
   {
      Int32 exitCode = 0;
      String output = "";
      outFilePaths = null;
      if (!ExecuteP4Command("opened", null, showMessageOnFail, out output, out exitCode))
         return false;
      outFilePaths = new List<String>();
      return true;
   }
   public static Boolean IsOpenForAddOrEdit(String inFilePath, Boolean showMessageOnFail, Boolean throwOnFail)
   {
      Int32 exitCode = 0;
      String output = "";
      List<String> filePaths = new List<String>() { inFilePath };

      if (!ExecuteP4Command("opened", filePaths, showMessageOnFail, out output, out exitCode))
      {
         if (throwOnFail)
         {
            throw new Exception("Perforce error calling ExecuteP4Command");
         }
         return false;
      }

      return !output.Contains("file(s) not opened");
   }

   public static Boolean GetFileDepotLocations(List<String> inFilePaths, out List<String> outDepotPaths, Boolean showMessageOnFail)
   {
      Int32 exitCode = 0;
      String output = "";
      outDepotPaths = null;
      if (!ExecuteP4Command("files", inFilePaths, showMessageOnFail, out output, out exitCode))
         return false;

      String[] outLines = output.Split('\n');
      outDepotPaths = new List<String>();
      foreach (String line in outLines)
      {
         if (String.IsNullOrEmpty(line.Trim()))
            continue;

         String depotFile = "";
         Int32 hashIndex = line.IndexOf('#');
         if (hashIndex != -1)
            depotFile = line.Substring(0, hashIndex);
         outDepotPaths.Add(depotFile);
      }
      return true;
   }

   #region Private Methods
   private static Boolean StartProcessAndWait(String exeName, String arguments, Boolean showMessageOnFail, ref String output, ref Int32 exitCode)
   {
      String finalOutput = "";
      try
      {
         ProcessStartInfo startInfo = new ProcessStartInfo();
         startInfo.UseShellExecute = false;
         startInfo.CreateNoWindow = true;
         startInfo.FileName = exeName;
         startInfo.Arguments = arguments;
         startInfo.RedirectStandardOutput = output != null;
         startInfo.RedirectStandardError = true;

         System.Diagnostics.Process p = System.Diagnostics.Process.Start(startInfo);
         p.OutputDataReceived += delegate(Object sender, DataReceivedEventArgs e)
         {
            if (!String.IsNullOrEmpty(e.Data))
               finalOutput += e.Data + "\n";
         };

         if (p != null)
         {
            p.BeginOutputReadLine();
            String error = p.StandardError.ReadToEnd();

            p.WaitForExit();
            if (output != null)
               output = finalOutput + error;
            exitCode = p.ExitCode;
            p.Dispose();
            if (exitCode == 0)
            {
               mHasShownMessage = false;
               return true;
            }
         }
      }
      catch (Exception e)
      {
         if (e.Message == "The system cannot find the file specified")
         {
            Console.WriteLine(exeName + " is not installed on this machine.\nPlease see your Administrator.");
         }
      }
      if (showMessageOnFail && exitCode == 1 && !mHasShownMessage)
      {
         Console.WriteLine("There was an error connecting with Perforce. More information may be available from your administrator.\n\n1. Make sure the default settings for Perforce are set up properly.\n2. Make sure the perforce server is running and accessible from this machine.\n3. Make sure that the perforce client is installed on this machine.\n\nChanges were not saved.\n\nReported Errors:\n" + output + "\n");
         mHasShownMessage = true;
      }
      return false;
   }
   private static Boolean StartProcessAndForget(String exeName, String arguments, Boolean showMessageOnFail)
   {
      try
      {
         ProcessStartInfo startInfo = new ProcessStartInfo();
         startInfo.UseShellExecute = false;
         startInfo.CreateNoWindow = true;
         startInfo.FileName = exeName;
         startInfo.Arguments = arguments;

         System.Diagnostics.Process p = System.Diagnostics.Process.Start(startInfo);
         return true;
      }
      catch (Exception e)
      {
         if (e.Message == "The system cannot find the file specified")
         {
            Console.WriteLine(exeName + " is not installed on this machine.\nPlease see your Administrator.");
         }
      }
      if (showMessageOnFail && !mHasShownMessage)
      {
         Console.WriteLine("There was an error connecting with Perforce. More information may be available from your administrator.\n\n1. Make sure the default settings for Perforce are set up properly.\n2. Make sure the perforce server is running and accessible from this machine.\n3. Make sure that the perforce client is installed on this machine.\n\nChanges were not saved.");
         mHasShownMessage = true;
      }
      return false;
   }
   private static Boolean ExecuteP4Command(String cmd, List<String> inFilePaths, Boolean showMessageOnFail, out String output, out Int32 exitCode)
   {
      output = "";
      exitCode = 0;
      if (inFilePaths == null)
         return StartProcessAndWait("p4.exe", cmd, showMessageOnFail, ref output, ref exitCode);
      else if (inFilePaths.Count == 1)
         return StartProcessAndWait("p4.exe", cmd + " " + inFilePaths[0], showMessageOnFail, ref output, ref exitCode);
      else
         return StartProcessAndWait("p4.exe", "-x " + MakeFileList(inFilePaths) + " -d \"\" " + cmd, showMessageOnFail, ref output, ref exitCode);
   }
   private static String MakeFileList(List<String> inFiles)
   {
      String p4FilePath = Environment.GetEnvironmentVariable("TEMP") + "\\p4files.txt";
      if (inFiles == mLastFilePathListUsed)
         return p4FilePath;
      mLastFilePathListUsed = inFiles;

      StringBuilder filePaths = new StringBuilder();
      foreach (String filePath in inFiles)
      {
         filePaths.AppendFormat("{0}\n", filePath);
      }
      filePaths = filePaths.Replace("\\", "/");
      File.WriteAllText(p4FilePath, filePaths.ToString());

      return p4FilePath;
   }
   #endregion
}

