#region Using directives

using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Xml.XPath;
using System.Diagnostics;

#endregion

namespace Tools.Common
{
/// <summary>
/// XmlNodeHelpers contains classes and functions to aid processing of XmlDocument classes.
/// </summary>
public class XmlNodeHelpers
{
/// <summary>
/// DepthIterator is a XmlDocument helper class that assists with iterating over elements.
/// Uses the concept of 'Entering' (not processed child elements) and 'Exiting' (processed child elements) to
/// aid with property merging.
/// </summary>
public class DepthIterator
{
   private struct SIterData
   {
      public void Initialize(XmlElement element)
      {
         SetElement(element);
         mIteratorInfo = EIteratorInfo.kII_EnteredElement;         
      }
      
   //----------------------------------------------------------------------------      

      public void SetElement(XmlElement element)
      {
         mElement = element;
         mNodeInfo = XmlNodeHelpers.HasChildElements(mElement) ? ENodeInfo.kNI_Branch : ENodeInfo.kNI_Leaf;  
      }
      
   //----------------------------------------------------------------------------
   
      public void SetElement(XmlNode node)
      {
         mElement = node as XmlElement;
         mNodeInfo = XmlNodeHelpers.HasChildElements(mElement) ? ENodeInfo.kNI_Branch : ENodeInfo.kNI_Leaf;  
      }
      
   //----------------------------------------------------------------------------
   
      public void MoveToNextElement(EMoveMode moveMode)
      {
         // Are we going to skip children?
         bool bSkipChildren = (moveMode == EMoveMode.kMM_SkipChildren);
         
         // Are we moving up the tree?
         bool bExitingElement = (mIteratorInfo == EIteratorInfo.kII_ExitedElement);
         if (bExitingElement)
         {
            XmlElement siblingElement = GetNextSiblingElement(mElement);

            if (siblingElement != null)
            {
               // Current element has a sibling, move there
               mIteratorInfo = EIteratorInfo.kII_EnteredElement;
               mNodeInfo = ENodeInfo.kNI_Branch;
               SetElement(siblingElement);
            }
            else
            {
               // Current element has no more siblings, move to it's parent
               mIteratorInfo = EIteratorInfo.kII_ExitedElement;
               mNodeInfo = ENodeInfo.kNI_Branch;
               SetElement(mElement.ParentNode);
            }
         }
         // We are descending.
         else if (XmlNodeHelpers.HasChildElements(mElement) && !bSkipChildren)
         {
            // Descend depth first to children.
            mIteratorInfo = EIteratorInfo.kII_EnteredElement;
            SetElement(GetFirstChildElement(mElement));
         }
         else
         {
            XmlElement siblingElement = GetNextSiblingElement(mElement);

            if (siblingElement != null)
            {
               // We have no children, or are skipping children, move to sibling.
               mIteratorInfo = EIteratorInfo.kII_EnteredElement;
               SetElement(siblingElement);
            }
            else
            {
               // We have no children or siblings, we need to move back up.            
               mIteratorInfo = EIteratorInfo.kII_ExitedElement;
               SetElement(mElement.ParentNode);
            }
         }
      }
      
   //----------------------------------------------------------------------------
   
      public EIteratorInfo mIteratorInfo;
      public ENodeInfo     mNodeInfo;
      public XmlElement    mElement;
   }; // SIterData

//----------------------------------------------------------------------------

public enum EIteratorInfo
{
   kII_EnteredElement, // Not processed current element's children, iterator is moving down the xml tree
   kII_ExitedElement,  // Have processed current element's children, iterator is moving up the xml tree
   
   kII_Count,
}

private enum ENodeInfo
{
   kNI_Branch,
   kNI_SkippedBranch,
   kNI_Leaf,
   
   kII_Count,
}

private enum EMoveMode
{
   kMM_Normal,
   kMM_SkipChildren,
   
   kMM_Count,
};
   
private enum EInternalState
{
   kIS_FirstElement,
   kIS_Normal,
   kIS_LastElement,
   
   kIS_Count,
};
   
private enum EIteratorState
{
   kIS_Normal,
   kIS_HoldCurrentElementPosition,     // Used for when nodes are deleted
   
   kIS_Count,
};
   
private XmlElement       mTop_Element;
private SIterData        mCurrentIter;
private EMoveMode        mMoveMode;
private EInternalState   mInternalState;
private EIteratorState   mIterState;

//----------------------------------------------------------------------------

public DepthIterator(XmlElement top_Element)
{
   mTop_Element = top_Element;      
   mCurrentIter.Initialize(mTop_Element);      
   mMoveMode = EMoveMode.kMM_Normal;
   mInternalState = EInternalState.kIS_FirstElement;
   mIterState = EIteratorState.kIS_Normal;
}

//----------------------------------------------------------------------------

public XmlElement GetTopLevelElement()
{
   return mTop_Element;
}
//----------------------------------------------------------------------------

// Returns false when iterated over all elements
public bool MoveToNextElement()
{
   // Handle top level element
   if (mInternalState == EInternalState.kIS_FirstElement)
   {
      mInternalState = EInternalState.kIS_Normal;
      return true;
   }
   
   // Handle deleted elements
   if (mIterState == EIteratorState.kIS_HoldCurrentElementPosition)
   {
      mIterState = EIteratorState.kIS_Normal;
      return true;
   }
   
   // Did we reach the top element last time?
   if (mInternalState == EInternalState.kIS_LastElement)
   {
      // We're done
      return false;
   }
   
   // Check to see if previous branch element has been changed to a leaf (this can occur if user removes child nodes)
   // Check to see if previous leaf element has been changed to a branch (this can occur if user adds child nodes)
   if (mCurrentIter.mNodeInfo == ENodeInfo.kNI_Leaf || mCurrentIter.mNodeInfo == ENodeInfo.kNI_Branch)
   {
      mCurrentIter.mNodeInfo = XmlNodeHelpers.HasChildElements(mCurrentIter.mElement) ? ENodeInfo.kNI_Branch : ENodeInfo.kNI_Leaf;
   }
   
   // Handle leaf elements so we have both entered and exited iterator info for leaf nodes (and skipped branches)
   if (((mCurrentIter.mNodeInfo == ENodeInfo.kNI_Leaf) || (mCurrentIter.mNodeInfo == ENodeInfo.kNI_SkippedBranch)) && (mCurrentIter.mIteratorInfo == EIteratorInfo.kII_EnteredElement))
   {
      mCurrentIter.mIteratorInfo = EIteratorInfo.kII_ExitedElement;

      // Check for last element
      if (mCurrentIter.mElement == mTop_Element)
      {
         // The leaf we just exited is also the top most element, so mark this one as the last.
         mInternalState = EInternalState.kIS_LastElement;
      }
      
      return true;         
   }
   
   // OK, all special case conditions completed, move to next element
   mCurrentIter.MoveToNextElement(mMoveMode);      
   
   // Reset to normal move if required.
   if (mMoveMode != EMoveMode.kMM_Normal)
   {
      mMoveMode = EMoveMode.kMM_Normal;   
   }
   
   // Check for last element
   if ((mCurrentIter.mElement == mTop_Element) && (mCurrentIter.mIteratorInfo == EIteratorInfo.kII_ExitedElement))
   {
      // We've returned to the top, terminate next time around.
      mInternalState = EInternalState.kIS_LastElement;      
   }
   
   return true;
}

//----------------------------------------------------------------------------

public void SkipChildren()
{
   mMoveMode = EMoveMode.kMM_SkipChildren;
   if (mCurrentIter.mNodeInfo == ENodeInfo.kNI_Branch)
   {
      // Force this element to be a skipped branch so we get an exited element state
      mCurrentIter.mNodeInfo = ENodeInfo.kNI_SkippedBranch;      
   }
}

//----------------------------------------------------------------------------

public void RemoveCurrentElementAndChildren()
{
   XmlElement delete_Element = mCurrentIter.mElement;
   // Skip iteration over children
   if (mCurrentIter.mIteratorInfo == EIteratorInfo.kII_EnteredElement)
   {
      SkipChildren();   
      // Exit this element
      MoveToNextElement();
   }
   // Move past deleted node
   MoveToNextElement();
   
   // Make sure we don't skip over element entries because of deletion
   mIterState = EIteratorState.kIS_HoldCurrentElementPosition;
   
   // Finally delete element
   delete_Element.ParentNode.RemoveChild(delete_Element);
}

//----------------------------------------------------------------------------

public void RenameCurrentElement(string newElementName)
{
   XmlElement newElement = XmlNodeHelpers.RenameElement(mCurrentIter.mElement, newElementName);
   
   if( mTop_Element == mCurrentIter.mElement )
      mTop_Element = newElement;

   mCurrentIter.mElement = newElement;
}

//----------------------------------------------------------------------------

public void SetCurrentElement(XmlElement element)
{
   if (mTop_Element == mCurrentIter.mElement)
      mTop_Element = element;

   mCurrentIter.mElement = element;
}

//----------------------------------------------------------------------------
/// <summary>
/// Returns a human readable path to the element referenced by the iterator.
/// </summary>
/// <param name="additionalDisplayAttribute">
/// Displays the value of the named attribute if present.
/// </param>
public string GetDebugPathString(string additionalDisplayAttribute)
{
   int kMaxParents = 128;
   return GetDebugPathString(additionalDisplayAttribute, kMaxParents);
}

//----------------------------------------------------------------------------
/// <summary>
/// Returns a human readable path to the element referenced by the iterator.
/// </summary>
/// <param name="additionalDisplayAttribute">
/// Displays the value of the named attribute if present.
/// </param>
/// <param name="maxParents">
/// Limits how many parents to display, useful for reducing output string length.
/// </param>
/// <returns></returns>
public string GetDebugPathString(string additionalDisplayAttribute, int maxParents)
{
   string output = String.Empty;
   for (XmlElement currentElement = Element; (currentElement != mTop_Element.ParentNode) && (currentElement != null); currentElement = (currentElement.ParentNode as XmlElement))
   {
      // Check parent limit
      maxParents--;
      if (maxParents < 0)
      {
         if (maxParents == -1)
         {
            // Add delimiter
            output = "/" + output;         
         }
         // Add '.' for each skipped parent
         output = "." + output;
         continue;
      }

      // Fix formatting '.'
      if (output.Length != 0)
      {
         output = "." + output;
      }
      
      string displayAttribute = currentElement.GetAttribute(additionalDisplayAttribute);
      if (displayAttribute.Length != 0)
      {
         output = String.Format("{0}({1}){2}", currentElement.LocalName, displayAttribute, output);
      }
      else
      {
         // No attribute to display
         output = String.Format("{0}{1}", currentElement.LocalName, output);         
      }      
   }
   
   if (IteratorInfo == EIteratorInfo.kII_ExitedElement)
   {
      output = "-" + output;
   }
   else
   {
      output = "+" + output;
   }
   
   return output;
}

//----------------------------------------------------------------------------

/// <summary>
/// Returns the current element being processed by the iterator.
/// Use in conjuction with 'IteratorInfo' to process elements.
/// NOTE: Use the class functions for renaming and removing Elements, otherwise behavior may be undefined.
/// </summary>
public XmlElement Element
{
   get { return mCurrentIter.mElement; }
}

//----------------------------------------------------------------------------

/// <summary>
/// Returns if we have 'entered' the current element (i.e. not processed it's children and iterator is moving down the xml tree),
/// or if we have 'exited' the current element (i.e. iterated over all of it's child elements and iterator is moving up the xml tree).
/// </summary>
public EIteratorInfo IteratorInfo
{
   get { return mCurrentIter.mIteratorInfo; }
}

}; // DepthIterator

//----------------------------------------------------------------------------
// Free functions
//----------------------------------------------------------------------------

public static void DeleteAllFromXmlNode(XmlNode node, string xpathExpression)
{
   XmlNodeList nodesToDelete = node.SelectNodes(xpathExpression);
   foreach (XmlNode nodeToDelete in nodesToDelete)
   {
      switch(nodeToDelete.NodeType)
      {
         case XmlNodeType.Attribute:
            {
               XmlAttribute attr = (XmlAttribute)nodeToDelete;
               attr.OwnerElement.RemoveAttributeNode(attr);
            }
            break;

         default:
            nodeToDelete.ParentNode.RemoveChild(nodeToDelete);
            break;
      }
   }
   
   return;
}

//----------------------------------------------------------------------------

public static bool HasChildElements(XmlNode node)
{
   if (node.NodeType != XmlNodeType.Element)
   {
      return false;
   }
   
   if (!node.HasChildNodes)
   {
      return false;
   }

   return GetFirstChildElement(node) != null;
}

//----------------------------------------------------------------------------

public static XmlElement GetFirstChildElement(XmlNode node)
{
   for ( XmlNode child = node.FirstChild; child != null; child = child.NextSibling )
   {
      if (child.NodeType == XmlNodeType.Element)
         return (XmlElement) child;
   }

   return null;
}

//----------------------------------------------------------------------------
  
public static XmlElement GetNextSiblingElement(XmlNode node)
{
   XmlNode sibling = node.NextSibling;
   while( sibling != null && sibling.NodeType != XmlNodeType.Element)
   {
      sibling = sibling.NextSibling;
   }

   return sibling as XmlElement;
}

//----------------------------------------------------------------------------

public static void SwapSiblingElements( XmlElement a, XmlElement b )
{
   XmlElement parent = (XmlElement) a.ParentNode;

   XmlElement oneBeforeA = (XmlElement) a.PreviousSibling;

   if ( oneBeforeA == b )
   {
      // If b is the prior sibling of a, then we can do a quick swap
      parent.RemoveChild( a );
      parent.InsertBefore( a, b );
   }
   else
   {
      // Otherwise, we remove a, replace b with a, then add b after a's prior sibling
      parent.RemoveChild( a );
      parent.ReplaceChild( a, b );
      parent.InsertAfter( b, oneBeforeA );
   }
}
   
//----------------------------------------------------------------------------
/// <summary>
/// Removes all childElements except for childElementToKeep.
/// </summary>
public static void RemoveChildElements_KeepNamed(XmlElement element, string childElementToKeep)
{
   for (int childElementLoop = 0; childElementLoop < element.ChildNodes.Count; )
   {
      XmlNode childElement = element.ChildNodes[childElementLoop];      
      bool bDelete = (childElement is XmlElement) && (childElementToKeep != childElement.Name);
      if (bDelete)
      {
         element.RemoveChild(childElement);
      }
      else
      {
         childElementLoop++;      
      }
   }   

   return;
}
//----------------------------------------------------------------------------
/// <summary>
/// Removes all childElements not in childElementsToKeep array.
/// </summary>
public static void RemoveChildElements_KeepNamed(XmlElement element, string[] childElementsToKeep)
{
   for (int childElementLoop = 0; childElementLoop < element.ChildNodes.Count; )
   {
      XmlNode childElement = element.ChildNodes[childElementLoop];      
      bool bDelete = (childElement is XmlElement) && (Array.IndexOf(childElementsToKeep, childElement.Name) == -1);
      if (bDelete)
      {
         element.RemoveChild(childElement);
      }
      else
      {
         childElementLoop++;      
      }
   }
   
   return;
}

//----------------------------------------------------------------------------
/// <summary>
/// Removes childElement childElementToDelete, keeps the rest.
/// </summary>
public static void RemoveChildElements_DeleteNamed(XmlElement element, string childElementToDelete)
{
   for (int childElementLoop = 0; childElementLoop < element.ChildNodes.Count; )
   {
      XmlNode childElement = element.ChildNodes[childElementLoop];      
      bool bDelete = (childElement is XmlElement) && (childElementToDelete == childElement.Name);
      if (bDelete)
      {
         element.RemoveChild(childElement);
      }
      else
      {
         childElementLoop++;      
      }
   }   
   
   return;
}

//----------------------------------------------------------------------------
/// <summary>
/// Removes childElements in childElementsToDelete, keeps the rest.
/// </summary>
public static void RemoveChildElements_DeleteNamed(XmlElement element, string[] childElementsToDelete)
{
   for (int childElementLoop = 0; childElementLoop < element.ChildNodes.Count; )
   {
      XmlNode childElement = element.ChildNodes[childElementLoop];      
      bool bDelete = (childElement is XmlElement) && (Array.IndexOf(childElementsToDelete, childElement.Name) != -1);
      if (bDelete)
      {
         element.RemoveChild(childElement);
      }
      else
      {
         childElementLoop++;      
      }
   }
   
   return;
}
//----------------------------------------------------------------------------
/// <summary>
/// Removes all attributes except for attributeToKeep.
/// </summary>
public static void RemoveAttributes_KeepNamed(XmlElement element, string attributeToKeep)
{
   for (int attribLoop = 0; attribLoop < element.Attributes.Count; )
   {
      XmlAttribute attrib = element.Attributes[attribLoop];      
      bool bDelete = (attributeToKeep != attrib.Name);
      if (bDelete)
      {
         element.RemoveAttributeNode(attrib);
      }
      else
      {
         attribLoop++;      
      }
   }
   
   return;
}
//----------------------------------------------------------------------------
/// <summary>
/// Removes all attributes not in attributesToKeep array.
/// </summary>
public static void RemoveAttributes_KeepNamed(XmlElement element, string[] attributesToKeep)
{
   for (int attribLoop = 0; attribLoop < element.Attributes.Count; )
   {
      XmlAttribute attrib = element.Attributes[attribLoop];      
      bool bDelete = (Array.IndexOf(attributesToKeep, attrib.Name) == -1);
      if (bDelete)
      {
         element.RemoveAttributeNode(attrib);
      }
      else
      {
         attribLoop++;      
      }
   }
   
   return;
}

//----------------------------------------------------------------------------
/// <summary>
/// Removes attribute attributeToDelete, keeps the rest.
/// </summary>
public static void RemoveAttributes_DeleteNamed(XmlElement element, string attributeToDelete)
{
   for (int attribLoop = 0; attribLoop < element.Attributes.Count; )
   {
      XmlAttribute attrib = element.Attributes[attribLoop];      
      bool bDelete = (attributeToDelete == attrib.Name);
      if (bDelete)
      {
         element.RemoveAttributeNode(attrib);
      }
      else
      {
         attribLoop++;      
      }
   }
   
   return;
}

//----------------------------------------------------------------------------
/// <summary>
/// Removes attributes in attributesToDelete, keeps the rest.
/// </summary>
public static void RemoveAttributes_DeleteNamed(XmlElement element, string[] attributesToDelete)
{
   for (int attribLoop = 0; attribLoop < element.Attributes.Count; )
   {
      XmlAttribute attrib = element.Attributes[attribLoop];      
      bool bDelete = (Array.IndexOf(attributesToDelete, attrib.Name) != -1);
      if (bDelete)
      {
         element.RemoveAttributeNode(attrib);
      }
      else
      {
         attribLoop++;      
      }
   }
   
   return;
}

//----------------------------------------------------------------------------

public static XmlElement RenameElement(XmlElement old_Element, string newElementName)
{
   XmlElement new_Element = old_Element.OwnerDocument.CreateElement(newElementName);
   
   // Copy attributes
   while (old_Element.Attributes.Count != 0)
   {
      XmlAttribute attrib = old_Element.Attributes[0];
      old_Element.RemoveAttributeNode(attrib);
      new_Element.SetAttributeNode(attrib);
   }
   
   // Copy children
   while (old_Element.HasChildNodes)
   {
      XmlNode child_Node = old_Element.FirstChild;
      old_Element.RemoveChild(child_Node);
      new_Element.AppendChild(child_Node);
   }
   
   // Replace old element with named element
   old_Element.ParentNode.ReplaceChild(new_Element, old_Element);
   
   return new_Element;
}

//----------------------------------------------------------------------------

public static bool AreElementAttributesEquivelent( XmlElement lhs, XmlElement rhs )
{
   
   if ( rhs.Attributes.Count != lhs.Attributes.Count )
   {
      return false;
   }

   foreach ( XmlAttribute attr in lhs.Attributes )
   {
      if ( !rhs.HasAttribute( attr.Name ) || rhs.GetAttribute( attr.Name ) != attr.Value )
      {
         return false;
      }
   }

   return true;
}
   
//----------------------------------------------------------------------------

public static XmlElement GetDocumentRootElement( XmlDocument doc )
{
   return doc.DocumentElement;
}

//----------------------------------------------------------------------------

public static XmlDocument CreateDocumentFromNode(XmlNode node)
{
   if (node.NodeType == XmlNodeType.Document)
   {
      return node.CloneNode(true) as XmlDocument;
   }
   else
   {
      XmlDocument document = new XmlDocument();
      document.AppendChild(document.ImportNode(node, true));
      return document;
   }
}

//----------------------------------------------------------------------------

public static String EscapeXml( String toEscape )
{
   return System.Security.SecurityElement.Escape( toEscape );
}

//----------------------------------------------------------------------------

   public static XmlDocument LoadDocumentOrNull( String fileName )
   {
      try
      {
         if ( !System.IO.File.Exists( fileName ) )
         {
            return null;
         }

         XmlDocument doc = new XmlDocument();
         doc.Load( fileName );

         return doc;
      }
      catch (System.Exception)
      {
         return null;
      }
   }

//----------------------------------------------------------------------------

public static String AsQuotedXml( int toQuote )
{
   return '"' + toQuote.ToString() + '"';
}

//----------------------------------------------------------------------------

public static String AsQuotedXml( String toQuote )
{
   return '"' + EscapeXml( toQuote ) + '"';
}

//----------------------------------------------------------------------------

private class XPathToXmlNodeEnumerator : IEnumerator<XmlNode>
{
   public XPathToXmlNodeEnumerator( XPathNodeIterator it )
   {
      mEnum = it.GetEnumerator();
   }

   XmlNode IEnumerator<XmlNode>.Current
   {
      get
      {
         return (XmlNode) ( (XPathNavigator) mEnum.Current ).UnderlyingObject;
      }
   }

   #region IDisposable Members

   void IDisposable.Dispose()
   {
      // IEnumerator<T> needs a Dispose, but we have nothing to dispose of
   }

   #endregion

   #region IEnumerator Members

   object System.Collections.IEnumerator.Current
   {
      get { return ( (XPathNavigator) mEnum.Current ).UnderlyingObject; }
   }

   void System.Collections.IEnumerator.Reset()
   {
      mEnum.Reset();
   }

   bool System.Collections.IEnumerator.MoveNext()
   {
      return mEnum.MoveNext();
   }

   #endregion

   System.Collections.IEnumerator mEnum;

}

//----------------------------------------------------------------------------

public class XPathToXmlNodeIterator : IEnumerable<XmlNode>
{
   public XPathToXmlNodeIterator( XPathNodeIterator it )
   {
      mIt = it;
   }

   #region IEnumerable<XmlNode> Members

   IEnumerator<XmlNode> IEnumerable<XmlNode>.GetEnumerator()
   {
      return new XPathToXmlNodeEnumerator( mIt );
   }

   #endregion

   #region IEnumerable Members

   System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
   {
      return new XPathToXmlNodeEnumerator( mIt );
   }

   #endregion

   public int Count { get { return mIt.Count; } }
   public XmlNode Current
   {
      get
      {
         return (XmlNode) mIt.Current.UnderlyingObject;
      }
   }
   public bool MoveNext()
   {
      return mIt.MoveNext();
   }

   XPathNodeIterator mIt;
}

//----------------------------------------------------------------------------

public static XPathToXmlNodeIterator SelectNodes( XmlNode context, XPathExpression compiledExpression )
{
   XPathNavigator nav = context.CreateNavigator();
   return new XPathToXmlNodeIterator( nav.Select( compiledExpression ) );
}

} // XmlNodeHelpers

} // Tools.Common
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    