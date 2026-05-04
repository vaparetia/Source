using System;
using System.Collections.Generic;
using System.Text;

namespace Tools.WorkQueue
{
   public enum WorkItemState
   {
      Created,
      Queued,
      Scheduled,
      Running,
      Failing,
      Completed
   }

   public abstract class WorkItem
   {
      private DateTime createdTime;
      private DateTime startedTime;
      private DateTime completedTime;
      private Exception failedException;
      private WorkItemState state;
      private WorkQueue workQueue;

      #region Constructors
      /// <summary>
      ///   Creates a new instance of the <see cref="WorkItem"/> class.
      /// </summary>
      protected WorkItem()
      {
         createdTime = DateTime.Now;
         state = WorkItemState.Created;
      }
      #endregion

      #region Properties

      /// <summary>
      ///   Gets or sets the <see cref="WorkQueue"/> containing this <see cref="WorkItem"/>.
      /// </summary>
      /// <value>
      ///   The <see cref="WorkQueue"/> that is scheduling this <see cref="WorkItem"/>.
      /// </value>
      public WorkQueue WorkQueue
      {
         get
         {
            return workQueue;
         }
         set
         {
            if (workQueue != value)
            {
               if (workQueue != null)
                  throw new NotSupportedException(String.Format("'{0}' is assigned to another WorkQueue '{1}'.", this, workQueue));

               workQueue = value;
            }
         }
      }

      /// <summary>
      ///   Gets or sets the <see cref="WorkItemState">state</see>.
      /// </summary>
      /// <value>
      ///   One of the <see cref="WorkItemState"/> values indicating the state of the current <b>WorkItem</b>. 
      ///   The initial value is <b>Created</b>.
      /// </value>
      /// <exception cref="InvalidTransitionException">
      ///   The <b>State</b> can not be transitioned to <paramref name="value"/>.
      /// </exception>
      /// <remarks>
      ///   The <b>State</b> represents where the <see cref="WorkItem"/> is in processing pipeline.
      ///   The following transition can take place:
      ///   <para>
      ///   <img src="WorkItemState.png" alt="WorkItem state transistions"/>
      ///   </para>
      ///   <para>
      ///   If the <see cref="WorkQueue"/> is not <b>null</b>, then its 
      ///   <see cref="WorkQueue.WorkItemStateChanged"/> method is called.
      ///   </para>
      /// </remarks>
      /// <seealso cref="ValidateStateTransition"/>
      public WorkItemState State
      {
         get { return state; }
         set
         {
            WorkItemState prev = state;
            state = value;
            switch (state)
            {
               case WorkItemState.Running:
                  StartedTime = DateTime.Now;
                  break;

               case WorkItemState.Completed:
                  CompletedTime = DateTime.Now;
                  break;
            }

            if (WorkQueue != null)
               WorkQueue.WorkItemStateChanged(this, prev);
         }
      }

      /// <summary>
      ///   Gets or sets the <see cref="Exception"/> that caused the <see cref="WorkItem"/> to
      ///   fail.
      /// </summary>
      public Exception FailedException
      {
         get { return failedException; }
         set { failedException = value; }
      }
      #endregion

      #region Processing
      /// <summary>
      ///   Perform the work.
      /// </summary>
      /// <remarks>
      ///   <b>Perform</b> performs the work. 
      ///   <para>
      ///   A thrown <see cref="Exception"/> is caught by the <see cref="IResourcePool"/> and the
      ///   workitem's
      ///   <see cref="FailedException"/> property is set and its <see cref="State"/> changed
      ///   to <see cref="WorkItemState">Failing</see>.
      ///   </para>
      ///   <para>
      ///   This is an <b>abstract</b> method and must be implemented by derived classes.
      ///   </para>
      /// </remarks>
      public abstract void Perform();

      #endregion

      #region Times
      /// <summary>
      ///   Gets or sets the time when processing <see cref="Perform">started</see>.
      /// </summary>
      /// <value>
      ///   A <see cref="DateTime"/> indicating when the <b>WorkItem</b> started.
      /// </value>
      /// <remarks>
      ///   The <b>StartedTime</b> is set when the <see cref="WorkItem"/> enters the
      ///   <see cref="WorkItemState">Running</see> state.
      /// </remarks>
      public DateTime StartedTime
      {
         get { return startedTime; }
         set { startedTime = value; }
      }

      /// <summary>
      ///   Gets or sets the time when processing completed.
      /// </summary>
      /// <value>
      ///   A <see cref="DateTime"/> indicating when the <b>WorkItem</b> finished.
      /// </value>
      /// <remarks>
      ///   The <b>CompletedTime</b> is set when the <see cref="WorkItem"/> enters the
      ///   <see cref="WorkItemState">Completed</see> state.
      /// </remarks>
      public DateTime CompletedTime
      {
         get { return completedTime; }
         set
         {
            completedTime = value;
         }
      }

      /// <summary>
      ///   Gets or sets the time when the instance was created.
      /// </summary>
      /// <value>
      ///   A <see cref="DateTime"/> indicating when the <b>WorkItem</b> was created.
      /// </value>
      /// <remarks>
      ///   The <b>CreatedTime</b> is set when the <see cref="WorkItem"/> is constructed.
      /// </remarks>
      public DateTime CreatedTime
      {
         get { return createdTime; }
         set
         {
            createdTime = value;
         }
      }

      /// <summary>
      ///   Gets the elapsed processing time.
      /// </summary>
      /// <value>
      ///   A <see cref="TimeSpan"/> indicating the amount of time spent processing.
      /// </value>
      /// <remarks>
      ///   <b>ProcessingTime</b> is the difference between the <see cref="CompletedTime"/> and 
      ///   <see cref="StartedTime"/>.
      /// </remarks>
      public TimeSpan ProcessingTime
      {
         get { return CompletedTime - StartedTime; }
      }
      #endregion
   }
}
