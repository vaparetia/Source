using System;
using System.Collections.Generic;
using System.Text;

namespace Tools.WorkQueue
{
   /// <summary>
   ///   Provides a pool of resources that can be used to perform a <see cref="WorkItem">work item</see>.
   /// </summary>
   public interface IResourcePool
   {
      /// <summary>
      ///   Requests that a <see cref="WorkItem">work item</see> is performed by a resource
      ///   in the pool.
      /// </summary>
      void BeginWork(WorkItem workItem);
      /// <summary>
      /// Don't call this unless you know what you are doing.
      /// </summary>
      void ShutDown();
   }
}
