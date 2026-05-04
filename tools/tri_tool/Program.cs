using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Diagnostics;
using System.Text.RegularExpressions;

namespace tri_tool
{
   class Program
   {
      struct TriData
      {
         public TriData(int _id, int _size)
         {
            id = _id;
            size = _size;
         }
         public int id, size;
      }
      static Dictionary<string, TriData> trinames = new Dictionary<string, TriData>();
      static Dictionary<string, int> commonNames = new Dictionary<string, int>();
      static Dictionary<string, string> texDic = new Dictionary<string, string>();
      static Dictionary<string, int> dupTex = new Dictionary<string, int>();
      static Dictionary<string, string> texDicByHash = new Dictionary<string, string>();

      static string RunProcess(string filename, string workingDirectory, string args)
      {
         Process p = new Process();
         // Redirect the output stream of the child process.
         p.StartInfo.UseShellExecute = false;
         p.StartInfo.RedirectStandardOutput = true;
         p.StartInfo.FileName = filename;
         p.StartInfo.Arguments = args;
         p.StartInfo.WorkingDirectory = workingDirectory;
         p.Start();
         // Do not wait for the child process to exit before
         // reading to the end of its redirected stream.
         // p.WaitForExit();
         // Read the output stream first and then wait.
         string output = p.StandardOutput.ReadToEnd();
         p.WaitForExit();

         return output;
      }

      static string GetMD5HashFromFile(string fileName)
      {
         FileStream file = new FileStream(fileName, FileMode.Open);
         System.Security.Cryptography.MD5 md5 = new System.Security.Cryptography.MD5CryptoServiceProvider();
         byte[] retVal = md5.ComputeHash(file);
         file.Close();

         StringBuilder sb = new StringBuilder();
         for (int i = 0; i < retVal.Length; i++)
         {
            sb.Append(retVal[i].ToString("x2"));
         }
         return sb.ToString();
      }

      static int numMissed = 0;
      static int numProcessed = 0;

      // go through and remove duplicates and empty folders
      static void RemoveDuplicateTextures(string f, bool deleteDuplicates, int game)
      {
         string[] folders = Directory.GetDirectories(f);
         foreach (string folder in folders)
         {
            // avoid infinite loop
            if ((File.GetAttributes(folder) & FileAttributes.ReparsePoint) == 0)
            {
               RemoveDuplicateTextures(folder, deleteDuplicates, game);
            }
         }

         string[] files = Directory.GetFiles(f);
         Regex reTGA = new Regex(@"\.tga$");
         foreach (string file in files)
         {
            if (reTGA.IsMatch(file))
            {
               try
               {
                  string md5 = GetMD5HashFromFile(file);
                  bool isMd5Dup = false;
                  bool isNewPath = false;
                  if (texDicByHash.ContainsKey(md5))
                  {
                     isMd5Dup = true;
                     if (texDicByHash[md5] != file)
                     {
                        isNewPath = true;
                     }
                  }
                  else
                  {
                     texDicByHash[md5] = file;
                  }
                  if (isMd5Dup && isNewPath && deleteDuplicates)
                  {
                     File.Delete(file);
                  }
               }
               catch (System.Exception)
               {
                  numMissed++;
               }
               ++numProcessed;
               if ((numProcessed & 15) == 0)
               {
                  System.Console.WriteLine("Num processed = {0}", numProcessed);
               }
            }
         }
         if (files.Length == 0 && folders.Length == 0)
         {
            Directory.Delete(f);
         }
      }

      static void ExtractTextures(string f, int game)
      {
         string[] folders = Directory.GetDirectories(f);
         foreach (string folder in folders)
         {
            // avoid infinite loop
            if ((File.GetAttributes(folder) & FileAttributes.ReparsePoint) == 0)
            {
               ExtractTextures(folder, game);
            }
         }
         string[] files = Directory.GetFiles(f);
         Regex reTri = new Regex(@"\.tri$");
         foreach (string filename in files)
         {
            if (!reTri.IsMatch(filename))
            {
               continue;
            }
            // now extract the textures in there
            RunProcess("maketri", ".", String.Format("-x {1} {0}", filename, game == 3 ? "-3" : ""));
         }
      }

      enum EMode
      {
         None,
         Extract,
         Delete,
         Information,
         Stats
      };

      static void GatherInformationFromTriFile(string basicTriName, int game)
      {
         // now extract the textures in there and checksum them
         string triContents = RunProcess("maketri", ".", String.Format("-x {1} -d -i {0}", basicTriName, game == 3 ? "-3" : ""));
         StringReader triContentsReader = new StringReader(triContents);
         // output of maketri is [i/N] Tex <path>\<name>.tga
         Regex getTexName = new Regex(@"^\[[0-9]+/[0-9]+\]\sTex\s([\w/\.\\]+\.tga)");
         string[] separators = new string[] { " " };
         do
         {
            string triContentsItem = triContentsReader.ReadLine();
            if (triContentsItem == null) break;
            string[] tokens = triContentsItem.Split(separators, StringSplitOptions.RemoveEmptyEntries);
            if (tokens.Length > 2)
            {
               string texCurPath = tokens[2];
               texCurPath = texCurPath.Replace("/", @"\");
               if (!File.Exists(texCurPath)) continue;
               string strippedTexName = Path.GetFileNameWithoutExtension(texCurPath);
               string md5 = GetMD5HashFromFile(texCurPath);
               bool isMd5Dup = false;
               bool isNewPath = false;
               if (texDicByHash.ContainsKey(md5))
               {
                  isMd5Dup = true;
                  if (texDicByHash[md5] != texCurPath)
                  {
                     isNewPath = true;
                  }
               }
               else
               {
                  texDicByHash[md5] = texCurPath;
               }
               if (!texDic.ContainsKey(strippedTexName))
               {
                  texDic[strippedTexName] = md5;
               }
               else
               {
                  if (md5 != texDic[strippedTexName])
                  {
                     if (dupTex.ContainsKey(strippedTexName))
                     {
                        ++dupTex[strippedTexName];
                     }
                     else
                     {
                        dupTex[strippedTexName] = 1;
                     }
                  }
               }
               if (isMd5Dup && isNewPath)
               {
                  File.Delete(texCurPath);
               }
            }
         }
         while (true);
      }

      static void GetTextureInformation(string considerThisFolder, int game, EMode mode)
      {
         string[] folders = Directory.GetDirectories(considerThisFolder);
         foreach (string folder in folders)
         {
            // avoid infinite loop
            if ((File.GetAttributes(folder) & FileAttributes.ReparsePoint) == 0)
            {
               GetTextureInformation(folder, game, mode);
            }
         }
         string[] files = Directory.GetFiles(considerThisFolder);
         Regex reQar = new Regex(@"\.qar$");
         // parse lines of the form "ID %x size %d : %s.tri"
         Regex reScan = new Regex(@"^ID\s([0-9a-fA-F]+)\ssize\s([0-9]+)\s:\s(\w+\.tri)$");
         foreach (string filename in files)
         {
            if (!reQar.IsMatch(filename))
            {
               continue;
            }
            // Start the child process.
            string qarContents = RunProcess("qar", ".", String.Format("tv {0}", filename));
            StringReader qarContentsReader = new StringReader(qarContents);
            do
            {
               string qarContentsItem = qarContentsReader.ReadLine();
               if (qarContentsItem == null) break;
               Match matches = reScan.Match(qarContentsItem);

               // first group is the whole line as matched
               if (matches.Groups.Count != 4) continue;
               int id, size;
               // parse hex number
               if (!Int32.TryParse(matches.Groups[1].Captures[0].ToString(), System.Globalization.NumberStyles.HexNumber, null, out id)) continue;
               if (!Int32.TryParse(matches.Groups[2].Captures[0].ToString(), out size)) continue;
               string basicTriName = matches.Groups[3].Captures[0].ToString();
               string triname = basicTriName;
               if (trinames.ContainsKey(triname))
               {
                  TriData value = trinames[triname];
                  if (value.id != id || value.size != size)
                  {
                     if (!commonNames.ContainsKey(triname))
                     {
                        commonNames[triname] = (value.size != size ? 2 : 1);
                     }
                     triname += String.Format("{0:x}", id);
                     trinames[triname] = new TriData(id, size);
                  }
               }
               else
               {
                  trinames[triname] = new TriData(id, size);
               }
               // first extract the tri from the qar
               RunProcess("qar", ".", String.Format("x {0} {1}", filename, basicTriName));

               if (mode == EMode.Information)
               {
                  GatherInformationFromTriFile(basicTriName, game);

                  System.Console.WriteLine("Checked {0} : {1}", filename, basicTriName);
               }
               else if (mode == EMode.Stats)
               {
                  GatherTextureStatsFromTRIFile(basicTriName, game);
               }

               File.Delete(basicTriName);
            }
            while (true);
         }
      }

      public struct GSREG
      {
         public GSREG(ulong _r, ulong _d)
         {
            reg = _r;
            data = _d;
         }
         public ulong reg;
         public ulong data;
      };

      static List<GSREG> gsregs = new List<GSREG>();
      static Dictionary<UInt64, int> dicToAvoidDuplicates = new Dictionary<UInt64, int>();
      static int numTextures = 0;

      static void GatherTextureStatsFromTRIFile(string basicTriName, int game)
      {
         // Start the child process.
         string triStats = RunProcess("maketri", ".", String.Format("-q {1} -i {0}", basicTriName, game == 3 ? "-3" : ""));

         // parse lines of the form "Reg: %d Data: %d"

         StringReader statsReader = new StringReader(triStats);
         do
         {
            string statsItem = statsReader.ReadLine();
            if (statsItem != null)
            {
               string[] counts = statsItem.Split(' ');
               if (counts.Length == 6)
               {
                  uint tex = 0;
                  ulong reg = 0, data = 0;
                  if (UInt32.TryParse(counts[1], System.Globalization.NumberStyles.HexNumber, null, out tex)
                     && UInt64.TryParse(counts[3], out reg)
                     && UInt64.TryParse(counts[5], System.Globalization.NumberStyles.HexNumber, null, out data))
                  {
                     UInt64 hash = (reg << 32) | tex;
                     if (!dicToAvoidDuplicates.ContainsKey(hash))
                     {
                        dicToAvoidDuplicates.Add(hash, 1);
                        gsregs.Add(new GSREG(reg, data));
                     }
                     if (reg == SCE_GS_CLAMP_1) ++numTextures;
                  }
               }
            }
            else
            {
               break;
            }
         }
         while (true);
      }

      static void GatherTextureStatsFromTRIFiles(string considerThisFolder, int game)
      {
         string[] folders = Directory.GetDirectories(considerThisFolder);
         foreach (string folder in folders)
         {
            // avoid infinite loop
            if ((File.GetAttributes(folder) & FileAttributes.ReparsePoint) == 0)
            {
               GatherTextureStatsFromTRIFiles(folder, game);
            }
         }
         string[] files = Directory.GetFiles(considerThisFolder);
         Regex reTri = new Regex(@"\.tri$");
         foreach (string filename in files)
         {
            if (!reTri.IsMatch(filename))
            {
               continue;
            }
            GatherTextureStatsFromTRIFile(filename, game);
         }
      }

      static void GatherInformationFromTRIFiles(string considerThisFolder, int game)
      {
         string[] folders = Directory.GetDirectories(considerThisFolder);
         foreach (string folder in folders)
         {
            // avoid infinite loop
            if ((File.GetAttributes(folder) & FileAttributes.ReparsePoint) == 0)
            {
               GatherInformationFromTRIFiles(folder, game);
            }
         }
         string[] files = Directory.GetFiles(considerThisFolder);
         Regex reTri = new Regex(@"\.tri$");
         foreach (string filename in files)
         {
            if (!reTri.IsMatch(filename))
            {
               continue;
            }
            GatherInformationFromTriFile(filename, game);
         }
      }

      static void Usage()
      {
         System.Console.WriteLine("Texture-Row-Image files utility tool");
         System.Console.WriteLine("Usage: tri_tool [option]+ <path>");
         System.Console.WriteLine("Options:");
         System.Console.WriteLine("    -? ... display this help text");
         System.Console.WriteLine("    -3 ... interpret data as MGS3 (default is MGS2)");
         System.Console.WriteLine("    -t ... extract all textures recursively in the given path");
         System.Console.WriteLine("    -d ... remove duplicates and empty folders from the given path");
         System.Console.WriteLine("    -i ... get information about all textures in qar files in the given path");
         System.Console.WriteLine("    -q ... get detailed stats textures in tri files in the given path");
      }

      // these are the only registers referenced in the tri files
      public const int SCE_GS_TEX0_1 = 0x06;
      public const int SCE_GS_TEX1_1 = 0x14;
      public const int SCE_GS_TEX2_1 = 0x16;
      public const int SCE_GS_CLAMP_1 = 0x08;
      public const int SCE_GS_ALPHA_1 = 0x42;

      static void Main(string[] args)
      {
         if (args.Length == 0)
         {
            Usage();
            System.Console.WriteLine("[No options specified.]");
            return;
         }
         int game = 2;
         string path = null;
         EMode mode = EMode.None;

         foreach (string arg in args)
         {
            if (arg.StartsWith("-3"))
            {
               System.Console.WriteLine("[MGS3 data format specified.]");
               game = 3;
            }
            else if (arg.StartsWith("-h") || arg.StartsWith("--help") || arg.StartsWith("-?") || arg.StartsWith("/?"))
            {
               Usage();
               System.Console.WriteLine("[Help text requested.]");
               return;
            }
            else if (arg.StartsWith("-t") || arg.StartsWith("-d") || arg.StartsWith("-i") || arg.StartsWith("-q") || arg.StartsWith("-c"))
            {
               if (mode != EMode.None)
               {
                  Usage();
                  System.Console.WriteLine("[Conflicting options given.]", arg);
                  return;
               }
               switch (arg[1])
               {
                  case 't': mode = EMode.Extract; break;
                  case 'd': mode = EMode.Delete; break;
                  case 'i': mode = EMode.Information; break;
                  case 'q': mode = EMode.Stats; break;
               }
            }
            else if (Directory.Exists(arg))
            {
               if (path != null)
               {
                  Usage();
                  System.Console.WriteLine("[Only one path can be specified. Sorry!]");
               }
               path = arg;
               path = path.Replace("/", "\\");
            }
            else
            {
               Usage();
               System.Console.WriteLine("[Unrecognized parameter {0}.]", arg);
               return;
            }
         }
         if (mode == EMode.None)
         {
            Usage();
            System.Console.WriteLine("[No mode option given.]");
            return;
         }
         if (path == null)
         {
            Usage();
            System.Console.WriteLine("[No path given.]");
            return;
         }
         switch (mode)
         {
         case EMode.Extract:
            ExtractTextures(path, game);
            break;
         case EMode.Delete:
            RemoveDuplicateTextures(path, true, game);
            System.Console.WriteLine("Unique textures = {0}, missed {1}", texDicByHash.Count, numMissed);
            break;
         case EMode.Information:
            if (game == 2)
            {
               GetTextureInformation(path, game, mode);
            }
            else
            {
               GatherInformationFromTRIFiles(path, game);
            }
            foreach (KeyValuePair<string, int> common in commonNames)
            {
               System.Console.WriteLine("Common: {0} {1}", common.Key, common.Value);
            }
            System.Console.WriteLine("Num common names: {0}", commonNames.Count);
            System.Console.WriteLine("Num unique textures: {0}", texDic.Count);
            System.Console.WriteLine("Num dup texture names: {0}", dupTex.Count);
            System.Console.WriteLine("Num unique textures (by hash): {0}", texDicByHash.Count);
            System.Console.WriteLine("Num unique files: {0}", trinames.Count);
            break;
         case EMode.Stats:
            //ReadRegisterNames("gsreg.txt");
            if (game == 2)
            {
               GetTextureInformation(path, game, mode);
            }
            else
            {
               GatherTextureStatsFromTRIFiles(path, game);
            }
            {
               int[] tex_0_psm = new int[64]; // pixel storage format
               int[] tex_0_tcc = new int[2];  // texture color component
               int[] tex_0_tfx = new int[4];  // texture function
               int[] tex_0_cpsm = new int[16];// clut pixel storage format
               int[] tex_0_csm = new int[2];  // clut storage mode
               int[] tex_0_cld = new int[8];  // clut buffer load control

               int[] tex_1_lcm = new int[2];  // LOD calculation method
               int[] tex_1_mmag = new int[2]; // filter when texture is expanded
               int[] tex_1_mmin = new int[8]; // filter when texture is reduced

               int[] clamp_1_wms = new int[4];// horizontal (S) wrap mode
               int[] clamp_1_wmt = new int[4];// vertical (T) wrap mode

               int[] alpha_1_abcd = new int[256]; // blending function ((A-B)*C)>>7 + D

               // for each register, break down which properties are used by looking at the data fields
               for (int i = 0; i < gsregs.Count; ++i)
               {
                  // these are the only used registers right now
                  switch (gsregs[i].reg)
                  {
                     case SCE_GS_TEX0_1:
                        ++tex_0_psm[(gsregs[i].data >> 20) & 63];
                        ++tex_0_tcc[(gsregs[i].data >> 34) & 1];
                        ++tex_0_tfx[(gsregs[i].data >> 35) & 3];
                        ++tex_0_csm[(gsregs[i].data >> 55) & 1];
                        ++tex_0_cpsm[(gsregs[i].data >> 51) & 15];
                        ++tex_0_cld[(gsregs[i].data >> 61) & 7];
                        break;
                     case SCE_GS_TEX1_1:
                        ++tex_1_lcm[(gsregs[i].data) & 1];
                        ++tex_1_mmag[(gsregs[i].data >> 5) & 1];
                        ++tex_1_mmin[(gsregs[i].data >> 6) & 7];
                        break;
                     case SCE_GS_TEX2_1:
                        ++tex_0_psm[(gsregs[i].data >> 20) & 63];
                        ++tex_0_csm[(gsregs[i].data >> 55) & 1];
                        ++tex_0_cpsm[(gsregs[i].data >> 51) & 15];
                        ++tex_0_cld[(gsregs[i].data >> 61) & 7];
                        break;
                     case SCE_GS_ALPHA_1:
                        ++alpha_1_abcd[gsregs[i].data & 255];
                        break;
                     case SCE_GS_CLAMP_1:
                        ++clamp_1_wms[(gsregs[i].data) & 3];
                        ++clamp_1_wmt[(gsregs[i].data >> 2) & 3];
                        break;
                  }
               }
               System.Console.WriteLine("Number of textures: {0}", numTextures);
               // output the arrays
               System.Console.WriteLine();
               System.Console.WriteLine("TEX_1:");
               System.Console.WriteLine("------");
               System.Console.WriteLine("PSM: Texture Pixel Storage Format");
               int[] psmval = new int[] { 0, 1, 2, 10, 19, 20, 27, 36, 44, 48, 49, 50, 58 };
               string[] psmstr = new string[] { "CT32", "CT24", "CT16", "CT16S", "T8", "T4", "T8H", "T4HL", "T4HH", "Z32", "Z24", "Z16", "Z16S" };
               for (int i = 0; i < psmval.Length; ++i)
               {
                  if (tex_0_psm[psmval[i]] > 0)
                  {
                     System.Console.WriteLine("{0} {1}", psmval[i], psmstr[i]);
                  }
               }
               System.Console.WriteLine("TCC: Texture Color Component");
               if (tex_0_tcc[0] > 0) System.Console.WriteLine("0 RGB");
               if (tex_0_tcc[1] > 0) System.Console.WriteLine("1 RGBA (TEXA register value is At in RGB24/RGBA16)");
               System.Console.WriteLine("TFX: Texture Function");
               if (tex_0_tfx[0] > 0) System.Console.WriteLine("00 MODULATE");
               if (tex_0_tfx[1] > 0) System.Console.WriteLine("01 DECAL");
               if (tex_0_tfx[2] > 0) System.Console.WriteLine("10 HIGHLIGHT");
               if (tex_0_tfx[3] > 0) System.Console.WriteLine("11 HIGHLIGHT2");
               System.Console.WriteLine("CPSM: CLUT Pixel Storage Format");
               if (tex_0_cpsm[0] > 0) System.Console.WriteLine("0000 PSMCT32");
               if (tex_0_cpsm[2] > 0) System.Console.WriteLine("0010 PSMCT16");
               if (tex_0_cpsm[10] > 0) System.Console.WriteLine("1010 PSMCT16S");
               if (false)
               {
                  System.Console.WriteLine("CSM: CLUT Storage mode");
                  if (tex_0_csm[0] > 0) System.Console.WriteLine("0 CSM1");
                  if (tex_0_csm[1] > 0) System.Console.WriteLine("1 CSM2");
                  System.Console.WriteLine("CLD: CLUT Buffer Load Control");
                  if (tex_0_cld[0] > 0) System.Console.WriteLine("000 Temp buffer contents not changed.");
                  if (tex_0_cld[1] > 0) System.Console.WriteLine("001 Load is performed to CSA pos of buffer.");
                  if (tex_0_cld[2] > 0) System.Console.WriteLine("010 Load is to CSA pos of buffer and CBP is copied to CBP0.");
                  if (tex_0_cld[3] > 0) System.Console.WriteLine("011 Load is to CSA pos of buffer and CBP is copied to CBP1.");
                  if (tex_0_cld[4] > 0) System.Console.WriteLine("100 If CBP0 != CBP, load is performed and CBP is copied to CBP0.");
                  if (tex_0_cld[5] > 0) System.Console.WriteLine("101 If CBP1 != CBP, load is performed and CBP is copied to CBP1.");
               }
               System.Console.WriteLine("LCM: LOD Calculation Method");
               if (tex_1_lcm[0] > 0) System.Console.WriteLine("0 Due to the formula LOD = (log2(1/|Q|)<<L) + K");
               if (tex_1_lcm[1] > 0) System.Console.WriteLine("1 Fixed value LOD = K");
               System.Console.WriteLine("MMAG: Filter when Texture is Expanded (LOD < 0)");
               if (tex_1_mmag[0] > 0) System.Console.WriteLine("0 NEAREST");
               if (tex_1_mmag[1] > 0) System.Console.WriteLine("1 LINEAR");
               System.Console.WriteLine("MMIN: Filter when Texture is Reduced (LOD >= 0)");
               if (tex_1_mmin[0] > 0) System.Console.WriteLine("000 NEAREST");
               if (tex_1_mmin[1] > 0) System.Console.WriteLine("001 LINEAR");
               if (tex_1_mmin[2] > 0) System.Console.WriteLine("010 NEAREST_MIPMAP_NEAREST");
               if (tex_1_mmin[3] > 0) System.Console.WriteLine("011 NEAREST_MIPMAP_LINEAR");
               if (tex_1_mmin[4] > 0) System.Console.WriteLine("100 LINEAR_MIPMAP_NEAREST");
               if (tex_1_mmin[5] > 0) System.Console.WriteLine("101 LINEAR_MIPMAP_LINEAR");
               System.Console.WriteLine();
               System.Console.WriteLine("CLAMP_1:");
               System.Console.WriteLine("--------");
               System.Console.WriteLine("WMS: Wrap Mode in Horizontal (S) Direction");
               if (clamp_1_wms[0] > 0) System.Console.WriteLine("[{0}] 00 REPEAT", clamp_1_wms[0]);
               if (clamp_1_wms[1] > 0) System.Console.WriteLine("[{0}] 01 CLAMP", clamp_1_wms[1]);
               if (clamp_1_wms[2] > 0) System.Console.WriteLine("[{0}] 10 REGION_CLAMP", clamp_1_wms[2]);
               if (clamp_1_wms[3] > 0) System.Console.WriteLine("[{0}] 11 REGION_REPEAT", clamp_1_wms[3]);
               System.Console.WriteLine("WMT: Wrap Mode in Vertical (T) Direction");
               if (clamp_1_wmt[0] > 0) System.Console.WriteLine("[{0}] 00 REPEAT", clamp_1_wmt[0]);
               if (clamp_1_wmt[1] > 0) System.Console.WriteLine("[{0}] 01 CLAMP", clamp_1_wmt[1]);
               if (clamp_1_wmt[2] > 0) System.Console.WriteLine("[{0}] 10 REGION_CLAMP", clamp_1_wmt[2]);
               if (clamp_1_wmt[3] > 0) System.Console.WriteLine("[{0}] 11 REGION_REPEAT", clamp_1_wmt[3]);
               System.Console.WriteLine();
               System.Console.WriteLine("ALPHA_1:");
               System.Console.WriteLine("--------");
               System.Console.WriteLine("ABCD: blending function parameters");
               string[] cstr = new string[] { "Cs", "Cd", "0", "Reserved" };
               string[] astr = new string[] { "As", "Ad", "FIX", "Reserved" };
               for (int i = 0; i < 256; ++i)
               {
                  if (alpha_1_abcd[i] > 0)
                  {
                     int a = i & 3;
                     int b = (i >> 2) & 3;
                     int c = (i >> 4) & 3;
                     int d = (i >> 6) & 3;
                     System.Console.WriteLine("[{4}] Cv = (({0} - {1})*{2})>>7 + {3}", cstr[a], cstr[b], astr[c], cstr[d], alpha_1_abcd[i]);
                  }
               }
            }
            System.Console.WriteLine();
            break;
         }
      }
   }
}
