namespace Tools.Property.UI
{
   partial class ModalPropertyDlg
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
         this.mMainPanel = new System.Windows.Forms.Panel();
         this.mOkButton = new System.Windows.Forms.Button();
         this.mCancelButton = new System.Windows.Forms.Button();
         this.mSeparator = new System.Windows.Forms.Label();
         this.SuspendLayout();
         // 
         // mMainPanel
         // 
         this.mMainPanel.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                     | System.Windows.Forms.AnchorStyles.Left)
                     | System.Windows.Forms.AnchorStyles.Right)));
         this.mMainPanel.AutoScroll = true;
         this.mMainPanel.Location = new System.Drawing.Point(3, 3);
         this.mMainPanel.Name = "mMainPanel";
         this.mMainPanel.Size = new System.Drawing.Size(395, 484);
         this.mMainPanel.TabIndex = 0;
         // 
         // mOkButton
         // 
         this.mOkButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
         this.mOkButton.DialogResult = System.Windows.Forms.DialogResult.OK;
         this.mOkButton.Location = new System.Drawing.Point(3, 500);
         this.mOkButton.Name = "mOkButton";
         this.mOkButton.Size = new System.Drawing.Size(75, 23);
         this.mOkButton.TabIndex = 0;
         this.mOkButton.Text = "OK";
         this.mOkButton.Click += new System.EventHandler(this.OnOk);
         // 
         // mCancelButton
         // 
         this.mCancelButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
         this.mCancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
         this.mCancelButton.Location = new System.Drawing.Point(84, 500);
         this.mCancelButton.Name = "mCancelButton";
         this.mCancelButton.Size = new System.Drawing.Size(75, 23);
         this.mCancelButton.TabIndex = 1;
         this.mCancelButton.Text = "Cancel";
         // 
         // mSeparator
         // 
         this.mSeparator.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                     | System.Windows.Forms.AnchorStyles.Right)));
         this.mSeparator.BackColor = System.Drawing.SystemColors.ControlDark;
         this.mSeparator.ForeColor = System.Drawing.SystemColors.ControlText;
         this.mSeparator.Location = new System.Drawing.Point(6, 494);
         this.mSeparator.Name = "mSeparator";
         this.mSeparator.Size = new System.Drawing.Size(389, 1);
         this.mSeparator.TabIndex = 2;
         // 
         // ModalPropertyDlg
         // 
         this.AcceptButton = this.mOkButton;
         this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
         this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
         this.CancelButton = this.mCancelButton;
         this.ClientSize = new System.Drawing.Size(401, 526);
         this.Controls.Add(this.mSeparator);
         this.Controls.Add(this.mOkButton);
         this.Controls.Add(this.mCancelButton);
         this.Controls.Add(this.mMainPanel);
         this.KeyPreview = true;
         this.Name = "ModalPropertyDlg";
         this.Text = "Properties";
         this.KeyDown += new System.Windows.Forms.KeyEventHandler(this.OnKeyDown);
         this.ResumeLayout(false);

      }

      #endregion

      private System.Windows.Forms.Panel mMainPanel;
      private System.Windows.Forms.Button mCancelButton;
      public System.Windows.Forms.Button mOkButton;
      private System.Windows.Forms.Label mSeparator;
   }
}