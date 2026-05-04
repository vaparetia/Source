namespace Tools.Property.UI.Help
{
   partial class HelpPanel
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
         System.Windows.Forms.Label label3;
         System.Windows.Forms.Label label4;
         System.Windows.Forms.Label label5;
         System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(HelpPanel));
         this.imageList1 = new System.Windows.Forms.ImageList(this.components);
         this.mTitleBar = new System.Windows.Forms.Label();
         this.mHelpTextControl = new System.Windows.Forms.Label();
         label3 = new System.Windows.Forms.Label();
         label4 = new System.Windows.Forms.Label();
         label5 = new System.Windows.Forms.Label();
         this.SuspendLayout();
         // 
         // label3
         // 
         label3.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                     | System.Windows.Forms.AnchorStyles.Right)));
         label3.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
         label3.Location = new System.Drawing.Point(6, 264);
         label3.Name = "label3";
         label3.Size = new System.Drawing.Size(318, 2);
         label3.TabIndex = 2;
         // 
         // label4
         // 
         label4.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                     | System.Windows.Forms.AnchorStyles.Right)));
         label4.AutoSize = true;
         label4.BackColor = System.Drawing.Color.Transparent;
         label4.Location = new System.Drawing.Point(24, 273);
         label4.Name = "label4";
         label4.Size = new System.Drawing.Size(115, 13);
         label4.TabIndex = 3;
         label4.Text = "Press F1 for more help.";
         // 
         // label5
         // 
         label5.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                     | System.Windows.Forms.AnchorStyles.Right)));
         label5.BackColor = System.Drawing.Color.Transparent;
         label5.ImageKey = "Help";
         label5.ImageList = this.imageList1;
         label5.Location = new System.Drawing.Point(6, 270);
         label5.Name = "label5";
         label5.Size = new System.Drawing.Size(19, 19);
         label5.TabIndex = 4;
         // 
         // imageList1
         // 
         this.imageList1.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("imageList1.ImageStream")));
         this.imageList1.TransparentColor = System.Drawing.Color.Magenta;
         this.imageList1.Images.SetKeyName(0, "Help");
         // 
         // mTitleBar
         // 
         this.mTitleBar.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                     | System.Windows.Forms.AnchorStyles.Right)));
         this.mTitleBar.BackColor = System.Drawing.Color.Transparent;
         this.mTitleBar.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
         this.mTitleBar.ForeColor = System.Drawing.SystemColors.WindowText;
         this.mTitleBar.Location = new System.Drawing.Point(12, 9);
         this.mTitleBar.Name = "mTitleBar";
         this.mTitleBar.Size = new System.Drawing.Size(304, 21);
         this.mTitleBar.TabIndex = 1;
         this.mTitleBar.Text = "HelpTitle";
         this.mTitleBar.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
         // 
         // mHelpTextControl
         // 
         this.mHelpTextControl.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                     | System.Windows.Forms.AnchorStyles.Left)
                     | System.Windows.Forms.AnchorStyles.Right)));
         this.mHelpTextControl.BackColor = System.Drawing.Color.Transparent;
         this.mHelpTextControl.Location = new System.Drawing.Point(15, 30);
         this.mHelpTextControl.Name = "mHelpTextControl";
         this.mHelpTextControl.Size = new System.Drawing.Size(301, 228);
         this.mHelpTextControl.TabIndex = 5;
         this.mHelpTextControl.Paint += new System.Windows.Forms.PaintEventHandler(this.OnPaint);
         // 
         // HelpPanel
         // 
         this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
         this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
         this.BackColor = System.Drawing.SystemColors.Window;
         this.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("$this.BackgroundImage")));
         this.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
         this.ClientSize = new System.Drawing.Size(328, 294);
         this.ControlBox = false;
         this.Controls.Add(this.mHelpTextControl);
         this.Controls.Add(label5);
         this.Controls.Add(label4);
         this.Controls.Add(label3);
         this.Controls.Add(this.mTitleBar);
         this.DoubleBuffered = true;
         this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
         this.MaximumSize = new System.Drawing.Size(330, 500);
         this.MinimumSize = new System.Drawing.Size(330, 2);
         this.Name = "HelpPanel";
         this.ShowInTaskbar = false;
         this.StartPosition = System.Windows.Forms.FormStartPosition.Manual;
         this.ResumeLayout(false);
         this.PerformLayout();

      }

      #endregion

      private System.Windows.Forms.Label mTitleBar;
      private System.Windows.Forms.ImageList imageList1;
      private System.Windows.Forms.Label mHelpTextControl;




   }
}