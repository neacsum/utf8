/*!
  \file UTF8.CPP Basic UTF-8 Conversion functions

  These functions facilitate handling of I18N problems using
  the strategy advocated by [UTF-8 Everywhere](http://utf8everywhere.org/)

  \copyright Mircea Neacsu 2014-2019. Licensed under MIT License.
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
  Conversion from wide character to UTF-8

  \param  s input string
  \return UTF-8 character string
*/
std::string narrow (const wchar_t* s)
{
  int nsz;
  if (!s || !(nsz = WideCharToMultiByte (CP_UTF8, 0, s, -1, 0, 0, 0, 0)))
    return string ();

  string out (nsz, 0);
  WideCharToMultiByte (CP_UTF8, 0, s, -1, &out[0], nsz, 0, 0);
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
  Conversion from UTF-8 to wide character

  \param  s input string
  \return wide character string
*/
std::wstring widen (const char* s)
{
  int wsz;
  if (!s || !(wsz = MultiByteToWideChar (CP_UTF8, 0, s, -1, 0, 0)))
    return wstring ();

  wstring out (wsz, 0);
  MultiByteToWideChar (CP_UTF8, 0, s, -1, &out[0], wsz);
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
  Verifies if string is a valid UTF-8 string
  \param s pointer to character string to verify
  \return true if string is a valid UTF-8 encoded string, false otherwise
*/
bool valid (const char *s)
{
  int rem = 0;
  while (*s)
  {
    if (rem)
    {
      if ((*s & 0xC0) != 0x80)
        return false;
      else
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


} //end namespace
