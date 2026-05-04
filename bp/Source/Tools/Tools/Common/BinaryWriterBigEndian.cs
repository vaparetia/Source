using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace Tools.Common
{
   /// <summary>
   /// Replacement for standard BinaryWriter (that writes in little endian mode).
   /// </summary>
   public class BinaryWriterBigEndian : System.IO.BinaryWriter
   {
      public BinaryWriterBigEndian(System.IO.Stream stream)
       : base(stream)
      {
      }
      
      public override void Write(System.Decimal val)
      {
         throw new System.InvalidOperationException("Not implemented");
      } 
      
      public override void Write(System.Double val)
      {
         WriteAsBigEndian(BitConverter.GetBytes(val));
      } 
      
      public override void Write(System.Int16 val)
      {
         WriteAsBigEndian(BitConverter.GetBytes(val));
      } 
      
      public override void Write(System.Int32 val)
      {
         WriteAsBigEndian(BitConverter.GetBytes(val));
      } 
      
      public override void Write(System.Int64 val)
      {
         WriteAsBigEndian(BitConverter.GetBytes(val));
      } 
      
      public override void Write(System.Single val)
      {
         WriteAsBigEndian(BitConverter.GetBytes(val));
      } 
      
      public override void Write(System.UInt16 val)
      {
         WriteAsBigEndian(BitConverter.GetBytes(val));
      } 
      
      public override void Write(System.UInt32 val)
      {
         WriteAsBigEndian(BitConverter.GetBytes(val));
      } 
      
      public override void Write(System.UInt64 val)
      {
         WriteAsBigEndian(BitConverter.GetBytes(val));
      } 
      
      public void WriteAsBigEndian(byte[] array)
      {
         // Reverse order
         for (int loop = (array.Length - 1); loop >= 0; loop--)
         {
            byte val = array[loop];
            Write(val);
         }
      }
   }
}
