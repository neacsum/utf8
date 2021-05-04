#pragma once
#include <Windows.h>
#include <string>
#include <ostream>

#undef MessageBox
#undef CopyFile
#undef LoadString

namespace utf8 {

int MessageBox (HWND hWnd, const std::string& text, const std::string& caption,
  unsigned int type);
bool CopyFile (const std::string& from, const std::string& to, bool fail_exist);
std::string LoadString (HINSTANCE hInst, UINT id);

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

/// An object-oriented wrapper for find_... functions
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
  return narrow (ptr);
}

/// Return buffer size
inline DWORD
  buffer::size () const
{
  return (DWORD)sz;
}

}

