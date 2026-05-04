using System;
using System.Text;
using System.IO;

namespace Tools.Common
{
   /// <summary>
   /// Get the FNV-1a Value for a String, Byte array or Stream
   /// </summary>
   public class HashUtils
   {
      readonly static Encoding skEncoder = Encoding.GetEncoding(437);  // Standard US OEM (IBM437 - OEM United States)
      readonly static uint skSeed = 2166136261u;
      readonly static uint skMult = 16777619u;

      /// <summary>
      /// Get the FNV-1a value for an entire Stream, setting the position to zero.
      /// </summary>
      /// <param name="st">The stream to perform the FNV-1a calculation on.</param>
      /// <returns>The FNV-1a value.</returns>
      static public uint CalculateFNV1a(Stream st) 
      {
         return CalculateFNV1a(st, 0, st.Length); 
      } 
      static public uint CalculateFNV1a(Stream st, uint hashVal) 
      {
         return CalculateFNV1a(st, 0, st.Length, hashVal); 
      } 

      /// <summary>
      /// Get the FNV-1a value for a Stream, setting the position to to the beginPosition and calculation to the length.
      /// The position is set back to the original position when complete.
      /// </summary>
      /// <param name="st">The stream to perform the FNV-1a calculation on.</param>
      /// <param name="beginPosition">The begin position of the stream to start the calculation.</param>
      /// <param name="length">The length you wish to run the calculation on.</param>
      /// <returns>The FNV-1a value.</returns>
      static public uint CalculateFNV1a(Stream st, long beginPosition, long length) 
      {
         return CalculateFNV1a(st, beginPosition, length, skSeed);
      }
      static public uint CalculateFNV1a(Stream st, long beginPosition, long length, uint hashVal) 
      {
         long originalPos = st.Position;
         long endPos = beginPosition + length;
         byte by;

         if (endPos > st.Length) 
         {
            throw new Exception("Out of bounds. beginposition + length is greater than the length of the Stream");
         }

         try
         {
            st.Position = beginPosition;
            BufferedStream bs = new BufferedStream(st);
            
            for(long i = beginPosition; i < endPos; i++) 
            { 
               by =  Convert.ToByte(bs.ReadByte());
               hashVal ^= by;
               hashVal *= skMult;
            }
         }
         catch (Exception e)
         {
            Console.WriteLine(e.ToString());
         }
         finally
         {
         //For now do nothing
         }
         st.Position = originalPos;
         
         return hashVal;
      } 

      /// <summary>
      /// Get the FNV-1a value for a String.
      /// </summary>
      /// <param name="text">The string you wish to calculate the FNV-1a value on.</param>
      /// <returns>The FNV-1a value.</returns>
      static public uint CalculateFNV1a(string text) 
      {
         byte[] buffer = skEncoder.GetBytes(text);
         return CalculateFNV1a(buffer); 
      } 
      static public uint CalculateFNV1a(string text, uint hashVal) 
      {
         byte[] buffer = skEncoder.GetBytes(text);
         return CalculateFNV1a(buffer, hashVal); 
      } 

      /// <summary>
      /// Get the FNV-1a value for a byte[].
      /// </summary>
      /// <param name="by">The byte[] you wish to calculate the FNV-1a value on</param>
      /// <returns>The FNV-1a value.</returns>
      static public uint CalculateFNV1a(byte[] by) 
      {
         return CalculateFNV1a(by, skSeed);
      }
      static public uint CalculateFNV1a(byte[] by, uint hashVal) 
      {
         long len = by.Length;
         for(long i = 0; i < len; i++) 
         {
            hashVal ^= by[i];
            hashVal *= skMult;
         }
         return hashVal;
      }
   }
}
