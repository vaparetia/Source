using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Windows.Forms;

namespace TextTool
{
   // General helper functions for TextTool
   public class Utils
   {
      public enum CheckFileAccessAction
      {
         kDoNothing,
         kShowModalDialog,
         kAutoCheckOut,
         kAskCheckOut,
      }

      public static bool ConfirmFilesAccess( IEnumerable<String> paths, CheckFileAccessAction action )
      {
         foreach ( String path in paths )
         {
            if ( !ConfirmFileAccess( path, action ) )
            {
               return false;
            }
         }

         return true;
      }

      // See if we can write to a file
      public static bool ConfirmFileAccess(string path, CheckFileAccessAction action)
      {
         if (!File.Exists(path))
         {
            return true;
         }

         if ((File.GetAttributes(path) & (FileAttributes.ReadOnly | FileAttributes.Directory)) != 0)
         {
            // ReadOnly or a directory, can't write
            if (action == CheckFileAccessAction.kDoNothing)
            {
               return false;
            }
            if (action == CheckFileAccessAction.kShowModalDialog)
            {
               System.Windows.Forms.MessageBox.Show(String.Format("Cannot write to file {0}", path), "StringDB Error");
               return false;
            }
            if (action == CheckFileAccessAction.kAutoCheckOut)
            {
               bool bIsFileCheckedOut = CheckOutFile(path);
               return bIsFileCheckedOut;
            }

            if (action == CheckFileAccessAction.kAskCheckOut)
            {
               DialogResult result = MessageBox.Show(String.Format("Cannot write to file {0}, check out from P4?", path),
                                                      "StringDB Warning",
                                                      MessageBoxButtons.YesNo,
                                                      MessageBoxIcon.Warning);
               if (result == DialogResult.Yes)
               {
                  bool bIsFileCheckedOut = CheckOutFile(path);
                  if (!bIsFileCheckedOut)
                  {
                     MessageBox.Show(String.Format("Couldn't check out file {0}", path), "StringDB Error");
                  }
                  return bIsFileCheckedOut;
               }
               return false;
            }

            // User doesn't want to check out file.
            return false;
         }

         // Shouldn't get here
         return true;
      }

      // Very basic functionality for P4 EDIT 
      public static bool CheckOutFile(string path)
      {
         //Create process
         System.Diagnostics.Process pProcess = new System.Diagnostics.Process();

         //strCommand is path and file name of command to run
         pProcess.StartInfo.FileName = "p4.exe";

         //strCommandParameters are parameters to pass to program
         pProcess.StartInfo.Arguments = String.Format("edit {0}", path);

         pProcess.StartInfo.UseShellExecute = false;

         //Set output of program to be written to process output stream
         pProcess.StartInfo.RedirectStandardOutput = true;

         //Hide window
         pProcess.StartInfo.CreateNoWindow = true;

         //Optional
         //pProcess.StartInfo.WorkingDirectory = strWorkingDirectory;

         //Start the process
         pProcess.Start();

         //Get program output
         string strOutput = pProcess.StandardOutput.ReadToEnd();

         //Wait for process to finish
         pProcess.WaitForExit();

         if (strOutput.Contains("opened for edit"))
         {
            // p4 checked out the file or it is already checked out
            return true;
         }

         // Couldn't check out the file
         return false;
      }
   }
}
