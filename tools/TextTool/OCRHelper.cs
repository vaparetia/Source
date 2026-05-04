using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Net;

namespace TextTool
{
   public static class DictionaryExtensions
   {
      public const string FormDataTemplate = "--{0}\r\nContent-Disposition: form-data; name=\"{1}\"\r\n\r\n{2}\r\n";

      public static void WriteMultipartFormData(this Dictionary<string, string> dictionary, Stream stream, string mimeBoundary)
      {
         if (dictionary == null || dictionary.Count == 0)
         {
            return;
         }
         if (stream == null)
         {
            throw new ArgumentNullException("stream");
         }
         if (mimeBoundary == null)
         {
            throw new ArgumentNullException("mimeBoundary");
         }
         if (mimeBoundary.Length == 0)
         {
            throw new ArgumentException("MIME boundary may not be empty.", "mimeBoundary");
         }
         foreach (string key in dictionary.Keys)
         {
            string item = String.Format(FormDataTemplate, mimeBoundary, key, dictionary[key]);
            byte[] itemBytes = System.Text.Encoding.UTF8.GetBytes(item);
            stream.Write(itemBytes, 0, itemBytes.Length);
         }
      }
   }

   public static class FileInfoExtensions
   {
      public const string HeaderTemplate = "--{0}\r\nContent-Disposition: form-data; name=\"{1}\"; filename=\"{2}\"\r\nContent-Type: {3}\r\n\r\n";

      public static void WriteMultipartFormData(this FileInfo file, Stream stream, string mimeBoundary, string mimeType, string formKey)
      {
         string header = String.Format(HeaderTemplate, mimeBoundary, formKey, file.Name, mimeType);
         byte[] headerbytes = Encoding.UTF8.GetBytes(header);
         stream.Write(headerbytes, 0, headerbytes.Length);
         using (FileStream fileStream = new FileStream(file.FullName, FileMode.Open, FileAccess.Read))
         {
            byte[] buffer = new byte[1024];
            int bytesRead = 0;
            while ((bytesRead = fileStream.Read(buffer, 0, buffer.Length)) != 0)
            {
               stream.Write(buffer, 0, bytesRead);
            }
            fileStream.Close();
         }
         byte[] newlineBytes = Encoding.UTF8.GetBytes("\r\n");
         stream.Write(newlineBytes, 0, newlineBytes.Length);
      }
   }

   public class OCRHelper
   {
      private static string CreateFormDataBoundary()
      {
         return "---------------------------" + DateTime.Now.Ticks.ToString("x");
      }

      private static string ExecutePostRequest(Uri url, Dictionary<string, string> postData, FileInfo fileToUpload, string fileMimeType, string fileFormKey)
      {
         HttpWebRequest request = (HttpWebRequest)WebRequest.Create(url.AbsoluteUri);
         request.Method = "POST";
         request.KeepAlive = true;
         string boundary = CreateFormDataBoundary();
         request.ContentType = "multipart/form-data; boundary=" + boundary;
         Stream requestStream = request.GetRequestStream();
         
         postData.WriteMultipartFormData(requestStream, boundary);
         
         if (fileToUpload != null)
            fileToUpload.WriteMultipartFormData(requestStream, boundary, fileMimeType, fileFormKey);

         byte[] endBytes = System.Text.Encoding.UTF8.GetBytes("--" + boundary + "--");
         
         requestStream.Write(endBytes, 0, endBytes.Length);
         requestStream.Close();
         
         using (WebResponse response = request.GetResponse())
         {
            using (StreamReader reader = new StreamReader(response.GetResponseStream()))
            {
               return reader.ReadToEnd();
            }
         }
      }

      public static void GetCharacterMatches(string filePath, out List<string> results)
      {
         //Uri url = new Uri("http://maggie.ocrgrid.org/cgi-bin/weocr/nhocr.cgi");
         Uri url = new Uri("http://appsv.ocrgrid.org/cgi-bin/weocr/nhocr.cgi");
         
         Dictionary<string, string> postData = new Dictionary<string, string>();
         
         postData["outputencoding"] = "utf-8";
         postData["outputformat"] = "txt"; 
         postData["eclass"] = "character";
         postData["ntop"] = "20";

         FileInfo fileToUpload = new FileInfo(filePath);

         string fileMimeType = "image/bmp";
         string fileFormKey = "userfile";

         string result = ExecutePostRequest(url, postData, fileToUpload, fileMimeType, fileFormKey);
         
         StringReader reader = new StringReader(result);
         results = new List<string>();


         for (; ; )
         {
            string line = reader.ReadLine();
            
            if (line == null)
               break;

            line = line.Trim();
            if( line.StartsWith("R") )
            {
               string[] resultElements = line.Split('\t');

               if( resultElements.Length == 6 )
               {
                  results.Add(resultElements[2]);
               }
            }
         }
      }
   }
}
