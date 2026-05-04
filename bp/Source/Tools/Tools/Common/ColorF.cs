using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;

namespace Tools.Common
{
   public class ColorF
   {
      /// <summary>
      /// Red
      /// </summary>
      public float R;
      /// <summary>
      /// Green
      /// </summary>
      public float G;
      /// <summary>
      /// Blue
      /// </summary>
      public float B;
      /// <summary>
      /// Alpha
      /// </summary>
      public float A;
      /// <summary>
      /// Intensity
      /// </summary>
      public float I;

      public ColorF()
      {
         R = 0;
         G = 0;
         B = 0;
         A = 0;
      }

      public ColorF Clone()
      {
         return ColorF.FromArgbi(A, R, G, B, I);
      }

      public static ColorF FromString(string value)
      {
         float[] comps = new float[] { 0, 0, 0, 1.0f, 1.0f };

         try
         {
            string[] numbers = value.Split(new char[1] { ' ' });

            for (int i = 0; i < numbers.Length; ++i)
               comps[i] = float.Parse(numbers[i]);

            return ColorF.FromArgbi(comps[3], comps[0], comps[1], comps[2], comps[4]);
         }
         catch (Exception)
         {
            return ColorF.Black;
         }
      }

      public static ColorF FromArgbi(float a, float r, float g, float b, float i)
      {
         ColorF temp = new ColorF();
         temp.R = r;
         temp.G = g;
         temp.B = b;
         temp.A = a;
         temp.I = i;

         return temp;
      }

      public static ColorF FromArgbi(float a, ColorF baseColor)
      {
         return FromArgbi(a, baseColor.R, baseColor.G, baseColor.B, baseColor.I);
      }

      static public ColorF Black
      {
         get { return FromArgbi(1, 0, 0, 0, 1); }
      }

      public Color ToColor()
      {
         int a = Tools.Common.Misc.Clamp((int)(A * 255), 0, 255);
         int r = Tools.Common.Misc.Clamp((int)(I * R * 255), 0, 255);
         int g = Tools.Common.Misc.Clamp((int)(I * G * 255), 0, 255);
         int b = Tools.Common.Misc.Clamp((int)(I * B * 255), 0, 255);

         return Color.FromArgb(a, r, g, b);
      }

      public void NormalizeIntensity()
      {
         R *= I;
         G *= I;
         B *= I;
         I = 1.0f;
      }
   }
}
