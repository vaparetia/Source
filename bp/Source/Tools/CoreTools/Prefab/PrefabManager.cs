using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using Tools.Property.Source;
using Tools.Common;
using Tools.Property.Build;
using Tools.Property.Misc;
using System.IO;
using System.Diagnostics;

namespace CoreTools.Prefab
{
   public class PrefabRepository
   {
      internal PrefabManager              mPrefabManager;
      internal Tools.AssetSystem.Manager  mAssetManager;
      internal IPropertyDataSource        mPropertySource;
      internal string                     mRepositoryRoot;

      Dictionary<Guid, Prefab>            mPrefabs;

      public List<Prefab> Prefabs
      {
         get { return new List<Prefab>(mPrefabs.Values); }
      }

      public PrefabRepository(PrefabManager prefabManager, Tools.AssetSystem.Manager assetManager, IPropertyDataSource propertySource, string repositoryRoot)
      {
         if( !Directory.Exists(repositoryRoot) )
         {
            Directory.CreateDirectory(repositoryRoot);
         }

         mPrefabManager = prefabManager;
         mAssetManager = assetManager;
         mPropertySource = propertySource;
         mRepositoryRoot = repositoryRoot;
         InvalidateRepository();
      }

      internal void AddPrefab(Prefab prefab)
      {
         mPrefabs.Add(prefab.Id, prefab);
      }

      public void RemovePrefab(Prefab prefab)
      {
         mPrefabs.Remove(prefab.Id);
      }

      public Prefab FindPrefab(Guid id)
      {
         Prefab prefab;
         
         if (mPrefabs.TryGetValue(id, out prefab))
            return prefab;
         
         return prefab;
      }

      private void LoadPrefabs()
      {
         mPrefabs = new Dictionary<Guid, Prefab>();

         if (Directory.Exists(mRepositoryRoot) )
         {
            foreach (string filePath in Directory.GetFiles(mRepositoryRoot, "*.prefab.meta", SearchOption.TopDirectoryOnly))
            {
               string fileName = Path.GetFileName(filePath);
               string idString = fileName.Substring(0, fileName.IndexOf('.'));

               Guid id = new Guid(idString);

               Prefab prefab = Prefab.LoadPrefab(this, id);
               
               if( prefab != null )
               {
                  mPrefabs.Add(id, prefab);
               }
            }
         }
      }

      public void InvalidateMergedPrefabs()
      {
         foreach (Prefab prefab in mPrefabs.Values)
         {
            prefab.Invalidate();
         }
      }

      public void InvalidateRepository()
      {
         LoadPrefabs();
      }
   }

   public class PrefabManager
   {
      Tools.AssetSystem.Manager mAssetManager;

      public Tools.AssetSystem.Manager AssetManager
      {
         get { return mAssetManager; }
      }

      IPropertyDataSource mPropertySource;
      public IPropertyDataSource PropertySource
      {
         get { return mPropertySource; }
      }

      private List<PrefabRepository> mRepositories;
      public List<PrefabRepository> Repositories
      {
         get { return mRepositories; }
      }

      public delegate void RepositoryActionDelegate(PrefabRepository repository);

      public PrefabManager(Tools.AssetSystem.Manager assetManager, IPropertyDataSource propertySource)
      {
         mAssetManager = assetManager;
         mPropertySource = propertySource;
         mRepositories = new List<PrefabRepository>();
      }

      public void RemoveAllRepositories()
      {
         while(mRepositories.Count > 0 )
         {
            PrefabRepository repository = mRepositories[0];
            mRepositories.RemoveAt(0);
         }
      }

      public void AddPrefabRepository(string folder)
      {
         string folderSystemPath = mAssetManager.GetSystemPath(folder);
         PrefabRepository prefabRepository = new PrefabRepository(this, mAssetManager, mPropertySource, folderSystemPath);
         mRepositories.Add(prefabRepository);
      }

      public void AddPrefabRepositoryForScriptFile(string scriptFile)
      {
         string workspacePath = Tools.Common.WorkspaceHelper.GetWorkspaceForPath(scriptFile, mAssetManager);

         if( !string.IsNullOrEmpty(workspacePath) && mAssetManager.IsSystemPathInsideRepository(workspacePath) )
         {
            string prefabsSystemPath = Path.Combine(Path.GetDirectoryName(workspacePath), "Prefabs");

            string prefabsAssetPath = mAssetManager.GetRepositoryRelativePath(prefabsSystemPath);
            AddPrefabRepository(prefabsAssetPath);
         }
      }

      public void InvalidateMergedPrefabs()
      {
         foreach(PrefabRepository repository in mRepositories)
         {
            repository.InvalidateMergedPrefabs();
         }
      }

      public void InvalidatePrefabRepositories()
      {
         foreach(PrefabRepository repository in mRepositories)
         {
            repository.InvalidateRepository();
         }
      }

      public Prefab GetPrefabById(Guid id)
      {
         Prefab prefab = null;

         foreach(PrefabRepository repository in mRepositories)
         {
            prefab = repository.FindPrefab(id);
            
            if (prefab != null)
               break;
         }

         return prefab;
      }

      public bool MergePrefab_Inplace(XmlDocument instanceDocument)
      {
         XmlElement prefabNodeElement = XmlNodeHelpers.GetDocumentRootElement( instanceDocument );
         if( prefabNodeElement != null )
         {
            return MergePrefab_Inplace(prefabNodeElement);
         }
         
         return false;
      }

      /// <summary>
      /// This function merges a prefab instance.
      /// </summary>
      public bool MergePrefab_Inplace(XmlElement prefabNodeElement)
      {
         XmlElement prefabDataElement = prefabNodeElement["PrefabData"];
         XmlElement prefabElement = prefabDataElement["AddPrefab"];

         if (prefabElement != null)
         {
            Open3d.DagNode prefabNode = Open3d.DagNode.Attach(prefabNodeElement);
            Open3d.Transform prefabTransform = prefabNode.GetTransform();

            Guid id = new Guid(prefabElement.GetAttribute("id"));

            Prefab prefab = GetPrefabById(id);
            if (prefab != null)
            {
               XmlElement mergedPrefab = prefab.MergedPrefab;

               // Build lookup of prefab editor id to nodes, this is used to reorder the nodes.
               Dictionary<Guid, XmlElement> prefabEditorIdToNode = new Dictionary<Guid, XmlElement>();
               {
                  foreach (XmlElement node in prefabElement.SelectNodes(".//UsingNode"))
                  {
                     Guid nodePrefabId = GetNodePrefabId(node);

                     prefabEditorIdToNode.Add(nodePrefabId, node);
                  }

                  foreach(XmlElement node in prefabEditorIdToNode.Values)
                  {
                     node.ParentNode.RemoveChild(node);
                  }
               }

               MergePrefabNode(ref prefabElement, mergedPrefab, mPropertySource, prefabEditorIdToNode);

               // Walk over all nodes and transform them into world space
               {
                  XmlNodeHelpers.DepthIterator iter = new XmlNodeHelpers.DepthIterator(prefabElement);
                  while (iter.MoveToNextElement())
                  {
                     if (iter.IteratorInfo == XmlNodeHelpers.DepthIterator.EIteratorInfo.kII_EnteredElement && iter.Element.Name == "Node")
                     {
                        Open3d.DagNode childNode = Open3d.DagNode.Attach(iter.Element);
                        Open3d.Transform childTransform = childNode.GetTransform();
                        childTransform.PreMultiplyInplace(prefabTransform);
                     }
                  }
               }
            }
            else
            {
               return false;
            }
         }

         // Now remove any remaining UsingNode elements, because those are just overrides for nodes that no longer exist in the prefab.
         {
            XmlNodeHelpers.DepthIterator iter = new XmlNodeHelpers.DepthIterator(prefabElement);

            while (iter.MoveToNextElement())
            {
               if (iter.IteratorInfo == XmlNodeHelpers.DepthIterator.EIteratorInfo.kII_EnteredElement)
               {
                  switch (iter.Element.Name)
                  {
                     case "UsingNode":
                        {
                           iter.RemoveCurrentElementAndChildren();
                        }
                        break;
                  }
               }
            }
         }

         // Remap all editor ids for the prefab instance.
         Dictionary<Guid, Guid> editorIdRemapping = new Dictionary<Guid, Guid>();
         AssignOrGetEditorIds(prefabElement, editorIdRemapping);
         RemapConnections(prefabElement, editorIdRemapping);
         
         return true;
      }

      public void UnmergePrefab_Inplace(XmlDocument instance)
      {
         XmlElement prefabRootNode = XmlNodeHelpers.GetDocumentRootElement( instance );
         XmlElement prefabDataElement = prefabRootNode["PrefabData"];
         XmlElement prefabElement = prefabDataElement["Prefab"];

         XmlNodeHelpers.DepthIterator iter = new XmlNodeHelpers.DepthIterator(prefabElement);
         while(iter.MoveToNextElement())
         {
            if(iter.IteratorInfo == XmlNodeHelpers.DepthIterator.EIteratorInfo.kII_EnteredElement)
            {
               switch(iter.Element.Name)
               {
                  case "Node":
                     {
                        bool isPrefabNode = iter.Element["PrefabData"] != null;
                        if( !isPrefabNode )
                        {
                           iter.RenameCurrentElement("UsingNode");
                           XmlElement transformElement = iter.Element["Transform"];
                           iter.Element.RemoveChild(transformElement);
                        }
                     }
                     break;

                  case "Prefab":
                     {
                        iter.RenameCurrentElement("AddPrefab");
                     }
                     break;

                  case "PropertyContainer":
                     {
                        XmlNodeHelpers.DepthIterator connectionIter = new XmlNodeHelpers.DepthIterator(iter.Element);
                        while( connectionIter.MoveToNextElement() )
                        {
                           if( connectionIter.IteratorInfo == XmlNodeHelpers.DepthIterator.EIteratorInfo.kII_EnteredElement)
                           {
                              switch(connectionIter.Element.Name)
                              {
                                 case "EventEntry":
                                 case "LinkEntry":
                                    {
                                       if( Helpers.IsEventOrLinkEntryPrefab(connectionIter.Element) )
                                       {
                                          PrefabHelper.RemoveDefaultAttributes(connectionIter.Element);
                                       }
                                    }
                                    break;
                              }
                           }
                        }

                        XmlElement propertyContainer = iter.Element;
                        MergePropertiesV2.Unmerge_Inplace(ref propertyContainer);
                        iter.SetCurrentElement(propertyContainer);
                        iter.SkipChildren();
                     }
                     break;
               }
            }
         }
      }

      /// <summary>
      /// Injecting of a prefab is a process by which the pure merged data of a prefab instance (which lives underneath the PrefabData element of the prefab root "Node")
      /// is transferred to the actual child nodes of that prefab.
      /// This is necessary because while the hierarchy of the prefab itself is fixed, the prefab instance can have additional nodes in the hierarchy.
      /// NOTE: This is a one way process, it is executed during cooking to prepare the Open3d document for writing of property containers as well
      /// as by prefabs itself to support prefabs within prefabs (by this process prefabs are flattened into the owner prefab).
      /// </summary>
      public void InjectPrefab(XmlDocument prefabRoot)
      {
         InjectPrefab( XmlNodeHelpers.GetDocumentRootElement( prefabRoot ) );
      }

      public void InjectPrefab(XmlElement prefabRoot)
      {
         XmlElement prefabData = prefabRoot["PrefabData"];
         XmlElement prefabElement = prefabData["Prefab"];

         InjectPrefabChildren(prefabRoot, prefabElement, prefabRoot);
         prefabRoot.RemoveChild(prefabData);
      }

      private void InjectPrefabChildren(XmlElement destOwner, XmlElement sourceOwner, XmlElement prefabRoot)
      {
         XmlDocument ownerDocument = destOwner.OwnerDocument;

         foreach(XmlNode sourceChild in sourceOwner.ChildNodes)
         {
            XmlElement sourceChildElement = sourceChild as XmlElement;
            if (sourceChildElement != null && sourceChildElement.Name == "Node")
            {
               Guid nodeInstanceId = GetNodeId(sourceChildElement);
               string nodeName = sourceChildElement.GetAttribute("name");

               string query = string.Format(".//Node[@prefabInstanceId='{0}']", nodeInstanceId);
               XmlElement destChildElement = prefabRoot.SelectSingleNode(query) as XmlElement;

               if (destChildElement != null)
               {
                  destChildElement.ParentNode.RemoveChild(destChildElement);
                  destOwner.AppendChild(destChildElement);
               }
               else
               {
                  destChildElement = ownerDocument.CreateElement("Node");
                  destOwner.AppendChild(destChildElement);
               }

               destChildElement.SetAttribute("name", nodeName);

               // Update transform of target node (unless it already has one)
               {
                  XmlElement transformElement = destChildElement["Transform"];

                  if (transformElement == null)
                  {
                     transformElement = ownerDocument.CreateElement("Transform");
                     destChildElement.AppendChild(transformElement);
                     transformElement.InnerXml = sourceChildElement["Transform"].InnerXml;
                  }
               }

               // Update properties of target node
               {
                  XmlElement propertiesElement = destChildElement["Properties"];
                  if (propertiesElement == null)
                  {
                     propertiesElement = ownerDocument.CreateElement("Properties");
                     destChildElement.AppendChild(propertiesElement);
                  }

                  propertiesElement.InnerXml = sourceChildElement["Properties"].InnerXml;
               }

               InjectPrefabChildren(destChildElement, sourceChildElement, prefabRoot);
            }
         }
      }

      public static Guid GetNodeId(XmlElement element)
      {
         XmlAttribute editorIdAttribute = element.SelectSingleNode("./Properties/*/@editorId") as XmlAttribute;
         if (editorIdAttribute != null)
         {
            return new Guid(editorIdAttribute.Value);
         }
         else
         {
            return Guid.Empty;
         }
      }

      public static Guid GetNodePrefabId(XmlElement element)
      {
         XmlAttribute editorIdAttribute = element.SelectSingleNode("./Properties/*/@prefabEditorId") as XmlAttribute;
         if (editorIdAttribute != null)
         {
            return new Guid(editorIdAttribute.Value);
         }
         else
         {
            return Guid.Empty;
         }
      }

      public static XmlElement GetNodeById(XmlElement rootElement, Guid nodeId)
      {
         XmlNode foundNode = rootElement.SelectSingleNode(string.Format(".//*[Properties/*/@editorId='{0}']", nodeId));
         return foundNode as XmlElement;
      }

      public static XmlElement GetNodeByPrefabId(XmlElement rootElement, Guid nodeId)
      {
         XmlNode foundNode = rootElement.SelectSingleNode(string.Format(".//*[Properties/*/@prefabEditorId='{0}']", nodeId));
         return foundNode as XmlElement;
      }

      private static void MergePrefabNode(ref XmlElement destElement, XmlElement sourceElement, IPropertyDataSource propertySource, Dictionary<Guid, XmlElement> prefabEditorIdToNode)
      {
         switch(sourceElement.Name)
         {
            case "PropertyContainer":
               {
                  // Merge in data from the prefab, this adds fully merged data from the prefab into our property container.
                  MergePropertiesV2.MergeNode(ref destElement, sourceElement);

                  // Then we merge the property container itself, this causes things that weren't part of the prefab to be merged (i.e. dynamic component groups that only exist in the instance).
                  MergePropertiesV2.Merge_Inplace( ref destElement, propertySource );

                  // reorder dynamic "non prefab" components to go last
                  MergePropertiesV2.ReorderDynamicComponentGroups(destElement, MergePropertiesV2.EReorderDynamicComponentGroupMode.kReorderDynamic);
               }
               break;

            case "Node":
               {
                  // We want to bring the name over from the prefab.
                  destElement.RemoveAttribute("name");

                  destElement = XmlNodeHelpers.RenameElement(destElement, sourceElement.Name);

                  MergePropertiesV2.MergeAttributes(destElement, sourceElement);

                  MergeChildren(destElement, sourceElement, propertySource, prefabEditorIdToNode);
               }
               break;

            default:
               {
                  destElement = XmlNodeHelpers.RenameElement(destElement, sourceElement.Name);

                  MergePropertiesV2.MergeAttributes(destElement, sourceElement);

                  MergeChildren(destElement, sourceElement, propertySource, prefabEditorIdToNode);
               }
               break;
         }
      }

      private static void MergeChildren(XmlElement destOwner, XmlElement sourceOwner, IPropertyDataSource propertySource, Dictionary<Guid, XmlElement> prefabEditorIdToNode)
      {
         switch (destOwner.Name)
         {
            default:
               {
                  foreach (XmlNode sourceChildNode in sourceOwner.ChildNodes)
                  {
                     XmlElement sourceChild = sourceChildNode as XmlElement;

                     if (sourceChild == null)
                        continue;

                     if( sourceChild.Name == "Node" )
                     {
                        Guid sourceNodeId = GetNodeId(sourceChild);
                        
                        XmlElement destChild = null;

                        if( prefabEditorIdToNode.TryGetValue(sourceNodeId, out destChild) )
                        {
                           prefabEditorIdToNode.Remove(sourceNodeId);

                           destOwner.AppendChild(destChild);
                           MergePrefabNode(ref destChild, sourceChild, propertySource, prefabEditorIdToNode);
                        }
                        else
                        {
                           destChild = destOwner.OwnerDocument.ImportNode(sourceChild, true) as XmlElement;
                           destOwner.AppendChild(destChild);
                        }
                     }
                     else
                     {
                        XmlElement destChild = FindMatchingChild(destOwner, sourceChild);

                        if (destChild != null)
                        {
                           destOwner.RemoveChild(destChild);
                           destOwner.AppendChild(destChild);

                           MergePrefabNode(ref destChild, sourceChild, propertySource, prefabEditorIdToNode);
                        }
                        else
                        {
                           destChild = destOwner.OwnerDocument.ImportNode(sourceChild, true) as XmlElement;
                           destOwner.AppendChild(destChild);
                        }
                     }
                  }
               }
               break;
         }
      }

      private static XmlElement FindMatchingChild(XmlElement destElement, XmlElement sourceChild)
      {
         switch (sourceChild.Name)
         {
            case "Node":
               {
                  Guid sourceNodeId = GetNodeId(sourceChild);
                  string sourceNodeName = sourceChild.GetAttribute("name");

                  foreach(XmlNode destChildNode in destElement.ChildNodes)
                  {
                     XmlElement destChild = destChildNode as XmlElement;
                     if( destChild != null )
                     {
                        switch (destChild.Name)
                        {
                           case "UsingNode":
                           case "Node":
                              {
                                 // Match node based on id if available
                                 if (sourceNodeId != Guid.Empty)
                                 {
                                    Guid destNodePrefabId = GetNodePrefabId(destChild);
                                    if (destNodePrefabId == sourceNodeId)
                                    {
                                       return destChild;
                                    }
                                 }
                                 else
                                 {
                                    // As a fall back look it up by name
                                    string destNodeName = destChild.GetAttribute("name");
                                    if (destNodeName == sourceNodeName)
                                    {
                                       return destChild;
                                    }
                                 }
                              }
                              break;
                        }
                     }
                  }

                  return null;
               }

            case "PropertyContainer":
               {
                  // Get the first usingPropertyContainer or PropertyContainer child element.
                  XmlElement destChild = destElement["UsingPropertyContainer"];
                  if( destChild == null )
                  {
                     destChild = destElement["UsingPropertyContainer"];
                  }

                  if( destChild != null )
                  {
                     Guid sourceEditorId = new Guid(sourceChild.GetAttribute("editorId"));
                     Guid destPrefabId = new Guid(destChild.GetAttribute("prefabEditorId"));
                     Debug.Assert(sourceEditorId == destPrefabId, "Unmatched property containers, shouldn't happen, because the node this property container is part of shouldn't have been selected as a dest node for merging!");

                     // Remove name attribute from destination, we always want to inherit the name from the prefab.
                     destChild.RemoveAttribute("name");
                     return destChild;
                  }

                  return null;
               }
            default:
               {
                  string sourceId = sourceChild.GetAttribute("id");
                  string sourceName = sourceChild.GetAttribute("name");

                  if (!string.IsNullOrEmpty(sourceId) || !string.IsNullOrEmpty(sourceName))
                  {
                     return MergePropertiesV2.FindChildElementByNameAndId(destElement, sourceId, sourceName);
                  }

                  return destElement[sourceChild.Name];
               }
         }
      }

      private static void AssignOrGetEditorIds(XmlElement mergedPrefab, Dictionary<Guid, Guid> editorIdRemapping)
      {
         foreach (XmlElement editorIdOwner in mergedPrefab.SelectNodes(".//PropertyContainer | .//ComponentGroup | .//EventEntry | .//LinkEntry"))
         {
            string idAttributeName;
            string prefabIdAttributeName;

            switch(editorIdOwner.Name)
            {
               case "PropertyContainer":
               case "ComponentGroup":
                  idAttributeName = "editorId";
                  prefabIdAttributeName = "prefabEditorId";
                  break;
               
               case "EventEntry":
               case "LinkEntry":
                  idAttributeName = "id";
                  prefabIdAttributeName = "prefabId";
                  break;

               default:
                  continue;
            }

            // we only assign new editor ids for prefab component groups.
            if (editorIdOwner.Name == Helpers.kElement_ComponentGroup && !Helpers.IsComponentGroupPrefab(editorIdOwner.CreateNavigator()))
               continue;

            XmlAttribute editorIdAttr = editorIdOwner.GetAttributeNode(idAttributeName);
            XmlAttribute prefabEditorIdAttr = editorIdOwner.GetAttributeNode(prefabIdAttributeName);

            Guid editorId = new Guid(editorIdAttr.Value);
            Guid prefabEditorId;

            // if we already have a prefab editor id we just add it to the mapping
            if (prefabEditorIdAttr != null)
            {
               prefabEditorId = new Guid(prefabEditorIdAttr.Value);
            }
            else
            {
               // otherwise the prefab editor id becomes the current editor id and we generate a new editor id.
               prefabEditorId = editorId;
               editorId = Guid.NewGuid();
            }

            editorIdOwner.SetAttribute(prefabIdAttributeName, prefabEditorId.ToString());
            editorIdOwner.SetAttribute(idAttributeName, editorId.ToString());

            editorIdRemapping[prefabEditorId] = editorId;
         }
      }

      private void RemapConnections(XmlElement mergedPrefab, Dictionary<Guid, Guid> editorIdRemapping)
      {
         foreach (XmlElement entry in mergedPrefab.SelectNodes(".//EventEntry | .//LinkEntry"))
         {
            XmlAttribute targetAttr = entry.GetAttributeNode("target");
            XmlAttribute prefabTargetAttr = entry.GetAttributeNode("prefabTarget");

            if( prefabTargetAttr == null )
            {
               Guid targetId = new Guid(targetAttr.Value);
               Guid newTargetId;

               if (editorIdRemapping.TryGetValue(targetId, out newTargetId))
               {
                  entry.SetAttribute("prefabTarget", targetId.ToString());
                  targetAttr.Value = newTargetId.ToString();
               }
            }
         }
      }
   }
}
