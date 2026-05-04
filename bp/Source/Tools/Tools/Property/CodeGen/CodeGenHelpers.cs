using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Xml.XPath;
using Tools.Property.UI;
using Tools.Property.Build;
using Tools.Property.Misc;
using Tools.Common;

namespace Tools.Property.CodeGen
{
   public class CodeGenHelpers
   {
      readonly static Encoding skEncoder = Encoding.GetEncoding(437);  // Standard US OEM (IBM437 - OEM United States)
      public readonly static string kApplyPropertiesArguments = "(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)";
      public readonly static string kUnrefApplyPropertiesArguments = "(allocator); (applyProperties);";

      /// <summary>
      /// Gets uint hash for property name.
      /// </summary>
      public static uint GetPropertyNameHash(string propertyName)
      {  
         // Just in case we want to use different hashing schemes later
         return GetStringHash(propertyName);
      }
      /// <summary>
      /// Calculates CRC hash for string.
      /// </summary>
      /// <param name="val">
      /// String to be hashed.
      /// </param>
      /// <returns>
      /// 32 bit hash
      /// </returns>
      public static uint GetStringHash(string val)
      {  
         uint hash = CRC32.CalculateCRC(val);
         return hash;
      }

      public static string GetPropertiesClassName(string srcName, MergeProperties.EBuildTreeSrcType type)
      {
         switch (type)
         {
            case MergeProperties.EBuildTreeSrcType.kPropertyObject:
               return "CPOLdr" + srcName;
            case MergeProperties.EBuildTreeSrcType.kMessageObject:
               return "CMOLdr" + srcName;
            case MergeProperties.EBuildTreeSrcType.kComponent:
               System.Diagnostics.Debug.Assert(false, "Use GetComponentClassName instead.");
               break;
            case MergeProperties.EBuildTreeSrcType.kComponentGroup:
               return "ICGLdr" + srcName;
            case MergeProperties.EBuildTreeSrcType.kPropertyContainer:
               return "CSOLdr" + srcName;
         }
         return "UnknownType" + srcName;
      }

      public static string GetComponentPropertiesClassName(string componentGroupType, string componentId)
      {
         return "CCPLdr" + componentGroupType + componentId;
      }

      public static string GetComponentClassName(string componentGroupType, string componentId)
      {
         return "CComponent" + componentGroupType + componentId;
      }
      
      public static string GetComponentGroupClassName(string componentGroupType)
      {
         return "CComponent" + componentGroupType;
      }

      public static void AddFunctionSeparator( StringBuilder builder )
      {
         builder.AppendLine( "//----------------------------------------------------------------------------" );
         builder.AppendLine();
      }

      public static void AddTabs(StringBuilder builder, int numTabs)
      {
         builder.Append(' ', numTabs * 3);
      }

      /// <summary>
      /// Gets 'type' attribute from node. Native type will be replace by 'cookas' type if present.
      /// </summary>
      /// <param name="propertyNode"></param>
      /// <returns>
      /// Cook type.
      /// </returns>
      public static string GetCookType(XmlNodeReader propertyNode)
      {
         string type = propertyNode.GetAttribute(Helpers.kProperty_Type, "");
         // 'cookas' overrides native type.
         // See comment on kProperty_CookAs.
         string cookas = propertyNode.GetAttribute(Helpers.kProperty_CookAs, "");
         if (cookas != null)
         {
            type = cookas;
         }
         return type;
      }

      /// <summary>
      /// Gets 'type' attribute from node. Native type will be replace by 'cookas' type if present.
      /// </summary>
      /// <param name="propertyNode"></param>
      /// <returns>
      /// Cook type.
      /// </returns>
      public static string GetPropertyObjectCookType(XmlNodeReader propertyNode)
      {
         string id = propertyNode.GetAttribute(Helpers.kProperty_Id, "");
         // 'cookas' overrides native type.
         // See comment on kProperty_CookAs.
         string cookas = propertyNode.GetAttribute(Helpers.kProperty_CookAs, "");
         if (cookas != null)
         {
            id = cookas;
         }
         return id;
      }

      public static string PropertyTypeToCppType(string propertyType, string subType)
      {
         switch (propertyType)
         {
            case Helpers.kProperty_Type_bool:
               return "bool";
            case Helpers.kProperty_Type_int:
               return "int";
            case Helpers.kProperty_Type_float:
               return "float";
            case Helpers.kProperty_Type_vector3:
               return "CVector3";
            case Helpers.kProperty_Type_matrix34:
               return "CMatrix34";
            case Helpers.kProperty_Type_color:
               return "CColorf";
            case Helpers.kProperty_Type_string:
               switch (subType)
               {
               case Helpers.kProperty_SubType_hashedString:
               case Helpers.kProperty_SubType_messageStringHash:
               case Helpers.kProperty_SubType_animationEventStringHash:
                  return "uint32";
               default:
                  return "std::string";
               }
            case Helpers.kProperty_Type_guid:
               return "CGuid";
            case Helpers.kProperty_Type_evaluatorContainer:
               return "CEvaluatorContainerProperty";
            case Helpers.kProperty_Type_custom:
               return string.Format("{0}", subType);
         }
         // Just use the type passed in
         return propertyType;
      }

      public static string GetCppPropertyDeclaration(string name, string type, string subType)
      {
         return String.Format("{0,-45}   m{1};", CodeGenHelpers.PropertyTypeToCppType(type, subType), name);
      }

      public static string GetCppPropertyObjectDeclaration(string name, string type)
      {
         string classType = String.Empty;

         // Process special case
         switch (type)
         {
            case Helpers.kProperty_Type_vector3:
               classType = "CVector3";
               break;
            case Helpers.kProperty_Type_matrix34:
               classType = "CMatrix34";
               break;
            default:
               classType = CodeGenHelpers.GetPropertiesClassName(type , MergeProperties.EBuildTreeSrcType.kPropertyObject);
               break;
         }
         return GetCppPropertyDeclaration(name, classType, "");
      }

      public static string GetCppComponentDeclaration(string name, string componentGroupType)
      {
         string classType = GetPropertiesClassName(componentGroupType, MergeProperties.EBuildTreeSrcType.kComponentGroup);
         string declaration = String.Format("boost::shared_ptr<{0}>", classType);
         return String.Format("{0,-45}   mp{1};", declaration, name);
      }

      /// <summary>
      /// Writes declaration code into string builder.
      /// </summary>
      /// <param name="propertyNode"></param>
      /// <param name="builder"></param>
      /// <param name="tabCount"></param>
      /// <returns>
      /// If this function returns true, it's skipped children and the nodeReader is on the next valid node so you don't need to do a Read().
      /// </returns>
      public static bool GeneratePropertyDeclaration(XmlNodeReader propertyNode, StringBuilder builder, int tabCount)
      {
         bool bSkippedChildren = false;

         // Check if it's flagged to not be exported
         if (!Helpers.ExportProperty(propertyNode))
         {
            // Yep, skip it
            propertyNode.Skip(); // Ignore all elements below this
            bSkippedChildren = true;  // Don't read next node, we've already got it
            return bSkippedChildren;
         }

         // We can have Property or PropertyObject elements, not interested in anything else
         switch (propertyNode.Name)
         {
            case Helpers.kElement_Property:
            {
               AddTabs(builder, tabCount);
               string name = propertyNode.GetAttribute(Helpers.kProperty_Name, "");
               string type = GetCookType(propertyNode);
               if (type == Helpers.kProperty_Type_enum)
               {
                  // Replace type with enum type
                  type = propertyNode.GetAttribute(Helpers.kProperty_EnumName, "");
                  string nameSpace = propertyNode.GetAttribute(Helpers.kEnumEntry_Namespace, "");
                  if (nameSpace != null)
                  {
                     type = nameSpace + "::" + type;
                  }                                  
               }
               string subType = propertyNode.GetAttribute(Helpers.kProperty_SubType, "");

               builder.AppendLine(CodeGenHelpers.GetCppPropertyDeclaration(name, type, subType));
               // Check to see if it's an asset so we can write token generation code
               if (type == Helpers.kProperty_Type_string)
               {
                  // Get subtype
                  if (subType == Helpers.kProperty_SubType_asset)
                  {
                     // Potential asset types
                     string[] assetTypes = propertyNode.GetAttribute(Helpers.kProperty_AssetTypes, "").Split(new char[] { ';' });
                     string resourceClassType = (AssetSystem.AssetTypes.GetMergedLoadDependenciesForAssetTypes(assetTypes) == AssetSystem.AssetType.ELoadNeedsDependencies.kComplexDependencies)
                                                ? "ResourceWithDependencies" : "Resource";

                     bool useUntypedAssetClass = false;

                     // Multiple or no asset types
                     if ((assetTypes.Length > 1) || (assetTypes.Length == 0))
                     {
                        if (assetTypes.Length > 1)
                        {
                           AssetSystem.AssetType firstAssetType = AssetSystem.AssetTypes.GetAssetTypeForTypeId(Tools.Common.Misc.GetFourCCFromString(assetTypes[0].ToUpper()));

                           // Check to see if class name is same for all types.
                           foreach(string assetTypeString in assetTypes)
                           {
                              AssetSystem.AssetType assetType = AssetSystem.AssetTypes.GetAssetTypeForTypeId(Tools.Common.Misc.GetFourCCFromString(assetTypeString.ToUpper()));
                              if ((firstAssetType == null) || (assetType == null) || (assetType.ClassName != firstAssetType.ClassName))
                              {
                                 // Runtime class names differ or asset types are unknown.
                                 useUntypedAssetClass = true;
                                 break;
                              }
                           }
                        }
                     }

                     if (useUntypedAssetClass)
                     {
                        // Multiple or unknown asset types, just use untyped CResource
                        AddTabs(builder, tabCount);
                        builder.AppendLine(String.Format("{0, -45}   m{1}_Resource;", String.Format("boost::optional<C{0}>", resourceClassType), name));
                     }
                     else
                     {
                        // Just one asset type, we can template this
                        AssetSystem.AssetType assetType = AssetSystem.AssetTypes.GetAssetTypeForTypeId(Tools.Common.Misc.GetFourCCFromString(assetTypes[0].ToUpper()));
                        AddTabs(builder, tabCount);
                        if (assetType != null)
                        {
                           builder.AppendLine(String.Format("{0, -45}   m{1}_Resource;", String.Format("boost::optional<T{0}<{1}> >", resourceClassType, assetType.ClassName), name));
                        }
                        else
                        {
                           // Don't know what this is, just use regular CResource
                           // Maybe print out an error?
                           builder.AppendLine(String.Format("{0, -45}   m{1}_Resource;", String.Format("boost::optional<C{0}>", resourceClassType), name));
                        }
                     }
                  }
               }
               propertyNode.Skip(); // Ignore all elements below this
               bSkippedChildren = true;  // Don't read next node, we've already got it
            }
            break;
            case Helpers.kElement_EvaluatorProperty:
            {
               string type = propertyNode.GetAttribute(Helpers.kEvaluatorProperty_Type, "");               
               string name = propertyNode.GetAttribute(Helpers.kEvaluatorProperty_Name, "");
               string interfaceClass = GetEvaluatorInterfaceClassFromType(type);
               AddTabs(builder, tabCount);
               builder.AppendFormat("{0, -45}   m{1};\n", String.Format("boost::shared_ptr<{0}>", interfaceClass), name);
               propertyNode.Skip(); // Ignore all elements below this
               bSkippedChildren = true;  // Don't read next node, we've already got it
            }
            break;
            case Helpers.kElement_PropertyObject:
            {
               AddTabs(builder, tabCount);
               string id = GetPropertyObjectCookType(propertyNode);
               string name = propertyNode.GetAttribute(Helpers.kProperty_Name, "");
               builder.AppendLine(CodeGenHelpers.GetCppPropertyObjectDeclaration(name, id));
               propertyNode.Skip();
               bSkippedChildren = true;  // Don't read next node, we've already got it
            }
            break;
            case Helpers.kElement_ComponentGroup:
            {
               AddTabs(builder, tabCount);
               //string type = GetCookType(propertyNode);
               string id = propertyNode.GetAttribute(Helpers.kComponent_Id, "");
               string name = propertyNode.GetAttribute(Helpers.kProperty_Name, "");
               builder.AppendLine(CodeGenHelpers.GetCppComponentDeclaration(name, id));
               propertyNode.Skip();
               bSkippedChildren = true;  // Don't read next node, we've already got it
            }
            break;
         }

         return bSkippedChildren;
      }

      /// <summary>
      /// Writes default initialization code into string builder for properties that don't have a default constructor.
      /// </summary>
      /// <param name="propertyNode"></param>
      /// <param name="builder"></param>
      /// <param name="tabCount"></param>
      /// <returns>
      /// If this function returns true, it's skipped children and the nodeReader is on the next valid node so you don't need to do a Read().
      /// </returns>
      public static bool GeneratePropertyDefaultInitialization(XmlNodeReader propertyNode, ref bool bFirstInitialization, StringBuilder builder, int tabCount)
      {
         bool bSkippedChildren = false;
         // Check if it's flagged to not be exported
         if (!Helpers.ExportProperty(propertyNode))
         {
            // Yep, skip it
            propertyNode.Skip(); // Ignore all elements below this
            bSkippedChildren = true;  // Don't read next node, we've already got it
            return bSkippedChildren;
         }

         // We can have Property or PropertyObject elements, not interested in anything else
         switch (propertyNode.Name)
         {
            case Helpers.kElement_Property:
            {
               AddTabs(builder, tabCount);
               string type = GetCookType(propertyNode);
               string name = propertyNode.GetAttribute(Helpers.kProperty_Name, "");
               switch (type)
               {
                  case Helpers.kProperty_Type_guid:
                     builder.AppendLine(String.Format(" {0} m{1}(CGuid::Null())", bFirstInitialization ? ":" : ",", name));
                     bFirstInitialization = false;
                     break;
               }
               propertyNode.Skip(); // Ignore all elements below this
               bSkippedChildren = true;  // Don't read next node, we've already got it
            }
            break;
            case Helpers.kElement_EvaluatorProperty:
            {
               propertyNode.Skip(); // Ignore all elements below this
               bSkippedChildren = true;  // Don't read next node, we've already got it
            }
            break;
            
            case Helpers.kElement_PropertyObject:
            {
               AddTabs(builder, tabCount);
               string id = GetPropertyObjectCookType(propertyNode);
               string name = propertyNode.GetAttribute(Helpers.kProperty_Name, "");
               switch (id)
               {
                  case Helpers.kProperty_Type_vector3:
                     builder.AppendLine(String.Format(" {0} m{1}(CVector3::kConstructUninitialized)", bFirstInitialization ? ":" : ",", name));
                     bFirstInitialization = false;
                     break;
                  case Helpers.kProperty_Type_matrix34:
                     builder.AppendLine(String.Format(" {0} m{1}(CMatrix34::kConstructUninitialized)", bFirstInitialization ? ":" : ",", name));
                     bFirstInitialization = false;
                     break;
               }
               propertyNode.Skip(); // Ignore all elements below this
               bSkippedChildren = true;  // Don't read next node, we've already got it
            }
            break;

            case Helpers.kElement_ComponentGroup:
            {
               // No intialization required for component groups
               propertyNode.Skip();
               bSkippedChildren = true;  // Don't read next node, we've already got it
            }
            break;
         }

         return bSkippedChildren;
      }

      private static string ConvertToValidFloatValue(string value)
      {
         if( value.Contains(".") )
            return value + "f";
         else
            return value + ".0f";
      }

      /// <summary>
      /// Writes default initialization code for class body into string builder.
      /// </summary>
      /// <param name="propertyNode"></param>
      /// <param name="builder"></param>
      /// <param name="tabCount"></param>
      /// <returns>
      /// If this function returns true, it's skipped children and the nodeReader is on the next valid node so you don't need to do a Read().
      /// </returns>
      public static bool GeneratePropertyDefaults(XmlNodeReader propertyNode, StringBuilder builder, int tabCount)
      {
         bool bSkippedChildren = false;
         // Check if it's flagged to not be exported
         if (!Helpers.ExportProperty(propertyNode))
         {
            // Yep, skip it
            propertyNode.Skip(); // Ignore all elements below this
            bSkippedChildren = true;  // Don't read next node, we've already got it
            return bSkippedChildren;
         }
         // We can have Property or PropertyObject elements, not interested in anything else
         switch (propertyNode.Name)
         {
            case Helpers.kElement_Property:
            {
               string type = GetCookType(propertyNode);
               string name = propertyNode.GetAttribute(Helpers.kProperty_Name, "");
               string subType = propertyNode.GetAttribute(Helpers.kProperty_SubType, "");

               string value = null;
               // Make sure we handle <Property/> and <Property></Property> cases correctly.
               if (!propertyNode.IsEmptyElement)
               {
                  XmlDocument valueDocument = new XmlDocument();
                  valueDocument.Load(propertyNode.ReadSubtree());
                  XmlNode valueNode = valueDocument.SelectSingleNode(".//Value/text()");
                  if (valueNode != null)
                  {
                     value = valueNode.Value;
                  }
               }
               
               if (value != null)
               {                  
                  switch (type)
                  {
                     case Helpers.kProperty_Type_color:
                        ColorF color = ColorF.FromString(value);
                        AddTabs(builder, tabCount);
                        builder.AppendLine(String.Format("m{0} = CColorf({1}, {2}, {3}, {4});",
                                                         name,
                                                         ConvertToValidFloatValue((color.R * color.I).ToString("r")),
                                                         ConvertToValidFloatValue((color.G * color.I).ToString("r")),
                                                         ConvertToValidFloatValue((color.B * color.I).ToString("r")),
                                                         ConvertToValidFloatValue(color.A.ToString("r"))));
                        break;
                     case Helpers.kProperty_Type_bool:
                        AddTabs(builder, tabCount);
                        builder.AppendLine(String.Format("m{0} = {1};", name, value));
                        break;
                     case Helpers.kProperty_Type_float:
                        AddTabs(builder, tabCount);
                        builder.AppendLine(String.Format("m{0} = {1};", name, ConvertToValidFloatValue(value)));
                        break;
                     case Helpers.kProperty_Type_int:
                        AddTabs(builder, tabCount);
                        builder.AppendLine(String.Format("m{0} = {1};", name, value));
                        break;
                     case Helpers.kProperty_Type_enum:
                        {
                           // Check to see if there's a namespace
                           string enumNamespace = propertyNode.GetAttribute(Helpers.kEnumEntry_Namespace);
                           string codeEnumName = value;
                           if (enumNamespace != null)
                           {
                              // Prepend namespace
                              codeEnumName = enumNamespace + "::" + codeEnumName;
                           }
                           AddTabs(builder, tabCount);
                           builder.AppendLine(String.Format("m{0} = {1};", name, codeEnumName));
                        }
                        break;
                     case Helpers.kProperty_Type_string:
                        AddTabs(builder, tabCount);
                        
                        string actualValue = String.Empty;
                        
                        switch (subType)
                        {
                        case Helpers.kProperty_SubType_asset:
                           actualValue = AssetSystem.Manager.GetCookedAssetPath(value);
                           builder.AppendLine(String.Format("m{0} = std::string(\"{1}\");", name, actualValue));
                           break;
                        case Helpers.kProperty_SubType_hashedString:
                           actualValue = Helpers.GetSubTypeHashedStringValue(value).ToString();
                           builder.AppendLine(String.Format("m{0} = {1};", name, actualValue));
                           break;
                        case Helpers.kProperty_SubType_messageStringHash:
                           actualValue = Helpers.GetMessageNameHash( value ).ToString();
                           builder.AppendLine( String.Format( "m{0} = {1};", name, actualValue ) );
                           break;
                        case Helpers.kProperty_SubType_animationEventStringHash:
                           actualValue = Helpers.GetAnimationEventNameHash(value).ToString();
                           builder.AppendLine(String.Format("m{0} = {1};", name, actualValue));
                           break;
                        default:
                           actualValue = value;
                           builder.AppendLine(String.Format("m{0} = std::string(\"{1}\");", name, actualValue));
                           break;
                        }

                        break;
                     case Helpers.kProperty_Type_guid:
                        {
                           string guidString = new Guid(value).ToString("D"); // Normalize to xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx 
                           string[] guidValues = guidString.Split(new char[] { '-' });
                           string byteValues = "";
                           foreach (char byteValue in guidValues[4])
                           {
                              byteValues += String.Format(", 0x{0:X}", byteValue);
                           }
                           AddTabs(builder, tabCount);
                           builder.AppendLine(String.Format("m{0} = CGuid(0x{1}, 0x{2}, 0x{3}, 0x{4}{5});", name, guidValues[0], guidValues[1], guidValues[2], guidValues[3], byteValues));
                        }
                        break;
                     case Helpers.kProperty_Type_evaluatorContainer:
                        break;   // No default properties
                  }
               }

               propertyNode.Skip(); // Ignore all elements below this
               bSkippedChildren = true;  // Don't read next node, we've already got it
            }
            break;
            case Helpers.kElement_EvaluatorProperty:
            {
               propertyNode.Skip(); // Ignore all elements below this
               bSkippedChildren = true;  // Don't read next node, we've already got it
            }
            break;
            case Helpers.kElement_PropertyObject:
            {
               string id = GetPropertyObjectCookType(propertyNode);
               string name = propertyNode.GetAttribute(Helpers.kProperty_Name, "");
               switch (id)
               {
                  case Helpers.kProperty_Type_vector3:
                     {
                        // Need a navigator to get values
                        XPathNavigator nav = new XPathDocument(propertyNode.ReadSubtree()).CreateNavigator();
                        double x = nav.SelectSingleNode("//Property[@name=\"X\"]/Value").ValueAsDouble;
                        double y = nav.SelectSingleNode("//Property[@name=\"Y\"]/Value").ValueAsDouble;
                        double z = nav.SelectSingleNode("//Property[@name=\"Z\"]/Value").ValueAsDouble;
                        AddTabs(builder, tabCount);
                        builder.AppendLine(String.Format("m{0} = CVector3({1}, {2}, {3});", name, ConvertToValidFloatValue(x.ToString("r")), ConvertToValidFloatValue(y.ToString("r")), ConvertToValidFloatValue(z.ToString("r"))));
                     }
                     break;
                  case Helpers.kProperty_Type_matrix34:
                     {
                        // Need a navigator to get values
                        XPathNavigator nav = new XPathDocument(propertyNode.ReadSubtree()).CreateNavigator();
                        string[] values = new string[12];
                        for (int row = 0; row < 4; ++row)
                        {
                           for (int column = 0; column < 3; ++column)
                           {
                              double val = nav.SelectSingleNode( String.Format( "//Property[@name=\"m{0}{1}\"]/Value", row, column ) ).ValueAsDouble;
                              values[row * 3 + column] = ConvertToValidFloatValue(val.ToString("r"));
                           }
                        }

                        AddTabs(builder, tabCount);
                        builder.AppendLine(String.Format("m{0} = CMatrix34( {1} );", name, String.Join(", ", values)));
                     }
                     break;
                  default:
                     break;
               }
               propertyNode.Skip(); // Ignore all elements below this
               bSkippedChildren = true;  // Don't read next node, we've already got it
            }
            break;
            
            case Helpers.kElement_ComponentGroup:
            {
               AddTabs(builder, tabCount);
               //string type = GetCookType(propertyNode);
               string id = propertyNode.GetAttribute(Helpers.kComponent_Id, "");
               string name = propertyNode.GetAttribute(Helpers.kProperty_Name, "");
               builder.AppendLine(String.Format("mb{0}Initialized = false;", name));
               propertyNode.Skip();
               bSkippedChildren = true;  // Don't read next node, we've already got it
            }
            break;
         }
         return bSkippedChildren;
      }

      public static void GenerateEnumPropertyLoadingCode(XmlNodeReader propertyNode, StringBuilder builder, int tabCount)
      {
         string propertyName = propertyNode.GetAttribute(Helpers.kProperty_Name, "");
         string enumNamespace = propertyNode.GetAttribute(Helpers.kEnumEntry_Namespace);
         XPathNavigator propertyNodeNav = new XPathDocument(propertyNode.ReadSubtree()).CreateNavigator();

         //AddTabs(builder, tabCount);
         builder.AppendLine("uint32 const enumFourCC = applyProperties.mStream.ReadUint32();");

         AddTabs(builder, tabCount);
         builder.AppendLine("switch (enumFourCC)");

         AddTabs(builder, tabCount);
         builder.AppendLine("{");

         tabCount++;

         XPathNodeIterator enumIter = propertyNodeNav.Select("Property/EnumEntry");
         while (enumIter.MoveNext())
         {
            XPathNavigator enumEntry = enumIter.Current;
            string enumName = enumEntry.GetAttribute(Helpers.kEnumEntry_Name, "");
            string codeEnumName = enumName;
            if (enumNamespace != null)
            {
               // Prepend namespace
               codeEnumName = enumNamespace +"::" + codeEnumName;
            }

            AddTabs(builder, tabCount);
            builder.AppendLine(String.Format(@"case 0x{0:X}:   // {1} - {2}", GetStringHash(enumName), enumName, enumEntry.GetAttribute(Helpers.kEnumEntry_Description, "")));
            tabCount++;

            AddTabs(builder, tabCount);
            builder.AppendLine(String.Format("m{0} = {1};", propertyName, codeEnumName));

            AddTabs(builder, tabCount);
            builder.AppendLine("break;");
            tabCount--;
         }

         tabCount--;
         AddTabs(builder, tabCount);
         builder.AppendLine("}");
      }

      /// <summary>
      /// Writes loading code for property into string builder.
      /// </summary>
      /// <param name="propertyNode"></param>
      /// <param name="builder"></param>
      /// <param name="tabCount"></param>
      /// <returns>
      /// If this function returns true, it's skipped children and the nodeReader is on the next valid node so you don't need to do a Read().
      /// </returns>
      public static bool GeneratePropertyLoadingCode(XmlNodeReader propertyNode, StringBuilder builder, int tabCount)
      {
         bool bSkippedChildren = false;
         // Check if it's flagged to not be exported
         if (!Helpers.ExportProperty(propertyNode))
         {
            // Yep, skip it
            propertyNode.Skip(); // Ignore all elements below this
            bSkippedChildren = true;  // Don't read next node, we've already got it
            return bSkippedChildren;
         }

         switch (propertyNode.Name)
         {
            case Helpers.kElement_Property:
            break;
            case Helpers.kElement_EvaluatorProperty:
            break;
            case Helpers.kElement_PropertyObject:
            break;
            case Helpers.kElement_ComponentGroup:
            break;
            default:
               // Not interested in this element, don't do anything
               return false;
         }

         string name = propertyNode.GetAttribute(Helpers.kProperty_Name, "");

         // Write case '...': //
         AddTabs(builder, tabCount);
         builder.AppendLine(String.Format(@"case kP_{0}:   // 0x{1:X08}", name, GetStringHash(name)));

         AddTabs(builder, tabCount);
         builder.AppendLine("{");

         tabCount++;
         AddTabs(builder, tabCount);

         // We can have Property or PropertyObject elements, not interested in anything else
         switch (propertyNode.Name)
         {
            case Helpers.kElement_Property:
            {
               string type = GetCookType(propertyNode);
               string subType = propertyNode.GetAttribute(Helpers.kProperty_SubType);

               switch (type)
               {
                  case Helpers.kProperty_Type_color:
                     builder.AppendLine(String.Format("m{0} = CColorf(applyProperties.mStream);", name));
                     break;
                  case Helpers.kProperty_Type_bool:
                     builder.AppendLine(String.Format("m{0} = applyProperties.mStream.ReadBool();", name));
                     break;
                  case Helpers.kProperty_Type_float:
                     builder.AppendLine(String.Format("m{0} = applyProperties.mStream.ReadReal32();", name));
                     break;
                  case Helpers.kProperty_Type_vector3:
                     builder.AppendLine(String.Format("m{0} = CVector3(applyProperties.mStream);", name));
                     break;
                  case Helpers.kProperty_Type_matrix34:
                     builder.AppendLine(String.Format("m{0} = CMatrix34(applyProperties.mStream);", name));
                     break;
                  case Helpers.kProperty_Type_int:
                     builder.AppendLine(String.Format("m{0} = applyProperties.mStream.ReadInt32();", name));
                     break;
                  case Helpers.kProperty_Type_enum:
                     GenerateEnumPropertyLoadingCode(propertyNode, builder, tabCount);
                     break;
                  case Helpers.kProperty_Type_string:
                     switch (subType)
                     {
                        case Helpers.kProperty_SubType_hashedString:
                        case Helpers.kProperty_SubType_messageStringHash:
                        case Helpers.kProperty_SubType_animationEventStringHash:
                           builder.AppendLine(String.Format("m{0} = applyProperties.mStream.ReadUint32();", name));
                           break;
                        default:
                           builder.AppendLine(String.Format("m{0} = applyProperties.mStream.ReadString();", name));
                           break;
                     }
                     break;
                  case Helpers.kProperty_Type_guid:
                     builder.AppendLine(String.Format("m{0} = CGuid(applyProperties.mStream);", name));
                     break;
                  case Helpers.kProperty_Type_evaluatorContainer:
                     builder.AppendLine(String.Format("m{0}.ApplyProperties(applyProperties);", name));
                     break;

                  case Helpers.kProperty_Type_custom:
                     builder.AppendLine(String.Format("m{0}.ConstructFromStream(applyProperties.mStream);", name));
                     break;
               }

               propertyNode.Skip(); // Ignore all elements below this
               bSkippedChildren = true;  // Don't read next node, we've already got it
            }
            break;
            case Helpers.kElement_EvaluatorProperty:
            {
               string type = propertyNode.GetAttribute(Helpers.kEvaluatorProperty_Type, "");
               string factoryFunction = GetEvaluatorFactoryFunctionFromType(type);
               
               builder.AppendLine("if (!applyProperties.UpdateOnly())");
               AddTabs(builder, tabCount);
               builder.AppendLine("{");
               
               tabCount++;
               AddTabs(builder, tabCount);
               builder.AppendFormat("m{0}.reset(static_cast<{1}*>(applyProperties.mFactory.{2}(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));\n", name, GetEvaluatorInterfaceClassFromType(type), factoryFunction);
               tabCount--;
               
               AddTabs(builder, tabCount);
               builder.AppendLine("}");
               AddTabs(builder, tabCount);
               builder.AppendLine("else");
               AddTabs(builder, tabCount);
               builder.AppendLine("{");
               
               tabCount++;
               AddTabs(builder, tabCount);
               builder.AppendFormat("IEvaluator::UpdateProperties(m{0}.get(), applyProperties, size, allocator);\n", name);
               tabCount--;
               
               AddTabs(builder, tabCount);
               builder.AppendLine("}");
               
               propertyNode.Skip(); // Ignore all elements below this
               bSkippedChildren = true;  // Don't read next node, we've already got it
            }
            break;
            case Helpers.kElement_PropertyObject:
            {
               string id = GetPropertyObjectCookType(propertyNode);
               switch (id)
               {
                  case Helpers.kProperty_Type_vector3:
                     builder.AppendLine(String.Format("m{0} = CVector3(applyProperties.mStream);", name));
                     break;
                  case Helpers.kProperty_Type_matrix34:
                     builder.AppendLine(String.Format("m{0} = CMatrix34(applyProperties.mStream);", name));
                     break;
                  default:
                     builder.AppendLine(String.Format("m{0}.ApplyProperties(applyProperties, allocator);", name));
                     break;
               }
               propertyNode.Skip(); // Ignore all elements below this
               bSkippedChildren = true;  // Don't read next node, we've already got it
            }
            break;
            case Helpers.kElement_ComponentGroup:
            {
               //string type = GetCookType(propertyNode);
               string id = propertyNode.GetAttribute(Helpers.kComponent_Id, String.Empty);
               string propertiesClassName = GetPropertiesClassName(id, MergeProperties.EBuildTreeSrcType.kComponentGroup);
               
               bool bUsingParentEvaluatorAllocator = Tools.Common.Misc.ParseBool(propertyNode.GetAttribute(Helpers.kComponentGroup_UseParentEvaluatorAllocator, String.Empty) ?? "false");
               if (bUsingParentEvaluatorAllocator)
               {
                  builder.AppendLine("// Using Parent Evaluator Allocator");
                  AddTabs(builder, tabCount);
               }
               string parentEvaluatorAllocator = bUsingParentEvaluatorAllocator ? ", &allocator" : ", NULL";

               builder.AppendLine("if (!applyProperties.UpdateOnly())");
               AddTabs(builder, tabCount);
               builder.AppendLine("{");
               tabCount++;
               AddTabs(builder, tabCount);
               
               builder.AppendFormat("mp{0}.reset(static_cast<{1}*>({1}::BuildComponentProperties(applyProperties{2})));\n", name, propertiesClassName, parentEvaluatorAllocator);

               AddTabs(builder, tabCount);
               builder.AppendLine(String.Format("mb{0}Initialized = true;", name));
               
               tabCount--;
               AddTabs(builder, tabCount);
               builder.AppendLine("}");
               AddTabs(builder, tabCount);
               builder.AppendLine("else");
               AddTabs(builder, tabCount);
               builder.AppendLine("{");
               tabCount++;
               
               AddTabs(builder, tabCount);
               builder.AppendFormat("CGameObjectComponentProperties::UpdateProperties(mp{0}.get(), applyProperties{1});\n", name, parentEvaluatorAllocator);
               
               tabCount--;
               AddTabs(builder, tabCount);
               builder.AppendLine("}");

               propertyNode.Skip();
               bSkippedChildren = true;  // Don't read next node, we've already got it
            }
            break;

         }

         AddTabs(builder, tabCount);
         builder.AppendLine("break;");

         tabCount--;
         AddTabs(builder, tabCount);
         builder.AppendLine("}");

         return bSkippedChildren;
      }

      /// <summary>
      /// Writes code for post property load into string builder.
      /// </summary>
      /// <param name="propertyNode"></param>
      /// <param name="builder"></param>
      /// <param name="tabCount"></param>
      /// <returns>
      /// If this function returns true, it's skipped children and the nodeReader is on the next valid node so you don't need to do a Read().
      /// </returns>
      public static bool GeneratePostLoaderCode(XmlNodeReader propertyNode, StringBuilder builder, int tabCount)
      {
         bool bSkippedChildren = false;
         // Check if it's flagged to not be exported
         if (!Helpers.ExportProperty(propertyNode))
         {
            // Yep, skip it
            propertyNode.Skip(); // Ignore all elements below this
            bSkippedChildren = true;  // Don't read next node, we've already got it
            return bSkippedChildren;
         }

         switch (propertyNode.Name)
         {
            case Helpers.kElement_ComponentGroup:
            {
               bSkippedChildren = true;
               // Generate statement to check for null
               string name = propertyNode.GetAttribute(Helpers.kComponentGroup_Name, "");
               string componentGroupId = propertyNode.GetAttribute(Helpers.kComponent_Id, "");
               XPathNavigator nav = new XPathDocument(propertyNode.ReadSubtree()).CreateNavigator();
               XPathNavigator valueNav = nav.SelectSingleNode("//ComponentGroup/Value");

               // Check to see if there's a default value (should be!)
               if (valueNav != null)
               {
                  string defaultValue = valueNav.Value;
                  if (defaultValue != "None")
                  {
                     AddTabs(builder, tabCount);
                     builder.AppendLine(String.Format("if (!mb{0}Initialized)", name));
                     AddTabs(builder, tabCount);
                     builder.AppendLine("{");
                     tabCount++;

                     AddTabs(builder, tabCount);
                     builder.AppendLine(String.Format("// No component found for m{0}, initialize to default value.", name));
                     AddTabs(builder, tabCount);
                     builder.AppendLine(String.Format("mp{0}.reset(new {1}());", name, CodeGenHelpers.GetComponentPropertiesClassName(componentGroupId, defaultValue)));
                     AddTabs(builder, tabCount);
                     builder.AppendLine(String.Format("mp{0}->PostLoadUpdate();", name));

                     tabCount--;
                     AddTabs(builder, tabCount);
                     builder.AppendLine("}");
                  }
                  else
                  {
                     AddTabs(builder, tabCount);
                     builder.AppendLine(String.Format("// mp{0} is NULL by default.", name));
                  }
               }
               propertyNode.Skip();
               bSkippedChildren = true;
            }
            break;
            case Helpers.kElement_Property:
            {
               // Check for assets
               string name = propertyNode.GetAttribute(Helpers.kProperty_Name, "");
               string type = propertyNode.GetAttribute(Helpers.kProperty_Type, "");
               string subType = propertyNode.GetAttribute(Helpers.kProperty_SubType, "");
               if ((type == Helpers.kProperty_Type_string) && (subType == Helpers.kProperty_SubType_asset))
               {
                  // OK, it's an asset
                  AddTabs(builder, tabCount);
                  builder.AppendLine("// Check for empty asset string");
                  AddTabs(builder, tabCount);
                  builder.AppendLine(String.Format("if (m{0}.size())", name));
                  AddTabs(builder, tabCount);
                  builder.AppendLine("{");
                  tabCount++;

                  AddTabs(builder, tabCount);
                  builder.AppendLine("// Lock dependencies, build resource.");
                  AddTabs(builder, tabCount);
                  builder.AppendLine(String.Format("CResId const resId(m{0});", name));
                  builder.AppendLine();

                  // Potential asset types
                  string[] assetTypes = propertyNode.GetAttribute(Helpers.kProperty_AssetTypes, "").Split(new char[] { ';' });
                  string resourceLoadFunction = string.Empty;
                  string resourceClass = string.Empty;
                  if( AssetSystem.AssetTypes.GetMergedLoadDependenciesForAssetTypes(assetTypes) == AssetSystem.AssetType.ELoadNeedsDependencies.kComplexDependencies )
                  {
                     resourceLoadFunction = "GetResourceWithDependencies";
                     resourceClass = "CResourceWithDependencies";
                  }
                  else
                  {
                     resourceLoadFunction = "GetResource";
                     resourceClass = "CResource";
                  }

                  AddTabs(builder, tabCount);
                  builder.AppendLine(String.Format("{0} newResource(gpResources->{1}(resId));", resourceClass, resourceLoadFunction));

                  // Requires lock on load? Defaults to true if not present
                  bool bLockOnLoad = Tools.Common.Misc.ParseBool(( propertyNode.GetAttribute(Helpers.kProperty_LockAssetOnLoad, String.Empty) ?? "true" ));
                  if( bLockOnLoad )
                  {
                     AddTabs(builder, tabCount);
                     builder.AppendLine(String.Format("newResource.Lock();"));
                  }
                  builder.AppendLine();

                  AddTabs(builder, tabCount);
                  builder.AppendLine(String.Format("m{0}_Resource = newResource;", name));

                  tabCount--;
                  AddTabs(builder, tabCount);
                  builder.AppendLine("}");

                  AddTabs(builder, tabCount);
                  builder.AppendLine("else");
                  
                  AddTabs(builder, tabCount);
                  builder.AppendLine("{");
                  tabCount++;

                  AddTabs(builder, tabCount);
                  builder.AppendLine(String.Format("m{0}_Resource.reset();", name));
                  
                  tabCount--;
                  AddTabs(builder, tabCount);
                  builder.AppendLine("}");

               }
               propertyNode.Skip();
               bSkippedChildren = true;
            }
            break;
            case Helpers.kElement_EvaluatorProperty:
            {
               // Call the PostLoadUpdate function on those property objects
               string name = propertyNode.GetAttribute(Helpers.kEvaluatorProperty_Name, "");
               AddTabs(builder, tabCount);
               builder.AppendLine(String.Format("if (m{0}) m{0}->PostLoadUpdate();", name));
               propertyNode.Skip();
               bSkippedChildren = true;
            }
            break;
            case Helpers.kElement_PropertyObject:
            {
               // Call the PostLoadUpdate function on those property objects
               string name = propertyNode.GetAttribute(Helpers.kProperty_Name, "");

               string id = GetPropertyObjectCookType(propertyNode);
               switch (id)
               {
                  case Helpers.kProperty_Type_vector3:
                  case Helpers.kProperty_Type_matrix34:
                     break;
                  default:
                     AddTabs(builder, tabCount);
                     builder.AppendLine(String.Format("m{0}.PostLoadUpdate();", name));
                     break;
               }
               propertyNode.Skip();
               bSkippedChildren = true;
            }
            break;
            
         }

         return bSkippedChildren;
      }
      
      /// <summary>
      /// Writes code for initializing evaluators
      /// </summary>
      /// <param name="propertyNode"></param>
      /// <param name="builder"></param>
      /// <param name="tabCount"></param>
      /// <returns>
      /// If this function returns true, it's skipped children and the nodeReader is on the next valid node so you don't need to do a Read().
      /// </returns>
      public static bool GenerateInitializeEvaluatorCode(XmlNodeReader propertyNode, StringBuilder builder, int tabCount)
      {
         bool bSkippedChildren = false;
         // Check if it's flagged to not be exported
         if (!Helpers.ExportProperty(propertyNode))
         {
            // Yep, skip it
            propertyNode.Skip(); // Ignore all elements below this
            bSkippedChildren = true;  // Don't read next node, we've already got it
            return bSkippedChildren;
         }

         switch (propertyNode.Name)
         {
            case Helpers.kElement_ComponentGroup:
            {
               // We'll ignore evaluators in components for now.
               // These will probably be initialized manually in the the top level component initialize as we need a base game manager.
               /*
               // Generate statement to check for null
               
               string name = propertyNode.GetAttribute(Helpers.kComponentGroup_Name, "");
               AddTabs(builder, tabCount);
               builder.AppendLine(String.Format("if (m{0}) m{0}->InitializeEvaluators(updateData);", name));
               */   
               propertyNode.Skip();
               bSkippedChildren = true;
            }
            break;
            case Helpers.kElement_Property:
            {
               // Don't care about properties
               propertyNode.Skip();
               bSkippedChildren = true;
            }
            break;
            case Helpers.kElement_EvaluatorProperty:
            {
               string name = propertyNode.GetAttribute(Helpers.kProperty_Name, "");
               AddTabs(builder, tabCount);
            
               // Check to see if we're allowed to initialize this
               if (!Tools.Common.Misc.ParseBool(propertyNode.GetAttribute(Helpers.kEvaluatorProperty_DisableCodeGenEvaluatorInitialization) ?? "false"))
               {
                  // Call the initialize function on evaluator
                  builder.AppendLine(String.Format("if (m{0}) m{0}->Initialize(updateData);", name));
               }
               else
               {
                  builder.AppendFormat("// Evaluator initialization disabled for {0}\n", name);
               }
               
               propertyNode.Skip();
               bSkippedChildren = true;
            }
            break;
            
            case Helpers.kElement_PropertyObject:
            {
               // Call the initialize function on these property objects
               string name = propertyNode.GetAttribute(Helpers.kProperty_Name, "");

               string id = GetPropertyObjectCookType(propertyNode);
               switch (id)
               {
                  case Helpers.kProperty_Type_vector3:
                  case Helpers.kProperty_Type_matrix34:
                     break;
                  default:
                     AddTabs(builder, tabCount);
                     builder.AppendLine(String.Format("m{0}.InitializeEvaluators(updateData);", name));
                     break;
               }
               propertyNode.Skip();
               bSkippedChildren = true;
            }
            break;
            
         }

         return bSkippedChildren;
      }

      /// <summary>
      /// Writes includes code into string builder.
      /// </summary>
      /// <param name="propertyNode"></param>
      /// <param name="builder"></param>
      /// <param name="tabCount"></param>
      /// <returns>
      /// If this function returns true, it's skipped children and the nodeReader is on the next valid node so you don't need to do a Read().
      /// </returns>
      public static bool GeneratePropertyIncludes(XmlNodeReader propertyNode, StringBuilder builder, int tabCount)
      {
         bool bSkippedChildren = false;

         // Check if it's flagged to not be exported
         if (!Helpers.ExportProperty(propertyNode))
         {
            // Yep, skip it
            propertyNode.Skip(); // Ignore all elements below this
            bSkippedChildren = true;  // Don't read next node, we've already got it
            return bSkippedChildren;
         }

         // We can have Property or PropertyObject elements, not interested in anything else
         switch (propertyNode.Name)
         {
            case Helpers.kElement_Property:
               {
                  string name = propertyNode.GetAttribute(Helpers.kProperty_Name, "");
                  string type = GetCookType(propertyNode);
                  // Check to see if it's an asset so we can write token generation code
                  if (type == Helpers.kProperty_Type_string)
                  {
                     // Get subtype
                     string subType = propertyNode.GetAttribute(Helpers.kProperty_SubType);
                     if (subType == Helpers.kProperty_SubType_asset)
                     {
                        // Potential asset types
                        string[] assetTypes = propertyNode.GetAttribute(Helpers.kProperty_AssetTypes, "").Split(new char[] { ';' });
                        string resourceClassType = (AssetSystem.AssetTypes.GetMergedLoadDependenciesForAssetTypes(assetTypes) == AssetSystem.AssetType.ELoadNeedsDependencies.kComplexDependencies)
                                                   ? "ResourceWithDependencies" : "Resource";
                        foreach (string assetTypeString in assetTypes)
                        {
                           // Just one asset type, we can template this
                           AssetSystem.AssetType assetType = AssetSystem.AssetTypes.GetAssetTypeForTypeId(Tools.Common.Misc.GetFourCCFromString(assetTypeString.ToUpper()));
                           if ((assetType != null) && (assetType.ClassInclude.Length != 0))
                           {
                              AddTabs(builder, tabCount);
                              builder.AppendLine(String.Format("#include \"{0}\"", assetType.ClassInclude));
                           }                           
                        }
                     }
                  }
                  propertyNode.Skip(); // Ignore all elements below this
                  bSkippedChildren = true;  // Don't read next node, we've already got it
               }
               break;
            case Helpers.kElement_EvaluatorProperty:
               {
                  propertyNode.Skip(); // Ignore all elements below this
                  bSkippedChildren = true;  // Don't read next node, we've already got it
               }
               break;
            case Helpers.kElement_PropertyObject:
               {
                  propertyNode.Skip();
                  bSkippedChildren = true;  // Don't read next node, we've already got it
               }
               break;
            case Helpers.kElement_ComponentGroup:
               {
                  propertyNode.Skip();
                  bSkippedChildren = true;  // Don't read next node, we've already got it
               }
               break;
         }

         return bSkippedChildren;
      }


      public static string GetPropertyObjectLoaderFunctionString(string propertyObjectName)
      {
         return String.Format("Initialize{0}", GetPropertiesClassName(propertyObjectName, MergeProperties.EBuildTreeSrcType.kPropertyObject));
      }
      
      public static void GeneratePropertiesEnumCode(XmlNodeReader nodeReader, StringBuilder builder, int tabCount)
      {
         // Class declaration
         AddTabs(builder, tabCount);
         builder.AppendLine("enum EProperties");
         AddTabs(builder, tabCount);
         builder.AppendLine("{");
         tabCount++;
         
         // Write properties declarations
         nodeReader.Read();   // Skip top level element
         bool bSkipped = false;
         while (bSkipped || nodeReader.Read())
         {
            bSkipped = false;
            if (nodeReader.NodeType != XmlNodeType.Element)
            {
               // Only interested in elements
               continue;
            }
            // Check if it's flagged to not be exported
            if (!Helpers.ExportProperty(nodeReader))
            {
               // Yep, skip it
               nodeReader.Skip(); // Ignore all elements below this
               bSkipped = true;  // Don't read next node, we've already got it
               continue;
            }
            
            string name = nodeReader.GetAttribute(Helpers.kProperty_Name, "");
            switch (nodeReader.Name)
            {
               case Helpers.kElement_Property:
                  break;
               case Helpers.kElement_EvaluatorProperty:
                  break;
               case Helpers.kElement_PropertyObject:
                  break;
               case Helpers.kElement_ComponentGroup:
                  break;
               default:
                  continue;
            }
            
            // Write enums
            AddTabs(builder, tabCount);
            builder.AppendLine(String.Format(@"kP_{0} = 0x{1:X08},", String.Format("{0,-45}", name), GetStringHash(name)));
            
            nodeReader.Skip();   // Ignore all elements below this
            bSkipped = true;     // Don't read next node, we've already got it
         }

         builder.AppendLine();
          
         AddTabs(builder, tabCount);
         builder.AppendLine(String.Format(@"kP_{0} = 0xFFFFFFFF", String.Format("{0,-45}", "Terminator")));
         
         tabCount--;
         AddTabs(builder, tabCount);
         builder.AppendLine("};");
      }
      
      public static void GenerateBitFieldsEnum(XPathNavigator nav, StringBuilder builder, int tabCount)
      {
         // Copy original navigator so we can add 'done' attribute to make our life easier
         XPathNavigator newNav = Helpers.NavigatorToDocument(nav).CreateNavigator();
         
         XPathExpression findTopBitField = XPathExpression.Compile("./*/Property[@bitFieldName and @bitFieldPosition and not(@done)]");
         XPathNavigator topBitFieldNav = null;
         while ((topBitFieldNav = newNav.SelectSingleNode(findTopBitField)) != null)
         {
            string bitFieldName = topBitFieldNav.GetAttribute(Helpers.kProperty_Type_bool_bitfield_name, String.Empty);
            
            // enum declaration
            AddTabs(builder, tabCount);
            builder.AppendFormat("enum E{0}\n", bitFieldName);
            AddTabs(builder, tabCount);
            builder.AppendLine("{");
            tabCount++;
            
            string xpathQuery = String.Format("parent::*/Property[@bitFieldName=\"{0}\" and @bitFieldPosition and not(@done)]", bitFieldName);
            XPathNodeIterator bitFieldIter = topBitFieldNav.Select(xpathQuery);
            while(bitFieldIter.MoveNext())
            {
               int bitPosition = int.Parse(bitFieldIter.Current.GetAttribute(Helpers.kProperty_Type_bool_bitfield_position, String.Empty));
               AddTabs(builder, tabCount);
               string propertyName = bitFieldIter.Current.GetAttribute(Helpers.kProperty_Name, String.Empty);
               
               builder.AppendLine(String.Format(@"k{0} = (1 << {1}),", String.Format("{0,-45}", bitFieldName + "_" + propertyName), bitPosition));
               bitFieldIter.Current.CreateAttribute(String.Empty, "done", String.Empty, String.Empty);
            }                                                                     
            AddTabs(builder, tabCount);
            builder.AppendLine(String.Format(@"k{0} = 0xFFFFFFFF", String.Format("{0,-45}", bitFieldName + "_Terminator")));
            
            tabCount--;
            AddTabs(builder, tabCount);
            builder.AppendLine("};");
            builder.AppendLine();
         }
      }
      
      public static void GenerateEnumCodeForEnumProperties(XPathNavigator nav, StringBuilder builder, int tabCount)
      {
         foreach(XPathNavigator enumNav in Helpers.XPathMultiSelect.LayoutSelectSkipParent(nav, "./Property[@type=\"enum\" and @generateCode=\"true\"]"))
         {
            string enumName = enumNav.GetAttribute(Helpers.kProperty_EnumName, String.Empty);
            
            // Class declaration
            AddTabs(builder, tabCount);
            builder.AppendFormat("enum {0}\n", enumName);
            AddTabs(builder, tabCount);
            builder.AppendLine("{");
            tabCount++;
            XPathNodeIterator enumsIter = enumNav.Select("./EnumEntry");
            while (enumsIter.MoveNext())
            {
               AddTabs(builder, tabCount);
               builder.AppendFormat("{0},\n", enumsIter.Current.GetAttribute(Helpers.kEnumEntry_Name, String.Empty));
            };
             builder.AppendLine();
            AddTabs(builder, tabCount);
            builder.AppendFormat("{0}_Count\n", enumName);
            tabCount--;
            AddTabs(builder, tabCount);
            builder.AppendLine("};");
            builder.AppendLine();
         }
      }

      public static string GetEvaluatorClassName(string name, string type)
      {
         string className = "";
         switch (type)
         {
            case Helpers.kEvaluatorProperty_Type_float:
               className = "CFE";
               break;
            case Helpers.kEvaluatorProperty_Type_int:
               className = "CIE";
               break;
            case Helpers.kEvaluatorProperty_Type_bool:
               className = "CBE";
               break;
            case Helpers.kEvaluatorProperty_Type_color:
               className = "CCE";
               break;
            case Helpers.kEvaluatorProperty_Type_vector3:
               className = "CVE";
               break;
            case Helpers.kEvaluatorProperty_Type_modifierPV:
               className = "CMPVE";
               break;
            case Helpers.kEvaluatorProperty_Type_emitterPV:
               className = "CEPVE";
               break;
            case Helpers.kEvaluatorProperty_Type_userData:
               className = "CUDE";
               break;
            default:
               // Custom Type
               className = "CCE";
               break;
         }
         className += name;
         return className;
      }      

      public static string GetEvaluatorInterfaceClassFromType(string type)
      {
         switch (type)
         {
            case Helpers.kEvaluatorProperty_Type_float:
               return "IFloatEvaluator";
            case Helpers.kEvaluatorProperty_Type_int:
               return "IIntEvaluator";
            case Helpers.kEvaluatorProperty_Type_bool:
               return "IBoolEvaluator";
            case Helpers.kEvaluatorProperty_Type_color:
               return "IColorEvaluator";
            case Helpers.kEvaluatorProperty_Type_vector3:
               return "IVectorEvaluator";
            case Helpers.kEvaluatorProperty_Type_modifierPV:
               return "IModifierPVEvaluator";
            case Helpers.kEvaluatorProperty_Type_emitterPV:
               return "IEmitterPVEvaluator";
            case Helpers.kEvaluatorProperty_Type_userData:
               return "IUserDataEvaluator";
         }

         // Must be custom type
         return "ICE" + type;
      }

      public static string GetEvaluatorFactoryFunctionFromType(string type)
      {
         switch (type)
         {
            case Helpers.kEvaluatorProperty_Type_float:
               return "BuildFloatEvaluator";
            case Helpers.kEvaluatorProperty_Type_int:
               return "BuildIntEvaluator";
            case Helpers.kEvaluatorProperty_Type_bool:
               return "BuildBoolEvaluator";
            case Helpers.kEvaluatorProperty_Type_color:
               return "BuildColorEvaluator";
            case Helpers.kEvaluatorProperty_Type_vector3:
               return "BuildVectorEvaluator";
            case Helpers.kEvaluatorProperty_Type_modifierPV:
               return "BuildModifierPVEvaluator";
            case Helpers.kEvaluatorProperty_Type_emitterPV:
               return "BuildEmitterPVEvaluator";
            case Helpers.kEvaluatorProperty_Type_userData:
               return "BuildUserDataEvaluator";
         }

         // Must be custom type
         return "BuildCustomEvaluator";
      }
   }
}
