/// \file utf8.cpp Basic UTF-8 Conversion functions

/*
  (c) Mircea Neacsu 2014-2019. Licensed under MIT License.
  See README file for full license terms.
*/

#include <windows.h>
#include <sys/stat.h>
#include <utf8/utf8.h>
#include <vector>
#include <assert.h>

using namespace std;
namespace utf8 {

/*!
  \defgroup basecvt Narowing/Widening Functions
  Basic conversion functions between UTF-8, UTF-16 and UTF-32

  \defgroup charclass Character Classification Functions
  Replacements for character classification functions.

  According to C standard, the [is...](https://en.cppreference.com/w/cpp/header/cctype)
  family of functions have undefined behavior if the argument is outside the
  range of printable characters. These replacement functions are well-behaved
  for any input string.

  The argument is a character pointer or a string iterator. Use them as in the
  following example:
\code
  //skip blanks in UTF-8 string
  string s{ u8"  日本語" };
  auto p = s.begin ();
  while (p != s.end () && utf8::isblank (p))
    utf8::next (p);

  //...
\endcode

*/


/*!
  Conversion from wide character to UTF-8

  \param  s   input string
  \param  nch number of character to convert or 0 if string is null-terminated
  \return UTF-8 character string
*/
std::string narrow (const wchar_t* s, size_t nch)
{
  int nsz;
  if (!s || !(nsz = WideCharToMultiByte (CP_UTF8, 0, s, (nch?(int)nch:-1), 0, 0, 0, 0)))
    return string ();

  string out (nsz, 0);
  WideCharToMultiByte (CP_UTF8, 0, s, -1, &out[0], nsz, 0, 0);
  if (!nch)
    out.resize (nsz - 1); //output is null-terminated
  return out;
}

/*!
  Conversion from wide character to UTF-8

  \param  s input string
  \return UTF-8 character string
*/
std::string narrow (const std::wstring& s)
{
  int nsz = WideCharToMultiByte (CP_UTF8, 0, s.c_str(), -1, 0, 0, 0, 0);
  if (!nsz)
    return string ();

  string out (nsz, 0);
  WideCharToMultiByte (CP_UTF8, 0, s.c_str (), -1, &out[0], nsz, 0, 0);
  out.resize (nsz - 1); //output is null-terminated
  return out;
}

/*!
  Conversion from UTF32 to UTF8
  \param s UTF-32 encoded string
  \param  nch number of character to convert or 0 if string is null-terminated
  \return UTF-8 encoded string

  Each character in the input string should be a valid UTF-32 code point
  ( <0x10FFFF)
*/
std::string narrow (const char32_t* s, size_t nch)
{
  string str;
  const char32_t* p = s;
  if (!nch)
  {
    //null terminated; count characters now
    while (*p++)
      nch++;
    p = s;
  }

  for (; nch; nch--, p++)
  {
    assert (*p < 0x10ffff);

    if (*p < 0x7f)
      str.push_back ((char)*p);
    else if (*p < 0x7ff)
    {
      str.push_back ((char)(0xC0 | *p >> 6));
      str.push_back (0x80 | *p & 0x3f);
    }
    else if (*p < 0xFFFF)
    {
      str.push_back ((char)(0xE0 | *p >> 12));
      str.push_back (0x80 | *p >> 6 & 0x3f);
      str.push_back (0x80 | *p & 0x3f);
    }
    else
    {
      str.push_back ((char)(0xF0 | *p >> 18));
      str.push_back (0x80 | *p >> 12 & 0x3f);
      str.push_back (0x80 | *p >> 6 & 0x3f);
      str.push_back (0x80 | *p & 0x3f);
    }
  }
  return str;
}

/*!
  Conversion from UTF32 to UTF8
  \param s UTF-32 encoded string
  \return UTF-8 encoded string

  Each character in the input string should be a valid UTF-32 code point
  ( <0x10FFFF)
*/
std::string narrow (const std::u32string& s)
{
  string str;
  for (auto p = s.begin (); p != s.end (); p++)
  {
    assert (*p < 0x10ffff);

    if (*p < 0x7f)
      str.push_back ((char)*p);
    else if (*p < 0x7ff)
    {
      str.push_back ((char)(0xC0 | *p >> 6));
      str.push_back (0x80 | *p & 0x3f);
    }
    else if (*p < 0xFFFF)
    {
      str.push_back ((char)(0xE0 | *p >> 12));
      str.push_back (0x80 | *p >> 6 & 0x3f);
      str.push_back (0x80 | *p & 0x3f);
    }
    else
    {
      str.push_back ((char)(0xF0 | *p >> 18));
      str.push_back (0x80 | *p >> 12 & 0x3f);
      str.push_back (0x80 | *p >> 6 & 0x3f);
      str.push_back (0x80 | *p & 0x3f);
    }
  }
  return str;
}

/*!
  Conversion from UTF-8 to wide character

  \param  s input string
  \param nch number of characters to convert or 0 if string is null-terminated
  \return wide character string
*/
std::wstring widen (const char* s, size_t nch)
{
  int wsz;
  if (!s || !(wsz = MultiByteToWideChar (CP_UTF8, 0, s, (nch?(int)nch:-1), 0, 0)))
    return wstring ();

  wstring out (wsz, 0);
  MultiByteToWideChar (CP_UTF8, 0, s, -1, &out[0], wsz);
  if (!nch)
    out.resize (wsz - 1); //output is null-terminated
  return out;
}

/*!
  Conversion from UTF-8 to wide character

  \param  s input string
  \return wide character string
*/
std::wstring widen (const std::string& s)
{
  int wsz = MultiByteToWideChar (CP_UTF8, 0, s.c_str(), -1, 0, 0);
  if (!wsz)
    return wstring ();

  wstring out (wsz, 0);
  MultiByteToWideChar (CP_UTF8, 0, s.c_str (), -1, &out[0], wsz);
  out.resize (wsz - 1); //output is null-terminated
  return out;
}

/*!
  Conversion from UTF-8 to UTF-32

  \param s UTF-8 encoded string
  \param nch number of characters to convert or 0 if string is null-terminated
  \return UTF-32 encoded string
*/
std::u32string runes (const char* s, size_t nch)
{
  u32string str;
  if (!nch)
    nch = strlen (s);

  while (nch)
  {
    str.push_back (rune (s));
    if (!next (s))
      break;
    nch--;
  }
  return str;
}

/*!
  Conversion from UTF-8 to UTF-32

  \param s UTF-8 encoded string
  \return UTF-32 encoded string
*/
std::u32string runes (const std::string& s)
{
  u32string str;
  for (auto p = s.begin (); p != s.end (); next (s, p))
    str.push_back (rune (p));
  return str;
}

/*!
  Return current Unicode code point.
  \param p pointer to character
  \return UTF-32 encoded character
*/
char32_t rune (const char* p)
{
  int rune = 0;
  if ((*p & 0x80) == 0)
  {
    rune = *p;
  }
  else if ((*p & 0xE0) == 0xc0)
  {
    rune = (*p++ & 0x1f) << 6;
    assert ((*p & 0xC0) == 0x80);
    rune += *p & 0x3f;
  }
  else if ((*p & 0xf0) == 0xE0)
  {
    rune = (*p++ & 0x0f) << 12;
    assert ((*p & 0xC0) == 0x80);
    rune += (*p++ & 0x3f) << 6;
    assert ((*p & 0xC0) == 0x80);
    rune += (*p & 0x3f);
  }
  else
  {
    rune = (*p++ & 0x07) << 18;
    assert ((*p & 0xC0) == 0x80);
    rune += (*p++ & 0x3f) << 12;
    assert ((*p & 0xC0) == 0x80);
    rune += (*p++ & 0x3f) << 6;
    assert ((*p & 0xC0) == 0x80);
    rune += (*p & 0x3f);
  }
  return rune;
}

/*!
  Verifies if string is a valid UTF-8 string
  \param s pointer to character string to verify
  \param nch number of characters to verify or 0 if string is null-terminated
  \return true if string is a valid UTF-8 encoded string, false otherwise
*/
bool valid (const char *s, size_t nch)
{
  int rem = 0;
  if (!nch)
    nch = strlen (s);

  while (nch)
  {
    if (rem)
    {
      if ((*s & 0xC0) != 0x80)
        return false;
      rem--;
    }
    else if (*s & 0x80)
    {
      if ((*s & 0xC0) == 0x80)
        return false;

      rem = ((*s & 0xE0) == 0xC0) ? 1 :
        ((*s & 0xF0) == 0xE0) ? 2 : 3;
    }
    s++;
    nch--;
  }
  return !rem;
}

/*!
  Advances a string iterator to next UTF-8 character

  \param s    UTF-8 string
  \param p    Iterator on s string to be advanced
  \return     True if iterator can be advanced or is already at end;
              false if string contains an invalid UTF-8 encoding at iterator's
              position.
*/
bool next (const std::string& s, std::string::const_iterator& p)
{
  int rem = 0;
  if (p == s.end ())
    return true;    //don't advance past end

  do
  {
    if ((*p & 0xC0) == 0x80)
    {
      if (rem)
        rem--;
      else
        return false;   //missing continuation byte
    }
    else if ((*p & 0xE0) == 0xC0)
      rem = 1;
    else if ((*p & 0xF0) == 0xE0)
      rem = 2;
    else if ((*p & 0xF8) == 0xF0)
      rem = 3;
    p++;
  } while (rem && p != s.end ());

  return !rem; // rem == 0 if sequence is complete
}

/*!
  Advances a character pointer to next UTF-8 character

  \param p    <b>Reference</b> to character pointer to be advanced
  \return     True if pointer can be advanced or is already at end;
              false if string contains an invalid UTF-8 encoding at current
              position.
*/
bool next (const char*& p)
{
  int rem = 0;
  if (*p == 0)
    return true;    //don't advance past end

  do
  {
    if ((*p & 0xC0) == 0x80)
    {
      if (rem)
        rem--;
      else
        return false;   //missing continuation byte
    }
    else if ((*p & 0xE0) == 0xC0)
      rem = 1;
    else if ((*p & 0xF0) == 0xE0)
      rem = 2;
    else if ((*p & 0xF8) == 0xF0)
      rem = 3;
    p++;
  } while (rem && *p != 0);

  return !rem; // rem == 0 if sequence is complete
}

bool next (char*& p)
{
  int rem = 0;
  if (*p == 0)
    return true;    //don't advance past end

  do
  {
    if ((*p & 0xC0) == 0x80)
    {
      if (rem)
        rem--;
      else
        return false;   //missing continuation byte
    }
    else if ((*p & 0xE0) == 0xC0)
      rem = 1;
    else if ((*p & 0xF0) == 0xE0)
      rem = 2;
    else if ((*p & 0xF8) == 0xF0)
      rem = 3;
    p++;
  } while (rem && *p != 0);

  return !rem; // rem == 0 if sequence is complete
}

/*!
  Counts number of characters in an UTF8 encoded string

  \param s UTF8-encoded string
  \return number of characters in string

  \note Algorithm from http://canonical.org/~kragen/strlen-utf8.html
*/
size_t length (const std::string& s)
{
  size_t nc = 0;
  auto p = s.begin ();
  while (p != s.end ())
  {
    if ((*p++ & 0xC0) != 0x80)
      nc++;
  }
  return nc;
}

/// \copydoc length()
size_t length (const char* s)
{
  size_t nc = 0;
  while (*s)
  {
    if ((*s++ & 0xC0) != 0x80)
      nc++;
  }
  return nc;
}

/*!
  Gets the current working directory
  \return UTF-8 encoded name of working directory
*/
std::string getcwd ()
{
  wchar_t tmp[_MAX_PATH];
  if (_wgetcwd (tmp, _countof (tmp)))
    return narrow (tmp);
  else
    return string ();
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

} //end namespace
