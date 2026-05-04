using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Xml;

namespace ArchiveTool
{
   class Exception : System.Exception
   {
      public Exception( String s, params object[] p )
         : base( String.Format( s, p ) )
      {
      }

      public Exception( String s )
      : base( s )
      {
      }
   }

   class UsageException : Exception
   {
      public UsageException( String s, params object[] p )
         : base( s, p )
      {
      }

      public UsageException( String s )
         : base( s )
      {
      }
   }

   class CommandLine
   {
      public struct PatternEntry
      {
         public PatternEntry( String folder, bool recurse )
         {
            mPattern = folder;
            mRecurse = recurse;
         }

         public readonly String mPattern;
         public readonly bool mRecurse;
      }

      public CommandLine( string[] args )
      {
         bool isRecursing = false;

         for ( int argIdx = 0; argIdx < args.Length; /* increment in loop */ )
         {
            String arg = args[argIdx];
            ++argIdx;

            if ( arg == "-o" )
            {
               mOutFile = args[argIdx];
               ++argIdx;
            }
            else if ( arg == "-r" )
            {
               isRecursing = true;
            }
            else if ( arg == "-nr" )
            {
               isRecursing = false;
            }
            else if ( arg == "-oa" )
            {
               mOutArchive = args[argIdx];
               ++argIdx;
            }
            else if ( arg == "-a" )
            {
               mIsAppend = true;
            }
            else if ( arg == "-?" )
            {
               throw new UsageException( "" );
            }
            else if ( arg == "-v" )
            {
               mIsVerbose = true;
               Console.WriteLine( "This tool is running with high verbosity and low brevity" );
            }
            else
            {
               mInPatterns.Add( new PatternEntry( arg, isRecursing ) );
            }
         }

         if ( mOutFile == null )
         {
            throw new UsageException( "Error: Output file wasn't specified" );
         }

         if ( mInPatterns.Count == 0 )
         {
            throw new UsageException( "Error: No input folders specified" );
         }

         if ( !mIsAppend && mOutArchive == null )
         {
            throw new UsageException( "Error: Out archive is require if append is not specified" );
         }

         if ( mIsAppend && mOutArchive != null )
         {
            throw new UsageException( "Error: Can't append and change the output archive (-oa and -a is invalid)" );
         }
      }

      public readonly String mOutFile = null;
      public readonly String mOutArchive = null;
      public readonly List<PatternEntry> mInPatterns = new List<PatternEntry>();
      public readonly bool mIsAppend = false;
      public readonly bool mIsVerbose = false;
   }
   
   class Program
   {
      static void PrintUsage()
      {
         Console.WriteLine( @"
Usage:
   ArchiveTool <-o file> <-a | -oa file> [-v] pattern [-r|-nr] [pattern...]

-o file  - file is the archive xml to output to
-oa file - file is the archive that the archive xml will output
-a       - appends files to the archive xml
-v       - verbose
-r       - causes future patterns on the command line to search recursively
-nr      - future patterns on the command line don't recurse (default)
pattern  - wildcard pattern of folders and files whose files belong in the xml
" );
      }

      static void PrintVerbose( CommandLine cmd, String s, params object[] fmt )
      {
         if ( cmd.mIsVerbose )
         {
            Console.WriteLine( s, fmt );
         }
      }

      static int Main( string[] args )
      {
         try
         {
            CommandLine cmd = new CommandLine( args );

            XmlDocument doc = new XmlDocument();
            XmlElement fileContainer;

            if ( cmd.mIsAppend )
            {
               PrintVerbose( cmd, "Opening {0} for append", cmd.mOutFile );
               doc.Load( cmd.mOutFile );
               fileContainer = (XmlElement)doc.SelectSingleNode( "psarc/create" );
               if ( fileContainer == null )
               {
                  throw new Exception( "{0} has malformed XML, can't find psarc/create tag", cmd.mOutFile );
               }
            }
            else
            {
               PrintVerbose( cmd, "Opening {0} for create", cmd.mOutFile );
               PrintVerbose( cmd, "... will write to archive {0}", cmd.mOutArchive );
               fileContainer = (XmlElement)doc.AppendChild( doc.CreateElement( "psarc" ) ).AppendChild( doc.CreateElement( "create" ) );
               fileContainer.SetAttribute( "archive", cmd.mOutArchive );
               fileContainer.SetAttribute( "absolute", "true" );
               fileContainer.SetAttribute( "overwrite", "true" );
            }

            foreach ( CommandLine.PatternEntry pattern in cmd.mInPatterns )
            {
               PrintVerbose( cmd, "Parsing folder: {0} Recurse:{1}", pattern.mPattern, pattern.mRecurse );

               int lastSlash = pattern.mPattern.LastIndexOfAny( new char[] { '\\', '/' } );

               String rootFolder;
               String leafPattern = "*";

               if ( lastSlash == -1 )
               {
                  rootFolder = pattern.mPattern;
                  leafPattern = "*";
               }
               else
               {
                  rootFolder = pattern.mPattern.Substring( 0, lastSlash );
                  leafPattern = pattern.mPattern.Substring( lastSlash + 1 );
                  if ( leafPattern == "" )
                  {
                     leafPattern = "*";
                  }
               }

               SearchOption searchOption = pattern.mRecurse ? SearchOption.AllDirectories : SearchOption.TopDirectoryOnly;

               foreach ( String file in Directory.GetFiles( rootFolder, leafPattern, searchOption ) )
               {
                  String srcFileName = file;
                  String dstFileName = "/" + file.ToLower().Replace( '\\', '/' );

                  XmlElement fileElement = (XmlElement)fileContainer.AppendChild( doc.CreateElement( "file" ) );
                  fileElement.SetAttribute( "path", srcFileName );
                  fileElement.SetAttribute( "archivepath", dstFileName );

                  PrintVerbose( cmd, "{0}", dstFileName );
               }
            }

            PrintVerbose( cmd, "Saving {0}", cmd.mOutFile );
            doc.Save( cmd.mOutFile );
            Console.WriteLine( "Built archive XML for {0}", cmd.mOutFile );
         }
         catch ( UsageException e )
         {
            Console.WriteLine( e.Message );
            Console.WriteLine();
            PrintUsage();
         }
         catch ( System.Exception e )
         {
            Console.WriteLine( "Error:" );
            Console.WriteLine( e.Message );
            Console.WriteLine( e.StackTrace );

            return 1;
         }

         return 0;
      }
   }
}
