using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Xml.XPath;
using System.Xml;
using Tools.Property.Build;
using Tools.Property.Misc;
using Tools.Common;

namespace Tools.Property.UI
{
   public partial class ModalPropertyDlg : Form
   {
      private List<XmlElement> mObjects = new List<XmlElement>();
      private List<Data.PropertyContainerDocument> mNewObjects = new List<Data.PropertyContainerDocument>();
      private Data.PropertyContainerDocument mGroupedObjects;
      private PropertyEditorGUI mGUI;

      public ModalPropertyDlg( List<XmlElement> objects, Tools.Property.Source.IPropertyDataSource dataSource )
      {
         InitializeComponent();

         foreach ( XmlElement obj in objects )
         {
            mObjects.Add( obj );
            mNewObjects.Add( new Data.PropertyContainerDocument( XmlNodeHelpers.CreateDocumentFromNode( obj ) ) );
         }

         mGroupedObjects = new Data.PropertyContainerDocument( GroupProperties.BuildGroupedProperties( mObjects, GroupProperties.EGroupOptions.kIgnoreTopName_IgnoreTopId ) );

         mGUI = new PropertyEditorGUI(mMainPanel);
         mGUI.mPropertyDB = dataSource;
         mGUI.mCallbacks.mChangeCurrentComponentInGroup = new PropertyEditorGUI.PropertyEditorCallbacks.ChangeCurrentComponentInGroupDelegate(OnComponentGroupComponentChanged);
         mGUI.Initialize( mGroupedObjects.Document );
      }


      void OnComponentGroupComponentChanged(Data.ComponentGroup componentGroup, String newComponentId)
      {
         List<Data.PropertyContainerDocument> documentsToChange = new List<Data.PropertyContainerDocument>();
         documentsToChange.Add(mGroupedObjects);
         documentsToChange.AddRange(mNewObjects);

         Data.PropertyContainerDocument.ChangeActiveComponentGroupInPropertyContainers(
            documentsToChange,
            componentGroup, newComponentId, true, mGUI.mPropertyDB );

         mGUI.RebuildAllControls();
      }

      private string ComponentNameFromComponentGuid( Guid id )
      {
         return id.ToString();
      }

      private void OnOk(object sender, EventArgs e)
      {
         // apply changes to all objects
         List<XmlElement> newObjectElements = new List<XmlElement>();
         foreach ( Data.PropertyContainerDocument doc in mNewObjects )
         {
            newObjectElements.Add( doc.PropertyContainer.Element );
         }

         GroupProperties.UngroupProperties(mGroupedObjects.Document, newObjectElements, GroupProperties.EGroupOptions.kIgnoreTopName_IgnoreTopId);

         for ( int i = 0; i < mObjects.Count; ++i )
         {
            XmlElement destElement = mObjects[i];
            XmlDocument destDoc = destElement.OwnerDocument;
            XmlElement sourceElement = newObjectElements[i];

            destElement.RemoveAll();
            foreach ( XmlAttribute attr in sourceElement.Attributes )
            {
               destElement.Attributes.Append( (XmlAttribute) destDoc.ImportNode( attr, false ) );
            }

            foreach ( XmlNode node in sourceElement.ChildNodes )
            {
               destElement.AppendChild( destDoc.ImportNode( node, true ) );
            }
         }
      }

      private void OnKeyDown( object sender, KeyEventArgs e )
      {
         if( e.KeyCode == Keys.F1 )
         {
            e.Handled = true;
            mGUI.ShowContextSensitiveHelp();
         }
      }
   }
}