using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Xml;

namespace streamdataconvert
{
   class Program
   {
      static string mOutputFormat;
      static string mInputFilename;
      static string mOutputFilename;
      static string mXmlInputFilename;

      enum EAudioFormat
      {
         kAudioFormat_PCM,
         kAudioFormat_STV,
      };

      class AbortException : Exception
      {
         public AbortException(String s, params object[] o)
            : base(String.Format(s, o))
         {
         }

         public AbortException(String s)
            : base(s)
         {
         }
      }

      class UsageException : Exception
      {
         public UsageException(String s, params object[] o)
            : base(String.Format(s, o))
         {
         }

         public UsageException(String s)
            : base(s)
         {
         }
      }

      static void ParseCommandLine(string[] args)
      {
         for (int argIndex = 0; argIndex != args.Length; /* increment in loop */ )
         {
            switch (args[argIndex].ToLower())
            {
               case "-f":
                  mOutputFormat = args[argIndex + 1];
                  argIndex += 2;
                  break;
               case "-o":
                  mOutputFilename = args[argIndex + 1];
                  argIndex += 2;
                  break;
               case "-xml":
                  mXmlInputFilename = args[argIndex + 1];
                  argIndex += 2;
                  break;
               default:
                  mInputFilename = args[argIndex];
                  argIndex += 1;
                  break;
            }
         }
         if (mInputFilename == null)
         {
            throw new UsageException("No output filename specified.");
         }
      }

      static void PrintUsage()
      {
         Console.WriteLine("Usage:");
         Console.WriteLine("streamdataconvert.exe [-f <output format>] [-o <output filename] [-xml <xml input filename>] <input filename>");
         Console.WriteLine("Supported output formats:");
         Console.WriteLine("* pcm");
         Console.WriteLine("* stv");
      }

      static void ConvertPcmStv( EAudioFormat audioFormat )
      {
         //Info on the output format should be in the extracted xml file.
         if( mOutputFilename == null )
         {
            mOutputFilename = mInputFilename.Replace( ".wav", ".pcm" );
         }

         if (mXmlInputFilename == null)
         {
            //Output of CStreamDriver_STV produces an xml with a different extension for not really a good reason!
            //Need to find it properly if not explicitly specified anyway.
            if (audioFormat == EAudioFormat.kAudioFormat_PCM)
            {
               mXmlInputFilename = mInputFilename.Replace(".wav", ".pcm") + ".xml";
            }
            else
            {
               mXmlInputFilename = mInputFilename.Replace(".wav", ".stv") + ".xml";
            }
         }

         XmlDocument xmlDoc = new XmlDocument();
         xmlDoc.Load(mXmlInputFilename);
         XmlElement frequencyElement = (XmlElement)xmlDoc.SelectSingleNode("//frequency");
         XmlElement formatElement = (XmlElement)xmlDoc.SelectSingleNode("//format");
         XmlElement channelsElement = (XmlElement)xmlDoc.SelectSingleNode("//channels");
         int frequency = Int32.Parse(frequencyElement.InnerText);
         int channels = Int32.Parse(channelsElement.InnerText);
         if (channels != 1 && channels != 2)
         {
            throw new AbortException("Unexpected channel count: should be 1 or 2!: " + channels);
         }
         int pcmFormat = 0;
         if( audioFormat == EAudioFormat.kAudioFormat_PCM )
         {
            pcmFormat = Int32.Parse(formatElement.InnerText);
            if (pcmFormat != 0)
            {
               throw new AbortException("TODO: non-adpcm format!");
            }
         }
         //STV is always ADPCM.

         //From here, format is essentially the same.  The stream writer will write out a slightly different
         //format header for each.

         string msfFilename = mInputFilename + ".msf";
         string processFileName = "msenc.exe";

         string processArguments = "";
         processArguments += " -in " + mInputFilename; //specify input filename.
         //Assume that the source wav is set up properly wrt expected channels...
         for( int i=0; i < channels; ++i )
         {
            processArguments += " " + i.ToString();
         }

         processArguments += " -out " + msfFilename; //specify output filename.

         processArguments += " -x"; //overwrite always.
         //NOTE: according to MultiStream_Tools-Users_Guide_e.pdf, you can choose Normal / High / Low / 4-bit for
         //adpcm.  No explanation as to what these mean, but "High" resulted in noticeable compression artifacts in
         //v040_010_r010_52 so I have switched this to use default.
         processArguments += " -adpcm 0"; //adpcm (default)

         System.Diagnostics.Process process = new System.Diagnostics.Process();
         process.StartInfo.UseShellExecute = false;

         process.StartInfo.FileName = processFileName;
         process.StartInfo.Arguments = processArguments;

         process.Start();
         process.WaitForExit();
         if (process.ExitCode != 0)
         {
            throw new AbortException("msenc failed: " + process.ExitCode);
         }

         //Read in the ADPCM data.  It is interleaved at one vag packet / channel.
         //We need to do two things: re-interleave at and round up to 2K bytes / channel
         //and set "loop" marker as second byte of each packet.

         Byte[] msfBytes;
         using (BinaryReader inReader = new Tools.Common.BinaryReaderBigEndian(File.Open(msfFilename, FileMode.Open)))
         {
            inReader.ReadInt32();   //skip over 'MSF' and version
            inReader.ReadInt32();   //skip over compression type
            int msfChannels = inReader.ReadInt32();
            int msfSampleSize = inReader.ReadInt32();
            int msfSampleRate = inReader.ReadInt32();
            inReader.ReadInt32();   //skip over miscInfo
            for( int i=0; i < 10; ++i )
            {
               inReader.ReadInt32();   //skip over loop markers
            }

            if (msfChannels != channels)
            {
               throw new AbortException("wav / source channel mismatch: " + msfChannels);
            }
            //Overwrite frequency and data size.
            frequency = msfSampleRate;

            msfBytes = inReader.ReadBytes(msfSampleSize);
         }

         if( msfBytes.Length % (16*channels) != 0 )
         {
            throw new AbortException("Error: MSF output data should be (16*channels) byte aligned in size!" );
         }

         const int kDstInterleaveBytes = 0x800;
         //Round up outBytesLength to 2K * channels; padding VAG packets will be 0.
         uint outBytesLength = (uint)(msfBytes.Length + (channels * kDstInterleaveBytes - 1)) & (~((uint)channels * kDstInterleaveBytes-1));
         Byte[] outBytes = new Byte[outBytesLength];
         int outi = 0;
         for (int i = 0; i < msfBytes.Length; i += 16*channels)
         {
            //Copy out one packet for each channel.
            for (int ch = 0; ch < channels; ++ch)
            {
               int src = i + ch * 16;
               int dst = outi + ch * kDstInterleaveBytes;
               for (int j = 0; j < 16; ++j)
               {
                  outBytes[dst + j] = msfBytes[src + j];
               }
            }

            //update dest index.
            outi += 16;
            if (( outi % kDstInterleaveBytes )== 0)
            {
               //Skip to the next "frame" at each interleave increment.
               outi += kDstInterleaveBytes * (channels-1);
            }
         }
         //Set loop flag.
         for (int i = 0; i < outBytes.Length; i+=16)
         {
            outBytes[i + 1] = 0x2;
         }

         int size = outBytes.Length;
         using (BinaryWriter outWriter = new BinaryWriter(File.Open(mOutputFilename, FileMode.Create)))
         {
            //N.B. PCM and STV can be left in same format at this step.
            outWriter.Write(size);
            outWriter.Write(frequency);
            outWriter.Write(channels);
            outWriter.Write(pcmFormat);
            outWriter.Write(outBytes);
         }

         File.Delete(msfFilename);
      }

      static void ConvertWav()
      {
         if (mOutputFormat == null )
         {
            throw new AbortException("Must specify output format for wavs!");
         }
         switch (mOutputFormat)
         {
            case "pcm":
               ConvertPcmStv(EAudioFormat.kAudioFormat_PCM);
               break;
            case "stv":
               ConvertPcmStv(EAudioFormat.kAudioFormat_STV);
               break;
            default:
               throw new AbortException("Unknown wav output format: " + mOutputFormat);
         }
      }
      static void Convert()
      {
         string ext = Path.GetExtension(mInputFilename);
         switch (ext)
         {
            case ".wav":
               ConvertWav();
               break;
            default:
               throw new AbortException("Unknown input extension: " + ext);
         }
      }

      static int Main(string[] args)
      {
         try
         {
            ParseCommandLine(args);
         }
         catch (UsageException e)
         {
            Console.WriteLine(e.Message);
            Console.WriteLine();
            PrintUsage();

            return 1;
         }

         try
         {
            Convert();
         }
         catch (AbortException e)
         {
            Console.WriteLine("Fatal error - aborting:");
            Console.WriteLine(e.Message);
            return 3;
         }
         catch (Exception e)
         {
            Console.WriteLine("Fatal error:");
            Console.WriteLine(e.Message);
            Console.WriteLine();
            Console.WriteLine(e.StackTrace);
            return 1;
         }

         return 0;
      }
   }
}
