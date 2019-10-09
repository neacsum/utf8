/// \file WIN.CPP Wrappers for common Windows functions 

/*
  (c) Mircea Neacsu 2014-2019. Licensed under MIT License.
  See README file for full license terms.
*/
#include <Windows.h>
#include <utf8/utf8.h>

using namespace std;
namespace utf8 {

/*!
  Wrapper for [MessageBox]
  (https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messagebox)
  function.
  \param hWnd handle to the owner window
  \param text UTF-8 encoded message to be displayed
  \param caption UTF-8 encoded dialog box title
  \param type flags for content and behavior of the dialog box
*/
int MessageBox (HWND hWnd, const std::string& text, const std::string& caption,
  unsigned int type)
{
  return ::MessageBoxW (hWnd, widen (text).c_str (), widen (caption).c_str (), type);
}

}