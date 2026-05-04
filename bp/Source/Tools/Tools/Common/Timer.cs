using System;
using System.Collections.Generic;
using System.Text;

namespace Tools.Common
{
   /// <summary>
   /// High resolution timing support via Win32 QueryPerformanceCount APIs.
   /// .NET DateTime only has around 15ms resolution.
   /// </summary>
   public class HighResolutionTimeProvider
   {
      [System.Runtime.InteropServices.DllImport("Kernel32.dll")]
      public static extern bool QueryPerformanceCounter(out long ticks);

      [System.Runtime.InteropServices.DllImport("Kernel32.dll")]
      public static extern bool QueryPerformanceFrequency(out long freq);

      static private long sFrequency = QueryPerformanceFrequency();
      
      private static long QueryPerformanceFrequency()
      {
         long freq;
         QueryPerformanceFrequency(out freq);
         return freq;
      }
      
      public static long QueryPerformanceCounter()
      {
         long ticks;
         QueryPerformanceCounter(out ticks);
         return ticks;
      }

      public static double TicksToMilliseconds(long deltaTicks)
      {
         return TicksToSeconds(deltaTicks) * 1000.0;
      }
      
      public static double TicksToSeconds(long deltaTicks)
      {
         // Use microsecond resolution
         long deltaTime_us = (deltaTicks * 1000 * 1000) / sFrequency;
         double deltaTime_secs = deltaTime_us * 1e-6;
         return deltaTime_secs;
      }
      
   }
   
   /// <summary>
   /// Quick helper class to use for timing code.
   /// using (new Timer("TimeThis") { code needing timing }
   /// </summary>
   public class Timer : IDisposable
   {
      string            mLabel;
      long              mStartTicks;
      bool              mAlreadyShownTime;
      
      public Timer(string label)
      {
         mLabel = label;
         mAlreadyShownTime = false;
         mStartTicks = HighResolutionTimeProvider.QueryPerformanceCounter();
      }
      
      public void ShowTimeDelta()
      {
         long deltaTicks = HighResolutionTimeProvider.QueryPerformanceCounter() - mStartTicks;
         System.Diagnostics.Trace.WriteLine(String.Format("{0}:{1} ms", mLabel, HighResolutionTimeProvider.TicksToMilliseconds(deltaTicks)));      
         mAlreadyShownTime = true;
      }
      
      public void Dispose()
      {
         if (!mAlreadyShownTime)
         {
            ShowTimeDelta();
         }
         GC.SuppressFinalize(true);
      }
   }
}
