using System;
using System.Collections.Generic;
using System.Text;
using Tools.AssetSystem;
using Tools.Property.Source;
using Tools.Property.Misc;
using System.Xml;
using Tools.Common;

namespace Tools.Property.Build
{
   public class MergeFromAssets
   {
      internal class XmlDocumentCache
      {
         public XmlDocument Load( Tools.AssetSystem.Manager assetManager, String asset )
         {
            XmlDocument doc;

            if ( !mCache.TryGetValue( asset, out doc ) )
            {
               doc = new XmlDocument();
               doc.Load( assetManager.GetSystemPath( asset ) );

               mCache.Add( asset, doc );
            }

            return doc;
         }

         public XmlDocument LoadOrNull( Tools.AssetSystem.Manager assetManager, String asset )
         {
            try
            {
               XmlDocument doc;
               if ( !mCache.TryGetValue( asset, out doc ) )
               {
                  String assetFilesystemPath = assetManager.GetSystemPath( asset );
                  if ( System.IO.File.Exists( assetFilesystemPath ) )
                  {
                     doc = new XmlDocument();
                     doc.Load( assetFilesystemPath );

                     mCache.Add( asset, doc );
                  }
               }
               return doc;
            }
            catch (System.Exception)
            {
               return null;
            }
         }

         private Dictionary<String, XmlDocument> mCache = new Dictionary<string,XmlDocument>();
      }

      public static void MergeDynamicMessagesEventsAndLinks(XmlElement topLevelNode, Manager assetManager, IPropertyDataSource propertyDb )
      {
         foreach (XmlElement component in topLevelNode.SelectNodes(".//ComponentGroup/Component"))
         {
            XmlDocumentCache cache = new XmlDocumentCache();

            ScanAndMergeDynamicItems(component, "Messages", "Message", assetManager, propertyDb, cache);
            ScanAndMergeDynamicItems( component, "Events", "Event", assetManager, propertyDb, cache );
            ScanAndMergeDynamicItems( component, "Links", "Link", assetManager, propertyDb, cache );
         }
      }

      // SQueryItem is a dictionary key used to stop a cascading import of data
      private struct SQueryItem
      {
         public String mType;
         public String mAssetPath;
         public String mQuery;

         public SQueryItem(String type, String assetPath, String query)
         {
            mType = type;
            mAssetPath = assetPath;
            mQuery = query;
         }

         public static bool operator ==(SQueryItem lhs, SQueryItem rhs)
         {
            return lhs.mType == rhs.mType && lhs.mAssetPath == rhs.mAssetPath && lhs.mQuery == rhs.mQuery;
         }

         public static bool operator !=(SQueryItem lhs, SQueryItem rhs)
         {
            return !(lhs == rhs);
         }

         public override bool Equals(object obj)
         {
            if (obj is SQueryItem)
            {
               return this == (SQueryItem)obj;
            }
            else
            {
               return base.Equals(obj);
            }
         }

         public override int GetHashCode()
         {
            return mType.GetHashCode() + mAssetPath.GetHashCode() + mQuery.GetHashCode();
         }
      }

      // ImportArguments is a simple class to manage various args throughout the process. 
      private class ImportArguments
      {
         // input
         public String mListType;
         public String mItemType;
         public Manager mAssetManager;
         public IPropertyDataSource mPropertyDb;
         public XmlDocumentCache mCache;

         // output
         public Dictionary<SQueryItem, XmlDocument> mImportResultSet = new Dictionary<SQueryItem, XmlDocument>();
      }

      private static void ScanAndMergeDynamicItems(XmlElement componentElement, String listElementName, String type, Manager assetManager, IPropertyDataSource propertyDb, XmlDocumentCache cache )
      {
         // This function simply scans the list element for Imports and marges them back in, killing the Imports in the process
         // Then it merges the imports with the current list

         XmlElement listElement = (XmlElement)componentElement.SelectSingleNode(listElementName);
         if (listElement != null)
         {
            ImportArguments args = new ImportArguments();
            args.mListType = listElementName;
            args.mItemType = type;
            args.mAssetManager = assetManager;
            args.mPropertyDb = propertyDb;
            args.mCache = cache;

            foreach (XmlElement el in listElement.SelectNodes(Helpers.kElement_DynamicItems))
            {
               // Import with the node, then remove the Import node
               RunImportQueryAndStoreResults(el, args);
               listElement.RemoveChild(el);
            }

            // Merge the resultant messages back
            foreach (XmlDocument doc in args.mImportResultSet.Values)
            {
               MergePropertiesV2.MergeMessagesElementsOrLinks_Inplace(listElement, doc.DocumentElement);
            }
         }
      }

      private static List<String> GetXmlFilesForRecursion( String rootXmlAsset, String query, ImportArguments args )
      {
         Dictionary<String, bool> xmlFiles = new Dictionary<string, bool>();

         Queue<String> toParse = new Queue<string>();

         toParse.Enqueue( rootXmlAsset );
         while ( toParse.Count > 0 )
         {
            String parsing = toParse.Dequeue();

            // If we're already parsed the file, don't re-parse
            if ( !xmlFiles.ContainsKey( parsing ) )
            {
               XmlDocument recursedDoc = args.mCache.LoadOrNull( args.mAssetManager, parsing );

               if ( recursedDoc != null )
               {
                  xmlFiles.Add( parsing, true );

                  foreach ( XmlElement subElement in recursedDoc.SelectNodes( query ) )
                  {
                     string newXmlAsset = subElement.InnerText;

                     if ( args.mAssetManager.IsValidRepositoryRelativePath( newXmlAsset ) )
                     {
                        toParse.Enqueue( newXmlAsset );
                     }
                  }
               }
            }
         }

         return new List<String>( xmlFiles.Keys );
      }

      private static void RunImportQueryAndStoreResults(XmlElement importNode, ImportArguments args )
      {
         // Note that this function is also called for sub-imports, so make sure to keep the function re-entrant

         XmlElement componentElement = (XmlElement)importNode.ParentNode.ParentNode;
         XmlElement listElement = (XmlElement)importNode.ParentNode;
         String componentXmlReference = importNode.GetAttribute( Helpers.kDynamicItems_Source);
         String xmlQuery = importNode.GetAttribute( Helpers.kDynamicItems_Query);
         String importType = importNode.GetAttribute( Helpers.kDynamicItems_Type );
         if (xmlQuery != "" && componentXmlReference != "" && importType != "" )
         {
            // Get the xml asset that this is going to load
            String xmlSourceAsset = Manager.NormalizeRepositoryRelativePath(componentXmlReference);
            if (xmlSourceAsset == null)
            {
               XmlElement propertyElement = (XmlElement)componentElement.SelectSingleNode(componentXmlReference);

               if (propertyElement != null)
               {
                  // We call NormalizeRepositoryRelativePath here both to validate we have a repos path and to
                  // make sure the string has the correct case, since we'll be using this asset path as part 
                  // of a dictionary key
                  xmlSourceAsset = Manager.NormalizeRepositoryRelativePath(Tools.Property.Misc.Helpers.GetPropertyValueElement(propertyElement));
               }
            }

            // Each "source asset" could recurse into multiple assets
            List<String> componentXmlAssets = null;
            if ( xmlSourceAsset != null )
            {
               if ( importNode.HasAttribute( Helpers.kDynamicItems_RecursiveSourceQuery ) )
               {
                  componentXmlAssets = GetXmlFilesForRecursion( xmlSourceAsset, importNode.GetAttribute( Helpers.kDynamicItems_RecursiveSourceQuery ), args );
               }
               else if ( args.mAssetManager.IsValidRepositoryRelativePath( xmlSourceAsset ) )
               {
                  componentXmlAssets = new List<string>();
                  componentXmlAssets.Add( xmlSourceAsset );
               }
            }

            if (componentXmlAssets != null && componentXmlAssets.Count > 0 )
            {
               foreach ( String componentXmlAsset in componentXmlAssets )
               {
                  SQueryItem qi = new SQueryItem( importType, componentXmlAsset, xmlQuery );
                  if ( !args.mImportResultSet.ContainsKey( qi ) )
                  {
                     // Now let's load the destination document
                     String xmlFile = args.mAssetManager.GetSystemPath( qi.mAssetPath );

                     // Ok, we have the file.  Let's load it, run the query, and import everything
                     XmlDocument doc = args.mCache.LoadOrNull( args.mAssetManager, xmlFile );

                     if ( doc != null )
                     {
                        // We have not processed this asset yet, so let's run through the nodes and make a document
                        // to hold the nodes
                        //
                        // Note that we add it to the import result set now.  Since this function is 
                        // reentrant, ProcessImportResultNodes could call back into this.  We don't want to start 
                        // another import process on the asset we're currently importing

                        XmlDocument importDoc = new XmlDocument();
                        importDoc.AppendChild( importDoc.CreateElement( listElement.Name ) );
                        args.mImportResultSet.Add( qi, importDoc );

                        // Let's fill out the result we just added to the result set
                        ProcessImportResultNodes( importDoc, importType, doc.SelectNodes( xmlQuery ), args );
                     }
                  }
               }
            }
         }
      }

      private static void ProcessImportResultNodes(XmlDocument importDoc, String importType, XmlNodeList resultNodes, ImportArguments args)
      {
         foreach (XmlElement resultElement in resultNodes )
         {
            switch ( importType )
            {
            case Helpers.kDynamicItems_Type_FromXML:
               if (resultElement.Name == Helpers.kElement_DynamicItems)
               {
                  // If we're an import node, run recursively
                  RunImportQueryAndStoreResults(resultElement, args);
               }
               else if (resultElement.Name == args.mItemType)
               {
                  // If we're a node of the type we expect, import it directly
                  importDoc.DocumentElement.AppendChild(importDoc.ImportNode(resultElement, true));
               }
               break;

            case Helpers.kDynamicItems_Type_FromPropertyContainer:
               if ( resultElement.Name == "UsingPropertyContainer" )
               {
                  // If we're pointing at an unmerged component, then we should import the nodes from that and merge them
                  // with our import doc

                  XmlDocument importedFromPropertyContainer = ImportUnmergedPropertyContainer( resultElement, args );

                  if ( importedFromPropertyContainer != null )
                  {
                     MergePropertiesV2.MergeMessagesElementsOrLinks_Inplace( importDoc.DocumentElement, importedFromPropertyContainer.DocumentElement );
                  }
               }
               break;

            case Helpers.kDynamicItems_Type_FromPropertyValues:
               if ( resultElement.Name == "UsingProperty" )
               {
                  // Get the property's value
                  String propertyValue = Helpers.GetPropertyValueElement( resultElement );

                  if ( !String.IsNullOrEmpty( propertyValue ) )
                  {
                     // We need to make a new "type" node, if we don't have one in the list already
                     if ( null == importDoc.DocumentElement.SelectSingleNode( string.Format( "{0}[@type={1}]", XmlNodeHelpers.EscapeXml( args.mItemType ), XmlNodeHelpers.AsQuotedXml( propertyValue ) ) ) )
                     {
                        XmlElement newElement = importDoc.CreateElement( args.mItemType );
                        newElement.SetAttribute( "type", propertyValue );
                        importDoc.DocumentElement.AppendChild( newElement );
                     }
                  }
               }
               break;

            }
         }
      }

      private static XmlDocument ImportUnmergedPropertyContainer(XmlElement element, ImportArguments args)
      {
         XmlDocument merged = MergePropertiesV2.Merge(element, args.mPropertyDb);
         if (merged != null)
         {
            XmlDocument fakeDoc = new XmlDocument();
            fakeDoc.AppendChild(fakeDoc.CreateElement(args.mListType));

            // Our document is now a component group with one child component
            // Let's run through the subtypes in the child component

            ProcessImportResultNodes( fakeDoc, Helpers.kDynamicItems_Type_FromXML, merged.SelectNodes( String.Format( "//Component/{0}/{1}", args.mListType, args.mItemType ) ), args );
            ProcessImportResultNodes( fakeDoc, Helpers.kDynamicItems_Type_FromXML, merged.SelectNodes( String.Format( "//Component/{0}/{1}", args.mListType, Helpers.kElement_DynamicItems ) ), args );

            return fakeDoc;
         }
         else
         {
            return null;
         }
      }
   }
}
