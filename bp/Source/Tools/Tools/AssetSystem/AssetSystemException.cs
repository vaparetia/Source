using System;
using System.Collections.Generic;
using System.Text;

namespace Tools.AssetSystem
{
   public class AssetSystemException : System.Exception
   {
      public AssetSystemException( String s )
         : base( s )
      {
      }
   }
}
