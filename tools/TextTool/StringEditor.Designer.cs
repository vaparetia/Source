namespace TextTool
{
   partial class StringEditor
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
         this.textListView = new System.Windows.Forms.ListView();
         this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
         this.columnHeader3 = new System.Windows.Forms.ColumnHeader();
         this.originalPreview = new System.Windows.Forms.PictureBox();
         this.remappedTextBox = new System.Windows.Forms.RichTextBox();
         this.remappedPreview = new System.Windows.Forms.PictureBox();
         this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
         this.panel3 = new System.Windows.Forms.Panel();
         this.label3 = new System.Windows.Forms.Label();
         this.panel4 = new System.Windows.Forms.Panel();
         this.panel6 = new System.Windows.Forms.Panel();
         this.label9 = new System.Windows.Forms.Label();
         this.previewModeComboBox = new System.Windows.Forms.ComboBox();
         this.label6 = new System.Windows.Forms.Label();
         this.previewPlatformComboBox = new System.Windows.Forms.ComboBox();
         this.label4 = new System.Windows.Forms.Label();
         this.panel2 = new System.Windows.Forms.Panel();
         this.label7 = new System.Windows.Forms.Label();
         this.overridePlatformComboBox = new System.Windows.Forms.ComboBox();
         this.label5 = new System.Windows.Forms.Label();
         this.removeOverrideButton = new System.Windows.Forms.Button();
         this.overrideButton = new System.Windows.Forms.Button();
         this.fileListView = new System.Windows.Forms.ListView();
         this.columnHeader2 = new System.Windows.Forms.ColumnHeader();
         this.contextMenuStrip1 = new System.Windows.Forms.ContextMenuStrip( this.components );
         this.setFileFilterToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.stringFilterTextBox = new System.Windows.Forms.TextBox();
         this.updateTextListTimer = new System.Windows.Forms.Timer( this.components );
         this.tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
         this.groupBox1 = new System.Windows.Forms.GroupBox();
         this.label8 = new System.Windows.Forms.Label();
         this.infoSearchTextBox = new System.Windows.Forms.TextBox();
         this.searchFilterComboBox = new System.Windows.Forms.ComboBox();
         this.matchCaseCheckBox = new System.Windows.Forms.CheckBox();
         this.showOverrideCheckBox = new System.Windows.Forms.CheckBox();
         this.label2 = new System.Windows.Forms.Label();
         this.label1 = new System.Windows.Forms.Label();
         this.fileFilterTextBox = new System.Windows.Forms.TextBox();
         this.panel5 = new System.Windows.Forms.Panel();
         this.groupBox2 = new System.Windows.Forms.GroupBox();
         this.infoRichTextBox = new System.Windows.Forms.RichTextBox();
         this.panel1 = new System.Windows.Forms.Panel();
         this.button6 = new System.Windows.Forms.Button();
         this.button4 = new System.Windows.Forms.Button();
         this.button3 = new System.Windows.Forms.Button();
         this.button2 = new System.Windows.Forms.Button();
         this.button5 = new System.Windows.Forms.Button();
         this.button1 = new System.Windows.Forms.Button();
         this.splitContainer1 = new System.Windows.Forms.SplitContainer();
         this.saveRemapTableTimer = new System.Windows.Forms.Timer( this.components );
         this.toolTip1 = new System.Windows.Forms.ToolTip( this.components );
         ((System.ComponentModel.ISupportInitialize)(this.originalPreview)).BeginInit();
         ((System.ComponentModel.ISupportInitialize)(this.remappedPreview)).BeginInit();
         this.tableLayoutPanel1.SuspendLayout();
         this.panel3.SuspendLayout();
         this.panel4.SuspendLayout();
         this.panel6.SuspendLayout();
         this.panel2.SuspendLayout();
         this.contextMenuStrip1.SuspendLayout();
         this.tableLayoutPanel2.SuspendLayout();
         this.groupBox1.SuspendLayout();
         this.panel5.SuspendLayout();
         this.groupBox2.SuspendLayout();
         this.panel1.SuspendLayout();
         this.splitContainer1.Panel1.SuspendLayout();
         this.splitContainer1.Panel2.SuspendLayout();
         this.splitContainer1.SuspendLayout();
         this.SuspendLayout();
         // 
         // textListView
         // 
         this.textListView.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                     | System.Windows.Forms.AnchorStyles.Left)
                     | System.Windows.Forms.AnchorStyles.Right)));
         this.textListView.CheckBoxes = true;
         this.textListView.Columns.AddRange( new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader3} );
         this.textListView.FullRowSelect = true;
         this.textListView.HideSelection = false;
         this.textListView.Location = new System.Drawing.Point( 9, 0 );
         this.textListView.Name = "textListView";
         this.textListView.Size = new System.Drawing.Size( 497, 451 );
         this.textListView.TabIndex = 0;
         this.textListView.UseCompatibleStateImageBehavior = false;
         this.textListView.View = System.Windows.Forms.View.Details;
         this.textListView.VirtualMode = true;
         this.textListView.RetrieveVirtualItem += new System.Windows.Forms.RetrieveVirtualItemEventHandler( this.OnRetrieveTextItem );
         this.textListView.ItemSelectionChanged += new System.Windows.Forms.ListViewItemSelectionChangedEventHandler( this.OnTextSelectionChanged );
         // 
         // columnHeader1
         // 
         this.columnHeader1.Text = "Original Text";
         this.columnHeader1.Width = 372;
         // 
         // columnHeader3
         // 
         this.columnHeader3.Text = "Overrides";
         this.columnHeader3.Width = 122;
         // 
         // originalPreview
         // 
         this.originalPreview.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                     | System.Windows.Forms.AnchorStyles.Left)
                     | System.Windows.Forms.AnchorStyles.Right)));
         this.originalPreview.BackColor = System.Drawing.Color.FromArgb( ((int)(((byte)(64)))), ((int)(((byte)(64)))), ((int)(((byte)(64)))) );
         this.originalPreview.Location = new System.Drawing.Point( 0, 25 );
         this.originalPreview.Name = "originalPreview";
         this.originalPreview.Size = new System.Drawing.Size( 1031, 419 );
         this.originalPreview.TabIndex = 1;
         this.originalPreview.TabStop = false;
         // 
         // remappedTextBox
         // 
         this.remappedTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                     | System.Windows.Forms.AnchorStyles.Left)
                     | System.Windows.Forms.AnchorStyles.Right)));
         this.remappedTextBox.Font = new System.Drawing.Font( "Microsoft Sans Serif", 16F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)) );
         this.remappedTextBox.Location = new System.Drawing.Point( 7, 27 );
         this.remappedTextBox.Name = "remappedTextBox";
         this.remappedTextBox.Size = new System.Drawing.Size( 1022, 182 );
         this.remappedTextBox.TabIndex = 2;
         this.remappedTextBox.Text = "";
         this.remappedTextBox.TextChanged += new System.EventHandler( this.OnTextChanged );
         // 
         // remappedPreview
         // 
         this.remappedPreview.BackColor = System.Drawing.Color.FromArgb( ((int)(((byte)(64)))), ((int)(((byte)(64)))), ((int)(((byte)(64)))) );
         this.remappedPreview.Dock = System.Windows.Forms.DockStyle.Fill;
         this.remappedPreview.Location = new System.Drawing.Point( 0, 0 );
         this.remappedPreview.Name = "remappedPreview";
         this.remappedPreview.Size = new System.Drawing.Size( 1031, 414 );
         this.remappedPreview.TabIndex = 1;
         this.remappedPreview.TabStop = false;
         // 
         // tableLayoutPanel1
         // 
         this.tableLayoutPanel1.ColumnCount = 1;
         this.tableLayoutPanel1.ColumnStyles.Add( new System.Windows.Forms.ColumnStyle( System.Windows.Forms.SizeType.Percent, 100F ) );
         this.tableLayoutPanel1.Controls.Add( this.panel3, 0, 0 );
         this.tableLayoutPanel1.Controls.Add( this.panel4, 0, 1 );
         this.tableLayoutPanel1.Controls.Add( this.panel2, 0, 2 );
         this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
         this.tableLayoutPanel1.Location = new System.Drawing.Point( 0, 0 );
         this.tableLayoutPanel1.Name = "tableLayoutPanel1";
         this.tableLayoutPanel1.RowCount = 3;
         this.tableLayoutPanel1.RowStyles.Add( new System.Windows.Forms.RowStyle( System.Windows.Forms.SizeType.Percent, 40F ) );
         this.tableLayoutPanel1.RowStyles.Add( new System.Windows.Forms.RowStyle( System.Windows.Forms.SizeType.Percent, 40F ) );
         this.tableLayoutPanel1.RowStyles.Add( new System.Windows.Forms.RowStyle( System.Windows.Forms.SizeType.Percent, 20F ) );
         this.tableLayoutPanel1.RowStyles.Add( new System.Windows.Forms.RowStyle( System.Windows.Forms.SizeType.Absolute, 20F ) );
         this.tableLayoutPanel1.Size = new System.Drawing.Size( 1037, 1125 );
         this.tableLayoutPanel1.TabIndex = 3;
         // 
         // panel3
         // 
         this.panel3.Controls.Add( this.label3 );
         this.panel3.Controls.Add( this.originalPreview );
         this.panel3.Dock = System.Windows.Forms.DockStyle.Fill;
         this.panel3.Location = new System.Drawing.Point( 3, 3 );
         this.panel3.Name = "panel3";
         this.panel3.Size = new System.Drawing.Size( 1031, 444 );
         this.panel3.TabIndex = 10;
         // 
         // label3
         // 
         this.label3.AutoSize = true;
         this.label3.Location = new System.Drawing.Point( 4, 6 );
         this.label3.Name = "label3";
         this.label3.Size = new System.Drawing.Size( 107, 13 );
         this.label3.TabIndex = 2;
         this.label3.Text = "Original Text Preview";
         // 
         // panel4
         // 
         this.panel4.Controls.Add( this.panel6 );
         this.panel4.Controls.Add( this.label9 );
         this.panel4.Controls.Add( this.previewModeComboBox );
         this.panel4.Controls.Add( this.label6 );
         this.panel4.Controls.Add( this.previewPlatformComboBox );
         this.panel4.Controls.Add( this.label4 );
         this.panel4.Dock = System.Windows.Forms.DockStyle.Fill;
         this.panel4.Location = new System.Drawing.Point( 3, 453 );
         this.panel4.Name = "panel4";
         this.panel4.Size = new System.Drawing.Size( 1031, 444 );
         this.panel4.TabIndex = 10;
         // 
         // panel6
         // 
         this.panel6.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                     | System.Windows.Forms.AnchorStyles.Left)
                     | System.Windows.Forms.AnchorStyles.Right)));
         this.panel6.Controls.Add( this.remappedPreview );
         this.panel6.Location = new System.Drawing.Point( 0, 27 );
         this.panel6.Name = "panel6";
         this.panel6.Size = new System.Drawing.Size( 1031, 414 );
         this.panel6.TabIndex = 13;
         // 
         // label9
         // 
         this.label9.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
         this.label9.AutoSize = true;
         this.label9.Location = new System.Drawing.Point( 599, 5 );
         this.label9.Name = "label9";
         this.label9.Size = new System.Drawing.Size( 75, 13 );
         this.label9.TabIndex = 12;
         this.label9.Text = "Preview Mode";
         // 
         // previewModeComboBox
         // 
         this.previewModeComboBox.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
         this.previewModeComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
         this.previewModeComboBox.FormattingEnabled = true;
         this.previewModeComboBox.Items.AddRange( new object[] {
            "Default",
            "MGS2 Codec",
            "MGS2 Codec (Japan)",
            "MGS3 Codec",
            "MGS3 Codec (Japan)"} );
         this.previewModeComboBox.Location = new System.Drawing.Point( 680, 2 );
         this.previewModeComboBox.Name = "previewModeComboBox";
         this.previewModeComboBox.Size = new System.Drawing.Size( 121, 21 );
         this.previewModeComboBox.TabIndex = 11;
         this.previewModeComboBox.SelectedIndexChanged += new System.EventHandler( this.OnPreviewModeChanged );
         // 
         // label6
         // 
         this.label6.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
         this.label6.AutoSize = true;
         this.label6.Location = new System.Drawing.Point( 815, 5 );
         this.label6.Name = "label6";
         this.label6.Size = new System.Drawing.Size( 86, 13 );
         this.label6.TabIndex = 10;
         this.label6.Text = "Preview Platform";
         // 
         // previewPlatformComboBox
         // 
         this.previewPlatformComboBox.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
         this.previewPlatformComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
         this.previewPlatformComboBox.FormattingEnabled = true;
         this.previewPlatformComboBox.Items.AddRange( new object[] {
            "PS3",
            "PS3 (Japan)",
            "X360",
            "Vita"} );
         this.previewPlatformComboBox.Location = new System.Drawing.Point( 908, 2 );
         this.previewPlatformComboBox.Name = "previewPlatformComboBox";
         this.previewPlatformComboBox.Size = new System.Drawing.Size( 121, 21 );
         this.previewPlatformComboBox.TabIndex = 9;
         this.previewPlatformComboBox.SelectedIndexChanged += new System.EventHandler( this.OnPreviewPlatformChanged );
         // 
         // label4
         // 
         this.label4.AutoSize = true;
         this.label4.Location = new System.Drawing.Point( 4, 5 );
         this.label4.Name = "label4";
         this.label4.Size = new System.Drawing.Size( 94, 13 );
         this.label4.TabIndex = 3;
         this.label4.Text = "New Text Preview";
         // 
         // panel2
         // 
         this.panel2.Controls.Add( this.label7 );
         this.panel2.Controls.Add( this.overridePlatformComboBox );
         this.panel2.Controls.Add( this.label5 );
         this.panel2.Controls.Add( this.removeOverrideButton );
         this.panel2.Controls.Add( this.remappedTextBox );
         this.panel2.Controls.Add( this.overrideButton );
         this.panel2.Dock = System.Windows.Forms.DockStyle.Fill;
         this.panel2.Location = new System.Drawing.Point( 3, 903 );
         this.panel2.Name = "panel2";
         this.panel2.Size = new System.Drawing.Size( 1031, 219 );
         this.panel2.TabIndex = 2;
         // 
         // label7
         // 
         this.label7.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
         this.label7.AutoSize = true;
         this.label7.Location = new System.Drawing.Point( 790, 5 );
         this.label7.Name = "label7";
         this.label7.Size = new System.Drawing.Size( 112, 13 );
         this.label7.TabIndex = 11;
         this.label7.Text = "Text Override Platform";
         // 
         // overridePlatformComboBox
         // 
         this.overridePlatformComboBox.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
         this.overridePlatformComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
         this.overridePlatformComboBox.FormattingEnabled = true;
         this.overridePlatformComboBox.Items.AddRange( new object[] {
            "Default",
            "PS3",
            "X360",
            "Vita",
            "KJP Default",
            "KJP PS3",
            "KJP X360",
            "KJP Vita"} );
         this.overridePlatformComboBox.Location = new System.Drawing.Point( 908, 0 );
         this.overridePlatformComboBox.Name = "overridePlatformComboBox";
         this.overridePlatformComboBox.Size = new System.Drawing.Size( 121, 21 );
         this.overridePlatformComboBox.TabIndex = 10;
         this.overridePlatformComboBox.SelectedIndexChanged += new System.EventHandler( this.OnOverridePlatformChanged );
         // 
         // label5
         // 
         this.label5.AutoSize = true;
         this.label5.Location = new System.Drawing.Point( 4, 0 );
         this.label5.Name = "label5";
         this.label5.Size = new System.Drawing.Size( 94, 13 );
         this.label5.TabIndex = 10;
         this.label5.Text = "Replacement Text";
         // 
         // removeOverrideButton
         // 
         this.removeOverrideButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
         this.removeOverrideButton.Enabled = false;
         this.removeOverrideButton.Location = new System.Drawing.Point( 657, 0 );
         this.removeOverrideButton.Name = "removeOverrideButton";
         this.removeOverrideButton.Size = new System.Drawing.Size( 121, 23 );
         this.removeOverrideButton.TabIndex = 1;
         this.removeOverrideButton.Text = "Remove Override";
         this.removeOverrideButton.UseVisualStyleBackColor = true;
         this.removeOverrideButton.Click += new System.EventHandler( this.OnRemoveTextOverride );
         // 
         // overrideButton
         // 
         this.overrideButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
         this.overrideButton.Enabled = false;
         this.overrideButton.Location = new System.Drawing.Point( 530, 0 );
         this.overrideButton.Name = "overrideButton";
         this.overrideButton.Size = new System.Drawing.Size( 121, 23 );
         this.overrideButton.TabIndex = 1;
         this.overrideButton.Text = "Override Text";
         this.overrideButton.UseVisualStyleBackColor = true;
         this.overrideButton.Click += new System.EventHandler( this.OnTextOverride );
         // 
         // fileListView
         // 
         this.fileListView.Columns.AddRange( new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader2} );
         this.fileListView.ContextMenuStrip = this.contextMenuStrip1;
         this.fileListView.Dock = System.Windows.Forms.DockStyle.Fill;
         this.fileListView.FullRowSelect = true;
         this.fileListView.Location = new System.Drawing.Point( 3, 977 );
         this.fileListView.MultiSelect = false;
         this.fileListView.Name = "fileListView";
         this.fileListView.Size = new System.Drawing.Size( 512, 145 );
         this.fileListView.TabIndex = 4;
         this.fileListView.UseCompatibleStateImageBehavior = false;
         this.fileListView.View = System.Windows.Forms.View.Details;
         this.fileListView.VirtualMode = true;
         this.fileListView.RetrieveVirtualItem += new System.Windows.Forms.RetrieveVirtualItemEventHandler( this.OnRetrieveFileItem );
         // 
         // columnHeader2
         // 
         this.columnHeader2.Text = "File";
         this.columnHeader2.Width = 465;
         // 
         // contextMenuStrip1
         // 
         this.contextMenuStrip1.Items.AddRange( new System.Windows.Forms.ToolStripItem[] {
            this.setFileFilterToolStripMenuItem} );
         this.contextMenuStrip1.Name = "contextMenuStrip1";
         this.contextMenuStrip1.Size = new System.Drawing.Size( 141, 26 );
         this.contextMenuStrip1.Opening += new System.ComponentModel.CancelEventHandler( this.OnFileMenuOpening );
         // 
         // setFileFilterToolStripMenuItem
         // 
         this.setFileFilterToolStripMenuItem.Name = "setFileFilterToolStripMenuItem";
         this.setFileFilterToolStripMenuItem.Size = new System.Drawing.Size( 140, 22 );
         this.setFileFilterToolStripMenuItem.Text = "Set File Filter";
         this.setFileFilterToolStripMenuItem.Click += new System.EventHandler( this.OnSetFileFilter );
         // 
         // stringFilterTextBox
         // 
         this.stringFilterTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                     | System.Windows.Forms.AnchorStyles.Right)));
         this.stringFilterTextBox.Location = new System.Drawing.Point( 119, 17 );
         this.stringFilterTextBox.Name = "stringFilterTextBox";
         this.stringFilterTextBox.Size = new System.Drawing.Size( 260, 20 );
         this.stringFilterTextBox.TabIndex = 5;
         this.stringFilterTextBox.TextChanged += new System.EventHandler( this.OnTextFilterChanged );
         // 
         // updateTextListTimer
         // 
         this.updateTextListTimer.Interval = 50;
         this.updateTextListTimer.Tick += new System.EventHandler( this.OnUpdateTextItemsTimer );
         // 
         // tableLayoutPanel2
         // 
         this.tableLayoutPanel2.ColumnCount = 1;
         this.tableLayoutPanel2.ColumnStyles.Add( new System.Windows.Forms.ColumnStyle( System.Windows.Forms.SizeType.Percent, 100F ) );
         this.tableLayoutPanel2.Controls.Add( this.groupBox1, 0, 0 );
         this.tableLayoutPanel2.Controls.Add( this.panel5, 0, 2 );
         this.tableLayoutPanel2.Controls.Add( this.panel1, 0, 1 );
         this.tableLayoutPanel2.Controls.Add( this.fileListView, 0, 3 );
         this.tableLayoutPanel2.Dock = System.Windows.Forms.DockStyle.Fill;
         this.tableLayoutPanel2.Location = new System.Drawing.Point( 0, 0 );
         this.tableLayoutPanel2.Name = "tableLayoutPanel2";
         this.tableLayoutPanel2.RowCount = 4;
         this.tableLayoutPanel2.RowStyles.Add( new System.Windows.Forms.RowStyle( System.Windows.Forms.SizeType.Absolute, 150F ) );
         this.tableLayoutPanel2.RowStyles.Add( new System.Windows.Forms.RowStyle( System.Windows.Forms.SizeType.Percent, 62.88056F ) );
         this.tableLayoutPanel2.RowStyles.Add( new System.Windows.Forms.RowStyle( System.Windows.Forms.SizeType.Percent, 37.11944F ) );
         this.tableLayoutPanel2.RowStyles.Add( new System.Windows.Forms.RowStyle( System.Windows.Forms.SizeType.Absolute, 150F ) );
         this.tableLayoutPanel2.Size = new System.Drawing.Size( 518, 1125 );
         this.tableLayoutPanel2.TabIndex = 6;
         // 
         // groupBox1
         // 
         this.groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                     | System.Windows.Forms.AnchorStyles.Left)
                     | System.Windows.Forms.AnchorStyles.Right)));
         this.groupBox1.Controls.Add( this.label8 );
         this.groupBox1.Controls.Add( this.infoSearchTextBox );
         this.groupBox1.Controls.Add( this.searchFilterComboBox );
         this.groupBox1.Controls.Add( this.matchCaseCheckBox );
         this.groupBox1.Controls.Add( this.showOverrideCheckBox );
         this.groupBox1.Controls.Add( this.label2 );
         this.groupBox1.Controls.Add( this.label1 );
         this.groupBox1.Controls.Add( this.fileFilterTextBox );
         this.groupBox1.Controls.Add( this.stringFilterTextBox );
         this.groupBox1.Location = new System.Drawing.Point( 3, 3 );
         this.groupBox1.Name = "groupBox1";
         this.groupBox1.Size = new System.Drawing.Size( 512, 144 );
         this.groupBox1.TabIndex = 6;
         this.groupBox1.TabStop = false;
         this.groupBox1.Text = "Filter";
         // 
         // label8
         // 
         this.label8.AutoSize = true;
         this.label8.Location = new System.Drawing.Point( 56, 72 );
         this.label8.Name = "label8";
         this.label8.Size = new System.Drawing.Size( 62, 13 );
         this.label8.TabIndex = 10;
         this.label8.Text = "Info Search";
         this.label8.TextAlign = System.Drawing.ContentAlignment.TopRight;
         // 
         // infoSearchTextBox
         // 
         this.infoSearchTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                     | System.Windows.Forms.AnchorStyles.Right)));
         this.infoSearchTextBox.Location = new System.Drawing.Point( 119, 69 );
         this.infoSearchTextBox.Name = "infoSearchTextBox";
         this.infoSearchTextBox.Size = new System.Drawing.Size( 260, 20 );
         this.infoSearchTextBox.TabIndex = 9;
         this.infoSearchTextBox.TextChanged += new System.EventHandler( this.OnTextFilterChanged );
         // 
         // searchFilterComboBox
         // 
         this.searchFilterComboBox.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
         this.searchFilterComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
         this.searchFilterComboBox.FormattingEnabled = true;
         this.searchFilterComboBox.Items.AddRange( new object[] {
            "Original Text",
            "Remapped Text",
            "Remapped (PS3)",
            "Remapped (Vita)"} );
         this.searchFilterComboBox.Location = new System.Drawing.Point( 385, 16 );
         this.searchFilterComboBox.Name = "searchFilterComboBox";
         this.searchFilterComboBox.Size = new System.Drawing.Size( 121, 21 );
         this.searchFilterComboBox.TabIndex = 8;
         this.searchFilterComboBox.SelectedIndexChanged += new System.EventHandler( this.OnSearchSourceChanged );
         // 
         // matchCaseCheckBox
         // 
         this.matchCaseCheckBox.AutoSize = true;
         this.matchCaseCheckBox.CheckAlign = System.Drawing.ContentAlignment.MiddleRight;
         this.matchCaseCheckBox.Location = new System.Drawing.Point( 51, 95 );
         this.matchCaseCheckBox.Name = "matchCaseCheckBox";
         this.matchCaseCheckBox.Size = new System.Drawing.Size( 83, 17 );
         this.matchCaseCheckBox.TabIndex = 7;
         this.matchCaseCheckBox.Text = "Match Case";
         this.matchCaseCheckBox.UseVisualStyleBackColor = true;
         this.matchCaseCheckBox.CheckedChanged += new System.EventHandler( this.filterCheckBoxChanged );
         // 
         // showOverrideCheckBox
         // 
         this.showOverrideCheckBox.AutoSize = true;
         this.showOverrideCheckBox.CheckAlign = System.Drawing.ContentAlignment.MiddleRight;
         this.showOverrideCheckBox.Location = new System.Drawing.Point( 9, 117 );
         this.showOverrideCheckBox.Name = "showOverrideCheckBox";
         this.showOverrideCheckBox.Size = new System.Drawing.Size( 125, 17 );
         this.showOverrideCheckBox.TabIndex = 7;
         this.showOverrideCheckBox.Text = "Show Overrides Only";
         this.showOverrideCheckBox.UseVisualStyleBackColor = true;
         this.showOverrideCheckBox.CheckedChanged += new System.EventHandler( this.filterCheckBoxChanged );
         // 
         // label2
         // 
         this.label2.AutoSize = true;
         this.label2.Location = new System.Drawing.Point( 56, 46 );
         this.label2.Name = "label2";
         this.label2.Size = new System.Drawing.Size( 60, 13 );
         this.label2.TabIndex = 6;
         this.label2.Text = "File Search";
         this.label2.TextAlign = System.Drawing.ContentAlignment.TopRight;
         // 
         // label1
         // 
         this.label1.AutoSize = true;
         this.label1.Location = new System.Drawing.Point( 51, 19 );
         this.label1.Name = "label1";
         this.label1.Size = new System.Drawing.Size( 65, 13 );
         this.label1.TabIndex = 6;
         this.label1.Text = "Text Search";
         this.label1.TextAlign = System.Drawing.ContentAlignment.TopRight;
         // 
         // fileFilterTextBox
         // 
         this.fileFilterTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                     | System.Windows.Forms.AnchorStyles.Right)));
         this.fileFilterTextBox.Location = new System.Drawing.Point( 119, 43 );
         this.fileFilterTextBox.Name = "fileFilterTextBox";
         this.fileFilterTextBox.Size = new System.Drawing.Size( 260, 20 );
         this.fileFilterTextBox.TabIndex = 5;
         this.fileFilterTextBox.TextChanged += new System.EventHandler( this.OnTextFilterChanged );
         // 
         // panel5
         // 
         this.panel5.Controls.Add( this.groupBox2 );
         this.panel5.Dock = System.Windows.Forms.DockStyle.Fill;
         this.panel5.Location = new System.Drawing.Point( 3, 671 );
         this.panel5.Name = "panel5";
         this.panel5.Size = new System.Drawing.Size( 512, 300 );
         this.panel5.TabIndex = 10;
         // 
         // groupBox2
         // 
         this.groupBox2.Controls.Add( this.infoRichTextBox );
         this.groupBox2.Dock = System.Windows.Forms.DockStyle.Fill;
         this.groupBox2.Location = new System.Drawing.Point( 0, 0 );
         this.groupBox2.Name = "groupBox2";
         this.groupBox2.Size = new System.Drawing.Size( 512, 300 );
         this.groupBox2.TabIndex = 1;
         this.groupBox2.TabStop = false;
         this.groupBox2.Text = "Info";
         // 
         // infoRichTextBox
         // 
         this.infoRichTextBox.Dock = System.Windows.Forms.DockStyle.Fill;
         this.infoRichTextBox.Enabled = false;
         this.infoRichTextBox.Location = new System.Drawing.Point( 3, 16 );
         this.infoRichTextBox.Name = "infoRichTextBox";
         this.infoRichTextBox.Size = new System.Drawing.Size( 506, 281 );
         this.infoRichTextBox.TabIndex = 0;
         this.infoRichTextBox.Text = "";
         this.infoRichTextBox.TextChanged += new System.EventHandler( this.OnInfoChanged );
         // 
         // panel1
         // 
         this.panel1.Controls.Add( this.button6 );
         this.panel1.Controls.Add( this.button4 );
         this.panel1.Controls.Add( this.button3 );
         this.panel1.Controls.Add( this.button2 );
         this.panel1.Controls.Add( this.button5 );
         this.panel1.Controls.Add( this.button1 );
         this.panel1.Controls.Add( this.textListView );
         this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
         this.panel1.Location = new System.Drawing.Point( 3, 153 );
         this.panel1.Name = "panel1";
         this.panel1.Size = new System.Drawing.Size( 512, 512 );
         this.panel1.TabIndex = 9;
         // 
         // button6
         // 
         this.button6.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
         this.button6.Location = new System.Drawing.Point( 378, 457 );
         this.button6.Name = "button6";
         this.button6.Size = new System.Drawing.Size( 117, 23 );
         this.button6.TabIndex = 2;
         this.button6.Text = "Generate Code";
         this.button6.UseVisualStyleBackColor = true;
         this.button6.Click += new System.EventHandler( this.OnGenerateCode );
         // 
         // button4
         // 
         this.button4.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
         this.button4.Location = new System.Drawing.Point( 132, 486 );
         this.button4.Name = "button4";
         this.button4.Size = new System.Drawing.Size( 117, 23 );
         this.button4.TabIndex = 2;
         this.button4.Text = "Remove KJP Ovr.";
         this.button4.UseVisualStyleBackColor = true;
         this.button4.Click += new System.EventHandler( this.OnRemoveKJPOverrides );
         // 
         // button3
         // 
         this.button3.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
         this.button3.Location = new System.Drawing.Point( 9, 486 );
         this.button3.Name = "button3";
         this.button3.Size = new System.Drawing.Size( 117, 23 );
         this.button3.TabIndex = 2;
         this.button3.Text = "Process Localization";
         this.button3.UseVisualStyleBackColor = true;
         this.button3.Click += new System.EventHandler( this.OnImportOverrides );
         // 
         // button2
         // 
         this.button2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
         this.button2.Location = new System.Drawing.Point( 9, 457 );
         this.button2.Name = "button2";
         this.button2.Size = new System.Drawing.Size( 117, 23 );
         this.button2.TabIndex = 1;
         this.button2.Text = "Export All";
         this.toolTip1.SetToolTip( this.button2, "Exports all strings into csv." );
         this.button2.UseVisualStyleBackColor = true;
         this.button2.Click += new System.EventHandler( this.OnExportAll );
         // 
         // button5
         // 
         this.button5.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
         this.button5.Location = new System.Drawing.Point( 132, 457 );
         this.button5.Name = "button5";
         this.button5.Size = new System.Drawing.Size( 117, 23 );
         this.button5.TabIndex = 1;
         this.button5.Text = "Export (Filter:File)";
         this.toolTip1.SetToolTip( this.button5, "Export strings into .csv filtered by file names." );
         this.button5.UseVisualStyleBackColor = true;
         this.button5.Click += new System.EventHandler( this.OnExportFilteredByFile );
         // 
         // button1
         // 
         this.button1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
         this.button1.Location = new System.Drawing.Point( 255, 457 );
         this.button1.Name = "button1";
         this.button1.Size = new System.Drawing.Size( 117, 23 );
         this.button1.TabIndex = 1;
         this.button1.Text = "Export (Filter:Terms)";
         this.toolTip1.SetToolTip( this.button1, "Export strings into .csv filtered by \"SearchTerms.txt\"." );
         this.button1.UseVisualStyleBackColor = true;
         this.button1.Click += new System.EventHandler( this.OnExportFiltered );
         // 
         // splitContainer1
         // 
         this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
         this.splitContainer1.Location = new System.Drawing.Point( 0, 0 );
         this.splitContainer1.Name = "splitContainer1";
         // 
         // splitContainer1.Panel1
         // 
         this.splitContainer1.Panel1.Controls.Add( this.tableLayoutPanel2 );
         // 
         // splitContainer1.Panel2
         // 
         this.splitContainer1.Panel2.Controls.Add( this.tableLayoutPanel1 );
         this.splitContainer1.Size = new System.Drawing.Size( 1559, 1125 );
         this.splitContainer1.SplitterDistance = 518;
         this.splitContainer1.TabIndex = 7;
         // 
         // saveRemapTableTimer
         // 
         this.saveRemapTableTimer.Interval = 1000;
         this.saveRemapTableTimer.Tick += new System.EventHandler( this.OnSaveRemapTimer );
         // 
         // StringEditor
         // 
         this.AutoScaleDimensions = new System.Drawing.SizeF( 6F, 13F );
         this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
         this.ClientSize = new System.Drawing.Size( 1559, 1125 );
         this.Controls.Add( this.splitContainer1 );
         this.Name = "StringEditor";
         this.Text = "StringEditor";
         ((System.ComponentModel.ISupportInitialize)(this.originalPreview)).EndInit();
         ((System.ComponentModel.ISupportInitialize)(this.remappedPreview)).EndInit();
         this.tableLayoutPanel1.ResumeLayout( false );
         this.panel3.ResumeLayout( false );
         this.panel3.PerformLayout();
         this.panel4.ResumeLayout( false );
         this.panel4.PerformLayout();
         this.panel6.ResumeLayout( false );
         this.panel2.ResumeLayout( false );
         this.panel2.PerformLayout();
         this.contextMenuStrip1.ResumeLayout( false );
         this.tableLayoutPanel2.ResumeLayout( false );
         this.groupBox1.ResumeLayout( false );
         this.groupBox1.PerformLayout();
         this.panel5.ResumeLayout( false );
         this.groupBox2.ResumeLayout( false );
         this.panel1.ResumeLayout( false );
         this.splitContainer1.Panel1.ResumeLayout( false );
         this.splitContainer1.Panel2.ResumeLayout( false );
         this.splitContainer1.ResumeLayout( false );
         this.ResumeLayout( false );

      }

      #endregion

      private System.Windows.Forms.ListView textListView;
      private System.Windows.Forms.PictureBox originalPreview;
      private System.Windows.Forms.RichTextBox remappedTextBox;
      private System.Windows.Forms.ColumnHeader columnHeader1;
      private System.Windows.Forms.PictureBox remappedPreview;
      private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
      private System.Windows.Forms.ListView fileListView;
      private System.Windows.Forms.ColumnHeader columnHeader2;
      private System.Windows.Forms.TextBox stringFilterTextBox;
      private System.Windows.Forms.Timer updateTextListTimer;
      private System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
      private System.Windows.Forms.SplitContainer splitContainer1;
      private System.Windows.Forms.GroupBox groupBox1;
      private System.Windows.Forms.Label label1;
      private System.Windows.Forms.CheckBox showOverrideCheckBox;
      private System.Windows.Forms.ComboBox searchFilterComboBox;
      private System.Windows.Forms.Panel panel1;
      private System.Windows.Forms.Button removeOverrideButton;
      private System.Windows.Forms.Button overrideButton;
      private System.Windows.Forms.Panel panel2;
      private System.Windows.Forms.ComboBox previewPlatformComboBox;
      private System.Windows.Forms.Timer saveRemapTableTimer;
      private System.Windows.Forms.CheckBox matchCaseCheckBox;
      private System.Windows.Forms.Button button1;
      private System.Windows.Forms.Label label2;
      private System.Windows.Forms.TextBox fileFilterTextBox;
      private System.Windows.Forms.ContextMenuStrip contextMenuStrip1;
      private System.Windows.Forms.ToolStripMenuItem setFileFilterToolStripMenuItem;
      private System.Windows.Forms.Button button2;
      private System.Windows.Forms.ToolTip toolTip1;
      private System.Windows.Forms.Button button5;
      private System.Windows.Forms.Panel panel3;
      private System.Windows.Forms.Label label3;
      private System.Windows.Forms.Panel panel4;
      private System.Windows.Forms.Label label4;
      private System.Windows.Forms.ComboBox overridePlatformComboBox;
      private System.Windows.Forms.Label label5;
      private System.Windows.Forms.Label label6;
      private System.Windows.Forms.Label label7;
      private System.Windows.Forms.ColumnHeader columnHeader3;
      private System.Windows.Forms.Panel panel5;
      private System.Windows.Forms.GroupBox groupBox2;
      private System.Windows.Forms.RichTextBox infoRichTextBox;
      private System.Windows.Forms.Label label8;
      private System.Windows.Forms.TextBox infoSearchTextBox;
      private System.Windows.Forms.Label label9;
      private System.Windows.Forms.ComboBox previewModeComboBox;
      private System.Windows.Forms.Panel panel6;
      private System.Windows.Forms.Button button3;
      private System.Windows.Forms.Button button4;
      private System.Windows.Forms.Button button6;
   }
}