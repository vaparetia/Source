// System name spaces
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;
using System.Collections;
using System.Runtime.InteropServices;

// Engine name spaces
using Helper;
using System.Xml;

namespace SplineEditor
{
   public partial class SplineEditorControl : UserControl
   {
      // Drag modes
      enum EDragMode
      {
         kDrag_None,
         kDrag_PanningView,
         kDrag_Select,
         kDrag_Points,
      };

      enum EKeySelectMode
      {
         kKeySelectMode_All,
         kKeySelectMode_Last
      }

      Spline.ETangentType mTangentType = Spline.ETangentType.Clamped;

      // Input
      EDragMode mDragMode = EDragMode.kDrag_None;
      Point mMouseLocation;
      Point mDragSelectStart;
      Point mDragSelectEnd;
      Rectangle mDragSelectRect = new Rectangle(0, 0, 0, 0);
      const int kOverPointThreshold = 8;
      
      const int kSplineKeyHighlightSize = 12;
      const int kSplineKeySize = 7;

      // Snapping
      bool mbTimeSnap = true;
      bool mbValueSnap = false;

      // Colors
      private Color mDisabledColor = Color.FromArgb(120, 120, 120);
      private Color mSplineColor = Color.FromArgb(255, 255, 255);
      private Color mPointColor = Color.FromArgb(0, 0, 0);
      private Color mKeyColor = Color.FromArgb(0, 0, 0);
      private Color mKeySelectedColor = Color.FromArgb(0, 128, 255);
      private Color mTangentSelectedColor = Color.FromArgb(214, 255, 97);
      private Color mTangentColor = Color.FromArgb(138, 71, 51);

      private Color mGridColor = Color.FromArgb(190, 190, 190);
      private Color mBackgroundColor = Color.FromArgb(228, 228, 228);
      private Color mBackgroundColorInvalidArea = Color.FromArgb(200, 200, 200);
      private Color mAxisXColor = Color.FromArgb(0, 0, 0);
      private Color mAxisYColor = Color.FromArgb(0, 0, 0);
      private static readonly Font mTextFont = new Font( "tahoma", 8.0f );
      private static readonly Brush mTextBrush = new SolidBrush(Color.FromArgb(0, 0, 0));
      private static readonly Pen mDragSelectPen = new Pen( Color.FromArgb( 64, 64, 64 ) );

      // Cursors
      // TO DO: Figure out why drop shadow is not present!
      private Cursor mMoveCursor = null;
      private Cursor mAddCursor = null;
      private Cursor mDeleteCursor = null;
      private Cursor mGrabCursor = null;

      // Graphics
      private Graphics mGraphics;
      private Bitmap mBitmap;

      // View
      private float mViewTimeMin = 0.0f;
      private float mViewTimeMax = 0.0f;
      private float mViewTimeScale = 1.0f;
      private float mViewTimeOrigin = 0.0f;

      private float mViewValueMin = 0.0f;
      private float mViewValueMax = 0.0f;
      private float mViewValueScale = 1.0f;
      private float mViewValueOrigin = 0.0f;

      private float mViewScale = 1.0f;

      private float? mTimeMin;
      
      public float? TimeMin
      {
         get { return mTimeMin; }
         set { mTimeMin = value; }
      }

      private float? mTimeMax;

      public float? TimeMax
      {
         get { return mTimeMax; }
         set { mTimeMax = value; }
      }

      // Array of splines to be edited
      public List<Spline> mSplines = new List<Spline>();
      int GetSplineCount() { return mSplines.Count; }
      Spline GetSpline(int index) { return mSplines[index]; }

      // Functions
      public SplineEditorControl()
      {
         // Setup control
         InitializeComponent();
         InitializeGraphics();

         mMoveCursor = new Cursor(new System.IO.MemoryStream(global::SplineEditor.Properties.Resources.Move));
         mAddCursor = new Cursor(new System.IO.MemoryStream(global::SplineEditor.Properties.Resources.Add));
         mDeleteCursor = new Cursor(new System.IO.MemoryStream(global::SplineEditor.Properties.Resources.Delete));
         mGrabCursor = new Cursor(new System.IO.MemoryStream(global::SplineEditor.Properties.Resources.Grab));

         // Give focus to graphics window
         BitmapPictureBox.Select();

         UpdateTangentToolbarButtons();
      }

      /// <summary> 
      /// Sets up view to frame all splines
      /// </summary>
      public void FrameAllSplines()
      {
         // No splines?
         if (GetSplineCount() == 0)
            return;

         // Grab bounds from first spline
         Spline S = GetSpline(0);
         Helper.SplineBounds SplineBounds = S.ComputeBounds();
         mViewTimeMin = SplineBounds.mTimeMin;
         mViewTimeMax = SplineBounds.mTimeMax;
         mViewValueMin = SplineBounds.mValueMin;
         mViewValueMax = SplineBounds.mValueMax;

         // Combine with other spline limits
         for (int i = 1; i < GetSplineCount(); i++)
         {
            S = GetSpline(i);
            SplineBounds = S.ComputeBounds();
            mViewTimeMin = Math.Min(mViewTimeMin, SplineBounds.mTimeMin);
            mViewTimeMax = Math.Max(mViewTimeMax, SplineBounds.mTimeMax);
            mViewValueMin = Math.Min(mViewValueMin, SplineBounds.mValueMin);
            mViewValueMax = Math.Max(mViewValueMax, SplineBounds.mValueMax);
         }

         // Make sure they aren't zero
         if( Math.Abs(mViewTimeMax - mViewTimeMin) < float.Epsilon)
            mViewTimeMax += 1.0f;

         if (Math.Abs(mViewValueMax - mViewValueMin) < float.Epsilon)
            mViewValueMax += 1.0f;

         // Setup to show all of spline
         float timeDelta = mViewTimeMax - mViewTimeMin;
         float valueDelta = mViewValueMax - mViewValueMin;
         mViewTimeScale = (float)mBitmap.Width / timeDelta;
         mViewValueScale = (float)mBitmap.Height / valueDelta;
         mViewTimeOrigin = (mViewTimeMin + mViewTimeMax) / 2.0f;
         mViewValueOrigin = (mViewValueMin + mViewValueMax) / 2.0f;

         // Zoom out some
         mViewScale = 0.7f;
      }

      /// <summary> 
      /// Initializes from spline xml document
      /// </summary>
      public void AddSplineFromXmlDoc(System.Xml.XmlDocument xmlDoc, Spline.EType type)
      {
         // Add new spline
         Spline spline = new Spline(xmlDoc, type);
         mSplines.Add(spline);

         // Add to list box
         SplineListBox.Items.Add(spline.GetName(), true);

         // Update view
         FrameAllSplines();

         // Draw spline
         UpdateGraphics(false);
      }

      /// <summary> 
      /// Initializes graphics
      /// </summary>
      private void InitializeGraphics()
      {
         // Lookup current bitmap size
         int BitmapW = 0;
         int BitmapH = 0;
         if (mBitmap != null)
         {
            BitmapW = mBitmap.Width;
            BitmapH = mBitmap.Height;
         }

         // Lookup valid picture box size
         int W = this.BitmapPictureBox.Width;
         int H = this.BitmapPictureBox.Height;
         if (W == 0)
         {
            W = 1;
         }
         if (H == 0)
         {
            H = 1;
         }

         // Create new graphics?
         if ((BitmapW != W) || (BitmapH != H))
         {
            // Delete old graphics
            if (mGraphics != null)
               mGraphics.Dispose();
            if (mBitmap != null)
               mBitmap.Dispose();

            // Create bitmap
            mBitmap = new Bitmap(W, H);

            // Create graphics
            mGraphics = Graphics.FromImage(mBitmap);
            mGraphics.Clear(mBackgroundColor);

            // Display bitmap
            this.BitmapPictureBox.Image = mBitmap;

            // Scale the view
            if ((BitmapW != 0) && (BitmapH != 0))
            {
               float X = (float)BitmapW / (float)W;
               float Y = (float)BitmapH / (float)H;
               mViewTimeScale /= X;
               mViewValueScale /= Y;
            }
         }
      }

      /// <summary> 
      /// Draw text to screen
      /// </summary>
      private void DrawText(string text, int x, int y)
      {
         mGraphics.DrawString(text, mTextFont, mTextBrush, x, y);
      }

      /// <summary> 
      /// Converts spline point to screen
      /// </summary>
      private Point GetScreenPosition(float time, float value)
      {
         int XOffset = mBitmap.Width / 2;
         int YOffset = mBitmap.Height / 2;
         int X = XOffset + (int)(0.5f + ((time - mViewTimeOrigin) * (mViewTimeScale * mViewScale)));
         int Y = YOffset - (int)(0.5f + ((value - mViewValueOrigin) * (mViewValueScale * mViewScale)));
         return new Point(X, Y);
      }

      /// <summary> 
      /// Converts screen position to spline point
      /// </summary>
      private PointF GetSplinePosition(Point screenPos)
      {
         int XOffset = mBitmap.Width / 2;
         int YOffset = mBitmap.Height / 2;
         float X = mViewTimeOrigin + (((float)screenPos.X - 0.5f - XOffset) / (mViewTimeScale * mViewScale));
         float Y = mViewValueOrigin + ((YOffset - (float)screenPos.Y - 0.5f) / (mViewValueScale * mViewScale));
         return new PointF(X, Y);
      }

      /// <summary> 
      /// Draws 2d line between points
      /// </summary>
      private void DrawLine(Color Color, Point P0, Point P1)
      {
         using ( Pen p = new Pen( Color ) )
         {
            mGraphics.DrawLine( p, P0, P1 );
         }
      }

      /// <summary> 
      /// Draws line between 2 spline points
      /// </summary>
      private void DrawSplineLine(Color Color, PointF P0, PointF P1)
      {
         using ( Pen p = new Pen( Color ) )
         {
            mGraphics.DrawLine( p, GetScreenPosition( P0.X, P0.Y ), GetScreenPosition( P1.X, P1.Y ) );
         }
      }

      /// <summary> 
      /// Draws spline point
      /// </summary>
      private void DrawSplinePoint(PointF K, Color Color, int Size, bool bFill)
      {
         Point S = GetScreenPosition(K.X, K.Y);
         Rectangle R = new Rectangle(S.X - (Size / 2), S.Y - (Size / 2), Size, Size);
         if (bFill)
         {
            using ( SolidBrush b = new SolidBrush( Color ) )
            {
               mGraphics.FillRectangle( b, R );
            }
         }
         else
         {
            R.Width -= 1;
            R.Height -= 1;
            using ( Pen p = new Pen( Color ) )
            {
               mGraphics.DrawRectangle( p, R );
            }
         }
      }

      /// <summary> 
      /// Computes easier to read step
      /// </summary>
      private float ComputeGridStep(float scale)
      {
         float stepEstimate = 120.0f / (scale * mViewScale);
         float step = stepEstimate;
         double stepLog10 = Math.Log10(step);         
         step = (float) Math.Pow(10.0, Math.Floor(stepLog10));
         float ratio = step / stepEstimate;
         if (ratio < 0.2f)
         {
            step *= 5.0f;
         }
         else if (ratio < 0.4f)
         {
            step *= 2.5f;
         }
         
         if (step == 0.0f)
         {
            // This shouldn't hit, but you never know
            step = 100.0f;
         }

         return step;
      }

      /// <summary> 
      /// Draws grid into bitmap
      /// </summary>
      private void DrawGrid()
      {
         int width = mBitmap.Width;
         int height = mBitmap.Height;

         int validRegionStartX = 0;
         int validRegionEndX = width;
         
         if( mTimeMin.HasValue )
            validRegionStartX = GetScreenPosition(mTimeMin.Value, 0.0f).X;

         if( mTimeMax.HasValue )
            validRegionEndX = GetScreenPosition(mTimeMax.Value, 0.0f).X;

         // build rectangle for valid region
         Rectangle validRegionRectangle = new Rectangle(validRegionStartX, 0, validRegionEndX - validRegionStartX, height);

         // build rectangle for invalid region (whole view minus valid region);
         Region invalidRegion = new Region(new Rectangle(0, 0, width, height));
         invalidRegion.Exclude(validRegionRectangle);

         // fill valid region with valid color
         using ( SolidBrush b = new SolidBrush( mBackgroundColor ) )
         {
            mGraphics.FillRectangle( b, validRegionRectangle );
         }

         // Fill invalid region with invalid color
         using ( SolidBrush b = new SolidBrush( mBackgroundColorInvalidArea ) )
         {
            mGraphics.FillRegion( b, invalidRegion );
         }

         // Compute value grid limits
         Point Origin = GetScreenPosition(0.0f, 0.0f);
         Point P;
         float ValueStep = ComputeGridStep(mViewValueScale);
         float TimeStep = ComputeGridStep(mViewTimeScale);

         // Compute grid value min
         int ValueMinSteps = 1 + (int)(Math.Abs(mViewValueMin) / ValueStep);
         float ValueMin = Math.Sign(mViewValueMin) * ValueStep * ValueMinSteps;

         // Compute grid value max
         int ValueMaxSteps = 1 + (int)(Math.Abs(mViewValueMax) / ValueStep);
         float ValueMax = Math.Sign(mViewValueMax) * ValueStep * ValueMaxSteps;

         // Compute grid value min
         int TimeMinSteps = 1 + (int)(Math.Abs(mViewTimeMin) / TimeStep);
         float TimeMin = Math.Sign(mViewTimeMin) * TimeStep * TimeMinSteps;

         // Compute grid value max
         int TimeMaxSteps = 1 + (int)(Math.Abs(mViewTimeMax) / TimeStep);
         float TimeMax = Math.Sign(mViewTimeMax) * TimeStep * TimeMaxSteps;

         // Draw horizontal grid lines
         float Value;
         for (Value = ValueMin; Value <= ValueMax; Value += ValueStep)
         {
            P = GetScreenPosition(0.0f, Value);
            DrawLine(mGridColor, new Point(0, P.Y), new Point(width, P.Y));
         }

         // Draw vertical grid lines
         float Time;
         for (Time = TimeMin; Time <= TimeMax; Time += TimeStep)
         {
            P = GetScreenPosition(Time, 0.0f);
            DrawLine(mGridColor, new Point(P.X, 0), new Point(P.X, height));
         }

         // Draw axis'
         DrawLine(mAxisXColor, new Point(0, Origin.Y), new Point(width, Origin.Y));
         DrawLine(mAxisYColor, new Point(Origin.X, 0), new Point(Origin.X, height));

         // Draw horizontal grid line text
         {
            int stepLog10 = (int) Math.Round(Math.Log10(ValueStep));
            for (Value = ValueMin + ValueStep; Value <= ValueMax; Value += ValueStep)
            {
               P = GetScreenPosition(0.0f, Value);
               if (stepLog10 > 0)
               {
                  DrawText(String.Format("{0:d}", (int)Value), 2, P.Y);
               }
               else
               {
                  if (stepLog10 > -2)
                  {
                     DrawText(String.Format("{0:f2}", Value), 2, P.Y);                  
                  }
                  else if (stepLog10 > -3)
                  {
                     DrawText(String.Format("{0:f3}", Value), 2, P.Y);                  
                  }
                  else if (stepLog10 > -4)
                  {
                     DrawText(String.Format("{0:f4}", Value), 2, P.Y);                  
                  }
                  else
                  {
                     DrawText(String.Format("{0:f5}", Value), 2, P.Y);                                    
                  }               
               }
            }
         }

         // Draw vertical grid line text
         {
            int stepLog10 = (int) Math.Round(Math.Log10(TimeStep));
            for (Time = TimeMin; Time <= TimeMax; Time += TimeStep)
            {
               P = GetScreenPosition(Time, 0.0f);
               if (stepLog10 > 0)
               {
                  DrawText(String.Format("{0:d}", (int) Time), P.X, height - mTextFont.Height - 2);             
               }
               else
               {
                  if (stepLog10 > -2)
                  {
                     DrawText(String.Format("{0:f2}", Time), P.X, height - mTextFont.Height - 2);             
                  }
                  else if (stepLog10 > -3)
                  {
                     DrawText(String.Format("{0:f3}", Time), P.X, height - mTextFont.Height - 2);             
                  }
                  else if (stepLog10 > -4)
                  {
                     DrawText(String.Format("{0:f4}", Time), P.X, height - mTextFont.Height - 2);             
                  }
                  else
                  {
                     DrawText(String.Format("{0:f5}", Time), P.X, height - mTextFont.Height - 2);             
                  }               
               }
            }
         }
      }

      /// <summary> 
      /// Draws splines into bitmap
      /// </summary>
      private void DrawSplines()
      {
         // Compute constant tangent size
         PointF tangentSize = new PointF(56.0f / (mViewScale * mViewTimeScale),
                                         56.0f / (mViewScale * mViewValueScale));

         // Draw in passes (disabled first)
         for (int pass = 0; pass < 2; pass++)
         {
            // Loop over all splines
            for (int i = 0; i < GetSplineCount(); i++)
            {
               // Lookup spline info
               Spline spline = GetSpline(i);

               // Render disabled in first pass, enabled in 2nd pass
               if (spline.Enabled && (pass == 0))
                  continue;
               if (!spline.Enabled && (pass == 1))
                  continue;

               // Set tangent size
               spline.SetTangentSize(tangentSize);

               DrawSpline(spline);
            }
         }
      }

      private void DrawSpline(Spline spline)
      {
         Helper.SplineBounds splineBounds = spline.Bounds;
         List<XmlElement> splineKeys = spline.Keys;

         // Lookup colors
         Color splineColor = mSplineColor;
         Color keyColor = mKeyColor;
         Color keySelectedColor = mKeySelectedColor;
         Color tangentColor = mTangentColor;
         Color tangentSelectedColor = mTangentSelectedColor;

         if (spline.Enabled)
         {
            switch (spline.GetSplineType())
            {
               case Spline.EType.kType_General:
               case Spline.EType.kType_R: splineColor = Color.FromArgb(255, 0, 0); break;
               case Spline.EType.kType_G: splineColor = Color.FromArgb(0, 224, 0); break;
               case Spline.EType.kType_B: splineColor = Color.FromArgb(0, 0, 255); break;
               case Spline.EType.kType_A: splineColor = Color.FromArgb(255, 255, 255); break;
               case Spline.EType.kType_X: splineColor = Color.FromArgb(255, 0, 0); break;
               case Spline.EType.kType_Y: splineColor = Color.FromArgb(0, 224, 0); break;
               case Spline.EType.kType_Z: splineColor = Color.FromArgb(0, 0, 255); break;
            }
         }
         else
         {
            splineColor = mDisabledColor;
            keyColor = mDisabledColor;
            keySelectedColor = mDisabledColor;
            tangentColor = mDisabledColor;
            tangentSelectedColor = mDisabledColor;
         }

         // Compute view port clipped time limits
         float splineTimeMin = Math.Max(mViewTimeMin, splineBounds.mTimeMin);
         float splineTimeMax = Math.Min(mViewTimeMax, splineBounds.mTimeMax);

         // draw pre infinity
         if (mViewTimeMin < splineTimeMin )
            DrawSplineSegment(spline, splineColor, mViewTimeMin, splineTimeMin, true);

         DrawSplineSegment(spline, splineColor, splineTimeMin, splineTimeMax, false);

         // draw post infinity
         if (splineTimeMax < mViewTimeMax)
            DrawSplineSegment(spline, splineColor, splineTimeMax, mViewTimeMax, true);

         // Draw tangent lines
         /*
         if( spline.Enabled )
         {
            foreach(XmlElement key in splineKeys)
            {
               PointF Key = spline.GetKeyPoint(key);
               PointF In = spline.GetInTangentPoint(key);
               PointF Out = spline.GetOutTangentPoint(key);
               DrawSplineLine(spline.GetInTangentSelected(key) ? tangentSelectedColor : tangentColor, Key, In);
               DrawSplineLine(spline.GetOutTangentSelected(key) ? tangentSelectedColor : tangentColor, Key, Out);
            }
         }
         */

         // Draw key and tangent points
         bool bMouseOverKey = false;
         foreach(XmlElement key in splineKeys)
         {
            /*
            if (spline.Enabled && (spline.GetKeySelected(keyIndex) || spline.GetInTangentSelected(keyIndex) || spline.GetOutTangentSelected(keyIndex)))
            {
               // Draw key in tangent point
               PointF In = spline.GetInTangentPoint(keyIndex);
               bool bInSelected = spline.GetInTangentSelected(keyIndex);
               DrawSplinePoint(In, bInSelected ? tangentSelectedColor : tangentColor, 3, bInSelected);

               // Draw key out tangent point
               PointF Out = spline.GetOutTangentPoint(keyIndex);
               bool bOutSelected = spline.GetOutTangentSelected(keyIndex);
               DrawSplinePoint(Out, bOutSelected ? tangentSelectedColor : tangentColor, 3, bOutSelected);
            }
            */

            // Lookup key info
            PointF keyPosition = spline.GetKeyPoint(key);
            bool bKeySelected = spline.GetKeySelected(key);

            bool bMouseOverThisKey = IsMouseOverPoint(keyPosition);
            bMouseOverKey |= bMouseOverThisKey;

            // Hilight key if deleting
            if ((PressingDeleteKey()) && (bMouseOverThisKey))
            {
               DrawSplinePoint(keyPosition, keySelectedColor, kSplineKeyHighlightSize, false);
            }

            // Draw point
            DrawSplinePoint(keyPosition, bKeySelected ? keySelectedColor : keyColor, kSplineKeySize, true);
         }

         // Draw point that will be added?
         if (spline.Enabled && PressingAddKey() && (!bMouseOverKey) && (mDragMode == EDragMode.kDrag_None))
         {
            // Mouse close to spline in pixels?
            PointF MousePos = GetMouseSplinePos();
            PointF SplinePos = new PointF(MousePos.X, spline.Evaluate(MousePos.X));
            if (IsMouseOverPoint(SplinePos))
            {
               // Draw point
               DrawSplinePoint(SplinePos, keySelectedColor, kSplineKeyHighlightSize, false);
               DrawSplinePoint(SplinePos, keyColor, kSplineKeySize, true);
            }
         }
      }

      private void DrawSplineSegment(Spline spline, Color splineColor, float timeStart, float timeEnd, bool drawInfinity)
      {
         float timeRange = timeEnd - timeStart;

         // Approx pixel time step
         float initialTimeStep = 0.01f * (float)mBitmap.Width / (mViewTimeScale * mViewScale);
         if (initialTimeStep >= timeRange)
         {
            initialTimeStep /= 2.0f;
         }
         float timeStep = initialTimeStep;
         
         Point[] screenPoints = new Point[mBitmap.Width];         

         float currentTime = timeStart - timeStep;
         int arrayIndex = 0;
         bool bTimeStepIncreased = false;
         
         do
         {
            currentTime += timeStep;
            if (currentTime > timeEnd)
            {
               currentTime -= timeStep;
               timeStep = timeEnd - currentTime;
               currentTime = timeEnd;
            }
            float value = spline.Evaluate(currentTime);
            Point screenPos = GetScreenPosition(currentTime, value);
            if (arrayIndex > 0) 
            {
               // See if we have to reduce/increase timestep
               Point prevScreenPos = screenPoints[arrayIndex - 1];
               int screenDeltaY = screenPos.Y - prevScreenPos.Y;
               int screenDeltaX = screenPos.X - prevScreenPos.X;               
               
               if (!bTimeStepIncreased && (Math.Abs(screenDeltaY) > 2))
               {
                  // Check to see if points are onscreen (fixes extreme zoom slowdown)
                  if (((screenPos.Y >= 0) && (screenPos.Y < mBitmap.Height)) ||
                     ((prevScreenPos.Y >= 0) && (prevScreenPos.Y < mBitmap.Height)))
                  {
                     // Points too far apart, subdivide
                     if (timeStep > (initialTimeStep / 16) )
                     {                     
                        currentTime -= timeStep;
                        timeStep /= 2.0f;                  
                        continue;
                     }                     
                  }
               }
               else if (Math.Abs(screenDeltaY) < 1)
               {
                  // Points too close together, increase step
                  if (timeStep < initialTimeStep || (Math.Abs(screenDeltaX) < 1))
                  {                     
                     bTimeStepIncreased = true;
                     timeStep *= 2.0f;                                       
                     if (Math.Abs(screenDeltaX) < 1)
                     {
                        // Zero length line, skip
                        continue;
                     }
                  }
               }
            }                     
            bTimeStepIncreased = false;
            screenPoints[arrayIndex] = screenPos;
            arrayIndex++;               
            if (screenPoints.Length == arrayIndex)
            {
               Array.Resize(ref screenPoints, screenPoints.Length * 2);
            }
         } while (currentTime < timeEnd);
         
         // Truncate array
         Array.Resize(ref screenPoints, arrayIndex);
         // Draw spline with line segments         
         using ( Pen pen = new Pen( splineColor ) )
         {
            if ( drawInfinity )
            {
               pen.DashStyle = System.Drawing.Drawing2D.DashStyle.Custom;
               pen.DashPattern = new float[] { 2, 3 };
            }            
            if (screenPoints.Length > 1)
            {
               mGraphics.DrawLines(pen, screenPoints);            
            }
         }
      }

      /// <summary> 
      /// Refreshes all graphics
      /// </summary>
      private void UpdateGraphics(bool bRefresh)
      {
         // Recompute visible time range
         float HalfTime = ((float)mBitmap.Width * 0.5f) / (mViewTimeScale * mViewScale);
         mViewTimeMin = mViewTimeOrigin - HalfTime;
         mViewTimeMax = mViewTimeOrigin + HalfTime;

         // Recompute visible value range
         float HalfValue = ((float)mBitmap.Height * 0.5f) / (mViewValueScale * mViewScale);
         mViewValueMin = mViewValueOrigin - HalfValue;
         mViewValueMax = mViewValueOrigin + HalfValue;

         // Draw elements
         DrawGrid();
         DrawSplines();

         // Draw drag select?
         if (mDragMode == EDragMode.kDrag_Select)
         {
            // Draw rectangle
            mDragSelectPen.DashStyle = System.Drawing.Drawing2D.DashStyle.Dash;
            mGraphics.DrawRectangle(mDragSelectPen, mDragSelectRect);
         }

         // Redraw
         if (bRefresh)
         {
            this.BitmapPictureBox.Refresh();
         }
      }

      // Returns true if mouse cursor is over given point
      private bool IsMouseOverPoint(PointF P)
      {
         // Lookup mouse position
         Point M = BitmapPictureBox.PointToClient(Cursor.Position);

         // Get screen position of point
         Point S = GetScreenPosition(P.X, P.Y);

         // Is mouse over point?
         return ((Math.Abs(M.X - S.X) < kOverPointThreshold) && (Math.Abs(M.Y - S.Y) < kOverPointThreshold));
      }

      // Returns true if mouse cursor is over a key or tangent point
      private bool IsMouseOverMovablePoint(bool bIncludeTangents)
      {
         // Loop through all splines
         for (int SplineIndex = 0; SplineIndex < GetSplineCount(); SplineIndex++)
         {
            // Lookup spline
            Spline spline = GetSpline(SplineIndex);
            
            if (spline.Enabled)
            {
               // Check all keys
               foreach(XmlElement key in spline.Keys)
               {
                  // Over key?
                  if (IsMouseOverPoint(spline.GetKeyPoint(key)))
                     return true;

                  /*
                  // Over tangent-in point?
                  if (IsMouseOverPoint(S.GetInTangentPoint(key)))
                     return true;

                  // Over tangent-out point?
                  if (IsMouseOverPoint(S.GetOutTangentPoint(key)))
                     return true;
                  */
               }
            }
         }

         return false;
      }

      private bool IsKeyUnderMouseSelected()
      {
         // Loop through all splines
         for (int SplineIndex = 0; SplineIndex < GetSplineCount(); SplineIndex++)
         {
            // Lookup spline
            Spline spline = GetSpline(SplineIndex);

            if (spline.Enabled)
            {
               // Check all keys
               foreach (XmlElement key in spline.Keys)
               {
                  // Over key?
                  if (IsMouseOverPoint(spline.GetKeyPoint(key)))
                  {
                     return spline.GetKeySelected(key);
                  }
               }
            }
         }

         return false;
      }

      // Clears all selected points
      private void ClearSelection()
      {
         // Loop through all splines
         for (int SplineIndex = 0; SplineIndex < GetSplineCount(); SplineIndex++)
         {
            // Lookup spline
            Spline S = GetSpline(SplineIndex);
            if (S.Enabled)
            {
               // Clear all keys
               S.ClearSelection();
            }
         }

         UpdateUIForSelectedKeys();
      }

      // Selected points in given rectangle
      private void SelectPoints(Rectangle Rect, bool bToggleSelected, bool bClearUnselected, EKeySelectMode keySelectMode)
      {
         /*
         // Update tangent point selection
         for (int SplineIndex = 0; SplineIndex < GetSplineCount(); SplineIndex++)
         {
            // Lookup spline
            Spline S = GetSpline(SplineIndex);
            if (S.Enabled)
            {
               // Check all keys
               foreach(XmlElement key in S.Keys)
               {
                  // Only select tangent points if visible
                  if (S.GetKeySelected(key) || S.GetInTangentSelected(key) || S.GetOutTangentSelected(key))
                  {
                     // Select tangent in point
                     {
                        Point P = GetScreenPosition(S.GetInTangentPoint(key));
                        bool bSelected = S.GetInTangentSelected(key);
                        if (Rect.Contains(P))
                        {
                           if (bToggleSelected)
                              S.SetInTangentSelected(key, !bSelected);
                           else
                              S.SetInTangentSelected(key, true);
                        }
                        else if (bClearUnselected)
                           S.SetInTangentSelected(key, false);
                     }

                     // Select tangent in point
                     {
                        Point P = GetScreenPosition(S.GetOutTangentPoint(key));
                        bool bSelected = S.GetOutTangentSelected(key);
                        if (Rect.Contains(P))
                        {
                           if (bToggleSelected)
                              S.SetOutTangentSelected(key, !bSelected);
                           else
                              S.SetOutTangentSelected(key, true);
                        }
                        else if (bClearUnselected)
                           S.SetOutTangentSelected(key, false);
                     }
                  }
               }
            }
         }
         */

         // Update key selection
         for (int SplineIndex = 0; SplineIndex < GetSplineCount(); SplineIndex++)
         {
            // Lookup spline
            Spline S = GetSpline(SplineIndex);
            if (S.Enabled)
            {
               List<XmlElement> keysToSelect = new List<XmlElement>();

               // Find all keys in rectangle
               foreach(XmlElement key in S.Keys)
               {
                  PointF splinePoint = S.GetKeyPoint(key);
                  Point P = GetScreenPosition(splinePoint.X, splinePoint.Y);

                  if (Rect.Contains(P))
                  {
                     keysToSelect.Add(key);
                  }
               }

               // if we're in select "last" mode (which is what we want when we single click in the viewport) we remove all but the last found key
               if( (keysToSelect.Count > 1) && keySelectMode == EKeySelectMode.kKeySelectMode_Last )
               {
                  keysToSelect.RemoveRange(0, keysToSelect.Count - 1);
               }

               // Actually do select operation on keys
               foreach(XmlElement key in S.Keys)
               {
                  bool bSelected = S.GetKeySelected(key);
                  if(keysToSelect.Contains(key))
                  {
                     if (bToggleSelected)
                        S.SetKeySelected(key, !bSelected);
                     else
                        S.SetKeySelected(key, true);
                  }
                  else if (bClearUnselected)
                  {
                     S.SetKeySelected(key, false);
                  }

               }
            }
         }

         UpdateUIForSelectedKeys();
      }

      private Point GetMouseScreenPos()
      {
         return BitmapPictureBox.PointToClient(Cursor.Position);
      }

      private PointF GetMouseSplinePos()
      {
         Point MousePos = GetMouseScreenPos();
         PointF SplinePos = GetSplinePosition(MousePos);
         return SplinePos;
      }

      private void UpdateStatusPanel()
      {
         // Update time and value
         PointF SplinePos = GetMouseSplinePos();
         TimeStatusLabel.Text = string.Format("{0:f}", SplinePos.X);
         ValueStatusLabel.Text = string.Format("{0:f}", SplinePos.Y);
      }

      private void OnMouseMove(object sender, MouseEventArgs e)
      {
         // Dragging?
         if (mDragMode == EDragMode.kDrag_PanningView)
         {
            // Compute mouse delta
            PointF Delta = new PointF();
            Delta.X = (float)(e.Location.X - mMouseLocation.X);
            Delta.Y = (float)(e.Location.Y - mMouseLocation.Y);
            mMouseLocation = e.Location;

            // Update origin
            mViewTimeOrigin -= Delta.X / (mViewTimeScale * mViewScale);
            mViewValueOrigin += Delta.Y / (mViewValueScale * mViewScale);

            // Refresh window
            UpdateGraphics(true);
         }

         // Drag select?
         else if (mDragMode == EDragMode.kDrag_Select)
         {
            // Compute drag rectangle limits
            mDragSelectEnd = e.Location;
            int MinX = Math.Min(mDragSelectStart.X, mDragSelectEnd.X);
            int MinY = Math.Min(mDragSelectStart.Y, mDragSelectEnd.Y);
            int MaxX = Math.Max(mDragSelectStart.X, mDragSelectEnd.X);
            int MaxY = Math.Max(mDragSelectStart.Y, mDragSelectEnd.Y);
            mDragSelectRect.X = MinX;
            mDragSelectRect.Y = MinY;
            mDragSelectRect.Width = MaxX - MinX;
            mDragSelectRect.Height = MaxY - MinY;

            // Refresh window
            UpdateGraphics(true);
         }
         // Drag points?
         else if (mDragMode == EDragMode.kDrag_Points)
         {
            // Compute delta pixels
            Point delta = new Point(e.Location.X - mMouseLocation.X, e.Location.Y - mMouseLocation.Y);
            mMouseLocation = e.Location;

            // Convert to delta position
            float deltaTime = (float)delta.X / (mViewScale * mViewTimeScale);
            float deltaValue = -(float)delta.Y / (mViewScale * mViewValueScale);

            // Update all selected spline points
            for (int splineIndex = 0; splineIndex < GetSplineCount(); splineIndex++)
            {
               // Lookup spline
               Spline S = GetSpline(splineIndex);
               if (S.Enabled)
               {
                  // Move all selected keys
                  foreach(XmlElement key in S.Keys)
                  {
                     // Update key point?
                     if (S.GetKeySelected(key))
                     {
                        float Value = S.GetKeyValue(key);
                        float Time = S.GetKeyTime(key);
                        S.SetKeyValue(key, Value + deltaValue);
                        S.SetKeyTime(key, Time + deltaTime);
                     }

                     // Update in tangent point?
                     if (S.GetInTangentSelected(key))
                     {
                        //float Value = S.GetKeyInTangentValue(KeyIndex);
                        //float Time = S.GetKeyInTangentTime(KeyIndex);
                        //S.SetKeyInTangentValue(KeyIndex, Value + DeltaValue);
                        //S.SetKeyInTangentTime(KeyIndex, Time + DeltaTime);
                     }

                     // Update out tangent point?
                     if (S.GetOutTangentSelected(key))
                     {
                        //float Value = S.GetKeyOutTangentValue(KeyIndex);
                        //float Time = S.GetKeyOutTangentTime(KeyIndex);
                        //S.SetKeyOutTangentValue(KeyIndex, Value + DeltaValue);
                        //S.SetKeyOutTangentTime(KeyIndex, Time + DeltaTime);
                     }
                  }
               }
            }

            UpdateUIForSelectedKeys();

            // Refresh
            UpdateGraphics(true);
         }
         else if (PressingDeleteKey() || PressingAddKey())
         {
            // Refresh
            UpdateGraphics(true);
         }

         // Update mouse and status
         UpdateMouseCursor();
         UpdateStatusPanel();
      }

      private void UpdateMouseCursor()
      {
         // Moving?
         if (mDragMode == EDragMode.kDrag_Points)
         {
            this.Cursor = mMoveCursor;
         }
         else
         {
            // Grabbing?
            if (PressingPanView())
            {
               this.Cursor = mGrabCursor;
            }
            // On a point?
            else if (IsMouseOverMovablePoint(true))
            {
               // Delete or move?
               if (PressingDeleteKey())
                  this.Cursor = mDeleteCursor;
               else
                  this.Cursor = mMoveCursor;
            }
            else
            {
               // Adding?
               if (PressingAddKey())
                  this.Cursor = mAddCursor;
               else if (PressingDeleteKey())
                  this.Cursor = mDeleteCursor;
               else
                  this.Cursor = Cursors.Default;
            }
         }
      }

      private bool PressingPanView()
      {
         return ((Control.MouseButtons & MouseButtons.Middle) != 0);
      }

      private bool PressingDeleteKey()
      {
         return false;// return (Control.ModifierKeys == Keys.Shift);
      }

      private bool PressingAddKey()
      {
         return (Control.ModifierKeys == Keys.Control);
      }

      private bool PressingAddToSelection()
      {
         return (Control.ModifierKeys == Keys.Shift);
      }

      private bool PressingToggleSelection()
      {
         return (Control.ModifierKeys == Keys.Alt);
      }

      private void OnMouseDown(object sender, MouseEventArgs e)
      {
         BitmapPictureBox.Select();

         // Drag origin?
         if (PressingPanView())
         {
            // Start dragging origing
            mDragMode = EDragMode.kDrag_PanningView;
            mMouseLocation = e.Location;
         }
         // Delete key?
         else if (PressingDeleteKey())
         {
            // Loop through all splines
            for (int SplineIndex = 0; SplineIndex < GetSplineCount(); SplineIndex++)
            {
               // Lookup spline
               Spline S = GetSpline(SplineIndex);
               if (S.Enabled)
               {
                  // Check all keys
                  foreach(XmlElement key in S.Keys)
                  {
                     // Over key?
                     if (IsMouseOverPoint(S.GetKeyPoint(key)))
                     {
                        S.DeleteKey(key);
                        break;
                     }
                  }
               }
            }
         }
         // Add key?
         else if (PressingAddKey() && (!IsMouseOverMovablePoint(false)))
         {
            // Mouse close to spline in pixels?
            PointF MousePos = GetMouseSplinePos();

            Spline spline = null;
            PointF splinePos = new PointF();

            // Determine number of enabled splines, if there is only a single enabled spline we can do it in a simple way.
            int enabledSplineCount = 0;
            {
               foreach (Spline currentSpline in mSplines)
               {
                  if (currentSpline.Enabled)
                     enabledSplineCount++;
               }
            }

            // Go over all splines
            for (int splineIndex = 0; splineIndex < GetSplineCount(); splineIndex++)
            {
               // Lookup spline
               Spline currentSpline = GetSpline(splineIndex);

               if (currentSpline.Enabled)
               {
                  // for a single spline we just add the key where the user clicked
                  if (enabledSplineCount == 1)
                  {
                     spline = currentSpline;
                     splinePos = new PointF(MousePos.X, MousePos.Y);
                     break;
                  }
                  else
                  {
                     // On spline?
                     PointF evaluatedSplinePos = new PointF(MousePos.X, currentSpline.Evaluate(MousePos.X));
                     if (IsMouseOverPoint(evaluatedSplinePos))
                     {
                        spline = currentSpline;
                        splinePos = evaluatedSplinePos;
                        break;
                     }
                  }
               }
            }

            // Add key to spline if we have a spline
            if( spline != null )
            {
               // Add new key
               XmlElement newKey = spline.AddKey(splinePos, mTangentType);

               // Select just that one key
               spline.ClearSelection();
               spline.SetKeySelected(newKey, true);
               UpdateUIForSelectedKeys();

               // Start dragging new key
               mDragMode = EDragMode.kDrag_Points;
               mMouseLocation = e.Location;
            }
         }
         else if (e.Button == MouseButtons.Left)
         {
            // On a point?
            if (IsMouseOverMovablePoint(true))
            {
               // Select points
               Rectangle Rect = new Rectangle(e.Location.X - kOverPointThreshold, e.Location.Y - kOverPointThreshold, kOverPointThreshold * 2, kOverPointThreshold * 2);
               
               bool toggleSelected = false;
               bool clearSelection = true;

               if( PressingToggleSelection() )
               {
                  toggleSelected = true;
                  clearSelection = false;
               }
               else if( PressingAddToSelection() )
               {
                  toggleSelected = false;
                  clearSelection = false;
               }
               else if (IsKeyUnderMouseSelected())
               {
                  clearSelection = false;
               }
         
               SelectPoints(Rect, toggleSelected, clearSelection, EKeySelectMode.kKeySelectMode_Last);

               // Start dragging points
               mDragMode = EDragMode.kDrag_Points;
               mMouseLocation = e.Location;
            }
            else if (!PressingAddKey() && !PressingDeleteKey())
            {
               // Start drag select
               mDragMode = EDragMode.kDrag_Select;
               mDragSelectStart = e.Location;
               mDragSelectEnd = e.Location;
               mDragSelectRect.X = e.Location.X;
               mDragSelectRect.Y = e.Location.Y;
               mDragSelectRect.Width = 0;
               mDragSelectRect.Height = 0;
            }
         }

         // Set mouse cursor
         UpdateMouseCursor();

         // Refresh window
         UpdateGraphics(true);
      }

      private void OnMouseUp(object sender, MouseEventArgs e)
      {
         // End of drag select?
         if (mDragMode == EDragMode.kDrag_Select)
         {
            bool toggleSelection = false;
            bool clearSelection = true;

            if (PressingToggleSelection())
            {
               toggleSelection = true;
               clearSelection = false;
            }
            else if(PressingAddToSelection())
            {
               toggleSelection = false;
               clearSelection = false;
            }

            SelectPoints(mDragSelectRect, toggleSelection, clearSelection, EKeySelectMode.kKeySelectMode_All);
         }

         // Release dragging
         mDragMode = EDragMode.kDrag_None;

         // Set mouse cursor
         UpdateMouseCursor();

         // Refresh window
         UpdateGraphics(true);
      }

      private void OnMouseWheel(object sender, MouseEventArgs e)
      {
         // Update status panel
         UpdateStatusPanel();

         // Compute delta
         float Delta = 1.0f + (float)(e.Delta * SystemInformation.MouseWheelScrollLines) / 2500.0f;
         if (Delta < 0.01f)
            Delta = 0.01f;

         // Apply
         mViewScale *= Delta;

         // Clamp
         if (mViewScale > 100000.0f)
         {
            mViewScale = 100000.0f;
         }
         else if (mViewScale < 0.0001f)
         {
            mViewScale = 0.0001f;
         }

         // Refresh window
         UpdateGraphics(true);
      }

      private void OnResize(object sender, EventArgs e)
      {
         // Re-init and draw graphics
         InitializeGraphics();
         UpdateGraphics(false);
      }

      private void FrameAllButton_Click(object sender, EventArgs e)
      {
         // Update view
         FrameAllSplines();

         // Refresh
         UpdateGraphics(true);
      }

      private void TimeSnapButton_Click(object sender, EventArgs e)
      {
         mbTimeSnap = TimeSnapButton.Checked;
      }

      private void ValueSnapButton_Click(object sender, EventArgs e)
      {
         mbValueSnap = ValueSnapButton.Checked;
      }

      private void BitmapPictureBox_Click(object sender, EventArgs e)
      {
         BitmapPictureBox.Select();
      }

      private void UpdateSplinesEnabled()
      {
         // Update all splines
         for (int i = 0; i < GetSplineCount(); i++)
         {
            // Lookup spline
            Spline S = GetSpline(i);

            // Update
            bool bChecked = SplineListBox.GetItemChecked(i);
            S.Enabled = bChecked;
         }

         // Redraw
         UpdateGraphics(true);
      }

      private void OnSplineListBoxSelectedValueChanged(object sender, EventArgs e)
      {
         // Update all splines
         UpdateSplinesEnabled();
      }

      private void OnSplineListBoxMouseDoubleClick(object sender, MouseEventArgs e)
      {
         // Update all splines
         UpdateSplinesEnabled();
      }

      private void SetSplineSelectedKeysTangentType(Spline.ETangentType Type)
      {
         // Update all splines
         for (int i = 0; i < GetSplineCount(); i++)
         {
            // Lookup spline
            Spline S = GetSpline(i);
            if (S.Enabled)
            {
               // Check all keys
               foreach(XmlElement key in S.Keys)
               {
                  // Update key tangents?
                  if (S.GetKeySelected(key))
                  {
                     // setting a spline IN tangent is an invalid thing to do.
                     if( Type != Spline.ETangentType.Step)
                        S.SetInTangentType(key, Type);

                     S.SetOutTangentType(key, Type);
                  }
               }
            }
         }

         // Refresh
         UpdateGraphics(true);
      }

      private void DeleteSelectedKeys()
      {
         for (int i = 0; i < GetSplineCount(); i++)
         {
            // Lookup spline
            Spline S = GetSpline(i);
            if (S.Enabled)
            {
               List<XmlElement> keys = S.Keys;
               for (int keyIndex = keys.Count - 1; keyIndex >= 0; keyIndex--)
               {
                  XmlElement key = keys[keyIndex] as XmlElement;

                  // Update key tangents?
                  if (S.GetKeySelected(key))
                  {
                     S.DeleteKey(key);
                  }
               }
            }
         }
         
         UpdateUIForSelectedKeys();
         UpdateGraphics(true);
      }


      private void TangentFixedButton_OnClick(object sender, EventArgs e)
      {
         mTangentType = Spline.ETangentType.Fixed;
         SetSplineSelectedKeysTangentType(mTangentType);
         UpdateTangentToolbarButtons();
      }

      private void TangentClampedButton_OnClick(object sender, EventArgs e)
      {
         mTangentType = Spline.ETangentType.Clamped;
         SetSplineSelectedKeysTangentType(mTangentType);
         UpdateTangentToolbarButtons();
      }

      private void TangentLinearButton_OnClick(object sender, EventArgs e)
      {
         mTangentType = Spline.ETangentType.Linear;
         SetSplineSelectedKeysTangentType(mTangentType);
         UpdateTangentToolbarButtons();
      }

      private void TangentFlatButton_OnClick(object sender, EventArgs e)
      {
         mTangentType = Spline.ETangentType.Flat;
         SetSplineSelectedKeysTangentType(mTangentType);
         UpdateTangentToolbarButtons();
      }

      private void TangentStepButton_OnClick(object sender, EventArgs e)
      {
         mTangentType = Spline.ETangentType.Step;
         SetSplineSelectedKeysTangentType(mTangentType);
         UpdateTangentToolbarButtons();
      }

      private void TangentPlateauButton_OnClick(object sender, EventArgs e)
      {
         mTangentType = Spline.ETangentType.Plateau;
         SetSplineSelectedKeysTangentType(mTangentType);
         UpdateTangentToolbarButtons();
      }

      private void UpdateTangentToolbarButtons()
      {
         TangentFixedButton.Checked = mTangentType == Spline.ETangentType.Fixed;
         TangentLinearButton.Checked = mTangentType == Spline.ETangentType.Linear;
         TangentFlatButton.Checked = mTangentType == Spline.ETangentType.Flat;
         TangentStepButton.Checked = mTangentType == Spline.ETangentType.Step;
         TangentClampedButton.Checked = mTangentType == Spline.ETangentType.Clamped;
         TangentPlateauButton.Checked = mTangentType == Spline.ETangentType.Plateau;
      }

      private void OnKeyDown(object sender, KeyEventArgs e)
      {
         switch(e.KeyCode)
         {
            case Keys.Delete:
               DeleteSelectedKeys();
               break;
            
            case Keys.F:
               {
                  // Update view
                  FrameAllSplines();

                  // Refresh
                  UpdateGraphics(true);
               }
               break;
         }

         UpdateMouseCursor();
      }

      private void OnKeyUp(object sender, KeyEventArgs e)
      {
         UpdateMouseCursor();
      }

      private void OnPreInfinityClick(object sender, EventArgs e)
      {
         ToolStripMenuItem menuItem = sender as ToolStripMenuItem;
         
         foreach(Spline spline in mSplines)
         {
            if( spline.Enabled )
               spline.SetPreInfinity(Spline.GetInfinityTypeFromName((string)menuItem.Tag));
         }

         UpdateGraphics(true);
      }

      private void OnPostInfinityClick(object sender, EventArgs e)
      {
         ToolStripMenuItem menuItem = sender as ToolStripMenuItem;

         foreach (Spline spline in mSplines)
         {
            if (spline.Enabled)
               spline.SetPostInfinity(Spline.GetInfinityTypeFromName((string)menuItem.Tag));
         }
         
         UpdateGraphics(true);
      }
      
      private void UpdateUIForSelectedKeys()
      {
         int selectedKeyCount = 0;

         List<float> selectedKeyTimes = new List<float>();
         List<float> selectedKeyValues = new List<float>();

         float firstSelectedKeyTime = float.MaxValue;
         float firstSelectedKeyValue = 0;

         foreach (Spline spline in mSplines)
         {
            if (spline.Enabled)
            {
               foreach(XmlElement key in spline.Keys)
               {
                  if( spline.GetKeySelected(key) )
                  {
                     selectedKeyCount++;
                     
                     float keyTime = spline.GetKeyTime(key);
                     float keyValue = spline.GetKeyValue(key);

                     selectedKeyTimes.Add(keyTime);
                     selectedKeyValues.Add(keyValue);

                     if (keyTime < firstSelectedKeyTime)
                     {
                        firstSelectedKeyTime = keyTime;
                        firstSelectedKeyValue = keyValue;;
                     }
                  }
               }
            }
         }

         if( selectedKeyCount > 0 )
         {
            TimeTextBox.Text = firstSelectedKeyTime.ToString("F4");
            ValueTextBox.Text = firstSelectedKeyValue.ToString("F4");

            // the time for multiple keys can not be on the same time, so if multiple keys are selected, then the times are different!
            if( selectedKeyCount > 1 )
               TimeTextBox.BackColor = Color.LightGray;
            else
               TimeTextBox.BackColor = SystemColors.Window;

            // for values multiple keys can either have or have not the same value
            selectedKeyValues.Sort();
            
            float firstKeyValue = selectedKeyValues[0];
            float lastKeyValue = selectedKeyValues[selectedKeyValues.Count - 1];

            if( Math.Abs(firstKeyValue - lastKeyValue) > float.Epsilon)
               ValueTextBox.BackColor = Color.LightGray;
            else
               ValueTextBox.BackColor = SystemColors.Window;
         }
         else
         {
            TimeTextBox.Text = "";
            ValueTextBox.Text = "";
         }
      }

      private void OnSetNumericTime(object sender, EventArgs e)
      {
         float time;

         if( float.TryParse(TimeTextBox.Text, out time))
         {
            bool didChange = false;

            foreach (Spline spline in mSplines)
            {
               if (spline.Enabled)
               {
                  XmlElement selectedKey = null;
                  int selectedKeyCount = 0;

                  foreach (XmlElement key in spline.Keys)
                  {
                     if (spline.GetKeySelected(key))
                     {
                        selectedKey = key;
                        selectedKeyCount++;
                     }
                  }

                  if (selectedKeyCount == 1 )
                  {
                     bool hasKeyAtTimeAlready = false;

                     foreach (XmlElement key in spline.Keys)
                     {
                        if( Math.Abs(spline.GetKeyTime(key) - time) < float.Epsilon )
                        {
                           hasKeyAtTimeAlready = true;
                           break;
                        }
                     }

                     if (!hasKeyAtTimeAlready)
                     {
                        spline.SetKeyTime(selectedKey, time);
                        didChange = true;
                     }
                  }
               }
            }

            if (didChange)
            {
               UpdateUIForSelectedKeys();
               UpdateGraphics(true);
            }

         }
      }

      private void OnNumericTimeKeyDown(object sender, KeyEventArgs e)
      {
         if (e.KeyCode == Keys.Return)
         {
            OnSetNumericTime(sender, e);
            TimeTextBox.SelectAll();
         }
      }

      private void OnSetNumericValue(object sender, EventArgs e)
      {
         float value;

         if (float.TryParse(ValueTextBox.Text, out value))
         {
            foreach (Spline spline in mSplines)
            {
               if (spline.Enabled)
               {
                  foreach (XmlElement key in spline.Keys)
                  {
                     if( spline.GetKeySelected(key) )
                        spline.SetKeyValue(key, value);
                  }
               }
            }

            UpdateUIForSelectedKeys();
            UpdateGraphics(true);
         }

      }

      private void OnNumericValueKeyDown(object sender, KeyEventArgs e)
      {
         if( e.KeyCode == Keys.Return)
         {
            OnSetNumericValue(sender, e);
            ValueTextBox.SelectAll();
         }
      }
   }
}
