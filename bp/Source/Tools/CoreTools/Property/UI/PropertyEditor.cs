using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Xml.XPath;
using System.Xml;
using System.Diagnostics;

using Tools;
using Tools.Property;
using Tools.Property.Source;
using Tools.Property.UI;
using Tools.Property.Build;
using Tools.Property.Misc;
using Tools.GameEngine;
using Tools.Common;
using Helper;
using WeifenLuo.WinFormsUI.Docking;

namespace CoreTools.Property.UI
{
   public class PropertyEditor
   {
      public delegate double[] GetDoubleArrayFromEditorIdDelegate(Guid editorId);
      public delegate XmlDocument RetrieveXmlDelegate(Guid id, bool isReadOnly);
      public delegate void UpdateShaderXMLFromMayaShadersDelegate();

      public GetDoubleArrayFromEditorIdDelegate mGetTransformFromEditorId;
      public GetDoubleArrayFromEditorIdDelegate mGetScaleFromEditorId;
      public RetrieveXmlDelegate mRetrieveDataUnmerged;
      public RetrieveXmlDelegate mRetrieveDataMerged;
      public ConnectionEditorGUI.RetrieveObjectIdsConnectedToObjectIdsDelegate mRetrieveObjectIdsConnectedToObjectIdsDelegate;
      public ConnectionEditorGUI.ConnectionsChanged mConnectionsChangedDelegate;
      public ConnectionEditorGUI.ChangeObjectSelection mChangeObjectSelection;

      private IPropertyDataSource mScriptTypeDB;
      private bool mLivePropertyUpdateEnabled = false;
      public IPropertyDataSource ScriptTypeDB
      {
         get { return mScriptTypeDB; }
      }

      public bool LivePropertyUpdateEnabled
      {
         get { return mLivePropertyUpdateEnabled; }
         set { mLivePropertyUpdateEnabled = value; }
      }

      Dictionary<String, XmlElement> mObjects = new Dictionary<String, XmlElement>();

      DockPanel mDockPanel;
      PropertyEditorDockPane mPropertyEditorDockPane;
      ConnectionEditorDockPane mConnectionEditorDockPane;

      public PropertyEditorDockPane PropertyEditorDockPane
      {
         get { return mPropertyEditorDockPane; }
      }

      public ConnectionEditorDockPane ConnectionEditorDockPane
      {
         get { return mConnectionEditorDockPane; }
      }

      Tools.Property.Misc.GameDataSenderConnection mPropertySender;

      public PropertyEditor( DockPanel dockPanel, IPropertyDataSource scriptTypeDB )
      {
         mScriptTypeDB = scriptTypeDB;

         mPropertySender = new GameDataSenderConnection(GameDataSenderConnection.mDefaultGameDataPort);
         mPropertySender.mCustomPropertyCookers += new Tools.Property.Cook.CookProperties.CookInfo.CustomPropertyCooker(Spline.ProcessCustomProperty);
         mPropertySender.mComponentGroupPropertiesModifier += new Tools.Property.Cook.CookProperties.CookInfo.ComponentPropertiesModifier(OnModifyComponentProperties);

         mDockPanel = dockPanel;

         mPropertyEditorDockPane = new PropertyEditorDockPane(this);
         mConnectionEditorDockPane = new ConnectionEditorDockPane(this);
      }

      public IDockContent ResolveDockContent(string persistString)
      {
         if (persistString == "CoreTools.Property.UI.PropertyEditorDockPane")
         {
            return PropertyEditorDockPane;
         }
         else if (persistString == "CoreTools.Property.UI.ConnectionEditorDockPane")
         {
            return ConnectionEditorDockPane;
         }

         return null;
      }

      public void Show()
      {
         ShowConnectionEditor();
         ShowPropertyEditor();
      }

      public void ShowPropertyEditor()
      {
         if (mPropertyEditorDockPane.DockState == DockState.Unknown)
         {
            mPropertyEditorDockPane.Show(mConnectionEditorDockPane.FloatPane, mConnectionEditorDockPane);
         }
         else
         {
            mPropertyEditorDockPane.IsHidden = false;
         }
      }

      public void ShowConnectionEditor()
      {
         if (mConnectionEditorDockPane.DockState == DockState.Unknown)
         {
            mConnectionEditorDockPane.Show(mDockPanel, new Rectangle(0, 0, 400, 1000));
         }
         else
         {
            mConnectionEditorDockPane.IsHidden = false;
         }
      }

      public void Hide()
      {
         mConnectionEditorDockPane.Hide();
         mPropertyEditorDockPane.Hide();
      }

      public void RefreshConnectionEditor()
      {
         ApplyPropertiesToObjects();
         mConnectionEditorDockPane.ConnectionEditorGUI.InitializeComponentProperties();
      }

      /// <summary>
      /// Callback function when we're about to send a component group down the wire for
      /// live property update. Applies the world space from Maya to the script object.
      /// </summary>
      /// <param name="componentObject">Navigator to the destination component group</param>
      void OnModifyComponentProperties(XmlElement componentGroup, Tools.AssetSystem.Manager assetManager, Tools.Property.Cook.CookProperties.CookInfo cookInfo)
      {
         XmlElement componentObject = Helpers.GetActiveComponentFromComponentGroup( componentGroup );
         // Since this function only updates the object's transform, punt if we're not the required
         // game component
         if ((componentObject == null) || (!RGCHelpers.IsComponentRequiredGameComponent(componentObject)))
         {
            return;
         }

         // We need to look up the object id by the component's editor id
         Guid componentEditorId = Helpers.GetInternalEditorIdGuid(componentObject );
         XmlDocument scriptObject = RetrieveObjectFromComponentId(componentEditorId, true);
         Guid editorId = Helpers.GetInternalEditorIdGuid(scriptObject.SelectSingleNode(Helpers.kElement_PropertyContainer));
         
         // Ok, we have found an object, let's get the transform node now
         XmlElement transformNode = RGCHelpers.GetTransformForRGC(componentObject);
         XmlElement scaleNode = RGCHelpers.GetScaleForRGC( componentObject );

         // Get the transform from Maya by the object's editor id
         double[] transform = mGetTransformFromEditorId(editorId);
         double[] scale = mGetScaleFromEditorId(editorId);

         // Jam the translations and orientations into the node
         for (int row = 0; row < 4; ++row)
         {
            for (int column = 0; column < 3; ++column)
            {
               double src = transform[row * 3 + column];
               XmlElement dstNav = Helpers.FindDirectChildProperty(transformNode, String.Format("m{0}{1}", row, column));
               Helpers.SetPropertyValueElement(dstNav, src.ToString() );
            }
         }

         Helpers.SetPropertyValueElement(Helpers.FindDirectChildProperty(scaleNode, "X"), scale[0].ToString());
         Helpers.SetPropertyValueElement(Helpers.FindDirectChildProperty(scaleNode, "Y"), scale[1].ToString());
         Helpers.SetPropertyValueElement(Helpers.FindDirectChildProperty(scaleNode, "Z"), scale[2].ToString());
      }

      public void BeginObjects()
      {
         mObjects.Clear();
      }

      public bool AddMergedObject(string objectId,
                                  XmlDocument xmlDoc)
      {
         mObjects.Add(objectId, Tools.Common.XmlNodeHelpers.GetDocumentRootElement( xmlDoc ) );
         return true;
      }

      public void EndObjects(bool showConnections)
      {
         List<XmlElement> objects = new List<XmlElement>(mObjects.Values);

         mPropertyEditorDockPane.Initialize(objects);

         if (showConnections)
         {
            mConnectionEditorDockPane.Initialize(objects, mPropertyEditorDockPane.PropertyEditorGUI.mCallbacks);
         }
         else
         {
            mConnectionEditorDockPane.Uninitialize();
         }
      }
      
      public Dictionary<string, XmlDocument> ApplyPropertiesToObjects()
      {
         Dictionary<string, XmlDocument> result = new Dictionary<string, XmlDocument>();

         if( mObjects.Count > 0 )
         {
            // Apply any changes done to message properties back into the documents.
            mConnectionEditorDockPane.ConnectionEditorGUI.StoreMessagePropertiesGUI();

            // Apply changes done to grouped document in property editor back into the documents that made up the grouped document.
            // NOTE: Ungrouping is currently a destructive process, any references to XmlElements in the document will become invalid, 
            // which is a problem for the connection editor since it holds on to XmlElements for the connections.
            {
               // Create list from for ungroup function.
               List<XmlElement> propertyContainers = new List<XmlElement>();

               foreach (KeyValuePair<string, XmlElement> keyValue in mObjects)
               {
                  XmlDocument clonedDocument = XmlNodeHelpers.CreateDocumentFromNode(keyValue.Value);

                  result.Add(keyValue.Key, clonedDocument);
                  propertyContainers.Add(XmlNodeHelpers.GetDocumentRootElement(clonedDocument));
               }

               // Ungroup current property editor data into the cloned objects.
               GroupProperties.UngroupProperties(mPropertyEditorDockPane.PropertyEditorGUI.Document, propertyContainers, GroupProperties.EGroupOptions.kIgnoreTopName_IgnoreTopId);
            }
         }

         return result;
      }

      public XmlElement GetUpdateableComponentXML(Guid componentEditorId)
      {
         foreach (XmlElement objNav in mObjects.Values)
         {
            XmlElement componentNav = Helpers.GetComponentFromEditorId(objNav, componentEditorId);
            if (componentNav != null)
            {
               return componentNav;
            }
         }

         // Nope, can't find it.
         return null;
      }

      public void SetLabelValue(string objectId, string label, string value)
      {
         Helpers.SetValueAtLabel(mObjects[objectId], label, value);
      }

      public bool SetTypeName(string objectId, string name)
      {
         XmlElement objectElement;
         if (mObjects.TryGetValue(objectId, out objectElement))
         {
            XmlAttribute nameNav = (XmlAttribute)objectElement.SelectSingleNode("//PropertyContainer/@name");
            
            if( nameNav != null)
            {
               if (nameNav.Value != name)
               {
                  nameNav.Value = name;
                  return true;
               }
               else
               {
                  return false;
               }
            }
         }

         // Can't find it, assume name has changed
         return true;
      }

      public void GUISetLabelValue(string label, string value)
      {
         mPropertyEditorDockPane.PropertyEditorGUI.SetValueAtLabel(label, value, Tools.Property.UI.Controls.PropertyControl.SetValueMode.KeepGroupedState);
      }

      // This callback is triggered whenever a value in the property data changes, no matter who changed it (user or application)
      internal void OnValueChanged(object sender, PropertyEditorGUI.PropertyEditorCallbacks.PropertyChangedEventArgs e)
      {
         if (mLivePropertyUpdateEnabled)
         {
            // Send modified properties to game
            if( mPropertySender != null )
            {
               if( mObjects.Count > 0 )
               {
                  mPropertySender.SendModifiedComponentProperties(e.DataNode, new List<XmlElement>(mObjects.Values));
               }
            }

            if (e.DataNode.NeedsExportForUpdate)
            {
               // Need to notify user that they need to re-export to see changes.
               MarkModified();
            }
         }
      }

      internal void OnEvaluatorChanged(object sender, PropertyEditorGUI.PropertyEditorCallbacks.EvaluatorPropertyEventArgs e)
      {
         MarkModified();
      }

      internal void OnComponentChanged(object sender, PropertyEditorGUI.PropertyEditorCallbacks.ComponentGroupEventArgs e)
      {
         MarkModified();
         RefreshConnectionEditor();
      }

      public void MarkModified()
      {
         PropertyEditorDockPane.Text = "Properties (Needs export)";
      }

      public void ClearModifiedFlags()
      {
         PropertyEditorDockPane.Text = "Properties";
      }

      public void SendLuaString(string luaString)
      {
         if( mPropertySender != null )
         {
            mPropertySender.SendLuaString(luaString);
         }
      }

      internal XmlDocument RetrieveObjectFromComponentId(Guid id, bool isReadOnly)
      {
         if (mRetrieveDataMerged != null)
         {
            XmlDocument mergedDoc = mRetrieveDataMerged(id, isReadOnly);
            return mergedDoc;
         }
         return null;
      }

      public XmlElement GetXmlFromObjectId(string objectId)
      {
         XmlElement objectElement = null;
         mObjects.TryGetValue(objectId, out objectElement);
         return objectElement;
      }
   }
}
