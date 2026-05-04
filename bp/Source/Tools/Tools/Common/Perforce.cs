using System;
using System.Collections.Generic;
using System.Text;
using P4API;

namespace Tools.Common.Perforce
{
   public class FileInfo
   {
      public enum EAction
      {
         Unknown,
         None,
         Add,
         Edit,
         Delete,
         Branch,
         Integrate
      }

      Connection  mConnection;
      P4Record    mFstat;

      public FileInfo(Connection connection, string depotPath)
      {
         mConnection = connection;

         if( !string.IsNullOrEmpty(depotPath) )
         {
            try
            {
               P4RecordSet recordSet = connection.mP4Api.Run("fstat", depotPath);
               if (recordSet.Records.Length > 0)
               {
                  mFstat = recordSet[0];
               }
            }
            catch (P4API.Exceptions.PerforceInitializationError e)
            {
               connection.HandlePerforceInitializationError(e);
            }
         }
      }

      internal FileInfo(Connection connection, P4Record fstatRecord)
      {
         mConnection = connection;
         mFstat = fstatRecord;
      }

      /// <summary>
      /// Depot path of file, i.e. //depot/readme.txt
      /// </summary>
      public string DepotPath
      {
         get { return mFstat["depotFile"]; }
      }

      /// <summary>
      /// Local path of file, i.e. C:\depot\readme.txt
      /// </summary>
      public string LocalPath
      {
         get { return mFstat["clientFile"]; }
      }

      public bool ExistsInDepot
      {
         get { return mFstat != null && mFstat["headRev"] != null; }
      }
      
      /// <summary>
      /// Determines if the path is mapped in the client spec.
      /// </summary>
      public bool IsMapped
      {
         get { return mFstat["isMapped"] != null; }
      }

      /// <summary>
      /// Return current action of the file (by THIS user)
      /// </summary>
      public EAction Action
      {
         get
         {
            if(mFstat != null )
            {
               string actionStr = mFstat["action"];

               switch (actionStr)
               {
                  case "add":
                     return EAction.Add;

                  case "edit":
                     return EAction.Edit;

                  case "delete":
                     return EAction.Delete;

                  case "branch":
                     return EAction.Branch;

                  case "integrate":
                     return EAction.Integrate;

                  default:
                     return EAction.None;
               }
            }
            else
            {
               return EAction.Unknown;
            }
         }
      }

      /// <summary>
      /// Determines if the latest revision on the server is deleted.
      /// </summary>
      public bool HeadDeleted
      {
         get { return mFstat["headAction"] == "delete"; }
      }

      /// <summary>
      /// Is this file checked out to me?
      /// </summary>
      public bool CheckedOutByMe
      {
         get { return Action != EAction.None && Action != EAction.Unknown; }
      }

      /// <summary>
      /// Is this file checked out to someone else but me?
      /// </summary>
      public bool CheckedOutBySomeoneElse
      {
         get { return mFstat != null && mFstat["otherOpen"] != null; }
      }

      /// <summary>
      /// Is this file checked out by anybody (including myself)
      /// </summary>
      public bool CheckedOutByAnyone
      {
         get { return CheckedOutByMe || CheckedOutBySomeoneElse; }
      }

      /// <summary>
      /// Revision the client is synced at
      /// </summary>
      public int LocalRevision
      {
         get { return int.Parse(mFstat["haveRev"]); }
      }

      /// <summary>
      /// Latest revision in server
      /// </summary>
      public int HeadRevision
      {
         get { return int.Parse(mFstat["headRev"]); }
      }

      /// <summary>
      /// Time of submit of top revision
      /// </summary>
      public DateTime HeadTime
      {
         get { return mConnection.mP4Api.ConvertDate( int.Parse(mFstat["headTime"])); }
      }

      /// <summary>
      /// Last modified time of the head revision file (the last time the file was written to before it was submitted to the server).
      /// </summary>
      public DateTime HeadModTime
      {
         get { return mConnection.mP4Api.ConvertDate(int.Parse(mFstat["headModTime"])); }
      }

      /// <summary>
      /// Changelist number the file is currently in, returns 0 for "default" and -1 if the file is in no change list.
      /// </summary>
      public int ChangeList
      {
         get
         {
            string changeStr = mFstat["change"];
            if( changeStr != null )
            {
               if (changeStr == "default")
               {
                  return 0;
               }
               else
               {
                  return int.Parse(changeStr);
               }
            }
            
            return -1;
         }
      }

      public void GetRevisionInfo(int revision, out string user, out DateTime time)
      {
         user = string.Empty;
         time = new DateTime();

         try
         {
            P4RecordSet recordSet = mConnection.mP4Api.Run("filelog", "-m", "1", string.Format("{0}#{1}", DepotPath, revision));
            if (recordSet.Records.Length > 0)
            {
               user = recordSet[0].ArrayFields["user"][0];
               time = mConnection.mP4Api.ConvertDate(int.Parse(recordSet[0].ArrayFields["time"][0]));
            }
         }
         catch (P4API.Exceptions.PerforceInitializationError e)
         {
            mConnection.HandlePerforceInitializationError(e);
         }
      }
   
      public string GetDataAsText()
      {
         return mConnection.mP4Api.PrintText(DepotPath);
      }

      public byte[] GetDataAsBinary()
      {
         return mConnection.mP4Api.PrintBinary(DepotPath);
      }
   }

   public class Changelist
   {
      public enum EStatus
      {
         Unknown,
         Submitted,
         Pending
      }

      Connection mConnection;

      public Changelist(Connection connection, string description)
      {
         mConnection = connection;
         
         P4Form form = mConnection.mP4Api.Fetch_Form("change");
         
         form["Description"] = description;

         form.ArrayFields["Jobs"] = new string[0];
         form.ArrayFields["Files"] = new string[0];

         try
         {
            P4UnParsedRecordSet res = mConnection.mP4Api.Save_Form(form);
            mChangelistNumber = int.Parse(res.Messages[0].Split(' ')[1]);
         }
         catch (System.Exception e)
         {
            string message = e.Message;
         }
      }

      public Changelist(Connection connection, int number)
      {
         mConnection = connection;
         mChangelistNumber = number;
      }

      int mChangelistNumber = -1;
      public int ChangelistNumber
      {
         get { return mChangelistNumber;  }
      }

      internal string ChangelistNumberAsPerforceString
      {
         get { return IsDefaultChangelist ? "default" : mChangelistNumber.ToString(); }
      }

      public string Description
      {
         get
         {
            if( !IsDefaultChangelist )
            {
               try
               {
                  P4UnParsedRecordSet recordSet = mConnection.mP4Api.RunUnParsed("change", "-o", ChangelistNumber.ToString());

                  P4Form form = mConnection.mP4Api.Parse_Form("change", recordSet.Messages[0]);
                  return form["Description"];
               }
               catch (P4API.Exceptions.PerforceInitializationError e)
               {
                  mConnection.HandlePerforceInitializationError(e);
                  return "";
               }
            }
            else
            {
               return "Default";
            }
         }

         set
         {
            // can't change description for default change list.
            if( !IsDefaultChangelist )
            {
               try
               {
                  P4UnParsedRecordSet recordSet = mConnection.mP4Api.RunUnParsed("change", "-o", ChangelistNumber.ToString());

                  P4Form form = mConnection.mP4Api.Parse_Form("change", recordSet.Messages[0]);
                  form["Description"] = value;
                  mConnection.mP4Api.Save_Form(form);
               }
               catch (P4API.Exceptions.PerforceInitializationError e)
               {
                  mConnection.HandlePerforceInitializationError(e);
               }
               catch (System.Exception e)
               {
                  string message = e.Message;
               }
            }
         }
      }

      public EStatus Status
      {
         get
         {
            EStatus status = EStatus.Unknown;

            try
            {
               P4UnParsedRecordSet recordSet = mConnection.mP4Api.RunUnParsed("change", "-o", ChangelistNumber.ToString());
               if (recordSet.Messages.Length > 0)
               {
                  P4Form form = mConnection.mP4Api.Parse_Form("change", recordSet.Messages[0]);
                  switch(form["Status"])
                  {
                     case "submitted":
                        status = EStatus.Submitted;
                        break;
                     case "pending":
                        status = EStatus.Pending;
                        break;
                  }
               }
            }
            catch (P4API.Exceptions.RunUnParsedException e)
            {
               string message = e.Message;
            }
            catch(P4API.Exceptions.PerforceInitializationError e)
            {
               mConnection.HandlePerforceInitializationError(e);
            }

            return status;
         }
      }

      public bool Valid
      {
         get
         {
            // default change list is always valid.
            if (IsDefaultChangelist)
               return true;

            List<Changelist> changelists = mConnection.GetPendingChangelists(false);
            
            if (changelists == null)
               return false;

            foreach(Changelist changelist in changelists)
            {
               if (changelist.ChangelistNumber == ChangelistNumber)
               {
                  return true;
               }
            }

            return false;
         }
      }

      public bool IsDefaultChangelist
      {
         get { return mChangelistNumber == 0; }
      }
      
      public List<FileInfo> Files
      {
         get
         {
            List<FileInfo> files = new List<FileInfo>();

            try
            {
               P4RecordSet res = mConnection.mP4Api.Run("opened", "-c", ChangelistNumberAsPerforceString);
               foreach (P4Record record in res)
               {
                  files.Add(new FileInfo(mConnection, record["depotFile"]));
               }
            }
            catch (P4API.Exceptions.PerforceInitializationError e)
            {
               mConnection.HandlePerforceInitializationError(e);
            }

            return files;
         }
      }

      public bool DeleteChangelist()
      {
         // Can't delete default change list
         if (IsDefaultChangelist)
            return false;

         try
         {
            P4RecordSet res;

            // Check for opened files, and fail delete if there are.
            res = mConnection.mP4Api.Run("opened", "-c", ChangelistNumber.ToString());
            if (res.Records.Length > 0)
            {
               return false;
            }

            res = mConnection.mP4Api.Run("change", "-d", ChangelistNumber.ToString());
            return !res.HasErrors();
         }
         catch (P4API.Exceptions.PerforceInitializationError e)
         {
            mConnection.HandlePerforceInitializationError(e);
            return false;
         }
      }

      public bool Open(string path, bool exclusive)
      {
         if (mConnection.IsConnected)
         {
            FileInfo info = mConnection.GetFileInfo(path);
            
            if( info == null )
            {
               return false;
            }

            if (info.ExistsInDepot)
            {
               if( info.CheckedOutByMe && info.Action == FileInfo.EAction.Delete )
               {
                  // NOTE: We want to revert the files no matter what change list they are in.
                  if (!mConnection.Revert(path, false))
                  {
                     return false;
                  }

                  if (!Edit(path, exclusive))
                  {
                     return false;
                  }
               }
               else if( info.CheckedOutByMe )
               {
                  if (!Reopen(path, exclusive))
                  {
                     return false;
                  }
               }
               else if( info.HeadDeleted )
               {
                  if( !Add(path, exclusive) )
                  {
                     return false;
                  }
               }
               else
               {
                  return Edit(path, exclusive);
               }
            }
            else
            {
               if (!Add(path, exclusive))
               {
                  return false;
               }
            }

            return true;
         }
         else
         {
            return false;
         }
      }

      public bool Add(string path, bool exclusive)
      {
         if (mConnection.IsConnected)
         {
            FileInfo info = mConnection.GetFileInfo(path);
            
            if( info == null )
            {
               return false;
            }

            if (info.CheckedOutByMe && (info.Action == FileInfo.EAction.Add || info.Action == FileInfo.EAction.Branch))
            {
               if (info.ChangeList != ChangelistNumber)
               {
                  if (!Reopen(path, exclusive))
                  {
                     return false;
                  }
               }
               
               return true;
            }
            else
            {
               if (exclusive && info.CheckedOutBySomeoneElse)
               {
                  return false;
               }

               try
               {
                  P4RecordSet res = mConnection.mP4Api.Run("add", "-c", ChangelistNumberAsPerforceString, path);

                  if (res.HasErrors())
                  {
                     return false;
                  }

                  return true;
               }
               catch (P4API.Exceptions.PerforceInitializationError e)
               {
                  mConnection.HandlePerforceInitializationError(e);
                  return false;
               }
            }
         }
         else
         {
            return false;
         }
      }

      public bool Edit(string path, bool exclusive)
      {
         if (mConnection.IsConnected)
         {
            try
            {
               FileInfo info = mConnection.GetFileInfo(path);

               if( info == null )
               {
                  return false;
               }

               if (info.CheckedOutByMe && info.Action == FileInfo.EAction.Edit)
               {
                  return true;
               }

               if (exclusive && info.CheckedOutBySomeoneElse)
               {
                  return false;
               }

               P4RecordSet res = mConnection.mP4Api.Run("edit", "-c", ChangelistNumberAsPerforceString, path);

               if (res.HasErrors())
               {
                  return false;
               }

               return true;
            }
            catch (P4API.Exceptions.PerforceInitializationError e)
            {
               mConnection.HandlePerforceInitializationError(e);
               return false;
            }
         }
         else
         {
            return false;
         }
      }

      /// <summary>
      /// This function will revert any files matching the passed path in THIS change list.
      /// </summary>
      /// <param name="path"></param>
      /// <returns></returns>
      public bool Revert(string path, bool revertUnchangedOnly)
      {
         if (mConnection.IsConnected)
         {
            try
            {
               FileInfo info = mConnection.GetFileInfo(path);

               if (info == null || !info.CheckedOutByMe)
               {
                  return false;
               }

               if (revertUnchangedOnly)
               {
                  P4RecordSet res = mConnection.mP4Api.Run("revert", "-a", "-c", ChangelistNumberAsPerforceString, path);
                  return !res.HasErrors();
               }
               else
               {
                  P4RecordSet res = mConnection.mP4Api.Run("revert", "-c", ChangelistNumberAsPerforceString, path);
                  return !res.HasErrors();
               }
            }
            catch (P4API.Exceptions.PerforceInitializationError e)
            {
               mConnection.HandlePerforceInitializationError(e);
               return false;
            }
         }
         else
         {
            return false;
         }
      }

      public bool Delete(string path, bool exclusive)
      {
         bool hasFilesToDelete = false;

         foreach(FileInfo info in mConnection.GetFileInfos(path))
         {
            if (!info.ExistsInDepot)
            {
               return false;
            }

            if (exclusive && info.CheckedOutBySomeoneElse)
            {
               return false;
            }

            // If this file needs deleting, we mark that we need to execute the delete operation.
            if( !info.CheckedOutByMe || info.Action != FileInfo.EAction.Delete )
            {
               hasFilesToDelete = true;
            }

            // if file is already checked out and we're not deleting it, then revert it first.
            if( info.CheckedOutByMe && info.Action != FileInfo.EAction.Delete )
            {
               if (!Revert(info.DepotPath, false))
               {
                  return false;
               }
            }
         }

         // No files needed to delete
         if (!hasFilesToDelete)
         {
            return true;
         }

         try
         {
            P4RecordSet res = mConnection.mP4Api.Run("delete", "-c", ChangelistNumberAsPerforceString, path);
            return !res.HasErrors();
         }
         catch (P4API.Exceptions.PerforceInitializationError e)
         {
            mConnection.HandlePerforceInitializationError(e);
            return false;
         }

      }

      public bool Reopen(string path, bool exclusive)
      {
         if (mConnection.IsConnected)
         {
            try
            {
               FileInfo info = mConnection.GetFileInfo(path);

               if (info == null || !info.CheckedOutByMe)
               {
                  return false;
               }

               P4RecordSet res = mConnection.mP4Api.Run("reopen", "-c", ChangelistNumberAsPerforceString, path);
               return !res.HasErrors();
            }
            catch (P4API.Exceptions.PerforceInitializationError e)
            {
               mConnection.HandlePerforceInitializationError(e);
               return false;
            }
         }
         else
         {
            return false;
         }
      }

      public bool Revert(bool revertUnchangedOnly)
      {
         if( mConnection.IsConnected )
         {
            try
            {
               P4RecordSet res;

               if (revertUnchangedOnly)
               {
                  res = mConnection.mP4Api.Run("revert", "-a", "-c", ChangelistNumberAsPerforceString, "//...");
               }
               else
               {
                  res = mConnection.mP4Api.Run("revert", "-c", ChangelistNumberAsPerforceString, "//...");
               }

               if (res.HasErrors())
               {
                  return false;
               }

               // we don't care if the delete change list fails if we're running in revertUnchanged mode, this is likely due to there still being changed files in the change list.
               if (revertUnchangedOnly)
               {
                  DeleteChangelist();
                  return true;
               }
               else
               {
                  return DeleteChangelist();
               }
            }
            catch (P4API.Exceptions.PerforceInitializationError e)
            {
               mConnection.HandlePerforceInitializationError(e);
               return false;
            }
         }
         else
         {
            return false;
         }
      }

      public bool HasChangedFiles()
      {
         if (mConnection.IsConnected)
         {
            try
            {
               // The -n flag displays what files would be affected but does not actually revert them.
               P4RecordSet res = mConnection.mP4Api.Run("revert", "-n", "-a", "-c", ChangelistNumberAsPerforceString, "//...");
               int numberOfFilesWithoutChanges = res.Records.Length;

               if (res.HasErrors())
               {
                  return true;
               }

               if (numberOfFilesWithoutChanges == Files.Count)
               {
                  return false;
               }

               return true;

            }
            catch (P4API.Exceptions.PerforceInitializationError e)
            {
               mConnection.HandlePerforceInitializationError(e);
               return false;
            }
         }
         else
         {
            return false;
         }
      }

      public bool Submit()
      {
         if (IsDefaultChangelist)
         {
            throw new Exception("Can't submit default change list");
         }

         if (mConnection.IsConnected)
         {
            try
            {
               P4RecordSet res = mConnection.mP4Api.Run("submit", "-c", ChangelistNumberAsPerforceString);
               return !res.HasErrors();
            }
            catch (P4API.Exceptions.PerforceInitializationError e)
            {
               mConnection.HandlePerforceInitializationError(e);
               return false;
            }
         }
         else
         {
            return false;
         }
      }
   }

   public class Connection
   {
      internal P4Connection mP4Api = new P4Connection();

      public Connection()
      {
      }

      public bool Connect()
      {
         try
         {
            mP4Api.Connect();

            if (mP4Api.IsValidConnection(true, true))
            {
               mIsConnected = true;
               return true;
            }
         }
         catch (System.Exception e)
         {
            string message = e.Message;
         }

         return false;
      }

      public void Disconnect()
      {
         mP4Api.Disconnect();
         mIsConnected = false;
      }

      private bool mIsConnected = false;
      
      public bool IsConnected
      {
         get { return mIsConnected; }
      }

      public string Port
      {
         get { return mP4Api.Port; }
         set { mP4Api.Port = value; }
      }

      public Changelist CreatePendingChangeList(string description)
      {
         if( IsConnected )
         {
            return new Changelist(this, description);
         }
         else
         {
            return null;
         }
      }

      public Changelist GetDefaultChangelist()
      {
         return new Changelist(this, 0);
      }

      public List<Changelist> GetPendingChangelists(bool includeDefaultChangelist)
      {
         if (IsConnected)
         {
            try
            {
               List<Changelist> pendingChangelists = new List<Changelist>();

               if (includeDefaultChangelist)
               {
                  pendingChangelists.Add(GetDefaultChangelist());
               }

               P4RecordSet res = mP4Api.Run("changes", "-c", mP4Api.Client, "-s", "pending");

               foreach (P4Record record in res)
               {
                  pendingChangelists.Add(new Changelist(this, int.Parse(record.Fields["change"])));
               }

               return pendingChangelists;
            }
            catch (P4API.Exceptions.PerforceInitializationError e)
            {
               HandlePerforceInitializationError(e);
               return null;
            }
         }
         else
         {
            return null;
         }
      }

      public FileInfo GetFileInfo(string path)
      {
         if( IsConnected )
         {
            List<FileInfo> fileInfos = GetFileInfos(path);

            if (fileInfos.Count > 0)
            {
               return fileInfos[0];
            }
         }

         return null;
      }

      public List<FileInfo> GetFileInfos(string path)
      {
         if (IsConnected)
         {
            List<FileInfo> fileInfos = new List<FileInfo>();

            try
            {
               P4RecordSet files = mP4Api.Run("fstat", path);

               foreach (string warning in files.Warnings)
               {
                  if (warning.Contains("no such file(s)."))
                  {
                     fileInfos.Add(new FileInfo(this, String.Empty));
                     return fileInfos;
                  }
               }
               foreach (P4Record file in files)
               {
                  fileInfos.Add(new FileInfo(this, file));
               }
            }
            catch (P4API.Exceptions.PerforceInitializationError e)
            {
               HandlePerforceInitializationError(e);
            }
         
            return fileInfos;
         }
         else
         {
            return null;
         }
      }

      internal void HandlePerforceInitializationError(P4API.Exceptions.PerforceInitializationError e)
      {
         mIsConnected = false;
      }

      public List<FileInfo> GetFileInfos(List<string> files)
      {
         if( IsConnected )
         {
            List<FileInfo> fileInfos = new List<FileInfo>();
            
            foreach(string file in files)
            {
               FileInfo info = GetFileInfo(file);
               if (info != null)
               {
                  fileInfos.Add(info);
               }
            }

            return fileInfos;
         }
         else
         {
            return null;
         }
      }

      public Changelist GetChangelistForPath(string path)
      {
         if( IsConnected )
         {
            try
            {
               P4RecordSet res = mP4Api.Run("fstat", path);

               if (res.Records.Length > 1)
               {
                  throw new Exception("Path matches multiple change list, shouldn't use wild cards for this function.");
               }
               else if (res.Records.Length == 1)
               {
                  string changeList = res.Records[0]["change"];
                  if (!string.IsNullOrEmpty(changeList))
                  {
                     return new Changelist(this, changeList == "default" ? 0 : int.Parse(changeList));
                  }
               }
            }
            catch(P4API.Exceptions.PerforceInitializationError e)
            {
               HandlePerforceInitializationError(e);
            }
         }

         return null;
      }

      public bool Revert(string path, bool revertUnchangedOnly)
      {
         if (IsConnected)
         {
            try
            {
               FileInfo info = GetFileInfo(path);

               if (info == null || !info.CheckedOutByMe)
               {
                  return false;
               }

               if (revertUnchangedOnly)
               {
                  P4RecordSet res = mP4Api.Run("revert", "-a", path);
                  return !res.HasErrors();
               }
               else
               {
                  P4RecordSet res = mP4Api.Run("revert", path);
                  return !res.HasErrors();
               }
            }
            catch (P4API.Exceptions.PerforceInitializationError e)
            {
               HandlePerforceInitializationError(e);
               return false;
            }
         }
         else
         {
            return false;
         }
      }

      public bool AreFilesAtHeadRevision(List<FileInfo> fileInfos)
      {
         if( IsConnected )
         {
            foreach (FileInfo fileInfo in fileInfos)
            {
               if (fileInfo.ExistsInDepot)
               {
                  if (fileInfo.HeadRevision != fileInfo.LocalRevision)
                  {
                     return false;
                  }
               }
            }

            return true;
         }
         else
         {
            return false;
         }
      }
   }
}
