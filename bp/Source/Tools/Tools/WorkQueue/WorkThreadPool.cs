using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;

namespace Tools.WorkQueue
{
    public class WorkThreadPool : IResourcePool, IDisposable
   {
      private int minThreads;
      private int maxThreads;
      private List<WorkThread> workers;
      private int waiters;

      internal Queue<WorkItem> workQueue;

      private readonly object eventLock = new object();

      private static WorkThreadPool defaultThreadPool = null;

      public static WorkThreadPool Default
      {
         get
         {
            if (defaultThreadPool == null)
            {
               lock (typeof(WorkThreadPool))
               {
                  if (defaultThreadPool == null)
                     defaultThreadPool = new WorkThreadPool();
               }
            }
            return defaultThreadPool;
         }
      }


      #region Constructors

      /// <summary>
      ///   Creates a new instance of the <see cref="WorkThreadPool"/> class.
      /// </summary>
      /// <seealso cref="Default"/>
      public WorkThreadPool()
         : this(1, 25)
      {
      }

      /// <summary>
      ///   Creates a new instance of the <see cref="WorkThreadPool"/> class with the
      ///   specified <see cref="MinThreads"/> and <see cref="MaxThreads"/>.
      /// </summary>
      /// <param name="minThreads">
      ///   The mininum number of threads.
      /// </param>
      /// <param name="maxThreads">
      ///   The maximum number of threads.
      /// </param>
      /// <seealso cref="Default"/>
      public WorkThreadPool(int minThreads, int maxThreads)
      {
         if (0 >= maxThreads)
            throw new ArgumentOutOfRangeException("maxThreads", maxThreads, "Must be greater than zero.");

         workQueue = new Queue<WorkItem>();
         workers = new List<WorkThread>(maxThreads);

         this.maxThreads = maxThreads;
         MinThreads = minThreads;
      }

      #endregion

      #region IDisposable Members

      /// <summary>
      ///   Terminates the threads of the current WorkThreadPool before it is reclaimed by the garbage collector.
      /// </summary>
      ~WorkThreadPool()
      {
         Dispose(false);
      }


      /// <summary>
      ///   Performs an orderly shutdown of the pooled <b>threads</b>.
      /// </summary>
      /// <exception cref="InvalidOperationException">
      ///   <c>this</c> equals <see cref="Default"/>.
      /// </exception>
      /// <remarks>
      ///   <b>Dispose</b> performs an orderly shutdown of the <b>threads</b>.  The method
      ///   waits for each working thread to complete, before terminating the thread.
      /// </remarks>
      public void Dispose()
      {
         if (this == defaultThreadPool)
            throw new InvalidOperationException("The Default WorkThreadPool can not be disposed.");

         Dispose(true);
         GC.SuppressFinalize(this);
      }

      private void Dispose(bool disposing)
      {
         ShutDown();
      }
      #endregion

      #region Properties
      /// <summary>
      ///   Gets or sets the number of idle threads the ThreadPool maintains in anticipation of new requests.
      /// </summary>
      /// <value>
      ///   The minimum number of worker threads in the thread pool.
      /// </value>
      /// <exception cref="ArgumentOutOfRangeException">
      ///   When setting and <i>value</i> is less than zero or greater than <see cref="MaxThreads"/>.
      /// </exception>
      /// <remarks>
      ///   <b>MinThreads</b> is the minimum number of idle threads maintained by the thread pool in order to reduce
      ///   the time required to satisfy requests for thread pool threads. Idle threads in excess of the 
      ///   minimum can be terminated, to save system resources. 
      /// </remarks>
      public int MinThreads
      {
         get
         {
            return minThreads;
         }
         set
         {
            if (value < 0)
               throw new ArgumentOutOfRangeException("MinThreads", value, "Must be positive or zero.");
            if (value > MaxThreads)
               throw new ArgumentOutOfRangeException("MinThreads", value, "Must be less than MaxThreads.");

            minThreads = value;
            lock (this)
            {
               while (workers.Count < minThreads)
                  CreateThread();
            }
         }
      }

      /// <summary>
      ///   Gets or sets the number of requests to the thread pool that can be active concurrently.
      /// </summary>
      /// <value>
      ///   The maximum number of worker threads in the thread pool.
      /// </value>
      /// <exception cref="ArgumentOutOfRangeException">
      ///   When setting and <i>value</i> is less than or equal to zero.
      /// </exception>
      /// <remarks>
      ///   <b>MaxThreads</b> is the number of <see cref="BeginWork">requests</see> to the thread pool that can
      ///   be active concurrently.  All requests above the number remain queued until a thread pool thread
      ///   become available.
      ///   <para>
      ///   When setting and <i>value</i> is less than the current <b>MaxThreads</b>, the appropiate number
      ///   of threads will be deleted.
      ///   </para>
      /// </remarks>
      public int MaxThreads
      {
         get
         {
            return maxThreads;
         }
         set
         {
            if (0 >= value)
               throw new ArgumentOutOfRangeException("MinThreads", value, "Must be greater than zero.");

            maxThreads = value;
            lock (this)
            {
               while (workers.Count > maxThreads)
                  DeleteThread();
            }
            if (MinThreads > maxThreads)
               MinThreads = maxThreads;
         }
      }
      #endregion

      private void CreateThread()
      {
         WorkThread worker = new WorkThread(this);
         workers.Add(worker);

         Thread thread = new Thread(new ThreadStart(worker.Start));
         thread.Name = "WT #" + workers.Count;
         thread.IsBackground = true;

         worker.Thread = thread;
         thread.Start();
      }

      private Thread DeleteThread()
      {
         int i = workers.Count - 1;
         WorkThread worker = (WorkThread)workers[i];

         worker.Stop();
         workers.RemoveAt(i);

         return worker.Thread;
      }

      /// <summary>
      ///   Requests that an <see cref="WorkItem">work item</see> is run on a <see cref="Thread"/>.
      /// </summary>
      public void BeginWork(WorkItem workItem)
      {
         if (workItem == null)
            throw new ArgumentNullException();

         lock (this)
         {
            // Queue the work.
            workQueue.Enqueue(workItem);

            // If all workers are busy, then create a thread if the limit
            // is not reached.
            if (waiters == 0 && workers.Count < MaxThreads)
               CreateThread();

            // Wakeup a worker.
            Monitor.Pulse(this);
         }
      }
       public void ShutDown()
       {
          if (workers == null)
             return;

          // Tell all workers to shutdown.
          lock (this)
          {
             foreach (WorkThread worker in workers)
             {
                worker.Stop();
             }
             Monitor.PulseAll(this);
          }

          // Wait for the worker threads to quit.
          while (workers.Count > 0)
          {
             Thread thread = null;
             lock (this)
             {
                if (workers.Count > 0)
                {
                   thread = DeleteThread();
                }
                Monitor.PulseAll(this);
             }
             if (thread != null)
             {
                thread.Join();
             }
          }
          workers = null;
       }

       private class WorkThread
       {
          private WorkThreadPool mThreadPool;
          private volatile bool mStopping;
          public Thread Thread;

          public WorkThread(WorkThreadPool threadPool)
          {
             mThreadPool = threadPool;
          }

          public void Start()
          {
          restart:
             try
             {
                while (!mStopping)
                {
                   WorkItem work = GetWork();

                   // Perform the work.
                   if (work != null)
                      DoWork(work);

                   // Yield to other threads, including the User Interface (if any).
                   if (!mStopping)
                   {
                      Thread.Sleep(0);
                   }
                }
             }
             catch (ThreadAbortException)
             {
                // Abort nicely.
                Stop();
                Thread.ResetAbort();
             }
             catch (Exception /*e*/)
             {
                //mThreadPool.OnThreadException(new ResourceExceptionEventArgs(this, e));
                goto restart;
             }
          }

          public WorkItem GetWork()
          {
             // Get some work
             lock (mThreadPool)
             {
                if (mStopping)
                   return null;

                if (mThreadPool.workQueue.Count == 0)
                {
                   ++mThreadPool.waiters;
                   Monitor.Wait(mThreadPool);
                   --mThreadPool.waiters;
                }

                if (mStopping)
                   return null;
                if (mThreadPool.workQueue.Count > 0)
                   return mThreadPool.workQueue.Dequeue();
             }

             return null;
          }

          public void DoWork(WorkItem workItem)
          {
             ThreadPriority originalPriority = Thread.CurrentThread.Priority;

             try
             {
                workItem.State = WorkItemState.Running;
              
                try
                {
                   workItem.Perform();
                   workItem.State = WorkItemState.Completed;
                }
                catch (Exception e)
                {
                   workItem.FailedException = e;
                   workItem.State = WorkItemState.Failing;
                }
             }
             catch (Exception e)
             {
                // If no work queue for the item, then let the WorkThreadPool raise
                // the exception event.
                if (workItem == null || workItem.WorkQueue == null)
                   throw;

                workItem.WorkQueue.HandleResourceException(new ResourceExceptionEventArgs(this, workItem, e));
             }
             finally
             {
                if (Thread.CurrentThread.Priority != originalPriority)
                   Thread.CurrentThread.Priority = originalPriority;
             }
          }

          public void Stop()
          {
             mStopping = true;
          }
       }
   }
}
