namespace Tools.Property.UI
{
   partial class ConnectionEditorGUI
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
         System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle2 = new System.Windows.Forms.DataGridViewCellStyle();
         System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle3 = new System.Windows.Forms.DataGridViewCellStyle();
         System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle1 = new System.Windows.Forms.DataGridViewCellStyle();
         System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(ConnectionEditorGUI));
         this.mAddConnectionButton = new System.Windows.Forms.Button();
         this.mRemoveConnectionButton = new System.Windows.Forms.Button();
         this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
         this.splitContainer1 = new System.Windows.Forms.SplitContainer();
         this.mSrcObjectsListView = new Tools.Property.UI.ConnectionEditorGUI.ConnectionListView();
         this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
         this.mDestObjectsListView = new Tools.Property.UI.ConnectionEditorGUI.ConnectionListView();
         this.columnHeader5 = new System.Windows.Forms.ColumnHeader();
         this.mConnectionsDataGridView = new System.Windows.Forms.DataGridView();
         this.ActiveColumn = new System.Windows.Forms.DataGridViewCheckBoxColumn();
         this.TimeColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
         this.EventColumn = new System.Windows.Forms.DataGridViewLinkColumn();
         this.MessageColumn = new System.Windows.Forms.DataGridViewLinkColumn();
         this.DestinationColumn = new System.Windows.Forms.DataGridViewLinkColumn();
         this.SenderQueryColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
         this.DestQueryColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
         this.PriorityColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
         this.ConnectionPropertiesBindingSource = new System.Windows.Forms.BindingSource(this.components);
         this.mPropertyPanel = new System.Windows.Forms.Panel();
         this.tableLayoutPanel1.SuspendLayout();
         this.splitContainer1.Panel1.SuspendLayout();
         this.splitContainer1.Panel2.SuspendLayout();
         this.splitContainer1.SuspendLayout();
         ((System.ComponentModel.ISupportInitialize)(this.mConnectionsDataGridView)).BeginInit();
         ((System.ComponentModel.ISupportInitialize)(this.ConnectionPropertiesBindingSource)).BeginInit();
         this.SuspendLayout();
         // 
         // mAddConnectionButton
         // 
         this.mAddConnectionButton.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                     | System.Windows.Forms.AnchorStyles.Right)));
         this.mAddConnectionButton.AutoSize = true;
         this.mAddConnectionButton.Location = new System.Drawing.Point(0, 277);
         this.mAddConnectionButton.Margin = new System.Windows.Forms.Padding(0);
         this.mAddConnectionButton.Name = "mAddConnectionButton";
         this.mAddConnectionButton.Size = new System.Drawing.Size(640, 23);
         this.mAddConnectionButton.TabIndex = 3;
         this.mAddConnectionButton.Text = "Add Connection";
         this.mAddConnectionButton.UseVisualStyleBackColor = true;
         this.mAddConnectionButton.Click += new System.EventHandler(this.AddConnectionButton_Click);
         // 
         // mRemoveConnectionButton
         // 
         this.mRemoveConnectionButton.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                     | System.Windows.Forms.AnchorStyles.Right)));
         this.mRemoveConnectionButton.AutoSize = true;
         this.mRemoveConnectionButton.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(200)))), ((int)(((byte)(200)))));
         this.mRemoveConnectionButton.Location = new System.Drawing.Point(0, 577);
         this.mRemoveConnectionButton.Margin = new System.Windows.Forms.Padding(0);
         this.mRemoveConnectionButton.MinimumSize = new System.Drawing.Size(140, 0);
         this.mRemoveConnectionButton.Name = "mRemoveConnectionButton";
         this.mRemoveConnectionButton.Size = new System.Drawing.Size(640, 23);
         this.mRemoveConnectionButton.TabIndex = 3;
         this.mRemoveConnectionButton.Text = "Remove Connection";
         this.mRemoveConnectionButton.UseVisualStyleBackColor = false;
         this.mRemoveConnectionButton.Click += new System.EventHandler(this.RemoveConnectionButton_Click);
         // 
         // tableLayoutPanel1
         // 
         this.tableLayoutPanel1.ColumnCount = 1;
         this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
         this.tableLayoutPanel1.Controls.Add(this.splitContainer1, 0, 0);
         this.tableLayoutPanel1.Controls.Add(this.mAddConnectionButton, 0, 1);
         this.tableLayoutPanel1.Controls.Add(this.mConnectionsDataGridView, 0, 2);
         this.tableLayoutPanel1.Controls.Add(this.mRemoveConnectionButton, 0, 3);
         this.tableLayoutPanel1.Controls.Add(this.mPropertyPanel, 0, 4);
         this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
         this.tableLayoutPanel1.Location = new System.Drawing.Point(3, 3);
         this.tableLayoutPanel1.Name = "tableLayoutPanel1";
         this.tableLayoutPanel1.RowCount = 5;
         this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 30F));
         this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
         this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 30F));
         this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
         this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 40F));
         this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
         this.tableLayoutPanel1.Size = new System.Drawing.Size(640, 971);
         this.tableLayoutPanel1.TabIndex = 4;
         // 
         // splitContainer1
         // 
         this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
         this.splitContainer1.Location = new System.Drawing.Point(3, 3);
         this.splitContainer1.Name = "splitContainer1";
         // 
         // splitContainer1.Panel1
         // 
         this.splitContainer1.Panel1.Controls.Add(this.mSrcObjectsListView);
         // 
         // splitContainer1.Panel2
         // 
         this.splitContainer1.Panel2.Controls.Add(this.mDestObjectsListView);
         this.splitContainer1.Size = new System.Drawing.Size(634, 271);
         this.splitContainer1.SplitterDistance = 316;
         this.splitContainer1.TabIndex = 0;
         // 
         // mSrcObjectsListView
         // 
         this.mSrcObjectsListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1});
         this.mSrcObjectsListView.Dock = System.Windows.Forms.DockStyle.Fill;
         this.mSrcObjectsListView.FullRowSelect = true;
         this.mSrcObjectsListView.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.None;
         this.mSrcObjectsListView.HideSelection = false;
         this.mSrcObjectsListView.Location = new System.Drawing.Point(0, 0);
         this.mSrcObjectsListView.Name = "mSrcObjectsListView";
         this.mSrcObjectsListView.ShowItemToolTips = true;
         this.mSrcObjectsListView.Size = new System.Drawing.Size(316, 271);
         this.mSrcObjectsListView.TabIndex = 0;
         this.mSrcObjectsListView.UseCompatibleStateImageBehavior = false;
         this.mSrcObjectsListView.View = System.Windows.Forms.View.Details;
         this.mSrcObjectsListView.SelectedIndexChanged += new System.EventHandler(this.SrcDestObjectsListBox_SelectedIndexChanged);
         this.mSrcObjectsListView.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(this.ObjectsListView_ColumnClick);
         // 
         // columnHeader1
         // 
         this.columnHeader1.Text = "Event/Link";
         this.columnHeader1.Width = 296;
         // 
         // mDestObjectsListView
         // 
         this.mDestObjectsListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader5});
         this.mDestObjectsListView.Dock = System.Windows.Forms.DockStyle.Fill;
         this.mDestObjectsListView.FullRowSelect = true;
         this.mDestObjectsListView.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.None;
         this.mDestObjectsListView.HideSelection = false;
         this.mDestObjectsListView.Location = new System.Drawing.Point(0, 0);
         this.mDestObjectsListView.Name = "mDestObjectsListView";
         this.mDestObjectsListView.ShowItemToolTips = true;
         this.mDestObjectsListView.Size = new System.Drawing.Size(314, 271);
         this.mDestObjectsListView.TabIndex = 7;
         this.mDestObjectsListView.UseCompatibleStateImageBehavior = false;
         this.mDestObjectsListView.View = System.Windows.Forms.View.Details;
         this.mDestObjectsListView.DoubleClick += new System.EventHandler(this.ObjectListView_DoubleClick);
         this.mDestObjectsListView.SelectedIndexChanged += new System.EventHandler(this.SrcDestObjectsListBox_SelectedIndexChanged);
         this.mDestObjectsListView.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(this.ObjectsListView_ColumnClick);
         // 
         // columnHeader5
         // 
         this.columnHeader5.Text = "Message";
         this.columnHeader5.Width = 294;
         // 
         // mConnectionsDataGridView
         // 
         this.mConnectionsDataGridView.AllowUserToAddRows = false;
         this.mConnectionsDataGridView.AllowUserToDeleteRows = false;
         this.mConnectionsDataGridView.AllowUserToResizeRows = false;
         this.mConnectionsDataGridView.AutoGenerateColumns = false;
         this.mConnectionsDataGridView.AutoSizeColumnsMode = System.Windows.Forms.DataGridViewAutoSizeColumnsMode.AllCellsExceptHeader;
         this.mConnectionsDataGridView.BackgroundColor = System.Drawing.Color.White;
         this.mConnectionsDataGridView.ClipboardCopyMode = System.Windows.Forms.DataGridViewClipboardCopyMode.EnableWithoutHeaderText;
         this.mConnectionsDataGridView.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
         this.mConnectionsDataGridView.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.ActiveColumn,
            this.TimeColumn,
            this.EventColumn,
            this.MessageColumn,
            this.DestinationColumn,
            this.SenderQueryColumn,
            this.DestQueryColumn,
            this.PriorityColumn});
         this.mConnectionsDataGridView.DataSource = this.ConnectionPropertiesBindingSource;
         dataGridViewCellStyle2.Alignment = System.Windows.Forms.DataGridViewContentAlignment.BottomLeft;
         dataGridViewCellStyle2.BackColor = System.Drawing.SystemColors.Window;
         dataGridViewCellStyle2.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
         dataGridViewCellStyle2.ForeColor = System.Drawing.SystemColors.ControlText;
         dataGridViewCellStyle2.SelectionBackColor = System.Drawing.SystemColors.Highlight;
         dataGridViewCellStyle2.SelectionForeColor = System.Drawing.SystemColors.HighlightText;
         dataGridViewCellStyle2.WrapMode = System.Windows.Forms.DataGridViewTriState.False;
         this.mConnectionsDataGridView.DefaultCellStyle = dataGridViewCellStyle2;
         this.mConnectionsDataGridView.Dock = System.Windows.Forms.DockStyle.Fill;
         this.mConnectionsDataGridView.Location = new System.Drawing.Point(3, 303);
         this.mConnectionsDataGridView.Name = "mConnectionsDataGridView";
         this.mConnectionsDataGridView.RowHeadersVisible = false;
         dataGridViewCellStyle3.SelectionBackColor = System.Drawing.Color.FromArgb(((int)(((byte)(211)))), ((int)(((byte)(241)))), ((int)(((byte)(252)))));
         dataGridViewCellStyle3.SelectionForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(64)))), ((int)(((byte)(64)))), ((int)(((byte)(64)))));
         this.mConnectionsDataGridView.RowsDefaultCellStyle = dataGridViewCellStyle3;
         this.mConnectionsDataGridView.SelectionMode = System.Windows.Forms.DataGridViewSelectionMode.FullRowSelect;
         this.mConnectionsDataGridView.Size = new System.Drawing.Size(634, 271);
         this.mConnectionsDataGridView.TabIndex = 0;
         this.mConnectionsDataGridView.CellClick += new System.Windows.Forms.DataGridViewCellEventHandler(this.OnConnectionsCellClick);
         this.mConnectionsDataGridView.CellContentClick += new System.Windows.Forms.DataGridViewCellEventHandler(this.OnConnectionsCellContentClick);
         this.mConnectionsDataGridView.CellPainting += new System.Windows.Forms.DataGridViewCellPaintingEventHandler(this.mConnectionsDataGridView_CellPainting);
         this.mConnectionsDataGridView.CellEndEdit += new System.Windows.Forms.DataGridViewCellEventHandler(this.ConnectionsDataGridView_CellEndEdit);
         this.mConnectionsDataGridView.RowHeightInfoNeeded += new System.Windows.Forms.DataGridViewRowHeightInfoNeededEventHandler(this.mConnectionsDataGridView_RowHeightInfoNeeded);
         this.mConnectionsDataGridView.RowPostPaint += new System.Windows.Forms.DataGridViewRowPostPaintEventHandler(this.mConnectionsDataGridView_RowPostPaint);
         this.mConnectionsDataGridView.SelectionChanged += new System.EventHandler(this.ConnectionsDataGridView_SelectionChanged);
         // 
         // ActiveColumn
         // 
         this.ActiveColumn.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.None;
         this.ActiveColumn.DataPropertyName = "Active";
         dataGridViewCellStyle1.Alignment = System.Windows.Forms.DataGridViewContentAlignment.BottomLeft;
         dataGridViewCellStyle1.NullValue = false;
         this.ActiveColumn.DefaultCellStyle = dataGridViewCellStyle1;
         this.ActiveColumn.HeaderText = "Active";
         this.ActiveColumn.MinimumWidth = 40;
         this.ActiveColumn.Name = "ActiveColumn";
         this.ActiveColumn.ToolTipText = "Disables the use of this connection.";
         this.ActiveColumn.Width = 48;
         // 
         // TimeColumn
         // 
         this.TimeColumn.DataPropertyName = "Time";
         this.TimeColumn.HeaderText = "Time";
         this.TimeColumn.MinimumWidth = 32;
         this.TimeColumn.Name = "TimeColumn";
         this.TimeColumn.ToolTipText = "Time from event trigger that message is sent. Normally 0.";
         this.TimeColumn.Width = 32;
         // 
         // EventColumn
         // 
         this.EventColumn.DataPropertyName = "EventName";
         this.EventColumn.HeaderText = "Event";
         this.EventColumn.LinkBehavior = System.Windows.Forms.LinkBehavior.HoverUnderline;
         this.EventColumn.LinkColor = System.Drawing.SystemColors.WindowText;
         this.EventColumn.MinimumWidth = 64;
         this.EventColumn.Name = "EventColumn";
         this.EventColumn.ToolTipText = "The name of the event on the sender component that triggers message.";
         this.EventColumn.TrackVisitedState = false;
         this.EventColumn.Width = 64;
         // 
         // MessageColumn
         // 
         this.MessageColumn.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.AllCells;
         this.MessageColumn.DataPropertyName = "Message";
         this.MessageColumn.HeaderText = "Message";
         this.MessageColumn.LinkBehavior = System.Windows.Forms.LinkBehavior.HoverUnderline;
         this.MessageColumn.LinkColor = System.Drawing.SystemColors.WindowText;
         this.MessageColumn.MinimumWidth = 64;
         this.MessageColumn.Name = "MessageColumn";
         this.MessageColumn.ToolTipText = "Message sent to target object on sender event trigger.";
         this.MessageColumn.TrackVisitedState = false;
         this.MessageColumn.Width = 64;
         // 
         // DestinationColumn
         // 
         this.DestinationColumn.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.AllCells;
         this.DestinationColumn.DataPropertyName = "DestinationObject";
         this.DestinationColumn.HeaderText = "Target";
         this.DestinationColumn.LinkBehavior = System.Windows.Forms.LinkBehavior.HoverUnderline;
         this.DestinationColumn.LinkColor = System.Drawing.SystemColors.WindowText;
         this.DestinationColumn.MinimumWidth = 64;
         this.DestinationColumn.Name = "DestinationColumn";
         this.DestinationColumn.ToolTipText = "Target object/component that will receive the message.";
         this.DestinationColumn.TrackVisitedState = false;
         this.DestinationColumn.Width = 64;
         // 
         // SenderQueryColumn
         // 
         this.SenderQueryColumn.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.AllCells;
         this.SenderQueryColumn.DataPropertyName = "SenderQuery";
         this.SenderQueryColumn.HeaderText = "SenderQuery";
         this.SenderQueryColumn.MinimumWidth = 64;
         this.SenderQueryColumn.Name = "SenderQueryColumn";
         this.SenderQueryColumn.ToolTipText = "Query script for message. If not empty, then this message will only be sent if se" +
             "nder passes query. E.g. \'CheckBoolFalse(\"PlayerDied\")\'. Query is performed befor" +
             "e sending, message time has no effect.";
         this.SenderQueryColumn.Width = 94;
         // 
         // DestQueryColumn
         // 
         this.DestQueryColumn.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.AllCells;
         this.DestQueryColumn.DataPropertyName = "DestQuery";
         this.DestQueryColumn.HeaderText = "DestQuery";
         this.DestQueryColumn.MinimumWidth = 64;
         this.DestQueryColumn.Name = "DestQueryColumn";
         this.DestQueryColumn.ToolTipText = resources.GetString("DestQueryColumn.ToolTipText");
         this.DestQueryColumn.Width = 82;
         // 
         // PriorityColumn
         // 
         this.PriorityColumn.DataPropertyName = "Priority";
         this.PriorityColumn.HeaderText = "Priority";
         this.PriorityColumn.MinimumWidth = 40;
         this.PriorityColumn.Name = "PriorityColumn";
         this.PriorityColumn.ToolTipText = "Priority of message. Message that are sent at the same time will use the priority" +
             " to determine which gets sent first.";
         this.PriorityColumn.Width = 40;
         // 
         // mPropertyPanel
         // 
         this.mPropertyPanel.AutoScroll = true;
         this.mPropertyPanel.Dock = System.Windows.Forms.DockStyle.Fill;
         this.mPropertyPanel.Location = new System.Drawing.Point(3, 603);
         this.mPropertyPanel.Name = "mPropertyPanel";
         this.mPropertyPanel.Size = new System.Drawing.Size(634, 365);
         this.mPropertyPanel.TabIndex = 6;
         // 
         // ConnectionEditorGUI
         // 
         this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
         this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
         this.Controls.Add(this.tableLayoutPanel1);
         this.Name = "ConnectionEditorGUI";
         this.Padding = new System.Windows.Forms.Padding(3);
         this.Size = new System.Drawing.Size(646, 977);
         this.tableLayoutPanel1.ResumeLayout(false);
         this.tableLayoutPanel1.PerformLayout();
         this.splitContainer1.Panel1.ResumeLayout(false);
         this.splitContainer1.Panel2.ResumeLayout(false);
         this.splitContainer1.ResumeLayout(false);
         ((System.ComponentModel.ISupportInitialize)(this.mConnectionsDataGridView)).EndInit();
         ((System.ComponentModel.ISupportInitialize)(this.ConnectionPropertiesBindingSource)).EndInit();
         this.ResumeLayout(false);

      }

      #endregion

      private System.Windows.Forms.Button mAddConnectionButton;
      private System.Windows.Forms.Button mRemoveConnectionButton;
      private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
      private System.Windows.Forms.DataGridView mConnectionsDataGridView;
      private System.Windows.Forms.BindingSource ConnectionPropertiesBindingSource;
      private System.Windows.Forms.ColumnHeader columnHeader1;
      private System.Windows.Forms.ColumnHeader columnHeader5;
      private System.Windows.Forms.SplitContainer splitContainer1;
      private ConnectionEditorGUI.ConnectionListView mDestObjectsListView;
      private ConnectionEditorGUI.ConnectionListView mSrcObjectsListView;
      private System.Windows.Forms.DataGridViewCheckBoxColumn ActiveColumn;
      private System.Windows.Forms.DataGridViewTextBoxColumn TimeColumn;
      private System.Windows.Forms.DataGridViewLinkColumn EventColumn;
      private System.Windows.Forms.DataGridViewLinkColumn MessageColumn;
      private System.Windows.Forms.DataGridViewLinkColumn DestinationColumn;
      private System.Windows.Forms.DataGridViewTextBoxColumn SenderQueryColumn;
      private System.Windows.Forms.DataGridViewTextBoxColumn DestQueryColumn;
      private System.Windows.Forms.DataGridViewTextBoxColumn PriorityColumn;
      private System.Windows.Forms.Panel mPropertyPanel;
   }
}
