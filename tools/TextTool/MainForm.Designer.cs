namespace TextTool
{
   partial class MainForm
   {
      /// <summary>
      /// Required designer variable.
      /// </summary>
      private System.ComponentModel.IContainer components = null;

      /// <summary>
      /// Clean up any resources being used.
      /// </summary>
      /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
      protected override void Dispose(bool disposing)
      {
         if (disposing && (components != null))
         {
            components.Dispose();
         }
         base.Dispose(disposing);
      }

      #region Windows Form Designer generated code

      /// <summary>
      /// Required method for Designer support - do not modify
      /// the contents of this method with the code editor.
      /// </summary>
      private void InitializeComponent()
      {
         this.components = new System.ComponentModel.Container();
         this.kpFontTextPreview = new System.Windows.Forms.PictureBox();
         this.textStringsListView = new System.Windows.Forms.ListView();
         this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
         this.unicodeRichTextBox = new System.Windows.Forms.RichTextBox();
         this.splitContainer1 = new System.Windows.Forms.SplitContainer();
         this.splitContainer3 = new System.Windows.Forms.SplitContainer();
         this.fileListView = new System.Windows.Forms.ListView();
         this.columnHeader4 = new System.Windows.Forms.ColumnHeader();
         this.filterProblemStringsCheckBox = new System.Windows.Forms.CheckBox();
         this.groupBox4 = new System.Windows.Forms.GroupBox();
         this.symbolCheckBox = new System.Windows.Forms.CheckBox();
         this.kanjiCheckBox = new System.Windows.Forms.CheckBox();
         this.katakanaCheckBox = new System.Windows.Forms.CheckBox();
         this.hiraganaCheckBox = new System.Windows.Forms.CheckBox();
         this.ansiiCheckBox = new System.Windows.Forms.CheckBox();
         this.splitContainer2 = new System.Windows.Forms.SplitContainer();
         this.groupBox3 = new System.Windows.Forms.GroupBox();
         this.unicodeDisplay = new System.Windows.Forms.TextBox();
         this.button3 = new System.Windows.Forms.Button();
         this.characterPreviewTextBox = new System.Windows.Forms.TextBox();
         this.ocrResultListView = new System.Windows.Forms.ListView();
         this.columnHeader5 = new System.Windows.Forms.ColumnHeader();
         this.button2 = new System.Windows.Forms.Button();
         this.button1 = new System.Windows.Forms.Button();
         this.unknownCharacterListBox = new System.Windows.Forms.ListView();
         this.columnHeader2 = new System.Windows.Forms.ColumnHeader();
         this.columnHeader3 = new System.Windows.Forms.ColumnHeader();
         this.characterMapInputTextBox = new System.Windows.Forms.TextBox();
         this.characterPreviewPictureBox = new System.Windows.Forms.PictureBox();
         this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
         this.groupBox1 = new System.Windows.Forms.GroupBox();
         this.groupBox5 = new System.Windows.Forms.GroupBox();
         this.bpFontTextPreview = new System.Windows.Forms.PictureBox();
         this.groupBox2 = new System.Windows.Forms.GroupBox();
         this.autoSaveTimer = new System.Windows.Forms.Timer(this.components);
         ((System.ComponentModel.ISupportInitialize)(this.kpFontTextPreview)).BeginInit();
         this.splitContainer1.Panel1.SuspendLayout();
         this.splitContainer1.Panel2.SuspendLayout();
         this.splitContainer1.SuspendLayout();
         this.splitContainer3.Panel1.SuspendLayout();
         this.splitContainer3.Panel2.SuspendLayout();
         this.splitContainer3.SuspendLayout();
         this.groupBox4.SuspendLayout();
         this.splitContainer2.Panel1.SuspendLayout();
         this.splitContainer2.Panel2.SuspendLayout();
         this.splitContainer2.SuspendLayout();
         this.groupBox3.SuspendLayout();
         ((System.ComponentModel.ISupportInitialize)(this.characterPreviewPictureBox)).BeginInit();
         this.tableLayoutPanel1.SuspendLayout();
         this.groupBox1.SuspendLayout();
         this.groupBox5.SuspendLayout();
         ((System.ComponentModel.ISupportInitialize)(this.bpFontTextPreview)).BeginInit();
         this.groupBox2.SuspendLayout();
         this.SuspendLayout();
         // 
         // kpFontTextPreview
         // 
         this.kpFontTextPreview.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(64)))), ((int)(((byte)(64)))), ((int)(((byte)(64)))));
         this.kpFontTextPreview.Dock = System.Windows.Forms.DockStyle.Fill;
         this.kpFontTextPreview.Location = new System.Drawing.Point(3, 16);
         this.kpFontTextPreview.Name = "kpFontTextPreview";
         this.kpFontTextPreview.Size = new System.Drawing.Size(1279, 247);
         this.kpFontTextPreview.TabIndex = 0;
         this.kpFontTextPreview.TabStop = false;
         // 
         // textStringsListView
         // 
         this.textStringsListView.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                     | System.Windows.Forms.AnchorStyles.Left)
                     | System.Windows.Forms.AnchorStyles.Right)));
         this.textStringsListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1});
         this.textStringsListView.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F);
         this.textStringsListView.FullRowSelect = true;
         this.textStringsListView.HideSelection = false;
         this.textStringsListView.Location = new System.Drawing.Point(0, 70);
         this.textStringsListView.MultiSelect = false;
         this.textStringsListView.Name = "textStringsListView";
         this.textStringsListView.Size = new System.Drawing.Size(377, 656);
         this.textStringsListView.TabIndex = 1;
         this.textStringsListView.UseCompatibleStateImageBehavior = false;
         this.textStringsListView.View = System.Windows.Forms.View.Details;
         this.textStringsListView.ItemSelectionChanged += new System.Windows.Forms.ListViewItemSelectionChangedEventHandler(this.OnTextStringSelected);
         // 
         // columnHeader1
         // 
         this.columnHeader1.Text = "Text";
         this.columnHeader1.Width = 375;
         // 
         // unicodeRichTextBox
         // 
         this.unicodeRichTextBox.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(64)))), ((int)(((byte)(64)))), ((int)(((byte)(64)))));
         this.unicodeRichTextBox.Dock = System.Windows.Forms.DockStyle.Fill;
         this.unicodeRichTextBox.Font = new System.Drawing.Font("Microsoft Sans Serif", 18F);
         this.unicodeRichTextBox.ForeColor = System.Drawing.Color.White;
         this.unicodeRichTextBox.Location = new System.Drawing.Point(3, 16);
         this.unicodeRichTextBox.Name = "unicodeRichTextBox";
         this.unicodeRichTextBox.Size = new System.Drawing.Size(1279, 157);
         this.unicodeRichTextBox.TabIndex = 2;
         this.unicodeRichTextBox.Text = "";
         this.unicodeRichTextBox.TextChanged += new System.EventHandler(this.OnTextChanged);
         // 
         // splitContainer1
         // 
         this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
         this.splitContainer1.Location = new System.Drawing.Point(0, 0);
         this.splitContainer1.Name = "splitContainer1";
         // 
         // splitContainer1.Panel1
         // 
         this.splitContainer1.Panel1.Controls.Add(this.splitContainer3);
         // 
         // splitContainer1.Panel2
         // 
         this.splitContainer1.Panel2.Controls.Add(this.splitContainer2);
         this.splitContainer1.Size = new System.Drawing.Size(1999, 998);
         this.splitContainer1.SplitterDistance = 381;
         this.splitContainer1.TabIndex = 3;
         // 
         // splitContainer3
         // 
         this.splitContainer3.Dock = System.Windows.Forms.DockStyle.Fill;
         this.splitContainer3.Location = new System.Drawing.Point(0, 0);
         this.splitContainer3.Name = "splitContainer3";
         this.splitContainer3.Orientation = System.Windows.Forms.Orientation.Horizontal;
         // 
         // splitContainer3.Panel1
         // 
         this.splitContainer3.Panel1.Controls.Add(this.fileListView);
         // 
         // splitContainer3.Panel2
         // 
         this.splitContainer3.Panel2.Controls.Add(this.filterProblemStringsCheckBox);
         this.splitContainer3.Panel2.Controls.Add(this.groupBox4);
         this.splitContainer3.Panel2.Controls.Add(this.textStringsListView);
         this.splitContainer3.Size = new System.Drawing.Size(381, 998);
         this.splitContainer3.SplitterDistance = 268;
         this.splitContainer3.TabIndex = 2;
         // 
         // fileListView
         // 
         this.fileListView.Activation = System.Windows.Forms.ItemActivation.OneClick;
         this.fileListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader4});
         this.fileListView.Dock = System.Windows.Forms.DockStyle.Fill;
         this.fileListView.FullRowSelect = true;
         this.fileListView.HideSelection = false;
         this.fileListView.Location = new System.Drawing.Point(0, 0);
         this.fileListView.MultiSelect = false;
         this.fileListView.Name = "fileListView";
         this.fileListView.Size = new System.Drawing.Size(381, 268);
         this.fileListView.TabIndex = 2;
         this.fileListView.UseCompatibleStateImageBehavior = false;
         this.fileListView.View = System.Windows.Forms.View.Details;
         this.fileListView.SelectedIndexChanged += new System.EventHandler(this.OnFileChanged);
         // 
         // columnHeader4
         // 
         this.columnHeader4.Text = "File";
         this.columnHeader4.Width = 375;
         // 
         // filterProblemStringsCheckBox
         // 
         this.filterProblemStringsCheckBox.AutoSize = true;
         this.filterProblemStringsCheckBox.Location = new System.Drawing.Point(6, 46);
         this.filterProblemStringsCheckBox.Name = "filterProblemStringsCheckBox";
         this.filterProblemStringsCheckBox.Size = new System.Drawing.Size(153, 17);
         this.filterProblemStringsCheckBox.TabIndex = 3;
         this.filterProblemStringsCheckBox.Text = "Show Problem Strings Only";
         this.filterProblemStringsCheckBox.UseVisualStyleBackColor = true;
         this.filterProblemStringsCheckBox.CheckedChanged += new System.EventHandler(this.OnHightlightProblemsChanged);
         // 
         // groupBox4
         // 
         this.groupBox4.Controls.Add(this.symbolCheckBox);
         this.groupBox4.Controls.Add(this.kanjiCheckBox);
         this.groupBox4.Controls.Add(this.katakanaCheckBox);
         this.groupBox4.Controls.Add(this.hiraganaCheckBox);
         this.groupBox4.Controls.Add(this.ansiiCheckBox);
         this.groupBox4.Dock = System.Windows.Forms.DockStyle.Top;
         this.groupBox4.Location = new System.Drawing.Point(0, 0);
         this.groupBox4.Name = "groupBox4";
         this.groupBox4.Size = new System.Drawing.Size(381, 40);
         this.groupBox4.TabIndex = 2;
         this.groupBox4.TabStop = false;
         this.groupBox4.Text = "Highlight Problems";
         // 
         // symbolCheckBox
         // 
         this.symbolCheckBox.AutoSize = true;
         this.symbolCheckBox.Checked = true;
         this.symbolCheckBox.CheckState = System.Windows.Forms.CheckState.Checked;
         this.symbolCheckBox.Location = new System.Drawing.Point(219, 19);
         this.symbolCheckBox.Name = "symbolCheckBox";
         this.symbolCheckBox.Size = new System.Drawing.Size(60, 17);
         this.symbolCheckBox.TabIndex = 0;
         this.symbolCheckBox.Text = "Symbol";
         this.symbolCheckBox.UseVisualStyleBackColor = true;
         this.symbolCheckBox.CheckedChanged += new System.EventHandler(this.OnHightlightProblemsChanged);
         // 
         // kanjiCheckBox
         // 
         this.kanjiCheckBox.AutoSize = true;
         this.kanjiCheckBox.Checked = true;
         this.kanjiCheckBox.CheckState = System.Windows.Forms.CheckState.Checked;
         this.kanjiCheckBox.Location = new System.Drawing.Point(285, 19);
         this.kanjiCheckBox.Name = "kanjiCheckBox";
         this.kanjiCheckBox.Size = new System.Drawing.Size(49, 17);
         this.kanjiCheckBox.TabIndex = 0;
         this.kanjiCheckBox.Text = "Kanji";
         this.kanjiCheckBox.UseVisualStyleBackColor = true;
         this.kanjiCheckBox.CheckedChanged += new System.EventHandler(this.OnHightlightProblemsChanged);
         // 
         // katakanaCheckBox
         // 
         this.katakanaCheckBox.AutoSize = true;
         this.katakanaCheckBox.Checked = true;
         this.katakanaCheckBox.CheckState = System.Windows.Forms.CheckState.Checked;
         this.katakanaCheckBox.Location = new System.Drawing.Point(141, 19);
         this.katakanaCheckBox.Name = "katakanaCheckBox";
         this.katakanaCheckBox.Size = new System.Drawing.Size(72, 17);
         this.katakanaCheckBox.TabIndex = 0;
         this.katakanaCheckBox.Text = "Katakana";
         this.katakanaCheckBox.UseVisualStyleBackColor = true;
         this.katakanaCheckBox.CheckedChanged += new System.EventHandler(this.OnHightlightProblemsChanged);
         // 
         // hiraganaCheckBox
         // 
         this.hiraganaCheckBox.AutoSize = true;
         this.hiraganaCheckBox.Checked = true;
         this.hiraganaCheckBox.CheckState = System.Windows.Forms.CheckState.Checked;
         this.hiraganaCheckBox.Location = new System.Drawing.Point(66, 19);
         this.hiraganaCheckBox.Name = "hiraganaCheckBox";
         this.hiraganaCheckBox.Size = new System.Drawing.Size(69, 17);
         this.hiraganaCheckBox.TabIndex = 0;
         this.hiraganaCheckBox.Text = "Hiragana";
         this.hiraganaCheckBox.UseVisualStyleBackColor = true;
         this.hiraganaCheckBox.CheckedChanged += new System.EventHandler(this.OnHightlightProblemsChanged);
         // 
         // ansiiCheckBox
         // 
         this.ansiiCheckBox.AutoSize = true;
         this.ansiiCheckBox.Checked = true;
         this.ansiiCheckBox.CheckState = System.Windows.Forms.CheckState.Checked;
         this.ansiiCheckBox.Location = new System.Drawing.Point(6, 19);
         this.ansiiCheckBox.Name = "ansiiCheckBox";
         this.ansiiCheckBox.Size = new System.Drawing.Size(54, 17);
         this.ansiiCheckBox.TabIndex = 0;
         this.ansiiCheckBox.Text = "ANSII";
         this.ansiiCheckBox.UseVisualStyleBackColor = true;
         this.ansiiCheckBox.CheckedChanged += new System.EventHandler(this.OnHightlightProblemsChanged);
         // 
         // splitContainer2
         // 
         this.splitContainer2.Dock = System.Windows.Forms.DockStyle.Fill;
         this.splitContainer2.Location = new System.Drawing.Point(0, 0);
         this.splitContainer2.Name = "splitContainer2";
         // 
         // splitContainer2.Panel1
         // 
         this.splitContainer2.Panel1.Controls.Add(this.groupBox3);
         // 
         // splitContainer2.Panel2
         // 
         this.splitContainer2.Panel2.Controls.Add(this.tableLayoutPanel1);
         this.splitContainer2.Size = new System.Drawing.Size(1614, 998);
         this.splitContainer2.SplitterDistance = 319;
         this.splitContainer2.TabIndex = 0;
         // 
         // groupBox3
         // 
         this.groupBox3.Controls.Add(this.unicodeDisplay);
         this.groupBox3.Controls.Add(this.button3);
         this.groupBox3.Controls.Add(this.characterPreviewTextBox);
         this.groupBox3.Controls.Add(this.ocrResultListView);
         this.groupBox3.Controls.Add(this.button2);
         this.groupBox3.Controls.Add(this.button1);
         this.groupBox3.Controls.Add(this.unknownCharacterListBox);
         this.groupBox3.Controls.Add(this.characterMapInputTextBox);
         this.groupBox3.Controls.Add(this.characterPreviewPictureBox);
         this.groupBox3.Dock = System.Windows.Forms.DockStyle.Fill;
         this.groupBox3.Location = new System.Drawing.Point(0, 0);
         this.groupBox3.Name = "groupBox3";
         this.groupBox3.Size = new System.Drawing.Size(319, 998);
         this.groupBox3.TabIndex = 3;
         this.groupBox3.TabStop = false;
         this.groupBox3.Text = "Characters";
         // 
         // unicodeDisplay
         // 
         this.unicodeDisplay.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                     | System.Windows.Forms.AnchorStyles.Right)));
         this.unicodeDisplay.Enabled = false;
         this.unicodeDisplay.Font = new System.Drawing.Font("Microsoft Sans Serif", 24F);
         this.unicodeDisplay.Location = new System.Drawing.Point(217, 942);
         this.unicodeDisplay.Name = "unicodeDisplay";
         this.unicodeDisplay.ReadOnly = true;
         this.unicodeDisplay.Size = new System.Drawing.Size(95, 44);
         this.unicodeDisplay.TabIndex = 10;
         // 
         // button3
         // 
         this.button3.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                     | System.Windows.Forms.AnchorStyles.Right)));
         this.button3.Location = new System.Drawing.Point(218, 913);
         this.button3.Name = "button3";
         this.button3.Size = new System.Drawing.Size(95, 24);
         this.button3.TabIndex = 9;
         this.button3.Text = "Update BP Font";
         this.button3.UseVisualStyleBackColor = true;
         this.button3.Click += new System.EventHandler(this.OnUpdatedBPFont);
         // 
         // characterPreviewTextBox
         // 
         this.characterPreviewTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
         this.characterPreviewTextBox.BackColor = System.Drawing.Color.Black;
         this.characterPreviewTextBox.BorderStyle = System.Windows.Forms.BorderStyle.None;
         this.characterPreviewTextBox.Font = new System.Drawing.Font("Microsoft Sans Serif", 60F);
         this.characterPreviewTextBox.ForeColor = System.Drawing.Color.White;
         this.characterPreviewTextBox.Location = new System.Drawing.Point(115, 811);
         this.characterPreviewTextBox.Multiline = true;
         this.characterPreviewTextBox.Name = "characterPreviewTextBox";
         this.characterPreviewTextBox.ReadOnly = true;
         this.characterPreviewTextBox.Size = new System.Drawing.Size(96, 96);
         this.characterPreviewTextBox.TabIndex = 8;
         this.characterPreviewTextBox.WordWrap = false;
         // 
         // ocrResultListView
         // 
         this.ocrResultListView.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                     | System.Windows.Forms.AnchorStyles.Right)));
         this.ocrResultListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader5});
         this.ocrResultListView.Font = new System.Drawing.Font("Microsoft Sans Serif", 18F);
         this.ocrResultListView.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.None;
         this.ocrResultListView.Location = new System.Drawing.Point(217, 812);
         this.ocrResultListView.Name = "ocrResultListView";
         this.ocrResultListView.Size = new System.Drawing.Size(97, 96);
         this.ocrResultListView.TabIndex = 7;
         this.ocrResultListView.UseCompatibleStateImageBehavior = false;
         this.ocrResultListView.View = System.Windows.Forms.View.Details;
         this.ocrResultListView.SelectedIndexChanged += new System.EventHandler(this.OnOcrItemSelected);
         // 
         // columnHeader5
         // 
         this.columnHeader5.Text = "Character";
         // 
         // button2
         // 
         this.button2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
         this.button2.Location = new System.Drawing.Point(11, 914);
         this.button2.Name = "button2";
         this.button2.Size = new System.Drawing.Size(98, 23);
         this.button2.TabIndex = 6;
         this.button2.Text = "OCR";
         this.button2.UseVisualStyleBackColor = true;
         this.button2.Click += new System.EventHandler(this.OnOCR);
         // 
         // button1
         // 
         this.button1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
         this.button1.Location = new System.Drawing.Point(115, 913);
         this.button1.Name = "button1";
         this.button1.Size = new System.Drawing.Size(96, 23);
         this.button1.TabIndex = 5;
         this.button1.Text = "Use Preview";
         this.button1.UseVisualStyleBackColor = true;
         this.button1.Click += new System.EventHandler(this.OnUsePreviewCharacter);
         // 
         // unknownCharacterListBox
         // 
         this.unknownCharacterListBox.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                     | System.Windows.Forms.AnchorStyles.Left)
                     | System.Windows.Forms.AnchorStyles.Right)));
         this.unknownCharacterListBox.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader2,
            this.columnHeader3});
         this.unknownCharacterListBox.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F);
         this.unknownCharacterListBox.FullRowSelect = true;
         this.unknownCharacterListBox.HideSelection = false;
         this.unknownCharacterListBox.Location = new System.Drawing.Point(13, 19);
         this.unknownCharacterListBox.MultiSelect = false;
         this.unknownCharacterListBox.Name = "unknownCharacterListBox";
         this.unknownCharacterListBox.Size = new System.Drawing.Size(301, 786);
         this.unknownCharacterListBox.TabIndex = 3;
         this.unknownCharacterListBox.UseCompatibleStateImageBehavior = false;
         this.unknownCharacterListBox.View = System.Windows.Forms.View.Details;
         this.unknownCharacterListBox.SelectedIndexChanged += new System.EventHandler(this.OnUnknownCharacterSelected);
         // 
         // columnHeader2
         // 
         this.columnHeader2.Text = "Code";
         this.columnHeader2.Width = 143;
         // 
         // columnHeader3
         // 
         this.columnHeader3.Text = "Character";
         this.columnHeader3.Width = 148;
         // 
         // characterMapInputTextBox
         // 
         this.characterMapInputTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
         this.characterMapInputTextBox.Enabled = false;
         this.characterMapInputTextBox.Font = new System.Drawing.Font("Microsoft Sans Serif", 24F);
         this.characterMapInputTextBox.Location = new System.Drawing.Point(13, 942);
         this.characterMapInputTextBox.Name = "characterMapInputTextBox";
         this.characterMapInputTextBox.Size = new System.Drawing.Size(198, 44);
         this.characterMapInputTextBox.TabIndex = 2;
         this.characterMapInputTextBox.KeyUp += new System.Windows.Forms.KeyEventHandler(this.OnUpdateUnknownCharacter);
         // 
         // characterPreviewPictureBox
         // 
         this.characterPreviewPictureBox.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
         this.characterPreviewPictureBox.BackColor = System.Drawing.Color.Black;
         this.characterPreviewPictureBox.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
         this.characterPreviewPictureBox.Location = new System.Drawing.Point(13, 812);
         this.characterPreviewPictureBox.Name = "characterPreviewPictureBox";
         this.characterPreviewPictureBox.Size = new System.Drawing.Size(96, 96);
         this.characterPreviewPictureBox.TabIndex = 1;
         this.characterPreviewPictureBox.TabStop = false;
         // 
         // tableLayoutPanel1
         // 
         this.tableLayoutPanel1.ColumnCount = 1;
         this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
         this.tableLayoutPanel1.Controls.Add(this.groupBox1, 0, 0);
         this.tableLayoutPanel1.Controls.Add(this.groupBox5, 0, 1);
         this.tableLayoutPanel1.Controls.Add(this.groupBox2, 0, 2);
         this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
         this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
         this.tableLayoutPanel1.Name = "tableLayoutPanel1";
         this.tableLayoutPanel1.RowCount = 3;
         this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 27.27273F));
         this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 54.54546F));
         this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 18.18182F));
         this.tableLayoutPanel1.Size = new System.Drawing.Size(1291, 998);
         this.tableLayoutPanel1.TabIndex = 3;
         // 
         // groupBox1
         // 
         this.groupBox1.Controls.Add(this.kpFontTextPreview);
         this.groupBox1.Dock = System.Windows.Forms.DockStyle.Fill;
         this.groupBox1.Location = new System.Drawing.Point(3, 3);
         this.groupBox1.Name = "groupBox1";
         this.groupBox1.Size = new System.Drawing.Size(1285, 266);
         this.groupBox1.TabIndex = 1;
         this.groupBox1.TabStop = false;
         this.groupBox1.Text = "KP Font";
         // 
         // groupBox5
         // 
         this.groupBox5.Controls.Add(this.bpFontTextPreview);
         this.groupBox5.Dock = System.Windows.Forms.DockStyle.Fill;
         this.groupBox5.Location = new System.Drawing.Point(3, 275);
         this.groupBox5.Name = "groupBox5";
         this.groupBox5.Size = new System.Drawing.Size(1285, 538);
         this.groupBox5.TabIndex = 2;
         this.groupBox5.TabStop = false;
         this.groupBox5.Text = "BP Font";
         // 
         // bpFontTextPreview
         // 
         this.bpFontTextPreview.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(64)))), ((int)(((byte)(64)))), ((int)(((byte)(64)))));
         this.bpFontTextPreview.Dock = System.Windows.Forms.DockStyle.Fill;
         this.bpFontTextPreview.Location = new System.Drawing.Point(3, 16);
         this.bpFontTextPreview.Name = "bpFontTextPreview";
         this.bpFontTextPreview.Size = new System.Drawing.Size(1279, 519);
         this.bpFontTextPreview.TabIndex = 0;
         this.bpFontTextPreview.TabStop = false;
         // 
         // groupBox2
         // 
         this.groupBox2.Controls.Add(this.unicodeRichTextBox);
         this.groupBox2.Dock = System.Windows.Forms.DockStyle.Fill;
         this.groupBox2.Location = new System.Drawing.Point(3, 819);
         this.groupBox2.Name = "groupBox2";
         this.groupBox2.Size = new System.Drawing.Size(1285, 176);
         this.groupBox2.TabIndex = 2;
         this.groupBox2.TabStop = false;
         this.groupBox2.Text = "Text";
         // 
         // autoSaveTimer
         // 
         this.autoSaveTimer.Enabled = true;
         this.autoSaveTimer.Interval = 1000;
         this.autoSaveTimer.Tick += new System.EventHandler(this.OnAutoSave);
         // 
         // MainForm
         // 
         this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
         this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
         this.ClientSize = new System.Drawing.Size(1999, 998);
         this.Controls.Add(this.splitContainer1);
         this.Name = "MainForm";
         this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
         this.Text = "MGS Text Tool";
         this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.OnClosing);
         ((System.ComponentModel.ISupportInitialize)(this.kpFontTextPreview)).EndInit();
         this.splitContainer1.Panel1.ResumeLayout(false);
         this.splitContainer1.Panel2.ResumeLayout(false);
         this.splitContainer1.ResumeLayout(false);
         this.splitContainer3.Panel1.ResumeLayout(false);
         this.splitContainer3.Panel2.ResumeLayout(false);
         this.splitContainer3.Panel2.PerformLayout();
         this.splitContainer3.ResumeLayout(false);
         this.groupBox4.ResumeLayout(false);
         this.groupBox4.PerformLayout();
         this.splitContainer2.Panel1.ResumeLayout(false);
         this.splitContainer2.Panel2.ResumeLayout(false);
         this.splitContainer2.ResumeLayout(false);
         this.groupBox3.ResumeLayout(false);
         this.groupBox3.PerformLayout();
         ((System.ComponentModel.ISupportInitialize)(this.characterPreviewPictureBox)).EndInit();
         this.tableLayoutPanel1.ResumeLayout(false);
         this.groupBox1.ResumeLayout(false);
         this.groupBox5.ResumeLayout(false);
         ((System.ComponentModel.ISupportInitialize)(this.bpFontTextPreview)).EndInit();
         this.groupBox2.ResumeLayout(false);
         this.ResumeLayout(false);

      }

      #endregion

      private System.Windows.Forms.PictureBox kpFontTextPreview;
      private System.Windows.Forms.ListView textStringsListView;
      private System.Windows.Forms.RichTextBox unicodeRichTextBox;
      private System.Windows.Forms.SplitContainer splitContainer1;
      private System.Windows.Forms.SplitContainer splitContainer2;
      private System.Windows.Forms.ColumnHeader columnHeader1;
      private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
      private System.Windows.Forms.GroupBox groupBox2;
      private System.Windows.Forms.GroupBox groupBox1;
      private System.Windows.Forms.GroupBox groupBox3;
      private System.Windows.Forms.TextBox characterMapInputTextBox;
      private System.Windows.Forms.PictureBox characterPreviewPictureBox;
      private System.Windows.Forms.ListView unknownCharacterListBox;
      private System.Windows.Forms.ColumnHeader columnHeader2;
      private System.Windows.Forms.ColumnHeader columnHeader3;
      private System.Windows.Forms.Timer autoSaveTimer;
      private System.Windows.Forms.SplitContainer splitContainer3;
      private System.Windows.Forms.ListView fileListView;
      private System.Windows.Forms.ColumnHeader columnHeader4;
      private System.Windows.Forms.Button button1;
      private System.Windows.Forms.GroupBox groupBox4;
      private System.Windows.Forms.CheckBox kanjiCheckBox;
      private System.Windows.Forms.CheckBox katakanaCheckBox;
      private System.Windows.Forms.CheckBox hiraganaCheckBox;
      private System.Windows.Forms.CheckBox ansiiCheckBox;
      private System.Windows.Forms.CheckBox filterProblemStringsCheckBox;
      private System.Windows.Forms.CheckBox symbolCheckBox;
      private System.Windows.Forms.Button button2;
      private System.Windows.Forms.ListView ocrResultListView;
      private System.Windows.Forms.ColumnHeader columnHeader5;
      private System.Windows.Forms.TextBox characterPreviewTextBox;
      private System.Windows.Forms.GroupBox groupBox5;
      private System.Windows.Forms.PictureBox bpFontTextPreview;
      private System.Windows.Forms.Button button3;
      private System.Windows.Forms.TextBox unicodeDisplay;
   }
}

