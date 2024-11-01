/*
  (c) Mircea Neacsu 2014-2023. Licensed under MIT License.
  See README file for full license terms.
*/

/// \file utf8.h UTF-8 Conversion functions
#pragma once

#include <string>
#include <vector>
#include <fstream>

/*!
  If USE_WINDOWS_API is not zero, the library issues direct Windows API
  calls. Otherwise it relies only on standard C++17 functions.
  If not defined, USE_WINDOWS_API defaults to 1 on Windows platform.
*/

//#define USE_WINDOWS_API 0

#if defined (_WIN32) && !defined (USE_WINDOWS_API)
#define USE_WINDOWS_API 1
#elif !defined (USE_WINDOWS_API)
#define USE_WINDOWS_API 0
#endif

#if !USE_WINDOWS_API
#include <filesystem>

#if (defined(_MSVC_LANG) && _MSVC_LANG < 201703L)                                                  \
  || (!defined(_MSVC_LANG) && (__cplusplus < 201703L))
#error "UTF8 requires c++17 or newer if not using Windows API functions"
#endif

#endif

namespace utf8 {

/// Exception thrown on encoding/decoding failure
struct exception : public std::exception
{
  /// Possible causes
  enum cause { invalid_utf8=1, invalid_wchar, invalid_char32 };

  /// Constructor
  explicit exception (cause c)
    : code (c)
  {}

  /// Exception message
  const char* what() const noexcept
  {
    return (code == cause::invalid_utf8)   ? "Invalid UTF-8 encoding"
         : (code == cause::invalid_wchar)  ? "Invalid UTF-16 encoding"
         : (code == cause::invalid_char32) ? "Invalid code-point value"
         : "Other UTF-8 exception";
  }
  /// Condition that triggered the exception
  cause code;
};

/// Error handling methods
enum action {
  replace, ///< Use replacement character for invalid encodings
  except   ///< Throw an exception on invalid encodings
};

/// Set error handling mode for this thread
action error_mode (action mode);

/// Replacement character used for invalid encodings
const char32_t REPLACEMENT_CHARACTER = 0xfffd;


/// \addtogroup basecvt
/// @{
std::string narrow (const wchar_t* s, size_t nch=0);
std::string narrow (const std::wstring& s);
std::string narrow (const char32_t* s, size_t nch = 0);
std::string narrow (const std::u32string& s);
std::string narrow (char32_t r);

std::wstring widen (const char* s, size_t nch = 0);
std::wstring widen (const std::string& s);
std::u32string runes (const char* s, size_t nch = 0);
std::u32string runes (const std::string& s);

char32_t rune (const char* p);
char32_t rune (const std::string::const_iterator& p);
/// @}

bool is_valid (const char* p);
bool is_valid (std::string::const_iterator p, const std::string::const_iterator last);
bool valid_str (const char* s, size_t nch = 0);
bool valid_str (const std::string& s);

char32_t next (std::string::const_iterator& ptr, const std::string::const_iterator last);
char32_t next (std::string::iterator& ptr, const std::string::const_iterator last);
char32_t next (const char*& ptr);
char32_t next (char*& p);

char32_t prev (const char*& ptr);
char32_t prev (char*& ptr);
char32_t prev (std::string::const_iterator& ptr, const std::string::const_iterator first);
char32_t prev (std::string::iterator& ptr, const std::string::const_iterator first);

size_t length (const std::string& s);
size_t length (const char* s);

/*!
  \addtogroup folding
  @{
*/
void make_lower (std::string& str);
void make_upper (std::string& str);
std::string tolower (const std::string& str);
std::string toupper (const std::string& str);
int icompare (const std::string& s1, const std::string& s2);
/// @}

/*!
  \addtogroup charclass
  @{
*/

bool isspace (char32_t r);
bool isspace (const char* p);
bool isspace (std::string::const_iterator p);

bool isblank (char32_t r);
bool isblank (const char* p);
bool isblank (std::string::const_iterator p);

bool isdigit (char32_t r);
bool isdigit (const char* p);
bool isdigit (std::string::const_iterator p);

bool isalnum (char32_t r);
bool isalnum (const char* p);
bool isalnum (std::string::const_iterator p);

bool isalpha (char32_t r);
bool isalpha (const char* p);
bool isalpha (std::string::const_iterator p);

bool isxdigit (char32_t r);
bool isxdigit (const char* p);
bool isxdigit (std::string::const_iterator p);

bool isupper (char32_t r);
bool isupper (const char* p);
bool isupper (std::string::const_iterator p);

bool islower (char32_t r);
bool islower (const char* p);
bool islower (std::string::const_iterator p);
/// @}

/// Input stream class using UTF-8 filename
#ifdef _WIN32
class ifstream : public std::ifstream
{
public:
  ifstream () : std::ifstream () {};
  explicit ifstream (const char* filename, std::ios_base::openmode mode = ios_base::in)
    : std::ifstream (utf8::widen (filename), mode) {};
  explicit ifstream (const std::string& filename, std::ios_base::openmode mode = ios_base::in)
    : std::ifstream (utf8::widen (filename), mode) {};
  ifstream (ifstream&& other) noexcept : std::ifstream ((std::ifstream&&)other) {};
  ifstream (const ifstream& rhs) = delete;

  void open (const char* filename, std::ios_base::openmode mode = ios_base::in)
  {
    std::ifstream::open (utf8::widen (filename), mode);
  }
  void open (const std::string& filename, ios_base::openmode mode = ios_base::in)
  {
    std::ifstream::open (utf8::widen (filename), mode);
  }
};
/// Output stream class using UTF-8 filename
class ofstream : public std::ofstream
{
public:
  ofstream () : std::ofstream () {};
  explicit ofstream (const char* filename, std::ios_base::openmode mode = ios_base::out)
    : std::ofstream (utf8::widen (filename), mode) {};
  explicit ofstream (const std::string& filename, std::ios_base::openmode mode = ios_base::out)
    : std::ofstream (utf8::widen (filename), mode) {};
  ofstream (ofstream&& other) noexcept : std::ofstream ((std::ofstream&&)other) {};
  ofstream (const ofstream& rhs) = delete;

  void open (const char* filename, ios_base::openmode mode = ios_base::out)
  {
    std::ofstream::open (utf8::widen (filename), mode);
  }
  void open (const std::string& filename, ios_base::openmode mode = ios_base::out)
  {
    std::ofstream::open (utf8::widen (filename), mode);
  }
};

/// Bidirectional stream class using UTF-8 filename
class fstream : public std::fstream
{
public:
  fstream () : std::fstream () {};
  explicit fstream (const char* filename, std::ios_base::openmode mode = ios_base::in | ios_base::out)
    : std::fstream (utf8::widen (filename), mode) {};
  explicit fstream (const std::string& filename, std::ios_base::openmode mode = ios_base::in | ios_base::out)
    : std::fstream (utf8::widen (filename), mode) {};
  fstream (fstream&& other) noexcept : std::fstream ((std::fstream&&)other) {};
  fstream (const fstream& rhs) = delete;

  void open (const char* filename, ios_base::openmode mode = ios_base::in | ios_base::out)
  {
    std::fstream::open (utf8::widen (filename), mode);
  }
  void open (const std::string& filename, ios_base::openmode mode = ios_base::in | ios_base::out)
  {
    std::fstream::open (utf8::widen (filename), mode);
  }
};

#else
//Under Linux file streams already use UTF-8 filenames
typedef std::ifstream ifstream;
typedef std::ofstream ofstream;
typedef std::fstream fstream;
#endif


// INLINES --------------------------------------------------------------------

/*!
  Check if pointer points to a valid UTF-8 encoding
  \param p pointer to string
  \return `true` if there is a valid UTF-8 encoding at the current pointer position,
          `false` otherwise.
*/
inline
bool is_valid (const char* p)
{
  auto prev_mode = error_mode (action::replace);
  bool valid =  (next (p) != REPLACEMENT_CHARACTER);
  error_mode (prev_mode);
  return valid;
}

/*!
  Check if iterator points to a valid UTF-8 encoding
  \param p    Iterator
  \param last Iterator pointing to end of range
  \return `true` if there is a valid UTF-8 encoding at the current iterator position,
          `false` otherwise.
*/
inline
bool is_valid (std::string::const_iterator p, const std::string::const_iterator last)
{
  auto len = last - p;
  auto prev_mode = error_mode (action::replace);
  bool valid = (next (p, last) != REPLACEMENT_CHARACTER);
  error_mode (prev_mode);
  return valid;
}

/// @copydoc char32_t prev (std::string::const_iterator& ptr, const std::string::const_iterator first);
inline
char32_t next (std::string::iterator& ptr, const std::string::const_iterator last)
{
  return next (*(std::string::const_iterator*)(&ptr), last);
}

/// @copydoc char32_t prev (std::string::const_iterator& ptr, const std::string::const_iterator first);
inline
char32_t prev (std::string::iterator& ptr, const std::string::const_iterator first)
{
  return prev (*(std::string::const_iterator*)(&ptr), first);
}

/*!
  Conversion from UTF-8 to UTF-32

  \param p pointer to character
  \return UTF-32 encoded character or utf8::REPLACEMENT_CHARACTER (0xfffd)
          if it is an invalid UTF-8 encoding
*/
inline
char32_t rune (const char* p)
{
  return next (p);
}


/*!
  Decodes a UTF-8 encoded character and advances pointer to next character

  \param ptr    <b>Reference</b> to character pointer to be advanced
  \return     decoded character

  If the string contains an invalid UTF-8 encoding, the function returns
  utf8::REPLACEMENT_CHARACTER (0xfffd) and advances pointer to beginning of
  next character or end of string.
*/
inline
char32_t next (char*& ptr)
{
  return next (const_cast<const char*&>(ptr));
}

/*!
  Decrements a character pointer to previous UTF-8 character

  \param ptr    <b>Reference</b> to character pointer to be decremented
  \return       previous UTF-8 encoded character

  If the string contains an invalid UTF-8 encoding, the function returns
  REPLACEMENT_CHARACTER (0xfffd) and pointer remains unchanged.
*/
inline
char32_t prev (char*& ptr)
{
  return prev (const_cast<const char*&>(ptr));
}


/*!
  Verifies if string is a valid UTF-8 encoded string
  \param s character string to verify
  \return `true` if string is a valid UTF-8 encoded string, `false` otherwise
*/
inline
bool valid_str (const std::string& s)
{
  return valid_str (s.c_str (), s.size());
}

/// @copydoc rune()
inline
char32_t rune (const std::string::const_iterator& p)
{
  return rune (&(*p));
}


/*!
  Return true if character is blank(-ish).
  \param p pointer to character to check
  \return `true` if character is blank, `false` otherwise

  Returns `true` if Unicode character has the "White_Space=yes" property in the
  [Unicode Character Database](https://www.unicode.org/Public/UCD/latest/ucd/PropList.txt)
*/
inline
bool isspace (const char* p)
{
  return isspace (rune (p));
}

/// \copydoc isspace(const char* p)
inline
bool isspace (std::string::const_iterator p)
{
  return isspace (rune(p));
}


/*!
  Check if character is space or tab
  \param p pointer to character to check

  \return `true` if character is `\t` (0x09) or is in the "Space_Separator" (Zs)
          category, `false` otherwise.

  See [Unicode Character Database](https://www.unicode.org/Public/UCD/latest/ucd/PropList.txt)
  for a list of characters in the Zs (Space_Separator) category. The function adds
  HORIZONTAL_TAB (0x09 or '\\t') to the space separator category for compatibility
  with standard `isblank (char c)` C function.
*/
inline
bool isblank (const char *p)
{
  return isblank(rune(p));
}

/// \copydoc isblank(const char* p)
inline
bool isblank (std::string::const_iterator p)
{
  return isblank (rune (p));
}

/*!
  Check if character is a decimal digit (0-9)
  \param r character to check
  \return true if character is a digit, false otherwise
*/
inline
bool isdigit (char32_t r)
{
  return '0' <= r && r <= '9';
}

/*!
  Check if character is a decimal digit (0-9)
  \param p pointer to character to check
  \return `true` if character is a digit, `false` otherwise
*/
inline
bool isdigit (const char *p)
{
  return isdigit (rune (p));
}

/// \copydoc isdigit(const char* p)
inline
bool isdigit (std::string::const_iterator p)
{
  return isdigit (rune (p));
}

/*!
  Check if character is an alphanumeric character (0-9 or A-Z or a-z)
  \param r character to check
  \return `true` if character is alphanumeric, `false` otherwise
*/
inline
bool isalnum (char32_t r)
{
  return ('0' <= r && r <= '9') || ('A' <= r && r <= 'Z') || ('a' <= r && r <= 'z');
}

/*!
  Check if character is an alphanumeric character (0-9 or A-Z or a-z)
  \param p pointer to character to check
  \return `true` if character is alphanumeric, `false` otherwise
*/
inline
bool isalnum (const char *p)
{
  return isalnum (rune (p));
}

/// \copydoc isalnum(const char *p)
inline
bool isalnum (std::string::const_iterator p)
{
  return isalnum (rune (p));
}

/*!
  Check if character is an alphabetic character (A-Z or a-z)
  \param r character to check
  \return `true` if character is alphabetic, `false` otherwise
*/
inline
bool isalpha (char32_t r)
{
  return ('A' <= r && r <= 'Z') || ('a' <= r && r <= 'z');
}

/*!
  Return true if character is an alphabetic character (A-Z or a-z)
  \param p pointer to character to check
  \return true if character is alphabetic, false otherwise
*/
inline
bool isalpha (const char *p)
{
  return isalpha (rune (p));
}

/// \copydoc isalpha(const char *p)
inline
bool isalpha (std::string::const_iterator p)
{
  return isalpha (&*p);
}


/*!
  Check if character is a hexadecimal digit (0-9 or A-F or a-f)
  \param r character to check
  \return `true` if character is hexadecimal, `false` otherwise
*/
inline
bool isxdigit (char32_t r)
{
  return ('0' <= r && r <= '9') || ('A' <= r && r <= 'F') || ('a' <= r && r <= 'f');
}

/*!
  Check if character is a hexadecimal digit (0-9 or A-F or a-f)
  \param p pointer to character to check
  \return `true` if character is hexadecimal, `false` otherwise
*/
inline
bool isxdigit (const char *p)
{
  return isxdigit(rune(p));
}

/// \copydoc isxdigit(const char* p)
inline
bool isxdigit (std::string::const_iterator p)
{
  return isxdigit (rune(p));
}

/// \copydoc isupper(const char* p)
inline
bool isupper (std::string::const_iterator p)
{
  return isupper (rune(p));
}

/// \copydoc islower(const char*p)
inline
bool islower (std::string::const_iterator p)
{
  return islower (rune(p));
}

// File System functions -----------------------------------------------------

/*!
  Open a file

  \param filename UTF-8 encoded file name
  \param mode access mode
  \return pointer to the opened file or NULL if an error occurs
*/
inline
FILE* fopen (const std::string& filename, const std::string& mode)
{
  FILE* h = nullptr;
#ifdef _WIN32
  _wfopen_s (&h, widen (filename).c_str (), widen (mode).c_str ());
#else
  h = ::fopen (filename.c_str(), mode.c_str());
#endif
  return h;
}

/// \copydoc utf8::fopen()
inline
FILE* fopen (const char* filename, const char* mode)
{
  FILE* h = nullptr;
#ifdef _WIN32
  _wfopen_s (&h, widen (filename).c_str (), widen (mode).c_str ());
#else
  h = ::fopen (filename, mode);
#endif
  return h;
}

/*!
  Gets the current working directory
  \return UTF-8 encoded name of working directory
*/
inline
std::string getcwd ()
{
#if USE_WINDOWS_API
  wchar_t tmp[_MAX_PATH];
  if (_wgetcwd (tmp, _countof (tmp)))
    return narrow (tmp);
  else
    return std::string ();
#else
  std::error_code ec;
  std::filesystem::path wd = std::filesystem::current_path (ec);
  if (ec)
    return std::string ();
# ifdef _WIN32
  return narrow (wd.native());
# else
  return wd;
# endif
#endif
}

/*!
  Changes the current working directory

  \param dirname UTF-8 path of new working directory
  \return true if successful, false otherwise
*/
inline
bool chdir (const std::string& dirname)
{
#if USE_WINDOWS_API
  return (_wchdir (widen (dirname).c_str ()) == 0);
#else
# ifdef _WIN32
  std::filesystem::path dir (widen (dirname));
# else
  std::filesystem::path dir (dirname);
# endif
  std::error_code ec;
  std::filesystem::current_path (dir, ec);
  return !ec;
#endif
}

/// \copydoc utf8::chdir ()
inline
bool chdir (const char* dirname)
{
#if USE_WINDOWS_API
  return (_wchdir (widen (dirname).c_str ()) == 0);
#else
# ifdef _WIN32
  std::filesystem::path dir (widen (dirname));
# else
  std::filesystem::path dir (dirname);
# endif
  std::error_code ec;
  std::filesystem::current_path (dir, ec);
  return !ec;
#endif
}


/*!
  Creates a new directory

  \param dirname UTF-8 path for new directory
  \return true if successful, false otherwise
*/
inline
bool mkdir (const std::string& dirname)
{
#if USE_WINDOWS_API
  return (_wmkdir (widen (dirname).c_str ()) == 0);
#else
# ifdef _WIN32
  std::filesystem::path dir (widen (dirname));
# else
  std::filesystem::path dir (dirname);
# endif
  std::error_code ec;
  std::filesystem::create_directory (dir, ec);
  return !ec;
#endif
}


/// \copydoc utf8::mkdir ()
inline
bool mkdir (const char* dirname)
{
#if USE_WINDOWS_API
  return (_wmkdir (widen (dirname).c_str ()) == 0);
#else
# ifdef _WIN32
  std::filesystem::path dir (widen (dirname));
# else
  std::filesystem::path dir (dirname);
# endif
  std::error_code ec;
  std::filesystem::create_directory (dir, ec);
  return !ec;
#endif
}

/*!
  Deletes a directory

  \param dirname UTF-8 path of directory to be removed
  \return true if successful, false otherwise
*/
inline
bool rmdir (const std::string& dirname)
{
#if USE_WINDOWS_API
  return (_wrmdir (widen (dirname).c_str ()) == 0);
#else
# ifdef _WIN32
  std::filesystem::path dir (widen (dirname));
# else
  std::filesystem::path dir (dirname);
# endif
  std::error_code ec;
  std::filesystem::remove (dir, ec);
  return !ec;
#endif
}

/// \copydoc utf8::rmdir ()
inline
bool rmdir (const char* dirname)
{
#if USE_WINDOWS_API
  return (_wrmdir (widen (dirname).c_str ()) == 0);
#else
# ifdef _WIN32
  std::filesystem::path dir (widen (dirname));
# else
  std::filesystem::path dir (dirname);
# endif
  std::error_code ec;
  std::filesystem::remove (dir, ec);
  return !ec;
#endif
}

/*!
  Rename a file or directory

  \param oldname current UTF-8 encoded name of file or directory
  \param newname new UTF-8 name
  \return true if successful, false otherwise
*/
inline
bool rename (const std::string& oldname, const std::string& newname)
{
#if USE_WINDOWS_API
  return (_wrename (widen (oldname).c_str (), widen (newname).c_str ()) == 0);
#else
# ifdef _WIN32
  std::filesystem::path fn (widen (newname));
  std::filesystem::path fo (widen (oldname));
# else
  std::filesystem::path fn (newname);
  std::filesystem::path fo (oldname);
# endif
  std::error_code ec;
  std::filesystem::rename (fo, fn, ec);
  return !ec;
#endif
}

/// \copydoc utf8::rename()
inline 
bool rename (const char* oldname, const char* newname)
{
#if USE_WINDOWS_API
  return (_wrename (widen (oldname).c_str (), widen (newname).c_str ()) == 0);
#else
# ifdef _WIN32
  std::filesystem::path fn (widen (newname));
  std::filesystem::path fo (widen (oldname));
# else
  std::filesystem::path fn (newname);
  std::filesystem::path fo (oldname);
# endif
  std::error_code ec;
  std::filesystem::rename (fo, fn, ec);
  return !ec;
#endif
}

/*!
  Delete a file

  \param filename UTF-8 name of file to be deleted
  \return true if successful, false otherwise
*/
inline
bool remove (const std::string& filename)
{
#if USE_WINDOWS_API
  return (_wremove (widen (filename).c_str ()) == 0);
#else
# ifdef _WIN32
  std::filesystem::path f (widen(filename));
# else
  std::filesystem::path f (filename);
# endif
  std::error_code ec;
  std::filesystem::remove (f, ec);
  return !ec;
#endif
}


/// \copydoc utf8::remove ()
inline
bool remove (const char* filename)
{
#if USE_WINDOWS_API
  return (_wremove (widen (filename).c_str ()) == 0);
#else
# ifdef _WIN32
  std::filesystem::path f (widen (filename));
# else
  std::filesystem::path f (filename);
# endif
  std::error_code ec;
  std::filesystem::remove (f, ec);
  return !ec;
#endif
}

/// Extraction operator for exception objects
inline
std::ostream& operator<<(std::ostream& os, const exception& x)
{
  os << x.what ();
  return os;
}

/// Equality  operator for exception objects
inline
bool operator ==(const exception& lhs, const exception rhs)
{
  return (lhs.code == rhs.code);
}

/// Inequality operator for exception objects
inline
bool operator !=(const exception& lhs, const exception& rhs)
{
  return !operator ==(lhs, rhs);
}

}; //namespace utf8

#ifdef _WIN32
#include <utf8/winutf8.h>
#endif
#include <utf8/ini.h>

#ifdef _MSC_VER
#pragma comment (lib, "utf8")
#endif
