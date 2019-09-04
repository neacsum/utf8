#include <utf8/utf8.h>
#include <assert.h>
#include <string>

// Functions for UTF-32 (runes) conversions

using namespace std;
namespace utf8 {

/// Return current Unicode code point.
char32_t rune (std::string::const_iterator p)
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

/// Return current Unicode code point.
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

}  //namespace utf8
