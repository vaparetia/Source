namespace Tools.Property.UI
{
   partial class PropertyPanel
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
         this.mErrorProvider = new System.Windows.Forms.ErrorProvider(this.components);
         ((System.ComponentModel.ISupportInitialize)(this.mErrorProvider)).BeginInit();
         this.SuspendLayout();
         // 
         // PropertyPanel
         // 
         this.Margin = new System.Windows.Forms.Padding(0);
         this.Padding = new System.Windows.Forms.Padding(1, 0, 1, 0);
         this.Size = new System.Drawing.Size(409, 509);
         this.Paint += new System.Windows.Forms.PaintEventHandler(this.OnPaint);
         ((System.ComponentModel.ISupportInitialize)(this.mErrorProvider)).EndInit();
         this.ResumeLayout(false);

      }

      #endregion

      private System.Windows.Forms.ErrorProvider mErrorProvider;
   }
}
