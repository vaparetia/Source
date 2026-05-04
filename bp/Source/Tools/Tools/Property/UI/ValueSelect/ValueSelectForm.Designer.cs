namespace Tools.Property.UI.ValueSelect
{
   partial class ValueSelectForm
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
         this.SelectValuePanel = new System.Windows.Forms.Panel();
         this.SuspendLayout();
         // 
         // SelectValuePanel
         // 
         this.SelectValuePanel.Dock = System.Windows.Forms.DockStyle.Fill;
         this.SelectValuePanel.Location = new System.Drawing.Point( 0, 0 );
         this.SelectValuePanel.Name = "SelectValuePanel";
         this.SelectValuePanel.Size = new System.Drawing.Size( 345, 277 );
         this.SelectValuePanel.TabIndex = 0;
         this.SelectValuePanel.Paint += new System.Windows.Forms.PaintEventHandler( this.SelectValuePanel_Paint );
         this.SelectValuePanel.MouseClick += new System.Windows.Forms.MouseEventHandler( this.ValueSelectForm_MouseClick );
         // 
         // ValueSelectForm
         // 
         this.AutoScaleDimensions = new System.Drawing.SizeF( 6F, 13F );
         this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
         this.ClientSize = new System.Drawing.Size( 345, 277 );
         this.Controls.Add( this.SelectValuePanel );
         this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
         this.Name = "ValueSelectForm";
         this.Text = "ValueSelectForm";
         this.KeyDown += new System.Windows.Forms.KeyEventHandler( this.ValueSelectForm_KeyDown );
         this.ResumeLayout( false );

      }

      #endregion

      public System.Windows.Forms.Panel SelectValuePanel;

   }
}