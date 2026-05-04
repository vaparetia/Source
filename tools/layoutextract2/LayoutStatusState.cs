using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Xml;
using System.Windows.Forms;
using System.Drawing;

namespace LayoutExtract2
{
   public class LayoutStatusState
   {
      public static Dictionary<String, Paloma.TargaImage> mImageCache = new Dictionary<String, Paloma.TargaImage>();

      LayoutData mLayoutData;
      public UInt32 mCode;
      public List<LayoutStatusCmd> mCommands;

      public void ToXML(XmlElement statusElement)
      {
         XmlElement stateElement = statusElement.OwnerDocument.CreateElement("State");
         statusElement.AppendChild(stateElement);
         stateElement.SetAttribute("code", mCode.ToString("x8"));

         foreach (LayoutStatusCmd cmd in mCommands)
         {
            cmd.ToXML(stateElement);
         }
      }

      public LayoutStatusState(XmlElement element, LayoutData ld)
      {
         mLayoutData = ld;
         mCode = UInt32.Parse(element.GetAttribute("code"), System.Globalization.NumberStyles.HexNumber);
         mCommands = new List<LayoutStatusCmd>();
         foreach (XmlElement child in element.ChildNodes)
         {
            mCommands.Add(new LayoutStatusCmd(child, ld));
         }
      }

      public LayoutStatusState(BinaryReader br, LayoutData ld, UInt32 tex_strCode)
      {
         mLayoutData = ld;
         LayoutUtil.LogStreamLoad(br, "LayoutStatusState");

         mCode = br.ReadUInt32(); // this is the code corresponding to the action
         mCommands = new List<LayoutStatusCmd>();

         while (true)
         {
            LayoutStatusCmd newCmd = new LayoutStatusCmd(br, ld, tex_strCode);
            mCommands.Add(newCmd);
            if (newCmd.GetCommand() == LayoutStatusCmd.EStatCmd.END)
            {
               break;
            }
         }
      }
      public void Write(BinaryWriter bw)
      {
         bw.Write(mCode);
         foreach (LayoutStatusCmd cmd in mCommands)
         {
            cmd.Write(bw);
         }
      }
      public void PopulateTreeView(TreeNode node, PointF scale)
      {
         foreach (LayoutStatusCmd cmd in mCommands)
         {
            cmd.PopulateTreeView(node, scale);
         }
      }
      public RectangleF GetBounds(PointF inOffset)
      {
         RectangleF bounds = GetLocalBounds();
         bounds.X += inOffset.X;
         bounds.Y += inOffset.Y;
         return bounds;
      }
      public RectangleF GetLocalBounds()
      {
         RectangleF rect = RectangleF.Empty;
         SizeF size = SizeF.Empty;
         PointF center = PointF.Empty;

         Boolean hasBeenSet = false;
         foreach (LayoutStatusCmd cmd in mCommands)
         {
            switch (cmd.GetCommand())
            {
            case LayoutStatusCmd.EStatCmd.VERTEX:
               foreach (LayoutStatusCmd.Vertex vertex in cmd.vertexes)
               {
                  if (!hasBeenSet)
                     rect = new RectangleF(vertex.x, vertex.y, 0, 0);
                  else
                     rect = RectangleF.Union(rect, new RectangleF(vertex.x, vertex.y, 0, 0));
                  hasBeenSet = true;
               }
               break;

            case LayoutStatusCmd.EStatCmd.SIZE:
               size = new SizeF(cmd.size_w, cmd.size_h);
               break;

            case LayoutStatusCmd.EStatCmd.CENTER:
               center = new PointF(cmd.center_x, cmd.center_y);
               break;

            }
         }

         if (!rect.IsEmpty)
         {
            if (size != SizeF.Empty)
            {
               rect.Size = size;
            }
         }
         else
         {
            if (size != SizeF.Empty && center != PointF.Empty)
            {
               rect = new RectangleF(center.X - size.Width / 2, center.Y - size.Height / 2, size.Width, size.Height);
            }
            else if (size != SizeF.Empty)
            {
               rect.Size = size;
            }
            else if (center != PointF.Empty)
            {
               rect.Location = center;
            }
         }

         return rect;
      }
      public Color GetAverageColor()
      {
         Color color = Color.White;
         foreach (LayoutStatusCmd cmd in mCommands)
         {
            if (cmd.GetCommand() == LayoutStatusCmd.EStatCmd.RGBA)
            {
               Double[] dColor = new Double[] { 0, 0, 0, 0 };

               foreach (LayoutStatusCmd.Vertex vertex in cmd.vertexes)
               {
                  dColor[0] += vertex.r;
                  dColor[1] += vertex.g;
                  dColor[2] += vertex.b;
                  dColor[3] += vertex.a;
               }
               Double count = (Double)cmd.vertexes.Length;
               color = Color.FromArgb(
                  (Int32)(dColor[3] / count),
                  (Int32)(dColor[0] / count),
                  (Int32)(dColor[1] / count),
                  (Int32)(dColor[2] / count)
                  );
            }
         }

         return color;
      }
      public List<PointF> GetVertices(PointF offset)
      {
         List<PointF> points = new List<PointF>();

         foreach (LayoutStatusCmd cmd in mCommands)
         {
            if (cmd.GetCommand() == LayoutStatusCmd.EStatCmd.VERTEX)
            {
               foreach (LayoutStatusCmd.Vertex vertex in cmd.vertexes)
               {
                  points.Add(new PointF(offset.X + vertex.x, offset.Y + vertex.y));
               }
            }
         }

         return points;
      }

      public Paloma.TargaImage GetTexture()
      {
         String tgaName = GetTextureName();
         if (String.IsNullOrEmpty(tgaName))
            return null;

         if (mImageCache.ContainsKey(tgaName))
            return mImageCache[tgaName];

         try
         {
            Paloma.TargaImage tga = new Paloma.TargaImage(tgaName);
            mImageCache[tgaName] = tga;
            return tga;
         }
         catch (System.Exception)
         {
            return null;
         }
      }

      public String GetTextureName()
      {
         foreach (LayoutStatusCmd cmd in mCommands)
         {
            if (cmd.GetCommand() == LayoutStatusCmd.EStatCmd.TEX)
            {
               string repository = System.Environment.GetEnvironmentVariable("BPE_REPOSITORY");
               return String.Format("{0}\\textures\\flatlist\\{1:x8}.tga", repository, cmd.tex_code);
            }
         }
         return "";
      }

      public RectangleF GetUVCoordinates()
      {
         RectangleF uvCoords = new RectangleF(0, 0, 1, 1);

         foreach (LayoutStatusCmd cmd in mCommands)
         {
            switch (cmd.GetCommand())
            {
               case LayoutStatusCmd.EStatCmd.UV:
               case LayoutStatusCmd.EStatCmd.PARTUV:
                  uvCoords.Location = new PointF(cmd.pos_u, cmd.pos_v);
                  break;
               case LayoutStatusCmd.EStatCmd.UVSIZE:
               case LayoutStatusCmd.EStatCmd.PTXSIZ:
                  uvCoords.Size = new SizeF(cmd.size_u, cmd.size_v);
                  break;

            }
         }

         return uvCoords;
      }

      public bool GetVisible()
      {
         foreach (LayoutStatusCmd cmd in mCommands)
         {
            switch (cmd.GetCommand())
            {
               case LayoutStatusCmd.EStatCmd.DISP:
                  return cmd.disp;
            }
         }
         return false;
      }
   }


   public class LayoutStatusCmd
   {
      public enum EStatCmd
      {
         END    ,
         DISP   ,
         TEX    ,
         VERTEX ,
         RGBA   ,
         SIZE   ,
         UV     ,
         UVSIZE ,
         ALPHA  ,
         PRI    ,
         CENTER ,
         ANGLE  ,
         MAGNI  ,
         PARTUV ,
         PTXSIZ ,
      };
      public static UInt32 kStatCmd_MASK = 0xf0000000;

      public class Vertex
      {
         public Single x;
         public Single y;               /* 座標値                         */
         public Byte r;
         public Byte g;
         public Byte b;
         public Byte a; /* RGBA値                         */
         public Byte vert; //:1;
         public Byte rgba; //:1;    /* 設定されているかどうかのフラグ */
      }


      public UInt32 cmd;

      public UInt32 tex_code;       /* テクスチャの StrCode                   */
      public UInt32 tex_handle;     /* テクスチャのハンドル                   */
      public Single pos_u;
      public Single pos_v;  /* テクスチャの UV 位置(正規化)           */
      public Single size_u;
      public Single size_v; /* テクスチャのテクセルサイズ比           */

      /* 頂点情報 */
      public Vertex[] vertexes;
      //l2dVertex     * vertex;         /* 頂点座標、頂点RGBA値                   */

      /* ※頂点情報の配列の大きさは、対応するオブジェクトの
          操作可能な頂点数と同じ。 */

      /* 基本表示情報 */
      public Single size_w;
      public Single size_h; /* 表示サイズ(Sprite の場合のみ使用)      */
      public UInt32 pri_adj;        /* プライオリティ補正値                   */

      public UInt64 alpha;          /* アルファがある場合の、ALPHA レジスタ値 */

      public Single magni;          /* 下位オブジェクトの座標値係数           */

      public Single center_x;
      public Single center_y;
      public Single angle;

      public Byte    alpha_flg; //:1;    /* 0:アルファなし / 1: アルファあり       */

      public bool disp          /* 0:非表示       / 1:表示                */
      {
         get
         {
            return (cmd & 1u) > 0;
         }
         set
         {
            if (value)
            {
               cmd |= 1u;
            }
            else
            {
               cmd &= ~1u;
            }
         }
      }

      /* 水平反転フラグ */
      public bool h_rev
      {
         get
         {
            return (cmd & 2u) > 0;
         }
         set
         {
            if (value)
            {
               cmd |= 2u;
            }
            else
            {
               cmd &= ~2u;
            }
         }
      }

      /* 垂直反転フラグ */
      public bool v_rev
      {
         get
         {
            return (cmd & 4u) > 0;
         }
         set
         {
            if (value)
            {
               cmd |= 4u;
            }
            else
            {
               cmd &= ~4u;
            }
         }
      }

      // draw elements pixel perfect when stationary (not while morphing though)
      public bool pixel_perfect
      {
         get
         {
            return (cmd & 8u) > 0;
         }
         set
         {
            if (value)
            {
               cmd |= 8u;
            }
            else
            {
               cmd &= ~8u;
            }
         }
      }

      LayoutData mLayoutData;

      public LayoutStatusCmd(BinaryReader br, LayoutData ld, UInt32 tex_strCode)
      {
         mLayoutData = ld;

         tex_code = 0xffffffff;
         tex_handle = 0xffffffff;
         alpha_flg = 0;
         alpha = 0;
         pos_u = -1.0F;
         pos_v = -1.0F;
         size_u = -1.0F;
         size_v = -1.0F;
         magni = 1.0F;
         pri_adj = 0;

         cmd = br.ReadUInt32();
         switch (GetCommand())
         {
         case EStatCmd.DISP:
            break;

         case EStatCmd.TEX:
            {
               tex_code = (cmd & 0x0ffffff);
               tex_handle = tex_strCode;

               if (pos_u < 0.0F)
                  pos_u = pos_v = 0.0F;
               if (size_u < 0.0F)
                  size_u = size_v = 1.0F;
            }
            break;

         case EStatCmd.VERTEX:
            {
               Int32 begin, num, k;
               UInt16 x, y;
               Single fx, fy;

               begin = (Int32)(cmd & 0xff);
               num   = (Int32)((cmd >> 8) & 0xff);
               vertexes = new Vertex[begin + num];
               for(k = 0; k < num; k++)
               {
                  if (ld.version < LayoutData.OLA_VERSION_1_3)
                  {
                     x = br.ReadUInt16();
                     y = br.ReadUInt16();

                     fx = HalfToSingle(x);
                     fy = HalfToSingle(y);
                  }
                  else
                  {
                     fx = br.ReadSingle();
                     fy = br.ReadSingle();
                  }

                  vertexes[begin + k] = new Vertex();
                  vertexes[begin + k].x = fx;
                  vertexes[begin + k].y = fy;
                  vertexes[begin + k].vert = 1;
               }
            }
            break;

         case EStatCmd.RGBA:
            {
               Int32 begin, num, k;
               UInt32 tmp;

               begin = (Int32)(cmd & 0xff);
               num = (Int32)((cmd >> 8) & 0xff);
               vertexes = new Vertex[begin + num];
               for(k = 0; k < num; k++)
               {
                  tmp = br.ReadUInt32();

                  vertexes[begin + k] = new Vertex();
                  vertexes[begin + k].r = (Byte)(tmp        & 0xff);
                  vertexes[begin + k].g = (Byte)((tmp >> 8) & 0xff);
                  vertexes[begin + k].b = (Byte)((tmp >> 16) & 0xff);
                  vertexes[begin + k].a = (Byte)((tmp >> 24) & 0xff);
                  vertexes[begin + k].rgba = 1;
               }
            }
            break;

         case EStatCmd.SIZE:
            {
               if (ld.version < LayoutData.OLA_VERSION_1_3)
               {
                  UInt16 width, height;
                  Single fw, fh;

                  width = br.ReadUInt16();
                  height = br.ReadUInt16();

                  fw = HalfToSingle(width);
                  fh = HalfToSingle(height);

                  size_w = fw;
                  size_h = fh;
               }
               else
               {
                  size_w = br.ReadSingle();
                  size_h = br.ReadSingle();
               }
            }
            break;

         case EStatCmd.UV:
            {
               UInt16 u, v;
               Single fu, fv;

               u = br.ReadUInt16();
               v = br.ReadUInt16();

               fu = HalfToSingle(u);
               fv = HalfToSingle(v);

               pos_u = fu;
               pos_v = fv;
            }
            break;

         case EStatCmd.UVSIZE:
            {
               UInt16 width, height;
               Single fw, fh;

               width = br.ReadUInt16();
               height = br.ReadUInt16();

               fw = HalfToSingle(width);
               fh = HalfToSingle(height);

               size_u = fw;
               size_v = fh;
            }
            break;

         case EStatCmd.ALPHA:
            alpha_flg = 1;
            // the contents of the PS2 ALPHA register
            alpha = ((UInt64)cmd & 0xffff) | (((UInt64)cmd & 0xff0000) << 16);
            break;
         
         case EStatCmd.PRI:
            pri_adj = cmd & 7;
            break;
         
         case EStatCmd.CENTER:
            {
               if (ld.version < LayoutData.OLA_VERSION_1_3)
               {
                  UInt16 scx, scy;

                  scx = br.ReadUInt16();
                  scy = br.ReadUInt16();

                  center_x = HalfToSingle(scx);
                  center_y = HalfToSingle(scy);
               }
               else
               {
                  center_x = br.ReadSingle();
                  center_y = br.ReadSingle();
               }
            }
            break;

         case EStatCmd.ANGLE:
            {
               UInt16 sang;
               sang = (UInt16)(cmd & 0xffff);
               angle = HalfToSingle(sang);
            }
            break;

         case EStatCmd.MAGNI:
            {
               UInt16 _magni;
               _magni = (UInt16)(cmd & 0xffff);
               magni = HalfToSingle(_magni);
            }
            break;

         case EStatCmd.PARTUV:
            {
               UInt32 Vv;

               Vv = br.ReadUInt32();

               pos_u = (Single)((cmd >> 16) & 0x0fff) / (Single)(cmd & 0x0fff);
               pos_v = (Single)((Vv >> 16) & 0x0fff) / (Single)(Vv & 0x0fff);
            }
            break;

         case EStatCmd.PTXSIZ:
            {
               UInt32 Vv;

               Vv = br.ReadUInt32();

               size_u = (Single)((cmd >> 16) & 0x0fff) / (Single)(cmd & 0x0fff);
               size_v = (Single)((Vv >> 16) & 0x0fff) / (Single)(Vv & 0x0fff);
            }
            break;

         case EStatCmd.END:
         default:
            break;
         }
      }

      public static unsafe Single HalfToSingle(UInt16 inHalf)
      {
         const UInt16 E_BIAS = 8; 
         UInt32 s_dat, e_dat, f_dat ;
         Single f ;

         s_dat = (UInt32)(inHalf & 0x8000) << 16;

         e_dat = (UInt32)(inHalf & 0x7c00);
         if ( e_dat != 0 )
         {
            UInt32 bias = (UInt16)(127 - E_BIAS) << 10;
            e_dat += bias;
            e_dat <<= 13 ;
         }

         f_dat = (UInt32)( inHalf & 0x03ff ) << 13 ;

         *(UInt32*)&f = s_dat | e_dat | f_dat ;

         return ( f );
      }
      public static unsafe UInt16 SingleToHalf(Single inSingle)
      {
         const UInt16 E_BIAS = 8; 
         UInt32 mf = *(UInt32*)&inSingle;
         UInt32 s_dat, e_dat, f_dat;

         s_dat = (mf >> 16) & 0x8000;
         f_dat = (mf >> 13) & 0x03ff;

         e_dat = 0;
         if ((mf & 0x7f800000) != 0)
         {
            e_dat = mf & 0x7f800000;
            e_dat = e_dat >> 13;
            UInt32 bias = (UInt16)(127 - E_BIAS) << 10;
            e_dat = e_dat - bias;
            e_dat = e_dat & 0x7c00;
         }

         UInt16 r = (UInt16)(s_dat | e_dat | f_dat);
         return r;
      }

      public EStatCmd GetCommand()
      {
         return (EStatCmd)((cmd & kStatCmd_MASK)>>28);
      }

      public void ToXML(XmlElement statusElement)
      {
         XmlElement statCmdElement = statusElement.OwnerDocument.CreateElement("StatCmd");
         statCmdElement.SetAttribute("command", GetCommand().ToString());
         statusElement.AppendChild(statCmdElement);

         switch (GetCommand())
         {
         case EStatCmd.DISP:
            statCmdElement.SetAttribute("val", disp.ToString());
            break;
         case EStatCmd.TEX:
            // tex_handle (tri code) is 0, which sucks for finding out which tri files to add to!
            statCmdElement.SetAttribute("tex", tex_code.ToString("x8"));
            statCmdElement.InnerText = LayoutUtil.FlatListRemapping.GetRemappedName(tex_code);
            break;
         case EStatCmd.VERTEX:
            {
               UInt32 begin = cmd & 0xff;
               if (begin > 0)
               {
                  statCmdElement.SetAttribute("begin", begin.ToString());
               }

               foreach (Vertex vert in vertexes)
               {
                  XmlElement vertElement = statCmdElement.OwnerDocument.CreateElement("Vert");
                  vertElement.SetAttribute("x", vert.x.ToString());
                  vertElement.SetAttribute("y", vert.y.ToString());
                  statCmdElement.AppendChild(vertElement);
               }
            }
            break;
         case EStatCmd.RGBA:
            foreach (Vertex vert in vertexes)
            {
               XmlElement colElement = statCmdElement.OwnerDocument.CreateElement("RGBA");
               colElement.SetAttribute("r", vert.r.ToString());
               colElement.SetAttribute("g", vert.g.ToString());
               colElement.SetAttribute("b", vert.b.ToString());
               colElement.SetAttribute("a", vert.a.ToString());
               statCmdElement.AppendChild(colElement);
            }
            break;
         case EStatCmd.SIZE:
            statCmdElement.SetAttribute("w", size_w.ToString());
            statCmdElement.SetAttribute("h", size_h.ToString());
            break;
         case EStatCmd.UV:
            statCmdElement.SetAttribute("x", pos_u.ToString());
            statCmdElement.SetAttribute("y", pos_v.ToString());
            break;
         case EStatCmd.UVSIZE:
            statCmdElement.SetAttribute("x", size_u.ToString());
            statCmdElement.SetAttribute("y", size_v.ToString());
            break;
         case EStatCmd.ALPHA:
            statCmdElement.SetAttribute("val", alpha.ToString("x"));
            break;
         case EStatCmd.PRI:
            statCmdElement.SetAttribute("val", pri_adj.ToString());
            break;
         case EStatCmd.CENTER:
            statCmdElement.SetAttribute("x", center_x.ToString());
            statCmdElement.SetAttribute("y", center_y.ToString());
            break;
         case EStatCmd.ANGLE:
            statCmdElement.SetAttribute("val", angle.ToString());
            break;
         case EStatCmd.MAGNI:
            statCmdElement.SetAttribute("val", magni.ToString());
            break;
         case EStatCmd.PARTUV:
            statCmdElement.SetAttribute("x", pos_u.ToString());
            statCmdElement.SetAttribute("y", pos_v.ToString());
            break;
         case EStatCmd.PTXSIZ:
            statCmdElement.SetAttribute("x", size_u.ToString());
            statCmdElement.SetAttribute("y", size_v.ToString());
            break;
         case EStatCmd.END:
            break;
         }
      }

      public LayoutStatusCmd(XmlElement element, LayoutData ld)
      {
         tex_code = 0;
         tex_handle = 0;
         EStatCmd command = (EStatCmd)Enum.Parse(typeof(EStatCmd), element.GetAttribute("command"));
         cmd = ((UInt32)(command)) << 28;

         switch (GetCommand())
         {
            case EStatCmd.DISP:
               disp = element.GetAttribute("val") == "True";
               break;
            case EStatCmd.TEX:
               tex_code = UInt32.Parse(element.GetAttribute("tex"), System.Globalization.NumberStyles.HexNumber);
               cmd |= tex_code & 0x00ffffff;
               break;
            case EStatCmd.VERTEX:
               {
                  UInt32 begin = 0;
                  if (element.HasAttribute("begin"))
                  {
                     begin = UInt32.Parse(element.GetAttribute("begin"));
                  }
                  UInt32 num = (UInt32) element.ChildNodes.Count;
                  if (num > 0)
                  {
                     vertexes = new Vertex[num];
                     for (int i = 0; i < num; ++i)
                     {
                        XmlElement child = (XmlElement)element.ChildNodes[i];
                        vertexes[i] = new Vertex();
                        vertexes[i].x = Single.Parse(child.GetAttribute("x"));
                        vertexes[i].y = Single.Parse(child.GetAttribute("y"));
                     }
                  }
                  cmd |= (num << 8) | (begin);
               }
               break;
            case EStatCmd.RGBA:
               {
                  UInt32 begin = 0;
                  UInt32 num = (UInt32)element.ChildNodes.Count;
                  if (element.HasAttribute("begin"))
                  {
                     begin = UInt32.Parse(element.GetAttribute("begin"));
                  }
                  if (num > 0)
                  {
                     vertexes = new Vertex[num];
                     for (int i = 0; i < num; ++i)
                     {
                        XmlElement child = (XmlElement)element.ChildNodes[i];
                        vertexes[i] = new Vertex();
                        vertexes[i].r = Byte.Parse(child.GetAttribute("r"));
                        vertexes[i].g = Byte.Parse(child.GetAttribute("g"));
                        vertexes[i].b = Byte.Parse(child.GetAttribute("b"));
                        vertexes[i].a = Byte.Parse(child.GetAttribute("a"));
                     }
                  }
                  cmd |= (num << 8) | (begin);
               }
               break;
            case EStatCmd.SIZE:
               size_w = Single.Parse(element.GetAttribute("w"));
               size_h = Single.Parse(element.GetAttribute("h"));
               break;
            case EStatCmd.UV:
            case EStatCmd.PARTUV:
               pos_u = Single.Parse(element.GetAttribute("x"));
               pos_v = Single.Parse(element.GetAttribute("y"));
               break;
            case EStatCmd.UVSIZE:
            case EStatCmd.PTXSIZ:
               size_u = Single.Parse(element.GetAttribute("x"));
               size_v = Single.Parse(element.GetAttribute("y"));
               break;
            case EStatCmd.ALPHA:
               alpha = UInt64.Parse(element.GetAttribute("val"), System.Globalization.NumberStyles.HexNumber);
               cmd |= (UInt32)(((alpha >> 16) & 0xff0000) | (alpha & 0xffff));
               break;
            case EStatCmd.PRI:
               pri_adj = UInt32.Parse(element.GetAttribute("val"));
               cmd |= pri_adj & 7;
               break;
            case EStatCmd.CENTER:
               center_x = Single.Parse(element.GetAttribute("x"));
               center_y = Single.Parse(element.GetAttribute("y"));
               break;
            case EStatCmd.ANGLE:
               angle = Single.Parse(element.GetAttribute("val"));
               cmd |= SingleToHalf(angle);
               break;
            case EStatCmd.MAGNI:
               magni = Single.Parse(element.GetAttribute("val"));
               cmd |= SingleToHalf(magni);
               break;
            case EStatCmd.END:
               break;
         }
      }

      public void Write(BinaryWriter bw)
      {
         UInt32 cmdBit = cmd & kStatCmd_MASK;

         switch (GetCommand())
         {
         case EStatCmd.DISP:
            bw.Write(cmd);
            break;

         case EStatCmd.TEX:
            bw.Write(cmd);
            break;

         case EStatCmd.VERTEX:
            {
               Int32 begin, num;

               begin = (Int32)(cmd & 0xff);
               num = (Int32)((cmd >> 8) & 0xff);

               bw.Write(cmd);
               for (Int32 ii = begin; ii < begin + num; ++ii)
               {
                  bw.Write(vertexes[ii].x);
                  bw.Write(vertexes[ii].y);
               }
            }
            break;

         case EStatCmd.RGBA:
            {
               Int32 begin, num;
               UInt32 tmp;

               begin = (Int32)(cmd & 0xff);
               num = (Int32)((cmd >> 8) & 0xff);

               bw.Write(cmd);
               for (Int32 ii = begin; ii < begin + num; ++ii)
               {
                  tmp = vertexes[ii].r;
                  tmp |= ((UInt32)vertexes[ii].g) << 8;
                  tmp |= ((UInt32)vertexes[ii].b) << 16;
                  tmp |= ((UInt32)vertexes[ii].a) << 24;

                  bw.Write(tmp);
               }
            }
            break;

         case EStatCmd.SIZE:
            {
               bw.Write(cmd);
               bw.Write(size_w);
               bw.Write(size_h);
            }
            break;

         case EStatCmd.UV:
            {
               UInt16 u, v;

               u = SingleToHalf(pos_u);
               v = SingleToHalf(pos_v);

               bw.Write(cmd);
               bw.Write(u);
               bw.Write(v);
            }
            break;

         case EStatCmd.UVSIZE:
            {
               UInt16 u, v;

               u = SingleToHalf(size_u);
               v = SingleToHalf(size_v);

               bw.Write(cmd);
               bw.Write(u);
               bw.Write(v);
            }
            break;

         case EStatCmd.ALPHA:
            bw.Write(cmd);
            break;

         case EStatCmd.PRI:
            bw.Write(cmd);
            break;

         case EStatCmd.CENTER:
            {
               bw.Write(cmd);
               bw.Write(center_x);
               bw.Write(center_y);
            }
            break;

         case EStatCmd.ANGLE:
            bw.Write(cmd);
            break;

         case EStatCmd.MAGNI:
            bw.Write(cmd);
            break;

         case EStatCmd.PARTUV:
            {
               Fraction pos_u_frac = Fraction.ToFraction(pos_u);
               while (pos_u_frac.Numerator > 0xfff || pos_u_frac.Denominator > 0xfff)
               {
                  pos_u_frac.Numerator /= 2;
                  pos_u_frac.Denominator /= 2;
               }
               Fraction pos_v_frac = Fraction.ToFraction(pos_v);
               while (pos_v_frac.Numerator > 0xfff || pos_v_frac.Denominator > 0xfff)
               {
                  pos_v_frac.Numerator /= 2;
                  pos_v_frac.Denominator /= 2;
               }

               UInt32 Uu = ((UInt32)pos_u_frac.Numerator << 16) | (UInt32)pos_u_frac.Denominator;
               UInt32 Vv = ((UInt32)pos_v_frac.Numerator << 16) | (UInt32)pos_v_frac.Denominator;

               bw.Write(cmdBit | Uu);
               bw.Write(Vv);
            }
            break;

         case EStatCmd.PTXSIZ:
            {
               Fraction size_u_frac = Fraction.ToFraction(size_u);
               while (size_u_frac.Numerator > 0xfff || size_u_frac.Denominator > 0xfff)
               {
                  size_u_frac.Numerator /= 2;
                  size_u_frac.Denominator /= 2;
               }
               Fraction size_v_frac = Fraction.ToFraction(size_v);
               while (size_v_frac.Numerator > 0xfff || size_v_frac.Denominator > 0xfff)
               {
                  size_v_frac.Numerator /= 2;
                  size_v_frac.Denominator /= 2;
               }

               UInt32 Uu = ((UInt32)size_u_frac.Numerator << 16) | (UInt32)size_u_frac.Denominator;
               UInt32 Vv = ((UInt32)size_v_frac.Numerator << 16) | (UInt32)size_v_frac.Denominator;

               bw.Write(cmdBit | Uu);
               bw.Write(Vv);
            }
            break;

         case EStatCmd.END:
            bw.Write(cmd);
            break;
         }
      }
      public void PopulateTreeView(TreeNode node, PointF scale)
      {
         switch (GetCommand())
         {
         case EStatCmd.DISP:
            {
               string text = (disp ? "DisplayOn" : "DisplayOff");
               if (h_rev) text += ", HRev";
               if (v_rev) text += ", VRev";
               if (pixel_perfect) text += ", PixelPerfect";
               node.Nodes.Add(String.Format(text));
            }
            break;

         case EStatCmd.TEX:
            node.Nodes.Add(String.Format("Texture {0:x8} {1}", tex_code, LayoutUtil.FlatListRemapping.GetRemappedName(tex_code)));
            break;

         case EStatCmd.VERTEX:
            {
               if (vertexes.Length == 1)
               {
                  Vertex vert = vertexes[0];
                  node.Nodes.Add(String.Format("Vertex {0}, {1}", scale.X * vert.x, scale.Y * vert.y));
               }
               else
               {
                  TreeNode addedNode = node.Nodes.Add(String.Format("Vertex"));
                  foreach (Vertex vert in vertexes)
                  {
                     addedNode.Nodes.Add(String.Format("{0}, {1}", scale.X * vert.x, scale.Y * vert.y));
                  }
               }
            }
            break;

         case EStatCmd.RGBA:
            {
               if (vertexes.Length == 1)
               {
                  Vertex vert = vertexes[0];
                  node.Nodes.Add(String.Format("Color r={0}, g={1}, b={2}, a={3}", vert.r, vert.g, vert.b, vert.a));
               }
               else
               {
                  TreeNode addedNode = node.Nodes.Add(String.Format("Color"));
                  foreach (Vertex vert in vertexes)
                  {
                     addedNode.Nodes.Add(String.Format("r={0}, g={1}, b={2}, a={3}", vert.r, vert.g, vert.b, vert.a));
                  }
               }
            }
            break;

         case EStatCmd.SIZE:
            node.Nodes.Add(String.Format("Size w={0}, h={1}", scale.X*size_w, scale.Y*size_h));
            break;

         case EStatCmd.UV:
            node.Nodes.Add(String.Format("Tex Coords pu={0}, pv={1}", pos_u, pos_v));
            break;

         case EStatCmd.UVSIZE:
            node.Nodes.Add(String.Format("Tex Size su={0}, sv={1}", size_u, size_v));
            break;

         case EStatCmd.ALPHA:
            node.Nodes.Add(String.Format("Alpha {0:x}", alpha));
            break;

         case EStatCmd.PRI:
            node.Nodes.Add(String.Format("Priority {0}", pri_adj));
            pri_adj = cmd & 7;
            break;

         case EStatCmd.CENTER:
            node.Nodes.Add(String.Format("Center x={0}, y={1}", scale.X*center_x, scale.Y*center_y));
            break;

         case EStatCmd.ANGLE:
            node.Nodes.Add(String.Format("Angle {0}", angle));
            break;

         case EStatCmd.MAGNI:
            node.Nodes.Add(String.Format("Magnitude {0}", magni));
            break;

         case EStatCmd.PARTUV:
            node.Nodes.Add(String.Format("Fractional UV pu={0}, pv={1}", pos_u, pos_v));
            break;

         case EStatCmd.PTXSIZ:
            node.Nodes.Add(String.Format("Fractional UV Size su={0}, sv={1}", size_u, size_v));
            break;

         case EStatCmd.END:
            node.Nodes.Add(String.Format("End"));
            break;
         }
      }
   }
}
