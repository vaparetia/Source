using System.Drawing;

namespace SplineEditor
{
   partial class SplineEditorControl
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
         System.Windows.Forms.ToolStripStatusLabel Time;
         System.Windows.Forms.ToolStripStatusLabel Value;
         System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(SplineEditorControl));
         this.splitContainer1 = new System.Windows.Forms.SplitContainer();
         this.SplineListBox = new System.Windows.Forms.CheckedListBox();
         this.BitmapPictureBox = new System.Windows.Forms.PictureBox();
         this.StatusStrip = new System.Windows.Forms.StatusStrip();
         this.TimeStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
         this.ValueStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
         this.BottomToolStripPanel = new System.Windows.Forms.ToolStripPanel();
         this.TopToolStripPanel = new System.Windows.Forms.ToolStripPanel();
         this.ToolStrip = new System.Windows.Forms.ToolStrip();
         this.FrameAllButton = new System.Windows.Forms.ToolStripButton();
         this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
         this.TimeSnapButton = new System.Windows.Forms.ToolStripButton();
         this.ValueSnapButton = new System.Windows.Forms.ToolStripButton();
         this.toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
         this.TimeTextBox = new System.Windows.Forms.ToolStripTextBox();
         this.ValueTextBox = new System.Windows.Forms.ToolStripTextBox();
         this.toolStripSeparator3 = new System.Windows.Forms.ToolStripSeparator();
         this.TangentFixedButton = new System.Windows.Forms.ToolStripButton();
         this.TangentClampedButton = new System.Windows.Forms.ToolStripButton();
         this.TangentLinearButton = new System.Windows.Forms.ToolStripButton();
         this.TangentFlatButton = new System.Windows.Forms.ToolStripButton();
         this.TangentStepButton = new System.Windows.Forms.ToolStripButton();
         this.TangentPlateauButton = new System.Windows.Forms.ToolStripButton();
         this.toolStripSeparator4 = new System.Windows.Forms.ToolStripSeparator();
         this.toolStripDropDownButton1 = new System.Windows.Forms.ToolStripDropDownButton();
         this.PreInfinityContextMenu = new System.Windows.Forms.ContextMenuStrip(this.components);
         this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
         this.toolStripMenuItem2 = new System.Windows.Forms.ToolStripMenuItem();
         this.toolStripMenuItem3 = new System.Windows.Forms.ToolStripMenuItem();
         this.toolStripMenuItem4 = new System.Windows.Forms.ToolStripMenuItem();
         this.toolStripMenuItem8 = new System.Windows.Forms.ToolStripMenuItem();
         this.toolStripDropDownButton2 = new System.Windows.Forms.ToolStripDropDownButton();
         this.PostInfinityContextMenu = new System.Windows.Forms.ContextMenuStrip(this.components);
         this.toolStripMenuItem9 = new System.Windows.Forms.ToolStripMenuItem();
         this.toolStripMenuItem10 = new System.Windows.Forms.ToolStripMenuItem();
         this.toolStripMenuItem12 = new System.Windows.Forms.ToolStripMenuItem();
         this.toolStripMenuItem11 = new System.Windows.Forms.ToolStripMenuItem();
         this.toolStripMenuItem13 = new System.Windows.Forms.ToolStripMenuItem();
         this.RightToolStripPanel = new System.Windows.Forms.ToolStripPanel();
         this.LeftToolStripPanel = new System.Windows.Forms.ToolStripPanel();
         this.ContentPanel = new System.Windows.Forms.ToolStripContentPanel();
         this.splitContainer2 = new System.Windows.Forms.SplitContainer();
         this.toolStripMenuItem5 = new System.Windows.Forms.ToolStripMenuItem();
         this.toolStripMenuItem6 = new System.Windows.Forms.ToolStripMenuItem();
         this.toolStripMenuItem7 = new System.Windows.Forms.ToolStripMenuItem();
         Time = new System.Windows.Forms.ToolStripStatusLabel();
         Value = new System.Windows.Forms.ToolStripStatusLabel();
         this.splitContainer1.Panel1.SuspendLayout();
         this.splitContainer1.Panel2.SuspendLayout();
         this.splitContainer1.SuspendLayout();
         ((System.ComponentModel.ISupportInitialize)(this.BitmapPictureBox)).BeginInit();
         this.StatusStrip.SuspendLayout();
         this.ToolStrip.SuspendLayout();
         this.PreInfinityContextMenu.SuspendLayout();
         this.PostInfinityContextMenu.SuspendLayout();
         this.splitContainer2.Panel1.SuspendLayout();
         this.splitContainer2.Panel2.SuspendLayout();
         this.splitContainer2.SuspendLayout();
         this.SuspendLayout();
         // 
         // Time
         // 
         Time.BorderStyle = System.Windows.Forms.Border3DStyle.SunkenOuter;
         Time.Name = "Time";
         Time.Size = new System.Drawing.Size(33, 17);
         Time.Text = "Time:";
         Time.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
         // 
         // Value
         // 
         Value.Name = "Value";
         Value.Size = new System.Drawing.Size(37, 17);
         Value.Text = "Value:";
         // 
         // splitContainer1
         // 
         this.splitContainer1.BackColor = System.Drawing.Color.Transparent;
         this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
         this.splitContainer1.FixedPanel = System.Windows.Forms.FixedPanel.Panel1;
         this.splitContainer1.Location = new System.Drawing.Point(2, 2);
         this.splitContainer1.Name = "splitContainer1";
         // 
         // splitContainer1.Panel1
         // 
         this.splitContainer1.Panel1.Controls.Add(this.SplineListBox);
         this.splitContainer1.Panel1.Margin = new System.Windows.Forms.Padding(2);
         // 
         // splitContainer1.Panel2
         // 
         this.splitContainer1.Panel2.Controls.Add(this.BitmapPictureBox);
         this.splitContainer1.Panel2.Margin = new System.Windows.Forms.Padding(2);
         this.splitContainer1.Size = new System.Drawing.Size(644, 303);
         this.splitContainer1.SplitterDistance = 130;
         this.splitContainer1.TabIndex = 0;
         // 
         // SplineListBox
         // 
         this.SplineListBox.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
         this.SplineListBox.CheckOnClick = true;
         this.SplineListBox.Dock = System.Windows.Forms.DockStyle.Fill;
         this.SplineListBox.FormattingEnabled = true;
         this.SplineListBox.IntegralHeight = false;
         this.SplineListBox.Location = new System.Drawing.Point(0, 0);
         this.SplineListBox.Name = "SplineListBox";
         this.SplineListBox.Size = new System.Drawing.Size(130, 303);
         this.SplineListBox.TabIndex = 0;
         this.SplineListBox.ThreeDCheckBoxes = true;
         this.SplineListBox.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.OnSplineListBoxMouseDoubleClick);
         this.SplineListBox.SelectedValueChanged += new System.EventHandler(this.OnSplineListBoxSelectedValueChanged);
         // 
         // BitmapPictureBox
         // 
         this.BitmapPictureBox.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
         this.BitmapPictureBox.Dock = System.Windows.Forms.DockStyle.Fill;
         this.BitmapPictureBox.Location = new System.Drawing.Point(0, 0);
         this.BitmapPictureBox.Name = "BitmapPictureBox";
         this.BitmapPictureBox.Size = new System.Drawing.Size(510, 303);
         this.BitmapPictureBox.TabIndex = 2;
         this.BitmapPictureBox.TabStop = false;
         this.BitmapPictureBox.MouseWheel += new System.Windows.Forms.MouseEventHandler(this.OnMouseWheel);
         this.BitmapPictureBox.Click += new System.EventHandler(this.BitmapPictureBox_Click);
         this.BitmapPictureBox.MouseDown += new System.Windows.Forms.MouseEventHandler(this.OnMouseDown);
         this.BitmapPictureBox.MouseMove += new System.Windows.Forms.MouseEventHandler(this.OnMouseMove);
         this.BitmapPictureBox.KeyUp += new System.Windows.Forms.KeyEventHandler(this.OnKeyUp);
         this.BitmapPictureBox.MouseUp += new System.Windows.Forms.MouseEventHandler(this.OnMouseUp);
         this.BitmapPictureBox.SizeChanged += new System.EventHandler(this.OnResize);
         this.BitmapPictureBox.KeyDown += new System.Windows.Forms.KeyEventHandler(this.OnKeyDown);
         // 
         // StatusStrip
         // 
         this.StatusStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            Time,
            this.TimeStatusLabel,
            Value,
            this.ValueStatusLabel});
         this.StatusStrip.LayoutStyle = System.Windows.Forms.ToolStripLayoutStyle.HorizontalStackWithOverflow;
         this.StatusStrip.Location = new System.Drawing.Point(0, 338);
         this.StatusStrip.Name = "StatusStrip";
         this.StatusStrip.RenderMode = System.Windows.Forms.ToolStripRenderMode.Professional;
         this.StatusStrip.Size = new System.Drawing.Size(648, 22);
         this.StatusStrip.TabIndex = 0;
         this.StatusStrip.Text = "statusStrip1";
         // 
         // TimeStatusLabel
         // 
         this.TimeStatusLabel.AutoSize = false;
         this.TimeStatusLabel.BorderSides = ((System.Windows.Forms.ToolStripStatusLabelBorderSides)((((System.Windows.Forms.ToolStripStatusLabelBorderSides.Left | System.Windows.Forms.ToolStripStatusLabelBorderSides.Top)
                     | System.Windows.Forms.ToolStripStatusLabelBorderSides.Right)
                     | System.Windows.Forms.ToolStripStatusLabelBorderSides.Bottom)));
         this.TimeStatusLabel.BorderStyle = System.Windows.Forms.Border3DStyle.SunkenInner;
         this.TimeStatusLabel.Name = "TimeStatusLabel";
         this.TimeStatusLabel.Size = new System.Drawing.Size(48, 17);
         this.TimeStatusLabel.Text = "0.00";
         this.TimeStatusLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
         this.TimeStatusLabel.ToolTipText = "Spline time of mouse cursor";
         // 
         // ValueStatusLabel
         // 
         this.ValueStatusLabel.AutoSize = false;
         this.ValueStatusLabel.BorderSides = ((System.Windows.Forms.ToolStripStatusLabelBorderSides)((((System.Windows.Forms.ToolStripStatusLabelBorderSides.Left | System.Windows.Forms.ToolStripStatusLabelBorderSides.Top)
                     | System.Windows.Forms.ToolStripStatusLabelBorderSides.Right)
                     | System.Windows.Forms.ToolStripStatusLabelBorderSides.Bottom)));
         this.ValueStatusLabel.BorderStyle = System.Windows.Forms.Border3DStyle.SunkenInner;
         this.ValueStatusLabel.Name = "ValueStatusLabel";
         this.ValueStatusLabel.Size = new System.Drawing.Size(48, 17);
         this.ValueStatusLabel.Text = "0.00";
         this.ValueStatusLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
         this.ValueStatusLabel.ToolTipText = "Spline value of mouse cursor";
         // 
         // BottomToolStripPanel
         // 
         this.BottomToolStripPanel.Location = new System.Drawing.Point(0, 0);
         this.BottomToolStripPanel.Name = "BottomToolStripPanel";
         this.BottomToolStripPanel.Orientation = System.Windows.Forms.Orientation.Horizontal;
         this.BottomToolStripPanel.RowMargin = new System.Windows.Forms.Padding(3, 0, 0, 0);
         this.BottomToolStripPanel.Size = new System.Drawing.Size(0, 0);
         // 
         // TopToolStripPanel
         // 
         this.TopToolStripPanel.Location = new System.Drawing.Point(0, 0);
         this.TopToolStripPanel.Name = "TopToolStripPanel";
         this.TopToolStripPanel.Orientation = System.Windows.Forms.Orientation.Horizontal;
         this.TopToolStripPanel.RowMargin = new System.Windows.Forms.Padding(3, 0, 0, 0);
         this.TopToolStripPanel.Size = new System.Drawing.Size(0, 0);
         // 
         // ToolStrip
         // 
         this.ToolStrip.Dock = System.Windows.Forms.DockStyle.Fill;
         this.ToolStrip.GripStyle = System.Windows.Forms.ToolStripGripStyle.Hidden;
         this.ToolStrip.ImageScalingSize = new System.Drawing.Size(24, 24);
         this.ToolStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.FrameAllButton,
            this.toolStripSeparator1,
            this.TimeSnapButton,
            this.ValueSnapButton,
            this.toolStripSeparator2,
            this.TimeTextBox,
            this.ValueTextBox,
            this.toolStripSeparator3,
            this.TangentFixedButton,
            this.TangentClampedButton,
            this.TangentLinearButton,
            this.TangentFlatButton,
            this.TangentStepButton,
            this.TangentPlateauButton,
            this.toolStripSeparator4,
            this.toolStripDropDownButton1,
            this.toolStripDropDownButton2});
         this.ToolStrip.Location = new System.Drawing.Point(0, 0);
         this.ToolStrip.Name = "ToolStrip";
         this.ToolStrip.RenderMode = System.Windows.Forms.ToolStripRenderMode.Professional;
         this.ToolStrip.Size = new System.Drawing.Size(648, 30);
         this.ToolStrip.TabIndex = 0;
         this.ToolStrip.Text = "toolStrip1";
         // 
         // FrameAllButton
         // 
         this.FrameAllButton.BackColor = System.Drawing.Color.Transparent;
         this.FrameAllButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
         this.FrameAllButton.Image = global::SplineEditor.Properties.Resources.FrameAll;
         this.FrameAllButton.ImageScaling = System.Windows.Forms.ToolStripItemImageScaling.None;
         this.FrameAllButton.ImageTransparentColor = System.Drawing.Color.FromArgb(((int)(((byte)(236)))), ((int)(((byte)(233)))), ((int)(((byte)(216)))));
         this.FrameAllButton.Name = "FrameAllButton";
         this.FrameAllButton.Size = new System.Drawing.Size(28, 27);
         this.FrameAllButton.Text = "FrameAll";
         this.FrameAllButton.ToolTipText = "Frame all splines into view";
         this.FrameAllButton.Click += new System.EventHandler(this.FrameAllButton_Click);
         // 
         // toolStripSeparator1
         // 
         this.toolStripSeparator1.Name = "toolStripSeparator1";
         this.toolStripSeparator1.Size = new System.Drawing.Size(6, 30);
         // 
         // TimeSnapButton
         // 
         this.TimeSnapButton.CheckOnClick = true;
         this.TimeSnapButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
         this.TimeSnapButton.Image = global::SplineEditor.Properties.Resources.TimeSnap;
         this.TimeSnapButton.ImageScaling = System.Windows.Forms.ToolStripItemImageScaling.None;
         this.TimeSnapButton.ImageTransparentColor = System.Drawing.Color.FromArgb(((int)(((byte)(236)))), ((int)(((byte)(233)))), ((int)(((byte)(216)))));
         this.TimeSnapButton.Name = "TimeSnapButton";
         this.TimeSnapButton.Size = new System.Drawing.Size(28, 27);
         this.TimeSnapButton.Text = "TimeSnap";
         this.TimeSnapButton.ToolTipText = "Toggle time snap";
         this.TimeSnapButton.Click += new System.EventHandler(this.TimeSnapButton_Click);
         // 
         // ValueSnapButton
         // 
         this.ValueSnapButton.CheckOnClick = true;
         this.ValueSnapButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
         this.ValueSnapButton.Image = global::SplineEditor.Properties.Resources.ValueSnap;
         this.ValueSnapButton.ImageScaling = System.Windows.Forms.ToolStripItemImageScaling.None;
         this.ValueSnapButton.ImageTransparentColor = System.Drawing.Color.FromArgb(((int)(((byte)(236)))), ((int)(((byte)(233)))), ((int)(((byte)(216)))));
         this.ValueSnapButton.Name = "ValueSnapButton";
         this.ValueSnapButton.Size = new System.Drawing.Size(28, 27);
         this.ValueSnapButton.Text = "ValueSnap";
         this.ValueSnapButton.ToolTipText = "Toggle value snap";
         this.ValueSnapButton.Click += new System.EventHandler(this.ValueSnapButton_Click);
         // 
         // toolStripSeparator2
         // 
         this.toolStripSeparator2.Name = "toolStripSeparator2";
         this.toolStripSeparator2.Size = new System.Drawing.Size(6, 30);
         // 
         // TimeTextBox
         // 
         this.TimeTextBox.Name = "TimeTextBox";
         this.TimeTextBox.Size = new System.Drawing.Size(80, 30);
         this.TimeTextBox.KeyDown += new System.Windows.Forms.KeyEventHandler(this.OnNumericTimeKeyDown);
         this.TimeTextBox.Leave += new System.EventHandler(this.OnSetNumericTime);
         // 
         // ValueTextBox
         // 
         this.ValueTextBox.Name = "ValueTextBox";
         this.ValueTextBox.Size = new System.Drawing.Size(80, 30);
         this.ValueTextBox.KeyDown += new System.Windows.Forms.KeyEventHandler(this.OnNumericValueKeyDown);
         this.ValueTextBox.Leave += new System.EventHandler(this.OnSetNumericValue);
         // 
         // toolStripSeparator3
         // 
         this.toolStripSeparator3.Name = "toolStripSeparator3";
         this.toolStripSeparator3.Size = new System.Drawing.Size(6, 30);
         // 
         // TangentFixedButton
         // 
         this.TangentFixedButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
         this.TangentFixedButton.Image = global::SplineEditor.Properties.Resources.TangentFixed;
         this.TangentFixedButton.ImageScaling = System.Windows.Forms.ToolStripItemImageScaling.None;
         this.TangentFixedButton.ImageTransparentColor = System.Drawing.Color.FromArgb(((int)(((byte)(236)))), ((int)(((byte)(233)))), ((int)(((byte)(216)))));
         this.TangentFixedButton.Name = "TangentFixedButton";
         this.TangentFixedButton.Size = new System.Drawing.Size(28, 27);
         this.TangentFixedButton.Text = "T-Fixed";
         this.TangentFixedButton.ToolTipText = "Spline tangents";
         this.TangentFixedButton.Click += new System.EventHandler(this.TangentFixedButton_OnClick);
         // 
         // TangentClampedButton
         // 
         this.TangentClampedButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
         this.TangentClampedButton.Image = global::SplineEditor.Properties.Resources.TangentClamped;
         this.TangentClampedButton.ImageScaling = System.Windows.Forms.ToolStripItemImageScaling.None;
         this.TangentClampedButton.ImageTransparentColor = System.Drawing.Color.FromArgb(((int)(((byte)(236)))), ((int)(((byte)(233)))), ((int)(((byte)(216)))));
         this.TangentClampedButton.Name = "TangentClampedButton";
         this.TangentClampedButton.Size = new System.Drawing.Size(28, 27);
         this.TangentClampedButton.Text = "TangentClampedButton";
         this.TangentClampedButton.ToolTipText = "Clamped tangents";
         this.TangentClampedButton.Click += new System.EventHandler(this.TangentClampedButton_OnClick);
         // 
         // TangentLinearButton
         // 
         this.TangentLinearButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
         this.TangentLinearButton.Image = global::SplineEditor.Properties.Resources.TangentLinear;
         this.TangentLinearButton.ImageScaling = System.Windows.Forms.ToolStripItemImageScaling.None;
         this.TangentLinearButton.ImageTransparentColor = System.Drawing.Color.FromArgb(((int)(((byte)(236)))), ((int)(((byte)(233)))), ((int)(((byte)(216)))));
         this.TangentLinearButton.Name = "TangentLinearButton";
         this.TangentLinearButton.Size = new System.Drawing.Size(28, 27);
         this.TangentLinearButton.Text = "TangentLinearButton";
         this.TangentLinearButton.ToolTipText = "Linear tangents";
         this.TangentLinearButton.Click += new System.EventHandler(this.TangentLinearButton_OnClick);
         // 
         // TangentFlatButton
         // 
         this.TangentFlatButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
         this.TangentFlatButton.Image = global::SplineEditor.Properties.Resources.TangentFlat;
         this.TangentFlatButton.ImageScaling = System.Windows.Forms.ToolStripItemImageScaling.None;
         this.TangentFlatButton.ImageTransparentColor = System.Drawing.Color.FromArgb(((int)(((byte)(236)))), ((int)(((byte)(233)))), ((int)(((byte)(216)))));
         this.TangentFlatButton.Name = "TangentFlatButton";
         this.TangentFlatButton.Size = new System.Drawing.Size(28, 27);
         this.TangentFlatButton.Text = "TangentFlatButton";
         this.TangentFlatButton.ToolTipText = "Flat tangents";
         this.TangentFlatButton.Click += new System.EventHandler(this.TangentFlatButton_OnClick);
         // 
         // TangentStepButton
         // 
         this.TangentStepButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
         this.TangentStepButton.Image = global::SplineEditor.Properties.Resources.TangentStep;
         this.TangentStepButton.ImageScaling = System.Windows.Forms.ToolStripItemImageScaling.None;
         this.TangentStepButton.ImageTransparentColor = System.Drawing.Color.FromArgb(((int)(((byte)(236)))), ((int)(((byte)(233)))), ((int)(((byte)(216)))));
         this.TangentStepButton.Name = "TangentStepButton";
         this.TangentStepButton.Size = new System.Drawing.Size(28, 27);
         this.TangentStepButton.Text = "TangentStepButton";
         this.TangentStepButton.ToolTipText = "Step tangents";
         this.TangentStepButton.Click += new System.EventHandler(this.TangentStepButton_OnClick);
         // 
         // TangentPlateauButton
         // 
         this.TangentPlateauButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
         this.TangentPlateauButton.Image = global::SplineEditor.Properties.Resources.TangentPlateau;
         this.TangentPlateauButton.ImageScaling = System.Windows.Forms.ToolStripItemImageScaling.None;
         this.TangentPlateauButton.ImageTransparentColor = System.Drawing.Color.FromArgb(((int)(((byte)(236)))), ((int)(((byte)(233)))), ((int)(((byte)(216)))));
         this.TangentPlateauButton.Name = "TangentPlateauButton";
         this.TangentPlateauButton.Size = new System.Drawing.Size(28, 27);
         this.TangentPlateauButton.Text = "TangentPlateauButton";
         this.TangentPlateauButton.ToolTipText = "Plateau tangents";
         this.TangentPlateauButton.Click += new System.EventHandler(this.TangentPlateauButton_OnClick);
         // 
         // toolStripSeparator4
         // 
         this.toolStripSeparator4.Name = "toolStripSeparator4";
         this.toolStripSeparator4.Size = new System.Drawing.Size(6, 30);
         // 
         // toolStripDropDownButton1
         // 
         this.toolStripDropDownButton1.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
         this.toolStripDropDownButton1.DropDown = this.PreInfinityContextMenu;
         this.toolStripDropDownButton1.Image = ((System.Drawing.Image)(resources.GetObject("toolStripDropDownButton1.Image")));
         this.toolStripDropDownButton1.ImageTransparentColor = System.Drawing.Color.Magenta;
         this.toolStripDropDownButton1.Name = "toolStripDropDownButton1";
         this.toolStripDropDownButton1.Size = new System.Drawing.Size(73, 27);
         this.toolStripDropDownButton1.Text = "Pre Infinity";
         // 
         // PreInfinityContextMenu
         // 
         this.PreInfinityContextMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuItem1,
            this.toolStripMenuItem2,
            this.toolStripMenuItem3,
            this.toolStripMenuItem4,
            this.toolStripMenuItem8});
         this.PreInfinityContextMenu.Name = "contextMenuStrip1";
         this.PreInfinityContextMenu.OwnerItem = this.toolStripDropDownButton1;
         this.PreInfinityContextMenu.Size = new System.Drawing.Size(154, 114);
         // 
         // toolStripMenuItem1
         // 
         this.toolStripMenuItem1.Name = "toolStripMenuItem1";
         this.toolStripMenuItem1.Size = new System.Drawing.Size(153, 22);
         this.toolStripMenuItem1.Tag = "Constant";
         this.toolStripMenuItem1.Text = "Constant";
         this.toolStripMenuItem1.Click += new System.EventHandler(this.OnPreInfinityClick);
         // 
         // toolStripMenuItem2
         // 
         this.toolStripMenuItem2.Name = "toolStripMenuItem2";
         this.toolStripMenuItem2.Size = new System.Drawing.Size(153, 22);
         this.toolStripMenuItem2.Tag = "Linear";
         this.toolStripMenuItem2.Text = "Linear";
         this.toolStripMenuItem2.Click += new System.EventHandler(this.OnPreInfinityClick);
         // 
         // toolStripMenuItem3
         // 
         this.toolStripMenuItem3.Name = "toolStripMenuItem3";
         this.toolStripMenuItem3.Size = new System.Drawing.Size(153, 22);
         this.toolStripMenuItem3.Tag = "Cycle";
         this.toolStripMenuItem3.Text = "Cycle";
         this.toolStripMenuItem3.Click += new System.EventHandler(this.OnPreInfinityClick);
         // 
         // toolStripMenuItem4
         // 
         this.toolStripMenuItem4.Name = "toolStripMenuItem4";
         this.toolStripMenuItem4.Size = new System.Drawing.Size(153, 22);
         this.toolStripMenuItem4.Tag = "CycleRelative";
         this.toolStripMenuItem4.Text = "Cycle Relative";
         this.toolStripMenuItem4.Click += new System.EventHandler(this.OnPreInfinityClick);
         // 
         // toolStripMenuItem8
         // 
         this.toolStripMenuItem8.Name = "toolStripMenuItem8";
         this.toolStripMenuItem8.Size = new System.Drawing.Size(153, 22);
         this.toolStripMenuItem8.Tag = "Oscillate";
         this.toolStripMenuItem8.Text = "Oscillate";
         this.toolStripMenuItem8.Click += new System.EventHandler(this.OnPreInfinityClick);
         // 
         // toolStripDropDownButton2
         // 
         this.toolStripDropDownButton2.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
         this.toolStripDropDownButton2.DropDown = this.PostInfinityContextMenu;
         this.toolStripDropDownButton2.Image = ((System.Drawing.Image)(resources.GetObject("toolStripDropDownButton2.Image")));
         this.toolStripDropDownButton2.ImageTransparentColor = System.Drawing.Color.Magenta;
         this.toolStripDropDownButton2.Name = "toolStripDropDownButton2";
         this.toolStripDropDownButton2.Size = new System.Drawing.Size(78, 27);
         this.toolStripDropDownButton2.Text = "Post Infinity";
         // 
         // PostInfinityContextMenu
         // 
         this.PostInfinityContextMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuItem9,
            this.toolStripMenuItem10,
            this.toolStripMenuItem12,
            this.toolStripMenuItem11,
            this.toolStripMenuItem13});
         this.PostInfinityContextMenu.Name = "contextMenuStrip1";
         this.PostInfinityContextMenu.OwnerItem = this.toolStripDropDownButton2;
         this.PostInfinityContextMenu.Size = new System.Drawing.Size(154, 114);
         // 
         // toolStripMenuItem9
         // 
         this.toolStripMenuItem9.Name = "toolStripMenuItem9";
         this.toolStripMenuItem9.Size = new System.Drawing.Size(153, 22);
         this.toolStripMenuItem9.Tag = "Constant";
         this.toolStripMenuItem9.Text = "Constant";
         this.toolStripMenuItem9.Click += new System.EventHandler(this.OnPostInfinityClick);
         // 
         // toolStripMenuItem10
         // 
         this.toolStripMenuItem10.Name = "toolStripMenuItem10";
         this.toolStripMenuItem10.Size = new System.Drawing.Size(153, 22);
         this.toolStripMenuItem10.Tag = "Linear";
         this.toolStripMenuItem10.Text = "Linear";
         this.toolStripMenuItem10.Click += new System.EventHandler(this.OnPostInfinityClick);
         // 
         // toolStripMenuItem12
         // 
         this.toolStripMenuItem12.Name = "toolStripMenuItem12";
         this.toolStripMenuItem12.Size = new System.Drawing.Size(153, 22);
         this.toolStripMenuItem12.Tag = "CycleRelative";
         this.toolStripMenuItem12.Text = "Cycle Relative";
         this.toolStripMenuItem12.Click += new System.EventHandler(this.OnPostInfinityClick);
         // 
         // toolStripMenuItem11
         // 
         this.toolStripMenuItem11.Name = "toolStripMenuItem11";
         this.toolStripMenuItem11.Size = new System.Drawing.Size(153, 22);
         this.toolStripMenuItem11.Tag = "Cycle";
         this.toolStripMenuItem11.Text = "Cycle";
         this.toolStripMenuItem11.Click += new System.EventHandler(this.OnPostInfinityClick);
         // 
         // toolStripMenuItem13
         // 
         this.toolStripMenuItem13.Name = "toolStripMenuItem13";
         this.toolStripMenuItem13.Size = new System.Drawing.Size(153, 22);
         this.toolStripMenuItem13.Tag = "Oscillate";
         this.toolStripMenuItem13.Text = "Oscillate";
         this.toolStripMenuItem13.Click += new System.EventHandler(this.OnPostInfinityClick);
         // 
         // RightToolStripPanel
         // 
         this.RightToolStripPanel.Location = new System.Drawing.Point(0, 0);
         this.RightToolStripPanel.Name = "RightToolStripPanel";
         this.RightToolStripPanel.Orientation = System.Windows.Forms.Orientation.Horizontal;
         this.RightToolStripPanel.RowMargin = new System.Windows.Forms.Padding(3, 0, 0, 0);
         this.RightToolStripPanel.Size = new System.Drawing.Size(0, 0);
         // 
         // LeftToolStripPanel
         // 
         this.LeftToolStripPanel.Location = new System.Drawing.Point(0, 0);
         this.LeftToolStripPanel.Name = "LeftToolStripPanel";
         this.LeftToolStripPanel.Orientation = System.Windows.Forms.Orientation.Horizontal;
         this.LeftToolStripPanel.RowMargin = new System.Windows.Forms.Padding(3, 0, 0, 0);
         this.LeftToolStripPanel.Size = new System.Drawing.Size(0, 0);
         // 
         // ContentPanel
         // 
         this.ContentPanel.Size = new System.Drawing.Size(549, 288);
         // 
         // splitContainer2
         // 
         this.splitContainer2.BackColor = System.Drawing.Color.Transparent;
         this.splitContainer2.Dock = System.Windows.Forms.DockStyle.Fill;
         this.splitContainer2.FixedPanel = System.Windows.Forms.FixedPanel.Panel1;
         this.splitContainer2.IsSplitterFixed = true;
         this.splitContainer2.Location = new System.Drawing.Point(0, 0);
         this.splitContainer2.Name = "splitContainer2";
         this.splitContainer2.Orientation = System.Windows.Forms.Orientation.Horizontal;
         // 
         // splitContainer2.Panel1
         // 
         this.splitContainer2.Panel1.Controls.Add(this.ToolStrip);
         // 
         // splitContainer2.Panel2
         // 
         this.splitContainer2.Panel2.BackColor = System.Drawing.Color.Transparent;
         this.splitContainer2.Panel2.Controls.Add(this.splitContainer1);
         this.splitContainer2.Panel2.Padding = new System.Windows.Forms.Padding(2);
         this.splitContainer2.Size = new System.Drawing.Size(648, 338);
         this.splitContainer2.SplitterDistance = 30;
         this.splitContainer2.SplitterWidth = 1;
         this.splitContainer2.TabIndex = 7;
         // 
         // toolStripMenuItem5
         // 
         this.toolStripMenuItem5.Name = "toolStripMenuItem5";
         this.toolStripMenuItem5.Size = new System.Drawing.Size(32, 19);
         // 
         // toolStripMenuItem6
         // 
         this.toolStripMenuItem6.Name = "toolStripMenuItem6";
         this.toolStripMenuItem6.Size = new System.Drawing.Size(32, 19);
         this.toolStripMenuItem6.Text = "toolStripMenuItem6";
         // 
         // toolStripMenuItem7
         // 
         this.toolStripMenuItem7.Name = "toolStripMenuItem7";
         this.toolStripMenuItem7.Size = new System.Drawing.Size(32, 19);
         // 
         // SplineEditorControl
         // 
         this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
         this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
         this.BackColor = System.Drawing.Color.Transparent;
         this.Controls.Add(this.splitContainer2);
         this.Controls.Add(this.StatusStrip);
         this.Name = "SplineEditorControl";
         this.Size = new System.Drawing.Size(648, 360);
         this.Resize += new System.EventHandler(this.OnResize);
         this.splitContainer1.Panel1.ResumeLayout(false);
         this.splitContainer1.Panel2.ResumeLayout(false);
         this.splitContainer1.ResumeLayout(false);
         ((System.ComponentModel.ISupportInitialize)(this.BitmapPictureBox)).EndInit();
         this.StatusStrip.ResumeLayout(false);
         this.StatusStrip.PerformLayout();
         this.ToolStrip.ResumeLayout(false);
         this.ToolStrip.PerformLayout();
         this.PreInfinityContextMenu.ResumeLayout(false);
         this.PostInfinityContextMenu.ResumeLayout(false);
         this.splitContainer2.Panel1.ResumeLayout(false);
         this.splitContainer2.Panel1.PerformLayout();
         this.splitContainer2.Panel2.ResumeLayout(false);
         this.splitContainer2.ResumeLayout(false);
         this.ResumeLayout(false);
         this.PerformLayout();

      }

      #endregion

      private System.Windows.Forms.PictureBox BitmapPictureBox;
      private System.Windows.Forms.SplitContainer splitContainer1;
      private System.Windows.Forms.StatusStrip StatusStrip;
      private System.Windows.Forms.ToolStripStatusLabel ValueStatusLabel;
      private System.Windows.Forms.CheckedListBox SplineListBox;
      private System.Windows.Forms.ToolStrip ToolStrip;
      private System.Windows.Forms.ToolStripButton FrameAllButton;
      private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
      private System.Windows.Forms.ToolStripButton TimeSnapButton;
      private System.Windows.Forms.ToolStripButton ValueSnapButton;
      private System.Windows.Forms.ToolStripSeparator toolStripSeparator2;
      private System.Windows.Forms.ToolStripTextBox TimeTextBox;
      private System.Windows.Forms.ToolStripTextBox ValueTextBox;
      private System.Windows.Forms.ToolStripSeparator toolStripSeparator3;
      private System.Windows.Forms.ToolStripButton TangentFixedButton;
      private System.Windows.Forms.ToolStripButton TangentLinearButton;
      private System.Windows.Forms.ToolStripButton TangentFlatButton;
      private System.Windows.Forms.ToolStripButton TangentStepButton;
      private System.Windows.Forms.ToolStripButton TangentClampedButton;
      private System.Windows.Forms.ToolStripButton TangentPlateauButton;
      private System.Windows.Forms.ToolStripPanel BottomToolStripPanel;
      private System.Windows.Forms.ToolStripPanel TopToolStripPanel;
      private System.Windows.Forms.ToolStripPanel RightToolStripPanel;
      private System.Windows.Forms.ToolStripPanel LeftToolStripPanel;
      private System.Windows.Forms.ToolStripContentPanel ContentPanel;
      private System.Windows.Forms.SplitContainer splitContainer2;
      private System.Windows.Forms.ToolStripStatusLabel TimeStatusLabel;
      private System.Windows.Forms.ToolStripSeparator toolStripSeparator4;
      private System.Windows.Forms.ToolStripDropDownButton toolStripDropDownButton1;
      private System.Windows.Forms.ToolStripDropDownButton toolStripDropDownButton2;
      private System.Windows.Forms.ContextMenuStrip PreInfinityContextMenu;
      private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem1;
      private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem2;
      private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem3;
      private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem4;
      private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem5;
      private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem6;
      private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem7;
      private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem8;
      private System.Windows.Forms.ContextMenuStrip PostInfinityContextMenu;
      private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem9;
      private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem10;
      private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem12;
      private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem11;
      private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem13;
   }
}
