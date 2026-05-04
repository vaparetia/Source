using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;
using TextToolData;
using System.IO;

namespace TextTool
{
   static class Program
   {
      static string skRootPath = Environment.ExpandEnvironmentVariables("%BPE_REPOSITORY%");
      static Backend mBackend = new Backend();
      enum TextToolMode
      {
         kStringEditor,
         kCharacterEditor,
         kRebuildStringDB,
         kBuildOverrideTable
      }

      public static void BuildOverrideTable( StringDB stringDB )
      {
         // Write X360 string override table
         {
            string outputPath = Path.Combine(skRootPath, "EngineSupport\\BP_StringOverridesX360.cpp");
            if (TextTool.Utils.ConfirmFileAccess(outputPath, Utils.CheckFileAccessAction.kAskCheckOut))
               stringDB.WriteOverrideTable(TextPlatform.X360, outputPath);
         }

         // Write PS3 string override table
         {
            string outputPath = Path.Combine(skRootPath, "EngineSupport\\BP_StringOverridesPS3.cpp");
            if (TextTool.Utils.ConfirmFileAccess(outputPath, Utils.CheckFileAccessAction.kAskCheckOut))
               stringDB.WriteOverrideTable(TextPlatform.PS3, outputPath);
         }

         // Write Vita string override table
         {
            string outputPath = Path.Combine(skRootPath, "EngineSupport\\BP_StringOverridesVTA.cpp");
            if (TextTool.Utils.ConfirmFileAccess(outputPath, Utils.CheckFileAccessAction.kAskCheckOut))
               stringDB.WriteOverrideTable(TextPlatform.Vita, outputPath);
         }
      }

      /// <summary>
      /// The main entry point for the application.
      /// </summary>
      [STAThread]
      static int Main(string[] args)
      {
         Application.EnableVisualStyles();
         Application.SetCompatibleTextRenderingDefault(false);

         TextToolMode textToolMode = TextToolMode.kStringEditor;

         // Very simple command line args
         if (args.Length > 0)
         {
            // Ignore first character (ick)
            string theArg = args[0].Substring(1);

            if (theArg == "StringEditor")
            {
               textToolMode = TextToolMode.kStringEditor;
            }
            else if (theArg == "CharacterEditor")
            {
               textToolMode = TextToolMode.kCharacterEditor;
            }
            else if (theArg == "RebuildStringDB")
            {
               textToolMode = TextToolMode.kRebuildStringDB;
            }
            else if(theArg == "BuildOverrideTable")
            {
               textToolMode = TextToolMode.kBuildOverrideTable;
            }
            else
            {
               MessageBox.Show(String.Format("Unknown command line argument {0}\nValid arguments:\n -StringEditor\n -CharacterEditor\n -RebuildStringDB\n -BuildOverrideTable", args[0]), "TextTool");
               return 1;
            }
         }
         Form mainForm = null;
         
         if( textToolMode == TextToolMode.kBuildOverrideTable)
         {
            // BuildOverrideTable();
            Console.WriteLine( "Command line override table building is disabled." );

            // Immediately exit (treat this like a command line process).
            return 1;
         }
         else if (textToolMode != TextToolMode.kCharacterEditor)
         {
            // We're running in StringEditor mode
            bool bRebuildStringDB = false;
            if (textToolMode == TextToolMode.kRebuildStringDB)
            {
               // Tell StringEditor to rebuild the DB
               bRebuildStringDB = true;            
            }

            mainForm = new StringEditor(skRootPath, mBackend, bRebuildStringDB);

            if (bRebuildStringDB)
            {
               // Immediately exit (treat this like a command line process).
               return 0;
            }
         }
         else
         {
            // We're running in character editor mode
            mainForm = new MainForm(skRootPath, mBackend);
         }

         Application.Run(mainForm);

         return 0;
      }
   }
}
