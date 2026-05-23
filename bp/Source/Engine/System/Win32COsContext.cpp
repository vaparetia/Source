//----------------------------------------------------------------------------
// Win32COsContext.cpp
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Win32COsContext.h"

#include <windows.h>

//----------------------------------------------------------------------------

COsContext::COsContext()
:  CBaseOsContext()
,  mInstanceHandle(NULL)
,  mMainWindow(NULL)
,  mHostedWindow(NULL)
{
}

//----------------------------------------------------------------------------

std::string CBaseOsContext::GetMachineName()
{
   char computerName[256];
   uint32 bufferSize = sizeof(computerName);
   if (::GetComputerName(computerName, &bufferSize))
   {
      return std::string(computerName);
   }
   return "localhost";
}

//----------------------------------------------------------------------------

std::string CBaseOsContext::GetCurrentUserName()
{
   char userName[256];
   uint32 bufferSize = sizeof(userName);
   if (::GetUserName(userName, &bufferSize))
   {
      return std::string(userName);
   }
   return "unknown";
}

//----------------------------------------------------------------------------

void CBaseOsContext::TerminateProcess()
{
   // Win32 - kill process.
   ::TerminateProcess(GetCurrentProcess(), -1);
}

//----------------------------------------------------------------------------

void CBaseOsContext::SetUnsafeToShutDownFlag( uint32 const flag )
{
   mUnsafeToShutDownFlags |= flag;
}

//----------------------------------------------------------------------------

void CBaseOsContext::ClearUnsafeToShutDownFlag( uint32 const flag )
{
   mUnsafeToShutDownFlags &= (~flag);
}

//----------------------------------------------------------------------------

bool CBaseOsContext::IsSafeToShutDown() const
{
   return mUnsafeToShutDownFlags == 0;
}

//----------------------------------------------------------------------------

bool COsContext::HaveFocus() const
{
   bool bHaveFocus = false;
   if (mHostedWindow != NULL)
   {      
      // Hosted in window, assume Maya
      if(::IsWindow((HWND const)mHostedWindow) )
      {
         // Can't use ::GetFocus as message queue is owned by a different thread
         GUITHREADINFO guiThreadInfo;
         memset(&guiThreadInfo, 0, sizeof(guiThreadInfo));
         guiThreadInfo.cbSize = sizeof(guiThreadInfo);

         if (GetGUIThreadInfo(NULL, &guiThreadInfo))
         {
            HWND const focusedWindow = guiThreadInfo.hwndFocus;

            // Generic code
            {
               BPE_HANDLE parentWindow = mHostedWindow;
               while(parentWindow != NULL)
               {
                  if( focusedWindow == parentWindow )
                  {
                     bHaveFocus = true;
                     break;
                  }
                  parentWindow = ::GetParent((HWND)parentWindow);
               }
            }

            // Maya specific code
            {
               // Find 'focushole' window above us which is a sibling to the panel parent.
               // The focushole window owns the message thread so it's the only one we can use check focus
               // Move up to 'TopLevelPanelLayout', see spy++
               BPE_HANDLE parentWindow = mHostedWindow;
               parentWindow = ::GetParent((HWND const)parentWindow);
               parentWindow = ::GetParent((HWND const)parentWindow);
               HWND const focusHoleWindow = FindWindowEx((HWND const)parentWindow, NULL, NULL, "focushole");
               if ((focusHoleWindow != NULL) && (focusedWindow == focusHoleWindow))
               {
                  bHaveFocus = true;
               }
            }
         }
      }
   }
   else
   {
      // Regular window
      HWND const focusedWindow = ::GetFocus();
      if (mMainWindow == focusedWindow)
      {
         bHaveFocus = true;
      }
   }

   return bHaveFocus;
}

//----------------------------------------------------------------------------

bool COsContext::HostedInWindow() const
{
   bool const bHostedInWindow = (mHostedWindow != NULL);
   return bHostedInWindow;
}

//----------------------------------------------------------------------------

CBaseOsContext::EProfileStatus CBaseOsContext::GetUserProfile()
{
   return kPS_Valid;
}
