using System;
using System.Collections.Generic;
using System.Text;
using Tools.AssetSystem;
using Tools.WorkQueue;
using System.Threading;
using System.Diagnostics;

namespace AssetTool
{

   
   public class CookingOptions
   {
      /// <summary>
      /// Explicit copy, not using ICloneable because of all the gotchas.
      /// </summary>
      public CookingOptions Copy()
      {
         CookingOptions copy = (CookingOptions) MemberwiseClone();
         return copy;
      }

      /// <summary>
      /// Force a recook if older than this time (we use this to prevent force cook recooking assets multiple times)
      /// </summary>
      public DateTime? mForceRecookTime = null;

      /// <summary>
      /// Skip child assets cooking if true.
      /// </summary>
      public bool mCookOnlyTopLevelAsset = false;

      /// <summary>
      /// Will attempt to find the src asset associated with cooked asset.
      /// </summary>
      /// 
      public bool mInputFileIsCookedAsset = false;

      /// <summary>
      /// If this is false then this instance of the cooker will directly cook the asset instead of trying to multi thread the process
      /// </summary>
      public bool mEnableMultiThreadedCook = true;

      /// <summary>
      /// Target cooking platform.
      /// </summary>
      public PlatformType.PlatformList mPlatforms = new PlatformType.PlatformList();

      public bool mShowGUI = false;
      public int mVerboseOutputLevel = 0;

      public List<string> mCookerArguments = new List<string>();
   };

   public class AnnotatedDependencyCloud
   {
      public AnnotatedDependency Dependant( string assetPath )
      {
         AnnotatedDependency found = (AnnotatedDependency) mDependencies[ assetPath ];

         if ( found == null )
         {
            found = new AnnotatedDependency( this, assetPath );
            mDependencies[assetPath] = found;
         }

         return found;
      }

      internal void AnnotationAdded( AnnotatedDependency dep )
      {
         mHasAnnotations[dep] = true;
      }

      public bool HasAnyAnnotations()
      {
         return mHasAnnotations.Count > 0;
      }

      public void GetAnnotations( StringBuilder b )
      {
         foreach ( AnnotatedDependency dep in mHasAnnotations.Keys )
         {
            dep.BuildAnnotatedStatusTree( b, 0 );
            b.AppendLine();
         }
      }

      public void PrintAnnotations()
      {
         StringBuilder b = new StringBuilder();

         GetAnnotations( b );
         if ( b.Length > 0 )
         {
            Console.Write( b.ToString() );
         }
      }

      System.Collections.Hashtable mDependencies = new System.Collections.Hashtable( 2048 );
      Dictionary<AnnotatedDependency, bool> mHasAnnotations = new Dictionary<AnnotatedDependency, bool>();
   }

   public class AnnotatedDependency
   {
      internal AnnotatedDependency( AnnotatedDependencyCloud cloud, String assetPath )
      {
         mCloud = cloud;
         mAssetPath = assetPath;
      }

      public void AddAnnotation( String annotation )
      {
         if ( mAnnotations == null )
         {
            mAnnotations = new List<String>();
         }

         if ( !mAnnotations.Contains( annotation ) )
         {
            mAnnotations.Add( annotation );
            mCloud.AnnotationAdded( this );
         }
      }

      public void AddParent( AnnotatedDependency parent )
      {
         if ( parent == this )
         {
            return;
         }
         
         mParents[ parent ] = true;
      }

      public void BuildAnnotatedStatusTree( StringBuilder b, int indentLevel )
      {
         if ( indentLevel > 0 )
         {
            b.Append( '*', indentLevel );
            b.Append( ' ' );
         }

         if ( indentLevel > 40 )
         {
            b.AppendLine( "Done walking through parents...  infinite loop?" );
            return;
         }

         b.Append( mAssetPath );
         if ( mAnnotations != null )
         {
            b.AppendFormat( " - {0}", String.Join( ", ", mAnnotations.ToArray() ) );
         }
         b.AppendLine();

         foreach ( AnnotatedDependency parent in mParents.Keys )
         {
            parent.BuildAnnotatedStatusTree( b, indentLevel + 1 );
         }
      }

      readonly AnnotatedDependencyCloud mCloud;
      readonly String mAssetPath;
      List<String> mAnnotations = null;
      Dictionary<AnnotatedDependency, bool> mParents = new Dictionary<AnnotatedDependency, bool>();
   }

   class AssetBuildCookAssetEntry : IComparable<AssetBuildCookAssetEntry>, IEquatable<AssetBuildCookAssetEntry>
   {
      public readonly string mAssetPath;
      public readonly AssetType mAssetType;
      public PlatformType.PlatformList mPlatforms = new PlatformType.PlatformList();

      public AssetBuildCookAssetEntry( string assetPath )
      {
         mAssetPath = assetPath;
         mAssetType = AssetTypes.GetAssetTypeForSourceAsset( assetPath );
      }

      public int CompareTo( AssetBuildCookAssetEntry rhs )
      {
         // Sort by priority, then asset platform, then asset type
         // This will allow for easy batching.
         int result = 0;
         result = rhs.mAssetType.CookPriority - mAssetType.CookPriority;

         if ( result == 0 )
         {
            result = mPlatforms.CompareTo( rhs.mPlatforms );

            if ( result == 0 )
            {

               result = (int) ( mAssetType.TypeId - rhs.mAssetType.TypeId );

               if ( result == 0 )
               {
                  result = mAssetPath.CompareTo( rhs.mAssetPath );
               }
            }

         }

         return result;
      }

      public int GetHashCode( AssetBuildCookAssetEntry _this )
      {
         int result = _this.mAssetPath.GetHashCode() ^ _this.mPlatforms.GetHashCode();

         return result;
      }

      public bool Equals( AssetBuildCookAssetEntry rhs )
      {
         return ( CompareTo( rhs ) == 0 );
      }

   }

   public class AssetCookFailedException : Exception
   {
      public AssetCookFailedException()
      {
      }
      public AssetCookFailedException( string message )
         : base( message )
      {
      }
      public AssetCookFailedException( string message, Exception inner )
         : base( message, inner )
      {
      }
   }

   class AssetBuildCookAssetWorkItem : WorkItem
   {
      private static int sLocalCookCount = 0;
      private static object sConsolePrintLock = new object();
      /// <summary>
      /// Used to buffer up console output to prevent local cooks interfering with child process cooks.
      /// </summary>
      private static string sConsoleOutput = "";

      public List<AssetBuildCookAssetEntry> mAssetEntries;
      public PlatformType.PlatformList mPlatforms;
      public string mStandardOutput;
      public CookingOptions mCookingOptions;
      private AssetBuildSystem mBuildSystem;

      public AssetBuildCookAssetWorkItem( List<AssetBuildCookAssetEntry> assetEntries, PlatformType.PlatformList platforms, CookingOptions cookingOptions, AssetBuildSystem buildSystem )
      {
         mAssetEntries = assetEntries;
         mPlatforms = platforms.Clone();
         mCookingOptions = cookingOptions.Copy();
         mBuildSystem = buildSystem;
      }

      public override void Perform()
      {
         bool bLocalCook = false;
         // Check to see if we've got any local cooks running on this process
         if ( 1 == System.Threading.Interlocked.Increment( ref sLocalCookCount ) )
         {
            // Nope, nothing current cooking on this process.
            // cook the asset locally as it's faster than starting a new process.
            bLocalCook = true;
         }

         bool bResult = true;
         if ( bLocalCook )
         {
            bResult = CookAssetUsingCurrentProcess();
         }
         else
         {
            bResult = CookAssetUsingNewProcess();
         }

         if ( bResult && !mCookingOptions.mCookOnlyTopLevelAsset )
         {
            AnnotatedDependencyCloud cloud = new AnnotatedDependencyCloud();

            foreach ( AssetBuildCookAssetEntry assetEntry in mAssetEntries )
            {
               mBuildSystem.TryQueueAssetDependantsForBuild( assetEntry.mAssetPath, mPlatforms, cloud );
            }

            if ( cloud.HasAnyAnnotations() )
            {
               StringBuilder b = new StringBuilder();
               b.AppendLine( "Assets had errors queuing dependants" );
               cloud.GetAnnotations( b );

               throw new AssetCookFailedException( b.ToString() );
            }


         }

         // Buffer console output to prevent overlapping output from different processes.
         if ( !mCookingOptions.mShowGUI && Monitor.TryEnter( sConsolePrintLock ) )
         {
            lock ( sConsoleOutput )
            {
               Console.Write( sConsoleOutput );
               sConsoleOutput = "";
            }
            Monitor.Exit( sConsolePrintLock );
         }

         if ( !bResult )
         {
            throw new AssetCookFailedException( "Error returned during processing of asset." );
         }
      }

      private bool CookAssetUsingCurrentProcess()
      {
         // Cook this locally for performance.
         // This is faster with common case of just cooking 1 asset as we don't need to spawn another asset tool process.

         bool bResult = true;
         foreach ( AssetBuildCookAssetEntry assetEntry in mAssetEntries )
         {
            lock ( sConsolePrintLock )
            {
               // Prevent overlapping console output.
               bResult = AssetTool.Program.CookAsset( assetEntry.mAssetPath, mPlatforms, mCookingOptions.mVerboseOutputLevel );
               if ( !bResult )
               {
                  // Cooked failed, show parent assets that referenced this asset.
                  // This makes it easier to track down which file is using this asset if it doesn't exist.
/*
                  Console.Write( "---\n" );
                  Console.Write( "Source Asset:\n {0}\n", assetEntry.mAssetPath );
                  Console.Write( "Asset Parents:\n" );
                  foreach ( PackageAsset parentAsset in assetEntry.mParentAssets )
                  {
                     Console.Write( " {0}\n", parentAsset.mAssetPath );
                  }
                  Console.Write( "---\n" );
 */
                  break;
               }
            }
         }

         System.Threading.Interlocked.Decrement( ref sLocalCookCount );

         return bResult;
      }

      private bool CookAssetUsingNewProcess()
      {
         using ( Process proc = new Process() )
         {
            proc.StartInfo.FileName = System.Reflection.Assembly.GetExecutingAssembly().Location;

            // Specify platform if valid
            string platformArgs = "";
            foreach ( PlatformType.EPlatform platform in mPlatforms )
            {
               platformArgs += String.Format( "/p{0} ", PlatformType.GetPlatformTypeForEnum( platform ).TypeString );
            }

            // Specify verbose output
            string verboseOutputArgs = ( mCookingOptions.mVerboseOutputLevel > 0 ) ? "/v " : "";

            string cookerArgs = String.Empty;

            foreach (string cookerArg in mCookingOptions.mCookerArguments)
            {
               cookerArgs += String.Format("/a{0} ", cookerArg);
            }

            // Build list of assets
            string assetPaths = String.Empty;
            foreach ( AssetBuildCookAssetEntry assetEntry in mAssetEntries )
            {
               assetPaths += String.Format( "\"{0}\" ", assetEntry.mAssetPath );
            }

            proc.StartInfo.Arguments = string.Format( "/t /d /f {0}{1}{2}{3}", verboseOutputArgs, platformArgs, cookerArgs, assetPaths );
            proc.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;
            proc.StartInfo.UseShellExecute = false;
            proc.StartInfo.RedirectStandardOutput = true;
            proc.StartInfo.RedirectStandardError = true;

            //System.Console.WriteLine("Performing: {0}", proc.StartInfo.Arguments);

            if ( proc.Start() )
            {
               mBuildSystem.mJobObject.AssignProcess( proc );
               proc.PriorityClass = ProcessPriorityClass.Idle;

               string standardOutput = proc.StandardOutput.ReadToEnd();
               string standardError = proc.StandardError.ReadToEnd();

               proc.WaitForExit();

               mStandardOutput = standardOutput;

               if ( proc.ExitCode != 0 )
               {
                  if ( standardError.Length != 0 )
                  {
                     mStandardOutput += "Error Output: " + standardError;
                  }
               }

               // Buffer to prevent overlapping console output.
               lock ( sConsoleOutput )
               {
                  sConsoleOutput += mStandardOutput;
               }

               bool bResult = ( proc.ExitCode == 0 );
               return bResult;
            }
            else
            {
               return false;
            }
         }
      }
   }

   public class AssetBuildSystem : IDisposable
   {
      public AssetBuildSystem( DateTime? forceRecookTime, int numProcessorsToUse )
      {
         mCookingWorkQueue = new WorkQueue();
         mCookingWorkQueue.WorkerPool = new WorkThreadPool( 1, numProcessorsToUse );
         mCookingWorkQueue.ConcurrentLimit = numProcessorsToUse;
         mCookingWorkQueue.FailedWorkItem += new WorkItemEventHandler( OnWorkItemFailed );

         ForceRecookTime = forceRecookTime;
      }

      public void Dispose()
      {
         StatusWindow = null;

         if ( mCookingWorkQueue != null )
         {
            mCookingWorkQueue.ShutDownWorkerPool();
            mCookingWorkQueue = null;
         }
      }

      public CookStatusWindow StatusWindow
      {
         get { return mStatusWindow; }
         set
         {
            if ( mStatusWindow != null )
            {
               mStatusWindow.mWorkQueue = null; 
               mCookingWorkQueue.ChangedWorkItemState -= mStatusWindow.OnChangedWorkItemState;
            }
            
            mStatusWindow = value;

            if ( mStatusWindow != null )
            {
               mStatusWindow.mWorkQueue = mCookingWorkQueue;
               mCookingWorkQueue.ChangedWorkItemState += mStatusWindow.OnChangedWorkItemState;
            }
         }
      }

      public DateTime? ForceRecookTime;

      public void TryQueueAssetDependantsForBuild( string assetPath, PlatformType.PlatformList platforms, AnnotatedDependencyCloud errorReporting )
      {
         CheckAndQueueAssetDependants( assetPath, AssetTypes.GetAssetTypeForSourceAsset( assetPath ), platforms, errorReporting );
      }

      public void TryQueueAssetForBuild( string assetPath, PlatformType.PlatformList platforms, bool queueDependents, AnnotatedDependencyCloud errorReporting )
      {
         CheckAndQueueAsset( assetPath, AssetTypes.GetAssetTypeForSourceAsset( assetPath ), platforms, queueDependents, errorReporting );
      }

      public bool Build( CookingOptions options )
      {
         while ( !mCookingWorkQueue.Pausing && mPendingWorkEntries.Count > 0 )
         {
            Queue<AssetBuildCookAssetEntry> workBatch = SortPendingWorkAndGenerateWorkBatch();

            AddWorkBatchToWorkQueue( workBatch, options );

            try
            {
               while ( !mCookingWorkQueue.WaitAll( TimeSpan.FromMilliseconds( 100 ) ) )
               {
                  // Give the UI some time to handle messages
                  if ( mStatusWindow != null )
                  {
                     System.Windows.Forms.Application.DoEvents();

                     // did the user close the status window?
                     if ( mStatusWindow.WasAborted )
                     {
                        throw new AssetCookFailedException( "Cooking of assets aborted by user" );
                     }
                  }
               }
            }
            catch ( AssetCookFailedException e )
            {
               // Cook failed.
               string errorMessage = String.Empty;
               // Only display full exception stack if the inner exception is a crash.
               if ( e.InnerException != null && ( ( e.InnerException as AssetCookFailedException ) == null ) )
               {
                  // Unknown exception!
                  errorMessage = e.InnerException.ToString();
               }
               else
               {
                  // We deliberately threw this, just show message.
                  errorMessage = e.Message;
               }

               // Disable any pending jobs
               mCookingWorkQueue.Clear();

               System.Console.WriteLine( "\n{0}", errorMessage );
               if ( mStatusWindow != null )
               {
                  // We're cooking in a window                  
                  // Display cook error in tool and wait for user to close window.
                  mStatusWindow.TurnIntoErrorDisplayMode();
                  mStatusWindow.AddTextToDetailsOutput( "\n" + errorMessage );
                  mStatusWindow.WaitForFormClose();
               }

               return false;
            }
         }

         return true;
      }

      private bool CheckAndQueueAssetDependants( string assetPath, AssetType assetType, PlatformType.PlatformList platforms, AnnotatedDependencyCloud errorReporting )
      {
         bool succeeded = true;

         if ( !assetType.CooksToPlatformSpecificDirectory )
         {
            List<PackageAsset> packageAssets = PackageAssets.GetAssetsAllPlatforms( assetPath, mAssetManager );

            foreach ( PackageAsset packageAsset in packageAssets )
            {
               if ( !CheckAndQueueAsset( packageAsset.mAssetPath, AssetTypes.GetAssetTypeForSourceAsset( packageAsset.mAssetPath ), platforms, true, errorReporting ) )
               {
                  AnnotatedDependency myDependency = errorReporting.Dependant( assetPath );
                  AnnotatedDependency childDependency = errorReporting.Dependant( packageAsset.mAssetPath );

                  childDependency.AddParent( myDependency );
                  succeeded = false;
               }
            }
         }
         else
         {
            foreach ( PlatformType.EPlatform platform in platforms )
            {
               List<PackageAsset> packageAssets = PackageAssets.GetAssets( assetPath, mAssetManager, platform );

               foreach ( PackageAsset packageAsset in packageAssets )
               {
                  if ( !CheckAndQueueAsset( packageAsset.mAssetPath, AssetTypes.GetAssetTypeForSourceAsset( packageAsset.mAssetPath ), PlatformType.PlatformList.FromPlatform( platform ), true, errorReporting ) )
                  {
                     AnnotatedDependency myDependency = errorReporting.Dependant( assetPath );
                     AnnotatedDependency childDependency = errorReporting.Dependant( packageAsset.mAssetPath );

                     childDependency.AddParent( myDependency );
                     succeeded = false;
                  }
               }
            }
         }

         return succeeded;
      }

      private bool CheckAndQueueAsset( string assetPath, AssetType assetType, PlatformType.PlatformList inPlatforms, bool queueDependents, AnnotatedDependencyCloud errorReporting)
      {
         PlatformType.PlatformList needsCookingList = Tools.AssetSystem.Helper.GetPlatformsNeedingCooking( assetPath, mAssetManager, ForceRecookTime, inPlatforms );

         PlatformType.PlatformList doesntNeedCookingList = inPlatforms.Clone();
         doesntNeedCookingList.RemoveAllOf( needsCookingList );

         bool succeeded = true;

         if ( !needsCookingList.Empty )
         {
            AnnotatedDependency dep = AddAssetAsPendingWorkEntry( assetPath, assetType, needsCookingList, errorReporting );
            if ( dep != null )
            {
               AnnotatedDependency myDependency = errorReporting.Dependant( assetPath );

               dep.AddParent( myDependency );
               succeeded = false;
            }
         }

         if ( queueDependents && !doesntNeedCookingList.Empty )
         {
            if ( !CheckAndQueueAssetDependants( assetPath, assetType, doesntNeedCookingList, errorReporting ) )
            {
               succeeded = false;
            }
         }

         return succeeded;
      }

      private static void OnWorkItemFailed( object sender, WorkItemEventArgs e )
      {
         AssetBuildCookAssetWorkItem workItem = e.WorkItem as AssetBuildCookAssetWorkItem;

         List<string> assetNames = new List<string>();
         foreach ( AssetBuildCookAssetEntry entry in workItem.mAssetEntries )
         {
            assetNames.Add( entry.mAssetPath );
         }

         String allAssetsString = "\"" + String.Join( "\", \"", assetNames.ToArray() ) + "\"";

         if ( workItem.FailedException != null )
         {
            throw new AssetCookFailedException( String.Format( "Error during processing of one of {0}.\n", allAssetsString ), workItem.FailedException );
         }
         else
         {
            throw new AssetCookFailedException( String.Format( "Unknown error during processing of one of {0}.\n", allAssetsString ) );
         }
      }

      private AnnotatedDependency AddAssetAsPendingWorkEntry( string assetPath, AssetType type, PlatformType.PlatformList platforms, AnnotatedDependencyCloud dependencyCloud )
      {
         if ( !System.IO.File.Exists( mAssetManager.GetSystemPathWithBackslashes( assetPath ) ) )
         {
            AnnotatedDependency dep = dependencyCloud.Dependant( assetPath );

            dep.AddAnnotation( "File not found" );
            return dep;
         }

         lock ( mPendingWorkEntries )
         {
            if ( type.ShouldDistributePlatformCooks )
            {
               // If we should distribute platform cooks, then we need to create discrete jobs 
               // for discrete platforms.  First things first is to see if we already have these 
               // platforms in any work entries.

               PlatformType.PlatformList remainingToCook = platforms.Clone();

               foreach ( AssetBuildCookAssetEntry entry in mPendingWorkEntries )
               {
                  if ( entry.mAssetPath == assetPath )
                  {
                     remainingToCook.RemoveAllOf( entry.mPlatforms );
                  }
               }

               // Ok, now remainingToCook contains the platforms that were not already in asset build 
               // cook entries.  Run through and add new entries for them

               foreach ( PlatformType.EPlatform platform in remainingToCook )
               {
                  AssetBuildCookAssetEntry newEntry = new AssetBuildCookAssetEntry( assetPath );
                  newEntry.mPlatforms = PlatformType.PlatformList.FromPlatform( platform );
                  mPendingWorkEntries.Add( newEntry );
               }
            }
            else
            {
               // First, if we already have a work entry, just combine platforms with it
               foreach ( AssetBuildCookAssetEntry entry in mPendingWorkEntries )
               {
                  if ( entry.mAssetPath == assetPath )
                  {
                     entry.mPlatforms.CombineWith( platforms );
                     return null;
                  }
               }

               // Only makes it here if we didn't combine platforms with another entry
               AssetBuildCookAssetEntry newEntry = new AssetBuildCookAssetEntry( assetPath );
               newEntry.mPlatforms = platforms.Clone();
               mPendingWorkEntries.Add( newEntry );
            }

         }

         return null;
      }

      private Queue<AssetBuildCookAssetEntry> SortPendingWorkAndGenerateWorkBatch()
      {
         Queue<AssetBuildCookAssetEntry> workBatch = new Queue<AssetBuildCookAssetEntry>();

         // A work batch can only contain items of the same cook priority
         lock ( mPendingWorkEntries )
         {
            mPendingWorkEntries.Sort();

            AssetType cookingAssetType = mPendingWorkEntries[0].mAssetType;
            int endBatch = 0;
            for ( endBatch = 0; endBatch < mPendingWorkEntries.Count && mPendingWorkEntries[ endBatch ].mAssetType.CookPriority == cookingAssetType.CookPriority; ++endBatch )
            {
               workBatch.Enqueue( mPendingWorkEntries[ endBatch ] );
            }

            mPendingWorkEntries.RemoveRange( 0, endBatch );
         }

         return workBatch;
      }

      private void AddWorkBatchToWorkQueue( Queue<AssetBuildCookAssetEntry> workBatch, CookingOptions options )
      {
         int maxBatchSize = ( workBatch.Count + ( mCookingWorkQueue.ConcurrentLimit - 1 ) ) / mCookingWorkQueue.ConcurrentLimit;
         List<AssetBuildCookAssetEntry> currentWorkQueueAssets = new List<AssetBuildCookAssetEntry>();

         while ( workBatch.Count > 0 )
         {
            AssetBuildCookAssetEntry entry = workBatch.Dequeue();

            if ( currentWorkQueueAssets.Count > 0 )
            {
               AssetBuildCookAssetEntry head = currentWorkQueueAssets[0];

               // Clamp batch size.
               int batchSize = Math.Min(maxBatchSize, head.mAssetType.CookBatchSize);

               // If we're adding something that doesn't belong in our batch, we need to flush 
               // first
               if ( currentWorkQueueAssets.Count >= batchSize || head.mAssetType != entry.mAssetType || head.mPlatforms != entry.mPlatforms )
               {
                  mCookingWorkQueue.Add( new AssetBuildCookAssetWorkItem( currentWorkQueueAssets, head.mPlatforms, options, this ) );
                  currentWorkQueueAssets = new List<AssetBuildCookAssetEntry>();
               }

            }

            currentWorkQueueAssets.Add( entry );
         }

         if ( currentWorkQueueAssets.Count > 0 )
         {
            mCookingWorkQueue.Add( new AssetBuildCookAssetWorkItem( currentWorkQueueAssets, currentWorkQueueAssets[0].mPlatforms, options, this ) );
         }
      }

      public readonly Tools.Common.JobObject mJobObject = Tools.Common.JobObject.NewObjectThatKillsProcessesOnClose();
      WorkQueue mCookingWorkQueue;
      List<AssetBuildCookAssetEntry> mPendingWorkEntries = new List<AssetBuildCookAssetEntry>();
      List<AssetBuildCookAssetWorkItem> mCompletedWorkItems = new List<AssetBuildCookAssetWorkItem>();
      CookStatusWindow mStatusWindow;
      Manager mAssetManager = new Manager();
   }

}
