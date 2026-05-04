using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using TextToolData;
using System.IO;
using System.Text.RegularExpressions;
using System.Net;
using System.Web;

namespace TextTool
{
   public partial class StringEditor : Form
   {
      Backend mBackend;
      StringDB mStringDB;

      Dictionary<String, String> mRemappingPaths = new Dictionary<String, String>();

      public static string GetRemapPath(string rootPath)
      {
         return Path.Combine(rootPath, "StringRemapDB.xml");
      }

      static public String GetUniqueRemapPath( String rootPath, String unique )
      {
         return Path.Combine( rootPath, String.Format( "StringRemapDB_{0}.xml", unique ) );
      }

      public StringEditor(string rootPath, Backend backend, bool bRebuildStringDB)
      {
         mBackend = backend;
         mStringDB = new StringDB(mBackend);
         string stringDBPath = Path.Combine(rootPath, "StringDB.xml");
         string customDBPath = Path.Combine( rootPath, "StringCustomDB.xml" );
         InitializeComponent();

         if (bRebuildStringDB)
         {
            if (!Utils.ConfirmFileAccess(stringDBPath, Utils.CheckFileAccessAction.kAutoCheckOut))
            {
               // Can't write to StringDB, quit before doing several hours of processing then crashing!
               return;
            }

            Console.WriteLine("Rebuilding StringDB, this will take a while (1+ hours?)");
            List<string> matchStrings = new List<string>();
            matchStrings.Add("*.cap");
            matchStrings.Add("*.strings");
            Console.WriteLine("Matching files...");
            List<string> files = Helper.FileHelper.MatchFiles(rootPath, matchStrings, true, rootPath);
            files.Sort();
            Console.WriteLine("Unify strings...");
            mStringDB.UnifyStrings(rootPath, files);
            Console.WriteLine("Saving DB...");
            mStringDB.SaveDB(stringDBPath);
            Console.WriteLine("Done!");
            return;
         }

         mStringDB.LoadDB(stringDBPath);
         mStringDB.AddCustomDB( customDBPath );

         mRemappingPaths.Clear();
         mRemappingPaths[String.Empty] = GetRemapPath( rootPath );

         foreach ( String output in mStringDB.GetUniqueOutputs() )
         {
            mRemappingPaths[output] = GetUniqueRemapPath( rootPath, output );
         }

         mStringDB.ClearRemapping();

         foreach ( var remapping in mRemappingPaths )
         {
            mStringDB.AddRemapping( remapping.Value, remapping.Key );
         }

         searchFilterComboBox.SelectedIndex = 0;
         previewPlatformComboBox.SelectedIndex = 0;
         overridePlatformComboBox.SelectedIndex = 0;
         previewModeComboBox.SelectedIndex = 0;
         UpdateTextList();
      }

      List<int> filteredItems = new List<int>();
      
      enum StringFilterSource
      {
         Original,
         Remapped,
         RemappedVita,
         RemappedPS3
      }

      int mCurrentPlatform = 0;
      TextToolHelper.TextPaneMode mPreviewMode = TextToolHelper.TextPaneMode.Default;

      StringFilterSource mStringFilterSource = StringFilterSource.Original;
      bool mShowOverridesOnly = false;
      bool mMatchCase = false;

      List<StringEntry> mSourceEntryList;

      private void UpdateTextList()
      {
         string infoFilter = infoSearchTextBox.Text.ToLower();
         string fileFilter = fileFilterTextBox.Text.ToLower();

         bool showOverridesOnly = mShowOverridesOnly || !string.IsNullOrEmpty(infoFilter);

         string stringFilter = stringFilterTextBox.Text;
         if (stringFilter != null && !mMatchCase)
         {
            stringFilter = stringFilter.ToLower();
         }

         StringEntry selectedEntry = null;

         if( textListView.SelectedIndices.Count > 0 )
         {
            int realIndex = filteredItems[textListView.SelectedIndices[0]];
            selectedEntry = mSourceEntryList[realIndex];
         }

         mSourceEntryList = new List<StringEntry>(mStringDB.mUnifiedEntries);

         /*
         if (!string.IsNullOrEmpty(fileFilter) )
         {
            mSourceEntryList.Sort(
               delegate(StringEntry left, StringEntry right)
               {
                  StringFileRef leftFileRef = left.Files[0];
                  StringFileInfo leftFileInfo = mStringDB.mFiles[leftFileRef.FileIdx];

                  StringFileRef rightFileRef = right.Files[0];
                  StringFileInfo rightFileInfo = mStringDB.mFiles[rightFileRef.FileIdx];

                  float leftSortId = (float)leftFileRef.FileStringIdx / leftFileInfo.StringCount;
                  float rightSortId = (float)rightFileRef.FileStringIdx / rightFileInfo.StringCount;

                  if (leftSortId < rightSortId)
                     return -1;
                  else if (leftSortId > rightSortId)
                     return 1;
                  else
                  {
                     return leftFileInfo.FileName.CompareTo(rightFileInfo.FileName);
                  }
               }
            );
         }
		 */
		 
         filteredItems.Clear();

         int newSelectedIndex = -1;

         for (int i = 0; i < mSourceEntryList.Count; ++i)
         {
            StringEntry entry = mSourceEntryList[i];
            
            if( fileFilter != null )
            {
               bool fileFound = false;

               foreach(StringFileRef fileId in entry.Files)
               {
                  string entryFileName = mStringDB.mFiles[fileId.FileIdx].FileName;
                  if( entryFileName.ToLower().Contains(fileFilter) )
                  {
                     fileFound = true;
                     break;
                  }
               }

               if (!fileFound)
                  continue;
            }

            string originalString = entry.String;

            bool shouldAddEntry = true;

            if( stringFilter != null )
            {
               string compareString;

               switch (mStringFilterSource)
               {
                  case StringFilterSource.Remapped:
                     {
                        string remappedString;
                        mStringDB.GetFinalGameString(entry, GetOverridePlatform(), out remappedString);

                        if (remappedString != null)
                           compareString = remappedString;
                        else
                           compareString = originalString;
                     }
                     break;

                  case StringFilterSource.RemappedVita:
                     {
                        string remappedString;
                        mStringDB.GetFinalGameStringOnlyOverride( entry, TextPlatform.Vita, out remappedString );

                        compareString = remappedString ?? "";
                     }
                     break;

                  case StringFilterSource.RemappedPS3:
                     {
                        string remappedString;
                        mStringDB.GetFinalGameStringOnlyOverride( entry, TextPlatform.PS3, out remappedString );

                        compareString = remappedString ?? "";
                     }
                     break;

                  default:
                  case StringFilterSource.Original:
                     compareString = originalString;
                     break;
               }

               if( !mMatchCase )
               {
                  compareString = compareString.ToLower();
               }
               
               shouldAddEntry = FilterEntry(compareString, stringFilter);
            }

            if (showOverridesOnly )
            {
               StringOverride overrideEntry = mStringDB.GetOverrideEntry(entry);
               if( overrideEntry == null )
               {
                  shouldAddEntry = false;
               }
               else
               {
                  string info = (overrideEntry.Info != null) ? overrideEntry.Info : "";

                  if (!info.ToLower().Contains(infoFilter))
                  {
                     shouldAddEntry = false;
                  }
               }
            }

            if( shouldAddEntry )
            {
               if (entry == selectedEntry)
                  newSelectedIndex = filteredItems.Count;

               filteredItems.Add(i);
            }
         }

         textListView.VirtualListSize = filteredItems.Count;

         if (newSelectedIndex != -1 )
         {
            textListView.EnsureVisible(newSelectedIndex);

            textListView.SelectedIndices.Clear();
            textListView.SelectedIndices.Add(newSelectedIndex);
         }

         OnTextSelectionChanged(null, null);
      }

      private void OnRetrieveTextItem(object sender, RetrieveVirtualItemEventArgs e)
      {
         if (e.ItemIndex < filteredItems.Count)
         {
            int actualIndex = filteredItems[e.ItemIndex];

            StringEntry entry = mSourceEntryList[actualIndex];
            e.Item = new ListViewItem(entry.String);

            e.Item.Checked = true;

            StringOverride overrideEntry = mStringDB.GetOverrideEntry(entry);

            string overrideString = "";

            if(overrideEntry != null)
            {
               foreach(TextPlatform platform in overrideEntry.OverrideString.Keys)
               {
                  if (overrideString.Length > 0)
                     overrideString += ", ";
                  
                  overrideString += platform.ToString();
               }
            }
            else
            {
               e.Item.Checked = false;
            }

            e.Item.SubItems.Add(overrideString);
         }
      }

      private void OnRetrieveFileItem(object sender, RetrieveVirtualItemEventArgs e)
      {
         StringEntry entry = GetSelectedEntry();
         if (entry != null)
         {
            if (e.ItemIndex < entry.Files.Count)
            {
               string filePath = mStringDB.mFiles[entry.Files[e.ItemIndex].FileIdx].FileName;

               e.Item = new ListViewItem(filePath);
            }
         }
      }

      private StringEntry GetSelectedEntry()
      {
         int selectionCount = textListView.SelectedIndices.Count;
         int selectedIndex = (selectionCount > 0) ? textListView.SelectedIndices[selectionCount - 1] : -1;

         if( selectedIndex >= 0 )
         {
            selectedIndex = filteredItems[selectedIndex];
            return mSourceEntryList[selectedIndex];
         }
         
         return null;
      }

      private List<StringEntry> GetSelectedEntries()
      {
         List<StringEntry> result = new List<StringEntry>();
         foreach( int idx in textListView.SelectedIndices)
         {
            int realIdx = filteredItems[idx];
            result.Add(mSourceEntryList[realIdx]);
         }

         return result;
      }

      private void UpdateUIForCurrentTextItem()
      {
         UpdateOriginalTextPreview();

         StringEntry entry = GetSelectedEntry();

         if (entry != null)
         {
            TextPlatform platform = GetOverridePlatform();

            string remappedString;
            mStringDB.GetFinalGameString(entry, platform, out remappedString);
            
            bool hasPlatformOverride = mStringDB.HasOverride(entry, platform);
            bool isOverrideReadOnly = StringOverride.IsOverrideReadonly(platform);
            
            remappedTextBox.Text = remappedString;
            remappedTextBox.ReadOnly = isOverrideReadOnly || !hasPlatformOverride;
            
            overrideButton.Enabled = !isOverrideReadOnly && !hasPlatformOverride;
            removeOverrideButton.Enabled = !isOverrideReadOnly && hasPlatformOverride;

            fileListView.VirtualListSize = entry.Files.Count;
            fileListView.Refresh();

            // Update info text box
            {
               StringOverride overrideEntry = mStringDB.GetOverrideEntry(entry);
               infoRichTextBox.Text = (overrideEntry != null) ? overrideEntry.Info : "";
               infoRichTextBox.Enabled = mStringDB.HasOverride(entry);
            }
         }
         else
         {
            fileListView.VirtualListSize = 0;
         }
      }

      private void UpdateOriginalTextPreview()
      {
         StringEntry entry = GetSelectedEntry();
         if (entry != null)
         {
            string gameText = mStringDB.ApplyKeywordReplacement(entry.String);
            byte[] text = mBackend.ConvertStringToUTF8(gameText);
            originalPreview.Image = mBackend.TextRender.BP_RenderText(text, originalPreview.Width, originalPreview.Height, mCurrentPlatform);
         }
      }

      private void OnTextChanged(object sender, EventArgs e)
      {
         UpdateRemappedTextPreview();

         StringEntry entry = GetSelectedEntry();
         if (entry != null)
         {
            TextPlatform platform = GetOverridePlatform();

            if (mStringDB.HasOverride(entry, platform))
            {
               string currentOverride;
               mStringDB.GetFinalGameString(entry, platform, out currentOverride);
               
               string newOverride = remappedTextBox.Text.Replace("\r", "");

               if (currentOverride != newOverride)
               {
                  mStringDB.SetStringOverride(entry, platform, newOverride);
                  QueueRemapSave();
               }
            }
         }
      }

      private void UpdateRemappedTextPreview()
      {
         byte[] text;
         try
         {
            string gameText = mStringDB.ApplyKeywordReplacement(remappedTextBox.Text);
            text = mBackend.ConvertStringToUTF8(gameText);
         }
         catch (Exception /*ex*/)
         {
            text = mBackend.ConvertStringToUTF8("[Decode Error]");
         }

         if (remappedPreview.Width > 0 && remappedPreview.Height > 0 )
         {
            remappedPreview.Image = mBackend.TextRender.BP_RenderText(text, remappedPreview.Width, remappedPreview.Height, mCurrentPlatform, mPreviewMode);
         }
      }

      private void QueueTextItemsUpdate()
      {
         updateTextListTimer.Start();
      }

      private void OnUpdateTextItemsTimer(object sender, EventArgs e)
      {
         updateTextListTimer.Stop();
         UpdateTextList();
      }

      private void OnTextFilterChanged(object sender, EventArgs e)
      {
         QueueTextItemsUpdate();
      }

      private void filterCheckBoxChanged(object sender, EventArgs e)
      {
         mShowOverridesOnly = showOverrideCheckBox.Checked;
         mMatchCase = matchCaseCheckBox.Checked;
         QueueTextItemsUpdate();
      }

      private void OnSearchSourceChanged(object sender, EventArgs e)
      {
         switch(searchFilterComboBox.Text)
         {
         case "Original Text":
            mStringFilterSource = StringFilterSource.Original;
            break;
         
         case "Remapped Text":
            mStringFilterSource = StringFilterSource.Remapped;
            break;

         case "Remapped (PS3)":
            mStringFilterSource = StringFilterSource.RemappedVita;
            break;

         case "Remapped (Vita)":
            mStringFilterSource = StringFilterSource.RemappedVita;
            break;
         }

         QueueTextItemsUpdate();
      }


      private TextPlatform GetOverridePlatform()
      {
         switch(overridePlatformComboBox.Text)
         {
            case "X360":
               return TextPlatform.X360;

            case "PS3":
               return TextPlatform.PS3;

            case "Vita":
               return TextPlatform.Vita;

            case "KJP Default":
               return TextPlatform.KJP_Default;

            case "KJP X360":
               return TextPlatform.KJP_X360;

            case "KJP PS3":
               return TextPlatform.KJP_PS3;

            case "KJP Vita":
               return TextPlatform.KJP_Vita;
            
            case "Default":
            default:
               return TextPlatform.Default;
         }
      }

      private void OnTextOverride(object sender, EventArgs e)
      {
         TextPlatform platform = GetOverridePlatform();

         foreach (StringEntry entry in GetSelectedEntries())
         {
            if (!mStringDB.HasOverride(entry, platform))
            {
               mStringDB.SetStringOverride(entry, platform, mStringDB.ApplyOriginalCodeOverrides(entry.String));
               QueueRemapSave();
            }
         }

         UpdateUIForCurrentTextItem();
         
         textListView.Refresh();
      }

      private void OnRemoveTextOverride(object sender, EventArgs e)
      {
         TextPlatform platform = GetOverridePlatform();

         foreach (StringEntry entry in GetSelectedEntries())
         {
            mStringDB.RemoveOverride(entry, platform);
            QueueRemapSave();
         }

         UpdateUIForCurrentTextItem();

         textListView.Refresh();
      }

      private void QueueRemapSave()
      {
         // Make sure we can write to the file!
         if (Utils.ConfirmFilesAccess(mRemappingPaths.Values, Utils.CheckFileAccessAction.kAskCheckOut))
         {
            saveRemapTableTimer.Start();
         }
      }

      private void OnSaveRemapTimer(object sender, EventArgs e)
      {
         saveRemapTableTimer.Stop();

         foreach ( var remapping in mRemappingPaths )
         {
            mStringDB.SaveRemapping( remapping.Value, remapping.Key );
         }
      }

      static string ConvertStringForCsv(string input)
      {
         if (input.IndexOfAny("\",\x0A\x0D".ToCharArray()) != -1)
            return '"' + input.Replace("\"", "\"\"") + '"';
         else
            return input;
      }

      private bool FilterEntry(string inputString, string filter)
      {
         string input = inputString;
         
         // Convert line endings to be just "\n"
         input = input.Replace("\r", "");
         
         // Replace newline character plus newline with just a single new line.
         input = input.Replace("|\n", "\n");
         
         // Replace newline character with a new line
         input = input.Replace("|", "\n");
         
         // Replace new lines with spaces
         input = input.Replace('\n', ' ');

         if (input.Contains(filter))
         {
            return true;
         }

         return false;
      }

      private void OnExportFiltered(object sender, EventArgs e)
      {
         List<string> searchTerms = new List<string>();

         try
         {
            using (StreamReader reader = new StreamReader(Environment.ExpandEnvironmentVariables("%BPE_REPOSITORY%/EditorSupport/StringSearchTerms.txt")))
            {
               while (!reader.EndOfStream)
               {
                  searchTerms.Add(reader.ReadLine());
               }
            }
         }
         catch(Exception ex)
         {
            MessageBox.Show(ex.Message, "Error");
         }
         
         List<string> hashesAlreadyWritten = new List<string>();

         StreamWriter writer = new StreamWriter(Environment.ExpandEnvironmentVariables("%BPE_REPOSITORY%/StringExport.csv"), false, Encoding.UTF8);
         writer.WriteLine("Id, Search Term, Location, Original Text, New Text PS3, New Text X360, Ignore");

         List<string> foundHashes = new List<string>();

         foreach (string filter in searchTerms)
         {
            string lowerFilter = filter.ToLower();

            foreach (StringEntry entry in mStringDB.mUnifiedEntries)
            {
               if (!FilterEntry(entry.String.ToLower(), lowerFilter))
                  continue;

               if (foundHashes.Contains(entry.Hash))
                  continue;

               foundHashes.Add(entry.Hash);

               string location = mStringDB.mFiles[entry.Files[0].FileIdx].FileName;
               location = Path.GetFileName(location);

               writer.WriteLine("{0},{1},{2},{3},{4},{5},{6}",
                  ConvertStringForCsv(entry.Hash),
                  filter,
                  ConvertStringForCsv(location),
                  ConvertStringForCsv(entry.String),
                  "",
                  "",
                  "");
            }
         }

         writer.Close();
      }

      private void OnFileMenuOpening(object sender, CancelEventArgs e)
      {
         setFileFilterToolStripMenuItem.Enabled = fileListView.SelectedIndices.Count == 1;
      }

      private void OnSetFileFilter(object sender, EventArgs e)
      {
         if( fileListView.SelectedIndices.Count == 1 )
         {
            int selectedFileIndex = fileListView.SelectedIndices[0];

            StringEntry entry = GetSelectedEntry();

            if (entry != null)
            {
               string filePath = mStringDB.mFiles[entry.Files[selectedFileIndex].FileIdx].FileName;
               fileFilterTextBox.Text = Path.GetFileNameWithoutExtension(filePath);
            }
         }
      }

      private void OnExportAll(object sender, EventArgs e)
      {
         StreamWriter writer = new StreamWriter(Environment.ExpandEnvironmentVariables("%BPE_REPOSITORY%/StringExport_All.csv"), false, Encoding.UTF8);
         writer.WriteLine("Id, Location, Text, PS3/X360 Same, PS3 Text, X360 Text");

         foreach (StringEntry entry in mStringDB.mUnifiedEntries)
         {
            string location = mStringDB.mFiles[entry.Files[0].FileIdx].FileName;

            // Get final ps3 string.
            string ps3Text;
            mStringDB.GetFinalGameString(entry, TextPlatform.PS3, out ps3Text);

            // Get final x360 string.
            string x360Text;
            mStringDB.GetFinalGameString(entry, TextPlatform.X360, out x360Text);

            // If there are no specific x360 or ps3 overrides, clear out the entries.
            if ((ps3Text == x360Text) && (ps3Text == entry.String))
            {
               ps3Text = "";
               x360Text = "";
            }

            // Convert keywords to platform specific keywords to make it easier for people to understand!
            ps3Text = mStringDB.ConvertKeywordsToPS3(ps3Text);
            x360Text = mStringDB.ConvertKeywordsToX360(x360Text);

            // Determine if text for both platforms is identical.
            string platformsTheSame = (ps3Text == x360Text) ? "Y" : "N";

            // If there is no text, we don't need to say that both platforms are the same either!
            if (string.IsNullOrEmpty(ps3Text) && string.IsNullOrEmpty(x360Text))
               platformsTheSame = "";

            writer.WriteLine("{0},{1},{2},{3},{4},{5}", 
               ConvertStringForCsv(entry.Hash), 
               ConvertStringForCsv(location), 
               ConvertStringForCsv(entry.String), 
               platformsTheSame,
               ConvertStringForCsv(ps3Text ?? ""),
               ConvertStringForCsv(x360Text ?? ""));
         }

         writer.Close();
      }

      private void OnExportFilteredByFile(object sender, EventArgs e)
      {
         List<string> searchFiles = new List<string>();

         try
         {
            using (StreamReader reader = new StreamReader(Environment.ExpandEnvironmentVariables("%BPE_REPOSITORY%/EditorSupport/StringSearchFileList.txt")))
            {
               while (!reader.EndOfStream)
               {
                  searchFiles.Add(reader.ReadLine());
               }
            }
         }
         catch (Exception ex)
         {
            MessageBox.Show(ex.Message, "Error");
         }

         StreamWriter writer = new StreamWriter(Environment.ExpandEnvironmentVariables("%BPE_REPOSITORY%/StringExport_FileFilter.csv"), false, Encoding.UTF8);
         writer.WriteLine("Location, Text");

         List<string> foundHashes = new List<string>();

         foreach (string fileFilter in searchFiles)
         {
            string lowerFileFilter = fileFilter.ToLower();

            foreach (StringEntry entry in mStringDB.mUnifiedEntries)
            {
               bool fileFound = false;

               foreach(StringFileRef fileId in entry.Files)
               {
                  string entryFileName = mStringDB.mFiles[fileId.FileIdx].FileName;
                  if (entryFileName.ToLower().Contains(lowerFileFilter))
                  {
                     fileFound = true;
                     break;
                  }
               }

               if (!fileFound)
                  continue;

               if (foundHashes.Contains(entry.Hash))
                  continue;

               foundHashes.Add(entry.Hash);

               writer.WriteLine("{0},{1}", ConvertStringForCsv(fileFilter), ConvertStringForCsv(entry.String));
            }
         }

         writer.Close();
      }

      private void OnPreviewPlatformChanged(object sender, EventArgs e)
      {
         mCurrentPlatform = previewPlatformComboBox.SelectedIndex;
         UpdateRemappedTextPreview();
      }

      private void OnPreviewModeChanged(object sender, EventArgs e)
      {
         mPreviewMode = (TextToolHelper.TextPaneMode)previewModeComboBox.SelectedIndex;
         switch(mPreviewMode)
         {
            case TextToolHelper.TextPaneMode.Default:
               remappedPreview.Dock = DockStyle.Fill;
               break;

            default:
               {
                  int width = -1;
                  int height = -1;

                  mBackend.TextRender.BP_GetPaneSize(mPreviewMode, ref width, ref height);
                  remappedPreview.Dock = DockStyle.None;
                  remappedPreview.Top = 0;
                  remappedPreview.Left = 0;
                  remappedPreview.Width = width;
                  remappedPreview.Height = height;
               }
               break;
         }

         UpdateRemappedTextPreview();

      }

      private void OnOverridePlatformChanged(object sender, EventArgs e)
      {
         UpdateUIForCurrentTextItem();
      }

      private void OnInfoChanged(object sender, EventArgs e)
      {
         StringEntry entry = GetSelectedEntry();
         if (entry != null)
         {
            StringOverride overrideEntry = mStringDB.GetOverrideEntry(entry);
            if( overrideEntry != null )
            {
               if( overrideEntry.Info != infoRichTextBox.Text )
               {
                  overrideEntry.Info = infoRichTextBox.Text;
                  QueueRemapSave();
               }
            }
         }
      }

      private void OnTextSelectionChanged(object sender, ListViewItemSelectionChangedEventArgs e)
      {
         UpdateUIForCurrentTextItem();
      }

      private void OnImportOverrides(object sender, EventArgs e)
      {
         string path = Environment.ExpandEnvironmentVariables("%BPE_REPOSITORY%/EditorSupport/LocalizationImport.xml");

         ImportDialog dialog = new ImportDialog(mStringDB);
         dialog.DoImport(path);
         QueueRemapSave();
      }

      private void OnRemoveKJPOverrides(object sender, EventArgs e)
      {
         mStringDB.RemoveAllKJPOverrides();
         mStringDB.CullEmptyOverrides();
         QueueRemapSave();
      }

      private void OnGenerateCode(object sender, EventArgs e)
      {
         Program.BuildOverrideTable( mStringDB );
      }
   }
}
