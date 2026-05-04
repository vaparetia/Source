using System;
using System.Collections.Generic;
using System.Text;

namespace Tools.Common.Perforce
{
   public class Helper
   {
      public static bool IsFileInDataPerforce(string asset)
      {
         Tools.Common.Perforce.Connection connection = new Connection();

         String overridePort = System.Environment.GetEnvironmentVariable("BPE_DATA_P4PORT");
         if (overridePort != String.Empty)
         {
            connection.Port = overridePort;
         }

         try
         {
            if (connection.Connect())
            {
               Tools.Common.Perforce.FileInfo fileInfo = connection.GetFileInfo(asset);
               if (fileInfo != null)
               {
                  return fileInfo.ExistsInDepot;
               }
            }
         }
         catch(Exception /*e*/)
         {
            return false;
         }
         finally
         {
            if (connection.IsConnected)
            {
               connection.Disconnect();
            }
         }

         return false;

      }

      /// <summary>
      /// Checks to see if assets (and referenced assets) used in 
      /// workspace are present on the perforce server.
      /// </summary>
      public static bool FixSubAssetsInP4(List<string> assets)
      {
         if (assets.Count == 0)
         {
            return true;
         }

         string tempArgsFile = System.IO.Path.GetTempFileName();
         string tempOutputFile = System.IO.Path.GetTempFileName();

         try
         {
            // Write the assets to the args file
            using (System.IO.StreamWriter argsWriter = new System.IO.StreamWriter(tempArgsFile, true))
            {
               foreach (string asset in assets)
               {
                  argsWriter.WriteLine(asset);
               }
            }

            System.Diagnostics.ProcessStartInfo toolStartInfo = new System.Diagnostics.ProcessStartInfo();
            toolStartInfo.UseShellExecute = false;
            toolStartInfo.CreateNoWindow = true;
            toolStartInfo.FileName = "P4Tool.exe";
            toolStartInfo.RedirectStandardOutput = true;
            toolStartInfo.Arguments = String.Format("fix \"@{0}\"", tempArgsFile);

            using (System.Diagnostics.Process toolProc = new System.Diagnostics.Process())
            {
               toolProc.StartInfo = toolStartInfo;
               toolProc.Start();
               string standardOutput = toolProc.StandardOutput.ReadToEnd();
               toolProc.WaitForExit();

               using (System.IO.StreamWriter outputWriter = new System.IO.StreamWriter(tempOutputFile, true))
               {
                  outputWriter.Write(standardOutput);
               }

               switch (toolProc.ExitCode)
               {
                  case 0:
                     // Perforce asset status good, all assets are present on server.
                     break;
                  case 1:
                     {
                        // p4tool detected a problem with assets.
                        System.Diagnostics.ProcessStartInfo notepadStartInfo = new System.Diagnostics.ProcessStartInfo();
                        notepadStartInfo.UseShellExecute = false;
                        notepadStartInfo.CreateNoWindow = true;
                        notepadStartInfo.FileName = "NP2.EXE";
                        notepadStartInfo.Arguments = String.Format("\"{0}\"", tempOutputFile);

                        using (System.Diagnostics.Process notepadProc = new System.Diagnostics.Process())
                        {
                           notepadProc.StartInfo = notepadStartInfo;
                           notepadProc.Start();
                           notepadProc.WaitForExit();
                        }
                     }
                     break;
                  case 2:
                     // Couldn't connect to perforce, could add an error message later but
                     // this could look unprofessional during offline demos.
                     break;
               }
            }
         }
         finally
         {
            System.IO.File.Delete(tempArgsFile);
            System.IO.File.Delete(tempOutputFile);
         }

         return true;
      }
   }
}
