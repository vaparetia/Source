using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using LayoutExtract2;
using LayoutEditorMGS2;

namespace LayoutEditorMGS2
{
   public partial class StatusEditor : Form
   {
      LayoutEditorWindow mEditor;
      LayoutStatusState mLayoutStatusState;
      List<TextBox> mVertXBoxes;
      List<TextBox> mVertYBoxes;

      public StatusEditor(LayoutEditorWindow inEditor, LayoutStatusState inLayoutStatusState)
      {
         mEditor = inEditor;
         mLayoutStatusState = inLayoutStatusState;

         InitializeComponent();

         // Set up text boxes
         mVertXBoxes = new List<TextBox>() { vert0X, vert1X, vert2X, vert3X, sizeW, centerX };
         mVertYBoxes = new List<TextBox>() { vert0Y, vert1Y, vert2Y, vert3Y, sizeH, centerY };

         // disable uv boxes
         uvU.Enabled = false;
         uvV.Enabled = false;

         // Disable all the boxes
         foreach (TextBox b in mVertXBoxes)
            b.Enabled = false;
         foreach (TextBox b in mVertYBoxes)
            b.Enabled = false;

         statusId.Text = "State";
         foreach (LayoutStatusCmd cmd in mLayoutStatusState.mCommands)
         {
            switch (cmd.GetCommand())
            {
            case LayoutStatusCmd.EStatCmd.VERTEX:
               for (int ii = 0; ii < cmd.vertexes.Length; ++ii)
               {
                  LayoutStatusCmd.Vertex vert = cmd.vertexes[ii];
                  mVertXBoxes[ii].Enabled = true;
                  mVertXBoxes[ii].Text = vert.x.ToString();
                  mVertYBoxes[ii].Enabled = true;
                  mVertYBoxes[ii].Text = vert.y.ToString();
               }
               break;
            case LayoutStatusCmd.EStatCmd.SIZE:
               {
                  sizeW.Enabled = true;
                  sizeW.Text = cmd.size_w.ToString();
                  sizeH.Enabled = true;
                  sizeH.Text = cmd.size_h.ToString();
               }
               break;
            case LayoutStatusCmd.EStatCmd.CENTER:
               {
                  centerX.Enabled = true;
                  centerX.Text = cmd.center_x.ToString();
                  centerY.Enabled = true;
                  centerY.Text = cmd.center_y.ToString();
               }
               break;
            case LayoutStatusCmd.EStatCmd.PARTUV:
               {
                  uvU.Enabled = true;
                  uvU.Text = cmd.pos_u.ToString();
                  uvV.Enabled = true;
                  uvV.Text = cmd.pos_v.ToString();

               }
               break;
            }
         }
      }

      private void applyButton_Click(object sender, EventArgs e)
      {
         mEditor.PreserveForUndo();

         foreach (LayoutStatusCmd cmd in mLayoutStatusState.mCommands)
         {
            switch (cmd.GetCommand())
            {
            case LayoutStatusCmd.EStatCmd.VERTEX:
               for (int ii = 0; ii < cmd.vertexes.Length; ++ii)
               {
                  LayoutStatusCmd.Vertex vert = cmd.vertexes[ii];
                  Single vX = vert.x;
                  Single vY = vert.y;
                  Single.TryParse(mVertXBoxes[ii].Text, out vX);
                  Single.TryParse(mVertYBoxes[ii].Text, out vY);
                  vert.x = vX;
                  vert.y = vY;
               }
               break;
            case LayoutStatusCmd.EStatCmd.SIZE:
               {
                  Single sW = cmd.size_w;
                  Single sH = cmd.size_h;
                  Single.TryParse(sizeW.Text, out sW);
                  Single.TryParse(sizeH.Text, out sH);
                  cmd.size_w = sW;
                  cmd.size_h = sH;
               }
               break;
            case LayoutStatusCmd.EStatCmd.CENTER:
               {
                  Single cX = cmd.center_x;
                  Single cY = cmd.center_y;
                  Single.TryParse(centerX.Text, out cX);
                  Single.TryParse(centerX.Text, out cY);
                  cmd.center_x = cX;
                  cmd.center_y = cY;
               }
               break;

            case LayoutStatusCmd.EStatCmd.PARTUV:
               {
                  Single cU = cmd.pos_u;
                  Single cV = cmd.pos_v;
                  Single.TryParse(uvU.Text, out cU);
                  Single.TryParse(uvV.Text, out cV);
                  cmd.pos_u = cU;
                  cmd.pos_v = cV;
               }
               break;

            }
         }
         mEditor.RefreshWindow();
      }

      private void cancelButton_Click(object sender, EventArgs e)
      {
         Close();
      }
   }
}
