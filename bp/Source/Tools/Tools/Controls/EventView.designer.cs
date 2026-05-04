namespace Tools.Controls
{
   partial class EventView
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

      #region Component Designer generated code

      /// <summary> 
      /// Required method for Designer support - do not modify 
      /// the contents of this method with the code editor.
      /// </summary>
      private void InitializeComponent()
      {
         this.components = new System.ComponentModel.Container();
         System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle1 = new System.Windows.Forms.DataGridViewCellStyle();
         System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(EventView));
         this.dataGridView1 = new System.Windows.Forms.DataGridView();
         this.mDisplayNameColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
         this.mEventsColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
         this.mNameColumnHidden = new System.Windows.Forms.DataGridViewTextBoxColumn();
         this.mTagColumnHidden = new System.Windows.Forms.DataGridViewTextBoxColumn();
         this.mToolBar = new Tools.Controls.ToolStripEx();
         this.toolStripButton2 = new System.Windows.Forms.ToolStripButton();
         this.unselectedImageList = new System.Windows.Forms.ImageList(this.components);
         this.selectedImageList = new System.Windows.Forms.ImageList(this.components);
         ((System.ComponentModel.ISupportInitialize)(this.dataGridView1)).BeginInit();
         this.mToolBar.SuspendLayout();
         this.SuspendLayout();
         // 
         // dataGridView1
         // 
         this.dataGridView1.AllowUserToAddRows = false;
         this.dataGridView1.AllowUserToDeleteRows = false;
         this.dataGridView1.AllowUserToResizeColumns = false;
         this.dataGridView1.AllowUserToResizeRows = false;
         dataGridViewCellStyle1.BackColor = System.Drawing.Color.White;
         this.dataGridView1.AlternatingRowsDefaultCellStyle = dataGridViewCellStyle1;
         this.dataGridView1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                     | System.Windows.Forms.AnchorStyles.Left)
                     | System.Windows.Forms.AnchorStyles.Right)));
         this.dataGridView1.BackgroundColor = System.Drawing.SystemColors.Control;
         this.dataGridView1.BorderStyle = System.Windows.Forms.BorderStyle.None;
         this.dataGridView1.ClipboardCopyMode = System.Windows.Forms.DataGridViewClipboardCopyMode.Disable;
         this.dataGridView1.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
         this.dataGridView1.ColumnHeadersVisible = false;
         this.dataGridView1.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.mDisplayNameColumn,
            this.mEventsColumn,
            this.mNameColumnHidden,
            this.mTagColumnHidden});
         this.dataGridView1.Location = new System.Drawing.Point(3, 28);
         this.dataGridView1.Name = "dataGridView1";
         this.dataGridView1.RowHeadersVisible = false;
         this.dataGridView1.SelectionMode = System.Windows.Forms.DataGridViewSelectionMode.FullRowSelect;
         this.dataGridView1.ShowEditingIcon = false;
         this.dataGridView1.Size = new System.Drawing.Size(870, 496);
         this.dataGridView1.TabIndex = 2;
         this.dataGridView1.KeyDown += new System.Windows.Forms.KeyEventHandler(this.OnKeyDown);
         this.dataGridView1.CellPainting += new System.Windows.Forms.DataGridViewCellPaintingEventHandler(this.OnCellPaint);
         this.dataGridView1.CellContextMenuStripNeeded += new System.Windows.Forms.DataGridViewCellContextMenuStripNeededEventHandler(this.OnCellContextMenuStripNeeded);
         this.dataGridView1.CellEndEdit += new System.Windows.Forms.DataGridViewCellEventHandler(this.OnCellEndEdit);
         this.dataGridView1.EditingControlShowing += new System.Windows.Forms.DataGridViewEditingControlShowingEventHandler(this.OnEditingControlShowing);
         // 
         // mDisplayNameColumn
         // 
         this.mDisplayNameColumn.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.DisplayedCells;
         this.mDisplayNameColumn.DataPropertyName = "DisplayName";
         this.mDisplayNameColumn.HeaderText = "Name";
         this.mDisplayNameColumn.MinimumWidth = 150;
         this.mDisplayNameColumn.Name = "mDisplayNameColumn";
         this.mDisplayNameColumn.Resizable = System.Windows.Forms.DataGridViewTriState.False;
         this.mDisplayNameColumn.Width = 150;
         // 
         // mEventsColumn
         // 
         this.mEventsColumn.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
         this.mEventsColumn.HeaderText = "Events";
         this.mEventsColumn.Name = "mEventsColumn";
         this.mEventsColumn.ReadOnly = true;
         // 
         // mNameColumnHidden
         // 
         this.mNameColumnHidden.DataPropertyName = "Name";
         this.mNameColumnHidden.HeaderText = "NameHidden";
         this.mNameColumnHidden.Name = "mNameColumnHidden";
         this.mNameColumnHidden.Visible = false;
         // 
         // mTagColumnHidden
         // 
         this.mTagColumnHidden.DataPropertyName = "Tag";
         this.mTagColumnHidden.HeaderText = "TagHidden";
         this.mTagColumnHidden.Name = "mTagColumnHidden";
         this.mTagColumnHidden.Visible = false;
         // 
         // mToolBar
         // 
         this.mToolBar.GripStyle = System.Windows.Forms.ToolStripGripStyle.Hidden;
         this.mToolBar.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripButton2});
         this.mToolBar.Location = new System.Drawing.Point(0, 0);
         this.mToolBar.Name = "mToolBar";
         this.mToolBar.Size = new System.Drawing.Size(876, 25);
         this.mToolBar.TabIndex = 3;
         this.mToolBar.Text = "toolStrip1";
         // 
         // toolStripButton2
         // 
         this.toolStripButton2.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
         this.toolStripButton2.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButton2.Image")));
         this.toolStripButton2.ImageTransparentColor = System.Drawing.Color.Magenta;
         this.toolStripButton2.MergeIndex = 100;
         this.toolStripButton2.Name = "toolStripButton2";
         this.toolStripButton2.Size = new System.Drawing.Size(23, 22);
         this.toolStripButton2.Text = "Delete Event";
         this.toolStripButton2.Click += new System.EventHandler(this.OnDelete);
         // 
         // unselectedImageList
         // 
         this.unselectedImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("unselectedImageList.ImageStream")));
         this.unselectedImageList.TransparentColor = System.Drawing.Color.Magenta;
         this.unselectedImageList.Images.SetKeyName(0, "");
         this.unselectedImageList.Images.SetKeyName(1, "RangeBeginUnselected.bmp");
         this.unselectedImageList.Images.SetKeyName(2, "");
         this.unselectedImageList.Images.SetKeyName(3, "RangeEndUnselected.bmp");
         // 
         // selectedImageList
         // 
         this.selectedImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("selectedImageList.ImageStream")));
         this.selectedImageList.TransparentColor = System.Drawing.Color.Magenta;
         this.selectedImageList.Images.SetKeyName(0, "TickSelected.bmp");
         this.selectedImageList.Images.SetKeyName(1, "RangeBeginSelected.bmp");
         this.selectedImageList.Images.SetKeyName(2, "RangeMiddleSelected.bmp");
         this.selectedImageList.Images.SetKeyName(3, "RangeEndSelected.bmp");
         // 
         // EventView
         // 
         this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
         this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
         this.AutoScroll = true;
         this.Controls.Add(this.mToolBar);
         this.Controls.Add(this.dataGridView1);
         this.Name = "EventView";
         this.Size = new System.Drawing.Size(876, 527);
         ((System.ComponentModel.ISupportInitialize)(this.dataGridView1)).EndInit();
         this.mToolBar.ResumeLayout(false);
         this.mToolBar.PerformLayout();
         this.ResumeLayout(false);
         this.PerformLayout();

      }

      #endregion

      private System.Windows.Forms.DataGridView dataGridView1;
      private System.Windows.Forms.ToolStripButton toolStripButton2;
      private System.Windows.Forms.ImageList unselectedImageList;
      public Tools.Controls.ToolStripEx mToolBar;
      private System.Windows.Forms.ImageList selectedImageList;
      private System.Windows.Forms.DataGridViewTextBoxColumn mDisplayNameColumn;
      private System.Windows.Forms.DataGridViewTextBoxColumn mEventsColumn;
      private System.Windows.Forms.DataGridViewTextBoxColumn mNameColumnHidden;
      private System.Windows.Forms.DataGridViewTextBoxColumn mTagColumnHidden;
   }
}
