using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using WeifenLuo.WinFormsUI.Docking;
using Tools.Property.UI;
using System.Xml;

namespace CoreTools.Property.UI
{
   public partial class ConnectionEditorDockPane : DockContent
   {
      bool mIsInitialized;
      PropertyEditor mPropertyEditor;
      ConnectionEditorGUI mConnectionEditorGUI;

      public ConnectionEditorGUI ConnectionEditorGUI
      {
         get { return mConnectionEditorGUI; }
         set { mConnectionEditorGUI = value; }
      }

      public ConnectionEditorDockPane(PropertyEditor propertyEditor)
      {
         InitializeComponent();
      
         mPropertyEditor = propertyEditor;

         mConnectionEditorGUI = new ConnectionEditorGUI();
         mConnectionEditorGUI.Dock = DockStyle.Fill;

         Controls.Add(mConnectionEditorGUI);
      }

      public void Initialize(List<XmlElement> objects, PropertyEditorGUI.PropertyEditorCallbacks callbacks)
      {
         mConnectionEditorGUI.mRetrieveObjectIdsConnectedToObjectIdsDelegate = mPropertyEditor.mRetrieveObjectIdsConnectedToObjectIdsDelegate;
         mConnectionEditorGUI.mConnectionsChanged = mPropertyEditor.mConnectionsChangedDelegate;
         mConnectionEditorGUI.mChangeObjectSelection = mPropertyEditor.mChangeObjectSelection;

         mIsInitialized = true;
         mConnectionEditorGUI.Initialize(objects, mPropertyEditor.ScriptTypeDB, callbacks);
      }

      public void Uninitialize()
      {
         if (mIsInitialized)
         {
            mIsInitialized = false;
            mConnectionEditorGUI.Uninitialize();
         }
      }

      void OnKeyDown(object sender, KeyEventArgs e)
      {
         if (e.KeyCode == Keys.F1)
         {
            e.Handled = true;
            mConnectionEditorGUI.ShowContextSensitiveHelp();
         }

         if( !e.Handled )
         {
            mPropertyEditor.PropertyEditorDockPane.PropertyEditorGUI.mCallbacks.KeyDown(null, e);
         }
      }
   }
}