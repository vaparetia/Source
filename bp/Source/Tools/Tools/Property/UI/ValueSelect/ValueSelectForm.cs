using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace Tools.Property.UI.ValueSelect
{
   abstract public partial class ValueSelectForm : Form
   {
      private bool mAcceptOnClose = false;

      public delegate void SetValueDelegate(string value, bool setFinalValue);
      public delegate string GetValueDelegate();

      protected SetValueDelegate mSetValue;
      protected GetValueDelegate mGetValue;

      public string  mCancelValue = String.Empty;
      
      public ValueSelectForm(SetValueDelegate setValue, GetValueDelegate getValue)
      {
         mSetValue = setValue;
         mGetValue = getValue;
         InitializeComponent();
         SelectValuePanel.MouseMove += new MouseEventHandler(SelectValuePanel_MouseMove);
      }

      String GetDesiredValueForLocation( Point location )
      {
         float fractionX;
         float fractionY;

         fractionX = location.X / (float) SelectValuePanel.Size.Width;
         fractionX *= 2.0f;
         fractionX -= 1.0f;
         fractionY = location.Y / (float) SelectValuePanel.Size.Height;
         fractionY *= 2.0f;
         fractionY -= 1.0f;

         return GetDesiredValue( fractionX, fractionY );
      }

      void SelectValuePanel_MouseMove(object sender, MouseEventArgs e)
      {
         mSetValue( GetDesiredValueForLocation( e.Location ), false );
         Text = mGetValue();
      }

      protected override void OnClosed( EventArgs e )
      {
         base.OnClosed( e );

         // NASTY HACK: Necessary for undo for Maya transform attributes to work correctly.
         // While we're in "tweaking" mode we have to execute setAttr commands to update the transform with 
         // undo disabled (otherwise each incremental change becomes it's separate undo step).
         // Then when we're applying the final accepted value, we execute a setAttr command with undo enabled,
         // however this will store the position directly before the setAttr call as the undo step (which isn't our true starting position).
         string acceptedValue = mGetValue();

         mSetValue( mCancelValue, false );

         if ( mAcceptOnClose )
         {
            mSetValue( acceptedValue, true );
         }
         else
         {
            mSetValue( mCancelValue, true );
         }
      }

      void ValueSelectForm_MouseClick(object sender, MouseEventArgs e)
      {
         Control control = sender as Control;

         if ( ( e.Button & ( MouseButtons.Left | MouseButtons.Right ) ) != 0 )
         {
            if ( ( e.Button & MouseButtons.Left ) == MouseButtons.Left )
            {
               mAcceptOnClose = true;
            }

            Close();
         }
      }

      abstract public String GetDesiredValue( float fracX, float fracY );
      
      public Point SetupLocation(Point mouseClickPoint)
      {
         Point center = SelectValuePanel.Location;
         center.Offset(new Point(SelectValuePanel.Size.Width / 2, SelectValuePanel.Size.Height / 2));
         
         Point screenClickPosition = mouseClickPoint;
         screenClickPosition.Offset(new Point(-Size.Width, -Size.Height));
         screenClickPosition.Offset(center);
         
         Location = screenClickPosition;
         StartPosition =  FormStartPosition.Manual;
         
         return screenClickPosition;
      }

      private void ValueSelectForm_KeyDown( object sender, KeyEventArgs e )
      {
         if ( e.KeyCode == Keys.Escape )
         {
            Close();
         }
         else if ( e.KeyCode == Keys.Enter )
         {
            mAcceptOnClose = true;
            Close();
         }
      }

      private int GetExtentValue( int max, int position )
      {
         return max * position / 2;
      }

      private void SelectValuePanel_Paint( object sender, PaintEventArgs e )
      {
         Size maxPoint = SelectValuePanel.Size - new System.Drawing.Size( 1, 1 );

         Point[] funnelWinding = 
         {
            new Point( 0, maxPoint.Height ),
            new Point( maxPoint.Width * 3 / 8, 0 ),
            new Point( maxPoint.Width * 5 / 8, 0 ),
            new Point( maxPoint.Width, maxPoint.Height )
         };


         e.Graphics.FillPolygon( SystemBrushes.ControlLight, funnelWinding );

         e.Graphics.DrawLine( SystemPens.ControlLightLight, funnelWinding[0], funnelWinding[1] );
         e.Graphics.DrawLine( SystemPens.ControlLightLight, funnelWinding[2], funnelWinding[3] );

         for ( int xPosition = 0; xPosition < 3; ++xPosition )
         {
            for ( int yPosition = 0; yPosition < 3; ++yPosition )
            {
               Point p = new Point(
                  GetExtentValue( maxPoint.Width, xPosition ),
                  GetExtentValue( maxPoint.Height, yPosition ) );

               StringFormat format = new StringFormat();
               format.Alignment = (StringAlignment) xPosition;
               format.LineAlignment = (StringAlignment) yPosition;
               e.Graphics.DrawString( GetDesiredValueForLocation( p ), SystemFonts.DefaultFont, SystemBrushes.ControlText, SelectValuePanel.ClientRectangle, format );
            }
         }

      }
   }
}