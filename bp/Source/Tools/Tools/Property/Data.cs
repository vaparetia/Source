using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using Tools.Common;
using Tools.Property.Source;

namespace Tools.Property
{
   public class Data
   {
      public class PropertyContainerDocument : ICloneable
      {
         public PropertyContainerDocument( XmlDocument doc )
         {
            mDocument = doc;
         }

         public static PropertyContainerDocument FromComponentGroupDocument( XmlDocument doc, out ComponentGroup componentGroup )
         {
            XmlDocument myDocument = new XmlDocument();
            XmlElement propertyContainer = myDocument.CreateElement( PropertyContainer.kTypeString );
            XmlElement componentGroupXml = (XmlElement) myDocument.ImportNode( doc.DocumentElement, true );
            myDocument.AppendChild( propertyContainer );
            propertyContainer.AppendChild( componentGroupXml );

            componentGroup = (ComponentGroup) DataNodeFromElement( componentGroupXml );

            PropertyContainerDocument newDoc = new PropertyContainerDocument( myDocument );
            return newDoc;
         }

         public XmlDocument Document
         {
            get { return mDocument; }
         }

         public PropertyContainer PropertyContainer
         {
            get { return PropertyContainerFromDocument( mDocument ); }
         }

         public DataNode Child
         {
            get { return DataNodeFromElement(mDocument.DocumentElement);  }         
         }

         public String GetValueAtLabel( String label )
         {
            return Tools.Property.Misc.Helpers.GetValueAtLabel( mDocument, label );
         }

         public void SetValueAtLabel( String label, String value )
         {
            Tools.Property.Misc.Helpers.SetValueAtLabel( mDocument, label, value );
         }

         public PropertyContainerDocument Clone()
         {
            return new PropertyContainerDocument( (XmlDocument) mDocument.Clone() );
         }

         object ICloneable.Clone()
         {
            return this.Clone();
         }

         static public void ChangeActiveComponentGroupInPropertyContainers( 
            IEnumerable< PropertyContainerDocument > docs, 
            ComponentGroup groupTemplate,
            String newActiveGroup, 
            bool useDefaultMerge,
            IPropertyDataSource dataSource )
         {
            List<XmlDocument> docCopies = new List<XmlDocument>();
            foreach ( PropertyContainerDocument doc in docs )
            {
               docCopies.Add( doc.Document );
            }

            Misc.Helpers.ChangeComponentGroupInPropertyContainers( docCopies, groupTemplate.Element, newActiveGroup, useDefaultMerge, dataSource );
         }

         XmlDocument mDocument;
      }

      static private DataNode DataNodeFromElement( XmlElement element )
      {
         if ( element == null )
         {
            return null;
         }

         switch ( element.Name )
         {
         case Component.kTypeString:
            return new Component( element );
         case ComponentGroup.kTypeString:
            return new ComponentGroup( element );
         case PropertyContainer.kTypeString:
            return new PropertyContainer( element );
         case Message.kTypeString:
            return new Message( element );
         case Property.kTypeString:
            return new Property( element );
         case EvaluatorProperty.kTypeString:
            return new EvaluatorProperty( element );
         case Evaluator.kTypeString:
            return new Evaluator( element );
         case MessageObject.kTypeString:
            return new MessageObject( element );
         case PropertyObject.kTypeString:
            return new PropertyObject( element );
         case EnumEntry.kTypeString:
            return new EnumEntry( element );
         case Layout.kTypeString:
            return new Layout( element );
         case Event.kTypeString:
            return new Event( element );
         case Link.kTypeString:
            return new Link( element );
         }

         return new DataNode( element );
      }

      static public PropertyContainer PropertyContainerFromDocument( XmlDocument doc )
      {
         PropertyContainer container = DataNodeFromElement( doc.DocumentElement ) as PropertyContainer;

         return container;
      }

      public class DataNodeImplicitCollection<T> : IEnumerable<T> where T:DataNode
      {
         private class Enumerator : IEnumerator<T>
         {
            public Enumerator( System.Collections.IEnumerator xmlNodeEnumerator )
            {
               mXmlNodeEnum = xmlNodeEnumerator;
            }

            public T Current
            {
               get
               {
                  XmlElement element = (XmlElement) mXmlNodeEnum.Current;

                  if ( element == null )
                  {
                     return default( T );
                  }
                  else
                  {
                     return (T) DataNodeFromElement( element );
                  }
               }
            }

            public void Dispose()
            {
            }

            object System.Collections.IEnumerator.Current
            {
               get { return this.Current; }
            }

            public bool MoveNext()
            {
               return mXmlNodeEnum.MoveNext();
            }

            public void Reset()
            {
               mXmlNodeEnum.Reset();
            }

            System.Collections.IEnumerator mXmlNodeEnum;
         }

         internal DataNodeImplicitCollection( System.Collections.IEnumerable xmlElements )
         {
            mXmlElements = xmlElements;
         }

         public IEnumerator<T> GetEnumerator()
         {
            return new Enumerator( mXmlElements.GetEnumerator() );
         }
         System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
         {
            return this.GetEnumerator();
         }

         System.Collections.IEnumerable mXmlElements;
   }

      public class DataNodeExplicitCollection<T> : IEnumerable<T> where T:DataNode
      {
         private class Enumerator : IEnumerator<T>
         {
            public Enumerator( System.Collections.IEnumerator xmlNodeEnumerator )
            {
               mXmlNodeEnum = xmlNodeEnumerator;
            }

            public T Current
            {
               get 
               {
                  XmlElement element = (XmlElement) mXmlNodeEnum.Current;

                  if ( element == null )
                  {
                     return default(T);
                  }
                  else
                  {
                     return (T) DataNodeFromElement( element );
                  }
               }
            }

            public void Dispose()
            {
            }

            object System.Collections.IEnumerator.Current
            {
               get { return this.Current; }
            }

            public bool MoveNext()
            {
               return mXmlNodeEnum.MoveNext();
            }

            public void Reset()
            {
               mXmlNodeEnum.Reset();
            }

            System.Collections.IEnumerator mXmlNodeEnum;
         }

         internal DataNodeExplicitCollection( System.Collections.IEnumerable xmlElements )
         {
            mXmlElements = xmlElements;
         }

         public IEnumerator<T> GetEnumerator()
         {
            return new Enumerator( mXmlElements.GetEnumerator() );
         }
         System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
         {
            return this.GetEnumerator();
         }

         System.Collections.IEnumerable mXmlElements;
      }

      public class DataNodeList<T> : DataNodeImplicitCollection< T > where T:DataNode
      {
         public DataNodeList( XmlNodeList nodeList )
            : base( nodeList )
         {
            mXmlNodeList = nodeList;
         }

         public DataNodeList( System.Collections.IList nodeList )
            : base( nodeList )
         {
            mGenericNodeList = nodeList;
         }

         public int Count
         {
            get 
            {
               if ( mXmlNodeList != null )
               {
                  return mXmlNodeList.Count;
               }
               else if ( mGenericNodeList != null )
               {
                  return mGenericNodeList.Count;
               }
               else
               {
                  return 0;
               }
            }
         }

         XmlNodeList mXmlNodeList = null;
         System.Collections.IList mGenericNodeList = null;
      }

      public interface IDataNodeExportsVisibility
      {
         bool IsVisible
         {
             get;
         }
      }

      public class DataNode : IEquatable<DataNode>
      {
         internal DataNode( XmlElement element )
         {
            mElement = element;
         }

         public virtual String GetTypeString()
         {
            return "";
         }

         public XmlElement Element
         {
            get { return mElement; }
         }

         public DataNodeList<DataNode> ChildNodes
         {
            get { return new DataNodeList<DataNode>( mElement.ChildNodes ); }
         }

         public DataNodeList<DataNode> SelectNodes( String xpathQuery )
         {
            return new DataNodeList<DataNode>( mElement.SelectNodes( xpathQuery ) );
         }

         public DataNode SelectSingleNode( String xpathQuery )
         {
            return DataNodeFromElement( (XmlElement) mElement.SelectSingleNode( xpathQuery ) );
         }

         public String GetValueAtLabel( String label )
         {
            return Tools.Property.Misc.Helpers.GetValueAtLabel( mElement, label );
         }

         public void SetValueAtLabel( String label, String value )
         {
            Tools.Property.Misc.Helpers.SetValueAtLabel( mElement, label, value );
         }

         public DataNode FindSubnodeWithName( String itemName )
         {
            return SelectSingleNode( String.Format( "./*[@{0}={1}]", "name", XmlNodeHelpers.AsQuotedXml( itemName ) ) );
         }

         public DataNode FindSubnodeOfTypeWithName( String type, String itemName )
         {
            return SelectSingleNode( String.Format( "./{0}[@{1}={2}]", type, "name", XmlNodeHelpers.AsQuotedXml( itemName ) ) );
         }

         public bool IsGrouped
         {
            get
            {
               return Misc.Helpers.GetPropertyGrouped( mElement );
            }

            set
            {
               Misc.Helpers.SetPropertyGrouped( mElement, value );
            }
            
         }

         public String Label
         {
            get { return mElement.GetAttribute( "label" ); }
         }

         public String DisplayName
         {
            get { return Misc.Helpers.GetDisplayName( mElement ); }
         }

         public String TooltipText
         {
            get { return Misc.Helpers.GetToolTipText( mElement ); }
         }

         public DataNode Parent
         {
            get { return DataNodeFromElement( mElement.ParentNode as XmlElement ); }
         }

#region Overloaded

         public bool Equals( DataNode other )
         {
            return mElement == other.mElement;
         }

         public static bool operator ==( DataNode lhs, DataNode rhs )
         {
            bool lhsnull = ( (object) lhs == null );
            bool rhsnull = ( (object) rhs == null );

            if ( lhsnull || rhsnull )
            {
               return lhsnull && rhsnull;
            }
            else
            {
               return lhs.mElement == rhs.mElement;
            }
         }

         public static bool operator !=( DataNode lhs, DataNode rhs )
         {
            return !( lhs == rhs );
         }

         public override int GetHashCode()
         {
            return mElement.GetHashCode();
         }

         public override bool Equals( object o )
         {
            DataNode node = o as DataNode;
            if ( node != null )
            {
               return this.Equals( node );
            }
            else
            {
               return false;
            }
         }

#endregion

         protected XmlElement mElement;
      }

      public class PropertyContainer : DataNode
      {
         public const String kTypeString = "PropertyContainer";

         public override string GetTypeString()
         {
            return kTypeString;
         }

         internal PropertyContainer( XmlElement element )
            : base( element )
         {
         }

         public DataNodeList<ComponentGroup> ComponentGroups
         {
            get { return new DataNodeList<ComponentGroup>( mElement.SelectNodes( "ComponentGroup" ) ); }
         }

         public Guid EditorID
         {
            get { return new Guid( mElement.GetAttribute( "editorId" ) ); }
            set { mElement.SetAttribute( "editorId", value.ToString() ); }
         }

         public String Name
         {
            get { return mElement.GetAttribute( "name" ); }
            set { mElement.SetAttribute( "name", value ); }
         }

         public String GenerateNewDynamicComponentGroupName( String wantName )
         {
            return Misc.Helpers.GenerateNewDynamicComponentGroupName( Element.OwnerDocument, wantName );
         }

         public void AddDynamicComponentGroup( Source.IPropertyDataSource dataSource, string componentGroupId, string componentId, string name )
         {
            Misc.Helpers.AddDynamicComponentGroup( dataSource, Element.OwnerDocument, componentGroupId, componentId, name );
         }

         public void Remove( ComponentGroup group )
         {
            Misc.Helpers.DeleteTopLevelComponentGroup( Element.OwnerDocument, group.ID, group.ActiveComponentType, group.Name );
         }

         public void SwapComponentGroups( ComponentGroup a, ComponentGroup b )
         {
            if ( a.Parent != this || b.Parent != this )
            {
               throw new System.InvalidOperationException( "Tried to Swap two component groups owned by different property containers" );
            }
            else
            {
               XmlNodeHelpers.SwapSiblingElements( a.Element, b.Element );
            }
         }

         public ComponentGroup FindComponentGroupByName( String name )
         {
            return (ComponentGroup) SelectSingleNode( String.Format( "ComponentGroup[@name={0}]",
               XmlNodeHelpers.AsQuotedXml( name ) ) );
         }
      }

      public class ComponentGroup : DataNode
      {
         public const String kTypeString = "ComponentGroup";

         public override string GetTypeString()
         {
            return kTypeString;
         }

         internal ComponentGroup( XmlElement element )
            : base( element )
         {

         }

         public String Name
         {
            get { return mElement.GetAttribute( "name" ); }
            set { mElement.SetAttribute( "name", value ); }
         }

         public String ID
         {
            get { return mElement.GetAttribute( "id" ); }
            set { mElement.SetAttribute( "id", value ); }
         }

         public String ActiveComponentType
         {
            get { return Misc.Helpers.GetActiveComponentNameFromComponentGroup( Element ); }
            set { mElement.SetAttribute( "activeComponent", value ); }
         }

         public DataNodeList<Component> Components
         {
            get { return new DataNodeList<Component>( mElement.SelectNodes( "Component" ) ); }
         }

         public Component ActiveComponent
         {
            get
            {
               return (Component) DataNodeFromElement( Misc.Helpers.GetActiveComponentFromComponentGroup( Element ) );
            }
         }

         public bool IsPrefab
         {
            get { return Misc.Helpers.IsComponentGroupPrefab( Element ); }
         }

         public bool IsDynamic
         {
            get { return Misc.Helpers.IsComponentGroupDynamic( Element ); }
         }

         public List<String> GetValidComponentTypes( IPropertyDataSource propertyDb )
         {
            return Misc.Helpers.GetComponentsFromComponentGroup( Element, propertyDb );
         }
      }

      public class Component : DataNode
      {
         public const String kTypeString = "Component";

         public override string GetTypeString()
         {
            return kTypeString;
         }

         public Component( XmlElement element )
            : base( element )
         {
         }

         public String GroupName { get { return mElement.GetAttribute( "group" ); } }
         public String ID { get { return mElement.GetAttribute( "id" ); } }

         public DataNodeList<Message> Messages
         {
            get { return new DataNodeList<Message>( mElement.SelectNodes( "Messages/Message" ) ); }
         }

         public DataNodeList<Event> Events
         {
            get { return new DataNodeList<Event>( mElement.SelectNodes( "Events/Event" ) ); }
         }

         public DataNodeList<Link> Links
         {
            get { return new DataNodeList<Link>( mElement.SelectNodes( "Links/Link" ) ); }
         }

         public IEnumerable<String> PlatformCPPDefines
         {
            get
            {
               return Misc.Helpers.GetComponentPlatforms( mElement );
            }
         }
      }

      public class Message : DataNode
      {
         public const String kTypeString = "Message";

         public override string GetTypeString()
         {
            return kTypeString;
         }

         internal Message( XmlElement element )
            : base( element )
         {
         }

         public String Type
         {
            get { return mElement.GetAttribute( "type" ); }
            set { mElement.SetAttribute( "type", value ); }
         }
      }

      public class Event : DataNode
      {
         public const String kTypeString = "Event";

         public override string GetTypeString()
         {
            return kTypeString;
         }

         internal Event( XmlElement element )
            : base( element )
         {
         }

         public String Type
         {
            get { return mElement.GetAttribute( "type" ); }
            set { mElement.SetAttribute( "type", value ); }
         }
      }

      public class Link : DataNode
      {
         public const String kTypeString = "Link";

         public override string GetTypeString()
         {
            return kTypeString;
         }

         internal Link( XmlElement element )
            : base( element )
         {
         }

         public String Type
         {
            get { return mElement.GetAttribute( "type" ); }
            set { mElement.SetAttribute( "type", value ); }
         }
      }

      public class Property : DataNode, IDataNodeExportsVisibility
      {
         public const String kTypeString = "Property";

         public override string GetTypeString()
         {
            return kTypeString;
         }

         internal Property( XmlElement element )
            : base( element )
         {
         }

         public String ValueTextNullable
         {
            get
            {
               return Misc.Helpers.GetPropertyValueElement( Element );
            }
         }

         public String ValueText
         {
            get
            {
               return ValueTextNullable ?? "";
            }

            set
            {
               Misc.Helpers.SetPropertyValueElement( mElement, value );
            }
         }

         public bool IsVisible { get { return Misc.Helpers.IsVisible( Element ); } }
         public bool NeedsExportForUpdate { get { return Tools.Common.Misc.ParseBool( mElement.GetAttribute( Misc.Helpers.kProperty_NeedsExportForUpdate, String.Empty ) ); } }
         public String Name { get { return mElement.GetAttribute( "name" ); } }
         public String Type { get { return mElement.GetAttribute( "type" ); } }
         public String MinValue { get { return mElement.GetAttribute( "min" ); } }
         public String MaxValue { get { return mElement.GetAttribute( "max" ); } }
         public String MayaLabel { get { return mElement.GetAttribute( "mayaLabel" ); } }

         public bool IsModified
         {
            set { Misc.Helpers.SetPropertyModified( mElement, value ); }
            get { return Misc.Helpers.GetPropertyModified( mElement ); }
         }

         public String SubType { get { return mElement.GetAttribute( "subType" ); } }

         public string[] AssetTypes
         {
            get
            {
               return Misc.Helpers.GetAssetTypesForAssetProperty( Element );
            }
         }

         public EnumEntry FindEnumEntryByName( String name )
         {
            return (EnumEntry) SelectSingleNode( String.Format( "EnumEntry[@name={0}]", XmlNodeHelpers.AsQuotedXml( name ) ) );
         }

         public EnumEntry FindEnumEntryByDesc( String desc )
         {
            return (EnumEntry) SelectSingleNode( String.Format( "EnumEntry[@desc={0}]", XmlNodeHelpers.AsQuotedXml( desc ) ) );
         }

         public DataNodeList<EnumEntry> EnumEntries
         {
            get
            {
               return new DataNodeList<EnumEntry>( mElement.SelectNodes( "EnumEntry" ) );
            }
         }
      }

      public class EvaluatorProperty : DataNode, IDataNodeExportsVisibility
      {
         public const String kTypeString = "EvaluatorProperty";

         public override string GetTypeString()
         {
            return kTypeString;
         }


         internal EvaluatorProperty( XmlElement element )
            : base( element )
         {
         }

         public List<String> GetValidEvaluators( IPropertyDataSource propertyDb )
         {
            return Misc.Helpers.GetEvaluatorsForEvaluatorProperty( propertyDb, Element );
         }

         public String ActiveEvaluatorType
         {
            get { return Misc.Helpers.GetActiveEvaluatorFromEvaluatorProperty( Element ); }
         }

         public String Name { get { return mElement.GetAttribute( "name" ); } }
         public String InnerXml
         {
            get { return Element.InnerXml; }
            set { Element.InnerXml = value; }
         }

         public Evaluator Evaluator
         {
            get { return (Evaluator) SelectSingleNode( Misc.Helpers.kElement_Evaluator ); }
         }

         public void SetActiveEvaluatorType( String type, IPropertyDataSource propertyDb )
         {
            Misc.Helpers.SetActiveEvaluatorForEvaluatorProperty( propertyDb, Element, type );
         }

         public bool IsVisible
         {
            get { return Misc.Helpers.IsVisible( Element ); }
         }

         public String Type { get { return mElement.GetAttribute( "type" ); } }

         public bool IsModified
         {
            set
            {
               Misc.Helpers.SetPropertyModified( mElement, value );
            }

            get
            {
               return Misc.Helpers.GetPropertyModified( mElement );
            }
         }

      }

      public class Evaluator : DataNode
      {
         public const String kTypeString = "Evaluator";

         public override string GetTypeString()
         {
            return kTypeString;
         }

         public Evaluator( XmlElement element )
            : base( element )
         {
         }

         public String ID { get { return mElement.GetAttribute( "id" ); } }

      }

      public class Layout : DataNode
      {
         public const String kTypeString = "Layout";

         public override string GetTypeString() { return kTypeString; }

         internal Layout( XmlElement element )
            : base( element )
         {
         }

         public String Type { get { return mElement.GetAttribute( "type" ); } }
      }

      public class MessageObject : DataNode
      {
         public const String kTypeString = "MessageObject";

         public override string GetTypeString()
         {
            return kTypeString;
         }

         public MessageObject( XmlElement element )
            : base( element )
         {
         }

         public String ID { get { return mElement.GetAttribute( "id" ); } }
      }

      public class PropertyObject : DataNode, IDataNodeExportsVisibility
      {
         public const String kTypeString = "PropertyObject";

         public override string GetTypeString()
         {
            return kTypeString;
         }

         internal PropertyObject( XmlElement element )
            : base( element )
         {
         }

         public bool IsVisible { get { return Misc.Helpers.IsVisible( Element ); } }

         public bool IsVectorProperty
         {
            get { return Misc.Helpers.IsVectorProperty( Element ); }
         }
      }

      public class EnumEntry : DataNode
      {
         public const String kTypeString = "EnumEntry";

         public override string GetTypeString()
         {
            return kTypeString;
         }

         internal EnumEntry( XmlElement element )
            : base( element )
         {
         }

         public String Name { get { return mElement.GetAttribute( "name" ); } }
         public String Description { get { return mElement.GetAttribute( "desc" ); } }
      }

   }
}
