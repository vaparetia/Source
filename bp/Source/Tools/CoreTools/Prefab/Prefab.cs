using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using Tools.Property.Misc;
using Tools.Common;

namespace CoreTools.Prefab
{
   public class Prefab
   {
      const string kPrefabExtension = ".prefab";

      PrefabRepository  mRepository;
      Guid              mId;
      string            mName;
      List<string>      mTags;
      XmlElement        mMergedPrefab;
      List<string>      mScriptDependencies = new List<string>();

      public static Prefab LoadPrefab(PrefabRepository repository, Guid id)
      {
         string filePath = Prefab.GetMetaDataPath(repository, id);
         XmlDocument metaData = new XmlDocument();
         metaData.Load(filePath);

         if (metaData != null)
         {
            return new Prefab(repository, id, metaData);
         }
         else
         {
            return null;
         }
      }

      public Prefab(PrefabRepository repository, Guid id, XmlDocument metaData)
      {
         mRepository = repository;
         mId = id;

         XmlNode nameElement = metaData.SelectSingleNode("//Name");
         mName = nameElement.InnerText;

         mTags = new List<string>();

         foreach (XmlNode tag in metaData.SelectNodes("//Tag"))
         {
            mTags.Add(tag.InnerText);
         }
      }

      public static Prefab Create(PrefabRepository repository, Guid id, string name)
      {
         string filePath = GetFilePath(repository, id);

         XmlDocument prefabMetaData = new XmlDocument();

         XmlElement prefabElement = prefabMetaData.CreateElement("Prefab");
         prefabMetaData.AppendChild(prefabElement);

         XmlElement nameElement = prefabMetaData.CreateElement("Name");
         nameElement.InnerText = name;
         prefabElement.AppendChild(nameElement);

         string metaDataPath = Prefab.GetMetaDataPath(repository, id);
         prefabMetaData.Save(metaDataPath);

         Prefab prefab = new Prefab(repository, id, prefabMetaData);
         repository.AddPrefab(prefab);

         return prefab;
      }

      public PrefabRepository Repository
      {
         get { return mRepository; }
      }

      public Guid Id
      {
         get { return mId; }
      }

      public string Name
      {
         get { return mName; }
      }

      public List<string> Tags
      {
         get { return mTags; }
      }

      public void SetTags(List<string> tags)
      {
         string filePath = FilePathMetaData;
         XmlDocument metaData = new XmlDocument();
         metaData.Load(filePath);
         if( metaData != null )
         {
            XmlElement prefabElement = metaData.SelectSingleNode("//Prefab") as XmlElement;
            
            XmlElement tagsElement = prefabElement.SelectSingleNode(".//Tags") as XmlElement;
            if( tagsElement == null )
            {
               tagsElement = metaData.CreateElement("Tags");
               prefabElement.AppendChild(tagsElement);
            }

            // Remove all tag nodes.
            while(tagsElement.HasChildNodes)
            {
               tagsElement.RemoveChild(tagsElement.FirstChild);
            }

            foreach(string tag in tags)
            {
               XmlElement tagElement = metaData.CreateElement("Tag");
               tagElement.InnerText = tag;
               tagsElement.AppendChild(tagElement);
            }

            mTags = tags;
            
            metaData.Save(filePath);
         }
      }

      public bool IsWriteable()
      {
         int writeableCount = 0;

         string prefabPath = FilePath;
         if (System.IO.File.Exists(prefabPath) && (System.IO.File.GetAttributes(prefabPath) & System.IO.FileAttributes.ReadOnly) == 0)
         {
            writeableCount++;
         }

         string metaDataPath = mRepository.mAssetManager.GetMetaDataSystemPath(prefabPath);
         if (System.IO.File.Exists(metaDataPath) && (System.IO.File.GetAttributes(metaDataPath) & System.IO.FileAttributes.ReadOnly) == 0)
         {
            writeableCount++;
         }

         string prefabMayaPath = FilePathMaya;
         if (System.IO.File.Exists(prefabMayaPath) && (System.IO.File.GetAttributes(prefabMayaPath) & System.IO.FileAttributes.ReadOnly) == 0)
         {
            writeableCount++;
         }

         return writeableCount > 0;
      }

      public bool HasData()
      {
         return System.IO.File.Exists(FilePath) && System.IO.File.Exists(FilePathMaya);
      }

      public bool Rename(string newName)
      {
         string metaDataPath = FilePathMetaData;
         if( (System.IO.File.GetAttributes(metaDataPath) & System.IO.FileAttributes.ReadOnly) == 0 )
         {
            mName = newName;

            XmlDocument metaData = new XmlDocument();
            metaData.Load(metaDataPath);
            
            XmlNode nameNode = metaData.SelectSingleNode("//Name");
            nameNode.InnerText = mName;

            metaData.Save(metaDataPath);

            return true;
         }

         return false;
      }

      public XmlElement MergedPrefab
      {
         get
         {
            EnsureMerged();
            return mMergedPrefab;
         }
      }

      public string FilePath
      {
         get { return GetFilePath(mRepository, mId); }
      }

      public string FilePathMaya
      {
         get { return System.IO.Path.ChangeExtension(FilePath, "pma").Replace('\\', '/'); }
      }

      public string FilePathMetaData
      {
         get { return GetMetaDataPath(mRepository, mId); }
      }

      public string GetPerforcePathForAllFiles()
      {
         return System.IO.Path.GetFullPath(System.IO.Path.Combine(mRepository.mRepositoryRoot, mId.ToString())) + "...";
      }

      public List<string> FilesForVersionControl
      {
         get
         {
            List<string> files = new List<string>();
            
            files.Add(FilePath);
            files.Add(mRepository.mAssetManager.GetMetaDataSystemPath(FilePath));
            files.Add(FilePathMaya);

            return files;
         }
      }

      public List<string> ScriptDependencies
      {
         get 
         {
            EnsureMerged(); 
            return mScriptDependencies; 
         }
      }

      public void Invalidate()
      {
         mMergedPrefab = null;
      }

      private void EnsureMerged()
      {
         if (mMergedPrefab == null)
            mMergedPrefab = MergePrefab();
      }

      private XmlElement MergePrefab()
      {
         mScriptDependencies.Clear();
         mScriptDependencies.Add(FilePath);

         XmlDocument sourceDocument = new XmlDocument();
         sourceDocument.Load(FilePath);

         XmlElement sourceRootNode = Tools.Common.XmlNodeHelpers.GetFirstChildElement(sourceDocument);

         // Remove non script nodes
         {
            XmlNodeHelpers.DepthIterator iter = new XmlNodeHelpers.DepthIterator(sourceRootNode);
            while(iter.MoveToNextElement())
            {
               if( iter.IteratorInfo == XmlNodeHelpers.DepthIterator.EIteratorInfo.kII_ExitedElement && iter.Element.Name == "Node")
               {
                  // Delete non-script nodes, promote all child nodes to the current tree level.

                  if( iter.Element.SelectSingleNode("./Properties | ./PrefabData") == null )
                  {
                     // Only delete node if it's not a prefab instance node.
                     if( String.IsNullOrEmpty( iter.Element.GetAttribute("prefabInstanceId") ) )
                     {
                        XmlNode parentNode = iter.Element.ParentNode;

                        foreach(XmlElement childNode in iter.Element.SelectNodes("./Node") )
                        {
                           parentNode.InsertAfter(childNode, iter.Element);
                        }

                        iter.RemoveCurrentElementAndChildren();
                     }
                  }
               }
            }
         }

         MergePrefabNodes(sourceRootNode);

         sourceRootNode = XmlNodeHelpers.RenameElement(sourceRootNode, "Prefab");
         sourceRootNode.RemoveAllAttributes();

         return sourceRootNode;
      }

      private static string GetFilePath(PrefabRepository repository, Guid id)
      {
         return System.IO.Path.GetFullPath(System.IO.Path.Combine(repository.mRepositoryRoot, id.ToString()) + kPrefabExtension);
      }

      private static string GetMetaDataPath(PrefabRepository repository, Guid id)
      {
         return GetFilePath(repository, id) + ".meta";
      }

      private void MergePrefabNodes(XmlElement sourceRootNode)
      {
         Tools.Common.XmlNodeHelpers.DepthIterator it = new Tools.Common.XmlNodeHelpers.DepthIterator(sourceRootNode);
         while (it.MoveToNextElement())
         {
            XmlElement element = it.Element;

            if (it.IteratorInfo == Tools.Common.XmlNodeHelpers.DepthIterator.EIteratorInfo.kII_EnteredElement)
            {
               switch (element.Name)
               {
                  case "Node":
                     {
                        XmlElement prefabData = element["PrefabData"];
                        if (prefabData != null)
                        {
                           Guid prefabId = new Guid(prefabData.SelectSingleNode("./AddPrefab/@id").Value);
                           Prefab dependentPrefab = mRepository.mPrefabManager.GetPrefabById(prefabId);
                           if( dependentPrefab != null )
                           {
                              mScriptDependencies.AddRange(dependentPrefab.ScriptDependencies);
                           }
                           mRepository.mPrefabManager.MergePrefab_Inplace(element);
                           mRepository.mPrefabManager.InjectPrefab(element);
                           
                           it.SkipChildren();
                        }

                     }
                     break;

                  case "Properties":
                     {
                        Tools.Property.Build.MergePropertiesV2.Merge_Inplace( ref element, mRepository.mPropertySource );
                        
                        it.SetCurrentElement(element);
                     }
                     break;

                  case "ComponentGroup":
                     {
                        element.SetAttribute("isPrefab", "true");
                     }
                     break;

                  case "EventEntry":
                  case "LinkEntry":
                     {
                        element.SetAttribute("isPrefab", "true");
                        PrefabHelper.CreateDefaultAttributes(element);
                     }
                     break;

                  case "Value":
                     {
                        // Mark all values as default values
                        element.SetAttribute("default", element.InnerXml);
                     }
                     break;
               }
            }
         }
      }
   }
}
