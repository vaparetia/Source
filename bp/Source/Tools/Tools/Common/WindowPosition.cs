using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Windows.Forms;
using System.Drawing;
using System.IO;
using WeifenLuo.WinFormsUI.Docking;

namespace Tools.Common
{
   public class WindowPosition
   {
      public static void SaveDockingPosition(String windowPositionName, DockPanel dockPanel)
      {
         if (EnsureWPPathExists())
         {
            dockPanel.SaveAsXml(MakeWPFileName(windowPositionName + "_DockInfo"));
         }
      }

      public static void LoadDockingPosition(String windowPositionName, DockPanel dockPanel, DeserializeDockContent dscDelegate )
      {
         if (EnsureWPPathExists())
         {
            string fileName = MakeWPFileName(windowPositionName + "_DockInfo");
            if (File.Exists(fileName))
            {
               dockPanel.LoadFromXml(fileName, dscDelegate);
            }
         }
      }

      public static void SaveWindowPosition(String windowPositionName, String windowName, Form window)
      {
         Dictionary< String, Form > windowDescs = new Dictionary<string,Form>();

         windowDescs[ windowName ] = window;

         SaveWindowPositions( windowPositionName, windowDescs );
      }

      public static bool SaveWindowPositions( String windowPositionName, Dictionary< String, Form > windowDescs )
      {
         if ( EnsureWPPathExists() )
         {
            XmlDocument doc = new XmlDocument();
            XmlElement root = doc.CreateElement( "WindowPositions" );
            root.SetAttribute( "version", "2" );
            doc.AppendChild( root );

            foreach ( KeyValuePair<String, Form> entry in windowDescs )
            {
               root.AppendChild( CreateXMLForWindowPosition( doc, entry.Key, entry.Value ) );
            }

            try
            {
               doc.Save( MakeWPFileName( windowPositionName ) );
               return true;
            }
            catch (System.Exception)
            {
            }
         }

         return false;
      }

      public static bool LoadWindowPosition( String windowPositionName, String windowName, Form window )
      {
         Dictionary<String, Form> windowDescs = new Dictionary<string, Form>();

         windowDescs[windowName] = window;

         return LoadWindowPositions( windowPositionName, windowDescs );
      }

      public static bool LoadWindowPositions( String windowPositionName, Dictionary<String, Form> windowDescs )
      {
         try
         {
            XmlDocument doc = new XmlDocument();

            doc.Load( MakeWPFileName( windowPositionName ) );

            XmlElement rootElement = (XmlElement) doc.SelectSingleNode( "WindowPositions" );

            // Make sure that we have a window positions file we understand
            if ( rootElement != null && Int32.Parse( rootElement.GetAttribute( "version" ) ) == 2 )
            {
               foreach ( KeyValuePair<String, Form> entry in windowDescs )
               {
                  XmlElement el = (XmlElement) doc.SelectSingleNode( String.Format(
                     "WindowPositions/WindowPosition[@name={0}]", XmlNodeHelpers.AsQuotedXml( entry.Key ) ) );

                  if ( el != null )
                  {
                     PositionWindowFromXML( el, entry.Value );
                  }
               }

               return true;
            }
            else
            {
               return false;
            }
         }
         catch (System.Exception e )
         {
            Console.WriteLine(e.Message);
            
            return false;
         }
      }

      private static string GetWPPath()
      {
         return Tools.Common.FileUtils.GetBluepointAppdataFolder() + "\\WindowPositions";
      }

      private static bool EnsureWPPathExists()
      {
         String wpPath = GetWPPath();
         if ( Directory.Exists( wpPath ) )
         {
            return true;
         }
         else
         {
            try
            {
               Directory.CreateDirectory( wpPath );
               return true;
            }
            catch ( System.Exception )
            {
               return false;
            }
         }
      }

      private static String MakeWPFileName( String wpName )
      {
         return GetWPPath() + "\\" + wpName + ".xml";
      }

      private static XmlElement CreateXMLForWindowPosition( XmlDocument doc, String name, Form form )
      {
         Rectangle restoreBounds = ( form.WindowState == FormWindowState.Normal ) ? form.Bounds : form.RestoreBounds;
         FormWindowState windowState = form.WindowState;

         XmlElement el = doc.CreateElement( "WindowPosition" );
         el.SetAttribute( "name", name );

         XmlElement rectElement = doc.CreateElement( "Rectangle" );
         rectElement.SetAttribute( "x", restoreBounds.X.ToString() );
         rectElement.SetAttribute( "y", restoreBounds.Y.ToString() );
         rectElement.SetAttribute( "width", restoreBounds.Width.ToString() );
         rectElement.SetAttribute( "height", restoreBounds.Height.ToString() );

         XmlElement stateElement = doc.CreateElement( "WindowState" );
         stateElement.InnerText = windowState.ToString();

         DockContent dc = form as DockContent;

         el.AppendChild( rectElement );
         el.AppendChild( stateElement );

         if (dc != null)
         {
            XmlElement dockStateElement = doc.CreateElement( "DockState" );
            dockStateElement.InnerText = dc.DockState.ToString();

            el.AppendChild(dockStateElement);
         }

         return el;
      }

      private static void PositionWindowFromXML( XmlElement windowPosition, Form form )
      {
         try
         {
            DockContent dc = form as DockContent;

            Rectangle restoreBounds = ( form.WindowState == FormWindowState.Normal ) ? form.Bounds : form.RestoreBounds;
            FormWindowState windowState = form.WindowState;
            DockState? dockState = null;

            if ( dc != null )
            {
               dockState = dc.DockState;
            }

            bool loadedAnything = false;
            XmlElement rectElement = (XmlElement) windowPosition.SelectSingleNode( "Rectangle" );

            if ( rectElement != null )
            {
               restoreBounds = new Rectangle(
                  Int32.Parse( rectElement.GetAttribute( "x" ) ),
                  Int32.Parse( rectElement.GetAttribute( "y" ) ),
                  Int32.Parse( rectElement.GetAttribute( "width" ) ),
                  Int32.Parse( rectElement.GetAttribute( "height" ) ) );

               loadedAnything = true;
            }

            XmlElement wsElement = (XmlElement) windowPosition.SelectSingleNode( "WindowState" );
            if ( wsElement != null )
            {
               FormWindowState newState = (FormWindowState) System.Enum.Parse( typeof( FormWindowState ), wsElement.InnerText );

               if ( newState != FormWindowState.Minimized )
               {
                  windowState = newState;
                  loadedAnything = true;
               }
            }

            XmlElement dsElement = (XmlElement)windowPosition.SelectSingleNode("DockState");
            if (dsElement != null && dc != null)
            {
               dockState = (DockState)System.Enum.Parse(typeof(DockState), dsElement.InnerText);
               loadedAnything = true;
            }

            if ( loadedAnything )
            {
               // Make sure the window fits on a monitor
               restoreBounds = OffsetBoundsToBestScreen(restoreBounds);

               // Set all of the bounds
               form.StartPosition = FormStartPosition.Manual;
               form.WindowState = windowState;
               form.SetBounds( restoreBounds.X, restoreBounds.Y, restoreBounds.Width, restoreBounds.Height );

               if (dc != null && dockState.HasValue )
               {
                  if (dc.IsHandleCreated)
                  {
                     dc.DockState = dockState.Value;
                  }

                  dc.ShowHint = dockState.Value;
               }
            }
         }
         catch (System.Xml.XmlException)
         {
         	// Something happened reading this in, just ignore it
         }
      }

      public static Rectangle OffsetBoundsToBestScreen(Rectangle bounds)
      {
         Rectangle workingArea = Screen.GetWorkingArea(bounds);

         if( bounds.Right >= workingArea.Right )
         {
            bounds.X = workingArea.Right - bounds.Width;
         }

         if (bounds.Bottom >= workingArea.Bottom)
         {
            bounds.Y = workingArea.Bottom - bounds.Height;
         }

         bounds.X = Math.Max(bounds.X, workingArea.X);
         bounds.Y = Math.Max(bounds.Y, workingArea.Y);

         return bounds;
      }
   }
}
