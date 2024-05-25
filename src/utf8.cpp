/*
  (c) Mircea Neacsu 2014-2023. Licensed under MIT License.
  See README file for full license terms.
*/

/// \file utf8.cpp Basic UTF-8 Conversion functions

#include <utf8/utf8.h>
#include <vector>
#include <cassert>
#include <cstring>

using namespace std;
namespace utf8 {

static thread_local action ermode{action::replace};

/*!
  \param mode new error handling mode
  \return previous error handling mode for this thread
*/
action error_mode (action mode)
{
  auto prev = ermode;
  ermode = mode;
  return prev;
}


static void encode (char32_t c, std::string& s);

inline char32_t throw_or_replace (exception::cause err)
{
  if (ermode == action::except)
    throw exception (err);
  else
    return REPLACEMENT_CHARACTER;
}

/*!
  \defgroup basecvt Narrowing/Widening Functions
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
#if USE_WINDOWS_API
  int nsz;
  if (!s || !(nsz = WideCharToMultiByte (CP_UTF8, 0, s, (nch?(int)nch:-1), 0, 0, 0, 0)))
    return string ();

  string out (nsz, 0);
  WideCharToMultiByte (CP_UTF8, 0, s, -1, &out[0], nsz, 0, 0);
  if (!nch)
    out.resize (nsz - 1); //output is null-terminated
#else
  std::string out;
  if (!nch)
    nch = wcslen (s);

  while (nch)
  {
    unsigned int c = (unsigned int)*s++;
    if (0xDBFF < c && c < 0xDFFF)
      c = throw_or_replace (exception::cause::invalid_wchar); //missing hi-surrogate
    else if (0xD7FF < c && c < 0xDC00)
    {
      //got high surrogate, get the low one now
      if (!nch)
        c = throw_or_replace (exception::cause::invalid_wchar); //missing lo-surrogate
      else
      {
        c -= 0xD800;
        unsigned int cl = (unsigned int)*s++;
        --nch;
        if (cl < 0xDC00 || cl > 0xDFFF)
          c = throw_or_replace (exception::cause::invalid_wchar); //not a lo-surrogate
        else
          c = ((c << 10) | (cl - 0xDC00)) + 0x10000;
      }
    }
    encode (c, out);
    --nch;
  }
#endif
  return out;
}

/*!
  Conversion from wide character to UTF-8

  \param  ws input string
  \return UTF-8 character string
*/
std::string narrow (const std::wstring& ws)
{
#if USE_WINDOWS_API
  size_t nsz = WideCharToMultiByte (CP_UTF8, 0, ws.c_str(), (int)ws.size(), 0, 0, 0, 0);
  if (!nsz)
    return string ();

  string out (nsz, 0);
  WideCharToMultiByte (CP_UTF8, 0, ws.c_str (), (int)ws.size(), &out[0], (int)nsz, 0, 0);
#else
  std::string out;
  auto in = ws.cbegin ();
  while (in != ws.end ())
  {
    unsigned int c = (unsigned int)*in++;
    if (0xDBFF < c && c < 0xDFFF)
      c = throw_or_replace(exception::cause::invalid_wchar); //missing hi-surrogate
    else if (c > 0xD7FF && c < 0xDC00)
    {
      //got high surrogate, get the low one now
      if (in == ws.end ())
        c = throw_or_replace(exception::cause::invalid_wchar); //missing lo-surrogate
      else
      {
        c -= 0xD800;
        unsigned int cl = (unsigned int)*in++;
        if (cl < 0xDC00 || cl > 0xDFFF)
          c = throw_or_replace(exception::cause::invalid_wchar); //not a lo-surrogate
        else
          c = ((c << 10) | (cl - 0xDC00)) + 0x10000;
      }
    }
    encode (c, out);
  }
#endif
  return out;
}

/*!
  Conversion from UTF32 to UTF8
  \param s UTF-32 encoded string
  \param  nch number of character to convert or 0 if string is null-terminated
  \return UTF-8 encoded string

  If the input string contains invalid UTF-32 characters, they are replaced with
  utf8::REPLACEMENT_CHARACTER (0xFFFD) or the function throws an exception,
  depending on error handling mode.
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
    encode (*p, str);
  return str;
}

/*!
  Conversion from UTF32 to UTF8
  \param s UTF-32 encoded string
  \return UTF-8 encoded string

  If the input string contains invalid UTF-32 characters, they are replaced with
  utf8::REPLACEMENT_CHARACTER (0xFFFD) or the function throws an exception,
  depending on error handling mode.
*/
std::string narrow (const std::u32string& s)
{
  string str;
  for (auto p = s.begin (); p != s.end (); p++)
    encode (*p, str);
  return str;
}

/*!
  Conversion from UTF32 to UTF8
  \param r UTF-32 encoded character
  \return UTF-8 encoded string

  If the input string contains invalid UTF-32 characters, they are replaced with
  utf8::REPLACEMENT_CHARACTER (0xFFFD) or the function throws an exception,
  depending on error handling mode.
*/
std::string narrow (char32_t r)
{
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
#if USE_WINDOWS_API
  size_t wsz;
  if (!s || !(wsz = MultiByteToWideChar (CP_UTF8, 0, s, (nch?(int)nch:-1), 0, 0)))
    return wstring ();

  wstring out (wsz, 0);
  MultiByteToWideChar (CP_UTF8, 0, s, -1, &out[0], (int)wsz);
  if (!nch)
    out.resize (wsz - 1); //output is null-terminated
#else
  wstring out;

  auto end = s + (nch ? nch : strlen (s));
  while (s < end)
  {
    auto c = next (s);
    if (c < 0x10000)
      out.push_back ((wchar_t)c);
    else
    {
      c -= 0x10000;
      wchar_t sh = (wchar_t)((c >> 10) + 0xD800);
      wchar_t sl = (wchar_t)((c & 0x3FF) + 0xDC00);
      out.push_back (sh);
      out.push_back (sl);
    }
  }
#endif
  return out;
}

/*!
  Conversion from UTF-8 to wide character

  \param  s input string
  \return wide character string
*/
std::wstring widen (const std::string& s)
{
#if USE_WINDOWS_API
  size_t wsz = MultiByteToWideChar (CP_UTF8, 0, s.c_str(), (int)s.size(), 0, 0);
  if (!wsz)
    return wstring ();

  wstring out (wsz, 0);
  MultiByteToWideChar (CP_UTF8, 0, s.c_str (), (int)s.size(), &out[0], (int)wsz);
#else
  wstring out;
  auto in = s.cbegin ();
  while (in != s.end ())
  {
    auto c = next (in, s.end());
    if (c < 0x10000)
      out.push_back ((wchar_t)c);
    else
    {
      c -= 0x10000;
      wchar_t sh = (wchar_t)((c >> 10) + 0xD800);
      wchar_t sl = (wchar_t)((c & 0x3FF) + 0xDC00);
      out.push_back (sh);
      out.push_back (sl);
    }
  }
#endif
  return out;
}

/*!
  Conversion from UTF-8 to UTF-32

  \param s UTF-8 encoded string
  \param nch number of characters to convert or 0 if string is null-terminated
  \return UTF-32 encoded string

  If the input string contains invalid UTF-8 characters, they are replaced with
  utf8::REPLACEMENT_CHARACTER (0xFFFD) or the function throws an exception,
  depending on error handling mode.
*/
std::u32string runes (const char* s, size_t nch)
{
  u32string str;
  if (!nch)
    nch = strlen (s);

  const char* end = s + nch;

  while (s < end)
  {
    char32_t  r = next (s);
    str.push_back (r);
  }
  return str;
}

/*!
  Converts a string of characters from UTF-8 to UTF-32

  \param s UTF-8 encoded string
  \return UTF-32 encoded string

  If the input string contains invalid UTF-8 characters, they are replaced with
  utf8::REPLACEMENT_CHARACTER (0xFFFD) or the function throws an exception,
  depending on error handling mode.
*/
std::u32string runes (const std::string& s)
{
  u32string str;
  auto ptr = s.cbegin ();
  while (ptr != s.cend())
  {
    char32_t r = next (ptr, s.cend());
    str.push_back (r);
  }
  return str;
}


/*!
  Verifies if string is a valid UTF-8 string

  \param s pointer to character string to verify
  \param nch number of characters to verify or 0 if string is null-terminated
  \return `true` if string is a valid UTF-8 encoded string, `false` otherwise
*/
bool valid_str (const char *s, size_t nch)
{
  if (!nch)
    nch = strlen (s);

  auto prev_mode = error_mode (action::replace);
  const char* last = s + nch;
  bool valid = true;
  while (s < last && valid)
    valid = (next (s) != REPLACEMENT_CHARACTER);
  error_mode (prev_mode);
  return (s == last);
}

/*!
  Decodes a UTF-8 encoded character and advances iterator to next code point

  \param ptr    Reference to iterator to be advanced
  \param last   Iterator pointing to the end of range  
  \return       decoded character

  If the iterator points to an invalid UTF-8 encoding or is at end, the function
  throws an exception  or returns utf8::REPLACEMENT_CHARACTER (0xfffd) depending
  on error handling mode. In any case, the iterator is advanced to beginning of
  next character or end of string.
*/
char32_t next (std::string::const_iterator& ptr, const std::string::const_iterator last)
{
  char32_t rune = 0;
  if (ptr == last)
    return throw_or_replace (utf8::exception::invalid_utf8);

  if ((*ptr & 0x80) == 0)
    return *ptr++;
  else if ((*ptr & 0xC0) == 0x80)
  {
    do {
      ++ptr;
    } while (ptr != last && (*ptr & 0x80) == 0x80);
    rune = throw_or_replace (utf8::exception::invalid_utf8);
  }
  else
  {
    size_t cont = 0;
    if ((*ptr & 0xE0) == 0xC0)
    {
      cont = 1;
      rune = *ptr++ & 0x1f;
    }
    else if ((*ptr & 0xF0) == 0xE0)
    {
      cont = 2;
      rune = *ptr++ & 0x0f;
    }
    else if ((*ptr & 0xF8) == 0xF0)
    {
      cont = 3;
      rune = *ptr++ & 0x07;
    }
    else
    {
      //code points > U+0x10FFFF are invalid
      do {
        ++ptr;
      } while (ptr != last && (*ptr & 0xC0) == 0x80);
      return throw_or_replace (utf8::exception::invalid_utf8);
    }
    size_t i;
    for (i=0; i<cont && ptr != last && (*ptr & 0xC0) == 0x80; i++)
    {
      rune <<= 6;
      rune += *ptr++ & 0x3f;
    }

    //sanity checks
    if (i != cont)
    {
      //short encoding
      return throw_or_replace (utf8::exception::invalid_utf8);
    }
    if (0xD800 <= rune && rune <= 0xdfff)
    {
      //surrogates (U+D000 to U+DFFF) are invalid
      return throw_or_replace (utf8::exception::invalid_utf8);
    }

    if (rune < 0x80
      || (cont > 1 && rune < 0x800)
      || (cont > 2 && rune < 0x10000))
    {
      //overlong encoding
      return throw_or_replace (utf8::exception::invalid_utf8);
    }
  }
  return rune;
}

/*!
  Decodes a UTF-8 encoded character and advances pointer to next character

  \param ptr    <b>Reference</b> to character pointer to be advanced
  \return       decoded character

  If the string contains an invalid UTF-8 encoding, the function throws an 
  exception  or returns utf8::REPLACEMENT_CHARACTER (0xfffd) depending on error
  handling mode. In any case, the pointer is advanced to beginning of next
  character or end of string.
*/
char32_t next (const char*& ptr)
{
  char32_t rune = 0;
  if ((*ptr & 0x80) == 0)
  {
    if ((rune = *ptr) != 0)
      ++ptr;
  }
  else if ((*ptr & 0xC0) == 0x80)
  {
    do {
      ptr++;
    } while (*ptr && (*ptr & 0x80) == 0x80);
    return throw_or_replace (utf8::exception::invalid_utf8);
  }
  else
  {
    size_t cont = 0;
    if ((*ptr & 0xE0) == 0xC0)
    {
      cont = 1;
      rune = *ptr++ & 0x1f;
    }
    else if ((*ptr & 0xF0) == 0xE0)
    {
      cont = 2;
      rune = *ptr++ & 0x0f;
    }
    else if ((*ptr & 0xF8) == 0xF0)
    {
      cont = 3;
      rune = *ptr++ & 0x07;
    }
    else
    {
      do {
        ptr++;
      } while (*ptr && (*ptr & 0xC0) == 0x80);
      //code points > U+0x10FFFF are invalid
      return throw_or_replace (utf8::exception::invalid_utf8);
    }
    size_t i;
    for (i = 0; i < cont && (*ptr & 0xC0) == 0x80; i++)
    {
      rune <<= 6;
      rune += *ptr++ & 0x3f;
    }

    //sanity checks
    if (i != cont)
      return throw_or_replace (utf8::exception::invalid_utf8); //short encoding
    if (0xD800 <= rune && rune <= 0xdfff)
    {
      //surrogates (U+D000 to U+DFFF) are invalid
      return throw_or_replace (utf8::exception::invalid_utf8);
    }

    if (rune < 0x80
      || (cont > 1 && rune < 0x800)
      || (cont > 2 && rune < 0x10000))
    {
      //overlong encoding
      return throw_or_replace (utf8::exception::invalid_utf8);
    }
  }
  return rune;
}

/*!
  Decrements a character pointer to previous UTF-8 character

  \param ptr    <b>Reference</b> to character pointer to be decremented
  \return       previous UTF-8 encoded character

  If the string contains an invalid UTF-8 encoding, the function throws an 
  exception  or returns utf8::REPLACEMENT_CHARACTER (0xfffd) depending on error
  handling mode. In this case the pointer remains unchanged.
*/
char32_t prev (const char* & ptr)
{
  int cont = 0;
  const char* in_ptr = ptr;
  char32_t rune = 0;
  unsigned char ch;
  while (((ch = *--ptr) & 0xc0) == 0x80 && cont < 3)
  {
    rune += (char32_t)(ch & 0x3f) << cont++ * 6;
  }
  if (cont == 3 && (ch & 0xF8) == 0xF0)
    rune += (char32_t)(ch & 0x0f) << 18;
  else if (cont == 2 && (ch & 0xF0) == 0xE0)
    rune += (char32_t)(ch & 0x1f) << 12;
  else if (cont == 1 && (ch & 0xE0) == 0xC0)
    rune += (char32_t)(ch & 0x3f) << 6;
  else if (cont == 0 && ch < 0x7f)
    rune += ch;
  else
  {
    ptr = in_ptr;
    return throw_or_replace (utf8::exception::invalid_utf8);
  }

  if ((0xD800 <= rune && rune <= 0xdfff) //surrogate
   || (cont > 0 && rune < 0x80)
   || (cont > 1 && rune < 0x800)
   || (cont > 2 && rune < 0x10000))  //overlong encoding
  {
    ptr = in_ptr;
    return throw_or_replace (utf8::exception::invalid_utf8);
  }

  return rune;
}

/*!
  Decrements an iterator to previous UTF-8 character

  \param ptr    iterator to be decremented
  \param first  iterator pointing to beginning of string
  \return       previous UTF-8 encoded character

  If the string contains an invalid UTF-8 encoding, the function returns
  REPLACEMENT_CHARACTER (0xfffd) and iterator remains unchanged.
*/
char32_t prev (std::string::const_iterator& ptr, const std::string::const_iterator first)
{
  int cont = 0;
  auto in_ptr = ptr;
  char32_t rune = 0;
  unsigned char ch;
  while (((ch = *--ptr) & 0xc0) == 0x80 && cont < 3 && ptr > first)
  {
    rune += (char32_t)(ch & 0x3f) << cont++ * 6;
  }
  if (cont == 3 && (ch & 0xF8) == 0xF0)
    rune += (char32_t)(ch & 0x0f) << 18;
  else if (cont == 2 && (ch & 0xF0) == 0xE0)
    rune += (char32_t)(ch & 0x1f) << 12;
  else if (cont == 1 && (ch & 0xE0) == 0xC0)
    rune += (char32_t)(ch & 0x3f) << 6;
  else if (cont == 0 && ch < 0x7f)
    rune += ch;
  else
  {
    ptr = in_ptr;
    return throw_or_replace (utf8::exception::invalid_utf8);
  }

  if ((0xD800 <= rune && rune <= 0xdfff) //surrogate
   || (cont > 0 && rune < 0x80)
   || (cont > 1 && rune < 0x800)
   || (cont > 2 && rune < 0x10000))  //overlong encoding
  {
    ptr = in_ptr;
    return throw_or_replace (utf8::exception::invalid_utf8);
  }

  return rune;
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
  \defgroup charclass Character Classification Functions
  Replacements for character classification functions.

  According to C standard, the [is...](https://en.cppreference.com/w/cpp/header/cctype)
  family of functions have undefined behavior if the argument is outside the
  range of printable characters. These replacement functions are well-behaved
  for any input string.

  The argument can be a `char32_t` character (rune), or a character pointer or
  a string iterator. Use them as in the following example:
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
  Check if character is space or tab
  \param r character to check
  \return `true` if character is `\t` (0x09) or is in the "Space_Separator" (Zs)
          category, `false` otherwise.

  See [Unicode Character Database](https://www.unicode.org/Public/UCD/latest/ucd/PropList.txt)
  for a list of characters in the Zs (Space_Separator) category. The function adds
  HORIZONTAL_TAB (0x09 or '\\t') to the space separator category for compatibility
  with standard `isblank (char c)` C function.
*/
bool isblank (char32_t r)
{
  const char32_t blanktab[]{ 0x09, 0x20, 0xA0, 0x1680, 0x2000, 0x2001, 0x2002,
    0x2003, 0x2004, 0x2005, 0x2006, 0x2007, 0x2008, 0x2009, 0x200A, 0x202f, 0x205f, 0x3000 };

  auto f = lower_bound (begin (blanktab), end (blanktab), r);
  return (f != end (blanktab) && *f == r);
}

/*!
  Check if character is white space.
  \param r character to check
  \return `true` if character is white space, `false` otherwise

  Returns `true` if Unicode character has the "White_Space=yes" property in the
  [Unicode Character Database](https://www.unicode.org/Public/UCD/latest/ucd/PropList.txt)
*/
bool isspace (char32_t r)
{
  const char32_t spacetab[]{ 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x20, 0x85, 0xA0, 0x1680,
    0x2000, 0x2001, 0x2002, 0x2003, 0x2004, 0x2005, 0x2006, 0x2007, 0x2008, 0x2009, 0x200A,
    0x2028, 0x2029, 0x202f, 0x205f, 0x3000 };

  auto f = lower_bound (begin (spacetab), end (spacetab), r);
  return (f != end (spacetab) && *f == r);
}


// ----------------------- Low level internal functions -----------------------

/// Encode a character and append it to a string
void encode (char32_t c, std::string& s)
{
  if (c <= 0x7f)
    s.push_back ((char)c);
  else if (c <= 0x7ff)
  {
    s.push_back (0xC0 | c >> 6);
    s.push_back (0x80 | c & 0x3f);
  }
  else if (c <= 0xFFFF)
  {
    if (c >= 0xD800 && c <= 0xdfff)
      c= throw_or_replace(exception::cause::invalid_char32);

    s.push_back (0xE0 | c >> 12);
    s.push_back (0x80 | c >> 6 & 0x3f);
    s.push_back (0x80 | c & 0x3f);
  }
  else if (c <= 0x10ffff)
  {
    s.push_back (0xF0 | c >> 18);
    s.push_back (0x80 | c >> 12 & 0x3f);
    s.push_back (0x80 | c >> 6 & 0x3f);
    s.push_back (0x80 | c & 0x3f);
  }
  else if (ermode == action::except)
    throw exception (exception::cause::invalid_char32);
  else
    s.append ("\xEF\xBF\xBD"); //append replacement character
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
    if (e.code == utf8::exception::invalid_utf8) {
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
