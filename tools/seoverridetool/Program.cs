using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.RegularExpressions;

namespace seoverridetool
{
   enum ESECategory
   {
      kSECategory_InternalSnake,       //resident snake voice
      kSECategory_InternalRaiden,      //resident raiden voice
      kSECategory_InternalShared,      //resident only need one copy
      kSECategory_External,            //general nonresident
      kSECategory_ExternalVrVox,       //VR / Alternative stage only overrides (VR enemy vox)
   };

   enum EResampleRate
   {
      kResampleRate_None,         //all source files 44.1k
      kResampleRate_22_05k        //half sample rate (for non-hero voice)
   };

   class SEOverride
   {
      public SEOverride(int code, ESECategory category, string name, bool stereo, bool noamp, bool forceHiQuality)
      {
         mCode = code;
         mCategory = category;
         mName = name;
         mbStereo = stereo;
         mbNoAmp = noamp;

         if (!IsInternal && !forceHiQuality)
         {
            //All non-hero vox that were spot checked are played back at 22.05K in original sound data.
            //Cut the sample rate in half for the replacement ones going in to match this rate and save space.
            mResampleRate = EResampleRate.kResampleRate_22_05k;
         }
         else
         {
            mResampleRate = EResampleRate.kResampleRate_None;
         }
      }

      public bool IsInternal
      {
         get
         {
            return mCategory == ESECategory.kSECategory_InternalSnake
                || mCategory == ESECategory.kSECategory_InternalRaiden
                || mCategory == ESECategory.kSECategory_InternalShared
                ;
         }
      }

      public int           mCode;
      public ESECategory   mCategory;
      public string        mName;
      public int           mFrequency;
      public byte[]        mWaveformData0;
      public byte[]        mWaveformData1;
      public bool          mbStereo;
      public bool          mbNoAmp;
      public EResampleRate mResampleRate;
   }

   struct SBPMidiCommandMGS2
   {
      public byte mdata4;
      public byte mdata3;
      public byte mdata2;
      public byte mdata1;
   };

   struct SBPSoundEffectOverrideTrackMGS2
   {
      public int mWavOffset;
      public int mWavSize;
      public SBPMidiCommandMGS2 mTempoSetCommand;
      public SBPMidiCommandMGS2 mVolChgCommand;
      public SBPMidiCommandMGS2 mFlgSetCommand;
      public SBPMidiCommandMGS2 mEofSetCommand;
      public SBPMidiCommandMGS2 mMnoSetCommand;
      public SBPMidiCommandMGS2 mDetuneSetCommand;
      public SBPMidiCommandMGS2 mPanSetCommand;
      public SBPMidiCommandMGS2 mNoteSetCommand;
      public SBPMidiCommandMGS2 mBlockEndCommand;
   };

   struct SBPSoundEffectOverrideMGS2
   {
      public uint                mSoundCode;
      public int                 mTrackCount;

      public SBPSoundEffectOverrideTrackMGS2 mTrack0;
      public SBPSoundEffectOverrideTrackMGS2 mTrack1;
   };

   unsafe struct PSQ_IWAV
   {
      public uint        adsr;
      public uint        addr;	// d31:memory stream / d23-d00:spu addr
      public ushort      frq;
      public ushort      size;
      public fixed byte  pad[4];
   };

   unsafe struct SBPSoundEffectOverrideMGS3
   {
      public uint                mSoundCode;
      public PSQ_IWAV            mIWAV;
      public fixed byte          mMidiProgram[64];
   };

   class Program
   {
      private static string mOutputFilename;
      private static List<string> mInputHeaderFilenames;
      private static string mSourceWavFolder;
      private static SortedDictionary<uint, SEOverride> mOverrideMap;
      private static bool mbVerifySourceFilesConsistent;
      private static int mMgsVersion;
      private static bool mbPatch;

      private static string[] skHighReverbSubfolderList =
      {
         "\\se0c\\", //sp03a
         "\\se0d\\", //sp03a
         "\\se16\\", //sp*, tsp*
         "\\se18\\", //sp*, tsp*
         "\\se1c\\", //sp*, st*
         "\\se1e\\", //wp*, twp*
         "\\se2e\\", //unused?
         "\\se38\\", //in-game
         "\\se3e\\", //in-game
         "\\se3c\\", //n_title
         "\\se41\\", //in-game
         "\\se42\\", //in-game
         "\\se44\\", //in-game
         "\\se45\\", //in-game
         "\\se48\\", //in-game
         "\\se49\\", //in-game
         "\\se4c\\", //in-game
         "\\se4e\\", //in-game
         "\\se4f\\", //in-game
         "\\se50\\", //in-game
         "\\se51\\", //in-game
         "\\se52\\", //in-game
         "\\se53\\", //in-game
         "\\se54\\", //in-game
         "\\se57\\", //in-game
         "\\se5a\\", //in-game
         "\\se5b\\", //in-game
         "\\se5d\\", //in-game
         "\\se5e\\", //in-game
         "\\se72\\", //unused?
         "\\se73\\", //in-game
         "\\se74\\", //in-game
         "\\se75\\", //in-game
         "\\se76\\", //in-game
         "\\se77\\", //in-game
         "\\se78\\", //in-game
         "\\se79\\", //in-game
         "\\se7b\\", //in-game
         "\\se7c\\", //in-game
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

      public static void WriteStruct<T>(Stream outStream, T data)
      {
         byte[] buffer = new byte[Marshal.SizeOf(typeof(T))];
         GCHandle handle = GCHandle.Alloc(buffer, GCHandleType.Pinned);
         Marshal.StructureToPtr((object)data, handle.AddrOfPinnedObject(), false);
         outStream.Write(buffer, 0, Marshal.SizeOf(typeof(T)));
         handle.Free();
      }

      private static bool IsHighReverbPath(string fullPath)
      {
         foreach (string highReverbPath in skHighReverbSubfolderList)
         {
            if (fullPath.Contains(highReverbPath))
            {
               //This is a high reverb source file subfolder.  Always prefer to take this version.
               return true;
            }
         }
         //Doesn't match our list of known high-reverb paths.
         return false;
      }

      private static uint BuildKeyFromCodeAndCategory(int code, ESECategory category)
      {
         //N.B. just use top byte for category since all the codes are 16-bit.
         if ((code & 0xff000000) != 0)
         {
            throw new AbortException("Unexpected code with nonzero top byte: " + code);
         }

         uint keyCode = (uint)code | ((uint)category << 24);
         return keyCode;
      }

      private static void ReadSEOverrideHeaderFile(string filename)
      {
         Regex commentRegex = new Regex(@"^\s*//.");
         Regex nameIndexRegex = new Regex(@"\s*#define\s*(?<name>\w+)\s+0x(?<code>[0-9a-fA-F]+)");

         using (StreamReader streamReader = File.OpenText(filename))
         {
            string line;
            while ((line = streamReader.ReadLine()) != null)
            {
               if (commentRegex.IsMatch(line))
               {
                  //skip over comments
                  continue;
               }
               Match nameIndexMatch = nameIndexRegex.Match(line);
               if (nameIndexMatch.Success)
               {
                  string labelString = nameIndexMatch.Groups["name"].Value;
                  string codeString = nameIndexMatch.Groups["code"].Value;
                  int code = Int32.Parse(codeString, NumberStyles.HexNumber);
                  bool stereo = line.Contains("BP_STEREO");
                  bool noamp = line.Contains("BP_NO_AMP");
                  bool snake_only = line.Contains("BP_SNAKE_ONLY");
                  bool force_hiq;
                  if (mMgsVersion == 2)
                  {
                     force_hiq = line.Contains("BP_FORCE_HIQ");
                  }
                  else
                  {
                     //In MGS3, there are far fewer SE overrides and they are almost all UI.
                     //Just force them all to be original sample rate.
                     force_hiq = true;
                  }
                  bool is_vox = labelString.StartsWith("SD_V_");
                  Console.WriteLine(labelString + " = " + codeString + (stereo ? " (stereo)" : "") + (noamp ? " (noamp)" : ""));
                  if (mMgsVersion == 2)
                  {
                     if (code <= 0xff || code == 0x5be || code == 0x512 || (code >= 0x513 && code <= 0x516))
                     {
                        if( code <= 0xff && !is_vox )
                        {
                           //Internal SE same b/t Raiden and Snake.  Insert one copy.
                           uint sharedCode = BuildKeyFromCodeAndCategory(code, ESECategory.kSECategory_InternalShared);
                           mOverrideMap.Add(sharedCode, new SEOverride(code, ESECategory.kSECategory_InternalShared, labelString, stereo, noamp, force_hiq));
                        }
                        else
                        {
                           //Internal SE: need an override for both Snake and Raiden version
                           //since the codes are the same but they have diff. waveforms based
                           //on which resident sound pak is loaded.
                           //NOTE: 0x5be is some weird jingle that was in internal but probably doesn't matter for us
                           //NOTE: 0x512 is the sneeze, which isn't an internal SE but needs to switch based on snake / raiden
                           //NOTE: 0x513 - 0x516 is the hero being electrocuted in the Shell 2 core.  Snake version was added for Substance.
                           uint snakeCode = BuildKeyFromCodeAndCategory(code, ESECategory.kSECategory_InternalSnake);
                           mOverrideMap.Add(snakeCode, new SEOverride(code, ESECategory.kSECategory_InternalSnake, labelString, stereo, noamp, force_hiq));
                           uint raidenCode = BuildKeyFromCodeAndCategory(code, ESECategory.kSECategory_InternalRaiden);
                           mOverrideMap.Add(raidenCode, new SEOverride(code, ESECategory.kSECategory_InternalRaiden, labelString, stereo, noamp, force_hiq));
                        }
                     }
                     else if (snake_only && (code == 0x435 || code == 0x436))
                     {
                        //BP_CODE_DATA_PATCH
                        //ANOTHER special case discovered after initial ship.  Raiden's two lines "I can't move..." and "My body..."
                        //reaction to Vamp's shadow knife attack are duplicated for Snake in Boss Survival, so we need a special case
                        //to build a patch BP_SE.DAT file to be built into code.  The existing Raiden voice override is in kSECategory_External
                        //in the shipped BP_SE.DAT, so just add the Snake one as kSECategory_Snake for the patch.
                        //The runtime code has new logic to handle both cases.

                        //For a proper unified BP_SE.DAT for use by a non patch build, these two codes would just be additional special cases
                        //which produce internal snake and internal raiden overrides above.
                        //i.e. change the line
                        //                      if (code <= 0xff || code == 0x5be || code == 0x512 || (code >= 0x513 && code <= 0x516))
                        // to
                        //                      if (code <= 0xff || code == 0x5be || code == 0x512 || (code >= 0x513 && code <= 0x516) || (code == 0x435 || code == 0x436))
                        // and remove this separate block.
                        uint snakeCode = BuildKeyFromCodeAndCategory(code, ESECategory.kSECategory_InternalSnake);
                        mOverrideMap.Add(snakeCode, new SEOverride(code, ESECategory.kSECategory_InternalSnake, labelString, stereo, noamp, force_hiq));
                     }
                     else
                     {
                        //General external.
                        uint keyCode = BuildKeyFromCodeAndCategory(code, ESECategory.kSECategory_External);
                        mOverrideMap.Add(keyCode, new SEOverride(code, ESECategory.kSECategory_External, labelString, stereo, noamp, force_hiq));
                        if (code >= 0x100 && code <= 0x165)
                        {
                           //All of these enemy vox sounds need an additional different version for VR missions.
                           //If the game tries to play one, the category will be set to VR if the player is in a VR stage.
                           //Otherwise, it will use the default "external" override with normal Japanese voice.
                           uint keyCodeVR = BuildKeyFromCodeAndCategory(code, ESECategory.kSECategory_ExternalVrVox);
                           mOverrideMap.Add(keyCodeVR, new SEOverride(code, ESECategory.kSECategory_ExternalVrVox, labelString, stereo, noamp, force_hiq));
                        }
                     }
                  }
                  else if (mMgsVersion == 3)
                  {
                     uint keyCode = (uint)code & 0x7ff;
                     mOverrideMap.Add(keyCode, new SEOverride(code, ESECategory.kSECategory_External, labelString, stereo, noamp, force_hiq));
                  }
               }
            }
         }
      }

      private static void ReadSEOverrideHeaderFiles()
      {
         mOverrideMap = new SortedDictionary<uint, SEOverride>();
         foreach (string s in mInputHeaderFilenames)
         {
            ReadSEOverrideHeaderFile(s);
         }
      }

      static void GetWaveformData( uint seCode, SEOverride se )
      {
         string aifFilename;
         string subfolder = "";

         if (mMgsVersion == 2)
         {
            //MGS2 se source folder layout:
            //se1f：ライデン声タンカー常駐SE (Raiden Tanker)
            //se2f：スネーク声プラント、ＶＲ常駐SE (Snake Plant+VR)
            //se5f：ライデン声プラント、ＶＲ常駐SE (Raiden Plant+VR)
            //se7f：スネーク声タンカー常駐SE (Snake Tanker)

            //se5f matches se1f contents except it contains 4 additional aifs for phone: 0x9B,0x9C,0x9D,0xA0
            //se2f matches se7f except for same.
            switch (se.mCategory)
            {
               case ESECategory.kSECategory_InternalSnake:
                  if (se.mCode == 0x512)
                  {
                     //Case for sneeze sound effect which isn't actually internal!
                     subfolder = "external\\se72\\";  //Any of the tanker sound pakfiles will do for Snake.
                  }
                  else if (se.mCode >= 0x513 && se.mCode <= 0x516)
                  {
                     //For these four, Snake's Japanese overrides are in a special redelivery folder from KJP.
                     subfolder = "bp_jp_substance\\snake";
                  }
                  else if (se.mCode == 0x435 || se.mCode == 0x436)
                  {
                     //Special case for Snake's equivalent of "I can't move..." and "My body..." in Vamp battle.
                     //These only exist in one pak file / subfolder.
                     subfolder = "external\\se2e\\";
                  }
                  else
                  {
                     //Default Snake internal folder
                     subfolder = "internal\\se2f\\";
                  }
                  break;
               case ESECategory.kSECategory_InternalRaiden:
                  if (se.mCode == 0x512)
                  {
                     //Case for sneeze sound effect which isn't actually internal!
                     subfolder = "external\\se41\\";  //Raiden version only present in arsenal gear section
                  }
                  else if (se.mCode >= 0x513 && se.mCode <= 0x516)
                  {
                     //Electrocution sound effects were not in resident pakfile.  They only existed in Shell 2 Core 1F for the
                     //electrified floor.
                     //There were two versions: se38 (which was for Snake Tales) and se54 (for the main game's Plant chapter).
                     //se38 in the source drop just contains binary copies of the English voice for these.
                     //Make sure we take the main game version for Raiden otherwise he'll just have David Hayter's voice!
                     subfolder = "external\\se54\\";
                  }
                  /*
                  // BP_CODE_DATA_PATCH
                  else if (se.mCode == 0x435 || se.mCode == 0x436)
                  {
                     //Special case for Raiden's equivalent of "I can't move..." and "My body..." in Vamp battle.
                     //These only exist in three pak file / subfolder.
                     //Make sure that we grab a Raiden version and not the Snake version.
                     subfolder = "external\\se53\\";
                  }
                  */
                  else
                  {
                     //Default Raiden internal folder
                     subfolder = "internal\\se5f\\";
                  }
                  break;
               case ESECategory.kSECategory_InternalShared:
                  {
                     //Snake and Raiden version of these are the same.  Take Raiden Plant.
                     subfolder = "internal\\se5f\\";
                  }
                  break;
               case ESECategory.kSECategory_External:
                  if (se.mCode == 0x5ef || se.mCode == 0x5d6)
                  {
                     //Two replacement Japanese SE for enemy voices that were added for substance.
                     //0x5ef is some guy falling off the bridge in a boss fight in snake tales E, a13c.
                     //0x5d6 is an unknown voice apparently used in [w|a|ta]01d (Deck-D of tanker)
                     //might have been added for a Raiden alternative mission in Tanker
                     //(description translates as "Raiden losing grip dismay voice")
                     subfolder = "bp_jp_substance\\extra";
                  }
                  else
                  {
                     subfolder = "external\\";
                  }
                  break;
               case ESECategory.kSECategory_ExternalVrVox:
                  {
                     subfolder = "bp_jp_substance\\vr";
                  }
                  break;
               default:
                  throw new AbortException("TODO: unhandled SE category!");
            }
            aifFilename = String.Format("0x{0:x2}", se.mCode) + ".aif";
         }
         else
         {
            aifFilename = String.Format("{0:d4}{1}", seCode, se.mName.Substring(3)) + ".aif";
            aifFilename = aifFilename.ToLower();
         }

         string searchPath = Path.Combine(mSourceWavFolder, subfolder);
         string[] files = Directory.GetFiles(searchPath, aifFilename, SearchOption.AllDirectories);

         Array.Sort(files);   //ordering doesn't matter, but we want it to be consistent across runs of the tool.

         if (files.Length == 0)
         {
            throw new AbortException("Error: could not find source file " + aifFilename);
         }

         if (mbVerifySourceFilesConsistent && files.Length > 1)
         {
            byte[] aifData = File.ReadAllBytes(files[0]);
            for (int i = 1; i < files.Length; ++i)
            {
               byte[] cmpData = File.ReadAllBytes(files[i]);
               if (!aifData.SequenceEqual(cmpData))
               {
                  throw new AbortException("Error: aif mismatch: " + files[0] + " and " + files[i]);
               }
            }
         }

         //Convert to wav, then to msf, then read the adpcm data.
         string aifFullPath = null;
         //Step to ensure that if there are multiple versions of a waveform in the source dump, we don't
         //inadvertently take one that was recorded from a no-reverb area such as a cinematic.
         //All gameplay areas pretty much have enough reverb that we want the high reverb version of all overrides
         //if possible.
         foreach (string fullPath in files)
         {
            if (IsHighReverbPath(fullPath))
            {
               aifFullPath = fullPath;
               break;
            }
         }
         if (string.IsNullOrEmpty(aifFullPath))
         {
            //If we didn't find one explicitly with high reverb, just punt to first one found.
            if (!se.IsInternal && mMgsVersion==2)
            {
               //Only a couple seem to only exist in rooms with no reverb.  Presumably they're not supposed to have reverb ever.
               Console.WriteLine("Warning: no explicitly high-reverb version found for " + aifFilename);
            }
            //(none of the internal SE have no-reverb versions sitting in the source sound data)
            aifFullPath = files[0];
         }

         string wavFilename = aifFullPath.Replace(".aif", ".wav");
         string msfFilenameBase = aifFullPath.Replace(".aif", ".msf");

         //to wav
         {
            string processFileName = "sox.exe";
            string processArguments = "";
            processArguments = aifFullPath + " " + wavFilename;
            //Most source waveforms are about 3db too quiet.  Amplify most of them at this step
            //to match the in-game volume.
            if (!se.mbNoAmp)
            {
               float gain = 3.0f;
               processArguments += " vol +" + gain.ToString() + " Db";
            }
            switch (se.mResampleRate)
            {
               case EResampleRate.kResampleRate_22_05k:
                  processArguments += " rate -h 22.05k";
                  break;
            }
            System.Diagnostics.Process process = new System.Diagnostics.Process();
            process.StartInfo.UseShellExecute = false;

            process.StartInfo.FileName = processFileName;
            process.StartInfo.Arguments = processArguments;

            process.Start();
            process.WaitForExit();
            if (process.ExitCode != 0)
            {
               throw new AbortException("sox failed: " + process.ExitCode);
            }

         }

         int channels = se.mbStereo ? 2 : 1;

         //to msf and get waveforms
         for( int ch=0; ch < channels; ++ch )
         {
            string msfFilename = msfFilenameBase + "." + ch.ToString();
            string processFileName = "msenc.exe";
            string processArguments = "";
            processArguments += " -in " + wavFilename; //specify input filename.
            string channelArg = ch==1 ? " X 0" : " 0 X";
            processArguments += channelArg;  //specify which input channel for the mono output channel.

            processArguments += " -out " + msfFilename; //specify output filename.

            processArguments += " -x"; //overwrite always.
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

            //Read in the ADPCM data.
            using (BinaryReader inReader = new Tools.Common.BinaryReaderBigEndian(File.Open(msfFilename, FileMode.Open)))
            {
               inReader.ReadInt32();   //skip over 'MSF' and version
               inReader.ReadInt32();   //skip over compression type
               int msfChannels = inReader.ReadInt32();
               int msfSampleSize = inReader.ReadInt32();
               int msfSampleRate = inReader.ReadInt32();
               inReader.ReadInt32();   //skip over miscInfo
               for (int i = 0; i < 10; ++i)
               {
                  inReader.ReadInt32();   //skip over loop markers
               }

               if (msfChannels != 1)
               {
                  throw new AbortException("wav / source channel mismatch: " + msfChannels);
               }

               //Fill in frequency and waveform data.
               se.mFrequency = msfSampleRate;

               switch(ch)
               {
               case 1:
                  {
                     se.mWaveformData1 = inReader.ReadBytes(msfSampleSize);
                     if ((se.mWaveformData1.Length % 16) != 0)
                     {
                        throw new AbortException("unaligned vag waveform length: " + se.mWaveformData1.Length);
                     }
                     //Add block end marker.
                     se.mWaveformData1[se.mWaveformData1.Length - 15] = 0x1;
                  }
                  break;
               case 0:
                  {
                     se.mWaveformData0 = inReader.ReadBytes(msfSampleSize);
                     if ((se.mWaveformData0.Length % 16) != 0)
                     {
                        throw new AbortException("unaligned vag waveform length: " + se.mWaveformData0.Length);
                     }
                     //Add block end marker.
                     se.mWaveformData0[se.mWaveformData0.Length - 15] = 0x1;
                  }
                  break;
               default:
                  throw new AbortException("TODO!");
               }

            }

            //Get rid of the intermediate files.
            File.Delete(msfFilename);
         }

         //Get rid of the intermediate files.
         File.Delete(wavFilename);
      }

      static void SetMidiTrackMGS2(ref SBPSoundEffectOverrideTrackMGS2 track, int wavCode, uint stepcount, int lrc, EResampleRate resampleRate )
      {
         //Each midi program varies only by waveform index and stepcount.
         //We need 12 bits for wav index since there are several hundred of them.
         //The rest of the mno_set nibbles are 0xf to indicate that this is a "wav override" type.
         if (wavCode > 0xfff)
         {
            throw new AbortException("TODO: need more space in MGS2 mno commands!");
         }

         //tempo_set (depends on waveform length)
         track.mTempoSetCommand.mdata4 = 0x00;
         track.mTempoSetCommand.mdata3 = 0x00;
         track.mTempoSetCommand.mdata2 = 0x7e;//default tempo
         track.mTempoSetCommand.mdata1 = 0xd0;
         //vol_chg (fixed)
         track.mVolChgCommand.mdata4 = 0x00;
         track.mVolChgCommand.mdata3 = 0x00;
         track.mVolChgCommand.mdata2 = 0xff;  //vol (max)
         track.mVolChgCommand.mdata1 = 0xd5;
         //flg_set (fixed)
         track.mFlgSetCommand.mdata4 = 0x00;
         track.mFlgSetCommand.mdata3 = 0x01;
         track.mFlgSetCommand.mdata2 = 0x01;
         track.mFlgSetCommand.mdata1 = 0xfe;  //flag set cmd
         //eof_set (fixed)
         track.mEofSetCommand.mdata4 = 0x00;
         track.mEofSetCommand.mdata3 = 0x00;
         track.mEofSetCommand.mdata2 = 0x00;
         track.mEofSetCommand.mdata1 = 0xf7;  //effect off cmd
         //mno_set (fixed except for mdata4 and mdata3)
         track.mMnoSetCommand.mdata4 = 0x00;
         track.mMnoSetCommand.mdata3 = 0x00;
         track.mMnoSetCommand.mdata2 = 0xff;  //code for BP override wav
         track.mMnoSetCommand.mdata1 = 0xfd;  //mno command
         //detune_set (fixed)
         track.mDetuneSetCommand.mdata4 = 0x00;
         track.mDetuneSetCommand.mdata3 = 0x00;
         track.mDetuneSetCommand.mdata2 = 0x00;//detune: filled in below.
         track.mDetuneSetCommand.mdata1 = 0xe0;
         //pan_set (l or r or c)
         track.mPanSetCommand.mdata4 = 0x00;
         track.mPanSetCommand.mdata3 = 0x00; //center pan
         track.mPanSetCommand.mdata2 = 0x00;
         track.mPanSetCommand.mdata1 = 0xdd;
         //note_set (fixed except for mdata2)
         track.mNoteSetCommand.mdata4 = 0x78; //vol
         track.mNoteSetCommand.mdata3 = 0x63; //ngg = 100%
         track.mNoteSetCommand.mdata2 = 0x00; //stepcount: filled in below
         track.mNoteSetCommand.mdata1 = 0x00; //note: filled in below
         //block_end (fixed)
         track.mBlockEndCommand.mdata4 = 0x00;
         track.mBlockEndCommand.mdata3 = 0x00;
         track.mBlockEndCommand.mdata2 = 0xfe;
         track.mBlockEndCommand.mdata1 = 0xff;

         //custom data in commands.
         track.mMnoSetCommand.mdata4 = (byte)(wavCode & 0xff);
         track.mMnoSetCommand.mdata3 = (byte)(((wavCode >> 8) & 0x0f) | 0xf0);

         uint tempo = 0x7e;
         //handle sounds longer than 2.55 seconds by cutting tempo and stepcount in half
         //until it works.
         while (stepcount > 0xff)
         {
            stepcount /= 2;
            tempo /= 2;
         }

         track.mTempoSetCommand.mdata2 = (byte)tempo;
         //Pan goes from -20 to 20
         switch (lrc)
         {
            case 0:  //l
               track.mPanSetCommand.mdata3 = 0xec;
               break;
            case 1:  //r
               track.mPanSetCommand.mdata3 = 0x14;
               break;
         }
         track.mNoteSetCommand.mdata2 = (byte)stepcount;

         switch (resampleRate)
         {
            case EResampleRate.kResampleRate_None:
               {
                  //44.1kHz
                  //freq_tbl[0x2e]==3818 with a detune of -16 = 3764
                  //(3764/4096) * 48000 -> 44109.375
                  track.mNoteSetCommand.mdata1 = 0x2e;   //note: freq_tbl[0x2e]==3818
                  track.mDetuneSetCommand.mdata2 = 0xf0; //detune: -16
               }
               break;
            case EResampleRate.kResampleRate_22_05k:
               {
                  //22.05kHz
                  //freq_tbl[0x22]==1909 with a detune of -16 = 1882
                  //(1882/4096) * 48000 -> 22054.6875
                  track.mNoteSetCommand.mdata1 = 0x22;   //note: freq_tbl[0x22]==1909
                  track.mDetuneSetCommand.mdata2 = 0xf0; //detune: -16
               }
               break;
            default:
               throw new AbortException("TODO!");
         }
      }

      static void WriteOutputFileMGS2()
      {
         MemoryStream waveformOutStream = new MemoryStream(8 * 1024 * 1024);
         BinaryWriter waveformStreamWriter = new BinaryWriter(waveformOutStream);

         //Write out waveform data; collect offsets.
         int[] wavOffsets = new int[mOverrideMap.Count*2];
         int wavIndex = 0;
         int currWavOffset = 0;
         foreach (KeyValuePair<uint, SEOverride> kvp in mOverrideMap)
         {
            uint code = kvp.Key;
            SEOverride se = kvp.Value;
            waveformStreamWriter.Write( se.mWaveformData0 );
            wavOffsets[wavIndex++] = currWavOffset;
            currWavOffset += se.mWaveformData0.Length;
            if (se.mbStereo)
            {
               waveformStreamWriter.Write(se.mWaveformData1);
               wavOffsets[wavIndex++] = currWavOffset;
               currWavOffset += se.mWaveformData1.Length;
            }
         }
         //Write out override entries.
         using (BinaryWriter outWriter = new BinaryWriter(File.Open(mOutputFilename, FileMode.Create)))
         {
            string magicNumber = "SEO2";
            int version = 1;
            outWriter.Write( magicNumber.ToCharArray(), 0, 4 );
            outWriter.Write(version);
            outWriter.Write(mOverrideMap.Count);

            wavIndex = 0;
            SBPSoundEffectOverrideMGS2 seEntrySize = new SBPSoundEffectOverrideMGS2();
            int baseWavOffset = 2 * sizeof(int) //main header
                              + System.Runtime.InteropServices.Marshal.SizeOf(seEntrySize) * mOverrideMap.Count;   //entries
            //Align waveform data.
            baseWavOffset = (baseWavOffset + 15) & (~15);
            int entryIndex = 0;
            int patch = mbPatch ? 1 : 0;
            foreach (KeyValuePair<uint, SEOverride> kvp in mOverrideMap)
            {
               uint code = kvp.Key;
               SEOverride se = kvp.Value;

               //stepcount determines time before note cutoff, measured in 1/100s if default tempo.
               uint stepcount = (uint)(se.mWaveformData0.Length * 28 / 16 / (se.mFrequency / 100));

               SBPSoundEffectOverrideMGS2 seEntry = new SBPSoundEffectOverrideMGS2();
               seEntry.mSoundCode = code;
               seEntry.mTrackCount = se.mbStereo ? 2 : 1;

               for (int tr = 0; tr < seEntry.mTrackCount; ++tr)
               {
                  //Set top bit of 12-bit wav code to indicate second track.
                  //Bottom 11 bits are se override entry index 0...2047.
                  if (entryIndex > 0x3ff)
                  {
                     throw new AbortException("Error: can't support more than 10 bits of SE entry!");
                  }
                  int wavCode = entryIndex | (tr << 11) | (patch << 10);
                  if (tr == 0)
                  {
                     //2 == center, if this is a mono SE.
                     int lrc = (seEntry.mTrackCount == 2) ? 0 : 2;
                     seEntry.mTrack0.mWavOffset = baseWavOffset + wavOffsets[wavIndex];
                     seEntry.mTrack0.mWavSize = se.mWaveformData0.Length;
                     SetMidiTrackMGS2(ref seEntry.mTrack0, wavCode, stepcount, lrc, se.mResampleRate);
                  }
                  else
                  {
                     seEntry.mTrack1.mWavOffset = baseWavOffset + wavOffsets[wavIndex];
                     seEntry.mTrack1.mWavSize = se.mWaveformData1.Length;
                     SetMidiTrackMGS2(ref seEntry.mTrack1, wavCode, stepcount, 1, se.mResampleRate);
                  }
                  ++wavIndex;
               }
               //Write out this entry.
               WriteStruct(outWriter.BaseStream, seEntry);
               ++entryIndex;
            }

            while (outWriter.BaseStream.Position < baseWavOffset)
            {
               outWriter.Write((byte)0);
            }
            //Write out the waveforms.
            outWriter.Write( waveformOutStream.GetBuffer(), 0, (int)waveformOutStream.Position );
         }
      }

      unsafe static void WriteOutputFileMGS3()
      {
         MemoryStream waveformOutStream = new MemoryStream(8 * 1024 * 1024);
         BinaryWriter waveformStreamWriter = new BinaryWriter(waveformOutStream);

         //Write out waveform data; collect offsets.
         int[] wavOffsets = new int[mOverrideMap.Count];
         int wavIndex = 0;
         int currWavOffset = 0;
         foreach (KeyValuePair<uint, SEOverride> kvp in mOverrideMap)
         {
            uint code = kvp.Key;
            SEOverride se = kvp.Value;
            if (se.mbStereo)
            {
               throw new AbortException("TODO!");
            }
            waveformStreamWriter.Write(se.mWaveformData0);
            wavOffsets[wavIndex++] = currWavOffset;
            currWavOffset += se.mWaveformData0.Length;
         }

         SBPSoundEffectOverrideMGS3 seEntry = new SBPSoundEffectOverrideMGS3();
         seEntry.mIWAV.adsr = 0x1fc300ff; //same for all.
         //Write out all but the variable part of the midi program.
         //N.B. All multi-byte numbers are big endian.
         int midiWavIndexAt;
         int midiDelayAt;
         {
            //Header
            seEntry.mMidiProgram[ 0] = 0x4d; //'M'
            seEntry.mMidiProgram[ 1] = 0x54; //'T'
            seEntry.mMidiProgram[ 2] = 0x68; //'h'
            seEntry.mMidiProgram[ 3] = 0x64; //'d'
            seEntry.mMidiProgram[ 7] = 0x06; //apparently unused; just filling in what I saw in existing data
            seEntry.mMidiProgram[ 9] = 0x01; //apparently unused; just filling in what I saw in existing data
            //nTrack
            int ntrack = 1;
            seEntry.mMidiProgram[10] = (byte)(ntrack >> 8);
            seEntry.mMidiProgram[11] = (byte)(ntrack & 0xff);
            //timebase
            int timebase = 960;
            seEntry.mMidiProgram[12] = (byte)(timebase >> 8);
            seEntry.mMidiProgram[13] = (byte)(timebase & 0xff);

            int i = 14;
            //Track
            seEntry.mMidiProgram[i++] = 0x4d;   //'M'
            seEntry.mMidiProgram[i++] = 0x54;   //'T'
            seEntry.mMidiProgram[i++] = 0x72;   //'r'
            seEntry.mMidiProgram[i++] = 0x6b;   //'k'
            //Size (only matters if there's a track after this one)
            int trackSize = 64 - i;
            seEntry.mMidiProgram[i++] = (byte)(trackSize >> 24);
            seEntry.mMidiProgram[i++] = (byte)(( trackSize >> 16 ) & 0xff);
            seEntry.mMidiProgram[i++] = (byte)(( trackSize >> 8 ) & 0xff);
            seEntry.mMidiProgram[i++] = (byte)(trackSize & 0xff);
            //delta 0
            seEntry.mMidiProgram[i++] = 0;
            //mdCtrl( ch0 -> vol = 0x7f ) 127 is max
            int ch = 0;
            seEntry.mMidiProgram[i++] = (byte)(0xb0 & ch);
            seEntry.mMidiProgram[i++] = 0x07;
            seEntry.mMidiProgram[i++] = 0x7f;
            //delta 0
            seEntry.mMidiProgram[i++] = 0;
            //mdCtrl( pan = 64 ) NOTE: mdCtrl command 0xb0 need not be repeated
            seEntry.mMidiProgram[i++] = 0x0a;
            seEntry.mMidiProgram[i++] = 0x40;
            //delta 0
            seEntry.mMidiProgram[i++] = 0;
            //mdCtrl( all reset controller ) NOTE: mdCtrl command 0xb0 need not be repeated
            seEntry.mMidiProgram[i++] = 0x79;
            seEntry.mMidiProgram[i++] = 0;
            //delta 0
            seEntry.mMidiProgram[i++] = 0;
            //Reverb ON for all tracks in SE overrides unless we find a case where it should be otherwise.
            //mdCtrl( dsp on ) NOTE: mdCtrl command 0xb0 need not be repeated
            seEntry.mMidiProgram[i++] = 0x5b;
            seEntry.mMidiProgram[i++] = 21;     //reverb depth
            //delta 0
            seEntry.mMidiProgram[i++] = 0;
            //BP_mdNoteOnWavOverride( wav high byte, wav low byte )
            seEntry.mMidiProgram[i++] = 0xfe;
            midiWavIndexAt = i;
            seEntry.mMidiProgram[i++] = 0;
            seEntry.mMidiProgram[i++] = 0;
            //delay( nonzero, based on waveform playback time)
            midiDelayAt = i;
            seEntry.mMidiProgram[i++] = 0x80;
            seEntry.mMidiProgram[i++] = 0x80;
            seEntry.mMidiProgram[i++] = 0x00;
            //mdNoteOn( ch0-> note, vel )
            seEntry.mMidiProgram[i++] = 0x90;
            seEntry.mMidiProgram[i++] = 0x3c;   //note FIXED in code!  (do not change)
            seEntry.mMidiProgram[i++] = 0;      //vel==0 means off
            //delta 0
            seEntry.mMidiProgram[i++] = 0;
            //mdMeta(FIN)
            seEntry.mMidiProgram[i++] = 0xff;
            seEntry.mMidiProgram[i++] = 0x2f;
         }

         //Write out override entries.
         using (BinaryWriter outWriter = new BinaryWriter(File.Open(mOutputFilename, FileMode.Create)))
         {
            string magicNumber = "SEO3";
            outWriter.Write( magicNumber.ToCharArray(), 0, 4 );
            outWriter.Write( mOverrideMap.Count );

            wavIndex = 0;
            int baseWavOffset = 2 * sizeof(int) //main header
                              + System.Runtime.InteropServices.Marshal.SizeOf(seEntry) * mOverrideMap.Count;   //entries
            //Align waveform data.
            baseWavOffset = (baseWavOffset + 15) & (~15);
            foreach (KeyValuePair<uint, SEOverride> kvp in mOverrideMap)
            {
               uint code = kvp.Key;
               SEOverride se = kvp.Value;

               //midi command has two bytes for waveform index.
               if (wavIndex > 0xffff)
               {
                  throw new AbortException("TODO: need more space in MGS3 midi commands!");
               }

               seEntry.mSoundCode = code;
               seEntry.mIWAV.size = (ushort)(se.mWaveformData0.Length / 16); //measured in VAG packets
               seEntry.mIWAV.addr = (uint)(baseWavOffset + wavOffsets[wavIndex]);
               seEntry.mIWAV.frq = (ushort)(se.mFrequency * 4096 / 48000);

               seEntry.mMidiProgram[midiWavIndexAt] = (byte)(wavIndex >> 8);
               seEntry.mMidiProgram[midiWavIndexAt+1] = (byte)(wavIndex & 0xff);

               //calculate delay before voiceOff. (vag bytes -> samples -> seconds -> ms)
               uint lengthMs = (uint)(se.mWaveformData0.Length * 28 / 16 / (se.mFrequency / 1000));
               //There are basically a bunch of magic numbers multiplied together at runtime...
               //so for our purposes, the unit of delay is a little more than 1/4ms.
               //this is a conservative calculation given the fixed timestep above.
               //(For reference: 0xb20 or 2848 results in a delay of 750ms.)
               //this is a variable-length number, each byte is unsigned 0...127 with the top bit
               //indicating whether it's the last byte or not.  3 should be plenty.
               uint delay = (uint)(3.7 * lengthMs);
               if( delay > 0x1fffff )
               {
                  throw new AbortException("TODO: need more space in MGS3 note off delay!");
               }
               uint delayRemain = delay;
               seEntry.mMidiProgram[midiDelayAt+2] = (byte)(delayRemain & 0x7f);
               delayRemain >>= 7;
               seEntry.mMidiProgram[midiDelayAt+1] = (byte)(0x80 | (delayRemain & 0x7f));
               delayRemain >>= 7;
               seEntry.mMidiProgram[midiDelayAt] = (byte)(0x80 | (delayRemain & 0x7f));

               //Write out this entry.
               WriteStruct(outWriter.BaseStream, seEntry);
               ++wavIndex;
            }

            while (outWriter.BaseStream.Position < baseWavOffset)
            {
               outWriter.Write((byte)0);
            }
            //Write out the waveforms.
            outWriter.Write( waveformOutStream.GetBuffer(), 0, (int)waveformOutStream.Position );
         }
      }

      static void ParseCommandLine(string[] args)
      {
         mInputHeaderFilenames = new List<string>();
         for (int argIndex = 0; argIndex != args.Length; /* increment in loop */ )
         {
            switch (args[argIndex].ToLower())
            {
               case "-o":
                  mOutputFilename = args[argIndex + 1];
                  argIndex += 2;
                  break;
               case "-s":
                  mSourceWavFolder = args[argIndex + 1];
                  argIndex += 2;
                  break;
               case "-v":
                  mbVerifySourceFilesConsistent = true;
                  argIndex += 1;
                  break;
               case "-p":
                  mbPatch = true;
                  argIndex += 1;
                  break;
               case "-mgs2":
                  mMgsVersion = 2;
                  argIndex += 1;
                  break;
               case "-mgs3":
                  mMgsVersion = 3;
                  argIndex += 1;
                  break;
               default:
                  mInputHeaderFilenames.Add( args[argIndex] );
                  argIndex += 1;
                  break;
            }
         }
         if (mOutputFilename == null)
         {
            throw new UsageException("No output filename specified.");
         }
         if (mInputHeaderFilenames.Count == 0)
         {
            throw new UsageException("No input header filenames specified.");
         }
         if (mMgsVersion == 0)
         {
            throw new UsageException("No mgs version specified.");
         }
      }

      static void PrintUsage()
      {
         Console.WriteLine("Usage:");
         Console.WriteLine("seoverridetool.exe -o <output filename> -s <source wav folder> <input header filename> -mgs[2|3] [-v]");
         Console.WriteLine("-v: verify that all source .aif files with the same name are binary same.");
         Console.WriteLine("-mgs[]: specify mgs version, 2 or 3.");
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
            ReadSEOverrideHeaderFiles();

            if (mOverrideMap.Count > 0xfff)
            {
               throw new AbortException("TODO: need more space in MGS2 mno commands!");
            }

            foreach (KeyValuePair<uint, SEOverride> kvp in mOverrideMap)
            {
               GetWaveformData(kvp.Key, kvp.Value);
            }

            if (mMgsVersion == 2)
            {
               WriteOutputFileMGS2();
            }
            else if (mMgsVersion == 3)
            {
               WriteOutputFileMGS3();
            }
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
