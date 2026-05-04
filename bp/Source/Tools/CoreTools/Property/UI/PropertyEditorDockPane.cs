using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using WeifenLuo.WinFormsUI.Docking;
using System.Xml;
using Tools.Property.UI;
using Tools.Property.Build;

namespace CoreTools.Property.UI
{
   public partial class PropertyEditorDockPane : DockContent
   {
      PropertyEditor mPropertyEditor;
      PropertyEditorGUI mPropertyEditorGUI;

      public PropertyEditorGUI PropertyEditorGUI
      {
         get { return mPropertyEditorGUI; }
         set { mPropertyEditorGUI = value; }
      }

      public PropertyEditorDockPane(PropertyEditor propertyEditor)
      {
         InitializeComponent();

         mPropertyEditor = propertyEditor;

         mPropertyEditorGUI = new PropertyEditorGUI(this);
         mPropertyEditorGUI.mCallbacks.mPropertyValueChanged += propertyEditor.OnValueChanged;
         mPropertyEditorGUI.mCallbacks.mRetrieveXmlDelegate += propertyEditor.RetrieveObjectFromComponentId;
         mPropertyEditorGUI.mCallbacks.mGetCustomEditors += Tools.Property.UI.CustomEditor.VTexEditor.GetCustomEditors;
         mPropertyEditorGUI.mCallbacks.mGetCustomEditors += Tools.Property.UI.CustomEditor.CopyPasteEditor.GetCustomEditors;
         mPropertyEditorGUI.mCallbacks.mComponentChanged += propertyEditor.OnComponentChanged;
         mPropertyEditorGUI.mCallbacks.mEvaluatorChanged += propertyEditor.OnEvaluatorChanged;
         mPropertyEditorGUI.mCallbacks.mBuildCustomPropertyUI += CoreTools.Property.UI.PropertySpline.BuildCustomPropertyUI;
         mPropertyEditorGUI.mCallbacks.mPostCreateComponentGroup = Tools.GameEngine.RGCHelpers.PostCreateComponentGroupCheckForBaseComponent;

         mPropertyEditorGUI.mPropertyDB = propertyEditor.ScriptTypeDB;

         Tools.Property.UI.PropertyEditorSupport.ExtendPropertyEditor(mPropertyEditorGUI, "Game", delegate(String componentGroupId){ return componentGroupId != "RequiredGameComponent"; });
      }

      public void Initialize(List<XmlElement> objects)
      {
         if (objects.Count > 0)
         {
            XmlDocument doc = GroupProperties.BuildGroupedProperties(objects, GroupProperties.EGroupOptions.kIgnoreTopName_IgnoreTopId);
            mPropertyEditorGUI.Initialize(doc);
         }
         else
         {
            mPropertyEditorGUI.Initialize(null);
         }
      }

      void OnKeyDown(object sender, KeyEventArgs e)
      {
         if (e.KeyCode == Keys.F1)
         {
            e.Handled = true;
            mPropertyEditorGUI.ShowContextSensitiveHelp();
         }

         if( !e.Handled )
         {
            mPropertyEditorGUI.mCallbacks.KeyDown(mPropertyEditorGUI, e);
         }
      }
   }
}