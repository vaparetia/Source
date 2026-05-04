using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;

namespace Tools.WorkQueue
{
   /// <summary>
   ///   Represents the method that will handle the <see cref="WorkQueue.WorkerException"/> 
   ///   and <see cref="WorkThreadPool.ThreadException"/> events.
   /// </summary>
   /// <param name="sender">
   ///   The source of the event.
   /// </param>
   /// <param name="e">
   ///   A <see cref="ResourceExceptionEventArgs"/> than contains the event data.
   /// </param>
   public delegate void ResourceExceptionEventHandler(object sender, ResourceExceptionEventArgs e);

   /// <summary>
   ///   Provides data for the <see cref="WorkQueue.WorkerException"/> 
   ///   and <see cref="WorkThreadPool.ThreadException"/> events.
   /// </summary>
   public sealed class ResourceExceptionEventArgs : EventArgs
   {
      private object resource;
      private System.Exception exception;
      private WorkItem workItem;

      private ResourceExceptionEventArgs()
      {
      }

      /// <summary>
      ///   Initialise a new instance of the <see cref="ResourceExceptionEventArgs"/> class with the
      ///   specified resource and <see cref="System.Exception"/>.
      /// </summary>
      /// <param name="resource">
      ///   The <see cref="object"/> that raised the exception.
      /// </param>
      /// <param name="exception">
      ///   The <see cref="System.Exception"/> that occured.
      /// </param>
      /// <remarks>
      ///   Use this constructor to create and initialize a new instance of the <see cref="ResourceExceptionEventArgs"/>
      ///   with the specified <see cref="System.Exception"/>.
      /// </remarks>
      public ResourceExceptionEventArgs(object resource, Exception exception)
         : base()
      {
         this.resource = resource;
         this.exception = exception;
      }

      /// <summary>
      ///   Initialize a new instance of the <see cref="ResourceExceptionEventArgs"/> class with the
      ///   specified resource, <see cref="WorkItem"/> and <see cref="System.Exception"/>.
      /// </summary>
      /// <param name="resource">
      ///   The <see cref="object"/> that raised the exception.
      /// </param>
      /// <param name="workItem">
      ///   The <see cref="WorkItem"/> that the <paramref name="resource"/> was working on.
      /// </param>
      /// <param name="exception">
      ///   The <see cref="System.Exception"/> that occured.
      /// </param>
      /// <remarks>
      ///   Use this constructor to create and initialize a new instance of the <see cref="ResourceExceptionEventArgs"/>
      ///   with the specified <see cref="System.Exception"/>.
      /// </remarks>
      public ResourceExceptionEventArgs(object resource, WorkItem workItem, Exception exception)
         : base()
      {
         this.resource = resource;
         this.workItem = workItem;
         this.exception = exception;
      }

      /// <summary>
      ///   Gets the exception that occured.
      /// </summary>
      /// <value>
      ///   The <see cref="System.Exception"/> that occured.
      /// </value>
      public System.Exception Exception
      {
         get { return exception; }
      }

      /// <summary>
      ///   Gets the work item.
      /// </summary>
      /// <value>
      ///   A <see cref="WorkItem"/> or <b>null</b>.
      /// </value>
      public WorkItem WorkItem
      {
         get { return workItem; }
      }

      /// <summary>
      ///   Gets the resource that raised the exception.
      /// </summary>
      public object Resource
      {
         get { return resource; }
      }

   }
   
   public delegate void ChangedWorkItemStateEventHandler(object sender, ChangedWorkItemStateEventArgs e);

   public delegate void WorkItemEventHandler(object sender, WorkItemEventArgs e);

   public class WorkItemEventArgs : EventArgs
   {
      WorkItem mWorkItem;
      public WorkItem WorkItem
      {
         get { return mWorkItem; }
      }

      private WorkItemEventArgs()
      {
      }

      public WorkItemEventArgs(WorkItem workItem)
      : base()
      {
         mWorkItem = workItem;
      }

   }

   public class ChangedWorkItemStateEventArgs : WorkItemEventArgs
   {
      private WorkItemState mPreviousState;
      public WorkItemState PreviousState
      {
         get { return mPreviousState; }
      }

      public ChangedWorkItemStateEventArgs(WorkItem workItem, WorkItemState previousState)
         : base(workItem)
      {
         mPreviousState = previousState;
      }
   }

   /// <summary>
   ///   Allows concurrent execution of <see cref="WorkItem">work items</see>.
   /// </summary>
   public class WorkQueue
   {
      private Queue<WorkItem> queue;
      private IResourcePool resourcePool;
      private object completed = new object();
      private int concurrentLimit = 4;
      private bool pausing;

      private int runningItems;
      private int failingItems;
      private volatile Exception internalException;

      /// <summary>
      ///   A lock when accessing our events.
      /// </summary>
      private readonly object eventLock = new object();

      #region Constructors

      /// <summary>
      ///   Creates a new instance of the <see cref="WorkQueue"/> class.
      /// </summary>
      public WorkQueue()
      {
         queue = new Queue<WorkItem>();
      }
      #endregion

      #region Properties

      /// <summary>
      ///   Gets or sets the <see cref="IResourcePool"/> that performs the <see cref="WorkItem"/>.
      /// </summary>
      /// <value>
      ///   An object that implements <see cref="IResourcePool"/>.  The default is <see cref="WorkThreadPool.Default"/>.
      /// </value>
      /// <remarks>
      ///   The <b>WorkerPool</b> allocates "workers" to perform an <see cref="WorkItem"/>.
      ///   When the <see cref="WorkItem.State"/> of a <b>work item</b> becomes
      ///   <see cref="WorkItemState">Scheduled</see>, the <see cref="IResourcePool.BeginWork"/>
      ///   method of the <b>WorkerPool</b> is called.
      /// </remarks>
      public IResourcePool WorkerPool
      {
         get
         {
            if (resourcePool == null)
            {
               resourcePool = WorkThreadPool.Default;
            }
            return resourcePool;
         }
         set
         {
            resourcePool = value;
         }
      }

      /// <summary>
      ///   Gets the number of items that are waiting or running.
      /// </summary>
      /// <value>
      ///   The number of items that are waiting or running.
      /// </value>
      public int Count
      {
         get { return runningItems + queue.Count; }
      }

      #endregion

      #region Scheduling
      /// <summary>
      ///   Add some work to execute.
      /// </summary>
      /// <param name="workItem">
      ///   An <see cref="WorkItem"/> to execute.
      /// </param>
      /// <remarks>
      ///   If the <see cref="ConcurrentLimit"/> is not reached and not <see cref="Pause">pausing</see>, 
      ///   then the <paramref name="workItem"/>
      ///   is immediately executed on the <see cref="WorkerPool"/>.  Otherwise it is placed in a
      ///   holding queue and executed when another <see cref="WorkItem"/> completes.
      /// </remarks>
      public void Add(WorkItem workItem)
      {
         if (workItem == null)
            throw new ArgumentNullException("workItem");
         if (internalException != null)
            throw new NotSupportedException("WorkQueue encountered an internal error.", internalException);

         // Assign it to this queue.
         workItem.WorkQueue = this;

         // Can we schedule it for execution now?
         lock (this)
         {
            if (!pausing && runningItems < ConcurrentLimit)
            {
               workItem.State = WorkItemState.Scheduled;
            }
            else
            {
               // Add the workitem to queue.
               queue.Enqueue(workItem);
               workItem.State = WorkItemState.Queued;
            }
         }
      }

      private bool DoNextWorkItem()
      {
         lock (this)
         {
            // Get some work and start it.
            if (!pausing && runningItems < ConcurrentLimit && queue.Count != 0)
            {
               WorkItem item = queue.Dequeue();
               item.State = WorkItemState.Scheduled;
               return true;
            }
         }

         return false;
      }

      #endregion

      #region Queue Control
      /// <summary>
      ///   Gets or sets the limit on concurrently running <see cref="WorkItem">work items</see>.
      /// </summary>
      /// <value>
      ///   An <see cref="int"/>.  The default value is 4.
      /// </value>
      /// <remarks>
      ///   <b>ConcurrentLimit</b> is the maximum number of <see cref="WorkItem">work items</see>
      ///   that can be concurrently executing.
      /// </remarks>
      public int ConcurrentLimit
      {
         get
         {
            return concurrentLimit;
         }
         set
         {
            concurrentLimit = value;
         }
      }

      /// <summary>
      ///   Stop executing <see cref="WorkItem">work items</see>.
      /// </summary>
      /// <seealso cref="Resume"/>
      /// <remarks>
      ///   <b>Pause</b> inhibits the <see cref="Add"/> method from immediately executing a <see cref="WorkItem"/>.
      ///   However, work items that are already executing will continue to completion.
      ///   <para>
      ///   Calling <b>Pause</b> is equivalent to setting the <see cref="Pausing"/> property to <b>true</b>.
      ///   </para>
      /// </remarks>
      /// <seealso cref="Resume"/>
      public void Pause()
      {
         Pausing = true;
      }

      /// <summary>
      ///   Resume executing the <see cref="WorkItem">work items</see>.
      /// </summary>
      /// <remarks>
      ///   <para>
      ///   Calling <b>Resume</b> is equivalent to setting the <see cref="Pausing"/> property to <b>false</b>.
      ///   </para>
      /// </remarks>
      /// <seealso cref="Pause"/>
      public void Resume()
      {
         Pausing = false;
      }

      /// <summary>
      ///   Removes any <see cref="WorkItem"/> that is queued to execute.
      /// </summary>
      /// <remarks>
      ///   <b>Clear</b> removes any <see cref="WorkItem"/> that is queued to execute.
      ///   However, work items that are already executing will continue 
      ///   to completion.
      /// </remarks>
      public void Clear()
      {
         lock (this)
         {
            queue.Clear();
         }
      }

      /// <summary>
      ///   Determines if a <see cref="WorkItem"/> is only queued for execution.
      /// </summary>
      /// <value>
      ///   <b>true</b> if a <see cref="WorkItem"/> is only queued; otherwise, <b>false</b>
      ///   to indicate that a <b>WorkItem</b> can be executed.
      /// </value>
      /// <remarks>
      ///   Setting <b>Pausing</b> to <b>true, </b>inhibits the <see cref="Add"/> method from immediately
      ///   executing a <see cref="WorkItem"/>.  However, work items that are already executing will continue 
      ///   to completion.
      /// </remarks>
      public bool Pausing
      {
         get { return pausing; }
         set
         {
            if (pausing != value)
            {
               pausing = value;

               // Start executing some work.
               while (!pausing && DoNextWorkItem())
               {
               }
            }
         }
      }

      #endregion

      #region Waiting
      /// <summary>
      ///   Waits for all work to complete.
      /// </summary>
      /// <remarks>
      ///   <b>WaitAll</b> returns when all work is completed.
      ///   <para>
      ///   If the <b>WorkQueue</b> is <see cref="Pause">pausing</see> then the <see cref="InvalidOperationException"/>
      ///   is <c>throw</c> to avoid an infinite wait.
      ///   </para>
      ///   <para>
      ///   Any type of <see cref="Exception"/> is thrown when a WorkQueue <see cref="Thread"/> throws an
      ///   exception outside of the <see cref="WorkItem.Perform"/> method.
      ///   </para>
      /// </remarks>
      /// <exception cref="InvalidOperationException">
      ///   If the <b>WorkQueue</b> is <see cref="Pause">pausing</see>.
      /// </exception>
      /// <seealso cref="AllWorkCompleted">AllWorkCompleted event</seealso>
      public void WaitAll()
      {
         lock (this)
         {
            if (internalException != null)
               throw internalException;

            if (pausing)
               throw new InvalidOperationException("The queue is paused, no work will be performed.");

            if (runningItems == 0 && queue.Count == 0)
               return;
         }

         lock (completed)
         {
            if (internalException != null)
               throw internalException;

            if (runningItems == 0 && queue.Count == 0)
               return;

            Monitor.Wait(completed);

            if (internalException != null)
               throw internalException;
         }
      }

      /// <summary>
      ///   Waits for all work to complete or a specified amount of time elapses.
      /// </summary>
      /// <param name="timeout">
      ///   A <see cref="TimeSpan"/> representing the amount of time to wait before this method returns.
      /// </param>
      /// <returns>
      ///   <b>true</b> if all work is completed; otherwise, <b>false</b> to indicate that the specified
      ///   time has elapsed.
      /// </returns>
      /// <remarks>
      ///   <b>WaitAll</b> returns when all work is completed or the specified amount of time has elapsed.
      ///   <para>
      ///   Any type of <see cref="Exception"/> is thrown when a WorkQueue <see cref="Thread"/> throws an
      ///   exception outside of the <see cref="WorkItem.Perform"/> method.
      ///   </para>
      /// </remarks>
      /// <seealso cref="AllWorkCompleted">AllWorkCompleted event</seealso>
      public bool WaitAll(TimeSpan timeout)
      {
         lock (this)
         {
            if (internalException != null)
               throw internalException;
         }

         lock (completed)
         {
            if (runningItems == 0 && queue.Count == 0)
               return true;

            if (!Monitor.Wait(completed, timeout))
               return false;

            if (internalException != null)
               throw internalException;
         }

         return true;
      }
      #endregion

      #region Raised Events

      /// <summary>
      ///   Occurs when the state of a work item is changed.
      /// </summary>
      /// <remarks>
      ///   The <b>ChangedWorkItemState</b> event is raised when the <see cref="WorkItem.State"/>
      ///   property of a <see cref="WorkItem"/> is changed.
      /// </remarks>
      public event ChangedWorkItemStateEventHandler ChangedWorkItemState
      {
         add
         {
            lock (eventLock)
            {
               changedWorkItemState += value;
            }
         }
         remove
         {
            lock (eventLock)
            {
               changedWorkItemState -= value;
            }
         }
      }
      private ChangedWorkItemStateEventHandler changedWorkItemState;

      /// <summary>
      ///   Raises the <see cref="ChangedWorkItemState"/> event.
      /// </summary>
      /// <param name="workItem">
      ///   The <see cref="WorkItem"/> that has changed <see cref="WorkItem.State"/>.
      /// </param>
      /// <param name="previousState">
      ///    One of the <see cref="WorkItemState"/> values indicating the previous state of the <paramref name="workItem"/>.
      /// </param>
      /// <remarks>
      ///   The <b>OnChangedWorkItemState</b> method allows derived classes to handle the event without attaching a delegate. This
      ///   is the preferred technique for handling the event in a derived class.
      ///   <para>
      ///   When a derived class calls the <b>OnChangedWorkItemState</b> method, it raises the <see cref="ChangedWorkItemState"/> event by 
      ///   invoking the event handler through a delegate. For more information, see 
      ///   <a href="ms-help://MS.VSCC.2003/MS.MSDNQTR.2004JAN.1033/cpguide/html/cpconProvidingEventFunctionality.htm">Raising an Event</a>.
      ///   </para>
      /// </remarks>
      protected virtual void OnChangedWorkItemState(WorkItem workItem, WorkItemState previousState)
      {
         ChangedWorkItemStateEventHandler handler;

         lock (eventLock)
         {
            handler = changedWorkItemState;
         }
         if (handler != null)
         {
            handler(this, new ChangedWorkItemStateEventArgs(workItem, previousState));
         }
      }

      /// <summary>
      ///   Occurs when the last <see cref="WorkItem"/> has completed.
      /// </summary>
      public event EventHandler AllWorkCompleted
      {
         add
         {
            lock (eventLock)
            {
               allWorkCompleted += value;
            }
         }
         remove
         {
            lock (eventLock)
            {
               allWorkCompleted -= value;
            }
         }
      }
      private EventHandler allWorkCompleted;

      /// <summary>
      ///   Raises the <see cref="AllWorkCompleted"/> event.
      /// </summary>
      /// <param name="e">
      ///   An <see cref="EventArgs"/> that contains the event data.
      /// </param>
      /// <remarks>
      ///   The <b>OnAllWorkCompleted</b> method allows derived classes to handle the event without attaching a delegate. This
      ///   is the preferred technique for handling the event in a derived class.
      ///   <para>
      ///   When a derived class calls the <b>OnAllWorkCompleted</b> method, it raises the <see cref="AllWorkCompleted"/> event by 
      ///   invoking the event handler through a delegate. For more information, see 
      ///   <a href="ms-help://MS.VSCC.2003/MS.MSDNQTR.2004JAN.1033/cpguide/html/cpconProvidingEventFunctionality.htm">Raising an Event</a>.
      ///   </para>
      /// </remarks>
      protected virtual void OnAllWorkCompleted(EventArgs e)
      {
         EventHandler handler;

         lock (eventLock)
         {
            handler = allWorkCompleted;
         }
         if (handler != null)
         {
            handler(this, e);
         }
      }


      /// <summary>
      ///   Occurs when an <see cref="WorkItem"/> is starting execution.
      /// </summary>
      public event WorkItemEventHandler RunningWorkItem
      {
         add
         {
            lock (eventLock)
            {
               runningWorkItem += value;
            }
         }
         remove
         {
            lock (eventLock)
            {
               runningWorkItem -= value;
            }
         }
      }
      private event WorkItemEventHandler runningWorkItem;

      /// <summary>
      ///   Raises the <see cref="RunningWorkItem"/> event.
      /// </summary>
      /// <param name="workItem">
      ///   The <see cref="WorkItem"/> that has started.
      /// </param>
      /// <remarks>
      ///   The <b>OnRunningWorkItem</b> method allows derived classes to handle the <see cref="RunningWorkItem"/>
      ///   event without attaching a delegate. This is the preferred technique for handling the event in a derived class.
      ///   <para>
      ///   When a derived class calls the <b>OnStartedWorkItem</b> method, it raises the <see cref="RunningWorkItem"/> event by 
      ///   invoking the event handler through a delegate. For more information, see 
      ///   <a href="ms-help://MS.VSCC.2003/MS.MSDNQTR.2004JAN.1033/cpguide/html/cpconProvidingEventFunctionality.htm">Raising an Event</a>.
      ///   </para>
      /// </remarks>
      protected virtual void OnRunningWorkItem(WorkItem workItem)
      {
         WorkItemEventHandler handler;

         lock (eventLock)
         {
            handler = runningWorkItem;
         }
         if (handler != null)
         {
            handler(this, new WorkItemEventArgs(workItem));
         }
      }


      /// <summary>
      ///   Occurs when an <see cref="WorkItem"/> has completed execution.
      /// </summary>
      public event WorkItemEventHandler CompletedWorkItem
      {
         add
         {
            lock (eventLock)
            {
               completedWorkItem += value;
            }
         }
         remove
         {
            lock (eventLock)
            {
               completedWorkItem -= value;
            }
         }
      }
      private event WorkItemEventHandler completedWorkItem;
      /// <summary>
      ///   Raises the <see cref="CompletedWorkItem"/> event.
      /// </summary>
      /// <param name="workItem">
      ///   The <see cref="WorkItem"/> that has completed.
      /// </param>
      /// <remarks>
      ///   The <b>OnCompletedWorkItem</b> method allows derived classes to handle the <see cref="CompletedWorkItem"/>
      ///   event without attaching a delegate. This is the preferred technique for handling the event in a derived class.
      ///   <para>
      ///   When a derived class calls the <b>OnCompletedWorkItem</b> method, it raises the <see cref="CompletedWorkItem"/> event by 
      ///   invoking the event handler through a delegate. For more information, see 
      ///   <a href="ms-help://MS.VSCC.2003/MS.MSDNQTR.2004JAN.1033/cpguide/html/cpconProvidingEventFunctionality.htm">Raising an Event</a>.
      ///   </para>
      /// </remarks>
      protected virtual void OnCompletedWorkItem(WorkItem workItem)
      {
         WorkItemEventHandler handler;

         lock (eventLock)
         {
            handler = completedWorkItem;
         }
         if (handler != null)
         {
            handler(this, new WorkItemEventArgs(workItem));
         }
      }


      /// <summary>
      ///   Occurs when an <see cref="WorkItem"/> has failed execution.
      /// </summary>
      /// <remarks>
      ///   The <see cref="FailedWorkItem"/> event is raised when an <see cref="WorkItem"/>
      ///   throws an <see cref="Exception"/>.  The <see cref="WorkItem.FailedException"/> property
      ///   contains the <b>Exception</b>.
      /// </remarks>
      public event WorkItemEventHandler FailedWorkItem
      {
         add
         {
            lock (eventLock)
            {
               failedWorkItem += value;
            }
         }
         remove
         {
            lock (eventLock)
            {
               failedWorkItem -= value;
            }
         }
      }
      private event WorkItemEventHandler failedWorkItem;
      /// <summary>
      ///   Raises the <see cref="FailedWorkItem"/> event.
      /// </summary>
      /// <param name="workItem">
      ///   The <see cref="WorkItem"/> that failed.
      /// </param>
      /// <remarks>
      ///   The <b>OnFailedWorkItem</b> method allows derived classes to handle the <see cref="FailedWorkItem"/>
      ///   event without attaching a delegate. This is the preferred technique for handling the event in a derived class.
      ///   <para>
      ///   When a derived class calls the <b>OnFailedWorkItem</b> method, it raises the <see cref="FailedWorkItem"/> event by 
      ///   invoking the event handler through a delegate. For more information, see 
      ///   <a href="ms-help://MS.VSCC.2003/MS.MSDNQTR.2004JAN.1033/cpguide/html/cpconProvidingEventFunctionality.htm">Raising an Event</a>.
      ///   </para>
      /// </remarks>
      protected virtual void OnFailedWorkItem(WorkItem workItem)
      {
         WorkItemEventHandler handler;

         lock (eventLock)
         {
            handler = failedWorkItem;
         }
         if (handler != null)
         {
            handler(this, new WorkItemEventArgs(workItem));
         }
      }

      /// <summary>
      ///   Occurs when the <see cref="WorkerPool"/> throws an 
      ///   <see cref="Exception"/> that is not related to the work item.
      /// </summary>
      public event ResourceExceptionEventHandler WorkerException
      {
         add
         {
            lock (eventLock)
            {
               workerException += value;
            }
         }
         remove
         {
            lock (eventLock)
            {
               workerException -= value;
            }
         }
      }
      private event ResourceExceptionEventHandler workerException;
      /// <summary>
      ///   Raises the <see cref="WorkerException"/> event.
      /// </summary>
      /// <param name="e">
      ///   A <see cref="ResourceExceptionEventArgs"/> that contains the event data.
      /// </param>
      /// <remarks>
      ///   The <b>OnWorkerException</b> method allows derived classes to handle the <see cref="WorkerException"/>
      ///   event without attaching a delegate. This is the preferred technique for handling the event in a derived class.
      ///   <para>
      ///   When a derived class calls the <b>OnWorkerException</b> method, it raises the <see cref="WorkerException"/> event by 
      ///   invoking the event handler through a delegate. For more information, see 
      ///   <a href="ms-help://MS.VSCC.2003/MS.MSDNQTR.2004JAN.1033/cpguide/html/cpconProvidingEventFunctionality.htm">Raising an Event</a>.
      ///   </para>
      ///   <para>
      ///   The <b>WorkQueue</b> is <see cref="Pause">paused</see> and in an inconsistent state.
      ///   The <b>WorkQueue</b> should not be used again.
      ///   </para>
      /// </remarks>
      protected virtual void OnWorkerException(ResourceExceptionEventArgs e)
      {
         ResourceExceptionEventHandler handler;

         lock (eventLock)
         {
            handler = workerException;
         }
         if (handler != null)
            handler(this, e);
      }
      #endregion

      #region WorkQueue Members

      /// <summary>
      ///   Invoked by an <see cref="WorkItem"/> to inform a work queue that its <see cref="WorkItem.State"/>
      ///   has changed.
      /// </summary>
      /// <param name="workItem">
      ///   The <see cref="WorkItem"/> that has changed <see cref="WorkItem.State"/>.
      /// </param>
      /// <param name="previousState">
      ///    One of the <see cref="WorkItemState"/> values indicating the previous state of the <paramref name="workItem"/>.
      /// </param>
      /// <remarks>
      ///   The <see cref="ChangedWorkItemState"/> event is raised by calling the
      ///   <see cref="OnChangedWorkItemState"/> method.  Then the following actions are performed, 
      ///   based on the new <see cref="WorkItem.State"/> of <paramref name="workItem"/>:
      ///   <list type="table">
      ///   <listheader>
      ///     <term>State</term>
      ///     <description>Action</description>
      ///   </listheader>
      ///   <item>
      ///     <term><see cref="WorkItemState">Scheduled</see></term>
      ///     <description>Assign the <paramref name="workItem"/> to the <see cref="WorkerPool"/>.</description>
      ///   </item>
      ///   <item>
      ///     <term><see cref="WorkItemState">Running</see></term>
      ///     <description>Raise the <see cref="RunningWorkItem"/> event.</description>
      ///   </item>
      ///   <item>
      ///     <term><see cref="WorkItemState">Failing</see></term>
      ///     <description>Raise the <see cref="FailedWorkItem"/> event.</description>
      ///   </item>
      ///   <item>
      ///     <term><see cref="WorkItemState">Completed</see></term>
      ///     <description>Raise the <see cref="CompletedWorkItem"/> event and schedule the next work item in the queue.</description>
      ///   </item>
      ///   </list>
      /// </remarks>
      public void WorkItemStateChanged(WorkItem workItem, WorkItemState previousState)
      {
         OnChangedWorkItemState(workItem, previousState);

         switch (workItem.State)
         {
            case WorkItemState.Scheduled:
               lock (this)
               {
                  // Housekeeping chores.
                  ++runningItems;

                  // Now start it.
                  WorkerPool.BeginWork(workItem);
               }
               break;

            case WorkItemState.Running:
               OnRunningWorkItem(workItem);
               break;

            case WorkItemState.Failing:
               ++failingItems;
               OnFailedWorkItem(workItem);
               break;

            case WorkItemState.Completed:
               bool allDone = false;
               lock (this)
               {
                  --runningItems;
                  allDone = queue.Count == 0 && runningItems == 0;
               }

               // Tell the world that the workitem has completed.
               OnCompletedWorkItem(workItem);

               // Find some more work.
               if (allDone)
               {
                  // Wakeup.
                  OnAllWorkCompleted(EventArgs.Empty);
                  lock (completed)
                  {
                     Monitor.PulseAll(completed);
                  }
               }
               else
               {
                  DoNextWorkItem();
               }
               break;
         }

      }

      /// <summary>
      ///   Invoked by the <see cref="WorkerPool"/> when an exception is thrown outside of normal
      ///   processing.
      /// </summary>
      public void HandleResourceException(ResourceExceptionEventArgs e)
      {
         lock (completed)
         {
            Pause();
            internalException = e.Exception;

            // Tell the world.
            OnWorkerException(e);

            // Wakeup any threads in WaitAll and let them throw the exception.
            Monitor.PulseAll(completed);
         }

      }

      public void ShutDownWorkerPool()
      {
         // Only shut down the non-default resource pool
         if (resourcePool != null)
         {
            resourcePool.ShutDown();
         }
      }

      public int FailingItems
      {
         get { return failingItems; }
      }

      #endregion
   }
}
