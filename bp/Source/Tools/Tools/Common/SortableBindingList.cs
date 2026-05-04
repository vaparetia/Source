#region Using directives

using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Runtime.Serialization.Formatters.Binary;
using System.Text;

#endregion

namespace Tools.Common
{
   // Persistance support removed
   // Stolen from http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dnforms/html/winforms02182005.asp
   // Modified slightly to deal with ListSortDirection and PropertyDescriptor.
   /// <summary>
   /// Use this to create sortable columns in a DataGridView.
   /// Currently used in Properties.MessageEditorGUI.
   /// </summary>
   /// <typeparam name="T"></typeparam>
   public class SortableBindingList<T> : BindingList<T>
   {
      private bool                  _isSorted;
      private PropertyDescriptor    _propertyDescriptor;
      private ListSortDirection     _listSortDirection;

      protected override bool SupportsSortingCore { get { return true; } }
      protected override bool IsSortedCore { get { return _isSorted; } }
      protected override void RemoveSortCore() { _isSorted = false; }
      protected override PropertyDescriptor SortPropertyCore { get { return _propertyDescriptor; } }
      protected override ListSortDirection SortDirectionCore { get { return _listSortDirection; } }

      protected override void ApplySortCore(PropertyDescriptor property, ListSortDirection direction)
      {
         _propertyDescriptor = property;
         _listSortDirection = direction;

         // Get list to sort
         List<T> items = this.Items as List<T>;

         // Apply and set the sort, if items to sort
         if( items != null )
         {
           PropertyComparer<T> pc = new PropertyComparer<T>(property, direction);
           items.Sort(pc);
           _isSorted = true;
         }
         else
         {
           _isSorted = false;
         }
         
         // Let bound controls know they should refresh their views
         this.OnListChanged(new ListChangedEventArgs(ListChangedType.Reset, -1));
      }
   }
}
