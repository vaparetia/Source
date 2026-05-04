using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Drawing;
using System.Xml;
using System.Xml.XPath;
using Tools.GameEngine;
using Tools.Property.Misc;

namespace Tools.Property.UI
{
   /// <summary>
   /// Helper functions for the Property Editor GUI go in here to keep stuff nicely separated
   /// </summary>
   public class GUIHelpers
   {
      public delegate XmlDocument RetrieveObjectFromComponentIdDelegate(Guid componentGuid, bool readOnly);
      public static Font skBoldDialogFont = new Font(System.Drawing.SystemFonts.DialogFont, FontStyle.Bold);
      public static Font skBoldMenuFont = new Font(System.Drawing.SystemFonts.MenuFont, FontStyle.Bold);

      public static bool ResolveComponentIdToName(Guid id, RetrieveObjectFromComponentIdDelegate retrieveXmlDelegate, out string outComponentName)
      {
         try
         {
            if (retrieveXmlDelegate != null)
            {
               XmlDocument doc = retrieveXmlDelegate(id, true);
               if (doc != null)
               {
                  XmlElement scriptObjectNav = (XmlElement) doc.SelectSingleNode("//PropertyContainer");
                  XmlElement componentGroupNav = (XmlElement) doc.SelectSingleNode( string.Format( "//ComponentGroup[@editorId='{0}']", id ) );

                  if (scriptObjectNav != null && componentGroupNav != null)
                  {
                     string objName = Helpers.GetShortDisplayName(scriptObjectNav);

                     string componentGroupId = componentGroupNav.GetAttribute(Helpers.kComponentGroup_Id, String.Empty);
                     string componentName = Helpers.GetActiveComponentNameFromComponentGroup(componentGroupNav);
                     string componentGroupName = Helpers.GetDisplayName(componentGroupNav);

                     string fullComponentName = String.Format("{0} ({1})", objName, RGCHelpers.GetComponentDisplayString(componentGroupId, componentName, componentGroupName));

                     outComponentName = fullComponentName;
                     return true;
                  }
               }
            }
         }
         catch (System.Exception e)
         {
            System.Diagnostics.Debug.Write(e.Message);
         }

         outComponentName = string.Format("Unknown ({0})", id.ToString());
         return false;
      }

      public static XPathNavigator GetComponentByIdFromPropertyContainer(XPathNavigator scriptObjectNav, Guid componentId)
      {
         return scriptObjectNav.SelectSingleNode(string.Format("//ComponentGroup[@editorId='{0}']/Component", componentId));
      }

      public static Control GetLeafControlForPosition(Control topLevelControl, ref Point screenPos)
      {
         Point clientPos = topLevelControl.PointToClient(screenPos);
         Control control = topLevelControl.GetChildAtPoint(clientPos, GetChildAtPointSkip.Invisible);
         while (control != null)
         {
            Point nextClientPos = control.PointToClient(screenPos);
            Control nextControl = control.GetChildAtPoint(nextClientPos, GetChildAtPointSkip.Invisible);
            if (nextControl == null)
            {
               break;
            }

            control = nextControl;
         }
         return control;
      }
   }
}
