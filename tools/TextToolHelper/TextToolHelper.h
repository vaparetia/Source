// TextToolHelper.h

#pragma once

using namespace System;
using namespace System::Drawing;

namespace TextToolHelper 
{
   public enum class TextPaneMode
   {
      Default = 0,
      MGS2_Codec,
      MGS2_Codec_Japan,
      MGS3_Codec,
      MGS3_Codec_Japan,
   };

	public ref class TextRender
	{
   public:
      TextRender();

      void LoadResidentFont(String^ fontPath);
      void SetFontData(int type, array<Byte>^ fontData);
      Bitmap^ RenderText(array<Byte>^ text, int width, int height, bool displayRubi);

      void BP_LoadFont(String^ fontPath);
      Bitmap^ BP_RenderText(array<Byte>^ text, int width, int height, int platform);
      Bitmap^ BP_RenderText(array<Byte>^ text, int width, int height, int platform, TextToolHelper::TextPaneMode mode);
      void BP_GetPaneSize(TextPaneMode mode, int % width, int % height);
	};
}
