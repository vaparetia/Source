using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;

namespace CoreTools.Prefab.UI
{
   public class TagButton : System.Windows.Forms.Button
   {
      public event EventHandler DeleteEvent;

      public TagButton()
      {
         this.ForeColor = Color.Transparent;
         this.AutoSize = true;
         this.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
      }

      public Rectangle DeleteButtonRectangle
      {
         get
         {
            return new Rectangle(4, 0, 13, Height);
         }
      }

      protected override void OnPaint(System.Windows.Forms.PaintEventArgs pevent)
      {
         if( Enabled )
         {
            base.OnPaint(pevent);
         }
         else
         {
            System.Windows.Forms.ButtonRenderer.DrawParentBackground(pevent.Graphics, ClientRectangle, this);
            System.Windows.Forms.ButtonRenderer.DrawButton(pevent.Graphics, ClientRectangle, System.Windows.Forms.VisualStyles.PushButtonState.Disabled);
         }

         SizeF textSize = pevent.Graphics.MeasureString(Text, Font);
         pevent.Graphics.DrawString(Text, Font, SystemBrushes.WindowText, DeleteButtonRectangle.Width + 8, (Height - textSize.Height) / 2);

         System.Drawing.Imaging.ImageAttributes attr = new System.Drawing.Imaging.ImageAttributes();
         attr.SetColorKey(Color.Magenta, Color.Magenta);

         Image deleteImage = Enabled ? Properties.Resources.TagButtonDelete : Properties.Resources.TagButtonDeleteDisabled;
         int yOffset = (Height - deleteImage.Height) / 2;

         Rectangle destRect = DeleteButtonRectangle;
         destRect.Y = yOffset;
         destRect.Height = deleteImage.Height;

         pevent.Graphics.DrawImage(deleteImage, destRect, 0, 0, deleteImage.Width, deleteImage.Height, GraphicsUnit.Pixel, attr);

         int lineX = DeleteButtonRectangle.Right + 2;
         int linePadding = 4;
         pevent.Graphics.DrawLine(SystemPens.GrayText, lineX, linePadding, lineX, Height - linePadding);
      }

      public override Size GetPreferredSize(Size proposedSize)
      {
         Size size = base.GetPreferredSize(proposedSize);
         size.Width += DeleteButtonRectangle.Right + 4;
         return size;
      }

      protected override void OnMouseUp(System.Windows.Forms.MouseEventArgs mevent)
      {
         Rectangle deleteRectangle = DeleteButtonRectangle;
         if( deleteRectangle.Contains(mevent.Location))
         {
            DeleteEvent(this, new EventArgs());
         }
         else
         {
            base.OnMouseUp(mevent);
         }
      }
   }
}
