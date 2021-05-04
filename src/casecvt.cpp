/*!
  \file casecvt.cpp Implementation case folding functions
*/
/*
  (c) Mircea Neacsu 2014-2020. Licensed under MIT License.
  See README file for full license terms.
*/

#include <utf8/utf8.h>
#include <algorithm>
#include <assert.h>
#include <ctype.h>

using namespace std;


namespace utf8 {

/*!
  \defgroup folding Character Folding Functions
  Conversion between upper case and lower case letters.
  
  toupper() and tolower() functions use standard tables published by Unicode
  Consortium to perform case folding.
  There is also a function icompare() that performs string comparison ignoring the case.

  A small ancillary program (gen_casetab) converts the original table
  in two tables of equal size, one with the upper case letters and the other
  with the lower case ones. The upper case table is sorted to allow for
  binary searching. If a code is found in the upper case table, it is replaced
  with the matching code from the lower case.

  Case folding tables take about 22k. Finding a code takes at most 11 comparisons.
*/

//definition of 'u2l' and 'lc' tables
#include "uppertab.c"

// definition of 'l2u' and 'uc' tables
#include "lowertab.c"

/// Return `true` if character is a lowercase character
/// \param p pointer to character to check
bool islower (const char* p)
{
  //trivial case - default to standard function
  if ((unsigned char)*p <= 0x7f)
    return ::islower (*p);

  //search character in lowercase table
  char32_t r = rune (p);
  char32_t* f = lower_bound (begin (l2u), end (l2u), r);
  return (f != end (l2u) && *f == r);
}

/*!
  Convert UTF-8 string to lower case using case folding table published by
  Unicode Consortium. (http://www.unicode.org/Public/12.1.0/ucd/CaseFolding.txt)

  \param str UTF-8 string to convert to lowercase.
  \return lower case UTF-8 string
*/

std::string tolower (const std::string& str)
{
  u32string wstr = runes (str);
  for (auto ptr = wstr.begin (); ptr < wstr.end (); ptr++)
  {
    char32_t *f = lower_bound (begin (u2l), end (u2l), *ptr);
    if (f != end (u2l) && *f == *ptr)
      *ptr = lc[f - u2l];
  }
  return narrow (wstr);
}

/// In place version converts a UTF-8 encoded string to lowercase
void tolower (std::string& str)
{
  str = tolower (const_cast<const string&>(str));
}

/// Return `true` if character is an uppercase character
/// \param p pointer to character to check
bool isupper (const char* p)
{
  //trivial case - default to standard function
  if ((unsigned char)*p <= 0x7f)
    return ::isupper (*p);

  //search character in uppercase table
  char32_t r = rune (p);
  char32_t* f = lower_bound (begin (u2l), end (u2l), r);
  return (f != end (u2l) && *f == r);
}

/*!
  Convert UTF-8 string to upper case using case folding table published by
  Unicode Consortium. (http://www.unicode.org/Public/12.1.0/ucd/CaseFolding.txt)

  \param str UTF-8 string to convert to uppercase.
  \return upper case UTF-8 string
*/
std::string toupper (const std::string& str)
{
  u32string wstr = runes (str);
  for (auto ptr = wstr.begin(); ptr < wstr.end(); ptr++)
  {
    char32_t *f = lower_bound (begin (l2u), end (l2u), *ptr);
    if (f != end(l2u) && *f == *ptr)
      *ptr = uc[f - l2u];
  }
  return narrow (wstr);
}

/// In place version converts a UTF-8 encoded string to lowercase
void toupper (std::string& str)
{
  str = toupper (const_cast<const string&>(str));
}

/*!
  Compare two strings in a case-insensitive way.

  \param s1 first string
  \param s2 second string
  \return <0 if first string is lexicographically before the second one
  \return >0 if first string is lexicographically after the second string
  \return =0 if the two strings are equal

  Strings must be valid UTF8 strings.
*/
int icompare (const std::string& s1, const std::string& s2)
{
  assert (valid (s1) && valid (s2));

  auto p1 = s1.begin (), p2 = s2.begin ();
  while (p1 < s1.end () && p2 < s2.end())
  {
    char32_t lc1, lc2, c1 = rune(p1), c2 = rune(p2);
    char32_t* f = lower_bound (begin (u2l), end (u2l), c1);
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

    next (s1, p1);
    next (s2, p2);
  }
  if (p1 != s1.end ())
    return 1;
  if (p2 != s2.end ())
    return -1;
  return 0;
}

}
