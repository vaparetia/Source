using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using Tools.Common;

namespace CoreTools.Prefab
{
   public class PrefabHelper
   {
      public struct SAttribute
      {
         public string mName;
         public Misc.EComparisonType mType;

         public SAttribute(string name, Misc.EComparisonType type)
         {
            mName = name;
            mType = type;
         }
      }
      
      public static readonly SAttribute[] kEventLinkEntry_defaultAttributes = new SAttribute[] 
      { 
         new SAttribute("active", Misc.EComparisonType.kBool),
         new SAttribute("time", Misc.EComparisonType.kFloat), 
         new SAttribute("priority", Misc.EComparisonType.kInt), 
         new SAttribute("identifier", Misc.EComparisonType.kString), 
         new SAttribute("query", Misc.EComparisonType.kString), 
         new SAttribute("senderQuery", Misc.EComparisonType.kString),
      };

      public static void CreateDefaultAttributes(XmlElement node)
      {
         foreach (SAttribute attribute in kEventLinkEntry_defaultAttributes)
         {
            string defaultAttributeName = attribute.mName + "Default";

            string attributeValue = node.GetAttribute(attribute.mName);
            if( node.GetAttributeNode(defaultAttributeName) == null )
            {
               node.SetAttribute(defaultAttributeName, attributeValue);
            }
         }
      }

      public static void RemoveDefaultAttributes(XmlElement node)
      {
         foreach (SAttribute attribute in kEventLinkEntry_defaultAttributes)
         {
            XmlAttribute attributeNode = node.GetAttributeNode(attribute.mName);

            string defaultAttributeName = attribute.mName + "Default";
            XmlAttribute defaultAttribute = node.GetAttributeNode(defaultAttributeName);

            if( defaultAttribute != null )
            {
               if (attributeNode != null)
               {
                  if ( Misc.CompareStringsAsType(attributeNode.Value, defaultAttribute.Value, attribute.mType) )
                  {
                     node.RemoveAttributeNode(attributeNode);
                  }
               }

               node.RemoveAttributeNode(defaultAttribute);
            }
         }
      }

      public static void MergePrefabInstancesForScene(PrefabManager prefabManager, XmlDocument open3dScene)
      {
      }
   }
}
