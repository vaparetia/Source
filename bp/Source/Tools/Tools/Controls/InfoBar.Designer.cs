namespace Tools.Controls
{
   partial class InfoBar
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
         this.mClose = new System.Windows.Forms.Button();
         this.SuspendLayout();
         // 
         // mClose
         // 
         this.mClose.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                     | System.Windows.Forms.AnchorStyles.Right)));
         this.mClose.BackColor = System.Drawing.SystemColors.Info;
         this.mClose.Cursor = System.Windows.Forms.Cursors.Hand;
         this.mClose.FlatAppearance.BorderColor = System.Drawing.SystemColors.Info;
         this.mClose.FlatAppearance.MouseDownBackColor = System.Drawing.SystemColors.Info;
         this.mClose.FlatAppearance.MouseOverBackColor = System.Drawing.SystemColors.Info;
         this.mClose.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
         this.mClose.Font = new System.Drawing.Font("Marlett", 8.25F);
         this.mClose.Location = new System.Drawing.Point(146, 4);
         this.mClose.Name = "mClose";
         this.mClose.Size = new System.Drawing.Size(23, 23);
         this.mClose.TabIndex = 0;
         this.mClose.Text = "r";
         this.mClose.UseVisualStyleBackColor = false;
         this.mClose.Click += new System.EventHandler(this.mClose_Click);
         // 
         // InfoBar
         // 
         this.BackColor = System.Drawing.SystemColors.Info;
         this.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
         this.Controls.Add(this.mClose);
         this.Dock = System.Windows.Forms.DockStyle.Top;
         this.Size = new System.Drawing.Size(174, 30);
         this.ResumeLayout(false);

      }

      #endregion

      private System.Windows.Forms.Button mClose;
   }
}
