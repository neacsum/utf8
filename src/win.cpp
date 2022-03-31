/// \file win.cpp Wrappers for common Windows functions 

/*
  (c) Mircea Neacsu 2014-2019. Licensed under MIT License.
  See README file for full license terms.
*/
#include <utf8/utf8.h>

using namespace std;
namespace utf8 {

static void copy_fdat (WIN32_FIND_DATAW& wfd, find_data& fdat)
{
  fdat.attributes = wfd.dwFileAttributes;
  fdat.creation_time = wfd.ftCreationTime;
  fdat.access_time = wfd.ftLastAccessTime;
  fdat.write_time = wfd.ftLastWriteTime;
  fdat.size = ((__int64)wfd.nFileSizeHigh << 32) | (wfd.nFileSizeLow);
  fdat.filename = narrow (wfd.cFileName);
  fdat.short_name = narrow (wfd.cAlternateFileName);
}

/*!
  Searches a directory for a file or subdirectory with a name that matches
  a name (that can have wildcards).

  \param name File name (or partial file name) to find
  \param fdat Information structure containing file name and attributes
  \return _true_ if a file was found or _false_ otherwise.

  \note Wrapper for [FindFirstFile](https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-findfirstfilew)
  Windows API function.

  If successful, the function opens a "search handle" stored in the fdat
  structure. The handle has to be closed using find_close() function.
*/
bool find_first (const string& name, find_data& fdat)
{
  WIN32_FIND_DATAW wfd;
  memset (&wfd, 0, sizeof (wfd));
  fdat.handle = FindFirstFileW (widen (name).c_str (), &wfd);
  if (fdat.handle != INVALID_HANDLE_VALUE)
  {
    copy_fdat (wfd, fdat);
    return true;
  }
  return false;
}

/*!
  Continues a search started by find_first() function.
  \param fdat search results structure containing file information
  \return _true_ if a file was found or _false_ otherwise.

  \note Wrapper for [FindNextFileW](https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-findnextfilew)
  Windows API function.
  
  If there are no more files, the function returns _false_ and GetLastError
  function returns __ERROR_NO_MORE_FILES__
*/
bool find_next (find_data& fdat)
{
  if (fdat.handle == INVALID_HANDLE_VALUE)
    return false;

  WIN32_FIND_DATAW wfd;
  memset (&wfd, 0, sizeof (wfd));

  if (FindNextFileW (fdat.handle, &wfd))
  {
    copy_fdat (wfd, fdat);
    return true;
  }
  else
  {
    FindClose (fdat.handle);
    fdat.handle = INVALID_HANDLE_VALUE;
  }
  return false;
}

/// Closes a "search handle" opened by find_first() function
void find_close (find_data& fdat)
{
  if (fdat.handle != INVALID_HANDLE_VALUE)
  {
    FindClose (fdat.handle);
    fdat.handle = INVALID_HANDLE_VALUE;
  }
}


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


/*!
  Convenience wrapper for Windows [ShellExecute]
  (https://docs.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shellexecutew)
  function.
  \param file file or object on which to execute the specified verb
  \param verb action to be performed
  \param parameters parameters to be passed to the application
  \param directory default (working) directory
  \param hwnd handle to the parent window
  \param show parameter for the ShowWindow function

  \return pseudo instance handle >32 if function succeeds.
*/
HINSTANCE ShellExecute (const std::string& file, const std::string& verb, const std::string& parameters, const std::string& directory, HWND hWnd, int show)
{
  return ShellExecuteW (hWnd,
    (verb.empty () ? NULL : utf8::widen (verb).c_str ()),
    utf8::widen (file).c_str (),
    utf8::widen(parameters).c_str(),
    utf8::widen (directory).c_str (),
    show);
}

/*!
  Create a symbolic link

  \param path target path name
  \param link name of symbolic link
  \param directory `true` if link target is a directory
  \return true if successful, false otherwise
*/
bool symlink (const char* path, const char* link, bool directory)
{
  return CreateSymbolicLinkW (widen (link).c_str (), widen (path).c_str (),
    (directory ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0) | SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE) != 0;
}

/// \copydoc symlink()
bool symlink (const std::string& path, const std::string& link, bool directory)
{
  return CreateSymbolicLinkW (widen (link).c_str (), widen (path).c_str (),
    (directory ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0) | SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE) != 0;
}


/*!
  \class file_enumerator
  This object wraps a Windows search handle used in find_first/find_next
  functions.
  Use like in the following example:
  \code
  utf8::file_enumerator collection("sample.*");
  while (collection.ok())
  {
    cout << collection.filename () << endl;
    collection.next ();
  }
  \endcode
*/


}