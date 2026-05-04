using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;
using Tools.Common.Perforce;
using System.Xml;

namespace CoreTools.Prefab.UI
{
   public partial class PrefabViewer : UserControl
   {
      static Guid kDefaultNoneViewId = new Guid("e1c759d6-1264-4381-aa86-ad8f1d17ba0b");
      static Guid kDefaultAllViewId = new Guid("8111535b-5e6a-457b-bb0b-695998cb277f");

      internal class PrefabView
      {
         internal enum EViewType
         {
            Default,
            Custom
         }

         internal enum EMatchType
         {
            Any,
            All,
            Exact
         }
         
         Guid           mId;
         List<string>   mIncludeTags;
         EMatchType     mMatchType;
         EViewType      mViewType;

         internal PrefabView(Guid id, List<string> includeTags, EMatchType matchType, EViewType viewType)
         {
            mId = id;
            mIncludeTags = includeTags;
            mMatchType = matchType;
            mViewType = viewType;
         }

         internal string DisplayName
         {
            get
            {
               string namePostFix = (mViewType == EViewType.Default) ? " (Default)" : "";

               if( mIncludeTags.Count > 0 )
               {
                  string displayName = "";

                  bool first = true;

                  foreach (string tag in mIncludeTags)
                  {
                     if (!first)
                        displayName += ", ";
                     else
                        first = false;

                     displayName += tag;
                  }

                  return displayName + namePostFix;
               }
               else
               {
                  switch( mMatchType )
                  {
                  case EMatchType.Any:
                  case EMatchType.All:
                     return "All" + namePostFix;

                  case EMatchType.Exact:
                     return "None" + namePostFix;

                  default:
                     return "[Unknown]";
                  }
               }
            }
         }

         internal Guid Id
         {
            get { return mId; }
         }

         internal List<string> IncludeTags
         {
            get
            {
               return new List<string>(mIncludeTags);
            }

            set
            {
               mIncludeTags = value;
            }
         }

         internal EMatchType MatchType { get { return mMatchType; } }
         internal EViewType ViewType { get { return mViewType;  } }

         internal bool IsMatch(Prefab prefab)
         {
            List<string> prefabTags = prefab.Tags;
            
            switch(mMatchType)
            {
            case EMatchType.Exact:
               {
                  if (mIncludeTags.Count != prefabTags.Count)
                     return false;

                  foreach (string tag in prefabTags)
                  {
                     if (!mIncludeTags.Contains(tag))
                     {
                        return false;
                     }
                  }
                  return true;
               }

            case EMatchType.All:
               {
                  foreach (string tag in mIncludeTags)
                  {
                     if (!prefabTags.Contains(tag))
                     {
                        return false;
                     }
                  }
                  return true;
               }

            case EMatchType.Any:
               {
                  bool match = (mIncludeTags.Count > 0) ? false : true;

                  foreach (string tag in prefabTags)
                  {
                     if (mIncludeTags.Contains(tag))
                     {
                        match = true;
                     }
                  }

                  return match;
               }
            default:
               return false;
            }
         }
      }

      public enum ObjectType
      {
         Object,
         Prefab,
         Invalid = -1
      }

      public delegate void BuildCustomContextMenuDelegate(ContextMenuStrip menu, TreeNode node, object nodeData);

      public event BuildCustomContextMenuDelegate mBuildCustomContextMenu;

      private Tools.Common.Perforce.Connection mPerforce;
      private PrefabManager mPrefabManager;
      
      private int mLastNodeHandle = -1;
      private ToolTip mToolTip = new ToolTip();

      private bool mShowCheckedOutOnly = false;
      private List<PrefabView> mViews = new List<PrefabView>();
      private List<Guid> mDeleteViews = new List<Guid>();

      public PrefabManager PrefabManager
      {
         get { return mPrefabManager; }
         set 
         { 
            mPrefabManager = value;
            if (mPrefabManager != null )
            {
               RebuildUI();
            }
         }
      }

      public PrefabViewer()
      {
         mPerforce = new Tools.Common.Perforce.Connection();

         String overridePort = System.Environment.GetEnvironmentVariable("BPE_DATA_P4PORT");
         if (overridePort != String.Empty)
         {
            mPerforce.Port = overridePort;
         }

         mPerforce.Connect();

         LoadViews();

         InitializeComponent();

         mToolTip.InitialDelay = 500;
         mToolTip.ReshowDelay = 0;
         mToolTip.UseFading = false;
         mToolTip.UseAnimation = false;

         viewTagControl.mGetTagDBDelegate = new TagControl.GetTagDBDelegate(GetViewTagDB);
         viewTagControl.mAddTagDelegate = new TagControl.TagChangeDelegate(AddViewTag);
         viewTagControl.mRemoveTagDelegate = new TagControl.TagChangeDelegate(RemoveViewTag);
         viewTagControl.mRenameTagDelegate = new TagControl.TagRenameDelegate(RenameViewTag);

         prefabTagControl.mGetTagDBDelegate = new TagControl.GetTagDBDelegate(GetTagDB);
         prefabTagControl.mAddTagDelegate = new TagControl.TagChangeDelegate(AddTag);
         prefabTagControl.mRemoveTagDelegate = new TagControl.TagChangeDelegate(RemoveTag);
         prefabTagControl.mRenameTagDelegate = new TagControl.TagRenameDelegate(RenameTag);
      }

      void OnTreeViewMouseMove(object sender, MouseEventArgs e)
      {
         TreeNode node = prefabTreeView.GetNodeAt(e.X, e.Y);
         if( node != null )
         {
            int currentNodeHandle = node.Handle.ToInt32();
            if( currentNodeHandle != mLastNodeHandle )
            {
               mLastNodeHandle = currentNodeHandle;

               mToolTip.Active = false;

               Prefab prefab = node.Tag as Prefab;
               if (prefab != null)
               {
                  string tooltipText = string.Format("Name: {0}", prefab.Name);

                  FileInfo fileInfo = mPerforce.GetFileInfo(prefab.FilePath);
                  if( fileInfo != null )
                  {
                     if (fileInfo.ExistsInDepot)
                     {
                        string user;
                        DateTime time;
                        fileInfo.GetRevisionInfo(fileInfo.LocalRevision, out user, out time);

                        tooltipText += string.Format("\nRevision: {1}/{2}\nChecked in by {3} at {4}", prefab.Name, fileInfo.LocalRevision, fileInfo.HeadRevision, user, time);
                     }
                     else
                     {
                        tooltipText += "\nNot under version control.";
                     }

                     mToolTip.SetToolTip(prefabTreeView, tooltipText);
                     mToolTip.Active = true;
                  }
               }
            }
         }
         else
         {
            mLastNodeHandle = -1;
            mToolTip.Active = false;
         }
      }

      public bool GetPlaceableTypeInfo(out ObjectType objectType, out string displayName, out object typeData)
      {
         if (prefabTreeView.SelectedNodes.Count == 1)
         {
            TreeNode selectedNode = prefabTreeView.SelectedNode;
            if( selectedNode.ImageKey.StartsWith("Object") )
            {
               objectType = ObjectType.Object;
               displayName = selectedNode.Text;
               typeData = displayName;
               return true;
            }
            else if( selectedNode.ImageKey.StartsWith("Prefab") )
            {
               objectType = ObjectType.Prefab;
               Prefab prefab = selectedNode.Tag as Prefab;
               displayName = prefab.Name;
               typeData = prefab;
               return true;
            }
         }

         objectType = ObjectType.Invalid;
         displayName = null;
         typeData = null;
         return false;
      }

      public void RebuildUI()
      {
         List<string> expandedState = new List<string>();
         GetExpandedState(prefabTreeView.Nodes, expandedState);

         List<String> selectedState = new List<string>();
         StoreSelectionState(prefabTreeView, selectedState);

         // clear selection before removing all nodes.
         prefabTreeView.SelectedNode = null;
         prefabTreeView.Nodes.Clear();

         prefabTreeView.BeginUpdate();
         
         AddScriptObjects();
         AddPrefabs();

         RestoreExpandedState(prefabTreeView.Nodes, expandedState);
         
         RestoreSelectionState(prefabTreeView, selectedState);

         UpdateSelectionBasedUI();

         prefabTreeView.EndUpdate();
      }

      private string GetViewsSettingsPath()
      {
         string folder = System.Environment.GetFolderPath(System.Environment.SpecialFolder.ApplicationData) + "\\BPEPrefabView";
         if (!System.IO.Directory.Exists(folder))
            System.IO.Directory.CreateDirectory(folder);

         return folder + "\\Views.xml";
      }

      private XmlDocument LoadViewsFromSettings()
      {
         XmlDocument doc = new XmlDocument();
         string filePath = GetViewsSettingsPath();
         
         if( System.IO.File.Exists(filePath) )
         {
            doc.Load(filePath);
         }
         else
         {
            XmlElement root = doc.CreateElement("Views");
            doc.AppendChild(root);
         }
         
         return doc;
      }

      private void LoadViews()
      {
         mViews.Clear();

         // No tags
         {
            List<string> includeTags = new List<string>();
            mViews.Add(new PrefabView(kDefaultNoneViewId, includeTags, PrefabView.EMatchType.Exact, PrefabView.EViewType.Default));
         }

         // Any tags
         {
            List<string> includeTags = new List<string>();
            mViews.Add(new PrefabView(kDefaultAllViewId, includeTags, PrefabView.EMatchType.Any, PrefabView.EViewType.Default));
         }

         XmlDocument doc = LoadViewsFromSettings();
         
         foreach(XmlElement view in doc.SelectNodes("//View"))
         {
            Guid viewId = new Guid(view.GetAttribute("id"));
            
            PrefabView.EMatchType matchType;

            try
            {
               matchType = (PrefabView.EMatchType)Enum.Parse(typeof(PrefabView.EMatchType), view.GetAttribute("matchType"));
            }
            catch(System.ArgumentException)
            {
               matchType = PrefabView.EMatchType.Any;
            }
            
            List<string> tags = new List<string>();
            foreach(XmlElement tag in view.SelectNodes(".//Tag"))
            {
               tags.Add(tag.InnerText);
            }

            mViews.Add(new PrefabView(viewId, tags, matchType, PrefabView.EViewType.Custom));
         }
      }

      private void SaveViews()
      {
         XmlDocument doc = LoadViewsFromSettings();

         XmlNode viewsElement = doc.SelectSingleNode("//Views");
         if( viewsElement != null )
         {
            foreach (Guid deletedViewId in mDeleteViews)
            {
               XmlElement deletedView = viewsElement.SelectSingleNode(string.Format("//View[@id='{0}']", deletedViewId)) as XmlElement;
               if (deletedView != null)
               {
                  deletedView.ParentNode.RemoveChild(deletedView);
               }
            }

            mDeleteViews.Clear();

            foreach (PrefabView view in mViews)
            {
               if (view.ViewType == PrefabView.EViewType.Custom)
               {
                  XmlElement viewElement = viewsElement.SelectSingleNode(string.Format("//View[@id='{0}']", view.Id)) as XmlElement;
                  if( viewElement == null )
                  {
                     viewElement = doc.CreateElement("View");
                     viewsElement.AppendChild(viewElement);
                  }
                  
                  viewElement.RemoveAll();

                  viewElement.SetAttribute("id", view.Id.ToString());
                  viewElement.SetAttribute("matchType", view.MatchType.ToString());

                  foreach(string tag in view.IncludeTags)
                  {
                     XmlElement tagElement = doc.CreateElement("Tag");
                     tagElement.InnerText = tag;
                     viewElement.AppendChild(tagElement);
                  }
               }
            }
         }

         doc.Save(GetViewsSettingsPath());
      }

      List<Tools.Controls.TreeNodeEx> AddViewNodes(List<PrefabView> views, Tools.Controls.TreeNodeEx parentNode)
      {
         List<Tools.Controls.TreeNodeEx> viewNodes = new List<Tools.Controls.TreeNodeEx>();

         foreach(PrefabView view in views)
         {
            Tools.Controls.TreeNodeEx viewNode = new Tools.Controls.TreeNodeEx();
            parentNode.Nodes.Add(viewNode);
            viewNode.Text = view.DisplayName;
            viewNode.Name = view.Id.ToString();
            viewNode.Tag = view;
            switch(view.ViewType)
            {
               case PrefabView.EViewType.Custom:
                  viewNode.ImageKey = "View";
                  break;
               
               case PrefabView.EViewType.Default:
                  viewNode.ImageKey = "Folder";
                  break;
            }

            viewNodes.Add(viewNode);
         }

         return viewNodes;
      }

      private void AddPrefabs()
      {
         foreach (PrefabRepository repository in mPrefabManager.Repositories)
         {
            Tools.Controls.TreeNodeEx repositoryNode = new Tools.Controls.TreeNodeEx();
            prefabTreeView.Nodes.Add(repositoryNode);
            repositoryNode.Text = string.Format("Prefabs ({0})", mPrefabManager.AssetManager.GetRepositoryRelativePath(repository.mRepositoryRoot));
            repositoryNode.Name = repositoryNode.FullPath;
            repositoryNode.Tag = repository;
            repositoryNode.ImageKey = "Folder";

            List<Tools.Controls.TreeNodeEx> viewNodes = AddViewNodes(mViews, repositoryNode);

            List<Prefab> allPrefabs = new List<Prefab>();

            foreach (Prefab prefab in repository.Prefabs)
            {
               allPrefabs.Add(prefab);
            }

            if (mPerforce.IsConnected)
            {
               List<Changelist> pendingChangelists = mPerforce.GetPendingChangelists(true);
               if (pendingChangelists != null)
               {
                  foreach (Changelist pending in pendingChangelists)
                  {
                     foreach (FileInfo file in pending.Files)
                     {
                        string path = file.LocalPath;
                        if (path.ToLower().EndsWith(".prefab") && file.Action == FileInfo.EAction.Delete)
                        {
                           string metaDataPath = PrefabManager.AssetManager.GetMetaDataSystemPath(path);
                           FileInfo metaDataFileInfo = mPerforce.GetFileInfo(metaDataPath);
                           string metaDataText = metaDataFileInfo.GetDataAsText();

                           System.Xml.XmlDocument metaData = new System.Xml.XmlDocument();
                           metaData.LoadXml(metaDataText);

                           Guid prefabId = new Guid(System.IO.Path.GetFileNameWithoutExtension(path));

                           Prefab prefab = new Prefab(repository, prefabId, metaData);

                           allPrefabs.Add(prefab);
                        }
                     }
                  }
               }
            }

            for( int viewIndex = 0; viewIndex < mViews.Count; ++viewIndex)
            {
               PrefabView view = mViews[viewIndex];
               Tools.Controls.TreeNodeEx viewNode = viewNodes[viewIndex];

               foreach (Prefab prefab in allPrefabs)
               {
                  if( view.IsMatch(prefab) )
                  {
                     Tools.Controls.TreeNodeEx prefabTreeNode = BuildTreeNodeForPrefab(prefab);

                     if( !mShowCheckedOutOnly || prefab.IsWriteable() )
                     {
                        viewNode.Nodes.Add(prefabTreeNode);
                     }
                  }
               }
            }

            repositoryNode.Expand();
         }
      }

      private Tools.Controls.TreeNodeEx BuildTreeNodeForPrefab(Prefab prefab)
      {
         Tools.Controls.TreeNodeEx prefabTreeNode = new Tools.Controls.TreeNodeEx();
         prefabTreeNode.Text = prefab.Name;
         prefabTreeNode.Name = prefab.Id.ToString();
         prefabTreeNode.Tag = prefab;
         prefabTreeNode.ImageKey = "Prefab";

         UpdateVersionControlStatus(prefabTreeNode);
         return prefabTreeNode;
      }

      private void AddScriptObjects()
      {
         Tools.Controls.TreeNodeEx node = new Tools.Controls.TreeNodeEx();
         prefabTreeView.Nodes.Add(node);

         node.Text = "Objects";
         node.ImageKey = "Folder";
         node.Name = node.FullPath;

         List<string> scriptObjects = mPrefabManager.PropertySource.GetAllPropertyContainers("ScriptObject");
         foreach(string scriptObject in scriptObjects)
         {
            Tools.Controls.TreeNodeEx scriptObjectNode = new Tools.Controls.TreeNodeEx();
            node.Nodes.Add(scriptObjectNode);
            scriptObjectNode.Text = scriptObject;
            scriptObjectNode.Name = scriptObjectNode.FullPath;
            scriptObjectNode.ImageKey = "Object";
         }
      }

      public void RefreshPrefabs()
      {
         mPrefabManager.InvalidatePrefabRepositories();
         LoadViews();

         RebuildUI();
      }

      private void OnRefresh(object sender, EventArgs e)
      {
         RefreshPrefabs();
      }

      private void OnRepositoryAdded(PrefabRepository repository)
      {
         RebuildUI();
      }

      private void OnRepositoryRemoved(PrefabRepository repository)
      {
         RebuildUI();
      }

      private void OnNodeMouseClick(object sender, TreeNodeMouseClickEventArgs e)
      {
         if ((e.Button & MouseButtons.Right) != 0)
         {
            mContextMenu.Items.Clear();

            PrefabRepository prefabRepository = e.Node.Tag as PrefabRepository;
            PrefabView prefabView = e.Node.Tag as PrefabView;
            
            if (prefabRepository != null || prefabView != null )
            {
               string createItemText = string.Format("Create new prefab{0}...", (prefabView != null) ? " inheriting tags from view" : "");
               ToolStripItem createItem = mContextMenu.Items.Add(createItemText, null, new EventHandler(OnCreatePrefab));
               createItem.Name = "CreatePrefab";
               createItem.Tag = e.Node;
            }

            Prefab prefab = e.Node.Tag as Prefab;
            if (prefab != null)
            {
               FileInfo fileInfo = null;

               bool connected = mPerforce.IsConnected;

               if (connected)
               {
                  fileInfo = mPerforce.GetFileInfo(prefab.FilePath);
               }

               bool hasMultiSelection = prefabTreeView.SelectedNodes.Count > 1;
               bool isWriteable = prefab.IsWriteable();

               bool canOpenInMaya;
               bool canRename;
               bool canDelete;

               if( !hasMultiSelection )
               {
                  if( fileInfo != null )
                  {
                     // Don't allow editing of files that are deleted.
                     canOpenInMaya = fileInfo.Action != FileInfo.EAction.Delete;
                     
                     // Can't rename files unless they are open for edit.
                     canRename = isWriteable;
                     
                     // Can only delete files if we're not already doing some other action to it.
                     canDelete = !fileInfo.ExistsInDepot || fileInfo.Action == FileInfo.EAction.None;
                  }
                  else
                  {
                     canRename = isWriteable;
                     canDelete = isWriteable;
                     canOpenInMaya = true;
                  }
               }
               else
               {
                  canOpenInMaya = false;
                  canRename = false;
                  canDelete = false;
               }

               ToolStripItem openInMayaItem = mContextMenu.Items.Add("Open in new Maya...", null, new EventHandler(OnOpenPrefabInMaya));
               openInMayaItem.Name = "OpenNewMaya";
               openInMayaItem.Enabled = canOpenInMaya;
               openInMayaItem.Tag = prefab;

               ToolStripItem renameItem = mContextMenu.Items.Add("Rename prefab", null, new EventHandler(OnRenamePrefab));
               renameItem.Name = "RenamePrefab";
               renameItem.Enabled = canRename;
               renameItem.Tag = e.Node;

               ToolStripItem deleteItem = mContextMenu.Items.Add("Delete prefab", null, new EventHandler(OnDeletePrefab));
               deleteItem.Enabled = canDelete;
               deleteItem.Tag = prefab;

               if (fileInfo != null)
               {
                  mContextMenu.Items.Add("-");

                  switch (fileInfo.Action)
                  {
                     case FileInfo.EAction.Unknown:
                        {
                           ToolStripItem addItem = mContextMenu.Items.Add("Open for Add", null, new EventHandler(OnAddOrEditPrefab));
                           addItem.Enabled = !hasMultiSelection;
                           addItem.Tag = prefab;
                        }
                        break;

                     case FileInfo.EAction.None:
                        {
                           ToolStripItem editItem = mContextMenu.Items.Add("Open for Edit", null, new EventHandler(OnAddOrEditPrefab));
                           editItem.Enabled = !hasMultiSelection;
                           editItem.Tag = prefab;
                        }
                        break;

                     case FileInfo.EAction.Add:
                     case FileInfo.EAction.Branch:
                     case FileInfo.EAction.Delete:
                     case FileInfo.EAction.Edit:
                     case FileInfo.EAction.Integrate:
                        {
                           ToolStripItem submitItem = mContextMenu.Items.Add("Submit...", null, new EventHandler(OnSubmitPrefab));
                           submitItem.Enabled =  !hasMultiSelection;
                           submitItem.Tag = prefab;
                        }
                        break;
                  }

                  mContextMenu.Items.Add("-");

                  ToolStripItem showHistoryItem = mContextMenu.Items.Add("Show History...", null, new EventHandler(OnShowHistory));
                  showHistoryItem.Enabled = fileInfo.ExistsInDepot && !hasMultiSelection;
                  showHistoryItem.Tag = prefab;

                  ToolStripItem showPeforceItem = mContextMenu.Items.Add("Show in Perforce...", null, new EventHandler(OnShowPerforce));
                  showPeforceItem.Enabled = fileInfo.ExistsInDepot && !hasMultiSelection;
                  showPeforceItem.Tag = prefab;

               }
            }

            if (mBuildCustomContextMenu != null)
            {
               mBuildCustomContextMenu(mContextMenu, e.Node, e.Node.Tag);
            }

            if( mContextMenu.Items.Count > 0 )
            {
               mContextMenu.Show(Cursor.Position);
            }
         }
      }

      void OnOpenPrefabInMaya(object sender, EventArgs e)
      {
         ToolStripItem menuItem = (ToolStripMenuItem)sender;
         Prefab prefab = (Prefab)menuItem.Tag;

         ProcessStartInfo info = new ProcessStartInfo();
         info.UseShellExecute = false;
         info.FileName = "maya.exe";

         if( prefab.HasData() )
         {
            string prefabMayaPath = prefab.FilePathMaya;
            info.Arguments = string.Format("-file \"{0}\"", prefabMayaPath);
         }
         else
         {
            string command = GetMayaMelCommandToCreatePrefab(prefab);
            command = command.Replace("\"", "\\\"");

            info.Arguments = string.Format("-command \"{0}\"", command);
         }
         
         Process proc = Process.Start(info);
         proc.WaitForInputIdle();
      }

      void OnRenamePrefab(object sender, EventArgs e)
      {
         ToolStripItem menuItem = (ToolStripMenuItem)sender;
         TreeNode node = (TreeNode)menuItem.Tag;
         prefabTreeView.LabelEdit = true;
         node.BeginEdit();
      }

      void OnDeletePrefab(object sender, EventArgs eventArgs)
      {
         ToolStripItem menuItem = (ToolStripMenuItem)sender;
         Prefab prefab = menuItem.Tag as Prefab;
         if (prefab != null)
         {
            if( mPerforce.IsConnected )
            {
               FileInfo info = mPerforce.GetFileInfo(prefab.FilePath);
               if( info.ExistsInDepot )
               {
                  DeletePrefabFromVersionControl(prefab);
               }
               else
               {
                  DeletePrefabLocal(prefab);
               }
            }
            else
            {
               DeletePrefabLocal(prefab);
            }
         }
      }

      private void DeletePrefabFromVersionControl(Prefab prefab)
      {
         Changelist pending = null;

         try
         {
            List<string> files = prefab.FilesForVersionControl;
            List<FileInfo> fileInfos = mPerforce.GetFileInfos(files);

            if (!mPerforce.AreFilesAtHeadRevision(fileInfos))
            {
               throw new Exception("Can't check out prefab for edit because it's not synced to the latest revision.");
            }

            pending = mPerforce.CreatePendingChangeList(string.Format("Prefab: {0}\nDeleted prefab\n", prefab.Name));

            foreach (string file in files)
            {
               if (!pending.Delete(file, true))
               {
                  throw new System.Exception(string.Format("Error while deleting file {0}", file));
               }
            }

            prefab.Repository.RemovePrefab(prefab);
         }
         catch (System.Exception e)
         {
            if (pending != null)
            {
               pending.Revert(false);
            }

            MessageBox.Show(e.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
         }
         finally
         {
            UpdateUIForPrefab(prefab, true);
            UpdateSelectionBasedUI();
         }
      }

      private void DeletePrefabLocal(Prefab prefab)
      {
         string message = string.Format("The prefab '{0}' is not under version control.\nDeleting it will result in it being moved to the recycle bin.\n\nAre you sure?", prefab.Name);
         if (MessageBox.Show(message, "Confirm Delete", MessageBoxButtons.YesNo, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button2) == DialogResult.Yes)
         {
            List<string> filesToDelete = new List<string>();

            foreach(string file in prefab.FilesForVersionControl)
            {
               if( System.IO.File.Exists(file) )
               {
                  filesToDelete.Add(file);
               }
            }

            if( filesToDelete.Count > 0 )
            {
               Tools.Common.FileUtils.RecycleFiles(filesToDelete);
            }

            prefab.Repository.RemovePrefab(prefab);

            foreach(TreeNode foundPrefabNode in FindPrefabTreeNodes(prefab, prefabTreeView.Nodes) )
            {
               foundPrefabNode.Remove();
            }
         }
      }

      void OnAddOrEditPrefab(object sender, EventArgs eventArgs)
      {
         ToolStripItem menuItem = (ToolStripMenuItem)sender;
         Prefab prefab = menuItem.Tag as Prefab;
         if (prefab != null)
         {
            Changelist pending = null;  

            try
            {
               List<string> files = prefab.FilesForVersionControl;
               List<FileInfo> fileInfos = mPerforce.GetFileInfos(files);
               
               if (!mPerforce.AreFilesAtHeadRevision(fileInfos))
               {
                  throw new Exception("Can't check out prefab for edit because it's not synced to the latest revision.");
               }

               foreach (string file in files)
               {
                  if (!System.IO.File.Exists(file))
                  {
                     throw new System.Exception(string.Format("Prefab not fully complete yet, you first have to edit and save it inside Maya."));
                  }
               }

               bool openExclusive = true;

               foreach(FileInfo file in fileInfos)
               {
                  if( file.CheckedOutBySomeoneElse )
                  {
                     if( MessageBox.Show("File is checked out by someone else.\n\nForce check out?", "Multiple Checkout Warning", MessageBoxButtons.YesNo, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button2) == DialogResult.Yes )
                     {
                        openExclusive = false;
                        break;
                     }
                     else
                     {
                        throw new Exception("Operation aborted because prefab is checked out by another user");
                     }
                  }
               }

               pending = mPerforce.CreatePendingChangeList(string.Format("Prefab: {0}\n\nChanges made:\n- ", prefab.Name));

               foreach (string file in files)
               {
                  if (!pending.Open(file, openExclusive))
                  {
                     throw new System.Exception(string.Format("Error while opening file {0}", file));
                  }
               }
            }
            catch (System.Exception e)
            {
               if (pending != null)
               {
                  pending.Revert(true);
               }

               MessageBox.Show(e.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            finally
            {
               UpdateUIForPrefab(prefab, true);
               UpdateSelectionBasedUI();
            }
         }
      }

      void OnSubmitPrefab(object sender, EventArgs e)
      {
         ToolStripItem menuItem = (ToolStripMenuItem)sender;

         Prefab prefab = menuItem.Tag as Prefab;
         if (prefab != null)
         {
            Changelist changelist = mPerforce.GetChangelistForPath(prefab.FilePath);
            
            // If the prefab is currently in the default change list, we move it to a new change list for the submit.
            if( changelist.IsDefaultChangelist )
            {
               changelist = mPerforce.CreatePendingChangeList(string.Format("Prefab: {0}\n\nChanges made:\n- ", prefab.Name));

               foreach (string file in prefab.FilesForVersionControl)
               {
                  changelist.Open(file, true);
               }
            }

            if( changelist != null )
            {
               ProcessStartInfo startInfo = new ProcessStartInfo();
               startInfo.UseShellExecute = false;
               startInfo.FileName = "p4win.exe";
               startInfo.Arguments = string.Format("-p {0} -S \"{1}\"", mPerforce.Port, changelist.Files[0].DepotPath);
               Process process = Process.Start(startInfo);
               process.WaitForExit();

               // At this point the user might have either submitted or updated the change list.
               // We can end up with files that have moved to the default change list (because they were unchanged)
               switch( changelist.Status )
               {
                  // If we're still pending at this point, we need to reopen all the files into the changelist
                  case Changelist.EStatus.Pending:
                     {
                        // After a submit attempt (update only) we move the files back into the changelist.
                        foreach (string file in prefab.FilesForVersionControl)
                        {
                           changelist.Reopen(file, true);
                        }
                     }
                     break;

                  // If the changelist was submitted the files that got moved to the default change list are unchanged and need to reverted
                  case Changelist.EStatus.Submitted:
                     foreach (string file in prefab.FilesForVersionControl)
                     {
                        // just to be on the save side we only revert UNCHANGED files.
                        mPerforce.Revert(file, true); 
                     }
                     break;
               }

               UpdateUIForPrefab(prefab, true);
               UpdateSelectionBasedUI();
            }
         }
      }

      void OnShowHistory(object sender, EventArgs e)
      {
         ToolStripItem menuItem = (ToolStripMenuItem)sender;

         Prefab prefab = menuItem.Tag as Prefab;
         if( prefab != null )
         {
            ProcessStartInfo startInfo = new ProcessStartInfo();
            startInfo.UseShellExecute = false;
            startInfo.FileName = "p4win.exe";
            startInfo.Arguments = string.Format("-p {0} -H {1}", mPerforce.Port, prefab.FilePath);
            Process.Start(startInfo);
         }
      }

      void OnShowPerforce(object sender, EventArgs e)
      {
         ToolStripItem menuItem = (ToolStripMenuItem)sender;

         Prefab prefab = menuItem.Tag as Prefab;
         if (prefab != null)
         {
            ProcessStartInfo startInfo = new ProcessStartInfo();
            startInfo.UseShellExecute = false;
            startInfo.FileName = "p4win.exe";
            startInfo.Arguments = string.Format("-p {0} -s {1}", mPerforce.Port, prefab.FilePath);
            Process.Start(startInfo);
         }
      }

      void OnCreatePrefab(object sender, EventArgs e)
      {
         ToolStripItem menuItem = (ToolStripMenuItem)sender;
         
         TreeNode node = menuItem.Tag as TreeNode;

         List<string> prefabTags = new List<string>();

         PrefabRepository repository = node.Tag as PrefabRepository;
         PrefabView view = node.Tag as PrefabView;
         
         if( view != null )
         {
            TreeNode repositoryNode = node;

            prefabTags = view.IncludeTags;
            while (repositoryNode != null)
            {
               repository = repositoryNode.Tag as PrefabRepository;
               if (repository != null)
                  break;

               repositoryNode = repositoryNode.Parent;
            }
         }

         if( repository != null )
         {
            Prefab prefab = Prefab.Create(repository, Guid.NewGuid(), "New Prefab");
            
            if (prefabTags.Count > 0 )
            {
               prefab.SetTags(prefabTags);
            }

            UpdateUIForPrefab(prefab, false);

            // Get list of all prefab tree nodes
            TreeNode[] foundPrefabTreeNodes = FindPrefabTreeNodes(prefab, node.Nodes);

            // Find the best one to select
            TreeNode foundPrefabTreeNode = FindBestTreeNodeForSelect(foundPrefabTreeNodes);
            
            if (foundPrefabTreeNode != null)
            {
               foundPrefabTreeNode.EnsureVisible();
               prefabTreeView.SelectedNode = foundPrefabTreeNode;

               prefabTreeView.LabelEdit = true;
               foundPrefabTreeNode.BeginEdit();
            }
         }
      }

      private TreeNode FindBestTreeNodeForSelect(TreeNode[] nodes)
      {
         // If any of them are visible , use that one
         foreach (TreeNode node in nodes)
         {
            if (IsTreeNodeVisible(node))
            {
               return node;
            }
         }

         // If we can't find one thats already visible, we use the first one.
         if (nodes.Length > 0)
         {
            return nodes[0];
         }

         return null;
      }

      /// <summary>
      /// This function determines if the passed in node is visible (all of the parents are expanded).
      /// </summary>
      private bool IsTreeNodeVisible(TreeNode node)
      {
         TreeNode parent = node.Parent;
         
         while( parent != null )
         {
            if (!parent.IsExpanded)
            {
               return false;
            }

            parent = parent.Parent;
         }

         return true;
      }

      private TreeNode GetNodeByPath(string path)
      {
         string[] pathElements = path.Split(';');

         TreeNodeCollection current = prefabTreeView.Nodes;
         
         for( int i = 0; i < pathElements.Length; ++i )
         {
            string nodeToFind = pathElements[i];

            bool didFind = false;

            foreach(TreeNode node in current)
            {
               if( node.Name == nodeToFind )
               {
                  if (i == (pathElements.Length - 1))
                  {
                     return node;
                  }
                  else
                  {
                     didFind = true;
                     current = node.Nodes;
                     break;
                  }
               }
            }
            
            if (!didFind)
               return null;
         }
         
         return null;
      }

      private string GetNodePath(TreeNode node)
      {
         string path = "";
         
         TreeNode current = node;
         
         while( current != null )
         {
            if( !string.IsNullOrEmpty(path) )
            {
               path = current.Name + ";" + path;
            }
            else
            {
               path = current.Name;
            }

            current = current.Parent;
         }

         return path;
      }

      private void GetExpandedState(TreeNodeCollection nodes, List<string> expandedNodes)
      {
         foreach(TreeNode node in nodes)
         {
            GetExpandedState(node.Nodes, expandedNodes);

            if( node.IsExpanded )
            {
               expandedNodes.Add(GetNodePath(node));
            }
         }
      }

      private void RestoreExpandedState(TreeNodeCollection nodes, List<string> expandedNodes)
      {
         foreach (string nodePath in expandedNodes)
         {
            TreeNode foundNode = GetNodeByPath(nodePath);
            if (foundNode != null)
            {
               foundNode.Expand();
            }
         }
      }

      private void StoreSelectionState(Tools.Controls.MultiSelectTreeView tree, List<string> selectedNodes)
      {
         foreach(TreeNode node in tree.SelectedNodes)
         {
            selectedNodes.Add(GetNodePath(node));
         }
      }

      private void RestoreSelectionState(Tools.Controls.MultiSelectTreeView tree, List<string> selectedNodes)
      {
         List<TreeNode> newSelection = new List<TreeNode>();
         
         foreach(string nodePath in selectedNodes)
         {
            TreeNode foundNode = GetNodeByPath(nodePath);
            
            if( foundNode != null )
            {
               newSelection.Add(foundNode);
            }
            else
            {
               string[] nodePathElements = nodePath.Split(';');
               string lastElement = nodePathElements[nodePathElements.Length - 1];

               TreeNode[] foundNodes = tree.Nodes.Find(lastElement, true);
               foundNode = FindBestTreeNodeForSelect(foundNodes);
               if( foundNode != null )
               {
                  foundNode.EnsureVisible();
                  newSelection.Add(foundNode);
               }
            }
         }

         tree.SelectedNodes = newSelection;
      }

      private void OnAfterLabelEdit(object sender, NodeLabelEditEventArgs e)
      {
         if( e.Label != null )
         {
            TreeNode node = e.Node;
            Prefab prefab = node.Tag as Prefab;
            
            if( prefab != null )
            {
               // Turn off label edit at this point to prevent bad stuff happening because the update UI call below removes the nodes we're currently editing.
               prefabTreeView.LabelEdit = false;

               if (prefab.Rename(e.Label))
               {
                  UpdateUIForPrefab(prefab, true);
               }
            }
         }
      }

      public static string GetMayaMelCommandToCreatePrefab(Prefab prefab)
      {
         return string.Format("mtGameExportLoadPlugin; file -f -new; file -rename \"{0}\"; file -f -save -type \"BPEPrefabMayaAscii\"", prefab.FilePathMaya);
      }

      public void UpdateVersionControlStatus(Tools.Controls.TreeNodeEx node)
      {
         Prefab prefab = node.Tag as Prefab;
         if (prefab != null)
         {
            List<FileInfo> fileInfos = mPerforce.GetFileInfos(prefab.GetPerforcePathForAllFiles());
            if( fileInfos != null && fileInfos.Count > 0 )
            {
               node.ForeColor = mPerforce.AreFilesAtHeadRevision(fileInfos) ? SystemColors.WindowText : System.Drawing.Color.Red;

               FileInfo fileInfo = fileInfos[0];

               if (fileInfo != null)
               {
                  if (fileInfo.CheckedOutByMe)
                  {
                     switch (fileInfo.Action)
                     {
                        case FileInfo.EAction.Add:
                           node.StatusImageKey = "add";
                           break;

                        case FileInfo.EAction.Edit:
                           node.StatusImageKey = "edit";
                           break;

                        case FileInfo.EAction.Delete:
                           node.StatusImageKey = "delete";
                           break;

                        default:
                           node.StatusImageKey = "locked";
                           break;
                     }
                  }
                  else if (fileInfo.CheckedOutBySomeoneElse)
                  {
                     node.StatusImageKey = "otherCheckout";
                  }
                  else if (fileInfo.ExistsInDepot)
                  {
                     node.StatusImageKey = "locked";
                  }
                  else
                  {
                     node.StatusImageKey = "";
                  }
               }
            }
         }
      }

      private void OnShowCheckedOutPrefabsToggle(object sender, EventArgs e)
      {
         mShowCheckedOutOnly = !mShowCheckedOutOnly;
         showCheckedOutPrefabsButton.Checked = mShowCheckedOutOnly;
         RebuildUI();
      }

      private void OnAfterSelect(object sender, TreeViewEventArgs e)
      {
         UpdateSelectionBasedUI();
      }

      private void UpdateSelectionBasedUI()
      {
         PrefabView view = GetCurrentView();
         if( view == null || (view.ViewType != PrefabView.EViewType.Custom))
         {
            deleteViewToolStripButton.Enabled = false;
         }
         else
         {
            deleteViewToolStripButton.Enabled = true;
         }

         UpdateTagControls();
      }

      private void UpdateTagControls()
      {
         // Update view tag control
         {
            PrefabView view = GetCurrentView();

            if (view != null)
            {
               viewTagControl.mCanAddNew = false;

               bool isEnabled = false;

               switch (view.ViewType)
               {
                  case PrefabView.EViewType.Custom:
                     isEnabled = true;
                     break;

                  case PrefabView.EViewType.Default:
                     isEnabled = false;
                     break;
               }

               viewTagControl.SetState(view.IncludeTags, isEnabled);
            }
            else
            {
               viewTagControl.SetState(new List<string>(), false);
            }
         }

         // Update prefab tag control
         {
            prefabTagControl.mCanAddNew = true;

            int prefabCount = 0;

            bool canEditTags = false;

            Dictionary<string, int> tags = new Dictionary<string, int>();

            foreach (TreeNode node in prefabTreeView.SelectedNodes)
            {
               Prefab prefab = node.Tag as Prefab;
               if (prefab != null)
               {
                  prefabCount++;

                  if (prefab.IsWriteable())
                  {
                     canEditTags = true;
                  }

                  foreach (string tag in prefab.Tags)
                  {
                     int count = 0;
                     tags.TryGetValue(tag, out count);

                     count++;

                     tags[tag] = count;
                  }
               }
            }

            // Get list of tags, only use tags that are used by all prefabs.
            List<string> tagList = new List<string>();
            foreach (KeyValuePair<string, int> tagEntry in tags)
            {
               if (tagEntry.Value == prefabCount)
               {
                  tagList.Add(tagEntry.Key);
               }
            }

            prefabSplitContainer.Panel2Collapsed = (prefabCount == 0);

            prefabTagControl.SetState(tagList, canEditTags);
         }
      }

      private List<string> GetTagDB()
      {
         List<string> tagDB = new List<string>();
         
         foreach(PrefabRepository repository in PrefabManager.Repositories)
         {
            foreach(Prefab prefab in repository.Prefabs)
            {
               foreach(string tag in prefab.Tags)
               {
                  if( !tagDB.Contains(tag) )
                  {
                     tagDB.Add(tag);
                  }
               }
            }
         }

         foreach(PrefabView view in mViews)
         {
            foreach (string tag in view.IncludeTags)
            {
               if (!tagDB.Contains(tag))
               {
                  tagDB.Add(tag);
               }
            }
         }
         
         return tagDB;
      }

      private void AddTag(string tag)
      {
         ModifyTagsOnSelectedPrefabs(string.Empty, tag);
      }
      
      private void RemoveTag(string tag)
      {
         ModifyTagsOnSelectedPrefabs(tag, string.Empty);
      }

      private void RenameTag(string oldTag, string newTag)
      {
         ModifyTagsOnSelectedPrefabs(oldTag, newTag);
      }

      private void ModifyTagsOnSelectedPrefabs(string tagToRemove, string tagToAdd)
      {
         List<Prefab> prefabsThatNeedUIUpdate = new List<Prefab>();

         foreach (TreeNode node in prefabTreeView.SelectedNodes)
         {
            Prefab prefab = node.Tag as Prefab;
            if (prefab != null)
            {
               if (prefab.IsWriteable())
               {
                  prefabsThatNeedUIUpdate.Add(prefab);

                  List<string> tags = prefab.Tags;

                  if (!string.IsNullOrEmpty(tagToRemove))
                     tags.Remove(tagToRemove);
                  
                  if (!string.IsNullOrEmpty(tagToAdd))
                     tags.Add(tagToAdd);

                  tags.Sort();

                  prefab.SetTags(tags);
               }
            }
         }

         UpdateUIForPrefabs(prefabsThatNeedUIUpdate, true);
      }

      private void UpdateUIForPrefabs(List<Prefab> prefabs, bool keepSelection)
      {
         prefabTreeView.BeginUpdate();

         List<string> selectedNodes = new List<string>();

         if (keepSelection)
         {
            StoreSelectionState(prefabTreeView, selectedNodes);
         }

         foreach (Prefab prefab in prefabs)
         {
            UpdateUIForPrefabInternal(prefab);
         }

         if (keepSelection)
         {
            RestoreSelectionState(prefabTreeView, selectedNodes);
         }
         
         prefabTreeView.EndUpdate();
      }

      private void UpdateUIForPrefab(Prefab prefab, bool keepSelection)
      {
         prefabTreeView.BeginUpdate();

         List<string> selectedNodes = new List<string>();

         if (keepSelection)
         {
            StoreSelectionState(prefabTreeView, selectedNodes);
         }

         UpdateUIForPrefabInternal(prefab);

         if (keepSelection)
         {
            RestoreSelectionState(prefabTreeView, selectedNodes);
         }

         prefabTreeView.EndUpdate();
      }

      private void UpdateUIForPrefabInternal(Prefab prefab)
      {
         Tools.Controls.TreeNodeEx prefabTreeNode = BuildTreeNodeForPrefab(prefab);

         foreach (PrefabView view in mViews)
         {
            TreeNode[] viewNodes = prefabTreeView.Nodes.Find(view.Id.ToString(), true);
            if (viewNodes.Length > 0)
            {
               TreeNode viewTreeNode = viewNodes[0];

               TreeNode[] foundPrefabTreeNodes = FindPrefabTreeNodes(prefab, viewTreeNode.Nodes);

               if (view.IsMatch(prefab))
               {
                  if (foundPrefabTreeNodes.Length == 0)
                  {
                     Tools.Controls.TreeNodeEx node = prefabTreeNode.Clone() as Tools.Controls.TreeNodeEx;
                     viewTreeNode.Nodes.Add(node);
                  }
                  else
                  {
                     foreach (TreeNode foundNode in foundPrefabTreeNodes)
                     {
                        // Replace node in tree with our new node
                        int foundIndex = viewTreeNode.Nodes.IndexOf(foundNode);
                        viewTreeNode.Nodes.RemoveAt(foundIndex);
                        Tools.Controls.TreeNodeEx node = prefabTreeNode.Clone() as Tools.Controls.TreeNodeEx;
                        viewTreeNode.Nodes.Insert(foundIndex, node);
                     }
                  }
               }
               else
               {
                  if (foundPrefabTreeNodes.Length != 0)
                  {
                     foreach (TreeNode foundNode in foundPrefabTreeNodes)
                     {
                        foundNode.Remove();
                     }
                  }
               }
            }
         }
      }

      private TreeNode[] FindPrefabTreeNodes(Prefab prefab, TreeNodeCollection root)
      {
         return root.Find(prefab.Id.ToString(), true);
      }

      private List<string> GetViewTagDB()
      {
         PrefabView view = GetCurrentView();

         bool viewHasTags = (view.IncludeTags.Count > 0);

         List<string> tagDB = new List<string>();
         foreach (PrefabRepository repository in PrefabManager.Repositories)
         {
            foreach (Prefab prefab in repository.Prefabs)
            {
               List<string> prefabTags = prefab.Tags;

               bool prefabSharesViewTags = false;

               if( viewHasTags )
               {
                  foreach (string viewTag in view.IncludeTags)
                  {
                     if (prefabTags.Contains(viewTag))
                     {
                        prefabSharesViewTags = true;
                        break;
                     }
                  }
               }
               else
               {
                  prefabSharesViewTags = true;
               }

               if( prefabSharesViewTags )
               {
                  foreach (string tag in prefabTags)
                  {
                     if (!tagDB.Contains(tag))
                     {
                        tagDB.Add(tag);
                     }
                  }
               }
            }
         }

         return tagDB;
      }

      private void AddViewTag(string tag)
      {
         ModifyTagsOnCurrentView(string.Empty, tag);
      }

      private void RemoveViewTag(string tag)
      {
         ModifyTagsOnCurrentView(tag, string.Empty);
      }

      private void RenameViewTag(string oldTag, string newTag)
      {
         ModifyTagsOnCurrentView(oldTag, newTag);
      }

      private void ModifyTagsOnCurrentView(string tagToRemove, string tagToAdd)
      {
         PrefabView view = GetCurrentView();

         if (view != null)
         {
            List<string> tags = view.IncludeTags;

            if (!string.IsNullOrEmpty(tagToRemove))
               tags.Remove(tagToRemove);

            if (!string.IsNullOrEmpty(tagToAdd))
               tags.Add(tagToAdd);

            tags.Sort();
            view.IncludeTags = tags;

            RebuildUI();
         }

         SaveViews();
      }

      private PrefabView GetCurrentView()
      {
         TreeNode node = prefabTreeView.SelectedNode;
         return GetViewForNode(node);
      }

      private static PrefabView GetViewForNode(TreeNode node)
      {
         while (node != null)
         {
            PrefabView view = node.Tag as PrefabView;

            if (view != null)
            {
               return view;
            }

            node = node.Parent;
         }

         return null;

      }

      private void OnAddView(object sender, EventArgs e)
      {
         List<string> includeTags = new List<string>();

         PrefabView view = new PrefabView(Guid.NewGuid(), includeTags, PrefabView.EMatchType.All, PrefabView.EViewType.Custom);
         mViews.Add(view);
         SaveViews();
         RebuildUI();

         TreeNode[] foundViews = prefabTreeView.Nodes.Find(view.Id.ToString(), true);
         TreeNode foundView = FindBestTreeNodeForSelect(foundViews);
         if( foundView != null )
         {
            prefabTreeView.SelectedNode = foundView;
            foundView.Expand();
         }
      }

      private void OnDeleteCurrentView(object sender, EventArgs e)
      {
         PrefabView view = GetCurrentView();
         if( view != null && view.ViewType == PrefabView.EViewType.Custom)
         {
            mDeleteViews.Add(view.Id);
            mViews.Remove(view);
            SaveViews();
            
            TreeNode[] viewNodes = prefabTreeView.Nodes.Find(view.Id.ToString(), true);
            
            foreach(TreeNode viewNode in viewNodes)
            {
               viewNode.Remove();
            }
            
            UpdateSelectionBasedUI();
         }
      }
   }
}
