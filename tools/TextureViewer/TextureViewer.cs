using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Xml;
using System.Diagnostics;
using Tools.Common.Perforce;

namespace TextureViewer
{
   public partial class TextureViewer : Form
   {
      string sProgressText;

      Tools.Common.Perforce.Connection m_P4Connection;

      class TextureEntry
      {
         public TextureEntry(string filename)
         {
            mFilename = filename;
            mTags = new List<string>();
            mStringsThatMapToMe = new List<string>();
            mImagesILinkTo = new List<string>();
            bNeedsSaving = false;

         }
         
         public string   mFilename;
         public string   mFileHash;
         public List<String> mTags;
         public string mTagString;
         public bool bNeedsSaving;
         public string mXmlMetaFilePath;
         public List<String> mStringsThatMapToMe;
         public List<String> mImagesILinkTo;
         public string mAddtionalTextString;
      }

      TextureEntry   ttCurrentSelection;
      PictureBox rightClickedPictureBox;
      string strSlotOrFace;

      bool m_bMGS3;
      bool m_bCopy;
      bool m_bClearFlatList;
      bool m_bClearCtxr;
      bool m_bMakeXmlMeta;
      bool m_bOnlyMakeList;

      class TextureEntryList
      {
         public TextureEntryList()
         {
            mAllTextures = new List<TextureEntry>();
         }
         
         public void CalculateUniqueTextures()
         {
            // Create hash of file, and add unique hashes to dictionary with all matching entries
            mFileHashToTextureEntries = new Dictionary<string, List<TextureEntry>>();
            foreach(TextureEntry textureEntry in mAllTextures)
            {
               string path = textureEntry.mFilename;
               textureEntry.mFileHash = GetMD5HashFromFile(path);
               List<TextureEntry> textureEntryList;
               if (!mFileHashToTextureEntries.TryGetValue(textureEntry.mFileHash, out textureEntryList))
               {
                  textureEntryList = new List<TextureEntry>();
                  mFileHashToTextureEntries.Add(textureEntry.mFileHash, textureEntryList);
               }
               textureEntryList.Add(textureEntry);               
            }
         }
         
         public void CalculateUniqueTextures_Cached()
         {
            if (mFileHashToTextureEntries == null)
            {
               CalculateUniqueTextures();
            }
         }

         public List<TextureEntry> GetUniqueTextureList()
         {
            List<TextureEntry> uniqueTextures = new List<TextureEntry>();
         
            if (mFileHashToTextureEntries == null)
            {
               CalculateUniqueTextures_Cached();
            }
            
            foreach(string filehash in mFileHashToTextureEntries.Keys)
            {
               uniqueTextures.Add(mFileHashToTextureEntries[filehash][0]);      
            }
            return uniqueTextures;
         }
         
         public List<TextureEntry>   mAllTextures;         
         Dictionary<string, List<TextureEntry>> mFileHashToTextureEntries;
      }

      Dictionary<string, TextureEntryList> mAssetHashToFileMapping = new Dictionary<string, TextureEntryList>();


      public class MultiMap<K, V>
      {
         // 1
         Dictionary<K, List<V>> _dictionary = new Dictionary<K, List<V>>();

         // 2
         public void Add(K key, V value)
         {
            List<V> list;
            if (this._dictionary.TryGetValue(key, out list))
            {
               // 2A.
               if (!list.Contains(value))
               {
                  list.Add(value);
               }
            }
            else
            {
               // 2B.
               list = new List<V>();
               list.Add(value);
               this._dictionary[key] = list;
            }
         }

         // 3
         public IEnumerable<K> Keys
         {
            get
            {
               return this._dictionary.Keys;
            }
         }

         // 4
         public List<V> this[K key]
         {
            get
            {
               List<V> list;
               if (this._dictionary.TryGetValue(key, out list))
               {
                  return list;
               }
               else
               {
                  return new List<V>();
               }
            }
         }
      }
      public MultiMap<uint, string> mNameMappings = new MultiMap<uint, string>();

      public void GetNameMappings()
      {
         string rootPath = Environment.ExpandEnvironmentVariables("%BPE_REPOSITORY%");
         string texturelookup = rootPath + "\\EditorSupport\\TextureLookup.txt";
         if (!File.Exists(texturelookup))
         {
            StreamWriter sw = File.CreateText(texturelookup);
            string[] listFiles = Directory.GetFiles("K:", "*.lst", SearchOption.AllDirectories);
            List<string> lines = new List<string>();
            foreach (string listFile in listFiles)
            {
               StreamReader fs = File.OpenText(listFile);
               while (!fs.EndOfStream)
               {
                  String line = fs.ReadLine();
                  string[] tokens = line.Split(new char[] { ' ', '\t', '(', ')' }, StringSplitOptions.RemoveEmptyEntries);
                  if (tokens.Length == 3 && tokens[2].ToLower().StartsWith("0x"))
                  {
                     uint key = 0;
                     if (UInt32.TryParse(tokens[1], out key))
                     {
                        mNameMappings.Add(key, tokens[0]);
                     }
                  }
               }
            }
            foreach (uint k in mNameMappings.Keys)
            {
               List<String> vals = mNameMappings[k];
               foreach (String val in vals)
               {
                  sw.WriteLine("{0} {1}", k, val);
               }
            }
         }
         else
         {
            StreamReader sr = File.OpenText(texturelookup);
            while (!sr.EndOfStream)
            {
               String line = sr.ReadLine();
               string[] tokens = line.Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);
               if (tokens.Length == 2)
               {
                  uint key = 0;
                  if (UInt32.TryParse(tokens[0], out key))
                  {
                     mNameMappings.Add(key, tokens[1]);
                  }
               }
            }
         }
      }

      public String GetNameMapping(String id)
      {
         String name = id;
         uint key = 0;
         if (UInt32.TryParse(id, System.Globalization.NumberStyles.HexNumber, null, out key))
         {
            if (mNameMappings[key].Count > 0)
            {
               name = mNameMappings[key][0];
            }
         }
         return name;
      }
   
      public TextureViewer()
      {
         InitializeComponent();
         GetNameMappings();

         /*
         m_P4Connection = new Tools.Common.Perforce.Connection();
//         String overridePort = System.Environment.GetEnvironmentVariable("BPE_DATA_P4PORT");
         String overridePort = "bp-server2:1701";
         if (overridePort != String.Empty)
         {
            m_P4Connection.Port = overridePort;
         }
         try
         {
            if( !m_P4Connection.Connect() )
            {
               MessageBox.Show("Could not connect to Perforce server. Tried " + m_P4Connection.Port, "P4 Connection Fail", MessageBoxButtons.OK, MessageBoxIcon.Error);	
            }
         }
         catch (System.Exception ex)
         {
            MessageBox.Show(ex.Message, "P4 Connection Fail", MessageBoxButtons.OK, MessageBoxIcon.Error);	
         }
         */

         parseFileWorker.RunWorkerAsync();
      }

      static string GetMD5HashFromFile(string fileName)
      {
         FileStream file = new FileStream(fileName, FileMode.Open, FileAccess.Read);
         System.Security.Cryptography.MD5 md5 = new System.Security.Cryptography.MD5CryptoServiceProvider();
         byte[] retVal = md5.ComputeHash(file);
         file.Close();

         StringBuilder sb = new StringBuilder();
         for (int i = 0; i < retVal.Length; i++)
         {
            sb.Append(retVal[i].ToString("x2"));
         }
         return sb.ToString();
      }


      private void OnParseFiles(object sender, DoWorkEventArgs e)
      {
         string rootPath = Environment.ExpandEnvironmentVariables("%BPE_REPOSITORY%");
         string flatListPath = "\\textures\\flatlist";

         if( !Directory.Exists( rootPath + flatListPath ))
         {
            Directory.CreateDirectory(rootPath + flatListPath);
         }
         
         string[] toProcess = Directory.GetFiles(rootPath + flatListPath, "*.tga", SearchOption.AllDirectories);

         for (int i = 0; i < toProcess.Length; ++i)
         {
            string texturePath = toProcess[i];
            TextureEntry textureEntry = new TextureEntry(texturePath);

            string id = Path.GetFileNameWithoutExtension(texturePath).ToLower();

            // try to convert it into a bitmap name
            id = GetNameMapping(id);

            TextureEntryList textureEntryList;
            if( !mAssetHashToFileMapping.TryGetValue(id, out textureEntryList))
            {
               textureEntryList = new TextureEntryList();
               mAssetHashToFileMapping.Add(id, textureEntryList);
            }

            XmlDocument metaData = new XmlDocument();
            string xmlPath = texturePath.Replace(".tga", ".xmlmeta");
            try
            {
               metaData.Load(xmlPath);

               XmlNodeList tagNodes = metaData.SelectNodes("TextureMetadata/TagList/*");
               foreach (XmlNode node in tagNodes)
               {
                  textureEntry.mTags.Add(node.InnerText);
                  textureEntry.mTagString += (node.InnerText) + ",";
               }

               XmlNodeList mappedNodes = metaData.SelectNodes("TextureMetadata/MappedPaths/*");
               foreach (XmlNode node in mappedNodes)
               {
                  textureEntry.mStringsThatMapToMe.Add(node.InnerText);
               }

               XmlNodeList linkedImages = metaData.SelectNodes("TextureMetadata/LinkedImages/*");
               foreach (XmlNode node in linkedImages)
               {
                  textureEntry.mImagesILinkTo.Add(node.InnerText);
               }

               XmlNodeList additionalText = metaData.SelectNodes("AdditionalText/Text");
               foreach (XmlNode node in additionalText)
               {
                  textureEntry.mAddtionalTextString = node.InnerText;
               }

               textureEntry.mXmlMetaFilePath = xmlPath;
            }
            catch (System.Exception ex)
            {
            	
            }
            textureEntryList.mAllTextures.Add(textureEntry);

            int iProgress = (int)(((float)i / (float)toProcess.Length) * 100.0f);
            parseFileWorker.ReportProgress(iProgress);
            sProgressText = "Parsing " + toProcess[i];
         }

         parseFileWorker.ReportProgress(100);

         this.Invoke(new MethodInvoker(delegate()
            {
               listView1.BeginUpdate();

               foreach (string fileName in mAssetHashToFileMapping.Keys)
               {
                  if( mAssetHashToFileMapping[fileName].mAllTextures.Count > 0 )
                     listView1.Items.Add(fileName);
               }

               listView1.AutoResizeColumns(ColumnHeaderAutoResizeStyle.ColumnContent);

               listView1.EndUpdate();
            }
            ));
      }

      private void OnParseProgressUpdate(object sender, ProgressChangedEventArgs e)
      {
         toolStripProgressBar1.Value = e.ProgressPercentage;
         toolStripStatusLabel1.Text = sProgressText;
      }

      private void OnParseCompleted(object sender, RunWorkerCompletedEventArgs e)
      {
         toolStripProgressBar1.Visible = false;
         toolStripStatusLabel1.Visible = false;
      }

      private void OnClosing(object sender, FormClosingEventArgs e)
      {
         if( ttCurrentSelection != null &&
            ttCurrentSelection.bNeedsSaving )
         {
            SaveCurrentTextureSelection();
         }

         parseFileWorker.CancelAsync();
         flatListMaker.CancelAsync();

         while(parseFileWorker.IsBusy || flatListMaker.IsBusy )
         {
            Application.DoEvents();
         }
      }

      private int CountColors(Image img)
      {
         Dictionary<int, int> uniqueColors = new Dictionary<int, int>();
         Bitmap bmp = new Bitmap(img);
         unsafe
         {
            System.Drawing.Imaging.BitmapData db = bmp.LockBits(new Rectangle(0, 0, img.Width, img.Height), System.Drawing.Imaging.ImageLockMode.ReadOnly, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
            for (int y = 0; y < db.Height; ++y)
            for (int x = 0; x < db.Width; ++x)
            {
               int *b = (int *) db.Scan0.ToPointer();
               int c = *(b + y*db.Stride/4 + x);
               if (!uniqueColors.ContainsKey(c))
               {
                  uniqueColors.Add(c, 1);
               }
            }
            bmp.UnlockBits(db);
         }
         return uniqueColors.Count;
      }

      private void FillInformationBox()
      {
         infoTextBox.Text = "";
         infoTextBox.Text += ttCurrentSelection.mFilename +"\n";
         infoTextBox.Text += "MD5 Hash: " + ttCurrentSelection.mFileHash + "\n";
         if (ttCurrentSelection.mImagesILinkTo.Count > 0)
         {
            infoTextBox.Text += "Images I'm linked to: \n";
            foreach (string s in ttCurrentSelection.mImagesILinkTo)
            {
               infoTextBox.Text += " * " + s + "\n";
            }
         }
         if( ttCurrentSelection.mStringsThatMapToMe.Count > 0)
         {
            infoTextBox.Text += "Strings that map to me: \n";
            foreach( string s in ttCurrentSelection.mStringsThatMapToMe )
            {
               infoTextBox.Text += " * " + s + "\n";
            }
         }
         if (ttCurrentSelection.mAddtionalTextString != "" && ttCurrentSelection.mAddtionalTextString != null )
         {
            infoTextBox.Text += "Additional Text: " + ttCurrentSelection.mAddtionalTextString + "\n";
         }

         txt_additionalText.Text = ttCurrentSelection.mAddtionalTextString;

      }

      private void SetFocusOnNewPicture( string id )
      {
         TextureEntryList textureEntryList = mAssetHashToFileMapping[id];
         List<TextureEntry> textures = textureEntryList.GetUniqueTextureList();
         if (ttCurrentSelection != null &&
             ttCurrentSelection != textures[0] &&
             ttCurrentSelection.bNeedsSaving)
         {
            //try and save
            SaveCurrentTextureSelection();
         }

         PictureBox pic = null;
         //iSwitchNum is an indicator of which picture in the tray we're going to swap with. 
         int iSwitchNum = -1;
         foreach( Control cc in flowLayoutPanel1.Controls )
         {
            if( (string) ((PictureBox)cc).Tag == id )
            {
               iSwitchNum = flowLayoutPanel1.Controls.GetChildIndex(cc);
               pic = (PictureBox)cc;
               break;
            }
               
         }

         ttCurrentSelection = textures[0];

         Image largest = null;
         foreach (TextureEntry textureEntry in textures)
         {
            string path = textureEntry.mFilename;
            if (iSwitchNum == -1)
            {
               pic = new PictureBox();
               pic.Size = new Size(128, 128); 
               pic.SizeMode = PictureBoxSizeMode.Zoom;
               pic.BackColor = Color.Black;
               pic.Image = Paloma.TargaImage.LoadTargaImage(path);
               pic.Tag = id;
               toolTip1.SetToolTip(pic, path);
               pic.Click += new EventHandler(pic_Click);
               pic.DoubleClick += new EventHandler(pic_DoubleClick);
               pic.MouseDown += new MouseEventHandler(pic_MouseDown);
               pic.ContextMenuStrip = rclickMenu;

               //we can safely remove the last picture here, since we are adding one.
               if( flowLayoutPanel1.Controls.Count >= 10)
               {
                  flowLayoutPanel1.Controls.RemoveAt(9);
               }
               flowLayoutPanel1.Controls.Add(pic);
               flowLayoutPanel1.Controls.SetChildIndex(pic, 0);
            }

            pic.BorderStyle = BorderStyle.Fixed3D;
            pic.Size = new Size(128, 128);

            FillInformationBox();

            /*
            selectedHighightBox.Size = new Size(128, 128);
            Point drawPoint = flowLayoutPanel1.Location;
            drawPoint.X += pic.Location.X;
            drawPoint.Y += pic.Location.Y;
            selectedHighightBox.Location = drawPoint;
            Color highlightColor = Color.FromArgb(32, 0, 0, 64);
            selectedHighightBox.BackColor = highlightColor;
            */

            // select the largest/most colorful
            if (largest == null)
            {
               largest = pic.Image;
            }
            else
            {
               int largestSize = largest.Width * largest.Height;
               int picSize = pic.Image.Width * pic.Image.Height;
               if (picSize > largestSize)
               {
                  largest = pic.Image;
               }
               else if (picSize == largestSize && CountColors(pic.Image) > CountColors(largest))
               {
                  largest = pic.Image;
               }
            }
            tagListBox.Items.Clear();
            tagListBox.BeginUpdate();

            foreach (string strTag in textureEntry.mTags)
            {
               tagListBox.Items.Add(strTag);
            }
            tagListBox.EndUpdate();


         }
         SetImage(pictureBox1, largest);
      }
      private void OnItemSelected(object sender, EventArgs e)
      {
         AdjustFlowLayoutPanel();
         pictureBox1.Image = null;
        
         if (listView1.SelectedItems.Count == 0)
            return;

         SetFocusOnNewPicture(listView1.SelectedItems[0].Text);
      }

      private void SetImage(PictureBox destBox, Image img)
      {
         if (img == null)
         {
            destBox.Image = null;
            return;
         }
         try
         {
            // create a new bitmap with the proper dimensions
            double scaleToFitX = ((double)destBox.Width) / img.Width;
            double scaleToFitY = ((double)destBox.Height) / img.Height;
            double scaleToFit = 1.0;
            Size newSize;
            if (scaleToFitX < scaleToFitY)
            {
               // use x
               newSize = new Size(destBox.Width, (int)(img.Height * scaleToFitX));
               scaleToFit = scaleToFitX;
            }
            else
            {
               // use y
               newSize = new Size((int)(img.Width * scaleToFitY), destBox.Height);
               scaleToFit = scaleToFitY;
            }
            Bitmap scaledImage = new Bitmap(newSize.Width, newSize.Height);

            // resize the image
            Graphics gfx = Graphics.FromImage(scaledImage);
            gfx.Clear(Color.Transparent);
            gfx.InterpolationMode = (scaleToFit < 1.0 ? System.Drawing.Drawing2D.InterpolationMode.HighQualityBicubic : System.Drawing.Drawing2D.InterpolationMode.NearestNeighbor);
            gfx.PixelOffsetMode = System.Drawing.Drawing2D.PixelOffsetMode.HighQuality;
            gfx.DrawImage(img, 0, 0, newSize.Width, newSize.Height);

            // replace it
            destBox.Image = scaledImage;
         }
         catch (System.Exception e)
         {
            MessageBox.Show(e.Message);
         }
      }

      void pic_MouseDown(object sender, EventArgs e)
      {
         PictureBox me = (PictureBox)sender;
         if (me != null)
         {
            rclickMenu.Items[0].Text = "Link " + (string)me.Tag + " with current image";
            rightClickedPictureBox = me;
         }
      }
      
      void pic_Click(object sender, EventArgs e)
      {
         AdjustFlowLayoutPanel();
         SetFocusOnNewPicture((string)((PictureBox)sender).Tag);
      }

      void pic_DoubleClick(object sender, EventArgs e)
      {
         PictureBox pic = (PictureBox)sender;
         TextureEntryList textureEntryList = mAssetHashToFileMapping[(string)pic.Tag];
         List<TextureEntry> textures = textureEntryList.GetUniqueTextureList();
         string path = textures[0].mFilename;
         System.Diagnostics.Process.Start(path);
      }

      private void calculateStatsToolStripMenuItem_Click(object sender, EventArgs e)
      {
         // Calculate hashes for all files
         toolStripProgressBar1.Visible = true;
         toolStripProgressBar1.ProgressBar.Value = 0;
         toolStripStatusLabel1.Visible = true;
         
         foreach (ListViewItem listViewItem in listView1.Items)
         {
            // Testing
            float ratioCompleted = ((float) listViewItem.Index) / ((float) listView1.Items.Count);
            
#if false
            // Testing
            if (ratioCompleted > 0.1)
            {
               break;            
            }
#endif            
            
            if ((listViewItem.Index % 10) == 0)
            {
               toolStripStatusLabel1.Text = String.Format("Calculating Hashes ({0}%)", (int) (ratioCompleted * 100));               
               toolStripProgressBar1.ProgressBar.Increment(1);
               this.Refresh();
            }
            
            string assetHash = listViewItem.Text;
            TextureEntryList textureEntryList = mAssetHashToFileMapping[assetHash];
            textureEntryList.CalculateUniqueTextures_Cached();
            if (textureEntryList.GetUniqueTextureList().Count > 1)
            {
               listViewItem.ForeColor = Color.Red;
            }            
         }
         
         toolStripProgressBar1.Visible = false;
         toolStripStatusLabel1.Visible = false;
         
      }

      private void searchBox_TextChanged(object sender, EventArgs e)
      {
         listView1.Items.Clear();
         listView1.BeginUpdate();

         foreach (string fileName in mAssetHashToFileMapping.Keys)
         {
            if (mAssetHashToFileMapping[fileName].mAllTextures.Count > 0)
            {
               if (fileName.Contains(searchBox.Text))
               {
                  listView1.Items.Add(fileName);
               }
               else if (mAssetHashToFileMapping[fileName].mAllTextures[0].mTagString != null &&
                        mAssetHashToFileMapping[fileName].mAllTextures[0].mTagString.Contains(searchBox.Text))
               {
                  listView1.Items.Add(fileName);
               }
            }
         }

         listView1.AutoResizeColumns(ColumnHeaderAutoResizeStyle.ColumnContent);

         listView1.EndUpdate();
      }

      private void createFlatListToolStripMenuItem_Click(object sender, EventArgs e)
      {
         FlatListMakerDialog form_makeList = new FlatListMakerDialog();
         form_makeList.mainForm = this;
         form_makeList.Show();

         /*
         toolStripProgressBar1.Visible = true;
         toolStripStatusLabel1.Visible = true;
         toolStripStatusLabel1.Text = "Creating flat list of textures.";
         flatListMaker.RunWorkerAsync();
         */
      }

      private void OnMakeFlatListUpdate(object sender, ProgressChangedEventArgs e)
      {
         toolStripProgressBar1.Value = e.ProgressPercentage;
         toolStripStatusLabel1.Text = sProgressText;
      }

      private void OnMakeFlatListCompleted(object sender, RunWorkerCompletedEventArgs e)
      {
         toolStripProgressBar1.Visible = false;
         toolStripStatusLabel1.Visible = false;

         parseFileWorker.RunWorkerAsync();
      }

      public void CommenceFlatList(bool bMGS3, bool bCopy, bool bClearFlatlistFirst, bool bClearCTXRs, bool bMakeXmlMeta, bool bOnlyMakeList )
      {
         m_bMGS3 = bMGS3;
         m_bCopy = bCopy;
         m_bClearFlatList = bClearFlatlistFirst;
         m_bClearCtxr = bClearCTXRs;
         m_bMakeXmlMeta = bMakeXmlMeta;
         m_bOnlyMakeList = bOnlyMakeList;

         if( bMGS3 )
         {
            strSlotOrFace = "\\slot";
         }
         else
         {
            strSlotOrFace = "\\face";
         }

         toolStripProgressBar1.Visible = true;
         toolStripStatusLabel1.Visible = true;
         flatListMaker.RunWorkerAsync();
      }

      private void OnMakeFlatList(object sender, DoWorkEventArgs e)
      {
         System.DateTime startTime = System.DateTime.Now;

         string flatListPath = "\\textures\\flatlist";
         string rootPath = Environment.ExpandEnvironmentVariables("%BPE_REPOSITORY%");

         if( m_bClearFlatList )
         {
            string[] strDelete = Directory.GetFiles(rootPath + flatListPath, "*.tga", SearchOption.TopDirectoryOnly);
            foreach ( string str in strDelete )
            {
               File.Delete(str);
               sProgressText = "Removing " + str;
               flatListMaker.ReportProgress(0);
            }

            strDelete = Directory.GetFiles(rootPath + flatListPath, "*.xmlmeta", SearchOption.TopDirectoryOnly);
            foreach (string str in strDelete)
            {
               File.Delete(str);
               sProgressText = "Removing " + str;
               flatListMaker.ReportProgress(0);
            }

            if( m_bClearCtxr )
            {
               sProgressText = "Removing _win cooked files";
               flatListMaker.ReportProgress(0);
               Directory.Delete(rootPath + flatListPath + "/_win");

               sProgressText = "Removing _360 cooked files";
               flatListMaker.ReportProgress(0);
               Directory.Delete(rootPath + flatListPath + "/_360");

               sProgressText = "Removing _ps3 cooked files";
               flatListMaker.ReportProgress(0);
               Directory.Delete(rootPath + flatListPath + "/_ps3");
            }

         }



         flatListMaker.ReportProgress(0);
         sProgressText = "Getting all " + strSlotOrFace + " files...";
         string[] slotProcess = Directory.GetFiles(rootPath + strSlotOrFace, "*.tga", SearchOption.AllDirectories);
         flatListMaker.ReportProgress(0);
         sProgressText = "Getting all \\stage files...";
         string[] stageProcess = Directory.GetFiles(rootPath + "\\stage", "*.tga", SearchOption.AllDirectories);
         flatListMaker.ReportProgress(0);

         string[] toProcess = new string[slotProcess.Length + stageProcess.Length];
         int iSlotStageCount = 0;

         for (int t = 0; t < slotProcess.Length; t++)
         {
            toProcess[iSlotStageCount] = slotProcess[t];
            iSlotStageCount++;
         }
         for (int t = 0; t < stageProcess.Length; t++)
         {
            toProcess[iSlotStageCount] = stageProcess[t];
            iSlotStageCount++;
         }

         SortedDictionary<string, string> OldPathToNewPath = new SortedDictionary<string, string>();
         Dictionary<string, string> FileMd5ToNewPath = new Dictionary<string, string>();
         Dictionary<string, TextureEntry> FriendlyNameBasedTextureEntryList = new Dictionary<string, TextureEntry>();
         int iDebugTextureEntriesCreated = 0;

         for (int i = 0; i < toProcess.Length; ++i)
//         for (int i = 0; i < 1000; ++i)
         {
            //texture path is going to go into the metadata somewhere. 
            string texturePath = toProcess[i];
            string id = Path.GetFileNameWithoutExtension(texturePath).ToLower();

            // try to convert it into a bitmap name
            string mappedId = GetNameMapping(id);

           
            if (!Directory.Exists(rootPath + flatListPath))
               Directory.CreateDirectory(rootPath + flatListPath);

            //load the image up
            string tgaHashed = GetMD5HashFromFile(texturePath);
            string friendlyName;
 
            //jshep: organizing textures further. Compare the file MD5s, not just the names of the files.
            if (FileMd5ToNewPath.ContainsKey(tgaHashed))
            {
               //the file is not unique. This very file is somewhere else in the original slot/stage collection
               //but with a different name. So we map the slot/stage name here to the new friendlyPath name.
               
               FileMd5ToNewPath.TryGetValue( tgaHashed, out friendlyName);
               OldPathToNewPath.Add( texturePath, friendlyName);
            }
            else
            {
               //this file is (so far) unique. So we save it into the flatlist, and map the old path
               //to the new friendly path.
               friendlyName = GetNameMapping(id);

               string strOldName = rootPath + flatListPath + "\\" + friendlyName + ".tga";

               //save the file in the flatlist
               if (!File.Exists(strOldName) )
               {
                  if (!m_bOnlyMakeList)
                  {
                     try
                     {
                        if (m_bCopy)
                           File.Copy(texturePath, strOldName);
                        else
                           File.Move(texturePath, strOldName);
                     }
                     catch (System.Exception ex)
                     {
                     	
                     }

                  }
               }
               else
               {
                  if( tgaHashed != GetMD5HashFromFile( strOldName ) )
                  {
                     //the file does exist already. hmm.
                     Debug.WriteLine("File " + texturePath + " has the same friendly name (" + friendlyName + ") as a file that is already in the flatlist, but a different MD5 hash.");
                     friendlyName.Replace(".bmp", "");
                     friendlyName += "_" + tgaHashed;
                     Debug.WriteLine("So it will be copied over as " + friendlyName + " and " + texturePath + " will point to that.");
                     Debug.WriteLine("--------------------");
                     if (!m_bOnlyMakeList)
                     {
                        try
                        {
                           if (m_bCopy)
                              File.Copy(texturePath, rootPath + flatListPath + "\\" + friendlyName + ".tga");
                           else
                              File.Move(texturePath, rootPath + flatListPath + "\\" + friendlyName + ".tga");
                        }
                        catch (System.Exception ex)
                        {

                        }
                     }
                  }
                  else
                  {
                     Debug.WriteLine("File " + texturePath + " already exists.");
                  }

               }

               //save the friendly name to the list-- with hash appended if need be.
               FileMd5ToNewPath.Add(tgaHashed, friendlyName);

               //tie the old path to the new path
               OldPathToNewPath.Add( texturePath, friendlyName);
               }
            TextureEntry tEntry;
            FriendlyNameBasedTextureEntryList.TryGetValue(friendlyName, out tEntry);
            if (tEntry == null)
            {
               tEntry = new TextureEntry(friendlyName);
               FriendlyNameBasedTextureEntryList.Add(friendlyName, tEntry);
               iDebugTextureEntriesCreated++;
            }

            //save the path that maps to this image.
            string strFullOldPath = texturePath;
            int iSplitLoc = strFullOldPath.LastIndexOf(strSlotOrFace);
            if (iSplitLoc == -1)
               iSplitLoc = strFullOldPath.LastIndexOf("\\stage");

            string splitKey = strFullOldPath.Substring(iSplitLoc);
            splitKey = splitKey.Replace("\\", "/");
            splitKey = splitKey.Replace(".tga", ".ctxr");
            splitKey = splitKey.Substring(1);

            tEntry.mStringsThatMapToMe.Add(splitKey);

            int iProgress = (int)(((float)i / (float)toProcess.Length) * 100.0f);
            sProgressText = "Copying to flatlist: " + texturePath;
            flatListMaker.ReportProgress(iProgress);
         }


         int iXmlCount = 0;
         foreach (TextureEntry tt in FriendlyNameBasedTextureEntryList.Values)
         {
            //create a metadata file if there isn't one.
            if (!File.Exists(rootPath + flatListPath + "\\" + tt.mFilename + ".xmlmeta"))
            {
               SaveTextureEntryMetadata(tt, rootPath + flatListPath);
               sProgressText = "Generating mapping header";
               int iProgress = (int)(((float)iXmlCount / (float)FriendlyNameBasedTextureEntryList.Values.Count) * 100.0f);
               sProgressText = "Writing metadata for : " + tt.mFilename;
               flatListMaker.ReportProgress(iProgress);
            }
         }


         //write out some include files!
         sProgressText = "Generating mapping header";
         flatListMaker.ReportProgress(99);

         Debug.WriteLine(iDebugTextureEntriesCreated + " texture entries created. " + OldPathToNewPath.Count() + " mappings exist from " + OldPathToNewPath.Keys.Count + " old paths to " + OldPathToNewPath.Values.Count + " new paths.");
         Debug.WriteLine(toProcess.Length + " slot/stage references were considered.");

         FileStream includeFS = new FileStream(rootPath + "\\BP_FlatlistTextureMapping.txt", FileMode.Create);
         StreamWriter writeFS = new StreamWriter(includeFS);


         writeFS.Write( OldPathToNewPath.Count + "\0" );
         foreach (KeyValuePair<string, string> keyCount in OldPathToNewPath)
         {
            string strFullOldPath = keyCount.Key;
            int iSplitLoc = strFullOldPath.LastIndexOf(strSlotOrFace);
            if (iSplitLoc == -1)
               iSplitLoc = strFullOldPath.LastIndexOf("\\stage");

            string splitKey = strFullOldPath.Substring(iSplitLoc);
            splitKey = splitKey.Replace("\\", "/");
            splitKey = splitKey.Replace(".tga", ".ctxr");
            splitKey = splitKey.Substring(1);

            string strValue = keyCount.Value;
            strValue = strValue.Replace("\\", "/");
            writeFS.Write( splitKey + "\0" + strValue + "\0");
         }

         writeFS.Close();
         includeFS.Close();

         flatListMaker.ReportProgress(100);

      }

      private void AcceptNewAdditionalText(object sender, KeyEventArgs e)
      {
         if (e.KeyCode == Keys.Enter || e.KeyCode == Keys.Return)
         {
            if( ttCurrentSelection == null )
            {
               MessageBox.Show("You must select an image to add Additional Text to.", "!", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
            }
            else
            {
               ttCurrentSelection.mAddtionalTextString = txt_additionalText.Text;
            }

            FillInformationBox();
         }
      }

      private void AcceptNewTag(object sender, KeyEventArgs e)
      {
         if( e.KeyCode == Keys.Enter || e.KeyCode == Keys.Return )
         {

            string strNewTag = txtNewTagBox.Text;
            strNewTag = strNewTag.Replace(" ","");

            if (strNewTag == "")
               return;

            string id = listView1.SelectedItems[0].Text;
            if (ttCurrentSelection == null)
            {
               TextureEntryList textureEntryList = mAssetHashToFileMapping[id];
               List<TextureEntry> textures = textureEntryList.GetUniqueTextureList();
               ttCurrentSelection = textures[0];
            }

            if (ttCurrentSelection.mTagString == null)
            {
               ttCurrentSelection.mTagString = "";
            }

            if (!ttCurrentSelection.mTagString.Contains(strNewTag))
            {
               ttCurrentSelection.mTags.Add(strNewTag);
               ttCurrentSelection.mTagString += (strNewTag + ",");
               ttCurrentSelection.bNeedsSaving = true;
               tagListBox.Items.Add(strNewTag);
            }

            txtNewTagBox.Text = "";

         }
      }

      private bool SaveTextureEntryMetadata( TextureEntry tt, string strPath )
      {
         XmlDocument newMetaDataDocument = new XmlDocument();
         XmlElement rootElement = newMetaDataDocument.CreateElement("TextureMetadata");
         newMetaDataDocument.AppendChild(rootElement);

         //save out name
         XmlElement nameElement = newMetaDataDocument.CreateElement("Name");
         rootElement.AppendChild(nameElement);
         nameElement.InnerText = tt.mFilename;

         //save out made up tags
         XmlElement tagListElement = newMetaDataDocument.CreateElement("TagList");
         rootElement.AppendChild(tagListElement);

         //save out those that link to us
         XmlElement mappedPathElement = newMetaDataDocument.CreateElement("MappedPaths");
         rootElement.AppendChild(mappedPathElement);

         foreach ( string s in tt.mStringsThatMapToMe )
         {
            XmlElement pathString = newMetaDataDocument.CreateElement("Path");
            pathString.InnerText = s;
            mappedPathElement.AppendChild(pathString);
         }

         //save out linked images
         XmlElement linkedImageElement = newMetaDataDocument.CreateElement("LinkedImages");
         rootElement.AppendChild(linkedImageElement);

         //save out additional text string
         XmlElement additionalTextElement = newMetaDataDocument.CreateElement("AdditionalText");
         rootElement.AppendChild(additionalTextElement);

         XmlElement insideText = newMetaDataDocument.CreateElement("Text");
         insideText.InnerText = tt.mAddtionalTextString;

         foreach (string s in tt.mImagesILinkTo)
         {
            XmlElement pathString = newMetaDataDocument.CreateElement("Link");
            pathString.InnerText = s;
            linkedImageElement.AppendChild(pathString);
         }

         try
         {
            newMetaDataDocument.Save(strPath + "\\" + tt.mFilename + ".xmlmeta");
            tt.bNeedsSaving = false;
            return true;

         }
         catch (System.Exception ex)
         {
         	
         }
         
         return false;

      }

      private bool SaveCurrentTextureSelection()
      {
         string rootPath = Environment.ExpandEnvironmentVariables("%BPE_REPOSITORY%") + "\\textures\\flatlist";
         return SaveTextureEntryMetadata(ttCurrentSelection, rootPath);
      }

      //might have to do more than this? 
      private void AdjustFlowLayoutPanel()
      {
         foreach ( Control cc in flowLayoutPanel1.Controls)
         {
            PictureBox pb = (PictureBox)cc;
            if( pb != null )
            {
               pb.BorderStyle = BorderStyle.None;
               pb.Size = new Size(92, 92);
            }
         }
      }

      private void LinkTwoImages( TextureEntry a, TextureEntry b)
      {
         a.mImagesILinkTo.Add(b.mFilename);
         b.mImagesILinkTo.Add(a.mFilename);

         string rootPath = Environment.ExpandEnvironmentVariables("%BPE_REPOSITORY%") + "\\textures\\flatlist";
         SaveTextureEntryMetadata(a, rootPath);
         SaveTextureEntryMetadata(b, rootPath);
      }

      //When a picture in the top shelf has been right clicked and selected to link with the current picture.
      private void rightClickinToolStripMenuItem_Click(object sender, EventArgs e)
      {
         if( rightClickedPictureBox != null )
         {
            TextureEntryList textureEntryList = mAssetHashToFileMapping[(string)rightClickedPictureBox.Tag];
            if (textureEntryList != null)
            {
               TextureEntry linkEntry = textureEntryList.mAllTextures[0];
               if (linkEntry != ttCurrentSelection)
               {
                  LinkTwoImages(linkEntry, ttCurrentSelection);
               }
            }
         }
      }

      private void lblTagLabel_Click(object sender, EventArgs e)
      {

      }

      private void txtNewTagBox_TextChanged(object sender, EventArgs e)
      {

      }
   }
}
