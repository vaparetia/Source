namespace Tools.Editors.AnimationEventEditor
{
   partial class AnimationEventEditor
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
         System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(AnimationEventEditor));
         this.mAnimationEditorToolStrip = new System.Windows.Forms.ToolStrip();
         this.toolStripButton1 = new System.Windows.Forms.ToolStripButton();
         this.mEventView = new Tools.Controls.EventView();
         this.mJointsListView = new System.Windows.Forms.ListView();
         this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
         this.label3 = new System.Windows.Forms.Label();
         this.jointFilterTextBox = new System.Windows.Forms.TextBox();
         this.mAnimationEditorToolStrip.SuspendLayout();
         this.SuspendLayout();
         // 
         // mAnimationEditorToolStrip
         // 
         this.mAnimationEditorToolStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripButton1});
         this.mAnimationEditorToolStrip.Location = new System.Drawing.Point(0, 0);
         this.mAnimationEditorToolStrip.Name = "mAnimationEditorToolStrip";
         this.mAnimationEditorToolStrip.Size = new System.Drawing.Size(996, 25);
         this.mAnimationEditorToolStrip.TabIndex = 1;
         this.mAnimationEditorToolStrip.Text = "toolStrip1";
         this.mAnimationEditorToolStrip.Visible = false;
         // 
         // toolStripButton1
         // 
         this.toolStripButton1.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
         this.toolStripButton1.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButton1.Image")));
         this.toolStripButton1.ImageTransparentColor = System.Drawing.Color.Magenta;
         this.toolStripButton1.MergeAction = System.Windows.Forms.MergeAction.Insert;
         this.toolStripButton1.MergeIndex = 0;
         this.toolStripButton1.Name = "toolStripButton1";
         this.toolStripButton1.Size = new System.Drawing.Size(23, 22);
         this.toolStripButton1.Text = "Add Event";
         this.toolStripButton1.Click += new System.EventHandler(this.OnAddEvent);
         // 
         // mEventView
         // 
         this.mEventView.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                     | System.Windows.Forms.AnchorStyles.Left)
                     | System.Windows.Forms.AnchorStyles.Right)));
         this.mEventView.AutoScroll = true;
         this.mEventView.Location = new System.Drawing.Point(209, 0);
         this.mEventView.Name = "mEventView";
         this.mEventView.Size = new System.Drawing.Size(787, 589);
         this.mEventView.TabIndex = 3;
         // 
         // mJointsListView
         // 
         this.mJointsListView.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                     | System.Windows.Forms.AnchorStyles.Left)));
         this.mJointsListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1});
         this.mJointsListView.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.None;
         this.mJointsListView.Location = new System.Drawing.Point(3, 42);
         this.mJointsListView.MultiSelect = false;
         this.mJointsListView.Name = "mJointsListView";
         this.mJointsListView.Size = new System.Drawing.Size(200, 544);
         this.mJointsListView.TabIndex = 2;
         this.mJointsListView.UseCompatibleStateImageBehavior = false;
         this.mJointsListView.View = System.Windows.Forms.View.Details;
         this.mJointsListView.SelectedIndexChanged += new System.EventHandler(this.OnJointSelected);
         // 
         // columnHeader1
         // 
         this.columnHeader1.Text = "Joint Name";
         // 
         // label3
         // 
         this.label3.AutoSize = true;
         this.label3.Location = new System.Drawing.Point(3, 0);
         this.label3.Name = "label3";
         this.label3.Size = new System.Drawing.Size(37, 13);
         this.label3.TabIndex = 0;
         this.label3.Text = "Joints:";
         // 
         // jointFilterTextBox
         // 
         this.jointFilterTextBox.Location = new System.Drawing.Point(6, 16);
         this.jointFilterTextBox.Name = "jointFilterTextBox";
         this.jointFilterTextBox.Size = new System.Drawing.Size(197, 20);
         this.jointFilterTextBox.TabIndex = 1;
         this.jointFilterTextBox.TextChanged += new System.EventHandler(this.OnJointFilterTextChanged);
         // 
         // AnimationEventEditor
         // 
         this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
         this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
         this.Controls.Add(this.jointFilterTextBox);
         this.Controls.Add(this.mJointsListView);
         this.Controls.Add(this.label3);
         this.Controls.Add(this.mAnimationEditorToolStrip);
         this.Controls.Add(this.mEventView);
         this.Name = "AnimationEventEditor";
         this.Size = new System.Drawing.Size(996, 589);
         this.mAnimationEditorToolStrip.ResumeLayout(false);
         this.mAnimationEditorToolStrip.PerformLayout();
         this.ResumeLayout(false);
         this.PerformLayout();

      }

      #endregion

      public Tools.Controls.EventView mEventView;
      private System.Windows.Forms.ToolStrip mAnimationEditorToolStrip;
      private System.Windows.Forms.ToolStripButton toolStripButton1;
      private System.Windows.Forms.ListView mJointsListView;
      private System.Windows.Forms.ColumnHeader columnHeader1;
      private System.Windows.Forms.Label label3;
      private System.Windows.Forms.TextBox jointFilterTextBox;
   }
}
