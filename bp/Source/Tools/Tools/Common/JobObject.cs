using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace Tools.Common
{
   [Flags]
   public enum JobLimitFlags : uint
   {
      None = 0x00000000,
      WorkingSet = 0x00000001,
      ProcessTime = 0x00000002,
      JobTime = 0x00000004,
      ActiveProcess = 0x00000008,
      Affinity = 0x00000010,
      PriorityClass = 0x00000020,
      PreserveJobTime = 0x00000040,
      SchedulingClass = 0x00000080,
      ProcessMemory = 0x00000100,
      JobMemory = 0x00000200,
      DieOnUnhandledException = 0x00000400,
      BreakawayOk = 0x00000800,
      SilentBreakawayOk = 0x00001000,
      KillJobOnClose = 0x00002000,
   }

   [StructLayout(LayoutKind.Sequential)]
   public class JobBasicLimitInformation
   {
      public long PerProcessUserTimeLimit;
      public long PerJobUserTimeLimit;
      public JobLimitFlags LimitFlags;
      public uint MinimumWorkingSetSize;
      public uint MaximumWorkingSetSize;
      public uint ActiveProcessLimit;
      public IntPtr Affinity;
      public uint PriorityClass;
      public uint SchedulingClass;
   }

   [StructLayout(LayoutKind.Sequential)]
   public class JobExtendedLimitInformation : JobBasicLimitInformation
   {
      public ulong ReadOperationCount;
      public ulong WriteOperationCount;
      public ulong OtherOperationCount;
      public ulong ReadTransferCount;
      public ulong WriteTransferCount;
      public ulong OtherTransferCount;
      public uint ProcessMemoryLimit;
      public uint JobMemoryLimit;
      public uint PeakProcessMemoryUsed;
      public uint PeakJobMemoryUsed;
   }

   public class JobObject : IDisposable
   {
      HandleRef mJobObject;

      public JobObject()
      {
         mJobObject = new HandleRef(this, CreateJobObject(null, null));
      }

      public static JobObject NewObjectThatKillsProcessesOnClose()
      {
         JobObject job = new JobObject();

         JobExtendedLimitInformation limitInfo = job.ExtendedLimitInformation;
         limitInfo.LimitFlags = JobLimitFlags.KillJobOnClose;
         job.ExtendedLimitInformation = limitInfo;

         return job;
      }

      public JobExtendedLimitInformation ExtendedLimitInformation
      {
         get 
         {
            JobExtendedLimitInformation limitInfo = new JobExtendedLimitInformation();
            
            if (!QueryInformationJobObject(mJobObject, JobObjectInfo.JobObjectExtendedLimitInformation, limitInfo, Marshal.SizeOf(limitInfo), IntPtr.Zero))
               throw new System.ComponentModel.Win32Exception();

            return limitInfo;
         }

         set
         {
            if (!SetInformationJobObject(mJobObject, JobObjectInfo.JobObjectExtendedLimitInformation, value, Marshal.SizeOf(value)))
               throw new System.ComponentModel.Win32Exception();
         }
      }
      public void AssignProcess(Process process)
      {
         AssignProcessToJobObject(mJobObject.Handle, process.Handle);
      }

      public void Dispose()
      {
         CloseHandle(mJobObject.Handle);
         mJobObject = new HandleRef();
      }

      #region PInvoke support code & structs
      public enum JobObjectInfo
      {
         JobObjectBasicAccountingInformation = 1,
         JobObjectBasicLimitInformation,
         JobObjectBasicProcessIdList,
         JobObjectBasicUIRestrictions,
         JobObjectSecurityLimitInformation,
         JobObjectEndOfJobTimeInformation,
         JobObjectAssociateCompletionPortInformation,
         JobObjectBasicAndIoAccountingInformation,
         JobObjectExtendedLimitInformation,
         JobObjectJobSetInformation,
         MaxJobObjectInfoClass
      }

      [StructLayout(LayoutKind.Sequential)]
      class SecurityAttributes
      {
         public int Length;
         public IntPtr SecurityDescriptor;
         public int InheritHandle;
      }

      [DllImport("kernel32.dll", CharSet = CharSet.Unicode)]
      static extern IntPtr CreateJobObject([In, MarshalAs(UnmanagedType.LPStruct)] SecurityAttributes lpJobAttributes, string lpName);

      [DllImport("kernel32.dll", SetLastError = true)]
      static extern bool SetInformationJobObject(HandleRef hjob, JobObjectInfo ic, [In, MarshalAs(UnmanagedType.LPStruct)] JobBasicLimitInformation bli, int cbBai);

      [DllImport("kernel32.dll", SetLastError = true)]
      static extern bool QueryInformationJobObject(HandleRef hjob, JobObjectInfo ic, [Out, MarshalAs(UnmanagedType.LPStruct)] JobBasicLimitInformation bli, int cbBai, IntPtr ignore);
      
      [DllImport("kernel32.dll")]
      [return: MarshalAs(UnmanagedType.Bool)]
      static extern bool AssignProcessToJobObject(IntPtr hJob, IntPtr hProcess);

      [DllImport("kernel32.dll", SetLastError = true)]
      [return: MarshalAs(UnmanagedType.Bool)]
      static extern bool CloseHandle(IntPtr hObject);
      #endregion
   }
}
