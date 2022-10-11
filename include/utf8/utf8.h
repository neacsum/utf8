/*!
  \file utf8.h UTF-8 Conversion functions
  (c) Mircea Neacsu 2014-2020

*/
#pragma once
#include <string>
#include <vector>
#include <fstream>

namespace utf8 {

/// Exception thrown on encoding/decoding failure
struct exception : public std::exception
{
  enum reason { invalid_utf8, invalid_char32 };
  exception (reason c)
    : std::exception (
      c == reason::invalid_utf8 ? "Invalid UTF-8 encoding" :
      c == reason::invalid_char32 ? "Invalid code-point value" :
      "Other UTF-8 exception")
    , cause (c)
  {}
  reason cause;
};

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

bool next (std::string::const_iterator& p, const std::string::const_iterator last);
bool next (const char*& p);
bool next (char*& p);

size_t length (const std::string& s);
size_t length (const char* s);


std::vector<std::string> get_argv ();
char** get_argv (int *argc);
void free_argv (int argc, char **argv);

bool mkdir (const char* dirname);
bool mkdir (const std::string& dirname);

bool rmdir (const char* dirname);
bool rmdir (const std::string& dirname);

bool chdir (const char* dirname);
bool chdir (const std::string& dirname);

bool chmod (const char* filename, int mode);
bool chmod (const std::string& filename, int mode);

std::string getcwd ();

bool access (const char* filename, int mode);
bool access (const std::string& filename, int mode);

bool remove (const char* filename);
bool remove (const std::string& filename);

bool rename (const char* oldname, const char* newname);
bool rename (const std::string& oldname, const std::string& newname);

FILE* fopen (const char* filename, const char* mode);
FILE* fopen (const std::string& filename, const std::string& mode);

bool splitpath (const std::string& path, char* drive, char* dir, char* fname, char* ext);
bool splitpath (const std::string& path, std::string& drive, std::string& dir,
                std::string& fname, std::string& ext);
bool makepath (std::string& path, const std::string& drive, const std::string& dir,
                const std::string& fname, const std::string& ext);
std::string fullpath (const std::string& relpath);

std::string getenv (const std::string& var);
bool putenv (const std::string& str);
bool putenv (const std::string& var, const std::string& val);

bool symlink (const char* path, const char* link, bool directory);
bool symlink (const std::string& path, const std::string& target, bool directory);

/*!
  \addtogroup folding
  @{
*/
void tolower (std::string& str);
void toupper (std::string& str);
std::string tolower (const std::string& str);
std::string toupper (const std::string& str);
int icompare (const std::string& s1, const std::string& s2);
/// @}

/*!
  \addtogroup charclass
  @{
*/

bool isspace (const char* p);
bool isspace (std::string::const_iterator p);
bool isblank (const char* p);
bool isblank (std::string::const_iterator p);
bool isdigit (const char* p);
bool isdigit (std::string::const_iterator p);
bool isalnum (const char* p);
bool isalnum (std::string::const_iterator p);
bool isalpha (const char* p);
bool isalpha (std::string::const_iterator p);
bool isxdigit (const char* p);
bool isxdigit (std::string::const_iterator p);
bool isupper (const char* p);
bool isupper (std::string::const_iterator p);
bool islower (const char* p);
bool islower (std::string::const_iterator p);
/// @}

/// Input stream class using UTF-8 filename
class ifstream : public std::ifstream
{
public:
  ifstream () : std::ifstream () {};
  ifstream (const char* filename, std::ios_base::openmode mode = ios_base::in)
    : std::ifstream (utf8::widen (filename), mode) {};
  ifstream (const std::string& filename, std::ios_base::openmode mode = ios_base::in)
    : std::ifstream (utf8::widen (filename), mode) {};
  ifstream (ifstream&& other) noexcept : std::ifstream ((std::ifstream&&)other) {};
  ifstream (const ifstream& rhs) = delete;

  void open (const char* filename, ios_base::openmode mode = ios_base::in,
    int prot = (int)ios_base::_Openprot)
  {
    std::ifstream::open (utf8::widen (filename), mode, prot);
  }
  void open (const std::string& filename, ios_base::openmode mode = ios_base::in,
    int prot = (int)ios_base::_Openprot)
  {
    std::ifstream::open (utf8::widen (filename), mode, prot);
  }
};

/// Output stream class using UTF-8 filename
class ofstream : public std::ofstream
{
public:
  ofstream () : std::ofstream () {};
  ofstream (const char* filename, std::ios_base::openmode mode = ios_base::out)
    : std::ofstream (utf8::widen (filename), mode) {};
  ofstream (const std::string& filename, std::ios_base::openmode mode = ios_base::out)
    : std::ofstream (utf8::widen (filename), mode) {};
  ofstream (ofstream&& other) noexcept : std::ofstream ((std::ofstream&&)other) {};
  ofstream (const ofstream& rhs) = delete;

  void open (const char* filename, ios_base::openmode mode = ios_base::out,
    int prot = (int)ios_base::_Openprot)
  {
    std::ofstream::open (utf8::widen (filename), mode, prot);
  }
  void open (const std::string& filename, ios_base::openmode mode = ios_base::out,
    int prot = (int)ios_base::_Openprot)
  {
    std::ofstream::open (utf8::widen (filename), mode, prot);
  }
};

/// Bidirectional stream class using UTF-8 filename
class fstream : public std::fstream
{
public:
  fstream () : std::fstream () {};
  fstream (const char* filename, std::ios_base::openmode mode = ios_base::in | ios_base::out)
    : std::fstream (utf8::widen (filename), mode) {};
  fstream (const std::string& filename, std::ios_base::openmode mode = ios_base::in | ios_base::out)
    : std::fstream (utf8::widen (filename), mode) {};
  fstream (fstream&& other) noexcept : std::fstream ((std::fstream&&)other) {};
  fstream (const fstream& rhs) = delete;

  void open (const char* filename, ios_base::openmode mode = ios_base::in | ios_base::out,
    int prot = (int)ios_base::_Openprot)
  {
    std::fstream::open (utf8::widen (filename), mode, prot);
  }
  void open (const std::string& filename, ios_base::openmode mode = ios_base::in | ios_base::out,
    int prot = (int)ios_base::_Openprot)
  {
    std::fstream::open (utf8::widen (filename), mode, prot);
  }
};


// INLINES --------------------------------------------------------------------

/*!
  Advances a character pointer to next UTF-8 character

  \param p    <b>Reference</b> to character pointer to be advanced
  \return     `true` if pointer was advanced or `false` otherwise.

  The function throws an exception if iterator points to an invalid UTF-8 encoding.
*/
inline
bool next (char*& p)
{
  return next (const_cast<const char*&>(p));
}


/*!
  Creates a new directory

  \param dirname UTF-8 path for new directory
  \return true if successful, false otherwise
*/
inline
bool mkdir (const char* dirname)
{
  return (_wmkdir (widen (dirname).c_str ()) == 0);
}

/// \copydoc mkdir()
inline
bool mkdir (const std::string& dirname)
{
  return (_wmkdir (widen (dirname).c_str ()) == 0);
}

/*!
  Deletes a directory

  \param dirname UTF-8 path of directory to be removed
  \return true if successful, false otherwise
*/
inline
bool rmdir (const char* dirname)
{
  return (_wrmdir (widen (dirname).c_str ()) == 0);
}

/// \copydoc rmdir()
inline
bool rmdir (const std::string& dirname)
{
  return (_wrmdir (widen (dirname).c_str ()) == 0);
}


/*!
  Changes the current working directory

  \param dirname UTF-8 path of new working directory
  \return true if successful, false otherwise
*/
inline
bool chdir (const char* dirname)
{
  return (_wchdir (widen (dirname).c_str ()) == 0);
}

/// \copydoc chdir()
inline
bool chdir (const std::string& dirname)
{
  return (_wchdir (widen (dirname).c_str ()) == 0);
}

/*!
  Changes the file access permissions

  \param filename UTF-8 name of file
  \param mode access permissions. Or'ed combination of:
              - _S_IWRITE write permission
              - _S_IREAD  read permission

  \return true if successful, false otherwise
*/
inline
bool chmod (const char* filename, int mode)
{
  return (_wchmod (widen (filename).c_str (), mode) == 0);
}

/// \copydoc chmod()
inline
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
inline
bool access (const char* filename, int mode)
{
  return (_waccess (widen (filename).c_str (), mode) == 0);
}

/// \copydoc access()
inline
bool access (const std::string& filename, int mode)
{
  return (_waccess (widen (filename).c_str (), mode) == 0);
}


/*!
  Delete a file

  \param filename UTF-8 name of file to be deleted
  \return true if successful, false otherwise
*/
inline
bool remove (const char* filename)
{
  return (_wremove (widen (filename).c_str ()) == 0);
}

/// \copydoc remove()
inline
bool remove (const std::string& filename)
{
  return (_wremove (widen (filename).c_str ()) == 0);
}

/*!
  Rename a file or directory

  \param oldname current UTF-8 encoded name of file or directory
  \param newname new UTF-8 name
  \return true if successful, false otherwise
*/
inline
bool rename (const char* oldname, const char* newname)
{
  return (_wrename (widen (oldname).c_str (), widen (newname).c_str ()) == 0);
}

/// \copydoc rename()
inline
bool rename (const std::string& oldname, const std::string& newname)
{
  return (_wrename (widen (oldname).c_str (), widen (newname).c_str ()) == 0);
}

/*!
  Open a file

  \param filename UTF-8 encoded file name
  \param mode access mode
  \return pointer to the opened file or NULL if an error occurs
*/
inline
FILE* fopen (const char* filename, const char* mode)
{
  FILE* h = nullptr;
  _wfopen_s (&h, widen (filename).c_str (), widen (mode).c_str ());
  return h;
}

/// \copydoc fopen()
inline
FILE* fopen (const std::string& filename, const std::string& mode)
{
  FILE* h = nullptr;
  _wfopen_s (&h, widen (filename).c_str (), widen (mode).c_str ());
  return h;
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


/// \copydoc isspace()
inline
bool isspace (std::string::const_iterator p)
{
  return isspace (&*p);
}


/*!
  Return true if character is blank or tab
  \param p pointer to character to check
  \return true if character is blank, false otherwise
*/
inline
bool isblank (const char *p)
{
  return (*p == ' ' || *p == '\t');
}

/// \copydoc isblank()
inline
bool isblank (std::string::const_iterator p)
{
  return isblank (&*p);
}

/*!
  Return true if character is a decimal digit (0-9)
  \param p pointer to character to check
  \return true if character is a digit, false otherwise
*/
inline
bool isdigit (const char *p)
{
  char c = *p;
  return '0' <= c && c <= '9';
}

/// \copydoc isdigit()
inline
bool isdigit (std::string::const_iterator p)
{
  return isdigit (&*p);
}

/*!
  Return true if character is an alphanumeric character (0-9 or A-Z or a-z)
  \param p pointer to character to check
  \return true if character is alphanumeric, false otherwise
*/
inline
bool isalnum (const char *p)
{
  char c = *p;
  return ('0' <= c && c <= '9') || ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') ;
}

/// \copydoc isalnum()
inline
bool isalnum (std::string::const_iterator p)
{
  return isalnum (&*p);
}

/*!
  Return true if character is an alphabetic character (A-Z or a-z)
  \param p pointer to character to check
  \return true if character is alphabetic, false otherwise
*/
inline
bool isalpha (const char *p)
{
  char c = *p;
  return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
}

/// \copydoc isalpha()
inline
bool isalpha (std::string::const_iterator p)
{
  return isalpha (&*p);
}

/*!
  Return true if character is a hexadecimal digit (0-9 or A-F or a-f)
  \param p pointer to character to check
  \return true if character is hexadecimal, false otherwise
*/
inline
bool isxdigit (const char *p)
{
  char c = *p;
  return ('0' <= c && c <= '9') || ('A' <= c && c <= 'F') || ('a' <= c && c <= 'f');
}

/// \copydoc isxdigit()
inline
bool isxdigit (std::string::const_iterator p)
{
  return isxdigit (&*p);
}

/// \copydoc isupper()
inline
bool isupper (std::string::const_iterator p)
{
  return isupper (&*p);
}

/// \copydoc islower()
inline
bool islower (std::string::const_iterator p)
{
  return isupper (&*p);
}

/*!
  Creates, modifies, or removes environment variables.
  This is a wrapper for [_wputenv function]
  (https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/putenv-wputenv)

  \param str environment string to modify
  \return true if successful, false otherwise.
*/
inline
bool putenv (const std::string& str)
{
  return (_wputenv (utf8::widen (str).c_str ()) == 0);
}

/*!
  Creates, modifies, or removes environment variables.
  This is a wrapper for [_wputenv_s function]
  https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/putenv-s-wputenv-s

  \param var  name of environment variable
  \param val  new value of environment variable. If empty, the environment
              variable is removed
  \return true if successful, false otherwise
*/
inline
bool putenv (const std::string& var, const std::string& val)
{
  return (_wputenv_s (widen (var).c_str (),
    widen (val).c_str ()) == 0);
}

}; //namespace utf8

#include <utf8/winutf8.h>
#include <utf8/ini.h>


#pragma comment (lib, "utf8")

