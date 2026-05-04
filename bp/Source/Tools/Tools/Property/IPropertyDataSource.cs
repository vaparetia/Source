using System;
using System.Collections.Generic;
using System.Text;
using System.Xml.XPath;

namespace Tools.Property.Source
{
   public enum ESourceVersion
   {
      kSourceVersion_1,
      kSourceVersion_2
   };

   public interface IPropertyDataSource
   {

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
      XPathNavigator FindPropertyObjectById(string id);
      XPathNavigator FindPropertyObjectById(string id, ESourceVersion version);

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
      XPathNavigator FindMessageObjectById(string id);
      XPathNavigator FindMessageObjectById(string id, ESourceVersion version);

      /// <summary>
      /// Retrieves an evaluator by id.
      /// </summary>
      /// <param name="id">
      /// Id is unique identifier of a evaluator object.
      /// </param>
      /// <returns>
      /// XPath to evaluator object. null if id cannot be found.
      /// </returns>
      XPathNavigator FindEvaluatorById(string id);
      XPathNavigator FindEvaluatorById(string id, ESourceVersion version);
      
      /// <summary>
      /// Retrieves list of evaluators by type (float, int, etc).
      /// </summary>
      List<XPathNavigator> FindEvaluatorsByType(string type);
      List<XPathNavigator> FindEvaluatorsByType(string type, ESourceVersion version);
      
      /// <summary>
      /// Retrieves a component by group and id.
      /// We use both together so we can use multiple duplicate component names. i.e. we don't require component names to be unique.
      /// However, the 'group' names must be.
      /// </summary>
      /// <param name="groupAndId">
      /// groupAndId is unique identifier of a component object.
      /// It consists of the group name followed by a colon, followed by component id.
      /// e.g. Timer:BasicTimer.
      /// </param>
      /// <returns>
      /// XPath to component object. null if groupAndId cannot be found.
      /// </returns>
      XPathNavigator FindComponentByGroupAndId(string groupAndId);
      XPathNavigator FindComponentByGroupAndId(string groupAndId, ESourceVersion version);

      /// <summary>
      /// Retrieves list of components by group.
      /// </summary>
      /// <param name="group">
      /// Type is list of components you want to receive.
      /// </param>
      /// <returns>
      /// list of XPaths to component objects. null if group cannot be found.
      /// </returns>
      List<XPathNavigator> FindComponentsByGroup(string group);
      List<XPathNavigator> FindComponentsByGroup(string group, ESourceVersion version);

      /// <summary>
      /// Retrieves a script object by id.
      /// </summary>
      /// <param name="id">
      /// Id is unique identifier of a script object.
      /// </param>
      /// <returns>
      /// XPath to script object. null if id cannot be found.
      /// </returns>
      XPathNavigator FindPropertyContainerById(string id);
      XPathNavigator FindPropertyContainerById(string id, ESourceVersion version);      

      /// <summary>
      /// Returns list of all script objects that match the given type
      /// </summary>
      /// <returns>list of all type ids</returns>
      List<string> GetAllPropertyContainers(string typeName);
      
      /// <summary>
      /// Returns list of all component group ids
      /// </summary>
      List<string> GetAllComponentGroupIds();
      
      /// <summary>
      /// This reloads all types, used in maya to get newly edited types without having to restart application.
      /// </summary>
      void RefreshTypes();

      /// <summary>
      /// This function returns the file path that this navigator belongs to (this requires for the navigator to actually be one of the ones retrieved inside this class), otherwise it will return an empty string.
      /// </summary>
      string GetFilePathForNavigator(XPathNavigator navigator);
   }
}
