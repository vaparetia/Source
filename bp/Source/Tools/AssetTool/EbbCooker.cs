using System;
using System.Collections.Generic;
using System.Xml;
using Tools.AssetSystem;
using Tools.Property.Source;
using Tools.Property.Cook;
using Tools.Common;
using Tools.Property.Misc;

// --------------------------------------------------------------------------------------------------------
// EBB STATE MACHINE COOKER
// --------------------------------------------------------------------------------------------------------

namespace AssetTool
{
   partial class Cookers
   {
      // this should match the enum in Tools.Ebb.Node
      public enum EEbbNodeType
      {
         kState,
         kTrigger,
         kFunction,
         kRelay
      }

      class CEbbLink
      {
         public enum EFollowType
         {
            kPass,
            kFail
         };

         public CEbbLink(Guid destinationId, int priority, EFollowType followType)
         {
            mDestinationId = destinationId;
            mPriority = priority;
            mFollowType = followType;
         }
         public readonly Guid mDestinationId;
         public readonly int mPriority;
         public EFollowType mFollowType;
      }

      class CEbbLinkComparer : IComparer<CEbbLink>
      {
         int IComparer<CEbbLink>.Compare(CEbbLink left, CEbbLink right)
         {
            int diff = right.mPriority - left.mPriority;
            if ( diff != 0 )
            {
               return diff;
            }
            else
            {
               return right.mFollowType.CompareTo( left.mFollowType );
            }
         }
      }

      class CEbbNode
      {
         public CEbbNode(Guid id, string name, EEbbNodeType type)
         {
            mId = id;
            mName = name;
            mType = type;
            mLinkedTo = new List<CEbbLink>();
            mIndex = 0;
         }
         public Guid mId;
         public string mName;
         public EEbbNodeType mType;
         public List<CEbbLink> mLinkedTo;
         public int mIndex;
         public System.IO.MemoryStream mPropertyStream;
      }

      /// <summary>
      /// Cooks Ebb State Machine.
      /// Converts sesm (Source Ebb State Machine: an xml doc) into cesm (Cooked Ebb State Machine: binary data).
      /// </summary>
      public static bool CookEBSM(string inputFilename, string outputFilename, Manager assetManager, PlatformType.EPlatform platform)
      {
         FileBasedPropertyObjectEnumerator propertySource = new FileBasedPropertyObjectEnumerator();
         propertySource.AddFiles(FileBasedPropertyObjectEnumerator.FileSearchParams.DefaultFileSearchParams(assetManager));

         XmlDocument doc = new XmlDocument();
         doc.Load(inputFilename);

         // dictionary contains all nodes (includes Relays)
         Dictionary<Guid, CEbbNode> nodeDictionary = new Dictionary<Guid, CEbbNode>();
         // list only contains the nodes we're going to write out (excludes Relays)
         List<CEbbNode> nodeList = new List<CEbbNode>();

         CookProperties.CookInfo cookInfo = new CookProperties.CookInfo();
         // Add custom cooker for splines
         cookInfo.mCustomPropertyCooker += new CookProperties.CookInfo.CustomPropertyCooker(Helper.Spline.ProcessCustomProperty);
         cookInfo.mPlatform = platform;

         CookProperties.CookOutput cookOutput = new CookProperties.CookOutput();

         int nodeIndex = 0;
         foreach (XmlElement node in doc.SelectNodes("EbbDocument/Nodes/Node"))
         {
            EEbbNodeType type = (EEbbNodeType) Enum.Parse(typeof(EEbbNodeType), node.GetAttribute("type"));

            XmlElement properties = (XmlElement)node.SelectSingleNode("UsingPropertyContainer");
            Guid editorId = new Guid(properties.GetAttribute("editorId"));

            XmlDocument mergedNode = new XmlDocument();
            bool doDeepClone = true;
            mergedNode.AppendChild(mergedNode.ImportNode(properties, doDeepClone));
            Tools.Property.Build.MergePropertiesV2.Merge_Inplace(mergedNode, propertySource);

            // now I have merged properties for a node (state/trigger/function)

            string name = Tools.Property.Misc.Helpers.GetValueAtLabel(mergedNode, "baseebbproperties_name");
            if (name != null)
            {
               CEbbNode ebbNode = new CEbbNode(editorId, name, type);
               nodeDictionary.Add(editorId, ebbNode);

               // cook all properties for this node at once
               if (ebbNode.mType != EEbbNodeType.kRelay)
               {
                  ebbNode.mIndex = nodeIndex++;
                  ebbNode.mPropertyStream = new System.IO.MemoryStream();
                  nodeList.Add(ebbNode);
                  XmlElement propertyContainer = (XmlElement)mergedNode.SelectSingleNode("PropertyContainer");
                  CookProperties.WriteCookedPropertiesForPropertyContainer( assetManager, propertyContainer, cookInfo, cookOutput, ebbNode.mPropertyStream );
                  // Write zero children terminator
                  ebbNode.mPropertyStream.Write(new byte[] {0, 0, 0, 0}, 0, 4);
                  CookProperties.AddPropertyDependencies( assetManager, propertySource, propertyContainer, cookInfo, cookOutput );
               }
            }
         }

         foreach (XmlElement link in doc.SelectNodes("EbbDocument/Links/Link"))
         {
            int version = Int32.Parse(link.GetAttribute("version"));
            if (version == 1)
            {
               Guid sourceId = new Guid(link.GetAttribute("source"));
               Guid destinationId = new Guid(link.GetAttribute("destination"));
               int priority = Int32.Parse(link.GetAttribute("priority"));

               if (nodeDictionary.ContainsKey(sourceId)
                  && nodeDictionary.ContainsKey(destinationId))
               {
                  CEbbNode ebbNode = nodeDictionary[sourceId];
                  ebbNode.mLinkedTo.Add(new CEbbLink(destinationId, priority, CEbbLink.EFollowType.kPass));
               }
            }
            else
            {
               XmlElement properties = (XmlElement)link.SelectSingleNode("UsingPropertyContainer");

               XmlDocument mergedLink = new XmlDocument();
               bool doDeepClone = true;
               mergedLink.AppendChild(mergedLink.ImportNode(properties, doDeepClone));
               Tools.Property.Build.MergePropertiesV2.Merge_Inplace(mergedLink, propertySource);

               // now I have merged properties for a link

               Guid sourceId = new Guid(Tools.Property.Misc.Helpers.GetValueAtLabel(mergedLink, "baseebblink_source"));
               Guid destinationId = new Guid(Tools.Property.Misc.Helpers.GetValueAtLabel(mergedLink, "baseebblink_destination"));
               int priority = Int32.Parse(Tools.Property.Misc.Helpers.GetValueAtLabel(mergedLink, "baseebblink_priority"));
               CEbbLink.EFollowType followType =
                  Misc.ParseBool( Helpers.GetValueAtLabel( mergedLink, "baseebblink_followonfail" ), false )
                  ? CEbbLink.EFollowType.kFail
                  : CEbbLink.EFollowType.kPass;

               if (nodeDictionary.ContainsKey(sourceId)
                  && nodeDictionary.ContainsKey(destinationId))
               {
                  CEbbNode ebbNode = nodeDictionary[sourceId];
                  ebbNode.mLinkedTo.Add( new CEbbLink( destinationId, priority, followType ) );
               }
            }
         }

         // sort the links for each node by priority
         foreach (KeyValuePair<Guid, CEbbNode> ebbNode in nodeDictionary)
         {
            ebbNode.Value.mLinkedTo.Sort(new CEbbLinkComparer());
         }

         // need to deal with relays - replace with the onward links
         foreach (CEbbNode ebbNode in nodeList)
         {
            bool done = false;
            while (!done) 
            {
               done = true;
               int index = 0;
               foreach (CEbbLink ebbLink in ebbNode.mLinkedTo)
               {
                  CEbbNode destNode = nodeDictionary[ebbLink.mDestinationId];
                  if (destNode.mType == EEbbNodeType.kRelay)
                  {
                     CEbbLink.EFollowType followType = ebbLink.mFollowType;
                     ebbNode.mLinkedTo.Remove(ebbLink);
                     if (destNode.mLinkedTo.Count == 0)
                     {
                        // find the destination relay(s)
                        foreach (KeyValuePair<Guid, CEbbNode> potentialDestNode in nodeDictionary)
                        {
                           if (potentialDestNode.Value.mType == EEbbNodeType.kRelay
                              && potentialDestNode.Value.mName == destNode.mName
                              && potentialDestNode.Value.mLinkedTo.Count > 0)
                           {
                              // copy the links and set the follow type
                              List<CEbbLink> linkedTo = new List<CEbbLink>();
                              foreach (CEbbLink cloneStamp in potentialDestNode.Value.mLinkedTo)
                              {
                                 linkedTo.Add(new CEbbLink(cloneStamp.mDestinationId, cloneStamp.mPriority, followType));
                              }
                              ebbNode.mLinkedTo.InsertRange(index, linkedTo);
                              // need to restart the link loop to keep the index in sync
                              break;
                           }
                        }
                     }
                     else
                     {
                        // copy the links and set the follow type
                        List<CEbbLink> linkedTo = new List<CEbbLink>();
                        foreach (CEbbLink cloneStamp in destNode.mLinkedTo)
                        {
                           linkedTo.Add(new CEbbLink(cloneStamp.mDestinationId, cloneStamp.mPriority, followType));
                        }
                        ebbNode.mLinkedTo.InsertRange(index, linkedTo);
                     }
                     done = false;
                     break;
                  }
                  index++;
               }
            }
         }

         System.IO.FileStream outStream = new System.IO.FileStream(outputFilename, System.IO.FileMode.Create);
         using (BinaryWriterBigEndian writer = new BinaryWriterBigEndian(outStream))
         {
            // 4cc and version
            writer.Write(Misc.GetFourCCFromString("EBSM"));
            
            // VERSION HISTORY:
            // Version 1, 2 - Initial
            // Version 3 - added link flags for follow on fail
            // Version 4 - fix for relay removal to preserve follow on fail
            // Version 5 - additional fix for relay removal that *copies* the onward links rather than referencing them!
            writer.Write(5);

            // write out the number of nodes
            writer.Write(nodeList.Count);

            // write out the name, type and properties
            foreach (CEbbNode ebbNode in nodeList)
            {
               // write the properties (now includes the type implicitly and the name explicitly)
               outStream.Write(ebbNode.mPropertyStream.GetBuffer(), 0, (int) ebbNode.mPropertyStream.Length);
            }
            // write out links
            foreach (CEbbNode ebbNode in nodeList)
            {
               writer.Write(ebbNode.mLinkedTo.Count);
               foreach (CEbbLink ebbLink in ebbNode.mLinkedTo)
               {
                  CEbbNode ebbDestNode = nodeDictionary[ebbLink.mDestinationId];
                  uint linkFlags = 0;

                  if ( ebbLink.mFollowType == CEbbLink.EFollowType.kFail )
                  {
                     // Follow on fail is a flag
                     linkFlags |= 1;
                  }
                  
                  writer.Write( linkFlags );
                  writer.Write(ebbDestNode.mIndex);
               }
            }
         }

         outStream.Close();

         // write out assets file
         PackageAssets.WriteAssets(inputFilename, cookOutput.PackageAssets, assetManager, platform );
         // write out build file
         BuildAssets.WriteAssets( inputFilename, cookOutput.BuildAssets, assetManager, platform );

         return true;
      }
   }
}