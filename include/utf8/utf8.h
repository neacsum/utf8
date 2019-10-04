/*!
  \file UTF8.H UTF-8 Conversion functions
  (c) Mircea Neacsu 2014-2019

*/
#pragma once
#include <string>
#include <vector>
#include <fstream>

namespace utf8 {

std::string narrow (const wchar_t* s);
std::string narrow (const std::wstring& s);
std::string narrow (const std::u32string& s);

std::wstring widen (const char* s);
std::wstring widen (const std::string& s);
std::u32string runes (const std::string& s);

size_t length (const std::string& s);
bool next (const std::string& s, std::string::const_iterator& p);
bool next (const char*& p);

char32_t rune (const std::string::const_iterator& p);
char32_t rune (const char* p);

bool valid (const std::string& s);
bool valid (const char *s);

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

void splitpath (const std::string& path, char* drive, char* dir, char* fname, char* ext);
void splitpath (const std::string& path, std::string& drive, std::string& dir,
                std::string& fname, std::string& ext);
bool makepath (std::string& path, const std::string& drive, const std::string& dir,
                const std::string& fname, const std::string& ext);

std::string getenv (const std::string& var);
bool putenv (const std::string& str);
bool putenv (const std::string& var, const std::string& val);

/// Input stream class using UTF-8 filename
class ifstream : public std::ifstream
{
public:
  ifstream () : std::ifstream () {};
  ifstream (const char* filename, std::ios_base::openmode mode = ios_base::in)
    : std::ifstream (utf8::widen (filename), mode) {};
  ifstream (const std::string& filename, std::ios_base::openmode mode = ios_base::in)
    : std::ifstream (utf8::widen (filename), mode) {};
  ifstream (ifstream&& other) : std::ifstream ((std::ifstream&&)other) {};
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
  ofstream (ofstream&& other) : std::ofstream ((std::ofstream&&)other) {};
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
  fstream (fstream&& other) : std::fstream ((std::fstream&&)other) {};
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

/*!
  Verifies if string is a valid UTF-8 string
  \param s character string to verify
  \return true if string is a valid UTF-8 encoded string, false otherwise
*/
inline
bool valid (const std::string& s)
{
  return valid (s.c_str ());
}

/*!
  \defgroup charclass Character Classification

  Replacements for character classification functions.
*/


/*!
  Return true if character is blank(-ish)
  \param p pointer to character to check
  \return true if character is blank, false otherwise
  \ingroup charclass
*/
inline
bool isblank (const char *p)
{
  return (strchr (" \t\n\r\f\v", *p) != nullptr);
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
  \ingroup charclass
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
  \ingroup charclass
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
  \ingroup charclass
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
  \ingroup charclass
*/
inline
bool isxdigit (const char *p)
{
  char c = *p;
  return ('0' <= c && c <= '9') || ('A' <= c && c <= 'F') || ('a' <= c && c <= 'F');
}

/// \copydoc isxdigit()
inline
bool isxdigit (std::string::const_iterator p)
{
  return isxdigit (&*p);
}

#ifdef _WINDOWS_
#undef MessageBox
int MessageBox (HWND hWnd, const std::string& text, const std::string& caption,
  unsigned int type);
#endif

}; //namespace utf8

#pragma comment (lib, "utf8")

