using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;
using System.Xml.XPath;
using Tools.Property.Source;
using Tools.Common;
using Tools.GameEngine;
using Tools.Property.Misc;
using Tools.Property.Build;
using System.Xml;

namespace Tools.Property.UI
{
   public partial class ConnectionEditorGUI : UserControl
   {
      class ConnectionEditorRowInfo
      {
         public enum ERowType
         {
            kEvent,
            kLink,
            kGroupHeader,
            kObjectHeader
         }

         public enum EDirection
         {
            kIncoming,
            kOutgoing,
            kInvalid
         }

         string mSourceObject;

         XmlElement mConnectionElement;
         ERowType mConnectionType;
         bool mbActive;
         bool mbTargetExists = true;
         string mEventName;
         string mMessage;
         string mDestinationObject;
         Guid mDestinationId;
         int mPriority;
         float mTime;
         string mSenderQuery;
         string mDestQuery;
         EDirection mDirection = EDirection.kOutgoing;

         public ConnectionEditorRowInfo(XmlElement connectionElement,
                                        ERowType type,
                                        bool bActive,
                                        string eventName,
                                        string message,
                                        string destinationObject,
                                        Guid destinationId,
                                        int priority,
                                        float time,
                                        string senderQuery,
                                        string destQuery)
         {
            mConnectionElement = connectionElement;
            mConnectionType = type;
            mbActive = bActive;
            mEventName = eventName;
            mMessage = message;
            mDestinationObject = destinationObject;
            mDestinationId = destinationId;
            mPriority = priority;
            mTime = time;
            mSenderQuery = senderQuery;
            mDestQuery = destQuery;
         }

         public ConnectionEditorRowInfo(ERowType type, string displayName, Guid destinationId)
         {
            mConnectionType = type;
            mDirection = EDirection.kInvalid;
            mSourceObject = displayName;
            mDestinationId = destinationId;
         }

         /// <summary>
         /// Propagates modifiable values in class to xml.
         /// </summary>
         public void UpdateXml()
         {
            // Updated values in xml with those class (will have been modified by datagridview)
            mConnectionElement.SetAttribute( Helpers.kEventEntry_active, mbActive.ToString() );
            mConnectionElement.SetAttribute( Helpers.kEventEntry_priority, mPriority.ToString() );
            mConnectionElement.SetAttribute( Helpers.kEventEntry_time, mTime.ToString() );
            {
               // Deal with null strings from empty boxes
               if (mSenderQuery == null) mSenderQuery = String.Empty;
               mConnectionElement.SetAttribute( Helpers.kEventEntry_senderQuery, mSenderQuery );
            }

            {
               // Deal with null strings from empty boxes
               if (mDestQuery == null) mDestQuery = String.Empty;
               mConnectionElement.SetAttribute( Helpers.kEventEntry_destQuery, mDestQuery );
            }

         }

         public XmlElement ConnectionElement { get { return mConnectionElement; } }
         public ERowType ConnectionType { get { return mConnectionType; } }
         public bool Active { get { return mbActive; } set { mbActive = value; } }
         public string SourceObject { get { return mSourceObject; } }
         public string EventName { get { return mEventName; } }
         public string Message { get { return mMessage; } }
         public string DestinationObject { get { return mDestinationObject; } }
         public Guid DestinationId { get { return mDestinationId; } }
         public int Priority { get { return mPriority; } set { mPriority = value; } }
         public float Time { get { return mTime; } set { mTime = value; } }
         public string SenderQuery { get { return mSenderQuery; } set { mSenderQuery = value; } }
         public string DestQuery { get { return mDestQuery; } set { mDestQuery = value; } }
         public EDirection Direction { get { return mDirection; } set { mDirection = value; } }
         public bool TargetExists { get { return mbTargetExists; } set { mbTargetExists = value; } }
      }

      // Implements the manual sorting of items by columns.
      class ListViewItemComparer : System.Collections.IComparer
      {
         public int mCol;
         public bool mAscending;
         public ListViewItemComparer()
         {
            mCol = 0;
            mAscending = true;
         }
         public ListViewItemComparer(int column, bool ascending)
         {
            mCol = column;
            mAscending = ascending;
         }
         public int Compare(object x, object y)
         {
            int compare = String.Compare(((ListViewItem)x).SubItems[mCol].Text, ((ListViewItem)y).SubItems[mCol].Text);
            return mAscending ? compare : -compare;
         }
      }

      class ElementAndComponent
      {
         public ElementAndComponent(XmlElement nav, XmlElement componentNav)
         {
            mNav = nav;
            mComponentNav = componentNav;
         }

         public XmlElement mNav;
         public XmlElement mComponentNav;
      };

      class ConnectionListView : ListView
      {
         protected override void OnResize(EventArgs e)
         {
            // this helps out with wrong horizontal scroll bars showing up by FAST resizing of the control (making it smaller)
            Columns[0].Width = ClientSize.Width - 16;
         }
      }

      // Custom render
      int mCachedRowHeight;

      IPropertyDataSource mPropertySource;
      PropertyEditorGUI.PropertyEditorCallbacks mPropertyEditorCallbacks;
      List<XmlElement> mSelectedObjects;
      /// <summary>
      /// Contains only the active components in a script object (i.e. ones where the component group value matches)
      /// </summary>
      List<object> mConnectionPropertiesContainer;
      PropertyEditorGUI mMessagePropertiesGUI;
      /// <summary>
      /// Array of connections currently being edited by property editor.
      /// </summary>
      List<ConnectionEditorRowInfo> mMessagePropertiesGUIEditedConnections;
      List<XmlElement> mMessagePropertiesGUISrcProperties;

      public delegate void ConnectionsChanged(object sender);
      public ConnectionsChanged mConnectionsChanged;

      public delegate List<Guid> RetrieveObjectIdsConnectedToObjectIdsDelegate(List<Guid> targetComponentIds);
      public RetrieveObjectIdsConnectedToObjectIdsDelegate mRetrieveObjectIdsConnectedToObjectIdsDelegate;

      public delegate void ChangeObjectSelection(Guid targetObject);
      public ChangeObjectSelection mChangeObjectSelection;

      /// <summary>
      /// Properties of edited connections.
      /// </summary>
      bool mDisableUpdateMessagePropertiesOnSelectionChange;

      public ConnectionEditorGUI()
      {
         InitializeComponent();
      }

      public void Initialize(List<XmlElement> objects,
                             IPropertyDataSource propertySource,
                             PropertyEditorGUI.PropertyEditorCallbacks callbacks)
      {
         mPropertySource = propertySource;
         mPropertyEditorCallbacks = callbacks;
         mSelectedObjects = objects;
         mMessagePropertiesGUIEditedConnections = new List<ConnectionEditorRowInfo>();
         mMessagePropertiesGUISrcProperties = new List<XmlElement>();
         mDisableUpdateMessagePropertiesOnSelectionChange = false;

         mPropertyPanel.Hide();
         mPropertyPanel.SuspendLayout();

         InitializeComponentProperties();

         mPropertyPanel.ResumeLayout();
         mPropertyPanel.Show();
      }

      public void Uninitialize()
      {
         Initialize(new List<XmlElement>(), mPropertySource, mPropertyEditorCallbacks);
      }

      /// <summary>
      /// Updates controls to new values from active components.
      /// Call this if the active component changes, but ungroup first.
      /// You'll probably want to call 'ValidateChildren' on the form 
      /// (or PropertyEditorGUI.FinalizePropertyInput) before calling this so 
      /// that the message properties get flushed.
      /// </summary>
      public void InitializeComponentProperties()
      {
         if ( mSelectedObjects != null )
         {
            StoreMessagePropertiesGUI();
            FillSrcAndDestListBoxes();
            Update_AddConnectionButtonStatus();
            UpdateConnectionsListBox();
            Update_RemoveConnectionButtonStatus();
         }
      }

      void mConnectionsDataGridView_RowPostPaint(object sender, DataGridViewRowPostPaintEventArgs e)
      {
         DataGridView dg = sender as DataGridView;

         ConnectionEditorRowInfo connectionProperties = mConnectionPropertiesContainer[e.RowIndex] as ConnectionEditorRowInfo;
         if (connectionProperties != null)
         {
            switch (connectionProperties.ConnectionType)
            {
               case ConnectionEditorRowInfo.ERowType.kGroupHeader:
                  {
                     // Fill with background color

                     e.Graphics.FillRectangle(Brushes.Black, e.RowBounds);

                     // Draw header text
                     System.Drawing.Point textLocation = e.RowBounds.Location;
                     textLocation.Offset(-dg.HorizontalScrollingOffset + 4, 7);

                     Font font = GUIHelpers.skBoldDialogFont;

                     e.Graphics.DrawString(connectionProperties.SourceObject,
                                           font,
                                           System.Drawing.Brushes.White,
                                           textLocation);
                  }
                  break;

               case ConnectionEditorRowInfo.ERowType.kObjectHeader:
                  {
                     // Fill with background color
                     e.Graphics.FillRectangle(Brushes.LightGray, e.RowBounds);

                     // Draw header text
                     System.Drawing.Point textLocation = e.RowBounds.Location;
                     textLocation.Offset(-dg.HorizontalScrollingOffset + 4, 4);

                     Font font = GUIHelpers.skBoldDialogFont;

                     e.Graphics.DrawString(connectionProperties.SourceObject,
                                           font,
                                           System.Drawing.Brushes.Black,
                                           textLocation);
                  }
                  break;
            }
         }
      }

      void mConnectionsDataGridView_RowHeightInfoNeeded(object sender, DataGridViewRowHeightInfoNeededEventArgs e)
      {
         if (e.RowIndex >= mConnectionPropertiesContainer.Count)
         {
            // Out of bounds, occurs when last item is deleted. You'd think that it wouldn't call this!
            e.Height = 0;
            return;
         }

         DataGridView dg = sender as DataGridView;
         if (dg.DefaultCellStyle.WrapMode == DataGridViewTriState.False)
         {
            // Speedup for no row wrap
            if (e.RowIndex == 0)
            {
               mCachedRowHeight = dg.Rows[e.RowIndex].GetPreferredHeight(e.RowIndex, DataGridViewAutoSizeRowMode.AllCellsExceptHeader, true);
            }
            e.Height = mCachedRowHeight;
         }
         else
         {
            e.Height = dg.Rows[e.RowIndex].GetPreferredHeight(e.RowIndex, DataGridViewAutoSizeRowMode.AllCellsExceptHeader, true);
         }

         DataGridViewRow row = dg.Rows[e.RowIndex];
         ConnectionEditorRowInfo properties = row.DataBoundItem as ConnectionEditorRowInfo;
         if( properties != null )
         {
            switch(properties.ConnectionType)
            {
               case ConnectionEditorRowInfo.ERowType.kEvent:
               case ConnectionEditorRowInfo.ERowType.kLink:
                  {
                     if (properties.Direction == ConnectionEditorRowInfo.EDirection.kIncoming)
                     {
                        foreach (DataGridViewCell cell in row.Cells)
                        {
                           cell.ReadOnly = true;
                        }
                     }
                     else
                     {
                        foreach (DataGridViewCell cell in row.Cells)
                        {
                           cell.ReadOnly = false;
                        }

                        if (Helpers.IsEventOrLinkEntryPrefab(properties.ConnectionElement))
                        {
                           row.Cells["EventColumn"].ReadOnly = true;
                           row.Cells["MessageColumn"].ReadOnly = true;
                        }
                     }

                     foreach (DataGridViewCell cell in row.Cells)
                     {
                        Color cellColor = System.Drawing.SystemColors.WindowText;
                        
                        if( !properties.TargetExists )
                        {
                           cellColor = System.Drawing.Color.Red;
                        }
                        else if( cell.ReadOnly )
                        {
                           cellColor = System.Drawing.SystemColors.GrayText;
                        }

                        cell.Style.ForeColor = cellColor;
                        cell.Style.SelectionForeColor = cellColor;

                        DataGridViewLinkCell linkCell = cell as DataGridViewLinkCell;
                        if (linkCell != null)
                        {
                           linkCell.ActiveLinkColor = cellColor;
                           linkCell.LinkColor = cellColor;
                           linkCell.LinkBehavior = cell.ReadOnly ? LinkBehavior.NeverUnderline : LinkBehavior.AlwaysUnderline;
                        }
                     }
                  }
                  break;
               
               case ConnectionEditorRowInfo.ERowType.kGroupHeader:
                  {
                     e.Height += 6;

                     foreach (DataGridViewCell cell in row.Cells)
                        cell.ReadOnly = true;
                  }
                  break;
            }
         }
      }

      void ObjectsListView_ColumnClick(object sender, ColumnClickEventArgs e)
      {
         ListView listView = sender as ListView;
         // Set the ListViewItemSorter property to a new ListViewItemComparer 
         // object. Setting this property immediately sorts the 
         // ListView using the ListViewItemComparer object.
         bool bAscending = true;
         ListViewItemComparer lvComparer = (listView.ListViewItemSorter as ListViewItemComparer);
         if (lvComparer != null)
         {
            // reverse sort direction if clicked again (i.e. sorter present and has same column)
            if (lvComparer.mCol == e.Column)
            {
               bAscending = !lvComparer.mAscending;
            }
         }
         listView.ListViewItemSorter = new ListViewItemComparer(e.Column, bAscending);
      }

      void FillSrcAndDestListBoxes()
      {
         mSrcObjectsListView.BeginUpdate();
         mDestObjectsListView.BeginUpdate();

         mSrcObjectsListView.Groups.Clear();
         mSrcObjectsListView.ShowGroups = true;

         mSrcObjectsListView.Items.Clear();
         mDestObjectsListView.Items.Clear();

         XPathExpression componentSelectEvents = XPathExpression.Compile("./Events/Event");
         XPathExpression componentSelectLinks = XPathExpression.Compile("./Links/Link");
         XPathExpression componentSelectMessages = XPathExpression.Compile("./Messages/Message");
         
         foreach (XmlElement objectElement in mSelectedObjects)
         {
            // Get object name
            XmlElement objNameNav = (XmlElement) objectElement.SelectSingleNode("/PropertyContainer[@name]");
            string objName = Helpers.GetDisplayName(objNameNav);
            
            // Go over all components
            foreach ( XmlElement componentGroupElement in objectElement.SelectNodes( "/PropertyContainer/ComponentGroup" ) )
            {
               // Find the active component
               string activeComponent = Helpers.GetActiveComponentNameFromComponentGroup(componentGroupElement);
               if (activeComponent == Helpers.kEmptyComponentName)
               {
                  continue;
               }

               string componentName = Helpers.GetDisplayName(componentGroupElement);
               string componentGroupName = componentGroupElement.GetAttribute(Helpers.kComponentGroup_Id, String.Empty);

               XmlElement componentNav = Helpers.GetComponentFromComponentGroup(componentGroupElement, activeComponent);
               string componentDisplayName = Helpers.GetDisplayName(componentNav);

               string componentInfoStr = string.Format("{0} ({1})", objName, RGCHelpers.GetComponentDisplayString(componentGroupName, componentDisplayName, componentName));

               ListViewGroup group = mSrcObjectsListView.Groups[componentInfoStr];
               
               if( group == null )
               {
                  group = new ListViewGroup(componentInfoStr);
                  group.Name = componentInfoStr;
               
                  mSrcObjectsListView.Groups.Add(group);
                  mDestObjectsListView.Groups.Add(group);
               }

               // Add events
               foreach ( XmlElement eventNav in XmlNodeHelpers.SelectNodes( componentNav, componentSelectEvents ) )
               {
                  string eventType = eventNav.GetAttribute(Helpers.kEvent_type, String.Empty);
                  ListViewItem newItem = new ListViewItem(eventType);
                  newItem.Tag = new ElementAndComponent(eventNav, componentNav);
                  newItem.ToolTipText = Helpers.GetToolTipText(eventNav);
                  newItem.Group = group;
                  mSrcObjectsListView.Items.Add(newItem);
               }

               // Add links
               foreach ( XmlElement linkNav in XmlNodeHelpers.SelectNodes( componentNav, componentSelectLinks ) )
               {
                  string linkType = linkNav.GetAttribute(Helpers.kLink_type, String.Empty);
                  // Links draw in green
                  ListViewItem newItem = new ListViewItem(linkType);
                  newItem.ForeColor = System.Drawing.Color.Blue;
                  newItem.Tag = new ElementAndComponent(linkNav, componentNav);
                  newItem.ToolTipText = "[Link]:" + Helpers.GetToolTipText(linkNav);
                  newItem.Group = group;
                  mSrcObjectsListView.Items.Add(newItem);
               }

               // Add messages
               foreach ( XmlElement messageNav in XmlNodeHelpers.SelectNodes( componentNav, componentSelectMessages ) )
               {
                  string messageType = messageNav.GetAttribute(Helpers.kMessage_type, String.Empty);
                  ListViewItem newItem = new ListViewItem(messageType);
                  newItem.Tag = new ElementAndComponent(messageNav, componentNav);
                  newItem.ToolTipText = Helpers.GetToolTipText(messageNav);
                  newItem.Group = group;
                  mDestObjectsListView.Items.Add(newItem);
               }

               // Add link target entry
               {
                  ListViewItem newItem = new ListViewItem("[LinkTarget]");
                  newItem.Tag = new ElementAndComponent(null, componentNav);
                  newItem.ToolTipText = "Target for creating a link to this component";
                  newItem.Group = group;
                  newItem.ForeColor = GetListItemColor(newItem);
                  mDestObjectsListView.Items.Add(newItem);
               }
            }
         }

         mSrcObjectsListView.EndUpdate();
         mDestObjectsListView.EndUpdate();
      }

      private void SrcDestObjectsListBox_SelectedIndexChanged(object sender, EventArgs e)
      {
         /*
         Debug.Print("{0}", mSrcObjectsListBox.SelectedIndices.Count);
         // Reevaluate events and messages list boxes based of selected components
         foreach(ListBoxComponent component in mSrcObjectsListBox.SelectedItems)
         {
            //ListBoxComponent component = mSrcObjectsListBox.Items[selected] as ListBoxComponent;
            Debug.Print("{0}", component.ToString());
         }
         */
         // Check src selection to see if there's a link and then enable/disable dest selections based on that
         // Set color/tag on dest objects to default
         foreach (ListViewItem destItem in mDestObjectsListView.Items)
         {
            bool bCanConnect = true;

            ElementAndComponent destComponentAndNav = destItem.Tag as ElementAndComponent;
            if (destComponentAndNav.mComponentNav != null)
            {
               foreach (ListViewItem srcItem in mSrcObjectsListView.SelectedItems)
               {
                  XmlElement srcNav = (srcItem.Tag as ElementAndComponent).mNav;
                  if (srcNav != null)
                  {
                     if(srcNav.Name == Helpers.kElement_Link)
                     {
                        if (!Helpers.CanLinkToComponent(srcNav, destComponentAndNav.mComponentNav) ||
                            destComponentAndNav.mNav != null)
                        {
                           bCanConnect = false;
                           break;
                        }
                     }
                     else if(srcNav.Name == Helpers.kElement_Event)
                     {
                        if (destComponentAndNav.mNav == null)
                        {
                           bCanConnect = false;
                           break;
                        }
                     }
                  }
               }
            }

            if (bCanConnect)
            {
               // Reset back to normal color
               destItem.ForeColor = GetListItemColor(destItem);
            }
            else
            {
               // Nope, can't connect, deselect, change color
               destItem.ForeColor = System.Drawing.Color.Gray;
               destItem.Selected = false;
            }
         }


         Update_AddConnectionButtonStatus();
      }

      private Color GetListItemColor(ListViewItem destItem)
      {
         ElementAndComponent itemInfo = destItem.Tag as ElementAndComponent;
         if( itemInfo != null )
         {
            if (itemInfo.mNav == null)
               return System.Drawing.Color.Blue;
         }

         return System.Drawing.SystemColors.WindowText;
      }

      void Update_AddConnectionButtonStatus()
      {
         bool bIsActive = false;
         if ((mSrcObjectsListView.SelectedItems.Count != 0) &&
               (mDestObjectsListView.SelectedItems.Count != 0))
         {
            // We can add a special check here for links
            bIsActive = true;
         }

         mAddConnectionButton.Enabled = bIsActive;
      }

      void AddConnectionButton_Click(object sender, EventArgs e)
      {
         // make sure message properties are written back to xml
         StoreMessagePropertiesGUI();

         // Iterate over events / links
         foreach (ListViewItem selectedEvent in mSrcObjectsListView.SelectedItems)
         {
            ElementAndComponent selectedEventTag = selectedEvent.Tag as ElementAndComponent;
            XmlElement srcComponentNav = selectedEventTag.mComponentNav;
            XmlElement selectedEventNav = selectedEventTag.mNav;
            string eventTypeName = selectedEventNav.GetAttribute(Helpers.kEvent_type, String.Empty);
            foreach (ListViewItem selectedTarget in mDestObjectsListView.SelectedItems)
            {
               ElementAndComponent selectedTargetTag = selectedTarget.Tag as ElementAndComponent;
               XmlElement selectedTargetNav = selectedTargetTag.mNav;
               XmlElement destComponentNav = selectedTargetTag.mComponentNav;
               if (selectedEventNav.Name == Helpers.kElement_Event)
               {
                  // if this is null, then a "link target" was selected and we skip this for an "event" connection.
                  if( selectedTargetNav != null )
                  {
                     string messageTypeName = selectedTargetNav.GetAttribute(Helpers.kMessage_type, String.Empty);
                     Guid targetComponentGuid = Helpers.GetInternalEditorIdGuid(destComponentNav);
                     Helpers.CreateEvent(srcComponentNav,
                                        eventTypeName,
                                        messageTypeName,
                                        targetComponentGuid,
                                        mPropertySource);
                  }
               }
               else
               {
                  // if this is null, then a "link target" was selected and we're using it for this "link" connection.
                  if (selectedTargetNav == null)
                  {
                     Guid targetComponentGuid = Helpers.GetInternalEditorIdGuid(destComponentNav);
                     Helpers.CreateLink(srcComponentNav,
                                       eventTypeName,
                                       targetComponentGuid,
                                       mPropertySource);
                  }
               }
            }
         }

         UpdateConnectionsListBox();

         // Notify
         if (mConnectionsChanged != null) 
            mConnectionsChanged(this);
      }

      void Update_RemoveConnectionButtonStatus()
      {
         bool allowRemove = false;

         foreach (DataGridViewRow row in mConnectionsDataGridView.SelectedRows)
         {
            ConnectionEditorRowInfo connectionProperties = row.DataBoundItem as ConnectionEditorRowInfo;
            if( connectionProperties.Direction == ConnectionEditorRowInfo.EDirection.kOutgoing && !Helpers.IsEventOrLinkEntryPrefab(connectionProperties.ConnectionElement) )
            {
               allowRemove = true;
               break;
            }
         }

         mRemoveConnectionButton.Enabled = allowRemove;
      }

      void RemoveConnectionButton_Click(object sender, EventArgs e)
      {
         // make sure message properties are written back to xml
         StoreMessagePropertiesGUI();
         foreach (DataGridViewRow rowToDelete in mConnectionsDataGridView.SelectedRows)
         {
            ConnectionEditorRowInfo rowProperties = rowToDelete.DataBoundItem as ConnectionEditorRowInfo;
            switch(rowProperties.ConnectionType)
            {
               case ConnectionEditorRowInfo.ERowType.kEvent:
               case ConnectionEditorRowInfo.ERowType.kLink:
                  {
                     if( !Helpers.IsEventOrLinkEntryPrefab(rowProperties.ConnectionElement) )
                     {
                        rowProperties.ConnectionElement.ParentNode.RemoveChild( rowProperties.ConnectionElement );
                     }
                  }
                  break;
            }
         }

         UpdateConnectionsListBox();

         // Notify
         if (mConnectionsChanged != null) 
            mConnectionsChanged(this);
      }

      void UpdateConnectionsListBox()
      {
         mConnectionPropertiesContainer = new List<object>();

         List<Guid> selectedObjectIds = new List<Guid>();
         List<Guid> selectedComponentIds = new List<Guid>();

         // Add outgoing connections
         {
            // Go over all selected objects
            foreach (XmlNode objectNav in mSelectedObjects)
            {
               bool didAddGroupHeader = false;

               // Get object name
               XmlElement objNav = (XmlElement) objectNav.SelectSingleNode("/PropertyContainer");
               string objName = Helpers.GetDisplayName(objNav);

               Guid objectGuid = Helpers.GetInternalEditorIdGuid(objNav);

               if (!selectedObjectIds.Contains(objectGuid))
                  selectedObjectIds.Add(objectGuid);

               // Find all components
               foreach ( XmlElement componentGroupNav in objectNav.SelectNodes("/PropertyContainer/ComponentGroup") )
               {
                  string componentGroupName = componentGroupNav.GetAttribute(Helpers.kComponentGroup_Id, String.Empty);
                  string componentName = Helpers.GetDisplayName(componentGroupNav);

                  // Find the active component
                  string activeComponent = Helpers.GetActiveComponentNameFromComponentGroup(componentGroupNav);

                  if (activeComponent == Helpers.kEmptyComponentName)
                     continue;

                  XmlElement componentNav = Helpers.GetComponentFromComponentGroup(componentGroupNav, activeComponent);

                  Guid componentGuid = Helpers.GetInternalEditorIdGuid(componentGroupNav);

                  if (!selectedComponentIds.Contains(componentGuid))
                     selectedComponentIds.Add(componentGuid);

                  string sourceName = RGCHelpers.GetComponentDisplayString(componentGroupName, activeComponent, componentName);

                  XmlNodeList eventIterator = componentNav.SelectNodes("Events/Event/EventEntry");
                  XmlNodeList linkIterator = componentNav.SelectNodes("Links/Link/LinkEntry");

                  if (eventIterator.Count > 0 || linkIterator.Count > 0 )
                  {
                     if (!didAddGroupHeader)
                     {
                        mConnectionPropertiesContainer.Add(new ConnectionEditorRowInfo(ConnectionEditorRowInfo.ERowType.kGroupHeader, string.Format("Outgoing Connections From {0}", objName), objectGuid));
                        didAddGroupHeader = true;
                     }

                     mConnectionPropertiesContainer.Add(new ConnectionEditorRowInfo(ConnectionEditorRowInfo.ERowType.kObjectHeader, sourceName, objectGuid));
                  }

                  // Find all connections on active components
                  foreach(XmlElement eventConnectionNav in eventIterator)
                  {
                     ConnectionEditorRowInfo connectionProperties = BuildEventConnectionProperties(eventConnectionNav);
                     mConnectionPropertiesContainer.Add(connectionProperties);
                  }

                  // Find all connections on active components
                  foreach ( XmlElement linkConnectionNav in linkIterator )
                  {
                     ConnectionEditorRowInfo connectionProperties = BuildLinkConnectionProperties(linkConnectionNav);

                     mConnectionPropertiesContainer.Add(connectionProperties);
                  }
               }
            }
         }

         // Add incoming connections
         if (mRetrieveObjectIdsConnectedToObjectIdsDelegate != null)
         {
            // Go over all selected objects
            foreach (XmlNode selectedObjectNav in mSelectedObjects)
            {
               // Get object name for selected object
               XmlElement destObjectNavigator = (XmlElement) selectedObjectNav.SelectSingleNode("/PropertyContainer");
               string destObjectName = Helpers.GetDisplayName(destObjectNavigator);

               Guid destObjectGuid = Helpers.GetInternalEditorIdGuid(destObjectNavigator);

               // Build list of component ids for the current object
               List<Guid> componentsForSelectedObjectIds = new List<Guid>();
               {
                  foreach (XmlElement componentGroupNav in destObjectNavigator.SelectNodes("/PropertyContainer/ComponentGroup"))
                  {
                     Guid componentGuid = Helpers.GetInternalEditorIdGuid(componentGroupNav);
                     componentsForSelectedObjectIds.Add(componentGuid);
                  }
               }

               // Build list of incoming connections for selected object
               List<XmlElement> incomingConnections = new List<XmlElement>();
               {
                  List<Guid> selectedObjectIdList = new List<Guid>();
                  selectedObjectIdList.Add(destObjectGuid);

                  // Get a list of all object id's that are connected to the selected object
                  List<Guid> sourceObjectIds = mRetrieveObjectIdsConnectedToObjectIdsDelegate(selectedObjectIdList);

                  foreach (Guid sourceObjectId in sourceObjectIds)
                  {
                     // Don't add connection if the source object is already selected (it will be shown as an outgoing connection in that case anyway).
                     if (selectedObjectIds.Contains(sourceObjectId))
                        continue;

                     // Go over all events and links of the object, if it has one of our components as a target, we add the connection.
                     XmlDocument inputObjectDoc = mPropertyEditorCallbacks.mRetrieveXmlDelegate(sourceObjectId, false);

                     foreach (XmlElement connection in inputObjectDoc.SelectNodes("//EventEntry | //LinkEntry"))
                     {
                        Guid connectionTargetId = new Guid(connection.GetAttribute(Helpers.kEventEntry_target));

                        if (selectedComponentIds.Contains(connectionTargetId))
                           incomingConnections.Add(connection);
                     }
                  }
               }

               if (incomingConnections.Count > 0)
               {
                  mConnectionPropertiesContainer.Add(new ConnectionEditorRowInfo(ConnectionEditorRowInfo.ERowType.kGroupHeader, string.Format("Incoming Connections To {0}", destObjectName), Guid.Empty));
               }

               string lastObjectHeader = "";

               foreach (XmlElement incomingConnection in incomingConnections)
               {
                  Guid targetComponentGuid = new Guid(incomingConnection.GetAttribute(Helpers.kEventEntry_target, ""));
                  
                  // Skip over connection if target component isn't part of the selected object we're processing
                  if (!componentsForSelectedObjectIds.Contains(targetComponentGuid))
                     continue;

                  XmlElement componentNav = incomingConnection;
                  while ( componentNav.Name != Helpers.kElement_Component )
                  {
                     componentNav = (XmlElement) componentNav.ParentNode;
                  }

                  XmlElement componentGroupNav = (XmlElement) componentNav.ParentNode;
                  XmlElement objectNav = componentGroupNav;
                  while ( objectNav.Name != Helpers.kElement_PropertyContainer )
                  {
                     objectNav = (XmlElement) objectNav.ParentNode;
                  }

                  Guid objectGuid = Helpers.GetInternalEditorIdGuid(objectNav);

                  string objName = Helpers.GetDisplayName(objectNav);

                  string componentGroupId = componentGroupNav.GetAttribute(Helpers.kComponentGroup_Id, String.Empty);
                  string componentId = componentNav.GetAttribute(Helpers.kComponentGroup_Id, String.Empty);
                  string componentName = Helpers.GetDisplayName(componentGroupNav);

                  string sourceName = string.Format("{0} ({1})", objName, RGCHelpers.GetComponentDisplayString(componentGroupId, componentId, componentName));

                  if (lastObjectHeader != sourceName )
                  {
                     mConnectionPropertiesContainer.Add(new ConnectionEditorRowInfo(ConnectionEditorRowInfo.ERowType.kObjectHeader, sourceName, objectGuid));
                     lastObjectHeader = sourceName;
                  }

                  switch (incomingConnection.Name)
                  {
                     case Helpers.kEventEntry:
                        {
                           ConnectionEditorRowInfo connectionProperties = BuildEventConnectionProperties(incomingConnection);
                           connectionProperties.Direction = ConnectionEditorRowInfo.EDirection.kIncoming;
                           mConnectionPropertiesContainer.Add(connectionProperties);
                        }
                        break;

                     case Helpers.kLinkEntry:
                        {
                           ConnectionEditorRowInfo connectionProperties = BuildLinkConnectionProperties(incomingConnection);
                           connectionProperties.Direction = ConnectionEditorRowInfo.EDirection.kIncoming;
                           mConnectionPropertiesContainer.Add(connectionProperties);
                        }
                        break;
                  }
               }
            }
         }

         // For some reason, selection changed gets called lots of times when the property container is changed
         // Update manually after adding properties

         mDisableUpdateMessagePropertiesOnSelectionChange = true;
         ConnectionPropertiesBindingSource.DataSource = mConnectionPropertiesContainer;
         mDisableUpdateMessagePropertiesOnSelectionChange = false;

         UpdateMessagePropertiesGUI();
      }

      private ConnectionEditorRowInfo BuildEventConnectionProperties(XmlElement eventConnectionNav)
      {
         Guid target = new Guid(eventConnectionNav.GetAttribute(Helpers.kEventEntry_target, ""));
         string targetName;
         bool targetExists = GUIHelpers.ResolveComponentIdToName(target, mPropertyEditorCallbacks.mRetrieveXmlDelegate, out targetName);
         float time = 0;
         {
            string timeString = eventConnectionNav.GetAttribute(Helpers.kEventEntry_time, "");
            time = float.Parse(timeString);
         }
         bool bActive = false;
         {
            string activeString = eventConnectionNav.GetAttribute(Helpers.kEventEntry_active, "");
            bActive = Tools.Common.Misc.ParseBool(activeString);
         }
         int priority = 0;
         {
            string priorityString = eventConnectionNav.GetAttribute(Helpers.kEventEntry_priority, "");
            priority = int.Parse(priorityString);
         }

         string senderQuery = eventConnectionNav.GetAttribute(Helpers.kEventEntry_senderQuery, "");
         string destQuery = eventConnectionNav.GetAttribute(Helpers.kEventEntry_destQuery, "");

         XmlElement connectionParentNav = (XmlElement) eventConnectionNav.ParentNode;
         string eventType = connectionParentNav.GetAttribute(Helpers.kEvent_type, "");

         string messageType = "Unknown";
         XmlElement messageNav = (XmlElement) eventConnectionNav.SelectSingleNode("MessageObject");
         if (messageNav != null)
         {
            messageType = messageNav.GetAttribute(Helpers.kMessage_id, "");
         }
         ConnectionEditorRowInfo connectionProperties = new ConnectionEditorRowInfo(eventConnectionNav,
                                                                                    ConnectionEditorRowInfo.ERowType.kEvent,
                                                                                    bActive,
                                                                                    eventType,
                                                                                    messageType,
                                                                                    targetName,
                                                                                    target,
                                                                                    priority,
                                                                                    time,
                                                                                    senderQuery,
                                                                                    destQuery);

         connectionProperties.TargetExists = targetExists;
         return connectionProperties;
      }

      private ConnectionEditorRowInfo BuildLinkConnectionProperties(XmlElement linkConnectionNav)
      {
         Guid target = new Guid(linkConnectionNav.GetAttribute(Helpers.kLinkEntry_target, ""));
         string targetName;
         bool targetExists = GUIHelpers.ResolveComponentIdToName(target, mPropertyEditorCallbacks.mRetrieveXmlDelegate, out targetName);

         float time = 0;
         {
            string timeString = linkConnectionNav.GetAttribute(Helpers.kLinkEntry_time, "");
            time = float.Parse(timeString);
         }

         bool bActive = false;
         {
            string activeString = linkConnectionNav.GetAttribute(Helpers.kLinkEntry_active, "");
            bActive = Tools.Common.Misc.ParseBool(activeString);
         }
         int priority = 0;
         {
            string priorityString = linkConnectionNav.GetAttribute(Helpers.kLinkEntry_priority, "");
            priority = int.Parse(priorityString);
         }
         string senderQuery = linkConnectionNav.GetAttribute(Helpers.kEventEntry_senderQuery, "");
         string destQuery = linkConnectionNav.GetAttribute(Helpers.kEventEntry_destQuery, "");

         XmlElement connectionParentNav = (XmlElement) linkConnectionNav.ParentNode;
         string linkType = connectionParentNav.GetAttribute(Helpers.kLink_type, "");

         string messageType = "Link";
         ConnectionEditorRowInfo connectionProperties = new ConnectionEditorRowInfo(linkConnectionNav,
                                                                                    ConnectionEditorRowInfo.ERowType.kLink,
                                                                                    bActive,
                                                                                    linkType,
                                                                                    messageType,
                                                                                    targetName,
                                                                                    target,
                                                                                    priority,
                                                                                    time,
                                                                                    senderQuery,
                                                                                    destQuery);

         connectionProperties.TargetExists = targetExists;
         return connectionProperties;
      }

      void ConnectionsDataGridView_CellEndEdit(object sender, DataGridViewCellEventArgs e)
      {
         // if update message properties is disabled, we don't flush changes here.
         if (mDisableUpdateMessagePropertiesOnSelectionChange)
            return;

         DataGridView connectionsDataGridView = sender as DataGridView;
         
         DataGridViewCellEventArgs args = e as DataGridViewCellEventArgs;
         int rowIndex = args.RowIndex;

         // Update xml so that it's sync'd with cell contents
         ConnectionEditorRowInfo editedConnection = mConnectionPropertiesContainer[rowIndex] as ConnectionEditorRowInfo;
         editedConnection.UpdateXml();

         // Notify editor that some connections have changed.
         if (mConnectionsChanged != null)
         {
            mConnectionsChanged(this);
         }
      }

      void ConnectionsDataGridView_SelectionChanged(object sender, EventArgs e)
      {
         Update_RemoveConnectionButtonStatus();
         if (!mDisableUpdateMessagePropertiesOnSelectionChange)
         {
            StoreMessagePropertiesGUI();
            UpdateMessagePropertiesGUI();
         }
      }

      void UpdateMessagePropertiesGUI()
      {
         mMessagePropertiesGUIEditedConnections.Clear();
         mMessagePropertiesGUISrcProperties.Clear();
         foreach (DataGridViewRow selectedRow in mConnectionsDataGridView.SelectedRows)
         {
            ConnectionEditorRowInfo rowProperties = selectedRow.DataBoundItem as ConnectionEditorRowInfo;
            switch(rowProperties.ConnectionType)
            {
               case ConnectionEditorRowInfo.ERowType.kEvent:
               case ConnectionEditorRowInfo.ERowType.kLink:
                  {
                     // Move to child which contains properties
                     XmlElement unmergedPropertiesNav = Common.XmlNodeHelpers.GetFirstChildElement( rowProperties.ConnectionElement );
                     // Build full property tree
                     XmlDocument mergedXmlDocument = XmlNodeHelpers.CreateDocumentFromNode(unmergedPropertiesNav);
                     if (mergedXmlDocument != null)
                     {
                        mMessagePropertiesGUIEditedConnections.Add(rowProperties);
                        mMessagePropertiesGUISrcProperties.Add(XmlNodeHelpers.GetDocumentRootElement( mergedXmlDocument ) );
                     }
                  }
                  break;
            }
         }
         // Remove previous controls
         Tools.Common.Misc.DestroyControls( mPropertyPanel.Controls );

         if (mMessagePropertiesGUISrcProperties.Count == 0)
         {
            // No properties, remove gui
            mMessagePropertiesGUI = null;
            return;
         }
         XmlDocument propertiesDoc = GroupProperties.BuildGroupedProperties(mMessagePropertiesGUISrcProperties, GroupProperties.EGroupOptions.kIgnoreTopName_IgnoreTopId);
         mMessagePropertiesGUI = new PropertyEditorGUI(mPropertyPanel);
         mMessagePropertiesGUI.mPropertyDB = mPropertySource;
         mMessagePropertiesGUI.mCallbacks = mPropertyEditorCallbacks;

         mMessagePropertiesGUI.Initialize(propertiesDoc);
      }

      public void StoreMessagePropertiesGUI()
      {
         if (mMessagePropertiesGUISrcProperties == null)
            return;

         if (mMessagePropertiesGUISrcProperties.Count == 0)
         {
            return;
         }

         // Write ungrouped properties back into tree
         GroupProperties.UngroupProperties( mMessagePropertiesGUI.Document, mMessagePropertiesGUISrcProperties, GroupProperties.EGroupOptions.kIgnoreTopName_IgnoreTopId );
         // Unmerge properties
         int srcPropertiesIndex = 0;
         foreach (ConnectionEditorRowInfo connectionProperty in mMessagePropertiesGUIEditedConnections)
         {
            XmlElement newConnectionData = mMessagePropertiesGUISrcProperties[srcPropertiesIndex];
            XmlElement propertiesNav = XmlNodeHelpers.GetFirstChildElement( connectionProperty.ConnectionElement );

            // Remove old properties and append new
            propertiesNav.ParentNode.ReplaceChild(
               propertiesNav.OwnerDocument.ImportNode( newConnectionData, true ),
               propertiesNav );

            srcPropertiesIndex++;
         }
      }

      public void ShowContextSensitiveHelp()
      {
         // if we're not visible clearly we shouldn't be showing any help
         if (Visible == false)
            return;

         Form topLevelForm = FindForm();

         Point screenPos = Cursor.Position;
         Control control = GUIHelpers.GetLeafControlForPosition(topLevelForm, ref screenPos);

         bool helpHandled = false;

         if (control != null)
         {
            ListView listView = control as ListView;
            if (listView != null)
            {
               Point clientPos = listView.PointToClient(screenPos);
               ListViewItem item = listView.GetItemAt(clientPos.X, clientPos.Y);
               if (item != null)
               {
                  ElementAndComponent tag = item.Tag as ElementAndComponent;
                  if (tag != null)
                  {
                     string component = tag.mComponentNav.SelectSingleNode("./@" + Helpers.kComponent_Id).Value;
                     string componentGroup = tag.mComponentNav.SelectSingleNode("./@" + Helpers.kComponent_Group).Value;

                     switch (tag.mNav.Name)
                     {
                        case Helpers.kElement_Message:
                           {
                              string messageType = tag.mNav.SelectSingleNode("./@" + Helpers.kMessage_type).Value;
                              Helpers.SpawnHelp("Component", componentGroup, component, "message_" + messageType);
                              helpHandled = true;
                           }
                           break;

                        case Helpers.kElement_Event:
                           {
                              string eventType = tag.mNav.SelectSingleNode("./@" + Helpers.kEvent_type).Value;
                              Helpers.SpawnHelp("Component", componentGroup, component, "event_" + eventType);
                              helpHandled = true;
                           }
                           break;

                        case Helpers.kElement_Link:
                           {
                              string linkType = tag.mNav.SelectSingleNode("./@" + Helpers.kLink_type).Value;
                              Helpers.SpawnHelp("Component", componentGroup, component, "link_" + linkType);
                              helpHandled = true;
                           }
                           break;
                     }
                  }
               }
            }
         }


         if (!helpHandled)
         {
            Helpers.SpawnHelp("", "", "", "");
         }
      }

      // Object header clicks are intercepted as cell clicks because those cells don't have any actual content.
      private void OnConnectionsCellClick(object sender, DataGridViewCellEventArgs e)
      {
         if (e.RowIndex == -1)
            return;

         DataGridViewRow row = mConnectionsDataGridView.Rows[e.RowIndex];
         ConnectionEditorRowInfo properties = row.DataBoundItem as ConnectionEditorRowInfo;

         switch (properties.ConnectionType)
         {
            case ConnectionEditorRowInfo.ERowType.kObjectHeader:
               {
                  if (mChangeObjectSelection != null)
                  {
                     mChangeObjectSelection(properties.DestinationId);
                  }
               }
               break;
         }
      }

      // Event/Link cell clicks are intercepted as cell CONTENT clicks because we want the click to be accurate to the text inside the cell.
      private void OnConnectionsCellContentClick(object sender, DataGridViewCellEventArgs e)
      {
         if (e.RowIndex == -1)
            return;

         DataGridViewRow row = mConnectionsDataGridView.Rows[e.RowIndex];
         ConnectionEditorRowInfo properties = row.DataBoundItem as ConnectionEditorRowInfo;

         switch(properties.ConnectionType)
         {
            case ConnectionEditorRowInfo.ERowType.kEvent:
            case ConnectionEditorRowInfo.ERowType.kLink:
               {
                  DataGridViewLinkColumn linkColumn = mConnectionsDataGridView.Columns[e.ColumnIndex] as DataGridViewLinkColumn;
                  if (linkColumn != null)
                  {
                     if (properties.Direction == ConnectionEditorRowInfo.EDirection.kOutgoing)
                     {
                        if (linkColumn.Name == "DestinationColumn")
                        {
                           if (mChangeObjectSelection != null)
                           {
                              mChangeObjectSelection(properties.DestinationId);
                           }
                        }
                        else if (!Helpers.IsEventOrLinkEntryPrefab(properties.ConnectionElement))
                        {
                           DataGridViewCell cell = row.Cells[e.ColumnIndex];

                           ContextMenuStrip menuStrip = new ContextMenuStrip();
                           menuStrip.ShowImageMargin = false;

                           switch (linkColumn.Name)
                           {
                              case "EventColumn":
                                 {
                                    XmlElement connectionNav = properties.ConnectionElement;

                                    // Get name of parent (link/event)
                                    string connectionType = connectionNav.ParentNode.Name;

                                    // Move up to container for type of current connection (either events or links)
                                    XmlElement cnGrandParent = (XmlElement)connectionNav.ParentNode.ParentNode;

                                    foreach (XmlElement eventNav in cnGrandParent.SelectNodes(connectionType))
                                    {
                                       ToolStripMenuItem item = menuStrip.Items.Add(eventNav.GetAttribute(Helpers.kEvent_type, ""), null, new EventHandler(OnChangeEvent)) as ToolStripMenuItem;
                                       item.Tag = properties;
                                    }
                                 }
                                 break;

                              case "MessageColumn":
                                 {
                                    // Can only change message if it's an event and the target exists.
                                    if (properties.ConnectionType == ConnectionEditorRowInfo.ERowType.kEvent && properties.TargetExists)
                                    {
                                       // Get navigator to event entry
                                       XmlElement eventEntryNav = properties.ConnectionElement;

                                       Guid targetComponentId = new Guid(eventEntryNav.GetAttribute(Helpers.kEventEntry_target, ""));

                                       XmlDocument targetObject = mPropertyEditorCallbacks.mRetrieveXmlDelegate(targetComponentId, true);
                                       XmlElement targetComponent = targetObject.SelectSingleNode(string.Format("//ComponentGroup[@editorId='{0}']/Component", targetComponentId)) as XmlElement;

                                       foreach (XmlAttribute attr in targetComponent.SelectNodes(string.Format(".//{0}/@{1}", Helpers.kElement_Message, Helpers.kMessage_type)))
                                       {
                                          ToolStripMenuItem item = menuStrip.Items.Add(attr.Value, null, new EventHandler(OnChangeMessage)) as ToolStripMenuItem;
                                          item.Tag = properties;
                                       }
                                    }
                                 }
                                 break;
                           }

                           if (menuStrip.Items.Count > 0)
                           {
                              menuStrip.Show(Cursor.Position);
                           }
                        }
                     }
                  }
               }
               break;
         }
      }

      private void OnChangeEvent(object sender, EventArgs args)
      {
         StoreMessagePropertiesGUI();

         ToolStripMenuItem menuItem = sender as ToolStripMenuItem;
         ConnectionEditorRowInfo properties = menuItem.Tag as ConnectionEditorRowInfo;

         // can only change message if this isn't a prefab event
         if (!Helpers.IsEventOrLinkEntryPrefab(properties.ConnectionElement))
         {
            foreach (XmlAttribute replaceNav in properties.ConnectionElement.SelectNodes("//MessageObject/@name"))
            {
               replaceNav.Value = menuItem.Text;
            }

            // Get connection type (link/event)
            string connectionType = properties.ConnectionElement.ParentNode.Name;

            // Get to container node for connection type
            XmlElement containerNode = (XmlElement) properties.ConnectionElement.ParentNode.ParentNode;

            // Selecting the new event to move the entry to
            XmlElement newDest = (XmlElement) containerNode.SelectSingleNode( string.Format( ".//{0}[@type='{1}']", connectionType, menuItem.Text ) );

            properties.ConnectionElement.ParentNode.RemoveChild( properties.ConnectionElement );
            newDest.AppendChild( properties.ConnectionElement );

            UpdateConnectionsListBox();

            // Notify
            if (mConnectionsChanged != null)
               mConnectionsChanged(this);
         }
      }

      private void OnChangeMessage(object sender, EventArgs args)
      {
         StoreMessagePropertiesGUI();

         ToolStripMenuItem menuItem = sender as ToolStripMenuItem;
         ConnectionEditorRowInfo properties = menuItem.Tag as ConnectionEditorRowInfo;

         XmlElement eventEntryElement = properties.ConnectionElement;

         // can only change message if this isn't a prefab event
         if (!Helpers.IsEventOrLinkEntryPrefab(eventEntryElement))
         {
            XmlElement messageObjectNode = (XmlElement) eventEntryElement["MessageObject"];

            // Unmerge message object
            MergePropertiesV2.Unmerge_Inplace(ref messageObjectNode);

            // Update id of message object.
            XmlElement messageObjectElement = messageObjectNode as XmlElement;
            messageObjectElement.SetAttribute("id", menuItem.Text);
            
            // Remerge message object
            MergePropertiesV2.Merge_Inplace(ref messageObjectNode, mPropertySource);

            UpdateConnectionsListBox();

            // Notify
            if (mConnectionsChanged != null)
               mConnectionsChanged(this);
         }
      }

      private void mConnectionsDataGridView_CellPainting(object sender, DataGridViewCellPaintingEventArgs e)
      {
         if (e.RowIndex != -1)
         {
            if (mConnectionsDataGridView.Columns[e.ColumnIndex].Name == "ActiveColumn")
            {
               DataGridViewRow row = mConnectionsDataGridView.Rows[e.RowIndex];
               if( row.Cells[e.ColumnIndex].ReadOnly )
               {
                  e.PaintBackground(e.ClipBounds, true);

                  bool isChecked = (bool)e.Value;
                  System.Windows.Forms.VisualStyles.CheckBoxState state = isChecked ? System.Windows.Forms.VisualStyles.CheckBoxState.CheckedDisabled : System.Windows.Forms.VisualStyles.CheckBoxState.UncheckedDisabled;
                  Size size = CheckBoxRenderer.GetGlyphSize(e.Graphics, state);
                  
                  Point position = e.CellBounds.Location;
                  position.X += 3;
                  position.Y += (e.CellBounds.Height - size.Height) - 3;

                  CheckBoxRenderer.DrawCheckBox(e.Graphics, position, state);

                  e.Handled = true;
               }
            }
         }
      }

      private void ObjectListView_DoubleClick(object sender, EventArgs e)
      {
         if( mAddConnectionButton.Enabled )
         {
            AddConnectionButton_Click(sender, e);
         }
      }
   }
}

