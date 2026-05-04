using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace Tools.Controls
{
   public partial class InfoBar : Panel
   {
      public InfoBar()
      {
         InitializeComponent();

         DoubleBuffered = true;
         mOriginalClientSize = ClientSize;
         ClientSizeChanged += RecheckVerticalSize;
         TextChanged += RecheckVerticalSize;
         Paint += PaintText;
      }

      public static InfoBar ShowNew(Control parent, String text)
      {
         InfoBar bar = new InfoBar();
         bar.Text = text;
         parent.Controls.Add(bar);
         bar.BringToFront();

         return bar;
      }

      void PaintText(object sender, PaintEventArgs args)
      {
         Rectangle layoutRect = new Rectangle( new Point( skMargins, 0 ), new Size( mCurrentWidth, ClientSize.Height ) );
         StringFormat format = new StringFormat();
         format.LineAlignment = StringAlignment.Center;

         args.Graphics.DrawString(Text, Font, SystemBrushes.InfoText, layoutRect, format);
      }

      private void RecheckVerticalSize(object sender, EventArgs e)
      {
         int newWidth = mClose.Left - skMargins * 2;

         if (newWidth == mCurrentWidth || mIgnoreVerticalSizeRefcount != 0)
         {
            return;
         }

         using (Graphics g = CreateGraphics())
         {
            SizeF stringSize = g.MeasureString(Text, this.Font, newWidth);

            mCurrentWidth = newWidth;

            int newHeight = (int) ( stringSize.Height + 0.5f ) + skMargins * 2;
            if ( newHeight < mOriginalClientSize.Height )
            {
               newHeight = mOriginalClientSize.Height;
            }

            if (newHeight != ClientSize.Height)
            {
               ++mIgnoreVerticalSizeRefcount;
               try
               {
                  ClientSize = new Size(ClientSize.Width, newHeight);
                  Invalidate();
               }
               finally
               {
                  --mIgnoreVerticalSizeRefcount;
               }
            }
         }
      }

      private void mClose_Click(object sender, EventArgs e)
      {
         Dispose();
      }

      Size mOriginalClientSize;
      int mCurrentWidth = -1;
      int mIgnoreVerticalSizeRefcount = 0;
      const int skMargins = 3;


   }
}
