///  \file FIL.CPP File access functions with UTF-8 encoded arguments

/*
  (c) Mircea Neacsu 2014-2019. Licensed under MIT License.
  See README.md file for full license terms.
*/
#define _CRT_SECURE_NO_WARNINGS

#include <utf8/utf8.h>

using namespace std;
namespace utf8 {

/*!
  Changes the file access permissions

  \param filename UTF-8 name of file
  \param mode access permissions. Or'ed combination of:
              - _S_IWRITE write permission
              - _S_IREAD  read permission

  \return true if successful, false otherwise
*/
bool chmod (const char* filename, int mode)
{
  return (_wchmod (widen (filename).c_str (), mode) == 0);
}

/// \copydoc chmod()
bool chmod (const std::string& filename, int mode)
{
  return (_wchmod (widen (filename).c_str (), mode) == 0);
}


/*!
  Determines if a file has the requested access permissions

  \param filename UTF-8 file path of new working directory
  \param mode required access:
              - 0 existence only
              - 2 write permission
              - 4 read permission
              - 6 read/write permission

  \return true if successful, false otherwise

*/
bool access (const char* filename, int mode)
{
  return (_waccess (widen (filename).c_str (), mode) == 0);
}

/// \copydoc access()
bool access (const std::string& filename, int mode)
{
  return (_waccess (widen (filename).c_str (), mode) == 0);
}


/*!
  Delete a file

  \param filename UTF-8 name of file to be deleted
  \return true if successful, false otherwise
*/
bool remove (const char* filename)
{
  return (_wremove (widen (filename).c_str ()) == 0);
}

/// \copydoc remove()
bool remove (const std::string& filename)
{
  return (_wremove (widen (filename).c_str ()) == 0);
}

///\{
/*!
  Rename a file or directory

  \param oldname current UTF-8 encoded name of file or directory
  \param newname new UTF-8 name
  \return true if successful, false otherwise
*/
bool rename (const char* oldname, const char* newname)
{
  wstring oldn = widen (oldname);
  wstring newn = widen (newname);

  return (_wrename (oldn.c_str (), newn.c_str ()) == 0);
}

/// \copydoc rename()
bool rename (const std::string& oldname, const std::string& newname)
{
  wstring oldn = widen (oldname);
  wstring newn = widen (newname);

  return (_wrename (oldn.c_str (), newn.c_str ()) == 0);
}

/*!
  Open a file

  \param filename UTF-8 encoded file name
  \param mode access mode
  \return pointer to the opened file or NULL if an error occurs
*/
FILE* fopen (const char* filename, const char* mode)
{
  return (_wfopen (widen (filename).c_str (), widen (mode).c_str ()));
}

/// \copydoc fopen()
FILE* fopen (const std::string& filename, const std::string& mode)
{
  return (_wfopen (widen (filename).c_str (), widen (mode).c_str ()));
}

} //namespace utf8