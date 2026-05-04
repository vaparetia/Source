namespace TextureViewer
{
   partial class FlatListMakerDialog
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
         System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FlatListMakerDialog));
         this.radioMGS_Choice_01 = new System.Windows.Forms.RadioButton();
         this.radioMGS_Choice_02 = new System.Windows.Forms.RadioButton();
         this.groupBox1 = new System.Windows.Forms.GroupBox();
         this.groupBox2 = new System.Windows.Forms.GroupBox();
         this.radio_generateOnly = new System.Windows.Forms.RadioButton();
         this.radio_moveFiles = new System.Windows.Forms.RadioButton();
         this.radio_copyFiles = new System.Windows.Forms.RadioButton();
         this.chkBox_GenerateXML = new System.Windows.Forms.CheckBox();
         this.chkBox_CleanFlatlist = new System.Windows.Forms.CheckBox();
         this.chkBox_RemoveCtxr = new System.Windows.Forms.CheckBox();
         this.btnConversionIsGo = new System.Windows.Forms.Button();
         this.groupBox1.SuspendLayout();
         this.groupBox2.SuspendLayout();
         this.SuspendLayout();
         // 
         // radioMGS_Choice_01
         // 
         this.radioMGS_Choice_01.AutoSize = true;
         this.radioMGS_Choice_01.Checked = true;
         this.radioMGS_Choice_01.ForeColor = System.Drawing.Color.White;
         this.radioMGS_Choice_01.Location = new System.Drawing.Point(6, 13);
         this.radioMGS_Choice_01.Name = "radioMGS_Choice_01";
         this.radioMGS_Choice_01.Size = new System.Drawing.Size(186, 17);
         this.radioMGS_Choice_01.TabIndex = 0;
         this.radioMGS_Choice_01.TabStop = true;
         this.radioMGS_Choice_01.Text = "Convert MGS3 ( /slot and /stage )";
         this.radioMGS_Choice_01.UseVisualStyleBackColor = true;
         // 
         // radioMGS_Choice_02
         // 
         this.radioMGS_Choice_02.AutoSize = true;
         this.radioMGS_Choice_02.ForeColor = System.Drawing.Color.White;
         this.radioMGS_Choice_02.Location = new System.Drawing.Point(6, 36);
         this.radioMGS_Choice_02.Name = "radioMGS_Choice_02";
         this.radioMGS_Choice_02.Size = new System.Drawing.Size(191, 17);
         this.radioMGS_Choice_02.TabIndex = 1;
         this.radioMGS_Choice_02.Text = "Convert MGS2 ( /face and /stage )";
         this.radioMGS_Choice_02.UseVisualStyleBackColor = true;
         this.radioMGS_Choice_02.CheckedChanged += new System.EventHandler(this.radioMGS_Choice_02_CheckedChanged);
         // 
         // groupBox1
         // 
         this.groupBox1.BackColor = System.Drawing.Color.Transparent;
         this.groupBox1.Controls.Add(this.radioMGS_Choice_02);
         this.groupBox1.Controls.Add(this.radioMGS_Choice_01);
         this.groupBox1.Location = new System.Drawing.Point(12, 10);
         this.groupBox1.Name = "groupBox1";
         this.groupBox1.Size = new System.Drawing.Size(249, 72);
         this.groupBox1.TabIndex = 2;
         this.groupBox1.TabStop = false;
         // 
         // groupBox2
         // 
         this.groupBox2.BackColor = System.Drawing.Color.Transparent;
         this.groupBox2.Controls.Add(this.radio_generateOnly);
         this.groupBox2.Controls.Add(this.radio_moveFiles);
         this.groupBox2.Controls.Add(this.radio_copyFiles);
         this.groupBox2.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
         this.groupBox2.ForeColor = System.Drawing.Color.Transparent;
         this.groupBox2.Location = new System.Drawing.Point(12, 88);
         this.groupBox2.Name = "groupBox2";
         this.groupBox2.Size = new System.Drawing.Size(249, 86);
         this.groupBox2.TabIndex = 3;
         this.groupBox2.TabStop = false;
         // 
         // radio_generateOnly
         // 
         this.radio_generateOnly.AutoSize = true;
         this.radio_generateOnly.ForeColor = System.Drawing.Color.White;
         this.radio_generateOnly.Location = new System.Drawing.Point(6, 59);
         this.radio_generateOnly.Name = "radio_generateOnly";
         this.radio_generateOnly.Size = new System.Drawing.Size(199, 17);
         this.radio_generateOnly.TabIndex = 2;
         this.radio_generateOnly.Text = "ONLY generate a new remapping file";
         this.radio_generateOnly.UseVisualStyleBackColor = true;
         // 
         // radio_moveFiles
         // 
         this.radio_moveFiles.AutoSize = true;
         this.radio_moveFiles.ForeColor = System.Drawing.Color.White;
         this.radio_moveFiles.Location = new System.Drawing.Point(6, 36);
         this.radio_moveFiles.Name = "radio_moveFiles";
         this.radio_moveFiles.Size = new System.Drawing.Size(118, 17);
         this.radio_moveFiles.TabIndex = 1;
         this.radio_moveFiles.Text = "MOVE files to flatlist";
         this.radio_moveFiles.UseVisualStyleBackColor = true;
         // 
         // radio_copyFiles
         // 
         this.radio_copyFiles.AutoSize = true;
         this.radio_copyFiles.Checked = true;
         this.radio_copyFiles.ForeColor = System.Drawing.Color.White;
         this.radio_copyFiles.Location = new System.Drawing.Point(6, 13);
         this.radio_copyFiles.Name = "radio_copyFiles";
         this.radio_copyFiles.Size = new System.Drawing.Size(116, 17);
         this.radio_copyFiles.TabIndex = 0;
         this.radio_copyFiles.TabStop = true;
         this.radio_copyFiles.Text = "COPY files to flatlist";
         this.radio_copyFiles.UseVisualStyleBackColor = true;
         // 
         // chkBox_GenerateXML
         // 
         this.chkBox_GenerateXML.AutoSize = true;
         this.chkBox_GenerateXML.BackColor = System.Drawing.Color.Transparent;
         this.chkBox_GenerateXML.Checked = true;
         this.chkBox_GenerateXML.CheckState = System.Windows.Forms.CheckState.Checked;
         this.chkBox_GenerateXML.ForeColor = System.Drawing.Color.White;
         this.chkBox_GenerateXML.Location = new System.Drawing.Point(18, 190);
         this.chkBox_GenerateXML.Name = "chkBox_GenerateXML";
         this.chkBox_GenerateXML.Size = new System.Drawing.Size(208, 17);
         this.chkBox_GenerateXML.TabIndex = 4;
         this.chkBox_GenerateXML.Text = "Generate .xmlmeta files for the textures";
         this.chkBox_GenerateXML.UseVisualStyleBackColor = false;
         // 
         // chkBox_CleanFlatlist
         // 
         this.chkBox_CleanFlatlist.AutoSize = true;
         this.chkBox_CleanFlatlist.BackColor = System.Drawing.Color.Transparent;
         this.chkBox_CleanFlatlist.ForeColor = System.Drawing.Color.White;
         this.chkBox_CleanFlatlist.Location = new System.Drawing.Point(18, 213);
         this.chkBox_CleanFlatlist.Name = "chkBox_CleanFlatlist";
         this.chkBox_CleanFlatlist.Size = new System.Drawing.Size(226, 17);
         this.chkBox_CleanFlatlist.TabIndex = 5;
         this.chkBox_CleanFlatlist.Text = "Clean out the tga and xml files in flatlist first";
         this.chkBox_CleanFlatlist.UseVisualStyleBackColor = false;
         this.chkBox_CleanFlatlist.CheckedChanged += new System.EventHandler(this.chkBox_CleanFlatlist_CheckedChanged);
         // 
         // chkBox_RemoveCtxr
         // 
         this.chkBox_RemoveCtxr.AutoSize = true;
         this.chkBox_RemoveCtxr.BackColor = System.Drawing.Color.Transparent;
         this.chkBox_RemoveCtxr.Enabled = false;
         this.chkBox_RemoveCtxr.ForeColor = System.Drawing.Color.White;
         this.chkBox_RemoveCtxr.Location = new System.Drawing.Point(29, 236);
         this.chkBox_RemoveCtxr.Name = "chkBox_RemoveCtxr";
         this.chkBox_RemoveCtxr.Size = new System.Drawing.Size(175, 17);
         this.chkBox_RemoveCtxr.TabIndex = 6;
         this.chkBox_RemoveCtxr.Text = "Get rid of the cooked .ctxrs too!";
         this.chkBox_RemoveCtxr.UseVisualStyleBackColor = false;
         // 
         // btnConversionIsGo
         // 
         this.btnConversionIsGo.Location = new System.Drawing.Point(37, 270);
         this.btnConversionIsGo.Name = "btnConversionIsGo";
         this.btnConversionIsGo.Size = new System.Drawing.Size(189, 35);
         this.btnConversionIsGo.TabIndex = 7;
         this.btnConversionIsGo.Text = "Convert";
         this.btnConversionIsGo.UseVisualStyleBackColor = true;
         this.btnConversionIsGo.Click += new System.EventHandler(this.btnConversionIsGo_Click);
         // 
         // FlatListMakerDialog
         // 
         this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
         this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
         this.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("$this.BackgroundImage")));
         this.ClientSize = new System.Drawing.Size(506, 317);
         this.Controls.Add(this.btnConversionIsGo);
         this.Controls.Add(this.chkBox_RemoveCtxr);
         this.Controls.Add(this.chkBox_CleanFlatlist);
         this.Controls.Add(this.chkBox_GenerateXML);
         this.Controls.Add(this.groupBox2);
         this.Controls.Add(this.groupBox1);
         this.MaximizeBox = false;
         this.MinimizeBox = false;
         this.Name = "FlatListMakerDialog";
         this.Text = "Flat List Conversion Options";
         this.TopMost = true;
         this.Load += new System.EventHandler(this.FlatListMakerDialog_Load);
         this.groupBox1.ResumeLayout(false);
         this.groupBox1.PerformLayout();
         this.groupBox2.ResumeLayout(false);
         this.groupBox2.PerformLayout();
         this.ResumeLayout(false);
         this.PerformLayout();

      }

      #endregion

      private System.Windows.Forms.RadioButton radioMGS_Choice_01;
      private System.Windows.Forms.RadioButton radioMGS_Choice_02;
      private System.Windows.Forms.GroupBox groupBox1;
      private System.Windows.Forms.GroupBox groupBox2;
      private System.Windows.Forms.RadioButton radio_moveFiles;
      private System.Windows.Forms.RadioButton radio_copyFiles;
      private System.Windows.Forms.CheckBox chkBox_GenerateXML;
      private System.Windows.Forms.CheckBox chkBox_CleanFlatlist;
      private System.Windows.Forms.CheckBox chkBox_RemoveCtxr;
      private System.Windows.Forms.Button btnConversionIsGo;
      private System.Windows.Forms.RadioButton radio_generateOnly;
   }
}