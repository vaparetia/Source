namespace AssetTool
{
   partial class CookStatusWindow
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
         System.Windows.Forms.ListViewGroup listViewGroup9 = new System.Windows.Forms.ListViewGroup( "Failed", System.Windows.Forms.HorizontalAlignment.Left );
         System.Windows.Forms.ListViewGroup listViewGroup10 = new System.Windows.Forms.ListViewGroup( "Processing", System.Windows.Forms.HorizontalAlignment.Left );
         System.Windows.Forms.ListViewGroup listViewGroup11 = new System.Windows.Forms.ListViewGroup( "Queued", System.Windows.Forms.HorizontalAlignment.Left );
         System.Windows.Forms.ListViewGroup listViewGroup12 = new System.Windows.Forms.ListViewGroup( "Completed", System.Windows.Forms.HorizontalAlignment.Left );
         this.listView1 = new System.Windows.Forms.ListView();
         this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
         this.progressBar1 = new System.Windows.Forms.ProgressBar();
         this.richTextBox1 = new System.Windows.Forms.RichTextBox();
         this.button1 = new System.Windows.Forms.Button();
         this.label1 = new System.Windows.Forms.Label();
         this.mShowTimer = new System.Windows.Forms.Timer( this.components );
         this.SuspendLayout();
         // 
         // listView1
         // 
         this.listView1.Anchor = ( (System.Windows.Forms.AnchorStyles) ( ( ( System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left )
                     | System.Windows.Forms.AnchorStyles.Right ) ) );
         this.listView1.Columns.AddRange( new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1} );
         this.listView1.FullRowSelect = true;
         listViewGroup9.Header = "Failed";
         listViewGroup9.Name = "Failed";
         listViewGroup10.Header = "Processing";
         listViewGroup10.Name = "Processing";
         listViewGroup11.Header = "Queued";
         listViewGroup11.Name = "Queued";
         listViewGroup12.Header = "Completed";
         listViewGroup12.Name = "Completed";
         this.listView1.Groups.AddRange( new System.Windows.Forms.ListViewGroup[] {
            listViewGroup9,
            listViewGroup10,
            listViewGroup11,
            listViewGroup12} );
         this.listView1.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.None;
         this.listView1.Location = new System.Drawing.Point( 12, 12 );
         this.listView1.MultiSelect = false;
         this.listView1.Name = "listView1";
         this.listView1.Size = new System.Drawing.Size( 848, 162 );
         this.listView1.TabIndex = 0;
         this.listView1.UseCompatibleStateImageBehavior = false;
         this.listView1.View = System.Windows.Forms.View.Details;
         // 
         // columnHeader1
         // 
         this.columnHeader1.Text = "Asset";
         this.columnHeader1.Width = 739;
         // 
         // progressBar1
         // 
         this.progressBar1.Anchor = ( (System.Windows.Forms.AnchorStyles) ( ( ( System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left )
                     | System.Windows.Forms.AnchorStyles.Right ) ) );
         this.progressBar1.Location = new System.Drawing.Point( 162, 180 );
         this.progressBar1.Name = "progressBar1";
         this.progressBar1.Size = new System.Drawing.Size( 617, 23 );
         this.progressBar1.TabIndex = 2;
         // 
         // richTextBox1
         // 
         this.richTextBox1.Anchor = ( (System.Windows.Forms.AnchorStyles) ( ( ( ( System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom )
                     | System.Windows.Forms.AnchorStyles.Left )
                     | System.Windows.Forms.AnchorStyles.Right ) ) );
         this.richTextBox1.Location = new System.Drawing.Point( 12, 209 );
         this.richTextBox1.Name = "richTextBox1";
         this.richTextBox1.ReadOnly = true;
         this.richTextBox1.Size = new System.Drawing.Size( 848, 0 );
         this.richTextBox1.TabIndex = 3;
         this.richTextBox1.Text = "";
         // 
         // button1
         // 
         this.button1.Anchor = ( (System.Windows.Forms.AnchorStyles) ( ( System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right ) ) );
         this.button1.Location = new System.Drawing.Point( 785, 180 );
         this.button1.Name = "button1";
         this.button1.Size = new System.Drawing.Size( 75, 23 );
         this.button1.TabIndex = 4;
         this.button1.Text = "Details >>";
         this.button1.UseVisualStyleBackColor = true;
         this.button1.Click += new System.EventHandler( this.OnPressedDetails );
         // 
         // label1
         // 
         this.label1.Location = new System.Drawing.Point( 12, 180 );
         this.label1.Name = "label1";
         this.label1.Size = new System.Drawing.Size( 144, 23 );
         this.label1.TabIndex = 5;
         this.label1.Text = "Checking Assets...";
         this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
         // 
         // mShowTimer
         // 
         this.mShowTimer.Enabled = true;
         this.mShowTimer.Interval = 1000;
         this.mShowTimer.Tick += new System.EventHandler( this.mShowTimer_Tick );
         // 
         // CookStatusWindow
         // 
         this.AutoScaleDimensions = new System.Drawing.SizeF( 6F, 13F );
         this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
         this.ClientSize = new System.Drawing.Size( 872, 215 );
         this.Controls.Add( this.label1 );
         this.Controls.Add( this.button1 );
         this.Controls.Add( this.listView1 );
         this.Controls.Add( this.progressBar1 );
         this.Controls.Add( this.richTextBox1 );
         this.MinimumSize = new System.Drawing.Size( 16, 249 );
         this.Name = "CookStatusWindow";
         this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
         this.Text = "Asset Tool";
         this.ResizeEnd += new System.EventHandler( this.OnResizeEnd );
         this.ResumeLayout( false );

      }

      #endregion

      private System.Windows.Forms.ListView listView1;
      private System.Windows.Forms.ProgressBar progressBar1;
      private System.Windows.Forms.RichTextBox richTextBox1;
      private System.Windows.Forms.ColumnHeader columnHeader1;
      private System.Windows.Forms.Button button1;
      private System.Windows.Forms.Label label1;
      private System.Windows.Forms.Timer mShowTimer;
   }
}