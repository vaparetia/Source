using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace Tools.Common
{
   public class WorkspaceHelper
   {
      /// <summary>
      /// Searches up current directory tree to find a workspace file.
      /// Stops before repository root, otherwise all script files would find a workspace if one was present in the root.
      /// Returns empty string if no workspace.
      /// </summary>
      static public string GetWorkspaceForPath(string filePath, Tools.AssetSystem.Manager assetManager)
      {
         string directoryName = Path.GetDirectoryName(filePath);

         DirectoryInfo directoryInfo = new DirectoryInfo(directoryName);
         while((directoryInfo != null) && (assetManager.GetRepositoryRelativePath(directoryInfo.FullName) != "$/"))
         {
            string[] workSpaceFiles = System.IO.Directory.GetFiles(directoryInfo.FullName, "*.bws");
            
            if (workSpaceFiles.Length > 0)
               return workSpaceFiles[0];

            directoryInfo = directoryInfo.Parent;
         }

         return "";
      }
   }
}
