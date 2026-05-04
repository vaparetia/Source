namespace TextureViewer
{
   partial class TextureViewer
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
         System.Windows.Forms.ToolStripMenuItem searchToolStripMenuItem;
         this.pictureBox1 = new System.Windows.Forms.PictureBox();
         this.tagListBox = new System.Windows.Forms.ListView();
         this.parseFileWorker = new System.ComponentModel.BackgroundWorker();
         this.statusStrip1 = new System.Windows.Forms.StatusStrip();
         this.toolStripProgressBar1 = new System.Windows.Forms.ToolStripProgressBar();
         this.toolStripStatusLabel1 = new System.Windows.Forms.ToolStripStatusLabel();
         this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
         this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
         this.columnHeader2 = new System.Windows.Forms.ColumnHeader();
         this.listView1 = new System.Windows.Forms.ListView();
         this.columnHeader3 = new System.Windows.Forms.ColumnHeader();
         this.menuStrip1 = new System.Windows.Forms.MenuStrip();
         this.toolsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.calculateStatsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.createFlatListToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.searchBox = new System.Windows.Forms.ToolStripTextBox();
         this.lblTagLabel = new System.Windows.Forms.Label();
         this.txtNewTagBox = new System.Windows.Forms.TextBox();
         this.flatListMaker = new System.ComponentModel.BackgroundWorker();
         this.flowLayoutPanel1 = new System.Windows.Forms.FlowLayoutPanel();
         this.infoTextBox = new System.Windows.Forms.RichTextBox();
         this.rclickMenu = new System.Windows.Forms.ContextMenuStrip(this.components);
         this.rightClickinToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.txt_additionalText = new System.Windows.Forms.TextBox();
         this.label1 = new System.Windows.Forms.Label();
         this.helpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         searchToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
         this.statusStrip1.SuspendLayout();
         this.menuStrip1.SuspendLayout();
         this.rclickMenu.SuspendLayout();
         this.SuspendLayout();
         // 
         // searchToolStripMenuItem
         // 
         searchToolStripMenuItem.Alignment = System.Windows.Forms.ToolStripItemAlignment.Right;
         searchToolStripMenuItem.Name = "searchToolStripMenuItem";
         searchToolStripMenuItem.Size = new System.Drawing.Size(54, 23);
         searchToolStripMenuItem.Text = "Search";
         // 
         // pictureBox1
         // 
         this.pictureBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                     | System.Windows.Forms.AnchorStyles.Left)
                     | System.Windows.Forms.AnchorStyles.Right)));
         this.pictureBox1.BackColor = System.Drawing.SystemColors.ControlDarkDark;
         this.pictureBox1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
         this.pictureBox1.Location = new System.Drawing.Point(477, 343);
         this.pictureBox1.Name = "pictureBox1";
         this.pictureBox1.Size = new System.Drawing.Size(806, 577);
         this.pictureBox1.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
         this.pictureBox1.TabIndex = 0;
         this.pictureBox1.TabStop = false;
         // 
         // tagListBox
         // 
         this.tagListBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                     | System.Windows.Forms.AnchorStyles.Right)));
         this.tagListBox.Location = new System.Drawing.Point(1289, 359);
         this.tagListBox.Name = "tagListBox";
         this.tagListBox.Size = new System.Drawing.Size(115, 535);
         this.tagListBox.TabIndex = 0;
         this.tagListBox.UseCompatibleStateImageBehavior = false;
         this.tagListBox.View = System.Windows.Forms.View.List;
         // 
         // parseFileWorker
         // 
         this.parseFileWorker.WorkerReportsProgress = true;
         this.parseFileWorker.WorkerSupportsCancellation = true;
         this.parseFileWorker.DoWork += new System.ComponentModel.DoWorkEventHandler(this.OnParseFiles);
         this.parseFileWorker.RunWorkerCompleted += new System.ComponentModel.RunWorkerCompletedEventHandler(this.OnParseCompleted);
         this.parseFileWorker.ProgressChanged += new System.ComponentModel.ProgressChangedEventHandler(this.OnParseProgressUpdate);
         // 
         // statusStrip1
         // 
         this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripProgressBar1,
            this.toolStripStatusLabel1});
         this.statusStrip1.Location = new System.Drawing.Point(0, 924);
         this.statusStrip1.Name = "statusStrip1";
         this.statusStrip1.Size = new System.Drawing.Size(1416, 22);
         this.statusStrip1.TabIndex = 2;
         this.statusStrip1.Text = "statusStrip1";
         // 
         // toolStripProgressBar1
         // 
         this.toolStripProgressBar1.Name = "toolStripProgressBar1";
         this.toolStripProgressBar1.Size = new System.Drawing.Size(100, 16);
         // 
         // toolStripStatusLabel1
         // 
         this.toolStripStatusLabel1.Name = "toolStripStatusLabel1";
         this.toolStripStatusLabel1.Size = new System.Drawing.Size(102, 17);
         this.toolStripStatusLabel1.Text = "Parsing Textures...";
         // 
         // columnHeader1
         // 
         this.columnHeader1.Text = "Name";
         this.columnHeader1.Width = 85;
         // 
         // columnHeader2
         // 
         this.columnHeader2.Text = "Unique Count";
         this.columnHeader2.Width = 115;
         // 
         // listView1
         // 
         this.listView1.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader3,
            this.columnHeader2});
         this.listView1.Dock = System.Windows.Forms.DockStyle.Left;
         this.listView1.Location = new System.Drawing.Point(0, 27);
         this.listView1.Name = "listView1";
         this.listView1.Size = new System.Drawing.Size(468, 897);
         this.listView1.TabIndex = 0;
         this.listView1.UseCompatibleStateImageBehavior = false;
         this.listView1.View = System.Windows.Forms.View.Details;
         this.listView1.SelectedIndexChanged += new System.EventHandler(this.OnItemSelected);
         // 
         // columnHeader3
         // 
         this.columnHeader3.Text = "P4 Status";
         // 
         // menuStrip1
         // 
         this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolsToolStripMenuItem,
            this.searchBox,
            searchToolStripMenuItem,
            this.helpToolStripMenuItem});
         this.menuStrip1.Location = new System.Drawing.Point(0, 0);
         this.menuStrip1.Name = "menuStrip1";
         this.menuStrip1.Size = new System.Drawing.Size(1416, 27);
         this.menuStrip1.TabIndex = 3;
         this.menuStrip1.Text = "menuStrip1";
         // 
         // toolsToolStripMenuItem
         // 
         this.toolsToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.calculateStatsToolStripMenuItem,
            this.createFlatListToolStripMenuItem});
         this.toolsToolStripMenuItem.Name = "toolsToolStripMenuItem";
         this.toolsToolStripMenuItem.Size = new System.Drawing.Size(48, 23);
         this.toolsToolStripMenuItem.Text = "Tools";
         // 
         // calculateStatsToolStripMenuItem
         // 
         this.calculateStatsToolStripMenuItem.Name = "calculateStatsToolStripMenuItem";
         this.calculateStatsToolStripMenuItem.Size = new System.Drawing.Size(151, 22);
         this.calculateStatsToolStripMenuItem.Text = "Calculate Stats";
         this.calculateStatsToolStripMenuItem.Click += new System.EventHandler(this.calculateStatsToolStripMenuItem_Click);
         // 
         // createFlatListToolStripMenuItem
         // 
         this.createFlatListToolStripMenuItem.Name = "createFlatListToolStripMenuItem";
         this.createFlatListToolStripMenuItem.Size = new System.Drawing.Size(151, 22);
         this.createFlatListToolStripMenuItem.Text = "Create &Flat List";
         this.createFlatListToolStripMenuItem.Click += new System.EventHandler(this.createFlatListToolStripMenuItem_Click);
         // 
         // searchBox
         // 
         this.searchBox.Alignment = System.Windows.Forms.ToolStripItemAlignment.Right;
         this.searchBox.BackColor = System.Drawing.SystemColors.Menu;
         this.searchBox.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
         this.searchBox.Name = "searchBox";
         this.searchBox.Size = new System.Drawing.Size(128, 23);
         this.searchBox.TextChanged += new System.EventHandler(this.searchBox_TextChanged);
         // 
         // lblTagLabel
         // 
         this.lblTagLabel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                     | System.Windows.Forms.AnchorStyles.Right)));
         this.lblTagLabel.AutoSize = true;
         this.lblTagLabel.Location = new System.Drawing.Point(1326, 343);
         this.lblTagLabel.Name = "lblTagLabel";
         this.lblTagLabel.Size = new System.Drawing.Size(31, 13);
         this.lblTagLabel.TabIndex = 4;
         this.lblTagLabel.Text = "Tags";
         this.lblTagLabel.Click += new System.EventHandler(this.lblTagLabel_Click);
         // 
         // txtNewTagBox
         // 
         this.txtNewTagBox.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
         this.txtNewTagBox.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
         this.txtNewTagBox.Location = new System.Drawing.Point(1289, 900);
         this.txtNewTagBox.Name = "txtNewTagBox";
         this.txtNewTagBox.Size = new System.Drawing.Size(115, 20);
         this.txtNewTagBox.TabIndex = 5;
         this.txtNewTagBox.TextChanged += new System.EventHandler(this.txtNewTagBox_TextChanged);
         this.txtNewTagBox.KeyDown += new System.Windows.Forms.KeyEventHandler(this.AcceptNewTag);
         // 
         // flatListMaker
         // 
         this.flatListMaker.WorkerReportsProgress = true;
         this.flatListMaker.WorkerSupportsCancellation = true;
         this.flatListMaker.DoWork += new System.ComponentModel.DoWorkEventHandler(this.OnMakeFlatList);
         this.flatListMaker.RunWorkerCompleted += new System.ComponentModel.RunWorkerCompletedEventHandler(this.OnMakeFlatListCompleted);
         this.flatListMaker.ProgressChanged += new System.ComponentModel.ProgressChangedEventHandler(this.OnMakeFlatListUpdate);
         // 
         // flowLayoutPanel1
         // 
         this.flowLayoutPanel1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                     | System.Windows.Forms.AnchorStyles.Right)));
         this.flowLayoutPanel1.BackColor = System.Drawing.Color.White;
         this.flowLayoutPanel1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
         this.flowLayoutPanel1.Location = new System.Drawing.Point(477, 27);
         this.flowLayoutPanel1.Name = "flowLayoutPanel1";
         this.flowLayoutPanel1.Size = new System.Drawing.Size(927, 138);
         this.flowLayoutPanel1.TabIndex = 6;
         // 
         // infoTextBox
         // 
         this.infoTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                     | System.Windows.Forms.AnchorStyles.Right)));
         this.infoTextBox.DetectUrls = false;
         this.infoTextBox.Location = new System.Drawing.Point(477, 171);
         this.infoTextBox.Name = "infoTextBox";
         this.infoTextBox.ReadOnly = true;
         this.infoTextBox.Size = new System.Drawing.Size(927, 140);
         this.infoTextBox.TabIndex = 7;
         this.infoTextBox.Text = "";
         // 
         // rclickMenu
         // 
         this.rclickMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.rightClickinToolStripMenuItem});
         this.rclickMenu.Name = "rclickMenu";
         this.rclickMenu.Size = new System.Drawing.Size(140, 26);
         // 
         // rightClickinToolStripMenuItem
         // 
         this.rightClickinToolStripMenuItem.Name = "rightClickinToolStripMenuItem";
         this.rightClickinToolStripMenuItem.Size = new System.Drawing.Size(139, 22);
         this.rightClickinToolStripMenuItem.Text = "Right clickin";
         this.rightClickinToolStripMenuItem.Click += new System.EventHandler(this.rightClickinToolStripMenuItem_Click);
         // 
         // txt_additionalText
         // 
         this.txt_additionalText.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                     | System.Windows.Forms.AnchorStyles.Right)));
         this.txt_additionalText.Location = new System.Drawing.Point(555, 317);
         this.txt_additionalText.Name = "txt_additionalText";
         this.txt_additionalText.Size = new System.Drawing.Size(849, 20);
         this.txt_additionalText.TabIndex = 8;
         this.txt_additionalText.KeyDown += new System.Windows.Forms.KeyEventHandler(this.AcceptNewAdditionalText);
         // 
         // label1
         // 
         this.label1.AutoSize = true;
         this.label1.Location = new System.Drawing.Point(474, 317);
         this.label1.Name = "label1";
         this.label1.Size = new System.Drawing.Size(75, 13);
         this.label1.TabIndex = 9;
         this.label1.Text = "Addtional Text";
         // 
         // helpToolStripMenuItem
         // 
         this.helpToolStripMenuItem.Name = "helpToolStripMenuItem";
         this.helpToolStripMenuItem.Size = new System.Drawing.Size(44, 23);
         this.helpToolStripMenuItem.Text = "Help";
         // 
         // TextureViewer
         // 
         this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
         this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
         this.ClientSize = new System.Drawing.Size(1416, 946);
         this.Controls.Add(this.txt_additionalText);
         this.Controls.Add(this.label1);
         this.Controls.Add(this.infoTextBox);
         this.Controls.Add(this.flowLayoutPanel1);
         this.Controls.Add(this.txtNewTagBox);
         this.Controls.Add(this.pictureBox1);
         this.Controls.Add(this.tagListBox);
         this.Controls.Add(this.lblTagLabel);
         this.Controls.Add(this.listView1);
         this.Controls.Add(this.statusStrip1);
         this.Controls.Add(this.menuStrip1);
         this.Name = "TextureViewer";
         this.Text = "Texture Viewer";
         this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.OnClosing);
         ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
         this.statusStrip1.ResumeLayout(false);
         this.statusStrip1.PerformLayout();
         this.menuStrip1.ResumeLayout(false);
         this.menuStrip1.PerformLayout();
         this.rclickMenu.ResumeLayout(false);
         this.ResumeLayout(false);
         this.PerformLayout();

      }

      #endregion

      private System.ComponentModel.BackgroundWorker parseFileWorker;
      private System.Windows.Forms.StatusStrip statusStrip1;
      private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel1;
      private System.Windows.Forms.ToolStripProgressBar toolStripProgressBar1;
      private System.Windows.Forms.PictureBox pictureBox1;
      private System.Windows.Forms.ToolTip toolTip1;
      private System.Windows.Forms.ColumnHeader columnHeader1;
      private System.Windows.Forms.ColumnHeader columnHeader2;
      private System.Windows.Forms.ListView listView1;
      private System.Windows.Forms.MenuStrip menuStrip1;
      private System.Windows.Forms.ToolStripMenuItem toolsToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem calculateStatsToolStripMenuItem;
      private System.Windows.Forms.ToolStripTextBox searchBox;
      private System.Windows.Forms.ListView tagListBox;
      private System.Windows.Forms.Label lblTagLabel;
      private System.Windows.Forms.TextBox txtNewTagBox;
      private System.Windows.Forms.ToolStripMenuItem createFlatListToolStripMenuItem;
      private System.ComponentModel.BackgroundWorker flatListMaker;
      private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel1;
      private System.Windows.Forms.RichTextBox infoTextBox;
      private System.Windows.Forms.ContextMenuStrip rclickMenu;
      private System.Windows.Forms.ToolStripMenuItem rightClickinToolStripMenuItem;
      private System.Windows.Forms.TextBox txt_additionalText;
      private System.Windows.Forms.Label label1;
      private System.Windows.Forms.ColumnHeader columnHeader3;
      private System.Windows.Forms.ToolStripMenuItem helpToolStripMenuItem;
   }
}

