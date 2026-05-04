#region Using directives

using System;
using System.Collections.Generic;
using System.Xml;
using System.Xml.XPath;
using System.Diagnostics;
using System.IO;
using Tools.Property.Misc;
using System.Xml.Schema;
#endregion

namespace Tools.Property.Source
{  
   /// <summary>
   /// Used to build searchable arrays for property objects.
   /// Construct object, then call <see cref="AddFiles"/>.
   /// Use <see cref="FindById"/> as the  <see cref="MergeProperties.FindPropertyObjectById"/> delegate.
   /// </summary>
   /// 
   /// <remarks>
   /// Property objects elements look like this..
   /// 
   /// <para>
   /// &lt;PropertyObject id="some id"&gt;
   /// &lt;!---Xml data---&gt;
   /// &lt;/PropertyObject&gt;
   /// </para>
   /// The 'id' is added as a key to a map containing the xml object.
   /// </remarks>
   public class FileBasedPropertyObjectEnumerator : IPropertyDataSource
   {
      // Common delegate used for error output
      public delegate void ReportErrorDelegate(string errorString);

      /// <summary>
      /// Internal container class for property objects.
      /// </summary>
      private class PropertyObjectContainer
      {
         public string           mId;
         public string           mFilename;
         public string           mType;
         private XPathNavigator  mXMLNavV1;
         private XPathNavigator  mXMLNavV2;
         
         public PropertyObjectContainer(string id, string filename, string type, XPathNavigator xmlNavV1, XPathNavigator xmlNavV2)
         {
            mId = id;
            mFilename = filename;
            mType = type;
            mXMLNavV1 = xmlNavV1;
            mXMLNavV2 = xmlNavV2;
         }

         public XPathNavigator GetXPath(ESourceVersion version)
         {
            switch(version)
            {
               case ESourceVersion.kSourceVersion_1:
                  return mXMLNavV1;
               case ESourceVersion.kSourceVersion_2:
                  return mXMLNavV2;
               default:
                  return null;
            }
         }
      } 

      /// <summary>
      /// Internal container class for message objects.
      /// </summary>
      private class MessageObjectContainer
      {
         public string           mId;
         public string           mFilename;
         public string           mType;
         private XPathNavigator  mXMLNavV1;
         private XPathNavigator  mXMLNavV2;

         public MessageObjectContainer(string id, string filename, string type, XPathNavigator xmlNavV1, XPathNavigator xmlNavV2)
         {
            mId = id;
            mFilename = filename;
            mType = type;
            mXMLNavV1 = xmlNavV1;
            mXMLNavV2 = xmlNavV2;
         }

         public XPathNavigator GetXPath(ESourceVersion version)
         {
            switch (version)
            {
               case ESourceVersion.kSourceVersion_1:
                  return mXMLNavV1;
               case ESourceVersion.kSourceVersion_2:
                  return mXMLNavV2;
               default:
                  return null;
            }
         }

      } 
      
      /// <summary>
      /// Internal container class for evaluators
      /// </summary>
      private class EvaluatorContainer
      {
         public string           mId;
         public string           mFilename;
         public string           mType;
         public string           mScope;
         private XPathNavigator  mXMLNavV1;
         private XPathNavigator  mXMLNavV2;

         public EvaluatorContainer(string id, string filename, string type, string scope, XPathNavigator xmlNavV1, XPathNavigator xmlNavV2)
         {
            mId = id;
            mFilename = filename;
            mType = type;
            mScope = scope;
            mXMLNavV1 = xmlNavV1;
            mXMLNavV2 = xmlNavV2;
         }

         public XPathNavigator GetXPath(ESourceVersion version)
         {
            switch (version)
            {
               case ESourceVersion.kSourceVersion_1:
                  return mXMLNavV1;
               case ESourceVersion.kSourceVersion_2:
                  return mXMLNavV2;
               default:
                  return null;
            }
         }

      } 

      /// <summary>
      /// Internal container class for components.
      /// </summary>
      private class ComponentContainer
      {
         public string           mId;
         public string           mFilename;
         public string           mGroup;
         private XPathNavigator  mXMLNavV1;
         private XPathNavigator  mXMLNavV2;

         public ComponentContainer(string id, string filename, string group, XPathNavigator xmlNavV1, XPathNavigator xmlNavV2)
         {
            mId = id;
            mFilename = filename;
            mGroup = group;
            mXMLNavV1 = xmlNavV1;
            mXMLNavV2 = xmlNavV2;
         }
         
         public string GroupAndId
         {
            get { return mGroup + ":" + mId; }
         }

         public XPathNavigator GetXPath(ESourceVersion version)
         {
            switch (version)
            {
               case ESourceVersion.kSourceVersion_1:
                  return mXMLNavV1;
               case ESourceVersion.kSourceVersion_2:
                  return mXMLNavV2;
               default:
                  return null;
            }
         }

      } 

      /// <summary>
      /// Internal container class for scriptobjects.
      /// </summary>
      private class PropertyContainer
      {
         public string           mId;
         public string           mFilename;
         public string           mType;
         private XPathNavigator  mXMLNavV1;
         private XPathNavigator  mXMLNavV2;

         public PropertyContainer(string id, string filename, string type, XPathNavigator xmlNavV1, XPathNavigator xmlNavV2)
         {
            mId = id;
            mFilename = filename;
            mType = type;
            mXMLNavV1 = xmlNavV1;
            mXMLNavV2 = xmlNavV2;
         }

         public XPathNavigator GetXPath(ESourceVersion version)
         {
            switch (version)
            {
               case ESourceVersion.kSourceVersion_1:
                  return mXMLNavV1;
               case ESourceVersion.kSourceVersion_2:
                  return mXMLNavV2;
               default:
                  return null;
            }
         }

      } 
      
      public class FileSearchParams
      {
         public enum ESubDirs
         {
            kSearchSubDirs,
            kDontSearchSubDirs
         }
         
         public FileSearchParams(string searchPath, string searchPattern, ESubDirs searchSubDirs)
         {
            mSearchPath = searchPath;
            mSearchPattern = searchPattern;
            mSearchSubDirs = searchSubDirs;
         }
         
         /// <summary>
         /// Factory function for default search params for repository script types.
         /// </summary>
         static public FileSearchParams DefaultFileSearchParams(AssetSystem.Manager manager)
         {
            return new FileSearchParams(  manager.GetSystemPath("$/EditorSupport/ScriptTypes"),
                                          "*.xml", 
                                          FileSearchParams.ESubDirs.kSearchSubDirs);
         }
         
         public string     mSearchPath;
         public string     mSearchPattern;
         public ESubDirs   mSearchSubDirs;
      }
      
      Dictionary<string, PropertyObjectContainer>     mPropertyObjects           = new Dictionary<string, PropertyObjectContainer>();
      Dictionary<string, MessageObjectContainer>      mMessageObjects            = new Dictionary<string, MessageObjectContainer>();
      Dictionary<string, EvaluatorContainer>          mEvaluatorsById            = new Dictionary<string, EvaluatorContainer>();
      Dictionary<string, List<EvaluatorContainer> >   mEvaluatorsByType          = new Dictionary<string, List<EvaluatorContainer> >();
      Dictionary<string, ComponentContainer>          mComponentsByGroupAndId    = new Dictionary<string, ComponentContainer>();
      Dictionary<string, List<ComponentContainer> >   mComponentsByGroup         = new Dictionary<string, List<ComponentContainer> >();
      Dictionary<string, PropertyContainer>           mPropertyContainersById    = new Dictionary<string, PropertyContainer>();
      List<FileSearchParams>                          mFileSearchParams          = new List<FileSearchParams>();
      ReportErrorDelegate                             mReportErrorDelegate;
      XmlSchema                                       mSchema;

      public FileBasedPropertyObjectEnumerator()
      {
         LoadSchema();
      }

      public FileBasedPropertyObjectEnumerator(ReportErrorDelegate reportErrorDelegate)
      {
         mReportErrorDelegate = reportErrorDelegate;
         LoadSchema();
      }

      void ReportErrorInternal(string filename, string error)
      {
         if (mReportErrorDelegate != null)
         {
            mReportErrorDelegate(String.Format("Error: {0} - {1}", filename, error));
         }
      }

      /// <summary>
      /// Find all property objects specified by <paramref name="searchPath"/>
      /// and add them to searchable list.
      /// </summary>
      /// <param name="searchPath">
      /// Directory path to search. e.g. 'c:\script'.
      /// </param>
      /// <param name="searchPattern">
      /// Wildcard for file types. e.g. '*.xml'.
      /// </param>
      /// <param name="searchSubDirs">
      /// Recurse down subdirs?
      /// </param>
      public void AddFiles(FileSearchParams searchParams)
      {
         bool addSearchParams = true;
         AddFilesInternal(searchParams, addSearchParams);
      }
      
      void AddFilesInternal(FileSearchParams searchParams, bool addSearchParams)
      {
         DirectoryInfo dir = new DirectoryInfo(searchParams.mSearchPath);
         if (searchParams.mSearchSubDirs == FileSearchParams.ESubDirs.kSearchSubDirs)
         {
            DirectoryInfo[] dirItems = dir.GetDirectories();
            foreach(DirectoryInfo item in dirItems)
            {
               AddFilesInternal( new FileSearchParams(item.FullName,
                                                      "*.xml", 
                                                      FileBasedPropertyObjectEnumerator.FileSearchParams.ESubDirs.kSearchSubDirs),
                                 false);
            }
         }
         
         FileInfo[] fileItems = dir.GetFiles(searchParams.mSearchPattern);
         foreach(FileInfo item in fileItems)
         {
            AddFile(item.FullName);
         }
         
         if (addSearchParams) mFileSearchParams.Add(searchParams);
      }
      
      /// <summary>
      /// This reloads all types, used in maya to get newly edited types without having to restart application.
      /// </summary>
      public void RefreshTypes()
      {
         mPropertyObjects.Clear();
         mMessageObjects.Clear();
         mEvaluatorsById.Clear();
         mEvaluatorsByType.Clear();
         mComponentsByGroupAndId.Clear();
         mComponentsByGroup.Clear();
         mPropertyContainersById.Clear();
         foreach(FileSearchParams searchParams in mFileSearchParams)
         {
            bool addSearchParams = false;
            AddFilesInternal(searchParams, addSearchParams);
         }
      }

      /// <summary>
      /// Add property objects in <paramref name=" filename"/> to searchable list.
      /// </summary>
      /// <param name="filename">
      /// File with property objects.
      /// </param>
      public void AddFile(string filename)
      {
         try
         {
            AddFileInternal(filename);
         }
         catch (System.Exception ex)
         {
            // Catch all for exceptions so they get reported in a nicer way
            ReportErrorInternal(filename, ex.ToString());

            // Throw the exception again if the debugger is attached so we don't miss any
            if (System.Diagnostics.Debugger.IsAttached)
            {
               throw ex;
            }
         }
      }

      /// <summary>
      /// Internal (placeholder) helper function that will convert source properties to the desired format.
      /// We remove this function when we move fully to V2 properties.
      /// </summary>
      private void ConvertSourcePropertiesV2ToV1(ref XPathNavigator nav)
      {
         XPathNavigator parent_Nav = nav.Clone();
         parent_Nav.MoveToParent();
         Debug.Assert(parent_Nav.Name == "PropertyContainerFile", "Source properties for conversion must be below a PropertyContainerFile!");
         if (parent_Nav.GetAttribute("pcfVersion", String.Empty) == "2")
         {
            // Convert to source properties version 1 and replace current navigator.
            nav = Tools.Property.Build.MergePropertiesV2.ConvertSourcePropertiesV2ToV1(nav).FirstChild.CreateNavigator();            
         }
      }

      private void ValidationEventHandler(object obj, System.Xml.Schema.ValidationEventArgs args)
      {
         throw new Exception(string.Format("Script type validation error Line {0}, Col {1} in {2}: {3}", args.Exception.LineNumber, args.Exception.LinePosition, args.Exception.SourceUri, args.Message));
      }

      void AddFileInternal(string filename)
      {
         XmlDocument document = new XmlDocument();

         XmlReaderSettings settings = new XmlReaderSettings();
         settings.ValidationType = ValidationType.Schema;
         settings.IgnoreWhitespace = true;
         settings.Schemas.Add(mSchema);
         settings.ValidationEventHandler += new System.Xml.Schema.ValidationEventHandler(ValidationEventHandler);

         using ( XmlReader reader = XmlReader.Create(filename, settings) )
         {
            document.Load(reader);
                     
            {
               // Enumerate through top level elements until we find a property object
               XPathNavigator xmlNav = document.CreateNavigator();
               XPathNodeIterator xpathIter = xmlNav.Select("/PropertyContainerFile/PropertyObject[attribute::id][attribute::type]");
               while (xpathIter.MoveNext())
               {
                  XPathNavigator iterXmlNavV2 = xpathIter.Current.Clone();
                  string id = iterXmlNavV2.GetAttribute("id", String.Empty);
                  string type = iterXmlNavV2.GetAttribute("type", String.Empty);

                  XPathNavigator iterXmlNavV1 = iterXmlNavV2.Clone();
                  ConvertSourcePropertiesV2ToV1(ref iterXmlNavV1);
                  PropertyObjectContainer propContainer = new PropertyObjectContainer(id, filename, type, iterXmlNavV1, iterXmlNavV2);

                  // Error check
                  if (mPropertyObjects.ContainsKey(id))
                  {
                     ReportErrorInternal(filename, String.Format("{0} is a duplicate property object.", id));
                     continue;
                  }
                  mPropertyObjects.Add(id, propContainer);
               }
            }
            {
               // Enumerate through top level elements until we find a message object
               XPathNavigator xmlNav = document.CreateNavigator();
               XPathNodeIterator xpathIter = xmlNav.Select("/PropertyContainerFile/MessageObject[attribute::id][attribute::type]");
               while (xpathIter.MoveNext())
               {
                  XPathNavigator iterXmlNavV2 = xpathIter.Current.Clone();
                  string id = iterXmlNavV2.GetAttribute("id", String.Empty);
                  string type = iterXmlNavV2.GetAttribute("type", String.Empty);

                  XPathNavigator iterXmlNavV1 = iterXmlNavV2.Clone();
                  ConvertSourcePropertiesV2ToV1(ref iterXmlNavV1);

                  MessageObjectContainer propContainer = new MessageObjectContainer(id, filename, type, iterXmlNavV1, iterXmlNavV2);
                  // Error check
                  if (mMessageObjects.ContainsKey(id))
                  {
                     ReportErrorInternal(filename, String.Format("{0} is a duplicate message object.", id));
                     continue;
                  }

                  mMessageObjects.Add(id, propContainer);
               }
            }
            
            {
               // Enumerate through top level elements until we find an evaluator
               XPathNavigator xmlNav = document.CreateNavigator();
               XPathNodeIterator xpathIter = xmlNav.Select("/PropertyContainerFile/Evaluator[attribute::id][attribute::type]");
               while (xpathIter.MoveNext())
               {
                  XPathNavigator iterXmlNavV2 = xpathIter.Current.Clone();
                  string id = iterXmlNavV2.GetAttribute("id", String.Empty);
                  string type = iterXmlNavV2.GetAttribute("type", String.Empty);
                  string scope = iterXmlNavV2.GetAttribute("scope", String.Empty);

                  XPathNavigator iterXmlNavV1 = iterXmlNavV2.Clone();
                  ConvertSourcePropertiesV2ToV1(ref iterXmlNavV1);

                  EvaluatorContainer evalContainer = new EvaluatorContainer(id, filename, type, scope, iterXmlNavV1, iterXmlNavV2);
                  // Error check
                  if (mEvaluatorsById.ContainsKey(id))
                  {
                     ReportErrorInternal(filename, String.Format("{0} is a duplicate evaluator.", id));
                     continue;
                  }

                  mEvaluatorsById.Add(id, evalContainer);
                  
                  // Check for existing type
                  if (!mEvaluatorsByType.ContainsKey(type))
                  {
                     mEvaluatorsByType.Add(type, new List<EvaluatorContainer>());
                  }
                  // Add to existing list of types
                  mEvaluatorsByType[type].Add(evalContainer);               
               }
            }

            {
               // Enumerate through top level elements until we find components
               XPathNavigator xmlNav = document.CreateNavigator();
               XPathNodeIterator xpathIter = xmlNav.Select("/PropertyContainerFile/Component[attribute::id][attribute::group]");
               while (xpathIter.MoveNext())
               {
                  XPathNavigator iterXmlNavV2 = xpathIter.Current.Clone();
                  string id = iterXmlNavV2.GetAttribute("id", String.Empty);
                  string group = iterXmlNavV2.GetAttribute("group", String.Empty);

                  XPathNavigator iterXmlNavV1 = iterXmlNavV2.Clone();
                  ConvertSourcePropertiesV2ToV1(ref iterXmlNavV1);

                  ComponentContainer componentContainer = new ComponentContainer(id, filename, group, iterXmlNavV1, iterXmlNavV2);
                  // Error check
                  if (mComponentsByGroupAndId.ContainsKey(componentContainer.GroupAndId))
                  {
                     ReportErrorInternal(filename, String.Format("{0} is a duplicate component object.", componentContainer.GroupAndId));
                     continue;
                  }

                  mComponentsByGroupAndId.Add(componentContainer.GroupAndId, componentContainer);

                  // Check for existing type
                  if (!mComponentsByGroup.ContainsKey(group))
                  {
                     mComponentsByGroup.Add(group, new List<ComponentContainer>());
                  }
                  // Add to existing list of types
                  mComponentsByGroup[group].Add(componentContainer);               
               }
            }

            {
               // Enumerate through top level elements until we find PropertyContainers
               XPathNavigator xmlNav = document.CreateNavigator();
               XPathNodeIterator xpathIter = xmlNav.Select("/PropertyContainerFile/PropertyContainer[attribute::id][attribute::type]");
               while (xpathIter.MoveNext())
               {
                  XPathNavigator iterXmlNavV2 = xpathIter.Current.Clone();
                  string id = iterXmlNavV2.GetAttribute("id", String.Empty);
                  string type = iterXmlNavV2.GetAttribute("type", String.Empty);

                  XPathNavigator iterXmlNavV1 = iterXmlNavV2.Clone();
                  ConvertSourcePropertiesV2ToV1(ref iterXmlNavV1);

                  PropertyContainer scriptObjectContainer = new PropertyContainer(id, filename, type, iterXmlNavV1, iterXmlNavV2);
                  // Error check
                  if (mPropertyContainersById.ContainsKey(id))
                  {
                     ReportErrorInternal(filename, String.Format("{0} is a duplicate property container.", id));
                     continue;
                  }

                  mPropertyContainersById.Add(id, scriptObjectContainer);
               }
            }
         }
      }

      private void LoadSchema()
      {
         System.Reflection.Assembly assembly = System.Reflection.Assembly.GetExecutingAssembly();
         Stream stream = assembly.GetManifestResourceStream("Tools.Property.Schema.SourceXML.xsd");
         mSchema = XmlSchema.Read(stream, new System.Xml.Schema.ValidationEventHandler(ValidationEventHandler));
      }
      
      /// <summary>
      /// Retrieves a property object by id.
      /// </summary>
      /// <param name="id">
      /// Id is unique identifier of a property object.
      /// Test is case sensitive.
      /// </param>
      /// <returns>
      /// XPath to property object. null if id cannot be found.
      /// </returns>
      public XPathNavigator FindPropertyObjectById(string id)
      {
         return FindPropertyObjectById(id, ESourceVersion.kSourceVersion_1);
      }

      public XPathNavigator FindPropertyObjectById(string id, ESourceVersion version)
      {
         if (mPropertyObjects.ContainsKey(id))
         {
            return mPropertyObjects[id].GetXPath(version);
         }

         return null;
      }

      /// <summary>
      /// Retrieves a message object by id.
      /// </summary>
      /// <param name="id">
      /// Id is unique identifier of a message object.
      /// Test is case sensitive.
      /// </param>
      /// <returns>
      /// XPath to message object. null if id cannot be found.
      /// </returns>
      public XPathNavigator FindMessageObjectById(string id)
      {
         return FindMessageObjectById(id, ESourceVersion.kSourceVersion_1);
      }

      public XPathNavigator FindMessageObjectById(string id, ESourceVersion version)
      {
         if (mMessageObjects.ContainsKey(id))
         {
            return mMessageObjects[id].GetXPath(version);
         }

         return null;
      }

      /// <summary>
      /// Retrieves a evaluator.
      /// </summary>
      /// <param name="id">
      /// Id is unique identifier of an evaluator.
      /// Test is case sensitive.
      /// </param>
      /// <returns>
      /// XPath to evaluator. null if id cannot be found.
      /// </returns>
      public XPathNavigator FindEvaluatorById(string id)
      {
         return FindEvaluatorById(id, ESourceVersion.kSourceVersion_1);
      }

      public XPathNavigator FindEvaluatorById(string id, ESourceVersion version)
      {
         if (mEvaluatorsById.ContainsKey(id))
         {
            return mEvaluatorsById[id].GetXPath(version);
         }

         return null;
      }

      /// <summary>
      /// Retrieves a list of evaluators matching type (float, int, etc).
      /// </summary>
      public List<XPathNavigator> FindEvaluatorsByType(string id)
      {
         return FindEvaluatorsByType(id, ESourceVersion.kSourceVersion_1);
      }

      public List<XPathNavigator> FindEvaluatorsByType(string type, ESourceVersion version)
      {
         List<XPathNavigator> outputEvaluatorsList = new List<XPathNavigator>();
         if (mEvaluatorsByType.ContainsKey(type))
         {
            List<EvaluatorContainer> evaluatorList = mEvaluatorsByType[type]; 
            foreach(EvaluatorContainer evaluator in evaluatorList)
            {
               outputEvaluatorsList.Add(evaluator.GetXPath(version));
            }
            return outputEvaluatorsList;
         }

         return null;
      }
      
      /// <summary>
      /// Retrieves a component by group and id.
      /// </summary>
      /// <param name="groupAndId">
      /// groupAndId is unique identifier of a component object.
      /// It consists of the group name followed by a colon, followed by component id.
      /// e.g. Timer:BasicTimer.
      /// </param>
      /// <returns>
      /// XPath to component object. null if groupAndId cannot be found.
      /// </returns>
      public XPathNavigator FindComponentByGroupAndId(string id)
      {
         return FindComponentByGroupAndId(id, ESourceVersion.kSourceVersion_1);
      }

      public XPathNavigator FindComponentByGroupAndId(string groupAndId, ESourceVersion version)
      {
         if (mComponentsByGroupAndId.ContainsKey(groupAndId))
         {
            return mComponentsByGroupAndId[groupAndId].GetXPath(version);
         }

         return null;
      }

      /// <summary>
      /// Retrieves list of components by group.
      /// </summary>
      /// <param name="group">
      /// group is list of components you want to receive.
      /// </param>
      /// <returns>
      /// list of XPaths to component objects. null if group cannot be found.
      /// </returns>
      public List<XPathNavigator> FindComponentsByGroup(string id)
      {
         return FindComponentsByGroup(id, ESourceVersion.kSourceVersion_1);
      }

      public List<XPathNavigator> FindComponentsByGroup(string group, ESourceVersion version)
      {
         List<XPathNavigator> outputComponentsList = new List<XPathNavigator>();
         if (mComponentsByGroup.ContainsKey(group))
         {
            List<ComponentContainer> componentList = mComponentsByGroup[group]; 
            foreach(ComponentContainer component in componentList)
            {
               outputComponentsList.Add(component.GetXPath(version));
            }
            return outputComponentsList;
         }

         // Return empty list
         return outputComponentsList;
      }
      /// <summary>
      /// Retrieves a script object by id.
      /// </summary>
      /// <param name="id">
      /// Id is unique identifier of a script object.
      /// Test is case sensitive.
      /// </param>
      /// <returns>
      /// XPath to script object. null if id cannot be found.
      /// </returns>
      public XPathNavigator FindPropertyContainerById(string id)
      {
         return FindPropertyContainerById(id, ESourceVersion.kSourceVersion_1);
      }

      public XPathNavigator FindPropertyContainerById(string id, ESourceVersion version)
      {
         if (mPropertyContainersById.ContainsKey(id))
         {
            return mPropertyContainersById[id].GetXPath(version);
         }

         return null;
      }
      
      /// <summary>
      /// Returns list of all script objects that match the given type
      /// </summary>
      /// <returns>list of all type ids</returns>
      public List<string> GetAllPropertyContainers(string typeName)
      {
         List<string> scriptObjects = new List<string>();
         
         foreach( PropertyContainer scriptObject in mPropertyContainersById.Values )
         {
            if( string.Compare(scriptObject.mType, typeName, true) == 0 )
               scriptObjects.Add(scriptObject.mId);
         }

         return scriptObjects;
      }
      
      /// <summary>
      /// Returns list of all component group ids
      /// </summary>
      public List<string> GetAllComponentGroupIds()
      {
         List<string> componentGroupIds = new List<string>(mComponentsByGroup.Keys);
         
         return componentGroupIds;
      }

      public List<string> GetAllMessageIds()
      {
         List<string> messageIds = new List<string>(mMessageObjects.Keys);
         return messageIds;
      }

      public List<string> GetAllEvaluatorTypes()
      {
         List<string> evaluatorTypes = new List<string>(mEvaluatorsByType.Keys);
         return evaluatorTypes;
      }

      /// <summary>
      /// This function returns the file path that this navigator belongs to (this requires for the navigator to actually be one of the ones retrieved inside this class), otherwise it will return an empty string.
      /// </summary>
      public string GetFilePathForNavigator(XPathNavigator navigator)
      {
         foreach (PropertyObjectContainer container in mPropertyObjects.Values)
         {
            if (container.GetXPath(ESourceVersion.kSourceVersion_1) == navigator || container.GetXPath(ESourceVersion.kSourceVersion_2) == navigator)
               return container.mFilename;
         }

         foreach (MessageObjectContainer container in mMessageObjects.Values)
         {
            if (container.GetXPath(ESourceVersion.kSourceVersion_1) == navigator || container.GetXPath(ESourceVersion.kSourceVersion_2) == navigator)
               return container.mFilename;
         }

         foreach (EvaluatorContainer container in mEvaluatorsById.Values)
         {
            if (container.GetXPath(ESourceVersion.kSourceVersion_1) == navigator || container.GetXPath(ESourceVersion.kSourceVersion_2) == navigator)
               return container.mFilename;
         }

         foreach (ComponentContainer container in mComponentsByGroupAndId.Values)
         {
            if (container.GetXPath(ESourceVersion.kSourceVersion_1) == navigator || container.GetXPath(ESourceVersion.kSourceVersion_2) == navigator)
               return container.mFilename;
         }

         foreach (PropertyContainer container in mPropertyContainersById.Values)
         {
            if (container.GetXPath(ESourceVersion.kSourceVersion_1) == navigator || container.GetXPath(ESourceVersion.kSourceVersion_2) == navigator)
               return container.mFilename;
         }

         return "";
      }
   }
}

