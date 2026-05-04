using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Xml.XPath;
using Tools.Property.Source;
using Tools.Property.Misc;
using Tools.Property.CodeGen;
using Tools.Property.UI;
using Tools.Property.Build;
using Tools.Common;
using Tools.AssetSystem;

namespace Tools.Property.Cook
{
   public class CookProperties
   {   
      public enum EWriteResult
      {
         kIgnoredElement,
         kSkippedPropertyType,   // Implies implicit skip of children
         kWrotePropertyType,     // Implies implicit skip of children
      };

      // Must match those in CGameObjectProperties.h
      static readonly int kVersion     = 2;
      [Flags]
      enum EGameObjectPropertiesFlags
      {
         kNone,
         kDontCreateOnLoad             = 1,
         kAllowPropertiesDuplication   = 2,
      };
      
      /// <summary>Must match order in CEvaluatorContainerProperty.</summary>
      public enum EEvaluatorTypes
      {
         kFloat,
         kBool,
         kVector,
         kColor,
         kCount,
         kInvalid = -1
      };

      public class CookOutput : ICloneable
      {
         private List<AssetSystem.PackageAsset> mPackageAssets = new List<AssetSystem.PackageAsset>();
         private List<AssetSystem.BuildAsset> mBuildAssets = new List<AssetSystem.BuildAsset>();

         public IEnumerable<AssetSystem.BuildAsset> BuildAssets
         {
            get { return mBuildAssets; }
         }

         public IEnumerable<AssetSystem.PackageAsset> PackageAssets
         {
            get { return mPackageAssets; }
         }

         public void AddBuildAsset( AssetSystem.BuildAsset asset )
         {
            if ( !mBuildAssets.Contains( asset ) )
            {
               mBuildAssets.Add( asset );
            }
         }

         public void AddPackageAsset( AssetSystem.PackageAsset asset )
         {
            if ( !mPackageAssets.Contains( asset ) )
            {
               mPackageAssets.Add( asset );
            }
         }

         public CookOutput Clone()
         {
            CookOutput cloned = new CookOutput();

            mPackageAssets.AddRange( PackageAssets );
            mBuildAssets.AddRange( BuildAssets );

            return cloned;
         }

         object ICloneable.Clone()
         {
            return this.Clone();
         }
      }

      public class CookInfo
      {
         public delegate void ScriptObjectPropertiesModifier(XmlElement scriptObject, AssetSystem.Manager assetManager, CookInfo cookInfo, CookOutput cookOutput );
         public delegate void ComponentPropertiesModifier(XmlElement componentGroup, AssetSystem.Manager assetManager, CookInfo cookInfo);
         public delegate void CustomPropertyCooker(XmlNodeReader propertyNode, string propertySubType, string propertyValue, BinaryWriterBigEndian writer);

         public string                          mScriptingFilename;
         public int                             mComponentRecursionDepth   = 0;
         public PlatformType.EPlatform ?        mPlatform = null;
         public ScriptObjectPropertiesModifier  mScriptObjectPropertiesModifier;
         public ComponentPropertiesModifier     mComponentGroupPropertiesModifier;
         public CustomPropertyCooker            mCustomPropertyCooker;         
      };

      private static bool DoesComponentCookOnPlatform( Data.Component component, PlatformType.EPlatform? platform )
      {
         // If we're specifyign a platform, and the component has a list of platforms it supports, then
         // only cook this component if it's supported by the target platform.

         if ( platform.HasValue )
         {
            IEnumerable<String> platforms = component.PlatformCPPDefines;
            // if platforms is null, then it's all platforms

            if ( platforms == null )
            {
               return true;
            }
            else
            {
               String checkingPlatform = PlatformType.GetPlatformTypeForEnum( platform.Value ).CPPDefine;

               foreach ( String platformString in platforms )
               {
                  if ( platformString == checkingPlatform )
                  {
                     return true;
                  }
               }

               return false;
            }
         }
         else
         {
            return true;
         }

      }

      /// <summary>
      /// Writes binary version of properties that a ok for export.
      /// Deals correctly with properties, property objects, component groups, and components.
      /// It will recursively write out properties below parent objects (e.g. property objects below components).
      /// When dealing with component groups, it only writes the currently selected component.
      /// It deals correctly with component groups below components.
      /// The passed in node is assumed to be in merged format.
      /// </summary>
      public static EWriteResult WriteCookedProperties(AssetSystem.Manager assetManager, XmlNodeReader propertyNode, CookInfo cookInfo, CookOutput cookOutput, System.IO.Stream outputStream, Guid? parentEditorId)
      {
         // Check if it's flagged to not be exported
         if (!Helpers.ExportProperty(propertyNode))
         {
            // Yep, skip it
            propertyNode.Skip(); // Ignore all elements below this
            return EWriteResult.kSkippedPropertyType;
         }

         switch (propertyNode.Name)
         {
            case Helpers.kElement_Property:
            break;
            case Helpers.kElement_PropertyObject:
            break;
            case Helpers.kElement_EvaluatorProperty:
            break;
            case Helpers.kElement_ComponentGroup:
            break;
            case Helpers.kElement_Component:
            break;
            case Helpers.kElement_MessageObject:
            break;
            default:
               // Not interested in this element, don't do anything
               return EWriteResult.kIgnoredElement;
         }

         EWriteResult result = EWriteResult.kIgnoredElement;
         string name = propertyNode.GetAttribute(Helpers.kProperty_Name, "");
         string id = propertyNode.GetAttribute(Helpers.kProperty_Id, "");

         BinaryWriterBigEndian propertyWriter = new BinaryWriterBigEndian(outputStream);

         System.IO.MemoryStream tempStream = new System.IO.MemoryStream();
         using (BinaryWriterBigEndian tempWriter = new BinaryWriterBigEndian(tempStream))
         {
            // We can have Property or PropertyObject elements, not interested in anything else
            switch (propertyNode.Name)
            {
               case Helpers.kElement_Property:
               {
                  // Write 4CC
                  uint nameHash = CodeGenHelpers.GetStringHash(name);
                  propertyWriter.Write(nameHash);

                  string type = CodeGenHelpers.GetCookType(propertyNode);

                  if (type == Helpers.kProperty_Type_custom)
                  {
                     int i = 0;
                     ++i;
                  }

                  string propertyValueSubtree;
                  string propertyValue;
                  
                  Helpers.GetPropertyValueElement(propertyNode, out propertyValueSubtree, out propertyValue);
                  
                  switch (type)
                  {
                     case Helpers.kProperty_Type_color:
                        {
                           ColorF color = ColorF.FromString(propertyValue);
                           tempWriter.Write(color.R * color.I);
                           tempWriter.Write(color.G * color.I);
                           tempWriter.Write(color.B * color.I);
                           tempWriter.Write(color.A);
                        }
                        break;
                     case Helpers.kProperty_Type_bool:
                        tempWriter.Write(Tools.Common.Misc.ParseBool(propertyValue));
                        break;
                     case Helpers.kProperty_Type_float:
                        {
                           float value = 0.0f;
                           float.TryParse(propertyValue, out value);
                           tempWriter.Write(value);
                        }
                        break;
                     case Helpers.kProperty_Type_int:
                        {
                           int value = 0;
                           int.TryParse(propertyValue, out value);
                           tempWriter.Write(value);
                        }
                        break;
                     case Helpers.kProperty_Type_enum:
                        {
                           uint enumHash = CodeGenHelpers.GetStringHash(propertyValue);
                           tempWriter.Write(enumHash);
                        }
                        break;
                     case Helpers.kProperty_Type_string:
                        {
                           // Check the subType
                           string subType = propertyNode.GetAttribute(Helpers.kProperty_SubType, String.Empty);
                           switch (subType)
                           {
                              case Helpers.kProperty_SubType_asset:
                                 {
                                    // Check validity of asset
                                    if (!assetManager.IsValidRepositoryRelativePath( propertyValue, 
                                                                        propertyNode.GetAttribute(Helpers.kProperty_AssetTypes, String.Empty)))
                                    {
                                       // Nope, not valid, empty string
                                       tempWriter.Write(String.Empty);
                                       break;
                                    }

                                    AssetSystem.PackageAsset.EAssetSource assetSource = Tools.Common.Misc.ParseBool(propertyNode.GetAttribute(Helpers.kProperty_IsGeneratedAsset))
                                       ? AssetSystem.PackageAsset.EAssetSource.kGenerated : AssetSystem.PackageAsset.EAssetSource.kRepository;
                                    
                                    // Add as asset dependency is lockassetonload is not present or true
                                    AssetSystem.PackageAsset.EPackageType packageType = Tools.Common.Misc.ParseBool(propertyNode.GetAttribute(Helpers.kProperty_LockAssetOnLoad), true)
                                       ? AssetSystem.PackageAsset.EPackageType.kPackageAndLoad : AssetSystem.PackageAsset.EPackageType.kPackageOnly;

                                    cookOutput.AddPackageAsset(new AssetSystem.PackageAsset(propertyValue, packageType, assetSource));

                                    // Replace extension with cooked extension
                                    string cookedAssetPath = AssetSystem.Manager.GetCookedAssetPath(propertyValue);
                                    tempWriter.Write(cookedAssetPath);
                                 }
                                 break;
                              case Helpers.kProperty_SubType_hashedString:
                                 {
                                    uint stringHash = Helpers.GetSubTypeHashedStringValue(propertyValue);
                                    tempWriter.Write(stringHash);
                                 }
                                 break;
                              case Helpers.kProperty_SubType_messageStringHash:
                                 {
                                    uint stringHash = Helpers.GetMessageNameHash( propertyValue );
                                    tempWriter.Write( stringHash );
                                 }
                                 break;
                              case Helpers.kProperty_SubType_animationEventStringHash:
                                 {
                                    uint stringHash = Helpers.GetAnimationEventNameHash(propertyValue);
                                    tempWriter.Write(stringHash);
                                 }
                                 break;

                              default:    // Just write the string
                                 tempWriter.Write(propertyValue);
                                 break;
                           }
                        }
                        break;
                     case Helpers.kProperty_Type_guid:
                        tempWriter.Write(new System.Guid(propertyValue).ToByteArray());
                        break;
                     case Helpers.kProperty_Type_evaluatorContainer:
                        {
                           int containerCount = 0;
                           System.IO.MemoryStream tempContainerStream = new System.IO.MemoryStream();

                           if (propertyValueSubtree != String.Empty)
                           {
                              XmlDocument evaluatorContainerDoc = new XmlDocument();
                              evaluatorContainerDoc.LoadXml(propertyValueSubtree);
                              XmlNode evaluatorContainerRootNode = evaluatorContainerDoc.SelectSingleNode("//" + Helpers.kElement_EvaluatorContainerRoot);
                              if (evaluatorContainerRootNode != null)
                              {
                                 XmlNodeReader nodeReader = new XmlNodeReader(evaluatorContainerRootNode);
                                 nodeReader.Read(); // Move off root
                                 nodeReader.Read(); // Skip 'EvaluatorContainerRoot'
                                 BinaryWriterBigEndian tempContainerWriter = new BinaryWriterBigEndian(tempContainerStream);

                                 while (nodeReader.Name == Helpers.kElement_EvaluatorContainerEntry)
                                 {
                                    result = WriteEvaluatorContainer(assetManager, nodeReader, cookInfo, cookOutput, tempContainerWriter);
                                    containerCount++;
                                 }
                              }
                           }
                           tempWriter.Write((int) containerCount);
                           tempWriter.Write(tempContainerStream.ToArray());
                        }
                        break;
                     case Helpers.kProperty_Type_custom:
                        {
                           // Invoke delegate
                           string subType = propertyNode.GetAttribute(Helpers.kProperty_SubType) ?? "";
                           cookInfo.mCustomPropertyCooker(propertyNode, subType, propertyValueSubtree, tempWriter);
                        }
                        break;
                  }
                  result = EWriteResult.kWrotePropertyType;
               }
               break;
               case Helpers.kElement_PropertyObject:
                  result = WriteProperties(assetManager, propertyNode, cookInfo, cookOutput, propertyWriter, tempWriter);
                  break;
               case Helpers.kElement_MessageObject:
                  result = WriteProperties(assetManager, propertyNode, cookInfo, cookOutput, propertyWriter, tempWriter);
                  break;
               case Helpers.kElement_EvaluatorProperty:
                  result = WriteEvaluatorProperty(assetManager, propertyNode, cookInfo, cookOutput, propertyWriter, tempWriter);
                  break;
                  
               case Helpers.kElement_ComponentGroup:
               {
                  XmlDocument xmlDoc = new XmlDocument();
                  xmlDoc.LoadXml(propertyNode.ReadOuterXml());

                  XmlElement containedComponent = Helpers.GetActiveComponentFromComponentGroup(xmlDoc.DocumentElement);
                  if (containedComponent != null)
                  {
                     string noExportAttributeString = containedComponent.GetAttribute(Helpers.kProperty_NoExport);
                     if (Tools.Common.Misc.ParseBool(noExportAttributeString, false))
                     {
                        propertyNode.Skip();
                        return EWriteResult.kSkippedPropertyType;
                     }

                     if ( !DoesComponentCookOnPlatform( new Data.Component( containedComponent ), cookInfo.mPlatform ) )
                     {
                        propertyNode.Skip();

                        return EWriteResult.kSkippedPropertyType;
                     }
                  }

                  bool bChildComponent = (cookInfo.mComponentRecursionDepth > 0);
                  
                  if (!bChildComponent)
                  {
                     // Write 4CC for component group
                     uint idHash = CodeGenHelpers.GetStringHash(id);
                     propertyWriter.Write(idHash);
                  }
                  else
                  {
                     // Write name hash
                     uint nameHash = CodeGenHelpers.GetStringHash(name);
                     propertyWriter.Write(nameHash);
                  }

                  result = EWriteResult.kWrotePropertyType;

                  string activeComponentName = Helpers.kComponentGroup_DefaultComponent;
                  {
                     string res = Helpers.GetActiveComponentNameFromComponentGroup(Tools.Common.XmlNodeHelpers.GetDocumentRootElement( xmlDoc ) );
                     if (!string.IsNullOrEmpty(res))
                     {
                        activeComponentName = res;
                     }
                  }
                  
                  if (activeComponentName == Helpers.kComponentGroup_DefaultComponent ||
                      activeComponentName == "None" )
                  {
                     // Just write 'null' for component hash (will fall through to special case null component)
                     tempWriter.Write((uint)0);
                     tempWriter.Write((ushort)0);
                     break;
                  }

                  // Not a empty component, find active component element
                  XmlNode activeComponentNav = xmlDoc.SelectSingleNode(String.Format("./ComponentGroup/Component[@id=\"{0}\"]", activeComponentName));
                  if (activeComponentNav == null)
                  {
                     // Error?
                     // Just write 'null' for component hash (will fall through to special case null component)
                     tempWriter.Write((uint)0);
                     tempWriter.Write((ushort)0);
                     break;
                  }

                  // Write component
                  XmlDocument componentDoc = new XmlDocument();
                  componentDoc.LoadXml(activeComponentNav.OuterXml);
                  XmlNodeReader componentReader = new XmlNodeReader(Tools.Common.XmlNodeHelpers.GetDocumentRootElement( componentDoc ) );
                  componentReader.Read(); // Skip root

                  Guid componentGroupGuid = Helpers.GetInternalEditorIdGuid(xmlDoc.CreateNavigator());

                  WriteCookedProperties(assetManager, componentReader, cookInfo, cookOutput, tempStream, componentGroupGuid);
               }
               break;
               case Helpers.kElement_Component:
               {
                  // Write 4CC for component group
                  uint idHash = CodeGenHelpers.GetStringHash(id);
                  propertyWriter.Write(idHash);

                  XmlDocument xmlDoc = new XmlDocument();
                  xmlDoc.LoadXml(propertyNode.ReadOuterXml());

                  // Write Guid
                  // Write flag saying that we have a guid
                  tempWriter.Write(true);

                  Guid componentGuid = parentEditorId.Value;
                  tempWriter.Write(componentGuid.ToByteArray());

                  XmlNodeReader nodeReader = new XmlNodeReader(xmlDoc);
                  nodeReader.Read();   // Skip 'Component'
                  result = EWriteResult.kWrotePropertyType;
                  System.IO.MemoryStream tempComponentStream = new System.IO.MemoryStream();
                  ushort numProperties = 0;
                  using (BinaryWriterBigEndian tempComponentWriter = new BinaryWriterBigEndian(tempComponentStream))
                  {
                     EWriteResult iterateResult = EWriteResult.kIgnoredElement;
                     // Iterate over properties
                     while ((iterateResult != EWriteResult.kIgnoredElement) || nodeReader.Read())
                     {
                        iterateResult = EWriteResult.kIgnoredElement;
                        if (nodeReader.NodeType != XmlNodeType.Element)
                        {
                           // Only interested in elements
                           continue;
                        }
                        // Skip over events and links
                        if ((nodeReader.Name == Helpers.kElement_Events) || (nodeReader.Name == Helpers.kElement_Links))
                        {
                           nodeReader.Skip();
                           iterateResult = EWriteResult.kSkippedPropertyType;
                           continue;
                        }
                        cookInfo.mComponentRecursionDepth++;
                        iterateResult = WriteCookedProperties(assetManager, nodeReader, cookInfo, cookOutput, tempComponentStream, null);
                        cookInfo.mComponentRecursionDepth--;
                        if (iterateResult == EWriteResult.kWrotePropertyType)
                        {
                           numProperties++;
                        }
                     }
                     tempWriter.Write(numProperties);
                  }
                  tempWriter.Write(tempComponentStream.ToArray());
                  // Write messages and links
                  if (cookInfo.mComponentRecursionDepth == 0)
                  {
                     // Only top level components have messages and links
                     WriteMessagesAndLinks(assetManager, xmlDoc.CreateNavigator(), cookInfo, cookOutput, tempStream);
                  }
               }
               break;

            }

            // Write size first
            ushort propertySize = (ushort) (tempStream.Length);
            propertyWriter.Write(propertySize);

            // Now write property values
            propertyWriter.Write(tempStream.ToArray());
         }

         return result;
      }

      /// <summary>
      /// Writes cooked properties for 'PropertyObject' or 'MessageObject'
      /// </summary>
      private static EWriteResult WriteProperties( AssetSystem.Manager assetManager,
                                                   XmlNodeReader propertyNode, 
                                                   CookInfo cookInfo,
                                                   CookOutput cookOutput,
                                                   BinaryWriterBigEndian propertyWriter, 
                                                   BinaryWriterBigEndian tempWriter)
      {
         EWriteResult result = EWriteResult.kIgnoredElement;
         // Check to see if it's a message object or property object
         switch (propertyNode.Name)
         {
            case Helpers.kElement_MessageObject:
               // write message fourCC
               uint fourCC;
               string cookAs = propertyNode.GetAttribute(Helpers.kMessage_cookAs, "");
               if (cookAs != null)
               {
                  fourCC = Tools.Common.Misc.GetFourCCFromString(cookAs);
               }
               else
               {
                  string messageObjectId = propertyNode.GetAttribute(Helpers.kMessage_id, "");
                  fourCC = CodeGenHelpers.GetPropertyNameHash(messageObjectId);
               }
               // Check to see if it's an empty message object
               if (!Helpers.MessageObjectHasProperties(propertyNode))
               {
                  // Write special marker to signify an empty message object
                  propertyWriter.Write(0xFFFFFFFF);
                  // Write hash afterwards
                  tempWriter.Write(fourCC);
                  return EWriteResult.kIgnoredElement;
               }

               propertyWriter.Write(fourCC);
               break;
            case Helpers.kElement_PropertyObject:
               // Write name hash
               string name = propertyNode.GetAttribute(Helpers.kProperty_Name, "");
               // Write 4CC
               uint nameHash = CodeGenHelpers.GetStringHash(name);
               propertyWriter.Write(nameHash);
               break;
         }

         string propertyCookType = CodeGenHelpers.GetPropertyObjectCookType(propertyNode);
         switch (propertyCookType)
         {
            case Helpers.kProperty_Type_vector3:
               {
                  float[] xyz;
                  string propertyValue = Helpers.GetPropertyValueElementsSpaceDelimited(propertyNode, 3);
                  Helpers.GetFloatValues(propertyValue, 3, out xyz);
                  tempWriter.Write(xyz[0]);
                  tempWriter.Write(xyz[1]);
                  tempWriter.Write(xyz[2]);
                  result = EWriteResult.kWrotePropertyType;
               }
               break;

            case Helpers.kProperty_Type_matrix34:
               {
                  float[] matrix;
                  const int numComponents = 3 * 4;
                  string propertyValue = Helpers.GetPropertyValueElementsSpaceDelimited(propertyNode, numComponents);
                  Helpers.GetFloatValues(propertyValue, numComponents, out matrix);
                  for (int i = 0; i < numComponents; ++i)
                  {
                     tempWriter.Write(matrix[i]);
                  }
                  result = EWriteResult.kWrotePropertyType;

               }
               break;

            default:
               {
                  // Recursively iterate over properties
                  XmlDocument xmlDoc = new XmlDocument();
                  xmlDoc.LoadXml(propertyNode.ReadOuterXml());
                  XmlNodeReader nodeReader = new XmlNodeReader(xmlDoc);
                  nodeReader.Read();   // Skip 'PropertyObject'
                  result = EWriteResult.kWrotePropertyType;
                  System.IO.MemoryStream tempPropertyObjectStream = new System.IO.MemoryStream();
                  using (BinaryWriterBigEndian tempPropertyObjectWriter = new BinaryWriterBigEndian(tempPropertyObjectStream))
                  {
                     ushort numProperties = 0;
                     EWriteResult iterateResult = EWriteResult.kIgnoredElement;
                     while ((iterateResult != EWriteResult.kIgnoredElement) || nodeReader.Read())
                     {
                        iterateResult = EWriteResult.kIgnoredElement;
                        if (nodeReader.NodeType != XmlNodeType.Element)
                        {
                           // Only interested in elements
                           continue;
                        }
                        iterateResult = WriteCookedProperties( assetManager, nodeReader, cookInfo, cookOutput, tempPropertyObjectStream, null );
                        if (iterateResult == EWriteResult.kWrotePropertyType)
                        {
                           numProperties++;
                        }
                     }
                     tempWriter.Write(numProperties);
                  }
                  tempWriter.Write(tempPropertyObjectStream.ToArray());
               }
               break;
         }
         return result;
      }
      
      /// <summary>
      /// Writes 'EvaluatorProperty'
      /// </summary>
      private static EWriteResult WriteEvaluatorProperty(AssetSystem.Manager assetManager,
                                                         XmlNodeReader propertyNode, 
                                                         CookInfo cookInfo,
                                                         CookOutput cookOutput,
                                                         BinaryWriterBigEndian propertyWriter, 
                                                         BinaryWriterBigEndian tempWriter)
      {
         System.Diagnostics.Debug.Assert(propertyNode.Name == Helpers.kElement_EvaluatorProperty);
         
         // Get name before reading inner xml
         string name = propertyNode.GetAttribute(Helpers.kEvaluatorProperty_Name, "");
         
         // Write name hash
         // Write evaluator property 4CC
         {
            uint nameHash = CodeGenHelpers.GetStringHash(name);
            propertyWriter.Write(nameHash);
         }
         
         // Check for empty evaluator.
         bool isNull =  false;
         if (!propertyNode.ReadToDescendant(Helpers.kElement_Evaluator))
         {
            isNull = true;
         }
         else if (propertyNode.GetAttribute(Helpers.kEvaluator_Id, String.Empty) == Helpers.kEmptyEvaluatorName)
         {
            // 'None' evaluator
            isNull = true;
         }
         
         if (isNull)
         {
            // Null evaluator
            tempWriter.Write(0);
            // Skip to next element
            propertyNode.Read();
            return EWriteResult.kWrotePropertyType;
         }
         
         WriteEvaluator(assetManager, propertyNode, cookInfo, cookOutput, tempWriter);
         
         return EWriteResult.kWrotePropertyType;
      }
      
      private static EWriteResult WriteEvaluatorContainer(  AssetSystem.Manager assetManager,
                                                            XmlNodeReader propertyNode, 
                                                            CookInfo cookInfo,
                                                            CookOutput cookOutput,
                                                            BinaryWriterBigEndian tempWriter)
      {
         XmlDocument xmlDoc = new XmlDocument();
         xmlDoc.LoadXml(propertyNode.ReadOuterXml());
         XmlNodeReader nodeReader = new XmlNodeReader(xmlDoc);
         
         nodeReader.Read();   // Move to 'EvaluatorContainerEntry'
         System.Diagnostics.Debug.Assert(nodeReader.Name == Helpers.kElement_EvaluatorContainerEntry);
         string targetPropertiesString = nodeReader.GetAttribute(Helpers.kEvaluatorContainerEntry_targetProperty);
         
         EEvaluatorTypes type = EEvaluatorTypes.kInvalid;
         XmlNodeReader evaluatorNode = null;
         
         List<Guid> targetGuids = new List<Guid>();         
         bool bSkipped = false;
         while (bSkipped || nodeReader.Read())
         {
            bSkipped = false;
            if (nodeReader.NodeType != XmlNodeType.Element)
            {
               // Only interested in elements
               continue;
            }
            switch (nodeReader.Name)
            {
               case Helpers.kElement_TargetGuid:
                  targetGuids.Add(new Guid(nodeReader.ReadInnerXml()));
                  bSkipped = true;
                  break;
               case Helpers.kElement_Evaluator:
                  {
                     XmlDocument evaluatorXmlDoc = new XmlDocument();
                     evaluatorXmlDoc.LoadXml(nodeReader.ReadOuterXml());
                     evaluatorNode = new XmlNodeReader(evaluatorXmlDoc);
                     evaluatorNode.Read();   // Skip root
                     bSkipped = true;
                  }
                  break;
            }
         }
         // Get type
         int typeSize = 0;
         switch (evaluatorNode.GetAttribute(Helpers.kEvaluatorProperty_Type))
         {
            case Helpers.kEvaluatorProperty_Type_float:
               type = EEvaluatorTypes.kFloat;
               typeSize = sizeof(float);
               break;
            case Helpers.kEvaluatorProperty_Type_bool:
               type = EEvaluatorTypes.kBool;
               typeSize = 1;
               break;
            case Helpers.kEvaluatorProperty_Type_vector3:
               type = EEvaluatorTypes.kVector;
               typeSize = sizeof(float) * 3;
               break;
            case Helpers.kEvaluatorProperty_Type_color:
               type = EEvaluatorTypes.kColor;
               typeSize = sizeof(float) * 4;
               break;
            default:
               System.Diagnostics.Debug.Assert(false, "Unknown evaluator type");
               break;
         }
         
         // Write data
         // Type
         tempWriter.Write((uint)type);
         
         // Target components
         tempWriter.Write(targetGuids.Count);
         foreach( Guid target in targetGuids) tempWriter.Write(target.ToByteArray());
         
         // Target properties
         string kPX_ = "PX_";
         System.Diagnostics.Debug.Assert(targetPropertiesString.StartsWith(kPX_));
         List<string> targetProperties = new List<string>(targetPropertiesString.Split('_'));
         targetProperties.RemoveRange(0, 2); // erase "PX" and component name

         const int kPerPropertySizeOverhead = sizeof(ushort) + sizeof(int) + sizeof(ushort); // Count + hash + size for each property
         int totalSize = typeSize + sizeof(bool) + (targetProperties.Count * kPerPropertySizeOverhead);   
         tempWriter.Write(totalSize);
         tempWriter.Write(false);   // No editor id guid
         int currentSize = totalSize - 1; // Minus 1 because the bool for "IsEditorIdPresent" has already been written.
         foreach(string targetProperty in targetProperties)
         {
            currentSize -= kPerPropertySizeOverhead;
            tempWriter.Write((ushort)1);  // num properties              
            tempWriter.Write(CodeGenHelpers.GetStringHash(targetProperty));  // hash
            tempWriter.Write((ushort) currentSize);
         }
         // Pad target properties with type size
         for (int loop = 0; loop < typeSize; loop++) tempWriter.Write((byte)0);
         
         // Finally, the evaluator itself
         System.IO.MemoryStream tempEvaluatorStream = new System.IO.MemoryStream();
         using (BinaryWriterBigEndian tempEvaluatorWriter = new BinaryWriterBigEndian(tempEvaluatorStream))
         {
            WriteEvaluator(assetManager, evaluatorNode, cookInfo, cookOutput, tempEvaluatorWriter);

            // Write number of bytes associated with evaluator
            tempWriter.Write((ushort)tempEvaluatorStream.Length);
            tempWriter.Write(tempEvaluatorStream.ToArray());
         }
         
         return EWriteResult.kWrotePropertyType;
      }
      
      
      private static void WriteEvaluator( AssetSystem.Manager assetManager,
                                          XmlNodeReader propertyNode, 
                                          CookInfo cookInfo,
                                          CookOutput cookOutput,
                                          BinaryWriterBigEndian tempWriter)
      {
         // Recursively iterate over properties
         XmlDocument xmlDoc = new XmlDocument();
         xmlDoc.LoadXml(propertyNode.ReadOuterXml());
         XmlNodeReader nodeReader = new XmlNodeReader(xmlDoc);
         nodeReader.Read();   // Move to 'Evaluator'
         System.Diagnostics.Debug.Assert(nodeReader.Name == Helpers.kElement_Evaluator);
         {
            // Write evaluator 4CC, note that evaluators don't write a size, however implicit size known ('property size' - 4)
            uint nameHash = CodeGenHelpers.GetStringHash(nodeReader.GetAttribute(Helpers.kEvaluator_Id));
            tempWriter.Write(nameHash);
         }
         
         System.IO.MemoryStream tempPropertyObjectStream = new System.IO.MemoryStream();
         using (BinaryWriterBigEndian tempPropertyObjectWriter = new BinaryWriterBigEndian(tempPropertyObjectStream))
         {
            ushort numProperties = 0;
            EWriteResult iterateResult = EWriteResult.kIgnoredElement;
            while ((iterateResult != EWriteResult.kIgnoredElement) || nodeReader.Read())
            {
               iterateResult = EWriteResult.kIgnoredElement;
               if (nodeReader.NodeType != XmlNodeType.Element)
               {
                  // Only interested in elements
                  continue;
               }
               iterateResult = WriteCookedProperties( assetManager, nodeReader, cookInfo, cookOutput, tempPropertyObjectStream, null );
               if (iterateResult == EWriteResult.kWrotePropertyType)
               {
                  numProperties++;
               }
            }
            tempWriter.Write(numProperties);
         }
         tempWriter.Write(tempPropertyObjectStream.ToArray());
         
         return;
      }

      /// <summary>
      ///  Sort events/links by event type, time, then priority.
      /// </summary>
      static int CompareEventLink(XPathNavigator left, XPathNavigator right)
      {
         // Event type
         XPathNavigator leftTypeNav = left.Clone();
         leftTypeNav.MoveToParent();
         uint leftEventFourCC = GetCookedNameHash(leftTypeNav.GetAttribute(Helpers.kEvent_type, ""), leftTypeNav);

         XPathNavigator rightTypeNav = left.Clone();
         rightTypeNav.MoveToParent();
         uint rightEventFourCC = GetCookedNameHash(rightTypeNav.GetAttribute(Helpers.kEvent_type, ""), rightTypeNav);
         if (leftEventFourCC != rightEventFourCC)
         {
            return (int)(leftEventFourCC - rightEventFourCC);
         }
         // Time
         float leftTime = float.Parse(left.GetAttribute(Helpers.kEventEntry_time, ""));
         float rightTime = float.Parse(right.GetAttribute(Helpers.kEventEntry_time, ""));
         if (leftTime != rightTime)
         {
            return ((leftTime - rightTime) < 0) ? -1 : 1;
         }

         // Priority
         return int.Parse(left.GetAttribute(Helpers.kEventEntry_priority, "")) - int.Parse(right.GetAttribute(Helpers.kEventEntry_priority, "")); 
      }

      static void WriteMessagesAndLinks(  AssetSystem.Manager assetManager,
                                          XPathNavigator componentNav, 
                                          CookInfo cookInfo,
                                          CookOutput cookOutput,
                                          System.IO.Stream outputStream)
      {
         BinaryWriterBigEndian outputWriter = new BinaryWriterBigEndian(outputStream);
         
         // Get all active events
         List<XPathNavigator> allEvents = new List<XPathNavigator>();
         XPathNodeIterator eventsIter = componentNav.Select("/Component/Events/Event/EventEntry");
         while (eventsIter.MoveNext())
         {
            XPathNavigator eventNav = eventsIter.Current;
            bool bActive = Tools.Common.Misc.ParseBool(eventNav.GetAttribute(Helpers.kEventEntry_active, ""));
            if (bActive)
            {
               allEvents.Add(eventNav.Clone());
            }
         }

         allEvents.Sort(CompareEventLink);

         // Write sorted events
         // Num events
         outputWriter.Write(allEvents.Count);
         foreach(XPathNavigator eventNav in allEvents)
         {
            // Get fourCC
            XPathNavigator eventTypeNav = eventNav.Clone();
            eventTypeNav.MoveToParent();
            uint eventFourCC = GetCookedNameHash(eventTypeNav.GetAttribute(Helpers.kEvent_type, ""), eventTypeNav);
            outputWriter.Write(eventFourCC);
            Guid target = new Guid(eventNav.GetAttribute(Helpers.kEventEntry_target, ""));
            outputWriter.Write(target.ToByteArray());
            float triggerTime = float.Parse(eventNav.GetAttribute(Helpers.kEventEntry_time, ""));
            outputWriter.Write(triggerTime);
            int priority = int.Parse(eventNav.GetAttribute(Helpers.kLinkEntry_priority, ""));
            outputWriter.Write(priority);
            outputWriter.Write(eventNav.GetAttribute(Helpers.kEventEntry_senderQuery, String.Empty));
            outputWriter.Write(eventNav.GetAttribute(Helpers.kEventEntry_destQuery, String.Empty));
            outputWriter.Flush();

            // Now write message properties
            XmlDocument messagesDoc = new XmlDocument();
            messagesDoc.LoadXml(eventNav.OuterXml);
            XmlNodeReader messageNodeReader = new XmlNodeReader(Tools.Common.XmlNodeHelpers.GetDocumentRootElement( messagesDoc ) );
            messageNodeReader.Read(); // Move off the root
            messageNodeReader.Read(); // Move off the evententry

            // Write event message properties
            WriteCookedProperties( assetManager, messageNodeReader, cookInfo, cookOutput, outputStream, null );
         }

         // Get all active links
         List<XPathNavigator> allLinks = new List<XPathNavigator>();
         XPathNodeIterator linksIter = componentNav.Select("/Component/Links/Link/LinkEntry");
         while (linksIter.MoveNext())
         {
            XPathNavigator linkNav = linksIter.Current;
            bool bActive = Tools.Common.Misc.ParseBool(linkNav.GetAttribute(Helpers.kLinkEntry_active, ""));
            if (bActive)
            {
               allLinks.Add(linkNav.Clone());
            }
         }

         allLinks.Sort(CompareEventLink);

         // Write sorted events
         // Num links
         outputWriter.Write(allLinks.Count);
         foreach(XPathNavigator linkNav in allLinks)
         {
            // Get fourCC
            XPathNavigator linkTypeNav = linkNav.Clone();
            linkTypeNav.MoveToParent();
            uint linkFourCC = GetCookedNameHash(linkTypeNav.GetAttribute(Helpers.kLink_type, ""), linkTypeNav);
            outputWriter.Write(linkFourCC);
            Guid target = new Guid(linkNav.GetAttribute(Helpers.kLinkEntry_target, ""));
            outputWriter.Write(target.ToByteArray());
            float triggerTime = float.Parse(linkNav.GetAttribute(Helpers.kLinkEntry_time, ""));
            outputWriter.Write(triggerTime);
            int priority = int.Parse(linkNav.GetAttribute(Helpers.kLinkEntry_priority, ""));
            outputWriter.Write(priority);
            outputWriter.Write(linkNav.GetAttribute(Helpers.kLinkEntry_senderQuery, String.Empty));
            outputWriter.Write(linkNav.GetAttribute(Helpers.kLinkEntry_destQuery, String.Empty));
            outputWriter.Flush();

            // Now write message properties
            XmlDocument messagesDoc = new XmlDocument();
            messagesDoc.LoadXml(linkNav.OuterXml);
            XmlNodeReader messageNodeReader = new XmlNodeReader(Tools.Common.XmlNodeHelpers.GetDocumentRootElement( messagesDoc ) );
            messageNodeReader.Read(); // Move off the root
            messageNodeReader.Read(); // Move off the linkentry

            // Write link message properties
            WriteCookedProperties(assetManager, messageNodeReader, cookInfo, cookOutput, outputStream, null);
         }

      }
      
      /// <summary>
      /// Writes binary component properties for one merged script object.
      /// </summary>
      /// <param name="scriptObject">Navigator assumed to be pointing at the property container.</param>
      public static void WriteCookedPropertiesForPropertyContainer(  AssetSystem.Manager assetManager,
                                                                     XmlElement scriptObject, 
                                                                     CookInfo cookInfo,
                                                                     CookOutput cookOutput,
                                                                     System.IO.Stream outputStream)
      {
         // Add bitflags
         Helpers.AddBitFieldProperties(scriptObject);

         // Invoke property modifier delegate
         if (cookInfo.mScriptObjectPropertiesModifier != null )
         {
            cookInfo.mScriptObjectPropertiesModifier(scriptObject, assetManager, cookInfo, cookOutput );
         }

         // Invoke component group properties modifier
         // This allows us to update special case properties for components (such as custom transform in RGC) before cook.
         if (cookInfo.mComponentGroupPropertiesModifier != null)
         {
            foreach (XmlElement nav in scriptObject.SelectNodes("./ComponentGroup"))
            {
               cookInfo.mComponentGroupPropertiesModifier(nav, assetManager, cookInfo);
            }
         }

         // Create binary writer
         BinaryWriterBigEndian outputWriter = new BinaryWriterBigEndian(outputStream);

         // Version
         outputWriter.Write(kVersion);

         // Write script object editor id
         Guid propertyContainerGuid = Helpers.GetInternalEditorIdGuid(scriptObject);
         outputWriter.Write(propertyContainerGuid.ToByteArray());
         
         // Write name
         string scriptObjectName = scriptObject.GetAttribute(Helpers.kPropertyContainer_Name, String.Empty);
         outputWriter.Write(scriptObjectName);

         {
            // Flags
            EGameObjectPropertiesFlags cookFlags = EGameObjectPropertiesFlags.kNone;
            // Set create on load
            string createOnPropertiesLoad = Helpers.GetValueAtLabel(scriptObject, Helpers.kLabel_create_on_properties_load);
            if ((createOnPropertiesLoad != null) && !Tools.Common.Misc.ParseBool(createOnPropertiesLoad))
            {
               cookFlags |= EGameObjectPropertiesFlags.kDontCreateOnLoad;
            }
            // Write flags (as uint)
            outputWriter.Write((uint) cookFlags);
         }

         // We're going ignore all non-component properties below scriptobject except component groups
         System.Collections.IEnumerator componentGroupIter = scriptObject.SelectNodes("./ComponentGroup").GetEnumerator();
         int numComponents = 0;
         System.IO.MemoryStream tempStream = new System.IO.MemoryStream();
         using (BinaryWriterBigEndian tempWriter = new BinaryWriterBigEndian(tempStream))
         {
            while (componentGroupIter.MoveNext())
            {
               XmlElement componentGroupNav = (XmlElement) componentGroupIter.Current;
               XmlNodeReader componentGroupNodeReader = new XmlNodeReader(componentGroupNav);
               componentGroupNodeReader.Read(); // Move off the root
               if (WriteCookedProperties(assetManager, componentGroupNodeReader, cookInfo, cookOutput, tempStream, null) == EWriteResult.kWrotePropertyType)
               {
                  numComponents++;
               }
            }
         }
         outputWriter.Write(numComponents);
         outputWriter.Write(tempStream.ToArray());
      }

      private static List<XmlElement> GetSubPropertyContainersForNode(XmlNode nodeNav)
      {
         List<XmlElement> ret = new List<XmlElement>();
         foreach ( XmlElement subNode in nodeNav.SelectNodes( "Node[Properties/PropertyContainer or Node]" ) )
         {
            // If there are any property containers in this node, then we're done.  Add it and move on
            XmlElement subContainer = (XmlElement) subNode.SelectSingleNode( "Properties/PropertyContainer" );
            if (subContainer != null)
            {
               ret.Add(subContainer);
            }
            else
            {
               // If this node did not have sub-properties, then take its children and flatten them upwards
               ret.AddRange(GetSubPropertyContainersForNode(subNode));
            }
         }

         return ret;
      }

      private static bool WriteCookedPropertyContainerPropertiesForSceneNode(AssetSystem.Manager assetManager,
                                                                             XmlNode sceneNodeNav, 
                                                                             IPropertyDataSource propertySource,
                                                                             CookInfo cookInfo,
                                                                             CookOutput cookOutput, 
                                                                             System.IO.Stream outputStream)
      {
         XmlElement fullScriptObjectNav = (XmlElement) sceneNodeNav.SelectSingleNode("./Properties/PropertyContainer");
         if (fullScriptObjectNav == null)
            return false;

         // Cook
         WriteCookedPropertiesForPropertyContainer( assetManager, fullScriptObjectNav, cookInfo, cookOutput, outputStream );
         AddPropertyDependencies( assetManager, propertySource, fullScriptObjectNav, cookInfo, cookOutput );
         
         // Now find and write child objects
         int numChildObjects = 0;
         System.IO.MemoryStream tempStream = new System.IO.MemoryStream();

         foreach ( XmlElement childNav in GetSubPropertyContainersForNode( sceneNodeNav ) )
         {
            // Move back up to the 'Node' element
            XmlElement nodeElement = (XmlElement) childNav.ParentNode.ParentNode;
            bool result = WriteCookedPropertyContainerPropertiesForSceneNode(assetManager,
                                                                        nodeElement,
                                                                        propertySource,
                                                                        cookInfo,
                                                                        cookOutput,
                                                                        tempStream);
            if (result) numChildObjects++;                                                                        
         }
         
         BinaryWriterBigEndian tempWriter = new BinaryWriterBigEndian(outputStream);
         tempWriter.Write(numChildObjects);
         // Child objects
         tempWriter.Write(tempStream.ToArray());

         return true;
      }

      /// <summary>
      /// This function will add all the script type dependencies used by the script object to the cook info structure.
      /// </summary>
      public static void AddPropertyDependencies(Tools.AssetSystem.Manager assetManager, IPropertyDataSource propertySource, XmlNode fullScriptObject, CookInfo cookInfo, CookOutput cookOutput )
      {
         string dependencyQuery = ".//PropertyContainer | .//PropertyObject | .//Component | .//Evaluator | .//MessageObject";
         foreach (XmlElement node in fullScriptObject.SelectNodes(dependencyQuery))
         {
            XPathNavigator sourceTypeNavigator = null;
            switch(node.Name)
            {
               case Helpers.kElement_PropertyContainer:
                  sourceTypeNavigator = propertySource.FindPropertyContainerById(node.GetAttribute(Helpers.kUsing_Attribute_Id));
                  break;

               case Helpers.kElement_PropertyObject:
                  sourceTypeNavigator = propertySource.FindPropertyObjectById(node.GetAttribute(Helpers.kUsing_Attribute_Id));
                  break;

               case Helpers.kElement_Component:
                  {
                     string activeComponent = Helpers.GetActiveComponentNameFromComponentGroup((XmlElement) node.ParentNode);
                     string componentId = node.GetAttribute(Helpers.kComponent_Id);
                     if( componentId == activeComponent )
                     {
                        sourceTypeNavigator = propertySource.FindComponentByGroupAndId(node.GetAttribute(Helpers.kComponent_Group) + ":" + componentId);
                     }
                  }
                  break;

               case Helpers.kElement_Evaluator:
                  sourceTypeNavigator = propertySource.FindEvaluatorById(node.GetAttribute(Helpers.kEvaluator_Id));
                  break;

               case Helpers.kElement_MessageObject:
                  sourceTypeNavigator = propertySource.FindMessageObjectById(node.GetAttribute(Helpers.kUsing_Attribute_Id));
                  break;
            }

            if( sourceTypeNavigator != null )
            {
               string sourceTypePath = propertySource.GetFilePathForNavigator(sourceTypeNavigator);
               string sourceTypeAssetPath = assetManager.GetRepositoryRelativePath(sourceTypePath);

               Tools.AssetSystem.BuildAsset buildDependency = Tools.AssetSystem.BuildAsset.StandardCookingAsset("ScriptTypeDependency", sourceTypeAssetPath);
               cookOutput.AddBuildAsset( buildDependency );
            }
         }
      }
      
      /// <summary>
      /// Given an open3d scene, this will write all the script objects to a binary stream.
      /// Simple implementation for now, writes zero for the number of child objects.
      /// It will ignore all non-script objects below 'property' elements.
      /// </summary>
      /// <param name="assetManager"></param>
      /// <param name="open3dScene"></param>
      /// <param name="propertySource"></param>
      /// <param name="outputStream"></param>
      public static void WriteCookedPropertyContainerPropertiesForScene(AssetSystem.Manager assetManager,
                                                                        XmlDocument open3dScene, 
                                                                        IPropertyDataSource propertySource,
                                                                        CookInfo cookInfo,
                                                                        CookOutput cookOutput,
                                                                        System.IO.Stream outputStream)
      {
         // Clone the scene so we don't change the original
         XmlDocument clonedScene = (XmlDocument) open3dScene.CloneNode( true );

         // Find all script objects
         int numScriptObjects = 0;

         System.IO.MemoryStream tempStream = new System.IO.MemoryStream();
         foreach (XmlElement scriptObjectNav in GetSubPropertyContainersForNode(clonedScene.SelectSingleNode("Open3d")))
         {
            // Check to see if it's the top level script object, not a child of a script object.
            XmlElement nodeElement = (XmlElement) scriptObjectNav.ParentNode.ParentNode;
            bool result = WriteCookedPropertyContainerPropertiesForSceneNode(assetManager,
                                                                             nodeElement,
                                                                             propertySource,
                                                                             cookInfo,
                                                                             cookOutput,
                                                                             tempStream);
            if (result) numScriptObjects++;                                                                        
         }

         BinaryWriterBigEndian outputWriter = new BinaryWriterBigEndian(outputStream);

         // Write a simple header
         uint kHeaderFourCC = Tools.Common.Misc.GetFourCCFromString("SSOP");
         outputWriter.Write(kHeaderFourCC);

         // Num script objects
         outputWriter.Write(numScriptObjects);

         // Script object data
         outputWriter.Write(tempStream.ToArray());
      }

      /// <summary>
      /// Returns fourCC of cookAs attribute or hash of name if cookAs fourCC doesn't exist on element.
      /// </summary>
      /// <param name="name">string to be hashed if cookAs doesn't exist</param>
      /// <param name="elementNav">Element contiaining optional cookAs attribute.</param>
      /// <returns>Cooked fourCC id</returns>
      private static uint GetCookedNameHash(string name, XPathNavigator elementNav)
      {
         uint fourCC = 0;
         string cookAs = elementNav.GetAttribute(Helpers.kMessage_cookAs, "");
         if (cookAs.Length == 0)
         {
            fourCC = CodeGenHelpers.GetPropertyNameHash(name);
         }
         else
         {
            fourCC = Tools.Common.Misc.GetFourCCFromString(cookAs);
         }
         return fourCC;
      }
   }
}
