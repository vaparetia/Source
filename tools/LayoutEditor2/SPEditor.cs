using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Xml;
using LayoutExtract2;
using LayoutEditorMGS2;

namespace LayoutEditorMGS2
{
    public partial class SPEditor : Form
    {
       private LayoutEditorWindow mEditor;
       private LayoutObject mLayoutObject;

        public SPEditor()
        {
            InitializeComponent();
        }

        public SPEditor(LayoutEditorWindow editor)
        {
            InitializeComponent();

            mEditor = editor;
        }

        public void Initialize(LayoutObject layoutObj)
        {
           mLayoutObject = layoutObj;

           RectangleF worldRect = layoutObj.GetWorldBounds();
           worldRect = mEditor.ConvertToPreviewCoords(worldRect);
           newWidth.Text = currentWidth.Text    = worldRect.Width.ToString();
           newHeight.Text = currentHeight.Text   = worldRect.Height.ToString();
           newXPos.Text = currentX.Text        = worldRect.X.ToString();
           newYPos.Text = currentY.Text        = worldRect.Y.ToString();

           if (mLayoutObject.mSpriteType == SpriteType.Spin)
           {
              newCenterX.Enabled = true;
              newCenterY.Enabled = true;
              PointF center = mLayoutObject.GetCenter();
              center = mEditor.ConvertToPreviewCoords(center);
              newCenterX.Text = currentCenterX.Text = center.X.ToString();
              newCenterY.Text = currentCenterY.Text = center.Y.ToString();
           }
           else
           {
              newCenterX.Enabled = false;
              newCenterY.Enabled = false;
           }
        }

        private void SPEditor_Load(object sender, EventArgs e)
        {

        }

        private void button2_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void button1_Click(object sender, EventArgs e)
        {
           mEditor.PreserveForUndo();
           RectangleF oldWorldRect = mLayoutObject.GetWorldBounds();
           Single newX = oldWorldRect.X;
           Single newY = oldWorldRect.Y;
           Single newW = oldWorldRect.Width;
           Single newH = oldWorldRect.Height;
           bool parsedOK = false;

           if (Single.TryParse(newXPos.Text, out newX)
              && Single.TryParse(newYPos.Text, out newY)
              && Single.TryParse(newWidth.Text, out newW)
              && Single.TryParse(newHeight.Text, out newH))
           {
              parsedOK = true;
              RectangleF newRect = new RectangleF(newX, newY, newW, newH);
              newRect = mEditor.ConvertFromPreviewCoords(newRect);
              mLayoutObject.OffsetBounds((newRect.X - oldWorldRect.X), (newRect.Y - oldWorldRect.Y), (newRect.Width - oldWorldRect.Width), (newRect.Height - oldWorldRect.Height));

              if (mLayoutObject.mSpriteType == SpriteType.Spin)
              {
                 Single newCX = 0.0f;
                 Single newCY = 0.0f;
                 if (Single.TryParse(newCenterX.Text, out newCX) && Single.TryParse(newCenterY.Text, out newCY))
                 {
                    PointF center = new PointF(newCX, newCY);
                    center = mEditor.ConvertFromPreviewCoords(center);
                    mLayoutObject.OffsetCenter(center);
                 }
                 else
                 {
                    parsedOK = false;
                 }
              }
           }

           if (parsedOK)
           {
              mEditor.RefreshWindow();
              this.Close();
           }
        }

        private void label1_Click(object sender, EventArgs e)
        {

        }

        private void label1_Click_1(object sender, EventArgs e)
        {

        }

        private void label1_Click_2(object sender, EventArgs e)
        {

        }

        private void label4_Click(object sender, EventArgs e)
        {

        }

        private void label1_Click_3(object sender, EventArgs e)
        {

        }

        private void label2_Click(object sender, EventArgs e)
        {

        }
    }
}
