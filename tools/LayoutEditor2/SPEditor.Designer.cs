namespace LayoutEditorMGS2
{
    partial class SPEditor
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
           this.newXPos = new System.Windows.Forms.TextBox();
           this.OK = new System.Windows.Forms.Button();
           this.newYPos = new System.Windows.Forms.TextBox();
           this.newHeight = new System.Windows.Forms.TextBox();
           this.newWidth = new System.Windows.Forms.TextBox();
           this.Cancel = new System.Windows.Forms.Button();
           this.Current = new System.Windows.Forms.Label();
           this.New = new System.Windows.Forms.Label();
           this.XPos = new System.Windows.Forms.Label();
           this.width = new System.Windows.Forms.Label();
           this.height = new System.Windows.Forms.Label();
           this.YPos = new System.Windows.Forms.Label();
           this.currentHeight = new System.Windows.Forms.Label();
           this.currentWidth = new System.Windows.Forms.Label();
           this.currentY = new System.Windows.Forms.Label();
           this.currentX = new System.Windows.Forms.Label();
           this.centerX = new System.Windows.Forms.Label();
           this.centerY = new System.Windows.Forms.Label();
           this.currentCenterX = new System.Windows.Forms.Label();
           this.currentCenterY = new System.Windows.Forms.Label();
           this.newCenterX = new System.Windows.Forms.TextBox();
           this.newCenterY = new System.Windows.Forms.TextBox();
           this.SuspendLayout();
           // 
           // newXPos
           // 
           this.newXPos.Location = new System.Drawing.Point(164, 41);
           this.newXPos.Name = "newXPos";
           this.newXPos.Size = new System.Drawing.Size(56, 20);
           this.newXPos.TabIndex = 0;
           // 
           // OK
           // 
           this.OK.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
           this.OK.Location = new System.Drawing.Point(12, 203);
           this.OK.Name = "OK";
           this.OK.Size = new System.Drawing.Size(51, 23);
           this.OK.TabIndex = 6;
           this.OK.Text = "OK";
           this.OK.UseVisualStyleBackColor = true;
           this.OK.Click += new System.EventHandler(this.button1_Click);
           // 
           // newYPos
           // 
           this.newYPos.Location = new System.Drawing.Point(164, 67);
           this.newYPos.Name = "newYPos";
           this.newYPos.Size = new System.Drawing.Size(56, 20);
           this.newYPos.TabIndex = 1;
           // 
           // newHeight
           // 
           this.newHeight.Location = new System.Drawing.Point(164, 119);
           this.newHeight.Name = "newHeight";
           this.newHeight.Size = new System.Drawing.Size(56, 20);
           this.newHeight.TabIndex = 3;
           // 
           // newWidth
           // 
           this.newWidth.Location = new System.Drawing.Point(164, 93);
           this.newWidth.Name = "newWidth";
           this.newWidth.Size = new System.Drawing.Size(56, 20);
           this.newWidth.TabIndex = 2;
           // 
           // Cancel
           // 
           this.Cancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
           this.Cancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
           this.Cancel.Location = new System.Drawing.Point(170, 203);
           this.Cancel.Name = "Cancel";
           this.Cancel.Size = new System.Drawing.Size(56, 23);
           this.Cancel.TabIndex = 7;
           this.Cancel.Text = "Cancel";
           this.Cancel.UseVisualStyleBackColor = true;
           this.Cancel.Click += new System.EventHandler(this.button2_Click);
           // 
           // Current
           // 
           this.Current.AutoSize = true;
           this.Current.Location = new System.Drawing.Point(67, 15);
           this.Current.Name = "Current";
           this.Current.Size = new System.Drawing.Size(76, 13);
           this.Current.TabIndex = 20;
           this.Current.Text = "Current Values";
           this.Current.Click += new System.EventHandler(this.label1_Click);
           // 
           // New
           // 
           this.New.AutoSize = true;
           this.New.Location = new System.Drawing.Point(160, 15);
           this.New.Name = "New";
           this.New.Size = new System.Drawing.Size(64, 13);
           this.New.TabIndex = 21;
           this.New.Text = "New Values";
           this.New.Click += new System.EventHandler(this.label1_Click_1);
           // 
           // XPos
           // 
           this.XPos.AutoSize = true;
           this.XPos.Location = new System.Drawing.Point(12, 41);
           this.XPos.Name = "XPos";
           this.XPos.Size = new System.Drawing.Size(54, 13);
           this.XPos.TabIndex = 14;
           this.XPos.Text = "X Position";
           this.XPos.Click += new System.EventHandler(this.label1_Click_2);
           // 
           // width
           // 
           this.width.AutoSize = true;
           this.width.Location = new System.Drawing.Point(12, 93);
           this.width.Name = "width";
           this.width.Size = new System.Drawing.Size(35, 13);
           this.width.TabIndex = 16;
           this.width.Text = "Width";
           // 
           // height
           // 
           this.height.AutoSize = true;
           this.height.Location = new System.Drawing.Point(12, 119);
           this.height.Name = "height";
           this.height.Size = new System.Drawing.Size(38, 13);
           this.height.TabIndex = 17;
           this.height.Text = "Height";
           // 
           // YPos
           // 
           this.YPos.AutoSize = true;
           this.YPos.Location = new System.Drawing.Point(12, 67);
           this.YPos.Name = "YPos";
           this.YPos.Size = new System.Drawing.Size(54, 13);
           this.YPos.TabIndex = 15;
           this.YPos.Text = "Y Position";
           // 
           // currentHeight
           // 
           this.currentHeight.AutoSize = true;
           this.currentHeight.Location = new System.Drawing.Point(93, 119);
           this.currentHeight.Name = "currentHeight";
           this.currentHeight.Size = new System.Drawing.Size(31, 13);
           this.currentHeight.TabIndex = 11;
           this.currentHeight.Text = "1234";
           this.currentHeight.Click += new System.EventHandler(this.label1_Click_3);
           // 
           // currentWidth
           // 
           this.currentWidth.AutoSize = true;
           this.currentWidth.Location = new System.Drawing.Point(93, 93);
           this.currentWidth.Name = "currentWidth";
           this.currentWidth.Size = new System.Drawing.Size(31, 13);
           this.currentWidth.TabIndex = 10;
           this.currentWidth.Text = "1234";
           this.currentWidth.Click += new System.EventHandler(this.label2_Click);
           // 
           // currentY
           // 
           this.currentY.AutoSize = true;
           this.currentY.Location = new System.Drawing.Point(93, 67);
           this.currentY.Name = "currentY";
           this.currentY.Size = new System.Drawing.Size(31, 13);
           this.currentY.TabIndex = 9;
           this.currentY.Text = "1234";
           // 
           // currentX
           // 
           this.currentX.AutoSize = true;
           this.currentX.Location = new System.Drawing.Point(93, 41);
           this.currentX.Name = "currentX";
           this.currentX.Size = new System.Drawing.Size(31, 13);
           this.currentX.TabIndex = 8;
           this.currentX.Text = "1234";
           this.currentX.Click += new System.EventHandler(this.label4_Click);
           // 
           // centerX
           // 
           this.centerX.AutoSize = true;
           this.centerX.Location = new System.Drawing.Point(12, 145);
           this.centerX.Name = "centerX";
           this.centerX.Size = new System.Drawing.Size(48, 13);
           this.centerX.TabIndex = 18;
           this.centerX.Text = "Center X";
           // 
           // centerY
           // 
           this.centerY.AutoSize = true;
           this.centerY.Location = new System.Drawing.Point(12, 171);
           this.centerY.Name = "centerY";
           this.centerY.Size = new System.Drawing.Size(48, 13);
           this.centerY.TabIndex = 19;
           this.centerY.Text = "Center Y";
           // 
           // currentCenterX
           // 
           this.currentCenterX.AutoSize = true;
           this.currentCenterX.Location = new System.Drawing.Point(93, 145);
           this.currentCenterX.Name = "currentCenterX";
           this.currentCenterX.Size = new System.Drawing.Size(31, 13);
           this.currentCenterX.TabIndex = 12;
           this.currentCenterX.Text = "1234";
           // 
           // currentCenterY
           // 
           this.currentCenterY.AutoSize = true;
           this.currentCenterY.Location = new System.Drawing.Point(93, 171);
           this.currentCenterY.Name = "currentCenterY";
           this.currentCenterY.Size = new System.Drawing.Size(31, 13);
           this.currentCenterY.TabIndex = 13;
           this.currentCenterY.Text = "1234";
           // 
           // newCenterX
           // 
           this.newCenterX.Location = new System.Drawing.Point(164, 145);
           this.newCenterX.Name = "newCenterX";
           this.newCenterX.Size = new System.Drawing.Size(56, 20);
           this.newCenterX.TabIndex = 4;
           // 
           // newCenterY
           // 
           this.newCenterY.Location = new System.Drawing.Point(164, 171);
           this.newCenterY.Name = "newCenterY";
           this.newCenterY.Size = new System.Drawing.Size(56, 20);
           this.newCenterY.TabIndex = 5;
           // 
           // SPEditor
           // 
           this.AcceptButton = this.OK;
           this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
           this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
           this.CancelButton = this.Cancel;
           this.ClientSize = new System.Drawing.Size(238, 238);
           this.Controls.Add(this.newCenterY);
           this.Controls.Add(this.newCenterX);
           this.Controls.Add(this.currentCenterY);
           this.Controls.Add(this.currentCenterX);
           this.Controls.Add(this.centerY);
           this.Controls.Add(this.centerX);
           this.Controls.Add(this.currentX);
           this.Controls.Add(this.currentY);
           this.Controls.Add(this.currentWidth);
           this.Controls.Add(this.currentHeight);
           this.Controls.Add(this.YPos);
           this.Controls.Add(this.height);
           this.Controls.Add(this.width);
           this.Controls.Add(this.XPos);
           this.Controls.Add(this.New);
           this.Controls.Add(this.Current);
           this.Controls.Add(this.Cancel);
           this.Controls.Add(this.newWidth);
           this.Controls.Add(this.newHeight);
           this.Controls.Add(this.newYPos);
           this.Controls.Add(this.OK);
           this.Controls.Add(this.newXPos);
           this.MaximizeBox = false;
           this.MinimizeBox = false;
           this.Name = "SPEditor";
           this.ShowInTaskbar = false;
           this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
           this.Text = "Adjust Size and Position";
           this.Load += new System.EventHandler(this.SPEditor_Load);
           this.ResumeLayout(false);
           this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox newXPos;
        private System.Windows.Forms.Button OK;
        private System.Windows.Forms.TextBox newYPos;
        private System.Windows.Forms.TextBox newHeight;
        private System.Windows.Forms.TextBox newWidth;
        private System.Windows.Forms.Button Cancel;
        private System.Windows.Forms.Label Current;
        private System.Windows.Forms.Label New;
        private System.Windows.Forms.Label XPos;
        private System.Windows.Forms.Label width;
        private System.Windows.Forms.Label height;
        private System.Windows.Forms.Label YPos;
        private System.Windows.Forms.Label currentHeight;
        private System.Windows.Forms.Label currentWidth;
        private System.Windows.Forms.Label currentY;
        private System.Windows.Forms.Label currentX;
        private System.Windows.Forms.Label centerX;
        private System.Windows.Forms.Label centerY;
        private System.Windows.Forms.Label currentCenterX;
        private System.Windows.Forms.Label currentCenterY;
        private System.Windows.Forms.TextBox newCenterX;
        private System.Windows.Forms.TextBox newCenterY;
    }
}