///  \file SPLITPATH.CPP Path splitting functions using UTF-8 encoded filenames

/*
  (c) Mircea Neacsu 2014-2019. Licensed under MIT License.
  See README file for full license terms.
*/
#include <utf8/utf8.h>
#include <windows.h>
#include <assert.h>

using namespace std;
namespace utf8 {

/*!
  Breaks a path name into components

  \param path   UTF-8 encoded full path
  \param drive  drive letter followed by colon (or NULL if not needed)
  \param dir    directory path (or NULL if not needed)
  \param fname  base filename (or NULL if not needed)
  \param ext    file extension including the leading period (.)
                (or NULL if not needed)

  Returned strings are converted to UTF-8.
*/
void splitpath (const std::string& path, char* drive, char* dir, char* fname, char* ext)
{
  wstring wpath = widen (path);
  wchar_t wdrive[_MAX_DRIVE];
  wchar_t wdir[_MAX_DIR];
  wchar_t wfname[_MAX_FNAME];
  wchar_t wext[_MAX_EXT];

  _wsplitpath_s (wpath.c_str (), wdrive, wdir, wfname, wext);
  if (drive)
    strncpy_s (drive, _MAX_DRIVE, narrow (wdrive).c_str (), _MAX_DRIVE - 1);
  if (dir)
    strncpy_s (dir, _MAX_DIR, narrow (wdir).c_str (), _MAX_DIR - 1);
  if (fname)
    strncpy_s (fname, _MAX_FNAME, narrow (wfname).c_str (), _MAX_FNAME - 1);
  if (ext)
    strncpy_s (ext, _MAX_EXT, narrow (wext).c_str (), _MAX_EXT - 1);
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
void splitpath (const std::string& path, std::string& drive, std::string& dir, std::string& fname, std::string& ext)
{
  wstring wpath = widen (path);
  wchar_t wdrive[_MAX_DRIVE];
  wchar_t wdir[_MAX_DIR];
  wchar_t wfname[_MAX_FNAME];
  wchar_t wext[_MAX_EXT];

  _wsplitpath_s (wpath.c_str (), wdrive, wdir, wfname, wext);
  drive = narrow (wdrive);
  dir = narrow (wdir);
  fname = narrow (wfname);
  ext = narrow (wext);
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
  if (0 == _wmakepath_s (wpath, widen (drive).c_str(), widen (dir).c_str (),
                         widen (fname).c_str (), widen (ext).c_str()))
  {
    path = narrow (wpath);
    return true;
  }
  return false;

}

} //end namespace utf8