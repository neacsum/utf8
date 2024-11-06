/*
  Copyright (c) Mircea Neacsu (2014-2024) Licensed under MIT License.
  This is part of UTF8 project. See LICENSE file for full license terms.
*/

/// \file win.cpp Wrappers for common Windows functions 

#include <utf8/utf8.h>
#include <cassert>
#include <windows.h>

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
bool find_first (const std::string& name, find_data& fdat)
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

/*!
   Closes a "search handle" opened by find_first() function
*/
void find_close (find_data& fdat)
{
  if (fdat.handle != INVALID_HANDLE_VALUE)
  {
    FindClose (fdat.handle);
    fdat.handle = INVALID_HANDLE_VALUE;
  }
}


/*!
  Convenience wrapper for Windows
  [MessageBox](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messagebox)
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
  Convenience wrapper for Windows
  [CopyFile](https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-copyfile)
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
  Convenience wrapper for Windows
  [LoadString](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-loadstringw)
  function.
  \param id String identifier
  \param hInst module handle of the module containing the string resource
  \return UTF-8 resource string or an empty string if resource doesn't exist
*/
std::string LoadString (UINT id, HINSTANCE hInst)
{
  wchar_t *wptr;
  int ret = ::LoadStringW (hInst, id, (LPWSTR)&wptr, 0);
  return ret? utf8::narrow (wptr, ret) : string();
}


/*!
  Convenience wrapper for Windows
  [ShellExecute](https://docs.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shellexecutew)
  function.
  \param file file or object on which to execute the specified verb
  \param verb action to be performed
  \param parameters parameters to be passed to the application
  \param directory default (working) directory
  \param hWnd handle to the parent window
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
  Convenience wrapper for
  [GetTempPath](https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-gettemppathw)

  \return path of the directory designated for temporary files.
*/
std::string GetTempPath ()
{
  wchar_t tmp [_MAX_PATH + 1];
  ::GetTempPathW (_MAX_PATH+1, tmp);
  return narrow(tmp);
}

/*!
  Convenience wrapper for
  [GetTempFileName](https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-gettempfilenamew)

  \param path directory path
  \param prefix filename prefix (up to 3 characters)
  \param unique integer value used to create filename

  \return a filename with structure `<path>\<prefix>nnnnn.tmp`
*/
std::string GetTempFileName (const std::string& path, const std::string& prefix, UINT unique)
{
  auto wpath = utf8::widen (path);
  auto wpfx = utf8::widen (prefix);
  wchar_t fname[_MAX_PATH + 1];
  ::GetTempFileNameW (wpath.c_str (), wpfx.c_str (), unique, fname);

  return utf8::narrow (fname);
}

/*!
  Convenience wrapper for
  [GetFullPathName](https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getfullpathnamew)

  \param rel_path relative path name
  \return full pathname or empty string if there is an error
*/
std::string GetFullPathName (const std::string& rel_path)
{
  auto wrel = widen (rel_path);
  DWORD sz = GetFullPathNameW (wrel.c_str (), 0, 0, 0);
  if (sz)
  {
    buffer wfull (sz);
    GetFullPathNameW (wrel.c_str (), sz, wfull, 0);
    return wfull;
  }
  return std::string ();
}

/*!
  Convenience wrapper for
  [GetModuleFileName](https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulefilenamew)

  \param hModule module handle for which path will be retrieved or NULL to retrieve
                 path of current executable.
  \param filename UTF-8 encoded module file name
  \return `true` if successful `false` otherwise
*/
bool GetModuleFileName (HMODULE hModule, std::string& filename)
{
  wchar_t wfile[_MAX_PATH];
  if (GetModuleFileNameW (hModule, wfile, _countof (wfile)))
  {
    filename = narrow(wfile);
    return true;
  }
  return false;
}

/*!
  Convenience wrapper for
  [GetModuleFileName](https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulefilenamew)

  \param hModule module handle for which path will be retrieved or NULL to retrieve
                 path of current executable.
  \return UTF-8 encoded module file name or empty string if an error occurred
*/
std::string GetModuleFileName (HMODULE hModule)
{
  wchar_t wfile[_MAX_PATH];
  if (GetModuleFileNameW (hModule, wfile, _countof (wfile)))
    return narrow (wfile);

  return std::string ();
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

/// \copydoc utf8::symlink()
bool symlink (const std::string& path, const std::string& link, bool directory)
{
  return CreateSymbolicLinkW (widen (link).c_str (), widen (path).c_str (),
    (directory ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0) | SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE) != 0;
}

/*!
  Breaks a path name into components

  \param path   UTF-8 encoded full path
  \param drive  drive letter followed by colon (or NULL if not needed)
  \param dir    directory path (or NULL if not needed)
  \param fname  base filename (or NULL if not needed)
  \param ext    file extension including the leading period (.)
                (or NULL if not needed)
  \return       true if successful, false otherwise
  Returned strings are converted to UTF-8.
*/
bool splitpath (const std::string& path, char* drive, char* dir, char* fname, char* ext)
{
  wstring wpath = widen (path);
  wchar_t wdrive[_MAX_DRIVE];
  wchar_t wdir[_MAX_DIR];
  wchar_t wfname[_MAX_FNAME];
  wchar_t wext[_MAX_EXT];
  if (_wsplitpath_s (wpath.c_str (), wdrive, wdir, wfname, wext))
    return false;

  if (drive)
    strncpy_s (drive, _MAX_DRIVE, narrow (wdrive).c_str (), _MAX_DRIVE - 1);
  if (dir)
    strncpy_s (dir, _MAX_DIR, narrow (wdir).c_str (), _MAX_DIR - 1);
  if (fname)
    strncpy_s (fname, _MAX_FNAME, narrow (wfname).c_str (), _MAX_FNAME - 1);
  if (ext)
    strncpy_s (ext, _MAX_EXT, narrow (wext).c_str (), _MAX_EXT - 1);

  return true;
}

/*!
  Breaks a path name into components

  \param path   UTF-8 encoded full path
  \param drive  drive letter followed by colon
  \param dir    directory path
  \param fname  base filename
  \param ext    file extension including the leading period (.)

  Returned strings are converted to UTF-8.
*/
bool splitpath (const std::string& path, std::string& drive, std::string& dir, std::string& fname, std::string& ext)
{
  wstring wpath = widen (path);
  wchar_t wdrive[_MAX_DRIVE];
  wchar_t wdir[_MAX_DIR];
  wchar_t wfname[_MAX_FNAME];
  wchar_t wext[_MAX_EXT];

  if (_wsplitpath_s (wpath.c_str (), wdrive, wdir, wfname, wext))
    return false;

  drive = narrow (wdrive);
  dir = narrow (wdir);
  fname = narrow (wfname);
  ext = narrow (wext);
  return true;
}

/*!
  Creates a path from UTF-8 encoded components.

  \param path   Resulting path (UTF-8 encoded)
  \param drive  drive letter
  \param dir    directory path
  \param fname  filename
  \param ext    extension
  \return       True if successful; false otherwise

  If any required syntactic element (colon after drive letter, '\' at end of
  directory path, colon before extension) is missing, it is automatically added.
*/
bool makepath (std::string& path, const std::string& drive, const std::string& dir,
  const std::string& fname, const std::string& ext)
{
  wchar_t wpath[_MAX_PATH];
  if (_wmakepath_s (wpath, widen (drive).c_str (), widen (dir).c_str (), widen (fname).c_str (), widen (ext).c_str ()))
    return false;

  path = narrow (wpath);
  return true;
}

/*!
  Returns the absolute (full) path of a filename
  \param relpath relative path
*/
std::string fullpath (const std::string& relpath)
{
  wchar_t wpath[_MAX_PATH];
  if (_wfullpath (wpath, widen (relpath).c_str (), _MAX_PATH))
    return narrow (wpath);
  else
    return std::string ();
}

/*!
  Retrieves the value of an environment variable
  \param  var name of environment variable
  \return value of environment variable or an empty string if there is no such
          environment variable
*/
std::string getenv (const std::string& var)
{
  size_t nsz;
  wstring wvar = widen (var);
  _wgetenv_s (&nsz, 0, 0, wvar.c_str ());
  if (!nsz)
    return string ();

  wstring wval (nsz, L'\0');
  _wgetenv_s (&nsz, &wval[0], nsz, wvar.c_str ());
  wval.resize (nsz - 1);
  return narrow (wval);
}

/*!
  Converts wide byte command arguments to an array of pointers
  to UTF-8 strings.

  \param  argc Pointer to an integer that contains number of parameters
  \return array of pointers to each command line parameter or NULL if an error
  occurred.

  The space allocated for strings and array of pointers should be freed
  by calling free_utf8argv()
*/
char** get_argv (int* argc)
{
  char** uargv = nullptr;
  wchar_t** wargv = CommandLineToArgvW (GetCommandLineW (), argc);
  if (wargv)
  {
    uargv = new char* [*argc];
    for (int i = 0; i < *argc; i++)
    {
      int nc = WideCharToMultiByte (CP_UTF8, 0, wargv[i], -1, 0, 0, 0, 0);
      uargv[i] = new char[nc + 1];
      WideCharToMultiByte (CP_UTF8, 0, wargv[i], -1, uargv[i], nc, 0, 0);
    }
    LocalFree (wargv);
  }
  return uargv;
}

/*!
  Frees the memory allocated by get_argv(int *argc)

  \param  argc  number of arguments
  \param  argv  array of pointers to arguments
*/
void free_argv (int argc, char** argv)
{
  for (int i = 0; i < argc; i++)
    delete argv[i];
  delete argv;
}

/*!
  Converts wide byte command arguments to UTF-8 to a vector of UTF-8 strings.

  \return vector of UTF-8 strings. The vector is empty if an error occurred.
*/
std::vector<std::string> get_argv ()
{
  int argc;
  vector<string> uargv;

  wchar_t** wargv = CommandLineToArgvW (GetCommandLineW (), &argc);
  if (wargv)
  {
    for (int i = 0; i < argc; i++)
      uargv.push_back (narrow (wargv[i]));
    LocalFree (wargv);
  }
  return uargv;
}


//=============================================================================
/*! 
  \defgroup reg Registry Functions
  Wrappers for Windows registry functions.

  For all these functions wide character strings arguments are replaced
  with UTF-8 encoded C++ strings.
@{ 
*/

/*!
  Convenience wrapper for
  [RegCreateKeyEx](https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regcreatekeyexw)

  \param key    handle to an open registry key
  \param subkey name of a subkey that this function opens or creates
  \param result newly created key handle
  \param options option flags
  \param sam    access rights for the new key
  \param psa    pointer to SECURITY_ATTRIBUTES structure (can be NULL)
  \param disp   pointer to a flag showing if key was created (REG_CREATED_NEW_KEY)
                or opened (REG_OPENED_EXISTING_KEY). Can be NULL.
*/
LSTATUS RegCreateKey (HKEY key, const std::string& subkey, HKEY& result, DWORD options, REGSAM sam, const SECURITY_ATTRIBUTES* psa, DWORD* disp)
{
  auto wsubkey = widen (subkey);
  return RegCreateKeyExW (key, wsubkey.c_str (), 0, NULL, options, sam, (LPSECURITY_ATTRIBUTES)psa, &result, disp);
}

/*!
  Wrapper for
  [RegOpenKeyEx](https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regopenkeyexw)

  \param key    handle to an open registry key
  \param subkey name of a subkey that this function opens
  \param result opened key handle
  \param sam    access rights for opened key
  \param link   set to `true` if key is a symbolic link
*/
LSTATUS RegOpenKey (HKEY key, const std::string& subkey, HKEY& result, REGSAM sam, bool link)
{
  auto wsubkey = widen (subkey);
  return RegOpenKeyExW (key, wsubkey.c_str (), (link? REG_OPTION_OPEN_LINK : 0), sam, &result);
}

/*!
  Wrapper for
  [RegDeleteKeyEx](https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regdeletekeyexa)
  or [RegDeleteKey](https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regdeletekeyw)
  \param key    handle to an open registry key
  \param subkey name of a subkey to delete
  \param sam    access mask specifies the platform-specific view of the registry:
    - `KEY_WOW64_32KEY` - Delete from 32-bit view of the registry
    - `KEY_WOW64_64KEY` - Delete from 64-bit view of the registry

  If `sam` is 0, this function invokes
  [RegDeleteKey](https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regdeletekeyw)
  function, deleting the 32-bit key if the invoking application is 32-bit, or
  the 64-bit key if the invoking application is 64-bit. Otherwise it invokes the
  [RegDeleteKeyEx](https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regdeletekeyexa)
  function.
*/
LSTATUS RegDeleteKey (HKEY key, const std::string& subkey, REGSAM sam)
{
  auto wsubkey = widen (subkey);
  if (!sam)
    return RegDeleteKeyW (key, wsubkey.c_str ());
  else
    return RegDeleteKeyExW (key, wsubkey.c_str (), sam, 0);
}

/*!
  Wrapper for
  [RegDeleteValue](https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regdeletevaluew)

  \param key    handle to an opened registry key
  \param value  name of the value to delete.

  If `value` is the empty string, the function removes the default key value.
*/
LSTATUS RegDeleteValue (HKEY key, const std::string& value)
{
  auto wvalue = widen (value);
  return RegDeleteValueW (key, wvalue.c_str ());
}

/*!
  Wrapper for
  [RegDeleteTree](https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regdeletetreew)

  \param key    handle to an open registry key
  \param subkey name of a subkey to delete

  If `subkey` is empty, the key and all subkeys and values are deleted.
*/
LSTATUS RegDeleteTree (HKEY key, const std::string& subkey)
{
  if (subkey.empty ())
    return RegDeleteTreeW (key, NULL);

  auto wsubkey = widen (subkey);
  return RegDeleteTreeW (key, wsubkey.c_str ());
}

/*!
  Wrapper for 
  [RegRenameKey](https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regrenamekey)

  \param key    handle to an open registry key
  \param subkey name of a subkey to rename
  \param new_name new subkey name
*/
LSTATUS RegRenameKey (HKEY key, const std::string& subkey, const std::string& new_name)
{
  auto wold = widen (subkey);
  auto wnew = widen (new_name);
  return ::RegRenameKey (key, wold.c_str(), wnew.c_str());
}

/*!
  Wrapper for
  [RegSetValueEx](https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regsetvalueexw)

  \param key    handle to an open registry key
  \param value  name of the value to be set.  If empty, the function sets the
                type and data for the key's unnamed or default value
  \param type   the type of data
  \param data   pointer to data
  \param size   data size (in bytes)
*/
LSTATUS RegSetValue (HKEY key, const std::string& value, DWORD type, const void* data, DWORD size)
{
  auto wvalue = widen (value);
  return RegSetValueExW (key, wvalue.c_str (), 0, type, (const BYTE*)data, size);
}


/*!
  Convenience Wrapper for
  [RegSetValueEx](https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regsetvalueexw)
  for string data.

  \param key    handle to an open registry key
  \param value  name of the value to be set.  If empty, the function sets the
                type and data for the key's unnamed or default value
  \param data   value to be set

  Value type is set to `REG_SZ` and data is appended a terminating NULL character.
*/
LSTATUS RegSetValue (HKEY key, const std::string& value, const std::string& data)
{
  auto wdata = widen (data);
  return RegSetValue (key, value, REG_SZ, wdata.c_str(), (DWORD)(wdata.size()+1)*sizeof(wchar_t));
}


/*!
  Convenience Wrapper for
  [RegSetValueEx](https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regsetvalueexw)
  for vector of string data.

  \param key    handle to an open registry key
  \param value  name of the value to be set. If empty, the function sets the
                type and data for the key's unnamed or default value.
  \param data   value to be set

  Value type is set to `REG_MULTI_SZ` and formatted accordingly, with two
  terminating NULL characters.
*/
LSTATUS RegSetValue (HKEY key, const std::string& value, const std::vector<std::string>& data)
{
  size_t key_size = 0;
  for (auto& s : data)
    key_size += widen(s).size() + 1;

  key_size++; //one last NULL char
  wchar_t *buf = new wchar_t[key_size];
  wchar_t* ptr = buf;
  for (auto& s : data)
  {
    auto ws = widen (s);
    auto sz = ws.size ();
    memcpy (ptr, ws.c_str (), sz*sizeof (wchar_t));
    ptr += sz;
    *ptr++ = 0;
  }
  *ptr++ = 0;
  assert (ptr - buf == key_size);
  auto ret = RegSetValue (key, value, REG_MULTI_SZ, buf, (DWORD)key_size*sizeof(wchar_t));
  delete []buf;
  return ret;
}

/*!
  Wrapper for
  [RegQueryValueEx](https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regqueryvalueexw)

  \param key    handle to an open registry key
  \param value  name of the value to be retrieved. If empty, the function retrieves
                the type and data for the key's unnamed or default value
  \param type   pointer to type of data
  \param data   pointer to data
  \param size   pointer to size data size (in bytes)
*/
LSTATUS RegQueryValue (HKEY key, const std::string& value, DWORD* type, void* data, DWORD* size)
{
  auto wvalue = widen (value);
  return RegQueryValueExW (key, wvalue.c_str (), 0, type, (BYTE*)data, size);
}

/*!
  Wrapper for
  [RegGetValue](https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-reggetvaluew)

  \param key    handle to an open registry key
  \param subkey path of the subkey from which the function retrieves the value
  \param value  name of the value to be retrieved.  If empty, the function retrieves the
                type and data for the key's unnamed or default value
  \param flags  the flags that restrict the data type of value to be queried.
  \param data   pointer to data
  \param size   pointer to size data size (in bytes)
  \param type   pointer to type of data
*/
LSTATUS RegGetValue (HKEY key, const std::string& subkey, const std::string& value, 
  DWORD flags, void* data, DWORD* size, DWORD* type)
{
  auto wsubkey = widen (subkey);
  auto wvalue = widen (value);

  return RegGetValueW (key, wsubkey.c_str (), wvalue.c_str (), flags, type, data, size);
}

/*!
  Retrieves a string from a registry key value using the
  [RegGetValue](https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-reggetvaluew)
  function.

  \param key    handle to an open registry key
  \param subkey path of the subkey from which the function retrieves the value
  \param value  name of the value to be retrieved.  If empty, the function retrieves the
                type and data for the key's unnamed or default value
  \param data   retrieved registry data
  \param expand if false, the function does not expand the REG_EXPAND_SZ strings
*/
LSTATUS RegGetValue (HKEY key, const std::string& subkey, const std::string& value, std::string& data, bool expand)
{
  auto wsubkey = widen (subkey);
  auto wvalue = widen (value);
  DWORD sz = 0;
  const DWORD flags = RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ | (expand ? 0 :RRF_NOEXPAND);
  auto ret = RegGetValueW (key, wsubkey.c_str (), wvalue.c_str (), 
    flags, NULL, NULL, &sz);
  if (ret == ERROR_SUCCESS)
  {
    /* Cannot use a wstring as RegGetValueW seems to return a size one larger
    than the actual data size (maybe to account for a terminating NULL). See also:
    https://stackoverflow.com/questions/29223180/successive-calls-to-reggetvalue-return-two-different-sizes-for-the-same-string
    */
    wchar_t *wdat  = new wchar_t[sz / sizeof (wchar_t)];
    ret = RegGetValueW (key, wsubkey.c_str (), wvalue.c_str (), flags, NULL, 
      wdat, &sz);
    if (ret == ERROR_SUCCESS)
      data = narrow (wdat);
    delete[] wdat;
  }
  return ret;
}

/*!
  Retrieves a vector of strings from a registry key value using the
  [RegGetValue](https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-reggetvaluew)
  function.

  \param key    handle to an open registry key
  \param subkey path of the subkey from which the function retrieves the value
  \param value  name of the value to be retrieved.  If empty, the function retrieves the
                type and data for the key's unnamed or default value
  \param data   retrieved vector of strings
*/
LSTATUS RegGetValue (HKEY key, const std::string& subkey, const std::string& value, std::vector<std::string>& data)
{
  auto wsubkey = widen (subkey);
  auto wvalue = widen (value);
  DWORD sz = 0;
  auto ret = RegGetValueW (key, wsubkey.c_str (), wvalue.c_str (),
    RRF_RT_REG_MULTI_SZ, NULL, NULL, &sz);
  if (ret == ERROR_SUCCESS)
  {
    wchar_t *wdat = new wchar_t[sz/ sizeof (wchar_t)];
    ret = RegGetValueW (key, wsubkey.c_str (), wvalue.c_str (), RRF_RT_REG_MULTI_SZ,
      NULL, wdat, &sz);
    if (ret == ERROR_SUCCESS)
    {
      wchar_t* ptr = wdat;
      while (*ptr)
      {
        data.push_back (narrow (ptr));
        ptr += wcslen (ptr)+1;
      }
    }
    delete []wdat;
  }
  return ret;
}

/*!
  Wrapper for [RegEnumKeyEx](https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regenumkeyexw)

  \param key  handle of an opened registry key
  \param index enumeration index. Set to 0 for first subkey.
  \param name name of enumerated subkey
  \param maxlen maximum length (in characters) of subkey name
  \param last_write_time pointer to variable that receives the last time the
                         subkey was written.

  If `maxlen` parameter is 0, the function uses
  [RegQueryInfoKey](https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regqueryinfokeyw)
  function to determine the maximum length.
*/
LSTATUS RegEnumKey (HKEY key, DWORD index, std::string& name, DWORD maxlen, FILETIME* last_write_time)
{
  LSTATUS ret;
  if (maxlen == 0)
  {
    ret = RegQueryInfoKeyW (key, 0, 0, 0, 0, &maxlen, 0, 0, 0, 0, 0, 0);
    if (ret != ERROR_SUCCESS)
      return ret;
    maxlen++; //for terminating NULL
  }
  wchar_t* wnam = new wchar_t[maxlen];
  ret = RegEnumKeyExW (key, index, wnam, &maxlen, 0, 0, 0, last_write_time);
  if (ret == ERROR_SUCCESS)
    name = narrow (wnam);
  delete[] wnam;
  return ret;
}

/*!
  Enumerate all subkeys of a key

  \param key  handle of an opened registry key
  \param names vector of strings containing names of all enumerated subkeys
  \return ERROR_SUCCESS or a non-zero error code
*/
LSTATUS RegEnumKey (HKEY key, std::vector<std::string>& names)
{
  DWORD maxlen;
  LSTATUS ret = RegQueryInfoKeyW (key, 0, 0, 0, 0, &maxlen, 0, 0, 0, 0, 0, 0);
  if (ret != ERROR_SUCCESS)
    return ret;
  maxlen++; //for terminating NULL
  
  wchar_t* wnam = new wchar_t[maxlen];
  DWORD index = 0;
  names.clear ();
  while (ret == ERROR_SUCCESS)
  {
    DWORD len = maxlen;
    ret = RegEnumKeyExW (key, index++, wnam, &len, 0, 0, 0, 0);
    if (ret)
      break;
    names.push_back (narrow (wnam));
  }
  delete[] wnam;
  if (ret == ERROR_NO_MORE_ITEMS)
    ret= ERROR_SUCCESS;

  return ret;
}

/*!
  Wrapper for [RegEnumValue](https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regenumvaluew)

  \param key  handle of an opened registry key
  \param index enumeration index. Set to 0 for first subkey.
  \param value name of enumerated value
  \param maxlen maximum length (in characters) of value name
  \param type pointer to type of data stored in the value
  \param data pointer to data stored in that value
  \param data_len pointer to size of data buffer in bytes

  If `maxlen` parameter is 0, the function uses
  [RegQueryInfoKey](https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regqueryinfokeyw)
  function to determine the maximum length of value name.

  `data_len` parameter can be NULL only if `data` parameter is also NULL.
*/
LSTATUS RegEnumValue (HKEY key, DWORD index, std::string& value, DWORD maxlen,
  DWORD* type, void* data, DWORD* data_len)
{
  LSTATUS ret;
  if (maxlen == 0)
  {
    ret = RegQueryInfoKeyW (key, 0, 0, 0, 0, 0, 0, 0, &maxlen, 0, 0, 0);
    if (ret != ERROR_SUCCESS)
      return ret;
    maxlen++; //for terminating NULL
  }
  wchar_t* wval = new wchar_t[maxlen];
  ret = RegEnumValueW (key, index, wval, &maxlen, 0, type, (BYTE*)data, data_len);
  if (ret == ERROR_SUCCESS)
    value = narrow (wval);
  delete[] wval;
  return ret;
}

/*!
  Enumerate all values of a key

  \param key  handle of an opened registry key
  \param values vector of strings containing the names of all enumerated values
  \return ERROR_SUCCESS or a non-zero error code
*/
LSTATUS RegEnumValue (HKEY key, std::vector<std::string>& values)
{
  DWORD maxlen;
  LSTATUS ret = RegQueryInfoKeyW (key, 0, 0, 0, 0, 0, 0, 0, &maxlen, 0, 0, 0);
  if (ret != ERROR_SUCCESS)
    return ret;
  maxlen++; //for terminating NULL

  wchar_t* wval = new wchar_t[maxlen];
  DWORD index = 0;
  values.clear ();
  while (ret == ERROR_SUCCESS)
  {
    DWORD len = maxlen;
    ret = RegEnumValueW (key, index++, wval, &len, 0, 0, 0, 0);
    if (ret != ERROR_SUCCESS)
      break;
    values.push_back (narrow (wval));
  }

  delete[] wval;
  if (ret == ERROR_NO_MORE_ITEMS)
    ret = ERROR_SUCCESS;

  return ret;
}


} //end namespace utf8