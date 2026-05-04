namespace CoreTools.Prefab.UI
{
   partial class PrefabViewer
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
         System.Windows.Forms.ToolStripButton toolStripButton1;
         System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(PrefabViewer));
         this.nodeImageList = new System.Windows.Forms.ImageList(this.components);
         this.mContextMenu = new System.Windows.Forms.ContextMenuStrip(this.components);
         this.toolStrip1 = new System.Windows.Forms.ToolStrip();
         this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
         this.showCheckedOutPrefabsButton = new System.Windows.Forms.ToolStripButton();
         this.addViewToolStripButton = new System.Windows.Forms.ToolStripButton();
         this.deleteViewToolStripButton = new System.Windows.Forms.ToolStripButton();
         this.toolStripContainer1 = new System.Windows.Forms.ToolStripContainer();
         this.viewSplitContainer = new System.Windows.Forms.SplitContainer();
         this.label2 = new System.Windows.Forms.Label();
         this.prefabSplitContainer = new System.Windows.Forms.SplitContainer();
         this.prefabTreeView = new Tools.Controls.MultiSelectTreeView();
         this.statusImageList = new System.Windows.Forms.ImageList(this.components);
         this.label1 = new System.Windows.Forms.Label();
         this.overlayImageList = new System.Windows.Forms.ImageList(this.components);
         this.viewTagControl = new CoreTools.Prefab.UI.TagControl();
         this.prefabTagControl = new CoreTools.Prefab.UI.TagControl();
         toolStripButton1 = new System.Windows.Forms.ToolStripButton();
         this.toolStrip1.SuspendLayout();
         this.toolStripContainer1.ContentPanel.SuspendLayout();
         this.toolStripContainer1.TopToolStripPanel.SuspendLayout();
         this.toolStripContainer1.SuspendLayout();
         this.viewSplitContainer.Panel1.SuspendLayout();
         this.viewSplitContainer.Panel2.SuspendLayout();
         this.viewSplitContainer.SuspendLayout();
         this.prefabSplitContainer.Panel1.SuspendLayout();
         this.prefabSplitContainer.Panel2.SuspendLayout();
         this.prefabSplitContainer.SuspendLayout();
         this.SuspendLayout();
         // 
         // toolStripButton1
         // 
         toolStripButton1.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
         toolStripButton1.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButton1.Image")));
         toolStripButton1.ImageTransparentColor = System.Drawing.Color.Magenta;
         toolStripButton1.Name = "toolStripButton1";
         toolStripButton1.Size = new System.Drawing.Size(23, 22);
         toolStripButton1.Text = "Refresh";
         toolStripButton1.Click += new System.EventHandler(this.OnRefresh);
         // 
         // nodeImageList
         // 
         this.nodeImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("nodeImageList.ImageStream")));
         this.nodeImageList.TransparentColor = System.Drawing.Color.Transparent;
         this.nodeImageList.Images.SetKeyName(0, "Folder");
         this.nodeImageList.Images.SetKeyName(1, "Object");
         this.nodeImageList.Images.SetKeyName(2, "Prefab");
         this.nodeImageList.Images.SetKeyName(3, "View");
         // 
         // mContextMenu
         // 
         this.mContextMenu.Name = "contextMenuStrip1";
         this.mContextMenu.ShowImageMargin = false;
         this.mContextMenu.Size = new System.Drawing.Size(36, 4);
         // 
         // toolStrip1
         // 
         this.toolStrip1.Dock = System.Windows.Forms.DockStyle.None;
         this.toolStrip1.GripStyle = System.Windows.Forms.ToolStripGripStyle.Hidden;
         this.toolStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            toolStripButton1,
            this.toolStripSeparator1,
            this.showCheckedOutPrefabsButton,
            this.addViewToolStripButton,
            this.deleteViewToolStripButton});
         this.toolStrip1.Location = new System.Drawing.Point(0, 0);
         this.toolStrip1.Name = "toolStrip1";
         this.toolStrip1.Size = new System.Drawing.Size(470, 25);
         this.toolStrip1.Stretch = true;
         this.toolStrip1.TabIndex = 1;
         this.toolStrip1.Text = "toolStrip1";
         // 
         // toolStripSeparator1
         // 
         this.toolStripSeparator1.Name = "toolStripSeparator1";
         this.toolStripSeparator1.Size = new System.Drawing.Size(6, 25);
         // 
         // showCheckedOutPrefabsButton
         // 
         this.showCheckedOutPrefabsButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
         this.showCheckedOutPrefabsButton.Image = ((System.Drawing.Image)(resources.GetObject("showCheckedOutPrefabsButton.Image")));
         this.showCheckedOutPrefabsButton.ImageTransparentColor = System.Drawing.Color.Magenta;
         this.showCheckedOutPrefabsButton.Name = "showCheckedOutPrefabsButton";
         this.showCheckedOutPrefabsButton.Size = new System.Drawing.Size(23, 22);
         this.showCheckedOutPrefabsButton.Text = "Show checked out only";
         this.showCheckedOutPrefabsButton.Click += new System.EventHandler(this.OnShowCheckedOutPrefabsToggle);
         // 
         // addViewToolStripButton
         // 
         this.addViewToolStripButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
         this.addViewToolStripButton.Image = ((System.Drawing.Image)(resources.GetObject("addViewToolStripButton.Image")));
         this.addViewToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
         this.addViewToolStripButton.Name = "addViewToolStripButton";
         this.addViewToolStripButton.Size = new System.Drawing.Size(23, 22);
         this.addViewToolStripButton.Text = "Add View";
         this.addViewToolStripButton.Click += new System.EventHandler(this.OnAddView);
         // 
         // deleteViewToolStripButton
         // 
         this.deleteViewToolStripButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
         this.deleteViewToolStripButton.Enabled = false;
         this.deleteViewToolStripButton.Image = ((System.Drawing.Image)(resources.GetObject("deleteViewToolStripButton.Image")));
         this.deleteViewToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
         this.deleteViewToolStripButton.Name = "deleteViewToolStripButton";
         this.deleteViewToolStripButton.Size = new System.Drawing.Size(23, 22);
         this.deleteViewToolStripButton.Text = "Delete View";
         this.deleteViewToolStripButton.Click += new System.EventHandler(this.OnDeleteCurrentView);
         // 
         // toolStripContainer1
         // 
         // 
         // toolStripContainer1.ContentPanel
         // 
         this.toolStripContainer1.ContentPanel.Controls.Add(this.viewSplitContainer);
         this.toolStripContainer1.ContentPanel.Size = new System.Drawing.Size(470, 673);
         this.toolStripContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
         this.toolStripContainer1.Location = new System.Drawing.Point(0, 0);
         this.toolStripContainer1.Name = "toolStripContainer1";
         this.toolStripContainer1.Size = new System.Drawing.Size(470, 698);
         this.toolStripContainer1.TabIndex = 2;
         this.toolStripContainer1.Text = "toolStripContainer1";
         // 
         // toolStripContainer1.TopToolStripPanel
         // 
         this.toolStripContainer1.TopToolStripPanel.Controls.Add(this.toolStrip1);
         // 
         // viewSplitContainer
         // 
         this.viewSplitContainer.Dock = System.Windows.Forms.DockStyle.Fill;
         this.viewSplitContainer.FixedPanel = System.Windows.Forms.FixedPanel.Panel1;
         this.viewSplitContainer.Location = new System.Drawing.Point(0, 0);
         this.viewSplitContainer.Name = "viewSplitContainer";
         this.viewSplitContainer.Orientation = System.Windows.Forms.Orientation.Horizontal;
         // 
         // viewSplitContainer.Panel1
         // 
         this.viewSplitContainer.Panel1.Controls.Add(this.label2);
         this.viewSplitContainer.Panel1.Controls.Add(this.viewTagControl);
         // 
         // viewSplitContainer.Panel2
         // 
         this.viewSplitContainer.Panel2.Controls.Add(this.prefabSplitContainer);
         this.viewSplitContainer.Size = new System.Drawing.Size(470, 673);
         this.viewSplitContainer.SplitterDistance = 30;
         this.viewSplitContainer.SplitterWidth = 1;
         this.viewSplitContainer.TabIndex = 3;
         // 
         // label2
         // 
         this.label2.AutoSize = true;
         this.label2.Location = new System.Drawing.Point(4, 8);
         this.label2.Name = "label2";
         this.label2.Size = new System.Drawing.Size(85, 13);
         this.label2.TabIndex = 1;
         this.label2.Text = "View Filter Tags:";
         // 
         // prefabSplitContainer
         // 
         this.prefabSplitContainer.Dock = System.Windows.Forms.DockStyle.Fill;
         this.prefabSplitContainer.FixedPanel = System.Windows.Forms.FixedPanel.Panel2;
         this.prefabSplitContainer.Location = new System.Drawing.Point(0, 0);
         this.prefabSplitContainer.Name = "prefabSplitContainer";
         this.prefabSplitContainer.Orientation = System.Windows.Forms.Orientation.Horizontal;
         // 
         // prefabSplitContainer.Panel1
         // 
         this.prefabSplitContainer.Panel1.Controls.Add(this.prefabTreeView);
         // 
         // prefabSplitContainer.Panel2
         // 
         this.prefabSplitContainer.Panel2.Controls.Add(this.label1);
         this.prefabSplitContainer.Panel2.Controls.Add(this.prefabTagControl);
         this.prefabSplitContainer.Size = new System.Drawing.Size(470, 642);
         this.prefabSplitContainer.SplitterDistance = 616;
         this.prefabSplitContainer.SplitterWidth = 1;
         this.prefabSplitContainer.TabIndex = 2;
         // 
         // prefabTreeView
         // 
         this.prefabTreeView.Dock = System.Windows.Forms.DockStyle.Fill;
         this.prefabTreeView.ImageIndex = 0;
         this.prefabTreeView.ImageList = this.nodeImageList;
         this.prefabTreeView.Indent = 19;
         this.prefabTreeView.ItemHeight = 18;
         this.prefabTreeView.LabelEdit = true;
         this.prefabTreeView.Location = new System.Drawing.Point(0, 0);
         this.prefabTreeView.Name = "prefabTreeView";
         this.prefabTreeView.OverlayImageList = null;
         this.prefabTreeView.SelectedImageIndex = 0;
         this.prefabTreeView.SelectedNodes = ((System.Collections.Generic.List<System.Windows.Forms.TreeNode>)(resources.GetObject("prefabTreeView.SelectedNodes")));
         this.prefabTreeView.Size = new System.Drawing.Size(470, 616);
         this.prefabTreeView.StatusImageList = this.statusImageList;
         this.prefabTreeView.TabIndex = 0;
         this.prefabTreeView.AfterLabelEdit += new System.Windows.Forms.NodeLabelEditEventHandler(this.OnAfterLabelEdit);
         this.prefabTreeView.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.OnAfterSelect);
         this.prefabTreeView.MouseMove += new System.Windows.Forms.MouseEventHandler(this.OnTreeViewMouseMove);
         this.prefabTreeView.NodeMouseClick += new System.Windows.Forms.TreeNodeMouseClickEventHandler(this.OnNodeMouseClick);
         // 
         // statusImageList
         // 
         this.statusImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("statusImageList.ImageStream")));
         this.statusImageList.TransparentColor = System.Drawing.Color.Magenta;
         this.statusImageList.Images.SetKeyName(0, "undetermined");
         this.statusImageList.Images.SetKeyName(1, "edit");
         this.statusImageList.Images.SetKeyName(2, "add");
         this.statusImageList.Images.SetKeyName(3, "delete");
         this.statusImageList.Images.SetKeyName(4, "locked");
         this.statusImageList.Images.SetKeyName(5, "otherCheckout");
         // 
         // label1
         // 
         this.label1.AutoSize = true;
         this.label1.Location = new System.Drawing.Point(4, 7);
         this.label1.Name = "label1";
         this.label1.Size = new System.Drawing.Size(68, 13);
         this.label1.TabIndex = 1;
         this.label1.Text = "Prefab Tags:";
         // 
         // overlayImageList
         // 
         this.overlayImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("overlayImageList.ImageStream")));
         this.overlayImageList.TransparentColor = System.Drawing.Color.Transparent;
         this.overlayImageList.Images.SetKeyName(0, "Overlay_CheckedOutToMe.png");
         this.overlayImageList.Images.SetKeyName(1, "Overlay_CheckedOutToSomeoneElse.png");
         // 
         // viewTagControl
         // 
         this.viewTagControl.Location = new System.Drawing.Point(95, 0);
         this.viewTagControl.Name = "viewTagControl";
         this.viewTagControl.Size = new System.Drawing.Size(375, 29);
         this.viewTagControl.TabIndex = 0;
         // 
         // prefabTagControl
         // 
         this.prefabTagControl.Location = new System.Drawing.Point(78, -1);
         this.prefabTagControl.Name = "prefabTagControl";
         this.prefabTagControl.Size = new System.Drawing.Size(392, 29);
         this.prefabTagControl.TabIndex = 0;
         // 
         // PrefabViewer
         // 
         this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
         this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
         this.Controls.Add(this.toolStripContainer1);
         this.Name = "PrefabViewer";
         this.Size = new System.Drawing.Size(470, 698);
         this.toolStrip1.ResumeLayout(false);
         this.toolStrip1.PerformLayout();
         this.toolStripContainer1.ContentPanel.ResumeLayout(false);
         this.toolStripContainer1.TopToolStripPanel.ResumeLayout(false);
         this.toolStripContainer1.TopToolStripPanel.PerformLayout();
         this.toolStripContainer1.ResumeLayout(false);
         this.toolStripContainer1.PerformLayout();
         this.viewSplitContainer.Panel1.ResumeLayout(false);
         this.viewSplitContainer.Panel1.PerformLayout();
         this.viewSplitContainer.Panel2.ResumeLayout(false);
         this.viewSplitContainer.ResumeLayout(false);
         this.prefabSplitContainer.Panel1.ResumeLayout(false);
         this.prefabSplitContainer.Panel2.ResumeLayout(false);
         this.prefabSplitContainer.Panel2.PerformLayout();
         this.prefabSplitContainer.ResumeLayout(false);
         this.ResumeLayout(false);

      }

      #endregion

      private Tools.Controls.MultiSelectTreeView prefabTreeView;
      private System.Windows.Forms.ToolStrip toolStrip1;
      private System.Windows.Forms.ImageList nodeImageList;
      private System.Windows.Forms.ToolStripContainer toolStripContainer1;
      private System.Windows.Forms.ContextMenuStrip mContextMenu;
      private System.Windows.Forms.ImageList overlayImageList;
      private System.Windows.Forms.ImageList statusImageList;
      private System.Windows.Forms.ToolStripButton showCheckedOutPrefabsButton;
      private System.Windows.Forms.SplitContainer prefabSplitContainer;
      private TagControl prefabTagControl;
      private System.Windows.Forms.SplitContainer viewSplitContainer;
      private TagControl viewTagControl;
      private System.Windows.Forms.Label label1;
      private System.Windows.Forms.Label label2;
      private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
      private System.Windows.Forms.ToolStripButton addViewToolStripButton;
      private System.Windows.Forms.ToolStripButton deleteViewToolStripButton;
   }
}
