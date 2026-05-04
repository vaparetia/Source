using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace Tools.Common
{
   public class BinaryReaderBigEndian : System.IO.BinaryReader
   {
      public BinaryReaderBigEndian(System.IO.Stream stream)
       : base(stream)
      {
      }
      
      public override Decimal ReadDecimal()
      {
         throw new System.InvalidOperationException("Not implemented");
      } 
      
      public override Double ReadDouble()
      {
         return System.BitConverter.ToDouble( SwitchEndian(System.BitConverter.GetBytes(base.ReadDouble())), 0);
      } 
      
      public override Int16 ReadInt16()
      {
         return System.BitConverter.ToInt16( SwitchEndian(System.BitConverter.GetBytes(base.ReadInt16())), 0);
      } 
      
      public override Int32 ReadInt32()
      {
         return System.BitConverter.ToInt32( SwitchEndian(System.BitConverter.GetBytes(base.ReadInt32())), 0);
      } 
      
      public override Int64 ReadInt64()
      {
         return System.BitConverter.ToInt64( SwitchEndian(System.BitConverter.GetBytes(base.ReadInt64())), 0);
      } 
      
      public override Single ReadSingle()
      {
         return System.BitConverter.ToSingle( SwitchEndian(System.BitConverter.GetBytes(base.ReadSingle())), 0);
      } 
      
      public override UInt16 ReadUInt16()
      {
         return System.BitConverter.ToUInt16( SwitchEndian(System.BitConverter.GetBytes(base.ReadUInt16())), 0);
      } 
      
      public override UInt32 ReadUInt32()
      {
         return System.BitConverter.ToUInt32( SwitchEndian(System.BitConverter.GetBytes(base.ReadUInt32())), 0);
      } 
      
      public override UInt64 ReadUInt64()
      {
         return System.BitConverter.ToUInt64( SwitchEndian(System.BitConverter.GetBytes(base.ReadUInt64())), 0);
      } 
      
      byte[] SwitchEndian(byte[] array)
      {
         // Not the worlds greatest implementation, but we can speed it up if we have to.
         System.Diagnostics.Debug.Assert((array.Length & 1) == 0, "No endian swaps on odd arrays.");
         
         // Reverse order
         for (int loop = 0; loop < (array.Length / 2); loop++)
         {
            int swapIndex = array.Length - 1 - loop;
            byte val = array[loop];
            array[loop] = array[swapIndex];
            array[swapIndex] = val;
         }
         
         return array;
      }
      
   }
}
