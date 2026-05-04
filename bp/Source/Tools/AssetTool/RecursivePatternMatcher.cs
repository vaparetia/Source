using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using Tools.AssetSystem;

namespace AssetTool
{
   class RecursivePatternMatcher
   {
      Manager mAssetManager = new Manager();
      List<string> mInputFilenames = new List<string>();
      List<string> mMatchStrings = new List<string>();
      public bool mRecursive = false;

      public void AddSpecification(string specification)
      {
         if (specification.IndexOfAny(new char[] { '*', '?' }) >= 0)
         {
            mMatchStrings.Add(specification);
         }
         else
         {
            mInputFilenames.Add(specification);
         }
      }

      public IEnumerable<string> Inputs
      {
         get
         {
            foreach ( String s in mMatchStrings )
            {
               yield return s;
            }

            foreach ( String s in mInputFilenames )
            {
               yield return s;
            }
         }
      }

      public void Match(out List<string> matches)
      {
         // process all match strings and file paths
         {
            if (mMatchStrings.Count > 0)
            {
               string startFolder = Environment.CurrentDirectory;
               string resultFolder = mAssetManager.GetRepositoryRelativePath(startFolder);
               matches = Helper.FileHelper.MatchFiles(startFolder, mMatchStrings, mRecursive, resultFolder);
            }
            else
            {
               matches = new List<string>();
            }

            foreach (string inputFilename in mInputFilenames)
            {
               string workingInputFilename = inputFilename;

               ProcessFile(workingInputFilename, matches);
            }
         }
      }

      /// <summary>
      /// Helper function that calls asset cook or edit metadata delegate.
      /// </summary>
      void ProcessFile(string inputFilename, List<string> matches)
      {
         string assetPath = mAssetManager.GetRepositoryRelativePath(inputFilename);
         if (!matches.Contains(assetPath))
            matches.Add(assetPath);
      }
   }
}
