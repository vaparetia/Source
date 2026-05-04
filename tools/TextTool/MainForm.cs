using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;

using TextToolHelper;
using System.Xml;
using System.Runtime.InteropServices;
using System.Drawing.Drawing2D;
using System.Diagnostics;
using TextToolData;

namespace TextTool
{
   public partial class MainForm : Form
   {
      string mRootPath;
      Backend mBackend;

      const float kPreviewPaneTextFontSize = 18.0f;

      int mCurrentCode;
      
      XmlElement CurrentStringElement;

      byte[] CurrentStringOriginal
      {
         get
         {
            return GetTextBinaryFromElement(CurrentStringElement);
         }
      }

      private static byte[] GetTextBinaryFromElement(XmlElement element)
      {
         return Convert.FromBase64String(element.SelectSingleNode("./Text-Base64").InnerText);
      }

      public MainForm(string rootPath, Backend backend)
      {
         mRootPath = rootPath;
         mBackend = backend;

         InitializeComponent();

         List<string> matchStrings = new List<string>();
         matchStrings.Add("*.cap");
         matchStrings.Add("*.strings");
         List<string> files = Helper.FileHelper.MatchFiles(mRootPath, matchStrings, true, mRootPath);

         //List<string> files = new List<string>(Directory.GetFiles(Path.Combine(rootPath, "us/demo"), "*.cap", SearchOption.AllDirectories));
         
         files.Sort();

         UpdateFileList(files);
      }

      void LoadCaption(string path)
      {
         XmlDocument doc = new XmlDocument();
         doc.Load(path);

         textStringsListView.BeginUpdate();

         textStringsListView.Items.Clear();

         foreach (XmlElement stringElement in doc.SelectNodes("//Caption"))
         {
            ListViewItem item = new ListViewItem("");
            item.Tag = stringElement;
            textStringsListView.Items.Add(item);
         }

         textStringsListView.EndUpdate();

         if (textStringsListView.Items.Count > 0)
            textStringsListView.Items[0].Selected = true;

         UpdateTextItems(onlyShowProblemStrings);
      }

      void LoadStringTable(string path)
      {
         XmlDocument doc = new XmlDocument();
         doc.Load(path);

         textStringsListView.BeginUpdate();

         textStringsListView.Items.Clear();

         foreach(XmlElement stringElement in doc.SelectNodes("//String"))
         {
            string name = stringElement.SelectSingleNode("Text").InnerText;
            ListViewItem item = new ListViewItem(name);
            item.Tag = stringElement;
            textStringsListView.Items.Add(item);
         }

         textStringsListView.EndUpdate();

         if (textStringsListView.Items.Count > 0)
            textStringsListView.Items[0].Selected = true;

         UpdateTextItems(onlyShowProblemStrings);
      }

      void UpdateFileList(List<string> files)
      {
         fileListView.BeginUpdate();
         fileListView.Items.Clear();

         foreach(string file in files)
         {
            string localPath = file.Substring(mRootPath.Length + 1);

            fileListView.Items.Add(localPath);
         }

         fileListView.EndUpdate();
         
         if (fileListView.Items.Count > 0)
            fileListView.Items[0].Selected = true;
      }

      bool onlyShowProblemStrings = false;

      private byte[] EncodeString(string text)
      {
         List<byte> output = new List<byte>();

         foreach (char c in text)
         {
            output.Add((byte)c);
         }

         return output.ToArray();
      }

      private void OnTextChanged(object sender, EventArgs e)
      {
         // Whenever the text of the unicode preview updated we also update the preview of the character.
         if (CurrentStringElement != null)
         {
            // Update KP Font preview
            {
               int width = kpFontTextPreview.Width & (~1);
               int height = kpFontTextPreview.Height;

               try
               {
                  kpFontTextPreview.Image = mBackend.TextRender.RenderText(CurrentStringOriginal, width, height, true);
                  //kpFontTextPreview.Image = mBackend.TextRender.RenderText(EncodeString(unicodeRichTextBox.Text), width, height, true);
               }
               catch (Exception)
               {
                  kpFontTextPreview.Image = mBackend.TextRender.RenderText(EncodeString("[Error while rendering]"), width, height, true);
               }
            }

            // Update BP Font preview
            {
               int width = bpFontTextPreview.Width;
               int height = bpFontTextPreview.Height;

               try
               {
                  byte[] textUTF8 = mBackend.ConvertStringToUTF8(unicodeRichTextBox.Text);

                  bpFontTextPreview.Image = mBackend.TextRender.BP_RenderText(textUTF8, width, height, 0);
               }
               catch (Exception)
               {
                  bpFontTextPreview.Image = null;
               }
            }
         }
      }

      private void UpdateTextItem(ListViewItem item, bool forceDecodeStrings)
      {
         XmlElement stringElement = item.Tag as XmlElement;

         if (item.Selected || forceDecodeStrings)
         {
            List<int> problemCodes = new List<int>();
            item.Text = mBackend.DecodeString(stringElement, null, null, problemCodes);

            if (onlyShowProblemStrings && problemCodes.Count == 0)
            {
               item.Text = "[Filtered]";
               item.Tag = null;
            }
            else
            {
               if (problemCodes.Count > 0)
                  item.ForeColor = Color.Red;
               else
                  item.ForeColor = SystemColors.WindowText;
            }
         }
         else
         {
            item.Text = stringElement.SelectSingleNode("./Text").InnerText;
         }
      }

      private void UpdateTextItems(bool forceDecodeStrings)
      {
         textStringsListView.BeginUpdate();

         foreach(ListViewItem item in textStringsListView.Items)
         {
            UpdateTextItem(item, forceDecodeStrings);
            
            if (item.Tag == null)
               item.Remove();
         }

         textStringsListView.EndUpdate();
      }

      private void UpdateSelectedTextItems()
      {
         textStringsListView.BeginUpdate();

         foreach(ListViewItem item in textStringsListView.SelectedItems)
         {
            UpdateTextItem(item, false);
         }

         textStringsListView.EndUpdate();

      }

      private void OnTextStringSelected(object sender, ListViewItemSelectionChangedEventArgs e)
      {
         if( textStringsListView.SelectedItems.Count > 0 )
         {
            ListViewItem currentItem = textStringsListView.SelectedItems[0];
            UpdateTextItem(currentItem, false);

            CurrentStringElement = currentItem.Tag as XmlElement;

            List<int> uniqueCodes = new List<int>();
            string text = mBackend.DecodeString(CurrentStringElement, uniqueCodes, null, null);
            SetJapaneseTextInControl(unicodeRichTextBox, text, kPreviewPaneTextFontSize);

            //HACK
            //unicodeRichTextBox.Text = "#{abc,abc}\n#T{abc}\n#T{ABC}";

            uniqueCodes.Sort();

            unknownCharacterListBox.Items.Clear();
            foreach (int code in uniqueCodes)
            {
               ListViewItem item = new ListViewItem("");
               item.Tag = code;
               unknownCharacterListBox.Items.Add(item);
            }

            UpdateCharacterListMappings();
            SelectNextUnknownCharacterMapping();
         }
      }

      private void OnUnknownCharacterSelected(object sender, EventArgs e)
      {
         if( unknownCharacterListBox.SelectedItems.Count > 0 )
         {
            mCurrentCode = (int)unknownCharacterListBox.SelectedItems[0].Tag;
            
            ocrResultListView.Items.Clear();

            UpdateCharacterPreview();

            characterMapInputTextBox.Enabled = true;

            characterMapInputTextBox.Select();
         }
         else
         {
            mCurrentCode = -1;
            characterMapInputTextBox.Enabled = false;
         }
      }

      private void UpdateCharacterPreview()
      {
         try
         {
            Bitmap inImage = mBackend.GenerateCharacterPreviewImage(mCurrentCode);

            Rectangle imageRect = new Rectangle(0, 0, characterPreviewPictureBox.Width, characterPreviewPictureBox.Height);
            Bitmap scaledImage = new Bitmap(characterPreviewPictureBox.Width, characterPreviewPictureBox.Height);

            Graphics gfx = Graphics.FromImage(scaledImage);
            gfx.Clear(Color.Transparent);
            gfx.InterpolationMode = InterpolationMode.NearestNeighbor;
            gfx.PixelOffsetMode = PixelOffsetMode.HighQuality;
            gfx.DrawImage(inImage, imageRect);

            characterPreviewPictureBox.BackgroundImage = scaledImage;
         }
         catch (Exception)
         {
            characterPreviewPictureBox.BackgroundImage = null;
         }

         string character;
         if (mBackend.CharacterMap.LookupCode(mCurrentCode, mBackend.GenerateCharacterPreviewImage(mCurrentCode), out character))
         {
            UpdateCharacterPreviewText(character);
            characterMapInputTextBox.Text = character;
         }
         else
         {
            UpdateCharacterPreviewText("");
            characterMapInputTextBox.Text = "";
         }

         UpdateCharacterUnicodeDisplay();
      }

      private void UpdateCharacterListMappings()
      {
         unknownCharacterListBox.BeginUpdate();

         foreach(ListViewItem item in unknownCharacterListBox.Items)
         {
            item.SubItems.Clear();

            int code = (int)item.Tag;
            item.Text = string.Format("0x{0:x}", code);

            bool isProblem = false;

            string character;
            if (mBackend.CharacterMap.LookupCode(code, mBackend.GenerateCharacterPreviewImage(code), out character))
               item.SubItems.Add(character);
            else
               isProblem = mBackend.IsProblemCode(code);

            if( isProblem )
               item.ForeColor = Color.Red;
            else
               item.ForeColor = SystemColors.WindowText;

         }

         unknownCharacterListBox.EndUpdate();
      }

      private void SelectNextUnknownCharacterMapping()
      {
         unknownCharacterListBox.BeginUpdate();

         int startIndex = 0;
         
         if (unknownCharacterListBox.SelectedIndices.Count > 0)
         {
            startIndex = unknownCharacterListBox.SelectedIndices[0] + 1;
            unknownCharacterListBox.SelectedItems[0].Selected = false;
         }

         for( int i = startIndex; i < unknownCharacterListBox.Items.Count; ++i )
         {
            ListViewItem item = unknownCharacterListBox.Items[i];

            if( item.ForeColor == Color.Red )
            {
               item.Selected = true;
               unknownCharacterListBox.EnsureVisible(item.Index);
               break;
            }
         }

         unknownCharacterListBox.EndUpdate();
      }

      void UpdateCharacterUnicodeDisplay()
      {
         string character = characterMapInputTextBox.Text;
         
         if( character.Length == 1 )
            unicodeDisplay.Text = ((int)character[0]).ToString("x");
         else
            unicodeDisplay.Text = "";
      }

      private void OnUpdateUnknownCharacter(object sender, KeyEventArgs e)
      {
         if (mCurrentCode >= 0)
         {
            bool didChange = false;

            Bitmap currentCharacterBitmap = mBackend.GenerateCharacterPreviewImage(mCurrentCode);

            if (characterMapInputTextBox.Text.Length > 0)
            {
               string mappedCharacter = characterMapInputTextBox.Text;

               didChange = mBackend.CharacterMap.SetCode(mCurrentCode, currentCharacterBitmap, mappedCharacter);
            }
            else
            {
               if (mBackend.CharacterMap.RemoveCode(mCurrentCode, currentCharacterBitmap))
                  didChange = true;
            }

            if (didChange)
            {
               string text = mBackend.DecodeString(CurrentStringElement, null, null, null);
               SetJapaneseTextInControl(unicodeRichTextBox, text, kPreviewPaneTextFontSize);

               UpdateCharacterUnicodeDisplay();
               UpdateCharacterListMappings();
               UpdateSelectedTextItems();
            }
         }
      }

      private void OnAutoSave(object sender, EventArgs e)
      {
         autoSaveTimer.Stop();
         if (mBackend.CharacterMap.NeedsSaving() && Utils.ConfirmFileAccess(mBackend.CharacterMap.GetCharacterMapFilename(), Utils.CheckFileAccessAction.kAskCheckOut))
         {
            mBackend.CharacterMap.AutoSave();
         }
         autoSaveTimer.Start();
      }

      private void OnClosing(object sender, FormClosingEventArgs e)
      {
         if (Utils.ConfirmFileAccess(mBackend.CharacterMap.GetCharacterMapFilename(), Utils.CheckFileAccessAction.kAskCheckOut))
         {
            mBackend.CharacterMap.AutoSave();
         }
      }

      private void OnFileChanged(object sender, EventArgs e)
      {
         RebuildStringList();
      }

      private void RebuildStringList()
      {
         if (fileListView.SelectedItems.Count > 0)
         {
            ListViewItem item = fileListView.SelectedItems[0];

            string path = Path.Combine(mRootPath, item.Text);
            switch (Path.GetExtension(path.ToLower()))
            {
               case ".strings":
                  LoadStringTable(path);
                  break;

               case ".cap":
                  LoadCaption(path);
                  break;
            }
         }
      }

      private void OnUsePreviewCharacter(object sender, EventArgs e)
      {
         string previewCharacter = characterPreviewTextBox.Text;
         characterMapInputTextBox.Text = previewCharacter;
         OnUpdateUnknownCharacter(null, null);
         SelectNextUnknownCharacterMapping();
      }

      private void OnHightlightProblemsChanged(object sender, EventArgs e)
      {
         mBackend.showAnsiiProblem = ansiiCheckBox.Checked;
         mBackend.showHiraganaProblem = hiraganaCheckBox.Checked;
         mBackend.showKatakanaProblem = katakanaCheckBox.Checked;
         mBackend.showSymbolProblem = symbolCheckBox.Checked;
         mBackend.showKanjiProblem = kanjiCheckBox.Checked;

         if (onlyShowProblemStrings != filterProblemStringsCheckBox.Checked)
         {
            onlyShowProblemStrings = filterProblemStringsCheckBox.Checked;
            RebuildStringList();
         }
         else
         {
            UpdateTextItems(true);
         }
      }

      private void OnOCR(object sender, EventArgs e)
      {
         ocrResultListView.Items.Clear();

         Bitmap imageRGBA = mBackend.GenerateCharacterPreviewImage(mCurrentCode);
         Bitmap imageRGB = new Bitmap(imageRGBA.Width, imageRGBA.Height, System.Drawing.Imaging.PixelFormat.Format24bppRgb);
         using( Graphics graphics = Graphics.FromImage(imageRGB) )
         {
            graphics.DrawImage(imageRGBA, 0, 0);
         }

         string tempPath = Environment.ExpandEnvironmentVariables("%TEMP%/OCR.bmp");
         imageRGB.Save(tempPath, System.Drawing.Imaging.ImageFormat.Bmp);

         List<string> matches;
         OCRHelper.GetCharacterMatches(tempPath, out matches);

         foreach( string match in matches )
            ocrResultListView.Items.Add(new ListViewItem(match));

         if (ocrResultListView.Items.Count > 0)
         {
            ocrResultListView.Items[0].Selected = true;
         }
      }

      private void OnOcrItemSelected(object sender, EventArgs e)
      {
         if( ocrResultListView.SelectedItems.Count > 0)
         {
            UpdateCharacterPreviewText(ocrResultListView.SelectedItems[0].Text);
         }
      }

      private void UpdateCharacterPreviewText(string text)
      {
         characterPreviewTextBox.Font = new Font("DFHSGothic-W5", 72.0f);
         characterPreviewTextBox.Text = text;
      }

      private void SetJapaneseTextInControl(RichTextBox ctrl, string text, float fontSize)
      {
         ctrl.Text = text;
         ctrl.SelectAll();
         ctrl.SelectionFont = new Font("DFHSGothic-W5", fontSize);
         ctrl.SelectionLength = 0;
      }

      private void OnUpdatedBPFont(object sender, EventArgs e)
      {
         mBackend.CharacterMap.AutoSave();

         // Make sure we can write to the file before trying to cook it
         if (!Utils.ConfirmFileAccess(TextToolData.Backend.GetBPFontPath(), Utils.CheckFileAccessAction.kAskCheckOut))
         {
            // Can't write to file
            return;
         }

         ProcessStartInfo info = new ProcessStartInfo("fontgen");
         info.CreateNoWindow = true;
         info.UseShellExecute = false;
         Process proc = Process.Start(info);
         proc.WaitForExit();

         mBackend.LoadBPFont();
         OnTextStringSelected(null, null);
      }

      private void GetFilesForRegion(string region, out List<string> files)
      {
         files = new List<string>();
         files.AddRange(Directory.GetFiles(Path.Combine(mRootPath, region), "*.cap", SearchOption.AllDirectories));
         files.AddRange(Directory.GetFiles(Path.Combine(mRootPath, string.Format("assets\\gcx\\{0}", region)), "*.strings", SearchOption.AllDirectories));
         files.AddRange(Directory.GetFiles(Path.Combine(mRootPath, string.Format("{0}\\codec", region)), "*.strings", SearchOption.AllDirectories));
         files.Sort();
      }
   }
}
