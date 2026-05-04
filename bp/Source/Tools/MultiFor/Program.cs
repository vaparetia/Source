using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Diagnostics;
using System.Threading;

namespace MultiFor
{
   class Program
   {
      enum FileSetMode
      {
         PathSpec,
         TextFile
      };

      static int numProcesses = 1;
      static FileSetMode mode = FileSetMode.PathSpec;
      static string variableName;
      static string command;
      static string commandParameters;
      static string fileSpec;

      static void Usage()
      {
         Console.WriteLine("MultiFor [/p<count>] [/f] %variable IN (set) DO command [command-parameters]");
         Console.WriteLine();
         Console.WriteLine("/p<count>            Will spawn up to <count> simultaneous commands.");
         Console.WriteLine("/f                   (set) is text file to have lines processed.");
         Console.WriteLine("%variable            Specifies a single letter replaceable parameter.");
         Console.WriteLine("(set)                Specifies a set of one or more files. Wildcards may be used.");
         Console.WriteLine("command              Specifies the command to carry out for each file.");
         Console.WriteLine("command-parameters   Specifies parameters or switches for the specified command.");
      }

      static bool ParseArgs(string[] args)
      {
         for (int current = 0; current < args.Length; )
         {
            string currentArg = args[current];
            string currentArgLower = currentArg.ToLower();

            if( currentArg.StartsWith("%") )
            {
               variableName = currentArg;
               ++current;
            }
            else if( currentArg.StartsWith("/") )
            {
               string parameterName = currentArgLower.Substring(1);
               
               switch(parameterName[0])
               {
                  case 'p':
                     numProcesses = int.Parse(parameterName.Substring(1));
                     break;

                  case 'f':
                     mode = FileSetMode.TextFile;
                     break;
               }

               ++current;
            }
            else if( currentArgLower == "in" )
            {
               if ((current + 1) < args.Length)
               {
                  string fileSet = args[current + 1];
                  fileSpec = fileSet.Trim('(', ')');

                  current += 2;
               }
               else
                  return false;
            }
            else if( currentArgLower == "do" )
            {
               ++current;

               if (current < args.Length)
               {
                  command = args[current];
                  ++current;

                  commandParameters = "";

                  for( ; current < args.Length; ++current )
                  {
                     if (commandParameters.Length > 0)
                        commandParameters += " ";

                     commandParameters += args[current];
                  }
               }
               else
                  return false;
            }
         }

         return variableName != null && fileSpec != null && command != null && commandParameters != null;
      }

      static List<Process> runningProcesses = new List<Process>();

      static void StallUntilProcessesRunning(int maxRunningCount)
      {
         while (runningProcesses.Count > maxRunningCount)
         {
            List<Process> stillAliveProcesses = new List<Process>();

            foreach (Process process in runningProcesses)
            {
               if (!process.HasExited)
                  stillAliveProcesses.Add(process);
            }

            runningProcesses = stillAliveProcesses;

            if (runningProcesses.Count <= maxRunningCount)
               break;

            Thread.Sleep(100);
         }
      }

      static int Main(string[] args)
      {
         if( !ParseArgs(args) )
         {
            Usage();
            return -1;
         }

         List<string> files = new List<string>();

         switch(mode)
         {
            case FileSetMode.PathSpec:
               {
                  string path = Path.GetDirectoryName(fileSpec);

                  if (path == "")
                     path = Environment.CurrentDirectory;

                  string searchPattern = Path.GetFileName(fileSpec);

                  files.AddRange(Directory.GetFiles(path, searchPattern, SearchOption.TopDirectoryOnly));
               }
               break;

            case FileSetMode.TextFile:
               {
                  using(StreamReader reader = new StreamReader(fileSpec))
                  {
                     while(!reader.EndOfStream)
                     {
                        string line = reader.ReadLine();
                        files.Add(line);
                     }
                  }
               }
               break;
         }

         foreach( string file in files )
         {
            StallUntilProcessesRunning(numProcesses - 1);

            string curCommandParameters = commandParameters.Replace(variableName, string.Format("\"{0}\"", file));

            ProcessStartInfo startInfo = new ProcessStartInfo(command, curCommandParameters);
            startInfo.UseShellExecute = false;
            Process newProcess = Process.Start(startInfo);
            runningProcesses.Add(newProcess);
         }

         StallUntilProcessesRunning(0);

         return 0;
      }

   }
}
