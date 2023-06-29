/*
  (c) Mircea Neacsu 2014-2023. Licensed under MIT License.
  See README file for full license terms.
*/

/// \file utf8.cpp Basic UTF-8 Conversion functions

#include <windows.h>
#include <sys/stat.h>
#include <utf8/utf8.h>
#include <vector>
#include <assert.h>

using namespace std;
namespace utf8 {

static void encode (char32_t c, std::string& s);
static char32_t decode (char const*& p, size_t* len);
static int cont_bytes (char c);

/// Replacement character used for invalid encodings
const char32_t REPLACEMENT_CHARACTER = 0xfffd;

/*!
  \defgroup basecvt Narowing/Widening Functions
  Basic conversion functions between UTF-8, UTF-16 and UTF-32
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
  size_t nsz = WideCharToMultiByte (CP_UTF8, 0, s.c_str(), (int)s.size(), 0, 0, 0, 0);
  if (!nsz)
    return string ();

  string out (nsz, 0);
  WideCharToMultiByte (CP_UTF8, 0, s.c_str (), (int)s.size(), &out[0], (int)nsz, 0, 0);
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
    encode (*p, str);
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
    encode (*p, str);
  }
  return str;
}

/*!
  Conversion from UTF32 to UTF8
  \param r UTF-32 encoded character
  \return UTF-8 encoded string

  Input parameter must be a valid UTF-32 code point
  ( <0x10FFFF)
*/
std::string narrow (char32_t r)
{
  assert (r < 0x10ffff);
  string str;
  encode (r, str);
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
  size_t wsz;
  if (!s || !(wsz = MultiByteToWideChar (CP_UTF8, 0, s, (nch?(int)nch:-1), 0, 0)))
    return wstring ();

  wstring out (wsz, 0);
  MultiByteToWideChar (CP_UTF8, 0, s, -1, &out[0], (int)wsz);
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
  size_t wsz = MultiByteToWideChar (CP_UTF8, 0, s.c_str(), (int)s.size(), 0, 0);
  if (!wsz)
    return wstring ();

  wstring out (wsz, 0);
  MultiByteToWideChar (CP_UTF8, 0, s.c_str (), (int)s.size(), &out[0], (int)wsz);
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
    char32_t  r = decode (s, &nch);
    if (r == REPLACEMENT_CHARACTER)
      throw exception (exception::reason::invalid_utf8);
    str.push_back (r);
  }
  return str;
}

/*!
  Converts a string of characters from UTF-8 to UTF-32

  \param s UTF-8 encoded string
  \return UTF-32 encoded string
*/
std::u32string runes (const std::string& s)
{
  u32string str;
  size_t len = s.length ();
  const char* p = s.data ();
  while (len)
  {
    char32_t r = decode (p, &len);
    if (r == REPLACEMENT_CHARACTER)
      throw exception (exception::reason::invalid_utf8);
    str.push_back (r);
  }
  return str;
}

/*!
  Conversion from UTF-8 to UTF-32

  \param p pointer to character
  \return UTF-32 encoded character
*/
char32_t rune (const char* p)
{
  char32_t r = decode (p, nullptr);
  if (r == REPLACEMENT_CHARACTER)
    throw exception (exception::reason::invalid_utf8);
  return r;
}

/*!
  Check if pointer points to a valid UTF-8 encoding
  \param p pointer to string
  \return `true` if there is a valid UTF-8 encoding at the current pointer position,
          `false` otherwise.

*/
bool is_valid (const char* p)
{
  return decode (p, nullptr) != REPLACEMENT_CHARACTER;
}

/*!
  Check if iterator points to a valid UTF-8 encoding
  \param p    Iterator
  \param last Iterator pointing to end of range
  \return `true` if there is a valid UTF-8 encoding at the current iterator position,
          `false` otherwise.
*/
bool is_valid (std::string::const_iterator p, const std::string::const_iterator last)
{
  size_t len = last - p;
  const char* ptr = &(*p);
  return decode (ptr, &len) != REPLACEMENT_CHARACTER;
}

/*!
  Verifies if string is a valid UTF-8 string

  \param s pointer to character string to verify
  \param nch number of characters to verify or 0 if string is null-terminated
  \return `true` if string is a valid UTF-8 encoded string, `false` otherwise
*/
bool valid_str (const char *s, size_t nch)
{
  int cont = 0;
  if (!nch)
    nch = strlen (s);

  while (nch)
  {
    if (decode (s, &nch) == REPLACEMENT_CHARACTER)
      return false;
  }
  return true;
}

/*!
  Advances a string iterator to next code point

  \param p    Iterator to be advanced
  \param last Iterator pointing to the end of range  
  \return     `true` if iterator was advanced or `false` otherwise.

  The function throws an exception if iterator points to an invalid UTF-8 encoding.
*/
bool next (std::string::const_iterator& p, const std::string::const_iterator last)
{
  int cont = 0;
  if (p == last)
    return false;    //don't advance past end
  size_t len = last - p;
  const char* ptr = &(*p);
  if (decode (ptr, &len) == REPLACEMENT_CHARACTER)
    throw exception (exception::reason::invalid_utf8);
  p += ptr - &(*p);
  return true;
}

/*!
  Advances a character pointer to next UTF-8 character

  \param p    <b>Reference</b> to character pointer to be advanced
  \return     `true` if pointer was advanced or `false` otherwise.

  The function throws an exception if iterator points to an invalid UTF-8 encoding.
*/
bool next (const char*& p)
{
  int cont = 0;
  if (*p == 0)
    return false;    //don't advance past end

  if (decode (p, nullptr) == REPLACEMENT_CHARACTER)
    throw exception (exception::reason::invalid_utf8);

  return true;
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

/// \copydoc utf8::length()
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

/*!
  \defgroup charclass Character Classification Functions
  Replacements for character classification functions.

  According to C standard, the [is...](https://en.cppreference.com/w/cpp/header/cctype)
  family of functions have undefined behavior if the argument is outside the
  range of printable characters. These replacement functions are well-behaved
  for any input string.

  The argument is a character pointer or a string iterator. Use them as in the
  following example:
\code
  //skip spaces in UTF-8 string
  string s{ u8" \xA0日本語" };
  auto p = s.begin ();
  int blanks = 0;
  while (p != s.end () && utf8::isspace (p))
  {
    blanks++;
    utf8::next (p, s.end ());
  }
  assert (blanks == 2); //both space and "no-break space" are space characters
  //...
\endcode

*/

/*!
  Return true if character is blank(-ish).
  \param p pointer to character to check
  \return `true` if character is blank, `false` otherwise

  Returns `true` if Unicode character has the "White_Space=yes" property in the
  [Unicode Character Database](https://www.unicode.org/Public/UCD/latest/ucd/PropList.txt)
*/
bool isspace (const char* p)
{
  const char32_t spacetab[] {0x20, 0x09, 0x0d, 0x0a, 0x0b, 0x0c, 0x85, 0xA0, 0x1680,
    0x2000, 0x2001, 0x2002, 0x2004, 0x2005, 0x2006, 0x2007, 0x2008, 0x2009, 0x200A,
    0x2028, 0x2020, 0x202f, 0x205f, 0x3000};

  char32_t c = rune (p);
  for (auto i = 0; i < _countof (spacetab); i++)
    if (c == spacetab[i])
      return true;

  return false;
}

// ----------------------- Low level internal functions -----------------------

/// Encode a character and append it to a string
void encode (char32_t c, std::string& s)
{
  if (c < 0x7f)
    s.push_back ((char)c);
  else if (c < 0x7ff)
  {
    s.push_back (0xC0 | c >> 6);
    s.push_back (0x80 | c & 0x3f);
  }
  else if (c < 0xFFFF)
  {
    if (c >= 0xD800 && c <= 0xdfff)
      throw exception (exception::reason::invalid_char32);

    s.push_back (0xE0 | c >> 12);
    s.push_back (0x80 | c >> 6 & 0x3f);
    s.push_back (0x80 | c & 0x3f);
  }
  else if (c < 0x10ffff)
  {
    s.push_back (0xF0 | c >> 18);
    s.push_back (0x80 | c >> 12 & 0x3f);
    s.push_back (0x80 | c >> 6 & 0x3f);
    s.push_back (0x80 | c & 0x3f);
  }
  else
    throw exception (exception::reason::invalid_char32);
}

/// Return number of expected continuation bytes
int cont_bytes (char c)
{
  return (c & 0xE0) == 0xC0 ? 1 :
         (c & 0xF0) == 0xE0 ? 2 :
         (c & 0xF8) == 0xF0 ? 3 : 0;
}

/*!
  Decode a UTF-8 codepoint to a UTF-32
  \param ptr reference to UTF-8 character(s). On return, `ptr` points to next
             UTF-8 character
  \param len pointer to length of UTF-8 string. Can be NULL

  \return UTF-32 character or error replacement character (0xfffd) if input
          string is invalid
*/
char32_t decode (char const*& ptr, size_t* len)
{
  char c = *ptr;
  if (!(c & 0x80))
  {
    if (len)
    {
      (*len)--;
      ptr++;
    }
    else if (c)
      ptr++; //for null terminated strings don't advance past end
    return c;
  }
  if ((c & 0xC0) == 0x80)
    return REPLACEMENT_CHARACTER;

  size_t cont;
  char32_t rune = 0;
  if ((c & 0xE0) == 0xC0)
  {
    cont = 1;
    rune = c & 0x1f;
  }
  else if ((c & 0xF0) == 0xE0)
  {
    cont = 2;
    rune = c & 0x0f;
  }
  else if ((c & 0xF8) == 0xf0)
  {
    cont = 3;
    rune = c & 0x07;
  }
  else
    return REPLACEMENT_CHARACTER; //code points > U+0x10FFFF are invalid
  ptr++;
  if (len && --(*len) < cont)
    return REPLACEMENT_CHARACTER; //missing bytes

  for (int i = 0; i < cont; ++i)
  {
    if (len)
      --(*len);

    if ((*ptr & 0x80) != 0x80)
      return REPLACEMENT_CHARACTER; //missing continuation byte(s)
    rune <<= 6;
    rune += *ptr++ & 0x3f;
  }
  if (rune > 0x10ffff)
    return REPLACEMENT_CHARACTER; //code points > U+0x10FFFF or iterator at end

  if (0xD800 <= rune && rune <= 0xdfff)
    return REPLACEMENT_CHARACTER; //surrogates (U+D000 to U+DFFF) are invalid

  if (rune < 0x80
    || (cont > 1 && rune < 0x800)
    || (cont > 2 && rune < 0x10000))
    return REPLACEMENT_CHARACTER; //overlong encoding

  return rune;
}

/*!
  \class exception

  Most UTF8 functions will throw an exception if input string is not a valid
  encoding. So far there are two possible causes:
  - `invalid_utf8` if the string is not a valid UTF-8 encoding
  - `invalid_char32` if the string is not a valid UTF-32 codepoint.

  You can handle a utf8::exception using code like this:
\code

  //...
  catch (utf8::exception& e) {
    if (e.cause == utf8::exception::invalid_utf8) {
      // do something
    }
  }
\endcode
  or you can simply use the exception message:
\code
  //...
  catch (utf8::exception& e) {
    cout << e.what() << endl;
  }
\endcode

*/

} //end namespace
