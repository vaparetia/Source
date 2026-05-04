#region Using directives

using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Xml.XPath;
using System.Diagnostics;
using System.Windows.Forms;
using System.Drawing;
using Tools.Property.Misc;
using Tools.Property.Source;
using Tools.Property.Build;

#endregion

namespace Tools.Property.UI
{     
   /// <summary>
   /// Builds form from property xml tree.
   /// Takes output from <see cref=" MergeProperties.Merge"/>
   /// and <see cref=" GroupProperties.BuildGroupedProperties"/>.
   /// Use <see cref=" BuildForm"/> to initialize.
   /// Set <see cref=" mPropertyDisplayValueChanged"/> delegate to get callbacks when values change.
   /// Set <see cref=" mErrorProvider"/> for validation of controls.
   /// Use <see cref=" GetValueAtLabel"/> to obtain labeled values.
   /// Use <see cref=" SetValueAtLabel"/> to modify labeled values.   /// 
   /// </summary>
   public class PropertyEditorGUI
   {
      public class PropertyEditorCallbacks
      {
         public class CustomEditor
         {
            // if function returns non null value, this will become the new value of the property
            public delegate string SpawnEditorDelegate(Controls.PropertyControl property);

            public string Info;
            public string ToolTip;
            public SpawnEditorDelegate EditorFunction;

            public CustomEditor(string info, SpawnEditorDelegate editorFunction, string toolTip)
            {
               Info = info;
               EditorFunction = editorFunction;
               ToolTip = toolTip;
            }
         }

         public class GetCustomEditorsEventArgs : System.EventArgs
         {
            public GetCustomEditorsEventArgs()
            {
               CustomEditorList = new List<CustomEditor>();
            }

            public List<CustomEditor> CustomEditorList;
         };

         /// <summary>
         /// Fired when a property changes
         /// </summary>
         /// 

         public class DataNodeEventArgs : System.EventArgs
         {
            protected DataNodeEventArgs( Controls.PropertyControl property, Data.DataNode dataNode )
            {
               mProperty = property;
               mDataNode = dataNode;
            }

            public readonly Controls.PropertyControl mProperty;
            public Data.DataNode DataNode { get { return mDataNode; } }

            private Data.DataNode mDataNode;
         }

         public class PropertyChangedEventArgs : DataNodeEventArgs
         {
            public PropertyChangedEventArgs(Controls.PropertyControl property, Data.Property xmlElement, bool isPreview)
               : base( property, xmlElement )
            {
               IsPreview = isPreview;
            }

            /// <summary>
            /// IsPreview is true for value changes that happen while the value is being tweaked.
            /// </summary>
            public bool IsPreview;
            public new Data.Property DataNode { get { return (Data.Property) base.DataNode; } }
         };

         public class ComponentGroupEventArgs : DataNodeEventArgs
         {
            public ComponentGroupEventArgs( Controls.PropertyControl property, Data.ComponentGroup group )
               : base( property, group )
            {
            }

            public new Data.ComponentGroup DataNode { get { return (Data.ComponentGroup) base.DataNode; } }
         }

         public class EvaluatorPropertyEventArgs : DataNodeEventArgs
         {
            public EvaluatorPropertyEventArgs( Controls.PropertyControl property, Data.EvaluatorProperty xmlElement )
               : base( property, xmlElement )
            {
            }

            public new Data.EvaluatorProperty DataNode { get { return (Data.EvaluatorProperty) base.DataNode; } }
         }

         public delegate void UpdateToolBar(PropertyEditorGUI propertyEditor);
         public delegate void PropertyChanged(PropertyEditorGUI propertyEditor, PropertyChangedEventArgs e);
         public delegate void EvaluatorPropertyChanged( PropertyEditorGUI propertyEditor, EvaluatorPropertyEventArgs e );
         public delegate void ComponentGroupChanged( PropertyEditorGUI propertyEditor, ComponentGroupEventArgs e );
         public delegate void UIStateChanged();
         public delegate void ChangeCurrentComponentInGroupDelegate(Data.ComponentGroup componentGroup, String newComponentId);
         public delegate void AddNewComponentDelegate(String componentGroup, String component);
         public delegate void DeleteComponentGroupDelegate(String componentGroupType, String componentType, String name);
         public delegate void PostCreateComponentGroupDelegate(PropertyEditorGUI editor, Controls.ComponentGroupLayout componentGroup);
         public delegate void BuildCustomPropertyUIDelegate(PropertyEditorGUI editor, Controls.PropertyControl parentControl, Data.Property xml);
         public delegate void GetCustomEditorsDelegate(Controls.PropertyControl sender, GetCustomEditorsEventArgs e);
         public delegate void PostCreateControlsDelegate(PropertyEditorGUI editor, Controls.PropertyControl rootControl);
         public delegate void KeyDownDelegate(PropertyEditorGUI editor, KeyEventArgs keyEventArgs);

         public event UpdateToolBar mUpdateToolBar;
         /// <summary>
         /// Set this for callbacks when property values are changed by the user
         /// </summary>
         public PropertyChanged mPropertyDisplayValueChanged;

         /// <summary>
         /// Set this for callbacks when property values are changed either by the user or externally, e.g. SetValueAtLabel 
         /// </summary>
         public PropertyChanged mPropertyValueChanged;

         /// <summary>Set this for callbacks when evaluators are changed by the user</summary>
         public EvaluatorPropertyChanged mEvaluatorChanged;

         /// <summary>Set this for callbacks when components are changed by the user</summary>
         public ComponentGroupChanged mComponentChanged;

         /// <summary>
         /// This is invoked every time a UI modification has occurred (i.e. minimizing/maximizing layouts)
         /// </summary>
         public UIStateChanged mUIStateChanged;

         /// <summary>
         /// This is invoked to cause a component to be switched.
         /// </summary>
         public ChangeCurrentComponentInGroupDelegate mChangeCurrentComponentInGroup;

         public AddNewComponentDelegate mAddNewComponent;

         /// <summary>
         /// This is invoked to cause a component group to be deleted.
         /// </summary>
         public DeleteComponentGroupDelegate mDeleteComponentGroup;

         public GetCustomEditorsDelegate mGetCustomEditors;

         public PostCreateComponentGroupDelegate mPostCreateComponentGroup;

         public event BuildCustomPropertyUIDelegate mBuildCustomPropertyUI;

         public GUIHelpers.RetrieveObjectFromComponentIdDelegate mRetrieveXmlDelegate;

         public event PostCreateControlsDelegate mPostCreateControls;

         public event KeyDownDelegate mKeyDown;

         public void BuildCustomPropertyUI(PropertyEditorGUI editor, Controls.PropertyControl parentControl, Data.Property dataElement)
         {
            mBuildCustomPropertyUI(editor, parentControl, dataElement);
         }
         
         public void UpdatePropertyEditorToolBar(PropertyEditorGUI editor)
         {
            if( mUpdateToolBar != null )
            {
               mUpdateToolBar(editor);
            }
         }

         public void PostCreateControls(PropertyEditorGUI editor, Controls.PropertyControl rootControl)
         {
            if( mPostCreateControls != null )
            {
               mPostCreateControls(editor, rootControl);
            }
         }

         public void KeyDown(PropertyEditorGUI editor, KeyEventArgs keyEventArgs)
         {
            if( mKeyDown != null )
            {
               mKeyDown(editor, keyEventArgs);
            }
         }
      }
      
      public PropertyEditorCallbacks mCallbacks;

      /// <summary>
      /// Disables property changed callbacks when non-zero.
      /// Reference counted for possible reentrence.
      /// </summary>
      public int              mDontSendPropertyChangedEvents = 0;
      
      /// <summary>
      /// User specified parent control above properties.
      /// </summary>
      public Control          mParentControl;
      
      private PropertyPanel   mPropertyPanel;
      public PropertyPanel PropertyPanel
      {
         get { return mPropertyPanel; }
      }

      private Tools.Controls.ToolStripEx mToolBar;
      
      public Tools.Controls.ToolStripEx ToolBar
      {
         get { return mToolBar; }
      }

      private PropertyHelpSystem mHelpSystem;
      private Panel           mPropertyPanelContainer;
      private VScrollBar      mScrollBar;
      
      /// <summary>
      /// Root of src xml document used to build properties.
      /// </summary>
      XmlDocument   mPropertyXmlDocument;
      
      public XmlDocument Document
      {
         get { return mPropertyXmlDocument; }
      }

      public AssetSystem.Manager mAssetSystem;

      public IPropertyDataSource mPropertyDB;

      /// <summary>
      /// Default constructor. After construction, 
      /// assign <see cref=" mPropertyDisplayValueChanged"/> and 
      /// <see cref=" mErrorProvider"/>, then call
      /// <see cref=" Initialize"/>.
      /// </summary>
      public PropertyEditorGUI(Control parentControl)
      {
         mAssetSystem = new AssetSystem.Manager();
         mCallbacks = new PropertyEditorCallbacks();

         mParentControl = parentControl;
         mParentControl.Controls.Clear();

         mParentControl.Resize += new EventHandler(mParentControl_Resize);

         // Create tool bar, make it invisible by default.
         // It will be made visible when buttons are added.
         {
            mToolBar = new Tools.Controls.ToolStripEx();
            mToolBar.Dock = DockStyle.Top;
            mToolBar.GripStyle = ToolStripGripStyle.Hidden;
            mToolBar.Visible = false;
         }

         mPropertyPanelContainer = new Panel();
         mPropertyPanelContainer.Dock = DockStyle.Fill;

         mScrollBar = new VScrollBar();
         mScrollBar.Dock = DockStyle.Right;
         mScrollBar.Scroll += new ScrollEventHandler(OnScroll);
         mScrollBar.ValueChanged += new EventHandler(mScrollBar_ValueChanged);

         mPropertyPanel = new PropertyPanel();
         mPropertyPanel.PropertyInterface = this;
         mPropertyPanel.Dock = DockStyle.None;
         mPropertyPanel.Location = new Point(0, mToolBar.Height);
         mPropertyPanel.Resize += new EventHandler(mPropertyPanel_Resize);
         mPropertyPanel.MouseWheel += new MouseEventHandler(mPropertyPanel_MouseWheel);
         mPropertyPanel.TabStop = true;

         mParentControl.Controls.Add(mPropertyPanelContainer);
         mPropertyPanelContainer.Controls.Add(mPropertyPanel);
         mPropertyPanelContainer.Controls.Add(mScrollBar);
         mParentControl.Controls.Add(mToolBar);

         mHelpSystem = new PropertyHelpSystem(mPropertyPanel);
      }

      void OnScroll(object sender, ScrollEventArgs e)
      {
         if( !mPropertyPanel.CommitEdits() )
         {
            e.NewValue = mScrollBar.Value;
         }
      }

      public int GetScrollPosition()
      {
         return mScrollBar.Value;
      }

      public void SetScrollPosition(int pos)
      {
         int excessSize = Math.Max(0, mPropertyPanel.Height - mPropertyPanelContainer.Height);
         mScrollBar.Value = Math.Min(Math.Max(pos, 0), excessSize);
      }

      void mPropertyPanel_MouseWheel(object sender, MouseEventArgs e)
      {
         SetScrollPosition(GetScrollPosition() - e.Delta);
      }

      void mPropertyPanel_Resize(object sender, EventArgs e)
      {
         UpdateScrollBar();

         Rectangle propertyRect = mParentControl.ClientRectangle;

         if( mScrollBar.Visible )
         {
            propertyRect.Width -= mScrollBar.Width;
         }
         
         propertyRect.Size = mPropertyPanel.GetPreferredSize(new Size(propertyRect.Width, Int32.MaxValue));
         propertyRect.Y = -mScrollBar.Value;

         mPropertyPanel.Bounds = propertyRect;
         mPropertyPanel.Invalidate();
      }

      void mParentControl_Resize(object sender, EventArgs e)
      {
         UpdateScrollBar();
         mScrollBar_ValueChanged(sender, e);
      }

      void mScrollBar_ValueChanged(object sender, EventArgs e)
      {
         mPropertyPanel.Top = -mScrollBar.Value;
      }

      /// <summary>
      /// Call this when ready construct form in parent control.
      /// Assign <see cref=" mPropertyDisplayValueChanged"/> and 
      /// <see cref=" mErrorProvider"/> before calling this function.
      /// </summary>
      /// <param name="form">
      /// Parent control, property controls will be inserted into this.
      /// </param>
      /// <param name="xmlNav">
      /// Path to root of xml. Contains property description markup. 
      /// Takes output from <see cref=" MergeProperties.Merge"/>
      /// and <see cref=" GroupProperties.BuildGroupedProperties"/>.      
      /// </param>
      public void Initialize(XmlDocument xmlDocument)
      {
         mPropertyXmlDocument = xmlDocument;

         RebuildAllControls();

         mCallbacks.UpdatePropertyEditorToolBar(this);
      }

      public void RebuildAllControls()
      {
         if (mPropertyXmlDocument != null)
         {
            using (new Tools.Common.Timer("RebuildAllControls"))
            {
               Controls.PropertyLayout rootLayout = new Controls.PropertyLayout(mPropertyPanel, null, null);
               rootLayout.Margin = new Padding(0);

               using (new Tools.Common.Timer("Build Controls"))
               {
                  BuildControls(mPropertyPanel, rootLayout, new Data.PropertyContainerDocument(mPropertyXmlDocument).Child);
               }

               mCallbacks.PostCreateControls(this, rootLayout);

               mPropertyPanel.SetRootControl(rootLayout);
            }
         }
         else
         {
            mPropertyPanel.SetRootControl(null);
         }

         UpdateScrollBar();
      }

      private void UpdateScrollBar()
      {
         mScrollBar.LargeChange = mPropertyPanelContainer.Height;
         mScrollBar.SmallChange = 20;
         mScrollBar.Minimum = 0;
         mScrollBar.Maximum = mPropertyPanel.Height;

         int remainingHeight = mPropertyPanel.Height - mScrollBar.Value;

         if (remainingHeight < mPropertyPanelContainer.Height)
         {
            int diff = mPropertyPanelContainer.Height - remainingHeight;
            mScrollBar.Value = Math.Max(mScrollBar.Minimum, mScrollBar.Value - diff);
         }
         else
         {
            mPropertyPanel.Top = -mScrollBar.Value;
         }

         mScrollBar.Visible = (mPropertyPanel.Height > mPropertyPanelContainer.Height);
      }

      public void ShowContextSensitiveHelp()
      {
         // if we're not visible clearly we shouldn't be showing any help
         if (mParentControl == null || mParentControl.Visible == false)
            return;

         bool helpHandled = false;

         ControlInfo controlInfo;
         if (mPropertyPanel.GetControlInfoForScreenPosition(Cursor.Position, out controlInfo))
         {
            for (Data.DataNode dataNode = controlInfo.Control.DataElement; dataNode != null && !helpHandled; dataNode = dataNode.Parent)
            {
               switch( dataNode.GetTypeString() )
               {
                  case Data.ComponentGroup.kTypeString:
                     {
                        Data.ComponentGroup group = (Data.ComponentGroup) dataNode;

                        string componentGroup = group.ID;
                        string component = group.ActiveComponentType;

                        if( component != "None" )
                        {
                           Helpers.SpawnHelp("Component", group.ID, component, "");
                        }
                        else
                        {
                           Helpers.SpawnHelp("Index", "Component", group.ID, "");
                        }

                        helpHandled = true;
                     }
                     break;

                  case Data.EvaluatorProperty.kTypeString:
                     {
                        Data.EvaluatorProperty evaluatorProperty = (Data.EvaluatorProperty) dataNode;

                        string evaluator = evaluatorProperty.ActiveEvaluatorType;

                        if (evaluator != "None")
                        {
                           Helpers.SpawnHelp("Evaluator", "", evaluator, "");
                        }
                        else
                        {
                           Helpers.SpawnHelp("Index", "Evaluator", evaluatorProperty.Type, "");
                        }

                        helpHandled = true;
                     }
                     break;

                  default:
                     break;
               }
            }
         }


         if( !helpHandled )
         {
            Helpers.SpawnHelp("", "", "", "");
         }
      }

      /// <summary>
      /// This function will validate any data that has not yet been validated 
      /// thus causing data entered to be flushed to the xml document
      /// </summary>
      /// <param name="form">form for which to validate the controls for</param>
      static public void FinalizePropertyInput(System.Windows.Forms.Form form)
      {
         form.Validate(true);
      }

      /// <summary>
      /// Returns the 'Value' string for a label.
      /// </summary>
      /// <param name="label">
      /// Name of label. e.g. "Translation"
      /// </param>
      /// <returns>
      /// String value at label is exists, nulled string otherwise.
      /// </returns>                                        
      public string GetValueAtLabel( string label )
      {
         return Helpers.GetValueAtLabel( mPropertyXmlDocument, label );
      }

      /// <summary>
      /// Sets value for labeled property. Also updates control.
      /// Not type safe, see <see cref=" Helpers.SetValueAtLabel"/>.      
      /// </summary>
      /// <param name="label">
      /// Name of label.
      /// </param>
      /// <param name="value">
      /// New string value to set to 'Value' element.
      /// Not type safe, see <see cref=" Helpers.SetValueAtLabel"/>.
      /// </param>
      /// <param name="setValueMode">
      /// Determines how the grouping attribute of the property should be handled.</param>
      /// <returns>
      /// Returns the previous value if label exists, nulled string otherwise.
      /// </returns>
      public string SetValueAtLabel(string label, string value, Controls.PropertyControl.SetValueMode setValueMode)
      {
         string previousValue = null;
         // Prevent modified value broadcasting back to the object that sent the new value
         try
         {
            mDontSendPropertyChangedEvents++;
            previousValue = SetValueAtLabelInternal(mPropertyPanel.RootControl, label, value, setValueMode);
         }
         finally
         {
            mDontSendPropertyChangedEvents--;
         }

         return previousValue;
      }

      private string SetValueAtLabelInternal(Controls.PropertyControl currentControl, string label, string value, Controls.PropertyControl.SetValueMode setValueMode)
      {
         Data.DataNode xml = currentControl.DataElement;
         
         if (xml != null)
         {
            if (xml.Label == label)
            {
               string previousValue = currentControl.GetValue();
               currentControl.SetValue(value, setValueMode);
               mPropertyPanel.InvalidateControl(currentControl);

               return previousValue;
            }
         }

         // Try children
         foreach (Controls.PropertyControl child in currentControl.Children)
         {
            string previousValue = SetValueAtLabelInternal(child, label, value, setValueMode);
            
            if (previousValue != null)
            {
               return previousValue;
            }
         }
         
         return null;
      }

      public void BuildControls(PropertyPanel panel, Controls.PropertyControl parentControl, Data.DataNode rootElement)
      {
         foreach (XmlElement layoutItem in rootElement.Element.SelectNodes("./" + MergeLayout.kElement_UILayout + "/*"))
         {
            BuildControlsForLayoutItem(panel, parentControl, layoutItem, rootElement);
         }
      }

      public void BuildControlsForLayoutItem(PropertyPanel panel, Controls.PropertyControl parentControl, XmlElement layoutElement, Data.DataNode dataElement)
      {
         switch (layoutElement.Name)
         {
            case Helpers.kElement_Layout:
               {
                  BuildLayout(panel, parentControl, layoutElement, dataElement);
               }
               break;

            case "LayoutItem":
               {
                  string itemName = layoutElement.GetAttribute(MergeLayout.kItem_Name, "");

                  Data.DataNode propertyNode = dataElement.FindSubnodeWithName( itemName );

                  if (propertyNode != null)
                  {
                     switch (propertyNode.GetTypeString() )
                     {
                        case Data.ComponentGroup.kTypeString:
                           BuildComponentGroup(panel, parentControl, layoutElement, propertyNode);
                           break;

                        case Data.Property.kTypeString:
                           BuildProperty(panel, parentControl, layoutElement, (Data.Property) propertyNode);
                           break;

                        case Data.PropertyObject.kTypeString:
                           {
                              Data.PropertyObject propertyObject = (Data.PropertyObject) propertyNode;

                              if ( propertyObject.IsVisible )
                              {
                                 if ( propertyObject.IsVectorProperty )
                                 {
                                    BuildVectorProperty( panel, parentControl, layoutElement, propertyNode );
                                 }
                                 else
                                 {
                                    BuildControls( panel, parentControl, propertyNode );
                                 }
                              }
                           }
                           break;

                        case Data.PropertyContainer.kTypeString:
                           BuildControls(panel, parentControl, propertyNode);
                           break;

                        case Data.MessageObject.kTypeString:
                           BuildControls(panel, parentControl, propertyNode);
                           break;

                        case Data.EvaluatorProperty.kTypeString:
                           BuildEvaluatorProperty(panel, parentControl, layoutElement, propertyNode);
                           break;

                        case Data.Evaluator.kTypeString:
                           BuildControls(panel, parentControl, propertyNode);
                           break;
                     }
                  }
               }
               break;
         }
      }

      private void BuildVectorProperty(PropertyPanel panel, Controls.PropertyControl parentControl, XmlElement layoutElement, Data.DataNode dataElement)
      {
         Controls.PropertyControl layout = new Controls.GroupLayout(mPropertyPanel, dataElement, layoutElement);
         parentControl.AddChild(layout);

         BuildControls(panel, layout, dataElement);
      }

      private void BuildEvaluatorProperty(PropertyPanel panel, Controls.PropertyControl parentControl, XmlElement layoutElement, Data.DataNode dataElement)
      {
         Controls.PropertyEvaluator evaluatorProperty = new Controls.PropertyEvaluator(panel, dataElement, layoutElement);
         //Controls.BaseGroupBarLayout evaluatorProperty = new Controls.EvaluatorLayout(panel, dataElement, layoutElement);
         parentControl.AddChild(evaluatorProperty);
      }

      private void BuildTabGroupLayout(PropertyPanel panel, Controls.PropertyControl parentControl, XmlElement layoutElement, Data.DataNode dataElement)
      {
         /*
         TabControl tabControl = new FixedTabControl();
         tabControl.Tag = layoutElement.Clone();

         tabControl.Dock = DockStyle.Top;
         tabControl.AutoSize = true;
         tabControl.Appearance = TabAppearance.FlatButtons;

         tabControl.Selected += new TabControlEventHandler(OnTabSelected);
         parentControl.Controls.Add(tabControl);

         foreach ( XmlElement tabPageLayoutItem in layoutElement.SelectNodes( "./" + Helpers.kElement_Layout + "[@" + Helpers.kLayout_Type + "='" + Helpers.kLayout_Type_tab + "']" ) )
         {
            string tabName = Helpers.GetDisplayName(tabPageLayoutItem);
            if( tabName != "" )
            {
               TabPage tabPage = new TabPage(tabName);
               tabControl.TabPages.Add(tabPage);
               
               tabPage.AutoSize = true;

               foreach ( XmlElement tabPageChildLayoutItem in tabPageLayoutItem.ChildNodes )
               {
                  BuildControlsForLayoutItem(tabPage, tabPageChildLayoutItem, dataNav);
               }
            }
         }

         string activeTab = MergeLayout.GetUIState(layoutElement, "activeTab", "");
         
         for (int i = 0; i < tabControl.TabPages.Count; ++i )
         {
            TabPage tabPage = tabControl.TabPages[i];
            if (tabPage.Text == activeTab)
            {
               tabControl.SelectedTab = tabPage;
            }
         }
          * */
      }

      /*
      void OnTabSelected(object sender, TabControlEventArgs e)
      {
         TabControl tabControl = sender as TabControl;
         TabPage selectedTab = tabControl.SelectedTab;
         if (selectedTab != null)
         {
            XmlElement xmlNav = tabControl.Tag as XmlElement;
            MergeLayout.SetUIState(xmlNav, "activeTab", selectedTab.Text, "");
            SendUIStateChangedEvents();
         }
      }
      */

      private void BuildProperty(PropertyPanel panel, Controls.PropertyControl parentControl, XmlElement layoutElement, Data.Property dataElement)
      {
         if ( !dataElement.IsVisible ) 
            return;   // We can't see this attribute

         string type = dataElement.Type;
         string subType = dataElement.SubType;

         if( type != null )
         {
            switch( type )
            {
               case Helpers.kProperty_Type_float:
                  {
                     Controls.PropertyControl control = new Controls.PropertyFloat(panel, dataElement);
                     parentControl.AddChild(control);
                  }
                  break;

               case Helpers.kProperty_Type_bool:
                  {
                     Controls.PropertyControl control = new Controls.PropertyBool(panel, dataElement);
                     parentControl.AddChild(control);
                  }
                  break;

               case Helpers.kProperty_Type_string:
                  {
                     switch (subType)
                     {
                        case Helpers.kProperty_SubType_asset:
                           {
                              Controls.PropertyControl control = new Controls.PropertyAsset(panel, dataElement);
                              parentControl.AddChild(control);
                           }
                           break;

                        default:
                           {
                              Controls.PropertyControl control = new Controls.PropertyString(panel, dataElement);
                              parentControl.AddChild(control);
                           }
                           break;
                     }
                  }
                  break;

               case Helpers.kProperty_Type_int:
                  {
                     Controls.PropertyControl control = new Controls.PropertyInt(panel, dataElement);
                     parentControl.AddChild(control);
                  }
                  break;

               case Helpers.kProperty_Type_enum:
                  {
                     Controls.PropertyControl control = new Controls.PropertyEnum(panel, dataElement);
                     parentControl.AddChild(control);
                  }
                  break;

               case Helpers.kProperty_Type_color:
                  {
                     Controls.PropertyControl control = new Controls.PropertyColor(panel, dataElement);
                     parentControl.AddChild(control);
                  }
                  break;

               case Helpers.kProperty_Type_evaluatorContainer:
                  //new EvaluatorContainerProperty( xmlElement, this ).BuildUI( parentControl );
                  break;


               case Helpers.kProperty_Type_custom:
                  mCallbacks.BuildCustomPropertyUI(this, parentControl, dataElement);
                  break;

               // no controls for these property types
               case Helpers.kProperty_Type_guid:
                  break;
               case Helpers.kProperty_Type_matrix34:
                  break;

               default:
                  Debug.Assert(false, "Invalid type during PropertyEditorGUI::BuildProperty");
                  break;
            }
         }
      }

      private void BuildLayout(PropertyPanel panel, Controls.PropertyControl parentControl, XmlElement layoutElement, Data.DataNode dataElement)
      {
         string type = layoutElement.GetAttribute(Helpers.kLayout_Type, "");

         switch( type )
         {
            case Helpers.kLayout_Type_group:
               BuildGroupLayout(panel, parentControl, layoutElement, dataElement);
               break;

            case Helpers.kLayout_Type_groupBar:
               BuildGroupBarLayout(panel, parentControl, layoutElement, dataElement);
               break;

            case Helpers.kLayout_Type_tabgroup:
               BuildTabGroupLayout(panel, parentControl, layoutElement, dataElement);
               break;
         }
      }

      private void BuildGroupBarLayout(PropertyPanel panel, Controls.PropertyControl parentControl, XmlElement layoutElement, Data.DataNode dataElement)
      {
         Controls.BaseGroupBarLayout layout = new Controls.GroupBarLayout(panel, null, layoutElement);
         parentControl.AddChild(layout);

         foreach ( XmlElement layoutItemNav in layoutElement.ChildNodes )
         {
            BuildControlsForLayoutItem(panel, layout, layoutItemNav, dataElement);
         }
      }

      private void BuildGroupLayout( PropertyPanel panel, Controls.PropertyControl parentControl, XmlElement layoutElement, Data.DataNode dataElement )
      {
         Controls.BaseGroupBarLayout layout = new Controls.GroupLayout(panel, null, layoutElement);
         parentControl.AddChild(layout);

         foreach (XmlElement layoutItemNav in layoutElement.ChildNodes)
         {
            BuildControlsForLayoutItem(panel, layout, layoutItemNav, dataElement);
         }
      }

      private void BuildComponentGroup( PropertyPanel panel, Controls.PropertyControl parentControl, XmlElement layoutElement, Data.DataNode dataElement )
      {
         Controls.BaseGroupBarLayout componentGroup = new Controls.ComponentGroupLayout(panel, dataElement, layoutElement);
         parentControl.AddChild(componentGroup);
      }

      public void SendPropertyChangedEvents(PropertyEditorGUI.PropertyEditorCallbacks.PropertyChangedEventArgs args)
      {
         // Don't send events if disabled (main case being values modified by external code via SetValueAtLabel)
         if (mDontSendPropertyChangedEvents == 0)
         {
            if (mCallbacks.mPropertyDisplayValueChanged != null)
               mCallbacks.mPropertyDisplayValueChanged(this, args);
         }

         if (mCallbacks.mPropertyValueChanged != null)
            mCallbacks.mPropertyValueChanged(this, args);
      }

      public void SendUIStateChangedEvents()
      {
         if (mCallbacks.mUIStateChanged != null)
            mCallbacks.mUIStateChanged();
      }

      public void SendEvaluatorChangedEvents(PropertyEditorGUI.PropertyEditorCallbacks.EvaluatorPropertyEventArgs args)
      {
         if (mCallbacks.mEvaluatorChanged != null)
            mCallbacks.mEvaluatorChanged(this, args);
      }

      public void SendComponentChangedEvents( PropertyEditorGUI.PropertyEditorCallbacks.ComponentGroupEventArgs args )
      {
         if (mCallbacks.mComponentChanged != null)
            mCallbacks.mComponentChanged(this, args);
      }

      public void AddToolBarItem(ToolStripItem item)
      {
         mToolBar.Items.Add(item);

         if(!mToolBar.Visible)
         {
            mToolBar.Visible = true;
         }
      }
   }
}
