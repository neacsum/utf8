/*
  Copyright (c) Mircea Neacsu (2014-2024) Licensed under MIT License.
  This is part of UTF8 project. See LICENSE file for full license terms.
*/

///  \file casecvt.cpp Implementation case folding functions

#include <utf8/utf8.h>
#include <algorithm>
#include <assert.h>
#include <ctype.h>

using namespace std;

namespace utf8 {

/*!
  \defgroup folding Character Folding Functions
  Conversion between upper case and lower case letters.

  toupper() and tolower() functions and their in-place counterparts
  make_upper() and make_lower(), use standard tables published by Unicode
  Consortium to perform case folding.
  There is also a function, icompare(), that performs string comparison ignoring
  the case.

  If input strings are not valid UTF-8 encoded strings, these function will
  throw a utf8::exception.

  A small ancillary program (gen_casetab) converts the original table
  in two tables of equal size, one with the upper case letters and the other
  with the lower case ones. The upper case table is sorted to allow for
  binary searching. If a code is found in the upper case table, it is replaced
  with the matching code from the lower case.

  Case folding tables take about 22k. Finding a code takes at most 11 comparisons.
*/

//definition of 'u2l' and 'lc' tables
#include "uppertab.h"

// definition of 'l2u' and 'uc' tables
#include "lowertab.h"


/// Return `true` if character is a lowercase character
/// \param r character to check
bool islower (char32_t r)
{
  //search character in lowercase table
  auto f = lower_bound (begin (l2u), end (l2u), r);
  return (f != end (l2u) && *f == r);
}

/// Return `true` if character is a lowercase character
/// \param p pointer to character to check
bool islower (const char* p)
{
  //trivial case - default to standard function
  if ((unsigned char)*p <= 0x7f)
    return ::islower (*p);

  return islower (rune (p));
}

/*!
  Convert UTF-8 string to lower case.

  \param str UTF-8 string to convert to lowercase.
  \return lower case UTF-8 string

  Uses case mapping table published by Unicode Consortium
  (https://www.unicode.org/Public/UCD/latest/ucd/UnicodeData.txt)
*/

std::string tolower (const std::string& str)
{
  u32string wstr;
  auto ptr = str.begin ();
  while (ptr < str.end ())
  {
    auto c = next (ptr, str.end ());
    auto f = lower_bound (begin (u2l), end (u2l), c);
    if (f != end (u2l) && *f == c)
      wstr.push_back (lc[f - u2l]);
    else
      wstr.push_back (c);
  }
  return narrow (wstr);
}

/*!
  In place version converts a UTF-8 encoded string to lowercase
  \param str  UTF-8 encoded string to be converted

  Note that, in general, the size of the resulting string will be different from
  that of the original string.
*/
void make_lower (std::string& str)
{
  str = tolower (str);
}

/// Return `true` if character is an uppercase character
/// \param r character to check
bool isupper (char32_t r)
{
  //search character in uppercase table
  auto f = lower_bound (begin (u2l), end (u2l), r);
  return (f != end (u2l) && *f == r);
}

/// Return `true` if character is an uppercase character
/// \param p pointer to character to check
bool isupper (const char* p)
{
  //trivial case - default to standard function
  if ((unsigned char)*p <= 0x7f)
    return ::isupper (*p);

  return isupper(rune (p));
}

/*!
  Convert a UTF-8 string to upper case.

  \param str UTF-8 string to convert to uppercase.
  \return upper case UTF-8 string

  Uses case mapping table published by Unicode Consortium
  (https://www.unicode.org/Public/UCD/latest/ucd/UnicodeData.txt)
*/
std::string toupper (const std::string& str)
{
  u32string wstr;
  auto ptr = str.begin();
  while (ptr < str.end())
  {
    auto c = next (ptr, str.end ());
    auto f = lower_bound (begin (l2u), end (l2u), c);
    if (f != end (l2u) && *f == c)
      wstr.push_back (uc[f - l2u]);
    else
      wstr.push_back (c);
  }
  return narrow (wstr);
}

/*!
  In place version converts a UTF-8 encoded string to lowercase.
  \param str  string to be converted

  Note that, in general, the size of the resulting string will be different from
  that of the original string.
*/
void make_upper (std::string& str)
{
  str = toupper (str);
}

/*!
  Compare two strings in a case-insensitive way.

  \param s1 first string
  \param s2 second string
  \return <0 if first string is lexicographically before the second one
  \return >0 if first string is lexicographically after the second string
  \return =0 if the two strings are equal

  Strings must be valid UTF-8 strings.
*/
int icompare (const std::string& s1, const std::string& s2)
{
  assert (valid_str (s1) && valid_str (s2));

  auto p1 = begin (s1), p2 = begin (s2);
  while (p1 < end (s1) && p2 < end (s2))
  {
    char32_t lc1, lc2,
      c1 = next (p1, end (s1)),
      c2 = next (p2, end (s2));

    auto f = lower_bound (begin (u2l), end (u2l), c1);
    if (f != end (u2l) && *f == c1)
      lc1 = lc[f - u2l];
    else
      lc1 = c1;
    f = lower_bound (begin (u2l), end (u2l), c2);
    if (f != end (u2l) && *f == c2)
      lc2 = lc[f - u2l];
    else
      lc2 = c2;
    if ((lc1 != lc2))
      return (lc1 < lc2)? -1 : 1;
  }
  if (p1 != end (s1))
    return 1;
  if (p2 != end (s2))
    return -1;
  return 0;
}

}
