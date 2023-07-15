/*
  (c) Mircea Neacsu 2014-2023. Licensed under MIT License.
  See README file for full license terms.
*/

/// \file winutf8.h Wrapper for popular Windows API functions
#pragma once

#include <Windows.h>
#include <string>
#include <ostream>
#include <utf8/utf8.h>

#undef MessageBox
#undef CopyFile
#undef LoadString
#undef ShellExecute
#undef GetTempPath
#undef GetTempFileName
#undef GetFullPathName
#undef GetModuleFileName
#undef RegCreateKey
#undef RegDeleteKey
#undef RegOpenKey
#undef RegSetValue
#undef RegQueryValue
#undef RegGetValue
#undef RegDeleteTree
#undef RegEnumKey
#undef RegDeleteValue
#undef RegEnumValue

namespace utf8 {

int MessageBox (HWND hWnd, const std::string& text, const std::string& caption,
  unsigned int type);
bool CopyFile (const std::string& from, const std::string& to, bool fail_exist);
std::string LoadString (UINT id, HINSTANCE hInst = NULL);

//registry functions
LSTATUS RegCreateKey (HKEY key, const std::string& subkey, HKEY& result,
  DWORD options = REG_OPTION_NON_VOLATILE, REGSAM sam = KEY_ALL_ACCESS,
  const SECURITY_ATTRIBUTES* psa = nullptr, DWORD* disp = nullptr);
LSTATUS RegOpenKey (HKEY key, const std::string& subkey, HKEY& result,
  REGSAM sam = KEY_ALL_ACCESS, bool link = false);
LSTATUS RegDeleteKey (HKEY key, const std::string& subkey, REGSAM sam = 0);
LSTATUS RegDeleteValue (HKEY key, const std::string& value);
LSTATUS RegDeleteTree (HKEY key, const std::string& subkey = std::string ());
LSTATUS RegRenameKey (HKEY key, const std::string& subkey, const std::string& new_name);
LSTATUS RegSetValue (HKEY key, const std::string& value, DWORD type, const void* data, DWORD size);
LSTATUS RegSetValue (HKEY key, const std::string& value, const std::string& data);
LSTATUS RegSetValue (HKEY key, const std::string& value, const std::vector<std::string>& data);
LSTATUS RegQueryValue (HKEY key, const std::string& value, DWORD* type, void* data, DWORD* size);
LSTATUS RegGetValue (HKEY key, const std::string& subkey, const std::string& value,
  DWORD flags, void* data, DWORD* size, DWORD* type = NULL);
LSTATUS RegGetValue (HKEY key, const std::string& subkey, const std::string& value,
  std::string& data, bool expand = false);
LSTATUS RegGetValue (HKEY key, const std::string& subkey, const std::string& value,
  std::vector<std::string>& data);
LSTATUS RegEnumKey (HKEY key, DWORD index, std::string& name, DWORD maxlen = 0, FILETIME* last_write_time = NULL);
LSTATUS RegEnumKey (HKEY key, std::vector<std::string>& names);
LSTATUS RegEnumValue (HKEY key, DWORD index, std::string& value, DWORD maxlen = 0, DWORD* type = 0, void* data = 0, DWORD* data_len = 0);
LSTATUS RegEnumValue (HKEY key, std::vector<std::string>& values);

HINSTANCE ShellExecute (const std::string& file, const std::string& verb = std::string (),
                        const std::string& parameters = std::string (),
                        const std::string& directory = std::string ("."),
                        HWND hWnd = NULL, int show = SW_SHOW);
std::string GetTempPath ();
std::string GetTempFileName (const std::string& path, const std::string& prefix, UINT unique=0);
std::string GetFullPathName (const std::string& rel_path);
bool GetModuleFileName (HMODULE hModule, std::string& filename);

/// File enumeration structure used by find_first() and find_next() functions 
struct find_data {
  find_data ()                        ///< Initializes the structure
    : handle{ INVALID_HANDLE_VALUE }
    , attributes{ 0 }
    , creation_time{ 0, 0 }
    , access_time{ 0,0 }
    , write_time{ 0,0 }
    , size{ 0 }
  {}
  HANDLE   handle;                    ///< search handle
  DWORD    attributes;                ///< file attributes
  FILETIME creation_time;             ///< file creation time
  FILETIME access_time;               ///< file last access time
  FILETIME write_time;                ///< file last write time
  __int64  size;                      ///< file size
  std::string  filename;              ///< file name
  std::string  short_name;            ///< 8.3 file name
};

bool find_first (const std::string& name, find_data& fdat);
bool find_next (find_data& fdat);
void find_close (find_data& fdat);

/*!
  An object - oriented wrapper for find_... functions

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
class file_enumerator : protected find_data
{
public:
  file_enumerator (const std::string& name);
  ~file_enumerator ();
  bool ok () const;
  bool next ();

  operator bool () const;

  find_data::attributes;
  find_data::creation_time;
  find_data::access_time;
  find_data::write_time;
  find_data::size;
  find_data::filename;
  find_data::short_name;
};

/// A simple buffer for caching values returned by Windows API 
class buffer {
public:
  buffer (size_t size_);
  ~buffer ();
  buffer (const buffer& other);
  buffer& operator = (const buffer& rhs);
  buffer& operator = (const std::string& rhs);
  operator wchar_t* ();
  operator std::string () const;
  DWORD size () const;

private:
  wchar_t *ptr;
  size_t sz;
};

inline std::ostream&
operator << (std::ostream& s, const buffer& b)
{
  s << (std::string)b;
  return s;
}


//--------------------- INLINE FUNCTIONS --------------------------------------
/// Constructs a file_enumerator object and tries to locate the first file
inline
file_enumerator::file_enumerator (const std::string& name)
{
  find_first (name, *this);
}

/// Closes the search handle associated with this object
inline
file_enumerator::~file_enumerator ()
{
  if (handle != INVALID_HANDLE_VALUE)
    find_close (*this);
}

/// Return _true_ if a file has been enumerated
inline
bool file_enumerator::ok () const
{
  return (handle != INVALID_HANDLE_VALUE);
}

//! Syntactic sugar for ok() function
//! Return _true_ if a file has been enumerated
inline
file_enumerator::operator bool () const
{
  return ok ();
}

/// Advance the enumerator to next file
inline
bool file_enumerator::next ()
{
  return find_next (*this);
}


/// Return a pointer to buffer
inline
  buffer::operator wchar_t* ()
{
  return ptr;
}

/// Convert buffer to an UTF-8 encoded string
inline
  buffer::operator std::string () const
{
  return utf8::narrow (ptr);
}

/// Return buffer size
inline DWORD
  buffer::size () const
{
  return (DWORD)sz;
}

}

