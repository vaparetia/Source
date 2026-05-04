using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace TextureViewer
{
   public partial class FlatListMakerDialog : Form
   {
      public FlatListMakerDialog()
      {
         InitializeComponent();
      }

      public TextureViewer mainForm;

      private void FlatListMakerDialog_Load(object sender, EventArgs e)
      {

      }

      private void btnConversionIsGo_Click(object sender, EventArgs e)
      {
         bool bMGS3 = radioMGS_Choice_01.Checked;
         bool bCopy = radio_copyFiles.Checked;
         bool bClearFlatlist = chkBox_CleanFlatlist.Checked;
         bool bClearCTXR = chkBox_RemoveCtxr.Checked;
         bool bMakeXml = chkBox_GenerateXML.Checked;
         bool bOnlyMakeList = radio_generateOnly.Checked;

         mainForm.CommenceFlatList( bMGS3, bCopy, bClearFlatlist, bClearCTXR, bMakeXml, bOnlyMakeList);
         Hide();

      }

      private void chkBox_CleanFlatlist_CheckedChanged(object sender, EventArgs e)
      {
         if( chkBox_CleanFlatlist.Checked == false )
         {
            chkBox_RemoveCtxr.Checked = false;
            chkBox_RemoveCtxr.Enabled = false;
         }
         else
         {
            chkBox_RemoveCtxr.Enabled = true;
         }
      }

      private void radioMGS_Choice_02_CheckedChanged(object sender, EventArgs e)
      {

      }
   }
}
