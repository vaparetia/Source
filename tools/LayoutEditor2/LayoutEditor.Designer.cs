namespace LayoutEditorMGS2
{
   // http://anandopaul.wordpress.com/2010/01/01/c-treeview-checkbox-double-click-bug-for-vista-and-win7/
   public class TreeViewWithoutDoubleClick : System.Windows.Forms.TreeView
   {
      protected override void WndProc(ref System.Windows.Forms.Message m)
      {
         if (m.Msg == 0x203)
         {
            m.Result = System.IntPtr.Zero;
         }
         else
         {
            base.WndProc(ref m);
         }
      }
   }

   partial class LayoutEditorWindow
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
         this.splitContainer1 = new System.Windows.Forms.SplitContainer();
         this.splitContainer2 = new System.Windows.Forms.SplitContainer();
         this.label1 = new System.Windows.Forms.Label();
         this.objectTreeView = new LayoutEditorMGS2.TreeViewWithoutDoubleClick();
         this.label2 = new System.Windows.Forms.Label();
         this.actionTreeView = new System.Windows.Forms.TreeView();
         this.previewPictureBox = new LayoutEditorMGS2.SelectablePictureBox();
         this.statusStrip1 = new System.Windows.Forms.StatusStrip();
         this.toolStripStatusLabel1 = new System.Windows.Forms.ToolStripStatusLabel();
         this.menuStrip1 = new System.Windows.Forms.MenuStrip();
         this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.openToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.recentItemsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.perforceCheckOutToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.saveToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.exportToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
         this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.editToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.undoToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.redoToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.findToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
         this.editNodeDataToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.exportReferencedTexturesToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.exportReferencedTexturesForAllFilesToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.viewToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.widescreenPreviewToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.hdToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.vitaToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.viewTextureToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.safeZoneToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.toolsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.backgroundColorToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.toolStripMenuItem2 = new System.Windows.Forms.ToolStripSeparator();
         this.makeElementsWidescreenToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.selectedElementsMadeWidescreenToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.selectedElementsMadeWidescreenCenterToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.selectedElementsToLeftToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.selectedElementsToRightToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.selectedElementsFillToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.togglePixelPerfectOnSelectedToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.toolStripMenuItem3 = new System.Windows.Forms.ToolStripSeparator();
         this.validateExportProcessOnAllFilesToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.exportAllFilesToWidescreenToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
         this.gatherStringIDsFromSourceFilesToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.optionsToolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
         this.newStrCodesInXMLToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.helpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.keyboardShortcutsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.optionsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.exploreToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.splitContainer1.Panel1.SuspendLayout();
         this.splitContainer1.Panel2.SuspendLayout();
         this.splitContainer1.SuspendLayout();
         this.splitContainer2.Panel1.SuspendLayout();
         this.splitContainer2.Panel2.SuspendLayout();
         this.splitContainer2.SuspendLayout();
         ((System.ComponentModel.ISupportInitialize)(this.previewPictureBox)).BeginInit();
         this.statusStrip1.SuspendLayout();
         this.menuStrip1.SuspendLayout();
         this.SuspendLayout();
         // 
         // splitContainer1
         // 
         this.splitContainer1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                     | System.Windows.Forms.AnchorStyles.Left)
                     | System.Windows.Forms.AnchorStyles.Right)));
         this.splitContainer1.Location = new System.Drawing.Point(12, 27);
         this.splitContainer1.Name = "splitContainer1";
         // 
         // splitContainer1.Panel1
         // 
         this.splitContainer1.Panel1.Controls.Add(this.splitContainer2);
         // 
         // splitContainer1.Panel2
         // 
         this.splitContainer1.Panel2.Controls.Add(this.previewPictureBox);
         this.splitContainer1.Size = new System.Drawing.Size(949, 540);
         this.splitContainer1.SplitterDistance = 200;
         this.splitContainer1.TabIndex = 2;
         // 
         // splitContainer2
         // 
         this.splitContainer2.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                     | System.Windows.Forms.AnchorStyles.Left)
                     | System.Windows.Forms.AnchorStyles.Right)));
         this.splitContainer2.Location = new System.Drawing.Point(0, 3);
         this.splitContainer2.Name = "splitContainer2";
         this.splitContainer2.Orientation = System.Windows.Forms.Orientation.Horizontal;
         // 
         // splitContainer2.Panel1
         // 
         this.splitContainer2.Panel1.Controls.Add(this.label1);
         this.splitContainer2.Panel1.Controls.Add(this.objectTreeView);
         this.splitContainer2.Panel1MinSize = 40;
         // 
         // splitContainer2.Panel2
         // 
         this.splitContainer2.Panel2.Controls.Add(this.label2);
         this.splitContainer2.Panel2.Controls.Add(this.actionTreeView);
         this.splitContainer2.Panel2MinSize = 40;
         this.splitContainer2.Size = new System.Drawing.Size(201, 537);
         this.splitContainer2.SplitterDistance = 268;
         this.splitContainer2.TabIndex = 2;
         // 
         // label1
         // 
         this.label1.AutoSize = true;
         this.label1.Location = new System.Drawing.Point(3, 4);
         this.label1.Name = "label1";
         this.label1.Size = new System.Drawing.Size(43, 13);
         this.label1.TabIndex = 2;
         this.label1.Text = "Objects";
         // 
         // objectTreeView
         // 
         this.objectTreeView.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                     | System.Windows.Forms.AnchorStyles.Left)
                     | System.Windows.Forms.AnchorStyles.Right)));
         this.objectTreeView.CheckBoxes = true;
         this.objectTreeView.HideSelection = false;
         this.objectTreeView.LabelEdit = true;
         this.objectTreeView.Location = new System.Drawing.Point(3, 20);
         this.objectTreeView.Name = "objectTreeView";
         this.objectTreeView.Size = new System.Drawing.Size(195, 245);
         this.objectTreeView.TabIndex = 1;
         this.objectTreeView.AfterCheck += new System.Windows.Forms.TreeViewEventHandler(this.objectTreeView_AfterCheck);
         this.objectTreeView.AfterLabelEdit += new System.Windows.Forms.NodeLabelEditEventHandler(this.objectTreeView_AfterLabelEdit);
         this.objectTreeView.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.objectTreeView_AfterSelect);
         this.objectTreeView.NodeMouseClick += new System.Windows.Forms.TreeNodeMouseClickEventHandler(this.objectTreeView_NodeMouseClick);
         this.objectTreeView.BeforeLabelEdit += new System.Windows.Forms.NodeLabelEditEventHandler(this.objectTreeView_BeforeLabelEdit);
         this.objectTreeView.KeyDown += new System.Windows.Forms.KeyEventHandler(this.objectTreeView_KeyDown);
         // 
         // label2
         // 
         this.label2.AutoSize = true;
         this.label2.Location = new System.Drawing.Point(3, 4);
         this.label2.Name = "label2";
         this.label2.Size = new System.Drawing.Size(42, 13);
         this.label2.TabIndex = 1;
         this.label2.Text = "Actions";
         // 
         // actionTreeView
         // 
         this.actionTreeView.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                     | System.Windows.Forms.AnchorStyles.Left)
                     | System.Windows.Forms.AnchorStyles.Right)));
         this.actionTreeView.HideSelection = false;
         this.actionTreeView.LabelEdit = true;
         this.actionTreeView.Location = new System.Drawing.Point(3, 20);
         this.actionTreeView.Name = "actionTreeView";
         this.actionTreeView.ShowNodeToolTips = true;
         this.actionTreeView.Size = new System.Drawing.Size(194, 242);
         this.actionTreeView.TabIndex = 0;
         this.actionTreeView.AfterLabelEdit += new System.Windows.Forms.NodeLabelEditEventHandler(this.actionTreeView_AfterLabelEdit);
         this.actionTreeView.NodeMouseClick += new System.Windows.Forms.TreeNodeMouseClickEventHandler(this.actionTreeView_NodeMouseClick);
         this.actionTreeView.BeforeLabelEdit += new System.Windows.Forms.NodeLabelEditEventHandler(this.actionTreeView_BeforeLabelEdit);
         this.actionTreeView.KeyDown += new System.Windows.Forms.KeyEventHandler(this.actionTreeView_KeyDown);
         // 
         // previewPictureBox
         // 
         this.previewPictureBox.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                     | System.Windows.Forms.AnchorStyles.Left)
                     | System.Windows.Forms.AnchorStyles.Right)));
         this.previewPictureBox.Location = new System.Drawing.Point(3, 3);
         this.previewPictureBox.Name = "previewPictureBox";
         this.previewPictureBox.Size = new System.Drawing.Size(739, 534);
         this.previewPictureBox.TabIndex = 0;
         this.previewPictureBox.TabStop = false;
         this.previewPictureBox.Resize += new System.EventHandler(this.previewPictureBox_Resize);
         this.previewPictureBox.MouseClick += new System.Windows.Forms.MouseEventHandler(this.previewPictureBox_MouseClick);
         this.previewPictureBox.Paint += new System.Windows.Forms.PaintEventHandler(this.previewPictureBox_Paint);
         // 
         // statusStrip1
         // 
         this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabel1});
         this.statusStrip1.Location = new System.Drawing.Point(0, 570);
         this.statusStrip1.Name = "statusStrip1";
         this.statusStrip1.Size = new System.Drawing.Size(973, 22);
         this.statusStrip1.TabIndex = 3;
         this.statusStrip1.Text = "statusStrip1";
         // 
         // toolStripStatusLabel1
         // 
         this.toolStripStatusLabel1.Name = "toolStripStatusLabel1";
         this.toolStripStatusLabel1.Size = new System.Drawing.Size(39, 17);
         this.toolStripStatusLabel1.Text = "Ready";
         this.toolStripStatusLabel1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
         // 
         // menuStrip1
         // 
         this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.editToolStripMenuItem,
            this.viewToolStripMenuItem,
            this.toolsToolStripMenuItem,
            this.optionsToolStripMenuItem1,
            this.helpToolStripMenuItem});
         this.menuStrip1.Location = new System.Drawing.Point(0, 0);
         this.menuStrip1.Name = "menuStrip1";
         this.menuStrip1.Size = new System.Drawing.Size(973, 24);
         this.menuStrip1.TabIndex = 4;
         this.menuStrip1.Text = "menuStrip1";
         // 
         // fileToolStripMenuItem
         // 
         this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.openToolStripMenuItem,
            this.recentItemsToolStripMenuItem,
            this.perforceCheckOutToolStripMenuItem,
            this.exploreToolStripMenuItem,
            this.saveToolStripMenuItem,
            this.exportToolStripMenuItem,
            this.toolStripSeparator1,
            this.exitToolStripMenuItem});
         this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
         this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
         this.fileToolStripMenuItem.Text = "&File";
         this.fileToolStripMenuItem.DropDownOpening += new System.EventHandler(this.fileToolStripMenuItem_DropDownOpening);
         // 
         // openToolStripMenuItem
         // 
         this.openToolStripMenuItem.Name = "openToolStripMenuItem";
         this.openToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.O)));
         this.openToolStripMenuItem.Size = new System.Drawing.Size(192, 22);
         this.openToolStripMenuItem.Tag = "Open .la2 or .xml file";
         this.openToolStripMenuItem.Text = "&Open...";
         this.openToolStripMenuItem.Click += new System.EventHandler(this.openToolStripMenuItem_Click);
         // 
         // recentItemsToolStripMenuItem
         // 
         this.recentItemsToolStripMenuItem.Name = "recentItemsToolStripMenuItem";
         this.recentItemsToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.R)));
         this.recentItemsToolStripMenuItem.Size = new System.Drawing.Size(192, 22);
         this.recentItemsToolStripMenuItem.Tag = "Show recently opened files";
         this.recentItemsToolStripMenuItem.Text = "Open &Recent...";
         // 
         // perforceCheckOutToolStripMenuItem
         // 
         this.perforceCheckOutToolStripMenuItem.Name = "perforceCheckOutToolStripMenuItem";
         this.perforceCheckOutToolStripMenuItem.Size = new System.Drawing.Size(192, 22);
         this.perforceCheckOutToolStripMenuItem.Text = "Perforce check out";
         this.perforceCheckOutToolStripMenuItem.Click += new System.EventHandler(this.perforceCheckOutToolStripMenuItem_Click);
         // 
         // saveToolStripMenuItem
         // 
         this.saveToolStripMenuItem.Name = "saveToolStripMenuItem";
         this.saveToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.S)));
         this.saveToolStripMenuItem.Size = new System.Drawing.Size(192, 22);
         this.saveToolStripMenuItem.Tag = "Save as an xml file";
         this.saveToolStripMenuItem.Text = "&Save to .xml";
         this.saveToolStripMenuItem.Click += new System.EventHandler(this.saveToolStripMenuItem_Click);
         // 
         // exportToolStripMenuItem
         // 
         this.exportToolStripMenuItem.Name = "exportToolStripMenuItem";
         this.exportToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.E)));
         this.exportToolStripMenuItem.Size = new System.Drawing.Size(192, 22);
         this.exportToolStripMenuItem.Tag = "Export binary file with .la2.dup extension";
         this.exportToolStripMenuItem.Text = "&Export to .o2d";
         this.exportToolStripMenuItem.Click += new System.EventHandler(this.exportToolStripMenuItem_Click);
         // 
         // toolStripSeparator1
         // 
         this.toolStripSeparator1.Name = "toolStripSeparator1";
         this.toolStripSeparator1.Size = new System.Drawing.Size(189, 6);
         // 
         // exitToolStripMenuItem
         // 
         this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
         this.exitToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Alt | System.Windows.Forms.Keys.F4)));
         this.exitToolStripMenuItem.Size = new System.Drawing.Size(192, 22);
         this.exitToolStripMenuItem.Tag = "Exit the application";
         this.exitToolStripMenuItem.Text = "E&xit";
         this.exitToolStripMenuItem.Click += new System.EventHandler(this.exitToolStripMenuItem_Click);
         // 
         // editToolStripMenuItem
         // 
         this.editToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.undoToolStripMenuItem,
            this.redoToolStripMenuItem,
            this.findToolStripMenuItem,
            this.toolStripMenuItem1,
            this.editNodeDataToolStripMenuItem,
            this.exportReferencedTexturesToolStripMenuItem,
            this.exportReferencedTexturesForAllFilesToolStripMenuItem});
         this.editToolStripMenuItem.Name = "editToolStripMenuItem";
         this.editToolStripMenuItem.Size = new System.Drawing.Size(39, 20);
         this.editToolStripMenuItem.Text = "Edit";
         // 
         // undoToolStripMenuItem
         // 
         this.undoToolStripMenuItem.Name = "undoToolStripMenuItem";
         this.undoToolStripMenuItem.ShortcutKeyDisplayString = "Ctrl+Z";
         this.undoToolStripMenuItem.Size = new System.Drawing.Size(279, 22);
         this.undoToolStripMenuItem.Text = "Undo";
         this.undoToolStripMenuItem.Click += new System.EventHandler(this.undoToolStripMenuItem_Click);
         // 
         // redoToolStripMenuItem
         // 
         this.redoToolStripMenuItem.Name = "redoToolStripMenuItem";
         this.redoToolStripMenuItem.ShortcutKeyDisplayString = "Ctrl+Y";
         this.redoToolStripMenuItem.Size = new System.Drawing.Size(279, 22);
         this.redoToolStripMenuItem.Text = "Redo";
         this.redoToolStripMenuItem.Click += new System.EventHandler(this.redoToolStripMenuItem_Click);
         // 
         // findToolStripMenuItem
         // 
         this.findToolStripMenuItem.Name = "findToolStripMenuItem";
         this.findToolStripMenuItem.ShortcutKeyDisplayString = "Ctrl+F";
         this.findToolStripMenuItem.Size = new System.Drawing.Size(279, 22);
         this.findToolStripMenuItem.Text = "Find Object in Action";
         this.findToolStripMenuItem.Click += new System.EventHandler(this.findToolStripMenuItem_Click);
         // 
         // toolStripMenuItem1
         // 
         this.toolStripMenuItem1.Name = "toolStripMenuItem1";
         this.toolStripMenuItem1.Size = new System.Drawing.Size(276, 6);
         // 
         // editNodeDataToolStripMenuItem
         // 
         this.editNodeDataToolStripMenuItem.Name = "editNodeDataToolStripMenuItem";
         this.editNodeDataToolStripMenuItem.ShortcutKeyDisplayString = "";
         this.editNodeDataToolStripMenuItem.ShortcutKeys = System.Windows.Forms.Keys.F3;
         this.editNodeDataToolStripMenuItem.Size = new System.Drawing.Size(279, 22);
         this.editNodeDataToolStripMenuItem.Text = "Edit Node Data";
         this.editNodeDataToolStripMenuItem.Click += new System.EventHandler(this.editNodeDataToolStripMenuItem_Click);
         // 
         // exportReferencedTexturesToolStripMenuItem
         // 
         this.exportReferencedTexturesToolStripMenuItem.Name = "exportReferencedTexturesToolStripMenuItem";
         this.exportReferencedTexturesToolStripMenuItem.Size = new System.Drawing.Size(279, 22);
         this.exportReferencedTexturesToolStripMenuItem.Text = "Export Referenced Textures";
         this.exportReferencedTexturesToolStripMenuItem.Click += new System.EventHandler(this.exportReferencedTexturesToolStripMenuItem_Click);
         // 
         // exportReferencedTexturesForAllFilesToolStripMenuItem
         // 
         this.exportReferencedTexturesForAllFilesToolStripMenuItem.Name = "exportReferencedTexturesForAllFilesToolStripMenuItem";
         this.exportReferencedTexturesForAllFilesToolStripMenuItem.Size = new System.Drawing.Size(279, 22);
         this.exportReferencedTexturesForAllFilesToolStripMenuItem.Text = "Export Referenced Textures For All Files";
         this.exportReferencedTexturesForAllFilesToolStripMenuItem.Click += new System.EventHandler(this.exportReferencedTexturesForAllFilesToolStripMenuItem_Click);
         // 
         // viewToolStripMenuItem
         // 
         this.viewToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.widescreenPreviewToolStripMenuItem,
            this.hdToolStripMenuItem,
            this.vitaToolStripMenuItem,
            this.viewTextureToolStripMenuItem,
            this.safeZoneToolStripMenuItem});
         this.viewToolStripMenuItem.Name = "viewToolStripMenuItem";
         this.viewToolStripMenuItem.Size = new System.Drawing.Size(44, 20);
         this.viewToolStripMenuItem.Text = "View";
         // 
         // widescreenPreviewToolStripMenuItem
         // 
         this.widescreenPreviewToolStripMenuItem.CheckOnClick = true;
         this.widescreenPreviewToolStripMenuItem.Name = "widescreenPreviewToolStripMenuItem";
         this.widescreenPreviewToolStripMenuItem.Size = new System.Drawing.Size(178, 22);
         this.widescreenPreviewToolStripMenuItem.Text = "Fit to Screen Height";
         this.widescreenPreviewToolStripMenuItem.ToolTipText = "Previews a 4:3 layout in widescreen";
         this.widescreenPreviewToolStripMenuItem.CheckedChanged += new System.EventHandler(this.fitToScreenHeightMenuItem_CheckedChanged);
         // 
         // hdToolStripMenuItem
         // 
         this.hdToolStripMenuItem.CheckOnClick = true;
         this.hdToolStripMenuItem.Name = "hdToolStripMenuItem";
         this.hdToolStripMenuItem.Size = new System.Drawing.Size(178, 22);
         this.hdToolStripMenuItem.Text = "HD units 1280x720";
         this.hdToolStripMenuItem.CheckedChanged += new System.EventHandler(this.hdToolStripMenuItem_CheckedChanged);
         // 
         // vitaToolStripMenuItem
         // 
         this.vitaToolStripMenuItem.CheckOnClick = true;
         this.vitaToolStripMenuItem.Name = "vitaToolStripMenuItem";
         this.vitaToolStripMenuItem.Size = new System.Drawing.Size(178, 22);
         this.vitaToolStripMenuItem.Text = "Vita units 960x540";
         this.vitaToolStripMenuItem.CheckedChanged += new System.EventHandler(this.vitaToolStripMenuItem_CheckedChanged);
         // 
         // viewTextureToolStripMenuItem
         // 
         this.viewTextureToolStripMenuItem.Name = "viewTextureToolStripMenuItem";
         this.viewTextureToolStripMenuItem.ShortcutKeyDisplayString = "";
         this.viewTextureToolStripMenuItem.ShortcutKeys = System.Windows.Forms.Keys.F6;
         this.viewTextureToolStripMenuItem.Size = new System.Drawing.Size(178, 22);
         this.viewTextureToolStripMenuItem.Text = "View Texture";
         this.viewTextureToolStripMenuItem.Click += new System.EventHandler(this.viewTextureToolStripMenuItem_Click);
         // 
         // safeZoneToolStripMenuItem
         // 
         this.safeZoneToolStripMenuItem.CheckOnClick = true;
         this.safeZoneToolStripMenuItem.Name = "safeZoneToolStripMenuItem";
         this.safeZoneToolStripMenuItem.ShortcutKeyDisplayString = "";
         this.safeZoneToolStripMenuItem.ShortcutKeys = System.Windows.Forms.Keys.F8;
         this.safeZoneToolStripMenuItem.Size = new System.Drawing.Size(178, 22);
         this.safeZoneToolStripMenuItem.Text = "Show Safe Zone";
         this.safeZoneToolStripMenuItem.CheckedChanged += new System.EventHandler(this.safeZoneToolStripMenuItem_CheckedChanged);
         // 
         // toolsToolStripMenuItem
         // 
         this.toolsToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.backgroundColorToolStripMenuItem,
            this.toolStripMenuItem2,
            this.makeElementsWidescreenToolStripMenuItem,
            this.selectedElementsMadeWidescreenToolStripMenuItem,
            this.selectedElementsMadeWidescreenCenterToolStripMenuItem,
            this.selectedElementsToLeftToolStripMenuItem,
            this.selectedElementsToRightToolStripMenuItem,
            this.selectedElementsFillToolStripMenuItem,
            this.togglePixelPerfectOnSelectedToolStripMenuItem,
            this.toolStripMenuItem3,
            this.validateExportProcessOnAllFilesToolStripMenuItem,
            this.exportAllFilesToWidescreenToolStripMenuItem,
            this.toolStripSeparator2,
            this.gatherStringIDsFromSourceFilesToolStripMenuItem});
         this.toolsToolStripMenuItem.Name = "toolsToolStripMenuItem";
         this.toolsToolStripMenuItem.Size = new System.Drawing.Size(48, 20);
         this.toolsToolStripMenuItem.Text = "&Tools";
         // 
         // backgroundColorToolStripMenuItem
         // 
         this.backgroundColorToolStripMenuItem.Name = "backgroundColorToolStripMenuItem";
         this.backgroundColorToolStripMenuItem.Size = new System.Drawing.Size(337, 22);
         this.backgroundColorToolStripMenuItem.Tag = "Pick a background color for the preview window";
         this.backgroundColorToolStripMenuItem.Text = "Set Background &Color...";
         this.backgroundColorToolStripMenuItem.Click += new System.EventHandler(this.backgroundColorToolStripMenuItem_Click);
         // 
         // toolStripMenuItem2
         // 
         this.toolStripMenuItem2.Name = "toolStripMenuItem2";
         this.toolStripMenuItem2.Size = new System.Drawing.Size(334, 6);
         // 
         // makeElementsWidescreenToolStripMenuItem
         // 
         this.makeElementsWidescreenToolStripMenuItem.Name = "makeElementsWidescreenToolStripMenuItem";
         this.makeElementsWidescreenToolStripMenuItem.Size = new System.Drawing.Size(337, 22);
         this.makeElementsWidescreenToolStripMenuItem.Text = "Resize Elements for Widescreen";
         this.makeElementsWidescreenToolStripMenuItem.Click += new System.EventHandler(this.makeElementsWidescreenToolStripMenuItem_Click);
         // 
         // selectedElementsMadeWidescreenToolStripMenuItem
         // 
         this.selectedElementsMadeWidescreenToolStripMenuItem.Name = "selectedElementsMadeWidescreenToolStripMenuItem";
         this.selectedElementsMadeWidescreenToolStripMenuItem.ShortcutKeys = System.Windows.Forms.Keys.F4;
         this.selectedElementsMadeWidescreenToolStripMenuItem.Size = new System.Drawing.Size(337, 22);
         this.selectedElementsMadeWidescreenToolStripMenuItem.Text = "Scale selected to widescreen (from Upper Left)";
         this.selectedElementsMadeWidescreenToolStripMenuItem.Click += new System.EventHandler(this.selectedElementsMadeWidescreenToolStripMenuItem_Click);
         // 
         // selectedElementsMadeWidescreenCenterToolStripMenuItem
         // 
         this.selectedElementsMadeWidescreenCenterToolStripMenuItem.Name = "selectedElementsMadeWidescreenCenterToolStripMenuItem";
         this.selectedElementsMadeWidescreenCenterToolStripMenuItem.Size = new System.Drawing.Size(337, 22);
         this.selectedElementsMadeWidescreenCenterToolStripMenuItem.Text = "Scale selected to widescreen (from Center)";
         this.selectedElementsMadeWidescreenCenterToolStripMenuItem.Click += new System.EventHandler(this.selectedElementsMadeWidescreenCenterToolStripMenuItem_Click);
         // 
         // selectedElementsToLeftToolStripMenuItem
         // 
         this.selectedElementsToLeftToolStripMenuItem.Name = "selectedElementsToLeftToolStripMenuItem";
         this.selectedElementsToLeftToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.Left)));
         this.selectedElementsToLeftToolStripMenuItem.Size = new System.Drawing.Size(337, 22);
         this.selectedElementsToLeftToolStripMenuItem.Text = "Snap Selected Element to Left";
         this.selectedElementsToLeftToolStripMenuItem.Click += new System.EventHandler(this.selectedElementsToLeftToolStripMenuItem_Click);
         // 
         // selectedElementsToRightToolStripMenuItem
         // 
         this.selectedElementsToRightToolStripMenuItem.Name = "selectedElementsToRightToolStripMenuItem";
         this.selectedElementsToRightToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.Right)));
         this.selectedElementsToRightToolStripMenuItem.Size = new System.Drawing.Size(337, 22);
         this.selectedElementsToRightToolStripMenuItem.Text = "Snap Selected Element to Right";
         this.selectedElementsToRightToolStripMenuItem.Click += new System.EventHandler(this.selectedElementsToRightToolStripMenuItem_Click);
         // 
         // selectedElementsFillToolStripMenuItem
         // 
         this.selectedElementsFillToolStripMenuItem.Name = "selectedElementsFillToolStripMenuItem";
         this.selectedElementsFillToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.Up)));
         this.selectedElementsFillToolStripMenuItem.Size = new System.Drawing.Size(337, 22);
         this.selectedElementsFillToolStripMenuItem.Text = "Expand Selected Element to Fill Width";
         this.selectedElementsFillToolStripMenuItem.Click += new System.EventHandler(this.selectedElementsFillToolStripMenuItem_Click);
         // 
         // togglePixelPerfectOnSelectedToolStripMenuItem
         // 
         this.togglePixelPerfectOnSelectedToolStripMenuItem.Name = "togglePixelPerfectOnSelectedToolStripMenuItem";
         this.togglePixelPerfectOnSelectedToolStripMenuItem.ShortcutKeys = System.Windows.Forms.Keys.F9;
         this.togglePixelPerfectOnSelectedToolStripMenuItem.Size = new System.Drawing.Size(337, 22);
         this.togglePixelPerfectOnSelectedToolStripMenuItem.Text = "Toggle pixel perfect on selected";
         this.togglePixelPerfectOnSelectedToolStripMenuItem.Click += new System.EventHandler(this.togglePixelPerfectOnSelectedToolStripMenuItem_Click);
         // 
         // toolStripMenuItem3
         // 
         this.toolStripMenuItem3.Name = "toolStripMenuItem3";
         this.toolStripMenuItem3.Size = new System.Drawing.Size(334, 6);
         // 
         // validateExportProcessOnAllFilesToolStripMenuItem
         // 
         this.validateExportProcessOnAllFilesToolStripMenuItem.Name = "validateExportProcessOnAllFilesToolStripMenuItem";
         this.validateExportProcessOnAllFilesToolStripMenuItem.Size = new System.Drawing.Size(337, 22);
         this.validateExportProcessOnAllFilesToolStripMenuItem.Text = "Validate Export Process On All Files";
         this.validateExportProcessOnAllFilesToolStripMenuItem.Click += new System.EventHandler(this.validateExportProcessOnAllFilesToolStripMenuItem_Click);
         // 
         // exportAllFilesToWidescreenToolStripMenuItem
         // 
         this.exportAllFilesToWidescreenToolStripMenuItem.Name = "exportAllFilesToWidescreenToolStripMenuItem";
         this.exportAllFilesToWidescreenToolStripMenuItem.Size = new System.Drawing.Size(337, 22);
         this.exportAllFilesToWidescreenToolStripMenuItem.Text = "Export All Files To Widescreen";
         this.exportAllFilesToWidescreenToolStripMenuItem.Click += new System.EventHandler(this.exportAllFilesToWidescreenToolStripMenuItem_Click);
         // 
         // toolStripSeparator2
         // 
         this.toolStripSeparator2.Name = "toolStripSeparator2";
         this.toolStripSeparator2.Size = new System.Drawing.Size(334, 6);
         // 
         // gatherStringIDsFromSourceFilesToolStripMenuItem
         // 
         this.gatherStringIDsFromSourceFilesToolStripMenuItem.Name = "gatherStringIDsFromSourceFilesToolStripMenuItem";
         this.gatherStringIDsFromSourceFilesToolStripMenuItem.Size = new System.Drawing.Size(337, 22);
         this.gatherStringIDsFromSourceFilesToolStripMenuItem.Text = "Gather string IDs from source files";
         this.gatherStringIDsFromSourceFilesToolStripMenuItem.Click += new System.EventHandler(this.gatherStringIDsFromSourceFilesToolStripMenuItem_Click);
         // 
         // optionsToolStripMenuItem1
         // 
         this.optionsToolStripMenuItem1.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.newStrCodesInXMLToolStripMenuItem});
         this.optionsToolStripMenuItem1.Name = "optionsToolStripMenuItem1";
         this.optionsToolStripMenuItem1.Size = new System.Drawing.Size(61, 20);
         this.optionsToolStripMenuItem1.Text = "Options";
         // 
         // newStrCodesInXMLToolStripMenuItem
         // 
         this.newStrCodesInXMLToolStripMenuItem.CheckOnClick = true;
         this.newStrCodesInXMLToolStripMenuItem.Name = "newStrCodesInXMLToolStripMenuItem";
         this.newStrCodesInXMLToolStripMenuItem.Size = new System.Drawing.Size(188, 22);
         this.newStrCodesInXMLToolStripMenuItem.Text = "New StrCodes in XML";
         // 
         // helpToolStripMenuItem
         // 
         this.helpToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.keyboardShortcutsToolStripMenuItem});
         this.helpToolStripMenuItem.Name = "helpToolStripMenuItem";
         this.helpToolStripMenuItem.Size = new System.Drawing.Size(44, 20);
         this.helpToolStripMenuItem.Text = "&Help";
         // 
         // keyboardShortcutsToolStripMenuItem
         // 
         this.keyboardShortcutsToolStripMenuItem.Name = "keyboardShortcutsToolStripMenuItem";
         this.keyboardShortcutsToolStripMenuItem.ShortcutKeys = System.Windows.Forms.Keys.F1;
         this.keyboardShortcutsToolStripMenuItem.Size = new System.Drawing.Size(205, 22);
         this.keyboardShortcutsToolStripMenuItem.Text = "Keyboard Shortcuts...";
         this.keyboardShortcutsToolStripMenuItem.Click += new System.EventHandler(this.keyboardShortcutsToolStripMenuItem_Click);
         // 
         // optionsToolStripMenuItem
         // 
         this.optionsToolStripMenuItem.Name = "optionsToolStripMenuItem";
         this.optionsToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
         this.optionsToolStripMenuItem.Text = "Options...";
         // 
         // exploreToolStripMenuItem
         // 
         this.exploreToolStripMenuItem.Name = "exploreToolStripMenuItem";
         this.exploreToolStripMenuItem.Size = new System.Drawing.Size(192, 22);
         this.exploreToolStripMenuItem.Text = "Explore";
         this.exploreToolStripMenuItem.Click += new System.EventHandler(this.exploreToolStripMenuItem_Click);
         // 
         // LayoutEditorWindow
         // 
         this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
         this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
         this.ClientSize = new System.Drawing.Size(973, 592);
         this.Controls.Add(this.statusStrip1);
         this.Controls.Add(this.menuStrip1);
         this.Controls.Add(this.splitContainer1);
         this.MainMenuStrip = this.menuStrip1;
         this.Name = "LayoutEditorWindow";
         this.Text = "Layout Editor MGS2";
         this.splitContainer1.Panel1.ResumeLayout(false);
         this.splitContainer1.Panel2.ResumeLayout(false);
         this.splitContainer1.ResumeLayout(false);
         this.splitContainer2.Panel1.ResumeLayout(false);
         this.splitContainer2.Panel1.PerformLayout();
         this.splitContainer2.Panel2.ResumeLayout(false);
         this.splitContainer2.Panel2.PerformLayout();
         this.splitContainer2.ResumeLayout(false);
         ((System.ComponentModel.ISupportInitialize)(this.previewPictureBox)).EndInit();
         this.statusStrip1.ResumeLayout(false);
         this.statusStrip1.PerformLayout();
         this.menuStrip1.ResumeLayout(false);
         this.menuStrip1.PerformLayout();
         this.ResumeLayout(false);
         this.PerformLayout();

      }

      #endregion

      private SelectablePictureBox previewPictureBox;
      private TreeViewWithoutDoubleClick objectTreeView;
      private System.Windows.Forms.SplitContainer splitContainer1;
      private System.Windows.Forms.SplitContainer splitContainer2;
      private System.Windows.Forms.StatusStrip statusStrip1;
      private System.Windows.Forms.TreeView actionTreeView;
      private System.Windows.Forms.Label label1;
      private System.Windows.Forms.Label label2;
      private System.Windows.Forms.MenuStrip menuStrip1;
      private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem helpToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem openToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem saveToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem toolsToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem optionsToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem backgroundColorToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem recentItemsToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem exportToolStripMenuItem;
      private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
      private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel1;
      private System.Windows.Forms.ToolStripMenuItem makeElementsWidescreenToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem editToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem undoToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem redoToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem findToolStripMenuItem;
      private System.Windows.Forms.ToolStripSeparator toolStripMenuItem1;
      private System.Windows.Forms.ToolStripMenuItem editNodeDataToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem exportReferencedTexturesToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem exportReferencedTexturesForAllFilesToolStripMenuItem;
      private System.Windows.Forms.ToolStripSeparator toolStripMenuItem2;
      private System.Windows.Forms.ToolStripSeparator toolStripMenuItem3;
      private System.Windows.Forms.ToolStripMenuItem selectedElementsMadeWidescreenToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem selectedElementsMadeWidescreenCenterToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem selectedElementsToLeftToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem selectedElementsToRightToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem selectedElementsFillToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem validateExportProcessOnAllFilesToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem exportAllFilesToWidescreenToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem keyboardShortcutsToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem viewToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem widescreenPreviewToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem hdToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem vitaToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem togglePixelPerfectOnSelectedToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem viewTextureToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem safeZoneToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem perforceCheckOutToolStripMenuItem;
      private System.Windows.Forms.ToolStripSeparator toolStripSeparator2;
      private System.Windows.Forms.ToolStripMenuItem gatherStringIDsFromSourceFilesToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem optionsToolStripMenuItem1;
      private System.Windows.Forms.ToolStripMenuItem newStrCodesInXMLToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem exploreToolStripMenuItem;

   }
}

