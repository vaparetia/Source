using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Xml;
using System.Drawing;
using Tools.Property.Misc;

namespace Tools.Property.UI
{
   public class PropertyHelpSystem
   {
      PropertyPanel mPropertyPanel;
      Timer mTimer;
      Help.HelpPanel mHelpPanel;
      bool mIgnoreNextTimerTick = false;

      public PropertyHelpSystem(PropertyPanel panel)
      {
         mPropertyPanel = panel;

         mTimer = new Timer();
         mTimer.Interval = 150;
         mTimer.Tick += OnTimerElapsed;

         panel.MouseMove += OnMouseMove;
         panel.MouseEnter += OnMouseEnter;
         panel.MouseLeave += OnMouseLeave;
         panel.Move += OnMove;
      }

      void OnMove(object sender, EventArgs e)
      {
         UpdateHelpForMousePosition();
      }

      void OnMouseEnter(object sender, EventArgs e)
      {
         mTimer.Start();
         mIgnoreNextTimerTick = false;
      }

      void OnMouseLeave(object sender, EventArgs e)
      {
         mTimer.Stop();
         CloseHelpPanel();
      }

      void OnMouseMove(object sender, System.Windows.Forms.MouseEventArgs e)
      {
         // Did this rather than restarting the timer, because in Windows Vista, 
         // stopping and starting a timer causes errant MouseMove messages to be
         // sent
         mIgnoreNextTimerTick = true;

         UpdateHelpForMousePosition();
      }

      private void UpdateHelpForMousePosition()
      {
         if (IsHelpVisible())
         {
            ControlInfo control;
            if (mPropertyPanel.GetControlInfoForScreenPosition(Cursor.Position, out control))
            {
               ShowHelpForControl(control);
            }
            else
            {
               DimExistingHelp();
            }
         }
      }

      void OnTimerElapsed(object sender, EventArgs e)
      {
         if ( mIgnoreNextTimerTick )
         {
            mIgnoreNextTimerTick = false;
         }
         else
         {
            if ( !IsHelpVisible() )
            {
               ControlInfo control;
               if ( mPropertyPanel.GetControlInfoForScreenPosition( Cursor.Position, out control ) )
               {
                  ShowHelpForControl( control );
               }
               else
               {
                  DimExistingHelp();
               }
            }
         }
      }

      private bool IsHelpVisible()
      {
         return mHelpPanel != null;
      }

      private void ShowHelpForControl(ControlInfo control)
      {
         string title;
         string toolTip;

         GetToolTipForControl(control, out title, out toolTip);

         if( string.IsNullOrEmpty(title) )
         {
            return;
         }

         if (string.IsNullOrEmpty(toolTip))
         {
            return;
         }

         if( mHelpPanel == null )
         {
            mHelpPanel = new Help.HelpPanel();
            mHelpPanel.Owner = mPropertyPanel.FindForm();
         }

         // Force opacity to not be fully opaque, otherwise you notice flashing when switching between layered window mode and nonlayered.
         mHelpPanel.Opacity = 254.0 / 255.0;

         UpdateHelpPanelLocation(control);

         mHelpPanel.Title = title;
         mHelpPanel.HelpText = toolTip;

         mHelpPanel.Show();
      }

      private void DimExistingHelp()
      {
         if( mHelpPanel != null )
         {
            mHelpPanel.Opacity = 0.75;
         }
      }

      private static void GetToolTipForControl(ControlInfo control, out string title, out string toolTip)
      {
         Data.DataNode propertyElement = control.Control.DataElement;

         if (propertyElement != null)
         {
            Data.ComponentGroup componentGroup = propertyElement as Data.ComponentGroup;
            if ( componentGroup != null )
            {
               propertyElement = componentGroup.ActiveComponent;
            }

            title = propertyElement.DisplayName;
            toolTip = propertyElement.TooltipText;
         }
         else
         {
            title = string.Empty;
            toolTip = string.Empty;
         }
      }

      private void UpdateHelpPanelLocation(ControlInfo control)
      {
         if (mHelpPanel != null)
         {
            Rectangle controlClientRect = control.Control.GetClientRect();
            Rectangle controlScreenRect = mPropertyPanel.RectangleToScreen(controlClientRect);

            // Initial location is to the right of the property.
            Rectangle panelBounds = new Rectangle(controlScreenRect.Right, controlScreenRect.Top, mHelpPanel.Width, mHelpPanel.Height);

            // Check if this fits on the screen
            Rectangle screenArea = Screen.GetWorkingArea(controlScreenRect.Location);
            if (panelBounds.Right > screenArea.Right)
            {
               // If it doesn't, try it on the left side of the property area.
               panelBounds.X = controlScreenRect.Left - panelBounds.Width;
            }

            // Do any other adjustment necessary to keep it on the screen (so it doesn't go outside on the top or bottom).
            panelBounds = Tools.Common.WindowPosition.OffsetBoundsToBestScreen(panelBounds);

            mHelpPanel.Bounds = panelBounds;
         }
      }

      private void CloseHelpPanel()
      {
         if (mHelpPanel != null)
         {
            mHelpPanel.Dispose();
            mHelpPanel = null;
         }
      }
   }
}
