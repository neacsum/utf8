/// \file WIN.CPP Wrappers for common Windows functions 

/*
  (c) Mircea Neacsu 2014-2019. Licensed under MIT License.
  See README file for full license terms.
*/
#include <utf8/utf8.h>

using namespace std;
namespace utf8 {

/*!
  Convenience wrapper for Windows [MessageBox]
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

/*!
  Convenience wrapper for Windows [CopyFile]
  (https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-copyfile)
  function.
  \param from Name of the existing file
  \param to Name of the new file
  \param fail_exist
*/
bool CopyFile (const std::string& from, const std::string& to, bool fail_exist)
{
  return ::CopyFileW (widen (from).c_str (), widen (to).c_str (), fail_exist);
}

/*!
  Convenience wrapper for Windows [LoadString]
  (https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-loadstringw)
  function.
  \param hInst module handle of the module containing the string resource
  \param id String identifier
  \return UTF-8 resource string or an empty string if resource doesn't exist
*/
std::string LoadString (HINSTANCE hInst, UINT id)
{
  wchar_t *wptr;
  int ret = ::LoadStringW (hInst, id, (LPWSTR)&wptr, 0);
  return ret? utf8::narrow (wptr, ret) : string();
}


}